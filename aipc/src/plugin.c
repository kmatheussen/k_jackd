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
				

#include <pthread.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#include <aipc/plugin.h>
#include <aipc/pipe.h>
#include <aipc/sharedmem.h>
#include <aipc/lock.h>

#include "plugin.h"




struct Private{
  struct aipc_pipe *pipe;
  struct aipc_pipe *control_pipe;

  struct aipc_sharedmem *sms;
  struct PluginAndHost *pah;
  pthread_t call_thread;
  char *filenames_prefix;

  AipcPluginCallback call;

  int receivedatasize;
  void *receivedata;

  void *arg;

  int returndatasize_max;
};



static void *EXCHANGE_call_thread(void *pointer){
  struct aipc_plugin *ep=(struct aipc_plugin*)pointer;
  struct Private *p=(struct Private*)ep->private;
  struct PluginAndHost *pah=p->pah;

  for(;;){
    int num_read=aipc_pipe_receive(p->pipe,p->receivedata,p->receivedatasize);

    if(num_read==0){
      fprintf(stderr,"libaipc: plugin.c/EXCHANGE_call_thread: Seems like host has died. Exiting.\n");
      goto exit;
    }

    if(pah->h2p_datasize<=0){
      if(pah->h2p_datasize==0 || pah->h2p_datasize==-1){
	printf("libaipc: plugin.c/EXCHANGE_call_thread: received delete.\n");
	if(pah->h2p_datasize==-1){
	  if(aipc_pipe_send_int(p->control_pipe,0)==false){
	    fprintf(stderr,"libaipc: plugin.c/EXCHANGE_call_thread: Shutdownproblem. Bad.\n");
	  }
	}
	goto exit;
      }else{
	p->receivedatasize=-pah->h2p_datasize;
	p->receivedata=realloc(p->receivedata,p->receivedatasize);
	aipc_pipe_send_int(p->pipe,1);
	continue;
      }
    }

    if( num_read != pah->h2p_datasize ){
      fprintf(
	      stderr,
	      "libaipc: plugin.c/EXCHANGE_call_thread: num_read not %d but %d.\n",
	      pah->h2p_datasize,num_read
	      );
      aipc_pipe_send_int(p->pipe,1);
      continue;
    }

    {
      int dummy;
      int returndatasize=p->receivedatasize;
      void *returndata=p->call(ep,p->receivedatasize,p->receivedata,&returndatasize,p->arg);

      if(returndata==NULL){
	returndata=&dummy;
	returndatasize=sizeof(int);
	pah->p2h_datasize=0;
      }else{
	if(returndatasize>p->returndatasize_max){
	  pah->p2h_datasize=-returndatasize;
	  aipc_pipe_send_int(p->pipe,1);
	  aipc_pipe_receive_int(p->pipe,&dummy);
	  p->returndatasize_max=returndatasize;
	}
	pah->p2h_datasize=returndatasize;
      }

      if(aipc_pipe_send(p->pipe,returndata,returndatasize)==false){
	fprintf(
		stderr,
		"libaipc: plugin.c/EXCHANGE_call_thread: Unable to send back %d bytes back to host.\n",
		returndatasize
		);
      }
    }

  }

  
 exit:
  return NULL;
}



void aipc_plugin_delete(struct aipc_plugin *ep){
  struct Private *p=(struct Private*)ep->private;
  char temp[500];
  struct aipc_lock *shutdownlock;

  if(p->pah!=NULL){
    int sleeptime=0;
    p->pah->okey_to_call=0;
    while(p->pah->is_calling==1){
      usleep(200);
      sleeptime+=200;
      if(sleeptime>50000){
	fprintf(stderr,"aipc_plugin_delete: Waited 5 seconds for calling to stop. Thats too much. Something is wrong. Giving up.\n");
	break;
      }
    }
  }


  if(p->pah!=NULL){
    sprintf(temp,"%s%s",p->filenames_prefix,SHUTDOWN_LOCKNAME);
    shutdownlock=aipc_lock_new(temp,0);
    
    if(p->pah->status==0){
      if(aipc_pipe_send_int(p->control_pipe,-1)==false){
	fprintf(stderr,"Shutdownproblem,plugin, bad\n");
      }else{
	p->pah->status=1;
      }
    }
    aipc_lock_delete(shutdownlock);
  }


  if(p->call_thread!=0){
    pthread_join(p->call_thread,NULL);
  }

  if(p->pipe!=NULL) aipc_pipe_delete(p->pipe);
  if(p->control_pipe!=NULL) aipc_pipe_delete(p->control_pipe);

  if(p->sms!=NULL) aipc_sharedmem_delete(p->sms);

  if(p->filenames_prefix!=NULL) free(p->filenames_prefix);

  free(p->receivedata);
  free(p);

  free(ep);
}



struct aipc_plugin *aipc_plugin_new(
				    char *filenames_prefix,
				    AipcPluginCallback call,
				    void *arg,
				    int sizeof_shared_memarea,
				    int timeout
				    )
{

  struct aipc_plugin *ep=calloc(1,sizeof(struct aipc_plugin));
  struct Private *p=calloc(1,sizeof(struct Private));


  char pipename[500];
  char controlpipename[500];
  char sharedmem_prefix[500];
  int result;


  p->call=call;
  p->arg=arg;

  ep->private=p;
  ep->sizeof_shared_memarea=sizeof_shared_memarea;

  p->receivedatasize=sizeof(int);
  p->receivedata=calloc(1,sizeof(int));
  p->returndatasize_max=sizeof(int);


  /**** Setting up directory, pipes and lock names. *****/

  p->filenames_prefix=strdup(filenames_prefix);

  sprintf(pipename,"%s%s",p->filenames_prefix,EXCHANGE_PIPENAME);
  sprintf(controlpipename,"%s%s",p->filenames_prefix,EXCHANGE_CONTROL_PIPENAME);

  sprintf(sharedmem_prefix,"%s%s",p->filenames_prefix,SHAREDMEM_PREFIX);


  /**** Try to make connection with host. *****/

  if((p->pipe=aipc_pipe_new(pipename,timeout))==NULL){
    fprintf(stderr,"aipc_plugin_new: Could not open pipe.\n");
    aipc_plugin_delete(ep);
    return NULL;
  }

  if((p->control_pipe=aipc_pipe_new(controlpipename,timeout))==NULL){
    fprintf(stderr,"aipc_plugin_new: Could not open control pipe.\n");
    aipc_plugin_delete(ep);
    return NULL;
  }



  /******* Set up shared memory. ***********/

  p->sms=aipc_sharedmem_new(
			    sharedmem_prefix,
			    sizeof(struct PluginAndHost)
			    + sizeof_shared_memarea,
			    0
			    );
  if(p->sms==NULL){
    fprintf(stderr,"aipc_plugin_new: Unable to allocate shared memory.\n");
    aipc_plugin_delete(ep);
    return NULL;
  }

  p->pah=(struct PluginAndHost*)p->sms->addr;
  ep->shared_memarea=(p->sms->addr + sizeof(struct PluginAndHost));

  p->pah->status=0;
  p->pah->is_calling=0;
  p->pah->okey_to_call=0;



  /****** Receive status from host. **********/

  aipc_pipe_receive_int(p->pipe,&result);
  if(result==0){
    fprintf(stderr,"aipc_plugin_new: Host was unable to make new thread.\n");
    aipc_plugin_delete(ep);
    return NULL;
  }



  /******* Setup call thread. *******/

  if(pthread_create(&p->call_thread,NULL,EXCHANGE_call_thread,ep)!=0){
    fprintf(stderr,"aipc_plugin_new: Unable to make new thread.\n");
    aipc_pipe_send_int(p->pipe,0);
    aipc_plugin_delete(ep);
    return NULL;
  }



  /****** Tell host that everything is okey. **********/

  aipc_pipe_send_int(p->pipe,1);


  return ep;

}


