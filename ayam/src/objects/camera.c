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

/* camera.c - camera object */

static char *ay_camera_name = "Camera";


/* ay_camera_createcb:
 */
int
ay_camera_createcb(int argc, char *argv[], ay_object *o)
{
 char fname[] = "crtcamera";
 ay_camera_object *camera = NULL;

  if(!o)
    return AY_ENULL;

  if(!(camera = calloc(1, sizeof(ay_camera_object))))
    {
      ay_error(AY_EOMEM, fname, NULL);
      return AY_ERROR;
    }


  o->refine = camera;

  camera->from[2] = 10.0;
  camera->up[1] = 1.0;
  camera->zoom = 1.0;

 return AY_OK;
} /* ay_camera_createcb */


/* ay_camera_deletecb:
 */
int
ay_camera_deletecb(void *c)
{
 ay_camera_object *camera = NULL;

  camera = (ay_camera_object *)(c);
  if(!camera)
    return AY_ENULL;    

  free(camera);

 return AY_OK;
} /* ay_camera_deletecb */


int
ay_camera_copycb(void *src, void **dst)
{
 ay_camera_object *camera = NULL;

  if(!(camera = calloc(1, sizeof(ay_camera_object))))
    return AY_EOMEM; 

  memcpy(camera, src, sizeof(ay_camera_object)); 

  *dst = camera;

 return AY_OK;
} /* ay_camera_copycb */


int
ay_camera_drawcb(struct Togl *togl, ay_object *o)
{
 ay_camera_object *camera = NULL;

  if(!o)
    return AY_ENULL;

  camera = (ay_camera_object *)o->refine;

  if(!camera)
    return AY_ENULL;

  /* ignore transformation */
  glLoadIdentity();

  glBegin(GL_LINES);
  glVertex3d((GLdouble)camera->from[0],(GLdouble)camera->from[1],
	     (GLdouble)camera->from[2]);
  glVertex3d((GLdouble)camera->to[0],(GLdouble)camera->to[1],
	     (GLdouble)camera->to[2]);
  glEnd();

  ay_draw_arrow(togl, camera->from, camera->to);

 return AY_OK;
} /* ay_camera_drawcb */


int
ay_camera_drawhcb(struct Togl *togl, ay_object *o)
{
 ay_camera_object *camera = NULL;

  if(!o)
    return AY_ENULL;

  camera = (ay_camera_object *)o->refine;

  if(!camera)
    return AY_ENULL;

  /* ignore transformation */
  glLoadIdentity();

      /* draw */
  glBegin(GL_POINTS);
  glVertex3d((GLdouble)camera->from[0],(GLdouble)camera->from[1],
	     (GLdouble)camera->from[2]);
  glVertex3d((GLdouble)camera->to[0],(GLdouble)camera->to[1],
	     (GLdouble)camera->to[2]);
  glEnd();

 return AY_OK;
} /* ay_camera_drawhcb */


int
ay_camera_shadecb(struct Togl *togl, ay_object *o)
{
 ay_camera_object *camera = NULL;

  if(!o)
    return AY_ENULL;

  camera = (ay_camera_object *)o->refine;

  if(!camera)
    return AY_ENULL;

 return AY_OK;
} /* ay_camera_shadecb */


/* ay_camera_setpropcb:
 *  Tcl -> C!
 *  configure camera from Tcl
 */
int
ay_camera_setpropcb(Tcl_Interp *interp, int argc, char *argv[], ay_object *o)
{
 ay_camera_object *camera = NULL;
 char *n1 = "CameraData";
 Tcl_Obj *to = NULL, *toa = NULL, *ton = NULL;
 char fname[] = "camera_setpropcb";

  if(!o)
    return AY_ENULL;

  camera = (ay_camera_object *)o->refine;

  toa = Tcl_NewStringObj(n1, -1);
  ton = Tcl_NewStringObj("From_X",-1);
  to = Tcl_ObjGetVar2(interp, toa, ton, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetDoubleFromObj(interp, to, &camera->from[0]);
  Tcl_SetStringObj(ton, "From_Y", -1);
  to = Tcl_ObjGetVar2(interp, toa, ton, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetDoubleFromObj(interp, to, &camera->from[1]);
  Tcl_SetStringObj(ton, "From_Z", -1);
  to = Tcl_ObjGetVar2(interp, toa, ton, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetDoubleFromObj(interp, to, &camera->from[2]);

  Tcl_SetStringObj(ton, "To_X", -1);
  to = Tcl_ObjGetVar2(interp, toa, ton, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetDoubleFromObj(interp, to, &camera->to[0]);
  Tcl_SetStringObj(ton, "To_Y", -1);
  to = Tcl_ObjGetVar2(interp, toa, ton, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetDoubleFromObj(interp, to, &camera->to[1]);
  Tcl_SetStringObj(ton, "To_Z", -1);
  to = Tcl_ObjGetVar2(interp, toa, ton, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetDoubleFromObj(interp, to, &camera->to[2]);

  Tcl_SetStringObj(ton, "Up_X", -1);
  to = Tcl_ObjGetVar2(interp, toa, ton, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetDoubleFromObj(interp, to, &camera->up[0]);
  Tcl_SetStringObj(ton, "Up_Y", -1);
  to = Tcl_ObjGetVar2(interp, toa, ton, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetDoubleFromObj(interp, to, &camera->up[1]);
  Tcl_SetStringObj(ton, "Up_Z", -1);
  to = Tcl_ObjGetVar2(interp, toa, ton, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetDoubleFromObj(interp, to, &camera->up[2]);

  Tcl_SetStringObj(ton, "Near", -1);
  to = Tcl_ObjGetVar2(interp, toa, ton, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetDoubleFromObj(interp, to, &camera->nearp);
  Tcl_SetStringObj(ton, "Far", -1);
  to = Tcl_ObjGetVar2(interp, toa, ton, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetDoubleFromObj(interp, to, &camera->farp);

  Tcl_SetStringObj(ton, "Zoom", -1);
  to = Tcl_ObjGetVar2(interp, toa, ton, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetDoubleFromObj(interp, to, &camera->zoom);

  Tcl_SetStringObj(ton, "Roll", -1);
  to = Tcl_ObjGetVar2(interp, toa, ton, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetDoubleFromObj(interp, to, &camera->roll);

  /* check clipping planes */
  if(camera->nearp < 0.0)
    {
      ay_error(AY_EWARN, fname,
	       "Near has to be positive for perspective views!");
    }

  if((camera->nearp != 0.0 || camera->farp != 0.0) &&
     (camera->nearp >= camera->farp))
    {
      ay_error(AY_EWARN, fname, "Near should be smaller than far!");
    }

  Tcl_IncrRefCount(toa); Tcl_DecrRefCount(toa);
  Tcl_IncrRefCount(ton); Tcl_DecrRefCount(ton);

  return TCL_OK;
} /* ay_camera_setpropcb */


/* ay_camera_getpropcb:
 *  C -> Tcl!
 *  copy all information about the camera *togl
 *  to the global camera() array (Tcl)
 */
int
ay_camera_getpropcb(Tcl_Interp *interp, int argc, char *argv[], ay_object *o)
{
 ay_camera_object *camera = NULL;
 char *n1 = "CameraData";
 Tcl_Obj *to = NULL, *toa = NULL, *ton = NULL;

  if(!o)
    return AY_ENULL;

  camera = (ay_camera_object *)o->refine;

  toa = Tcl_NewStringObj(n1, -1);

  ton = Tcl_NewStringObj("From_X", -1);
  to = Tcl_NewDoubleObj(camera->from[0]);
  Tcl_ObjSetVar2(interp, toa, ton, to, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_SetStringObj(ton, "From_Y", -1);
  to = Tcl_NewDoubleObj(camera->from[1]);
  Tcl_ObjSetVar2(interp, toa, ton, to, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_SetStringObj(ton, "From_Z", -1);
  to = Tcl_NewDoubleObj(camera->from[2]);
  Tcl_ObjSetVar2(interp, toa, ton, to, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton, "To_X", -1);
  to = Tcl_NewDoubleObj(camera->to[0]);
  Tcl_ObjSetVar2(interp, toa, ton, to, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_SetStringObj(ton, "To_Y", -1);
  to = Tcl_NewDoubleObj(camera->to[1]);
  Tcl_ObjSetVar2(interp, toa, ton, to, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_SetStringObj(ton, "To_Z", -1);
  to = Tcl_NewDoubleObj(camera->to[2]);
  Tcl_ObjSetVar2(interp, toa, ton, to, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton, "Up_X", -1);
  to = Tcl_NewDoubleObj(camera->up[0]);
  Tcl_ObjSetVar2(interp, toa, ton, to, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_SetStringObj(ton, "Up_Y", -1);
  to = Tcl_NewDoubleObj(camera->up[1]);
  Tcl_ObjSetVar2(interp, toa, ton, to, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_SetStringObj(ton, "Up_Z", -1);
  to = Tcl_NewDoubleObj(camera->up[2]);
  Tcl_ObjSetVar2(interp, toa, ton, to, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton, "Near", -1);
  to = Tcl_NewDoubleObj(camera->nearp);
  Tcl_ObjSetVar2(interp, toa, ton, to, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_SetStringObj(ton, "Far", -1);
  to = Tcl_NewDoubleObj(camera->farp);
  Tcl_ObjSetVar2(interp, toa, ton, to, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton, "Roll", -1);
  to = Tcl_NewDoubleObj(camera->roll);
  Tcl_ObjSetVar2(interp, toa, ton, to, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton, "Zoom", -1);
  to = Tcl_NewDoubleObj(camera->zoom);
  Tcl_ObjSetVar2(interp, toa, ton, to, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_IncrRefCount(toa); Tcl_DecrRefCount(toa);
  Tcl_IncrRefCount(ton); Tcl_DecrRefCount(ton);

 return AY_OK;
} /* ay_camera_getpropcb */


int
ay_camera_getpntcb(int mode, ay_object *o, double *p)
{
 ay_camera_object *camera = NULL;
 double min_dist = ay_prefs.pick_epsilon, dist = 0.0;
 double *pecoord = NULL, **pecoords = NULL, *c;
 int a;

  if(!o || !p)
    return AY_ENULL;

  camera = (ay_camera_object *)(o->refine);

  if(min_dist == 0.0)
    min_dist = DBL_MAX;

  if(ay_point_edit_coords)
    free(ay_point_edit_coords);

  ay_point_edit_coords = NULL;


  /* select all points? */
  if(mode == 0)
    { /* yes */

      if(!(ay_point_edit_coords = calloc(2, sizeof(double*))))
	return AY_EOMEM;

      ay_point_edit_coords[0] = camera->from;
      ay_point_edit_coords[1] = camera->to;
      ay_point_edit_coords_homogenous = AY_FALSE;
      ay_point_edit_coords_number = 2;

    }
  else
    { /* no */

      /* selection based on a single point? */
      if(mode == 1)
	{ /* yes */

	  dist = AY_VLEN((p[0] - camera->from[0]),
			 (p[1] - camera->from[1]),
			 (p[2] - camera->from[2]));

	  if(dist < min_dist)
	    {
	      pecoord = camera->from;
	      min_dist = dist;
	    }

	  dist = AY_VLEN((p[0] - camera->to[0]),
			 (p[1] - camera->to[1]),
			 (p[2] - camera->to[2]));

	  if(dist < min_dist)
	    {
	      pecoord = camera->to;
	      min_dist = dist;
	    
	    }

	  if(!pecoord)
	    return AY_OK; /* XXXX should this return a 'AY_EPICK' ? */

	  if(!(ay_point_edit_coords = calloc(1, sizeof(double*))))
	    return AY_EOMEM;

	  ay_point_edit_coords[0] = pecoord;
	  ay_point_edit_coords_homogenous = AY_FALSE;
	  ay_point_edit_coords_number = 1;

	}
      else
	{ /* no */

	  /* selection based on planes */
	  c = camera->from;
	  a = 0;
	  /* test point c against the four planes in p */
	  if(((p[0]*c[0] + p[1]*c[1] + p[2]*c[2] + p[3]) < 0.0) && 
	     ((p[4]*c[0] + p[5]*c[1] + p[6]*c[2] + p[7]) < 0.0) && 
	     ((p[8]*c[0] + p[9]*c[1] + p[10]*c[2] + p[11]) < 0.0) && 
	     ((p[12]*c[0] + p[13]*c[1] + p[14]*c[2] + p[15]) < 0.0))
	    {

	      if(!(pecoords = realloc(pecoords, (a+1)*sizeof(double *))))
		return AY_EOMEM;
	      pecoords[a] = c;
	      a++;		  
	    } /* if */

	  c = camera->to;
	  /* test point c against the four planes in p */
	  if(((p[0]*c[0] + p[1]*c[1] + p[2]*c[2] + p[3]) < 0.0) && 
	     ((p[4]*c[0] + p[5]*c[1] + p[6]*c[2] + p[7]) < 0.0) && 
	     ((p[8]*c[0] + p[9]*c[1] + p[10]*c[2] + p[11]) < 0.0) && 
	     ((p[12]*c[0] + p[13]*c[1] + p[14]*c[2] + p[15]) < 0.0))
	    {

	      if(!(pecoords = realloc(pecoords, (a+1)*sizeof(double *))))
		return AY_EOMEM;
	      pecoords[a] = c;
	      a++;		  
	    } /* if */

	  if(!pecoords)
	    return AY_OK; /* XXXX should this return a 'AY_EPICK' ? */

	  ay_point_edit_coords_homogenous = AY_FALSE;
	  ay_point_edit_coords = pecoords;
	  ay_point_edit_coords_number = a;

	} /* if */
    } /* if */

 return AY_OK;
} /* ay_camera_getpntcb */


int
ay_camera_readcb(FILE *fileptr, ay_object *o)
{
 ay_camera_object *camera = NULL;


  if(!(camera = calloc(1, sizeof(ay_camera_object))))
    { return AY_EOMEM; }

  fscanf(fileptr,"%lg\n", &camera->from[0]);
  fscanf(fileptr,"%lg\n", &camera->from[1]);
  fscanf(fileptr,"%lg\n", &camera->from[2]);
  fscanf(fileptr,"%lg\n", &camera->to[0]);
  fscanf(fileptr,"%lg\n", &camera->to[1]);
  fscanf(fileptr,"%lg\n", &camera->to[2]);
  fscanf(fileptr,"%lg\n", &camera->up[0]);
  fscanf(fileptr,"%lg\n", &camera->up[1]);
  fscanf(fileptr,"%lg\n", &camera->up[2]);

  fscanf(fileptr,"%lg\n", &camera->roll);
  fscanf(fileptr,"%lg\n", &camera->zoom);

  if(ay_read_version >= 4)
    {
      fscanf(fileptr,"%lg\n", &camera->nearp);
      fscanf(fileptr,"%lg\n", &camera->farp);
    }

  o->refine = camera;

 return AY_OK;
} /* ay_camera_readcb */


int
ay_camera_writecb(FILE *fileptr, ay_object *o)
{
 ay_camera_object *camera = NULL;

  camera = (ay_camera_object *)(o->refine);
 
  if(!camera)
    return AY_ENULL;

  fprintf(fileptr,"%g\n",camera->from[0]);
  fprintf(fileptr,"%g\n",camera->from[1]);
  fprintf(fileptr,"%g\n",camera->from[2]);
  fprintf(fileptr,"%g\n",camera->to[0]);
  fprintf(fileptr,"%g\n",camera->to[1]);
  fprintf(fileptr,"%g\n",camera->to[2]);
  fprintf(fileptr,"%g\n",camera->up[0]);
  fprintf(fileptr,"%g\n",camera->up[1]);
  fprintf(fileptr,"%g\n",camera->up[2]);

  fprintf(fileptr,"%g\n",camera->roll);
  fprintf(fileptr,"%g\n",camera->zoom);

  fprintf(fileptr,"%g\n",camera->nearp);
  fprintf(fileptr,"%g\n",camera->farp);

 return AY_OK;
} /* ay_camera_writecb */


int
ay_camera_wribcb(char *file, ay_object *o)
{
 ay_camera_object *camera = NULL;

  if(!o)
   return AY_ENULL;

  camera = (ay_camera_object*)o->refine;

  if(!camera)
    return AY_ENULL;

 return AY_OK;
} /* ay_camera_wribcb */


int
ay_camera_bbccb(ay_object *o, double *bbox, int *flags)
{
 double xmin = DBL_MAX, xmax = -DBL_MAX, ymin = DBL_MAX;
 double ymax = -DBL_MAX, zmin = DBL_MAX, zmax = -DBL_MAX;
 double from[3] = {0};
 double to[3] = {0};
 ay_camera_object *camera = NULL;

  if(!o || !bbox)
    return AY_ENULL;

  camera = (ay_camera_object *)o->refine;

  from[0] = camera->from[0];
  from[1] = camera->from[1];
  from[2] = camera->from[2];

  to[0] = camera->to[0];
  to[1] = camera->to[1];
  to[2] = camera->to[2];

  if(from[0] < xmin)
    xmin = from[0];
  if(from[0] > xmax)
    xmax = from[0];

  if(from[1] < ymin)
    ymin = from[1];
  if(from[1] > ymax)
    ymax = from[1];

  if(from[2] < zmin)
    zmin = from[2];
  if(from[2] > zmax)
    zmax = from[2];
  if(to[0] < xmin)
    xmin = to[0];
  if(to[0] > xmax)
    xmax = to[0];

  if(to[1] < ymin)
    ymin = to[1];
  if(to[1] > ymax)
    ymax = to[1];

  if(to[2] < zmin)
    zmin = to[2];
  if(to[2] > zmax)
    zmax = to[2];

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
} /* ay_camera_bbccb */


/* ay_camera_dropcb:
 *  objects are dropped onto a camera object
 *
 */
int
ay_camera_dropcb(ay_object *o)
{
 int ay_status = AY_OK;
 char fname[] = "camera_drop";
 ay_list_object *sel = ay_selection;
 ay_object *s = NULL;
 ay_camera_object *camera;
 ay_view_object *view;
 char argsave[] = "save", argsel[] = "savsel";
 char *argvsave[2] = {NULL, argsave}, *argvsel[2] = {NULL, argsel};
 ay_list_object *oldsel = NULL, newsel = {0};

  if(!sel)
    {
      ay_error(AY_ENOSEL, fname, NULL);
      return AY_EDONOTLINK;
    }

  camera = (ay_camera_object *)o->refine;

  while(sel)
    {
      s = sel->object; 

      if(s->type == AY_IDVIEW)
	{
	  view = (ay_view_object *)s->refine;

	  /* fake selection containing the camera, so that it
	     may be saved by the undo facility */
	  oldsel = ay_selection;
	  newsel.object = o;
	  ay_selection = &newsel;
	  ay_status = ay_undo_undotcmd(NULL, ay_interp, 2, argvsave);

	  memcpy(camera->from, view->from, 3*sizeof(double));
	  memcpy(camera->to, view->to, 3*sizeof(double));
	  memcpy(camera->up, view->up, 3*sizeof(double));
	  camera->roll = view->roll;
	  camera->zoom = view->zoom;

	  /* save new state (because selection will not be changed
	     to drop target) */
	  ay_status = ay_undo_undotcmd(NULL, ay_interp, 2, argvsel);

	  /* now restore original selection */
	  ay_selection = oldsel;

	}
      sel = sel->next;
    }

 return AY_EDONOTLINK;
} /* ay_camera_dropcb */

int
ay_camera_init(Tcl_Interp *interp)
{
 int ay_status = AY_OK;

 ay_status = ay_otype_registercore(ay_camera_name,
				   ay_camera_createcb,
				   ay_camera_deletecb,
				   ay_camera_copycb,
				   ay_camera_drawcb,
				   ay_camera_drawhcb,
				   NULL, /* no shading! */
				   ay_camera_setpropcb,
				   ay_camera_getpropcb,
				   ay_camera_getpntcb,
				   ay_camera_readcb,
				   ay_camera_writecb,
				   NULL, /* no RIB export */
				   ay_camera_bbccb,
				   AY_IDCAMERA);


  /* register drop callback */
  ay_status = ay_tree_registerdrop(ay_camera_dropcb, AY_IDCAMERA);

  ay_matt_nomaterial(AY_IDCAMERA);

 return ay_status;
} /* ay_camera_init */
