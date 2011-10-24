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


#include <aipc/input.h>
#include <aipc/output.h>


#include <aipc/simpleio.h>



int aipc_simpleio_receive(
			    char *filenames_prefix,
			    void *data,
			    int maxsize
			    )
{
  int size;
  struct aipc_input *input;
  char temp[500];
  sprintf(temp,"%s_aipc_simpleio_socket",filenames_prefix);
  
  if((input=aipc_input_new(temp,-2))==NULL){
    fprintf(stderr,"aipc_simpleio_receive: Could not make input socket \"%s\".\n",temp);
    return -1;
  }

  if(aipc_input_accept_incoming_connection(input)==false){
    fprintf(stderr,"aipc_simpleio_receive: Could not accept input socket \"%s\". 2\n",temp);
    return -1;
  }

  size=aipc_input_receive(input,data,maxsize);

  aipc_input_delete(input);

  return size;
}



bool aipc_simpleio_send(
			char *filenames_prefix,
			void *data,
			int sizeof_data,
			int timeout
			)
{
  struct aipc_output *output=NULL;
  bool ret=false;

  char socketname[500];
  sprintf(socketname,"%s_aipc_simpleio_socket",filenames_prefix);

  if((output=aipc_output_new_wait(socketname,-2,timeout))==NULL){
    fprintf(stderr,"aipc_simpleio_send: Unable to open output socket \"%s\".\n",socketname);
    goto exit;
  }

  if(aipc_output_send(output,data,sizeof_data)!=sizeof_data){
    fprintf(stderr,"aipc_simpleio_send: Could not send to receiver \"%s\".\n",socketname);
    goto exit;
  }

  ret=true;

 exit:
  if(output!=NULL) aipc_output_delete(output);
  return ret;
}



