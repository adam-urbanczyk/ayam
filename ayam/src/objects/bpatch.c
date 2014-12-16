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

/* bpatch.c - bilinear patch object */

static char *ay_bpatch_name = "BPatch";

/* functions: */

/* ay_bpatch_createcb:
 *  create callback function of bpatch object
 */
int
ay_bpatch_createcb(int argc, char *argv[], ay_object *o)
{
 int ay_status = AY_OK;
 int tcl_status = TCL_OK;
 char fname[] = "crtbpatch";
 char option_handled = AY_FALSE;
 int optnum = 0, i = 2, j = 0;
 int acvlen = 0;
 char **acv = NULL;
 double *cv = NULL;
 ay_bpatch_object *bpatch = NULL;

  if(!o || !argv)
    return AY_ENULL;

  /* parse args */
  while(i < argc)
    {
      if(i+1 >= argc)
	{
	  ay_error(AY_EOPT, fname, argv[i]);
	  ay_status = AY_ERROR;
	  goto cleanup;
	}

      tcl_status = TCL_OK;
      option_handled = AY_FALSE;
      optnum = i;
      if(argv[i] && argv[i][0] != '\0')
	{
	  switch(argv[i][1])
	    {
	      /* -cv */
	    case 'c':
	      if(Tcl_SplitList(ay_interp, argv[i+1], &acvlen, &acv) == TCL_OK)
		{
		  if(cv)
		    {
		      free(cv);
		    }
		  if(!(cv = calloc(acvlen, sizeof(double))))
		    {
		      Tcl_Free((char *) acv);
		      ay_status = AY_EOMEM;
		      goto cleanup;
		    }
		  for(j = 0; j < acvlen; j++)
		    {
		      tcl_status = Tcl_GetDouble(ay_interp, acv[j], &cv[j]);
		      if(tcl_status != TCL_OK)
			{
			  break;
			}
		    } /* for */
		  Tcl_Free((char *) acv);
		}
	      option_handled = AY_TRUE;
	    } /* switch */

	  if(option_handled && (tcl_status != TCL_OK))
	    {
	      ay_error(AY_EOPT, fname, argv[i]);
	      ay_status = AY_ERROR;
	      goto cleanup;
	    }

	  i += 2;
	}
      else
	{
	  i++;
	} /* if */

      if(!option_handled)
	{
	  ay_error(AY_EUOPT, fname, argv[optnum]);
	  ay_status = AY_ERROR;
	  goto cleanup;
	}
    } /* while */

  if(!(bpatch = calloc(1, sizeof(ay_bpatch_object))))
    {
      ay_status = AY_EOMEM;
      goto cleanup;
    }

  if(cv)
    {
      if(acvlen > 2)
	{
	  bpatch->p1[0]  = cv[0];
	  bpatch->p1[1]  = cv[1];
	  bpatch->p1[2]  = cv[2];
	}
      if(acvlen > 5)
	{
	  bpatch->p2[0]  = cv[3];
	  bpatch->p2[1]  = cv[4];
	  bpatch->p2[2]  = cv[5];
	}
      if(acvlen > 8)
	{
	  bpatch->p3[0]  = cv[6];
	  bpatch->p3[1]  = cv[7];
	  bpatch->p3[2]  = cv[8];
	}
      if(acvlen > 11)
	{
	  bpatch->p4[0]  = cv[9];
	  bpatch->p4[1]  = cv[10];
	  bpatch->p4[2]  = cv[11];
	}
    }
  else
    {
      bpatch->p1[0]  = -0.5;
      bpatch->p1[1]  = -0.5;

      bpatch->p2[0]  = 0.5;
      bpatch->p2[1]  = -0.5;

      bpatch->p3[0]  = 0.5;
      bpatch->p3[1]  = 0.5;

      bpatch->p4[0]  = -0.5;
      bpatch->p4[1]  = 0.5;
    }

  o->refine = bpatch;

cleanup:

  if(cv)
    free(cv);

 return ay_status;
} /* ay_bpatch_createcb */


/* ay_bpatch_deletecb:
 *  delete callback function of bpatch object
 */
int
ay_bpatch_deletecb(void *c)
{
 ay_bpatch_object *bpatch;

  if(!c)
    return AY_ENULL;

  bpatch = (ay_bpatch_object *)(c);

  free(bpatch);

 return AY_OK;
} /* ay_bpatch_deletecb */


/* ay_bpatch_copycb:
 *  copy callback function of bpatch object
 */
int
ay_bpatch_copycb(void *src, void **dst)
{
 ay_bpatch_object *bpatch;

  if(!src || !dst)
    return AY_ENULL;

  if(!(bpatch = malloc(sizeof(ay_bpatch_object))))
    return AY_EOMEM;

  memcpy(bpatch, src, sizeof(ay_bpatch_object));

  *dst = bpatch;

 return AY_OK;
} /* ay_bpatch_copycb */


/* ay_bpatch_drawcb:
 *  draw (display in an Ayam view window) callback function of bpatch object
 */
int
ay_bpatch_drawcb(struct Togl *togl, ay_object *o)
{
 ay_bpatch_object *bpatch;
 double h1[9*3], h2[9*3], w1[9*3], w2[9*3], d[3], len = 0.0;
 int i;

  if(!o)
    return AY_ENULL;

  bpatch = (ay_bpatch_object *)o->refine;

  if(!bpatch)
    return AY_ENULL;

  /* draw */
  /*
  glBegin(GL_LINE_LOOP);
   glVertex3dv((GLdouble *)(bpatch->p1));
   glVertex3dv((GLdouble *)(bpatch->p2));
   glVertex3dv((GLdouble *)(bpatch->p3));
   glVertex3dv((GLdouble *)(bpatch->p4));
  glEnd();
  */

  /* interpolate */
  memcpy(h1,bpatch->p1,3*sizeof(double));
  for(i = 1; i <= 8; i++)
    {
      memcpy(&(h1[i*3]),bpatch->p1,3*sizeof(double));
      d[0] =  bpatch->p4[0]-bpatch->p1[0];
      d[1] =  bpatch->p4[1]-bpatch->p1[1];
      d[2] =  bpatch->p4[2]-bpatch->p1[2];
      len = AY_V3LEN(d);
      if(len > 0.0)
	{
	  AY_V3SCAL(d, i/8.0);
	}
      h1[i*3]   += d[0];
      h1[i*3+1] += d[1];
      h1[i*3+2] += d[2];
    }
  memcpy(h2,bpatch->p2,3*sizeof(double));
  for(i = 1; i <= 8; i++)
    {
      memcpy(&(h2[i*3]),bpatch->p2,3*sizeof(double));
      d[0] =  bpatch->p3[0]-bpatch->p2[0];
      d[1] =  bpatch->p3[1]-bpatch->p2[1];
      d[2] =  bpatch->p3[2]-bpatch->p2[2];
      len = AY_V3LEN(d);
      if(len > 0.0)
	{
	  AY_V3SCAL(d, i/8.0);
	}
      h2[i*3]   += d[0];
      h2[i*3+1] += d[1];
      h2[i*3+2] += d[2];
    }

  memcpy(w1,bpatch->p1,3*sizeof(double));
  for(i = 1; i <= 8; i++)
    {
      memcpy(&(w1[i*3]),bpatch->p1,3*sizeof(double));
      d[0] =  bpatch->p2[0]-bpatch->p1[0];
      d[1] =  bpatch->p2[1]-bpatch->p1[1];
      d[2] =  bpatch->p2[2]-bpatch->p1[2];
      len = AY_V3LEN(d);
      if(len > 0.0)
	{
	  AY_V3SCAL(d, i/8.0);
	}
      w1[i*3]   += d[0];
      w1[i*3+1] += d[1];
      w1[i*3+2] += d[2];
    }
  memcpy(w2,bpatch->p4,3*sizeof(double));
  for(i = 1; i <= 8; i++)
    {
      memcpy(&(w2[i*3]),bpatch->p4,3*sizeof(double));
      d[0] =  bpatch->p3[0]-bpatch->p4[0];
      d[1] =  bpatch->p3[1]-bpatch->p4[1];
      d[2] =  bpatch->p3[2]-bpatch->p4[2];
      len = AY_V3LEN(d);
      if(len > 0.0)
	{
	  AY_V3SCAL(d, i/8.0);
	}
      w2[i*3]   += d[0];
      w2[i*3+1] += d[1];
      w2[i*3+2] += d[2];
    }

  glBegin(GL_LINES);
  for(i = 0; i <= 8; i++)
    {
      glVertex3dv((GLdouble*)(&(h1[i*3])));
      glVertex3dv((GLdouble*)(&(h2[i*3])));
      glVertex3dv((GLdouble*)(&(w1[i*3])));
      glVertex3dv((GLdouble*)(&(w2[i*3])));
    }
  glEnd();

 return AY_OK;
} /* ay_bpatch_drawcb */


/* ay_ncurve_drawhcb:
 *  draw handles (in an Ayam view window) callback function of bpatch object
 */
int
ay_bpatch_drawhcb(struct Togl *togl, ay_object *o)
{
 ay_bpatch_object *bpatch;

  if(!o)
    return AY_ENULL;

  bpatch = (ay_bpatch_object *)o->refine;

  if(!bpatch)
    return AY_ENULL;

  /* draw */
  glBegin(GL_POINTS);
   glVertex3dv((GLdouble *)(bpatch->p1));
   glVertex3dv((GLdouble *)(bpatch->p2));
   glVertex3dv((GLdouble *)(bpatch->p3));
   glVertex3dv((GLdouble *)(bpatch->p4));
  glEnd();

 return AY_OK;
} /* ay_bpatch_drawhcb */


/* ay_bpatch_shadecb:
 *  shade (display in an Ayam view window) callback function of bpatch object
 */
int
ay_bpatch_shadecb(struct Togl *togl, ay_object *o)
{
 ay_bpatch_object *bpatch;
 double n[3] = {0};

  if(!o)
    return AY_ENULL;

  bpatch = (ay_bpatch_object *)o->refine;

  if(!bpatch)
    return AY_ENULL;

  glBegin(GL_QUADS);
   /*  XXXX should we cache the normals? */
   ay_geom_calcnfrom3(bpatch->p4, bpatch->p1, bpatch->p2, n);
   glNormal3dv((GLdouble *)n);
   glVertex3dv((GLdouble *)(bpatch->p1));
   ay_geom_calcnfrom3(bpatch->p1, bpatch->p2, bpatch->p3, n);
   glNormal3dv((GLdouble *)n);
   glVertex3dv((GLdouble *)(bpatch->p2));
   ay_geom_calcnfrom3(bpatch->p2, bpatch->p3, bpatch->p4, n);
   glNormal3dv((GLdouble *)n);
   glVertex3dv((GLdouble *)(bpatch->p3));
   ay_geom_calcnfrom3(bpatch->p3, bpatch->p4, bpatch->p1, n);
   glNormal3dv((GLdouble *)n);
   glVertex3dv((GLdouble *)(bpatch->p4));
  glEnd();

 return AY_OK;
} /* ay_bpatch_shadecb */


/* ay_bpatch_getpntcb:
 *  get point (editing and selection) callback function of bpatch object
 */
int
ay_bpatch_getpntcb(int mode, ay_object *o, double *p, ay_pointedit *pe)
{
 ay_bpatch_object *bpatch;
 ay_point *pnt = NULL, **lastpnt = NULL;
 double min_dist = ay_prefs.pick_epsilon, dist = 0.0;
 double *pecoord = NULL, **ctmp, *c = NULL;
 int i, j, a;
 unsigned int *itmp, peindex = 0;

  if(!o || ((mode != 3) && (!p || !pe)))
    return AY_ENULL;

  bpatch = (ay_bpatch_object *)(o->refine);

  if(!bpatch)
    return AY_ENULL;

  if(min_dist == 0.0)
    min_dist = DBL_MAX;

  if(pe)
    pe->rational = AY_FALSE;

  switch(mode)
    {
    case 0:
      /* select all points */

      if(!(pe->coords = calloc(4, sizeof(double*))))
	return AY_EOMEM;

      if(!(pe->indices = calloc(4, sizeof(unsigned int))))
	return AY_EOMEM;

      pe->coords[0] = bpatch->p1;
      pe->coords[1] = bpatch->p2;
      pe->coords[2] = bpatch->p3;
      pe->coords[3] = bpatch->p4;

      pe->indices[0] = 0;
      pe->indices[1] = 1;
      pe->indices[2] = 2;
      pe->indices[3] = 3;

      pe->num = 4;
      break;
    case 1:
      /* selection based on a single point */

      dist = AY_VLEN((p[0] - bpatch->p1[0]),
		     (p[1] - bpatch->p1[1]),
		     (p[2] - bpatch->p1[2]));

      if(dist < min_dist)
	{
	  pecoord = bpatch->p1;
	  peindex = 0;
	  min_dist = dist;
	}

      dist = AY_VLEN((p[0] - bpatch->p2[0]),
		     (p[1] - bpatch->p2[1]),
		     (p[2] - bpatch->p2[2]));

      if(dist < min_dist)
	{
	  pecoord = bpatch->p2;
	  peindex = 1;
	  min_dist = dist;
	}

      dist = AY_VLEN((p[0] - bpatch->p3[0]),
		     (p[1] - bpatch->p3[1]),
		     (p[2] - bpatch->p3[2]));

      if(dist < min_dist)
	{
	  pecoord = bpatch->p3;
	  peindex = 2;
	  min_dist = dist;
	}

      dist = AY_VLEN((p[0] - bpatch->p4[0]),
		     (p[1] - bpatch->p4[1]),
		     (p[2] - bpatch->p4[2]));

      if(dist < min_dist)
	{
	  pecoord = bpatch->p4;
	  peindex = 3;
	  /*min_dist = dist;*/
	}

      if(!pecoord)
	return AY_OK; /* XXXX should this return a 'AY_EPICK' ? */

      if(!(pe->coords = calloc(1, sizeof(double*))))
	return AY_EOMEM;

      if(!(pe->indices = calloc(1, sizeof(unsigned int))))
	return AY_EOMEM;

      pe->coords[0] = pecoord;
      pe->indices[0] = peindex;
      pe->num = 1;
      break;
    case 2:
      /* selection based on planes */
      j = 0;
      a = 0;
      for(i = 0; i < 4; i++)
	{
	  if(i == 0)
	    c = bpatch->p1;
	  if(i == 1)
	    c = bpatch->p2;
	  if(i == 2)
	    c = bpatch->p3;
	  if(i == 3)
	    c = bpatch->p4;

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

	      pe->coords[a] = &(c[0]);
	      pe->indices[a] = i;
	      a++;
	    } /* if */

	  j += 4;
	} /* for */

      pe->num = a;
      break;
    case 3:
      pnt = o->selp;
      lastpnt = &o->selp;
      while(pnt)
	{
	  if(pnt->index < 4)
	    {
	      switch(pnt->index)
		{
		case 0:
		  pnt->point = bpatch->p1;
		  break;
		case 1:
		  pnt->point = bpatch->p2;
		  break;
		case 2:
		  pnt->point = bpatch->p3;
		  break;
		case 3:
		  pnt->point = bpatch->p4;
		  break;
		default:
		  break;
		}
	      pnt->rational = AY_FALSE;
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
} /* ay_bpatch_getpntcb */


/* ay_bpatch_setpropcb:
 *  set property (from Tcl to C context) callback function of bpatch object
 */
int
ay_bpatch_setpropcb(Tcl_Interp *interp, int argc, char *argv[], ay_object *o)
{
 /*int ay_status = AY_OK;*/
 char *n1 = "BPatchAttrData";
 Tcl_Obj *to = NULL, *toa = NULL, *ton = NULL;
 ay_bpatch_object *bpatch;

  if(!interp || !o)
    return AY_ENULL;

  bpatch = (ay_bpatch_object *)o->refine;

  if(!bpatch)
    return AY_ENULL;

  toa = Tcl_NewStringObj(n1,-1);

  ton = Tcl_NewStringObj("P1_X",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetDoubleFromObj(interp,to, &bpatch->p1[0]);

  Tcl_SetStringObj(ton,"P1_Y",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetDoubleFromObj(interp,to, &bpatch->p1[1]);

  Tcl_SetStringObj(ton,"P1_Z",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetDoubleFromObj(interp,to, &bpatch->p1[2]);


  Tcl_SetStringObj(ton,"P2_X",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetDoubleFromObj(interp,to, &bpatch->p2[0]);

  Tcl_SetStringObj(ton,"P2_Y",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetDoubleFromObj(interp,to, &bpatch->p2[1]);

  Tcl_SetStringObj(ton,"P2_Z",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetDoubleFromObj(interp,to, &bpatch->p2[2]);


  Tcl_SetStringObj(ton,"P3_X",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetDoubleFromObj(interp,to, &bpatch->p3[0]);

  Tcl_SetStringObj(ton,"P3_Y",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetDoubleFromObj(interp,to, &bpatch->p3[1]);

  Tcl_SetStringObj(ton,"P3_Z",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetDoubleFromObj(interp,to, &bpatch->p3[2]);


  Tcl_SetStringObj(ton,"P4_X",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetDoubleFromObj(interp,to, &bpatch->p4[0]);

  Tcl_SetStringObj(ton,"P4_Y",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetDoubleFromObj(interp,to, &bpatch->p4[1]);

  Tcl_SetStringObj(ton,"P4_Z",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetDoubleFromObj(interp,to, &bpatch->p4[2]);

  Tcl_IncrRefCount(toa);Tcl_DecrRefCount(toa);
  Tcl_IncrRefCount(ton);Tcl_DecrRefCount(ton);

  o->modified = AY_TRUE;
  ay_notify_parent();

 return AY_OK;
} /* ay_bpatch_setpropcb */


/* ay_bpatch_getpropcb:
 *  get property (from C to Tcl context) callback function of bpatch object
 */
int
ay_bpatch_getpropcb(Tcl_Interp *interp, int argc, char *argv[], ay_object *o)
{
 char *n1="BPatchAttrData";
 Tcl_Obj *to = NULL, *toa = NULL, *ton = NULL;
 ay_bpatch_object *bpatch;

  if(!interp || !o)
    return AY_ENULL;

  bpatch = (ay_bpatch_object *)o->refine;

  if(!bpatch)
    return AY_ENULL;

  toa = Tcl_NewStringObj(n1,-1);

  ton = Tcl_NewStringObj("P1_X",-1);
  to = Tcl_NewDoubleObj(bpatch->p1[0]);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton,"P1_Y",-1);
  to = Tcl_NewDoubleObj(bpatch->p1[1]);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton,"P1_Z",-1);
  to = Tcl_NewDoubleObj(bpatch->p1[2]);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);


  Tcl_SetStringObj(ton,"P2_X",-1);
  to = Tcl_NewDoubleObj(bpatch->p2[0]);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton,"P2_Y",-1);
  to = Tcl_NewDoubleObj(bpatch->p2[1]);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton,"P2_Z",-1);
  to = Tcl_NewDoubleObj(bpatch->p2[2]);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);


  Tcl_SetStringObj(ton,"P3_X",-1);
  to = Tcl_NewDoubleObj(bpatch->p3[0]);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton,"P3_Y",-1);
  to = Tcl_NewDoubleObj(bpatch->p3[1]);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton,"P3_Z",-1);
  to = Tcl_NewDoubleObj(bpatch->p3[2]);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);


  Tcl_SetStringObj(ton,"P4_X",-1);
  to = Tcl_NewDoubleObj(bpatch->p4[0]);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton,"P4_Y",-1);
  to = Tcl_NewDoubleObj(bpatch->p4[1]);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton,"P4_Z",-1);
  to = Tcl_NewDoubleObj(bpatch->p4[2]);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);


  Tcl_IncrRefCount(toa);Tcl_DecrRefCount(toa);
  Tcl_IncrRefCount(ton);Tcl_DecrRefCount(ton);

 return AY_OK;
} /* ay_bpatch_getpropcb */


/* ay_bpatch_readcb:
 *  read (from scene file) callback function of bpatch object
 */
int
ay_bpatch_readcb(FILE *fileptr, ay_object *o)
{
 ay_bpatch_object *bp;

  if(!fileptr || !o)
   return AY_ENULL;

  if(!(bp = calloc(1, sizeof(ay_bpatch_object))))
    { return AY_EOMEM; }

  fscanf(fileptr,"%lg %lg %lg\n", &bp->p1[0], &bp->p1[1], &bp->p1[2]);
  fscanf(fileptr,"%lg %lg %lg\n", &bp->p2[0], &bp->p2[1], &bp->p2[2]);
  fscanf(fileptr,"%lg %lg %lg\n", &bp->p3[0], &bp->p3[1], &bp->p3[2]);
  fscanf(fileptr,"%lg %lg %lg\n", &bp->p4[0], &bp->p4[1], &bp->p4[2]);

  o->refine = bp;

 return AY_OK;
} /* ay_bpatch_readcb */


/* ay_bpatch_writecb:
 *  write (to scene file) callback function of bpatch object
 */
int
ay_bpatch_writecb(FILE *fileptr, ay_object *o)
{
 ay_bpatch_object *bpatch;

  if(!fileptr || !o)
    return AY_ENULL;

  bpatch = (ay_bpatch_object *)(o->refine);

  if(!bpatch)
    return AY_ENULL;

  fprintf(fileptr, "%g %g %g\n", bpatch->p1[0], bpatch->p1[1], bpatch->p1[2]);
  fprintf(fileptr, "%g %g %g\n", bpatch->p2[0], bpatch->p2[1], bpatch->p2[2]);
  fprintf(fileptr, "%g %g %g\n", bpatch->p3[0], bpatch->p3[1], bpatch->p3[2]);
  fprintf(fileptr, "%g %g %g\n", bpatch->p4[0], bpatch->p4[1], bpatch->p4[2]);

 return AY_OK;
} /* ay_bpatch_writecb */


/* ay_bpatch_wribcb:
 *  RIB export callback function of bpatch object
 */
int
ay_bpatch_wribcb(char *file, ay_object *o)
{
 ay_bpatch_object *bpatch;
 RtPoint rect[4];
 RtToken *tokens = NULL;
 RtPointer *parms = NULL;
 int i, n = 0, pvc = 0;

  if(!o)
   return AY_ENULL;

  bpatch = (ay_bpatch_object*)o->refine;

  if(!bpatch)
    return AY_ENULL;

  /* XXXX order correct for RIB? */
  rect[0][0] = (RtFloat)(bpatch->p1[0]);
  rect[0][1] = (RtFloat)(bpatch->p1[1]);
  rect[0][2] = (RtFloat)(bpatch->p1[2]);

  rect[1][0] = (RtFloat)(bpatch->p2[0]);
  rect[1][1] = (RtFloat)(bpatch->p2[1]);
  rect[1][2] = (RtFloat)(bpatch->p2[2]);

  rect[2][0] = (RtFloat)(bpatch->p4[0]);
  rect[2][1] = (RtFloat)(bpatch->p4[1]);
  rect[2][2] = (RtFloat)(bpatch->p4[2]);

  rect[3][0] = (RtFloat)(bpatch->p3[0]);
  rect[3][1] = (RtFloat)(bpatch->p3[1]);
  rect[3][2] = (RtFloat)(bpatch->p3[2]);

  /* Do we have any primitive variables? */
  if(!(pvc = ay_pv_count(o)))
    {
      /* No */
      RiPatch(RI_BILINEAR, RI_P, (RtPointer)rect, RI_NULL);
    }
  else
    {
      /* Yes, we have primitive variables. */
      if(!(tokens = calloc(pvc+1, sizeof(RtToken))))
	return AY_EOMEM;

      if(!(parms = calloc(pvc+1, sizeof(RtPointer))))
	{free(tokens); return AY_EOMEM;}

      tokens[0] = "P";
      parms[0] = (RtPointer)rect;

      n = 1;
      ay_pv_filltokpar(o, AY_TRUE, 1, &n, tokens, parms);

      RiPatchV(RI_BILINEAR, (RtInt)n, tokens, parms);

      for(i = 1; i < n; i++)
	{
	  free(tokens[i]);
	  free(parms[i]);
	}

      free(tokens);
      free(parms);
    } /* if */

 return AY_OK;
} /* ay_bpatch_wribcb */


/* ay_bpatch_bbccb:
 *  bounding box calculation callback function of bpatch object
 */
int
ay_bpatch_bbccb(ay_object *o, double *bbox, int *flags)
{
 double xmin = DBL_MAX, xmax = -DBL_MAX, ymin = DBL_MAX;
 double ymax = -DBL_MAX, zmin = DBL_MAX, zmax = -DBL_MAX;
 double cv[12] = {0};
 int i = 0, a = 0;
 ay_bpatch_object *b;

  if(!o || !bbox || !flags)
    return AY_ENULL;

  b = (ay_bpatch_object *)o->refine;

  if(!b)
    return AY_ENULL;

  memcpy(&(cv[0]), b->p1, 3*sizeof(double));
  memcpy(&(cv[3]), b->p2, 3*sizeof(double));
  memcpy(&(cv[6]), b->p3, 3*sizeof(double));
  memcpy(&(cv[9]), b->p4, 3*sizeof(double));

  for(i = 0; i < 4; i++)
    {
      if(cv[a] < xmin)
	xmin = cv[a];
      if(cv[a] > xmax)
	xmax = cv[a];

      if(cv[a+1] < ymin)
	ymin = cv[a+1];
      if(cv[a+1] > ymax)
	ymax = cv[a+1];

      if(cv[a+2] < zmin)
	zmin = cv[a+2];
      if(cv[a+2] > zmax)
	zmax = cv[a+2];

      a += 3;
    } /* for */

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
} /* ay_bpatch_bbccb */


/* ay_bpatch_convertcb:
 *  convert callback function of bpatch object
 */
int
ay_bpatch_convertcb(ay_object *o, int in_place)
{
 int ay_status = AY_OK;
 ay_object *new = NULL;

  if(!o)
    return AY_ENULL;

  ay_status = ay_provide_object(o, AY_IDNPATCH, &new);

  if(new)
    {
      if(!in_place)
	{
	  ay_object_link(new);
	}
      else
	{
	  ay_status = ay_object_replace(new, o);
	}
    }

 return ay_status;
} /* ay_bpatch_convertcb */


/* ay_bpatch_providecb:
 *  provide callback function of bpatch object
 */
int
ay_bpatch_providecb(ay_object *o, unsigned int type, ay_object **result)
{
 int ay_status = AY_OK;
 char fname[] = "bpatch_providecb";
 ay_bpatch_object *bp = NULL;
 ay_object *new = NULL;
 double *cv = NULL;

  if(!o)
    return AY_ENULL;

  if(!result)
    {
      if(type == AY_IDNPATCH)
	return AY_OK;
      else
	return AY_ERROR;
    }

  bp = (ay_bpatch_object *) o->refine;

  if(!bp)
    return AY_ENULL;

  if(type == AY_IDNPATCH)
    {
      if(!(new = calloc(1, sizeof(ay_object))))
	return AY_EOMEM;

      new->type = AY_IDNPATCH;
      ay_object_defaults(new);
      ay_trafo_copy(o, new);

      if((ay_status = ay_npt_create(2, 2, 2, 2, AY_KTNURB, AY_KTNURB,
				    NULL, NULL, NULL,
			       (ay_nurbpatch_object**)(void*)&(new->refine))))
	{
	  free(new);
	  ay_error(ay_status, fname, NULL);
	  return ay_status;
	} /* if */

      cv = ((ay_nurbpatch_object*)(new->refine))->controlv;

      memcpy(&(cv[0]), bp->p1, 3*sizeof(double));
      cv[3] = 1.0;
      memcpy(&(cv[4]), bp->p4, 3*sizeof(double));
      cv[7] = 1.0;
      memcpy(&(cv[8]), bp->p2, 3*sizeof(double));
      cv[11] = 1.0;
      memcpy(&(cv[12]), bp->p3, 3*sizeof(double));
      cv[15] = 1.0;

      /* copy some tags */
      (void)ay_tag_copyselected(o, new, ay_prefs.converttags,
				ay_prefs.converttagslen);

      *result = new;
    } /* if */

 return ay_status;
} /* ay_bpatch_providecb */


/* ay_bpatch_init:
 *  initialize the bpatch object module
 */
int
ay_bpatch_init(Tcl_Interp *interp)
{
 int ay_status = AY_OK;

 ay_status = ay_otype_registercore(ay_bpatch_name,
				   ay_bpatch_createcb,
				   ay_bpatch_deletecb,
				   ay_bpatch_copycb,
				   ay_bpatch_drawcb,
				   ay_bpatch_drawhcb,
				   ay_bpatch_shadecb,
				   ay_bpatch_setpropcb,
				   ay_bpatch_getpropcb,
				   ay_bpatch_getpntcb,
				   ay_bpatch_readcb,
				   ay_bpatch_writecb,
				   ay_bpatch_wribcb,
				   ay_bpatch_bbccb,
				   AY_IDBPATCH);

  Tcl_SetVar(interp, "propertyList", "BPatchAttr", TCL_APPEND_VALUE |
	     TCL_LIST_ELEMENT | TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  ay_status += ay_convert_register(ay_bpatch_convertcb, AY_IDBPATCH);

  ay_status += ay_provide_register(ay_bpatch_providecb, AY_IDBPATCH);

 return ay_status;
} /* ay_bpatch_init */
