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

/* npatch.c - npatch object */

static char *ay_npatch_name = "NPatch";

int
ay_npatch_createcb(int argc, char *argv[], ay_object *o)
{
 int ay_status = AY_OK;
 char fname[] = "crtnpatch";
 int uorder = 4, vorder = 4, width = 4, height = 4;
 int ukt = AY_KTNURB, vkt = AY_KTNURB, i = 0, j = 0, k = 0;
 double *cv = NULL, *ukv = NULL, *vkv = NULL, dx = 0.25;

  if(!o)
    return AY_ENULL;

  /* parse args */
  while(i+1 < argc)
    {
      if(!strcmp(argv[i],"-width"))
	{
	  Tcl_GetInt(ay_interp, argv[i+1], &width);
	  if(width <= 0) width = 4;
	  if(width < 4) uorder = width;
	  i+=2;
	}
      else
      if(!strcmp(argv[i],"-height"))
	{
	  Tcl_GetInt(ay_interp, argv[i+1], &height);
	  if(height <= 0) height = 4;
	  if(height < 4) vorder = height;
	  i+=2;
	}
      else
	i++;
    }

  if((ay_status = ay_npt_create(uorder, vorder, width, height, ukt, vkt,
				cv, ukv, vkv,
				(ay_nurbpatch_object**)&(o->refine))))
    {
      ay_error(ay_status, fname, NULL);
      return ay_status;
    }

  cv = ((ay_nurbpatch_object*)(o->refine))->controlv;

  k = 0;
  for(i=0;i<(width);i++)
    {
      for(j=0;j<(height);j++)
	{
	  cv[k]   = (double)i*dx;
	  cv[k+1] = (double)j*dx;
	  cv[k+3] = 1.0;
	  k += 4;
	}
    }

  ((ay_nurbpatch_object*)(o->refine))->is_rat = AY_FALSE;

  o->parent = AY_TRUE;
  o->hide_children = AY_TRUE;

 return AY_OK;
} /* ay_npatch_createcb */


int
ay_npatch_deletecb(void *c)
{
 ay_nurbpatch_object *npatch = NULL;

  if(!c)
    return AY_ENULL;

  npatch = (ay_nurbpatch_object *)(c);

  /* free knots */
  if(npatch->uknotv)
    free(npatch->uknotv);
  if(npatch->vknotv)
    free(npatch->vknotv);

  /* free controlv */
  if(npatch->controlv)
    free(npatch->controlv);

  /* free gluNurbsRenderer */
  if(npatch->no)
    gluDeleteNurbsRenderer(npatch->no);

  free(npatch);

 return AY_OK;
} /* ay_npatch_deletecb */


int
ay_npatch_copycb(void *src, void **dst)
{
 ay_nurbpatch_object *npatch = NULL, *npatchsrc = NULL;
 int kl;

  if(!src || !dst)
    return AY_ENULL;

  npatchsrc = (ay_nurbpatch_object *)src;

  if(!(npatch = calloc(1, sizeof(ay_nurbpatch_object))))
    return AY_EOMEM;

  memcpy(npatch, src, sizeof(ay_nurbpatch_object));

  npatch->no = NULL;

  /* copy knots */
  kl = npatch->uorder + npatch->width;
  if(!(npatch->uknotv = calloc(kl, sizeof(double))))
    return AY_EOMEM;
  memcpy(npatch->uknotv, npatchsrc->uknotv, kl * sizeof(double));

  kl = npatch->vorder + npatch->height;
  if(!(npatch->vknotv = calloc(kl, sizeof(double))))
    return AY_EOMEM;
  memcpy(npatch->vknotv, npatchsrc->vknotv, kl * sizeof(double));

  /* copy controlv */
  if(!(npatch->controlv = calloc(4 * npatch->width * npatch->height,
				 sizeof(double))))
    return AY_EOMEM;
  memcpy(npatch->controlv, npatchsrc->controlv,
	 4 * npatch->width * npatch->height * sizeof(double));

  *dst = (void *)npatch;

 return AY_OK;
} /* ay_npatch_copycb */


int
ay_npatch_drawstesscb(struct Togl *togl, ay_object *o)
{
 int ay_status = AY_OK;
 /*char fname[] = "npatch_drawstesscb";*/
 ay_nurbpatch_object *npatch = NULL;
 int a, i, j, tessw, tessh;
 double *tessv = NULL;
 int qf = ay_prefs.stess_qf;

  if(!o)
    return AY_ENULL;

  npatch = (ay_nurbpatch_object *)o->refine;

  if(!npatch)
    return AY_ENULL;

  if(npatch->glu_sampling_tolerance != 0.0)
    {
      qf = ay_stess_GetQF(npatch->glu_sampling_tolerance);
    }

  if(o->modified || (npatch->tessqf != qf))
    {
      if(npatch->tessv)
	{
	  free(npatch->tessv);
	  npatch->tessv = NULL;
	}
      /*
      if(npatch->tesstv)
	{

	}
      */
      o->modified = AY_FALSE;
    } /* if */

  if(!npatch->tessv)
    {
      ay_status = ay_stess_TessNP(o);
    }

  tessv = npatch->tessv;
  tessw = npatch->tessw;
  tessh = npatch->tessh;

  if(tessv)
    {
      a = 0;
      for(i = 0; i < tessw; i++)
	{
	  glBegin(GL_LINE_STRIP);
	  for(j = 0;  j < tessh; j++)
	    {
	      glVertex3dv(&(tessv[a]));
	      a += 3;
	    } /* for */
	  glEnd();
	} /* for */

      for(j = 0;  j < tessh; j++)
	{
	  a = j * 3;
	  glBegin(GL_LINE_STRIP);
	  for(i = 0; i < tessw; i++)
	    {
	      glVertex3dv(&(tessv[a]));
	      a += (tessh*3);
	    } /* for */
	  glEnd();
	} /* for */
    } /* if */

 return AY_OK;
}  /* ay_npatch_drawstesscb */


int
ay_npatch_drawglucb(struct Togl *togl, ay_object *o)
{
 int ay_status = AY_OK;
 ay_nurbpatch_object *npatch = NULL;
 int uorder = 0, vorder = 0, width = 0, height = 0;
 int uknot_count = 0, vknot_count = 0, i = 0, a = 0, b = 0;
 GLdouble sampling_tolerance = ay_prefs.glu_sampling_tolerance;
 static GLfloat *uknots = NULL, *vknots = NULL, *controls = NULL;
 ay_object *trim = NULL, *loop = NULL, *nc = NULL;
 int display_mode = ay_prefs.np_display_mode;
 /* int cache = ay_prefs.glu_cache_float;*/

  if(!o)
    return AY_ENULL;

  npatch = (ay_nurbpatch_object *)o->refine;

  if(!npatch)
    return AY_ENULL;

  if(controls)
    {
      free(controls);
      controls = NULL;
    }
  if(uknots)
    {
      free(uknots);
      uknots = NULL;
    }
  if(vknots)
    {
      free(vknots);
      vknots = NULL;
    }

  uorder = npatch->uorder;
  vorder = npatch->vorder;
  width = npatch->width;
  height = npatch->height;

  if(npatch->glu_sampling_tolerance > 0.0)
    sampling_tolerance = npatch->glu_sampling_tolerance;

  if(npatch->glu_display_mode != 0)
    {
      display_mode = npatch->glu_display_mode-1;
    }

  uknot_count = width + uorder;
  vknot_count = height + vorder;

  if((uknots = calloc(uknot_count, sizeof(GLfloat))) == NULL)
    return AY_EOMEM;
  if((vknots = calloc(vknot_count, sizeof(GLfloat))) == NULL)
    { free(uknots); uknots = NULL; return AY_EOMEM; }
  if((controls = calloc(width*height*(npatch->is_rat?4:3),
			sizeof(GLfloat))) == NULL)
    {
      free(uknots); uknots = NULL; free(vknots); vknots = NULL;
      return AY_EOMEM;
    }

  a = 0;
  for(i = 0; i < uknot_count; i++)
    {
      uknots[a] = (GLfloat)npatch->uknotv[a];
      a++;
    }
  a = 0;
  for(i = 0; i < vknot_count; i++)
    {
      vknots[a] = (GLfloat)npatch->vknotv[a];
      a++;
    }
  a = 0;
  for(i = 0; i < width*height; i++)
    {
      controls[a] = (GLfloat)npatch->controlv[b];
      a++; b++;
      controls[a] = (GLfloat)npatch->controlv[b];
      a++; b++;
      controls[a] = (GLfloat)npatch->controlv[b];
      a++; b++;
      if(npatch->is_rat)
	{
	  controls[a] = (GLfloat)npatch->controlv[b];
	  a++;
	}
      b++;
    }

  if(!npatch->no)
    {
      npatch->no = gluNewNurbsRenderer();
      if(npatch->no == NULL)
	{
	  free(uknots); uknots = NULL;
	  free(vknots); vknots = NULL;
	  free(controls); controls = NULL;
	  return AY_EOMEM;
	}
    }

#if defined(WIN32) && !defined(AYUSESUPERGLU)
  gluNurbsCallback(npatch->no, GLU_ERROR, (GLUnurbsErrorProc)ay_error_glucb);
#else
  gluNurbsCallback(npatch->no, GLU_ERROR, ay_error_glucb);
#endif

  gluBeginSurface(npatch->no);

  gluNurbsProperty(npatch->no, GLU_SAMPLING_TOLERANCE,
		   (GLfloat)sampling_tolerance);


  if(display_mode == 2)
    gluNurbsProperty(npatch->no, GLU_DISPLAY_MODE, GLU_OUTLINE_PATCH);
  else
    gluNurbsProperty(npatch->no, GLU_DISPLAY_MODE, GLU_OUTLINE_POLYGON);


  gluNurbsProperty(npatch->no, GLU_CULLING, GL_TRUE);

  gluNurbsSurface(npatch->no, (GLint)uknot_count, uknots,
		  (GLint)vknot_count, vknots,
		  (GLint)height*(npatch->is_rat?4:3),
		  (GLint)(npatch->is_rat?4:3),
		  controls,
		  (GLint)npatch->uorder, (GLint)npatch->vorder,
		  (npatch->is_rat?GL_MAP2_VERTEX_4:GL_MAP2_VERTEX_3));

  /* propagate changes to trimcurve walking code also to:
     shadecb(), nurbs/npt.c/wribtrimcurves(), and
     nurbs/npt.c/ay_npt_topolymesh()! */

  /* draw trimcurves */
  if(o->down && o->down->next)
    {
      trim = o->down;
      ay_errno = AY_OK;
      while(trim->next)
	{
	  switch(trim->type)
	    {
	    case AY_IDNCURVE:
	      gluBeginTrim(npatch->no);
	       ay_status = ay_npt_drawtrimcurve(togl, trim, npatch->no);
	      gluEndTrim(npatch->no);
	      break;
	    case AY_IDLEVEL:
	      /* XXXX check, whether level is of type trimloop? */
	      loop = trim->down;
	      if(loop && loop->next)
		{
		  gluBeginTrim(npatch->no);
		  while(loop->next)
		    {

		      if(loop->type == AY_IDNCURVE)
			{
			  ay_status = ay_npt_drawtrimcurve(togl, loop,
							   npatch->no);
			}
		      else
			{
			  nc = NULL;
			  ay_status = ay_provide_object(loop, AY_IDNCURVE,
							&nc);
			  if(nc)
			    {
			      ay_status = ay_npt_drawtrimcurve(togl, nc,
							       npatch->no);
			      ay_object_delete(nc);
			    } /* if */
			} /* if */
		      loop = loop->next;
		    } /* while */
		  gluEndTrim(npatch->no);
		} /* if */
	      break;
	    default:
	      nc = NULL;
	      ay_status = ay_provide_object(trim, AY_IDNCURVE, &nc);
	      if(nc)
		{
		  gluBeginTrim(npatch->no);
		   ay_status = ay_npt_drawtrimcurve(togl, nc, npatch->no);
		  gluEndTrim(npatch->no);
		  ay_object_delete(nc);
		}
	      break;
	    } /* switch */
	  trim = trim->next;
	} /* while */
    } /* if */

  gluEndSurface(npatch->no);

 return AY_OK;
} /* ay_npatch_drawglucb */


int
ay_npatch_drawchcb(struct Togl *togl, ay_object *o)
{
 ay_nurbpatch_object *npatch = NULL;
 double *ver = NULL;
 int i, j, a, width, height;

  if(!o)
    return AY_ENULL;

  npatch = (ay_nurbpatch_object *)o->refine;

  if(!npatch)
    return AY_ENULL;

  width = npatch->width;
  height = npatch->height;

  ver = npatch->controlv;
  a = 0;
  for(i = 0; i < width; i++)
    {
      glBegin(GL_LINE_STRIP);
	for(j = 0; j < height; j++)
	  {
	    glVertex3dv((GLdouble *)&ver[a]);
	    a += 4;
	  }
      glEnd();
    }

  a = 0;
  for(j = 0; j < height; j++)
    {
      a = j * 4;

      glBegin(GL_LINE_STRIP);
	for(i = 0; i < width; i++)
	  {
	    glVertex3dv((GLdouble *)&ver[a]);

	    a += (4 * height);
	  }
      glEnd();
    }

 return AY_OK;
} /* ay_npatch_drawchcb */


int
ay_npatch_drawcb(struct Togl *togl, ay_object *o)
{
 int display_mode = ay_prefs.np_display_mode;
 ay_nurbpatch_object *npatch = NULL;

  if(!o)
    return AY_ENULL;

  npatch = (ay_nurbpatch_object *)o->refine;

  if(!npatch)
    return AY_ENULL;

  if(npatch->glu_display_mode != 0)
    {
      display_mode = npatch->glu_display_mode-1;
    }

  switch(display_mode)
    {
    case 0: /* ControlHull */
      ay_npatch_drawchcb(togl, o);
      break;
    case 1: /* OutlinePolygon (GLU) */
      ay_npatch_drawglucb(togl, o);
      break;
    case 2: /* OutlinePatch (GLU) */
      ay_npatch_drawglucb(togl, o);
      break;
    case 3: /* OutlinePatch (STESS) */
      ay_npatch_drawstesscb(togl, o);
      break;
    } /* switch */

 return AY_OK;
} /* ay_npatch_drawcb */


int
ay_npatch_shadestesscb(struct Togl *togl, ay_object *o)
{
 /*int ay_status = AY_OK;*/
 ay_nurbpatch_object *npatch = NULL;
 int qf = ay_prefs.glu_sampling_tolerance;

  if(!o)
    return AY_ENULL;

  npatch = (ay_nurbpatch_object *)o->refine;

  if(!npatch)
    return AY_ENULL;

  if(npatch->glu_sampling_tolerance != 0.0)
    {
      qf = ay_stess_GetQF(npatch->glu_sampling_tolerance);
    }

 return AY_OK;
} /* ay_npatch_shadestesscb */


int
ay_npatch_shadecb(struct Togl *togl, ay_object *o)
{
 int ay_status = AY_OK;
 ay_nurbpatch_object *npatch = NULL;
 int uorder = 0, vorder = 0, width = 0, height = 0;
 int uknot_count = 0, vknot_count = 0, i = 0, a = 0, b = 0;
 GLdouble sampling_tolerance = ay_prefs.glu_sampling_tolerance;
 static GLfloat *uknots = NULL, *vknots = NULL, *controls = NULL;
 ay_object *trim = NULL, *loop = NULL, *nc = NULL;

  if(!o)
    return AY_ENULL;

  npatch = (ay_nurbpatch_object *)o->refine;

  if(!npatch)
    return AY_ENULL;

  if(npatch->glu_display_mode > 2)
    return(ay_npatch_shadestesscb(togl, o));

  if(controls)
    {
      free(controls);
      controls = NULL;
    }
  if(uknots)
    {
      free(uknots);
      uknots = NULL;
    }
  if(vknots)
    {
      free(vknots);
      vknots = NULL;
    }

  uorder = npatch->uorder;
  vorder = npatch->vorder;
  width = npatch->width;
  height = npatch->height;

  if(npatch->glu_sampling_tolerance > 0.0)
    sampling_tolerance = npatch->glu_sampling_tolerance;

  uknot_count = width + uorder;
  vknot_count = height + vorder;

  if((uknots = calloc(uknot_count, sizeof(GLfloat))) == NULL)
    return AY_EOMEM;
  if((vknots = calloc(vknot_count, sizeof(GLfloat))) == NULL)
    { free(uknots); uknots = NULL; return AY_EOMEM; }
  if((controls = calloc(width*height*(npatch->is_rat?4:3),
				      sizeof(GLfloat))) == NULL)
    {
      free(uknots); uknots = NULL; free(vknots); vknots = NULL;
      return AY_EOMEM;
    }

  a = 0;
  for(i = 0; i < uknot_count; i++)
    {
      uknots[a] = (GLfloat)npatch->uknotv[a];
      a++;
    }
  a = 0;
  for(i = 0; i < vknot_count; i++)
    {
      vknots[a] = (GLfloat)npatch->vknotv[a];
      a++;
    }
  a = 0;
  for(i = 0; i < width*height; i++)
    {
      controls[a] = (GLfloat)npatch->controlv[b];
      a++; b++;
      controls[a] = (GLfloat)npatch->controlv[b];
      a++; b++;
      controls[a] = (GLfloat)npatch->controlv[b];
      a++; b++;
      if(npatch->is_rat)
	{
	  controls[a] = (GLfloat)npatch->controlv[b];
	  a++;
	}
      b++;
    }

  if(!npatch->no)
    {
      npatch->no = gluNewNurbsRenderer();
      if(npatch->no == NULL)
	{
	  free(uknots); uknots = NULL;
	  free(vknots); vknots = NULL;
	  free(controls); controls = NULL;
	  return AY_EOMEM;
	}
    }

#if defined(WIN32) && !defined(AYUSESUPERGLU)
  gluNurbsCallback(npatch->no, GLU_ERROR, (GLUnurbsErrorProc)ay_error_glucb);
#else
  gluNurbsCallback(npatch->no, GLU_ERROR, ay_error_glucb);
#endif

  gluBeginSurface(npatch->no);

  gluNurbsProperty(npatch->no, GLU_SAMPLING_TOLERANCE,
		   (GLfloat)sampling_tolerance);

  gluNurbsProperty(npatch->no, GLU_DISPLAY_MODE, GLU_FILL);

  /*gluNurbsProperty(npatch->no, GLU_AUTO_LOAD_MATRIX, GL_FALSE);*/
  gluNurbsProperty(npatch->no, GLU_CULLING, GL_TRUE);

  gluNurbsSurface(npatch->no, (GLint)uknot_count, uknots,
		  (GLint)vknot_count, vknots,
		  (GLint)height*(npatch->is_rat?4:3),
		  (GLint)(npatch->is_rat?4:3),
		  controls,
		  (GLint)npatch->uorder, (GLint)npatch->vorder,
		  (npatch->is_rat?GL_MAP2_VERTEX_4:GL_MAP2_VERTEX_3));

  /* draw trimcurves */
  if(o->down && o->down->next)
    {
      trim = o->down;
      ay_errno = AY_OK;
      while(trim->next)
	{
	  switch(trim->type)
	    {
	    case AY_IDNCURVE:
	      gluBeginTrim(npatch->no);
	       ay_status = ay_npt_drawtrimcurve(togl, trim, npatch->no);
	      gluEndTrim(npatch->no);
	      break;
	    case AY_IDLEVEL:
	      /* XXXX check, whether level is of type trimloop? */
	      loop = trim->down;
	      if(loop && loop->next)
		{
		  gluBeginTrim(npatch->no);
		  while(loop->next)
		    {

		      if(loop->type == AY_IDNCURVE)
			{
			  ay_status = ay_npt_drawtrimcurve(togl, loop,
							   npatch->no);
			}
		      else
			{
			  nc = NULL;
			  ay_status = ay_provide_object(loop, AY_IDNCURVE,
							&nc);
			  if(nc)
			    {
			      ay_status = ay_npt_drawtrimcurve(togl, nc,
							       npatch->no);
			      ay_object_delete(nc);
			    } /* if */
			} /* if */
		      loop = loop->next;
		    } /* while */
		  gluEndTrim(npatch->no);
		} /* if */
	      break;
	    default:
	      nc = NULL;
	      ay_status = ay_provide_object(trim, AY_IDNCURVE, &nc);
	      if(nc)
		{
		  gluBeginTrim(npatch->no);
		   ay_status = ay_npt_drawtrimcurve(togl, nc, npatch->no);
		  gluEndTrim(npatch->no);
		  ay_object_delete(nc);
		}
	      break;
	    } /* switch */
	  trim = trim->next;
	} /* while */
    } /* if */

  gluEndSurface(npatch->no);

 return AY_OK;
} /* ay_npatch_shadecb */


int
ay_npatch_drawhcb(struct Togl *togl, ay_object *o)
{
 int width = 0, height = 0, i = 0, a = 0;
 ay_nurbpatch_object *patch = (ay_nurbpatch_object *) o->refine;
 GLdouble *ver = NULL;
 double point_size = ay_prefs.handle_size;

  width = patch->width;
  height = patch->height;

  ver = patch->controlv;

  glPointSize((GLfloat)point_size);

  glBegin(GL_POINTS);
  for(i = 0; i < (width * height); i++)
    {
      glVertex3dv((GLdouble *)&ver[a]);
      a += 4;
    }
  glEnd();

  /* draw arrows */
  ay_draw_arrow(togl, &(ver[width*height*4-8]), &(ver[width*height*4-4]));

 return AY_OK;
} /* ay_npatch_drawhcb */


int
ay_npatch_getpntcb(int mode, ay_object *o, double *p)
{
 ay_nurbpatch_object *npatch = NULL;
 double min_dist = ay_prefs.pick_epsilon, dist = 0.0;
 double *pecoord = NULL, **pecoords = NULL, *control = NULL, *c;
 int i = 0, j = 0, a = 0, found = AY_FALSE;

  if(!o || !p)
    return AY_ENULL;

  npatch = (ay_nurbpatch_object *)(o->refine);

  if(min_dist == 0.0)
    min_dist = DBL_MAX;

  if(ay_point_edit_coords)
    free(ay_point_edit_coords);

  ay_point_edit_coords = NULL;

  /* select all points? */
  if(mode == 0)
    { /* yes */
      if(!(ay_point_edit_coords = calloc(npatch->width * npatch->height,
					 sizeof(double*))))
	return AY_EOMEM;

      for(i = 0; i < (npatch->width*npatch->height); i++)
	{
	  ay_point_edit_coords[i] = &(npatch->controlv[a]);
	  a += 4;
	}

      ay_point_edit_coords_homogenous = AY_TRUE;
      ay_point_edit_coords_number = npatch->width * npatch->height;
    }
  else
    { /* no */

      /* selection based on a single point? */
      if(mode == 1)
	{ /* yes */

	  control = npatch->controlv;
	  for(i = 0; i < (npatch->width * npatch->height); i++)
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
	    }

	  if(!pecoord)
	    return AY_OK; /* XXXX should this return a 'AY_EPICK' ? */

	  ay_point_edit_coords_homogenous = AY_TRUE;

	  if(!found)
	    {

	      if(!(ay_point_edit_coords = calloc(1, sizeof(double*))))
		return AY_EOMEM;

	      ay_point_edit_coords[0] = pecoord;
	      ay_point_edit_coords_number = 1;
	    }

	}
      else
	{ /* no */

	  /* selection based on planes */
	  control = npatch->controlv;
	  j = 0;
	  a = 0;
	  for(i = 0; i < npatch->width * npatch->height; i++)
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
} /* ay_npatch_getpntcb */


/* Tcl -> C! */
int
ay_npatch_setpropcb(Tcl_Interp *interp, int argc, char *argv[], ay_object *o)
{
 int ay_status = AY_OK;
 char *n1 = "NPatchAttrData";
 char fname[] = "npatch_setpropcb";
 Tcl_Obj *to = NULL, *toa = NULL, *ton = NULL;
 ay_nurbpatch_object *npatch = NULL;
 int new_uorder, new_width, new_uknot_type;
 int new_vorder, new_height, new_vknot_type;
 double *nknotv = NULL;
 int updateKnots = AY_FALSE;
 int knotc, i;
 char **knotv;

  if(!o)
    return AY_ENULL;

  npatch = (ay_nurbpatch_object *)o->refine;

  toa = Tcl_NewStringObj(n1,-1);
  ton = Tcl_NewStringObj(n1,-1);

  /* get new values from Tcl */
  Tcl_SetStringObj(ton,"Width",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp,to, &new_width);

  Tcl_SetStringObj(ton,"Height",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp,to, &new_height);

  Tcl_SetStringObj(ton,"Order_U",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp,to, &new_uorder);

  Tcl_SetStringObj(ton,"Order_V",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp,to, &new_vorder);

  Tcl_SetStringObj(ton,"Knot-Type_U",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp,to, &new_uknot_type);

  Tcl_SetStringObj(ton,"Knot-Type_V",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp,to, &new_vknot_type);

  Tcl_SetStringObj(ton,"Tolerance",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetDoubleFromObj(interp,to, &(npatch->glu_sampling_tolerance));

  Tcl_SetStringObj(ton,"DisplayMode",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp,to, &(npatch->glu_display_mode));

  Tcl_IncrRefCount(toa);Tcl_DecrRefCount(toa);
  Tcl_IncrRefCount(ton);Tcl_DecrRefCount(ton);


  /* apply changed values to patch */

  /* resize patch */
  if(new_width != npatch->width && (new_width > 1))
    {
      if(o->selp)
	{
	  ay_selp_clear(o);
	}

      ay_status = ay_npt_resizew(npatch, new_width);

      if(ay_status)
       ay_error(AY_ERROR,fname,"Could not resize patch!");

      updateKnots = AY_TRUE;
      o->modified = AY_TRUE;
    } /* if */

  if(new_height != npatch->height && (new_height > 1))
    {
      if(o->selp)
	{
	  ay_selp_clear(o);
	}

      ay_status = ay_npt_resizeh(npatch, new_height);

      if(ay_status)
       ay_error(AY_ERROR,fname,"Could not resize patch!");

      updateKnots = AY_TRUE;
      o->modified = AY_TRUE;
    } /* if */

  /* apply new order */
  if((npatch->uorder != new_uorder) && (new_uorder > 1))
    {
      npatch->uorder = new_uorder;
      updateKnots = AY_TRUE;
      o->modified = AY_TRUE;
    }

  if((npatch->vorder != new_vorder) && (new_vorder > 1))
    {
      npatch->vorder = new_vorder;
      updateKnots = AY_TRUE;
      o->modified = AY_TRUE;
    }

  /* change knot type */
  if((new_uknot_type != npatch->uknot_type) || (updateKnots))
    {
      npatch->uknot_type = new_uknot_type;
      updateKnots = AY_TRUE;
      o->modified = AY_TRUE;
    }

  if((new_vknot_type != npatch->vknot_type) || (updateKnots))
    {
      npatch->vknot_type = new_vknot_type;
      updateKnots = AY_TRUE;
      o->modified = AY_TRUE;
    }

  /* plausibility checks */
  if(npatch->uknot_type == AY_KTBEZIER)
    {
      if(npatch->uorder != npatch->width)
	{
	  ay_error(AY_EWARN,fname, "Changing uorder to match width!");
	  npatch->uorder = npatch->width;
	}
    }

  if(npatch->vknot_type == AY_KTBEZIER)
    {
      if(npatch->vorder != npatch->height)
	{
	  ay_error(AY_EWARN, fname, "Changing vorder to match height!");
	  npatch->vorder = npatch->height;
	}
    }

  if(npatch->width < npatch->uorder)
    {
      ay_error(AY_EWARN, fname, "Lowering uorder to match width!");
      npatch->uorder = npatch->width;
    }

  if(npatch->height < npatch->vorder)
    {
      ay_error(AY_EWARN, fname, "Lowering vorder to match height!");
      npatch->vorder = npatch->height;
    }

  if(updateKnots)
    {
      ay_status = ay_knots_createnp(npatch);
      if(ay_status)
	ay_error(AY_ERROR, fname, "Error creating new knots!");
    }

  /* decompose uknot-list (create custom knot sequence) */
  if(npatch->uknot_type == AY_KTCUSTOM)
    {
      Tcl_SplitList(interp, Tcl_GetVar2(interp, n1, "Knots_U",
					TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY),
		    &knotc, &knotv);

      if(!(nknotv = calloc(knotc, sizeof(double))))
	{
	  ay_error(AY_EOMEM, fname, NULL);
	  Tcl_Free((char *) knotv);
	  return TCL_OK;
	}

      for(i = 0; i < knotc; i++)
	{
	  Tcl_GetDouble(interp, knotv[i], &nknotv[i]);
	} /* for */

      if(!(ay_status = ay_knots_check(new_width, new_uorder, knotc, nknotv)))
	{/* the knots are ok */
	  free(npatch->uknotv);
	  npatch->uknotv = nknotv;
	}
      else
	{/* the knots are wrong */
	  /* first, tell the user what went wrong */
	  switch (ay_status)
	    {
	    case 1:
	      ay_error(AY_ERROR, fname, "Knot sequence has too few knots!");
	      break;
	    case 2:
	      ay_error(AY_ERROR, fname, "Knot sequence has too much knots!");
	      break;
	    case 3:
	      ay_error(AY_ERROR, fname,
		       "Knot multiplicity higher than order!");
	      break;
	    case 4:
	      ay_error(AY_ERROR, fname, "Knot sequence has decreasing knots!");
	      break;
	    } /* switch */

	  free(nknotv);

	  /* create new knots */
	  ay_error(AY_EWARN,fname,
			 "Falling back to knot type NURB!");
	  npatch->uknot_type = AY_KTNURB;

	  ay_status = ay_knots_createnp(npatch);

	  if(ay_status)
	    ay_error(AY_ERROR, fname, "Error creating new knots!");
	} /* if */

      /* XXXX compare old and new knots before setting this flag */
      o->modified = AY_TRUE;

      Tcl_Free((char *) knotv);
    } /* if */

  /* decompose vknot-list (create custom knot sequence) */
  if(npatch->vknot_type == AY_KTCUSTOM)
    {
      Tcl_SplitList(interp,Tcl_GetVar2(interp, n1, "Knots_V",
				       TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY),
		    &knotc, &knotv);

      if(!(nknotv = calloc(knotc, sizeof(double))))
	{
	  ay_error(AY_EOMEM, fname, NULL);
	  Tcl_Free((char *) knotv);
	  return TCL_OK;
	}

      for(i = 0; i < knotc; i++)
	{
	  Tcl_GetDouble(interp, knotv[i], &nknotv[i]);
	} /* for */

      if(!(ay_status = ay_knots_check(new_height, new_vorder, knotc, nknotv)))
	{/* the knots are ok */
	  free(npatch->vknotv);
	  npatch->vknotv = nknotv;
	}
      else
	{/* the knots are wrong */
	  /* first, tell the user what went wrong */
	  switch (ay_status)
	    {
	    case 1:
	      ay_error(AY_ERROR, fname, "Knot sequence has too few knots!");
	      break;
	    case 2:
	      ay_error(AY_ERROR, fname, "Knot sequence has too much knots!");
	      break;
	    case 3:
	      ay_error(AY_ERROR, fname,
		       "Knot multiplicity higher than order!");
	      break;
	    case 4:
	      ay_error(AY_ERROR, fname, "Knot sequence has decreasing knots!");
	      break;
	    } /* switch */

	  free(nknotv);

	  /* create new knots */
	  ay_error(AY_EWARN,fname,
			 "Falling back to knot type NURB!");
	  npatch->vknot_type = AY_KTNURB;

	  ay_status = ay_knots_createnp(npatch);

	  if(ay_status)
	    ay_error(AY_ERROR, fname, "Error creating new knots!");
	} /* if */

      /* XXXX compare old and new knots before setting this flag */
      o->modified = AY_TRUE;

      Tcl_Free((char *) knotv);
    } /* if */


  ay_status = ay_notify_parent();

 return AY_OK;
} /* ay_npatch_setpropcb */


/* C -> Tcl! */
int
ay_npatch_getpropcb(Tcl_Interp *interp, int argc, char *argv[], ay_object *o)
{
 char *n1="NPatchAttrData";
 Tcl_Obj *to = NULL, *toa = NULL, *ton = NULL;
 ay_nurbpatch_object *npatch = NULL;
 int i;

  if(!o)
    return AY_ENULL;

  npatch = (ay_nurbpatch_object *)(o->refine);

  toa = Tcl_NewStringObj(n1,-1);

  ton = Tcl_NewStringObj(n1,-1);


  Tcl_SetStringObj(ton,"Width",-1);
  to = Tcl_NewIntObj(npatch->width);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton,"Height",-1);
  to = Tcl_NewIntObj(npatch->height);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton,"Order_U",-1);
  to = Tcl_NewIntObj(npatch->uorder);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton,"Order_V",-1);
  to = Tcl_NewIntObj(npatch->vorder);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton,"Knot-Type_U",-1);
  to = Tcl_NewIntObj(npatch->uknot_type);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton,"Knot-Type_V",-1);
  to = Tcl_NewIntObj(npatch->vknot_type);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);

  Tcl_SetVar2(interp,n1,"Knots_U","", TCL_LEAVE_ERR_MSG |
	      TCL_GLOBAL_ONLY);
  Tcl_SetStringObj(ton,"Knots_U",-1);
  for(i=0; i<npatch->width+npatch->uorder; i++)
    {

      to = Tcl_NewDoubleObj((npatch->uknotv)[i]);

      Tcl_ObjSetVar2(interp,toa,ton,to,TCL_APPEND_VALUE |
		     TCL_LIST_ELEMENT | TCL_LEAVE_ERR_MSG |
		     TCL_GLOBAL_ONLY);
    }

  Tcl_SetVar2(interp,n1,"Knots_V","", TCL_LEAVE_ERR_MSG |
	      TCL_GLOBAL_ONLY);
  Tcl_SetStringObj(ton,"Knots_V",-1);
  for(i=0; i<npatch->height+npatch->vorder; i++)
    {

      to = Tcl_NewDoubleObj((npatch->vknotv)[i]);

      Tcl_ObjSetVar2(interp,toa,ton,to,TCL_APPEND_VALUE |
		     TCL_LIST_ELEMENT | TCL_LEAVE_ERR_MSG |
		     TCL_GLOBAL_ONLY);
    }

  Tcl_SetStringObj(ton,"Tolerance",-1);
  to = Tcl_NewDoubleObj(npatch->glu_sampling_tolerance);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton,"DisplayMode",-1);
  to = Tcl_NewIntObj(npatch->glu_display_mode);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton,"IsRat", -1);
  if(npatch->is_rat)
    to = Tcl_NewStringObj("yes", -1);
  else
    to = Tcl_NewStringObj("no", -1);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);

  Tcl_IncrRefCount(toa);Tcl_DecrRefCount(toa);
  Tcl_IncrRefCount(ton);Tcl_DecrRefCount(ton);

 return AY_OK;
} /* ay_npatch_getpropcb */


int
ay_npatch_readcb(FILE *fileptr, ay_object *o)
{
 int ay_status = AY_OK;
 ay_nurbpatch_object *npatch = NULL;
 int i, a;

 if(!o)
   return AY_ENULL;

  if(!(npatch = calloc(1, sizeof(ay_nurbpatch_object))))
    { return AY_EOMEM; }


  fscanf(fileptr,"%d\n",&npatch->width);
  fscanf(fileptr,"%d\n",&npatch->height);
  fscanf(fileptr,"%d\n",&npatch->uorder);
  fscanf(fileptr,"%d\n",&npatch->vorder);
  fscanf(fileptr,"%d\n",&npatch->uknot_type);
  fscanf(fileptr,"%d\n",&npatch->vknot_type);

  ay_status = ay_knots_createnp(npatch);
  if(ay_status)
    { free(npatch); return ay_status; }

  if(npatch->uknot_type == AY_KTCUSTOM)
    {
      if(npatch->uknotv)
	free(npatch->uknotv);
      npatch->uknotv = NULL;
      if(!(npatch->uknotv =
	   calloc((npatch->width + npatch->uorder), sizeof(double))))
	{ free(npatch); return AY_EOMEM; }

      for(i=0; i<(npatch->width + npatch->uorder); i++)
	fscanf(fileptr,"%lg\n",&(npatch->uknotv[i]));
    }

  if(npatch->vknot_type == AY_KTCUSTOM)
    {
      if(npatch->vknotv)
	free(npatch->vknotv);
      npatch->vknotv = NULL;

      if(!(npatch->vknotv =
	   calloc((npatch->height + npatch->vorder), sizeof(double))))
	{ free(npatch); free(npatch->uknotv); return AY_EOMEM; }

      for(i=0; i<(npatch->height + npatch->vorder); i++)
	fscanf(fileptr,"%lg\n",&(npatch->vknotv[i]));
    }

  if(!(npatch->controlv = calloc(npatch->width*npatch->height*4,
				 sizeof(double))))
    {
      free(npatch->uknotv); free(npatch->vknotv); free(npatch);
      return AY_EOMEM;
    }

  a = 0;
  for(i=0; i < npatch->width*npatch->height; i++)
    {
      fscanf(fileptr,"%lg %lg %lg %lg\n",&(npatch->controlv[a]),
	     &(npatch->controlv[a+1]),
	     &(npatch->controlv[a+2]),
	     &(npatch->controlv[a+3]));
      a+=4;
    }

  fscanf(fileptr,"%lg\n",&(npatch->glu_sampling_tolerance));
  fscanf(fileptr,"%d\n",&(npatch->glu_display_mode));

  npatch->is_rat = ay_npt_israt(npatch);

  o->refine = npatch;

 return AY_OK;
} /* ay_npatch_readcb */


int
ay_npatch_writecb(FILE *fileptr, ay_object *o)
{
 ay_nurbpatch_object *npatch = NULL;
 int i, a;

  if(!o)
    return AY_ENULL;

  npatch = (ay_nurbpatch_object *)(o->refine);

  fprintf(fileptr, "%d\n", npatch->width);
  fprintf(fileptr, "%d\n", npatch->height);
  fprintf(fileptr, "%d\n", npatch->uorder);
  fprintf(fileptr, "%d\n", npatch->vorder);
  fprintf(fileptr, "%d\n", npatch->uknot_type);
  fprintf(fileptr, "%d\n", npatch->vknot_type);

  if(npatch->uknot_type == AY_KTCUSTOM)
    {
      for(i=0;i<(npatch->width+npatch->uorder);i++)
	fprintf(fileptr,"%g\n",npatch->uknotv[i]);
    }

  if(npatch->vknot_type == AY_KTCUSTOM)
    {
      for(i=0;i<(npatch->height+npatch->vorder);i++)
	fprintf(fileptr,"%g\n",npatch->vknotv[i]);
    }

  a = 0;
  for(i=0;i<npatch->width*npatch->height;i++)
    {
      fprintf(fileptr,"%g %g %g %g\n", npatch->controlv[a],
	      npatch->controlv[a+1],
	      npatch->controlv[a+2],
	      npatch->controlv[a+3]);
      a+=4;
    }

  fprintf(fileptr, "%g\n", npatch->glu_sampling_tolerance);
  fprintf(fileptr, "%d\n", npatch->glu_display_mode);

 return AY_OK;
} /* ay_npatch_writecb */


int
ay_npatch_wribcb(char *file, ay_object *o)
{
 int ay_status = AY_OK;
 ay_nurbpatch_object *patch = NULL;
 double umind, umaxd, vmind, vmaxd;
 RtInt nu, nv, uorder, vorder;
 RtFloat *uknots = NULL, *vknots = NULL, umin, umax, vmin, vmax;
 RtFloat *controls = NULL;
 RtToken *tokens = NULL;
 RtPointer *parms = NULL;
 int i = 0, j = 0, a = 0, b = 0, n = 0, pvc = 0;

  if(!o)
    return AY_OK;

  if(o->down)
    {
      ay_status = ay_npt_wribtrimcurves(o);
    }

  patch = (ay_nurbpatch_object*)(o->refine);

  nu = (RtInt)patch->width;
  uorder = (RtInt)patch->uorder;
  nv = (RtInt)patch->height;
  vorder = (RtInt)patch->vorder;

  if((uknots = calloc(nu+uorder, sizeof(RtFloat))) == NULL)
    return AY_EOMEM;
  if((vknots = calloc(nv+vorder, sizeof(RtFloat))) == NULL)
    return AY_EOMEM;
  if((controls = calloc(nu*nv*(patch->is_rat?4:3), sizeof(RtFloat))) == NULL)
    return AY_EOMEM;

  a = 0;
  for(i = 0; i < nu+uorder; i++)
    {
      uknots[a] = (RtFloat)patch->uknotv[a];
      a++;
    }
  a = 0;
  for(i = 0; i < nv+vorder; i++)
    {
      vknots[a] = (RtFloat)patch->vknotv[a];
      a++;
    }
  a = 0;
  /* RenderMan expects u-major order! */
  for(i = 0; i < nv; i++)
    {
      b = i*4;
      for(j = 0; j < nu; j++)
	{
	  controls[a] = (RtFloat)patch->controlv[b];
	  a++;
	  controls[a] = (RtFloat)patch->controlv[b+1];
	  a++;
	  controls[a] = (RtFloat)patch->controlv[b+2];
	  a++;
	  if(patch->is_rat)
	    {
	      controls[a] = (RtFloat)patch->controlv[b+3];
	      a++;
	    }
	  b += (nv*4);
	} /* for */
    } /* for */

  ay_knots_getuminmax(o, patch->uorder, patch->uorder + patch->width,
		      patch->uknotv, &umind, &umaxd);
  umin = (RtFloat)umind;
  umax = (RtFloat)umaxd;
  ay_knots_getvminmax(o, patch->vorder, patch->vorder + patch->height,
		      patch->vknotv, &vmind, &vmaxd);
  vmin = (RtFloat)vmind;
  vmax = (RtFloat)vmaxd;

  /* Do we have any primitive variables? */
  if(!(pvc = ay_pv_count(o)))
    {
      /* No */
      RiNuPatch(nu, uorder, uknots,
		/*(RtFloat)uknots[uorder-1], (RtFloat)uknots[nu],*/
		umin, umax,
		nv, vorder, vknots,
		/*(RtFloat)vknots[vorder-1], (RtFloat)vknots[nv],*/
		vmin, vmax,
		patch->is_rat?"Pw":"P", controls, NULL);
    }
  else
    {
      /* Yes, we have primitive variables. */
      if(!(tokens = calloc(pvc+1, sizeof(RtToken))))
	return AY_EOMEM;

      if(!(parms = calloc(pvc+1, sizeof(RtPointer))))
	return AY_EOMEM;

      if(patch->is_rat)
	tokens[0] = "Pw";
      else
	tokens[0] = "P";
      parms[0] = (RtPointer)controls;

      n = 1;
      ay_pv_filltokpar(o, AY_TRUE, 1, &n, tokens, parms);

      RiNuPatchV(nu, uorder, uknots,
		 /*(RtFloat)uknots[uorder-1], (RtFloat)uknots[nu],*/
		 umin, umax,
		 nv, vorder, vknots,
		 /*(RtFloat)vknots[vorder-1], (RtFloat)vknots[nv],*/
		 vmin, vmax,
		 (RtInt)n, tokens, parms);

      for(i = 1; i < n; i++)
	{
	  free(tokens[i]);
	  free(parms[i]);
	}

      free(tokens);
      free(parms);
    } /* if */

  free(uknots);
  free(vknots);
  free(controls);

 return ay_status;
} /* ay_npatch_wribcb */


int
ay_npatch_bbccb(ay_object *o, double *bbox, int *flags)
{
 double xmin, xmax, ymin, ymax, zmin, zmax;
 double *controlv = NULL;
 int i, a;
 ay_nurbpatch_object *npatch = NULL;

  if(!o || !bbox)
    return AY_ENULL;

  npatch = (ay_nurbpatch_object *)o->refine;

  controlv = npatch->controlv;

  xmin = controlv[0];
  xmax = xmin;
  ymin = controlv[1];
  ymax = ymin;
  zmin = controlv[2];
  zmax = zmin;

  a = 0;
  for(i = 0; i < npatch->width*npatch->height; i++)
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

  /* exclusive bounding box, discard children/trim curves bbox */
  *flags = 1;

 return AY_OK;
} /* ay_npatch_bbccb */


int
ay_npatch_providecb(ay_object *o, unsigned int type, ay_object **result)
{
 int ay_status = AY_OK;
 /*char fname[] = "npatch_providecb";*/
 ay_tag_object *tag = NULL;
 int smethod = ay_prefs.smethod;
 double sparamu = ay_prefs.sparamu, sparamv = ay_prefs.sparamv;

  if(!o)
    return AY_ENULL;

  if(!result)
    {
      if(type == AY_IDPOMESH)
	return AY_OK;
      else
	return AY_ERROR;
    }

  if(type == AY_IDPOMESH)
    {

      /* infer parameters from (eventually present) TP tag */
      tag = o->tags;
      while(tag)
	{
	  if(tag->type == ay_tp_tagtype)
	    {
	      if(tag->val)
		sscanf(tag->val,"%d,%lg,%lg",&smethod, &sparamu, &sparamv);
	    }
	  tag = tag->next;
	} /* while */

      ay_status = ay_tess_npatch(o, smethod, sparamu, sparamv, result);
    } /* if */

 return ay_status;
} /* ay_npatch_providecb */

int
ay_npatch_convertcb(ay_object *o, int in_place)
{
 int ay_status = AY_OK;
 /*char fname[] = "npatch_convertcb";*/
 ay_object *new = NULL;

  if(!o)
    return AY_ENULL;

  ay_status = ay_npatch_providecb(o, AY_IDPOMESH, &new);

  if(new)
    {
      if(!in_place)
	{
	  ay_trafo_copy(o, new);
	  ay_status = ay_object_link(new);
	}
      else
	{
	  ay_status = ay_object_replace(new, o);
	}
    }

 return ay_status;
} /* ay_npatch_convertcb */

int
ay_npatch_init(Tcl_Interp *interp)
{
 int ay_status = AY_OK;

  ay_status = ay_otype_registercore(ay_npatch_name,
				    ay_npatch_createcb,
				    ay_npatch_deletecb,
				    ay_npatch_copycb,
				    ay_npatch_drawcb,
				    ay_npatch_drawhcb,
				    ay_npatch_shadecb,
				    ay_npatch_setpropcb,
				    ay_npatch_getpropcb,
				    ay_npatch_getpntcb,
				    ay_npatch_readcb,
				    ay_npatch_writecb,
				    ay_npatch_wribcb,
				    ay_npatch_bbccb,
				    AY_IDNPATCH);

  ay_status = ay_provide_register(ay_npatch_providecb, AY_IDNPATCH);

  ay_status = ay_convert_register(ay_npatch_convertcb, AY_IDNPATCH);

 return ay_status;
} /* ay_npatch_init */

