
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#include <aipc/plugincaller.h>


int main(){
  struct aipc_plugincaller *eh=NULL;
  float testmem[5]={0.0f,1.0f,2.0f,3.0f,4.0f};
  float *retmem;
  int retmem_size;

  system("rm -fr /tmp/test_dir");

  system("mkdir /tmp/test_dir");

  system("./test_plugin secretoptionass /tmp/test_dir &");

  if((eh=aipc_plugincaller_new("/tmp/test_dir/",500000))==NULL){
    printf("host_oops\n");
    goto exit;
  }

  printf("Host with pid %d started successfully.\n",getpid());
  printf("Data before processing: %f %f %f %f %f\n",testmem[0],testmem[1],testmem[2],testmem[3],testmem[4]);

  retmem=aipc_plugincaller_call_plugin(eh,5*sizeof(float),testmem,&retmem_size);

  if(retmem==NULL){
    printf("aipc_plugincaller_call_plugin returned NULL. Plugin probably shut down.\n");
  }else{
    printf("Data after processing: %f %f %f %f %f\n",retmem[0],retmem[1],retmem[2],retmem[3],retmem[4]);
  }
  usleep(1000000);

  exit:
  fprintf(stderr,"host_going_to_exit\n");
  if(eh!=NULL) aipc_plugincaller_delete(eh);

  fprintf(stderr,"host_exit\n");
  return 0;
}

