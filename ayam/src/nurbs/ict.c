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

/** \file ict.c \brief interpolating curve tools */

/* prototypes of functions local to this module: */

int ay_ict_sanitize(int length, double *controlv,
		    int *slength, double **scontrolv);


/* functions: */

/* ay_ict_sanitize:
 *  sanitize (remove consecutive duplicate points) a vector
 *  of 3D data points in <controlv[<length>]>;
 *  puts sanitized result vector into <scontrolv[<slength>]>;
 *  if scontrolv is NULL after this function run, controlv was
 *  already clean...
 *
 * \returns AY_OK on success, error code otherwise.
 */
int
ay_ict_sanitize(int length, double *controlv,
		int *slength, double **scontrolv)
{
 int i, a = 0, b;

  *slength = length;
  *scontrolv = NULL;

  if(!(*scontrolv = calloc(length*3, sizeof(double))))
    return AY_EOMEM;

  /* always copy the first point */
  memcpy(*scontrolv, controlv, 3*sizeof(double));
  b = 3;

  /* check and copy the rest */
  for(i = 0; i < (length-1); i++)
    {
      if((fabs(controlv[a+3] - controlv[a])   > AY_EPSILON) ||
	 (fabs(controlv[a+4] - controlv[a+1]) > AY_EPSILON) ||
	 (fabs(controlv[a+5] - controlv[a+2]) > AY_EPSILON))
	{
	  /* copy point a+3 */
	  memcpy(&((*scontrolv)[b]), &(controlv[a+3]), 3*sizeof(double));
	  b += 3;
	}
      else
	{
	  /* discard point a+3 */
	  (*slength)--;
	}
      a += 3;
    } /* for */

  if(*slength == length)
    {
      free(*scontrolv);
      *scontrolv = NULL;
    }

 return AY_OK;
} /* ay_ict_sanitize */


/** ay_ict_interpolateC2C:
 * Global C2 cubic curve interpolation.
 *
 * \param[in] length number of data points
 * \param[in] sdlen length of automatically generated start derivative
 * \param[in] edlen length of automatically generated end derivative
 * \param[in] param_type parameterisation type (AY_KTCHORDAL, AY_KTCENTRI,
 *            or AY_KTUNIFORM)
 * \param[in] have_end_derivs should sderiv / ederiv be used?
 * \param[in] sderiv start derivative
 * \param[in] sderiv end derivative
 * \param[in] controlv vector of 3D data points [length*3]
 *
 * \param[in,out] c result (a NURBS curve)
 *
 * \returns AY_OK on success, error code otherwise.
 */
int
ay_ict_interpolateC2C(int length, double sdlen, double edlen, int param_type,
		      int have_end_derivs, double *sderiv, double *ederiv,
		      double *controlv,
		      ay_nurbcurve_object **c)
{
 int ay_status = AY_OK;
 int a, b, d, i, j, index;
 int nlength, slength;
 double v[3];
 double *ncontrolv = NULL, *ncv4D = NULL;
 double *knotv = NULL, *vk = NULL, knot = 0.0;
 double *lengths = NULL, totallength = 0.0;
 double *scontrolv = NULL;
 ay_nurbcurve_object *new = NULL;


  ay_ict_sanitize(length, controlv, &slength, &scontrolv);

  if(scontrolv)
    {
      controlv = scontrolv;
      length = slength;
    }

  if(length <= 1)
    { ay_status = AY_ERROR; goto cleanup; }

  nlength = length + 2;

  if(!(ncontrolv = calloc(nlength*3, sizeof(double))))
    { ay_status = AY_EOMEM; goto cleanup; }

  /* create knot vector */
  if(!(knotv = calloc(nlength+4, sizeof(double))))
    { ay_status = AY_EOMEM; goto cleanup; }

  if(param_type != AY_KTUNIFORM)
    {
      if(!(lengths = calloc(length-1, sizeof(double))))
	{ ay_status = AY_EOMEM; goto cleanup; }

      a = 0;
      for(i = 0; i < (length-1); i++)
	{
	  lengths[i] = AY_VLEN((controlv[a+3] - controlv[a]),
			       (controlv[a+4] - controlv[a+1]),
			       (controlv[a+5] - controlv[a+2]));

	  if(param_type == AY_KTCENTRI)
	    {
	      lengths[i] = sqrt(lengths[i]);
	    }

	  totallength += lengths[i];

	  a += 3;
	} /* for */

      if(!(vk = calloc(length+1, sizeof(double))))
	{ ay_status = AY_EOMEM; goto cleanup; }

      vk[0] = 0.0;
      j = 0;
      knot = 0.0;
      for(i = 1; i < length; i++)
	{
	  knot += lengths[j]/totallength;
	  vk[i] = knot;
	  j++;
	}
      vk[length] = 1.0;

      /* knot averaging (for 2 additional knots) */
      for(j = 0; j < length-2; j++)
	{
	  index = j + 4;
	  knotv[index] = 0.0;
	  for(i = j; i < j + 3; i++)
	    {
	      knotv[index] += vk[i];
	    }
	  knotv[index] /= 3.0;
	}
    }
  else
    {
      /* create uniform knots */
      for(j = 1; j < length-1; j++)
	{
	  index = j + 3;
	  knotv[index] = (double)j/(length-1);
	}
    }

  for(i = 0; i < 4; i++)
    knotv[i] = 0.0;
  for(i = nlength; i < nlength+4; i++)
    knotv[i] = 1.0;

  /* create first two and last two controls */
  memcpy(ncontrolv, controlv, 3*sizeof(double));

  a = (nlength-2)*3;
  if(!have_end_derivs)
    {
      b = 3;
      v[0] = controlv[b]   - controlv[0];
      v[1] = controlv[b+1] - controlv[1];
      v[2] = controlv[b+2] - controlv[2];

      AY_V3SCAL(v, sdlen)

      ncontrolv[3] = controlv[0] + v[0];
      ncontrolv[4] = controlv[1] + v[1];
      ncontrolv[5] = controlv[2] + v[2];

      b = (length-2)*3;

      v[0] = controlv[b+3] - controlv[b];
      v[1] = controlv[b+4] - controlv[b+1];
      v[2] = controlv[b+5] - controlv[b+2];

      AY_V3SCAL(v, edlen)

      ncontrolv[a]   = controlv[b+3] - v[0];
      ncontrolv[a+1] = controlv[b+4] - v[1];
      ncontrolv[a+2] = controlv[b+5] - v[2];
    }
  else
    {
      memcpy(&(ncontrolv[3]), sderiv, 3*sizeof(double));
      memcpy(&(ncontrolv[a]), ederiv, 3*sizeof(double));
    }

  a = (nlength-1)*3;
  b = (length-1)*3;
  ncontrolv[a]   = controlv[b];
  ncontrolv[a+1] = controlv[b+1];
  ncontrolv[a+2] = controlv[b+2];

  ay_status = ay_nb_SolveTridiagonal(length-1, controlv, knotv,
				     ncontrolv);

  if(ay_status)
    goto cleanup;

  /* copy results to 4D controlv */
  if(!(ncv4D = calloc(nlength*4, sizeof(double))))
    { ay_status = AY_EOMEM; goto cleanup; }
  a = 3; b = 0; d = 0;
  for(i = 0; i < nlength; i++)
    {
      memcpy(&(ncv4D[b]), &(ncontrolv[d]), 3*sizeof(double));
      ncv4D[a] = 1.0;
      a += 4;
      b += 4;
      d += 3;
    }

  ay_status = ay_nct_create(4, nlength, AY_KTCUSTOM, ncv4D, knotv, &new);

  if(!ay_status)
    {
      /* return result */
      *c = new;

      /* prevent cleanup code from doing something harmful */
      knotv = NULL;
      ncv4D = NULL;
    }

cleanup:

  if(scontrolv)
    free(scontrolv);
  if(ncontrolv)
    free(ncontrolv);
  if(knotv)
    free(knotv);
  if(lengths)
    free(lengths);
  if(vk)
    free(vk);
  if(ncv4D)
    free(ncv4D);

 return ay_status;
} /* ay_ict_interpolateC2C */


/** ay_ict_interpolateC2CClosed:
 * Global closed C2 cubic curve interpolation.
 *
 * \param[in] length number of data points
 * \param[in] sdlen length of automatically generated start derivative
 * \param[in] edlen length of automatically generated end derivative
 * \param[in] param_type parameterisation type (AY_KTCHORDAL, AY_KTCENTRI,
 *            or AY_KTUNIFORM)
 * \param[in] have_end_derivs should sderiv / ederiv be used?
 * \param[in] sderiv start derivative
 * \param[in] sderiv end derivative
 * \param[in] controlv vector of 3D data points [length*3]
 *
 * \param[in,out] c result (a NURBS curve)
 *
 * \returns AY_OK on success, error code otherwise.
 */
int
ay_ict_interpolateC2CClosed(int length, double sdlen, double edlen,
			    int param_type,
			    int have_end_derivs,
			    double *sderiv, double *ederiv,
			    double *controlv,
			    ay_nurbcurve_object **c)
{
 int ay_status = AY_OK;
 int a = 0, b, d, i, j;
 int nlength, slength, index;
 double v[3];
 double *ncontrolv = NULL, *ccontrolv = NULL, *ncv4D = NULL;
 double *knotv = NULL, *vk = NULL, knot = 0.0;
 double *lengths = NULL, totallength = 0.0;
 double *scontrolv = NULL;
 ay_nurbcurve_object *new = NULL;


  ay_ict_sanitize(length, controlv, &slength, &scontrolv);

  if(scontrolv)
    {
      controlv = scontrolv;
      length = slength;
    }

  nlength = length + 3;

  /* extend the control vector */
  if(!(ccontrolv = calloc((length+1)*3, sizeof(double))))
    { ay_status = AY_EOMEM; goto cleanup; }

  memcpy(&(ccontrolv[0]), &(controlv[0]), length*3*sizeof(double));
  memcpy(&(ccontrolv[length*3]), &(controlv[0]), 3*sizeof(double));

  if(!(ncontrolv = calloc(nlength*3, sizeof(double))))
    { ay_status = AY_EOMEM; goto cleanup; }

  /* create knot vector */
  if(!(knotv = calloc(nlength+4, sizeof(double))))
    { ay_status = AY_EOMEM; goto cleanup; }

  if(param_type != AY_KTUNIFORM)
    {
      if(!(lengths = calloc(length, sizeof(double))))
	{ ay_status = AY_EOMEM; goto cleanup; }

      for(i = 0; i < length; i++)
	{
	  lengths[i] = AY_VLEN((ccontrolv[a+3] - ccontrolv[a]),
			       (ccontrolv[a+4] - ccontrolv[a+1]),
			       (ccontrolv[a+5] - ccontrolv[a+2]));

	  if(param_type == AY_KTCENTRI)
	    {
	      lengths[i] = sqrt(lengths[i]);
	    }

	  totallength += lengths[i];

	  a += 3;
	} /* for */

      if(!(vk = calloc(length+2, sizeof(double))))
	{ ay_status = AY_EOMEM; goto cleanup; }

      vk[0] = 0.0;
      j = 0;
      knot = 0.0;
      for(i = 1; i < (length+1); i++)
	{
	  knot += lengths[j]/totallength;
	  vk[i] = knot;
	  j++;
	}
      vk[length+1] = 1.0;

      /* knot averaging (for 3 additional knots) */
      for(j = 0; j < length-1; j++)
	{
	  index = j+4;
	  knotv[index] = 0.0;
	  for(i = j; i < j + 3; i++)
	    {
	      knotv[index] += vk[i];
	    }
	  knotv[index] /= 3.0;
	}
    }
  else
    {
      /* create uniform knots */
      for(j = 1; j < length; j++)
	{
	  index = j + 3;
	  knotv[index] = (double)j/(length);
	}
    }

  for(i = 0; i < 4; i++)
    knotv[i] = 0.0;
  for(i = nlength; i < nlength+4; i++)
    knotv[i] = 1.0;

  /* create first two and last two controls */
  memcpy(ncontrolv, controlv, 3*sizeof(double));

  a = (nlength-2)*3;
  if(!have_end_derivs)
    {
      b = (length-1)*3;
      v[0] = controlv[3] - controlv[b];
      v[1] = controlv[4] - controlv[b+1];
      v[2] = controlv[5] - controlv[b+2];

      AY_V3SCAL(v, sdlen)

      ncontrolv[3] = controlv[0] + v[0];
      ncontrolv[4] = controlv[1] + v[1];
      ncontrolv[5] = controlv[2] + v[2];

      v[0] = controlv[3] - controlv[b];
      v[1] = controlv[4] - controlv[b+1];
      v[2] = controlv[5] - controlv[b+2];

      AY_V3SCAL(v, edlen)

      ncontrolv[a]   = controlv[0] - v[0];
      ncontrolv[a+1] = controlv[1] - v[1];
      ncontrolv[a+2] = controlv[2] - v[2];
    }
  else
    {
      memcpy(&(ncontrolv[3]), sderiv, 3*sizeof(double));
      memcpy(&(ncontrolv[a]), ederiv, 3*sizeof(double));
    }

  a = (nlength-1)*3;
  ncontrolv[a]   = controlv[0];
  ncontrolv[a+1] = controlv[1];
  ncontrolv[a+2] = controlv[2];

  /* solve the equation system */
  ay_status = ay_nb_SolveTridiagonal(length, ccontrolv, knotv, ncontrolv);

  if(ay_status)
    goto cleanup;

  /* copy results to 4D controlv */
  if(!(ncv4D = calloc(nlength*4, sizeof(double))))
    { ay_status = AY_EOMEM; goto cleanup; }
  a = 3; b = 0; d = 0;
  for(i = 0; i < nlength; i++)
    {
      memcpy(&(ncv4D[b]), &(ncontrolv[d]), 3*sizeof(double));
      ncv4D[a] = 1.0;
      a += 4;
      b += 4;
      d += 3;
    }

  ay_status = ay_nct_create(4, nlength, AY_KTCUSTOM, ncv4D, knotv, &new);

  if(!ay_status)
    {
      /* return result */
      *c = new;

      new->type = AY_CTCLOSED;

      /* prevent cleanup code from doing something harmful */
      knotv = NULL;
      ncv4D = NULL;
    }

cleanup:

  if(scontrolv)
    free(scontrolv);
  if(ncontrolv)
    free(ncontrolv);
  if(ccontrolv)
    free(ccontrolv);
  if(lengths)
    free(lengths);
  if(vk)
    free(vk);
  if(knotv)
    free(knotv);
  if(ncv4D)
    free(ncv4D);

 return ay_status;
} /* ay_ict_interpolateC2CClosed */


/** ay_ict_interpolateG3D:
 * Global curve interpolation.
 *
 * \param[in] iorder desired order of interpolation
 *            (for order == 4, rather use interpolateC2C() above!)
 * \param[in] length number of data points
 * \param[in] sdlen length of automatically generated start derivative
 * \param[in] edlen length of automatically generated end derivative
 * \param[in] param_type parameterisation type (AY_KTCHORDAL, AY_KTCENTRI,
 *            or AY_KTUNIFORM)
 * \param[in] have_end_derivs should sderiv / ederiv be used?
 * \param[in] sderiv start derivative
 * \param[in] sderiv end derivative
 * \param[in] controlv vector of 3D data points [length*3]
 *
 * \param[in,out] c result (a NURBS curve)
 *
 * \returns AY_OK on success, error code otherwise.
 */
int
ay_ict_interpolateG3D(int iorder, int length, double sdlen, double edlen,
		      int have_end_derivs, int param_type,
		      double *controlv, double *sderiv, double *ederiv,
		      ay_nurbcurve_object **c)
{
 int ay_status = AY_OK;
 int a, i, j, order, deg, index;
 int nlength, slength;
 double v1[3], v2[3], *ncontrolv = NULL;
 double *knotv = NULL, knot = 0.0;
 double *lengths = NULL, totallength = 0.0, *vk = NULL;
 double *scontrolv = NULL;
 ay_nurbcurve_object *new = NULL;


  ay_ict_sanitize(length, controlv, &slength, &scontrolv);

  if(scontrolv)
    {
      controlv = scontrolv;
      length = slength;
    }

  if(length <= 1)
    { ay_status = AY_ERROR; goto cleanup; }

  nlength = length+2;

  if(iorder > nlength)
    order = nlength;
  else
    order = iorder;

  deg = order-1;

  if(!(ncontrolv = calloc(nlength*4, sizeof(double))))
    { ay_status = AY_EOMEM; goto cleanup; }

  /* create knotvector */
  if(!(knotv = calloc(nlength+order, sizeof(double))))
    { ay_status = AY_EOMEM; goto cleanup; }

  if(!(lengths = calloc(length-1, sizeof(double))))
    { ay_status = AY_EOMEM; goto cleanup; }

  a = 0;
  for(i = 0; i < (length-1); i++)
    {

      if(param_type == AY_KTUNIFORM)
	{
	  lengths[i] = 0.01;
	}
      else
	{
	  lengths[i] = AY_VLEN((controlv[a+3] - controlv[a]),
			       (controlv[a+4] - controlv[a+1]),
			       (controlv[a+5] - controlv[a+2]));

	  if(param_type == AY_KTCENTRI)
	    {
	      lengths[i] = sqrt(lengths[i]);
	    }
	}
      totallength += lengths[i];

      a += 3;
    } /* for */

  if(!(vk = calloc(length+1, sizeof(double))))
    { ay_status = AY_EOMEM; goto cleanup; }

  vk[0] = 0.0;
  j = 0;
  knot = 0.0;
  for(i = 1; i < length; i++)
    {
      knot += lengths[j]/totallength;
      vk[i] = knot;
      j++;
    }
  vk[length] = 1.0;

  /* knot averaging (for 2 additional knots) */
  for(j = 0; j < length+1-deg; j++)
    {
      index = j+deg+1;
      knotv[index] = 0.0;
      for(i = j; i < j+deg; i++)
	{
	  knotv[index] += vk[i];
	}
      knotv[index] /= deg;
    }

  for(i = 0; i < order; i++)
    knotv[i] = 0.0;
  for(i = nlength; i < nlength+order; i++)
    knotv[i] = 1.0;

  /* derivatives */
  if(!have_end_derivs)
    {
      v1[0] = controlv[3] - controlv[0];
      v1[1] = controlv[4] - controlv[1];
      v1[2] = controlv[5] - controlv[2];

      AY_V3SCAL(v1, sdlen)

      i = (length-2)*3;

      v2[0] = controlv[i+3] - controlv[i];
      v2[1] = controlv[i+4] - controlv[i+1];
      v2[2] = controlv[i+5] - controlv[i+2];

      AY_V3SCAL(v2, edlen)
    }
  else
    {
      v1[0] = sderiv[0] - controlv[0];
      v1[1] = sderiv[1] - controlv[1];
      v1[2] = sderiv[2] - controlv[2];

      i = (length-2)*3;

      v2[0] = controlv[i+3] - ederiv[0];
      v2[1] = controlv[i+4] - ederiv[1];
      v2[2] = controlv[i+5] - ederiv[2];
    }

  /* set up a sparse control vector */
  /* first point */
  memcpy(ncontrolv,controlv,3*sizeof(double));
  ncontrolv[3] = 1.0;
  /* inner points */
  j = 1;
  for(i = 2; i < nlength-2; i++)
    {
      memcpy(&(ncontrolv[i*4]),&(controlv[j*3]),3*sizeof(double));
      ncontrolv[i*4+3] = 1.0;
      j++;
    }
  /* last point */
  memcpy(&(ncontrolv[(nlength-1)*4]), &(controlv[(length-1)*3]),
	 3*sizeof(double));
  ncontrolv[(nlength-1)*4+3] = 1.0;

  /* solve the interpolation problem */
  ay_status = ay_nb_GlobalInterpolation4DD(length-1, ncontrolv, vk, knotv,
					   deg, v1, v2);

  if(ay_status)
    { goto cleanup; }

  ay_status = ay_nct_create(order, nlength, AY_KTCUSTOM, ncontrolv,
			    knotv, &new);

  if(!ay_status)
    {
      /* return result */
      *c = new;

      /* prevent cleanup code from doing something harmful */
      knotv = NULL;
      ncontrolv = NULL;
    }

cleanup:

  if(scontrolv)
    free(scontrolv);
  if(ncontrolv)
    free(ncontrolv);
  if(lengths)
    free(lengths);
  if(vk)
    free(vk);
  if(knotv)
    free(knotv);

 return ay_status;
} /* ay_ict_interpolateG3D */


/** ay_ict_interpolateG3DClosed:
 * Global closed curve interpolation.
 *
 * \param[in] desired order of interpolation
 *            (for order == 4, rather use ay_ict_interpolateC2CClosed() above!)
 * \param[in] length number of data points
 * \param[in] sdlen length of automatically generated start derivative
 * \param[in] edlen length of automatically generated end derivative
 * \param[in] param_type parameterisation type (AY_KTCHORDAL, AY_KTCENTRI,
 *            or AY_KTUNIFORM)
 * \param[in] have_end_derivs should sderiv / ederiv be used?
 * \param[in] sderiv start derivative
 * \param[in] sderiv end derivative
 * \param[in] controlv vector of 3D data points [length*3]
 *
 * \param[in,out] c result (a NURBS curve)
 *
 * \returns AY_OK on success, error code otherwise.
 */
int
ay_ict_interpolateG3DClosed(int iorder, int length, double sdlen, double edlen,
			    int have_end_derivs, int param_type,
			    double *controlv, double *sderiv, double *ederiv,
			    ay_nurbcurve_object **c)
{
 int ay_status = AY_OK;
 int a, i, j, order = 0, deg, index;
 int nlength, slength;
 double *ncontrolv = NULL;
 double *knotv = NULL, knot = 0.0;
 double *lengths = NULL, totallength = 0.0, *vk = NULL;
 double v1[3], v2[3];
 double *scontrolv = NULL;
 ay_nurbcurve_object *new = NULL;


  ay_ict_sanitize(length, controlv, &slength, &scontrolv);

  if(scontrolv)
    {
      controlv = scontrolv;
      length = slength;
    }

  nlength = length+3;

  if(iorder > nlength)
    order = nlength;
  else
    order = iorder;

  deg = order - 1;

  if(!(ncontrolv = calloc(nlength*4, sizeof(double))))
    { ay_status = AY_EOMEM; goto cleanup; }

  if(!(knotv = calloc(nlength+order, sizeof(double))))
    { ay_status = AY_EOMEM; goto cleanup; }

  if(!(lengths = calloc(length, sizeof(double))))
    { ay_status = AY_EOMEM; goto cleanup; }

  a = 0;
  for(i = 0; i < length-1; i++)
    {

      if(param_type == AY_KTUNIFORM)
	{
	  lengths[i] = 0.01;
	}
      else
	{
	  lengths[i] = AY_VLEN((controlv[a+3] - controlv[a]),
			       (controlv[a+4] - controlv[a+1]),
			       (controlv[a+5] - controlv[a+2]));

	  if(param_type == AY_KTCENTRI)
	    {
	      lengths[i] = sqrt(lengths[i]);
	    }
	}

      totallength += lengths[i];

      a += 3;
    } /* for */

  if(param_type == AY_KTUNIFORM)
    {
      lengths[length-1] = 0.01;
    }
  else
    {
      a = (length-1)*3;
      lengths[length-1] = AY_VLEN((controlv[0] - controlv[a]),
				  (controlv[1] - controlv[a+1]),
				  (controlv[2] - controlv[a+2]));

      if(param_type == AY_KTCENTRI)
	{
	  lengths[length-1] = sqrt(lengths[length-1]);
	}
    }

  totallength += lengths[length-1];

  /* derivative */
  if(!have_end_derivs)
    {
      a = (length-1)*3;
      v1[0] = controlv[3] - controlv[a];
      v1[1] = controlv[4] - controlv[a+1];
      v1[2] = controlv[5] - controlv[a+2];

      memcpy(v2, v1, 3*sizeof(double));

      AY_V3SCAL(v1,sdlen)
      AY_V3SCAL(v2,edlen)
    }
  else
    {
      v1[0] = sderiv[0] - controlv[0];
      v1[1] = sderiv[1] - controlv[1];
      v1[2] = sderiv[2] - controlv[2];

      v2[0] = controlv[0] - ederiv[0];
      v2[1] = controlv[1] - ederiv[1];
      v2[2] = controlv[2] - ederiv[2];
    }

  /* calc parametric values */
  if(!(vk = calloc(length+2, sizeof(double))))
    { ay_status = AY_EOMEM; goto cleanup; }

  vk[0] = 0.0;
  j = 0;
  knot = 0.0;
  for(i = 1; i < (length+1); i++)
    {
      knot += lengths[j]/totallength;
      vk[i] = knot;
      j++;
    }
  vk[length+1] = 1.0;

  /* knot averaging (for 3 additional knots) */
  for(j = 0; j < length+2-deg; j++)
    {
      index = j+deg+1;
      knotv[index] = 0.0;
      for(i = j; i < j+deg; i++)
	{
	  knotv[index] += vk[i];
	}
      knotv[index] /= deg;
    }

  for(i = 0; i < order; i++)
    knotv[i] = 0.0;
  for(i = nlength; i < nlength+order; i++)
    knotv[i] = 1.0;

  /* set up a sparse control vector */
  /* first point */
  memcpy(ncontrolv,controlv,3*sizeof(double));
  ncontrolv[3] = 1.0;
  ncontrolv[7] = 1.0;
  /* inner points */
  j = 1;
  for(i = 2; i <= length; i++)
    {
      memcpy(&(ncontrolv[i*4]),&(controlv[j*3]),3*sizeof(double));
      ncontrolv[i*4+3] = 1.0;
      j++;
    }
  ncontrolv[((nlength-1)*4)-1] = 1.0;
  /* last point */
  memcpy(&(ncontrolv[(nlength-1)*4]),&(controlv[0]),3*sizeof(double));
  ncontrolv[((nlength-1)*4)+3] = 1.0;

  /* solve the interpolation problem */
  ay_status = ay_nb_GlobalInterpolation4DD(length, ncontrolv, vk,
					   knotv, deg, v1, v2);

  if(ay_status)
    { goto cleanup; }

  ay_status = ay_nct_create(order, nlength, AY_KTCUSTOM,
			    ncontrolv, knotv, &new);

  if(!ay_status)
    {
      /* return result */
      *c = new;

      new->type = AY_CTCLOSED;

      /* prevent cleanup code from doing something harmful */
      knotv = NULL;
      ncontrolv = NULL;
    }

cleanup:

  if(scontrolv)
    free(scontrolv);
  if(ncontrolv)
    free(ncontrolv);
  if(lengths)
    free(lengths);
  if(vk)
    free(vk);
  if(knotv)
    free(knotv);

 return ay_status;
} /* ay_ict_interpolateG3DClosed */


/* ay_ict_dumb_interpolate:
 *  left here for sentimental reasons, it was my first
 *  attempt, but it is just C1 and generates simply too
 *  much De-Boor-points
 */
/*
int
ay_ict_dumb_interpolate(double iparam, int closed, int length,
                        double *controlv,
		        ay_nurbcurve_object **c)
{
 int ay_status = AY_OK;
 int a,b,i,j;
 int nlength;
 double v[3], vlen = 0.0;
 double *ncontrolv = NULL;
 double *knotv, knot = 0.0, *lengths, chordlength = 0.0;
 ay_nurbcurve_object *new = NULL;


  if(closed)
    {
      nlength = length + length*4 - 1;
    }
  else
    {
      nlength = ((length-2)*3)+((length-1)*2)+2;
    }

  if(!(ncontrolv = calloc(nlength*3, sizeof(double))))
    return AY_EOMEM;

  if(!closed)
    {
      a = 6; b = 3;
      for(i = 1; i < (length-1); i++)
	{
	  for(j = 0; j < 5; j++)
	    {
	      memcpy(&(ncontrolv[a]), &(controlv[b]), 3*sizeof(double));
	      a += 3;
	    }

	  a -= 15;

	  v[0] = controlv[b+3]-controlv[b-3];
	  v[1] = controlv[b+4]-controlv[b-2];
	  v[2] = controlv[b+5]-controlv[b-1];

	  vlen = AY_V3LEN(v);
	  vlen *= iparam;

	  AY_V3SCAL(v, vlen)

	  ncontrolv[a]   -= v[0];
	  ncontrolv[a+1] -= v[1];
	  ncontrolv[a+2] -= v[2];

	  a += 12;
	  ncontrolv[a]   += v[0];
	  ncontrolv[a+1] += v[1];
	  ncontrolv[a+2] += v[2];

	  a += 3;
	  b += 3;
	}

      memcpy(&(ncontrolv[0]), &(controlv[0]), 3*sizeof(double));

      v[0] = ncontrolv[6] - ncontrolv[0];
      v[1] = ncontrolv[7] - ncontrolv[1];
      v[2] = ncontrolv[8] - ncontrolv[2];
      vlen = AY_V3LEN(v);
      vlen *= (iparam);

      AY_V3SCAL(v, vlen)

      ncontrolv[3] = ncontrolv[0] + v[0];
      ncontrolv[4] = ncontrolv[1] + v[1];
      ncontrolv[5] = ncontrolv[2] + v[2];

      a = (nlength-1)*3;
      b = (length-1)*3;
      memcpy(&(ncontrolv[a]), &(controlv[b]), 3*sizeof(double));

      v[0] = ncontrolv[a] - ncontrolv[a-6];
      v[1] = ncontrolv[a+1] - ncontrolv[a-5];
      v[2] = ncontrolv[a+2] - ncontrolv[a-4];
      vlen = AY_V3LEN(v);
      vlen *= (iparam);

      AY_V3SCAL(v, vlen)

      ncontrolv[a-3] = ncontrolv[a] - v[0];
      ncontrolv[a-2] = ncontrolv[a+1] - v[1];
      ncontrolv[a-1] = ncontrolv[a+2] - v[2];
    }
  else
    {
      memcpy(&(ncontrolv[0]), &(controlv[0]), 3*sizeof(double));
      b = length*3;
      v[0] = controlv[b-3]-controlv[3];
      v[1] = controlv[b-2]-controlv[4];
      v[2] = controlv[b-1]-controlv[5];
      vlen = AY_V3LEN(v);
      vlen *= iparam;

      AY_V3SCAL(v, vlen)

      ncontrolv[3] = controlv[0] - v[0];
      ncontrolv[4] = controlv[1] - v[1];
      ncontrolv[5] = controlv[2] - v[2];

      a = nlength*3;
      ncontrolv[a-6] = controlv[0] + v[0];
      ncontrolv[a-5] = controlv[1] + v[1];
      ncontrolv[a-4] = controlv[2] + v[2];

      memcpy(&(ncontrolv[(nlength*3)-3]), &(controlv[0]), 3*sizeof(double));

      a = 6; b = 3;
      for(i = 1; i < length; i++)
	{
	  for(j = 0; j < 5; j++)
	    {
	      memcpy(&(ncontrolv[a]), &(controlv[b]), 3*sizeof(double));
	      a += 3;
	    }

	  a -= 15;

	  if(i == (length - 1))
	    {
	      v[0] = controlv[0] - controlv[b-3];
	      v[1] = controlv[1] - controlv[b-2];
	      v[2] = controlv[2] - controlv[b-1];
	    }
	  else
	    {
	      v[0] = controlv[b+3] - controlv[b-3];
	      v[1] = controlv[b+4] - controlv[b-2];
	      v[2] = controlv[b+5] - controlv[b-1];
	    }


	  vlen = AY_V3LEN(v);
	  vlen *= iparam;

	  AY_V3SCAL(v, vlen)

	  ncontrolv[a]   -= v[0];
	  ncontrolv[a+1] -= v[1];
	  ncontrolv[a+2] -= v[2];

	  a += 12;
	  ncontrolv[a]   += v[0];
	  ncontrolv[a+1] += v[1];
	  ncontrolv[a+2] += v[2];

	  a += 3;
	  b += 3;
	}

    }


  if(!(lengths = calloc(nlength-1, sizeof(double))))
    return AY_EOMEM;

  if(!(knotv = calloc(nlength+4, sizeof(double))))
    return AY_EOMEM;

  a = 0;
  for(i = 0; i < (nlength-1); i++)
    {

      lengths[i] = AY_VLEN((ncontrolv[a+3] - ncontrolv[a]),
			     (ncontrolv[a+4] - ncontrolv[a+1]),
			     (ncontrolv[a+5] - ncontrolv[a+2]));

      chordlength += lengths[i];

      a += 3;
    }

  knotv[0] = 0.0;
  knotv[1] = 0.0;
  knotv[2] = 0.0;
  knotv[3] = 0.0;
  if(closed)
    j = 1;
  else
    j = 0;
  for(i = 4; i < (nlength+2); i++)
    {
      knot += lengths[j]/chordlength;
      knotv[i] = knot;
      j++;
    }
  knotv[nlength] = 1.0;
  knotv[nlength+1] = 1.0;
  knotv[nlength+2] = 1.0;
  knotv[nlength+3] = 1.0;

  free(lengths);

  ay_status = ay_nct_create(4, nlength, AY_KTCUSTOM, ncontrolv, knotv, &new);

  if(!ay_status)
    *c = new;

 return ay_status;
}*//* ay_ict_dumb_interpolate */


/** ay_ict_resize:
 * Resize an interpolating curve.
 *
 * \param[in,out] curve interpolating curve to process
 * \param[in] new_length desired new length
 *
 * \returns AY_OK on success, error code otherwise.
 */
int
ay_ict_resize(ay_icurve_object *curve, int new_length)
{
 int ay_status = AY_OK;
 int a, b, i, j;
 int *newpersec = NULL, new = 0;
 double *ncontrolv = NULL, v[3] = {0}, t = 0.0, *cv = NULL;

  if(new_length == curve->length)
    return ay_status;


  if(!(ncontrolv = calloc(3*new_length, sizeof(double))))
    return AY_EOMEM;

  if(new_length < curve->length)
    {
      a = 0;
      for(i = 0; i < new_length; i++)
	{
	  memcpy(&ncontrolv[a], &(curve->controlv[a]), 3*sizeof(double));
	  a+=3;
	}
    }
  else
    {
      /* distribute new points */
      new = new_length - curve->length;

      if(!(newpersec = calloc((curve->length-1), sizeof(int))))
	{free(ncontrolv); return AY_EOMEM;}
      cv = curve->controlv;

      while(new)
	{
	  for(i = 0; i < (curve->length-1); i++)
	    {
	      if((cv[i*3] != cv[(i+1)*3]) ||
		 (cv[i*3+1] != cv[(i+1)*3+1]) ||
		 (cv[i*3+2] != cv[(i+1)*3+2]))
		{
		  if(new)
		    {
		      (newpersec[i])++;
		      new--;
		    } /* if */
		} /* if */
	    } /* for */
	} /* while */

      a = 0;
      b = 0;
      for(i = 0; i < (curve->length-1); i++)
	{
	  memcpy(&ncontrolv[b], &(curve->controlv[a]), 3*sizeof(double));
	  b+=3;

	  if((cv[i*3] != cv[(i+1)*3]) ||
	     (cv[i*3+1] != cv[(i+1)*3+1]) ||
	     (cv[i*3+2] != cv[(i+1)*3+2]))
	    {
	      for(j = 1; j <= newpersec[i]; j++)
		{

		  v[0] = curve->controlv[a+3]   - curve->controlv[a];
		  v[1] = curve->controlv[a+3+1] - curve->controlv[a+1];
		  v[2] = curve->controlv[a+3+2] - curve->controlv[a+2];

		  t = j/(newpersec[i]+1.0);

		  AY_V3SCAL(v,t);

		  ncontrolv[b]   = curve->controlv[a]   + v[0];
		  ncontrolv[b+1] = curve->controlv[a+1] + v[1];
		  ncontrolv[b+2] = curve->controlv[a+2] + v[2];
		  ncontrolv[b+3] = 1.0;

		  b+=3;
		} /* for */
	    } /* if */

	  a+=3;
	} /* for */
      memcpy(&ncontrolv[(new_length-1)*3],
	     &(curve->controlv[(curve->length-1)*3]),
	     3*sizeof(double));

      free(newpersec);

    } /* if */

  free(curve->controlv);
  curve->controlv = ncontrolv;

  curve->length = new_length;

 return ay_status;
} /* ay_ict_resize */


/** ay_ict_revert:
 * Revert an interpolating curve.
 *
 * \param[in,out] curve interpolating curve to process
 *
 * \returns AY_OK on success, error code otherwise.
 */
int
ay_ict_revert(ay_icurve_object *curve)
{
 int i, j;
 double dtemp;
 double t[3];

  if(!curve)
    return AY_ENULL;

  /* revert control */
  j = (curve->length - 1)*3;
  i = 0;
  while(i < j)
    {
      dtemp = curve->controlv[j];
      curve->controlv[j] = curve->controlv[i];
      curve->controlv[i] = dtemp;

      dtemp = curve->controlv[j+1];
      curve->controlv[j+1] = curve->controlv[i+1];
      curve->controlv[i+1] = dtemp;

      dtemp = curve->controlv[j+2];
      curve->controlv[j+2] = curve->controlv[i+2];
      curve->controlv[i+2] = dtemp;

      i+=3;
      j-=3;
    } /* while */

  /* revert derivatives */
  memcpy(t, curve->sderiv, 3*sizeof(double));
  memcpy(curve->sderiv, curve->ederiv, 3*sizeof(double));
  memcpy(curve->ederiv, t, 3*sizeof(double));

  dtemp = curve->sdlen;
  curve->sdlen = curve->edlen;
  curve->edlen = dtemp;

 return AY_OK;
} /* ay_ict_revert */


/** ay_ict_getpntfromindex:
 * Get address of a single control point from its indices
 * (performing bounds checking).
 *
 * \param[in] curve interpolating curve object to process
 * \param[in] index index of desired control point
 * \param[in,out] p address where to store the resulting address
 *
 * \returns AY_OK on success, error code otherwise.
 */
int
ay_ict_getpntfromindex(ay_icurve_object *curve, int index, double **p)
{
 int stride = 3;
 char fname[] = "ict_getpntfromindex";

  if(!curve || !p)
    return AY_ENULL;

  if(index > (curve->length+1) || index < 0)
    return ay_error_reportirange(fname, "\"index\"", 0, curve->length+1);

  if(index < curve->length)
    {
      *p = &(curve->controlv[index*stride]);
    }
  else
    {
      /* allow access to end derivative */
      if(index == curve->length)
	{
	  *p = &(curve->sderiv[0]);
	}
      else
	{
	  *p = &(curve->ederiv[0]);
	}
    }

 return AY_OK;
} /* ay_ict_getpntfromindex */
