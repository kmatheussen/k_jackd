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


#ifndef AIPC_INPUT_H
#define AIPC_INPUT_H


#include <stdbool.h>
#include <sys/un.h>
#include <sys/types.h>
#include <sys/socket.h>

struct aipc_input{
  char *name;
  int id;
  int fd;

  struct sockaddr_un client_addr;
  socklen_t client_addrlen;
  int client_socket;
};



struct aipc_input *aipc_input_new(char *socketfilename,int id);

void aipc_input_delete(struct aipc_input *s_input);

int aipc_input_receive(struct aipc_input *s_input,void *buf,int maxsize);

bool aipc_input_accept_incoming_connection(struct aipc_input *s_input);



#endif

