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



#ifndef AIPC_AUDIOPLUGINMIXER_H
#define AIPC_AUDIOPLUGINMIXER_H

#include <pthread.h>

#include <aipc/audioplugincaller.h>


struct aipc_audiopluginmixer{
  int num_inputs;
  int num_outputs;


  // Private.

  int num_callers;
  struct aipc_audioplugincaller **callers;

  int is_calling;

  pthread_mutex_t mutex; // Is only used in add_caller and remove_caller. call_audioplugins is not blocked.
};


struct aipc_audiopluginmixer *aipc_audiopluginmixer_new(void);


bool
aipc_audiopluginmixer_add_caller(
				 struct aipc_audiopluginmixer *audiopluginmixer,
				 struct aipc_audioplugincaller *caller
				 );

void
aipc_audiopluginmixer_remove_caller(
				    struct aipc_audiopluginmixer *audiopluginmixer,
				    struct aipc_audioplugincaller *caller
				    );


void
aipc_audiopluginmixer_call_audioplugins(
					struct aipc_audiopluginmixer *audiopluginmixer,
					int num_inputs,
					float **inputs,
					int num_outputs,
					float **outputs,
					int num_frames
					);

void
aipc_audiopluginmixer_delete(
			     struct aipc_audiopluginmixer *audiopluginmixer
			     );



#endif


