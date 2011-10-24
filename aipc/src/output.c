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


#include <signal.h>
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

#include <aipc/output.h>

static void sigpipe(int sig){
  return;
}


static struct aipc_output *aipc_output_new_internal(char *socketfilename,int id,bool printout){
  char temp[500];
  struct aipc_output *c;
  struct sockaddr_un addr;

  signal(SIGPIPE,sigpipe);

  c=calloc(sizeof(struct aipc_output),1);
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
    fprintf(stderr,"k_communicate.c/aipc_output_new: Could not connect to socket.\n");
    free(c->name);
    free(c);
    return (struct aipc_output *)-1;
  }
  
  addr.sun_family=AF_UNIX;
  sprintf(addr.sun_path,c->name);

  if (connect (c->fd, (struct sockaddr *) &addr, sizeof (addr)) < 0) {
    if(printout==true){
      fprintf(stderr,"k_communicate.c/aipc_output_new: Could not connect -%s- %s\n",c->name, strerror (errno));
    }
    free(c->name);
    free(c);
    close (c->fd);
    return NULL;
  }

  return c;
}

struct aipc_output *aipc_output_new(char *socketfilename,int id){
  struct aipc_output *ret=aipc_output_new_internal(socketfilename,id,true);
  if(ret==(struct aipc_output *)-1){
    return NULL;
  }
  return ret;
}

struct aipc_output *aipc_output_new_wait(char *socketfilename,int id,int timeout){
  struct aipc_output *ret=aipc_output_new_internal(socketfilename,id,false);
  int waitedtime=0;

  if(ret==NULL){
    while(timeout==-1 || waitedtime<timeout){
      usleep(2000);
      waitedtime+=2020;
      ret=aipc_output_new_internal(socketfilename,id,false);
      if(ret!=NULL) break;
    }
  }

  if(ret==(struct aipc_output *)-1){
    return NULL;
  }

  return ret;
}


void aipc_output_delete(struct aipc_output *c){
  close(c->fd);

  free(c->name);
  free(c);
}

int aipc_output_send(struct aipc_output *c,void *buf,int size){
  return write(c->fd,buf,size);
}

bool aipc_output_send_int(struct aipc_output *c,int dasint){
  int s=dasint;

  if(write(c->fd,&s,sizeof(int))!=sizeof(int)){
    return false;
  }
  return true;
}

bool aipc_output_send_float(struct aipc_output *c,float dasfloat){
  float s=dasfloat;
  if(write(c->fd,&s,sizeof(float))!=sizeof(float)){
    return false;
  }
  return true;
}






