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

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


#include <aipc/pipe.h>


static struct aipc_pipe *aipc_pipe_new_internal(
						char *filenames_prefix,
						int timeout,
						int input_id,
						int output_id
						)
{
  struct aipc_pipe *pipe=calloc(1,sizeof(struct aipc_pipe));

  if( (pipe->input=aipc_input_new(filenames_prefix,input_id))==NULL){
    fprintf(
	    stderr,
	    "k_pipe.c/aipc_pipe_new_interal: Could not open input communicate %s%d\n",
	    filenames_prefix,
	    input_id
	    );
    free(pipe);
    return NULL;
  }

  if((pipe->output=aipc_output_new_wait(filenames_prefix,output_id,timeout))==NULL){
    fprintf(
	    stderr,
	    "k_pipe.c/aipc_pipe_new_interal: Could not open output communicate %s%d\n",
	    filenames_prefix,
	    output_id
	    );
    aipc_input_delete(pipe->input);
    free(pipe);
    return NULL;
  }

  if(aipc_input_accept_incoming_connection(pipe->input)==false){
    fprintf(
	    stderr,
	    "k_pipe.c/aipc_pipe_new_interal: Failed to accept incoming connection for %s%d\n",
	    filenames_prefix,
	    input_id
	    );
    aipc_input_delete(pipe->input);
    aipc_output_delete(pipe->output);
    free(pipe);
    return NULL;
  }

  return pipe;
}

struct aipc_pipe *aipc_pipe_new(
				char *filenames_prefix,
				int timeout
				)
{
  struct aipc_pipe *pipe;
  int fd;
  char temp[500];
    
  sprintf(temp,"%sfilenames_prefix_pipe.bool",filenames_prefix);
  fd=open(temp,O_RDWR|O_CREAT|O_EXCL,S_IRWXU);

  if(fd==-1){
    pipe=aipc_pipe_new_internal(filenames_prefix,timeout,0,1);
  }else{
    pipe=aipc_pipe_new_internal(filenames_prefix,timeout,1,0);
    close(fd);
  }

  unlink(temp);

  return pipe;
}


void aipc_pipe_delete(struct aipc_pipe *pipe){
  aipc_output_delete(pipe->output);
  aipc_input_delete(pipe->input);
  free(pipe);
}

bool aipc_pipe_send(struct aipc_pipe *pipe,void *buf,int size){
  if(aipc_output_send(pipe->output,buf,size)!=size) return false;
  return true;
}


bool aipc_pipe_send_int(struct aipc_pipe *pipe,int val){
  return aipc_output_send_int(pipe->output,val);
}

int aipc_pipe_receive(struct aipc_pipe *pipe,void *buf,int size){
  return aipc_input_receive(pipe->input,buf,size);
}

int aipc_pipe_receive_int(struct aipc_pipe *pipe,int *dasint){
  return aipc_pipe_receive(pipe,dasint,sizeof(int));
}



