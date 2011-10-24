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

#ifndef AIPC_AUDIOPLUGINCALLER_H
#define AIPC_AUDIOPLUGINCALLER_H

#include <stdbool.h>



/***************/
/* Structures. */
/***************/

struct aipc_audioplugincaller{
  int num_inputs;
  int num_outputs;

  void *private;
};


/******************/
/* Host Functions */
/******************/



/**
 * \brief Makes a new audioexchange host.
 *
 * @param directoryname A string pointing to a dedicated directory.
 *
 * @param timeout How long to wait before connection is established.
 * If timeout is -1, the function will wait forever.
 *
 * The function will wait until aipc_audioplugin_new (with the
 * same directoryname parameter) was called and successfully initialized from another process.
 *
 */

struct aipc_audioplugincaller *aipc_audioplugincaller_new(
				     char *directoryname,
				     int timeout
				     );



bool aipc_audioplugincaller_call_audioplugin(
	       struct aipc_audioplugincaller *aeh,
	       float **inputs,
	       float **outputs,
	       int num_frames
	       );


void aipc_audioplugincaller_delete(struct aipc_audioplugincaller *aeh);





#endif

