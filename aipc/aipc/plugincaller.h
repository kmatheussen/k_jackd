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


#ifndef AIPC_PLUGINCALLER_H
#define AIPC_PLUGINCALLER_H



struct aipc_plugincaller{
  int sizeof_shared_memarea;
  void *shared_memarea; // Shared memory between host and plugin process. Free use.
  void *private;
};



struct aipc_plugincaller *aipc_plugincaller_new(
				      char *directoryname,
				      int timeout
				      );


void *aipc_plugincaller_call_plugin(
			  struct aipc_plugincaller *eh,
			  int sizeof_data_to_send,
			  void *data_to_send,
			  int *sizeof_data_returned
			  );

void aipc_plugincaller_delete(struct aipc_plugincaller *eh);






#endif

