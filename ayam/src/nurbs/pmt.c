/*
 * Ayam, a free 3D modeler for the RenderMan interface.
 *
 * Ayam is copyrighted 1998-2001 by Randolf Schultz
 * (randolf.schultz@gmail.com) and others.
 *
 * All rights reserved.
 *
 * See the file License for details.
 *
 */

#include "ayam.h"

/** \file pmt.c \brief PatchMesh tools */

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
#if 0
static double mp[16] = {1, 0, 0, 0,  0, 1, 0, 0,  0, 0, 1, 0,  0, 0, 0, 1};
#endif

static double mbi[16];

static double msi[16];

/* for reference */
#if 0
RtBasis RiBezierBasis = { { -1.0,  3.0, -3.0,  1.0 },
                          {  3.0, -6.0,  3.0,  0.0 },
                          { -3.0,  3.0,  0.0,  0.0 },
                          {  1.0,  0.0,  0.0,  0.0 }
                         };


/* Define One Sixth of */
#define Sx(a) ((a)/6)
RtBasis RiBSplineBasis = { { Sx(-1.0), Sx( 3.0), Sx(-3.0), Sx( 1.0) },
                           { Sx( 3.0), Sx(-6.0), Sx( 3.0), Sx( 0.0) },
                           { Sx(-3.0), Sx( 0.0), Sx( 3.0), Sx( 0.0) },
                           { Sx( 1.0), Sx( 4.0), Sx( 1.0), Sx( 0.0) }
                         };

/* Define One Half of */
#define Hf(a) ((a)/2)
RtBasis RiCatmullRomBasis = { { Hf(-1.0), Hf( 3.0), Hf(-3.0), Hf( 1.0) },
                              { Hf( 2.0), Hf(-5.0), Hf( 4.0), Hf(-1.0) },
                              { Hf(-1.0), Hf( 0.0), Hf( 1.0), Hf( 0.0) },
                              { Hf( 0.0), Hf( 2.0), Hf( 0.0), Hf( 0.0) }
                             };

RtBasis RiHermiteBasis = { {  2.0,  1.0, -2.0,  1.0 },
                           { -3.0, -2.0,  3.0, -1.0 },
                           {  0.0,  1.0,  0.0,  0.0 },
                           {  1.0,  0.0,  0.0,  0.0 }
                          };

RtBasis RiPowerBasis = { { 1.0, 0.0, 0.0, 0.0 },
                         { 0.0, 1.0, 0.0, 0.0 },
                         { 0.0, 0.0, 1.0, 0.0 },
                         { 0.0, 0.0, 0.0, 1.0 }
                        };
#endif



/* prototypes of functions local to this module: */

int ay_pmt_tonpatchmulti(ay_object *o, ay_object **result);


/* functions: */

/** ay_pmt_valid:
 *  check patch mesh for validity
 *
 * \param[in] pamesh patch mesh object to process
 *
 * \returns AY_OK (0) if mesh is valid
 *   else:
 *  -1: NULL pointer delivered
 *   1: too few control points (need atleast 4 by 4)
 *   2: stepsize too small
 *   3: u-basistype width mismatch
 *   4: v-basistype height mismatch
 *
 */
int
ay_pmt_valid(ay_pamesh_object *pamesh)
{
 int stepu = 0, stepv = 0;

  if(!pamesh)
    return -1;

  if(pamesh->type == AY_PTBILINEAR)
    { /* bilinear patch mesh */
      /* nothing to check */
      return AY_OK;
    }
  else
    { /* bicubic patch mesh */

      if((pamesh->width < 4) || (pamesh->height < 4))
	{
	  return 1;
	}

      switch(pamesh->btype_u)
	{
	case AY_BTBEZIER:
	  stepu = 3;
	  break;
	case AY_BTBSPLINE:
	  stepu = 1;
	  break;
	case AY_BTCATMULLROM:
	  stepu = 1;
	  break;
	case AY_BTHERMITE:
	  stepu = 2;
	  break;
	case AY_BTCUSTOM:
	  stepu = pamesh->ustep;
	  break;
	default:
	  break;
	} /* switch */

      if(stepu <= 0)
	{
	  return 2;
	}

      if(pamesh->close_u)
	{ /* periodic patch */
	  if(fabs(fmod((double)pamesh->width-4+(4-stepu), stepu)) >
	     AY_EPSILON)
	    {
	      return 3;
	    }
	}
      else
	{ /* non periodic patch */
	  if(fabs(fmod((double)pamesh->width-4, stepu)) > AY_EPSILON)
	    {
	      return 3;
	    }
	}

      switch(pamesh->btype_v)
	{
	case AY_BTBEZIER:
	  stepv = 3;
	  break;
	case AY_BTBSPLINE:
	  stepv = 1;
	  break;
	case AY_BTCATMULLROM:
	  stepv = 1;
	  break;
	case AY_BTHERMITE:
	  stepv = 2;
	  break;
	case AY_BTCUSTOM:
	  stepv = pamesh->vstep;
	  break;
	default:
	  break;
	} /* switch */

      if(stepv <= 0)
	{
	  return 2;
	}

      if(pamesh->close_v)
	{ /* periodic patch */
	  if(fabs(fmod((double)pamesh->height-4+(4-stepv), stepv)) >
	     AY_EPSILON)
	    {
	      return 4;
	    }
	}
      else
	{ /* non periodic patch */
	  if(fabs(fmod((double)pamesh->height-4, stepv)) > AY_EPSILON)
	    {
	      return 4;
	    }
	}
    } /* if */

 return AY_OK;
} /* ay_pmt_valid */


/** ay_pmt_getpntfromindex:
 *  get address of a single control point from its indices
 *  (performing bounds checking)
 *
 * \param[in] pm patch mesh object to process
 * \param[in] indexu index of desired control point in U dimension (width)
 * \param[in] indexv index of desired control point in V dimension (height)
 * \param[in,out] p pointer to pointer where to store the resulting address
 *
 * \returns AY_OK on success, error code otherwise.
 */
int
ay_pmt_getpntfromindex(ay_pamesh_object *pm, int indexu, int indexv,
		       double **p)
{
 int stride = 4;
 char fname[] = "pmt_getpntfromindex";

  if(!pm || !p)
    return AY_ENULL;

  if(indexu >= pm->width || indexu < 0)
    return ay_error_reportirange(fname, "\"indexu\"", 0, pm->width-1);

  if(indexv >= pm->height || indexv < 0)
    return ay_error_reportirange(fname, "\"indexv\"", 0, pm->height-1);

  *p = &(pm->controlv[(indexu*pm->height+indexv)*stride]);

 return AY_OK;
} /* ay_pmt_getpntfromindex */


/** ay_pmt_swapuv:
 * swap U and V dimensions of a patch mesh
 *
 * \param[in,out] pm patch mesh object to process
 *
 * \returns AY_OK on success, error code otherwise.
 */
int
ay_pmt_swapuv(ay_pamesh_object *pm)
{
 int ay_status = AY_OK;
 int i;
 double *dt;

  if(!pm)
    return AY_ENULL;

  ay_status = ay_npt_swaparray(&(pm->controlv), 4, pm->width, pm->height);

  if(ay_status)
    return ay_status;

  i = pm->width;
  pm->width = pm->height;
  pm->height = i;

  i = pm->btype_u;
  pm->btype_u = pm->btype_v;
  pm->btype_v = i;

  i = pm->ustep;
  pm->ustep = pm->vstep;
  pm->vstep = i;

  dt = pm->ubasis;
  pm->ubasis = pm->vbasis;
  pm->vbasis = dt;

 return ay_status;
} /* ay_pmt_swapuv */


/** ay_pmt_revertu:
 * revert patch mesh along U (width)
 *
 * \param[in,out] pm patch mesh object to revert
 *
 * \returns AY_OK on success, error code otherwise.
 */
int
ay_pmt_revertu(ay_pamesh_object *pm)
{
 int ay_status = AY_OK;
 int i, j, ii, jj, stride = 4;
 double t[4];

  for(i = 0; i < pm->height; i++)
    {
      for(j = 0; j < pm->width/2; j++)
	{
	  ii = (j*pm->height+i)*stride;
	  jj = ((pm->width-1-j)*pm->height+i)*stride;
	  memcpy(t, &(pm->controlv[ii]), stride*sizeof(double));
	  memcpy(&(pm->controlv[ii]), &(pm->controlv[jj]),
		 stride*sizeof(double));
	  memcpy(&(pm->controlv[jj]), t, stride*sizeof(double));
	}
    }

 return ay_status;
} /* ay_pmt_revertu */


/** ay_pmt_revertv:
 * revert patch mesh along V (height)
 *
 * \param[in,out] pm patch mesh object to revert
 *
 * \returns AY_OK on success, error code otherwise.
 */
int
ay_pmt_revertv(ay_pamesh_object *pm)
{
 int ay_status = AY_OK;
 int i, j, ii, jj, stride = 4;
 double t[4];

  for(i = 0; i < pm->width; i++)
    {
      ii = i*pm->height*stride;
      jj = ii + ((pm->height-1)*stride);
      for(j = 0; j < pm->height/2; j++)
	{
	  memcpy(t, &(pm->controlv[ii]), stride*sizeof(double));
	  memcpy(&(pm->controlv[ii]), &(pm->controlv[jj]),
		 stride*sizeof(double));
	  memcpy(&(pm->controlv[jj]), t, stride*sizeof(double));
	  ii += stride;
	  jj -= stride;
	}
    }

 return ay_status;
} /* ay_pmt_revertv */


/** ay_pmt_israt:
 *  check whether any control point of patch mesh \a pm
 *  uses a weight value (!= 1.0)
 *
 * \param[in] pm PatchMesh to check
 *
 * \returns AY_TRUE if any weight is != 1.0, else returns AY_FALSE
 */
int
ay_pmt_israt(ay_pamesh_object *pm)
{
 int i, j;
 double *p;

  if(!pm)
    return AY_FALSE;

  p = &(pm->controlv[3]);
  for(i = 0; i < pm->width; i++)
    {
      for(j = 0; j < pm->height; j++)
	{
	  if((fabs(*p) < (1.0-AY_EPSILON)) || (fabs(*p) > (1.0+AY_EPSILON)))
	    return AY_TRUE;
	  p += 4;
	} /* for */
    } /* for */

 return AY_FALSE;
} /* ay_pmt_israt */


/** ay_pmt_tobasis:
 * Convert PatchMesh to a different matrix.
 *
 * \param[in,out] pm PatchMesh to process (must be open and of w/h 4)
 * \param[in] btype target basis type
 * \param[in] bstep target step size
 * \param[in] basis target basis (may be NULL unless \a btype is AY_BTCUSTOM)
 *
 * \returns AY_OK on success, error code otherwise.
 */
int
ay_pmt_tobasis(ay_pamesh_object *pm, int btype, int bstep, double *basis)
{
 int convertu = AY_FALSE, convertv = AY_FALSE, i, j, k, l, i1, i2;
 int have_mi = AY_FALSE;
 double *p1, *p2, *p3, *p4;
 double m[16], mi[16], tm[16], mu[16], mv[16], mut[16];

  if(!pm)
    return AY_ENULL;

  if((btype == AY_BTCUSTOM) && !basis)
    return AY_ERROR;

  if(pm->width != 4 || pm->height != 4 || pm->close_u || pm->close_v)
    return AY_ERROR;

  if(pm->btype_u != btype)
    {
      convertu = AY_TRUE;
    }
  else
    {
      if(pm->btype_u == AY_BTCUSTOM)
	{
	  if(memcmp(pm->ubasis, basis, 16*sizeof(double)))
	    convertu = AY_TRUE;
	}
    }

  if(pm->btype_v != btype)
    {
      convertv = AY_TRUE;
    }
  else
    {
      if(pm->btype_v == AY_BTCUSTOM)
	{
	  if(memcmp(pm->vbasis, basis, 16*sizeof(double)))
	    convertv = AY_TRUE;
	}
    }

  if(!convertu && !convertv)
    return AY_OK;

  /* create conversion matrices */
  if(convertu)
    {
      switch(btype)
	{
	case AY_BTBSPLINE:
	  memcpy(mu, msi, 16*sizeof(double));
	  break;
	case AY_BTBEZIER:
	  memcpy(mu, mbi, 16*sizeof(double));
	  break;
	default:
	  if(ay_trafo_invgenmatrix(basis, mi))
	    {
	      return AY_ERROR;
	    }
	  have_mi = AY_TRUE;
	  memcpy(mu, mi, 16*sizeof(double));
	  break;
	}

      switch(pm->btype_u)
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
	  ay_trafo_multmatrix(mu, pm->ubasis);
	  break;
	default:
	  return AY_ERROR;
	}

      /* transpose u conversion matrix */
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
    } /* if convertu */

  if(convertv)
    {
      switch(btype)
	{
	case AY_BTBSPLINE:
	  memcpy(mv, msi, 16*sizeof(double));
	  break;
	case AY_BTBEZIER:
	  memcpy(mv, mbi, 16*sizeof(double));
	  break;
	default:
	  if(!have_mi)
	    if(ay_trafo_invgenmatrix(basis, mi))
	      {
		return AY_ERROR;
	      }
	  memcpy(mv, mi, 16*sizeof(double));
	  break;
	}

      switch(pm->btype_v)
	{
	case AY_BTBEZIER:
	  ay_trafo_multmatrix(mv, mb);
	  break;
	case AY_BTBSPLINE:
	  ay_trafo_multmatrix(mv, ms);
	  break;
	case AY_BTHERMITE:
	  ay_trafo_multmatrix(mv, mh);
	  break;
	case AY_BTCATMULLROM:
	  ay_trafo_multmatrix(mv, mc);
	  break;
	case AY_BTPOWER:
	  /* use inv(basis) unchanged */
	  break;
	case AY_BTCUSTOM:
	  ay_trafo_multmatrix(mv, pm->vbasis);
	  break;
	default:
	  return AY_ERROR;
	}
    } /* if convertv */

  p1 = pm->controlv;
  p2 = p1 + 16;
  p3 = p2 + 16;
  p4 = p3 + 16;

  /* for each control point component (x,y,z) */
  for(k = 0; k < 3; k++)
    {
      /* get coordinates into a 4x4 matrix */
      for(l = 0; l < 4; l++)
	{
	  m[l]    = *(p1+(l*4)+k);
	  m[l+4]  = *(p2+(l*4)+k);
	  m[l+8]  = *(p3+(l*4)+k);
	  m[l+12] = *(p4+(l*4)+k);
	}

      /* apply conversion matrices */
      if(convertv)
	{
	  memcpy(tm, mv, 16*sizeof(double));
	  ay_trafo_multmatrix(tm, m);
	  memcpy(m, tm, 16*sizeof(double));
	}

      if(convertu)
	{
	  ay_trafo_multmatrix(m, mut);
	}

      /* copy converted coordinates back */
      for(l = 0; l < 4; l++)
	{
	  *(p1+(l*4)+k) = m[l];
	  *(p2+(l*4)+k) = m[l+4];
	  *(p3+(l*4)+k) = m[l+8];
	  *(p4+(l*4)+k) = m[l+12];
	}
    } /* for each component */

  if(btype == AY_BTCUSTOM)
    {
      /* copy custom basis */
      if(pm->btype_u != AY_BTCUSTOM)
	{
	  if(!(pm->ubasis = malloc(16*sizeof(double))))
	     return AY_EOMEM;
	}
      memcpy(pm->ubasis, basis, 16*sizeof(double));

      if(pm->btype_v != AY_BTCUSTOM)
	{
	  if(!(pm->vbasis = malloc(16*sizeof(double))))
	    {
	      if(pm->btype_u != AY_BTCUSTOM)
		free(pm->ubasis);
	      return AY_EOMEM;
	    }
	}
      memcpy(pm->vbasis, basis, 16*sizeof(double));
    }

  pm->btype_u = btype;
  pm->btype_v = btype;
  pm->ustep = bstep;
  pm->vstep = bstep;

 return AY_OK;
} /* ay_pmt_tobasis */


/** ay_pmt_tobasistcmd:
 *  Convert selected PatchMesh objects to a different basis.
 *  Implements the \a tobasisPM scripting interface command.
 *  See also the corresponding section in the \ayd{scctobasispm}.
 *
 *  \returns TCL_OK in any case.
 */
int
ay_pmt_tobasistcmd(ClientData clientData, Tcl_Interp *interp,
		   int argc, char *argv[])
{
 int ay_status, tcl_status;
 ay_list_object *sel = ay_selection;
 ay_pamesh_object *pm = NULL;
 char **basisv = NULL;
 int i = 1, j = 0, basisc = 0, btype = AY_BTBSPLINE;
 int have_step = AY_FALSE, step = 0;
 double *basis = NULL;

  if(argc > 2)
    {
      /* parse args */
      while(i+1 < argc)
	{
	  if(!strcmp(argv[i], "-s"))
	    {
	      have_step = AY_TRUE;
	      tcl_status = Tcl_GetInt(interp, argv[i+1], &step);
	      AY_CHTCLERRRET(tcl_status, argv[0], interp);
	      if(step < 1 || step > 4)
		{
		  ay_error(AY_ERROR, argv[0], "Illegal step size.");
		  goto cleanup;
		}
	    }

	  if(!strcmp(argv[i], "-t"))
	    {
	      tcl_status = Tcl_GetInt(interp, argv[i+1], &btype);
	      AY_CHTCLERRRET(tcl_status, argv[0], interp);
	      if(btype < 0 || btype > AY_BTCUSTOM)
		{
		  ay_error(AY_ERROR, argv[0], "Illegal basis matrix type.");
		  goto cleanup;
		}
	    }

	  if(!strcmp(argv[i], "-b"))
	    {
	      btype = AY_BTCUSTOM;
	      Tcl_SplitList(interp, argv[i+1], &basisc, &basisv);
	      if(basisc == 16)
		{
		  if(!(basis = malloc(16*sizeof(double))))
		    {
		      ay_error(AY_EOMEM, argv[0], NULL);
		      goto cleanup;
		    }
		  for(j = 0; j < 16; j++)
		    {
		      tcl_status = Tcl_GetDouble(interp, basisv[j], &basis[j]);
		      AY_CHTCLERRGOT(tcl_status, argv[0], interp);
		    } /* for */
		}
	      else
		{
		  ay_error(AY_ERROR, argv[0], "Basis must be 16 floats.");
		  goto cleanup;
		}

	      Tcl_Free((char *) basisv);
	      basisv = NULL;
	    }

	  i += 2;
	} /* while */
    } /* if */

  if(!have_step)
    {
      switch(btype)
	{
	case AY_BTBEZIER:
	  step = 3;
	  break;
	case AY_BTBSPLINE:
	  step = 1;
	  break;
	case AY_BTHERMITE:
	  step = 2;
	  break;
	case AY_BTCATMULLROM:
	  step = 1;
	  break;
	case AY_BTPOWER:
	  step = 4;
	  break;
	default:
	  /* AY_BTCUSTOM */
	  ay_error(AY_ERROR, argv[0], "No step specified.");
	  goto cleanup;
	}
    }

  while(sel)
    {
      if(sel->object->type == AY_IDPAMESH)
	{
	  pm = (ay_pamesh_object*)sel->object->refine;
	  ay_status = ay_pmt_tobasis(pm, btype, step, basis);
	  if(ay_status)
	    {
	      ay_error(AY_ERROR, argv[0], "Conversion failed.");
	      break;
	    }
	  sel->object->modified = AY_TRUE;
	}
      if(sel->object->modified)
	{
	  if(sel->object->selp)
	    ay_selp_clear(sel->object);
	  (void)ay_notify_object(sel->object);
	}

      sel = sel->next;
    } /* while */

  (void)ay_notify_parent();

cleanup:

  if(basis)
    free(basis);

  if(basisv)
    Tcl_Free((char *)basisv);

 return TCL_OK;
} /* ay_pmt_tobasistcmd */


/* ay_pmt_tonpatchmulti:
 * helper for ay_pmt_tonpatch() below;
 * converts a complex patch mesh by moving an evaluation window
 * over the mesh (according to the parameters defined by the basis type
 * and step size), converting the respective window to a single NURBS
 * patch, then concatenating the resulting NURBS patches first row-wise,
 * then column-wise to the final resulting NURBS patch
 */
int
ay_pmt_tonpatchmulti(ay_object *o, ay_object **result)
{
 int ay_status = AY_OK;
 char *uv = NULL;
 double tcv[4*4*4], *cv = NULL;
 int w, h, su, sv;
 int ri, rj, i, j, ii, jj, a, b;
 ay_object t = {0}, *row, **nextrow, *col, **nextcol;
 ay_pamesh_object *pm = NULL, tpm = {0};

  if(!o || !result)
    return AY_ENULL;

  pm = (ay_pamesh_object*)o->refine;
  cv = pm->controlv;

  /* set up temporary pamesh (t and tpm) */
  ay_object_defaults(&t);
  t.type = AY_IDPAMESH;
  t.refine = &tpm;
  tpm.type = AY_PTBICUBIC;
  tpm.width = 4;
  tpm.height = 4;
  tpm.btype_u = pm->btype_u;
  tpm.btype_v = pm->btype_v;
  tpm.ubasis = pm->ubasis;
  tpm.vbasis = pm->vbasis;
  /* due to hard-coded width/height 4, no need to copy the steps */
  /*
  tpm.ustep = pm->ustep;
  tpm.vstep = pm->vstep;
  */
  tpm.controlv = tcv;

  col = NULL;
  nextcol = &col;

  switch(pm->btype_u)
    {
    case AY_BTBEZIER:
      su = 3;
      break;
    case AY_BTHERMITE:
      su = 2;
      break;
    case AY_BTCATMULLROM:
      su = 1;
      break;
    default:
      su = pm->ustep;
      break;
    }

  switch(pm->btype_v)
    {
    case AY_BTBEZIER:
      sv = 3;
      break;
    case AY_BTHERMITE:
      sv = 2;
      break;
    case AY_BTCATMULLROM:
      sv = 1;
      break;
    default:
      sv = pm->vstep;
      break;
    }

  w = pm->width/su;
  h = pm->height/sv;

  if(pm->close_u)
    {
      while(((w+1)*su) < pm->width)
	w++;
    }

  if(pm->close_v)
    {
      while(((h+1)*sv) < pm->height)
	h++;
    }

  if(h > 1)
    {
      if(!(uv = malloc(h*sizeof(char))))
	return AY_EOMEM;
      for(i = 0; i < h; i++)
	uv[i] = 'v';
    }

  for(i = 0; i < h; i++)
    {
      nextrow = &row;
      for(j = 0; j < w; j++)
	{
	  /* get eval window (fill tcv) */
	  a = 0;
	  for(ii = 0; ii < 4; ii++)
	    {
	      ri = j*su + ii;
	      if(ri > pm->width)
		ri = ii;

	      for(jj = 0; jj < 4; jj++)
		{
		  rj = i*sv + jj;
		  if(rj > pm->height)
		    rj = jj;

		  b = (ri*h*4+rj)*4;

		  memcpy(&(tcv[a]), &(cv[b]), 4*sizeof(double));
		  a += 4;
		}
	    }

	  /* convert to NPatch */
	  ay_status = ay_pmt_tonpatch(&t, AY_BTBEZIER, nextrow);
	  if(!ay_status && *nextrow)
	    nextrow = &((*nextrow)->next);
	  else
	    goto cleanup;
	} /* for all eval windows in x */

      /* concatenate row? */
      if(row && row->next)
	{
	  ay_npt_concat(row, AY_CTOPEN, /*order=*/4, AY_KTCUSTOM,
			/*fillet_type=*/0, /*ftlen=*/0.0,
			/*compatible=*/AY_TRUE,
			/*uv=*/NULL,
			nextcol);
	}
      else
	{
	  *nextcol = row;
	}
      row = NULL;
      nextcol = &((*nextcol)->next);
    } /* for all eval windows in y */

  /* concatenate all rows? */
  if(col && col->next)
    {
      ay_npt_concat(col, AY_CTOPEN, /*order=*/4, AY_KTCUSTOM,
		    /*fillet_type=*/0, /*ftlen=*/0.0,
		    /*compatible=*/AY_TRUE,
		    /*uv=*/uv,
		    result);
    }
  else
    {
      *result = col;
    }

cleanup:

  if(row)
    (void)ay_object_deletemulti(row, AY_FALSE);

  if(uv)
    free(uv);

 return ay_status;
} /* ay_pmt_tonpatchmulti */


/** ay_pmt_tonpatch:
 * Create a NURBS patch from a PatchMesh.
 * This function will not work properly or crash if the PatchMesh
 * fails the validity check using ay_pmt_valid() above!
 * This function only handles bilinear patch meshes, bicubic patch
 * meshes of basis type AY_BTBSPLINE, and bicubic patch meshes of
 * arbitrary basis type and width/height 4.
 * Other, complex, patch meshes are handled via ay_pmt_tonpatchmulti()
 * above.
 *
 * \param[in] o the patch mesh to convert
 * \param[in] btype desired basis type, must be AY_BTBSPLINE;
 *  the other valid type (AY_BTBEZIER) is used exclusively by
 *  ay_pmt_tonpatchmulti() when calling back
 * \param[in,out] result pointer where to store the resulting NPatch
 *
 * \returns AY_OK on success, error code otherwise.
 */
int
ay_pmt_tonpatch(ay_object *o, int btype, ay_object **result)
{
 int ay_status = AY_OK;
 double *cv = NULL;
 int w, h, ktu, ktv, uorder, vorder;
 int i = 0, a, b;
 ay_object *newo = NULL, *c = NULL;
 ay_pamesh_object *pamesh;

  if(!o || !result)
    return AY_ENULL;

  pamesh = (ay_pamesh_object*) o->refine;

  w = pamesh->width;
  h = pamesh->height;

  if(pamesh->type == AY_PTBILINEAR)
    {
      if(pamesh->close_u)
	w++;
      if(pamesh->close_v)
	h++;
      ktu = AY_KTNURB;
      uorder = 2;
      ktv = AY_KTNURB;
      vorder = 2;
    }
  else
    {
      if((pamesh->btype_u != AY_BTBSPLINE || pamesh->btype_v != AY_BTBSPLINE)
	 &&
	 (pamesh->width > 4 || pamesh->height > 4 || pamesh->close_u ||
	  pamesh->close_v))
	return ay_pmt_tonpatchmulti(o, result);

      if(pamesh->btype_u != btype || pamesh->btype_v != btype)
	{
	  (void)ay_object_copy(o, &c);
	  if(c)
	    ay_status = ay_pmt_tobasis(c->refine, btype, /*bstep=*/1,
				       /*basis=*/NULL);
	  else
	    return AY_ERROR;
	  if(ay_status)
	    goto cleanup;

	  pamesh = c->refine;
	}
      else
	{
	  if(pamesh->close_u)
	    w += 3;
	  if(pamesh->close_v)
	    h += 3;
	}

      if(btype == AY_BTBSPLINE)
	{
	  ktu = AY_KTBSPLINE;
	  ktv = AY_KTBSPLINE;
	}
      else
	{
	  ktu = AY_KTBEZIER;
	  ktv = AY_KTBEZIER;
	}

      uorder = 4;
      vorder = 4;
    } /* if */

  ay_status = ay_npt_createnpatchobject(&newo);

  if(ay_status || !newo)
    {
      if(!ay_status)
	ay_status = AY_ERROR;
      goto cleanup;
    }

  if(!(cv = malloc(w*h*4*sizeof(double))))
    {
      ay_status = AY_EOMEM;
      goto cleanup;
    }

  /* fill cv */
  a = 0;
  b = 0;
  for(i = 0; i < pamesh->width; i++)
    {
      memcpy(&(cv[a]), &(pamesh->controlv[b]),
	     pamesh->height*4*sizeof(double));
      a += pamesh->height*4;
      if(h > pamesh->height)
	{
	  memcpy(&(cv[a]), &(pamesh->controlv[b]),
		 (h-pamesh->height)*4*sizeof(double));
	  a += (h-pamesh->height)*4;
	}
      b += h*4;
    }
  b = 0;
  if(w > pamesh->width)
    {
      for(i = 0; i < (w-pamesh->width); i++)
	{
	  memcpy(&(cv[a]), &(pamesh->controlv[b]),
		 pamesh->height*4*sizeof(double));
	  a += pamesh->height*4;
	  if(h > pamesh->height)
	    {
	      memcpy(&(cv[a]), &(pamesh->controlv[b]),
		     (h-pamesh->height)*4*sizeof(double));
	      a += (h-pamesh->height)*4;
	    }
	  b += pamesh->height*4;
	}
    }

  ay_status = ay_npt_create(uorder, vorder, w, h, ktu, ktv, cv, NULL, NULL,
			    (ay_nurbpatch_object **)(void*)&(newo->refine));

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
} /* ay_pmt_tonpatch */


/** ay_pmt_init:
 * Initialize the patch mesh tools module.
 */
void
ay_pmt_init()
{

  /* invert Bezier basis matrix */
  (void)ay_trafo_invgenmatrix(mb, mbi);

  /* invert B-Spline basis matrix */
  (void)ay_trafo_invgenmatrix(ms, msi);

 return;
} /* ay_pmt_init */

