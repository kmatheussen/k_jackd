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

#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <aipc/lock.h>
#include <aipc/variable.h>


void aipc_variable_delete(
			  char *filenames_prefix
			  )
{
  struct aipc_lock *lock=NULL;
  char temp[500];

  sprintf(temp,"%s_variable.lock",filenames_prefix);
  lock=aipc_lock_new(temp,0);

  sprintf(temp,"%s_variable",filenames_prefix);
  if(unlink(temp)==-1){
    fprintf(stderr,"aipc_variable_delete: Unable to delete \"%s\" (%s).\n",temp,strerror(errno));
  }

  aipc_lock_delete(lock);
}



bool aipc_variable_create(
			  char *filenames_prefix,
			  int sizeof_data,
			  void *data
			  )
{
  struct aipc_lock *lock=NULL;
  char temp[500];
  FILE *file=NULL;
  bool result=false;

  sprintf(temp,"%s_variable.lock",filenames_prefix);

  lock=aipc_lock_new(temp,0);
  if(lock==NULL){
    goto exit;
  }


  sprintf(temp,"%s_variable",filenames_prefix);

  file=fopen(temp,"wb");
  if(file==NULL){
    fprintf(stderr,"aipc_variable_create: Unable to create \"%s\" (%s).\n",temp,strerror(errno));
    goto exit;
  }

  if(fwrite(data,1,sizeof_data,file)!=sizeof_data){
    goto exit;
  }

  result=true;

 exit:
  if(file!=NULL) fclose(file);
  if(lock!=NULL) aipc_lock_delete(lock);

  return result;

}


bool aipc_variable_get(
		       char *filenames_prefix,
		       int sizeof_data,
		       void *data,
		       int timeout
		       )
{
  struct aipc_lock *lock=NULL;
  char lockname[500];
  char temp[500];
  FILE *file=NULL;
  bool result=false;

  sprintf(lockname,"%s_variable.lock",filenames_prefix);

  if((lock=aipc_lock_new(lockname,0))==NULL){
    goto exit;
  }

  sprintf(temp,"%s_variable",filenames_prefix);
  file=fopen(temp,"rb");

  if(file==NULL){
    int waitedtime=0;
    while(timeout==-1 || waitedtime<timeout){
      if(lock!=NULL) aipc_lock_delete(lock);
      usleep(2000);
      waitedtime+=2020;
      if((lock=aipc_lock_new(lockname,0))==NULL){
	goto exit;
      }
      file=fopen(temp,"rb");
      if(file!=NULL) break;
    }
  }

  if(file==NULL){
    goto exit;
  }

  if(fread(data,1,sizeof_data,file)!=sizeof_data){
    goto exit;
  }

  result=true;

 exit:
  if(file!=NULL) fclose(file);
  if(lock!=NULL) aipc_lock_delete(lock);

  return result;
  
}


bool aipc_variable_create_int(
			  char *filenames_prefix,
			  int data
			  )
{
  return aipc_variable_create(filenames_prefix,sizeof(int),&data);
}

int aipc_variable_get_int(
			  char *filenames_prefix,
			  int timeout
			  )
{
  int ret=-1;
  aipc_variable_get(filenames_prefix,sizeof(int),&ret,timeout);
  return ret;
}


bool aipc_variable_create_float(
			  char *filenames_prefix,
			  float data
			  )
{
  return aipc_variable_create(filenames_prefix,sizeof(float),&data);
}

float aipc_variable_get_float(
			  char *filenames_prefix,
			  int timeout
			  )
{
  float ret=0;
  aipc_variable_get(filenames_prefix,sizeof(float),&ret,timeout);
  return ret;
}



void aipc_variable_delete_string(
			  char *filenames_prefix
			  )
{
  struct aipc_lock *lock=NULL;
  char temp[500];


  sprintf(temp,"%s_variable_string.lock",filenames_prefix);
  lock=aipc_lock_new(temp,0);

  sprintf(temp,"%s_variable",filenames_prefix);
  unlink(temp);

  sprintf(temp,"%s_variable_string_len",filenames_prefix);
  unlink(temp);

  aipc_lock_delete(lock);
}


bool aipc_variable_create_string(
			  char *filenames_prefix,
			  char *string
			  )
{
  char temp[500];
  struct aipc_lock *lock=NULL;
  bool ret=false;

  sprintf(temp,"%s_variable_string.lock",filenames_prefix);
  lock=aipc_lock_new(temp,0);
  if(lock==NULL){
    goto exit;
  }

  sprintf(temp,"%s_variable_string_len",filenames_prefix);
  if(aipc_variable_create_int(temp,strlen(string)+1)==false){
    goto exit;
  }

  ret=aipc_variable_create(filenames_prefix,strlen(string)+1,string);

  if(ret==false){
    aipc_variable_delete(temp);
  }

 exit:
  if(lock!=NULL) aipc_lock_delete(lock);

  return ret;
}

char *aipc_variable_get_string(
			  char *filenames_prefix,
			  int timeout
			  )
{
  char temp[500];
  struct aipc_lock *lock=NULL;
  char *ret=NULL;
  int stringlen;

  sprintf(temp,"%s_variable_string.lock",filenames_prefix);
  lock=aipc_lock_new(temp,0);
  if(lock==NULL){
    goto exit;
  }


  if(aipc_variable_get(filenames_prefix,sizeof(int),&stringlen,timeout)==false){
    goto exit;
  }

  ret=malloc(stringlen+1);

  if(aipc_variable_get(filenames_prefix,stringlen,ret,timeout)==false){
    free(ret);
    ret=NULL;
  }

 exit:
  if(lock!=NULL) aipc_lock_delete(lock);
  return ret;
}



