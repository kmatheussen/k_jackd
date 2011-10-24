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


#ifndef AIPC_VARIABLE_H
#define AIPC_VARIABLE_H


#include <stdbool.h>



void aipc_variable_delete(
			  char *filenames_prefix
			  );

bool aipc_variable_create(
			  char *filenames_prefix,
			  int sizeof_data,
			  void *data
			  );


bool aipc_variable_get(
		       char *filenames_prefix,
		       int sizeof_data,
		       void *data,
		       int timeout
		       );


bool aipc_variable_create_int(
			      char *filenames_prefix,
			      int data
			      );

int aipc_variable_get_int(
			  char *filenames_prefix,
			  int timeout
			  );


bool aipc_variable_create_float(
				char *filenames_prefix,
				float data
				);

float aipc_variable_get_float(
			      char *filenames_prefix,
			      int timeout
			      );


void aipc_variable_delete_string(
				 char *filenames_prefix
				 );

bool aipc_variable_create_string(
				 char *filenames_prefix,
				 char *string
				 );

char *aipc_variable_get_string(
			       char *filenames_prefix,
			       int timeout
			       );


#endif

