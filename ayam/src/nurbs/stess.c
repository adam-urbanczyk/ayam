/* simple NURB tesselators */

#include "mops.h"

typedef struct stess_uvp_s {
  struct stess_uvp_s *next;
  int type; /* 0 - original point, 1 - trimloop point */
  int dir;  /* direction of associated trimcurve, 1 - ccw, 0 - cw */
  double u, v;
  double C[4];
} stess_uvp;

typedef struct stess_uvpl_s {
  struct stess_uvpl_s *next;
  struct stess_uvp_s *uvp;
} stess_uvpl;

#define STESS_EPSILON 0.000001

int
mops_nb_2DCurvePoints4D(int n, int p, double *U, double *Pw, int num,
			int *Clen, double **C)
{
 int span, j, k, l, m;
 double *N = NULL, Cw[3], *Ct = NULL, u, ud, umin, umax; 

  if(!(N = calloc(p+1,sizeof(double))))
    return MOPS_OUTOFMEM;

  j = 0;
  while(U[j] == U[j+1])
    {
      j++;
    }
  umin = U[j];

  j = p+n;
  while(U[j] == U[j-1])
    {
      j--;
    }
  umax = U[j];

  *Clen = (n*num);

  if(!(Ct = calloc(*Clen*2, sizeof(double))))
    return MOPS_OUTOFMEM;

  ud = (umax-umin)/((*Clen)-1);
  u = umin;

  for(l=0; l<(*Clen); l++)
    {
      span = mops_nb_FindSpan(n,p,u,U);

      mops_nb_BasisFuns(span,u,p,U,N);
      
      memset(Cw,0,3*sizeof(double));

      for(j=0; j<=p; j++)
	{
	  k = (span-p+j)*4;
	  Cw[0] = Cw[0] + N[j]*Pw[k];
	  Cw[1] = Cw[1] + N[j]*Pw[k+1];
	  Cw[2] = Cw[2] + N[j]*Pw[k+3];
	}

	m = l*2;
	Ct[m]   = Cw[0]/Cw[2];
	Ct[m+1] = Cw[1]/Cw[2];

      u += ud;
    }

  *C = Ct;

  free(N);

 return MOPS_OK;
}


int
mops_nb_2DCurvePoints3D(int n, int p, double *U, double *P, int num,
		      int *Clen, double **C)
{
 int span, j, k, l, m;
 double *N = NULL, *Ct = NULL, u, ud, umin, umax; 

  if(!(N = calloc(p+1,sizeof(double))))
    return MOPS_OUTOFMEM;

  j = 0;
  while(U[j] == U[j+1])
    {
      j++;
    }
  umin = U[j];

  j = p+n;
  while(U[j] == U[j-1])
    {
      j--;
    }
  umax = U[j];

  *Clen = n*num;

  if(!(Ct = calloc(*Clen*2, sizeof(double))))
    return MOPS_OUTOFMEM;

  ud = (umax-umin)/((*Clen)-1);
  u = umin;

  for(l = 0; l < (*Clen); l++)
    {
      span = mops_nb_FindSpan(n,p,u,U);

      mops_nb_BasisFuns(span,u,p,U,N);
      
      m = l*2;
      for(j=0; j<=p; j++)
	{
	  k = (span-p+j)*3;
	  Ct[m]   = Ct[m]   + N[j]*P[k];
	  Ct[m+1] = Ct[m+1] + N[j]*P[k+1];
	}

      u += ud;
    }

  *C = Ct;

  free(N);

 return MOPS_OK;
}


int
mops_nb_CurvePoints4D(int n, int p, double *U, double *Pw, int num,
		      int *Clen, double **C)
{
 int span, j, k, l, m;
 double *N = NULL, Cw[4], *Ct = NULL, u, ud, umin, umax; 

  if(!(N = calloc(p+1,sizeof(double))))
    return MOPS_OUTOFMEM;

  j = 0;
  while(U[j] == U[j+1])
    {
      j++;
    }
  umin = U[j];

  j = p+n;
  while(U[j] == U[j-1])
    {
      j--;
    }
  umax = U[j];

  *Clen = (n*num);

  if(!(Ct = calloc(*Clen*3, sizeof(double))))
    return MOPS_OUTOFMEM;

  ud = (umax-umin)/((*Clen)-1);
  u = umin;

  for(l=0; l<(*Clen); l++)
    {
      span = mops_nb_FindSpan(n,p,u,U);

      mops_nb_BasisFuns(span,u,p,U,N);
      
      memset(Cw,0,4*sizeof(double));

      for(j=0; j<=p; j++)
	{
	  k = (span-p+j)*4;
	  Cw[0] = Cw[0] + N[j]*Pw[k];
	  Cw[1] = Cw[1] + N[j]*Pw[k+1];
	  Cw[2] = Cw[2] + N[j]*Pw[k+2];
	  Cw[3] = Cw[3] + N[j]*Pw[k+3];
	}

	m = l*3;
	Ct[m]   = Cw[0]/Cw[3];
	Ct[m+1] = Cw[1]/Cw[3];
	Ct[m+2] = Cw[2]/Cw[3];

      u += ud;
    }

  *C = Ct;

  free(N);

 return MOPS_OK;
}


int
mops_nb_CurvePoints3D(int n, int p, double *U, double *P, int num,
		      int *Clen, double **C)
{
 int span, j, k, l, m;
 double *N = NULL, *Ct = NULL, u, ud, umin, umax; 

  if(!(N = calloc(p+1,sizeof(double))))
    return MOPS_OUTOFMEM;

  j = 0;
  while(U[j] == U[j+1])
    {
      j++;
    }
  umin = U[j];

  j = p+n;
  while(U[j] == U[j-1])
    {
      j--;
    }
  umax = U[j];

  *Clen = n*num;

  if(!(Ct = calloc(*Clen*3, sizeof(double))))
    return MOPS_OUTOFMEM;

  ud = (umax-umin)/((*Clen)-1);
  u = umin;

  for(l = 0; l < (*Clen); l++)
    {
      span = mops_nb_FindSpan(n,p,u,U);

      mops_nb_BasisFuns(span,u,p,U,N);
      
      m = l*3;
      for(j=0; j<=p; j++)
	{
	  k = (span-p+j)*3;
	  Ct[m]   = Ct[m]   + N[j]*P[k];
	  Ct[m+1] = Ct[m+1] + N[j]*P[k+1];
	  Ct[m+2] = Ct[m+2] + N[j]*P[k+2];
	}

      u += ud;
    }

  *C = Ct;

  free(N);

 return MOPS_OK;
}


int
mops_nb_SurfacePoints4D(int n, int m, int p, int q, double *U, double *V,
			double *Pw, int num, int *Cn, int *Cm, double **C)
{
 int spanu = 0, spanv = 0, indu = 0, indv = 0, l = 0, k = 0, i = 0, j = 0;
 int a, b;
 double u, v, ud, vd, umin, umax, vmin, vmax, *Nu = NULL, *Nv = NULL;
 double Cw[4] = {0}, *Ct = NULL, temp[4] = {0}; 

  if(!(Nu = calloc(p+1,sizeof(double))))
    return MOPS_OUTOFMEM;
  if(!(Nv = calloc(q+1,sizeof(double))))
    return MOPS_OUTOFMEM;

  j = 0;
  while(U[j] == U[j+1])
    {
      j++;
    }
  umin = U[j];

  j = p+n;
  while(U[j] == U[j-1])
    {
      j--;
    }
  umax = U[j];

  *Cn = n*num;
  ud = (umax-umin)/((*Cn)-1);
  u = umin;

  j = 0;
  while(V[j] == V[j+1])
    {
      j++;
    }
  vmin = V[j];

  j = q+m;
  while(V[j] == V[j-1])
    {
      j--;
    }
  vmax = V[j];

  *Cm = m*num;
  vd = (umax-umin)/((*Cm)-1);

  if(!(Ct = calloc((*Cn)*(*Cm)*3, sizeof(double))))
    return MOPS_OUTOFMEM;

  for(a = 0; a < (*Cn); a++)
    {
      v = vmin;
      for(b = 0; b < (*Cm); b++)
	{

	  spanu = mops_nb_FindSpan(n,p,u,U);
	  mops_nb_BasisFuns(spanu,u,p,U,Nu);
	  spanv = mops_nb_FindSpan(m,q,v,V);
	  mops_nb_BasisFuns(spanv,v,q,V,Nv);

	  indu = spanu - p;
	  for(l=0; l<=q; l++)
	    {
	      memset(temp, 0, 4*sizeof(double));
	      indv = spanv - q + l;

	      for(k=0; k<=p; k++)
		{
		  /* was: temp = temp + Nu[k]*Pw[indu+k][indv]; */
		  i = (((indu+k)*(m+1))+indv)*4;

		  temp[0] += Nu[k]*Pw[i];
		  temp[1] += Nu[k]*Pw[i+1];
		  temp[2] += Nu[k]*Pw[i+2];
		  temp[3] += Nu[k]*Pw[i+3];
		}
	      /* was: Cw = Cw + Nv[l]*temp */
	      Cw[0] += Nv[l]*temp[0];
	      Cw[1] += Nv[l]*temp[1];
	      Cw[2] += Nv[l]*temp[2];
	      Cw[3] += Nv[l]*temp[3];
	    }
  
	  j = (a*(*Cn)+b)*3;
	  Ct[j]   = Cw[0]/Cw[3];
	  Ct[j+1] = Cw[1]/Cw[3];
	  Ct[j+2] = Cw[2]/Cw[3];
	  
	  v += vd;  
	}

      u += ud;
    }

  *C = Ct;

  free(Nu);
  free(Nv);

 return MOPS_OK;
}


int
mops_nb_SurfacePoints3D(int n, int m, int p, int q, double *U, double *V,
			double *P, int num, int *Cn, int *Cm, double **C)
{
 int spanu = 0, spanv = 0, indu = 0, indv = 0, l = 0, k = 0, i = 0, j = 0;
 int a, b;
 double u, v, ud, vd, umin, umax, vmin, vmax, *Nu = NULL, *Nv = NULL;
 double temp[3] = {0}, *Ct = NULL; 

  if(!(Nu = calloc(p+1,sizeof(double))))
    return MOPS_OUTOFMEM;
  if(!(Nv = calloc(q+1,sizeof(double))))
    return MOPS_OUTOFMEM;

  j = 0;
  while(U[j] == U[j+1])
    {
      j++;
    }
  umin = U[j];

  j = p+n;
  while(U[j] == U[j-1])
    {
      j--;
    }
  umax = U[j];

  *Cn = n*num;
  ud = (umax-umin)/((*Cn)-1);
  u = umin;

  j = 0;
  while(V[j] == V[j+1])
    {
      j++;
    }
  vmin = V[j];

  j = q+m;
  while(V[j] == V[j-1])
    {
      j--;
    }
  vmax = V[j];

  *Cm = m*num;
  vd = (umax-umin)/((*Cm)-1);

  if(!(Ct = calloc((*Cn)*(*Cm)*3, sizeof(double))))
    return MOPS_OUTOFMEM;

  for(a = 0; a < (*Cn); a++)
    {
      v = vmin;
      for(b = 0; b < (*Cm); b++)
	{

	  spanu = mops_nb_FindSpan(n,p,u,U);
	  mops_nb_BasisFuns(spanu,u,p,U,Nu);
	  spanv = mops_nb_FindSpan(m,q,v,V);
	  mops_nb_BasisFuns(spanv,v,q,V,Nv);

	  indu = spanu - p;
	  j = (a*(*Cn)+b)*3;
	  for(l=0; l<=q; l++)
	    {
	      memset(temp, 0, 3*sizeof(double));
	      indv = spanv - q + l;

	      for(k=0; k<=p; k++)
		{
		  /* was: temp = temp + Nu[k]*P[indu+k][indv]; */
		  i = (((indu+k)*(m+1))+indv)*3;

		  temp[0] += Nu[k]*P[i];
		  temp[1] += Nu[k]*P[i+1];
		  temp[2] += Nu[k]*P[i+2];
		}
	      /* was: Cw = Cw + Nv[l]*temp */
	      Ct[j]   += Nv[l]*temp[0];
	      Ct[j+1] += Nv[l]*temp[1];
	      Ct[j+2] += Nv[l]*temp[2];
	    }
  
	  v += vd;  
	}

      u += ud;
    }

  *C = Ct;

  free(Nu);
  free(Nv);

 return MOPS_OK;
}


/*
 * mops_intersect_lines2D:
 * Code taken from the c.g.algorithms FAQ, which in turn points to:
 * Graphics Gems III pp. 199-202 "Faster Line Segment Intersection"
 *
 * Input:   p1[2],p2[2]; p3[2],p4[2] - 2 2D line segments
 * Returns: 0 - no intersection in the given segments
 *          1 - segments intersect in point ip
 */
int
mops_intersect_lines2D(double *p1, double *p2, double *p3, double *p4,
		       double *ip)
{
 double r, s, den;

  den = ((p2[0]-p1[0])*(p4[1]-p3[1])-(p2[1]-p1[1])*(p4[0]-p3[0]));

  r = ((p1[1]-p3[1])*(p4[0]-p3[0])-(p1[0]-p3[0])*(p4[1]-p3[1]))/den;

  if((r < 0.0) || (r > 1.0))
    {
      return 0; /* XXXX early exit! */
    }

  s = ((p1[1]-p3[1])*(p2[0]-p1[0])-(p1[0]-p3[0])*(p2[1]-p1[1]))/den;

  if((s < 0.0) || (s > 1.0))
    {
      return 0; /* XXXX early exit! */
    }

  ip[0] = p1[0]+r*(p2[0]-p1[0]);
  ip[1] = p1[1]+r*(p2[1]-p1[1]);

 return 1;
}


int
mops_st_TessTrimCurves(mops_object *o, int num, int *nt, double ***tt,
		       int **tl, int **td)
{
 int mops_status = MOPS_OK;
 double *dtmp = NULL, angle, **tts, p1[4], p2[4];
 int i, j, numtrims = 0, *tls, *tds, got_object = 0;
 mops_object *d = NULL, *dd = NULL;
 mops_nurbcurve_object *c = NULL;
 GLfloat m[4][4];
 GLdouble mm[16];

  /* count trimloops */
  /* XXXX BUG! we count childs and not NC providing objects */
  d = o->down;
  while(d)
    {
      numtrims++;
      d = d->next;
    }

  /* tesselate trimloops and get their orientation */
  if(!(tts = calloc(numtrims, sizeof(double *))))
    return MOPS_OUTOFMEM;
  if(!(tls = calloc(numtrims, sizeof(int))))
    { free(tts); return MOPS_OUTOFMEM; }
  if(!(tds = calloc(numtrims, sizeof(int))))
    { free(tts); free(tls); return MOPS_OUTOFMEM; }

  i = 0;
  d = o->down;
  while(d)
    {
      if(d->type == MOPS_OTNURBCURVE)
	{
	  c = (mops_nurbcurve_object *)d->object;
	  glMatrixMode (GL_MODELVIEW);
	  glPushMatrix();
	   glLoadIdentity();
	   glTranslated(d->movx, d->movy, d->movz);
	   mops_build_rotmatrix(m, d->quat);
	   glMultMatrixf(&(m[0][0]));
	   glScaled (d->scalx, d->scaly, d->scalz);
	   glGetDoublev(GL_MODELVIEW_MATRIX, mm);
	  glPopMatrix();
	}
      else
	{
	  dd = NULL;
	  mops_status = mops_get_object(d, MOPS_OTNURBCURVE, &dd);
	  if(dd)
	    got_object = MOPS_TRUE;

	  c = (mops_nurbcurve_object *)dd->object;

	  glMatrixMode (GL_MODELVIEW);
	  glPushMatrix();
	   glLoadIdentity();
	   glTranslated(dd->movx, dd->movy, dd->movz);
	   mops_build_rotmatrix(m, dd->quat);
	   glMultMatrixf(&(m[0][0]));
	   glScaled (dd->scalx, dd->scaly, dd->scalz);
	   glGetDoublev(GL_MODELVIEW_MATRIX, mm);
	  glPopMatrix();
	}

      if(!(dtmp = calloc(c->length*(c->has_weights?4:3), sizeof(double))))
	{ return MOPS_OUTOFMEM; } /* XXXX Memory Leak? */

      /* apply transformations */
      if(c->has_weights)
	{
	  for(j = 0; j < c->length; j++)
	    {
	      memcpy(p1, &(c->controlv[j*4]), 4*sizeof(double));
	      MOPS_APTRAN4(p2, p1, mm)
	      memcpy(&(dtmp[j*4]), p2, 4*sizeof(double));
	    }
	}
      else
	{
	  for(j = 0; j < c->length; j++)
	    {
	      memcpy(p1, &(c->controlv[j*3]), 3*sizeof(double));
	      MOPS_APTRAN3(p2, p1, mm)
	      memcpy(&(dtmp[j*3]), p2, 3*sizeof(double));
	    }
	}
      
      if(c->has_weights)
	mops_nb_2DCurvePoints4D(c->length-1, c->order-1, c->knotv, dtmp,
				num, &(tls[i]), &(tts[i]));
      else
	mops_nb_2DCurvePoints3D(c->length-1, c->order-1, c->knotv, dtmp,
				num, &(tls[i]), &(tts[i]));

      free(dtmp);
      dtmp = NULL;

      /* get orientation of trimloop */
      angle = 0.0;
      mops_get_nc_orientation(c, &angle);
      if(angle > 0.0)
	tds[i] = 1;

      if(got_object)
	mops_delete_multi_objects(dd);

      i++;
      d = d->next;
    } /* while */

  *nt = numtrims;
  *tt = tts;
  *tl = tls;
  *td = tds;

 return MOPS_OK;
}


int
mops_nb_MergeUVectors(stess_uvp *a, stess_uvp *b)
{
 stess_uvp *p1, *p2, *p3;
 int done = 0, inserted = 0, count = 0, toggle = 0;

 p2 = b;
 while(p2)
   {
     if(toggle)
       toggle = 0;
     else
       toggle = 1;
     count++;
     p2 = p2->next;
   }

 /* never insert uneven numbers of points! */
 if(toggle)
   {
     /*
     fprintf(stderr,"Uneven number of trimloop points (%d) detected!\n",count);
     */

     /* free b */
     while(b)
       {
	 p2 = b->next;
	 free(b);
	 b = p2;
       }

     return MOPS_OK;
   }

  while(!done)
    {
      inserted = 0;
      p1 = a;
      p2 = b;
      while(!inserted)
	{
	  if(p1->next)
	    {
	      if(p2)
		if(p2->v == p1->v)
		  { /* Danger! Check for intersecting trimloops: */
		    if(p1->type == 1)
		      return MOPS_ERROR; /* XXXX early exit! */
		    
		    /* We, accidentally, have here a trimloop
		     * point that is identical to a wanted uv-point;
		     * we therefore have to transmogrify the uv-point to
		     * a trimloop point and delete the original trimloop
		     * point!
		     */
		    fprintf(stderr,"Transmogrifying point!\n");
		    p1->type = 1;
		    p1->dir = p2->dir;
		    p3 = p2->next;
		    free(p2);
		    p2 = p3;
		    b = p3;
		    inserted = 1;
		  }
	      if(p2)
		if((p2->v > p1->v) && (p2->v < p1->next->v))
		  {
		    p3 = p1->next;
		    p1->next = p2;
		    b = p2->next;
		    p2->next = p3;
		    inserted = 1;
		  }
		else
		  {
		    p1 = p1->next;
		  } /* if */
	    } /* if */
	} /* while */

      if(!b)
	done = 1;

    } /* while */

 return MOPS_OK;
}


int
mops_nb_MergeVVectors(stess_uvp *a, stess_uvp *b)
{
 stess_uvp *p1, *p2, *p3;
 int done = 0, inserted = 0, count = 0, toggle = 0;

 p2 = b;
 while(p2)
   {
     if(toggle)
       toggle = 0;
     else
       toggle = 1;
     count++;
     p2 = p2->next;
   }

 /* never insert uneven numbers of points! */
 if(toggle)
   {
     /*
     fprintf(stderr,"Uneven number of trimloop points (%d) detected!\n",count);
     */

     /* free b */
     while(b)
       {
	 p2 = b->next;
	 free(b);
	 b = p2;
       }

     return MOPS_OK;
   }

  while(!done)
    {
      inserted = 0;
      p1 = a;
      p2 = b;
      while(!inserted)
	{
	  if(p1->next)
	    {
	      if(p2)
		if(p2->u == p1->u)
		  { /* Danger! Check for intersecting trimloops: */
		    if(p1->type == 1)
		      return MOPS_ERROR; /* XXXX early exit! */
		    
		    /* We, accidentally, have here a trimloop
		     * point that is identical to a wanted uv-point;
		     * we therefore have to transmogrify the uv-point to
		     * a trimloop point and delete the original trimloop
		     * point!
		     */
		    fprintf(stderr,"Transmogrifying point!\n");
		    p1->type = 1;
		    p1->dir = p2->dir;
		    p3 = p2->next;
		    free(p2);
		    p2 = p3;
		    b = p3;
		    inserted = 1;
		  }
	      if(p2)
		if((p2->u > p1->u) && (p2->u < p1->next->u))
		  {
		    p3 = p1->next;
		    p1->next = p2;
		    b = p2->next;
		    p2->next = p3;
		    inserted = 1;
		  }
		else
		  {
		    p1 = p1->next;
		  } /* if */
	    } /* if */
	} /* while */

      if(!b)
	done = 1;

    } /* while */

 return MOPS_OK;
}


int
mops_nb_TessTrimmedNPU(Tcl_Interp *interp, mops_object *o, int num,
		       int numtrims, double **tcs, int *tcslens, int *tcsdirs,
		       int *flcw, stess_uvp ***result)
{
 int mops_status = MOPS_OK;
 mops_nurbpatch_object *p = NULL;
 stess_uvp **uvps = NULL, *uvpptr, *newuvp, **olduvp, *trimuvp;
 stess_uvp *uvpptr2, *uvpptr3;
 double *tt, ipoint[2] = {0};
 double p3[2], p4[2], *U, *V, u, v;
 double umin, umax, vmin, vmax, ud, vd;
 int i, j, k, l, ind;
 int out = 0, first_loop = 1, first_loop_cw = 0;
 int Cm, Cn, trimloop_point, done;
 char fname[] = "TessTrimmedNPU";

  p = (mops_nurbpatch_object *)o->object;

  /* calc desired uv coords for patch tesselation */
  U = p->uknotv;
  j = 0;
  while(U[j] == U[j+1])
    {
      j++;
    }
  umin = U[j];

  j = p->width-1+p->uorder-1;
  while(U[j] == U[j-1])
    {
      j--;
    }
  umax = U[j];

  Cn = (p->width-1)*num;
  ud = (umax-umin)/((Cn)-1);
  u = umin;

  if(!(uvps = calloc(Cn, sizeof(stess_uvp *))))
    {
      return MOPS_OUTOFMEM;
    }
  V = p->vknotv;
  j = 0;
  while(V[j] == V[j+1])
    {
      j++;
    }
  vmin = V[j];

  j = p->height-1+p->vorder-1;
  while(V[j] == V[j-1])
    {
      j--;
    }
  vmax = V[j];

  Cm = (p->height-1)*num;
  vd = (umax-umin)/((Cm)-1);

  /* fill desired uv-coords */
  for(i = 0; i < Cn; i++)
   {
     v = vmin;
     olduvp = &(uvps[i]);
     for(j = 0; j < Cm; j++)
     {
       if(!(newuvp = calloc(1, sizeof(stess_uvp))))
	 {
	   return MOPS_OUTOFMEM;
	 }
       /* type == 0 */
       newuvp->u = u;
       newuvp->v = v;
       *olduvp = newuvp;
       olduvp = &(newuvp->next);
       v += vd;
     }
     u += ud;
   }

  u = umin;
  /* match desired uv coords of patch tesselation with trimloops */
  for(i = 0; i < Cn; i++)
   {
    v = vmin;
    olduvp = &trimuvp;
    trimuvp = NULL;
    p3[0] = u;
    p4[0] = u;
    /* calc all intersections of all trimloops with current u */
    for(j = 0; j < (Cm-1); j++)
     {
      p3[1] = v;
      p4[1] = v+vd;
      for(k = 0; k < numtrims; k++)
       {	
	tt = tcs[k];

	for(l = 0; l < (tcslens[k]-1); l++)
	  {
	   /* pre-select trimloop section */
	   ind = l*2;
	   if(((tt[ind] <= u) && (tt[ind+2] >= u)) ||
	      ((tt[ind] >= u) && (tt[ind+2] <= u)))
	    {
	      ipoint[0] = 0.0;
	      ipoint[1] = 0.0;

	      if((mops_intersect_lines2D(&(tt[ind]), &(tt[ind+2]), p3, p4,
					 ipoint)))
		{ /* u-line intersects with trimcurve */

		  /* test commented out, because it will make
		     problems with uvp-types */

		  /* check whether ipoint is farther away enough
                     from current v, v+vd */
		  /* if((fabs(ipoint[1] - v) > STESS_EPSILON) &&
		     (fabs(ipoint[1] - (v+vd)) > STESS_EPSILON))
		    {*/ /* yes it is */
		      /* add new point */
		      if(!(newuvp = calloc(1, sizeof(stess_uvp))))
			{
			  return MOPS_OUTOFMEM;
			}
		      newuvp->type = 1;
		      newuvp->dir = tcsdirs[k];
		      newuvp->u = ipoint[0];
		      newuvp->v = ipoint[1];
		      *olduvp = newuvp;
		      olduvp = &(newuvp->next);
		      /*		    }*/ /* if */
		} /* if */
	    } /* if */
	  } /* for */
       } /* for */

      v += vd;
     } /* for */

    /* merge vectors */
    if(trimuvp)
      {
	mops_status = MOPS_OK;
	mops_status = mops_nb_MergeUVectors(uvps[i], trimuvp);
	if(mops_status)
	  {
	    mops_error(interp, MOPS_ERROR, fname,
		       "Intersecting or misoriented Trimcurves!\n");

	    if(trimuvp)
	      {
		uvpptr = trimuvp;
		while(uvpptr)
		  {
		    uvpptr2 = uvpptr->next;
		    free(uvpptr);
		    uvpptr = uvpptr2;
		  }
	      }
	    return MOPS_ERROR;
	  }
      }

    u += ud;
   } /* for */


  /* remove unwanted uvps */
  first_loop = 1;
  for(i = 0; i < Cn; i++)
   {
    olduvp = &(uvps[i]);
    uvpptr = uvps[i];

    while(uvpptr)
      {
	/* act only on trimloop-points */
	if(uvpptr->type == 1)
	  {
	    if(first_loop)
	      {
		if(uvpptr->dir == 0)
		  { /* cw */
		    /* remember that we probably have to delete whole lines */
		    first_loop_cw = 1;
		    /* since this loop is cw, we came from outside */
		    out = 1;
		  }
	      }
	    first_loop = 0;

	    if(out)
	      {
		/*
		 * we enter the patch again, and remove all
		 * uvps that we skipped since we left the patch
		 */
		/* free unwanted vertices */
		if(uvpptr != (*olduvp))
		  {
		    uvpptr2 = (*olduvp)->next;
		    (*olduvp)->next = uvpptr;
		    while(uvpptr2 != uvpptr)
		      {
			uvpptr3 = uvpptr2->next;
			free(uvpptr2);
			uvpptr2 = uvpptr3;
		      } /* while */
		  }

		/* we are now "in" */
		out = 0;
	      }
	    else
	      {
		/*
		 * we are about to leave the patch and remember
		 * where we are; we will delete later from
		 * uvpptr->next on, all intermediate uvps
		 */
		olduvp = &(uvpptr->next);
		/* we are now out */
		out = 1;
	      }
	  } /* if */
	uvpptr = uvpptr->next;
      } /* while */
   } /* for */

  /* remove unwanted lines (all lines that contain no trimloop point) */
  if(first_loop_cw == 1)
    {
      for(i=0; i < Cn; i++)
	{
	  trimloop_point = 0;
	  done = 0;
	  uvpptr = uvps[i]; 

	  while(uvpptr && (!done))
	    {
	      if(uvpptr->type == 1)
		{
		  trimloop_point = 1;
		  done = 1;
		}
	      uvpptr = uvpptr->next;
	    }

	  if(!trimloop_point)
	    {
	      uvpptr = uvps[i];
	      while(uvpptr)
		{
		  uvpptr2 = uvpptr->next;
		  free(uvpptr);
		  uvpptr = uvpptr2;
		}
	      uvps[i] = NULL;
	    }

	}
    }

  /* finally, calculate surfacepoints */
  for(i = 0; i < Cn; i++)
    {
      uvpptr = uvps[i]; 

      while(uvpptr)
	{
	  if(p->has_weights)
	    mops_nb_SurfacePoint4D(p->width-1, p->height-1,
	     p->uorder-1, p->vorder-1, p->uknotv, p->vknotv, 
	     p->controlv, uvpptr->u, uvpptr->v, uvpptr->C);
	  else
	    mops_nb_SurfacePoint3D(p->width-1, p->height-1,
	     p->uorder-1, p->vorder-1, p->uknotv, p->vknotv, 
	     p->controlv, uvpptr->u, uvpptr->v, uvpptr->C);

	  uvpptr = uvpptr->next;
	}
    }

  *flcw = first_loop_cw;
  *result = uvps;

 return mops_status;
}


int
mops_nb_TessTrimmedNPV(Tcl_Interp *interp, mops_object *o, int num,
		       int numtrims, double **tcs, int *tcslens, int *tcsdirs,
		       int flcw, stess_uvp ***result)
{
 int mops_status = MOPS_OK;
 mops_nurbpatch_object *p = NULL;
 stess_uvp **uvps = NULL, *uvpptr, *newuvp, **olduvp, *trimuvp;
 stess_uvp *uvpptr2, *uvpptr3;
 double *tt, ipoint[2] = {0};
 double p3[2], p4[2], *U, *V, u, v;
 double umin, umax, vmin, vmax, ud, vd;
 int i, j, k, l, ind;
 int out = 0, first_loop = 1, first_loop_cw = 0;
 int Cm, Cn, trimloop_point, done;
 char fname[] = "TessTrimmedNPU";

  p = (mops_nurbpatch_object *)o->object;
  first_loop_cw = flcw;


  /* calc desired uv coords for patch tesselation */
  U = p->uknotv;
  j = 0;
  while(U[j] == U[j+1])
    {
      j++;
    }
  umin = U[j];

  j = p->width-1+p->uorder-1;
  while(U[j] == U[j-1])
    {
      j--;
    }
  umax = U[j];

  Cn = (p->width-1)*num;
  ud = (umax-umin)/((Cn)-1);
  u = umin;

  V = p->vknotv;
  j = 0;
  while(V[j] == V[j+1])
    {
      j++;
    }
  vmin = V[j];

  j = p->height-1+p->vorder-1;
  while(V[j] == V[j-1])
    {
      j--;
    }
  vmax = V[j];

  Cm = (p->height-1)*num;
  vd = (umax-umin)/((Cm)-1);

  if(!(uvps = calloc(Cm, sizeof(stess_uvp *))))
    {
      return MOPS_OUTOFMEM;
    }

  /* fill desired uv-coords */
  v = vmin;
  for(i = 0; i < Cm; i++)
   {
     u = umin;
     olduvp = &(uvps[i]);
     for(j = 0; j < Cn; j++)
     {
       if(!(newuvp = calloc(1, sizeof(stess_uvp))))
	 {
	   return MOPS_OUTOFMEM;
	 }
       /* type == 0 */
       newuvp->u = u;
       newuvp->v = v;
       *olduvp = newuvp;
       olduvp = &(newuvp->next);
       u += ud;
     }
     v += vd;
   }

  v = vmin;
  /* match desired uv coords of patch tesselation with trimloops */
  for(i = 0; i < Cm; i++)
   {
    u = umin;
    olduvp = &trimuvp;
    trimuvp = NULL;
    p3[1] = v;
    p4[1] = v;
    /* calc all intersections of all trimloops with current u */
    for(j = 0; j < (Cn-1); j++)
     {
      p3[0] = u;
      p4[0] = u+ud;
      for(k = 0; k < numtrims; k++)
       {	
	tt = tcs[k];

	for(l = 0; l < (tcslens[k]-1); l++)
	  {
	   /* pre-select trimloop section */
	   ind = l*2;
	   if(((tt[ind+1] <= v) && (tt[ind+2+1] >= v)) ||
	      ((tt[ind+1] >= v) && (tt[ind+2+1] <= v)))
	    {
	      ipoint[0] = 0.0;
	      ipoint[1] = 0.0;

	      if((mops_intersect_lines2D(&(tt[ind]), &(tt[ind+2]), p3, p4,
					 ipoint)))
		{ /* u-line intersects with trimcurve */

		  /* test commented out, because it will make
		     problems with uvp-types */

		  /* check whether ipoint is farther away enough
                     from current v, v+vd */
		  /* if((fabs(ipoint[1] - v) > STESS_EPSILON) &&
		     (fabs(ipoint[1] - (v+vd)) > STESS_EPSILON))
		    {*/ /* yes it is */
		      /* add new point */
		      if(!(newuvp = calloc(1, sizeof(stess_uvp))))
			{
			  return MOPS_OUTOFMEM;
			}
		      newuvp->type = 1;
		      newuvp->dir = tcsdirs[k];
		      newuvp->u = ipoint[0];
		      newuvp->v = ipoint[1];
		      *olduvp = newuvp;
		      olduvp = &(newuvp->next);
		      /*		    }*/ /* if */
		} /* if */
	    } /* if */
	  } /* for */
       } /* for */

      u += ud;
     } /* for */

    /* merge vectors */
    if(trimuvp)
      {
	mops_status = MOPS_OK;
	mops_status = mops_nb_MergeVVectors(uvps[i], trimuvp);
	if(mops_status)
	  {
	    mops_error(interp, MOPS_ERROR, fname,
		       "Intersecting or misoriented Trimcurves!\n");

	    if(trimuvp)
	      {
		uvpptr = trimuvp;
		while(uvpptr)
		  {
		    uvpptr2 = uvpptr->next;
		    free(uvpptr);
		    uvpptr = uvpptr2;
		  }
	      }
	    return MOPS_ERROR;
	  }
      }

    v += vd;
   } /* for */


  /* remove unwanted uvps */
  first_loop = 1;
  for(i = 0; i < Cm; i++)
   {
    olduvp = &(uvps[i]);
    uvpptr = uvps[i];

    while(uvpptr)
      {
	/* act only on trimloop-points */
	if(uvpptr->type == 1)
	  {
	    if(first_loop)
	      {
		if(uvpptr->dir == 0)
		  { /* cw */
		    /* since this loop is cw, we came from outside */
		    out = 1;
		  }
	      }
	    first_loop = 0;

	    if(out)
	      {
		/*
		 * we enter the patch again, and remove all
		 * uvps that we skipped since we left the patch
		 */
		/* free unwanted vertices */
		if(uvpptr != (*olduvp))
		  {
		    uvpptr2 = (*olduvp)->next;
		    (*olduvp)->next = uvpptr;
		    while(uvpptr2 != uvpptr)
		      {
			uvpptr3 = uvpptr2->next;
			free(uvpptr2);
			uvpptr2 = uvpptr3;
		      } /* while */
		  }

		/* we are now "in" */
		out = 0;
	      }
	    else
	      {
		/*
		 * we are about to leave the patch and remember
		 * where we are; we will delete later from
		 * uvpptr->next on, all intermediate uvps
		 */
		olduvp = &(uvpptr->next);
		/* we are now out */
		out = 1;
	      }
	  } /* if */
	uvpptr = uvpptr->next;
      } /* while */
   } /* for */

  /* remove unwanted lines (all lines that contain no trimloop point) */
  if(first_loop_cw == 1)
    {
      for(i = 0; i < Cn; i++)
	{
	  trimloop_point = 0;
	  done = 0;
	  uvpptr = uvps[i]; 

	  while(uvpptr && (!done))
	    {
	      if(uvpptr->type == 1)
		{
		  trimloop_point = 1;
		  done = 1;
		}
	      uvpptr = uvpptr->next;
	    }

	  if(!trimloop_point)
	    {
	      uvpptr = uvps[i];
	      while(uvpptr)
		{
		  uvpptr2 = uvpptr->next;
		  free(uvpptr);
		  uvpptr = uvpptr2;
		}
	      uvps[i] = NULL;
	    }

	}
    }

  /* finally, calculate surfacepoints */
  for(i = 0; i < Cm; i++)
    {
      uvpptr = uvps[i]; 

      while(uvpptr)
	{
	  if(p->has_weights)
	    mops_nb_SurfacePoint4D(p->width-1, p->height-1,
	     p->uorder-1, p->vorder-1, p->uknotv, p->vknotv, 
	     p->controlv, uvpptr->u, uvpptr->v, uvpptr->C);
	  else
	    mops_nb_SurfacePoint3D(p->width-1, p->height-1,
	     p->uorder-1, p->vorder-1, p->uknotv, p->vknotv, 
	     p->controlv, uvpptr->u, uvpptr->v, uvpptr->C);

	  uvpptr = uvpptr->next;
	}
    }

  *result = uvps;

 return mops_status;
}


int
mops_stess_drawtrimmednurb(Tcl_Interp *interp, mops_object *o, int num)
{
 int mops_status = MOPS_OK;
 double **tcs = NULL;
 double p4[4] = {0};
 int numtrims = 0, *tcslens = NULL, *tcsdirs = NULL;
 int i, j, Cm, Cn, out = 0, first_loop_cw = 0;
 stess_uvp **ups = NULL, **vps = NULL, *uvpptr, *uvpptr2;
 mops_nurbpatch_object *p = NULL;

  p = (mops_nurbpatch_object *)o->object;
  Cn = (p->width-1)*num;
  Cm = (p->height-1)*num;

  mops_status = mops_st_TessTrimCurves(o, num, &numtrims,  &tcs,
				      &tcslens,  &tcsdirs);

  if(mops_status)
    goto cleanup;

  mops_status = mops_nb_TessTrimmedNPU(interp, o, num, numtrims, tcs,
				       tcslens, tcsdirs, &first_loop_cw, &ups);

  if(first_loop_cw != 1)
    out = 0;
  else
    out = 1;
  /* draw iso-u lines */
  for(i = 0; i < Cn; i++)
    {
      uvpptr = ups[i];

      if(!out)
	glBegin(GL_LINE_STRIP);

      while(uvpptr)
	{

	  if(uvpptr->type == 1)
	    {
	      if(out)
		{
		  glBegin(GL_LINE_STRIP);
		  glVertex3dv((GLdouble*)(uvpptr->C));
		  out = 0;
		}
	      else
		{
		  glVertex3dv((GLdouble*)(uvpptr->C));
		  glEnd();
		  out = 1;
		}
	    }
	  else
	    glVertex3dv((GLdouble*)(uvpptr->C));

	  uvpptr = uvpptr->next;
	}

      if(!out)
	glEnd();

    } /* for */

  mops_status = mops_nb_TessTrimmedNPV(interp, o, num, numtrims, tcs,
				       tcslens, tcsdirs, first_loop_cw, &vps);

  if(first_loop_cw != 1)
    out = 0;
  else
    out = 1;

  /* draw iso-v lines */
  for(i = 0; i < Cm; i++)
    {
      uvpptr = vps[i];

      if(!out)
	glBegin(GL_LINE_STRIP);

      while(uvpptr)
	{

	  if(uvpptr->type == 1)
	    {
	      if(out)
		{
		  glBegin(GL_LINE_STRIP);
		  glVertex3dv((GLdouble*)(uvpptr->C));
		  out = 0;
		}
	      else
		{
		  glVertex3dv((GLdouble*)(uvpptr->C));
		  glEnd();
		  out = 1;
		}
	    }
	  else
	    glVertex3dv((GLdouble*)(uvpptr->C));

	  uvpptr = uvpptr->next;
	}

      if(!out)
	glEnd();

    } /* for */


  /* draw trimcurves */
  for(i = 0; i < numtrims; i++)
    {
      glBegin(GL_LINE_STRIP);
      for(j = 0; j < tcslens[i]; j++)
	{
	  if(p->has_weights)
	    mops_nb_SurfacePoint4D(p->width-1, p->height-1,
	     p->uorder-1, p->vorder-1, p->uknotv, p->vknotv, 
	     p->controlv, tcs[i][j*2], tcs[i][j*2+1], p4);
	  else
	    mops_nb_SurfacePoint3D(p->width-1, p->height-1,
	     p->uorder-1, p->vorder-1, p->uknotv, p->vknotv, 
	     p->controlv, tcs[i][j*2], tcs[i][j*2+1], p4);

	  glVertex3dv((GLdouble*)(p4));
	}
      glEnd();
    }

  /* clean up the mess */
cleanup:

  if(tcs)
    {
      for(i = 0; i < numtrims; i++)
	{
	  free(tcs[i]);
	}
      free(tcs);
    }
  if(tcslens)
    free(tcslens);
  if(tcsdirs)
    free(tcsdirs);

  if(ups)
    {
      for(i = 0; i < Cn; i++)
	{
	  uvpptr = ups[i];
	  while(uvpptr)
	    {
	      uvpptr2 = uvpptr->next;
	      free(uvpptr);
	      uvpptr = uvpptr2;
	    }
	  ups[i] = NULL;
	}
      free(ups);
    }

  if(vps)
    {
      for(i = 0; i < Cm; i++)
	{
	  uvpptr = vps[i];
	  while(uvpptr)
	    {
	      uvpptr2 = uvpptr->next;
	      free(uvpptr);
	      uvpptr = uvpptr2;
	    }
	  vps[i] = NULL;
	}
      free(vps);
    }

 return MOPS_OK;
}

