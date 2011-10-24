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


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <errno.h>

#include <string.h>

#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <fcntl.h>

#include <aipc/lock.h>
#include <aipc/dir.h>


char *aipc_dir_create(char *filenames_prefix,int id){
  char temp[500];
  char *ret=NULL;
  int lokke;
  char *directoryname=filenames_prefix;
  struct aipc_lock *lock=NULL;

  if(directoryname==NULL || id==-1){
    if(directoryname==NULL){
      directoryname="/tmp/k_communicate_socket";
    }

    sprintf(temp,"%s.lock\n",directoryname);
    lock=aipc_lock_new(temp,0);
    if(lock==NULL) goto exit;

    for(lokke=1000;;lokke++){
      sprintf(temp,"%s%d",directoryname,lokke);
      if(access(temp,F_OK)!=0){
	id=lokke;
	break;
      }
      if(lokke>=100000){
	fprintf(
		stderr,
		"aipc_dir_create: Could not find free directory. Some directory is probably not present.\n"
		);
	goto exit;
      }
    }
  }

  if(id==-2){
    sprintf(temp,"%s",directoryname);
  }else{
    sprintf(temp,"%s%d",directoryname,id);
  }

  if( mkdir(temp,O_RDWR|O_CREAT|0600) == -1){
    fprintf(stderr,"aipc_dir_create: Unable to create directory \"%s\".\n",temp);
    goto exit;
  }

  ret=malloc(strlen(temp)+2);
  sprintf(ret,"%s/",temp);

 exit:
  if(lock!=NULL) aipc_lock_delete(lock);
  return ret;
}


int aipc_dir_delete(char *directoryname){
  return rmdir(directoryname);
}


