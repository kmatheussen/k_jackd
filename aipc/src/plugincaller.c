/*
    Copyright (C) 2003 Kjetil S. Matheussen / Notam.
    
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation; either version 2.1 of the License, or
    (at your option) any later version.
    
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.
    
    You should have received a copy of the GNU Lesser General Public License
    along with this program; if not, write to the Free Software 
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
    
*/



#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <errno.h>

#include <aipc/pipe.h>
#include <aipc/plugincaller.h>
#include <aipc/input.h>
#include <aipc/sharedmem.h>
#include <aipc/lock.h>

#include "plugin.h"



struct Private{
  struct aipc_pipe *pipe;
  struct aipc_pipe *control_pipe;

  pthread_t control_thread;

  struct PluginAndHost *pah;
  struct aipc_sharedmem *sms;
  char *filenames_prefix;

  int senddatasize_max;
  int receivedatasize;
  void *receivedata;
};



void *aipc_plugincaller_call_plugin(
		    struct aipc_plugincaller *eh,
		    int sizeof_data_to_send,
		    void *data_to_send,
		    int *sizeof_data_returned
		    )
{
  struct Private *p=(struct Private *)eh->private;
  struct PluginAndHost *pah=p->pah;
  int num_read;

  pah->is_calling=1;
  if(p->pah->okey_to_call==0){
    p->pah->is_calling=0;
    return NULL;
  }

  if(sizeof_data_to_send > p->senddatasize_max){
    int dummy;
    pah->h2p_datasize=-sizeof_data_to_send;
    aipc_pipe_send_int(p->pipe,0);
    aipc_pipe_receive_int(p->pipe,&dummy);
    p->senddatasize_max=sizeof_data_to_send;
  }

  pah->h2p_datasize=sizeof_data_to_send;
  if(aipc_pipe_send(p->pipe,data_to_send,sizeof_data_to_send)==false){
    fprintf(stderr,"aipc_plugincaller_call_plugin: Unable to send %d bytes to plugin.\n",sizeof_data_to_send);
  }

  num_read=aipc_pipe_receive(p->pipe,p->receivedata,p->receivedatasize);

  if(num_read==0){
    fprintf(stderr,"aipc_plugincaller_call_plugin: Seems like plugin has died.\n");
    return NULL;
  }


  if(pah->p2h_datasize<0){
    p->receivedatasize=-pah->p2h_datasize;
    p->receivedata=realloc(p->receivedata,p->receivedatasize);

    aipc_pipe_send_int(p->pipe,1);

    num_read=aipc_pipe_receive(p->pipe,p->receivedata,p->receivedatasize);
  }


  if(sizeof_data_returned!=NULL){
    *sizeof_data_returned=pah->p2h_datasize;
  }

  pah->is_calling=0;


  return p->receivedata;
}


static void *EXCHANGE_control_thread(void *pointer){
  struct aipc_plugincaller *eh=(struct aipc_plugincaller *)pointer;
  struct Private *p=(struct Private *)eh->private;
  struct PluginAndHost *pah=p->pah;

  for(;;){
    int num_frames;
    int num_read=aipc_pipe_receive_int(p->control_pipe,&num_frames);
    if(num_read!=sizeof(int)){
      if(num_read==0){
	fprintf(stderr,"libaipc/EXCHANGE_control_thread: Seems like plugin has died. Exiting.\n");
	goto exit;
      }else{
	fprintf(stderr,"libaipc/EXCHANGE_control_thread: num_read not %d but %d.\n",sizeof(int),num_read);
      }
      continue;
    }
    if(num_frames==0 || num_frames==-1){
      printf("libaipc/EXCHANGE_control_thread: received delete.\n");
      if(num_frames==-1){
	pah->h2p_datasize=0;
	if(aipc_pipe_send_int(p->pipe,0)==false){
	  fprintf(stderr,"libaipc/EXCHANGE_control_thread: Shutdown problem. Bad.\n");
	}
      }
      goto exit;
    }else{
      fprintf(stderr,"libaipc/EXCHANGE_control_thread: Strange message from plugin %d.\n",num_frames);
    }
  }

 exit:
  return NULL;
  
}

void aipc_plugincaller_delete(struct aipc_plugincaller *eh){
  struct Private *p=(struct Private*)eh->private;
  struct aipc_lock *shutdownlock;
  char temp[500];

  if(p->pah!=NULL){
    int sleeptime=0;
    p->pah->okey_to_call=0;
    while(p->pah->is_calling==1){
      usleep(200);
      sleeptime+=201;
      if(sleeptime>50000){
	fprintf(stderr,"aipc_plugincaller_delete: Waited 5 seconds for processing to stop. Thats too much. Something is wrong. Giving up.\n");
	break;
      }
    }
  }

  if(p->pah!=NULL){
    sprintf(temp,"%s%s",p->filenames_prefix,SHUTDOWN_LOCKNAME);
    shutdownlock=aipc_lock_new(temp,0);

    if(p->pah->status==0){
      p->pah->h2p_datasize=-1;
      if(aipc_pipe_send_int(p->pipe,1)==false){
	fprintf(stderr,"aipc_plugincaller_delete: Shutdownproblem. Bad. (%s)\n",strerror(errno));
      }else{
	p->pah->status=1;
      }
    }
    aipc_lock_delete(shutdownlock);
  }

  if(p->control_thread!=0){
    pthread_join(p->control_thread,NULL);
  }

  if(p->pipe!=NULL) aipc_pipe_delete(p->pipe);
  if(p->control_pipe!=NULL) aipc_pipe_delete(p->control_pipe);

  if(p->sms!=NULL)
    aipc_sharedmem_delete(p->sms);

  if(p->filenames_prefix!=NULL) free(p->filenames_prefix);

  free(p->receivedata);
  free(p);

  free(eh);

}



struct aipc_plugincaller *aipc_plugincaller_new(
						char *filenames_prefix,
						int timeout
			 	     )
{
  struct aipc_plugincaller *eh=calloc(1,sizeof(struct aipc_plugincaller));
  struct Private *p=calloc(1,sizeof(struct Private));

  char pipename[500];
  char controlpipename[500];
  char sharedmem_prefix[500];
  int result;

  eh->private=p;

  p->senddatasize_max=sizeof(int);
  p->receivedatasize=sizeof(int);
  p->receivedata=calloc(1,sizeof(int));

  /**** Set up socket and lock names. *****/

  p->filenames_prefix=strdup(filenames_prefix);

  sprintf(pipename,"%s%s",p->filenames_prefix,EXCHANGE_PIPENAME);
  sprintf(controlpipename,"%s%s",p->filenames_prefix,EXCHANGE_CONTROL_PIPENAME);

  sprintf(sharedmem_prefix,"%s%s",p->filenames_prefix,SHAREDMEM_PREFIX);


  /**** Try to make connection with plugin. *****/

  if((p->pipe=aipc_pipe_new(pipename,timeout))==NULL){
    fprintf(stderr,"aipc_plugincaller_new: Could not open pipe.\n");
    aipc_plugincaller_delete(eh);
    return NULL;
  }

  if((p->control_pipe=aipc_pipe_new(controlpipename,timeout))==NULL){
    fprintf(stderr,"aipc_plugincaller_new: Could not open control pipe.\n");
    aipc_plugincaller_delete(eh);
    return NULL;
  }



  /********** Set up shared memory. ****************/

  p->sms=aipc_sharedmem_new(sharedmem_prefix,-1,timeout);
  if(p->sms==NULL){
    fprintf(stderr,"aipc_plugincaller_new: Could not get shared memory.\n");
    aipc_pipe_send_int(p->pipe,0);
    aipc_plugincaller_delete(eh);
    return NULL;
  }

  p->pah=(struct PluginAndHost*)p->sms->addr;
  eh->shared_memarea=(p->sms->addr + sizeof(struct PluginAndHost));
  eh->sizeof_shared_memarea=p->sms->size;


  /****** Set up control thread to receive messages from plugin. ******/

  if(pthread_create(&p->control_thread,NULL,EXCHANGE_control_thread,eh)!=0){
    fprintf(stderr,"aipc_plugincaller_new: Unable to make new thread.\n");
    aipc_pipe_send_int(p->pipe,0);
    aipc_plugincaller_delete(eh);
    return NULL;
  }



  /****** Tell plugin that everything is okey. **********/

  aipc_pipe_send_int(p->pipe,1);



  /****** Receive confirmation from plugin. **********/

  aipc_pipe_receive_int(p->pipe,&result);

  if(result==0){
    fprintf(stderr,"aipc_plugincaller_new: Plugin was unable to make new thread..\n");
    aipc_plugincaller_delete(eh);
    return NULL;
  }


  p->pah->okey_to_call=1;

  return eh;
}





