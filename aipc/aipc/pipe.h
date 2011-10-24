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

#ifndef AIPC_PIPE_H
#define AIPC_PIPE_H

#include <aipc/input.h>
#include <aipc/output.h>

struct aipc_pipe{
  struct aipc_input *input;
  struct aipc_output *output;
};


/**
 * \brief Establish a pipe between two processes.
 *
 * @param filenames_prefix A unique prefix for all files created by the function.
 * Note: Remember to add an appending slash if filenames_prefix is the
 * name of a directory.
 *
 * @param timeoute How long time in usecs to wait before giving up a connection.
 * If timeout is -1, the function will wait forever.
 *
 * When two processes have called this function with the same filenames_prefix
 * value, a pipe is created between them.
 *
 * Note: Both processes gets the pipe simultaniously, so
 * if the second process waits for the first process to get the pipe before calling
 * this function, or the first process waits for the second, the pipe can not be created,
 * and a deadlock might happen.
 */
struct aipc_pipe *aipc_pipe_new(char *filenames_prefix,int timeout);

void aipc_pipe_delete(struct aipc_pipe *pipe);

bool aipc_pipe_send(struct aipc_pipe *pipe,void *buf,int size);
bool aipc_pipe_send_int(struct aipc_pipe *pipe,int val);

int aipc_pipe_receive(struct aipc_pipe *pipe,void *buf,int size);
int aipc_pipe_receive_int(struct aipc_pipe *pipe,int *dasint);

#endif

