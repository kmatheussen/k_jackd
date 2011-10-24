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




static t_int *k_jackd_perform(t_int *w)
{
    unsigned i = 0;
    t_k_jackd* x = NULL;
    t_float** audio_inputs = NULL;
    t_float** audio_outputs = NULL;
    int num_samples = 0;

    /* precondition(s) */
    //    assert (w != NULL);

    /* Unpack DSP parameter vector */
    x = (t_k_jackd*)(w[1]);
    num_samples = (int)(w[2]);
    audio_inputs = (t_float**)(&w[3]);
    audio_outputs = (t_float**)(&w[3 + x->mixer->num_inputs]);

    /* Call the LADSPA/VST plugin */
    //    plugin_tilde_apply_plugin (x);

    //    memset(audio_outputs[0],0,sizeof(float)*num_samples);

    //memcpy(audio_outputs[0],audio_inputs[0],sizeof(float)*num_samples);

    //fprintf(stderr,"num_samples: %d, num_inputs: %d, num_outputs: %d\n",num_samples,x->mixer->num_inputs,x->mixer->num_outputs);
    //    fprintf(stderr,"in: %x out: %x\n",(unsigned int)audio_inputs[0],(unsigned int)audio_outputs[0]);



    aipc_audiopluginmixer_call_audioplugins(
					    x->mixer,
					    x->mixer->num_inputs,
					    audio_inputs,
					    x->mixer->num_outputs,
					    audio_outputs,
					    num_samples
					    );


    return w + (x->dsp_vec_length + 1);
}


static void k_jackd_dsp(t_k_jackd *x, t_signal **sp)
{
    unsigned i = 0;
    unsigned long num_samples;

    num_samples = sp[0]->s_n;

    /* Pack vector of parameters for DSP routine */
    x->dsp_vec[0] = (t_int)x;
    x->dsp_vec[1] = (t_int)num_samples;
    /* Inputs are before outputs; ignore the first "null" input */

    for (i = 2; i < x->dsp_vec_length; i++) {
	x->dsp_vec[i] = (t_int)(sp[i - 1]->s_vec);
    }

    /* Connect audio ports with buffers (this is only done when DSP
       processing begins) */
    /*
    plugin_tilde_connect_audio (x,
				(float**)(&x->dsp_vec[2]),
				(float**)(&x->dsp_vec[2 + x->mixer->num_inputs]),
				num_samples);
    */


    /* add DSP routine to Pd's DSP chain */
    dsp_addv (k_jackd_perform, x->dsp_vec_length, x->dsp_vec);
}
