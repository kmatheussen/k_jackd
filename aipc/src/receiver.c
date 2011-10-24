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

#include <stdio.h>
#include <stdlib.h>

#include <aipc/lock.h>
#include <aipc/input.h>
#include <aipc/output.h>

#include <aipc/receiver.h>



void aipc_receiver_delete(struct aipc_receiver *receiver){
  if(receiver->input!=NULL) aipc_input_delete(receiver->input);
  if(receiver->data!=NULL) free(receiver->data);
  free(receiver);
}


struct aipc_receiver *aipc_receiver_new(char *filenames_prefix){
  struct aipc_receiver *receiver=calloc(1,sizeof(struct aipc_receiver));
  char temp[500];
  sprintf(temp,"%s_aipc_receiver_socket",filenames_prefix);

  receiver->input=aipc_input_new(temp,-2);
  if(receiver->input==NULL){
    fprintf(stderr,"aipc_receiver_new: Could not make input socket \"%s\".\n",temp);
    aipc_receiver_delete(receiver);
    return NULL;
  }

  receiver->data=malloc(sizeof(int));
  receiver->sizeof_data=sizeof(int);

  return receiver;
}




void *aipc_receiver_receive(
			    struct aipc_receiver *receiver,
			    int *sizeof_returndata
			    )
{
  int size;
  int num_bytes;

  while(aipc_input_accept_incoming_connection(receiver->input)==false);

  size=aipc_input_receive(receiver->input,&num_bytes,sizeof(int));
  if(size!=sizeof(int)){
    fprintf(stderr,"aipc_receiver_receive: Received %d intead of %d bytes. Is client dead?\n",size,sizeof(int));
    return NULL;
  }
  if(num_bytes>receiver->sizeof_data){
    receiver->data=realloc(receiver->data,num_bytes);
    receiver->sizeof_data=num_bytes;
  }

  size=aipc_input_receive(receiver->input,receiver->data,num_bytes);

  if(size!=num_bytes){
    fprintf(stderr,"aipc_receiver_receive: Received %d bytes instead of %d. Is client dead 2?\n",size,num_bytes);
    return NULL;
  }

  *sizeof_returndata=num_bytes;

  return receiver->data;
}



bool aipc_receiver_send(
			 char *filenames_prefix,
			 void *data,
			 int sizeof_data
			 )
{
  struct aipc_output *output=NULL;
  struct aipc_lock *lock=NULL;
  bool ret=false;
  char lockname[500];
  char socketname[500];
  sprintf(socketname,"%s_aipc_receiver_socket",filenames_prefix);
  sprintf(lockname,"%s_aipc_receiver_lock",filenames_prefix);

  lock=aipc_lock_new(lockname,0);
  if(lock==NULL){
    fprintf(stderr,"aipc_receiver_send: Unable to open lock file \"%s\".\n",lockname);
    goto exit;
  }

  output=aipc_output_new(socketname,-2);
  if(output==NULL){
    fprintf(stderr,"aipc_receiver_send: Unable to contact \"%s\".\n",socketname);
    goto exit;
  }

  if(aipc_output_send_int(output,sizeof_data)==false){
    fprintf(stderr,"aipc_receiver_send: Could not send to receiver \"%s\".\n",socketname);
    goto exit;
  }

  if(aipc_output_send(output,data,sizeof_data)!=sizeof_data){
    fprintf(stderr,"aipc_receiver_send: Could not send to receiver \"%s\". 2\n",socketname);
    goto exit;
  }

  ret=true;

 exit:
  if(output!=NULL) aipc_output_delete(output);
  if(lock!=NULL) aipc_lock_delete(lock);
  return ret;

}





