/*
 * Ayam, a free 3D modeler for the RenderMan interface.
 *
 * Ayam is copyrighted 1998-2004 by Randolf Schultz
 * (rschultz@informatik.uni-rostock.de) and others.
 *
 * All rights reserved.
 *
 * See the file License for details.
 *
 */

/* tti.c - truetype font parsing helpers */

/* original version of this file provided by Frank (copper) Pagels */

#include "ayam.h"
#include "tti.h"

#ifndef WIN32
 #include <unistd.h>
 #include <netinet/in.h>
#endif

#define step 0.15

#define read32bit(p) (((p[0]<<24)|(p[1]<<16)|(p[2]<<8)|p[3]))
#define read16bit(p) (((p[0]<<8)|p[1]))

int ay_tti_handle_simple_glyf(ay_tti_font *ttfont, ay_tti_glyf *glyf,
			      ay_tti_letter *vert, double *matrix);



#if 0
/*
**
**	make a bezier curve from 3 points
**
*/
void ay_tti_makearc(double x1, double y1,
		    double x2, double y2,
		    double x3, double y3,
		    ay_tti_outline *out, double st)
{
  int i;
  double t;

  i = out->numpoints;

  t = st;

  out->points[i].x = x1;
  out->points[i].y = y1;
  i++;

  while(t<1.0)
    {
      out->points[i].x = x1+(2*(x2-x1)+(x1-2*x2+x3)*t)*t;
      out->points[i].y = y1+(2*(y2-y1)+(y1-2*y2+y3)*t)*t;
      i++;
      t += st;
    }

  out->points[i].x = x3;
  out->points[i].y = y3;
  i++;
  out->numpoints = i;
}
#endif

/*
**
**	check if outline is filled
**
*/
#if 0
unsigned int ay_tti_isclockwise(signed short *x, signed short *y, int n)
{
 int rmin = 0;
 int xmin = x[0];
 int ymin = y[0];
 int i;

  /* first find rightmost lowest vertex of the polygon */
  for(i = 1; i < n; i++)
    {
      if(y[i] > ymin)
	continue;
      if(y[i] == ymin)
	{
	  if(x[i]< xmin)
	    continue;
	}
      rmin = i; /* a new rightmost lowest vertex */
      xmin = x[i];
      /* !!!!!!!!!!!!!!!!! check rmin
	 ymin = y[i];
	 possible out of array error */
    }

  if((rmin+1) == n)
    printf("CLOCKWISE RMIN ERROR !\n");

  /* test orientation at this rmin vertex */
  /* ccw <=> the edge leaving is left of the entering edge */
  if(rmin == 0)
    return ( ((x[0] - x[n-1]) * (y[1] - y[n-1]) - (x[1] - x[n-1]) *
	      (y[0] - y[n-1])) < 0 ? 1 : 0 );
  else
    return ( ((x[rmin] - x[rmin-1]) * (y[rmin+1] - y[rmin-1]) -
	      (x[rmin+1] - x[rmin-1]) * (y[rmin] - y[rmin-1])) < 0 ? 1 : 0 );
}
#endif


unsigned int
ay_tti_isclockwise(signed short *x, signed short *y, int N)
{
 int i, j;
 int area = 0;

  for(i = 0; i < N; i++)
    {
      j = (i + 1) % N;
      area += x[i] * y[j];
      area -= y[i] * x[j];
    }

 return (unsigned int) (area < 0 ? 1 : 0);
} /* ay_tti_isclockwise */


static double f2dot14(short x)
{
 short y = ntohs(x);
 return (y >> 14) + ((y & 0x3fff) / 16384.0);
} /* ay_tti_f2dot14 */


/* limit the recursion level to avoid cycles */
#define MAX_COMPOSITE_LEVEL 20

int
ay_tti_handle_comp_glyf(ay_tti_font *ttfont, ay_tti_glyf *glyf,
			double *orgmatrix, int level, ay_tti_letter *vert)
{
 int len;
 short ncontours;
 unsigned short	flagbyte, glyphindex;
 double	arg1, arg2;
 unsigned char *ptr;
 char *bptr;
 signed short *sptr;
 double	matrix[6], newmatrix[6];
 int go;
 unsigned int *uilo;
 unsigned short *uslo;

  /* get_glyf_table(glyphno, &glyf_table, &len); */

  if(ttfont->locformat)
    {
      len = ntohl(ttfont->loca[ttfont->nglyf + 1]) -
	ntohl(ttfont->loca[ttfont->nglyf]);
    }
  else
    {
      len = (ntohs(ttfont->loca[ttfont->nglyf + 1]) -
	     ntohs(ttfont->loca[ttfont->nglyf])) << 1;
    }


  if(ttfont->locformat)
    {
      /* long format */
      uilo = (unsigned int*)ttfont->loca;
      len = ntohl(uilo[ttfont->nglyf + 1]) - ntohl(uilo[ttfont->nglyf]);
    }
  else
    {
      uslo = (unsigned short*)ttfont->loca;
      len = (ntohl(uslo[ttfont->nglyf+1]) - ntohl(uslo[ttfont->nglyf])) << 1;
    }


  if(len <= 0) /* nothing to do */
    return 0;

  ncontours = ntohs(glyf->numberOfContours);
  if(ncontours >= 0) { /* simple case */
    /* draw_simple_glyf(g, glyph_list, glyphno, orgmatrix); */
    ay_tti_handle_simple_glyf(ttfont, glyf, vert, orgmatrix);
    return 0;
  }

  /* complex case */
  if(level >= MAX_COMPOSITE_LEVEL) {

    /*	fprintf(stderr,"*** Glyph: stopped (possibly infinite) recursion
	at depth %d\n",level); */
    return 0;
  }

  ptr = ((unsigned char *) glyf + sizeof(ay_tti_glyf));
  sptr = (signed short *) ptr;
  do {
    flagbyte = ntohs(*sptr);
    sptr++;
    glyphindex = ntohs(*sptr);
    sptr++;

    if(flagbyte & ARG_1_AND_2_ARE_WORDS)
      {
	arg1 = (signed short)ntohs(*sptr);
	sptr++;
	arg2 = (signed short)ntohs(*sptr);
	sptr++;
      }
    else
      {
	bptr = (char *) sptr;
	arg1 = (signed char) bptr[0];
	arg2 = (signed char) bptr[1];
	sptr++;
      }

    matrix[1] = matrix[2] = 0.0;

    if(flagbyte & WE_HAVE_A_SCALE) {
      matrix[0] = matrix[3] = f2dot14(*sptr);
      sptr++;
    } else if(flagbyte & WE_HAVE_AN_X_AND_Y_SCALE) {
      matrix[0] = f2dot14(*sptr);
      sptr++;
      matrix[3] = f2dot14(*sptr);
      sptr++;
    } else if(flagbyte & WE_HAVE_A_TWO_BY_TWO) {
      matrix[0] = f2dot14(*sptr);
      sptr++;
      matrix[2] = f2dot14(*sptr);
      sptr++;
      matrix[1] = f2dot14(*sptr);
      sptr++;
      matrix[3] = f2dot14(*sptr);
      sptr++;
    } else {
      matrix[0] = matrix[3] = 1.0;
    }

    /*
     * See *
     * http://fonts.apple.com/TTRefMan/RM06/Chap6g
     * lyf.html * matrix[0,1,2,3,4,5]=a,b,c,d,m,n
     */

    if(fabs(matrix[0]) > fabs(matrix[1]))
      matrix[4] = fabs(matrix[0]);
    else
      matrix[4] = fabs(matrix[1]);
    if(fabs(fabs(matrix[0]) - fabs(matrix[2])) <= 33. / 65536.)
      matrix[4] *= 2.0;

    if(fabs(matrix[2]) > fabs(matrix[3]))
      matrix[5] = fabs(matrix[2]);
    else
      matrix[5] = fabs(matrix[3]);
    if(fabs(fabs(matrix[2]) - fabs(matrix[3])) <= 33. / 65536.)
      matrix[5] *= 2.0;

    /*
     * fprintf (stderr,"Matrix Opp %hd
     * %hd\n",arg1,arg2);
     */
#if 0
    fprintf(stderr, "Matrix: %f %f %f %f %f %f\n",
	    matrix[0], matrix[1], matrix[2], matrix[3],
	    matrix[4], matrix[5]);
    fprintf(stderr, "Offset: %f %f (%s)\n",
	    arg1, arg2,
	    ((flagbyte & ARGS_ARE_XY_VALUES) ? "XY" : "index"));
#endif

    if(flagbyte & ARGS_ARE_XY_VALUES) {
      matrix[4] *= arg1;
      matrix[5] *= arg2;
    } else {
      /*  fprintf(stderr,
	"*** Glyph:  reusing scale from another glyph is unsupported\n");  */
      /*
       * must extract values from a glyph
       * but it seems to be too much pain
       * and it's not clear now that it
       * would be really used in any
       * interesting font
       */
    }

    /* at this point arg1,arg2 contain what logically should be matrix[4,5] */

    /* combine matrices */

    newmatrix[0] = orgmatrix[0]*matrix[0] + orgmatrix[2]*matrix[1];
    newmatrix[1] = orgmatrix[0]*matrix[2] + orgmatrix[2]*matrix[3];

    newmatrix[2] = orgmatrix[1]*matrix[0] + orgmatrix[3]*matrix[1];
    newmatrix[3] = orgmatrix[1]*matrix[2] + orgmatrix[3]*matrix[3];

    newmatrix[4] = orgmatrix[0]*matrix[4] + orgmatrix[2]*matrix[5] +
      orgmatrix[4];
    newmatrix[5] = orgmatrix[1]*matrix[4] + orgmatrix[3]*matrix[5] +
      orgmatrix[5];


    ttfont->nglyf = glyphindex; /* glyf number in Font */

    if(ttfont->locformat)
      {
	/* long format */
	uilo = (unsigned int*)ttfont->loca;
	go = ntohl(uilo[glyphindex]);
      }
    else
      {
	uslo = (unsigned short*)ttfont->loca;
	go = ntohs(uslo[glyphindex]) * 2;
      }

    glyf = (ay_tti_glyf*)(ttfont->glyf + go);

    ay_tti_handle_comp_glyf(ttfont, glyf, newmatrix, level + 1, vert);

  } while(flagbyte & MORE_COMPONENTS);

 return 0;
} /* ay_tti_handle_comp_glyf */


int
ay_tti_handle_simple_glyf(ay_tti_font *ttfont, ay_tti_glyf *glyf,
			  ay_tti_letter *vert, double *matrix)
{
 unsigned short *contour_end_pt;
 signed short ncontours, n_inst, last_point;
 unsigned char *ptr;
#define GLYFSZ 2000
 signed short xabs[GLYFSZ], yabs[GLYFSZ], xrel[GLYFSZ], yrel[GLYFSZ];
 unsigned char flags[GLYFSZ];
 signed short j, k, k1, co, lp, lsb, startx, yoffset;
 double	scale_factor;
 double	tx, ty;
 int i, maxpoint;
 double x1 = 0.0, y1 = 0.0, x2, y2, x3, y3;

  ncontours = ntohs(glyf->numberOfContours);
  contour_end_pt = (unsigned short *) ((unsigned char *) glyf +
				       sizeof(ay_tti_glyf));

  vert->numoutlines = ncontours;
  vert->outlines = (ay_tti_outline *)calloc(1, sizeof(ay_tti_outline) *
					    ncontours);

  last_point = ntohs(contour_end_pt[ncontours - 1]);
  n_inst = ntohs(contour_end_pt[ncontours]);

  ptr = ((unsigned char *) contour_end_pt) + (ncontours << 1) + n_inst + 2;

  j = k = 0;
  while(k <= last_point)
    {
      flags[k] = ptr[j];

      if(ptr[j] & REPEAT)
	{
	  for(k1 = 0; k1 < ptr[j + 1]; k1++)
	    {
	      k++;
	      flags[k] = ptr[j];
	    } /* for */
	  j++;
	} /* if */
      j++;
      k++;
    } /* while */

  for(k = 0; k <= last_point; k++)
    {
      if(flags[k] & XSHORT)
	{
	  if(flags[k] & XSAME)
	    {
	      xrel[k] = ptr[j];
	    }
	  else
	    {
	      xrel[k] = -ptr[j];
	    } /* if */
	  j++;
	}
      else
	{
	  if(flags[k] & XSAME)
	    {
	      xrel[k] = 0.0;
	    }
	  else
	    {
	      xrel[k] = (short)( ptr[j] * 256 + ptr[j + 1] );
	      j += 2;
	    } /* if */
	} /* if */

      if(k == 0)
	{
	  xabs[k] = xrel[k];
	}
      else
	{
	  xabs[k] = xrel[k] + xabs[k - 1];
	}

    } /* for */

  for(k = 0; k <= last_point; k++)
    {
      if(flags[k] & YSHORT)
	{
	  if(flags[k] & YSAME)
	    {
	      yrel[k] = ptr[j];
	    }
	  else
	    {
	      yrel[k] = -ptr[j];
	    }
	  j++;
	}
      else
	{
	  if(flags[k] & YSAME)
	    {
	      yrel[k] = 0;
	    }
	  else
	    {
	      yrel[k] = ptr[j] * 256 + ptr[j + 1];
	      j += 2;
	    }
	} /* if */

      if(k == 0)
	{
	  yabs[k] = yrel[k];
	}
      else
	{
	  yabs[k] = yrel[k] + yabs[k - 1];
	}
    } /* for */


  scale_factor = 1.0; /*2000.0 / (double) ttfont->unitem;*/

  if(matrix)
    {
      for(i = 0; i <= last_point; i++)
	{
	  tx = xabs[i];
	  ty = yabs[i];
	  xabs[i] = scale_factor*(matrix[0] * tx + matrix[2] * ty + matrix[4]);
	  yabs[i] = scale_factor*(matrix[1] * tx + matrix[3] * ty + matrix[5]);
	} /* for */
    }
  else
    {
      for(i = 0; i <= last_point; i++)
	{
	  xabs[i] = scale_factor*(xabs[i]);
	  yabs[i] = scale_factor*(yabs[i]);
	}
    }

  k = 0;
  lp = 0;

  lsb = ntohs(ttfont->hmtx[ttfont->nglyf].lsb);
  startx = ttfont->soffset;
  yoffset = ttfont->yoffset;

  for(co = 0; co < ncontours; co++)
    {
      x1 = 0.0;
      y1 = 0.0;

      last_point = ntohs(contour_end_pt[co]);

      maxpoint = 2000;

      vert->outlines[co].points = (ay_tti_point *)calloc(1,
					   sizeof(ay_tti_point)*(maxpoint));
      vert->outlines[co].numpoints = 0;
      lp = last_point+1;

      i = 0;

      vert->outlines[co].filled = ay_tti_isclockwise(&xabs[k], &yabs[k],
						 (last_point-k));

      while(k <= last_point)
	{
	  if(flags[k] & ONOROFF)
	    {
	      
	      x1 = (xabs[k] - lsb + startx) * scale_factor;
	      y1 = (yabs[k] - ttfont->descent + yoffset) * scale_factor;

	      if(k == last_point)
		{
		  i = vert->outlines[co].numpoints;

		  vert->outlines[co].points[i].x = x1;
		  vert->outlines[co].points[i].y = y1;
		  vert->outlines[co].numpoints++;

		  x2 = vert->outlines[co].points[0].x;
		  y2 = vert->outlines[co].points[0].y;
		  
		  vert->outlines[co].points[i+1].x = (x1+x2)/2;
		  vert->outlines[co].points[i+1].y = (y1+y2)/2;
		  vert->outlines[co].numpoints++;
 		  
		  vert->outlines[co].points[i+2].x = vert->outlines[co].points[0].x;
		  vert->outlines[co].points[i+2].y = vert->outlines[co].points[0].y;
		  vert->outlines[co].numpoints++;
		}

	      if(((k + 1) <= last_point) && (flags[k+1] & ONOROFF))
		{
		  i = vert->outlines[co].numpoints;

		  vert->outlines[co].points[i].x = x1;
		  vert->outlines[co].points[i].y = y1;
		  vert->outlines[co].numpoints++;
		
		  /* create a bezier curve for a line */
		  x2 = (xabs[k+1] - lsb + startx) * scale_factor;
	      y2 = (yabs[k+1] - ttfont->descent + yoffset) * scale_factor;

		  vert->outlines[co].points[i+1].x = (x1+x2)/2;
		  vert->outlines[co].points[i+1].y = (y1+y2)/2;
		  vert->outlines[co].numpoints++;

		  vert->outlines[co].points[i+2].x = x2;
		  vert->outlines[co].points[i+2].y = y2;
		  vert->outlines[co].numpoints++;
		  /*k++;*/
		}

	      k++;
	    }
	  else
	    {
	      if(k == last_point)
		{
		  x2 = (xabs[k] - lsb+startx) * scale_factor;
		  y2 = (yabs[k] - ttfont->descent + yoffset) * scale_factor;

		  x3 = vert->outlines[co].points[0].x;
		  y3 = vert->outlines[co].points[0].y;

		  /* ay_tti_makearc(x1, y1, x2, y2, x3, y3, &vert->outlines[co],
		     step); */

		  i = vert->outlines[co].numpoints;
		  vert->outlines[co].points[i].x = x1;
		  vert->outlines[co].points[i].y = y1;
		  vert->outlines[co].points[i+1].x = x2;
		  vert->outlines[co].points[i+1].y = y2;
		  vert->outlines[co].points[i+2].x = x3;
		  vert->outlines[co].points[i+2].y = y3;
		  vert->outlines[co].numpoints += 3;

		  k++;
		}
	      else
		{
		  if(flags[k+1] & ONOROFF)
		    {
		      x2 = (xabs[k] - lsb + startx) * scale_factor;
		      y2 = (yabs[k] - ttfont->descent + yoffset) *
			scale_factor;

		      x3 = (xabs[k+1] - lsb + startx) * scale_factor;
		      y3 = (yabs[k+1] - ttfont->descent + yoffset) *
			scale_factor;

		      /* ay_tti_makearc(x1, y1, x2, y2, x3, y3,
			 &vert->outlines[co], step); */

		      i = vert->outlines[co].numpoints;
		      vert->outlines[co].points[i].x = x1;
		      vert->outlines[co].points[i].y = y1;
		      vert->outlines[co].points[i+1].x = x2;
		      vert->outlines[co].points[i+1].y = y2;
		      vert->outlines[co].points[i+2].x = x3;
		      vert->outlines[co].points[i+2].y = y3;
		      vert->outlines[co].numpoints += 3;

		      if(flags[k+2] & ONOROFF)
			k += 1;
		      else
			k += 2;

		      x1 = x3;
		      y1 = y3;

		      if(k-1 == last_point)
			{
			  i = vert->outlines[co].numpoints;

			  vert->outlines[co].points[i].x =
			    vert->outlines[co].points[i-1].x;
			  vert->outlines[co].points[i].y =
			    vert->outlines[co].points[i-1].y;

			  vert->outlines[co].points[i+1].x =
			    (vert->outlines[co].points[i-1].x+
			     vert->outlines[co].points[0].x)/2.0;

			  vert->outlines[co].points[i+1].y =
			    (vert->outlines[co].points[i-1].y+
			     vert->outlines[co].points[0].y)/2.0;

			  vert->outlines[co].points[i+2].x =
			    vert->outlines[co].points[0].x;
			  vert->outlines[co].points[i+2].y =
			    vert->outlines[co].points[0].y;

			  vert->outlines[co].numpoints +=3 ;
			}
		    }
		  else
		    {
		      x2 = (xabs[k] - lsb + startx) * scale_factor;
		      y2 = (yabs[k] - ttfont->descent + yoffset) *
			scale_factor;

		      x3 = (xabs[k+1] - lsb+startx) * scale_factor;
		      y3 = (yabs[k+1] - ttfont->descent + yoffset) *
			scale_factor;

		      x3 = (x2 + x3) / 2; /* implicit on curve point */
		      y3 = (y2 + y3) / 2;

		      /* ay_tti_makearc(x1, y1, x2, y2, x3, y3,
			 &vert->outlines[co], step); */

		      i = vert->outlines[co].numpoints;
		      vert->outlines[co].points[i].x = x1;
		      vert->outlines[co].points[i].y = y1;
		      vert->outlines[co].points[i+1].x = x2;
		      vert->outlines[co].points[i+1].y = y2;
		      vert->outlines[co].points[i+2].x = x3;
		      vert->outlines[co].points[i+2].y = y3;
		      vert->outlines[co].numpoints += 3;

		      k += 1;
		      x1 = x3;
		      y1 = y3;
		    } /* end else*/

		} /* end else */

	    } /* end else */

	} /* end while */

      vert->outlines[co].points = realloc(vert->outlines[co].points,
			(sizeof(ay_tti_point)*vert->outlines[co].numpoints + 1));

    } /* end for */

 return 0;
} /* ay_tti_handle_simple_glyf */


void
ay_tti_read_head(ay_tti_font *ttfont)
{
 unsigned char *buffer,*tmp;

  buffer = calloc( 1, ttfont->size);

  if(buffer)
    {
      fread(buffer, 1, ttfont->size, ttfont->fontptr);

      tmp = buffer+16;
      ttfont->flags = read16bit(tmp);
      tmp = buffer+18;
      ttfont->unitem = read16bit(tmp);
      tmp = buffer+50;
      ttfont->locformat = read16bit(tmp);

      free(buffer);
    } /* if */

 return;
} /* ay_tti_read_head */


int
ay_tti_searchtable(ay_tti_font *ttfont, char *tablename)
{
 int count = 0;
 unsigned char dummy[5], *dummyptr;
 int success, offset;

  fseek(ttfont->fontptr, 12, SEEK_SET);

  dummy[4] = 0;
  success = 0;

  while((count < ttfont->numtable) && !success)
    {
      fread(dummy, 1, 4, ttfont->fontptr);

      if(!(strcmp(tablename, dummy)))
	{
	  success = 1;
	}
      else
	{
	  fseek(ttfont->fontptr, 12, SEEK_CUR);
	}

      count++;
    } /* while */

  if(success)
    {
      fread(ttfont->buffer, 1, 12, ttfont->fontptr);

      dummyptr = ttfont->buffer + 4;
      offset = read32bit(dummyptr);
      dummyptr += 4;

      ttfont->size = read32bit(dummyptr);

      fseek(ttfont->fontptr, offset, SEEK_SET);
    }

 return success;
} /* ay_tti_searchtable */


/*
**
** find glyf index
**
**/
int
ay_tti_findglyf(TTF_CMAP4 *cm, int c)
{
 int i, seg_c2, cmap_n_segs, n;
 char *ptr;
 unsigned short *cmap_seg_end, *cmap_seg_start;
 signed short *cmap_idDelta, *cmap_idRangeOffset, ro, delta;
 unsigned int found;

  if(ntohs(cm->format) == 4)
    {
      seg_c2 = ntohs(cm->segCountX2);
      cmap_n_segs = seg_c2 >> 1;
      ptr = (char *) cm + 14;
      cmap_seg_end = (unsigned short *) ptr;
      cmap_seg_start = (unsigned short *) (ptr + seg_c2 + 2);
      cmap_idDelta = (signed short *) (ptr + (seg_c2 * 2) + 2);
      cmap_idRangeOffset = (signed short *) (ptr + (seg_c2 * 3) + 2);

      /* search range for code */
      i = 0;
      found = 0;
      do
	{
	  if(ntohs(cmap_seg_end[i]) >= c)
	    found = 1;
	}
      while((i < cmap_n_segs) && (!found));

      if(ntohs(cmap_seg_start[i]) > c)
	return 0; /* glyf not found */

      ro = ntohs(cmap_idRangeOffset[i]);
      delta = ntohs(cmap_idDelta[i]);

      if(ro == 0)
	{
	  n = c + delta;
	}
      else
	{
	  n = ntohs(*((ro >> 1) +
	       (c - ntohs(cmap_seg_start[i])) + &cmap_idRangeOffset[i]));
	}

      return n;
    } /* if */

 return 0;
} /* ay_tti_findglyf */


/*
**
** get wanted glyf
**
**/
void
ay_tti_getglyf(ay_tti_font *ttfont, int c, ay_tti_glyf **glyf)
{
 int i, k, gl, go, format, platform;
 unsigned int  b;
 TTF_CMAP_ENTRY *cme = 0;
 TTF_CMAP4 *cm4 = 0;
 unsigned int *uilo;
 unsigned short *uslo;

  /* number of tables in font */
  k = ntohs(ttfont->cmap->numberOfEncodingTables);

  i = 0;
  b = 0;

  while((i < k) && !b)
    {
      cme = &(ttfont->cmap->encodingTable[i]);
      format = ntohs(cme->encodingID);
      platform = ntohs(cme->platformID);
      if((format == 1) && (platform == 3)) /* CMAP4 */
	{
	  b = 1;
	}

      i++;
    } /* while */

  k = ntohl(cme->offset); /* table offset */

  cm4 = (TTF_CMAP4 *)((unsigned char *)ttfont->cmap + k);

  gl = ay_tti_findglyf(cm4, c); /* find glyf */

  ttfont->nglyf = gl; /* glyf number in Font */

  if(ttfont->locformat)
    {
      /* long format */
      uilo = (unsigned int*)ttfont->loca;
      go = ntohl(uilo[gl]);
    }
  else
    {
      uslo = (unsigned short*)ttfont->loca;
      go = ntohs(uslo[gl]) * 2;
    }

  /* printf("Glyf offset: %x\n",go); */

  *glyf = (ay_tti_glyf*)(ttfont->glyf + go);

 return;
} /* ay_tti_getglyf */


#if 0
static TMODAPI int
ay_tti_getstring(TMOD_TRUETYPE *ttype, ay_tti_font *ttfont, char *s,
		 TTSTR *tstr)
{
  int		i,sl,lines,o;
  ay_tti_glyf *glyf = 0;
  char c;
  double scale,tw;
  signed short 	out;
  signed short w,l,xmin,xmax;
  signed short pp1;
  char * tmpstr;
  double  matrix[6];

  sl = strlen(s);
  tstr->numchar=sl;

  tmpstr = s;
  lines = 0;
  /* check for newlines */
  for(i=0; i<sl; i++)
    {
      if(*tmpstr=='\n')
	lines++;

      tmpstr++;
    }

  tstr->verts = (ay_tti_letter*)calloc(1, sizeof(ay_tti_letter)*sl);
  ttfont->soffset = 0;
  ttfont->yoffset = (ttfont->ascent - ttfont->descent)*lines;
  tstr->maxpoints = 0;

  scale = 1000.0 / (double) ttfont->unitem;

  for(i = 0; i < sl; i++)
    {
      c = *s;

      if(c == '\n')
	{
	  ttfont->yoffset -= (ttfont->ascent - ttfont->descent);
	  ttfont->soffset = 0;
	  s += 1;
	  i += 1;
	  c = *s;
	}

      s++;
      /*	printf("char %c\n",c); */

      ay_tti_getglyf(ttype, ttfont, c, &glyf);

      out = ntohs(glyf->numberOfContours);

      /*	printf("Outlines %d\n",out); */

      matrix[0] = matrix[3] = 1.0;
      matrix[1] = matrix[2] = matrix[4] = matrix[5] = 0.0;

      if(out < 0)
	{
	  /* printf("Component GLYF!!!!!!!!!\n"); */
	  ay_tti_handle_comp_glyf(ttfont, glyf, matrix, 0 /* level */,
				  &tstr->verts[i]);
	}
      else
	ay_tti_handle_simple_glyf(ttfont, glyf, &tstr->verts[i], matrix);

      for(o = 0; o < tstr->verts[i].numoutlines; o++)
	{
	  if(tstr->verts[i].outlines[o].numpoints > tstr->maxpoints)
	    tstr->maxpoints = tstr->verts[i].outlines[o].numpoints;
	}

      /* new start for next glyf */
      pp1 = ntohs(glyf->xMin) - ntohs(ttfont->hmtx[ttfont->nglyf].lsb);

      ttfont->soffset += (ntohs(ttfont->hmtx[ttfont->nglyf].advanceWidth));

      xmin = ntohs(glyf->xMin);
      xmax = ntohs(glyf->xMax);
      w = ntohs(ttfont->hmtx[ttfont->nglyf].advanceWidth);
      l = ntohs(ttfont->hmtx[ttfont->nglyf].lsb);

      /*	printf("rsb %d\n",w-(l+xmax-xmin)); */

      /*	ttfont->soffset += (w+(l+xmax-xmin)); */ /* courier ???? */

      /*	printf("width %d  lsb %d\n",w, l); */

      tw = ttfont->soffset * scale;

      if(tstr->width < tw)
	tstr->width = tw;

    }

  tstr->height = ((ttfont->ascent - ttfont->descent) * (lines + 1)) * scale;

 return 0;
}
#endif


int
ay_tti_getchar(ay_tti_font *ttfont, int c, ay_tti_letter *vert)
{
 int out;
 ay_tti_glyf *glyf = 0;
 double  matrix[6];

 /*  ttfont->soffset = 0;*/
     ttfont->yoffset = 0;

  ay_tti_getglyf(ttfont, c, &glyf);

  out = ntohs(glyf->numberOfContours);
  /*	printf("Outlines %d\n",out); */

  /* store number of outlines */
  vert->numoutlines = out;
  /*	vert->outlines = (ay_tti_outline *)TExecAlloc0(TExecBase, 0,
	sizeof(ay_tti_outline)*out); */

#if 0
  printf("xMin %d\n",TUtilNToHS(TUtilBase, glyf->xMin));
  printf("yMin %d\n",TUtilNToHS(TUtilBase, glyf->yMin));

  printf("xMax %d\n",TUtilNToHS(TUtilBase, glyf->xMax));
  printf("yMax %d\n",TUtilNToHS(TUtilBase, glyf->yMax));
#endif

  matrix[0] = matrix[3] = 1.0;
  matrix[1] = matrix[2] = matrix[4] = matrix[5] = 0.0;

  if(out < 0)
    {
      /*	printf("Component GLYF!!!!!!!!!\n"); */
      ay_tti_handle_comp_glyf(ttfont, glyf, matrix, 0 /* level */, vert);
    }
  else
    {
      ay_tti_handle_simple_glyf(ttfont, glyf, vert, matrix);
    }

  ttfont->soffset += (ntohs(ttfont->hmtx[(ttfont->numberOfHMetrics>1)?ttfont->nglyf:0].advanceWidth));

 return 0;
} /* ay_tti_getchar */


/* open font file to initialize font struct */
int
ay_tti_open(ay_tti_font *ttfont, char *font)
{
 unsigned char *dummy;
 int idummy;
 int error = 0;
 TTF_DIRECTORY *tdir;
 TTF_HHEA *hhea;

  ttfont->fontptr = fopen(font, "rb");

  if(ttfont->fontptr)
    {
      /* check if we have a truetype font */
      ttfont->buffer = calloc(1, 16);
      dummy = ttfont->buffer;

      fread(ttfont->buffer, 1, 16, ttfont->fontptr);

      tdir = (TTF_DIRECTORY *) ttfont->buffer;
      idummy = ntohl(tdir->sfntVersion);

      if((idummy == 0x00010000) || (idummy == 0x74727565))
	{
	  ttfont->numtable = 0;
	  ttfont->numtable = ntohs(tdir->numTables);

	  /* search head table */
	  if(!(ay_tti_searchtable(ttfont, "head")))
	    {error = AY_TTI_BADFONT; goto cleanup;}

	  ay_tti_read_head(ttfont);

	  /* search cmap table */
	  if(!(ay_tti_searchtable(ttfont, "cmap")))
	    {error = AY_TTI_BADFONT; goto cleanup;}

	  ttfont->cmap = (TTF_CMAP *)calloc(1, ttfont->size);
	  fread( (unsigned char *) ttfont->cmap, 1, ttfont->size,
		 ttfont->fontptr);

	  /* search loca table */
	  if(!(ay_tti_searchtable(ttfont, "loca")))
	    {error = AY_TTI_BADFONT; goto cleanup;}

	  ttfont->loca = calloc(1, ttfont->size);
	  fread(ttfont->loca, 1, ttfont->size, ttfont->fontptr);

	  /* search glyf table */
	  if(!(ay_tti_searchtable(ttfont, "glyf")))
	    {error = AY_TTI_BADFONT; goto cleanup;}

	  ttfont->glyf = calloc(1, ttfont->size);
	  fread(ttfont->glyf, 1, ttfont->size, ttfont->fontptr);


	  /* search hhea table */
	  if(!(ay_tti_searchtable(ttfont, "hhea")))
	    {error = AY_TTI_BADFONT; goto cleanup;}

	  hhea = calloc(1, ttfont->size);
	  if(hhea)
	    {
	      fread((unsigned char *) hhea, 1, ttfont->size, ttfont->fontptr);

	      ttfont->ascent = ntohs(hhea->ascender);
	      ttfont->descent = ntohs(hhea->descender);
	      ttfont->numberOfHMetrics = ntohs(hhea->numberOfHMetrics);
	      free(hhea);
	    }


	  /* search hmtx table */
	  if(!(ay_tti_searchtable(ttfont, "hmtx")))
	    {error = AY_TTI_BADFONT; goto cleanup;}

	  ttfont->hmtx = calloc(1, ttfont->size);
	  fread( (unsigned char *) ttfont->hmtx, 1, ttfont->size,
		 ttfont->fontptr);

	  fclose(ttfont->fontptr);
	  return AY_TTI_OK;
	}
      else
	{		/* not a font */
	  fclose(ttfont->fontptr);
	  free(ttfont->buffer);
	  return AY_TTI_NOFOUND;
	} /* if */
    }
  else
    {
      fclose(ttfont->fontptr);
      return AY_TTI_NOFOUND;
    } /* if */

cleanup:
  fclose(ttfont->fontptr);

 return error;
} /* ay_tti_open */


void
ay_tti_close(ay_tti_font *ttfont)
{

  if(ttfont)
    {
      free(ttfont->buffer);
      free(ttfont->cmap);
      free(ttfont->loca);
      free(ttfont->glyf);
      free(ttfont->hmtx);
    }

 return;
} /* ay_tti_close */


void
ay_tti_freeletter(ay_tti_letter *vert)
{
 int i;

  if(vert)
    {
      if(vert->outlines)
	{
	  for(i = 0; i < vert->numoutlines; i++)
	    {
	      if(vert->outlines[i].points)
		{
		  if(vert->outlines[i].points)
		    {
		      free(vert->outlines[i].points);
		    }
		}
	    } /* for */

	  free(vert->outlines);
	  vert->outlines = 0;
	} /* if */

    } /* if */

 return;
} /* ay_tti_freeletter */


int
ay_tti_getcurves(char *ttfname, int letter, ay_tti_letter *cur)
{
 static ay_tti_font *font = NULL;
 int error = 0;

  if(!ttfname)
    {
      if(font)
	{
	  ay_tti_close(font);
	  free(font);
	}
      font = NULL;
      return error;
    }

  if(!font)
    {
      if(!(font = calloc(1, sizeof(ay_tti_font))))
	return error;

      error = ay_tti_open(font, ttfname); /* open font */
      if(error != AY_TTI_OK)
	{
	  return error;
	}
    }

  error = ay_tti_getchar(font, letter, cur); /* get curves for letter */


 return error;
} /* ay_tti_getcurves */


int
ay_tti_outlinetoncurve(ay_tti_outline *outline, ay_object **result)
{
 int ay_status = AY_OK;
 ay_object *new = NULL;
 int i, a, order, length, stride = 4;
 double *controlv = NULL, *knotv = NULL;

  if(!outline || !result)
    return AY_ENULL;

  length = outline->numpoints;
  order = 3;
  
  if(!(controlv = calloc(length * stride, sizeof(double))))
    { ay_status = AY_EOMEM; goto cleanup; }

  a = 0;
  for(i = 0; i < length; i++)
    {
      controlv[a]     = (outline->points[i]).x/2048.0;
      controlv[a + 1] = (outline->points[i]).y/2048.0;
      controlv[a + 3] = 1.0;
      /*      printf("%g, %g\n",controlv[a],controlv[a + 1]);*/
      a += stride;
    }

  if(!(knotv = calloc(length + order, sizeof(double))))
    { ay_status = AY_EOMEM; goto cleanup; }
  
  a = 0;
  for(i = 0; i < (length + order)/3; i++)
    {
      knotv[a] = i;
      knotv[a + 1] = i;
      knotv[a + 2] = i;
      a += 3;
    }

  if(!(new = calloc(1, sizeof(ay_object))))
    { ay_status = AY_EOMEM; goto cleanup; }
  ay_object_defaults(new);
  new->type = AY_IDNCURVE;

  ay_status = ay_nct_create(order, length, AY_KTCUSTOM, controlv, knotv,
			    (ay_nurbcurve_object **)&(new->refine));

  if(ay_status)
    { goto cleanup; }

  *result = new;

  new = NULL;
  knotv = NULL;
  controlv = NULL;

 cleanup:
  if(controlv)
    free(controlv);
  if(knotv)
    free(knotv);
  if(new)
    free(new);

 return ay_status;
} /* ay_tti_outlinetoncurve */
