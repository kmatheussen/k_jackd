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


/*

  Deleting host and plugin is a bit complicated, and happens like this:

  If host is first to shut down. Things happens like this:
  1. shutdown_lock.
  2. host tells plugin_process to stop.
  3. plugin_process tells host_process to stop.
  4. shutdown_unlock.

  If plugin is first to shut down. Things happens like this:
  1. shutdown_lock.
  2. Plugin tells host_process to stop.
  3. host_process tells plugin_process to stop.
  4. shutdown_unlock.


 */


/* Shared memory area for both plugin and host. */
struct PluginAndHost{
  int status; //0=running, 1=ended (used when shutting down)
  int is_calling; //0=not calling, 1=calling
  int okey_to_call; //1=okey, 0=not okey

  int h2p_datasize;
  int p2h_datasize;

};



#define EXCHANGE_PIPENAME "pipe_"
#define EXCHANGE_CONTROL_PIPENAME "controlpipe_"

#define SHAREDMEM_PREFIX "sharedmem_"
#define SHUTDOWN_LOCKNAME "shutdown.lock"



