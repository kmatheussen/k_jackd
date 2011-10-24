/*
    Copyright (C) 2002 Kjetil S. Matheussen / Notam.

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

#ifndef AIPC_SHAREDMEM_H
#define AIPC_SHAREDMEM_H



/*
  Only aipc_sharedmem_delete is locked.
  Simultaniously calling aipc_sharedmem_delete and aipc_sharedmem_newFromKey may not work.
*/


struct aipc_sharedmem{

  char *filenames_prefix;

  char *addr;
  int size;

  int key;
  int id;

  int pid;
};


/**
 * Commenting so hard.
 */

void aipc_sharedmem_delete(struct aipc_sharedmem *sharedmem);



/**
 * @param size Number of bytes to allocate. This variable will be ignored if
 * another process has allready allocated sharedmem with the same \a filenames_prefix value.
 * If size is set to -1, the function will wait for the sharedmem to be allocated from
 * another process.
 * @param timeout How long time in usecs to wait before aipc_sharedmem_new is called
 * from another process with a \a size value higher than 0.
 * If timeout is -1, the function will wait forever. If timeout is 0, the
 * function will return immidiately. If \a size is higher than -1, this variable will be ignored.
 *
 */

struct aipc_sharedmem *aipc_sharedmem_new(
					  char *filenames_prefix,
					  int size,
					  int timeout
					  );



#endif

