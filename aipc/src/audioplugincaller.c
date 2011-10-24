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
#include <string.h>

#include <aipc/plugincaller.h>
#include <aipc/audioplugincaller.h>
#include <aipc/simpleio.h>

#include "audioplugin.h"



#ifdef MIN
#  undef MIN
#endif
#define MIN(a,b) (((a)<(b))?(a):(b))



struct Private{
  struct aipc_plugincaller *eh;

  float **inputs;
  float **outputs;
};



bool aipc_audioplugincaller_call_audioplugin(
	       struct aipc_audioplugincaller *aeh,
	       float **inputs,
	       float **outputs,
	       int num_frames
	       )
{
  int process_place=0;
  struct Private *p=(struct Private *)aeh->private;

  do{
    int lokke;
    int num_to_process=MIN(BUFFERSIZE,num_frames);

    for(lokke=0;lokke<aeh->num_inputs;lokke++){
      memcpy(p->inputs[lokke],inputs[lokke]+process_place,sizeof(float)*num_to_process);
    }

    if(aipc_plugincaller_call_plugin(
			   p->eh,
			   sizeof(int),
			   &num_to_process,
			   NULL
			   )
       ==NULL
       )
      {
	return false;
      }
    
    for(lokke=0;lokke<aeh->num_outputs;lokke++){
      memcpy(outputs[lokke]+process_place,p->outputs[lokke],sizeof(float)*num_to_process);
    }

    num_frames-=num_to_process;
    process_place+=num_to_process;

  }while(num_frames>0);


  return true;
}


void aipc_audioplugincaller_delete(struct aipc_audioplugincaller *aeh){
  struct Private *p=(struct Private *)aeh->private;

  if(p->eh!=NULL) aipc_plugincaller_delete(p->eh);

  free(p);
  free(aeh);
}



struct aipc_audioplugincaller *aipc_audioplugincaller_new(
							  char *filenames_prefix,
							  int timeout
							  )
{
  struct aipc_audioplugincaller *aeh=calloc(1,sizeof(struct aipc_audioplugincaller));
  struct Private *p=calloc(1,sizeof(struct Private));
  struct Plugin2Host p2h;
  char temp[500];

  aeh->private=p;

  p->eh=aipc_plugincaller_new(filenames_prefix,timeout);
  if(p->eh==NULL){
    aipc_audioplugincaller_delete(aeh);
    return NULL;
  }

  sprintf(temp,"%s%s",filenames_prefix,PIPENAME);

  if( (aipc_simpleio_receive(temp,&p2h,sizeof(struct Plugin2Host))) != sizeof(struct Plugin2Host)){
    fprintf(stderr,"aipc_audioplugincaller_new: Unable to receive information from Plugin.\n");
    aipc_audioplugincaller_delete(aeh);
    return NULL;
  }

  aeh->num_inputs=p2h.num_inputs;
  aeh->num_outputs=p2h.num_outputs;


  if(aeh->num_inputs>0){
    int lokke;
    float *mem=(float *) p->eh->shared_memarea;
    p->inputs=malloc(sizeof(float *) * aeh->num_inputs);

    for(lokke=0;lokke<aeh->num_inputs;lokke++){
      p->inputs[lokke]=mem + (lokke*BUFFERSIZE);
    }
  }

  if(aeh->num_outputs>0){
    int lokke;
    float *mem=(float *) p->eh->shared_memarea;
    p->outputs=malloc(sizeof(float*) * aeh->num_outputs);

    for(lokke=aeh->num_inputs;lokke < aeh->num_inputs + aeh->num_outputs;lokke++){
      p->outputs[lokke - aeh->num_inputs]=mem + (lokke*BUFFERSIZE);
    }
  }


  return aeh;
}






