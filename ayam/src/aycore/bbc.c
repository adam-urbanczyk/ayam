/*
 * Ayam, a free 3D modeler for the RenderMan interface.
 *
 * Ayam is copyrighted 1998-2001 by Randolf Schultz
 * (rschultz@informatik.uni-rostock.de) and others.
 *
 * All rights reserved.
 *
 * See the file License for details.
 *
 */

#include "ayam.h"

/* bbc.c - bounding box calculation */

/* ay_bbc_get:
 *  
 */
int
ay_bbc_get(ay_object *o, double *bbox)
{
 int ay_status = AY_OK;
 char fname[] = "bbc_get";
 ay_object *d = NULL;
 double xmin = DBL_MAX, xmax = -DBL_MAX, ymin = DBL_MAX;
 double ymax = -DBL_MAX, zmin = DBL_MAX, zmax = -DBL_MAX;
 double bbt[24] = {0};
 int i, a, flags = AY_FALSE;
 void **arr = NULL;
 ay_bbccb *cb = NULL;
 double m[16] = {0}, mr[16];
 int have_child_bb = AY_FALSE;

  if(!o || !bbox)
    return AY_ENULL;

  /* get transformations */
  glMatrixMode (GL_MODELVIEW);
  glPushMatrix();
   glLoadIdentity();

   glTranslated(o->movx, o->movy, o->movz);
	
   ay_quat_torotmatrix(o->quat, mr);
   glMultMatrixd(mr);
  
   glScaled (o->scalx, o->scaly, o->scalz);
   glGetDoublev(GL_MODELVIEW_MATRIX, (GLdouble *)m);
  glPopMatrix();

  /* get bounding boxes of children */
  if(o->down)
    {
      d = o->down;  
      while(d->next)
	{
	  ay_status = ay_bbc_get(d, bbt);
	  if(!ay_status)
	    {
	      /* apply transformations */
	      if(o->inherit_trafos)
		{
		  a = 0;
		  for(i = 0; i < 8; i++)
		    {
		      ay_trafo_apply3(&(bbt[a]), m);
		      a += 3;
		    }
		}

	      a = 0;
	      for(i = 0; i < 8; i++)
		{
		  if(bbt[a] < xmin)
		    xmin = bbt[a];
		  if(bbt[a] > xmax)
		    xmax = bbt[a];
		  a += 3;
		}
	  
	      a = 1;
	      for(i = 0; i < 8; i++)
		{
		  if(bbt[a] < ymin)
		    ymin = bbt[a];
		  if(bbt[a] > ymax)
		    ymax = bbt[a];
		  a += 3;
		}

	      a = 2;
	      for(i = 0; i < 8; i++)
		{
		  if(bbt[a] < zmin)
		    zmin = bbt[a];
		  if(bbt[a] > zmax)
		    zmax = bbt[a];
		  a += 3;
		}
	      have_child_bb = AY_TRUE;
	    } /* if */

	  d = d->next;
	} /* while */
    } /* if */

  if(o)
    {
      arr = ay_bbccbt.arr;
      cb = (ay_bbccb *)(arr[o->type]);
      if(cb)
	ay_status = cb(o, bbt, &flags);

      if(ay_status)
	{
	  ay_error(AY_ERROR, fname, "bbc callback failed");
	  return AY_ERROR;
	}
    }

  if(flags == 1)
    { /* exclusive bounding box, discard child(ren) bounding box(es) */
      xmin = DBL_MAX; xmax = -DBL_MAX; ymin = DBL_MAX;
      ymax = -DBL_MAX; zmin = DBL_MAX; zmax = -DBL_MAX;
    }
  
  if(flags != 2)
    { /* bounding box of object o is not marked invalid/non-existent */
      /* thus, merge bounding box of object o with child(ren) bounding box */

      /* apply transformations */
      a = 0;
      for(i = 0; i < 8; i++)
	{
	  ay_trafo_apply3(&(bbt[a]), m);
	  a += 3;
	}

      a = 0;
      for(i = 0; i < 8; i++)
	{
	  if(bbt[a] < xmin)
	    xmin = bbt[a];
	  if(bbt[a] > xmax)
	    xmax = bbt[a];
	  a += 3;
	}

      a = 1;
      for(i = 0; i < 8; i++)
	{
	  if(bbt[a] < ymin)
	    ymin = bbt[a];
	  if(bbt[a] > ymax)
	    ymax = bbt[a];
	  a += 3;
	}

      a = 2;
      for(i = 0; i < 8; i++)
	{
	  if(bbt[a] < zmin)
	    zmin = bbt[a];
	  if(bbt[a] > zmax)
	    zmax = bbt[a];
	  a += 3;
	}
      }
  else
    { /* o marks its bb as nonexistent */

      /* check, if we have child bb's to use */
      if(!have_child_bb)
	{
	  /* no, so we have _no_ bb at all */
	  return AY_ERROR;
	} /* if */
    } /* if */

  /* fill in results */
  /* P1 */
  bbox[0] = xmin; bbox[1] = ymax; bbox[2] = zmax;
  /* P2 */
  bbox[3] = xmin; bbox[4] = ymax; bbox[5] = zmin;
  /* P3 */
  bbox[6] = xmax; bbox[7] = ymax; bbox[8] = zmin;
  /* P4 */
  bbox[9] = xmax; bbox[10] = ymax; bbox[11] = zmax;

  /* P5 */
  bbox[12] = xmin; bbox[13] = ymin; bbox[14] = zmax;
  /* P6 */
  bbox[15] = xmin; bbox[16] = ymin; bbox[17] = zmin;
  /* P7 */
  bbox[18] = xmax; bbox[19] = ymin; bbox[20] = zmin;
  /* P8 */
  bbox[21] = xmax; bbox[22] = ymin; bbox[23] = zmax;

 return AY_OK;
} /* ay_bbc_get */
