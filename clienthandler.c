/*
    Copyright (C) 2003 Kjetil S. Matheussen / Notam.

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


struct ClientList{
  struct ClientList *next;
  char name[500];
  char dirname[500];
  struct aipc_audioplugincaller *caller;
};

struct ClientList *ClientListRoot=NULL;

static struct aipc_audioplugincaller *k_jackd_getCaller(char *clientname){
  struct ClientList *cl=ClientListRoot;  

  while(cl!=NULL){
    if(!strcmp(cl->name,clientname)){
      return cl->caller;
    }
    cl=cl->next;
  }

  return NULL;
}


static void k_jackd_newClient(struct Client2Jackd *c2j){
  struct ClientList *cl;
  struct aipc_audioplugincaller *caller;


  printf("Adding new client \"%s\".\n",c2j->name);

  caller=aipc_audioplugincaller_new(
				    c2j->dirname,
				    -1
				    );


  if(caller==NULL) return;

  cl=calloc(1,sizeof(struct ClientList));

  sprintf(cl->name,"%s",c2j->name);
  sprintf(cl->dirname,"%s",c2j->dirname);

  cl->caller=caller;

  cl->next=ClientListRoot;
  ClientListRoot=cl;

  printf("New client \"%s\" added.\n",c2j->name);

}

static void k_jackd_deleteClient(struct Client2Jackd *c2j){
  struct ClientList *cl=ClientListRoot;
  struct ClientList *temp=NULL;

  while(cl!=NULL){
    if(!strcmp(cl->dirname,c2j->dirname)){
      if(temp==NULL){
	ClientListRoot=cl->next;
      }else{
	temp->next=cl->next;
      }

      aipc_audioplugincaller_delete(cl->caller);

      {
	int sendit=1;
	char ioname[500];
	sprintf(ioname,"%ssimpleio",c2j->dirname);
	aipc_simpleio_send(ioname,&sendit,sizeof(int),-1);
      }

      post("Have just deleted \"%s\".\n",c2j->name);
      return;
    }
    temp=cl;
    cl=cl->next;
  }

  fprintf(
	  stderr,
	  "k_jackd_deleteClient: Client \"%s\" living in directory \"%s\" not found in list of clients.\n",
	  c2j->name,
	  c2j->dirname
	  );
}


static void *k_jackd_inputthread(void *arg){
  struct aipc_receiver *main_receiver=(struct aipc_receiver*)arg;

  for(;;){
    int size;
    struct Client2Jackd *c2j=aipc_receiver_receive(main_receiver,&size);
    if(size!=sizeof(struct Client2Jackd)){
      fprintf(stderr,"k_jackd_inputthread: Strange size of input %d\n",size);
      continue;
    }
    
    switch(c2j->reqtype){
    case KJACK_new:
      k_jackd_newClient(c2j);
      break;
    case KJACK_delete:
      k_jackd_deleteClient(c2j);
      break;
    }


  }

  return NULL;
}
