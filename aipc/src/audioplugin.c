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
#include <stdlib.h>
#include <stdio.h>

#include <aipc/plugin.h>
#include <aipc/audioplugin.h>
#include <aipc/simpleio.h>

#include "audioplugin.h"


struct Private{
  struct aipc_plugin *ep;

  AipcAudiopluginCallback process;

  void *arg;
};


void aipc_audioplugin_delete(struct aipc_audioplugin *aep){
  struct Private *p=(struct Private *)aep->private;

  if(p->ep!=NULL) aipc_plugin_delete(p->ep);

  free(p);
  free(aep);
}


static void *callFromHost(
			  struct aipc_plugin *ep,
			  int sizeof_data,
			  void *data,
			  int *sizeof_returndata,
			  void *arg
			  )
{
  struct aipc_audioplugin *aep=(struct aipc_audioplugin *)arg;
  struct Private *p=(struct Private *)aep->private;
  int *num_frames=(int*)data;

  p->process(aep,*num_frames,p->arg);

  return NULL;
}


struct aipc_audioplugin *aipc_audioplugin_new(
					 char *filenames_prefix,
					 int num_inputs,
					 int num_outputs,
					 AipcAudiopluginCallback process,
					 void *arg,
					 int timeout
					 )
{
  struct aipc_audioplugin *aep=calloc(1,sizeof(struct aipc_audioplugin));
  struct Private *p=calloc(1,sizeof(struct Private));

  struct Plugin2Host p2h;
  char temp[500];

  aep->private=p;
  aep->num_inputs=num_inputs;
  aep->num_outputs=num_outputs;

  p->process=process;
  p->arg=arg;

  p->ep=aipc_plugin_new(
			filenames_prefix,
			callFromHost,
			aep,
			(sizeof(float)*BUFFERSIZE*(num_inputs+num_outputs)),
			timeout
			);

  if(p->ep==NULL){
    aipc_audioplugin_delete(aep);
    return NULL;
  }

  if(num_inputs>0){
    int lokke;
    float *mem=(float *) p->ep->shared_memarea;
    aep->inputs=malloc(sizeof(float *) * num_inputs);

    for(lokke=0;lokke<num_inputs;lokke++){
      aep->inputs[lokke]=mem + (lokke*BUFFERSIZE);
    }
  }

  if(num_outputs>0){
    int lokke;
    float *mem=(float *) p->ep->shared_memarea;
    aep->outputs=malloc(sizeof(float*) * num_outputs);

    for(lokke=num_inputs;lokke < num_inputs + num_outputs;lokke++){
      aep->outputs[lokke - num_inputs]=mem + (lokke*BUFFERSIZE);
    }
  }

  p2h.num_inputs=num_inputs;
  p2h.num_outputs=num_outputs;

  sprintf(temp,"%s%s",filenames_prefix,PIPENAME);

  aipc_simpleio_send(temp,&p2h,sizeof(struct Plugin2Host),timeout);

  return aep;
}




