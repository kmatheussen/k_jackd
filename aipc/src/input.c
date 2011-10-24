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



#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <errno.h>

#include <string.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>

#include <aipc/lock.h>

#include <aipc/input.h>


/* socketfilename can not be NULL if id is -2. */


/* lock er brukt. socketfilename må bli til filenames_prefix. */

struct aipc_input *aipc_input_new(char *socketfilename,int id){
  struct sockaddr_un addr;
  char temp[500];
  struct aipc_input *c;
  struct aipc_lock *locks=NULL;

  if(socketfilename==NULL || id==-1){
    int lokke;
    char lockfilename[500];

    if(socketfilename==NULL){
      socketfilename="/tmp/aipc_input_socket";
    }

    sprintf(lockfilename,"%s_lock_",socketfilename);
    locks=aipc_lock_new(lockfilename,1);

    if(locks==NULL) return NULL;
 
    for(lokke=1000;;lokke++){
      sprintf(temp,"%s%d",socketfilename,lokke);
      if(access(temp,F_OK)!=0){
	id=lokke;
	break;
      }
      if(lokke>=100000){
	fprintf(
		stderr,
		"aipc_input_new: Could not find free socket. Some directory is probably not present.\n"
		);
	aipc_lock_delete(locks);
	return NULL;
      }
    }

  }

  c=calloc(sizeof(struct aipc_input),1);

  c->client_socket=-1;
  c->id=id;

  if(id!=-2){
    sprintf(temp,"%s%d",socketfilename,id);
  }else{
    sprintf(temp,"%s",socketfilename);
  }

  c->name=malloc(strlen(temp)+1);
  sprintf(c->name,"%s",temp);
  
  c->fd=socket(AF_UNIX,SOCK_STREAM,0);
  
  if(c->fd<0){
    fprintf(stderr,"aipc_input_new: Could not create socket.\n");
    free(c->name);
    free(c);
    if(locks!=NULL) aipc_lock_delete(locks);
    return NULL;
  }
  

  addr.sun_family=AF_UNIX;
  sprintf(addr.sun_path,c->name);

  
  if( bind(c->fd,(struct sockaddr*)&addr,sizeof(struct sockaddr_un)) <0 ){
    fprintf(stderr,"aipc_input_new: Creation of InputSocket -%s- failed 1.\n",c->name);
    close(c->fd);
    free(c->name);
    free(c);
    if(locks!=NULL) aipc_lock_delete(locks);
    return NULL;
  }
  
  if (listen (c->fd, 10000) < 0) {
    fprintf(stderr,"aipc_input_new: Creation of InputSocket -%s- failed 2.\n",c->name);
    close(c->fd);
    free(c->name);
    free(c);
    if(locks!=NULL) aipc_lock_delete(locks);
    return NULL;
  }
  
  if(locks!=NULL) aipc_lock_delete(locks);

  return c;
}



void aipc_input_delete(struct aipc_input *c){
  //  char temp[500];

  if(c->client_socket!=-1 && c->client_socket!=0){
    close(c->client_socket);
  }

  close(c->fd);

  //sprintf(temp,"rm -f %s",c->name);
  //system(temp);
  unlink(c->name);
  
  free(c->name);
  free(c);
}

bool aipc_input_accept_incoming_connection(struct aipc_input *c){
  if(c->client_socket!=-1) close(c->client_socket);

  c->client_addrlen=sizeof(struct sockaddr_un);
  c->client_socket=accept(c->fd,(struct sockaddr *)&c->client_addr,&c->client_addrlen);
  if(c->client_socket<0){
    fprintf(
	    stderr,
	    "aipc_input_accept_incoming_connection: Not able to accept socket for %s. \n\tErrornum: %d / %s\n",
	    c->name,
	    c->client_socket,
	    strerror (errno)
	    );
    return false;
  }

  return true;

}


int aipc_input_receive(struct aipc_input *c,void *buf,int maxsize){

  if(c->client_socket==-1){
    fprintf(stderr,"aipc_input_receive: No incoming sockets are bounded to this socket. %s.\n",c->name);
    return -1;
  }

  return read(c->client_socket,buf,maxsize);
}



