
#include <unistd.h>
#include <stdio.h>
#include <string.h>

#include <aipc/plugin.h>


#ifdef MIN
#  undef MIN
#endif
#define MIN(a,b) (((a)<(b))?(a):(b))



static void *callFromHost(
			struct aipc_plugin *ep,
			int sizeof_data,
			void *data,
			int *sizeof_returndata,
			void *arg
			)
{
  float *floatdata=data;
  int lokke;

  // data is not read-only.
  for(lokke=0;lokke<sizeof_data/sizeof(float);lokke++){
    floatdata[lokke]+=1.0f;
  }
  
  // Its legal to return the same data.
  *sizeof_returndata=sizeof_data;
  return data;
}


int main(int argc,char **argv){
  struct aipc_plugin *ep=NULL;

  if(argc<3 || strcmp(argv[1],"secretoptionass")){
    printf("test_plugin is started by test_host.\n");
    goto exit;
  }


  ep=aipc_plugin_new(
		   "/tmp/test_dir/",
		   callFromHost,
		   NULL,
		   0,
		   50000
		   );
  
  if(ep==NULL){
    printf("plugin_oops\n");
    goto exit;
  }



  printf("Plugin started successfully.\n");
  usleep(500000);

  // return 0;

  exit:
  fprintf(stderr,"plugin_going_to_exit\n");
  if(ep!=NULL) aipc_plugin_delete(ep);
  fprintf(stderr,"plugin_exit\n");

  return 0;
}
