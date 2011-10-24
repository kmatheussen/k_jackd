

#include <sys/ipc.h>

#include <sys/shm.h>

int key;



void proc2(void){
  printf("proc2 %d\n",key);
}


int main(){
  key=random();

  if(fork()){
    proc2();
    return 0;
  }

  printf("proc1 %d\n",key);

  return 0;
}


