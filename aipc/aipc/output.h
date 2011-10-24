/*
    Copyright (C) 2002 Kjetil S. Matheussen / Notam.
    
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




#ifndef AIPC_OUTPUT_H
#define AIPC_OUTPUT_H


#include <stdbool.h>
#include <sys/un.h>
#include <sys/types.h>
#include <sys/socket.h>


struct aipc_output{
  char *name;
  int id;

  int fd;
};


struct aipc_output *aipc_output_new(char *socketfilename,int id);

/* timeout is in microseconds. -1 means wait forever. */
struct aipc_output *aipc_output_new_wait(char *socketfilename,int id,int timeout);

void aipc_output_delete(struct aipc_output *c);
int aipc_output_send(struct aipc_output *c,void *buf,int size);
bool aipc_output_send_int(struct aipc_output *c,int dasint);
bool aipc_output_send_float(struct aipc_output *c,float dasfloat);


#endif

