/*
    Copyright (C) 2002 Kjetil S. Matheussen / Notam.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>

#include <sys/ipc.h>
#include <sys/shm.h>

#include <sys/types.h>
#include <dirent.h>


#include <aipc/lock.h>
#include <aipc/dir.h>
#include <aipc/variable.h>

#include <aipc/sharedmem.h>







#include "sharedmem_procinfo.c"





/*

  operations           locks

  new                  new_lock
  check_allocated      new_lock
  new_from_key         new_from_key_lock
  delete               new_lock + new_from_key_lock
*/


#include "sharedmem_locking.c"






void aipc_sharedmem_delete(struct aipc_sharedmem *sharedmem){
  char temp[500];
  struct delete_lock *lock=SHM_getDeleteLock(sharedmem->filenames_prefix);

  sprintf(temp,"%s%s/%d",sharedmem->filenames_prefix,PROCINFODIRNAME,sharedmem->pid);
  SHM_deleteProcInfo(temp);

  if(SHM_checkIfMemCanBeFreed(sharedmem,sharedmem->filenames_prefix)==1){
    char temp[500];
    shmctl(sharedmem->id,IPC_RMID,NULL);
    fprintf(stderr,"aipc_sharedmem_delete: Shared memory deleted. (everything is okey)\n");
    fprintf(stderr,"aipc_sharedmem_delete: If you dont see this message, shared mem was not freed.\n");
    SHM_cleanUpProcInfos(sharedmem->filenames_prefix);

    sprintf(temp,"%s_key",sharedmem->filenames_prefix);
    aipc_variable_delete(temp);
  }

  free(sharedmem->filenames_prefix);
  free(sharedmem);

  SHM_deleteDeleteLock(lock);
}



static bool SHM_makeKeyVariable(
			       char *filenames_prefix,
			       int key
			       )
{
  char temp[500];
  sprintf(temp,"%s_key",filenames_prefix);
  return aipc_variable_create_int(temp,key);
}


static bool SHM_checkAllocated(
			       char *filenames_prefix,
			       int timeout
			       )
{
  char temp[500];
  sprintf(temp,"%s_key",filenames_prefix);

  if(aipc_variable_get_int(temp,timeout)==-1) return false;

  return true;
}


static int SHM_getKeyVariable(
			      char *filenames_prefix
			      )
{
  char temp[500];
  sprintf(temp,"%s_key",filenames_prefix);

  return aipc_variable_get_int(temp,0);
}


static struct aipc_sharedmem *SHM_newFromKey(
					     char *filenames_prefix,
					     int timeout
					     )
{
  struct shmid_ds ds;
  struct aipc_sharedmem *sharedmem=calloc(1,sizeof(struct aipc_sharedmem));


  if(SHM_checkAllocated(filenames_prefix,timeout)==false){
    free(sharedmem);
    return NULL;
  }

  sharedmem->key=SHM_getKeyVariable(filenames_prefix);
  sharedmem->filenames_prefix=strdup(filenames_prefix);

  sharedmem->id=shmget(sharedmem->key,0,0);
  if(sharedmem->id==-1){
    fprintf(
	    stderr,
	    "SHM_newFromKey: Problem getting shared mem from server. No seg-id.\n"
	    );
    free(sharedmem);
    return NULL;
  }

  sharedmem->addr=shmat(sharedmem->id,0,0);
  if(sharedmem->addr==NULL){
    fprintf(stderr,"SHM_newFromKey: Problem getting shared mem from server. No mem.\n");
    free(sharedmem);
    return NULL;
  }

  if(shmctl(sharedmem->id,IPC_STAT,&ds)==-1){
    fprintf(stderr,"SHM_newFromKey: Problem getting shared mem from server. No information.\n");
    free(sharedmem);
    return NULL;
  }

  sharedmem->size=ds.shm_segsz;


  if(SHM_saveProcInfo(filenames_prefix)==false){
    fprintf(stderr,"SHM_newFromKey: Problem saving process info.\n");
    free(sharedmem);
    return NULL;
  }

  return sharedmem;
}



static struct aipc_sharedmem *aipc_sharedmem_new_make(
						      char *filenames_prefix,
						      int size
						      )
{
  struct aipc_sharedmem *sharedmem=calloc(1,sizeof(struct aipc_sharedmem));
  sharedmem->size=size;
  sharedmem->filenames_prefix=strdup(filenames_prefix);

  sharedmem->key=random();
  sharedmem->id=shmget(sharedmem->key,size+sizeof(int),IPC_CREAT|0600);
  if(sharedmem->id==-1){
    fprintf(
	    stderr,
	    "aipc_sharedmem_new: Unable to allocate %d bytes of shared memory.\n",
	    size+sizeof(int)
	    );
    free(sharedmem);
    return NULL;
  }

  sharedmem->addr=shmat(sharedmem->id,0,0);
  if(sharedmem->addr==(void*)-1){
    fprintf(stderr,"aipc_sharedmem_new: Unable to allocate shared memory. 2\n");
    shmctl(sharedmem->id,IPC_RMID,NULL);
    free(sharedmem);
    return NULL;
  }


  if(SHM_makeKeyVariable(filenames_prefix,sharedmem->key)==false){
    fprintf(stderr,"aipc_sharedmem_new: Unable to make key identifier file.\n");
    shmctl(sharedmem->id,IPC_RMID,NULL);
    free(sharedmem);
    return NULL;
  }

  if(SHM_saveProcInfo(filenames_prefix)==false){
    fprintf(stderr,"aipc_sharedmem_new: Problem saving process info.\n");
    shmctl(sharedmem->id,IPC_RMID,NULL);
    free(sharedmem);
    return NULL;
  }

  return sharedmem;
}





struct aipc_sharedmem *aipc_sharedmem_new(
					  char *filenames_prefix,
					  int size,
					  int timeout
					  )
{
  struct aipc_lock *lock=NULL;
  struct aipc_sharedmem *ret=NULL;

  if(size==-1){
    lock=SHM_getNewFromKeyLock(filenames_prefix);
    if(lock==NULL) goto exit;
    ret=SHM_newFromKey(filenames_prefix,timeout);
  }else{
    lock=SHM_getNewLock(filenames_prefix);
    if(lock==NULL) goto exit;
    if(SHM_checkAllocated(filenames_prefix,0)==true){
      // If the sharedmem allready exist, its safe to lock both lockfiles.
      struct aipc_lock *lock2;
      lock2=SHM_getNewFromKeyLock(filenames_prefix);
      if(lock2!=NULL){
	ret=SHM_newFromKey(filenames_prefix,0);
	aipc_lock_delete(lock2);
      }
    }else{
      ret=aipc_sharedmem_new_make(filenames_prefix,size);
    }
  }

  ret->pid=getpid();
  
 exit:
  if(lock!=NULL) aipc_lock_delete(lock);
  return ret;
}

