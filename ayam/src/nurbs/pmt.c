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

/* pmt.c PatchMesh tools */

/* functions local to this module */
int ay_pmt_bilinearcltonpatch(ay_pamesh_object *pamesh, ay_object **result);

/* ay_pmt_bilinearcltonpatch:
 *   create a NURBS patch from a closed bilinear PatchMesh
 */
int
ay_pmt_bilinearcltonpatch(ay_pamesh_object *pamesh, ay_object **result)
{
 double *cv = NULL;
 ay_object *o = NULL;
 int w, h, i, a, b;
 int ay_status = AY_OK;

  if(!(o = calloc(1, sizeof(ay_object))))
    return AY_EOMEM;
  ay_object_defaults(o);
  o->type = AY_IDNPATCH;

  if(pamesh->close_u)
    w = pamesh->width+1;
  else
    w = pamesh->width;

  if(pamesh->close_v)
    h = pamesh->height+1;
  else
    h = pamesh->height;

  if(!(cv = calloc(w*h*4, sizeof(double))))
    {
      free(o);
      return AY_EOMEM;
    }


  a = 0; b = 0;
  for(i = 0; i < pamesh->width; i++)
    {
      memcpy(&(cv[a]), &(pamesh->controlv[b]),
	     pamesh->height*4*sizeof(double));
      

      a += pamesh->height*4;
      if(pamesh->close_v)
	{
	  memcpy(&(cv[a]), &(pamesh->controlv[b]), 4*sizeof(double));
	  a += 4;
	}
      b += pamesh->height*4;

    } /* for */

  if(pamesh->close_u)
    {
      b = 0;
      if(pamesh->close_v)
	{
	  memcpy(&(cv[a]), &(pamesh->controlv[b]),
		 pamesh->height*4*sizeof(double));
	}
      else
	{
	  memcpy(&(cv[a]), &(pamesh->controlv[b]),
		 h*4*sizeof(double));
	}
    }

  ay_status = ay_npt_create(2, 2, w, h, AY_KTNURB, AY_KTNURB, cv, NULL, NULL,
			    (ay_nurbpatch_object **)&(o->refine));
  *result = o;

 return AY_OK;
} /* ay_pmt_bilinearcltonpatch */


/* ay_pmt_tonpatch:
 *   create a number of NURBS patches from a PatchMesh
 */
int
ay_pmt_tonpatch(ay_pamesh_object *pamesh, ay_object **result)
{
 double *cv = NULL;
 int evwinwidth, evwinheight, ktu, ktv, uorder, vorder;
 int i = 0, j = 0, winu, winv, k, a, b;
 ay_object *o = NULL, **nexto = NULL;
 char fname[] = "ay_pmt_tonpatch";
 int ay_status = AY_OK;

  if(pamesh->type == AY_PTBILINEAR)
    {
      ktu = AY_KTNURB;
      uorder = 2;
      evwinwidth = pamesh->width;
      ktv = AY_KTNURB;
      vorder = 2;
      evwinheight = pamesh->height;

      if((pamesh->close_u) || (pamesh->close_v))
	{
	  ay_status = ay_pmt_bilinearcltonpatch(pamesh, result);
	  return ay_status;
	}

    }
  else
    {
      uorder = 4;
      vorder = 4;
      switch(pamesh->btype_u)
	{
	case AY_BTBEZIER:
	  ktu = AY_KTBEZIER;
	  evwinwidth = 4;
	  break;

	case AY_BTBSPLINE:
	  ktu = AY_KTBSPLINE;
	  evwinwidth = pamesh->width;
      break;
	default:
	  ktu = AY_KTBSPLINE;
	  evwinwidth = 4;
	  break;
	}

      switch(pamesh->btype_v)
	{
	case AY_BTBEZIER:
	  ktv = AY_KTBEZIER;
	  evwinheight = 4;
	  break;
	case AY_BTBSPLINE:
	  ktv = AY_KTBSPLINE;
	  evwinheight = pamesh->height;
	  break;
	default:
	  ktv = AY_KTBSPLINE;
	  evwinheight = 4;
	  break;
	}
    } /* if */


  if((evwinwidth == pamesh->width) && (evwinheight == pamesh->height))
    {
      if(!(o = calloc(1, sizeof(ay_object))))
	return AY_EOMEM;
      ay_object_defaults(o);
      o->type = AY_IDNPATCH;

      if(!(cv = calloc(evwinwidth*evwinheight*4, sizeof(double))))
      {
	free(o);
	return AY_EOMEM;
      }
      memcpy(cv, pamesh->controlv, evwinwidth*evwinheight*4*sizeof(double));
      ay_status = ay_npt_create(uorder, vorder, evwinwidth, evwinheight,
				ktu, ktv, cv, NULL, NULL,
				(ay_nurbpatch_object **)&(o->refine));
      *result = o;
    }
  else
    {

      winu = (pamesh->width/(evwinwidth-1));
      winv = (pamesh->height/(evwinheight-1));

      if(winu == 0)
	{
	  ay_error(AY_ERROR, fname,
		   "Not enough control points in u direction.");
	  return AY_ERROR;
	}
      if(winv == 0)
	{
	  ay_error(AY_ERROR, fname,
		   "Not enough control points in v direction.");
	  return AY_ERROR;
	}

      for(i = 0; i < winu; i++)
	{
	  for(j = 0; j < winv; j++)
	    {
	      /* create new object */
	      o = NULL;
	      if(!(o = calloc(1, sizeof(ay_object))))
		return AY_EOMEM;
	      ay_object_defaults(o);
	      o->type = AY_IDNPATCH;

	      cv = NULL;
	      if(!(cv = calloc(evwinwidth*evwinheight*4, sizeof(double))))
		{
		  free(o);
		  return AY_EOMEM;
		}

	      /* place evaluation window and copy control points */
	      a = 0;
	      for(k = 0; k < evwinwidth; k++)
		{
		  b = ((3*i*pamesh->height)+(j*3)+(k*pamesh->height))*4;

		  /* because row/column order is identical for
		     PatchMeshes and NURBS Patches,
		     we may copy a whole line of control points
		     at once */
		  memcpy(&(cv[a]), &(pamesh->controlv[b]),
			     4*evwinheight*sizeof(double));

		  a += (evwinheight*4);
		} /* for */

	      ay_status = ay_npt_create(uorder, vorder,
					evwinwidth, evwinheight,
					ktu, ktv, cv, NULL, NULL,
					(ay_nurbpatch_object **)&(o->refine));

	      /* link new object */
	      if(nexto)
		{
		  *nexto = o;
		}
	      else
		{
		  *result = o;
		}
	      nexto = &(o->next);


	    }/* for */
	}/* for */

    } /* if */

 return AY_OK;
} /* ay_pmt_tonpatch */
