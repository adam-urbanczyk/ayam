/*
 * Ayam, a free 3D modeler for the RenderMan interface.
 *
 * Ayam is copyrighted 1998-2004 by Randolf Schultz
 * (randolf.schultz@gmail.com) and others.
 *
 * All rights reserved.
 *
 * See the file License for details.
 *
 */

#include "ayam.h"

/* selp.c - selected points related functions */

/* ay_selp_clear:
 *  clear list of selected points from object o
 */
void
ay_selp_clear(ay_object *o)
{
 ay_point *next = NULL, *p = NULL;

  if(!o)
    return;

  p = o->selp;

  while(p)
    {
      next = p->next;
      free(p);
      p = next;
    }

  o->selp = NULL;

 return;
} /* ay_selp_clear */


/* ay_selp_selall:
 *  select (tag) all points
 */
int
ay_selp_selall(ay_object *o)
{
 /*int ay_status = AY_OK;*/
 char fname[] = "selp_selall";
 double p[3] = {DBL_MIN, DBL_MIN, DBL_MIN};
 unsigned int i = 0;
 ay_point *newp = NULL;
 ay_pointedit pe = {0};

  if(!o)
    return AY_ENULL;

  ay_selp_clear(o);
  ay_pact_getpoint(0, o, p, &pe);

  for(i = 0; i < pe.num; i++)
    {
      newp = NULL;
      if(!(newp = calloc(1, sizeof(ay_point))))
	{
	  ay_error(AY_EOMEM, fname, NULL);
	  return AY_ERROR;
	}

      newp->next = o->selp;
      o->selp = newp;
      newp->point = pe.coords[i];
      if(pe.indices)
	{
	  newp->index = pe.indices[i];
	}
      newp->homogenous = pe.homogenous;
    } /* for */

  ay_pact_clearpointedit(&pe);

 return AY_OK;
} /* ay_selp_selall */


/* ay_selp_applytrafotcmd:
 *  apply transformation to editable points of selected objects
 */
int
ay_selp_applytrafotcmd(ClientData clientData, Tcl_Interp *interp,
		       int argc, char *argv[])
{
 /*int ay_status = AY_OK;*/
 ay_object *o = NULL;
 ay_list_object *sel = ay_selection;
 ay_point *bak = NULL, *p = NULL;
 int all = AY_FALSE;
 double m[16];

 if(argc < 2)
   {
     ay_error(AY_EARGS, argv[0], "sel|all");
     return TCL_OK;
   }

 if(!strcmp(argv[1],"all"))
   all = AY_TRUE;

  while(sel)
    {
      o = sel->object;

      if(all)
	{
	  bak = o->selp;
	  o->selp = NULL;
	  ay_selp_selall(o);
	}

      /* create trafo */
      ay_trafo_creatematrix(o, m);

      /* transform points */
      p = o->selp;
      while(p)
	{
	  if(p->homogenous)
	    ay_trafo_apply4(p->point, m);
	  else
	    ay_trafo_apply3(p->point, m);

	  p = p->next;
	}

      /* clear trafo */
      o->movx = 0.0;
      o->movy = 0.0;
      o->movz = 0.0;

      o->rotx = 0.0;
      o->roty = 0.0;
      o->rotz = 0.0;

      o->scalx = 1.0;
      o->scaly = 1.0;
      o->scalz = 1.0;


      o->quat[0] = 0.0;
      o->quat[1] = 0.0;
      o->quat[2] = 0.0;
      o->quat[3] = 1.0;

      /* restore old selected points */
      if(all)
	{
	  ay_selp_clear(o);
	  o->selp = bak;
	}

      sel = sel->next;
    } /* while */

 return TCL_OK;
} /* ay_selp_applytrafotcmd */


/* ay_selp_invert:
 *  invert list of selected points for object o
 */
int
ay_selp_invert(ay_object *o)
{
 int ay_status = AY_OK;
 ay_point *t = NULL, *p = NULL, **last2 = NULL, *p2 = NULL;

  if(!o)
    return AY_ENULL;

  if(!o->selp)
    {
      ay_status = ay_selp_selall(o);
      return ay_status;
    }

  /* save old selection of points to p */
  p = o->selp;

  /* select all points */
  o->selp = NULL;
  ay_status = ay_selp_selall(o);
  if(ay_status)
    return ay_status;

  /* remove all points, that are in p (old selection), from p2 (new
     selection); additionally, clear p */
  while(p)
    {
      last2 = &(o->selp);
      p2 = o->selp;
      while(p2)
	{
	  if(p->point == p2->point)
	    {
	      *last2 = p2->next;
	      free(p2);
	      /* XXXX should be unneeded, because of the break below: */
	      p2 = *last2;
	      break;
	    }
	  else
	    {
	      last2 = &(p2->next);
	      p2 = p2->next;
	    } /* if */
	} /* while */
      t = p;
      p = p->next;
      free(t);
    } /* while */

 return AY_OK;
} /* ay_selp_invert */


/* ay_selp_inverttcmd:
 *  invert selection of editable points of selected objects
 */
int
ay_selp_inverttcmd(ClientData clientData, Tcl_Interp *interp,
		   int argc, char *argv[])
{
 int ay_status = AY_OK;
 ay_object *o = NULL;
 ay_list_object *sel = ay_selection;

  while(sel)
    {
      o = sel->object;

      ay_status = ay_selp_invert(o);

      if(ay_status)
	{
	  ay_error(AY_ERROR, argv[0], NULL);
	  return TCL_OK;
	}

      sel = sel->next;
    } /* while */

 return TCL_OK;
} /* ay_selp_inverttcmd */


/* ay_selp_center:
 *  center all selected points
 *  mode - control in which dimensions centering shall occur
 *   0: all dimensions
 *   1: only x-y
 *   2: only y-z
 *   3: only x-z
 */
int
ay_selp_center(ay_object *o, int mode)
{
 int ay_status = AY_OK;
 double *p1, *p2, x, y, z;
 int i = 0, k;
 ay_point *po = NULL;

  if(!o)
    return AY_ENULL;

  po = o->selp;
  if(!po)
    return AY_OK;

  /* count selected points */
  while(po)
    {
      i++;
      po = po->next;
    }

  /* compute weighted sum of all coordinate values (points)
     to be considered, omitting consecutive equal points */
  po = o->selp;
  x = po->point[0]/i;
  y = po->point[1]/i;
  z = po->point[2]/i;
  p1 = po->point;
  po = po->next;
  for(k = 1; k < i; k++)
    {
      p2 = po->point;

      /* XXXX What about other multiple points? Should we not rather
	 build a list of unique points and iterate over that? */
      if(!AY_V3COMP(p1,p2))
	{
	  x += (p2[0]/i);
	  y += (p2[1]/i);
	  z += (p2[2]/i);
	}
      p1 = p2;
      po = po->next;
    } /* for */

  /* center points, by translating them */
  po = o->selp;
  switch(mode)
    {
    case 0:
      while(po)
	{
	  po->point[0] -= x;
	  po->point[1] -= y;
	  po->point[2] -= z;
	  po = po->next;
	}
      break;
    case 1:
      while(po)
	{
	  po->point[0] -= x;
	  po->point[1] -= y;
	  po = po->next;
	}
      break;
    case 2:
      while(po)
	{
	  po->point[1] -= y;
	  po->point[2] -= z;
	  po = po->next;
	}
      break;
    case 3:
      while(po)
	{
	  po->point[0] -= x;
	  po->point[2] -= z;
	  po = po->next;
	}
      break;
    default:
      break;
    } /* switch */

 return ay_status;
} /* ay_selp_center */


/* ay_sel_centertcmd:
 *  centerPnts command that moves all points of an object so that their
 *  center is identical to the object coordinate systems center
 */
int
ay_selp_centertcmd(ClientData clientData, Tcl_Interp *interp,
		   int argc, char *argv[])
{
 int ay_status = AY_OK;
 ay_list_object *sel = ay_selection;
 ay_point *oldpointsel = NULL;
 ay_object *o = NULL;
 int mode = 0;

  if(!sel)
    {
      ay_error(AY_ENOSEL, argv[0], NULL);
      return TCL_OK;
    }

  if(argc > 1)
    {
      Tcl_GetInt(interp, argv[1], &mode);
    }

  while(sel)
    {
      o = sel->object;
      if(o)
	{
	  /* save old point selection */
	  oldpointsel = o->selp;
	  o->selp = NULL;

	  /* center all points */
	  switch(o->type)
	    {
	    case AY_IDNCURVE:
	      ay_status = ay_nct_center(mode, (ay_nurbcurve_object*)o->refine);
	      break;
	    default:
	      ay_status = ay_selp_selall(o);
	      if(!ay_status)
		{
		  ay_status = ay_selp_center(o, mode);
		}
	      break;
	    } /* switch */

	  /* recover point selection */
	  ay_selp_clear(o);
	  o->selp = oldpointsel;

	  if(ay_status)
	    {
	      ay_error(ay_status, argv[0], "Could not center object!");
	    }
	  else
	    {
	      o->modified = AY_TRUE;
	      ay_notify_force(o);
	    }
	} /* if */
      sel = sel->next;
    } /* while */

  ay_notify_parent();

 return TCL_OK;
} /* ay_sel_centertcmd */


/* ay_selp_sel:
 *  select points from indices
 */
int
ay_selp_sel(ay_object *o, int indiceslen, int *indices)
{
 int ay_status = AY_OK;
 ay_point *po = NULL, *p = NULL;
 int i = 0, have_it = AY_FALSE;
 double obj;
 ay_pointedit pe;

  if(!o || !indices)
    return AY_ENULL;

  for(i = 0; i < indiceslen; i++)
    {
      if(!(po = calloc(1, sizeof(ay_point))))
	{
	  return AY_EOMEM;
	}
      have_it = AY_FALSE;
      p = o->selp;
      while(p)
	{
	  if(indices[i] == p->index)
	    {
	      have_it = AY_TRUE;
	      break;
	    }
	  p = p->next;
	}
      if(!have_it)
	{
	  po->index = indices[i];
	}
      po->next = o->selp;
      o->selp = po;
    }

  /* update pointers */
  ay_status = ay_pact_getpoint(3, o, &obj, &pe);

 return ay_status;
} /* ay_selp_sel */


/* ay_selp_seltcmd:
 *  select all editable points of selected objects
 */
int
ay_selp_seltcmd(ClientData clientData, Tcl_Interp *interp,
		int argc, char *argv[])
{
 int ay_status = AY_OK;
 int i = 1, indiceslen = 0, *tmp = NULL, *indices = NULL;
 int select_all = AY_FALSE, select_none = AY_FALSE;
 ay_object *o = NULL;
 ay_list_object *sel = ay_selection;

  if(!sel)
    {
      ay_error(AY_ENOSEL, argv[0], NULL);
      return TCL_OK;
    }

  if(argc == 1)
    {
      select_none = AY_TRUE;
    }

  /* parse list of indices */
  if(!select_none)
    {
      while(i < argc)
	{
	  if(!strcmp(argv[i],"-all"))
	    {
	      select_all = AY_TRUE;
	      break;
	    }
	  if(!(tmp = realloc(tmp, i*sizeof(int))))
	    {
	      ay_error(AY_EOMEM, argv[0], NULL);
	      goto cleanup;
	    }
	  indices = tmp;
	  sscanf(argv[i], "%d", &(indices[indiceslen]));
	  indiceslen++;
	  i++;
	} /* while */
    } /* if */

  if(!select_none && !select_all && (indiceslen == 0))
    {
      ay_error(AY_ERROR, argv[0], NULL);
      goto cleanup;
    }

  while(sel)
    {
      o = sel->object;

      if(select_none)
	{
	  ay_selp_clear(o);
	}
      else
	{
	  if(select_all)
	    {
	      ay_status = ay_selp_selall(o);
	    }
	  else
	    {
	      ay_status = ay_selp_sel(o, indiceslen, indices);
	    }
	}

      if(ay_status)
	{
	  ay_error(AY_ERROR, argv[0], NULL);
	  /*
	  goto cleanup;
	  */
	}

      sel = sel->next;
    } /* while */

cleanup:

  if(indices)
    free(indices);

 return TCL_OK;
} /* ay_selp_seltcmd */


/* ay_selp_calccog:
 *  calc cog from selected points
 */
void
ay_selp_calccog(ay_point *pnts, double *cog)
{
 unsigned int i, a = 0, numpoints = 0;
 ay_point *p = NULL;
 int stride = 3;

  if(!pnts || !cog)
    {
      return;
    }

  p = pnts;
  while(p)
    {
      numpoints++;
      p = p->next;
    }

  /*
  if(!(vbuf = calloc(numpoints, stride*sizeof(double))))
    {
      return;
    }
  */

  p = pnts;
  for(i = 0; i < numpoints; i++)
    {
      /* XXXX ToDo: check, whether point is alread in vbuf;
	 only if not: copy point to vbuf and update cog */

      cog[0] += p->point[0]/numpoints;
      cog[1] += p->point[1]/numpoints;
      cog[2] += p->point[2]/numpoints;

      a += stride;
      p = p->next;
    }


  /* free(vbuf); */

 return;
} /* ay_selp_calccog */
