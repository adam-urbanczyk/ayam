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

/* fifodspy.c - a RenderMan Display Driver that writes RGBA-Data to a FIFO */

#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/stat.h>

#include <ndspy.h>

#ifdef WIN32
#include "mkfifo.h"
#endif

typedef struct fifoimagetype_s
{
  FILE *file;
  int channels;
  int width, height;
} fifoimagetype;


PtDspyError
DspyImageOpen(PtDspyImageHandle *imagehandle,
	      const char *drivername,
	      const char *filename,
	      int width, int height, int paramCount,
	      const UserParameter *parameters,
	      int formatCount,
	      PtDspyDevFormat *format,
	      PtFlagStuff *flagstuff)
{
 PtDspyError ret = PkDspyErrorNone;
 int i, err;
 fifoimagetype *image;
 PtDspyDevFormat ourformat[4];
#ifdef WIN32
 size_t lf, lt;
 char *pipe = NULL;
 char *tmpenv = NULL;
 char *tmp = NULL;
#endif

  if(formatCount != 4)
    return PkDspyErrorBadParams;

  if(!(image = malloc(sizeof(fifoimagetype))))
    return PkDspyErrorNoMemory;

#ifdef WIN32
  /* construct pipe name from filename */
  lf = strlen(filename);
  if(!(pipe = malloc((lf+10)*sizeof(char))))
    {
      ret = PkDspyErrorNoMemory;
      goto cleanup;
    }
  memcpy(pipe, "\\\\.\\pipe\\", 9*sizeof(char));
  memcpy(&(pipe[9]), filename, lf*sizeof(char));
  pipe[lf+9] = '\0';

  /* construct sync file name */
  tmpenv = getenv("TEMP");
  if(!tmpenv)
    tmpenv = getenv("TMP");
  if(!tmpenv)
    tmpenv = getenv("USERPROFILE");

  if(tmpenv)
    {
      lt = strlen(tmpenv);
      if(!(tmp = malloc((lf+lt+2)*sizeof(char))))
	{
	  ret = PkDspyErrorNoMemory;
	  goto cleanup;
	}
      memcpy(tmp, tmpenv, lt*sizeof(char));
      tmp[lt] = '\\';
      memcpy(&(tmp[lt+1]), filename, lf*sizeof(char));

      tmp[lf+lt+1] = '\0';
    }

  /* create named pipe */
  if((image->file = mkfifo(pipe, tmp)) == NULL)
    {
      ret = PkDspyErrorNoResource;
      goto cleanup;
    }
#else
  if((err = mkfifo(filename, 0666)) != 0)
    {
      ret = PkDspyErrorNoResource;
      goto cleanup;
    }
  image->file = fopen(filename, "wb");
#endif

  if(0 == width)
    width = 640;
  if(0 == height)
    height = 480;

  image->channels = formatCount;
  image->width = width;
  image->height = height;

  for(i = 0; i < formatCount; i++)
    {
      format[i].type = PkDspyUnsigned8;
    }

  for(i = 0; i < formatCount; i++)
    {
      switch(*format[i].name)
	{
	case 'r':
	  ourformat[0] = format[i];
	  break;
	case 'g':
	  ourformat[1] = format[i];
	  break;
	case 'b':
	  ourformat[2] = format[i];
	  break;
	case 'a':
	  ourformat[3] = format[i];
	  break;
	default:
	  break;
	}
    }

  for(i = 0; i < formatCount; i++)
    {
      format[i].name = ourformat[i].name;
    }

  flagstuff->flags |= PkDspyFlagsWantsScanLineOrder;

  *imagehandle = image;

  /* prevent cleanup code from doing something harmful */
  image = NULL;

cleanup:

  if(image)
    free(image);

#ifdef WIN32
  if(pipe)
    free(pipe);
  if(tmp)
    free(tmp);
#endif

 return ret;
}


PtDspyError
DspyImageQuery(PtDspyImageHandle imagehandle,
	       PtDspyQueryType querytype,
	       size_t datalen,
	       void *data)
{
 PtDspyError ret;
 fifoimagetype *image = (fifoimagetype*)imagehandle;

  ret = PkDspyErrorNone;

  if(datalen > 0 && NULL != data)
    {
      switch(querytype)
      {
      case PkOverwriteQuery:
	{
	  PtDspyOverwriteInfo overwriteinfo;

	  if(datalen > sizeof(overwriteinfo))
	    {
	      datalen = sizeof(overwriteinfo);
	    }
	  overwriteinfo.overwrite = 1;
	  overwriteinfo.interactive = 0;
	  memcpy(data, &overwriteinfo, datalen);
	  break;
	}
      case PkSizeQuery:
	{
	  PtDspySizeInfo sizeinfo;

	  if(datalen > sizeof(sizeinfo))
	    {
	      datalen = sizeof(sizeinfo);
	    }
	  if(image)
	    {
	      if(0 == image->width || 0 == image->height)
		{
		  image->width = 640;
		  image->height = 480;
		}
	      sizeinfo.width = image->width;
	      sizeinfo.height = image->height;
	      sizeinfo.aspectRatio = 1.0f;
	    }
	  else
	    {
	      sizeinfo.width = 640;
	      sizeinfo.height = 480;
	      sizeinfo.aspectRatio = 1.0f;
	    }
	  memcpy(data, &sizeinfo, datalen);
	  break;
	}
#if 0
      case PkRenderingStartQuery:
	{
	  PtDspyRenderingStartQuery startlocation;

	  if(datalen > sizeof(startlocation))
	    {
	      datalen = sizeof(startlocation);
	    }
	  if(image)
	    {
	      /*
	       * initialize values in startLocation
	       */
	      memcpy(data, &startlocation, datalen);
	    }
	  else
	    {
	      ret = PkDspyErrorUndefined;
	    }
	  break;
	}
#endif
      default:
	ret = PkDspyErrorUnsupported;
	break;
      }
    }
  else
    {
      ret = PkDspyErrorBadParams;
    }

 return ret;
}


PtDspyError
DspyImageData(PtDspyImageHandle imagehandle,
	      int xmin,
	      int xmax_plusone,
	      int ymin,
	      int ymax_plusone,
	      int entrysize,
	      const unsigned char *data)
{
 int x, y;
 int xy[4] = {xmin, xmax_plusone, ymin, ymax_plusone};
 fifoimagetype *image = (fifoimagetype*)imagehandle;

  fwrite(xy, sizeof(int), 4, image->file);

  for(y = ymin; y < ymax_plusone; y++)
    {
      for(x = xmin; x < xmax_plusone; x++)
	{
	  fwrite(data, sizeof(char), image->channels, image->file);
	  data += entrysize;
	}
    }

 return PkDspyErrorNone;
}


PtDspyError
DspyImageClose(PtDspyImageHandle imagehandle)
{
 fifoimagetype *image = (fifoimagetype*)imagehandle;

  fclose(image->file);
  free(image);

 return PkDspyErrorNone;
}
