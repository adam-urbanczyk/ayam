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

/* ict.c - icurve tools */

/* ay_ict_interpolateC2C:
 *  C2 Cubic interpolation of length points
 *  in controlv, creates a NURBS curve and
 *  saves a pointer to it to c
 */
int
ay_ict_interpolateC2C(double iparam, int closed, int length,
		      double *controlv, ay_nurbcurve_object **c)
{
 int ay_status = AY_OK;
 int a,b,d,i,j;
 int nlength;
 double v[3], vlen = 0.0;
 double *ncontrolv = NULL, *ccontrolv = NULL, *ncv4D = NULL;
 double *knotv, knot = 0.0, *lengths, chordlength = 0.0;
 ay_nurbcurve_object *new = NULL;

  nlength = length+2;
  
  if(closed)
    {
      nlength++;
      if(!(ccontrolv = calloc((length+1)*3, sizeof(double))))
	return AY_EOMEM;

      memcpy(&(ccontrolv[0]),&(controlv[0]),length*3*sizeof(double));
      memcpy(&(ccontrolv[length*3]),&(controlv[0]),3*sizeof(double));

    }
 
  if(!(ncontrolv = calloc(nlength*3, sizeof(double))))
    return AY_EOMEM;

  /* create knotvector */
  if(!(knotv = calloc(nlength+4, sizeof(double))))
    return AY_EOMEM;

  a = 0;
  if(!closed)
    {

      if(!(lengths = calloc(length-1, sizeof(double))))
	return AY_EOMEM;
 


      for(i = 0; i < (length-1); i++)
	{
      
	  lengths[i] = AY_VLEN((controlv[a+3] - controlv[a]),
				 (controlv[a+4] - controlv[a+1]),
				 (controlv[a+5] - controlv[a+2]));

	  chordlength += lengths[i];
	  a += 3;
	}
    }
  else
    {
      if(!(lengths = calloc(length, sizeof(double))))
	return AY_EOMEM;
      for(i = 0; i < length; i++)
	{
      
	  lengths[i] = AY_VLEN((ccontrolv[a+3] - ccontrolv[a]),
				 (ccontrolv[a+4] - ccontrolv[a+1]),
				 (ccontrolv[a+5] - ccontrolv[a+2]));

	  chordlength += lengths[i];
	  a += 3;
	}
    }

  for(i = 0; i < 4; i++)
    knotv[i] = 0.0;
  j = 0;
  knot = 0.0;

  for(i = 4; i < nlength; i++)
    {
      knot += lengths[j]/chordlength/*1.0/(nlength-3)*/;
      knotv[i] = knot;
      j++;
    }
  for(i = nlength; i < nlength+4; i++)
    knotv[i] = 1.0;

  free(lengths);


  /* create first two and last two controls */
  if(!closed)
    {
      ncontrolv[0] = controlv[0];
      ncontrolv[1] = controlv[1];
      ncontrolv[2] = controlv[2];
      
      v[0] = controlv[3]-controlv[0];
      v[1] = controlv[4]-controlv[1];
      v[2] = controlv[5]-controlv[2];

      vlen = AY_V3LEN(v);
      vlen *= iparam;

      AY_V3SCAL(v, vlen)

      ncontrolv[3] = controlv[0]+v[0];
      ncontrolv[4] = controlv[1]+v[1];
      ncontrolv[5] = controlv[2]+v[2];

      a = (nlength-2)*3;
      b = (length-2)*3;

      v[0] = controlv[b+3]-controlv[b];
      v[1] = controlv[b+4]-controlv[b+1];
      v[2] = controlv[b+5]-controlv[b+2];

      vlen = AY_V3LEN(v);
      vlen *= iparam;

      AY_V3SCAL(v, vlen)

      ncontrolv[a]   = controlv[b+3]-v[0];
      ncontrolv[a+1] = controlv[b+4]-v[1];
      ncontrolv[a+2] = controlv[b+5]-v[2];

      a = (nlength-1)*3;
      b = (length-1)*3;
      ncontrolv[a]   = controlv[b];
      ncontrolv[a+1] = controlv[b+1];
      ncontrolv[a+2] = controlv[b+2];
    }
  else
    {
      ncontrolv[0] = controlv[0];
      ncontrolv[1] = controlv[1];
      ncontrolv[2] = controlv[2];

      b = (length-1)*3;
      v[0] = controlv[3]-controlv[b];
      v[1] = controlv[4]-controlv[b+1];
      v[2] = controlv[5]-controlv[b+2];

      vlen = AY_V3LEN(v);
      vlen *= iparam;

      AY_V3SCAL(v, vlen)

      ncontrolv[3] = controlv[0]+v[0];
      ncontrolv[4] = controlv[1]+v[1];
      ncontrolv[5] = controlv[2]+v[2];

      a = (nlength-2)*3;
      ncontrolv[a]   = controlv[0]-v[0];
      ncontrolv[a+1] = controlv[1]-v[1];
      ncontrolv[a+2] = controlv[2]-v[2];

      a = (nlength-1)*3;
      ncontrolv[a]   = controlv[0];
      ncontrolv[a+1] = controlv[1];
      ncontrolv[a+2] = controlv[2];
    }

  if(!closed)
    {
      ay_status = ay_nb_SolveTridiagonal(length-1, controlv, knotv,
					 ncontrolv);
    }
  else
    {
      ay_status = ay_nb_SolveTridiagonal(length, ccontrolv, knotv,
					 ncontrolv);
      free(ccontrolv);
    }


  /* copy to 4D controlv */
  if(!(ncv4D = calloc(nlength*4, sizeof(double))))
    return AY_EOMEM;
  a = 3; b = 0; d = 0;
  for(i = 0; i < nlength; i++)
    {
      memcpy(&(ncv4D[b]),&(ncontrolv[d]),3*sizeof(double));
      ncv4D[a] = 1.0;
      a+=4;
      b+=4;
      d+=3;
    }


  ay_status = ay_nct_create(4, nlength, AY_KTCUSTOM, ncv4D, knotv, &new);

  if(!ay_status)
    *c = new;
  
  free(ncontrolv);

 return ay_status;
} /* ay_ict_interpolateC2C */


/* ay_ict_interpolateG4D:
 *  Global 4D interpolation of length points
 *  in controlv, creates a NURBS curve and
 *  saves a pointer to it to c
 */
int
ay_ict_interpolateG4D(int iorder, int length,
		      double *controlv, ay_nurbcurve_object **c)
{
 int ay_status = AY_OK;
 int a,i,j,k,order/*,iN,index*/;
 int nlength;
 double *ncontrolv = NULL;
 double *knotv, knot = 0.0, *lengths, chordlength = 0.0, *vk = NULL;
 ay_nurbcurve_object *new = NULL;


  nlength = length;
  
  if(iorder > nlength)
    order = nlength;
  else
    order = iorder;

  if(!(ncontrolv = calloc(nlength*4, sizeof(double))))
    return AY_EOMEM;

  /* create knotvector */
  if(!(knotv = calloc(nlength+order, sizeof(double))))
    return AY_EOMEM;

  a = 0;
  if(!(lengths = calloc(length-1, sizeof(double))))
    return AY_EOMEM;
  for(i = 0; i < (length-1); i++)
    {
      
      lengths[i] = AY_VLEN((controlv[a+3] - controlv[a]),
			     (controlv[a+4] - controlv[a+1]),
			     (controlv[a+5] - controlv[a+2]));

      chordlength += lengths[i];

      a += 3;
    }

  if(!(vk = calloc(length, sizeof(double))))
    return AY_EOMEM;

  vk[0] = 0.0;
  j = 0;
  knot = 0.0;
  for(i = 1; i < (length-1); i++)
    {
      knot += lengths[j]/chordlength;
      vk[i] = knot;
      j++;
    }
  vk[length-1] = 1.0;
  
  for(i = 1; i < nlength-(order-1); i++)
    {
      knotv[i+(order-1)] = 0.0;
      for(k = i; k < (i+(order-1)); k++)
	{
	  knotv[i+(order-1)] += vk[k];
	}
      knotv[i+(order-1)] /= (order-1);
    }

  for(i = 0; i < order; i++)
    knotv[i] = 0.0;
  for(i = nlength; i < nlength+order; i++)
    knotv[i] = 1.0;

  free(lengths);

  for(i = 0; i < length; i++)
    {
      memcpy(&(ncontrolv[i*4]),&(controlv[i*3]),3*sizeof(double));
      ncontrolv[i*4+3] = 1.0;
    }

  ay_status = ay_nb_GlobalInterpolation4D(nlength-1, ncontrolv, vk,
					      knotv, order-1);

  if(ay_status)
    { free(vk); free(ncontrolv); free(knotv); return ay_status; }

  ay_status = ay_nct_create(order, nlength, AY_KTCUSTOM, ncontrolv,
			    knotv, &new);
  
  if(!ay_status)
    *c = new;

  free(vk);

 return ay_status;
} /* ay_ict_interpolateG4D */


/* ay_ict_interpolateG4DC:
 *  Closed Global 4D interpolation of length points
 *  in controlv, creates a NURBS curve and
 *  saves a pointer to it to c
 */
int
ay_ict_interpolateG4DC(int iorder, int length, double iparam,
		       double *controlv, ay_nurbcurve_object **c)
{
 int ay_status = AY_OK;
 int a, i, j, order = 0, iN, index;
 int nlength;
 double *ncontrolv = NULL;
 double *knotv, knot = 0.0, *lengths, chordlength = 0.0, *vk = NULL;
 double D[3] = {0};
 ay_nurbcurve_object *new = NULL;

  if(iorder > (length+3))
    order = length + 3;
  else
    order = iorder;

  nlength = length+3;

  if(!(ncontrolv = calloc(nlength*4, sizeof(double))))
    return AY_EOMEM;

  if(!(knotv = calloc(nlength+order, sizeof(double))))
    return AY_EOMEM;

  if(!(lengths = calloc(length, sizeof(double))))
    return AY_EOMEM;

  a = 0;
  for(i = 0; i < length-1; i++)
    {
      
      lengths[i] = AY_VLEN((controlv[a+3] - controlv[a]),
			     (controlv[a+4] - controlv[a+1]),
			     (controlv[a+5] - controlv[a+2]));

      chordlength += lengths[i];

      a += 3;
    }
  a = (length-1)*3;
  lengths[length-1] = AY_VLEN((controlv[0] - controlv[a]),
				(controlv[1] - controlv[a+1]),
				(controlv[2] - controlv[a+2]));

  chordlength += lengths[length-1];

  /* derivative */
  D[0] = controlv[3] - controlv[a];
  D[1] = controlv[4] - controlv[a+1];
  D[2] = controlv[5] - controlv[a+2];

  AY_V3SCAL(D,iparam)

  /* calc parametric values */
  if(!(vk = calloc(length+1, sizeof(double))))
    return AY_EOMEM;

  vk[0] = 0.0;
  j = 0;
  knot = 0.0;
  for(i = 1; i < (length+1); i++)
    {
      knot += lengths[j]/chordlength;
      vk[i] = knot;
      j++;
    }
  vk[length] = 1.0;

  iN = length+1-(order-1)+1;
  for(j = 0; j <= iN; j++)
    {
      index = j+order;
      knotv[index] = 0.0;
	for(i = j; i <= j+(order-1)-1; i++)
	  {
	    knotv[index] += vk[i];
	  }
      knotv[index] /= (order - 1);
    }

  for(i = 0; i < order; i++)
    knotv[i] = 0.0;
  for(i = nlength; i < nlength+order; i++)
    knotv[i] = 1.0;

  free(lengths);

  /* first point */
  memcpy(ncontrolv,controlv,3*sizeof(double));
  ncontrolv[3] = 1.0;
  ncontrolv[7] = 1.0;
  /* inner points */
  j=1;
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

  ay_status = ay_nb_GlobalInterpolation4DD(length, ncontrolv, vk,
					       knotv, order-1, D, D);

  if(ay_status)
    { free(vk); free(ncontrolv); free(knotv); return ay_status; }

  ay_status = ay_nct_create(order, nlength, AY_KTCUSTOM,
			    ncontrolv, knotv, &new);
  
  if(!ay_status)
    *c = new;

  free(vk);

 return ay_status;
} /* ay_ict_interpolateG4DC */


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


/* ay_ict_resize:
 *  resize an interpolating curve
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
      for(i=0; i<new_length; i++)
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
	return AY_EOMEM;
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

		  v[0] = curve->controlv[a+3] - curve->controlv[a];
		  v[1] = curve->controlv[a+3+1] - curve->controlv[a+1];
		  v[2] = curve->controlv[a+3+2] - curve->controlv[a+2];

		  t = j/(newpersec[i]+1.0);
	      
		  AY_V3SCAL(v,t);

		  ncontrolv[b] = curve->controlv[a]+v[0];
		  ncontrolv[b+1] = curve->controlv[a+1]+v[1];
		  ncontrolv[b+2] = curve->controlv[a+2]+v[2];
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


/* ay_ict_revert:
 *  revert an interpolating curve
 */
int
ay_ict_revert(ay_icurve_object *curve)
{
 int i, j;
 double dtemp;

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

 return AY_OK;
} /* ay_ict_revert */


/* ay_ict_reverttcmd:
 *  revert an interpolating curve
 */
int
ay_ict_reverttcmd(ClientData clientData, Tcl_Interp *interp,
		  int argc, char *argv[])
{
 int ay_status;
 ay_list_object *sel = ay_selection;
 ay_object *src = NULL;
 ay_icurve_object *curve = NULL;
 char fname[] = "revert_icurve";

  if(!sel)
    {
      ay_error(AY_ENOSEL, fname, NULL);
      return TCL_OK;
    }

  while(sel)
    {
      src = sel->object;
      if(src->type != AY_IDICURVE)
	{
	  ay_error(AY_ERROR, fname, "Object is not an ICurve!");
	}
      else
	{
	  if(src->selp)
	    {
	      ay_selp_clear(src);
	    }

	  curve = (ay_icurve_object*)src->refine;

	  ay_status = ay_ict_revert(curve);
	  if(ay_status)
	    {
	      ay_error(ay_status, fname, "Could not revert ICurve!");
	    }


	} /* if */

      sel = sel->next;
    } /* while */

  ay_notify_parent();

 return TCL_OK;
} /* ay_ict_reverttcmd */
