/*
 * Ayam, a free 3D modeler for the RenderMan interface.
 *
 * Ayam is copyrighted 1998-2018 by Randolf Schultz
 * (randolf.schultz@gmail.com) and others.
 *
 * All rights reserved.
 *
 * See the file License for details.
 *
 */

#include "ayam.h"

/* sdcurve.c - subdivsion curve custom object */

char sdcurve_version_ma[] = AY_VERSIONSTR;
char sdcurve_version_mi[] = AY_VERSIONSTRMI;

static char *sdcurve_name = "SDCurve";

static unsigned int sdcurve_id;

typedef struct sdcurve_object_s
{
  int closed; /**< is the curve closed? */
  int length; /**< number of control points */
  int level; /**< number of subdivisions */
  int type; /* subdivision method */

  double *controlv; /**< control points [length*3] */

  int scontrolvlen; /**< number of subdivided control points */
  double *scontrolv; /**< subdivided control points [scontrolvlen*3] */
} sdcurve_object;

#ifdef WIN32
  __declspec (dllexport)
#endif /* WIN32 */
int Sdcurve_Init(Tcl_Interp *interp);

/** sdcurve_toncurve:
 * Convert SDCurve to NURBS curve.
 *
 * \param[in] sdcurve SDCurve to convert
 * \param[in,out] newo where to store the new NURBS curve object
 *
 * \returns AY_OK on success, error code otherwise.
 */
int
sdcurve_toncurve(sdcurve_object *sdcurve, ay_object **newo)
{
 int ay_status = AY_OK;
 ay_object *new = NULL;
 ay_nurbcurve_object *ncurve = NULL;
 double *cv = NULL;
 double *scv = NULL, *a, *b;
 int i, len = 0;

  if(!sdcurve || !newo)
    return AY_ENULL;

  if(sdcurve->level)
    {
      len = sdcurve->scontrolvlen;
      scv = sdcurve->scontrolv;
    }
  else
    {
      len = sdcurve->length;
      scv = sdcurve->controlv;
    }

  if(!(cv = malloc(4*(len+(sdcurve->closed?1:0))*sizeof(double))))
    return AY_EOMEM;

  a = cv;
  b = scv;
  for(i = 0; i < len; i++)
    {
      memcpy(a, b, 3*sizeof(double));
      a[3] = 1.0;

      a += 4;
      b += 3;
    }

  if(sdcurve->closed)
    {
      memcpy(a, scv, 3*sizeof(double));
      a[3] = 1.0;
      len++;
    }

  ay_status = ay_nct_create(2, len, AY_KTNURB, cv, NULL, &ncurve);

  if(ncurve)
    {
      cv = NULL;
      if(!(new = calloc(1, sizeof(ay_object))))
	{
	  ay_status = AY_EOMEM;
	  goto cleanup;
	}
      ay_object_defaults(new);
      new->type = AY_IDNCURVE;
      new->refine = ncurve;
      *newo = new;
      ncurve = NULL;
    }

cleanup:

 if(ncurve)
   {
     ay_nct_destroy(ncurve);
   }

 if(cv)
   {
     free(cv);
   }

 return ay_status;
} /* sdcurve_toncurve */


int
sdcurve_chaikinc(sdcurve_object *sdcurve)
{
 int ay_status = AY_OK;
 int i, j, d, d2, new_length, cur_length;
 double *cv, *scv, *a, *b, v[3], p1[3], p2[3];

  if(!sdcurve)
    return AY_ENULL;

  d = 6;
  d2 = 3;
  cv = sdcurve->controlv;
  cur_length = sdcurve->length;

  for(i = 0; i < sdcurve->level; i++)
    {
      new_length = (cur_length+2)*2;

      if(!(scv = calloc(3, new_length*sizeof(double))))
	return AY_EOMEM;

      /* spread original points */
      memcpy(scv, &(cv[cur_length*3-3]), 3*sizeof(double));
      a = cv;
      b = &(scv[d]);

      for(j = 0; j < cur_length; j++)
	{
	  memcpy(b, a, 3*sizeof(double));
	  a += 3;
	  b += d;
	} /* for */

      memcpy(&(scv[new_length*3-6]), cv, 3*sizeof(double));
      cur_length++;

      /* subdivide */
      a = scv;
      b = a + d;
      for(j = 0; j < cur_length; j++)
	{
	  AY_V3SUB(v, b, a);
	  AY_V3SCAL(v, 0.25);
	  AY_V3ADD(p1, a, v);

	  AY_V3SUB(v, b, a);
	  AY_V3SCAL(v, 0.75);
	  AY_V3ADD(p2, a, v);

	  memcpy(a, p1, 3*sizeof(double));
	  memcpy(a+d2, p2, 3*sizeof(double));

	  a += d;
	  b += d;
	} /* for */

      new_length -= 4;

      if(cv != sdcurve->controlv)
	free(cv);

      cv = scv;
      cur_length = new_length;
    } /* for */

  /* put result to sdcurve */
  new_length = sdcurve->length;
  for(i = 0; i < sdcurve->level; i++)
    new_length *= 2;

  sdcurve->scontrolvlen = new_length;
  sdcurve->scontrolv = scv;

 return ay_status;
} /* sdcurve_chaikinc */


int
sdcurve_chaikino(sdcurve_object *sdcurve)
{
 int ay_status = AY_OK;
 int i, j, d, d2, new_length, cur_length;
 double *scv, *a, *b, v[3], p1[3], p2[3];

  if(!sdcurve)
    return AY_ENULL;

  new_length = sdcurve->length+1;
  d = 6;
  d2 = 3;
  for(i = 0; i < sdcurve->level; i++)
    {
      new_length *= 2;
      if(i > 0)
	{
	  d *= 2;
	  d2 *= 2;
	}
    }

  if(!(scv = calloc(3, new_length*sizeof(double))))
    return AY_EOMEM;

  /* spread original points */
  a = sdcurve->controlv;
  b = scv;

  for(i = 0; i < sdcurve->length; i++)
    {
      memcpy(b, a, 3*sizeof(double));
      a += 3;
      b += d;
    } /* for */

  cur_length = sdcurve->length;

  /* subdivide */
  for(i = 0; i < sdcurve->level; i++)
    {
      a = scv;
      b = a + d;
      for(j = 0; j < cur_length; j++)
	{
	  AY_V3SUB(v, b, a);
	  AY_V3SCAL(v, 0.25);
	  AY_V3ADD(p1, a, v);

	  AY_V3SUB(v, b, a);
	  AY_V3SCAL(v, 0.75);
	  AY_V3ADD(p2, a, v);

	  memcpy(a, p1, 3*sizeof(double));
	  memcpy(a+d2, p2, 3*sizeof(double));

	  a += d;
	  b += d;
	} /* for */

      /* prepare next iteration */
      d /= 2;
      d2 /= 2;
      cur_length *= 2;
    } /* for */

  /* put result to sdcurve */
  new_length = sdcurve->length;

  for(i = 0; i < sdcurve->level; i++)
    {
      new_length *= 2;
      new_length -= 2;
    }

  sdcurve->scontrolvlen = new_length;
  sdcurve->scontrolv = scv;

 return ay_status;
} /* sdcurve_chaikino */

int
sdcurve_cubicc(sdcurve_object *sdcurve)
{
 int ay_status = AY_OK;
 int i, j, d, d2, new_length, cur_length;
 double *cv, *scv, *a, *b, *c, v[3];

  if(!sdcurve)
    return AY_ENULL;

  d = 6;
  d2 = 3;
  cv = sdcurve->controlv;
  cur_length = sdcurve->length;

  for(i = 0; i < sdcurve->level; i++)
    {
      new_length = (cur_length+1)*2;

      if(!(scv = malloc(3*new_length*sizeof(double))))
	return AY_EOMEM;

      /* spread original points */
      a = cv;
      b = scv;
      for(j = 0; j < cur_length; j++)
	{
	  memcpy(b, a, 3*sizeof(double));
	  a += 3;
	  b += d;
	} /* for */

      memcpy(&(scv[new_length*3-6]), scv, 3*sizeof(double));

      /* subdivide */

      /* first run: construct new vertices at each half original edge */
      a = scv;
      b = a+d;
      c = a+d2;
      for(j = 0; j < cur_length; j++)
	{
	  AY_V3SUB(v, b, a);
	  AY_V3SCAL(v, 0.5);
	  AY_V3ADD(v, v, a);
	  memcpy(c, v, 3*sizeof(double));

	  a += d;
	  b += d;
	  c += d;
	} /* for */

      /* second run: move old vertices to barycenters */
      a = &(scv[d2]);
      b = a + d2;
      c = b + d2;
      for(j = 1; j < cur_length; j++)
	{
	  v[0] = (a[0]+b[0]+c[0])*1.0/3.0;
	  v[1] = (a[1]+b[1]+c[1])*1.0/3.0;
	  v[2] = (a[2]+b[2]+c[2])*1.0/3.0;

	  memcpy(b, v, 3*sizeof(double));

	  a += d;
	  b += d;
	  c += d;
	} /* for */

      /* first point */
      a = &(scv[(new_length*3)-d-d2]);
      b = scv;
      c = b + d2;
      v[0] = (a[0]+b[0]+c[0])*1.0/3.0;
      v[1] = (a[1]+b[1]+c[1])*1.0/3.0;
      v[2] = (a[2]+b[2]+c[2])*1.0/3.0;
      memcpy(b, v, 3*sizeof(double));

      new_length -= 2;

      if(cv != sdcurve->controlv)
	free(cv);

      cv = scv;
      cur_length = new_length;
    } /* for */

  new_length = sdcurve->length;

  for(i = 0; i < sdcurve->level; i++)
    new_length *= 2;

  sdcurve->scontrolvlen = new_length;
  sdcurve->scontrolv = scv;

 return ay_status;
} /* sdcurve_cubicc */


int
sdcurve_cubico(sdcurve_object *sdcurve)
{
 int ay_status = AY_OK;
 int i, j, new_length, cur_length;
 double *cv, *scv, *a, *b, *c, *d, v[3];

  if(!sdcurve)
    return AY_ENULL;

  cv = sdcurve->controlv;
  cur_length = sdcurve->length;

  for(i = 0; i < sdcurve->level; i++)
    {
      new_length = cur_length*2-3;

      if(!(scv = malloc(3*new_length*sizeof(double))))
	return AY_EOMEM;

      /* subdivide */

      /* first run: construct new vertices at each half original edge */
      a = cv;
      b = a+3;
      c = scv;
      for(j = 0; j < cur_length-1; j++)
	{
	  AY_V3SUB(v, b, a);
	  AY_V3SCAL(v, 0.5);
	  AY_V3ADD(v, v, a);
	  memcpy(c, v, 3*sizeof(double));

	  a += 3;
	  b += 3;
	  c += 6;
	} /* for */

      /* second run: move old vertices to barycenters */
      a = scv;
      b = &(cv[3]);
      c = a + 6;
      d = a + 3;
      for(j = 2; j < cur_length; j++)
	{
	  v[0] = (a[0]+b[0]+c[0])*1.0/3.0;
	  v[1] = (a[1]+b[1]+c[1])*1.0/3.0;
	  v[2] = (a[2]+b[2]+c[2])*1.0/3.0;

	  memcpy(d, v, 3*sizeof(double));

	  a += 6;
	  b += 3;
	  c += 6;
	  d += 6;
	} /* for */

      if(cv != sdcurve->controlv)
	free(cv);

      cv = scv;
      cur_length = new_length;
    } /* for */

  new_length = sdcurve->length;

  for(i = 0; i < sdcurve->level; i++)
    {
      new_length *= 2;
      new_length -= 3;
    }

  sdcurve->scontrolvlen = new_length;
  sdcurve->scontrolv = scv;

 return ay_status;
} /* sdcurve_cubico */


/* sdcurve_createcb:
 *  create callback function of sdcurve object
 */
int
sdcurve_createcb(int argc, char *argv[], ay_object *o)
{
 sdcurve_object *sdcurve = NULL;
 char fname[] = "crtsdcurve";
 int a, i;

  if(!o)
    return AY_ENULL;

  if(!(sdcurve = calloc(1, sizeof(sdcurve_object))))
    {
      ay_error(AY_EOMEM, fname, NULL);
      return AY_ERROR;
    }

  sdcurve->length = 4;

  if(!(sdcurve->controlv = malloc(3 * sdcurve->length * sizeof(double))))
    {
      free(sdcurve);
      return AY_EOMEM;
    }

  a = 0;
  for(i = 0; i < sdcurve->length; i++)
    {
      sdcurve->controlv[a] = i*0.25;
      sdcurve->controlv[a+1] = 0;
      sdcurve->controlv[a+2] = 0;
      a += 3;
    }

  o->refine = sdcurve;

  ay_notify_object(o);

 return AY_OK;
} /* sdcurve_createcb */


/* sdcurve_deletecb:
 *  delete callback function of sdcurve object
 */
int
sdcurve_deletecb(void *c)
{
 sdcurve_object *sdcurve = NULL;

  if(!c)
    return AY_ENULL;

  sdcurve = (sdcurve_object *)(c);

  if(sdcurve->controlv)
    free(sdcurve->controlv);

  if(sdcurve->scontrolv)
    free(sdcurve->scontrolv);

  free(sdcurve);

 return AY_OK;
} /* sdcurve_deletecb */


/* sdcurve_copycb:
 *  copy callback function of sdcurve object
 */
int
sdcurve_copycb(void *src, void **dst)
{
 sdcurve_object *sdcurve = NULL, *sdcurvesrc;

  if(!src || !dst)
    return AY_ENULL;

  sdcurvesrc = (sdcurve_object *)src;

  if(!(sdcurve = calloc(1, sizeof(sdcurve_object))))
    return AY_EOMEM;

  memcpy(sdcurve, src, sizeof(sdcurve_object));

  sdcurve->controlv = NULL;
  sdcurve->scontrolv = NULL;

  /* copy controlv */
  if(!(sdcurve->controlv = malloc(3 * sdcurve->length * sizeof(double))))
    {
      free(sdcurve);
      return AY_EOMEM;
    }
  memcpy(sdcurve->controlv, sdcurvesrc->controlv,
	 3 * sdcurve->length * sizeof(double));

  *dst = (void *)sdcurve;

 return AY_OK;
} /* sdcurve_copycb */


/* sdcurve_drawcb:
 *  draw (display in an Ayam view window) callback function of sdcurve object
 */
int
sdcurve_drawcb(struct Togl *togl, ay_object *o)
{
 sdcurve_object *sdcurve = NULL;
 int i = 0, a = 0;

  if(!o)
    return AY_ENULL;

  sdcurve = (sdcurve_object *)o->refine;

  if(!sdcurve)
    return AY_ENULL;

  if(sdcurve->scontrolv)
    {
      if(sdcurve->closed)
	glBegin(GL_LINE_LOOP);
      else
	glBegin(GL_LINE_STRIP);
      for(i = 0; i < sdcurve->scontrolvlen; i++)
	{
	  glVertex3dv(&(sdcurve->scontrolv[a]));
	  a += 3;
	}
      glEnd();
    }

 return AY_OK;
} /* sdcurve_drawcb */


/* sdcurve_drawacb:
 *  draw annotations callback function of sdcurve object
 */
int
sdcurve_drawacb(struct Togl *togl, ay_object *o)
{
 sdcurve_object *sdcurve = NULL;
 GLdouble *ver = NULL;
 int len;

  sdcurve = (sdcurve_object *) o->refine;

  if(!sdcurve)
    return AY_ENULL;

  /* draw orientation arrow */
  if(sdcurve->scontrolv)
    {
      ver = sdcurve->scontrolv;
      len = sdcurve->scontrolvlen;
    }
  else
    {
      ver = sdcurve->controlv;
      len = sdcurve->length;
    }

  if(sdcurve->closed)
    ay_draw_arrow(togl, &(ver[len*3-3]), &(ver[0]));
  else
    ay_draw_arrow(togl, &(ver[len*3-6]), &(ver[len*3-3]));

 return AY_OK;
} /* sdcurve_drawacb */


/* sdcurve_drawhcb:
 *  draw handles callback function of sdcurve object
 */
int
sdcurve_drawhcb(struct Togl *togl, ay_object *o)
{
 sdcurve_object *sdcurve = NULL;
 double *cv = NULL;
 int i;

  if(!o)
    return AY_ENULL;

  sdcurve = (sdcurve_object *)o->refine;

  if(!sdcurve)
    return AY_ENULL;

  cv = sdcurve->controlv;

  /* draw normal points */
  glBegin(GL_POINTS);
   for(i = 0; i < sdcurve->length; i++)
     {
       glVertex3dv((GLdouble *)cv);
       cv += 3;
     }
  glEnd();

 return AY_OK;
} /* sdcurve_drawhcb */


/* sdcurve_getpntcb:
 *  get point (editing and selection) callback function of sdcurve object
 */
int
sdcurve_getpntcb(int mode, ay_object *o, double *p, ay_pointedit *pe)
{
 sdcurve_object *sdcurve = NULL;
 ay_point *pnt = NULL, **lastpnt = NULL;
 double min_dist = ay_prefs.pick_epsilon, dist = 0.0;
 double *pecoord = NULL, **ctmp;
 double *control = NULL, *c;
 int i = 0, j = 0, a = 0;
 unsigned int *itmp, peindex = 0;

  if(!o || ((mode != 3) && (!p || !pe)))
    return AY_ENULL;

  sdcurve = (sdcurve_object *)(o->refine);

  if(!sdcurve)
    return AY_ENULL;

  if(min_dist == 0.0)
    min_dist = DBL_MAX;

  switch(mode)
    {
    case 0:
      /* select all points */
      if(!(pe->coords = malloc(sdcurve->length * sizeof(double*))))
	return AY_EOMEM;
      if(!(pe->indices = malloc(sdcurve->length * sizeof(unsigned int))))
	return AY_EOMEM;

      for(i = 0; i < sdcurve->length; i++)
	{
	  pe->coords[i] = &(sdcurve->controlv[a]);
	  pe->indices[i] = i;
	  a += 3;
	}

      pe->num = sdcurve->length;
      break;
    case 1:
      /* selection based on a single point */
      control = sdcurve->controlv;
      for(i = 0; i < sdcurve->length; i++)
	{
	  dist = AY_VLEN((p[0] - control[j]),
			 (p[1] - control[j+1]),
			 (p[2] - control[j+2]));

	  if(dist < min_dist)
	    {
	      pecoord = &(control[j]);
	      peindex = i;
	      min_dist = dist;
	    }

	  j += 3;
	} /* for */

      if(!pecoord)
	return AY_OK; /* XXXX should this return a 'AY_EPICK' ? */

      if(!(pe->coords = calloc(1, sizeof(double *))))
	return AY_EOMEM;

      if(!(pe->indices = calloc(1, sizeof(unsigned int))))
	return AY_EOMEM;

      pe->coords[0] = pecoord;
      pe->indices[0] = peindex;
      pe->num = 1;
      break;
    case 2:
      /* selection based on planes */
      control = sdcurve->controlv;
      j = 0;
      a = 0;

      for(i = 0; i < sdcurve->length; i++)
	{
	  c = &(control[j]);

	  /* test point c against the four planes in p */
	  if(((p[0]*c[0] + p[1]*c[1] + p[2]*c[2] + p[3]) < 0.0) &&
	     ((p[4]*c[0] + p[5]*c[1] + p[6]*c[2] + p[7]) < 0.0) &&
	     ((p[8]*c[0] + p[9]*c[1] + p[10]*c[2] + p[11]) < 0.0) &&
	     ((p[12]*c[0] + p[13]*c[1] + p[14]*c[2] + p[15]) < 0.0))
	    {
	      if(!(ctmp = realloc(pe->coords, (a+1)*sizeof(double *))))
		return AY_EOMEM;
	      pe->coords = ctmp;
	      if(!(itmp = realloc(pe->indices, (a+1)*sizeof(unsigned int))))
		  return AY_EOMEM;
	      pe->indices = itmp;

	      pe->coords[a] = &(control[j]);
	      pe->indices[a] = i;
	      a++;
	    } /* if */

	  j += 3;
	} /* for */

      pe->num = a;
      break;
    case 3:
      /* rebuild from o->selp */
      pnt = o->selp;
      lastpnt = &o->selp;
      while(pnt)
	{
	  if(pnt->index < (unsigned int)sdcurve->length)
	    {
	      pnt->point = &(sdcurve->controlv[pnt->index*3]);
	      lastpnt = &(pnt->next);
	      pnt = pnt->next;
	    }
	  else
	    {
	      *lastpnt = pnt->next;
	      free(pnt);
	      pnt = *lastpnt;
	    }
	} /* while */
      break;
    default:
      break;
    } /* switch */

 return AY_OK;
} /* sdcurve_getpntcb */


/* sdcurve_setpropcb:
 *  set property (from Tcl to C context) callback function of sdcurve object
 */
int
sdcurve_setpropcb(Tcl_Interp *interp, int argc, char *argv[], ay_object *o)
{
 int ay_status = AY_OK;
 char fname[] = "sdcurve_setpropcb";
 char *n1 = "SDCurveAttrData";
 Tcl_Obj *to = NULL, *toa = NULL, *ton = NULL;
 sdcurve_object *sdcurve = NULL;
 int update = AY_FALSE, new_closed, new_length, new_level, new_type;

  if(!interp || !o)
    return AY_ENULL;

  sdcurve = (sdcurve_object *)o->refine;

  if(!sdcurve)
    return AY_ENULL;

  toa = Tcl_NewStringObj(n1, -1);
  ton = Tcl_NewStringObj(n1, -1);

  Tcl_SetStringObj(ton, "Closed", -1);
  to = Tcl_ObjGetVar2(interp, toa, ton, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp, to, &new_closed);

  if(new_closed != sdcurve->closed)
    {
      update = AY_TRUE;
      sdcurve->closed = new_closed;
    }

  Tcl_SetStringObj(ton, "Length", -1);
  to = Tcl_ObjGetVar2(interp, toa, ton, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp, to, &new_length);

  if(new_length != sdcurve->length)
    {
      update = AY_TRUE;
    }

  Tcl_SetStringObj(ton, "Level", -1);
  to = Tcl_ObjGetVar2(interp, toa, ton, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp, to, &new_level);

  if(new_level != sdcurve->level)
    {
      update = AY_TRUE;
      sdcurve->level = new_level;
    }

  Tcl_SetStringObj(ton, "Type", -1);
  to = Tcl_ObjGetVar2(interp, toa, ton, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp, to, &new_type);

  if(new_type != sdcurve->type)
    {
      update = AY_TRUE;
      sdcurve->type = new_type;
    }

  /* resize curve */
  if(new_length != sdcurve->length && (new_length > 1))
    {
      ay_status = ay_npt_resizearrayw(&(sdcurve->controlv), 3, sdcurve->length,
				      1, new_length);
      if(ay_status)
	{
	  ay_error(AY_ERROR, fname, "Could not resize curve!");
	}
      else
	{
	  (void)sdcurve_getpntcb(3, o, NULL, NULL);
	  update = AY_TRUE;
	  sdcurve->length = new_length;
	}
    } /* if */

  if(update)
    {
      (void)ay_notify_object(o);

      o->modified = AY_TRUE;
      (void)ay_notify_parent();
    }

  Tcl_IncrRefCount(toa);Tcl_DecrRefCount(toa);
  Tcl_IncrRefCount(ton);Tcl_DecrRefCount(ton);

 return AY_OK;
} /* sdcurve_setpropcb */


/* sdcurve_getpropcb:
 *  get property (from C to Tcl context) callback function of sdcurve object
 */
int
sdcurve_getpropcb(Tcl_Interp *interp, int argc, char *argv[], ay_object *o)
{
 char *n1 = "SDCurveAttrData";
 Tcl_Obj *to = NULL, *toa = NULL, *ton = NULL;
 sdcurve_object *sdcurve = NULL;

  if(!interp || !o)
    return AY_ENULL;

  sdcurve = (sdcurve_object *)(o->refine);

  if(!sdcurve)
    return AY_ENULL;

  toa = Tcl_NewStringObj(n1, -1);
  ton = Tcl_NewStringObj(n1, -1);

  Tcl_SetStringObj(ton, "Closed", -1);
  to = Tcl_NewIntObj(sdcurve->closed);
  Tcl_ObjSetVar2(interp, toa, ton, to, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton, "Length", -1);
  to = Tcl_NewIntObj(sdcurve->length);
  Tcl_ObjSetVar2(interp, toa, ton, to, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton, "Level", -1);
  to = Tcl_NewIntObj(sdcurve->level);
  Tcl_ObjSetVar2(interp, toa, ton, to, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton, "Type", -1);
  to = Tcl_NewIntObj(sdcurve->type);
  Tcl_ObjSetVar2(interp, toa, ton, to, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton, "SLength", -1);
  if(sdcurve->scontrolv)
    to = Tcl_NewIntObj(sdcurve->scontrolvlen);
  else
    to = Tcl_NewIntObj(0);
  Tcl_ObjSetVar2(interp, toa, ton, to, TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);

  Tcl_IncrRefCount(toa);Tcl_DecrRefCount(toa);
  Tcl_IncrRefCount(ton);Tcl_DecrRefCount(ton);

 return AY_OK;
} /* sdcurve_getpropcb */


/* sdcurve_readcb:
 *  read (from scene file) callback function of sdcurve object
 */
int
sdcurve_readcb(FILE *fileptr, ay_object *o)
{
 sdcurve_object *sdcurve = NULL;
 int i, a;

  if(!fileptr || !o)
    return AY_ENULL;

  if(!(sdcurve = calloc(1, sizeof(sdcurve_object))))
    { return AY_EOMEM; }

  fscanf(fileptr, "%d\n", &sdcurve->closed);
  fscanf(fileptr, "%d\n", &sdcurve->length);
  fscanf(fileptr, "%d\n", &sdcurve->level);
  fscanf(fileptr, "%d\n", &sdcurve->type);

  if(!(sdcurve->controlv = calloc(sdcurve->length*3, sizeof(double))))
    {
      free(sdcurve);
      return AY_EOMEM;
    }

  a = 0;
  for(i = 0; i < sdcurve->length; i++)
    {
      fscanf(fileptr, "%lg %lg %lg\n", &(sdcurve->controlv[a]),
	     &(sdcurve->controlv[a+1]),
	     &(sdcurve->controlv[a+2]));
      a += 3;
    }

  o->refine = sdcurve;

 return AY_OK;
} /* sdcurve_readcb */


/* sdcurve_writecb:
 *  write (to scene file) callback function of sdcurve object
 */
int
sdcurve_writecb(FILE *fileptr, ay_object *o)
{
 sdcurve_object *sdcurve = NULL;
 int i, a;

  if(!fileptr || !o)
    return AY_ENULL;

  sdcurve = (sdcurve_object *)(o->refine);

  if(!sdcurve)
    return AY_ENULL;

  fprintf(fileptr, "%d\n", sdcurve->closed);
  fprintf(fileptr, "%d\n", sdcurve->length);
  fprintf(fileptr, "%d\n", sdcurve->level);
  fprintf(fileptr, "%d\n", sdcurve->type);

  a = 0;
  for(i = 0; i < sdcurve->length; i++)
    {
      fprintf(fileptr, "%g %g %g\n", sdcurve->controlv[a],
	      sdcurve->controlv[a+1],
	      sdcurve->controlv[a+2]);
      a += 3;
    }

 return AY_OK;
} /* sdcurve_writecb */


/* sdcurve_bbccb:
 *  bounding box calculation callback function of sdcurve object
 */
int
sdcurve_bbccb(ay_object *o, double *bbox, int *flags)
{
 sdcurve_object *sdcurve = NULL;

  if(!o || !bbox || !flags)
    return AY_ENULL;

  sdcurve = (sdcurve_object *)o->refine;

  if(!sdcurve)
    return AY_ENULL;

 return ay_bbc_fromarr(sdcurve->controlv, sdcurve->length, 3, bbox);
} /* sdcurve_bbccb */


/* sdcurve_notifycb:
 *  notification callback function of sdcurve object
 */
int
sdcurve_notifycb(ay_object *o)
{
 int ay_status = AY_OK;
 sdcurve_object *sdcurve = NULL;

  if(!o)
    return AY_ENULL;

  sdcurve = (sdcurve_object *)(o->refine);

  if(!sdcurve)
    return AY_ENULL;

  /* remove old subdivision */
  if(sdcurve->scontrolv)
    free(sdcurve->scontrolv);
  sdcurve->scontrolv = NULL;

  if(sdcurve->level > 0)
    {
      /* create new subdivision */
      if(!sdcurve->type)
	{
	  if(sdcurve->closed)
	    ay_status = sdcurve_chaikinc(sdcurve);
	  else
	    ay_status = sdcurve_chaikino(sdcurve);
	}
      else
	{
	  if(sdcurve->closed)
	    ay_status = sdcurve_cubicc(sdcurve);
	  else
	    ay_status = sdcurve_cubico(sdcurve);
	}
      if(ay_status)
	goto cleanup;
    } /* if level */

cleanup:

 return ay_status;
} /* sdcurve_notifycb */


/* sdcurve_convertcb:
 *  convert callback function of sdcurve object
 */
int
sdcurve_convertcb(ay_object *o, int in_place)
{
 int ay_status = AY_OK;
 sdcurve_object *sdcurve = NULL;
 ay_object *new = NULL;

  if(!o)
    return AY_ENULL;

  sdcurve = (sdcurve_object *) o->refine;

  if(!sdcurve)
    return AY_ENULL;

  ay_status = sdcurve_toncurve(sdcurve, &new);

  if(new)
    {
      ay_trafo_copy(o, new);

      if(!in_place)
	{
	  ay_object_link(new);
	}
      else
	{
	  ay_status = ay_object_replace(new, o);
	} /* if */
    } /* if */

 return ay_status;
} /* sdcurve_convertcb */


/* sdcurve_providecb:
 *  provide callback function of sdcurve object
 */
int
sdcurve_providecb(ay_object *o, unsigned int type, ay_object **result)
{
 int ay_status = AY_OK;
 sdcurve_object *sdcurve = NULL;

  if(!o)
    return AY_ENULL;

  if(!result)
    {
      if(type == AY_IDNCURVE)
	return AY_OK;
      else
	return AY_ERROR;
    }

  sdcurve = (sdcurve_object *) o->refine;

  if(!sdcurve)
    return AY_ENULL;

  if(type == AY_IDNCURVE)
    {
      ay_status = sdcurve_toncurve(sdcurve, result);

      if(*result)
	{
	  ay_trafo_copy(o, *result);
	}
    } /* if */

 return ay_status;
} /* sdcurve_providecb */


/** sdcurve_insertpntcb:
 * Insert a new control point into a SDCurve.
 *
 * \param[in,out] o curve object to process
 * \param[in,out] index where to store the index of the inserted point
 * \param[in] objXYZ coordinates of point after which to insert
 * \param[in] edit interactive editing support
 *
 * \returns AY_OK on success, error code otherwise.
 */
int
sdcurve_insertpntcb(ay_object *o, int *index, double *objXYZ, int edit)
{
 int ay_status = AY_OK;
 ay_acurve_object ac = {0};
 sdcurve_object *sd = NULL;

  if(!o || !index || !objXYZ)
    return AY_ENULL;

  if(o->type != sdcurve_id)
    return AY_ERROR;

  sd = (sdcurve_object *)o->refine;
  ac.length = sd->length;
  ac.closed = sd->closed;
  ac.controlv = sd->controlv;

  ay_status = ay_pact_insertac(&ac, index, objXYZ, edit);

  if(!ay_status)
    {
      sd->length = ac.length;
      sd->controlv = ac.controlv;
    }

 return ay_status;
} /* sdcurve_insertpntcb */


/** sdcurve_deletepntcb:
 * Remove a control point from a SDCurve.
 *
 * \param[in,out] o curve object to process
 * \param[in,out] index where to store the index of the removed point
 * \param[in] objXYZ coordinates of point to delete
 *
 * \returns AY_OK on success, error code otherwise.
 */
int
sdcurve_deletepntcb(ay_object *o, int *index, double *objXYZ)
{
 int ay_status = AY_OK;
 ay_acurve_object ac = {0};
 sdcurve_object *sd = NULL;

  if(!o || !index || !objXYZ)
    return AY_ENULL;

  if(o->type != sdcurve_id)
    return AY_ERROR;

  sd = (sdcurve_object *)o->refine;
  ac.length = sd->length;
  ac.closed = sd->closed;
  ac.controlv = sd->controlv;

  ay_status = ay_pact_deleteac(&ac, index, objXYZ);

  if(!ay_status)
    {
      sd->length = ac.length;
      sd->controlv = ac.controlv;
    }

 return ay_status;
} /* sdcurve_deletepntcb */


/** sdcurve_genericopcb:
 * Execute generic operation (AY_OP*) on SDCurve.
 *
 * \param[in,out] o curve object to process
 * \param[in] op operation designation
 *
 * \returns AY_OK on success, error code otherwise.
 */
int
sdcurve_genericopcb(ay_object *o, int op)
{
 sdcurve_object *sd = NULL;

  if(!o)
    return AY_ENULL;

  if(o->type != sdcurve_id)
    return AY_ERROR;

  sd = (sdcurve_object *)o->refine;

  switch(op)
    {
    case AY_OPREVERT:
      (void) ay_nct_revertarr(sd->controlv, sd->length, 3);
      break;
    case AY_OPOPEN:
      sd->closed = AY_FALSE;
      break;
    case AY_OPCLOSE:
      sd->closed = AY_TRUE;
      break;
    default:
      break;
    }

 return AY_OK;
} /* sdcurve_genericopcb */


/** sdcurve_convtcmd:
 *  Tcl command to convert curve objects to SDCurve objects
 *  Implements the \a sdcconvertC scripting interface command.
 *  See also the corresponding section in the \ayd{scsdcconvc}.
 *
 *  \returns TCL_OK in any case.
 */
int
sdcurve_convtcmd(ClientData clientData, Tcl_Interp *interp,
		 int argc, char *argv[])
{
 int ay_status = AY_OK;
 ay_list_object *sel = ay_selection;
 ay_object *o = NULL, *newo = NULL;
 sdcurve_object *sdcurve = NULL;
 ay_nurbcurve_object *nc;
 ay_icurve_object *ic;
 ay_acurve_object *ac;
 ay_pointedit pe = {0};
 double *cv = NULL, *newcv = NULL, p[4], *pnt;
 double m[16];
 int applytrafo = AY_FALSE, closed = AY_FALSE, a, b, i, cvlen, stride = 3;

  /* parse args */
  if(argc > 2)
    {
      i = 1;
      while(i+1 < argc)
	{
	  if(!strcmp(argv[i], "-a"))
	    {
	      sscanf(argv[i+1], "%d", &applytrafo);
	    }
	  if(!strcmp(argv[i], "-r"))
	    {
	      /*
	      mode = 0;
	      sscanf(argv[i+1], "%lg", &rmin);
	      sscanf(argv[i+2], "%lg", &rmax);
	      */
	    }
	  if(!strcmp(argv[i], "-d"))
	    {
	      /*
	      mode = 1;
	      sscanf(argv[i+1], "%lg", &mindist);
	      */
	    }
	  i += 2;
	} /* while */
    } /* if */

  /* check selection */
  if(!sel)
    {
      ay_error(AY_ENOSEL, argv[0], NULL);
      return TCL_OK;
    }

  while(sel)
    {
      o = sel->object;

      switch(o->type)
	{
	case AY_IDNCURVE:
	  nc = (ay_nurbcurve_object*)o->refine;
	  cv = nc->controlv;
	  cvlen = nc->length;
	  if(nc->type == AY_CTCLOSED)
	    {
	      closed = AY_TRUE;
	      cvlen--;
	    }
	  if(nc->type == AY_CTPERIODIC)
	    {
	      closed = AY_TRUE;
	      cvlen -= (nc->order-1);
	    }
	  stride = 4;
	  break;
	case AY_IDICURVE:
	  ic = (ay_icurve_object*)o->refine;
	  cv = ic->controlv;
	  cvlen = ic->length;
	  if(ic->type == AY_CTCLOSED)
	    {
	      closed = AY_TRUE;
	    }
	  break;
	case AY_IDACURVE:
	  ac = (ay_acurve_object*)o->refine;
	  cv = ac->controlv;
	  cvlen = ac->length;
	  closed = ac->closed;
	  break;
	default:
	  break;
	} /* switch */

      if(cv)
	{
	  if(!(newcv = malloc(3 * cvlen * sizeof(double))))
	    {
	      ay_error(AY_EOMEM, argv[0], NULL);
	      return TCL_OK;
	    }

	  if(stride == 3)
	    {
	      memcpy(newcv, cv, 3*cvlen*sizeof(double));
	    }
	  else
	    {
	      a = 0;
	      b = 0;
	      for(i = 0; i < cvlen; i++)
		{
		  memcpy(&(newcv[a]), &(cv[b]), 3*sizeof(double));
		  a += 3;
		  b += stride;
		}
	    }
	}
      else
	{
	  ay_status = ay_pact_getpoint(0, o, p, &pe);
	  if(ay_status)
	    return TCL_OK;
	  cvlen = pe.num;
	  if(!(newcv = malloc(3 * cvlen * sizeof(double))))
	    {
	      ay_error(AY_EOMEM, argv[0], NULL);
	      return TCL_OK;
	    }
	  pnt = newcv;
	  for(i = 0; i < cvlen; i++)
	    {
	      memcpy(pnt, pe.coords[i], 3*sizeof(double));
	      pnt += 3;
	    }
	  ay_pact_clearpointedit(&pe);
	} /* if cv */

      if(newcv)
	{
	  if(!(newo = calloc(1, sizeof(ay_object))))
	    {
	      free(newcv);
	      ay_error(AY_EOMEM, argv[0], NULL);
	      return TCL_OK;
	    }

	  ay_object_defaults(newo);
	  newo->type = sdcurve_id;

	  if(!(sdcurve = calloc(1, sizeof(sdcurve_object))))
	    {
	      free(newcv);
	      free(newo);
	      ay_error(AY_EOMEM, argv[0], NULL);
	      return TCL_OK;
	    }

	  sdcurve->length = cvlen;
	  sdcurve->controlv = newcv;
	  sdcurve->closed = closed;

	  newo->refine = sdcurve;

	  if(applytrafo)
	    {
	      if(AY_ISTRAFO(o))
		{
		  ay_trafo_creatematrix(o, m);
		  pnt = sdcurve->controlv;
		  for(i = 0; i < sdcurve->length; i++)
		    {
		      ay_trafo_apply3(pnt, m);
		      pnt += 3;
		    } /* for */
		}
	    }
	  else
	    {
	      ay_trafo_copy(o, newo);
	    }

	  ay_object_link(newo);
	} /* if newcv */

      sel = sel->next;
    } /* while */

  ay_notify_parent();

 return TCL_OK;
} /* sdcurve_convtcmd */


/* Sdcurve_Init:
 * initializes the sdcurve module/plugin by registering a new
 * object type (sdcurve) and loading the accompanying Tcl script file.
 */
#ifdef WIN32
  __declspec (dllexport)
#endif /* WIN32 */
int
Sdcurve_Init(Tcl_Interp *interp)
{
 int ay_status = AY_OK;
 char fname[] = "sdcurve_init";
 int i, ops[3] = {AY_OPREVERT, AY_OPOPEN, AY_OPCLOSE};

#ifdef WIN32
  if(Tcl_InitStubs(interp, "8.2", 0) == NULL)
    {
      return TCL_ERROR;
    }
#endif /* WIN32 */

  if(ay_checkversion(fname, sdcurve_version_ma, sdcurve_version_mi))
    {
      return TCL_ERROR;
    }

  ay_status = ay_otype_register(sdcurve_name,
				sdcurve_createcb,
				sdcurve_deletecb,
				sdcurve_copycb,
				sdcurve_drawcb,
				sdcurve_drawhcb,
				NULL, /* no surface */
				sdcurve_setpropcb,
				sdcurve_getpropcb,
				sdcurve_getpntcb,
				sdcurve_readcb,
				sdcurve_writecb,
				NULL, /* no RIB export */
				sdcurve_bbccb,
				&sdcurve_id);

  ay_status += ay_draw_registerdacb(sdcurve_drawacb, sdcurve_id);

  ay_status += ay_notify_register(sdcurve_notifycb, sdcurve_id);

  ay_status += ay_convert_register(sdcurve_convertcb, sdcurve_id);

  ay_status += ay_provide_register(sdcurve_providecb, sdcurve_id);

  ay_status += ay_pact_registerinsert(sdcurve_insertpntcb, sdcurve_id);

  ay_status += ay_pact_registerdelete(sdcurve_deletepntcb, sdcurve_id);

  for(i = 0; i < 3; i++)
    {
      ay_status += ay_tcmd_registergeneric(ops[i], sdcurve_genericopcb,
					   sdcurve_id);
    }

  if(ay_status)
    {
      ay_error(AY_ERROR, fname, "Error registering custom object!");
      return TCL_OK;
    }

  /* sdcurve objects may not be associated with materials */
  ay_matt_nomaterial(sdcurve_id);

  Tcl_CreateCommand(interp, "sdcconvertC",
		    (Tcl_CmdProc*) sdcurve_convtcmd,
		    (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  /* source sdcurve.tcl, it contains Tcl-code to build
     the SDCurve-Attributes Property GUI */
  if((Tcl_EvalFile(interp, "sdcurve.tcl")) != TCL_OK)
     {
       ay_error(AY_ERROR, fname,
		  "Error while sourcing \"sdcurve.tcl\"!");
       return TCL_OK;
     }

  ay_error(AY_EOUTPUT, fname,
	   "Custom object \"SDCurve\" successfully loaded.");

 return TCL_OK;
} /* Sdcurve_Init */
