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
#include <string.h>

#include <aipc/variable.h>
#include <aipc/simpleio.h>
#include <aipc/dir.h>
#include <aipc/receiver.h>
#include <aipc/audioplugin.h>

#include <jack/jack.h>

#include "jack.h"


void *jack_port_get_buffer (jack_port_t *port, jack_nframes_t num_frames){
  jack_client_t *client=port->client;
  struct aipc_audioplugin *audioplugin=client->audioplugin;

  if(port->isinput==true){
    return audioplugin->inputs[port->num];
  }

  return audioplugin->outputs[port->num];
}



static void AudiopluginCallback(
			 struct aipc_audioplugin *audioplugin,
			 int num_frames,
			 void *arg
			 )
{
  jack_client_t *client=(jack_client_t *)arg;

  client->process_callback(num_frames,client->process_arg);
}



/* Try to connect k_jackd */

int jack_activate (jack_client_t *client){
  char temp[500];
  char *dirname=NULL;
  int ret=-1;

  sprintf(temp,"%s/.k_jackd/datadir_",getenv("HOME"));
  dirname=aipc_dir_create(temp,-1);
  if(dirname==NULL) goto exit;

  client->c2j.reqtype=KJACK_new;
  sprintf(client->c2j.dirname,"%s",dirname);
  sprintf(client->c2j.name,"%s",client->name);

  sprintf(temp,"%s/.k_jackd/main_socket",getenv("HOME"));
  if(aipc_receiver_send(temp,&client->c2j,sizeof(struct Client2Jackd))==false){
    goto exit;
  }

  client->audioplugin=aipc_audioplugin_new(
				   dirname,
				   client->num_inputs,
				   client->num_outputs,
				   AudiopluginCallback,
				   client,
				   -1
				   );

  if(client->audioplugin==NULL) goto exit;

  ret=0;

 exit:
  if(dirname!=NULL) free(dirname);
  return ret;
}

int jack_deactivate (jack_client_t *client){
  char temp[500];
  client->c2j.reqtype=KJACK_delete;

  printf("About to deactivate.\n");
  sprintf(temp,"%s/.k_jackd/main_socket",getenv("HOME"));
  aipc_receiver_send(temp,&client->c2j,sizeof(struct Client2Jackd));

  aipc_audioplugin_delete(client->audioplugin);
  client->audioplugin=NULL;

  {
    int receive;
    sprintf(temp,"%ssimpleio",client->c2j.dirname);
    aipc_simpleio_receive(temp,&receive,sizeof(int));
  }

  printf("Deactivated.\n");

  return 0;
}




/* Allocate buffer. */
jack_client_t *jack_client_new (const char *client_name){
  jack_client_t *client=calloc(1,sizeof(struct _jack_client));
  client->name=client_name;

  return client;
}


int jack_client_close (jack_client_t *client){

  if(client->audioplugin!=NULL) jack_deactivate(client);

  free(client);

  return 0;
}



int jack_set_process_callback (jack_client_t *client, JackProcessCallback process_callback, void *arg){
  client->process_callback=process_callback;
  client->process_arg=arg;
  return 0;
}


/* Only used to set number of input and output ports. */
jack_port_t *jack_port_register (jack_client_t *client,
                                 const char *port_name,
                                 const char *port_type,
                                 unsigned long flags,
                                 unsigned long buffer_size)
{
  struct _jack_port *ret;

  if(flags&JackPortIsInput){
    ret=&client->input_ports[client->num_inputs];
    ret->isinput=true;
    ret->num=client->num_inputs;
    client->num_inputs++;
  }else{
    ret=&client->output_ports[client->num_outputs];
    ret->isinput=false;
    ret->num=client->num_outputs;
    client->num_outputs++;
  }

  ret->client=client;

  return ret;
}



int jack_port_unregister (jack_client_t *from, jack_port_t *to){
  return 0;
}




void jack_on_shutdown (jack_client_t *client, void (*function)(void *arg), void *arg){
  client->on_shutdown_function=function;
  client->on_shutdown_arg=arg;
}


unsigned long jack_get_sample_rate (jack_client_t *client){
  char temp[500];
  sprintf(temp,"%s/.k_jackd/sample_rate",getenv("HOME"));
  return aipc_variable_get_float(temp,-1);
}

jack_nframes_t jack_get_buffer_size (jack_client_t *client){
  char temp[500];
  sprintf(temp,"%s/.k_jackd/buffer_size",getenv("HOME"));
  return aipc_variable_get_int(temp,-1);
}








