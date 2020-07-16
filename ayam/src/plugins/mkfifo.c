/*
 * Ayam, a free 3D modeler for the RenderMan interface.
 *
 * Ayam is copyrighted 1998-2020 by Randolf Schultz
 * (randolf.schultz@gmail.com) and others.
 *
 * All rights reserved.
 *
 * See the file License for details.
 *
 */

#include <stddef.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <errno.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#define WIN32_LEAN_AND_MEAN 1
#include <windows.h>
#include <io.h>

/** mkfifo:
 * mkfifo surrogate for Win32
 * Creates a named pipe, then opens a handle and a C-FILE descriptor on it.
 * This is the server part of the fifo/pipe, i.e. it will be opened for writing.
 *
 * \param[in] pipename full name of pipe, must be of form "\\.\pipe\name"
 * \param[in] syncfilename full name of accompanying file for sync
 *
 * \returns FILE handle of newly created named pipe or NULL
 */
FILE *
mkfifo(const char *pipename, const char *syncfilename)
{
 HANDLE pipe;
 int fd;
 FILE *file = NULL;

#define BUFSIZE 1024
#define TIMEOUT 1000

  if(pipename == NULL)
    {
      _set_errno(EINVAL);
      return NULL;
    }

  pipe = CreateNamedPipeA(pipename, PIPE_ACCESS_OUTBOUND,
			  PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | PIPE_WAIT,
			  PIPE_UNLIMITED_INSTANCES, BUFSIZE, BUFSIZE,
			  TIMEOUT, NULL);

  if(pipe == INVALID_HANDLE_VALUE)
    {
      _set_errno(EBADF);
      return NULL;
    }

  fd = _open_osfhandle((intptr_t) pipe, _O_BINARY | _O_WRONLY);

  if(fd != -1)
    {
      file = _fdopen(fd, "wb");

      if(syncfilename != NULL)
	{
	  CreateFileA(syncfilename,GENERIC_READ,0,NULL,CREATE_ALWAYS,0,NULL);
	}

      /*if(connect)*/
      ConnectNamedPipe(pipe, NULL);
    }
  else
    {
      CloseHandle(pipe);
    }

 return file;
}
