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

/* pamesh.c - pamesh object */

static char *ay_pamesh_name = "PatchMesh";

int
ay_pamesh_createcb(int argc, char *argv[], ay_object *o)
{
 int ay_status = AY_OK;
 char fname[] = "crtpamesh";
 int width = 4, height = 4;
 int i = 0, j = 0, k = 0;
 double *cv = NULL, dx = 0.25;
 ay_pamesh_object *p = NULL;

  if(!o)
    return AY_ENULL;

  /* parse args */
  while(i+1 < argc)
    {
      if(!strcmp(argv[i],"-width"))
	{
	  Tcl_GetInt(ay_interp, argv[i+1], &width);
	  if(width <= 0) width = 4;
	  i+=2;
	}
      else
      if(!strcmp(argv[i],"-height"))
	{
	  Tcl_GetInt(ay_interp, argv[i+1], &height);
	  if(height <= 0) height = 4;
	  i+=2;
	}
      else
	i++;
    }

  if(!(p = calloc(1,sizeof(ay_pamesh_object))))
    {
      return AY_EOMEM;
    }

  p->width = width;
  p->height = height;

  p->type = AY_PTBICUBIC;
  p->btype_u = AY_BTBEZIER;
  p->btype_v = AY_BTBEZIER;

  if(!(cv = calloc(width*height*4,sizeof(double))))
    {
      free(p);
      return AY_EOMEM;
    }
  
  k = 0;
  for(i=0;i<(width);i++)
    {
      for(j=0;j<(height);j++)
	{
	  cv[k]   = (double)i*dx;
	  cv[k+1] = (double)j*dx;
	  cv[k+3] = 1.0;
	  k += 4;
	}
    }


  p->controlv = cv;
  o->refine = (void *)p;

 return AY_OK;
} /* ay_pamesh_createcb */


int
ay_pamesh_deletecb(void *c)
{
 ay_pamesh_object *pamesh = NULL;

  if(!c)
    return AY_ENULL;    

  pamesh = (ay_pamesh_object *)(c);

  /* free controlv */
  if(pamesh->controlv)
    free(pamesh->controlv);

  /* free ubasis */
  if(pamesh->ubasis)
    free(pamesh->ubasis);

  /* free vbasis */
  if(pamesh->vbasis)
    free(pamesh->vbasis);

  free(pamesh);

 return AY_OK;
} /* ay_pamesh_deletecb */


int
ay_pamesh_copycb(void *src, void **dst)
{
 ay_pamesh_object *pamesh = NULL, *pameshsrc = NULL;

  if(!src || !dst)
    return AY_ENULL;

  pameshsrc = (ay_pamesh_object *)src;

  if(!(pamesh = calloc(1, sizeof(ay_pamesh_object))))
    return AY_EOMEM; 

  memcpy(pamesh, src, sizeof(ay_pamesh_object)); 

  pamesh->controlv = NULL;
  pamesh->ubasis = NULL;
  pamesh->vbasis = NULL;

  /* copy controlv */
  if(!(pamesh->controlv = calloc(4 * pamesh->width * pamesh->height,
				 sizeof(double))))
    return AY_EOMEM;
  memcpy(pamesh->controlv, pameshsrc->controlv,
	 4 * pamesh->width * pamesh->height * sizeof(double));

  /* copy ubasis */
  if(pameshsrc->ubasis)
    {
      if(!(pamesh->ubasis = calloc(16, sizeof(double))))
	return AY_EOMEM;
      memcpy(pamesh->ubasis, pameshsrc->ubasis, 16 * sizeof(double));
    }

  /* copy vbasis */
  if(pameshsrc->vbasis)
    {
      if(!(pamesh->vbasis = calloc(16, sizeof(double))))
	return AY_EOMEM;
      memcpy(pamesh->vbasis, pameshsrc->vbasis, 16 * sizeof(double));
    }


  *dst = (void *)pamesh;

 return AY_OK;
} /* ay_pamesh_copycb */


int
ay_pamesh_drawchcb(struct Togl *togl, ay_object *o)
{
 ay_pamesh_object *pamesh = NULL;
 double *ver = NULL;
 int i, j, a, width, height;

  if(!o)
    return AY_ENULL;

  pamesh = (ay_pamesh_object *)o->refine;

  if(!pamesh)
    return AY_ENULL;

  width = pamesh->width;
  height = pamesh->height;

  ver = pamesh->controlv;
  a = 0;
  for(i = 0; i < width; i++)
    {
      glBegin(GL_LINE_STRIP);
	for(j = 0; j < height; j++)
	  {
	    glVertex3dv((GLdouble *)&ver[a]);
	    a += 4;
	  }
      glEnd();
    }
  
  a = 0;
  for(j = 0; j < height; j++)
    {
      a = j * 4;
      
      glBegin(GL_LINE_STRIP);
	for(i = 0; i < width; i++)
	  {
	    glVertex3dv((GLdouble *)&ver[a]);

	    a += (4 * height);
	  }
      glEnd();
    }

 return AY_OK;
} /* ay_pamesh_drawchcb */


int
ay_pamesh_drawcb(struct Togl *togl, ay_object *o)
{
 int display_mode = ay_prefs.glu_display_mode;
 ay_pamesh_object *pamesh = NULL;

  if(!o)
    return AY_ENULL;

  pamesh = (ay_pamesh_object *)o->refine;

  if(!pamesh)
    return AY_ENULL;

  ay_pamesh_drawchcb(togl, o);

#if 0
  if(pamesh->glu_display_mode != 0)
    {
      display_mode = pamesh->glu_display_mode-1;
    }

  if(display_mode == 0)
    ay_pamesh_drawchcb(togl, o);

  if(display_mode == 1)
    ay_pamesh_drawglucb(togl, o);

  if(display_mode == 2)
    ay_pamesh_drawglucb(togl, o);
#endif

  return AY_OK;
}


int
ay_pamesh_shadecb(struct Togl *togl, ay_object *o)
{
 int ay_status = AY_OK;
 ay_pamesh_object *pm = NULL;
 static ay_nurbpatch_object *np = NULL;
 static ay_object p;

  pm = (ay_pamesh_object *)o->refine;

  if(pm->type == AY_PTBICUBIC)
    {
      if(pm->width == 4 && pm->height == 4)
	{
	  if(!np)
	    {
	      ay_npt_create(4,4,4,4,
			    AY_KTBEZIER, AY_KTBEZIER, pm->controlv, NULL, NULL,
			    &np);
	    }
	  else
	    {
	      np->controlv = pm->controlv;
	    }
	  memset(&p, 0, sizeof(ay_object));
	  ay_object_defaults(&p);
	  p.refine = np;
	  p.type = AY_IDNPATCH;
	  ay_shade_object(togl, &p);

	}
    }
  else
    {
      /* shade bilinear patch mesh */


    }


 return AY_OK;
} /* ay_pamesh_shadecb */


int
ay_pamesh_drawhcb(struct Togl *togl, ay_object *o)
{
 int width = 0, height = 0, i = 0, a = 0;
 ay_pamesh_object *patch = (ay_pamesh_object *) o->refine;
 GLdouble *ver = NULL;
 double point_size = ay_prefs.handle_size;

  width = patch->width;
  height = patch->height;

  ver = patch->controlv;

  glPointSize((GLfloat)point_size);

  glBegin(GL_POINTS);
  for(i=0; i<(width*height); i++)
    {
      glVertex3dv((GLdouble *)&ver[a]);
      a += 4;
    }
  glEnd();

  /* draw arrows */
  ay_draw_arrow(togl, &(ver[width*height*4-8]), &(ver[width*height*4-4])); 

 return AY_OK;
} /* ay_pamesh_drawhcb */


int
ay_pamesh_getpntcb(ay_object *o, double *p)
{
 ay_pamesh_object *pamesh = NULL;
 double min_dist = ay_prefs.pick_epsilon, dist = 0.0;
 double *pecoords = NULL, *control = NULL;
 int i = 0, j = 0, a = 0, found = AY_FALSE;

  if(!o || !p)
    return AY_ENULL;

  pamesh = (ay_pamesh_object *)(o->refine);

  if(min_dist == 0.0)
    min_dist = DBL_MAX;

  if(ay_point_edit_coords)
    free(ay_point_edit_coords);

  ay_point_edit_coords = NULL;

  /* select all points? */
  if((p[0] == DBL_MIN) && (p[1] == DBL_MIN) && (p[2] == DBL_MIN))
    { /* yes */
      if(!(ay_point_edit_coords = calloc(pamesh->width * pamesh->height,
					 sizeof(double*))))
	return AY_EOMEM;

      for(i = 0; i < (pamesh->width*pamesh->height); i++)
	{
	  ay_point_edit_coords[i] = &(pamesh->controlv[a]);
	  a += 4;
	}

      ay_point_edit_coords_homogenous = AY_TRUE;
      ay_point_edit_coords_number = pamesh->width * pamesh->height;
    }
  else
    { /* no */
      control = pamesh->controlv;
      for(i = 0; i < (pamesh->width * pamesh->height); i++)
	{
	  dist = AY_VLEN((p[0] - control[j]),
			     (p[1] - control[j+1]),
			     (p[2] - control[j+2]));

	  if(dist < min_dist)
	    {
	      pecoords = &(control[j]);
	      min_dist = dist;
	    }

	  j += 4;
	}

      if(!pecoords)
	return AY_OK; /* XXXX should this return a 'AY_EPICK' ? */

      ay_point_edit_coords_homogenous = AY_TRUE;

      if(!found)
	{

	  if(!(ay_point_edit_coords = calloc(1, sizeof(double*))))
	    return AY_EOMEM;

	  ay_point_edit_coords[0] = pecoords;
	  ay_point_edit_coords_number = 1;
	}
    }

 return AY_OK;
} /* ay_pamesh_getpntcb */


/* Tcl -> C! */
int
ay_pamesh_setpropcb(Tcl_Interp *interp, int argc, char *argv[], ay_object *o)
{
 int ay_status = AY_OK;
 char *n1 = "PatchMeshAttrData";
 char fname[] = "pamesh_setpropcb";
 Tcl_Obj *to = NULL, *toa = NULL, *ton = NULL;
 ay_pamesh_object *pamesh = NULL;
 int new_uclose, new_width, new_btype_u;
 int new_vclose, new_height, new_btype_v;
 int new_type, j;
 double dtemp;
 char *man[] = {"_0","_1","_2","_3","_4","_5","_6","_7","_8","_9","_10","_11","_12","_13","_14","_15"};

  if(!o)
    return AY_ENULL;

  pamesh = (ay_pamesh_object *)o->refine;
  
  toa = Tcl_NewStringObj(n1,-1);
  ton = Tcl_NewStringObj(n1,-1);

  Tcl_SetStringObj(ton,"Width",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp,to, &new_width);

  Tcl_SetStringObj(ton,"Height",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp,to, &new_height);

  Tcl_SetStringObj(ton,"Close_U",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp,to, &new_uclose);

  Tcl_SetStringObj(ton,"Close_V",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp,to, &new_vclose);

  Tcl_SetStringObj(ton,"Type",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp,to, &new_type);

  Tcl_SetStringObj(ton,"BType_U",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp,to, &new_btype_u);

  Tcl_SetStringObj(ton,"BType_V",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp,to, &new_btype_v);

  pamesh->btype_u = new_btype_u;
  pamesh->btype_v = new_btype_v;

  if(pamesh->btype_u == AY_BTCUSTOM)
    {
      if(!pamesh->ubasis)
	{
	  if(!(pamesh->ubasis = calloc(16, sizeof(double))))
	    {
	      return AY_OK;
	    } /* if */
	} /* if */
      for(j = 0; j < 16; j++)
	{
	  Tcl_SetStringObj(ton,"Basis_U",-1);
	  Tcl_AppendStringsToObj(ton,man[j],NULL);
	  to  = Tcl_ObjGetVar2(interp, toa, ton, TCL_LEAVE_ERR_MSG |
			       TCL_GLOBAL_ONLY);
	  Tcl_GetDoubleFromObj(interp, to, &dtemp);
	  pamesh->ubasis[j] = (float)dtemp;
	} /* for */

      Tcl_SetStringObj(ton,"Step_U",-1);
      to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
      Tcl_GetIntFromObj(interp,to, &(pamesh->ustep));

    }
  else
    {
      if(pamesh->ubasis)
	free(pamesh->ubasis);
      pamesh->ubasis = NULL;
    } /* if */

  if(pamesh->btype_v == AY_BTCUSTOM)
    {
      if(!pamesh->vbasis)
	{
	  if(!(pamesh->vbasis = calloc(16, sizeof(double))))
	    {
	      return AY_OK;
	    } /* if */
	} /* if */
      for(j = 0; j < 16; j++)
	{
	  Tcl_SetStringObj(ton,"Basis_V",-1);
	  Tcl_AppendStringsToObj(ton,man[j],NULL);
	  to  = Tcl_ObjGetVar2(interp, toa, ton, TCL_LEAVE_ERR_MSG |
			       TCL_GLOBAL_ONLY);
	  Tcl_GetDoubleFromObj(interp, to, &dtemp);
	  pamesh->vbasis[j] = (float)dtemp;
	} /* for */

      Tcl_SetStringObj(ton,"Step_V",-1);
      to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
      Tcl_GetIntFromObj(interp,to, &(pamesh->vstep));

    }
  else
    {
      if(pamesh->vbasis)
	free(pamesh->vbasis);
      pamesh->vbasis = NULL;
    } /* if */

  Tcl_SetStringObj(ton,"Tolerance",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetDoubleFromObj(interp,to, &(pamesh->glu_sampling_tolerance));

  Tcl_SetStringObj(ton,"DisplayMode",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp,to, &(pamesh->glu_display_mode));


  Tcl_IncrRefCount(toa);Tcl_DecrRefCount(toa);
  Tcl_IncrRefCount(ton);Tcl_DecrRefCount(ton);


  /* apply changed values to patch */
#if 0
  /* resize patch */
  if(new_width != pamesh->width && (new_width > 1))
    {
      if(o->selp)
	{
	  ay_selp_clear(o);
	}

      ay_status = ay_npt_resizew(pamesh, new_width);

      if(ay_status)
       ay_error(AY_ERROR,fname,"Could not resize patch!");

      updateKnots = 1;
    }
  if(new_height != pamesh->height && (new_height > 1))
    {
      if(o->selp)
	{
	  ay_selp_clear(o);
	}

      ay_status = ay_npt_resizeh(pamesh, new_height);

      if(ay_status)
       ay_error(AY_ERROR,fname,"Could not resize patch!");

      updateKnots = 1;
    }
#endif
  ay_status = ay_notify_parent();

 return AY_OK;
} /* ay_pamesh_setpropcb */


/* C -> Tcl! */
int
ay_pamesh_getpropcb(Tcl_Interp *interp, int argc, char *argv[], ay_object *o)
{
 char *n1="PatchMeshAttrData";
 Tcl_Obj *to = NULL, *toa = NULL, *ton = NULL;
 ay_pamesh_object *pamesh = NULL;
 int j;
 char *man[] = {"_0","_1","_2","_3","_4","_5","_6","_7","_8","_9","_10","_11","_12","_13","_14","_15"};

  if(!o)
    return AY_ENULL;

  pamesh = (ay_pamesh_object *)(o->refine);

  toa = Tcl_NewStringObj(n1,-1);

  ton = Tcl_NewStringObj(n1,-1);


  Tcl_SetStringObj(ton,"Width",-1);
  to = Tcl_NewIntObj(pamesh->width);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton,"Height",-1);
  to = Tcl_NewIntObj(pamesh->height);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton,"Close_U",-1);
  to = Tcl_NewIntObj(pamesh->close_u);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton,"Close_V",-1);
  to = Tcl_NewIntObj(pamesh->close_v);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton,"Type",-1);
  to = Tcl_NewIntObj(pamesh->type);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton,"BType_U",-1);
  to = Tcl_NewIntObj(pamesh->btype_u);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton,"BType_V",-1);
  to = Tcl_NewIntObj(pamesh->btype_v);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);


  if(pamesh->btype_u == AY_BTCUSTOM)
    {
      if(pamesh->ubasis)
	{
	  for(j = 0; j < 16; j++)
	    {
	      Tcl_SetStringObj(ton, "Basis_U", -1);
	      Tcl_AppendStringsToObj(ton, man[j], NULL);
	      to = Tcl_NewDoubleObj(pamesh->ubasis[j]);
	      Tcl_ObjSetVar2(interp, toa, ton, to, TCL_LEAVE_ERR_MSG |
			     TCL_GLOBAL_ONLY);
	    } /* for */
	} /* if */
      Tcl_SetStringObj(ton,"Step_U",-1);
      to = Tcl_NewIntObj(pamesh->ustep);
      Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG |
		     TCL_GLOBAL_ONLY);
    } /* if */

  if(pamesh->btype_v == AY_BTCUSTOM)
    {
      if(pamesh->vbasis)
	{
	  for(j = 0; j < 16; j++)
	    {
	      Tcl_SetStringObj(ton, "Basis_V", -1);
	      Tcl_AppendStringsToObj(ton, man[j], NULL);
	      to = Tcl_NewDoubleObj(pamesh->vbasis[j]);
	      Tcl_ObjSetVar2(interp, toa, ton, to, TCL_LEAVE_ERR_MSG |
			     TCL_GLOBAL_ONLY);
	    } /* for */
	} /* if */
      Tcl_SetStringObj(ton,"Step_V",-1);
      to = Tcl_NewIntObj(pamesh->vstep);
      Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG |
		     TCL_GLOBAL_ONLY);
    } /* if */

  Tcl_SetStringObj(ton,"Tolerance",-1);
  to = Tcl_NewDoubleObj(pamesh->glu_sampling_tolerance);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton,"DisplayMode",-1);
  to = Tcl_NewIntObj(pamesh->glu_display_mode);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);

  Tcl_IncrRefCount(toa);Tcl_DecrRefCount(toa);
  Tcl_IncrRefCount(ton);Tcl_DecrRefCount(ton);

 return AY_OK;
} /* ay_pamesh_getpropcb */


int
ay_pamesh_readcb(FILE *fileptr, ay_object *o)
{
 int ay_status = AY_OK;
 ay_pamesh_object *pamesh = NULL;
 int i, a;

 if(!o)
   return AY_ENULL;

  if(!(pamesh = calloc(1, sizeof(ay_pamesh_object))))
    { return AY_EOMEM; }


  fscanf(fileptr,"%d\n",&pamesh->width);
  fscanf(fileptr,"%d\n",&pamesh->height);
  fscanf(fileptr,"%d\n",&pamesh->close_u);
  fscanf(fileptr,"%d\n",&pamesh->close_v);
  fscanf(fileptr,"%d\n",&pamesh->type);
  fscanf(fileptr,"%d\n",&pamesh->btype_u);
  fscanf(fileptr,"%d\n",&pamesh->btype_v);
  fscanf(fileptr,"%d\n",&pamesh->ustep);
  fscanf(fileptr,"%d\n",&pamesh->vstep);

  if(pamesh->btype_u == AY_BTCUSTOM)
    {
      if(!(pamesh->ubasis = calloc(16, sizeof(double))))
	{
	  free(pamesh);
	  return AY_EOMEM;
	}
      for(i=0; i < 16; i++)
	{
	  fscanf(fileptr, "%lg\n", &(pamesh->vbasis[i]));
	}
    }

  if(pamesh->btype_v == AY_BTCUSTOM)
    {
      if(!(pamesh->vbasis = calloc(16, sizeof(double))))
	{
	  if(pamesh->ubasis)
	    free(pamesh->ubasis);
	  free(pamesh);
	  return AY_EOMEM;
	}
      for(i=0; i < 16; i++)
	{
	  fscanf(fileptr, "%lg\n", &(pamesh->vbasis[i]));
	}
    }

  if(!(pamesh->controlv = calloc(pamesh->width*pamesh->height*4,
				 sizeof(double))))
    {
      if(pamesh->ubasis)
	free(pamesh->ubasis);
      if(pamesh->vbasis)
	free(pamesh->vbasis);
      free(pamesh);
      return AY_EOMEM;
    }

  a = 0;
  for(i=0; i < pamesh->width*pamesh->height; i++)
    {
      fscanf(fileptr,"%lg %lg %lg %lg\n",&(pamesh->controlv[a]),
	     &(pamesh->controlv[a+1]),
	     &(pamesh->controlv[a+2]),
	     &(pamesh->controlv[a+3]));
      a+=4;
    }

  fscanf(fileptr,"%lg\n",&(pamesh->glu_sampling_tolerance));
  fscanf(fileptr,"%d\n",&(pamesh->glu_display_mode));

  o->refine = pamesh;

 return AY_OK;
} /* ay_pamesh_readcb */


int
ay_pamesh_writecb(FILE *fileptr, ay_object *o)
{
 ay_pamesh_object *pamesh = NULL;
 int i, a;

  if(!o)
    return AY_ENULL;

  pamesh = (ay_pamesh_object *)(o->refine);

  fprintf(fileptr, "%d\n", pamesh->width);
  fprintf(fileptr, "%d\n", pamesh->height);
  fprintf(fileptr, "%d\n", pamesh->close_u);
  fprintf(fileptr, "%d\n", pamesh->close_v);
  fprintf(fileptr, "%d\n", pamesh->type);
  fprintf(fileptr, "%d\n", pamesh->btype_u);
  fprintf(fileptr, "%d\n", pamesh->btype_v);
  fprintf(fileptr, "%d\n", pamesh->ustep);
  fprintf(fileptr, "%d\n", pamesh->vstep);

  if(pamesh->btype_u == AY_BTCUSTOM)
    {
      for(i=0; i < 16; i++)
	{
	  fprintf(fileptr, "%g\n", pamesh->ubasis[i]);
	}
    }
  if(pamesh->btype_v == AY_BTCUSTOM)
    {
      for(i=0; i < 16; i++)
	{
	  fprintf(fileptr, "%g\n", pamesh->vbasis[i]);
	}
    }

  a = 0;
  for(i=0;i<pamesh->width*pamesh->height;i++)
    {
      fprintf(fileptr,"%g %g %g %g\n", pamesh->controlv[a],
	      pamesh->controlv[a+1],
	      pamesh->controlv[a+2],
	      pamesh->controlv[a+3]);
      a+=4;
    }

  fprintf(fileptr, "%g\n", pamesh->glu_sampling_tolerance);
  fprintf(fileptr, "%d\n", pamesh->glu_display_mode);

 return AY_OK;
} /* ay_pamesh_writecb */


int
ay_pamesh_wribcb(char *file, ay_object *o)
{
 int ay_status = AY_OK;
 ay_pamesh_object *patch = NULL;
 RtInt nu, nv;
 RtToken uwrap = RI_NONPERIODIC, vwrap = RI_NONPERIODIC, type;
 RtInt ustep, vstep;
 RtBasis *ubasisptr, *vbasisptr;
 RtBasis ubasis, vbasis;
 RtFloat *controls = NULL;
 int i = 0, j = 0, a = 0, b = 0;

  if(!o)
    return AY_OK;

  patch = (ay_pamesh_object*)(o->refine);

  if(patch->type == AY_PTBICUBIC)
    {
      type = RI_BICUBIC;
    }
  else
    {
      type = RI_BILINEAR;
    }

  nu = (RtInt)patch->width;
  if(patch->close_u)
    uwrap = RI_PERIODIC;
  nv = (RtInt)patch->height;
  if(patch->close_v)
    vwrap = RI_PERIODIC;

  if((controls = calloc(nu*nv*4, sizeof(RtFloat))) == NULL)
    return AY_EOMEM;

  a=0;
  /* RenderMan expects u-major order! */
  for(i=0;i<nv;i++)
    {
      b = i*4;
      for(j=0;j<nu;j++)
	{
	  controls[a] = (RtFloat)patch->controlv[b]; 
	  a++;
	  controls[a] = (RtFloat)patch->controlv[b+1]; 
	  a++;
	  controls[a] = (RtFloat)patch->controlv[b+2]; 
	  a++;
	  controls[a] = (RtFloat)patch->controlv[b+3]; 
	  a++;
	  b+=(nv*4);
	}
    }

  switch(patch->btype_u)
    {
    case AY_BTBEZIER:
      ubasisptr = &(RiBezierBasis);
      ustep = RI_BEZIERSTEP;
      break;
    case AY_BTBSPLINE:
      ubasisptr = &(RiBSplineBasis);
      ustep = RI_BSPLINESTEP;
      break;
    case AY_BTCATMULLROM:
      ubasisptr = &(RiCatmullRomBasis);
      ustep = RI_CATMULLROMSTEP;
      break;
    case AY_BTHERMITE:
      ubasisptr = &(RiHermiteBasis);
      ustep = RI_HERMITESTEP;
      break;
    case AY_BTCUSTOM:
      ubasisptr = &(ubasis);
      ustep = (RtInt)patch->ustep;
      for(i = 0; i < 4; i++)
	{
	  for(j = 0; j < 4; j++)
	    {
	      ubasis[i][j] = (RtFloat)patch->ubasis[i*4+j];
	    } /* for */
	} /* for */
      break;
    default:
      ubasisptr = &(RiBezierBasis);
      ustep = RI_BEZIERSTEP;
      break;
    } /* switch */

  switch(patch->btype_v)
    {
    case AY_BTBEZIER:
      vbasisptr = &(RiBezierBasis);
      vstep = RI_BEZIERSTEP;
      break;
    case AY_BTBSPLINE:
      vbasisptr = &(RiBSplineBasis);
      vstep = RI_BSPLINESTEP;
      break;
    case AY_BTCATMULLROM:
      vbasisptr = &(RiCatmullRomBasis);
      vstep = RI_CATMULLROMSTEP;
      break;
    case AY_BTHERMITE:
      vbasisptr = &(RiHermiteBasis);
      vstep = RI_HERMITESTEP;
      break;
    case AY_BTCUSTOM:
      vbasisptr = &(vbasis);
      vstep = (RtInt)patch->vstep;
      for(i = 0; i < 4; i++)
	{
	  for(j = 0; j < 4; j++)
	    {
	      vbasis[i][j] = (RtFloat)patch->vbasis[i*4+j];
	    } /* for */
	} /* for */
      break;
    default:
      vbasisptr = &(RiBezierBasis);
      vstep = RI_BEZIERSTEP;
      break;
    } /* switch */

  if((ubasisptr != &RiBezierBasis) ||
     (vbasisptr != &RiBezierBasis))
    {
      RiBasis(*ubasisptr, ustep, *vbasisptr, vstep);
    }

  RiPatchMesh(type, nu, uwrap, nv, vwrap, "Pw", controls, NULL);

  free(controls);

 return ay_status;
} /* ay_pamesh_wribcb */


int
ay_pamesh_bbccb(ay_object *o, double *bbox, int *flags)
{
 double xmin, xmax, ymin, ymax, zmin, zmax;
 double *controlv = NULL;
 int i, a;
 ay_pamesh_object *pamesh = NULL;

  if(!o || !bbox)
    return AY_ENULL;

  pamesh = (ay_pamesh_object *)o->refine; 

  controlv = pamesh->controlv;

  xmin = controlv[0];
  xmax = xmin;
  ymin = controlv[1];
  ymax = ymin;
  zmin = controlv[2];
  zmax = zmin;

  a = 0;
  for(i = 0; i < pamesh->width*pamesh->height; i++)
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

      a += 4;
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
} /* ay_pamesh_bbccb */


int
ay_pamesh_init(Tcl_Interp *interp)
{
 int ay_status = AY_OK;

  ay_status = ay_otype_registercore(ay_pamesh_name,
				    ay_pamesh_createcb,
				    ay_pamesh_deletecb,
				    ay_pamesh_copycb,
				    ay_pamesh_drawcb,
				    ay_pamesh_drawhcb,
				    ay_pamesh_shadecb,
				    ay_pamesh_setpropcb,
				    ay_pamesh_getpropcb,
				    ay_pamesh_getpntcb,
				    ay_pamesh_readcb,
				    ay_pamesh_writecb,
				    ay_pamesh_wribcb,
				    ay_pamesh_bbccb,
				    AY_IDPAMESH);

 return ay_status;
} /* ay_pamesh_init */

