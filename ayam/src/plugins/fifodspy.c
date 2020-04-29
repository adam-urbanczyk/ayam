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

/* fifodspy.c - a RenderMan Display Driver that writes RGBA-Float to a FIFO */

#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/stat.h>

#include <ndspy.h>

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
	      const UserParameter  *parameters,
	      int formatCount,
	      PtDspyDevFormat *format,
	      PtFlagStuff *flagstuff)
{
 int i, j, err;
 fifoimagetype *image;
 PtDspyDevFormat ourformat[4];

  if(formatCount != 4)
    return PkDspyErrorBadParams;

  if(!(image = malloc(sizeof(fifoimagetype))))
    return PkDspyErrorNoMemory;

  if((err = mkfifo(filename, 0666)) != 0)
    {
      free(image);
      return PkDspyErrorNoResource;
    }

  image->file = fopen(filename, "wb");

  if(0 == width)
    width = 640;
  if(0 == height)
    height = 480;

  image->channels = formatCount;
  image->width = width;
  image->height = height;

  for(i = 0; i < formatCount; i++)
    {
      format[i].type = PkDspyFloat32;
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

 return PkDspyErrorNone;
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
	  PtDspyOverwriteInfo overwriteInfo;

	  if(datalen > sizeof(overwriteInfo))
	    datalen = sizeof(overwriteInfo);
	  overwriteInfo.overwrite = 1;
	  overwriteInfo.interactive = 0;
	  memcpy(data, &overwriteInfo, datalen);
	  break;
	}
      case PkSizeQuery:
	{
	  PtDspySizeInfo sizeInfo;

	  if(datalen > sizeof(sizeInfo))
	    {
	      datalen = sizeof(sizeInfo);
	    }
	  if(image)
	    {
	      if(0 == image->width || 0 == image->height)
		{
		  image->width = 640;
		  image->height = 480;
		}
	      sizeInfo.width = image->width;
	      sizeInfo.height = image->height;
	      sizeInfo.aspectRatio = 1.0f;
	    }
	  else
	    {
	      sizeInfo.width = 640;
	      sizeInfo.height = 480;
	      sizeInfo.aspectRatio = 1.0f;
	    }
	  memcpy(data, &sizeInfo, datalen);
	  break;
	}
#if 0
      case PkRenderingStartQuery :
	{
	  PtDspyRenderingStartQuery startLocation;

	  if (datalen > sizeof(startLocation))
	    datalen = sizeof(startLocation);

	  if(image)
	    {
	      /*
	       * initialize values in startLocation
	       */
	      memcpy(data, &startLocation, datalen);
	    }
	  else
	    {
	      ret = PkDspyErrorUndefined;
	    }
	  break;
	}
#endif
      default :
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
	  fwrite(data, sizeof(float), image->channels, image->file);
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
