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

/* pamesh.c -  PatchMesh object */

static char *ay_pamesh_name = "PatchMesh";

int
ay_pamesh_createcb(int argc, char *argv[], ay_object *o)
{
 int ay_status = AY_OK;
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
	  /* bicubic case is default! */
	  if(width <= 3) width = 4;
	  i+=2;
	}
      else
      if(!strcmp(argv[i],"-height"))
	{
	  Tcl_GetInt(ay_interp, argv[i+1], &height);
	  /* bicubic case is default! */
	  if(height <= 3) height = 4;
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

  /* immediately create NURBS patch representation */
  ay_status = ay_pmt_tonpatch(p, &(p->npatch));

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

  /* free NURBS patch(es) */
  if(pamesh->npatch)
    ay_object_deletemulti(pamesh->npatch);

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
  /* copy NURBS patch(es) */
  if(pameshsrc->npatch)
    {
      ay_object_copymulti(pameshsrc->npatch, &(pamesh->npatch));
    }

  *dst = (void *)pamesh;

 return AY_OK;
} /* ay_pamesh_copycb */


int
ay_pamesh_drawcpcb(struct Togl *togl, ay_object *o)
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
  if(pamesh->close_v)
    {
      for(i = 0; i < width; i++)
	{
	  glBegin(GL_LINE_LOOP);
	   for(j = 0; j < height; j++)
	     {
	       glVertex3dv((GLdouble *)&ver[a]);
	       a += 4;
	     }
	  glEnd();
	}
    }
  else
    {
      for(i = 0; i < width; i++)
	{
	  glBegin(GL_LINE_STRIP);
	   for(j = 0; j < height; j++)
	     {
	       glVertex3dv((GLdouble *)&ver[a]);
	       a += 4;
	     } /* for */
	  glEnd();
	} /* for */
    } /* if */

  a = 0;
  if(pamesh->close_u)
    {
      for(j = 0; j < height; j++)
	{
	  a = j * 4;
	  glBegin(GL_LINE_LOOP);
	   for(i = 0; i < width; i++)
	     {
	       glVertex3dv((GLdouble *)&ver[a]);

	       a += (4 * height);
	     }
	  glEnd();
	}
    }
  else
    {
      for(j = 0; j < height; j++)
	{
	  a = j * 4;
	  glBegin(GL_LINE_STRIP);
	   for(i = 0; i < width; i++)
	     {
	       glVertex3dv((GLdouble *)&ver[a]);

	       a += (4 * height);
	     } /* for */
	  glEnd();
	} /* for */
    } /* if */

 return AY_OK;
} /* ay_pamesh_drawcpcb */


int
ay_pamesh_drawcb(struct Togl *togl, ay_object *o)
{
 int display_mode = ay_prefs.glu_display_mode;
 ay_pamesh_object *pamesh = NULL;
 ay_object *p = NULL;

  if(!o)
    return AY_ENULL;

  pamesh = (ay_pamesh_object *)o->refine;

  if(pamesh->glu_display_mode != 0)
    {
      display_mode = pamesh->glu_display_mode-1;
    }

  /* draw just the control polygon? */
  if(display_mode == 0)
    {
      /* Yes */
      ay_pamesh_drawcpcb(togl, o);
    }
  else
    {
      /* No, draw the NURBS patch(es), if present */
      p = pamesh->npatch;
      while(p)
	{
	  ay_draw_object(togl, p, AY_FALSE);
	  p = p->next;
	} /* while */
    } /* if */

 return AY_OK;
} /* ay_pamesh_drawcb */


int
ay_pamesh_shadecb(struct Togl *togl, ay_object *o)
{
 ay_pamesh_object *pamesh = NULL;
 ay_object *p = NULL;

  pamesh = (ay_pamesh_object *)o->refine;

  p = pamesh->npatch;
  while(p)
    {
      ay_shade_object(togl, p, AY_FALSE);
      p = p->next;
    } /* while */

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
ay_pamesh_getpntcb(int mode, ay_object *o, double *p)
{
 ay_pamesh_object *pamesh = NULL;
 double min_dist = ay_prefs.pick_epsilon, dist = 0.0;
 double *pecoord = NULL, **pecoords = NULL, *control = NULL, *c;
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
  if(mode == 0)
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

      /* selection based on a single point? */
      if(mode == 1)
	{ /* yes */

	  control = pamesh->controlv;

	  for(i = 0; i < (pamesh->width * pamesh->height); i++)
	    {
	      dist = AY_VLEN((p[0] - control[j]),
			     (p[1] - control[j+1]),
			     (p[2] - control[j+2]));

	      if(dist < min_dist)
		{
		  pecoord = &(control[j]);
		  min_dist = dist;
		}

	      j += 4;
	    }

	  if(!pecoord)
	    return AY_OK; /* XXXX should this return a 'AY_EPICK' ? */

	  ay_point_edit_coords_homogenous = AY_TRUE;

	  if(!found)
	    {

	      if(!(ay_point_edit_coords = calloc(1, sizeof(double*))))
		return AY_EOMEM;

	      ay_point_edit_coords[0] = pecoord;
	      ay_point_edit_coords_number = 1;
	    }
	}
      else
	{ /* no */

	  /* selection based on planes */
	  control = pamesh->controlv;
	  j = 0;
	  a = 0;
	  for(i = 0; i < pamesh->width * pamesh->height; i++)
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

	      j += 4;
	    } /* for */

	  if(!pecoords)
	    return AY_OK; /* XXXX should this return a 'AY_EPICK' ? */

	  ay_point_edit_coords_homogenous = AY_TRUE;
	  ay_point_edit_coords = pecoords;
	  ay_point_edit_coords_number = a;

	} /* if */
    } /* if */

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
 int new_close_u, new_width, new_btype_u, new_step_u;
 int new_close_v, new_height, new_btype_v, new_step_v;
 int new_type, j, update = AY_FALSE;
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
  Tcl_GetIntFromObj(interp,to, &new_close_u);

  Tcl_SetStringObj(ton,"Close_V",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp,to, &new_close_v);

  Tcl_SetStringObj(ton,"Type",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp,to, &new_type);

  Tcl_SetStringObj(ton,"BType_U",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp,to, &new_btype_u);

  Tcl_SetStringObj(ton,"BType_V",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp,to, &new_btype_v);

  if((pamesh->width != new_width)||(pamesh->height != new_height)||
     (pamesh->btype_u != new_btype_u)||(pamesh->btype_v != new_btype_v)||
     (pamesh->close_u != new_close_u)||(pamesh->close_v != new_close_v)||
     (pamesh->type != new_type))
    {
      update = AY_TRUE;
    }

  pamesh->type = new_type;
  pamesh->btype_u = new_btype_u;
  pamesh->btype_v = new_btype_v;
  
  pamesh->close_u = new_close_u;
  pamesh->close_v = new_close_v;

  if(pamesh->btype_u == AY_BTCUSTOM)
    {
      if(!pamesh->ubasis)
	{
	  if(!(pamesh->ubasis = calloc(16, sizeof(double))))
	    {
	      Tcl_IncrRefCount(toa);Tcl_DecrRefCount(toa);
	      Tcl_IncrRefCount(ton);Tcl_DecrRefCount(ton);
	      ay_error(AY_EOMEM, fname, NULL);
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
      Tcl_GetIntFromObj(interp,to, &(new_step_u));
      if(new_step_u <= 0)
	pamesh->ustep = 1;


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
	      Tcl_IncrRefCount(toa);Tcl_DecrRefCount(toa);
	      Tcl_IncrRefCount(ton);Tcl_DecrRefCount(ton);
	      ay_error(AY_EOMEM, fname, NULL);
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
      Tcl_GetIntFromObj(interp,to, &(new_step_v));
      if(new_step_v <= 0)
	pamesh->vstep = 1;
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

  /* resize patch */
  if(new_width != pamesh->width && (new_width > 1))
    {
      if(o->selp)
	{
	  ay_selp_clear(o);
	}

      ay_status = ay_npt_resizearrayw(&(pamesh->controlv), 4,
				      pamesh->width, pamesh->height,
				      new_width);

      if(ay_status)
	ay_error(AY_ERROR,fname,"Could not resize patch!");
      else
	pamesh->width = new_width;
    } /* if */

  if(new_height != pamesh->height && (new_height > 1))
    {
      if(o->selp)
	{
	  ay_selp_clear(o);
	}

      ay_status = ay_npt_resizearrayh(&(pamesh->controlv), 4,
				      pamesh->width, pamesh->height,
				      new_height);

      if(ay_status)
	ay_error(AY_ERROR,fname,"Could not resize patch!");
      else
	pamesh->height = new_height;
    } /* if */

  if(update)
    ay_notify_force(o);

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
ay_pamesh_notifycb(ay_object *o)
{
 int ay_status = AY_OK;
 ay_object *p = NULL;
 ay_pamesh_object *pamesh = NULL;
 ay_nurbpatch_object *np = NULL;
 int detail;

  if(!o)
    return AY_ENULL;

  pamesh = (ay_pamesh_object *)o->refine; 

  if(pamesh->npatch)
    {
      ay_object_deletemulti(pamesh->npatch);
      pamesh->npatch = NULL;
    }

  if(ay_pmt_valid(pamesh, &detail))
    {
      return AY_OK;
    }

  ay_status = ay_pmt_tonpatch(pamesh, &(pamesh->npatch));

  p = pamesh->npatch;
  while(p)
    {
      if(p->type == AY_IDNPATCH)
	{
	  np = (ay_nurbpatch_object *)p->refine;
	  np->glu_display_mode = pamesh->glu_display_mode;
	  np->glu_sampling_tolerance = pamesh->glu_sampling_tolerance;
	}

      p = p->next;
    } /* while */

 return AY_OK;
} /* ay_pamesh_notifycb */


int
ay_pamesh_providecb(ay_object *o, unsigned int type, ay_object **result)
{
 int ay_status = AY_OK;
 char fname[] = "pamesh_providecb";
 ay_pamesh_object *pm = NULL;
 ay_object *new = NULL, **t = NULL, *p = NULL;

  if(!o || !result)
    return AY_ENULL;

  pm = (ay_pamesh_object *) o->refine;

  if(type == AY_IDNPATCH)
    {
      t = &(new);
      p = pm->npatch;
      while(p)
	{
	  ay_status = ay_object_copy(p, t);
	  if(ay_status)
	    {
	      ay_error(ay_status, fname, NULL);
	      return AY_ERROR;
	    }
	  ay_trafo_copy(o, *t);
	  t = &((*t)->next);
	  p = p->next;
	} /* while */

      *result = new;
    } /* if */

 return ay_status;
} /* ay_pamesh_providecb */


int
ay_pamesh_convertcb(ay_object *o, int in_place)
{
 int ay_status = AY_OK;
 ay_pamesh_object *pamesh = NULL;
 ay_object *p = NULL, *new = NULL;

  if(!o)
    return AY_ENULL;

  pamesh = (ay_pamesh_object *) o->refine;
  p = pamesh->npatch;
  while(p)
    {
      ay_status = ay_object_copy(p, &new);
      ay_trafo_copy(o, new);
      ay_status = ay_object_link(new);

      p = p->next;
    } /* while */

 return ay_status;
} /* ay_pamesh_convertcb */


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

  ay_status = ay_notify_register(ay_pamesh_notifycb, AY_IDPAMESH);

  ay_status = ay_convert_register(ay_pamesh_convertcb, AY_IDPAMESH);

  ay_status = ay_provide_register(ay_pamesh_providecb, AY_IDPAMESH);

 return ay_status;
} /* ay_pamesh_init */

