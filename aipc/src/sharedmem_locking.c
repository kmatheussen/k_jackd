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


#define NEW_LOCKNAME "_aipc_sharedmem_new.lock"
#define NEWFROMKEY_LOCKNAME "_aipc_sharedmem_new_from_key.lock"

struct delete_lock{
  struct aipc_lock *lock1;
  struct aipc_lock *lock2;
};


static struct aipc_lock *SHM_getNewLock(char *filenames_prefix){
  char temp[500];
  sprintf(temp,"%s%s",filenames_prefix,NEW_LOCKNAME);
  return aipc_lock_new(temp,0);
}

static struct aipc_lock *SHM_getNewFromKeyLock(char *filenames_prefix){
  char temp[500];
  sprintf(temp,"%s%s",filenames_prefix,NEWFROMKEY_LOCKNAME);
  return aipc_lock_new(temp,0);
}

static void SHM_deleteDeleteLock(struct delete_lock *lock){
  if(lock->lock1!=NULL) aipc_lock_delete(lock->lock1);
  if(lock->lock2!=NULL) aipc_lock_delete(lock->lock2);
  free(lock);
}

static struct delete_lock *SHM_getDeleteLock(char *filenames_prefix){
  struct delete_lock *ret=calloc(1,sizeof(struct delete_lock));
  ret->lock1=SHM_getNewLock(filenames_prefix);
  if(ret->lock1==NULL){
    SHM_deleteDeleteLock(ret);
    return NULL;
  }
  ret->lock2=SHM_getNewFromKeyLock(filenames_prefix);
  if(ret->lock2==NULL){
    SHM_deleteDeleteLock(ret);
    return NULL;
  }
  return ret;
}


