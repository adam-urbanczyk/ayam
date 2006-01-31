/*
 * Ayam, a free 3D modeler for the RenderMan interface.
 *
 * Ayam is copyrighted 1998-2005 by Randolf Schultz
 * (rschultz@informatik.uni-rostock.de) and others.
 *
 * All rights reserved.
 *
 * See the file License for details.
 *
 */

#include "ayam.h"

/* ncurve.c - ncurve object */

static char *ay_ncurve_name = "NCurve";

int
ay_ncurve_createcb(int argc, char *argv[], ay_object *o)
{
 int ay_status = AY_OK;
 char fname[] = "crtncurve";
 int order = 4, length = 4, kt = AY_KTNURB, i = 0;
 double *cv = NULL, *kv = NULL, dx = 0.25;

  if(!o)
    return AY_ENULL;

  /* parse args */
  while(i+1 < argc)
    {
      if(!strcmp(argv[i],"-length"))
	{
	  Tcl_GetInt(ay_interp, argv[i+1], &length);

	  if(length <= 1)
	    length = 4;

	  if(length < order)
	    order = length;

	  i+=2;
	}
      else
	{
	  i++;
	}
    } /* while */

  if((ay_status = ay_nct_create(order, length, kt, cv, kv,
				(ay_nurbcurve_object**)&(o->refine))))
    {
      ay_error(ay_status, fname, NULL);
      return ay_status;
    }

  cv = ((ay_nurbcurve_object*)(o->refine))->controlv;


  for(i = 0; i < (length); i++)
    {
      cv[i*4] = (double)i*dx;
      cv[i*4+3] = 1.0;
    }

  ((ay_nurbcurve_object*)(o->refine))->createmp = AY_TRUE;

  ((ay_nurbcurve_object*)(o->refine))->is_rat = AY_FALSE;

 return AY_OK;
} /* ay_ncurve_createcb */


int
ay_ncurve_deletecb(void *c)
{
 ay_nurbcurve_object *ncurve = NULL;

  if(!c)
    return AY_ENULL;

  ncurve = (ay_nurbcurve_object *)(c);

  /* free knots */
  if(ncurve->knotv)
    free(ncurve->knotv);

  /* free controlv */
  if(ncurve->controlv)
    free(ncurve->controlv);

  /* free mpoints */
  ay_nct_clearmp(ncurve);

  /* free gluNurbsRenderer */
  if(ncurve->no)
    gluDeleteNurbsRenderer(ncurve->no);

  /* free (simple) tesselation */
  if(ncurve->tessv)
    free(ncurve->tessv);

  free(ncurve);

 return AY_OK;
} /* ay_ncurve_deletecb */


int
ay_ncurve_copycb(void *src, void **dst)
{
 int ay_status;
 ay_nurbcurve_object *ncurve = NULL, *ncurvesrc = NULL;
 int kl;

  if(!src || !dst)
    return AY_ENULL;

  ncurvesrc = (ay_nurbcurve_object *)src;

  if(!(ncurve = calloc(1, sizeof(ay_nurbcurve_object))))
    return AY_EOMEM;

  memcpy(ncurve, src, sizeof(ay_nurbcurve_object));

  ncurve->no = NULL;

  /* copy knots */
  kl = ncurve->order + ncurve->length;
  if(!(ncurve->knotv = calloc(kl, sizeof(double))))
    return AY_EOMEM;
  memcpy(ncurve->knotv, ncurvesrc->knotv, kl * sizeof(double));

  /* copy controlv */
  if(!(ncurve->controlv = calloc(4 * ncurve->length, sizeof(double))))
    return AY_EOMEM;
  memcpy(ncurve->controlv, ncurvesrc->controlv,
	 4 * ncurve->length * sizeof(double));

  /* copy tessv */
  if(ncurvesrc->tessv)
    {
      if(!(ncurve->tessv = calloc(3 * ncurve->tesslen, sizeof(double))))
	return AY_EOMEM;
      memcpy(ncurve->tessv, ncurvesrc->tessv,
	     3 * ncurve->tesslen * sizeof(double));
    }

  /* copy mpoints */
  ncurve->mpoints = NULL;
  if(ncurvesrc->mpoints)
    {
      ay_status = ay_nct_recreatemp(ncurve);
    }

  *dst = (void *)ncurve;

 return AY_OK;
} /* ay_ncurve_copycb */


int
ay_ncurve_drawstesscb(struct Togl *togl, ay_object *o)
{
 int ay_status = AY_OK;
 ay_nurbcurve_object *ncurve = NULL;
 int a = 0, i, tesslen, tstride;
 double *tessv;
 int qf = ay_prefs.stess_qf;

  if(!o)
    return AY_ENULL;

  ncurve = (ay_nurbcurve_object *)o->refine;

  if(!ncurve)
    return AY_ENULL;

  if(ncurve->order == 2)
    {
      tstride = 4;
      tessv = ncurve->controlv;
      tesslen = ncurve->length;
    }
  else
    {
      if(ncurve->glu_sampling_tolerance != 0.0)
	{
	  qf = ay_stess_GetQF(ncurve->glu_sampling_tolerance);
	}

      if(o->modified || (ncurve->tessqf != qf))
	{
	  if(ncurve->tessv)
	    {
	      free(ncurve->tessv);
	      ncurve->tessv = NULL;
	    }
	  o->modified = AY_FALSE;
	}

      tesslen = 0;
      if(!ncurve->tessv)
	{
	  ay_status = ay_stess_CurvePoints3D(ncurve->length, ncurve->order-1,
		         ncurve->knotv, ncurve->controlv, ncurve->is_rat, qf,
					   &ncurve->tesslen, &ncurve->tessv);
	}
      tstride = 3;
      tessv = ncurve->tessv;
      tesslen = ncurve->tesslen;
      ncurve->tessqf = qf;
    } /* if */

  if(tesslen)
    {
      glBegin(GL_LINE_STRIP);
      for(i = 0; i < tesslen; i++)
	{
	  glVertex3dv(&(tessv[a]));
	  a += tstride;
	}
      glEnd();
    } /* if */

 return AY_OK;
} /* ay_ncurve_drawstesscb */


int
ay_ncurve_drawglucb(struct Togl *togl, ay_object *o)
{
 ay_nurbcurve_object *ncurve = NULL;
 int order = 0, length = 0, knot_count = 0, i = 0, a = 0, b = 0;
 GLdouble sampling_tolerance = ay_prefs.glu_sampling_tolerance;
 static GLfloat *knots = NULL, *controls = NULL;


  if(!o)
    return AY_ENULL;

  ncurve = (ay_nurbcurve_object *)o->refine;

  if(!ncurve)
    return AY_ENULL;

  if(controls)
    {
      free(controls);
      controls = NULL;
    }

  if(knots)
    {
      free(knots);
      knots = NULL;
    }

  order = ncurve->order;
  length = ncurve->length;

  if(ncurve->glu_sampling_tolerance > 0.0)
    sampling_tolerance = ncurve->glu_sampling_tolerance;

  knot_count = length + order;

  if((knots = calloc(knot_count, sizeof(GLfloat))) == NULL)
    return AY_EOMEM;
  if((controls = calloc(length*(ncurve->is_rat?4:3),
			sizeof(GLfloat))) == NULL)
    { free(knots); knots = NULL; return AY_EOMEM; }

  a = 0;
  for(i = 0; i < knot_count; i++)
    {
      knots[a] = (GLfloat)ncurve->knotv[a];
      a++;
    }
  a = 0;
  for(i = 0; i < length; i++)
    {
      controls[a] = (GLfloat)ncurve->controlv[b];
      a++; b++;
      controls[a] = (GLfloat)ncurve->controlv[b];
      a++; b++;
      controls[a] = (GLfloat)ncurve->controlv[b];
      a++; b++;
      if(ncurve->is_rat)
	{
	  controls[a] = (GLfloat)ncurve->controlv[b];
	  a++;
	}
      b++;
    } /* for */
#ifndef AYWITHAQUA
  if(!ncurve->no)
  {
#endif /* !AYWITHAQUA */
      ncurve->no = gluNewNurbsRenderer();
      if(ncurve->no == NULL)
	{
	  free(knots); knots = NULL;
	  free(controls); controls = NULL;
	  return AY_EOMEM;
	}
#ifndef AYWITHAQUA
      } /* if */
#endif /* !AYWITHAQUA */

#if defined(WIN32) && !defined(AYUSESUPERGLU)
  gluNurbsCallback(ncurve->no, GLU_ERROR, (GLUnurbsErrorProc)ay_error_glucb);
#else
  gluNurbsCallback(ncurve->no, GLU_ERROR, ay_error_glucb);
#endif

  gluBeginCurve(ncurve->no);

#ifdef AYIRIXBUG
   gluNurbsProperty(ncurve->no, GLU_NURBS_MODE_EXT, GLU_NURBS_RENDERER_EXT);
#endif /* AYIRIXBUG */

   gluNurbsProperty(ncurve->no, GLU_SAMPLING_TOLERANCE,
		    (GLfloat)sampling_tolerance);

   gluNurbsProperty(ncurve->no, GLU_CULLING, GL_TRUE);

   gluNurbsCurve(ncurve->no, (GLint)knot_count, knots,
		 (GLint)(ncurve->is_rat?4:3), controls,
		 (GLint)ncurve->order,
		 (ncurve->is_rat?GL_MAP1_VERTEX_4:GL_MAP1_VERTEX_3));

  gluEndCurve(ncurve->no);

#ifdef AYWITHAQUA
  gluDeleteNurbsRenderer(ncurve->no);
  ncurve->no = NULL;
#endif /* AYWITHAQUA */

 return AY_OK;
} /* ay_ncurve_drawglucb */


int
ay_ncurve_drawchcb(struct Togl *togl, ay_object *o)
{
 int a, i;
 ay_nurbcurve_object *ncurve = NULL;

  if(!o)
    return AY_ENULL;

  ncurve = (ay_nurbcurve_object *)o->refine;

  if(!ncurve)
    return AY_ENULL;

  a = 0;
  glBegin(GL_LINE_STRIP);
  for(i = 0; i < ncurve->length; i++)
    {
      glVertex3dv((GLdouble *)&(ncurve->controlv[a]));
      a += 4;
    }
  glEnd();

 return AY_OK;
} /* ay_ncurve_drawchcb */


int
ay_ncurve_drawcb(struct Togl *togl, ay_object *o)
{
 int display_mode = ay_prefs.nc_display_mode;
 ay_nurbcurve_object *ncurve = NULL;

  if(!o)
    return AY_ENULL;

  ncurve = (ay_nurbcurve_object *)o->refine;

  if(!ncurve)
    return AY_ENULL;

  if(ncurve->display_mode != 0)
    {
      display_mode = ncurve->display_mode-1;
    }

  switch(display_mode)
    {
    case 0: /* ControlHull */
      ay_ncurve_drawchcb(togl, o);
      break;
    case 1: /* CurveAndHull (GLU) */
      ay_ncurve_drawchcb(togl, o);
      ay_ncurve_drawglucb(togl, o);
      break;
    case 2: /* Curve (GLU) */
      ay_ncurve_drawglucb(togl, o);
      break;
    case 3: /* CurveAndHull (STESS) */
      ay_ncurve_drawchcb(togl, o);
      ay_ncurve_drawstesscb(togl, o);
      break;
    case 4: /* Curve (STESS) */
      ay_ncurve_drawstesscb(togl, o);
      break;
    } /* switch */

 return AY_OK;
} /* ay_ncurve_drawcb */


int
ay_ncurve_shadecb(struct Togl *togl, ay_object *o)
{

 return AY_OK;
} /* ay_ncurve_shadecb */


int
ay_ncurve_drawhcb(struct Togl *togl, ay_object *o)
{
 int length = 0, i = 0, a = 0;
 ay_nurbcurve_object *curve = (ay_nurbcurve_object *) o->refine;
 ay_mpoint_object *mp = NULL;
 GLdouble *ver = NULL;
 double point_size = ay_prefs.handle_size;

  length = curve->length;

  ver = curve->controlv;

  glPointSize((GLfloat)point_size);

  glBegin(GL_POINTS);
  for(i=0; i<length; i++)
    {
      glVertex3dv((GLdouble *)&ver[a]);
      a += 4;
    }
  glEnd();

  /* draw mpoints */
  if(curve->mpoints)
    {

      glPointSize((GLfloat)(point_size*1.25));
      glBegin(GL_POINTS);
      mp = curve->mpoints;
      while(mp)
	{
	  glVertex3dv((GLdouble *)(mp->points[0]));
	  mp = mp->next;
	}
      glEnd();
      glPointSize((GLfloat)point_size);
    }

  /* draw arrow */
  ay_draw_arrow(togl, &(ver[curve->length*4-8]), &(ver[curve->length*4-4]));

 return AY_OK;
} /* ay_ncurve_drawhcb */


int
ay_ncurve_getpntcb(int mode, ay_object *o, double *p)
{
 ay_nurbcurve_object *ncurve = NULL;
 ay_mpoint_object *mp = NULL;
 double min_dist = ay_prefs.pick_epsilon, dist = 0.0;
 double **pecoords = NULL, *pecoord = NULL, *control = NULL, *c;
 int i = 0, j = 0, a = 0, found = AY_FALSE;

  if(!o || !p)
    return AY_ENULL;

  ncurve = (ay_nurbcurve_object *)(o->refine);

  if(min_dist == 0.0)
    min_dist = DBL_MAX;

  if(ay_point_edit_coords)
    free(ay_point_edit_coords);

  ay_point_edit_coords = NULL;

  /* select all points? */
  if(mode == 0)
    { /* yes */

      if(!(ay_point_edit_coords = calloc(ncurve->length, sizeof(double*))))
	return AY_EOMEM;

      for(i = 0; i < ncurve->length; i++)
	{
	  ay_point_edit_coords[i] = &(ncurve->controlv[a]);
	  a += 4;
	}

      ay_point_edit_coords_homogenous = AY_TRUE;
      ay_point_edit_coords_number = ncurve->length;
    }
  else
    { /* no */

      /* selection based on a single point? */
      if(mode == 1)
	{ /* yes */

	  control = ncurve->controlv;
	  for(i = 0; i < ncurve->length; i++)
	    {
	      dist = AY_VLEN((p[0] - control[j]),
			     (p[1] - control[j+1]),
			     (p[2] - control[j+2]));

	      if(dist < min_dist)
		{
		  pecoord = &(control[j]);
		  min_dist = dist;
		}

	      j += 4;
	    } /* for */

	  if(!pecoord)
	    return AY_OK; /* XXXX should this return a 'AY_EPICK' ? */

	  ay_point_edit_coords_homogenous = AY_TRUE;

	  if(ncurve->mpoints)
	    {
	      mp = ncurve->mpoints;
	      while(mp && !found)
		{
		  found = AY_FALSE;
		  for(i = 0; i < mp->multiplicity; i++)
		    {
		      if(mp->points[i] == pecoord)
			{
			  found = AY_TRUE;
			  ay_point_edit_coords_number = mp->multiplicity;
			  if(!(ay_point_edit_coords = calloc(mp->multiplicity,
							     sizeof(double*))))
			    return AY_EOMEM;
			  memcpy(ay_point_edit_coords, mp->points,
				 mp->multiplicity * sizeof(double *));
			} /* if */
		    } /* for */

		  mp = mp->next;
		} /* while */
	    } /* if */

	  if(!found)
	    {

	      if(!(ay_point_edit_coords = calloc(1, sizeof(double *))))
		return AY_EOMEM;

	      ay_point_edit_coords[0] = pecoord;
	      ay_point_edit_coords_number = 1;
	    } /* if */

	}
      else
	{ /* no */

	  /* selection based on planes */
	  control = ncurve->controlv;
	  j = 0;
	  a = 0;
	  for(i = 0; i < ncurve->length; i++)
	    {
	      c = &(control[j]);

	      /* test point c against the four planes in p */
	      if(((p[0]*c[0] + p[1]*c[1] + p[2]*c[2] + p[3]) < 0.0) &&
		 ((p[4]*c[0] + p[5]*c[1] + p[6]*c[2] + p[7]) < 0.0) &&
		 ((p[8]*c[0] + p[9]*c[1] + p[10]*c[2] + p[11]) < 0.0) &&
		 ((p[12]*c[0] + p[13]*c[1] + p[14]*c[2] + p[15]) < 0.0))
		{

		  if(!(pecoords = realloc(pecoords, (a+1)*sizeof(double *))))
		    return AY_EOMEM;
		  pecoords[a] = &(control[j]);
		  a++;
		} /* if */

	      j += 4;
	    } /* for */

	  if(!pecoords)
	    return AY_OK; /* XXXX should this return a 'AY_EPICK' ? */

	  ay_point_edit_coords_homogenous = AY_TRUE;
	  ay_point_edit_coords = pecoords;
	  ay_point_edit_coords_number = a;

	} /* if */

    } /* if */

 return AY_OK;
} /* ay_ncurve_getpntcb */


/* Tcl -> C! */
int
ay_ncurve_setpropcb(Tcl_Interp *interp, int argc, char *argv[], ay_object *o)
{
 int ay_status = AY_OK;
 char *n1 = "NCurveAttrData";
 char fname[] = "ncurve_setpropcb";
 Tcl_Obj *to = NULL, *toa = NULL, *ton = NULL;
 ay_nurbcurve_object *ncurve = NULL;
 ay_mpoint_object *mp = NULL;
 int new_order, new_length, new_knot_type, new_type, knots_modified = 0;
 double *nknotv = NULL;
 int updateKnots = AY_FALSE, updateMPs = AY_TRUE;
 int knotc, i;
 char **knotv;

  if(!o)
    return AY_ENULL;

  ncurve = (ay_nurbcurve_object *)o->refine;

  toa = Tcl_NewStringObj(n1,-1);
  ton = Tcl_NewStringObj(n1,-1);

  /* get new values from Tcl */
  Tcl_SetStringObj(ton, "Length", -1);
  to = Tcl_ObjGetVar2(interp, toa, ton, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp, to, &new_length);

  Tcl_SetStringObj(ton, "Order", -1);
  to = Tcl_ObjGetVar2(interp, toa, ton, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp, to, &new_order);

  Tcl_SetStringObj(ton, "Knot-Type", -1);
  to = Tcl_ObjGetVar2(interp, toa, ton, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp, to, &new_knot_type);

  Tcl_SetStringObj(ton, "Type", -1);
  to = Tcl_ObjGetVar2(interp, toa, ton, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp, to, &new_type);

  Tcl_SetStringObj(ton, "CreateMP", -1);
  to = Tcl_ObjGetVar2(interp, toa, ton, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp, to, &(ncurve->createmp));

  Tcl_SetStringObj(ton, "Tolerance", -1);
  to = Tcl_ObjGetVar2(interp, toa, ton, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetDoubleFromObj(interp, to, &(ncurve->glu_sampling_tolerance));

  Tcl_SetStringObj(ton, "DisplayMode", -1);
  to = Tcl_ObjGetVar2(interp, toa, ton, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp, to, &(ncurve->display_mode));

  Tcl_SetStringObj(ton, "Knots-Modified", -1);
  to = Tcl_ObjGetVar2(interp, toa, ton, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp, to, &knots_modified);

  Tcl_IncrRefCount(toa);Tcl_DecrRefCount(toa);
  Tcl_IncrRefCount(ton);Tcl_DecrRefCount(ton);


  /* apply changed values to curve */

  /* resize curve */
  if(new_length != ncurve->length && (new_length > 1))
    {
      if(o->selp)
	{
	  ay_selp_clear(o);
	}

      ay_status = ay_nct_resize(ncurve, new_length);

      if(ay_status)
       ay_error(AY_ERROR, fname, "Could not resize curve!");

      updateKnots = AY_TRUE;
      o->modified = AY_TRUE;
    }

  /* apply new order */
  if((ncurve->order != new_order) && (new_order > 1))
    {
      ncurve->order = new_order;
      updateKnots = AY_TRUE;
      o->modified = AY_TRUE;
    }

  /* change knot type */
  if((new_knot_type != ncurve->knot_type) || (updateKnots))
    {
      ncurve->knot_type = new_knot_type;
      updateKnots = AY_TRUE;
      o->modified = AY_TRUE;
    }

  /* plausibility checks */
  if(ncurve->knot_type == AY_KTBEZIER)
    {
      if(ncurve->order != ncurve->length)
	{
	  ay_error(AY_EWARN, fname, "Changing order to match length!");
	  ncurve->order = ncurve->length;
	}
    }

  if(ncurve->length < ncurve->order)
    {
      ay_error(AY_EWARN, fname, "Lowering order to match length!");
      ncurve->order = ncurve->length;
    }

  /* create new knot vectors */
  if((updateKnots) && (ncurve->knot_type != AY_KTCUSTOM))
    ay_status = ay_knots_createnc(ncurve);

  /* decompose knot-list (create custom knot sequence) */
  if((ncurve->knot_type == AY_KTCUSTOM) && knots_modified)
    {
      ay_error(AY_EOUTPUT, fname, "Checking new knots...");
      Tcl_SplitList(interp,Tcl_GetVar2(interp, n1, "Knots",
				       TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY),
		    &knotc, &knotv);

      if(!(nknotv = calloc(knotc, sizeof(double))))
	{
	  ay_error(AY_EOMEM, fname, NULL);
	  Tcl_Free((char *) knotv);
	  return AY_ERROR;
	}

      for(i = 0; i < knotc; i++)
	{
	  Tcl_GetDouble(interp,knotv[i],&nknotv[i]);
	} /* for */

      if(!(ay_status = ay_knots_check(new_length,new_order,knotc,nknotv)))
	{/* the knots are ok */
	  free(ncurve->knotv);
	  ncurve->knotv = nknotv;
	  /*
	  if(ncurve->knotv[0] != 0.0 || ncurve->knotv[
	      ncurve->length+ncurve->order-1] != 1.0)
	    {
	      ay_status = ay_knots_rescale(ncurve->length+ncurve->order,
	      ncurve->knotv);
	    }
	  */
	}
      else
	{/* the knots are wrong */
	  /* first, tell the user what went wrong */
	  switch (ay_status)
	    {
	    case 1:
	      ay_error(AY_ERROR,fname, "Knot sequence has too few knots!");
	      break;
	    case 2:
	      ay_error(AY_ERROR,fname, "Knot sequence has too much knots!");
	      break;
	    case 3:
	      ay_error(AY_ERROR,fname, "Knot multiplicity higher than order!");
	      break;
	    case 4:
	      ay_error(AY_ERROR,fname, "Knot sequence has decreasing knots!");
	      break;
	    } /* switch */

	  free(nknotv);

	  /* create new knots */
	  ay_error(AY_EWARN, fname,
			 "Falling back to knot type NURB!");
	  ncurve->knot_type = AY_KTNURB;

	  ay_status = ay_knots_createnc(ncurve);

	  if(ay_status)
	    ay_error(AY_ERROR, fname, "Error creating new knots!");
	} /* if */

      /* XXXX compare old and new knots before setting this flag */
      o->modified = AY_TRUE;

      Tcl_Free((char *) knotv);
    } /* if */

  /* close curve? */
  if(((new_type != AY_CTOPEN) && (new_type != ncurve->type)) ||
     (o->modified))
    {
      /* close it */
      if(o->selp)
	{
	  ay_selp_clear(o);
	}
      o->modified = AY_TRUE;
      ncurve->type = new_type;
      ay_status = ay_nct_close(ncurve);
      if(ay_status)
	{
	  ay_error(AY_ERROR, fname, "Could not close curve!");
	}
      else
	{
	  if(ncurve->type == AY_CTPERIODIC)
	    {
	      if((ncurve->knot_type == AY_KTNURB) ||
		 (ncurve->knot_type == AY_KTBEZIER))
		{
		  ay_error(AY_EWARN, fname, "Changing knot type to B-Spline!");
		  ncurve->knot_type = AY_KTBSPLINE;
		  ay_status = ay_knots_createnc(ncurve);

		  if(ay_status)
		    ay_error(ay_status, fname, "Error creating new knots!");
		} /* if */
	    }
	  else
	    {
	      if(ncurve->knot_type == AY_KTBSPLINE)
		{
		  ay_error(AY_EWARN, fname, "Changing knot type to NURB!");
		  ncurve->knot_type = AY_KTNURB;
		  ay_status = ay_knots_createnc(ncurve);

		  if(ay_status)
		    ay_error(ay_status, fname, "Error creating new knots!");
		} /* if */
	    } /* if */
	} /* if */
    } /* if */

  /* break curve? */
  if((new_type == AY_CTOPEN) && (ncurve->type != AY_CTOPEN))
    {
      ncurve->type = AY_CTOPEN;
      /* we assume user wants the end cvs to be exploded
	 we clear all mpoints, user may re-create them easily
	 pressing apply another time */
      while(ncurve->mpoints)
	{
	  mp = ncurve->mpoints->next;
	  if(ncurve->mpoints->points)
	    free(ncurve->mpoints->points);
	  free(ncurve->mpoints);
	  ncurve->mpoints = mp;
	} /* while */
      updateMPs = AY_FALSE;
    } /* if */

  /*  if(ncurve->mpoints)*/
  if(updateMPs)
    {
      ay_status = ay_nct_recreatemp(ncurve);
      if(ay_status)
	ay_error(AY_ERROR, fname, "Error re-creating MPoints!");
    }

  ay_status = ay_notify_parent();

 return AY_OK;
} /* ay_ncurve_setpropcb */


/* C -> Tcl! */
int
ay_ncurve_getpropcb(Tcl_Interp *interp, int argc, char *argv[], ay_object *o)
{
 char *n1="NCurveAttrData";
 Tcl_Obj *to = NULL, *toa = NULL, *ton = NULL;
 ay_nurbcurve_object *ncurve = NULL;
 int i;

  if(!o)
    return AY_ENULL;

  ncurve = (ay_nurbcurve_object *)(o->refine);

  toa = Tcl_NewStringObj(n1,-1);

  ton = Tcl_NewStringObj(n1,-1);


  Tcl_SetStringObj(ton,"Length",-1);
  to = Tcl_NewIntObj(ncurve->length);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton,"Order",-1);
  to = Tcl_NewIntObj(ncurve->order);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton,"Type",-1);
  to = Tcl_NewIntObj(ncurve->type);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton,"CreateMP",-1);
  to = Tcl_NewIntObj(ncurve->createmp);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton,"Knot-Type",-1);
  to = Tcl_NewIntObj(ncurve->knot_type);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);

  Tcl_SetVar2(interp,n1,"Knots","", TCL_LEAVE_ERR_MSG |
	      TCL_GLOBAL_ONLY);
  Tcl_SetStringObj(ton,"Knots",-1);
  for(i=0; i<ncurve->length+ncurve->order; i++)
    {

      to = Tcl_NewDoubleObj((ncurve->knotv)[i]);

      Tcl_ObjSetVar2(interp,toa,ton,to,TCL_APPEND_VALUE |
		     TCL_LIST_ELEMENT | TCL_LEAVE_ERR_MSG |
		     TCL_GLOBAL_ONLY);
    }

  Tcl_SetStringObj(ton,"Tolerance",-1);
  to = Tcl_NewDoubleObj(ncurve->glu_sampling_tolerance);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton,"DisplayMode",-1);
  to = Tcl_NewIntObj(ncurve->display_mode);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton,"IsRat", -1);
  if(ncurve->is_rat)
    to = Tcl_NewStringObj("yes", -1);
  else
    to = Tcl_NewStringObj("no", -1);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton,"Knots-Modified",-1);
  to = Tcl_NewIntObj(0);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);

  Tcl_IncrRefCount(toa);Tcl_DecrRefCount(toa);
  Tcl_IncrRefCount(ton);Tcl_DecrRefCount(ton);

 return AY_OK;
} /* ay_ncurve_getpropcb */


int
ay_ncurve_readcb(FILE *fileptr, ay_object *o)
{
 int ay_status = AY_OK;
 ay_nurbcurve_object *ncurve = NULL;
 int i, a;

 if(!o)
   return AY_ENULL;

  if(!(ncurve = calloc(1, sizeof(ay_nurbcurve_object))))
    { return AY_EOMEM; }


  fscanf(fileptr,"%d\n",&ncurve->length);
  fscanf(fileptr,"%d\n",&ncurve->order);
  fscanf(fileptr,"%d\n",&ncurve->knot_type);

  if(ncurve->knot_type == AY_KTCUSTOM)
    {
      if(!(ncurve->knotv =
	   calloc((ncurve->length + ncurve->order), sizeof(double))))
	{ free(ncurve); return AY_EOMEM; }

      for(i=0; i<(ncurve->length + ncurve->order); i++)
	fscanf(fileptr,"%lg\n",&(ncurve->knotv[i]));
    }
  else
    {
      ay_status = ay_knots_createnc(ncurve);
      if(ay_status)
	{ free(ncurve); return ay_status; }
    }

  if(!(ncurve->controlv = calloc(ncurve->length*4, sizeof(double))))
    { free(ncurve->knotv); free(ncurve); return AY_EOMEM;}

  a = 0;
  for(i=0; i < ncurve->length; i++)
    {
      fscanf(fileptr,"%lg %lg %lg %lg\n",&(ncurve->controlv[a]),
	     &(ncurve->controlv[a+1]),
	     &(ncurve->controlv[a+2]),
	     &(ncurve->controlv[a+3]));
      a+=4;
    }

  fscanf(fileptr,"%d\n",&(ncurve->type));
  fscanf(fileptr,"%lg\n",&(ncurve->glu_sampling_tolerance));
  fscanf(fileptr,"%d\n",&(ncurve->display_mode));

  if(ay_read_version >= 1)
    {
      fscanf(fileptr,"%d\n",&(ncurve->createmp));
    }
  else
    {
      ncurve->createmp = AY_TRUE;
    }

  ay_nct_recreatemp(ncurve);

  if(ay_read_version < 8)
    if(ncurve->type == AY_CTCLOSED)
      ncurve->type = AY_CTPERIODIC;

  ncurve->is_rat = ay_nct_israt(ncurve);

  o->refine = ncurve;

 return AY_OK;
} /* ay_ncurve_readcb */


int
ay_ncurve_writecb(FILE *fileptr, ay_object *o)
{
 ay_nurbcurve_object *ncurve = NULL;
 int i, a;

  if(!o)
    return AY_ENULL;

  ncurve = (ay_nurbcurve_object *)(o->refine);

  fprintf(fileptr, "%d\n", ncurve->length);
  fprintf(fileptr, "%d\n", ncurve->order);
  fprintf(fileptr, "%d\n", ncurve->knot_type);

  if(ncurve->knot_type == AY_KTCUSTOM)
    {
      for(i=0;i<(ncurve->length+ncurve->order);i++)
	fprintf(fileptr,"%g\n",ncurve->knotv[i]);
    }

  a = 0;
  for(i=0;i<ncurve->length;i++)
    {
      fprintf(fileptr,"%g %g %g %g\n", ncurve->controlv[a],
	      ncurve->controlv[a+1],
	      ncurve->controlv[a+2],
	      ncurve->controlv[a+3]);
      a+=4;
    }

  fprintf(fileptr, "%d\n", ncurve->type);
  fprintf(fileptr, "%g\n", ncurve->glu_sampling_tolerance);
  fprintf(fileptr, "%d\n", ncurve->display_mode);
  fprintf(fileptr, "%d\n", ncurve->createmp);

 return AY_OK;
} /* ay_ncurve_writecb */


int
ay_ncurve_wribcb(char *file, ay_object *o)
{
 ay_nurbcurve_object *ncurve = NULL;

  if(!o)
   return AY_ENULL;

  ncurve = (ay_nurbcurve_object*)o->refine;


 return AY_OK;
} /* ay_ncurve_wribcb */


int
ay_ncurve_bbccb(ay_object *o, double *bbox, int *flags)
{
 double xmin, xmax, ymin, ymax, zmin, zmax;
 double *controlv = NULL;
 int i, a;
 ay_nurbcurve_object *ncurve = NULL;

  if(!o || !bbox)
    return AY_ENULL;

  ncurve = (ay_nurbcurve_object *)o->refine;

  controlv = ncurve->controlv;

  xmin = controlv[0];
  xmax = xmin;
  ymin = controlv[1];
  ymax = ymin;
  zmin = controlv[2];
  zmax = zmin;

  a = 0;
  for(i = 0; i < ncurve->length; i++)
    {
      if(controlv[a] < xmin)
	xmin = controlv[a];
      if(controlv[a] > xmax)
	xmax = controlv[a];

      if(controlv[a+1] < ymin)
	ymin = controlv[a+1];
      if(controlv[a+1] > ymax)
	ymax = controlv[a+1];

      if(controlv[a+2] < zmin)
	zmin = controlv[a+2];
      if(controlv[a+2] > zmax)
	zmax = controlv[a+2];

      a += 4;
    }

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
} /* ay_ncurve_bbccb */


int
ay_ncurve_convertcb(ay_object *o, int in_place)
{
 int ay_status = AY_OK;
 int i, a, b;
 ay_nurbcurve_object *nc = NULL;
 ay_icurve_object *ic = NULL;
 ay_object *new = NULL;

  if(!o)
    { return AY_ENULL; }

  nc = (ay_nurbcurve_object *) o->refine;

  if(!(new = calloc(1, sizeof(ay_object))))
    { return AY_EOMEM; }

  ay_object_defaults(new);
  new->type = AY_IDICURVE;
  ay_trafo_copy(o, new);

  if(!(ic = calloc(1, sizeof(ay_icurve_object))))
    { free(new); return AY_EOMEM; }

  new->refine = ic;

  ic->length = nc->length;
  ic->closed = nc->type>0?AY_TRUE:AY_FALSE;
  ic->glu_sampling_tolerance = nc->glu_sampling_tolerance;
  ic->display_mode = nc->display_mode;
  ic->iparam = 1.0/8.0;
  ic->iorder = nc->order; /* XXXX ? */

  if(!(ic->controlv = calloc(1, ic->length*3*sizeof(double))))
    { free(new); free(ic); return AY_EOMEM; }

  a = 0; b = 0;
  for(i = 0; i < ic->length; i++)
    {
      memcpy(&(ic->controlv[a]), &(nc->controlv[b]), 3*sizeof(double));
      a += 3;
      b += 4;
    }

  ay_notify_force(new);

  if(!in_place)
    {
      ay_status = ay_object_link(new);
    }
  else
    {
      ay_status = ay_object_replace(new, o);
    }

 return ay_status;
} /* ay_ncurve_convertcb */



int
ay_ncurve_init(Tcl_Interp *interp)
{
 int ay_status = AY_OK;

  ay_status = ay_otype_registercore(ay_ncurve_name,
				    ay_ncurve_createcb,
				    ay_ncurve_deletecb,
				    ay_ncurve_copycb,
				    ay_ncurve_drawcb,
				    ay_ncurve_drawhcb,
				    NULL, /* no shading */
				    ay_ncurve_setpropcb,
				    ay_ncurve_getpropcb,
				    ay_ncurve_getpntcb,
				    ay_ncurve_readcb,
				    ay_ncurve_writecb,
				    NULL, /* no RIB export */
				    ay_ncurve_bbccb,
				    AY_IDNCURVE);

  ay_status = ay_convert_register(ay_ncurve_convertcb, AY_IDNCURVE);

  ay_matt_nomaterial(AY_IDNCURVE);

 return ay_status;
} /* ay_ncurve_init */

