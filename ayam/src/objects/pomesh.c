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

/* pomesh.c - PolyMesh object */

static char *ay_pomesh_name = "PolyMesh";

int
ay_pomesh_createcb(int argc, char *argv[], ay_object *o)
{
 ay_pomesh_object *pomesh = NULL;

  if(!(pomesh = calloc(1, sizeof(ay_pomesh_object))))
    {
      return AY_EOMEM;
    }

  o->refine = (void *)pomesh;

 return AY_OK;
} /* ay_pomesh_createcb */


int
ay_pomesh_deletecb(void *c)
{
 ay_pomesh_object *pomesh = NULL;

  if(!c)
    return AY_ENULL;    

  pomesh = (ay_pomesh_object *)(c);

  /* free nloops */
  if(pomesh->nloops)
    free(pomesh->nloops);

  /* free nverts */
  if(pomesh->nverts)
    free(pomesh->nverts);

  /* free verts */
  if(pomesh->verts)
    free(pomesh->verts);

  /* free controlv */
  if(pomesh->controlv)
    free(pomesh->controlv);

  free(pomesh);

 return AY_OK;
} /* ay_pomesh_deletecb */


int
ay_pomesh_copycb(void *src, void **dst)
{
 ay_pomesh_object *pomesh = NULL, *pomeshsrc = NULL;
 unsigned int total_loops = 0, total_verts = 0;
 int i, stride = 0;

  if(!src || !dst)
    return AY_ENULL;

  pomeshsrc = (ay_pomesh_object *)src;

  if(pomeshsrc->has_normals)
    stride = 6;
  else
    stride = 3;

  if(!(pomesh = calloc(1, sizeof(ay_pomesh_object))))
    return AY_EOMEM; 

  memcpy(pomesh, src, sizeof(ay_pomesh_object)); 

  pomesh->nloops = NULL;
  pomesh->nverts = NULL;
  pomesh->verts = NULL;
  pomesh->controlv = NULL;

  /* copy nloops */
  if(pomeshsrc->nloops)
    {
      if(!(pomesh->nloops = calloc(pomeshsrc->npolys, sizeof(unsigned int))))
	return AY_EOMEM;
      memcpy(pomesh->nloops, pomeshsrc->nloops,
	     pomesh->npolys * sizeof(unsigned int));
    }

  for(i = 0; i < pomeshsrc->npolys; i++)
    {
      total_loops += pomeshsrc->nloops[i];
    } /* for */
  
  /* copy nverts */
  if(pomeshsrc->nverts)
    {
      if(!(pomesh->nverts = calloc(total_loops, sizeof(unsigned int))))
	return AY_EOMEM;
      memcpy(pomesh->nverts, pomeshsrc->nverts,
	     total_loops * sizeof(unsigned int));

      for(i = 0; i < total_loops; i++)
	{
	  total_verts += pomeshsrc->nverts[i];
	} /* for */
    } /* if */
  
  /* copy verts */
  if(pomeshsrc->verts)
    {
      if(!(pomesh->verts = calloc(total_verts, sizeof(unsigned int))))
	return AY_EOMEM;
      memcpy(pomesh->verts, pomeshsrc->verts,
	     total_verts * sizeof(unsigned int));
    }

  /* copy controlv */
  if(pomeshsrc->controlv)
    {
      if(!(pomesh->controlv = calloc(stride * pomeshsrc->ncontrols,
				     sizeof(double))))
	return AY_EOMEM;
      memcpy(pomesh->controlv, pomeshsrc->controlv,
	     stride * pomesh->ncontrols * sizeof(double));
    }

  *dst = (void *)pomesh;

 return AY_OK;
} /* ay_pomesh_copycb */


int
ay_pomesh_drawcb(struct Togl *togl, ay_object *o)
{
 ay_pomesh_object *pomesh = NULL;
 int i = 0, j = 0, k = 0, l = 0, stride = 0;
 unsigned int a;

  if(!o)
    return AY_ENULL;

  pomesh = (ay_pomesh_object *)(o->refine);

  if(pomesh->has_normals)
    stride = 6;
  else
    stride = 3;

  for(i = 0; i < pomesh->npolys; i++)
    {
      for(j = 0; j < pomesh->nloops[i]; j++)
	{
	  glBegin(GL_LINE_LOOP);
	   for(k = 0; k < pomesh->nverts[j]; k++)
	    {
	      a = pomesh->verts[l++];
	      glVertex3dv((GLdouble*)(&(pomesh->controlv[a * stride])));
	    }
	  glEnd();
	} /* for */
    } /* for */

 return AY_OK;
} /* ay_pomesh_drawcb */

int
ay_pomesh_shadecb(struct Togl *togl, ay_object *o)
{
 int ay_status = AY_OK;
 ay_pomesh_object *pomesh = NULL;
 /*
 int i = 0, j = 0, k = 0, l = 0;
 unsigned int a;
 GLUtesselator *tess;
 */
  if(!o)
    return AY_ENULL;

  pomesh = (ay_pomesh_object *)(o->refine);

  if(1/*o->modified*/)
    {
      ay_status = ay_pomesht_tesselate(pomesh);
    }
 
 return AY_OK;
} /* ay_pomesh_shadecb */


int
ay_pomesh_drawhcb(struct Togl *togl, ay_object *o)
{
 ay_pomesh_object *pomesh = NULL;
 GLdouble *ver = NULL;
 double point_size = ay_prefs.handle_size;
 int i = 0, stride = 0;

  if(!o)
    return AY_ENULL;

  pomesh = (ay_pomesh_object *)(o->refine);

  if(pomesh->has_normals)
    stride = 6;
  else
    stride = 3;

  ver = pomesh->controlv;

  glPointSize((GLfloat)point_size);

  glBegin(GL_POINTS);
  for(i = 0; i < pomesh->ncontrols; i++)
    {
      glVertex3dv(ver);
      ver += stride;
    }
  glEnd();

 return AY_OK;
} /* ay_pomesh_drawhcb */


int
ay_pomesh_getpntcb(ay_object *o, double *p)
{
 ay_pomesh_object *pomesh = NULL;
 double min_dist = ay_prefs.pick_epsilon, dist = 0.0;
 double *pecoords = NULL, *control = NULL;
 int i = 0, j = 0, k = 0, a = 0, numfound = 0, stride = 0;

  if(!o || !p)
    return AY_ENULL;

  pomesh = (ay_pomesh_object *)(o->refine);

  if(pomesh->has_normals)
    stride = 6;
  else
    stride = 3;

  if(min_dist == 0.0)
    min_dist = DBL_MAX;

  if(ay_point_edit_coords)
    free(ay_point_edit_coords);

  ay_point_edit_coords = NULL;

  /* select all points? */
  if((p[0] == DBL_MIN) && (p[1] == DBL_MIN) && (p[2] == DBL_MIN))
    { /* yes */
      if(!(ay_point_edit_coords = calloc(pomesh->ncontrols, sizeof(double*))))
	return AY_EOMEM;

      for(i = 0; i < pomesh->ncontrols; i++)
	{
	  ay_point_edit_coords[i] = &(pomesh->controlv[a]);
	  a += stride;
	} /* for */

      ay_point_edit_coords_homogenous = AY_FALSE;
      ay_point_edit_coords_number = pomesh->ncontrols;
    }
  else
    { /* no */
      control = pomesh->controlv;
      for(i = 0; i < pomesh->ncontrols; i++)
	{
	  dist = AY_VLEN((p[0] - control[j]),
			 (p[1] - control[j+1]),
			 (p[2] - control[j+2]));

	  if(dist < min_dist)
	    {
	      pecoords = &(control[j]);
	      min_dist = dist;
	      numfound = 1;
	    }
	  else
	    {
	      if(dist == min_dist)
		{
		  numfound++;
		}
	    } /* if */

	  j += stride;
	} /* for */

      if(!pecoords)
	return AY_OK; /* XXXX should this return a 'AY_EPICK' ? */

      ay_point_edit_coords_homogenous = AY_FALSE;
      ay_point_edit_coords_number = numfound;
      if(!(ay_point_edit_coords = calloc(numfound, sizeof(double*))))
	return AY_EOMEM;

      j = 0;
      for(i = 0; i < pomesh->ncontrols; i++)
	{
	  dist = AY_VLEN((p[0] - control[j]),
			 (p[1] - control[j+1]),
			 (p[2] - control[j+2]));

	  if(dist == min_dist)
	    {
	      ay_point_edit_coords[k] = &(control[j]);
	      k++;
	    } /* if */
	  j += stride;
	} /* for */

    } /* if */

 return AY_OK;
} /* ay_pomesh_getpntcb */


/* Tcl -> C! */
int
ay_pomesh_setpropcb(Tcl_Interp *interp, int argc, char *argv[], ay_object *o)
{
 int ay_status = AY_OK;
 char *n1 = "PolyMeshAttrData";
 /*char fname[] = "pomesh_setpropcb";*/
 Tcl_Obj *to = NULL, *toa = NULL, *ton = NULL;
 ay_pomesh_object *pomesh = NULL;
 int new_type;

  if(!o)
    return AY_ENULL;

  pomesh = (ay_pomesh_object *)o->refine;
  
  toa = Tcl_NewStringObj(n1, -1);
  ton = Tcl_NewStringObj(n1, -1);

  Tcl_SetStringObj(ton, "Type", -1);
  to = Tcl_ObjGetVar2(interp, toa, ton, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp, to, &new_type);

  pomesh->type = new_type;

  Tcl_IncrRefCount(toa);Tcl_DecrRefCount(toa);
  Tcl_IncrRefCount(ton);Tcl_DecrRefCount(ton);

  ay_status = ay_notify_parent();

 return AY_OK;
} /* ay_pomesh_setpropcb */


/* C -> Tcl! */
int
ay_pomesh_getpropcb(Tcl_Interp *interp, int argc, char *argv[], ay_object *o)
{
 char *n1="PolyMeshAttrData";
 Tcl_Obj *to = NULL, *toa = NULL, *ton = NULL;
 ay_pomesh_object *pomesh = NULL;

  if(!o)
    return AY_ENULL;

  pomesh = (ay_pomesh_object *)(o->refine);

  toa = Tcl_NewStringObj(n1, -1);

  ton = Tcl_NewStringObj(n1, -1);


  Tcl_SetStringObj(ton, "Type", -1);
  to = Tcl_NewIntObj(pomesh->type);
  Tcl_ObjSetVar2(interp, toa, ton, to, TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);

  Tcl_IncrRefCount(toa);Tcl_DecrRefCount(toa);
  Tcl_IncrRefCount(ton);Tcl_DecrRefCount(ton);

 return AY_OK;
} /* ay_pomesh_getpropcb */


int
ay_pomesh_readcb(FILE *fileptr, ay_object *o)
{
 ay_pomesh_object *pomesh = NULL;
 unsigned int total_loops = 0, total_verts = 0;
 int i, a;

 if(!o)
   return AY_ENULL;

  if(!(pomesh = calloc(1, sizeof(ay_pomesh_object))))
    { return AY_EOMEM; }


  fscanf(fileptr, "%d\n", &pomesh->type);
  fscanf(fileptr, "%u\n", &pomesh->npolys);
  if(!(pomesh->nloops = calloc(pomesh->npolys, sizeof(unsigned int))))
    { return AY_EOMEM; }
  for(i = 0; i < pomesh->npolys; i++)
    {
      fscanf(fileptr, "%u\n", &(pomesh->nloops[i]));
    }

  fscanf(fileptr, "%u\n", &total_loops);
  if(!(pomesh->nverts = calloc(total_loops, sizeof(unsigned int))))
    { return AY_EOMEM; }
  for(i = 0; i < total_loops; i++)
    {
      fscanf(fileptr, "%u\n", &(pomesh->nverts[i]));
    }

  fscanf(fileptr, "%u\n", &total_verts);
  if(!(pomesh->verts = calloc(total_verts, sizeof(unsigned int))))
    { return AY_EOMEM; }
  for(i = 0; i < total_verts; i++)
    {
      fscanf(fileptr, "%u\n", &(pomesh->verts[i]));
    }

  /* read controlv */
  fscanf(fileptr,"%u\n",&pomesh->ncontrols);
  fscanf(fileptr, "%d\n", &pomesh->has_normals);

  if(!(pomesh->controlv = calloc(pomesh->ncontrols * 3 + (pomesh->has_normals*
				 pomesh->ncontrols * 3), sizeof(double))))
    {return AY_EOMEM;}
  if(pomesh->has_normals)
    {
      a = 0;
      for(i = 0; i < pomesh->ncontrols; i++)
	{
	  fscanf(fileptr, "%lg %lg %lg\n", &(pomesh->controlv[a]),
		 &(pomesh->controlv[a+1]),
		 &(pomesh->controlv[a+2]));
	  a += 3;
	  fscanf(fileptr, "%lg %lg %lg\n", &(pomesh->controlv[a]),
		 &(pomesh->controlv[a+1]),
		 &(pomesh->controlv[a+2]));
	  a += 3;
	} /* for */
    }
  else
    {
      a = 0;
      for(i = 0; i < pomesh->ncontrols; i++)
	{
	  fscanf(fileptr, "%lg %lg %lg\n", &(pomesh->controlv[a]),
		 &(pomesh->controlv[a+1]),
		 &(pomesh->controlv[a+2]));
	  a += 3;
	} /* for */
    } /* if */

  o->refine = pomesh;

 return AY_OK;
} /* ay_pomesh_readcb */


int
ay_pomesh_writecb(FILE *fileptr, ay_object *o)
{
 ay_pomesh_object *pomesh = NULL;
 unsigned int total_loops = 0, total_verts = 0;
 int i = 0, a = 0;

  if(!o)
    return AY_ENULL;

  pomesh = (ay_pomesh_object *)(o->refine);

  fprintf(fileptr, "%d\n", pomesh->type);
  fprintf(fileptr, "%u\n", pomesh->npolys);

  /* write nloops */
  for(i = 0; i < pomesh->npolys; i++)
    {
      fprintf(fileptr, "%u\n", pomesh->nloops[i]);
      total_loops += pomesh->nloops[i];
    } /* for */

  /* write nverts */
  fprintf(fileptr, "%u\n", total_loops);
  for(i = 0; i < total_loops; i++)
    {
      fprintf(fileptr, "%u\n", pomesh->nverts[i]);
      total_verts += pomesh->nverts[i];
    } /* for */

  /* write verts */
  fprintf(fileptr, "%u\n", total_verts);
  for(i = 0; i < total_verts; i++)
    {
      fprintf(fileptr, "%u\n", pomesh->verts[i]);
    } /* for */

  /* write controlv */
  fprintf(fileptr, "%u\n", pomesh->ncontrols);
  fprintf(fileptr, "%d\n", pomesh->has_normals);
  if(pomesh->has_normals)
    {
      a = 0;
      for(i = 0; i < pomesh->ncontrols; i++)
	{
	  fprintf(fileptr,"%g %g %g\n", pomesh->controlv[a],
		  pomesh->controlv[a+1], pomesh->controlv[a+2]);
	  a += 3;
	  fprintf(fileptr,"%g %g %g\n", pomesh->controlv[a],
		  pomesh->controlv[a+1], pomesh->controlv[a+2]);
	  a += 3;
	}
    }
  else
    {
      a = 0;
      for(i = 0; i < pomesh->ncontrols; i++)
	{
	  fprintf(fileptr,"%g %g %g\n", pomesh->controlv[a],
		  pomesh->controlv[a+1], pomesh->controlv[a+2]);
	  a += 3;
	}
    }

 return AY_OK;
} /* ay_pomesh_writecb */


int
ay_pomesh_wribcb(char *file, ay_object *o)
{
 int ay_status = AY_OK;
 ay_pomesh_object *pomesh = NULL;
 RtInt *nloops = NULL, *nverts = NULL, *verts = NULL;
 RtPoint *controls = NULL, *normals = NULL;
 int i = 0, a = 0, stride = 0;
 unsigned int total_loops = 0, total_verts = 0;

  if(!o)
    return AY_OK;

  pomesh = (ay_pomesh_object*)(o->refine);

  if(pomesh->has_normals)
    stride = 6;
  else
    stride = 3;

  if(!(controls = calloc(pomesh->ncontrols, sizeof(RtPoint))))
    return AY_EOMEM;

  a = 0;
  for(i = 0; i < pomesh->ncontrols; i++)
    {
      controls[i][0] = (RtFloat)pomesh->controlv[a];
      controls[i][1] = (RtFloat)pomesh->controlv[a+1];
      controls[i][2] = (RtFloat)pomesh->controlv[a+2];
      a += stride;
    } /* for */

  if(pomesh->has_normals)
    {
      if(!(normals = calloc(pomesh->ncontrols, sizeof(RtPoint))))
	{
	  free(controls); return AY_EOMEM;
	}

      a = 3;
      for(i = 0; i < pomesh->ncontrols; i++)
	{
	  normals[i][0] = (RtFloat)pomesh->controlv[a];
	  normals[i][1] = (RtFloat)pomesh->controlv[a+1];
	  normals[i][2] = (RtFloat)pomesh->controlv[a+2];
	  a += stride;
	} /* for */
    } /* if */


  if(!(nloops = calloc(pomesh->npolys, sizeof(RtInt))))
    {
      free(controls); 
      if(normals)
	free(normals);
      return AY_EOMEM;
    }

  for(i = 0; i < pomesh->npolys; i++)
    {
      nloops[i] = (RtInt)(pomesh->nloops[i]);
      total_loops += pomesh->nloops[i];
    } /* for */
  
  if(!(nverts = calloc(total_loops, sizeof(RtInt))))
    {
      free(controls); 
      if(normals)
	free(normals);
      free(nloops);
      return AY_EOMEM;
    }

  for(i = 0; i < total_loops; i++)
    {
      nverts[i] = (RtInt)(pomesh->nverts[i]);
      total_verts += pomesh->nverts[i];
    } /* for */

  if(!(verts = calloc(total_verts, sizeof(RtInt))))
    {
      free(controls);
      if(normals)
	free(normals);
      free(nloops);
      free(nverts);
      return AY_EOMEM; }

  for(i = 0; i < total_verts; i++)
    {
      verts[i] = (RtInt)(pomesh->verts[i]);
    } /* for */

  if(pomesh->has_normals)
    RiPointsGeneralPolygons((RtInt)pomesh->npolys, nloops, nverts, verts,
			    "P", controls, "N", normals, NULL);
  else
    RiPointsGeneralPolygons((RtInt)pomesh->npolys, nloops, nverts, verts,
			  "P", controls, NULL);

  /* clean up */
  free(controls);
  if(normals)
    free(normals);
  free(nloops);
  free(nverts);
  free(verts);

 return ay_status;
} /* ay_pomesh_wribcb */


int
ay_pomesh_bbccb(ay_object *o, double *bbox, int *flags)
{
 double xmin, xmax, ymin, ymax, zmin, zmax;
 double *controlv = NULL;
 int i, a, stride = 0;
 ay_pomesh_object *pomesh = NULL;

  if(!o || !bbox)
    return AY_ENULL;

  pomesh = (ay_pomesh_object *)o->refine; 

  if(pomesh->has_normals)
    stride = 6;
  else
    stride = 3;

  controlv = pomesh->controlv;

  xmin = controlv[0];
  xmax = xmin;
  ymin = controlv[1];
  ymax = ymin;
  zmin = controlv[2];
  zmax = zmin;

  a = 0;
  for(i = 0; i < pomesh->ncontrols; i++)
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

      a += stride;
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
} /* ay_pomesh_bbccb */


int
ay_pomesh_notifycb(ay_object *o)
{
  /* int ay_status = AY_OK;*/

 return AY_OK;
} /* ay_pomesh_notifycb */


int
ay_pomesh_convertcb(ay_object *o)
{
 int ay_status = AY_OK;
 ay_pomesh_object *pomesh = NULL;

  if(!o)
    return AY_ENULL;

  pomesh = (ay_pomesh_object *) o->refine;

 return ay_status;
} /* ay_pomesh_convertcb */


int
ay_pomesh_init(Tcl_Interp *interp)
{
 int ay_status = AY_OK;

  ay_status = ay_otype_registercore(ay_pomesh_name,
				    ay_pomesh_createcb,
				    ay_pomesh_deletecb,
				    ay_pomesh_copycb,
				    ay_pomesh_drawcb,
				    ay_pomesh_drawhcb,
				    ay_pomesh_shadecb,
				    ay_pomesh_setpropcb,
				    ay_pomesh_getpropcb,
				    ay_pomesh_getpntcb,
				    ay_pomesh_readcb,
				    ay_pomesh_writecb,
				    ay_pomesh_wribcb,
				    ay_pomesh_bbccb,
				    AY_IDPOMESH);

  /*
    ay_status = ay_notify_register(ay_pomesh_notifycb, AY_IDPOMESH);

    ay_status = ay_convert_register(ay_pomesh_convertcb, AY_IDPOMESH);
  */

 return ay_status;
} /* ay_pomesh_init */

