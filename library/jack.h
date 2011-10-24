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


#include "../k_jack.h"

struct _jack_port{
  jack_client_t *client;
  bool isinput;
  int num;
};


struct _jack_client{
  const char *name;

  int num_inputs;
  int num_outputs;

  struct Client2Jackd c2j;

  struct aipc_audioplugin *audioplugin;

  JackProcessCallback process_callback;
  void *process_arg;

  void (*on_shutdown_function)(void *arg);
  void *on_shutdown_arg;

  struct _jack_port input_ports[512];
  struct _jack_port output_ports[512];
};


