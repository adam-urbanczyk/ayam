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

/* geom.c - miscellaneous small geometric algorithms */


/* ay_geom_intersectlines3D:
 *  calculate intersection of the two 3D lines <p1><t1> and <p2>t2>
 *  (where p is a point on the line and t is the tangent)
 *  outputs the point in <p>
 *  returns 0 when no intersection exists (colinear/parallel lines)
 *  otherwise returns 1
 */
int
ay_geom_intersectlines3D(double *p1, double *t1,
			 double *p2, double *t2,
			 double *p)
{
 double v[3] = {0}, px[3] = {0};
 double pd[3] = {0};
 double vw = 0.0, vw2 = 0.0;

  if(!p1 || !t1 || !p2 || !t2 || !p)
   return 0;

  AY_V3CROSS(px, t1, t2)
  AY_V3CROSS(v, px, t1)
  vw = AY_V3DOT(v, t2);

  if((vw*vw) < 1e-7)
    return 0;

  AY_V3SUB(pd, p1, p2)
  vw2 = AY_V3DOT(v, pd);
  p[0] = p2[0]+(vw2/vw)*t2[0];
  p[1] = p2[1]+(vw2/vw)*t2[1];
  p[2] = p2[2]+(vw2/vw)*t2[2];

 return 1;
} /* ay_geom_intersectlines3D */


/* ay_geom_intersectlines2D:
 *  calculate intersection of the two 2D lines <p1><t1> and <p2>t2>
 *  (where p is a point on the line and t is the tangent)
 *  outputs the point in <p>
 *  returns 0 when no intersection exists (colinear/parallel lines)
 *  otherwise returns 1
 */
int
ay_geom_intersectlines2D(double *p1, double *t1,
			 double *p2, double *t2,
			 double *p)
{
 double ua, /*ub,*/ den, nomua, nomub;

  if(!p1 || !t1 || !p2 || !t2 || !p)
   return 0;

  den = t2[1]*t1[0] - t2[0]*t1[1];

  if(fabs(den) < AY_EPSILON)
    return 0;

  nomua = (t2[0]*(p1[1]-p2[1]) - t2[1]*(p1[0]-p2[0]));
  nomub = (t1[0]*(p1[1]-p2[1]) - t1[1]*(p1[0]-p2[0]));

  if((fabs(den) < AY_EPSILON) &&
     (fabs(nomua) < AY_EPSILON) &&
     (fabs(nomub) < AY_EPSILON))
    return 0;

  ua = nomua/den;
  /*ub = nomub/den;*/

  p[0] = p1[0] + ua*t1[0];
  p[1] = p1[1] + ua*t1[1];

 return 1;
} /* ay_geom_intersectlines2D */


/* ay_geom_calcnfrom3:
 *  calculate normal from three 3D points <p1>, <p2>, and <p3>
 *  returns the normal in <n>
 */
void
ay_geom_calcnfrom3(double *p1, double *p2, double *p3, double *n)
{
 double v1[3] = {0}, v2[3] = {0}, len = 0.0;

  if(!p1 || !p2 || !p3 || !n)
   return;

  AY_V3SUB(v2, p1, p2)
  AY_V3SUB(v1, p3, p2)
  AY_V3CROSS(n, v1, v2)
  len = AY_V3LEN(n);
  AY_V3SCAL(n, 1.0/len)

 return;
} /* ay_geom_calcnfrom3 */


/* ay_geom_extractmiddlepoint:
 */
int
ay_geom_extractmiddlepoint(int mode, double *cv, int cvlen, int cvstride,
			  double **tcv, double *result)
{
 int ay_status = AY_OK;
 int stride = 4, a, i, j;
 double *p = NULL, *t = NULL;
 double minmax[6];

  if(!result)
    return AY_ENULL;

  memset(result, 0, 4*sizeof(double));

  if(mode == 0)
    {
      minmax[0] = DBL_MAX;
      minmax[1] = -DBL_MAX;
      minmax[2] = DBL_MAX;
      minmax[3] = -DBL_MAX;
      minmax[4] = DBL_MAX;
      minmax[5] = -DBL_MAX;
      p = cv;
      for(i = 0; i < cvlen; i++)
	{
	  if(p[0] < minmax[0])
	    minmax[0] = p[0];
	  if(p[0] > minmax[1])
	    minmax[1] = p[0];

	  if(p[1] < minmax[2])
	    minmax[2] = p[1];
	  if(p[1] > minmax[3])
	    minmax[3] = p[1];

	  if(p[2] < minmax[4])
	    minmax[4] = p[2];
	  if(p[2] > minmax[5])
	    minmax[5] = p[2];

	  p += cvstride;
	} /* for */

      result[0] = minmax[0]+((minmax[1]-minmax[0])*0.5);
      result[1] = minmax[2]+((minmax[3]-minmax[2])*0.5);
      result[2] = minmax[4]+((minmax[5]-minmax[4])*0.5);
    }
  else
    {
      /* mode != 0 */
      if(!tcv)
	return AY_ENULL;

      if(!*tcv)
	if(!(*tcv = malloc(cvlen*stride*sizeof(double))))
	  return AY_EOMEM;
      t = *tcv;
      p = cv;
      for(i = 0; i < cvlen; i++)
	{
	  memcpy(&(t[i*stride]), p, stride*sizeof(double));
	  p += cvstride;
	}

      qsort(t, cvlen, stride*sizeof(double), ay_nct_cmppnt);

      a = 0;
      i = 0;
      j = cvlen;
      while(i < cvlen)
	{
	  result[0] += t[a];
	  result[1] += t[a+1];
	  result[2] += t[a+2];
	  result[3] += t[a+3];

	  /* skip over sequence of equal points */
	  if((i < (cvlen-1)) &&
	     !ay_nct_cmppnt((void*)(&(t[a])),(void*)(&(t[a+stride]))))
	    {
	      do
		{
		  i++;
		  a += stride;
		  j--;
		}
	      while((i < (cvlen-1)) &&
		!ay_nct_cmppnt((void*)(&(t[a])),(void*)(&(t[a+stride]))));
	    }

	  i++;
	  a += stride;
	} /* while */

      result[0] /= j;
      result[1] /= j;
      result[2] /= j;
      result[3] /= j;
    } /* if */

 return ay_status;
} /* ay_geom_extractmiddlepoint */


/* ay_geom_extractmeannormal:
 */
int
ay_geom_extractmeannormal(double *cv, int cvlen, int cvstride,
			  double *result)
{
 int ay_status = AY_OK;
 int i, snlen = 0;
 double m[4] = {0}, *p1, *p2, *psn;
 double len, *sn = NULL;

  ay_status = ay_geom_extractmiddlepoint(0, cv, cvlen,
					 cvstride, NULL, m);

  if(ay_status)
    return ay_status;

  if(!(sn = malloc(cvlen*3*sizeof(double))))
    return AY_EOMEM;

  psn = sn;
  p1 = cv;
  p2 = p1+cvstride;

  for(i = 0; i < cvlen-1; i++)
    {
      if((!AY_V3COMP(p1,p2)) && (!AY_V3COMP(m,p1)) && (!AY_V3COMP(m,p2)))
	{
	  ay_geom_calcnfrom3(m, p1, p2, psn);
	  len = AY_V3LEN(psn);
	  if(fabs(len) > AY_EPSILON)
	    AY_V3SCAL(psn,1.0/len);
	  snlen++;
	  psn += 3;
	}

      p1 = p2;
      p2 += cvstride;
    }

  psn = sn;
  for(i = 0; i < snlen; i++)
    {
      result[0] += psn[0]/snlen;
      result[1] += psn[1]/snlen;
      result[2] += psn[2]/snlen;
      psn += 3;
    }

  len = AY_V3LEN(result);
  if(fabs(len) > AY_EPSILON)
    AY_V3SCAL(result,1.0/len);

  free(sn);

 return ay_status;
} /* ay_geom_extractmeannormal */


/* ay_geom_meandist:
 */
void
ay_geom_meandist(double *cv1, int cv1stride,
		 double *cv2, int cv2stride,
		 int cvlen, double *result)
{
 int i;
 double v[3], vlen;

  *result = 0.0;

  for(i = 0; i < cvlen; i++)
    {
      if(!AY_V3COMP(cv1,cv2))
	{
	  AY_V3SUB(v,cv1,cv2);
	  vlen = AY_V3LEN(v);
	  *result += vlen/cvlen;
	}

      cv1 += cv1stride;
      cv2 += cv2stride;
    }

 return;
} /* ay_geom_meandist */
