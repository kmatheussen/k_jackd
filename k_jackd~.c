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



#include <sys/types.h>
#include <dirent.h>


#include <pthread.h>

#include <jack/jack.h>

#include <aipc/variable.h>
#include <aipc/receiver.h>
#include <aipc/audioplugincaller.h>
#include <aipc/audiopluginmixer.h>
#include <aipc/simpleio.h>

#include "m_pd.h"
#include <math.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "k_jack.h"


static pthread_t pthread=0;

typedef struct _k_jackd
{
  t_object x_obj;
  struct _k_jackd *next;
  struct aipc_audiopluginmixer *mixer;

  t_outlet *control_outlet;

  t_inlet *inlets[1024]; // Should hopefully be enough.

  t_outlet *outlets[1024]; // Should hopefully be enough.

  t_int*		dsp_vec;
  unsigned		dsp_vec_length;

} t_k_jackd;


static t_k_jackd *JackDListRoot=NULL;



static t_class *k_jackd_class;



#include "clienthandler.c"

#include "dsp.c"


static void k_jackd_free(t_k_jackd *x){
  
}


static void *k_jackd_new(t_symbol *s, t_int argc, t_atom* argv){
  long i;
  
  t_k_jackd *x = (t_k_jackd *)pd_new(k_jackd_class);

  x->mixer=aipc_audiopluginmixer_new();

  for(i=0;i<argc;i++){
    struct aipc_audioplugincaller *caller;
    switch(argv[i].a_type){
    case 2:
      caller=k_jackd_getCaller(atom_getsymbolarg(i,argc,argv)->s_name);
      if(caller!=NULL){
	printf("1\n");
	aipc_audiopluginmixer_add_caller(x->mixer,caller);
	printf("2\n");
      }else{
	fprintf(stderr,"\"%s\" client not found.\n",atom_getsymbolarg(i,argc,argv)->s_name);
      }
      break;
    default:
      break;
    }
  }

  if(x->mixer->num_callers==0){
    post("no clients found.\n");
    return NULL;
  }

  for(i=0;i<x->mixer->num_inputs;i++){
    x->inlets[i]=inlet_new(
			   &x->x_obj,
			   &x->x_obj.ob_pd,
			   gensym ("signal"),
			   gensym ("signal")
			   );
  }

  x->control_outlet = outlet_new (&x->x_obj, gensym ("control"));

  for(i=0;i<x->mixer->num_outputs;i++){
    x->outlets[i] = outlet_new (&x->x_obj, gensym ("signal"));
  }

  x->dsp_vec_length = x->mixer->num_inputs + x->mixer->num_outputs + 2;
  x->dsp_vec = (t_int*)calloc (x->dsp_vec_length, sizeof (t_int));

  return x;
}



void k_jackd_tilde_setup(void)
{
  static struct aipc_receiver *main_receiver=NULL;

  // jack-server init
  {
    char *homedir=getenv("HOME");
    char temp[500];
    DIR *dir;

    {
      sprintf(temp,"%s/.k_jackd",homedir);
      dir=opendir(temp);

      if(dir==NULL){
	sprintf(temp,"mkdir %s/.k_jackd",homedir);
	system(temp);
      }else{
	closedir(dir);
      }
    }
    
    sprintf(temp,"%s/.k_jackd/main_socket",homedir);
    main_receiver=aipc_receiver_new(temp);
    if(main_receiver==NULL){
      fprintf(stderr,"k_jackd_init: Unable to open socket \"%s\".\n",temp);
      fprintf(stderr,"Unless k_jackd is allready running, just delete that file.\n");
      return;
    }
    
    sprintf(temp,"rm -f %s/.k_jackd/datadir_*",homedir);
    system(temp);

    sprintf(temp,"%s/.k_jackd/sample_rate",homedir); 
    aipc_variable_create_float(temp,sys_getsr());

    sprintf(temp,"%s/.k_jackd/buffer_size",homedir);    
    aipc_variable_create_int(temp,sys_getblksize());
  }


    k_jackd_class = class_new(
			      gensym("k_jackd~"),
			      (t_newmethod)k_jackd_new,
			      (t_method)k_jackd_free,
			      sizeof(t_k_jackd),
			      0,
			      A_GIMME,
			      0
			      );

    class_addmethod(k_jackd_class, (t_method)k_jackd_dsp, gensym("dsp"), 0);

    class_addmethod (k_jackd_class,
		     nullfn,
		     gensym ("signal"),
		     0);


    class_sethelpsymbol(k_jackd_class, gensym("help-k_jackd~.pd"));

    pthread_create(&pthread,NULL,k_jackd_inputthread,main_receiver);

}
