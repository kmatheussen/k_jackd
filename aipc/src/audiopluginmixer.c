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
#include <unistd.h>

#include <aipc/audiopluginmixer.h>


#ifdef MIN
#  undef MIN
#endif
#define MIN(a,b) (((a)<(b))?(a):(b))


static void setNumPuts(struct aipc_audiopluginmixer *audiopluginmixer){
  int num_inputs=0;
  int num_outputs=0;
  int lokke;

  for(lokke=0;lokke<audiopluginmixer->num_callers;lokke++){
    if(num_inputs<audiopluginmixer->callers[lokke]->num_inputs)
      num_inputs=audiopluginmixer->callers[lokke]->num_inputs;
    if(num_outputs<audiopluginmixer->callers[lokke]->num_outputs)
      num_outputs=audiopluginmixer->callers[lokke]->num_outputs;
  }

  audiopluginmixer->num_inputs=num_inputs;
  audiopluginmixer->num_outputs=num_outputs;
}


struct aipc_audiopluginmixer *aipc_audiopluginmixer_new(void){
  struct aipc_audiopluginmixer *audiopluginmixer=calloc(1,sizeof(struct aipc_audiopluginmixer));

  pthread_mutex_init(&audiopluginmixer->mutex,NULL);

  return audiopluginmixer;
}



void
aipc_audiopluginmixer_call_audioplugins(
					struct aipc_audiopluginmixer *audiopluginmixer,
					int num_inputs,
					float **inputs,
					int num_outputs,
					float **outputs,
					int num_frames
					)
{
  int lokke;
#if 1
  struct aipc_audioplugincaller *caller;
  float dummy[num_frames];
  bool dummy_is_cleared=false;
#endif
  audiopluginmixer->is_calling=1;

  lokke=0;


#if 0
  aipc_audioplugincaller_call_audioplugin(
					  audiopluginmixer->callers[0],
					  inputs,
					  outputs,
					  num_frames
					  );


#else
  for(;;lokke++){
    if(lokke >= audiopluginmixer->num_callers) break;
    caller=audiopluginmixer->callers[lokke];

    {
      float *inputs_proc[caller->num_inputs];
      float *outputs_proc[caller->num_outputs];
      float outputdata[caller->num_outputs*num_frames];
      int lokke2;

      for(lokke2=0;lokke2<caller->num_inputs;lokke2++){
	if(lokke2 < num_inputs){
	  inputs_proc[lokke2]=inputs[lokke2];
	}else{
	  inputs_proc[lokke2]=dummy;
	  if(dummy_is_cleared==false){
	    memset(&dummy,0,num_frames*sizeof(float));
	    dummy_is_cleared=true;
	  }
	}
      }

      for(lokke2=0;lokke2<caller->num_outputs;lokke2++){
	outputs_proc[lokke2]=&outputdata[lokke2*num_frames];
      }

      aipc_audioplugincaller_call_audioplugin(caller,inputs_proc,outputs_proc,num_frames);

      if(lokke==0){
	for(lokke2=0;lokke2<num_outputs;lokke2++){
	  if(lokke2<caller->num_outputs){
	    memcpy(outputs[lokke2],outputs_proc[lokke2],num_frames*sizeof(float));
	  }else{
	    memset(outputs[lokke2],0,num_frames*sizeof(float));
	  }
	}
      }else{
	for(lokke2=0;lokke2<MIN(num_outputs,caller->num_outputs);lokke2++){
	  int lokke3;
	  for(lokke3=0;lokke3<num_frames;lokke3++){
	    outputs[lokke2][lokke3] += outputs_proc[lokke2][lokke3];
	  }
	}
      }

    }


  }
  
#endif


  audiopluginmixer->is_calling=0;
}


bool
aipc_audiopluginmixer_add_caller(
				 struct aipc_audiopluginmixer *audiopluginmixer,
				 struct aipc_audioplugincaller *caller
				 )
{
  struct aipc_audioplugincaller **new_callers;
  struct aipc_audioplugincaller **old_callers;

  pthread_mutex_lock(&audiopluginmixer->mutex);

  new_callers=calloc(sizeof(struct aipc_audioplugincaller*),audiopluginmixer->num_callers+1);

  if(audiopluginmixer->num_callers>0){ // Is this check necesarry?
    memcpy(
	   new_callers,
	   audiopluginmixer->callers,
	   sizeof(struct aipc_audioplugincaller*)*audiopluginmixer->num_callers
	   );
  }

  new_callers[audiopluginmixer->num_callers]=caller;

  old_callers=audiopluginmixer->callers;
  audiopluginmixer->callers=new_callers;

  audiopluginmixer->num_callers++;

  setNumPuts(audiopluginmixer);

  free(old_callers);

  while(audiopluginmixer->is_calling==1) usleep(200);

  pthread_mutex_unlock(&audiopluginmixer->mutex);

  return true;
}


void
aipc_audiopluginmixer_remove_caller(
				    struct aipc_audiopluginmixer *audiopluginmixer,
				    struct aipc_audioplugincaller *caller
				    )
{
  int lokke;

  pthread_mutex_lock(&audiopluginmixer->mutex);

  for(lokke=0;lokke<audiopluginmixer->num_callers - 1;lokke++){
    if(audiopluginmixer->callers[lokke]==caller){
      audiopluginmixer->num_callers--;
      audiopluginmixer->callers[lokke]
	=
	audiopluginmixer->callers[audiopluginmixer->num_callers];

      setNumPuts(audiopluginmixer);

      while(audiopluginmixer->is_calling==1) usleep(200);      

      pthread_mutex_unlock(&audiopluginmixer->mutex);
      return;
    }
  }

  fprintf(stderr,"aipc_audiopluginmixer_remove_caller: caller not found.\n");

  pthread_mutex_unlock(&audiopluginmixer->mutex);
}




void 
aipc_audiopluginmixer_delete(
			     struct aipc_audiopluginmixer *audiopluginmixer
			     )
{

  pthread_mutex_destroy(&audiopluginmixer->mutex);

  free(audiopluginmixer->callers);
  free(audiopluginmixer);
}
