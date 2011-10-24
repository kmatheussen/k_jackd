
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#include <aipc/audioplugincaller.h>


int main(){
  struct aipc_audioplugincaller *aeh=NULL;
  float testmem[5]={0.0f,1.0f,2.0f,3.0f,4.0f};
  float **testmem2;

  testmem2=malloc(sizeof(float*));
  testmem2[0]=&testmem[0];



  system("rm -fr /tmp/test_dir");

  system("mkdir /tmp/test_dir");

  system("./test_audioplugin secretoptionass /tmp/test_dir &");

  if((aeh=aipc_audioplugincaller_new("/tmp/test_dir/",5000000))==NULL){
    printf("host_oops\n");
    goto exit;
  }

  printf("Host started successfully.\n");

  printf("Signal before processing: %f %f %f %f %f\n",testmem[0],testmem[1],testmem[2],testmem[3],testmem[4]);

  if(aipc_audioplugincaller_call_audioplugin(aeh,testmem2,testmem2,5)==false){
    printf("aipc_audioplugincaller_call_audioplugin returned false. Connection shut down.\n");
  }else{
    printf("Signal after processing:  %f %f %f %f %f\n",testmem[0],testmem[1],testmem[2],testmem[3],testmem[4]);
  }

  usleep(1000000);

  exit:
  fprintf(stderr,"host_going_to_exit\n");
  if(aeh!=NULL) aipc_audioplugincaller_delete(aeh);

  fprintf(stderr,"host_exit\n");
  return 0;
}

