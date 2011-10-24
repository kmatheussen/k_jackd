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



#ifndef AIPC_PLUGIN_H
#define AIPC_PLUGIN_H



struct aipc_plugin{
  int sizeof_shared_memarea;

  /**
   * Shared memory accessable for plugin and plugincaller process. Free use.
   */
  void *shared_memarea;

  void *private;
};


typedef void *(*AipcPluginCallback)(
				    struct aipc_plugin *ep,
				    int sizeof_data,
				    void *data,
				    int *sizeof_returndata,
				    void *arg
				    );


struct aipc_plugin *aipc_plugin_new(
				    char *directoryname,
				    AipcPluginCallback plugin_callback,
				    void *arg,
				    int sizeof_shared_memarea,
				    int timeout
				    );


void aipc_plugin_delete(struct aipc_plugin *ep);



#endif

