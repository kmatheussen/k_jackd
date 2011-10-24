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

#ifndef AIPC_AUDIOPLUGIN_H
#define AIPC_AUDIOPLUGIN_H

#include <stdbool.h>



/***************/
/* Structures. */
/***************/

struct aipc_audioplugin{
  int num_inputs;
  int num_outputs;

  float **inputs;
  float **outputs;

  void *private;
};



typedef void (*AipcAudiopluginCallback)(
					struct aipc_audioplugin *audioplugin,
					int num_frames,
					void *arg
					);


/**
 * @param filenames_prefix A unique prefix for all files created by aipc_audioplugin_new and
 * aipc_audioplugcaller_new. Note: Remember to add an appending slash if filenames_prefix is the
 * name of a directory.
 *
 */

struct aipc_audioplugin *aipc_audioplugin_new(
					 char *filenames_prefix,
					 int num_inputs,
					 int num_outputs,
					 AipcAudiopluginCallback audioplugin_callback,
					 void *arg,
					 int timeout
					 );
					 

void aipc_audioplugin_delete(struct aipc_audioplugin *aep);


#endif

