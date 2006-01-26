/*
 * Ayam, a free 3D modeler for the RenderMan interface.
 *
 * Ayam is copyrighted 1998-2002 by Randolf Schultz
 * (rschultz@informatik.uni-rostock.de) and others.
 *
 * All rights reserved.
 *
 * See the file License for details.
 *
 */

#include "ayam.h"

/* sdmesh.c - SubDivisionMesh object */

static char *ay_sdmesh_name = "SDMesh";

int
ay_sdmesh_createcb(int argc, char *argv[], ay_object *o)
{
 ay_sdmesh_object *sdmesh = NULL;

  if(!(sdmesh = calloc(1, sizeof(ay_sdmesh_object))))
    {
      return AY_EOMEM;
    }

  o->refine = (void *)sdmesh;

 return AY_OK;
} /* ay_sdmesh_createcb */


int
ay_sdmesh_deletecb(void *c)
{
 ay_sdmesh_object *sdmesh = NULL;

  if(!c)
    return AY_ENULL;    

  sdmesh = (ay_sdmesh_object *)(c);

  /* free nverts */
  if(sdmesh->nverts)
    free(sdmesh->nverts);

  /* free verts */
  if(sdmesh->verts)
    free(sdmesh->verts);

  /* free tags and their args */
  if(sdmesh->tags)
    free(sdmesh->tags);
  if(sdmesh->nargs)
    free(sdmesh->nargs);
  if(sdmesh->intargs)
    free(sdmesh->intargs);
  if(sdmesh->floatargs)
    free(sdmesh->floatargs);

  /* free controlv */
  if(sdmesh->controlv)
    free(sdmesh->controlv);

  free(sdmesh);

 return AY_OK;
} /* ay_sdmesh_deletecb */


int
ay_sdmesh_copycb(void *src, void **dst)
{
 ay_sdmesh_object *sdmesh = NULL, *sdmeshsrc = NULL;
 unsigned int i, total_verts = 0, total_intargs = 0, total_floatargs = 0;

  if(!src || !dst)
    return AY_ENULL;

  sdmeshsrc = (ay_sdmesh_object *)src;

  if(!(sdmesh = calloc(1, sizeof(ay_sdmesh_object))))
    return AY_EOMEM; 

  memcpy(sdmesh, src, sizeof(ay_sdmesh_object)); 

  sdmesh->nverts = NULL;
  sdmesh->verts = NULL;
  sdmesh->tags = NULL;
  sdmesh->nargs = NULL;
  sdmesh->intargs = NULL;
  sdmesh->floatargs = NULL;
  sdmesh->controlv = NULL;

  /* copy nverts */
  if(sdmeshsrc->nverts)
    {
      if(!(sdmesh->nverts = calloc(sdmeshsrc->nfaces, sizeof(unsigned int))))
	return AY_EOMEM;
      memcpy(sdmesh->nverts, sdmeshsrc->nverts,
	     sdmeshsrc->nfaces * sizeof(unsigned int));

      for(i = 0; i < sdmeshsrc->nfaces; i++)
	{
	  total_verts += sdmeshsrc->nverts[i];
	} /* for */
    } /* if */

  /* copy verts */
  if(sdmeshsrc->verts)
    {
      if(!(sdmesh->verts = calloc(total_verts, sizeof(unsigned int))))
	return AY_EOMEM;
      memcpy(sdmesh->verts, sdmeshsrc->verts,
	     total_verts * sizeof(unsigned int));
    }

  /* copy tags and their args */
  if(sdmeshsrc->tags)
    {
      if(!(sdmesh->tags = calloc(sdmeshsrc->ntags, sizeof(int))))
	return AY_EOMEM;
      memcpy(sdmesh->tags, sdmeshsrc->tags,
	     sdmeshsrc->ntags * sizeof(int));

      if(!(sdmesh->nargs = calloc(2 * sdmeshsrc->ntags, sizeof(unsigned int))))
	return AY_EOMEM;
      memcpy(sdmesh->nargs, sdmeshsrc->nargs,
	     2 * sdmeshsrc->ntags * sizeof(unsigned int));

      for(i = 0; i < (2 * sdmeshsrc->ntags); i += 2)
	{
	  total_intargs += sdmeshsrc->nargs[i];
	  total_floatargs += sdmeshsrc->nargs[i+1];
	} /* for */

      if(!(sdmesh->intargs = calloc(total_intargs, sizeof(int))))
	return AY_EOMEM;
      memcpy(sdmesh->intargs, sdmeshsrc->intargs,
	     total_intargs * sizeof(int));

      if(!(sdmesh->floatargs = calloc(total_floatargs, sizeof(double))))
	return AY_EOMEM;
      memcpy(sdmesh->floatargs, sdmeshsrc->floatargs,
	     total_floatargs * sizeof(double));
    } /* if */

  /* copy controlv */
  if(sdmeshsrc->controlv)
    {
      if(!(sdmesh->controlv = calloc(3 * sdmeshsrc->ncontrols,
				     sizeof(double))))
	return AY_EOMEM;
      memcpy(sdmesh->controlv, sdmeshsrc->controlv,
	     3 * sdmesh->ncontrols * sizeof(double));
    }

  *dst = (void *)sdmesh;

 return AY_OK;
} /* ay_sdmesh_copycb */


int
ay_sdmesh_drawcb(struct Togl *togl, ay_object *o)
{
 ay_sdmesh_object *sdmesh = NULL;
 unsigned int i = 0, k = 0, m = 0, n = 0;
 unsigned int a;

  if(!o)
    return AY_ENULL;

  sdmesh = (ay_sdmesh_object *)(o->refine);

  for(i = 0; i < sdmesh->nfaces; i++)
    {
      glBegin(GL_LINE_LOOP);
       for(k = 0; k < sdmesh->nverts[m]; k++)
	 {
	   a = sdmesh->verts[n++];
	   glVertex3dv((GLdouble*)(&(sdmesh->controlv[a * 3])));
	 }
      glEnd();
      m++;
    } /* for */

 return AY_OK;
} /* ay_sdmesh_drawcb */


int
ay_sdmesh_shadecb(struct Togl *togl, ay_object *o)
{
 int ay_status = AY_OK;
 ay_sdmesh_object *sdmesh = NULL;

  if(!o)
    return AY_ENULL;

  sdmesh = (ay_sdmesh_object *)(o->refine);


  ay_status = ay_sdmesht_tesselate(sdmesh);

 return AY_OK;
} /* ay_sdmesh_shadecb */


int
ay_sdmesh_drawhcb(struct Togl *togl, ay_object *o)
{
 ay_sdmesh_object *sdmesh = NULL;
 GLdouble *ver = NULL;
 double point_size = ay_prefs.handle_size;
 unsigned int i = 0;

  if(!o)
    return AY_ENULL;

  sdmesh = (ay_sdmesh_object *)(o->refine);

  ver = sdmesh->controlv;

  glPointSize((GLfloat)point_size);

  glBegin(GL_POINTS);
   for(i = 0; i < sdmesh->ncontrols; i++)
     {
       glVertex3dv(ver);
       ver += 3;
     }
  glEnd();

 return AY_OK;
} /* ay_sdmesh_drawhcb */


int
ay_sdmesh_getpntcb(int mode, ay_object *o, double *p)
{
 ay_sdmesh_object *sdmesh = NULL;
 double min_dist = ay_prefs.pick_epsilon, dist = 0.0;
 double *pecoord = NULL, **pecoords = NULL, *control = NULL, *c = NULL;
 unsigned int i = 0, j = 0, k = 0, a = 0, numfound = 0;

  if(!o || !p)
    return AY_ENULL;

  sdmesh = (ay_sdmesh_object *)(o->refine);

  if(min_dist == 0.0)
    min_dist = DBL_MAX;

  if(ay_point_edit_coords)
    free(ay_point_edit_coords);

  ay_point_edit_coords = NULL;

  /* select all points? */
  if(mode == 0)
    { /* yes */

      if(!(ay_point_edit_coords = calloc(sdmesh->ncontrols, sizeof(double*))))
	return AY_EOMEM;

      for(i = 0; i < sdmesh->ncontrols; i++)
	{
	  ay_point_edit_coords[i] = &(sdmesh->controlv[a]);
	  a += 3;
	} /* for */

      ay_point_edit_coords_homogenous = AY_FALSE;
      ay_point_edit_coords_number = sdmesh->ncontrols;
    }
  else
    { /* no */

      /* selection based on a single point? */
      if(mode == 1)
	{ /* yes */

	  control = sdmesh->controlv;
	  for(i = 0; i < sdmesh->ncontrols; i++)
	    {
	      dist = AY_VLEN((p[0] - control[j]),
			     (p[1] - control[j+1]),
			     (p[2] - control[j+2]));

	      if(dist < min_dist)
		{
		  pecoord = &(control[j]);
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

	      j += 3;
	    } /* for */

	  if(!pecoord)
	    return AY_OK; /* XXXX should this return a 'AY_EPICK' ? */

	  ay_point_edit_coords_homogenous = AY_FALSE;
	  ay_point_edit_coords_number = numfound;
	  if(!(ay_point_edit_coords = calloc(numfound, sizeof(double*))))
	    return AY_EOMEM;

	  j = 0;
	  for(i = 0; i < sdmesh->ncontrols; i++)
	    {
	      dist = AY_VLEN((p[0] - control[j]),
			     (p[1] - control[j+1]),
			     (p[2] - control[j+2]));

	      if(dist == min_dist)
		{
		  ay_point_edit_coords[k] = &(control[j]);
		  k++;
		} /* if */
	      j += 3;
	    } /* for */

	}
      else
	{ /* no */

	  /* selection based on planes */
	  control = sdmesh->controlv;
	  j = 0;
	  a = 0;
	  for(i = 0; i < sdmesh->ncontrols; i++)
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
	      j += 3;
	    } /* for */

	  if(!pecoords)
	    return AY_OK; /* XXXX should this return a 'AY_EPICK' ? */

	  ay_point_edit_coords_homogenous = AY_FALSE;
	  ay_point_edit_coords = pecoords;
	  ay_point_edit_coords_number = a;

	} /* if */
    } /* if */

 return AY_OK;
} /* ay_sdmesh_getpntcb */


/* Tcl -> C! */
int
ay_sdmesh_setpropcb(Tcl_Interp *interp, int argc, char *argv[], ay_object *o)
{
 int ay_status = AY_OK;
 char *n1 = "SDMeshAttrData";
 /*char fname[] = "sdmesh_setpropcb";*/
 Tcl_Obj *to = NULL, *toa = NULL, *ton = NULL;
 ay_sdmesh_object *sdmesh = NULL;
 int new_scheme;

  if(!o)
    return AY_ENULL;

  sdmesh = (ay_sdmesh_object *)o->refine;
  
  toa = Tcl_NewStringObj(n1, -1);
  ton = Tcl_NewStringObj(n1, -1);

  Tcl_SetStringObj(ton, "Scheme", -1);
  to = Tcl_ObjGetVar2(interp, toa, ton, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp, to, &new_scheme);

  sdmesh->scheme = new_scheme;

  Tcl_IncrRefCount(toa);Tcl_DecrRefCount(toa);
  Tcl_IncrRefCount(ton);Tcl_DecrRefCount(ton);

  ay_status = ay_notify_parent();

 return AY_OK;
} /* ay_sdmesh_setpropcb */


/* C -> Tcl! */
int
ay_sdmesh_getpropcb(Tcl_Interp *interp, int argc, char *argv[], ay_object *o)
{
 char *n1="SDMeshAttrData";
 Tcl_Obj *to = NULL, *toa = NULL, *ton = NULL;
 ay_sdmesh_object *sdmesh = NULL;

  if(!o)
    return AY_ENULL;

  sdmesh = (ay_sdmesh_object *)(o->refine);

  toa = Tcl_NewStringObj(n1, -1);

  ton = Tcl_NewStringObj(n1, -1);


  Tcl_SetStringObj(ton, "Scheme", -1);
  to = Tcl_NewIntObj(sdmesh->scheme);
  Tcl_ObjSetVar2(interp, toa, ton, to, TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton, "NFaces", -1);
  to = Tcl_NewIntObj((int)sdmesh->nfaces);
  Tcl_ObjSetVar2(interp, toa, ton, to, TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton, "NControls", -1);
  to = Tcl_NewIntObj((int)sdmesh->ncontrols);
  Tcl_ObjSetVar2(interp, toa, ton, to, TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);

  Tcl_IncrRefCount(toa);Tcl_DecrRefCount(toa);
  Tcl_IncrRefCount(ton);Tcl_DecrRefCount(ton);

 return AY_OK;
} /* ay_sdmesh_getpropcb */


int
ay_sdmesh_readcb(FILE *fileptr, ay_object *o)
{
 ay_sdmesh_object *sdmesh = NULL;
 unsigned int total_verts = 0, total_intargs = 0, total_floatargs = 0;
 unsigned int i, a;

  if(!o)
   return AY_ENULL;

  if(!(sdmesh = calloc(1, sizeof(ay_sdmesh_object))))
    { return AY_EOMEM; }

  fscanf(fileptr, "%d\n", &sdmesh->scheme);
  fscanf(fileptr, "%u\n", &sdmesh->nfaces);

  /* read nverts */
  if(!(sdmesh->nverts = calloc(sdmesh->nfaces, sizeof(unsigned int))))
    { return AY_EOMEM; }
  for(i = 0; i < sdmesh->nfaces; i++)
    {
      fscanf(fileptr, "%u\n", &(sdmesh->nverts[i]));
    }

  /* read verts */
  fscanf(fileptr, "%u\n", &total_verts);
  if(!(sdmesh->verts = calloc(total_verts, sizeof(unsigned int))))
    { return AY_EOMEM; }
  for(i = 0; i < total_verts; i++)
    {
      fscanf(fileptr, "%u\n", &(sdmesh->verts[i]));
    }

  /* read tags and their args */
  fscanf(fileptr, "%u\n", &sdmesh->ntags);
  if(sdmesh->ntags > 0)
    {
      if(!(sdmesh->tags = calloc(sdmesh->ntags, sizeof(int))))
	{ return AY_EOMEM; }
      for(i = 0; i < sdmesh->ntags; i++)
	{
	  fscanf(fileptr, "%d\n", &(sdmesh->tags[i]));
	}

      if(!(sdmesh->nargs = calloc(2 * sdmesh->ntags, sizeof(unsigned int))))
	{ return AY_EOMEM; }
      for(i = 0; i < (2 * sdmesh->ntags); i++)
	{
	  fscanf(fileptr, "%u\n", &(sdmesh->nargs[i]));
	}

      fscanf(fileptr, "%u\n", &total_intargs);
      if(!(sdmesh->intargs = calloc(total_intargs, sizeof(int))))
	{ return AY_EOMEM; }
      for(i = 0; i < total_intargs; i++)
	{
	  fscanf(fileptr, "%d\n", &(sdmesh->intargs[i]));
	}

      fscanf(fileptr, "%u\n", &total_floatargs);
      if(!(sdmesh->floatargs = calloc(total_floatargs, sizeof(double))))
	{ return AY_EOMEM; }
      for(i = 0; i < total_floatargs; i++)
	{
	  fscanf(fileptr, "%lg\n", &(sdmesh->floatargs[i]));
	}
    } /* if */

  /* read controlv */
  fscanf(fileptr,"%u\n", &sdmesh->ncontrols);

  if(!(sdmesh->controlv = calloc(sdmesh->ncontrols * 3, sizeof(double))))
    {return AY_EOMEM;}

  a = 0;
  for(i = 0; i < sdmesh->ncontrols; i++)
    {
      fscanf(fileptr, "%lg %lg %lg\n", &(sdmesh->controlv[a]),
	     &(sdmesh->controlv[a+1]),
	     &(sdmesh->controlv[a+2]));
      a += 3;
    } /* for */

  o->refine = sdmesh;

 return AY_OK;
} /* ay_sdmesh_readcb */


int
ay_sdmesh_writecb(FILE *fileptr, ay_object *o)
{
 ay_sdmesh_object *sdmesh = NULL;
 unsigned int total_verts = 0, total_intargs = 0, total_floatargs = 0;
 unsigned int i = 0, a = 0;

  if(!o)
    return AY_ENULL;

  sdmesh = (ay_sdmesh_object *)(o->refine);

  fprintf(fileptr, "%d\n", sdmesh->scheme);
  fprintf(fileptr, "%u\n", sdmesh->nfaces);

  /* write nverts */
  for(i = 0; i < sdmesh->nfaces; i++)
    {
      fprintf(fileptr, "%u\n", sdmesh->nverts[i]);
      total_verts += sdmesh->nverts[i];
    } /* for */

  /* write verts */
  fprintf(fileptr, "%u\n", total_verts);
  for(i = 0; i < total_verts; i++)
    {
      fprintf(fileptr, "%u\n", sdmesh->verts[i]);
    } /* for */

  /* write tags and their args */
  fprintf(fileptr, "%u\n", sdmesh->ntags);
  if(sdmesh->ntags > 0)
    {
      for(i = 0; i < sdmesh->ntags; i++)
	{
	  fprintf(fileptr, "%d\n", sdmesh->tags[i]);
	} /* for */

      for(i = 0; i < (2 * sdmesh->ntags); i++)
	{
	  fprintf(fileptr, "%u\n", sdmesh->nargs[i]);
	} /* for */

      for(i = 0; i < (2 * sdmesh->ntags); i += 2)
	{
	  total_intargs += sdmesh->nargs[i];
	  total_floatargs += sdmesh->nargs[i+1];
	} /* for */

      fprintf(fileptr, "%u\n", total_intargs);
      for(i = 0; i < total_intargs; i++)
	{
	  fprintf(fileptr, "%d\n", sdmesh->intargs[i]);
	} /* for */

      fprintf(fileptr, "%u\n", total_floatargs);
      for(i = 0; i < total_floatargs; i++)
	{
	  fprintf(fileptr, "%g\n", sdmesh->floatargs[i]);
	} /* for */
    } /* if */

  /* write controlv */
  fprintf(fileptr, "%u\n", sdmesh->ncontrols);

  a = 0;
  for(i = 0; i < sdmesh->ncontrols; i++)
    {
      fprintf(fileptr,"%g %g %g\n", sdmesh->controlv[a],
	      sdmesh->controlv[a+1], sdmesh->controlv[a+2]);
      a += 3;
    }

 return AY_OK;
} /* ay_sdmesh_writecb */


int
ay_sdmesh_wribcb(char *file, ay_object *o)
{
 int ay_status = AY_OK;
 ay_sdmesh_object *sdmesh = NULL;
 RtToken scheme = NULL, ccscheme = "catmull-clark", lscheme = "loop";
 RtInt *nverts = NULL, *verts = NULL;
 RtPoint *controls = NULL;
 RtToken *tags = NULL;
 RtToken holetoken = "hole", creasetoken = "crease", cornertoken = "corner",
   ibtoken = "interpolateboundary";
 RtInt *nargs = NULL, *intargs = NULL;
 RtFloat *floatargs = NULL;
 RtToken *tokens = NULL;
 RtPointer *parms = NULL;
 unsigned int i = 0, a = 0;
 unsigned int total_verts = 0, total_intargs = 0, total_floatargs = 0;
 int j, n = 0, pvc = 0;

  if(!o)
    return AY_OK;

  sdmesh = (ay_sdmesh_object*)(o->refine);

  switch(sdmesh->scheme)
    {
    case AY_SDSCATMULL:
      scheme = ccscheme;
      break;
    case AY_SDSLOOP:
      scheme = lscheme;
      break;
    default:
      break;
    }

  if(!(controls = calloc(sdmesh->ncontrols, sizeof(RtPoint))))
    return AY_EOMEM;

  a = 0;
  for(i = 0; i < sdmesh->ncontrols; i++)
    {
      controls[i][0] = (RtFloat)sdmesh->controlv[a];
      controls[i][1] = (RtFloat)sdmesh->controlv[a+1];
      controls[i][2] = (RtFloat)sdmesh->controlv[a+2];
      a += 3;
    } /* for */

  if(!(nverts = calloc(sdmesh->nfaces, sizeof(RtInt))))
    {
      free(controls); return AY_EOMEM;
    }

  for(i = 0; i < sdmesh->nfaces; i++)
    {
      nverts[i] = (RtInt)(sdmesh->nverts[i]);
      total_verts += sdmesh->nverts[i];
    } /* for */
  
  if(!(verts = calloc(total_verts, sizeof(RtInt))))
    {
      free(controls); free(nverts); return AY_EOMEM;
    }

  for(i = 0; i < total_verts; i++)
    {
      verts[i] = (RtInt)(sdmesh->verts[i]);
    } /* for */

  if(sdmesh->ntags > 0)
    {
      if(!(tags = calloc(sdmesh->ntags, sizeof(RtToken))))
	{
	  free(controls); free(nverts); free(verts); return AY_EOMEM;
	}
      for(i = 0; i < sdmesh->ntags; i++)
	{
	  switch(sdmesh->tags[i])
	    {
	    case AY_SDTHOLE:
	      tags[i] = holetoken /*RI_HOLE*/;
	      break;
	    case AY_SDTCREASE:
	      tags[i] = creasetoken /*RI_CREASE*/;
	      break;
	    case AY_SDTCORNER:
	      tags[i] = cornertoken /*RI_CORNER*/;
	      break;
	    case AY_SDTIB:
	      tags[i] = ibtoken /*RI_INTERPOLATEBOUNDARY*/;
	      break;
	    default:
	      break;
	    }
	} /* for */

      if(!(nargs = calloc(total_verts, sizeof(RtInt))))
	{
	  free(controls); free(nverts); free(verts); free(tags);
	  return AY_EOMEM;
	}
      for(i = 0; i < (2 * sdmesh->ntags); i++)
	{
	  nargs[i] = (RtInt)(sdmesh->nargs[i]);
	}

      for(i = 0; i < (2 * sdmesh->ntags); i += 2)
	{
	  total_intargs += sdmesh->nargs[i];
	  total_floatargs += sdmesh->nargs[i+1]; 
	}

      if(!(intargs = calloc(total_intargs, sizeof(RtInt))))
	{
	  free(controls); free(nverts); free(verts); free(tags); free(nargs);
	  return AY_EOMEM;
	}
      for(i = 0; i < total_intargs; i++)
	{
	  intargs[i] = (RtInt)(sdmesh->intargs[i]);
	}

      if(!(floatargs = calloc(total_floatargs, sizeof(RtFloat))))
	{
	  free(controls); free(nverts); free(verts); free(tags); free(nargs);
	  if(intargs){free(intargs);} return AY_EOMEM;
	}
      for(i = 0; i < total_floatargs; i++)
	{
	  floatargs[i] = (RtFloat)(sdmesh->floatargs[i]);
	}
    }
  
  /* Do we have any primitive variables? */
  if(!(pvc = ay_pv_count(o)))
    {
      /* No */

      if(sdmesh->ntags > 0)
	{
	  /* export mesh with tags */
	  RiSubdivisionMesh(scheme, (RtInt)sdmesh->nfaces, nverts, verts,
			(RtInt)sdmesh->ntags, tags, nargs, intargs, floatargs,
			    "P", controls, NULL);
	}
      else
	{
	  /* export mesh without tags */
	  RiSubdivisionMesh(scheme, (RtInt)sdmesh->nfaces, nverts, verts,
			    (RtInt)0, NULL, NULL, NULL, NULL,
			    "P", controls, NULL);
	}
    }
  else
    {
      /* Yes, we have primitive variables. */
      if(!(tokens = calloc(pvc+1, sizeof(RtToken))))
	return AY_EOMEM;

      if(!(parms = calloc(pvc+1, sizeof(RtPointer))))
	return AY_EOMEM;
      
      tokens[0] = "P";
      parms[0] = (RtPointer)controls;

      n = 1;
      ay_pv_filltokpar(o, AY_TRUE, 1, &n, tokens, parms);

      if(sdmesh->ntags > 0)
	{
	  /* export mesh with tags */
	  RiSubdivisionMeshV(scheme, (RtInt)sdmesh->nfaces, nverts, verts,
			(RtInt)sdmesh->ntags, tags, nargs, intargs, floatargs,
			    (RtInt)n, tokens, parms);
	}
      else
	{
	  /* export mesh without tags */
	  RiSubdivisionMeshV(scheme, (RtInt)sdmesh->nfaces, nverts, verts,
			    (RtInt)0, NULL, NULL, NULL, NULL,
			    (RtInt)n, tokens, parms);
	} /* if */

      for(j = 1; j < n; j++)
	{
	  free(tokens[j]);
	  free(parms[j]);
	}

      free(tokens);
      free(parms);

    } /* if */

  /* clean up */
  free(controls);
  free(nverts);
  free(verts);
  if(sdmesh->ntags > 0)
    {
      free(tags);
      free(nargs);
      if(intargs)
	free(intargs);
      if(floatargs)
	free(floatargs);
    }

 return ay_status;
} /* ay_sdmesh_wribcb */


int
ay_sdmesh_bbccb(ay_object *o, double *bbox, int *flags)
{
 double xmin, xmax, ymin, ymax, zmin, zmax;
 double *controlv = NULL;
 unsigned int i, a;
 ay_sdmesh_object *sdmesh = NULL;

  if(!o || !bbox)
    return AY_ENULL;

  sdmesh = (ay_sdmesh_object *)o->refine; 

  controlv = sdmesh->controlv;

  xmin = controlv[0];
  xmax = xmin;
  ymin = controlv[1];
  ymax = ymin;
  zmin = controlv[2];
  zmax = zmin;

  a = 0;
  for(i = 0; i < sdmesh->ncontrols; i++)
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

      a += 3;
    } /* if */

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
} /* ay_sdmesh_bbccb */


int
ay_sdmesh_notifycb(ay_object *o)
{
  /* int ay_status = AY_OK;*/

 return AY_OK;
} /* ay_sdmesh_notifycb */


int
ay_sdmesh_convertcb(ay_object *o, int in_place)
{
 int ay_status = AY_OK;
 ay_sdmesh_object *sdmesh = NULL;

  if(!o)
    return AY_ENULL;

  sdmesh = (ay_sdmesh_object *) o->refine;

 return ay_status;
} /* ay_sdmesh_convertcb */


int
ay_sdmesh_init(Tcl_Interp *interp)
{
 int ay_status = AY_OK;

  ay_status = ay_otype_registercore(ay_sdmesh_name,
				    ay_sdmesh_createcb,
				    ay_sdmesh_deletecb,
				    ay_sdmesh_copycb,
				    ay_sdmesh_drawcb,
				    ay_sdmesh_drawhcb,
				    ay_sdmesh_shadecb,
				    ay_sdmesh_setpropcb,
				    ay_sdmesh_getpropcb,
				    ay_sdmesh_getpntcb,
				    ay_sdmesh_readcb,
				    ay_sdmesh_writecb,
				    ay_sdmesh_wribcb,
				    ay_sdmesh_bbccb,
				    AY_IDSDMESH);

  /*
    ay_status = ay_notify_register(ay_sdmesh_notifycb, AY_IDSDMESH);

    ay_status = ay_convert_register(ay_sdmesh_convertcb, AY_IDSDMESH);
  */

 return ay_status;
} /* ay_sdmesh_init */

