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


#define PROCINFODIRNAME "_aipc_sharedmem_procinfodir"

struct ProcInfo{
  pid_t pid;
  char cmdline[1020];
};


static void SHM_deleteProcInfo(char *procinfofilename_prefix){
  aipc_variable_delete(procinfofilename_prefix);
}

static void SHM_cleanUpProcInfos(char *filenames_prefix){
  char temp[500];
  sprintf(temp,"%s%s",filenames_prefix,PROCINFODIRNAME);
  aipc_dir_delete(temp);
}

static bool SHM_saveProcInfo(char *filenames_prefix){
  char temp[500];
  char *dirname;
  FILE *file;
  struct ProcInfo procinfo={0};
  bool ret=false;

  sprintf(temp,"%s%s",filenames_prefix,PROCINFODIRNAME);
  if(access(temp,F_OK)!=0){
    if((dirname=aipc_dir_create(temp,-2))==NULL){
      goto exit;
    }
  }else{
    dirname=malloc(strlen(temp)+2);
    sprintf(dirname,"%s/",temp);
  }


  sprintf(temp,"/proc/%d/cmdline",getpid());
  file=fopen(temp,"r");
  if(file==NULL){
    fprintf(stderr,"aipc_sharedmem: Unable to read file \"%s\".\n",temp);
    goto exit;
  }
  fgets(procinfo.cmdline,1019,file);
  fclose(file);

  procinfo.pid=getpid();

  sprintf(temp,"%s%d",dirname,getpid());
  if(aipc_variable_create(temp,sizeof(struct ProcInfo),&procinfo)==false){
    fprintf(stderr,"aipc_sharedmem: Unable to create variable in directory \"%s\".\n",dirname);
    goto exit;
  }


  ret=true;

 exit:
  if(dirname!=NULL) free(dirname);

  return ret;
}



/*
  Returns 2 if proc is the caller process,
  1 if proc is alive,
  0 if proc is dead
  and -1 the procinfo file couldn't be opened.

  BUG: May return 1 even if proc is dead. The check is not perfect.
*/

static int SHM_isProcAlive(
			   char *procinfofilename_prefix)
{
  FILE *file;
  struct ProcInfo procinfo={0};
  int ret=-1;
  char temp[1020];

  if(aipc_variable_get(procinfofilename_prefix,sizeof(struct ProcInfo),&procinfo,0)==false){
    fprintf(stderr,"SHM_isProcAlive: Very strange \"%s\".\n",procinfofilename_prefix);
    goto exit;
  }

  ret=2;

#if 0
  if(procinfo.pid==getpid() || procinfo.pid==sharedmem->pid){
    goto exit;
  }
#endif

  ret=0;

  sprintf(temp,"/proc/%d/cmdline",procinfo.pid);
  file=fopen(temp,"r");
  if(file==NULL){
    goto exit;
  }
  fgets(temp,1019,file);
  fclose(file);

  if(strcmp(temp,procinfo.cmdline)){
    goto exit;
  }

  ret=1;


 exit:
  return ret;
}


/* Returns 1 if mem can be freed, 0 is not. 
   NOTE! Deletes procinfo variable also if from calling process.
*/
static int SHM_checkIfMemCanBeFreed(
				    struct aipc_sharedmem *sharedmem,
				    char *filenames_prefix
				    ){
  DIR *dir=NULL;
  struct dirent *direntry;
  char dirname[500];
  char temp[500];
  int ret=0;

  sprintf(dirname,"%s%s/",filenames_prefix,PROCINFODIRNAME);
  if((dir=opendir(dirname))==NULL){
    fprintf(stderr,"SHM_checkIfMemCanBeFreed: Could not open dir \"%s\".\n",temp);
    goto exit;
  }

  for(;;){
    direntry=readdir(dir);

    if(direntry==NULL) break;

    sprintf(temp,"%s%s",dirname,direntry->d_name);

    if(strlen(temp)<strlen("_variable")) continue;
    if(temp[strlen(temp)]=='.') continue;
    if(strcmp(temp+strlen(temp)-strlen("_variable"),"_variable")) continue;
    temp[strlen(temp)-strlen("_variable")]=0;

    switch(SHM_isProcAlive(temp)){
    case 1:
      goto exit;
      break;
    default:
      SHM_deleteProcInfo(temp);
      break;
    }
  }

  ret=1;

 exit:
  if(dir!=NULL) closedir(dir);
  return ret;
}




