/*
 * Ayam, a free 3D modeler for the RenderMan interface.
 *
 * Ayam is copyrighted 1998-2017 by Randolf Schultz
 * (randolf.schultz@gmail.com) and others.
 *
 * All rights reserved.
 *
 * See the file License for details.
 *
 */

#include "ayam.h"

/* bcurve.c - basis curve custom object (bicubic patchmesh curve equivalent) */

char bcurve_version_ma[] = AY_VERSIONSTR;
char bcurve_version_mi[] = AY_VERSIONSTRMI;

static char *bcurve_name = "BCurve";

static unsigned int bcurve_id;

typedef struct bcurve_object_s
{
  int is_rat;
  int closed; /**< is the curve closed? */
  int length; /**< number of control points (> 4) */
  int btype; /* basis type (AY_BT*) */
  int step; /**< step size of basis */
  double *basis; /**< basis matrix (if btype is AY_BTCUSTOM) */

  double *controlv; /**< control points [length*4] */

  ay_object *ncurve; /**< cached NURBS curve */

  int display_mode; /**< drawing mode */
  double glu_sampling_tolerance; /**< drawing quality */
} bcurve_object;

#ifdef WIN32
  __declspec (dllexport)
#endif /* WIN32 */
int bcurve_Init(Tcl_Interp *interp);

/* prototypes of functions local to this module: */
int bcurve_tobasis(bcurve_object *bc, int btype, int bstep, double *basis);

int bcurve_toncurvemulti(ay_object *o, ay_object **result);

int bcurve_toncurve(ay_object *o, int btype, ay_object **result);


/* Bezier */
static double mb[16] = {-1, 3, -3, 1,  3, -6, 3, 0,  -3, 3, 0, 0,  1, 0, 0, 0};
/* Hermite (RenderMan flavour) */
static double mh[16] = {2, -3, 0, 1,  1, -2, 1, 0,  -2, 3, 0, 0,  1, -1, 0, 0};
/* Catmull Rom */
static double mc[16] = {-0.5, 1, -0.5, 0,  1.5, -2.5, 0, 1,  -1.5, 2, 0.5, 0,
			0.5, -0.5, 0, 0};
/* B-Spline */
static double ms[16] = {-1.0/6, 3.0/6, -3.0/6, 1.0/6,  3.0/6, -1, 0, 4.0/6,
			-3.0/6, 3.0/6, 3.0/6, 1.0/6,  1.0/6, 0, 0, 0};
/* Power */
static double mp[16] = {1, 0, 0, 0,  0, 1, 0, 0,  0, 0, 1, 0,  0, 0, 0, 1};


static double mbi[16];

static double msi[16];



/** bcurve_tobasis:
 * Convert BCurve to a different matrix.
 *
 * \param[in,out] bc BCurve to process (must be open and of length 4)
 * \param[in] btype target basis type
 * \param[in] bstep target step size
 * \param[in] basis target basis (may be NULL unless \a btype is AY_BTCUSTOM)
 *
 * \returns AY_OK on success, error code otherwise.
 */
int
bcurve_tobasis(bcurve_object *bc, int btype, int bstep, double *basis)
{
 int convert = AY_FALSE, i, j, k, l, i1, i2;
 double *p1, *p2, *p3, *p4;
 double mi[16], mu[16], mut[16];
 double v[4];

  if(!bc)
    return AY_ENULL;

  if((btype == AY_BTCUSTOM) && !basis)
    return AY_ERROR;

  if(bc->length != 4 || bc->closed)
    return AY_ERROR;

  if(bc->btype != btype)
    {
      convert = AY_TRUE;
    }
  else
    {
      if(bc->btype == AY_BTCUSTOM)
	{
	  if(memcmp(bc->basis, basis, 16*sizeof(double)))
	    convert = AY_TRUE;
	}
    }

  if(!convert)
    return AY_OK;

  /* create conversion matrices */
  switch(btype)
    {
    case AY_BTBSPLINE:
      memcpy(mu, msi, 16*sizeof(double));
      break;
    case AY_BTBEZIER:
      memcpy(mu, mbi, 16*sizeof(double));
      break;
    case AY_BTCATMULLROM:
      if(ay_trafo_invgenmatrix(mc, mi))
	{
	  return AY_ERROR;
	}
      memcpy(mu, mi, 16*sizeof(double));
      break;
    case AY_BTHERMITE:
      if(ay_trafo_invgenmatrix(mh, mi))
	{
	  return AY_ERROR;
	}
      memcpy(mu, mi, 16*sizeof(double));
      break;
    case AY_BTPOWER:
      ay_trafo_identitymatrix(mu);
      break;
    default:
      if(ay_trafo_invgenmatrix(basis, mi))
	{
	  return AY_ERROR;
	}
      memcpy(mu, mi, 16*sizeof(double));
      break;
    }

  switch(bc->btype)
    {
    case AY_BTBEZIER:
      ay_trafo_multmatrix(mu, mb);
      break;
    case AY_BTBSPLINE:
      ay_trafo_multmatrix(mu, ms);
      break;
    case AY_BTHERMITE:
      ay_trafo_multmatrix(mu, mh);
      break;
    case AY_BTCATMULLROM:
      ay_trafo_multmatrix(mu, mc);
      break;
    case AY_BTPOWER:
      /* use inv(basis) unchanged */
      break;
    case AY_BTCUSTOM:
      ay_trafo_multmatrix(mu, bc->basis);
      break;
    default:
      return AY_ERROR;
    }

  /* transpose conversion matrix */
  i1 = 0;
  for(i = 0; i < 4; i++)
    {
      i2 = i;
      for(j = 0; j < 4; j++)
	{
	  mut[i2] = mu[i1];

	  i1++;
	  i2 += 4;
	} /* for */
    } /* for */

  p1 = bc->controlv;
  p2 = p1 + 4;
  p3 = p2 + 4;
  p4 = p3 + 4;

  /* for each control point component (x,y,z) */
  for(k = 0; k < 3; k++)
    {
      /* get coordinates into a vector */
      for(l = 0; l < 4; l++)
	{
	  v[0] = *(p1+k);
	  v[1] = *(p2+k);
	  v[2] = *(p3+k);
	  v[3] = *(p4+k);
	}

      /* apply conversion matrix */
      ay_trafo_multvectmatrix(v, mut);

      /* copy converted coordinates back */
      for(l = 0; l < 4; l++)
	{
	  *(p1+k) = v[0];
	  *(p2+k) = v[1];
	  *(p3+k) = v[2];
	  *(p4+k) = v[3];
	}
    } /* for each component */

  if(btype == AY_BTCUSTOM)
    {
      /* copy custom basis */
      if(bc->btype != AY_BTCUSTOM)
	{
	  if(!(bc->basis = malloc(16*sizeof(double))))
	     return AY_EOMEM;
	}
      memcpy(bc->basis, basis, 16*sizeof(double));
    }

  bc->btype = btype;
  bc->step = bstep;

 return AY_OK;
} /* bcurve_tobasis */


/* bcurve_toncurvemulti:
 * helper for bcurve_toncurve() below;
 * converts a complex bcurve by moving an evaluation window
 * over the curve (according to the parameters defined by the basis type
 * and step size), converting the respective window to a single NURBS
 * curve, then concatenating the resulting curves to the final resulting
 * NURBS curve
 */
int
bcurve_toncurvemulti(ay_object *o, ay_object **result)
{
 int ay_status = AY_OK;
 double tcv[4*4], *cv = NULL;
 int l, s;
 int i, ii, a, b;
 ay_object t = {0}, *curves = NULL, **nextcurve;
 bcurve_object *bc = NULL, tbc = {0};

  if(!o || !result)
    return AY_ENULL;

  bc = (bcurve_object*)o->refine;
  cv = bc->controlv;

  /* set up temporary bcurve (t and tbc) */
  ay_object_defaults(&t);
  t.type = bcurve_id;
  t.refine = &tbc;
  tbc.length = 4;
  tbc.btype = bc->btype;
  tbc.basis = bc->basis;

  /* due to hard-coded length 4, no need to copy the steps */
  /*
  tbc.step = bc->step;
  */
  tbc.controlv = tcv;

  switch(bc->btype)
    {
    case AY_BTPOWER:
      s = 4;
      break;
    case AY_BTBEZIER:
      s = 3;
      break;
    case AY_BTHERMITE:
      s = 2;
      break;
    case AY_BTCATMULLROM:
    case AY_BTBSPLINE:
      s = 1;
      break;
    default:
      s = bc->step;
      break;
    }

  l = (bc->length-3)/s+1;

  if(bc->closed)
    {
      while(((l+1)*s) < bc->length)
	l++;
    }
  else
    {
      if(s == 1)
	l--;
    }

  nextcurve = &curves;

  for(i = 0; i < l; i++)
    {
      /* get eval window (fill tcv) */
      a = 0;
      b = s*i*4;
      for(ii = 0; ii < 4; ii++)
	{
	  /* wraparound? */
	  if(b > (bc->length-1))
	    b -= (bc->length*4);

	  memcpy(&(tcv[a]), &(cv[b]), 4*sizeof(double));
	  b += 4;
	  a += 4;
	}

      /* convert to NCurve */
      ay_status = bcurve_toncurve(&t, AY_BTBEZIER, nextcurve);
      if(!ay_status && *nextcurve)
	nextcurve = &((*nextcurve)->next);
      else
	goto cleanup;

    } /* for all eval windows */

  /* concatenate all curves? */
  if(curves && curves->next)
    {
      ay_nct_concatmultiple(/*closed=*/0, /*knot_type=*/1,
		    /*fillgaps=*/AY_FALSE, curves,
		    result);
    }
  else
    {
      *result = curves;
      curves = NULL;
    }

cleanup:

  if(curves)
    (void)ay_object_deletemulti(curves, AY_FALSE);

 return ay_status;
} /* bcurve_toncurvemulti */


/** bcurve_toncurve:
 * Create a NURBS curve from a BCurve.
 * This function will not work properly or crash if the PatchMesh
 * fails the validity check using ay_pmt_valid() above!
 * This function only handles curves of basis type AY_BTBSPLINE and
 * open curves of arbitrary basis type and length 4.
 * Other, "complex", curves are handled via bcurve_toncurvemulti()
 * above.
 *
 * \param[in] o the curve to convert
 * \param[in] btype desired basis type, must be AY_BTBSPLINE;
 *  the other valid type (AY_BTBEZIER) is used exclusively by
 *  bcurve_toncurvemulti() when calling back
 * \param[in,out] result pointer where to store the resulting NCurve
 *
 * \returns AY_OK on success, error code otherwise.
 */
int
bcurve_toncurve(ay_object *o, int btype, ay_object **result)
{
 int ay_status = AY_OK;
 double *cv = NULL;
 int l, kt;
 int i = 0, a, b;
 ay_object *newo = NULL, *c = NULL;
 bcurve_object *bcurve;

  if(!o || !result)
    return AY_ENULL;

  bcurve = (bcurve_object*) o->refine;

  l = bcurve->length;

  if((bcurve->btype != AY_BTBSPLINE) && (bcurve->length > 4 || bcurve->closed))
    return bcurve_toncurvemulti(o, result);

  if(bcurve->btype != btype)
    {
      (void)ay_object_copy(o, &c);
      if(c)
	{
	  ay_status = bcurve_tobasis(c->refine, btype, /*bstep=*/1,
				     /*basis=*/NULL);
	  if(ay_status)
	    goto cleanup;
	}
      else
	return AY_ERROR;

      bcurve = c->refine;
    }
  else
    {
      if(bcurve->closed)
	l += 3;
    }

  if(btype == AY_BTBSPLINE)
    {
      kt = AY_KTBSPLINE;
    }
  else
    {
      kt = AY_KTBEZIER;
    }

  if(!(newo = calloc(1, sizeof(ay_object))))
    {
      ay_status = AY_EOMEM;
      goto cleanup;
    }

  ay_object_defaults(newo);
  newo->type = AY_IDNCURVE;

  if(!(cv = malloc(l*4*sizeof(double))))
    {
      ay_status = AY_EOMEM;
      goto cleanup;
    }

  /* fill cv */
  a = 0;
  b = 0;
  for(i = 0; i < bcurve->length; i++)
    {
      memcpy(&(cv[a]), &(bcurve->controlv[b]), 4*sizeof(double));
      a += 4;
      b += 4;
    }
  b = 0;
  if(l > bcurve->length)
    {
      for(i = 0; i < (l-bcurve->length); i++)
	{
	  memcpy(&(cv[a]), &(bcurve->controlv[b]), 4*sizeof(double));
	  a += 4;
	  b += 4;
	}
    }

  ay_status = ay_nct_create(4, l, kt, cv, NULL,
			    (ay_nurbcurve_object **)(void*)&(newo->refine));

  if(ay_status)
    {
      goto cleanup;
    }

  /* return result */
  *result = newo;

  /* prevent cleanup code from doing something harmful */
  cv = NULL;
  newo = NULL;

cleanup:

  if(c)
    ay_object_delete(c);
  if(cv)
    free(cv);
  if(newo)
    free(newo);

 return ay_status;
} /* bcurve_toncurve */


/*
 ****
 ****
 */


/* bcurve_createcb:
 *  create callback function of bcurve object
 */
int
bcurve_createcb(int argc, char *argv[], ay_object *o)
{
 bcurve_object *bcurve = NULL;
 char fname[] = "crtbcurve";
 int a, i;

  if(!o)
    return AY_ENULL;

  if(!(bcurve = calloc(1, sizeof(bcurve_object))))
    {
      ay_error(AY_EOMEM, fname, NULL);
      return AY_ERROR;
    }

  bcurve->length = 4;
  bcurve->btype = AY_BTBEZIER;

  if(!(bcurve->controlv = malloc(4 * bcurve->length * sizeof(double))))
    {
      free(bcurve);
      return AY_EOMEM;
    }

  a = 0;
  for(i = 0; i < bcurve->length; i++)
    {
      bcurve->controlv[a] = i*0.25;
      bcurve->controlv[a+1] = 0;
      bcurve->controlv[a+2] = 0;
      bcurve->controlv[a+3] = 1.0;
      a += 4;
    }

  o->refine = bcurve;

  ay_notify_object(o);

 return AY_OK;
} /* bcurve_createcb */


/* bcurve_deletecb:
 *  delete callback function of bcurve object
 */
int
bcurve_deletecb(void *c)
{
 bcurve_object *bcurve = NULL;

  if(!c)
    return AY_ENULL;

  bcurve = (bcurve_object *)(c);

  if(bcurve->basis)
    {
      free(bcurve->basis);
      bcurve->basis = NULL;
    }

  /* free cached ncurve */
  ay_object_delete(bcurve->ncurve);

  free(bcurve);

 return AY_OK;
} /* bcurve_deletecb */


/* bcurve_copycb:
 *  copy callback function of bcurve object
 */
int
bcurve_copycb(void *src, void **dst)
{
  bcurve_object *bcurve = NULL, *bcurvesrc;

  if(!src || !dst)
    return AY_ENULL;

  bcurvesrc = (bcurve_object *)src;

  if(!(bcurve = calloc(1, sizeof(bcurve_object))))
    return AY_EOMEM;

  memcpy(bcurve, src, sizeof(bcurve_object));

  bcurve->controlv = NULL;
  bcurve->basis = NULL;

  bcurve->ncurve = NULL;

  /* copy controlv */
  if(!(bcurve->controlv = malloc(4 * bcurve->length * sizeof(double))))
    {
      free(bcurve);
      return AY_EOMEM;
    }
  memcpy(bcurve->controlv, bcurvesrc->controlv,
	 4 * bcurve->length * sizeof(double));

  /* copy basis */
  if(bcurvesrc->basis)
    {
      if(!(bcurve->basis = malloc(16 * sizeof(double))))
	{
	  free(bcurve->controlv);
	  free(bcurve);
	  return AY_EOMEM;
	}
      memcpy(bcurve->basis, bcurvesrc->basis, 16 * sizeof(double));
    }

  *dst = (void *)bcurve;

 return AY_OK;
} /* bcurve_copycb */


/* bcurve_drawcb:
 *  draw (display in an Ayam view window) callback function of bcurve object
 */
int
bcurve_drawcb(struct Togl *togl, ay_object *o)
{
 bcurve_object *bcurve = NULL;

  if(!o)
    return AY_ENULL;

  bcurve = (bcurve_object *)o->refine;

  if(!bcurve)
    return AY_ENULL;

  if(bcurve->ncurve)
    {
      ay_draw_object(togl, bcurve->ncurve, AY_TRUE);
    }

 return AY_OK;
} /* bcurve_drawcb */


/* bcurve_drawhcb:
 *  draw handles callback function of bcurve object
 */
int
bcurve_drawhcb(struct Togl *togl, ay_object *o)
{
 bcurve_object *bcurve = NULL;
 double w, *cv = NULL;
 int i;

  if(!o)
    return AY_ENULL;

  bcurve = (bcurve_object *)o->refine;

  if(!bcurve)
    return AY_ENULL;

  cv = bcurve->controlv;

  /* draw normal points */
  glBegin(GL_POINTS);
   if(bcurve->is_rat && ay_prefs.rationalpoints)
     {
       for(i = 0; i < bcurve->length; i++)
	 {
	   w = cv[3];
	   glVertex3d((GLdouble)(cv[0]*w),
		      (GLdouble)(cv[1]*w),
		      (GLdouble)(cv[2]*w));
	   cv += 4;
	 }
     }
   else
     {
       for(i = 0; i < bcurve->length; i++)
	 {
	   glVertex3dv((GLdouble *)cv);
	   cv += 4;
	 }
     }
  glEnd();

 return AY_OK;
} /* bcurve_drawhcb */


/* bcurve_getpntcb:
 *  get point (editing and selection) callback function of bcurve object
 */
int
bcurve_getpntcb(int mode, ay_object *o, double *p, ay_pointedit *pe)
{
 bcurve_object *bcurve = NULL;
 ay_point *pnt = NULL, **lastpnt = NULL;
 double min_dist = ay_prefs.pick_epsilon, dist = 0.0;
 double *pecoord = NULL, **ctmp;
 double *control = NULL, *c, h[3];
 int i = 0, j = 0, a = 0;
 unsigned int *itmp, peindex = 0;

  if(!o || ((mode != 3) && (!p || !pe)))
    return AY_ENULL;

  bcurve = (bcurve_object *)(o->refine);

  if(!bcurve)
    return AY_ENULL;

  if(min_dist == 0.0)
    min_dist = DBL_MAX;

  if(pe)
    pe->rational = AY_TRUE;

  switch(mode)
    {
    case 0:
      /* select all points */
      if(!(pe->coords = malloc(bcurve->length * sizeof(double*))))
	return AY_EOMEM;
      if(!(pe->indices = malloc(bcurve->length * sizeof(unsigned int))))
	return AY_EOMEM;

      for(i = 0; i < bcurve->length; i++)
	{
	  pe->coords[i] = &(bcurve->controlv[a]);
	  pe->indices[i] = i;
	  a += 4;
	}

      pe->num = bcurve->length;
      break;
    case 1:
      /* selection based on a single point */
      control = bcurve->controlv;
      for(i = 0; i < bcurve->length; i++)
	{
	  if(bcurve->is_rat && ay_prefs.rationalpoints)
	    {
	      dist = AY_VLEN((p[0] - (control[j]*control[j+3])),
			     (p[1] - (control[j+1]*control[j+3])),
			     (p[2] - (control[j+2]*control[j+3])));
	    }
	  else
	    {
	      dist = AY_VLEN((p[0] - control[j]),
			     (p[1] - control[j+1]),
			     (p[2] - control[j+2]));
	    }

	  if(dist < min_dist)
	    {
	      pecoord = &(control[j]);
	      peindex = i;
	      min_dist = dist;
	    }

	  j += 4;
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
      control = bcurve->controlv;
      j = 0;
      a = 0;
      if(bcurve->is_rat && ay_prefs.rationalpoints)
	{
	  c = h;
	}
      for(i = 0; i < bcurve->length; i++)
	{
	  if(bcurve->is_rat && ay_prefs.rationalpoints)
	    {
	      h[0] = control[j]*control[j+3];
	      h[1] = control[j+1]*control[j+3];
	      h[2] = control[j+2]*control[j+3];
	    }
	  else
	    {
	      c = &(control[j]);
	    }
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

	  j += 4;
	} /* for */

      pe->num = a;
      break;
    case 3:
      /* rebuild from o->selp */
      pnt = o->selp;
      lastpnt = &o->selp;
      while(pnt)
	{
	  if(pnt->index < (unsigned int)bcurve->length)
	    {
	      pnt->point = &(bcurve->controlv[pnt->index*4]);
	      pnt->rational = AY_TRUE;
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
} /* bcurve_getpntcb */


/* bcurve_setpropcb:
 *  set property (from Tcl to C context) callback function of bcurve object
 */
int
bcurve_setpropcb(Tcl_Interp *interp, int argc, char *argv[], ay_object *o)
{
 int ay_status = AY_OK;
 char fname[] = "bcurve_setpropcb";
 char *n1 = "BCurveAttrData";
 Tcl_Obj *to = NULL, *toa = NULL, *ton = NULL;
 bcurve_object *bcurve = NULL;
 double dtemp, *basis;
 int j, update = AY_FALSE, new_length, new_btype, new_step;
 char *man[] = {"_0","_1","_2","_3","_4","_5","_6","_7","_8","_9","_10","_11","_12","_13","_14","_15"};

  if(!interp || !o)
    return AY_ENULL;

  bcurve = (bcurve_object *)o->refine;

  if(!bcurve)
    return AY_ENULL;

  toa = Tcl_NewStringObj(n1,-1);
  ton = Tcl_NewStringObj(n1,-1);

  Tcl_SetStringObj(ton, "Closed", -1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp, to, &bcurve->closed);

  Tcl_SetStringObj(ton, "Length", -1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp, to, &new_length);

  if(new_length != bcurve->length)
    {
      update = AY_TRUE;
    }

  Tcl_SetStringObj(ton, "BType", -1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp, to, &new_btype);

  if(new_btype == AY_BTCUSTOM)
    {
      update = AY_TRUE;
      if(!bcurve->basis)
	{
	  if(!(bcurve->basis = calloc(16, sizeof(double))))
	    {
	      ay_error(AY_EOMEM, fname, NULL);
	      goto cleanup;
	    } /* if */
	} /* if */

      if(bcurve->btype == AY_BTCUSTOM)
	{
	  for(j = 0; j < 16; j++)
	    {
	      Tcl_SetStringObj(ton, "Basis", -1);
	      Tcl_AppendStringsToObj(ton, man[j], NULL);
	      to  = Tcl_ObjGetVar2(interp, toa, ton, TCL_LEAVE_ERR_MSG |
				   TCL_GLOBAL_ONLY);
	      Tcl_GetDoubleFromObj(interp, to, &dtemp);
	      bcurve->basis[j] = dtemp;
	    } /* for */
	}
      else
	{
	  /* switching from another basis to custom */
	  basis = NULL;
	  ay_pmt_getbasis(bcurve->btype, &basis);
	  if(basis)
	    {
	      memcpy(bcurve->basis, basis, 16*sizeof(double));
	    }
	}

      Tcl_SetStringObj(ton, "Step", -1);
      to = Tcl_ObjGetVar2(interp, toa, ton, TCL_LEAVE_ERR_MSG |
			  TCL_GLOBAL_ONLY);
      Tcl_GetIntFromObj(interp, to, &(new_step));
      if(new_step <= 0 || new_step > 4)
	bcurve->step = 1;
      else
	bcurve->step = new_step;
    }
  else
    {
      if(bcurve->basis)
	free(bcurve->basis);
      bcurve->basis = NULL;
    } /* if */

  /* resize patch */
  if(new_length != bcurve->length && (new_length > 1))
    {
      if(o->selp)
	{
	  ay_selp_clear(o);
	}

      ay_status = ay_npt_resizearrayw(&(bcurve->controlv), 4, bcurve->length,
				      1, new_length);

      if(ay_status)
	ay_error(AY_ERROR, fname, "Could not resize curve!");
      else
	bcurve->length = new_length;
    } /* if */

  bcurve->btype = new_btype;

  Tcl_SetStringObj(ton, "Tolerance", -1);
  to = Tcl_ObjGetVar2(interp, toa, ton, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetDoubleFromObj(interp, to, &(bcurve->glu_sampling_tolerance));

  Tcl_SetStringObj(ton, "DisplayMode", -1);
  to = Tcl_ObjGetVar2(interp, toa, ton, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp, to, &(bcurve->display_mode));

  if(update)
    {
      (void)ay_notify_object(o);

      o->modified = AY_TRUE;
      (void)ay_notify_parent();
    }

cleanup:
  Tcl_IncrRefCount(toa);Tcl_DecrRefCount(toa);
  Tcl_IncrRefCount(ton);Tcl_DecrRefCount(ton);

 return AY_OK;
} /* bcurve_setpropcb */


/* bcurve_getpropcb:
 *  get property (from C to Tcl context) callback function of bcurve object
 */
int
bcurve_getpropcb(Tcl_Interp *interp, int argc, char *argv[], ay_object *o)
{
 char *n1="BCurveAttrData";
 Tcl_Obj *to = NULL, *toa = NULL, *ton = NULL;
 bcurve_object *bcurve = NULL;
 int j;
 char *man[] = {"_0","_1","_2","_3","_4","_5","_6","_7","_8","_9","_10","_11","_12","_13","_14","_15"};

  if(!interp || !o)
    return AY_ENULL;

  bcurve = (bcurve_object *)(o->refine);

  if(!bcurve)
    return AY_ENULL;

  toa = Tcl_NewStringObj(n1,-1);
  ton = Tcl_NewStringObj(n1,-1);

  Tcl_SetStringObj(ton, "Closed", -1);
  to = Tcl_NewIntObj(bcurve->closed);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton, "Length", -1);
  to = Tcl_NewIntObj(bcurve->length);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton, "BType", -1);
  to = Tcl_NewIntObj(bcurve->btype);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  if(bcurve->btype == AY_BTCUSTOM)
    {
      if(bcurve->basis)
	{
	  for(j = 0; j < 16; j++)
	    {
	      Tcl_SetStringObj(ton, "Basis", -1);
	      Tcl_AppendStringsToObj(ton, man[j], NULL);
	      to = Tcl_NewDoubleObj(bcurve->basis[j]);
	      Tcl_ObjSetVar2(interp, toa, ton, to, TCL_LEAVE_ERR_MSG |
			     TCL_GLOBAL_ONLY);
	    } /* for */
	} /* if */
      Tcl_SetStringObj(ton, "Step", -1);
      to = Tcl_NewIntObj(bcurve->step);
      Tcl_ObjSetVar2(interp, toa, ton, to, TCL_LEAVE_ERR_MSG |
		     TCL_GLOBAL_ONLY);
    } /* if */

  Tcl_SetStringObj(ton, "IsRat", -1);
  if(bcurve->is_rat)
    to = Tcl_NewStringObj("yes", -1);
  else
    to = Tcl_NewStringObj("no", -1);
  Tcl_ObjSetVar2(interp, toa, ton, to, TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton,"Tolerance",-1);
  to = Tcl_NewDoubleObj(bcurve->glu_sampling_tolerance);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton,"DisplayMode",-1);
  to = Tcl_NewIntObj(bcurve->display_mode);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);

  Tcl_IncrRefCount(toa);Tcl_DecrRefCount(toa);
  Tcl_IncrRefCount(ton);Tcl_DecrRefCount(ton);

 return AY_OK;
} /* bcurve_getpropcb */


/* bcurve_readcb:
 *  read (from scene file) callback function of bcurve object
 */
int
bcurve_readcb(FILE *fileptr, ay_object *o)
{
 bcurve_object *bcurve = NULL;
 int i, a;

  if(!fileptr || !o)
    return AY_ENULL;

  if(!(bcurve = calloc(1, sizeof(bcurve_object))))
    { return AY_EOMEM; }

  fscanf(fileptr,"%d\n",&bcurve->closed);
  fscanf(fileptr,"%d\n",&bcurve->length);
  fscanf(fileptr,"%d\n",&bcurve->btype);

  if(bcurve->btype == AY_BTCUSTOM)
    {
      fscanf(fileptr,"%d\n",&bcurve->step);
      if(!(bcurve->basis = malloc(16*sizeof(double))))
	{
	  free(bcurve);
	  return AY_EOMEM;
	}

      a = 0;
      for(i = 0; i < 4; i++)
	{
	  fscanf(fileptr, "%lg %lg %lg %lg\n", &(bcurve->basis[a]),
		 &(bcurve->basis[a+1]),
		 &(bcurve->basis[a+2]),
		 &(bcurve->basis[a+3]));
	  a += 4;
	}
    }

  if(!(bcurve->controlv = calloc(bcurve->length*4, sizeof(double))))
    {
      if(bcurve->basis)
	{free(bcurve->basis);}
      free(bcurve);
      return AY_EOMEM;
    }

  a = 0;
  for(i = 0; i < bcurve->length; i++)
    {
      fscanf(fileptr, "%lg %lg %lg %lg\n", &(bcurve->controlv[a]),
	     &(bcurve->controlv[a+1]),
	     &(bcurve->controlv[a+2]),
	     &(bcurve->controlv[a+3]));
      a += 4;
    }

  fscanf(fileptr,"%lg\n",&(bcurve->glu_sampling_tolerance));
  fscanf(fileptr,"%d\n",&(bcurve->display_mode));

  o->refine = bcurve;

 return AY_OK;
} /* bcurve_readcb */


/* bcurve_writecb:
 *  write (to scene file) callback function of bcurve object
 */
int
bcurve_writecb(FILE *fileptr, ay_object *o)
{
 bcurve_object *bcurve = NULL;
 int i, a;

  if(!fileptr || !o)
    return AY_ENULL;

  bcurve = (bcurve_object *)(o->refine);

  if(!bcurve)
    return AY_ENULL;

  fprintf(fileptr, "%d\n", bcurve->closed);
  fprintf(fileptr, "%d\n", bcurve->length);
  fprintf(fileptr, "%d\n", bcurve->btype);

  if(bcurve->btype == AY_BTCUSTOM)
    {
      fprintf(fileptr, "%d\n", bcurve->step);
      a = 0;
      for(i = 0; i < 4; i++)
	{
	  fprintf(fileptr, "%g %g %g %g\n", bcurve->basis[a],
		  bcurve->basis[a+1], bcurve->basis[a+2], bcurve->basis[a+3]);
	  a += 4;
	}
    }

  a = 0;
  for(i = 0; i < bcurve->length; i++)
    {
      fprintf(fileptr, "%g %g %g %g\n", bcurve->controlv[a],
	      bcurve->controlv[a+1],
	      bcurve->controlv[a+2],
	      bcurve->controlv[a+3]);
      a += 4;
    }

  fprintf(fileptr, "%g\n", bcurve->glu_sampling_tolerance);
  fprintf(fileptr, "%d\n", bcurve->display_mode);

 return AY_OK;
} /* bcurve_writecb */


/* bcurve_bbccb:
 *  bounding box calculation callback function of bcurve object
 */
int
bcurve_bbccb(ay_object *o, double *bbox, int *flags)
{
 bcurve_object *bcurve = NULL;

  if(!o || !bbox || !flags)
    return AY_ENULL;

  bcurve = (bcurve_object *)o->refine;

  if(!bcurve)
    return AY_ENULL;

 return ay_bbc_fromarr(bcurve->controlv, bcurve->length, 4, bbox);
} /* bcurve_bbccb */


/* bcurve_notifycb:
 *  notification callback function of bcurve object
 */
int
bcurve_notifycb(ay_object *o)
{
 int ay_status = AY_OK;
 bcurve_object *bcurve = NULL;
 ay_nurbcurve_object *nc = NULL;

  if(!o)
    return AY_ENULL;

  bcurve = (bcurve_object *)(o->refine);

  if(!bcurve)
    return AY_ENULL;

  /* remove old NURBS curve */
  if(bcurve->ncurve)
    ay_object_delete(bcurve->ncurve);
  bcurve->ncurve = NULL;

  /* create new NURBS curve */
  ay_status = bcurve_toncurve(o, AY_BTBSPLINE, &(bcurve->ncurve));

  if(bcurve->ncurve && bcurve->ncurve->refine &&
     bcurve->ncurve->type == AY_IDNCURVE)
    {
      nc = (ay_nurbcurve_object*)bcurve->ncurve->refine;
      nc->display_mode = bcurve->display_mode;
      nc->glu_sampling_tolerance = bcurve->glu_sampling_tolerance;
    }

 return ay_status;
} /* bcurve_notifycb */


/* bcurve_convertcb:
 *  convert callback function of bcurve object
 */
int
bcurve_convertcb(ay_object *o, int in_place)
{
 int ay_status = AY_OK;
 bcurve_object *bcurve = NULL;
 ay_object *new = NULL;
 ay_nurbcurve_object *nc = NULL;

  if(!o)
    return AY_ENULL;

  bcurve = (bcurve_object *) o->refine;

  if(!bcurve)
    return AY_ENULL;

  if(bcurve->ncurve)
    {
      ay_status = ay_object_copy(bcurve->ncurve, &new);

      if(new)
	{
	  nc = (ay_nurbcurve_object *)(new->refine);

	  /* reset display mode and sampling tolerance
	     of new curve to "global"? */
	  if(!in_place && ay_prefs.conv_reset_display)
	    {
	      nc->display_mode = 0;
	      nc->glu_sampling_tolerance = 0.0;
	    }

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
    } /* if */

 return ay_status;
} /* bcurve_convertcb */


/* bcurve_providecb:
 *  provide callback function of bcurve object
 */
int
bcurve_providecb(ay_object *o, unsigned int type, ay_object **result)
{
 int ay_status = AY_OK;
 bcurve_object *bcurve = NULL;

  if(!o)
    return AY_ENULL;

  if(!result)
    {
      if(type == AY_IDNCURVE)
	return AY_OK;
      else
	return AY_ERROR;
    }

  bcurve = (bcurve_object *) o->refine;

  if(!bcurve)
    return AY_ENULL;

  if(type == AY_IDNCURVE)
    {
      if(bcurve->ncurve)
	{
	  ay_status = ay_object_copy(bcurve->ncurve, result);
	  if(*result)
	    {
	      ay_trafo_copy(o, *result);
	    } /* if */
	}
      else
	{
	  return AY_ERROR;
	} /* if */
    } /* if */

 return ay_status;
} /* bcurve_providecb */


/* Bcurve_Init:
 * initializes the bcurve module/plugin by registering a new
 * object type (bcurve) and loading the accompanying Tcl script file.
 */
#ifdef WIN32
  __declspec (dllexport)
#endif /* WIN32 */
int
Bcurve_Init(Tcl_Interp *interp)
{
 int ay_status = AY_OK;
 char fname[] = "bcurve_init";

#ifdef WIN32
  if(Tcl_InitStubs(interp, "8.2", 0) == NULL)
    {
      return TCL_ERROR;
    }
#endif /* WIN32 */

  if(ay_checkversion(fname, bcurve_version_ma, bcurve_version_mi))
    {
      return TCL_ERROR;
    }

  ay_status = ay_otype_register(bcurve_name,
				bcurve_createcb,
				bcurve_deletecb,
				bcurve_copycb,
				bcurve_drawcb,
				bcurve_drawhcb,
				NULL, /* no surface */
				bcurve_setpropcb,
				bcurve_getpropcb,
				bcurve_getpntcb,
				bcurve_readcb,
				bcurve_writecb,
				NULL, /* no RIB export */
				bcurve_bbccb,
				&bcurve_id);

  ay_status += ay_notify_register(bcurve_notifycb, bcurve_id);

  ay_status += ay_convert_register(bcurve_convertcb, bcurve_id);

  ay_status += ay_provide_register(bcurve_providecb, bcurve_id);

  if(ay_status)
    {
      ay_error(AY_ERROR, fname, "Error registering custom object!");
      return TCL_OK;
    }

  /* bcurve objects may not be associated with materials */
  ay_matt_nomaterial(bcurve_id);

  /* source bcurve.tcl, it contains Tcl-code to build
     the bcurve-Attributes Property GUI */
  if((Tcl_EvalFile(interp, "bcurve.tcl")) != TCL_OK)
     {
       ay_error(AY_ERROR, fname,
		  "Error while sourcing \"bcurve.tcl\"!");
       return TCL_OK;
     }

  ay_error(AY_EOUTPUT, fname,
	   "Custom object \"bcurve\" successfully loaded.");

  /* invert Bezier basis matrix */
  (void)ay_trafo_invgenmatrix(mb, mbi);

  /* invert B-Spline basis matrix */
  (void)ay_trafo_invgenmatrix(ms, msi);

 return TCL_OK;
} /* Bcurve_Init */
