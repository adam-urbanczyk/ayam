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
#include "tiffio.h"

/* view.c - view object */

static char *ay_view_name = "View";


/* ay_view_createcb:
 *  this callback does nothing,
 *  the real work of correctly creating a view object is done 
 *  in the Togl callback: ay_toglcb_create()
 */
int
ay_view_createcb(int argc, char *argv[], ay_object *o)
{
 char fname[] = "ay_view_createcb";

  ay_error(AY_ERROR, fname, "Can not create a view this way, use viewOpen!");

 return AY_ERROR;
} /* ay_view_createcb */


/* ay_view_deletecb:
 *  this callback does nothing,
 *  the real work of correctly deleting a view object is done 
 *  in the Togl callback: ay_toglcb_destroy()
 */
int
ay_view_deletecb(void *c)
{
 char fname[] = "ay_view_deletecb";
 ay_view_object *view = NULL;

  view = (ay_view_object *)(c);
  if(!view)
    return AY_ENULL;    

  ay_error(AY_ERROR, fname, "cannot delete a view this way, use View/Close");

 return AY_ERROR;
} /* ay_view_deletecb */


int
ay_view_copycb(void *src, void **dst)
{
 char fname[] = "ay_view_copycb";

  ay_error(AY_ERROR, fname, "can not copy a view object");

 return AY_ERROR;
} /* ay_view_copycb */


int
ay_view_drawcb(struct Togl *togl, ay_object *o)
{
 ay_view_object *view = NULL;
 /* double t[3] = {0};*/

  if(!o)
    return AY_ENULL;

  view = (ay_view_object *)o->refine;

  if(!view)
    return AY_ENULL;

  /* never draw in own view */
  if(view->togl != togl)
    {
      /* ignore transformation */
      glLoadIdentity();
      /*
      t[0] = (view->from[0]-view->to[0])*0.5;
      t[1] = (view->from[1]-view->to[1])*0.5;
      t[2] = (view->from[2]-view->to[2])*0.5;
      */
      glBegin(GL_LINES);
       glVertex3d((GLdouble)view->from[0],(GLdouble)view->from[1],
		  (GLdouble)view->from[2]);
       glVertex3d((GLdouble)view->to[0],(GLdouble)view->to[1],
		  (GLdouble)view->to[2]);
      glEnd();

      ay_draw_arrow(togl, view->from, view->to);

    }

 return AY_OK;
} /* ay_view_drawcb */


int
ay_view_drawhcb(struct Togl *togl, ay_object *o)
{
 ay_view_object *view = NULL;

  if(!o)
    return AY_ENULL;

  view = (ay_view_object *)o->refine;

  if(!view)
    return AY_ENULL;

  /* never draw in own view */
  if(view->togl != togl)
    { 
      /* ignore transformation */
      glLoadIdentity();

      /* draw */
      glBegin(GL_POINTS);
       glVertex3d((GLdouble)view->from[0],(GLdouble)view->from[1],
		  (GLdouble)view->from[2]);
       glVertex3d((GLdouble)view->to[0],(GLdouble)view->to[1],
		  (GLdouble)view->to[2]);
      glEnd();

    }


 return AY_OK;
} /* ay_view_drawhcb */


int
ay_view_shadecb(struct Togl *togl, ay_object *o)
{
 ay_view_object *view = NULL;

  if(!o)
    return AY_ENULL;

  view = (ay_view_object *)o->refine;

  if(!view)
    return AY_ENULL;

 return AY_OK;
} /* ay_view_shadecb */


/* ay_view_setpropcb:
 *  Tcl -> C!
 *  configure view from Tcl
 */
int
ay_view_setpropcb(Tcl_Interp *interp, int argc, char *argv[], ay_object *o)
{
 ay_view_object *view = NULL;
 char *n1 = "CameraData", *n2 = "ViewAttribData";
 Tcl_Obj *to = NULL, *toa = NULL, *ton = NULL;
 int itemp = 0;
 char *result;
 char fname[] = "view_setpropcb";

  if(!o)
    return AY_ENULL;

  view = (ay_view_object *)o->refine;

  toa = Tcl_NewStringObj(n1, -1);
  ton = Tcl_NewStringObj("From_X",-1);
  to = Tcl_ObjGetVar2(interp, toa, ton, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetDoubleFromObj(interp, to, &view->from[0]);
  Tcl_SetStringObj(ton, "From_Y", -1);
  to = Tcl_ObjGetVar2(interp, toa, ton, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetDoubleFromObj(interp, to, &view->from[1]);
  Tcl_SetStringObj(ton, "From_Z", -1);
  to = Tcl_ObjGetVar2(interp, toa, ton, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetDoubleFromObj(interp, to, &view->from[2]);

  Tcl_SetStringObj(ton, "To_X", -1);
  to = Tcl_ObjGetVar2(interp, toa, ton, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetDoubleFromObj(interp, to, &view->to[0]);
  Tcl_SetStringObj(ton, "To_Y", -1);
  to = Tcl_ObjGetVar2(interp, toa, ton, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetDoubleFromObj(interp, to, &view->to[1]);
  Tcl_SetStringObj(ton, "To_Z", -1);
  to = Tcl_ObjGetVar2(interp, toa, ton, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetDoubleFromObj(interp, to, &view->to[2]);

  Tcl_SetStringObj(ton, "Up_X", -1);
  to = Tcl_ObjGetVar2(interp, toa, ton, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetDoubleFromObj(interp, to, &view->up[0]);
  Tcl_SetStringObj(ton, "Up_Y", -1);
  to = Tcl_ObjGetVar2(interp, toa, ton, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetDoubleFromObj(interp, to, &view->up[1]);
  Tcl_SetStringObj(ton, "Up_Z", -1);
  to = Tcl_ObjGetVar2(interp, toa, ton, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetDoubleFromObj(interp, to, &view->up[2]);

  Tcl_SetStringObj(ton, "Zoom", -1);
  to = Tcl_ObjGetVar2(interp, toa, ton, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetDoubleFromObj(interp, to, &view->zoom);

  Tcl_SetStringObj(ton, "Roll", -1);
  to = Tcl_ObjGetVar2(interp, toa, ton, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetDoubleFromObj(interp, to, &view->roll);

  Tcl_SetStringObj(ton, "Rot_X", -1);
  to = Tcl_ObjGetVar2(interp, toa, ton, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetDoubleFromObj(interp, to, &view->rotx);
  Tcl_SetStringObj(ton, "Rot_Y", -1);
  to = Tcl_ObjGetVar2(interp, toa, ton, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetDoubleFromObj(interp, to, &view->roty);
  Tcl_SetStringObj(ton, "Rot_Z", -1);
  to = Tcl_ObjGetVar2(interp, toa, ton, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetDoubleFromObj(interp, to, &view->rotz);

  Tcl_SetStringObj(toa, n2, -1);
  Tcl_SetStringObj(ton, "Type", -1);
  to = Tcl_ObjGetVar2(interp, toa, ton, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp, to, &itemp);
  if(itemp != view->type)
    {
      ay_viewt_changetype(view, itemp);
    }

  Tcl_SetStringObj(ton, "Redraw", -1);
  to = Tcl_ObjGetVar2(interp, toa, ton, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp, to, &view->redraw);

  Tcl_SetStringObj(ton, "Shade", -1);
  to = Tcl_ObjGetVar2(interp, toa, ton, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp, to, &view->shade);

  Tcl_SetStringObj(ton, "DrawSel", -1);
  to = Tcl_ObjGetVar2(interp, toa, ton, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp, to, &view->drawsel);

  Tcl_SetStringObj(ton, "DrawLevel", -1);
  to = Tcl_ObjGetVar2(interp, toa, ton, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp, to, &view->drawlevel);

  Tcl_SetStringObj(ton, "Grid", -1);
  to = Tcl_ObjGetVar2(interp, toa, ton, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetDoubleFromObj(interp, to, &view->grid);

  Tcl_SetStringObj(ton, "DrawGrid", -1);
  to = Tcl_ObjGetVar2(interp, toa, ton, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp, to, &view->drawgrid);

  Tcl_SetStringObj(ton, "UseGrid", -1);
  to = Tcl_ObjGetVar2(interp, toa, ton, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp, to, &view->usegrid);

  Tcl_SetStringObj(ton, "Local", -1);
  to = Tcl_ObjGetVar2(interp, toa, ton, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp, to, &view->local);

  Tcl_SetStringObj(ton, "DrawBG", -1);
  to = Tcl_ObjGetVar2(interp, toa, ton, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp, to, &view->drawbg);

  result = Tcl_GetVar2(interp, "ViewAttribData", "BGImage",
		       TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  if((result && !view->bgimage)  ||
     (!result && view->bgimage)  ||
     (result && view->bgimage && strcmp(result,view->bgimage)))
    {
      if(view->bgimage)
	{
	  free(view->bgimage);
	  view->bgimage = NULL;
	} /* if */

      if(result)
	{
	  if(!(view->bgimage = calloc(strlen(result)+1, sizeof(char))))
	    {
	      ay_error(AY_EOMEM, fname, NULL);
	      return TCL_OK;
	    } /* if */
	  strcpy(view->bgimage, result);
	} /* if */
    } /* if */

  ay_notify_force(o);

  Tcl_IncrRefCount(toa); Tcl_DecrRefCount(toa);
  Tcl_IncrRefCount(ton); Tcl_DecrRefCount(ton);

  ay_toglcb_reshape(view->togl);
  ay_toglcb_display(view->togl);

  return TCL_OK;
} /* ay_view_setpropcb */


/* ay_view_getpropcb:
 *  C -> Tcl!
 *  copy all information about the view *togl
 *  to the global view() array (Tcl)
 */
int
ay_view_getpropcb(Tcl_Interp *interp, int argc, char *argv[], ay_object *o)
{
 ay_view_object *view = NULL;
 struct Togl *togl = NULL;
 int width = 0, height = 0;
 char *n1 = "CameraData", *n2 = "ViewAttribData";
 Tcl_Obj *to = NULL, *toa = NULL, *ton = NULL;

  if(!o)
    return AY_ENULL;

  view = (ay_view_object *)o->refine;
  togl = view->togl;
  width = Togl_Width(togl);
  height = Togl_Height(togl);

  toa = Tcl_NewStringObj(n1, -1);

  ton = Tcl_NewStringObj("From_X", -1);
  to = Tcl_NewDoubleObj(view->from[0]);
  Tcl_ObjSetVar2(interp, toa, ton, to, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_SetStringObj(ton, "From_Y", -1);
  to = Tcl_NewDoubleObj(view->from[1]);
  Tcl_ObjSetVar2(interp, toa, ton, to, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_SetStringObj(ton, "From_Z", -1);
  to = Tcl_NewDoubleObj(view->from[2]);
  Tcl_ObjSetVar2(interp, toa, ton, to, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton, "To_X", -1);
  to = Tcl_NewDoubleObj(view->to[0]);
  Tcl_ObjSetVar2(interp, toa, ton, to, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_SetStringObj(ton, "To_Y", -1);
  to = Tcl_NewDoubleObj(view->to[1]);
  Tcl_ObjSetVar2(interp, toa, ton, to, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_SetStringObj(ton, "To_Z", -1);
  to = Tcl_NewDoubleObj(view->to[2]);
  Tcl_ObjSetVar2(interp, toa, ton, to, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton, "Up_X", -1);
  to = Tcl_NewDoubleObj(view->up[0]);
  Tcl_ObjSetVar2(interp, toa, ton, to, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_SetStringObj(ton, "Up_Y", -1);
  to = Tcl_NewDoubleObj(view->up[1]);
  Tcl_ObjSetVar2(interp, toa, ton, to, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_SetStringObj(ton, "Up_Z", -1);
  to = Tcl_NewDoubleObj(view->up[2]);
  Tcl_ObjSetVar2(interp, toa, ton, to, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);


  Tcl_SetStringObj(ton, "Roll", -1);
  to = Tcl_NewDoubleObj(view->roll);
  Tcl_ObjSetVar2(interp, toa, ton, to, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton, "Zoom", -1);
  to = Tcl_NewDoubleObj(view->zoom);
  Tcl_ObjSetVar2(interp, toa, ton, to, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton, "Rot_X", -1);
  to = Tcl_NewDoubleObj(view->rotx);
  Tcl_ObjSetVar2(interp, toa, ton, to, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_SetStringObj(ton, "Rot_Y", -1);
  to = Tcl_NewDoubleObj(view->roty);
  Tcl_ObjSetVar2(interp, toa, ton, to, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_SetStringObj(ton, "Rot_Z", -1);
  to = Tcl_NewDoubleObj(view->rotz);
  Tcl_ObjSetVar2(interp, toa, ton, to, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);


  Tcl_SetStringObj(toa, n2, -1);
  Tcl_SetStringObj(ton, "Type", -1);
  to = Tcl_NewIntObj(view->type);
  Tcl_ObjSetVar2(interp, toa, ton, to, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton, "Width", -1);
  to = Tcl_NewIntObj(width);
  Tcl_ObjSetVar2(interp, toa, ton, to, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton, "Height", -1);
  to = Tcl_NewIntObj(height);
  Tcl_ObjSetVar2(interp, toa, ton, to, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton, "Redraw", -1);
  to = Tcl_NewIntObj(view->redraw);
  Tcl_ObjSetVar2(interp, toa, ton, to, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton, "Shade", -1);
  to = Tcl_NewIntObj(view->shade);
  Tcl_ObjSetVar2(interp, toa, ton, to, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton, "DrawSel", -1);
  to = Tcl_NewIntObj(view->drawsel);
  Tcl_ObjSetVar2(interp, toa, ton, to, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton, "DrawLevel", -1);
  to = Tcl_NewIntObj(view->drawlevel);
  Tcl_ObjSetVar2(interp, toa, ton, to, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton, "DrawLevel", -1);
  to = Tcl_NewIntObj(view->drawlevel);
  Tcl_ObjSetVar2(interp, toa, ton, to, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton, "Grid", -1);
  to = Tcl_NewDoubleObj(view->grid);
  Tcl_ObjSetVar2(interp, toa, ton, to, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton, "DrawGrid", -1);
  to = Tcl_NewIntObj(view->drawgrid);
  Tcl_ObjSetVar2(interp, toa, ton, to, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton, "UseGrid", -1);
  to = Tcl_NewIntObj(view->usegrid);
  Tcl_ObjSetVar2(interp, toa, ton, to, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton, "Local", -1);
  to = Tcl_NewIntObj(view->local);
  Tcl_ObjSetVar2(interp, toa, ton, to, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton, "DrawBG", -1);
  to = Tcl_NewIntObj(view->drawbg);
  Tcl_ObjSetVar2(interp, toa, ton, to, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton, "BGImage", -1);
  to = Tcl_NewStringObj(view->bgimage, -1);
  Tcl_ObjSetVar2(interp, toa, ton, to, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);


  Tcl_IncrRefCount(toa); Tcl_DecrRefCount(toa);
  Tcl_IncrRefCount(ton); Tcl_DecrRefCount(ton);

 return AY_OK;
} /* ay_view_getpropcb */


int
ay_view_getpntcb(int mode, ay_object *o, double *p)
{
 ay_view_object *view = NULL;
 double min_dist = ay_prefs.pick_epsilon, dist = 0.0;
 double *pecoord = NULL, **pecoords = NULL, *c;
 int a;

  if(!o || !p)
    return AY_ENULL;

  view = (ay_view_object *)(o->refine);

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

      ay_point_edit_coords[0] = view->from;
      ay_point_edit_coords[1] = view->to;
      ay_point_edit_coords_homogenous = AY_FALSE;
      ay_point_edit_coords_number = 2;

    }
  else
    { /* no */

      /* selection based on a single point? */
      if(mode == 1)
	{ /* yes */

	  dist = AY_VLEN((p[0] - view->from[0]),
			 (p[1] - view->from[1]),
			 (p[2] - view->from[2]));

	  if(dist < min_dist)
	    {
	      pecoord = view->from;
	      min_dist = dist;
	    }

	  dist = AY_VLEN((p[0] - view->to[0]),
			 (p[1] - view->to[1]),
			 (p[2] - view->to[2]));

	  if(dist < min_dist)
	    {
	      pecoord = view->to;
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
	  c = view->from;
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

	  c = view->to;
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
} /* ay_view_getpntcb */


int
ay_view_readcb(FILE *fileptr, ay_object *o)
{
 int width, height, read = 0;
 char command[255] = {0}, update_cmd[] = "update";
 ay_object *root = ay_root, *down, *last;
 ay_view_object vtemp = {0}, *v;


  fscanf(fileptr,"%d\n", &width);
  fscanf(fileptr,"%d\n", &height);
  fscanf(fileptr,"%d\n", &vtemp.type);
  vtemp.up[0] = 0.0;
  vtemp.up[1] = 0.0;
  vtemp.up[2] = 0.0;
  if(vtemp.type != AY_VTTOP)
    {
      vtemp.up[1]=1.0;
    }
  else
    {
      vtemp.up[2]=-1.0;
    }
  fscanf(fileptr,"%lg\n", &vtemp.grid);
  fscanf(fileptr,"%d\n", &vtemp.drawsel);
  fscanf(fileptr,"%d\n", &vtemp.drawlevel);
  fscanf(fileptr,"%d\n", &vtemp.redraw);
  fscanf(fileptr,"%d\n", &vtemp.drawgrid);
  fscanf(fileptr,"%d\n", &vtemp.usegrid);
  fscanf(fileptr,"%d\n", &vtemp.local);
  fscanf(fileptr,"%d\n", &vtemp.shade);

  fscanf(fileptr,"%lg\n", &vtemp.from[0]);
  fscanf(fileptr,"%lg\n", &vtemp.from[1]);
  fscanf(fileptr,"%lg\n", &vtemp.from[2]);
  fscanf(fileptr,"%lg\n", &vtemp.to[0]);
  fscanf(fileptr,"%lg\n", &vtemp.to[1]);
  fscanf(fileptr,"%lg\n", &vtemp.to[2]);
  fscanf(fileptr,"%lg\n", &vtemp.up[0]);
  fscanf(fileptr,"%lg\n", &vtemp.up[1]);
  fscanf(fileptr,"%lg\n", &vtemp.up[2]);

  fscanf(fileptr,"%lg\n", &vtemp.roll);
  fscanf(fileptr,"%lg\n", &vtemp.zoom);

  fscanf(fileptr,"%d\n", &vtemp.pos_x);
  fscanf(fileptr,"%d\n", &vtemp.pos_y);

  /*  printf("%d %d\n",vtemp.pos_x, vtemp.pos_x );*/

  fscanf(fileptr,"%lg\n", &vtemp.rotx);
  fscanf(fileptr,"%lg\n", &vtemp.roty);
  fscanf(fileptr,"%lg", &vtemp.rotz);

  read = fgetc(fileptr);

  if(ay_read_version == 2)
    {
      ay_read_string(fileptr, &(vtemp.bgimage));
      fscanf(fileptr,"%d\n", &vtemp.drawbg);
    }

  vtemp.drawhandles = AY_FALSE;

  /* open the view */
  sprintf(command,"viewOpen %d %d 0\n", width, height);
  Tcl_Eval(ay_interp, command);
  Tcl_Eval(ay_interp, update_cmd);
      
  /* configure it */
  down = root->down;
  last = root->down;
  while(down->next)
    {
      last = down;
      down = down->next;
    }

  v = (ay_view_object *)last->refine;

  vtemp.togl = v->togl;
  memcpy(v, &vtemp, sizeof(ay_view_object));
  v->bgimage = NULL;
  if(vtemp.bgimage)
    {
      if((v->bgimage = calloc(strlen(vtemp.bgimage)+1,sizeof(char))))
	{
	  strcpy(v->bgimage, vtemp.bgimage);
	}
    }

  /* was: ay_toglcb_reshape(v->togl);*/
  /* notify also includes reshape() and additionally loads the BGImage */
  ay_notify_force(last);

  /* set window title */
  if(vtemp.type == AY_VTFRONT)
    sprintf(command,
	    "global ay;viewTitle [lindex $ay(views) end] Front Pick");
  if(vtemp.type == AY_VTSIDE)
    sprintf(command,
	    "global ay;viewTitle [lindex $ay(views) end] Side Pick");
  if(vtemp.type == AY_VTTOP)
    sprintf(command,
	    "global ay;viewTitle [lindex $ay(views) end] Top Pick");
  if(vtemp.type == AY_VTPERSP)
    sprintf(command,
	    "global ay;viewTitle [lindex $ay(views) end] Persp Pick");
  if(vtemp.type == AY_VTTRIM)
    sprintf(command,
	    "global ay;viewTitle [lindex $ay(views)_views end] Trim Pick");

  Tcl_Eval(ay_interp, command);

  /* position window */
  
  sprintf(command,
	  "global ay;winMoveOrResize [lindex $ay(views) end] \"+%d+%d\"\n",
	  vtemp.pos_x, vtemp.pos_y);

  Tcl_Eval(ay_interp, command);

  sprintf(command,
	  "global ay;viewSetGridIcon [lindex $ay(views) end] %g\n",
	  vtemp.grid);

  Tcl_Eval(ay_interp, command);

  Tcl_Eval(ay_interp, update_cmd);
  
 return AY_EDONOTLINK;
} /* ay_view_readcb */


int
ay_view_writecb(FILE *fileptr, ay_object *o)
{
 ay_view_object *view = NULL;
 struct Togl *togl = NULL;

  view = (ay_view_object *)(o->refine);
 
  if(!view)
    return AY_ENULL;

  togl = view->togl;
  fprintf(fileptr,"%d\n",Togl_Width(togl));
  fprintf(fileptr,"%d\n",Togl_Height(togl));
  fprintf(fileptr,"%d\n",view->type);
  fprintf(fileptr,"%g\n",view->grid);
  fprintf(fileptr,"%d\n",view->drawsel);
  fprintf(fileptr,"%d\n",view->drawlevel);
  fprintf(fileptr,"%d\n",view->redraw);
  fprintf(fileptr,"%d\n",view->drawgrid);
  fprintf(fileptr,"%d\n",view->usegrid);
  fprintf(fileptr,"%d\n",view->local);
  fprintf(fileptr,"%d\n",view->shade);

  fprintf(fileptr,"%g\n",view->from[0]);
  fprintf(fileptr,"%g\n",view->from[1]);
  fprintf(fileptr,"%g\n",view->from[2]);
  fprintf(fileptr,"%g\n",view->to[0]);
  fprintf(fileptr,"%g\n",view->to[1]);
  fprintf(fileptr,"%g\n",view->to[2]);
  fprintf(fileptr,"%g\n",view->up[0]);
  fprintf(fileptr,"%g\n",view->up[1]);
  fprintf(fileptr,"%g\n",view->up[2]);

  fprintf(fileptr,"%g\n",view->roll);
  fprintf(fileptr,"%g\n",view->zoom);

  fprintf(fileptr,"%d\n",view->pos_x);
  fprintf(fileptr,"%d\n",view->pos_y);

  fprintf(fileptr,"%g\n",view->rotx);
  fprintf(fileptr,"%g\n",view->roty);
  fprintf(fileptr,"%g\n",view->rotz);
  if(view->bgimage)
    {
      fprintf(fileptr,"%s\n",view->bgimage);
    }
  else
    {
      fprintf(fileptr,"\n");
    }
  fprintf(fileptr,"%d\n",view->drawbg);

 return AY_OK;
} /* ay_view_writecb */


int
ay_view_wribcb(char *file, ay_object *o)
{
 ay_view_object *view = NULL;

  if(!o)
   return AY_ENULL;

  view = (ay_view_object*)o->refine;

  if(!view)
    return AY_ENULL;

 return AY_OK;
} /* ay_view_wribcb */


int
ay_view_bbccb(ay_object *o, double *bbox, int *flags)
{
 double xmin = DBL_MAX, xmax = -DBL_MAX, ymin = DBL_MAX;
 double ymax = -DBL_MAX, zmin = DBL_MAX, zmax = -DBL_MAX;
 double from[3] = {0};
 double to[3] = {0};
 ay_view_object *view = NULL;

  if(!o || !bbox)
    return AY_ENULL;

  view = (ay_view_object *)o->refine;

  from[0] = view->from[0];
  from[1] = view->from[1];
  from[2] = view->from[2];

  to[0] = view->to[0];
  to[1] = view->to[1];
  to[2] = view->to[2];

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
} /* ay_view_bbccb */

int
ay_view_notifycb(ay_object *o)
{
 ay_view_object *view = NULL;
 unsigned int bo = 1;  /* test byte order */
 unsigned char *r, b;
 uint32 *image = NULL, w, h, c;
 TIFF* tif;
 char fname[] = "view_notifycb";
 GLint result;

  if(!o)
    return AY_ENULL;    

  view = (ay_view_object *)(o->refine);

  /* reset rotx,rotz recalc roty */
  /*
  view->rotx = 0.0;
  view->roty = 0.0;
  view->rotz = 0.0;

  temp[0] = view->to[0]-view->from[0];
  temp[1] = view->to[2]-view->from[2];
  view->roty = 180.0+AY_R2D(atan2(temp[0],temp[1]));
  */
  /*
  switch(view->type)
    {
    case AY_VTFRONT:
    case AY_VTSIDE:
    case AY_VTPERSP:
    case AY_VTTRIM:
      view->up[0] = 0.0;
      view->up[1] = 1.0;
      view->up[2] = 0.0;
      break;
    case AY_VTTOP:
      view->up[0] = 0.0;
      view->up[1] = 0.0;
      view->up[2] = 1.0;
      break;
    default:
      break;
    }
  */
  view->dirty = AY_TRUE;

  ay_viewt_uprop(view);

  /* load texture */
  if(view->bgimage && view->bgimage[0] != '\0')
    {

      tif = TIFFOpen(view->bgimage, "rH");
      if(tif)
	{
	  TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, &w);
	  TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &h);
	  if(!(image = (uint32*)_TIFFmalloc(w*h*sizeof(uint32))))
	    {
	      ay_error(AY_EOMEM, fname, NULL);
	      return TCL_OK;
	    }
	  if(TIFFReadRGBAImage(tif, w, h, image, 0) == 0)
	    {
	      /* Error while reading TIFF */
	      ay_error(AY_ERROR, fname, "TIFFReadRGBAImage() failed for:");
	      ay_error(AY_ERROR, fname, view->bgimage);
	      TIFFClose(tif);
	      return AY_FALSE;
	    }
	  else
            {
              /* check/correct byte order */
              r = (unsigned char *)&bo;
              if(r[0] == 0)
                {
                  /* byte order must be corrected: we need intel format */
                  for(c = 0; c < w*h; c++)
                    {
                      r = (unsigned char *)&image[c];
                      b = r[0];
                      r[0] = r[3];
                      r[3] = b;
                      b = r[1];
                      r[1] = r[2];
                      r[2] = b;
                    } /* for */
                } /* if */
            } /* if */
	  TIFFClose(tif);
	}
      else
	{
	  /* Unable to open TIFF */
	  ay_error(AY_ERROR, fname, "TIFFOpen() failed for:");
	  ay_error(AY_ERROR, fname, view->bgimage);
	  return AY_FALSE;
	} /* if */

      Togl_MakeCurrent(view->togl);

      glEnable(GL_TEXTURE_2D);

      result = gluBuild2DMipmaps(GL_TEXTURE_2D, 4, w, h, GL_RGBA,
				 GL_UNSIGNED_BYTE, image);

      if(result != 0)
	{
	  ay_error(AY_ERROR, fname, "Unable to create texture.");
	  /*ay_error(AY_ERROR, fname, gluErrorString(result));*/
	}

      /* high quality texture mapping */
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
		      GL_LINEAR_MIPMAP_LINEAR);


      glDisable(GL_TEXTURE_2D);
      _TIFFfree(image);

    } /* if */


  return AY_OK;
} /* ay_view_notifycb */


/* ay_view_dropcb:
 *  objects are dropped onto a view object
 *
 */
int
ay_view_dropcb(ay_object *o)
{
 char fname[] = "view_drop";
 ay_list_object *sel = ay_selection;
 ay_object *s = NULL;
 ay_view_object *view = NULL;
 ay_camera_object *camera = NULL;
 ay_light_object *light = NULL;
 int width, height;
 double aspect;
 double from[3], to[3], mr[16];
 GLdouble m[16];

  if(!sel)
    {
      ay_error(AY_ENOSEL, fname, NULL);
      return AY_EDONOTLINK;
    }

  view = (ay_view_object *)o->refine;
  Togl_MakeCurrent(view->togl);
  width = Togl_Width(view->togl);
  height = Togl_Height(view->togl);


  while(sel)
    {
      s = sel->object; 
      switch(s->type)
	{
	case AY_IDLIGHT:
	  light = (ay_light_object *)s->refine;
	  if(light->type == AY_LITSPOT)
	    {
	      /* ay_viewt_changetype(view, AY_VTPERSP); */

	      memcpy(from, light->tfrom, 3*sizeof(double));
	      memcpy(to, light->tto, 3*sizeof(double));

	      glMatrixMode (GL_MODELVIEW);
	      glPushMatrix();
	       glLoadIdentity ();

	       ay_trafo_getall(ay_currentlevel->next);

	       glTranslated(s->movx, s->movy, s->movz);

	       ay_quat_torotmatrix(s->quat, mr);
	       glMultMatrixd(mr);

	       glScaled (s->scalx, s->scaly, s->scalz);
	       
	       glGetDoublev(GL_MODELVIEW_MATRIX, m);
	      glPopMatrix();
	      
	      ay_trafo_apply3(from, m);
	      ay_trafo_apply3(to, m);

	      memcpy(view->from, from, 3*sizeof(double));
	      memcpy(view->to, to, 3*sizeof(double));
	      
	      if(width<height)
		{
		  aspect = ((double)width) / ((double)height);
		  view->zoom = fabs(tan(light->cone_angle)/aspect);
		}
	      else
		{
		  view->zoom = fabs(tan(light->cone_angle));
		}
	      /* XXXX multiply zoom to accomodate parallel view types? */

	      ay_toglcb_reshape(view->togl);
	      ay_toglcb_display(view->togl);
	      ay_viewt_uprop(view);
	      view->drawmarker = AY_FALSE;

	    }
	  break;
	case AY_IDCAMERA:
	  camera = (ay_camera_object *)s->refine;
	  
	  memcpy(view->from, camera->from, 3*sizeof(double));
	  memcpy(view->to, camera->to, 3*sizeof(double));
	  memcpy(view->up, camera->up, 3*sizeof(double));
	  view->roll = camera->roll;
	  view->zoom = camera->zoom;
	  
	  ay_toglcb_reshape(view->togl);
	  ay_toglcb_display(view->togl);
	  ay_viewt_uprop(view);
	  view->drawmarker = AY_FALSE;
	  break;
	default:
	  break;
	}
      sel = sel->next;
    }

  ay_notify_force(o);

 return AY_EDONOTLINK;
} /* ay_view_dropcb */


int
ay_view_init(Tcl_Interp *interp)
{
 int ay_status = AY_OK;

 ay_status = ay_otype_registercore(ay_view_name,
				   ay_view_createcb,
				   ay_view_deletecb,
				   ay_view_copycb,
				   ay_view_drawcb,
				   ay_view_drawhcb,
				   NULL, /* no shading! */
				   ay_view_setpropcb,
				   ay_view_getpropcb,
				   ay_view_getpntcb,
				   ay_view_readcb,
				   ay_view_writecb,
				   NULL, /* no RIB export */
				   ay_view_bbccb,
				   AY_IDVIEW);

  Tcl_SetVar(interp, "propertyList", "ViewAttr", TCL_APPEND_VALUE |
	     TCL_LIST_ELEMENT | TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  ay_status = ay_notify_register(ay_view_notifycb, AY_IDVIEW);

  /* register drop callback */
  ay_status = ay_tree_registerdrop(ay_view_dropcb, AY_IDVIEW);

  ay_matt_nomaterial(AY_IDVIEW);

 return ay_status;
} /* ay_view_init */
