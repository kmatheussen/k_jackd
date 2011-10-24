
#include <unistd.h>
#include <stdio.h>
#include <string.h>

#include <aipc/audioplugin.h>


#ifdef MIN
#  undef MIN
#endif
#define MIN(a,b) (((a)<(b))?(a):(b))



static void process(
		   struct aipc_audioplugin *aep,
		   int num_frames,
		   void *arg
		   )
{
  int channel;
  int lokke;

  for(channel=0;channel<MIN(aep->num_inputs,aep->num_outputs);channel++){
    for(lokke=0;lokke<num_frames;lokke++){
      aep->outputs[channel][lokke]=aep->inputs[channel][lokke]+1.0f;
    }
  }

}


int main(int argc,char **argv){
  struct aipc_audioplugin *aep=NULL;

  if(argc<3 || strcmp(argv[1],"secretoptionass")){
    printf("test_plugin is automaticly started by test_host. Dont run test_plugin directly.\n");
    goto exit;
  }

  aep=aipc_audioplugin_new(
		   "/tmp/test_dir/",
		   1,
		   1,
		   process,
		   NULL,
		   5000000
		   );
  
  if(aep==NULL){
    printf("plugin_oops\n");
    goto exit;
  }

  printf("Plugin started successfully.\n");
  //  usleep(500000);

  exit:
  fprintf(stderr,"plugin_going_to_exit\n");
  if(aep!=NULL) aipc_audioplugin_delete(aep);
  fprintf(stderr,"plugin_exit\n");

  return 0;
}
