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

/* vact.c - view related interactive actions */


int
ay_vact_movetcb(struct Togl *togl, int argc, char *argv[])
{
 Tcl_Interp *interp = Togl_Interp (togl);
 ay_view_object *view = (ay_view_object *)Togl_GetClientData(togl);
 static double oldwinx = 0.0, oldwiny = 0.0;
 double winx = 0.0, winy = 0.0;
 static double olddx = 0.0, olddy = 0.0;
 double dxw = 0.0, dyw = 0.0;
 double t[3] = {0}, t2[2] = {0};
 GLdouble mm[16]= {0}, rotx = 0.0, roty = 0.0;
 char fname[] = "move_view";

  /* parse args */
  if(argc == 5)
    {
      if(!strcmp(argv[2],"-winxy"))
      {
	Tcl_GetDouble(interp, argv[3], &winx);
	Tcl_GetDouble(interp, argv[4], &winy);

	if(view->type == AY_VTPERSP)
	  {
	    view->drawmarker = AY_FALSE;
	  }
      }
      else
	if(!strcmp(argv[2],"-start"))
	  {
	    Tcl_GetDouble(interp, argv[3], &winx);
	    Tcl_GetDouble(interp, argv[4], &winy);
	    oldwinx = winx;
	    oldwiny = winy;
	    olddx = 0.0;
	    olddy = 0.0;
	  }
    }
  else
    {
      ay_error(AY_EARGS, fname, "[-start winx winy|-winxy winx winy]");
      return TCL_OK;
    }

  /* adjust marker */
  if(view->drawmarker)
    {
      view->markx -= (oldwinx - winx);
      view->marky -= (oldwiny - winy);
    }
  
  dxw = (oldwinx - winx) * view->conv_x;
  dyw = -(oldwiny - winy) * view->conv_y;
  oldwinx = winx;
  oldwiny = winy;

  if(view->type == AY_VTPERSP)
    {
     dxw *= (1.0/view->zoom)*4;
     dyw *= (1.0/view->zoom)*4;
    }

  t[0] = view->from[0] - view->to[0];
  t[1] = view->from[1] - view->to[1];
  t[2] = view->from[2] - view->to[2];
  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
   glLoadIdentity();
   if(!t[0] && !t[2])
     {
       glTranslated(dxw, 0.0, -dyw);
     }
   else
     {
       roty = AY_R2D(atan2(t[0], t[2]));

       t2[0]=t[0];
       t2[1]=t[2];
       if(t[0] && t[2])
	 rotx = AY_R2D(atan2(t[1], AY_V2LEN(t2)));

       glRotated(roty, 0.0, 1.0, 0.0);
       glRotated(-rotx, 1.0, 0.0, 0.0);
       if(view->up[1] > 0.0)
	 glTranslated(dxw, dyw, 0.0);
       else
	 glTranslated(-dxw, -dyw, 0.0);
       glRotated(rotx, 1.0, 0.0, 0.0);
       glRotated(-roty, 0.0, 1.0, 0.0);
     }
   glGetDoublev(GL_MODELVIEW_MATRIX, mm);
  glPopMatrix();

  ay_trafo_apply3(view->from, mm);
  ay_trafo_apply3(view->to, mm);

  ay_toglcb_reshape(togl);
  ay_toglcb_display(togl);

  ay_viewt_uprop(view);

 return TCL_OK;
} /* ay_vact_movetcb */


int
ay_vact_zoomtcb(struct Togl *togl, int argc, char *argv[])
{
 Tcl_Interp *interp = Togl_Interp (togl);
 ay_view_object *view = (ay_view_object *)Togl_GetClientData(togl);
 static double oldwiny = 0.0;
 double winy = 0.0;
 double dy = 1.0;
 char fname[] = "zoom_view";

  /* parse args */
  if(argc == 4)
    {
      if(!strcmp(argv[2],"-winy"))
      {
	Tcl_GetDouble(interp, argv[3], &winy);

	view->drawmarker = AY_FALSE;

      }
      else
	if(!strcmp(argv[2],"-start"))
	  {
	    Tcl_GetDouble(interp, argv[3], &winy);
	    oldwiny = winy;
	  }
    }
  else
    {
      ay_error(AY_EARGS, fname, "[-start winy|-winy winy]");
      return TCL_OK;
    }

  if(oldwiny == winy)
    return TCL_OK;

  dy = 1.1;
  if((oldwiny - winy)>0)
    dy = 0.9;

  /* zoom the view */
  view->zoom *= dy;

  oldwiny = winy;

  if(dy != 1.0)
    {
      ay_toglcb_reshape(togl);
      ay_toglcb_display(togl);
    }

  ay_viewt_uprop(view);

 return TCL_OK;
} /* ay_vact_zoomtcb */


int
ay_vact_moveztcb(struct Togl *togl, int argc, char *argv[])
{
 Tcl_Interp *interp = Togl_Interp (togl);
 ay_view_object *view = (ay_view_object *)Togl_GetClientData(togl);
 static double oldwiny = 0.0;
 double winy = 0.0;
 static double olddy = 0.0;
 double dyw = 0.0;
 double v[3] = {0};
 char fname[] = "movez_view";

  /* parse args */
  if(argc == 4)
    {
      if(!strcmp(argv[2],"-winy"))
      {
	Tcl_GetDouble(interp, argv[3], &winy);

	if(view->type == AY_VTPERSP)
	  {
	    view->drawmarker = AY_FALSE;
	  }
      }
      else
	if(!strcmp(argv[2],"-start"))
	  {
	    Tcl_GetDouble(interp, argv[3], &winy);
	    oldwiny = winy;
	    olddy = 0.0;
	  }
    }
  else
    {
      ay_error(AY_EARGS, fname, "[-start winy|-winy winy]");
      return TCL_OK;
    }

  dyw = -(oldwiny - winy) * view->conv_x * 2.0;
  oldwiny = winy;

  /* move the view along its current z axis */
  v[0] = view->from[0] - view->to[0];
  v[1] = view->from[1] - view->to[1];
  v[2] = view->from[2] - view->to[2];

  AY_V3SCAL(v, dyw)

  view->from[0] += v[0];
  view->to[0] += v[0];
  view->from[1] += v[1];
  view->to[1] += v[1];
  view->from[2] += v[2];
  view->to[2] += v[2];    

  oldwiny = winy;

  if(dyw != 0.0)
    {
      ay_toglcb_reshape(togl);
      ay_toglcb_display(togl);
    }

  ay_viewt_uprop(view);

 return TCL_OK;
} /* ay_vact_moveztcb */
