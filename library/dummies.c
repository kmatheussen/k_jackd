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

#include <stdio.h>
#include <jack/jack.h>

int jack_set_buffer_size_callback (jack_client_t *client, JackBufferSizeCallback bufsize_callback, void *arg){
  return 0;
}

int jack_set_sample_rate_callback (jack_client_t *client, JackSampleRateCallback srate_callback, void *arg){return 0;}

int jack_set_port_registration_callback (jack_client_t *client, JackPortRegistrationCallback registration_callback, void *arg){return 0;}

int jack_set_graph_order_callback (jack_client_t *client, JackGraphOrderCallback graph_callback, void *arg){return 0;}

int jack_set_xrun_callback (jack_client_t *client, JackXRunCallback xrun_callback, void *arg){return 0;}

const char * jack_port_name (const jack_port_t *port){return NULL;}

const char * jack_port_short_name (const jack_port_t *port){return NULL;}

int          jack_port_flags (const jack_port_t *port){return 0;}

const char * jack_port_type (const jack_port_t *port){return NULL;}

int          jack_port_is_mine (const jack_client_t *client, const jack_port_t *port){return 0;}

int jack_port_connected (const jack_port_t *port){return 0;}

int jack_port_connected_to (const jack_port_t *port, const char *portname){return 0;}

int jack_port_connected_to_port (const jack_port_t *port, const jack_port_t *other_port){return 0;}

const char ** jack_port_get_connections (const jack_port_t *port){return NULL;}

int jack_port_set_name (jack_port_t *port, const char *name){return 0;}

int jack_connect (jack_client_t *client,
		  const char *source_port,
		  const char *destination_port){return 0;}

int jack_disconnect (jack_client_t *client,
		     const char *source_port,
		     const char *destination_port){return 0;}

int jack_port_connect (jack_client_t *client, jack_port_t *src, jack_port_t *dst){return 0;}

int jack_port_disconnect (jack_client_t *client, jack_port_t *port){return 0;}

int  jack_port_tie (jack_port_t *src, jack_port_t *dst){return 0;}

int  jack_port_untie (jack_port_t *port){return 0;}

int jack_port_lock (jack_client_t *client, jack_port_t *port){return 0;}

int jack_port_unlock (jack_client_t *client, jack_port_t *port){return 0;}

jack_nframes_t jack_port_get_latency (jack_port_t *port){return 0;}

jack_nframes_t jack_port_get_total_latency (jack_client_t *client, jack_port_t *port){return 0;}

void jack_port_set_latency (jack_port_t *port, jack_nframes_t frames){return;}

int jack_port_request_monitor (jack_port_t *port, int onoff){return 0;}

int jack_port_request_monitor_by_name (jack_client_t *client, const char *port_name, int onoff){return 0;}

int jack_port_ensure_monitor (jack_port_t *port, int onoff){return 0;}

int jack_port_monitoring_input (jack_port_t *port){return 0;}

const char ** jack_get_ports (jack_client_t *client, 
			      const char *port_name_pattern, 
			      const char *type_name_pattern, 
			      unsigned long flags){return NULL;}

jack_port_t *jack_port_by_id (jack_client_t *client, jack_port_id_t id){return 0;}

int  jack_engine_takeover_timebase (jack_client_t *client){return 0;}

void jack_update_time (jack_client_t *client, jack_nframes_t frames){return;}

jack_nframes_t jack_frames_since_cycle_start (const jack_client_t *client){return 0;}

jack_nframes_t jack_frame_time (const jack_client_t *client){return 0;}

float jack_cpu_load (jack_client_t *client){return 0.0f;}
