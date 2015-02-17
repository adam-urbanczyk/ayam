/*
 * Ayam, a free 3D modeler for the RenderMan interface.
 *
 * Ayam is copyrighted 1998-2005 by Randolf Schultz
 * (randolf.schultz@gmail.com) and others.
 *
 * All rights reserved.
 *
 * See the file License for details.
 *
 */

#include "ayam.h"
#include "tiffio.h"

#ifndef AYNOUNISTDH
#include <sys/times.h>
#include <unistd.h>
#endif

/* viewt.c - view management tools */


/* global variables: */

static Tcl_Obj *ayobj = NULL;
static Tcl_Obj *cvredrawobj = NULL;
static Tcl_Obj *cvdmodeobj = NULL;
static Tcl_Obj *cvdrawselobj = NULL;
static Tcl_Obj *cvdrawlevelobj = NULL;
static Tcl_Obj *cvdrawocsobj = NULL;
static Tcl_Obj *cvaalinesobj = NULL;
static Tcl_Obj *cvdrawbgobj = NULL;
static Tcl_Obj *cvdrawgridobj = NULL;
static Tcl_Obj *cvusegridobj = NULL;
static Tcl_Obj *cvmmodeobj = NULL;
static Tcl_Obj *cvgridsizeobj = NULL;
static Tcl_Obj *cvfovobj = NULL;
static Tcl_Obj *cvtypeobj = NULL;
static Tcl_Obj *cvbgimageobj = NULL;
static Tcl_Obj *cvfromxobj = NULL;
static Tcl_Obj *cvfromyobj = NULL;
static Tcl_Obj *cvfromzobj = NULL;
static Tcl_Obj *cvtoxobj = NULL;
static Tcl_Obj *cvtoyobj = NULL;
static Tcl_Obj *cvtozobj = NULL;
static Tcl_Obj *cvzoomobj = NULL;
static Tcl_Obj *cvpntsobj = NULL;
static Tcl_Obj *cvdrawmarkobj = NULL;
static Tcl_Obj *cvundoobj = NULL;
static Tcl_Obj *aymarkobj = NULL;
static Tcl_Obj *xyzobj[3] = {0};


/* functions: */

/* ay_viewt_setupprojection:
 *  setup the current projection matrix
 *  to reflect "from" and "to" camera settings
 *  of view *togl
 */
void
ay_viewt_setupprojection(struct Togl *togl)
{
 ay_view_object *view = (ay_view_object *)Togl_GetClientData(togl);
 int width = Togl_Width(togl);
 int height = Togl_Height(togl);
 GLdouble aspect = ((GLdouble)width) / ((GLdouble)height);
 GLfloat light_posf[4] = { 0.0f,  0.0f, 1000.0f, 0.0f};
 GLfloat light_poss[4] = {1000.0f,  0.0f,  0.0f, 0.0f};
 GLfloat light_post[4] = { 0.0f, 1000.0f,  0.0f, 0.0f};
 GLdouble nearp = view->nearp, farp = view->farp;

  glViewport(0, 0, width, height);

  glMatrixMode(GL_PROJECTION);

  glLoadIdentity();

  if(view->type == AY_VTPERSP)
    {
      if(view->nearp <= 1.0)
	{
	  nearp = 1.0;
	}

      if((view->farp == 0.0) || (view->farp <= nearp))
	{
	  farp = 1000.0;
	}

      glFrustum(-aspect*view->zoom*nearp, aspect*view->zoom*nearp,
		-1.0*view->zoom*nearp, 1.0*view->zoom*nearp,
		nearp, farp);
    }
  else
    {
      if(view->nearp == 0.0)
	{
	  nearp = -100.0;
	}

      if(view->farp == 0.0)
	{
	  farp = 100.0;
	}

      glOrtho(-aspect*view->zoom, aspect*view->zoom,
	      -1.0*view->zoom, 1.0*view->zoom,
	      nearp, farp);
    } /* if */

  if(view->roll != 0.0)
    glRotated(view->roll, 0.0, 0.0, 1.0);
  if(view->type != AY_VTTOP)
    gluLookAt(view->from[0], view->from[1], view->from[2],
	      view->to[0], view->to[1], view->to[2],
	      view->up[0], view->up[1], view->up[2]);
  else
    gluLookAt(view->from[0], view->from[1], view->from[2],
	      view->to[0], view->to[1], view->to[2],
	      view->up[0], view->up[1], view->up[2]);

  glMatrixMode(GL_MODELVIEW);

  /* XXXX set light0 at cam-pos? */
  glPushMatrix();
   glRotated(view->rotz, 0.0, 0.0, 1.0);
   glRotated(view->roty, 0.0, 1.0, 0.0);
   glRotated(view->rotx, 1.0, 0.0, 0.0);
   if((view->type == AY_VTPERSP) || (view->type == AY_VTFRONT))
     {
       if(view->farp > 1000.0)
	 light_posf[2] = (GLfloat)(view->farp*2);
       glLightfv(GL_LIGHT0, GL_POSITION, light_posf);
     }
   if(view->type == AY_VTSIDE)
     {
       if(view->farp > 1000.0)
	 light_poss[0] = (GLfloat)(view->farp*2);
       glLightfv(GL_LIGHT0, GL_POSITION, light_poss);
     }
   if(view->type == AY_VTTOP)
     {
       if(view->farp > 1000.0)
	 light_post[1] = (GLfloat)(view->farp*2);
       glLightfv(GL_LIGHT0, GL_POSITION, light_post);
     }
  glPopMatrix();

 return;
} /* ay_viewt_setupprojection */


/* ay_viewt_rotate:
 *  calculate new "from" and "to" values from
 *  rotx, roty and rotz
 */
void
ay_viewt_rotate(ay_view_object *view, double rotx, double roty, double rotz)
{
 GLdouble m[16] = {0};
 double x, y, z;

  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
   glLoadIdentity();

   if(rotx != 0.0)
     {
       glRotated(view->roty, 0.0, 1.0, 0.0);
       glRotated(rotx, 1.0, 0.0, 0.0);
       glRotated(-view->roty, 0.0, 1.0, 0.0);
     }

   if(rotz != 0.0)
     {
       glRotated(view->roty, 0.0, 1.0, 0.0);
       glRotated(rotz, 0.0, 0.0, 1.0);
       glRotated(-view->roty, 0.0, 1.0, 0.0);
     }

   if(roty != 0.0)
     {
       glRotated(roty, 0.0, 1.0, 0.0);
     }

   glGetDoublev(GL_MODELVIEW_MATRIX, m);
  glPopMatrix();

  x = view->to[0]-view->from[0];
  y = view->to[1]-view->from[1];
  z = view->to[2]-view->from[2];

  view->from[0] = view->to[0]+(m[0]*x + m[4]*y + m[8]*z + m[12]);
  view->from[1] = view->to[1]+(m[1]*x + m[5]*y + m[9]*z + m[13]);
  view->from[2] = view->to[2]+(m[2]*x + m[6]*y + m[10]*z + m[14]);

  x = view->up[0];
  y = view->up[1];
  z = view->up[2];

  view->up[0] = (m[0]*x + m[4]*y + m[8]*z + m[12]);
  view->up[1] = (m[1]*x + m[5]*y + m[9]*z + m[13]);
  view->up[2] = (m[2]*x + m[6]*y + m[10]*z + m[14]);

 return;
} /* ay_viewt_rotate */


/* ay_viewt_wintoobj:
 * transforms the window coordinates winX winY,
 * to object coordinates for the given object *o
 * Assumes the standard transformations are
 * in use!
 */
void
ay_viewt_wintoobj(struct Togl *togl, ay_object *o,
		  double winX, double winY,
		  double *objX, double *objY, double *objZ)
{
 int height = Togl_Height(togl);
 GLint viewport[4];
 GLdouble modelMatrix[16], projMatrix[16], winx, winy;
 GLfloat winz = 0.0f;

  if(!togl || !o || !objX || !objY || !objZ)
    return;

  winx = winX;
  winy = height - winY;

  glReadPixels((GLint)winx,(GLint)winy,1,1,GL_DEPTH_COMPONENT,GL_FLOAT,&winz);

  /* fprintf(stderr,"WinZ:%g\n",winz); */

  glGetIntegerv(GL_VIEWPORT, viewport);

  ay_viewt_setupprojection(togl);

  glGetDoublev(GL_PROJECTION_MATRIX, projMatrix);

  ay_trafo_identitymatrix(modelMatrix);
  ay_trafo_getall(ay_currentlevel, o, modelMatrix);

  gluUnProject(winx, winy, (GLdouble)winz, modelMatrix, projMatrix, viewport,
	       objX, objY, objZ);

  /* fprintf(stderr,"ObjX:%g; ObjY:%g; ObjZ:%g\n",*objX, *objY, *objZ); */

 return;
} /* ay_viewt_wintoobj */


/* ay_viewt_winrecttoobj:
 * transforms the rectangle formed by the window coordinates
 * winX, winY, winX2, winY2,
 * to object coordinates (obj[24]!) for the given object *o
 * Assumes the standard transformations are
 * in use!
 */
void
ay_viewt_winrecttoobj(struct Togl *togl, ay_object *o,
		      double winX, double winY, double winX2, double winY2,
		      double *obj)
{
 int height = Togl_Height(togl), i, j;
 GLint viewport[4];
 GLdouble modelMatrix[16], projMatrix[16], win[24];

  if(!togl || !o || !obj)
    return;

  j = AY_TRUE;

  for(i = 0; i < 8; i++)
    {
      if(j)
	{
	  win[i*3] = winX;
	  j = AY_FALSE;
	}
      else
	{
	  win[i*3] = winX2;
	  j = AY_TRUE;
	}

      win[i*3+1] = (height - winY);

      if(i < 4)
	{
	  win[i*3+2] = 0.0;
	}
      else
	{
	  win[i*3+2] = 1.0;
	}
    } /* for */

  win[7] = (height - winY2);
  win[10] = (height - winY2);
  win[19] = (height - winY2);
  win[22] = (height - winY2);

  glGetIntegerv(GL_VIEWPORT, viewport);

  ay_viewt_setupprojection(togl);

  glGetDoublev(GL_PROJECTION_MATRIX, projMatrix);

  ay_trafo_identitymatrix(modelMatrix);
  ay_trafo_getall(ay_currentlevel, o, modelMatrix);

  for(i = 0; i < 8; i++)
    {
      gluUnProject(win[i*3], win[i*3+1], win[i*3+2],
		   modelMatrix, projMatrix, viewport,
		   &(obj[i*3]), &(obj[i*3+1]), &(obj[i*3+2]));
    }

 return;
} /* ay_viewt_winrecttoobj */


/* ay_viewt_wintoworld:
 *  transforms the window coordinates winX winY
 *  to world coordinates
 */
void
ay_viewt_wintoworld(struct Togl *togl, double winX, double winY,
		    double *worldX, double *worldY, double *worldZ)
{
 int height = Togl_Height(togl);
 GLint viewport[4];
 GLdouble modelMatrix[16], projMatrix[16], winx, winy;
 GLfloat winz = 0.0f;

  if(!togl || !worldX || !worldY || !worldZ)
    return;

  winx = winX;
  winy = height - winY;

  glReadPixels((GLint)winx,(GLint)winy,1,1,GL_DEPTH_COMPONENT,GL_FLOAT,&winz);

  glGetIntegerv(GL_VIEWPORT, viewport);
  ay_viewt_setupprojection(togl);
  glGetDoublev(GL_PROJECTION_MATRIX, projMatrix);

  ay_trafo_identitymatrix(modelMatrix);

  gluUnProject(winx, winy, (GLdouble)winz, modelMatrix, projMatrix, viewport,
	       worldX, worldY, worldZ);

 return;
} /* ay_viewt_wintoworld */


/* ay_viewt_zoomtoobj:
 *
 *
 */
int
ay_viewt_zoomtoobj(struct Togl *togl, int argc, char *argv[])
{
 int ay_status = AY_OK;
 ay_view_object *view = (ay_view_object *)Togl_GetClientData(togl);
 ay_object *o = NULL;
 ay_list_object *sel, *oldsel = ay_selection;
 int zoom_to_all = AY_FALSE, restore_selection = AY_FALSE;
 double bb[24] = {0};
 double xmin = DBL_MAX, xmax = -DBL_MAX, ymin = DBL_MAX;
 double ymax = -DBL_MAX, zmin = DBL_MAX, zmax = -DBL_MAX;
 double cog[3] = {0}, dt[3] = {0}, dt2[3] = {0}, l, lx, ly, lz;
 int i, a, have_bb = AY_FALSE, set_far = AY_FALSE;
 GLdouble mt[16] = {0};

  if((argc > 2) && !strcmp(argv[2], "-all"))
    {
      ay_selection = NULL;
      o = ay_root->next;
      while(o && o->next)
	{
	  ay_sel_add(o, AY_FALSE);
	  o = o->next;
	}

      zoom_to_all = AY_TRUE;
      restore_selection = AY_TRUE;
   }

  if(ay_selection)
    {
      sel = ay_selection;
      while(sel)
	{
	  o = sel->object;

	  ay_status = ay_bbc_get(o, bb);
	  if(!ay_status)
	    {
	      have_bb = AY_TRUE;
	      a = 0;
	      for(i = 0; i < 8; i++)
		{
		  if(bb[a] < xmin)
		    xmin = bb[a];
		  if(bb[a] > xmax)
		    xmax = bb[a];
		  a += 3;
		}

	      a = 1;
	      for(i = 0; i < 8; i++)
		{
		  if(bb[a] < ymin)
		    ymin = bb[a];
		  if(bb[a] > ymax)
		    ymax = bb[a];
		  a += 3;
		}

	      a = 2;
	      for(i = 0; i < 8; i++)
		{
		  if(bb[a] < zmin)
		    zmin = bb[a];
		  if(bb[a] > zmax)
		    zmax = bb[a];
		  a += 3;
		}
	    } /* if */

	  sel = sel->next;
	} /* while */

      if(!have_bb)
	{
	  goto cleanup;
	}

      ay_trafo_identitymatrix(mt);
      if(!zoom_to_all && ay_currentlevel->object != ay_root)
	{
	  ay_trafo_getparent(ay_currentlevel->next, mt);
	}

      /* calc center of gravity (middle of bounding box) */
      cog[0] = xmin+((fabs(xmax-xmin))/2.0);
      cog[1] = ymin+((fabs(ymax-ymin))/2.0);
      cog[2] = zmin+((fabs(zmax-zmin))/2.0);
      /*
      printf("BB X:%g,%g,  Y:%g,%g, Z:%g,%g\n",xmin,xmax,ymin,ymax,zmin,zmax);
      */
      ay_trafo_apply3(cog, mt);

      AY_V3SUB(dt, view->to, cog);

      /* let the view point to objects COG */
      AY_V3SUB(view->to, view->to, dt);
      AY_V3SUB(view->from, view->from, dt);

      dt[0] = xmin;
      dt[1] = ymin;
      dt[2] = zmin;

      dt2[0] = xmax;
      dt2[1] = ymax;
      dt2[2] = zmax;

      ay_trafo_apply3(dt, mt);
      ay_trafo_apply3(dt2, mt);

      lx = fabs(dt[0]-dt2[0]);
      ly = fabs(dt[1]-dt2[1]);
      lz = fabs(dt[2]-dt2[2]);

      if(ly > lx)
	l = ly;
      else
	l = lx;
      if(l < lz)
	l = lz;

      if(view->type != AY_VTPERSP)
	{
	  /* parallel view */

	  /* calc new zoom factor */
	  view->zoom = l*0.9;

	  if(view->zoom < 1E-6 || view->zoom > 1E6)
	    {
	      view->zoom = 3.0;
	    }

	  /* adjust clipping planes */
	  if(view->nearp != 0.0)
	    lx = fabs(view->nearp);
	  else
	    lx = 100.0;
	  if(l > lx)
	    view->nearp = -l*1.1;

	  if(view->farp != 0.0)
	    lx = view->farp;
	  else
	    lx = 100.0;
	  if(l > lx)
	    view->farp = l*1.1;
	}
      else
	{
	  /* perspective view */

	  /* reset the zoom */
	  view->zoom = 0.25;

	  /* offset the view along eye vector */
	  AY_V3SUB(dt, view->from, view->to);
	  lx = AY_V3LEN(dt);
	  ly = ((l/2.0)/tan(view->zoom))/lx*2.1;
	  AY_V3SCAL(dt, ly);
	  AY_V3ADD(view->from, view->to, dt);

	  /* adjust clipping planes */
	  AY_V3SUB(dt, view->from, view->to);
	  lx = AY_V3LEN(dt);

	  if(view->nearp != 0.0)
	    ly = view->nearp;
	  else
	    ly = 0.1;
	  if(lx < ly)
	    {
	      set_far = AY_TRUE;
	      view->nearp = lx*0.1;
	    }

	  if(view->farp != 0.0)
	    ly = view->farp;
	  else
	    ly = 1000.0;
	  if(((l+lx) > ly) || set_far)
	    {
	      view->farp = (l+lx)*1.1;
	      if(!set_far)
		view->nearp = 0.1;
	    }
	} /* if */

      if(zoom_to_all)
	{
	  ay_sel_free(AY_FALSE);
	  ay_selection = oldsel;
	}

      /* realize camera changes */
      Togl_MakeCurrent(togl);
      ay_toglcb_reshape(togl);
      if(view->drawmark)
	{
	  ay_viewt_updatemark(togl, /*local=*/AY_TRUE);
	}
      ay_toglcb_display(togl);

      view->full_notify = AY_FALSE;
      ay_viewt_uprop(view, AY_TRUE);

      restore_selection = AY_FALSE;
    } /* if (sel) */

cleanup:

  if(restore_selection && zoom_to_all)
    {
      ay_sel_free(AY_FALSE);
      ay_selection = oldsel;
    }

 return TCL_OK;
} /* ay_viewt_zoomtoobj */


/* ay_viewt_align:
 *
 */
int
ay_viewt_align(struct Togl *togl, int argc, char *argv[])
{
 ay_view_object *view = (ay_view_object *)Togl_GetClientData(togl);
 GLdouble m[16] = {0};
 double from[3] = {0.0,0.0,10.0}, to[3] = {0}, up[3] = {0.0,1.0,0.0};
 double m2[16];
 int use_m2 = AY_FALSE;

  if(view->local == 2)
    {
      /* No parent object and no selection? This means there is nothing
	 we could align the view to. Thus, we just bail out. */
      if((ay_currentlevel->object == ay_root) && !ay_selection)
	return TCL_OK;

      if(ay_selection)
	{
	  ay_trafo_identitymatrix(m2);
	  ay_quat_torotmatrix(ay_selection->object->quat, m2);
	  use_m2 = AY_TRUE;
	}
    }

  /* get parent objects transformation */
  ay_trafo_identitymatrix(m);
  if(ay_currentlevel->object != ay_root)
    {
      ay_trafo_getparent(ay_currentlevel->next, m);
    }

  if(view->type == AY_VTSIDE)
    {
      from[0] = 10.0;
      from[2] = 0.0;
    }

  if(view->type == AY_VTTOP)
    {
      from[1] = 10.0;
      from[2] = 0.0;

      up[1] = 0.0;
      up[2] = -1.0;
    }

  if(use_m2)
    {
      ay_trafo_apply3(from, m2);
      ay_trafo_apply3(to, m2);
      ay_trafo_apply3(up, m2);
      view->aligned = AY_TRUE;
    }

  ay_trafo_apply3(from, m);
  ay_trafo_apply3(to, m);
  ay_trafo_apply3(up, m);

  memcpy(view->from, from, 3*sizeof(double));
  memcpy(view->to, to, 3*sizeof(double));
  memcpy(view->up, up, 3*sizeof(double));

  ay_toglcb_reshape(togl);
  ay_toglcb_display(togl);

  view->full_notify = AY_FALSE;
  ay_viewt_uprop(view, AY_TRUE);

  view->drawmark = AY_FALSE;

 return TCL_OK;
} /* ay_viewt_align */


/* ay_viewt_alignlocal:
 *
 */
void
ay_viewt_alignlocal(void)
{
 ay_object *o = ay_root;
 ay_view_object *view = NULL;

  o = o->down;

  if(!o)
    return;

  while(o->next)
    {
      if(o->type == AY_IDVIEW)
	{
	  view = (ay_view_object *)o->refine;
	  if(view->local == 2)
	    {
	      (void)ay_viewt_makecurtcb(view->togl, 0, NULL);
	      (void)ay_viewt_align(view->togl, 0, NULL);
	      (void)ay_viewt_zoomtoobj(view->togl, 0, NULL);
	    }
	}
      o = o->next;
    } /* while */

 return;
} /* ay_viewt_alignlocal */


/* ay_viewt_makecurtcb:
 *
 */
int
ay_viewt_makecurtcb(struct Togl *togl, int argc, char *argv[])
{
 Tcl_Interp *interp = ay_interp;
 Tcl_Obj *to = NULL;
 ay_view_object *view = NULL;

  view = (ay_view_object *)Togl_GetClientData(togl);

  Togl_MakeCurrent(togl);

  ay_viewt_setupprojection(togl);

  ay_currentview = view;

  to = Tcl_NewIntObj(view->redraw);
  Tcl_ObjSetVar2(interp, ayobj, cvredrawobj, to, TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);

  to = Tcl_NewIntObj(view->drawmode);
  Tcl_ObjSetVar2(interp, ayobj, cvdmodeobj, to, TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);

  to = Tcl_NewIntObj(view->drawsel);
  Tcl_ObjSetVar2(interp, ayobj, cvdrawselobj, to, TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);

  to = Tcl_NewIntObj(view->drawlevel);
  Tcl_ObjSetVar2(interp, ayobj, cvdrawlevelobj, to, TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);

  to = Tcl_NewIntObj(view->drawobjectcs);
  Tcl_ObjSetVar2(interp, ayobj, cvdrawocsobj, to, TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);

  to = Tcl_NewIntObj(view->antialiaslines);
  Tcl_ObjSetVar2(interp, ayobj, cvaalinesobj, to, TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);

  to = Tcl_NewIntObj(view->drawbgimage);
  Tcl_ObjSetVar2(interp, ayobj, cvdrawbgobj, to, TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);

  to = Tcl_NewIntObj(view->drawgrid);
  Tcl_ObjSetVar2(interp, ayobj, cvdrawgridobj, to, TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);

  to = Tcl_NewIntObj(view->usegrid);
  Tcl_ObjSetVar2(interp, ayobj, cvusegridobj, to, TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);

  to = Tcl_NewIntObj(view->local);
  Tcl_ObjSetVar2(interp, ayobj, cvmmodeobj, to, TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);

  to = Tcl_NewDoubleObj(view->grid);
  Tcl_ObjSetVar2(interp, ayobj, cvgridsizeobj, to, TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);

  to = Tcl_NewDoubleObj((atan(view->zoom)*180.0)/AY_PI*2.0);
  Tcl_ObjSetVar2(interp, ayobj, cvfovobj, to, TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);

  to = Tcl_NewIntObj(view->type);
  Tcl_ObjSetVar2(interp, ayobj, cvtypeobj, to, TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);

  to = Tcl_NewStringObj(view->bgimage, -1);
  Tcl_ObjSetVar2(interp, ayobj, cvbgimageobj, to, TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);

  to = Tcl_NewDoubleObj(view->from[0]);
  Tcl_ObjSetVar2(interp, ayobj, cvfromxobj, to, TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);

  to = Tcl_NewDoubleObj(view->from[1]);
  Tcl_ObjSetVar2(interp, ayobj, cvfromyobj, to, TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);

  to = Tcl_NewDoubleObj(view->from[2]);
  Tcl_ObjSetVar2(interp, ayobj, cvfromzobj, to, TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);

  to = Tcl_NewDoubleObj(view->to[0]);
  Tcl_ObjSetVar2(interp, ayobj, cvtoxobj, to, TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);

  to = Tcl_NewDoubleObj(view->to[1]);
  Tcl_ObjSetVar2(interp, ayobj, cvtoyobj, to, TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);

  to = Tcl_NewDoubleObj(view->to[2]);
  Tcl_ObjSetVar2(interp, ayobj, cvtozobj, to, TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);

  to = Tcl_NewDoubleObj(view->zoom);
  Tcl_ObjSetVar2(interp, ayobj, cvzoomobj, to, TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);

  to = Tcl_NewIntObj(view->transform_points);
  Tcl_ObjSetVar2(interp, ayobj, cvpntsobj, to, TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);

  to = Tcl_NewIntObj(view->drawmark);
  Tcl_ObjSetVar2(interp, ayobj, cvdrawmarkobj, to, TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);

  to = Tcl_NewIntObj(view->enable_undo);
  Tcl_ObjSetVar2(interp, ayobj, cvundoobj, to, TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);

 return TCL_OK;
} /* ay_viewt_makecurtcb */


/* ay_viewt_changetype:
 *
 */
void
ay_viewt_changetype(ay_view_object *view, int newtype)
{

  switch(newtype)
    {
    case AY_VTFRONT:
      view->from[0] = 0.0;
      view->from[1] = 0.0;
      view->from[2] = 10.0;
      view->to[0] = 0.0;
      view->to[1] = 0.0;
      view->to[2] = 0.0;
      view->up[0] = 0.0;
      view->up[1] = 1.0;
      view->up[2] = 0.0;
      break;
    case AY_VTSIDE:
      view->from[0] = 10.0;
      view->from[1] = 0.0;
      view->from[2] = 0.0;
      view->to[0] = 0.0;
      view->to[1] = 0.0;
      view->to[2] = 0.0;
      view->up[0] = 0.0;
      view->up[1] = 1.0;
      view->up[2] = 0.0;
      break;
    case AY_VTTOP:
      view->from[0] = 0.0;
      view->from[1] = 10.0;
      view->from[2] = 0.0;
      view->to[0] = 0.0;
      view->to[1] = 0.0;
      view->to[2] = 0.0;
      view->up[0] = 0.0;
      view->up[1] = 0.0;
      view->up[2] = 1.0;
      break;
    case AY_VTPERSP:
      view->from[0] = 10.0;
      view->from[1] = 10.0;
      view->from[2] = 10.0;
      view->to[0] = 0.0;
      view->to[1] = 0.0;
      view->to[2] = 0.0;
      view->up[0] = -0.707;
      view->up[1] = 1;
      view->up[2] = -0.707;
      break;
    case AY_VTTRIM:
      view->from[0] = 0.0;
      view->from[1] = 0.0;
      view->from[2] = 10.0;
      view->to[0] = 0.0;
      view->to[1] = 0.0;
      view->to[2] = 0.0;
      view->up[0] = 0.0;
      view->up[1] = 1.0;
      view->up[2] = 0.0;
      break;
    default:
      break;
    } /* switch */

  view->rotx = 0.0;
  view->roty = 0.0;
  view->rotz = 0.0;

  if(newtype != view->type)
    {
      if(view->type == AY_VTPERSP)
	{
	  view->zoom *= 12.0;

	  view->nearp = 0.0;
	  view->farp = 0.0;
	}
      else
	{
	  if(newtype == AY_VTPERSP)
	    {
	      view->zoom /= 12.0;
	    }
	} /* if */

    } /* if */

  view->type = newtype;
  view->aligned = AY_FALSE;

  if(view->drawmark)
    {
      ay_viewt_updatemark(view->togl, /*local=*/AY_TRUE);
    }

 return;
} /* ay_viewt_changetype */


/* ay_viewt_reshapetcb:
 *
 */
int
ay_viewt_reshapetcb(struct Togl *togl, int argc, char *argv[])
{

  ay_toglcb_reshape(togl);

 return TCL_OK;
} /* ay_viewt_reshapetcb */


/* ay_viewt_redrawtcb:
 *  draw a view regardless of automatic redraw setting (used by
 *  "View/Redraw Ctrl+d")
 *  apply changes to this function also to ay_toglcb_display()!
 */
int
ay_viewt_redrawtcb(struct Togl *togl, int argc, char *argv[])
{
 ay_view_object *view = (ay_view_object *)Togl_GetClientData(togl);

  if(view->altdispcb)
    {
      (view->altdispcb)(togl);
    }
  else
    {
      glClearColor((GLfloat)ay_prefs.bgr, (GLfloat)ay_prefs.bgg,
		   (GLfloat)ay_prefs.bgb, (GLfloat)1.0);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

      /* draw */
      if(view->drawmode)
	ay_shade_view(togl);
      else
	ay_draw_view(togl, AY_FALSE);

      /* XXXX is this really necessary? */
      /*glFlush();*/

      Togl_SwapBuffers(togl);
    }

#ifdef AYENABLEPPREV
  /* redraw permanent preview window? */
  if(view->ppreview)
    ay_wrib_pprevdraw(view);
#endif

 return TCL_OK;
} /* ay_viewt_redrawtcb */


/* ay_viewt_setconftcb:
 *
 */
int
ay_viewt_setconftcb(struct Togl *togl, int argc, char *argv[])
{
 int ay_status = AY_OK;
 char fname[] = "view_setconf";
 Tcl_Interp *interp = ay_interp;
 ay_object *o = NULL;
 ay_view_object *view = (ay_view_object *)Togl_GetClientData(togl);
 ay_list_object *sel;
 ay_pointedit pe = {0};
 int width = Togl_Width(togl);
 int height = Togl_Height(togl);
 int argi = 0, need_redraw = AY_TRUE, need_updatemark = AY_FALSE;
 double argd = 0.0, rotx = 0.0, roty = 0.0, rotz = 0.0;
 double old_rect_xmin, old_rect_ymin, old_rect_xmax, old_rect_ymax;
 double minlevelscale = 1.0, temp[3] = {0};
 double dxw, dyw, t[3] = {0}, t2[2] = {0}, mm[16] = {0};
 int i = 2;
 int kbdact_in_progress = AY_FALSE;
 int rectw, recth, dispx, dispy;
 char arg0[] = "undo";
 char arg1[] = "save";
 char arg2[] = "-start", arg3[] = "0";
 char arg4[] = "-winxy";
 char *tclargv[5] = {0}, buf1[64], buf2[64];

#ifndef AYNOUNISTDH
 static clock_t t_lastcalled = 0, t_current = 0;
 struct tms tbuf;
#endif
#ifdef WIN32
 static DWORD t_lastcalled = 0, t_current = 0;
#endif

#ifndef AYNOUNISTDH
  t_current = times(&tbuf);

  if(t_lastcalled != 0)
    {
    /*printf("%g\n",(double)(t_current-t_lastcalled)/(sysconf(_SC_CLK_TCK)));*/
      if((double)(t_current-t_lastcalled)/sysconf(_SC_CLK_TCK) < 0.1)
	{
	  kbdact_in_progress = AY_TRUE;
	}
      else
	{
	  kbdact_in_progress = AY_FALSE;
	}
    }
  else
    {
      kbdact_in_progress = AY_FALSE;
    }
#endif
#ifdef WIN32
  t_current = GetTickCount();

  if(t_lastcalled != 0)
    {
      if((t_current-t_lastcalled) < 100)
	{
	  kbdact_in_progress = AY_TRUE;
	}
      else
	{
	  kbdact_in_progress = AY_FALSE;
	}
    }
  else
    {
      kbdact_in_progress = AY_FALSE;
    }
#endif

  Togl_MakeCurrent(togl);
  ay_currentview = view;

  /* XXXX the i += 2 at the end of this while enforces arguments to contain
     two words, if other arguments follow; you may add dummy words:
     "-undrotx dummy -drotx 5" instead of "-undrotx -drotx 5" */
  while(i+1 < argc)
    {
      rotx = 0.0;
      Tcl_GetDouble(interp, argv[i+1], &argd);
      Tcl_GetInt(interp, argv[i+1], &argi);

      /* XXXX this code assumes, every argument is atleast 2 chars long! */
      switch(argv[i][1])
	{
	case 'a':
	  /*if(!strcmp(argv[i], "-action"))*/
	  view->action_state = argi;
	  break;
	case 'b':
	  /*if(!strcmp(argv[i], "-bgimage"))*/
	    {
	      if(view->bgimage)
		{
		  free(view->bgimage);
		  view->bgimage = NULL;
		} /* if */

	      if(argv[i+1])
		{
		  if(!(view->bgimage = calloc(strlen(argv[i+1])+1,
					      sizeof(char))))
		    {
		      ay_error(AY_EOMEM, fname, NULL);
		      return TCL_OK;
		    } /* if */
		  strcpy(view->bgimage, argv[i+1]);
		  view->bgimagedirty = AY_TRUE;
		  o = ay_root->down;
		  while(o)
		    {
		      if(o->refine == view)
			{
			  ay_notify_object(o);
			}
		      o = o->next;
		    } /* while */
		} /* if */
	    }
	  break;
	case 'c':
	  /*if(!strcmp(argv[i], "-cmark"))*/
	  if(argv[i][2] == 'm')
	    {
	      if(ay_selection)
		{
		  switch(argi)
		    {
		    case 0:
		      ay_status = ay_viewt_markfromsel(togl);
		      if(ay_status)
			{
			  need_redraw = AY_FALSE;
			  ay_error(AY_ERROR, fname, NULL);
			}
		    break;
		    case 1:
		    case 2:
		      ay_status = ay_viewt_markfromselp(togl, argi-1);
		      if(ay_status)
			{
			  need_redraw = AY_FALSE;
			  if(ay_status == AY_ERROR)
			    {
			      ay_error(AY_ERROR, fname, "No points selected!");
			    }
			  else
			    {
			      ay_error(ay_status, fname, NULL);
			    }
			}
		      break;
		    default:
		      break;
		    } /* switch */
		}
	      else
		{
		  ay_error(AY_ENOSEL, fname, NULL);
		}
	    } /* if */
	  /*-cp*/
	  if(argv[i][2] == 'p')
	    {
	      view->nearp = argd;
	      Tcl_GetDouble(interp, argv[i+2], &view->farp);
	    }
	  break;
	case 'd':
	  if(!strcmp(argv[i], "-dfromx"))
	    {
	      view->from[0] += argd;
	      view->aligned = AY_FALSE;
	      need_updatemark = AY_TRUE;
	    }
	  if(!strcmp(argv[i], "-dfromy"))
	    {
	      view->from[1] += argd;
	      view->aligned = AY_FALSE;
	      need_updatemark = AY_TRUE;
	    }
	  if(!strcmp(argv[i], "-dfromz"))
	    {
	      view->from[2] += argd;
	      view->aligned = AY_FALSE;
	      need_updatemark = AY_TRUE;
	    }
	  if(!strcmp(argv[i], "-dtox"))
	    {
	      view->to[0] += argd;
	      view->aligned = AY_FALSE;
	      need_updatemark = AY_TRUE;
	    }
	  if(!strcmp(argv[i], "-dtoy"))
	    {
	      view->to[1] += argd;
	      view->aligned = AY_FALSE;
	      need_updatemark = AY_TRUE;
	    }
	  if(!strcmp(argv[i], "-dtoz"))
	    {
	      view->to[2] += argd;
	      view->aligned = AY_FALSE;
	      need_updatemark = AY_TRUE;
	    }
	  if(!strcmp(argv[i], "-drotx"))
	    {
	      roty = argd;
	      view->roty += argd;

	      ay_viewt_rotate(view, 0.0, roty, 0.0);
	      need_updatemark = AY_TRUE;
	      view->aligned = AY_FALSE;
	    }
	  if(!strcmp(argv[i], "-droty"))
	    {
	      rotz = 0.0;
	      rotx = 0.0;
	      if(view->type == AY_VTSIDE)
		{
		  rotz = -argd;
		  view->rotz -= argd;
		}
	      else
		{
		  rotx = argd;
		  view->rotx += argd;
		}

	      ay_viewt_rotate(view, rotx, 0.0, rotz);
	      need_updatemark = AY_TRUE;
	      view->aligned = AY_FALSE;
	    }
	  if(!strcmp(argv[i], "-droll"))
	    {
	      view->roll = view->roll + argd;
	      if(view->roll > 180.0)
		view->roll = view->roll - 180.0;
	      else
		if(view->roll < -180.0)
		  view->roll = view->roll + 180.0;
	      view->aligned = AY_FALSE;
	      need_updatemark = AY_TRUE;
	    }
	  if(!strcmp(argv[i], "-dzoom"))
	    {
	      view->zoom *= argd;
	      need_updatemark = AY_TRUE;
	    }
	  if(!strcmp(argv[i], "-dsel"))
	    {
	      view->drawsel = argi;
	    }
	  if(!strcmp(argv[i], "-dlev"))
	    {
	      view->drawlevel = argi;
	    }
	  if(!strcmp(argv[i], "-dbg"))
	    {
	      view->drawbgimage = argi;
	    }
	  if(!strcmp(argv[i], "-docs"))
	    {
	      view->drawobjectcs = argi;
	    }
	  if(!strcmp(argv[i], "-doaal"))
	    {
	      view->antialiaslines = argi;
	    }
	  if(!strcmp(argv[i], "-draw"))
	    {
	      view->redraw = argi;
	    }
	  if(!strcmp(argv[i], "-drawg"))
	    {
	      view->drawgrid = argi;
	    }
	  if(!strcmp(argv[i], "-drawh"))
	    {
	      if(argi == view->drawhandles)
		need_redraw = AY_FALSE;
	      view->drawhandles = argi;
	    }
	  break;
	case 'f':
	  if(!strcmp(argv[i], "-fromx"))
	    {
	      view->from[0] = argd;
	      view->aligned = AY_FALSE;
	      need_updatemark = AY_TRUE;
	    }
	  if(!strcmp(argv[i], "-fromy"))
	    {
	      view->from[1] = argd;
	      view->aligned = AY_FALSE;
	      need_updatemark = AY_TRUE;
	    }
	  if(!strcmp(argv[i], "-fromz"))
	    {
	      view->from[2] = argd;
	      view->aligned = AY_FALSE;
	      need_updatemark = AY_TRUE;
	    }
	  if(!strcmp(argv[i], "-fovx"))
	    {
	      view->zoom = fabs(tan(AY_D2R(argd/2.0)));
	      view->drawmark = AY_FALSE;
	      view->aligned = AY_FALSE;
	    }
	  break;
	case 'g':
	  /*if(!strcmp(argv[i], "-grid"))*/
	    {
	      if(argd >= 0.0)
		{
		  view->grid = argd;
		}
	      else
		{
		  view->grid = 0.0;
		} /* if */
	    }
	  break;
	case 'i':
	  /*if(!strcmp(argv[i], "-ico"))*/
	    {
	      view->isicon = argi;
	    }
	  break;
	case 'l':
	  /*if(!strcmp(argv[i], "-local"))*/
	    {
	      view->local = argi;
	      view->drawmark = AY_FALSE;
	    }
	  break;
	case 'm':
	  /*if(!strcmp(argv[i], "-mark"))*/
	    {
	      if(argv[i+1][0] == 'n')
		{
		  if(!view->drawmark)
		    {
		      need_redraw = AY_FALSE;
		    }

		  view->drawmark = AY_FALSE;

		  if(ay_prefs.globalmark)
		    {
		      ay_viewt_updateglobalmark(togl);
		    }
		}
	      else
		{
		  Tcl_GetDouble(interp, argv[i+1], &view->markx);
		  Tcl_GetDouble(interp, argv[i+2], &view->marky);
		  sel = ay_selection;
		  if(sel)
		    {
		      minlevelscale = ay_pact_getminlevelscale();
		    }
		  while(sel)
		    {
		      o = sel->object;
		      ay_viewt_wintoobj(togl, o,
					view->markx, view->marky,
					&(t[0]), &(t[1]), &(t[2]));

		      ay_status = ay_pact_pickpoint(o, view, minlevelscale,
						    t, &pe);
		      if(!ay_status && pe.coords)
			{
			  break;
			}
		      sel = sel->next;
		    }
		  if(pe.coords)
		    {
		      memcpy(t, pe.coords[0], 3*sizeof(double));

		      /* to world */
		      ay_trafo_identitymatrix(mm);
		      ay_trafo_getall(ay_currentlevel, o, mm);

		      ay_trafo_apply3(t, mm);
		      view->markworld[0] = t[0];
		      view->markworld[1] = t[1];
		      view->markworld[2] = t[2];
		    }
		  else
		    {
		      if(view->usegrid)
			ay_viewt_griddify(togl, &view->markx, &view->marky);
		      ay_viewt_wintoworld(togl, view->markx, view->marky,
					  &(temp[0]),
					  &(temp[1]),
					  &(temp[2]));

		      switch(view->type)
			{
			case AY_VTFRONT:
			case AY_VTTRIM:
			  view->markworld[0] = temp[0];
			  view->markworld[1] = temp[1];
			  if(!ay_prefs.globalmark)
			    view->markworld[2] = 0.0;
			  break;
			case AY_VTSIDE:
			  if(!ay_prefs.globalmark)
			    view->markworld[0] = 0.0;
			  view->markworld[1] = temp[1];
			  view->markworld[2] = temp[2];
			  break;
			case AY_VTTOP:
			  view->markworld[0] = temp[0];
			  if(!ay_prefs.globalmark)
			    view->markworld[1] = 0.0;
			  view->markworld[2] = temp[2];
			  break;
			default:
			  /* XXXX output proper error message */
			  break;
			} /* switch */
		    } /* if */

		  view->drawmark = AY_TRUE;

		  ay_pact_clearpointedit(&pe);

		  if(view->drawmark || ay_prefs.globalmark)
		    {
		      ay_viewt_updatemark(togl, /*local=*/AY_FALSE);
		      need_updatemark = AY_FALSE;
		    }
		} /* if */
	    } /* if */
	  break;
	case 'n':
	  /*if(!strcmp(argv[i], "-name"))*/
	    {
	      o = ay_root->down;
	      while(o)
		{
		  if(o->refine == view)
		    {
		      if(o->name)
			free(o->name);
		      o->name = NULL;
		      if(!(o->name = calloc(strlen(argv[i+1]) + 1,
					    sizeof(char))))
			{
			  ay_error(AY_EOMEM, fname, NULL);
			  return TCL_OK;
			}

		      strcpy(o->name, argv[i+1]);
		    } /* if */
		  o = o->next;
		} /* while */
	    } /* if */
	  break;
	case 'p':
	  if(!strcmp(argv[i], "-panm"))
	    {
	      if(view->drawmark)
		{
		  tclargv[2] = arg2;
		  tclargv[3] = arg3;
		  tclargv[4] = arg3;
		  ay_vact_movetcb(togl, 5, tclargv);
		  tclargv[2] = arg4;
		  sprintf(buf1, "%g", width/2.0-view->markx);
		  tclargv[3] = buf1;
		  sprintf(buf2, "%g", height/2.0-view->marky);
		  tclargv[4] = buf2;
		  ay_vact_movetcb(togl, 5, tclargv);
		  need_updatemark = AY_TRUE;
		}
	    }
	  if(!strcmp(argv[i], "-panx"))
	    {
	      tclargv[2] = arg2;
	      tclargv[3] = arg3;
	      tclargv[4] = arg3;
	      ay_vact_movetcb(togl, 5, tclargv);
	      tclargv[2] = arg4;
	      tclargv[3] = argv[i+1];
	      tclargv[4] = arg3;
	      ay_vact_movetcb(togl, 5, tclargv);
	      need_updatemark = AY_TRUE;
	    }
	  if(!strcmp(argv[i], "-pany"))
	    {
	      tclargv[2] = arg2;
	      tclargv[3] = arg3;
	      tclargv[4] = arg3;
	      ay_vact_movetcb(togl, 5, tclargv);
	      tclargv[2] = arg4;
	      tclargv[3] = arg3;
	      tclargv[4] = argv[i+1];
	      ay_vact_movetcb(togl, 5, tclargv);
	      need_updatemark = AY_TRUE;
	    }
	  if(!strcmp(argv[i], "-pnts"))
	    {
	      view->transform_points = argi;
	    }
	  if(!strcmp(argv[i], "-pos"))
	    {
	      Tcl_GetInt(interp, argv[i+1], &view->pos_x);
	      Tcl_GetInt(interp, argv[i+2], &view->pos_y);
	    }
#ifdef AYENABLEPPREV
	  if(!strcmp(argv[i], "-pprev"))
	    {
	      if(argi)
		ay_wrib_pprevopen(view);
	      else
		ay_wrib_pprevclose();
	    }
#endif
	  break;
	case 'r':
	  if(!strcmp(argv[i], "-roll"))
	    {
	      view->roll = argd;
	      view->aligned = AY_FALSE;
	      need_updatemark = AY_TRUE;
	    }
	  if(!strcmp(argv[i], "-rect"))
	    {
	      old_rect_xmin = view->rect_xmin;
	      old_rect_ymin = view->rect_ymin;
	      old_rect_xmax = view->rect_xmax;
	      old_rect_ymax = view->rect_ymax;

	      Tcl_GetDouble(interp, argv[i+1], &view->rect_xmin);
	      Tcl_GetDouble(interp, argv[i+2], &view->rect_ymin);
	      Tcl_GetDouble(interp, argv[i+3], &view->rect_xmax);
	      Tcl_GetDouble(interp, argv[i+4], &view->rect_ymax);

	      Tcl_GetInt(interp, argv[i+5], &view->drawrect);
	      need_redraw = AY_FALSE;
	      /*XXXX DBG*/
/*#undef GL_VERSION_1_1*/
#ifdef GL_VERSION_1_1
	      glDrawBuffer(GL_FRONT);
	      glEnable(GL_COLOR_LOGIC_OP);
	      glLogicOp(GL_XOR);
	      /*XXXX DBG*/
	      /*glLogicOp(GL_COPY);*/

	       glColor3f((GLfloat)ay_prefs.sxr, (GLfloat)ay_prefs.sxg,
			 (GLfloat)ay_prefs.sxb);

	       /* clear old rectangle? */
	       if((old_rect_xmin != 0.0) && (old_rect_ymin != 0.0) &&
		  (old_rect_xmax != 0.0) && (old_rect_ymax != 0.0))
		 {

		   ay_draw_rectangle(width, height,
				     old_rect_xmin, old_rect_ymin,
				     old_rect_xmax, old_rect_ymax);

		 }

	       if(view->drawrect)
		 {
		   ay_draw_rectangle(width, height,
				     view->rect_xmin, view->rect_ymin,
				     view->rect_xmax, view->rect_ymax);
		 }
	       else
		 {
		   view->rect_xmin = 0.0;
		   view->rect_ymin = 0.0;
		   view->rect_xmax = 0.0;
		   view->rect_ymax = 0.0;
		 } /* if */

	      /* the following line fixes problems with Intel
		 onboard graphics (i915) */
	      glLogicOp(GL_COPY);
	      glDisable(GL_COLOR_LOGIC_OP);
	      glFlush();
	      glDrawBuffer(GL_BACK);
#else
	      ay_toglcb_display(togl);
	      if(view->drawrect)
		{
		  glDrawBuffer(GL_FRONT);
		  glColor3f((GLfloat)ay_prefs.tpr, (GLfloat)ay_prefs.tpg,
			 (GLfloat)ay_prefs.tpb);
		  ay_draw_rectangle(width, height,
				    view->rect_xmin, view->rect_ymin,
				    view->rect_xmax, view->rect_ymax);

		  glDrawBuffer(GL_BACK);
		} /* if */
#endif
	    } /* if */
	  if(!strcmp(argv[i], "-redraw"))
	    {
	      need_redraw = argi;
	    }
	  break;
	case 's':
	  if(!strcmp(argv[i], "-shade"))
	    {
	      view->drawmode = argi;
	    }
	  if(!strcmp(argv[i], "-smark"))
	    {
	      Tcl_GetDouble(interp, argv[i+1], &(view->markworld[0]));
	      Tcl_GetDouble(interp, argv[i+2], &(view->markworld[1]));
	      Tcl_GetDouble(interp, argv[i+3], &(view->markworld[2]));
	      view->drawmark = AY_TRUE;
	      need_updatemark = AY_TRUE;
	    }
	  break;
	case 't':
	  if(!strcmp(argv[i], "-tox"))
	    {
	      view->to[0] = argd;
	      view->aligned = AY_FALSE;
	      need_updatemark = AY_TRUE;
	    }
	  if(!strcmp(argv[i], "-toy"))
	    {
	      view->to[1] = argd;
	      view->aligned = AY_FALSE;
	      need_updatemark = AY_TRUE;
	    }
	  if(!strcmp(argv[i], "-toz"))
	    {
	      view->to[2] = argd;
	      view->aligned = AY_FALSE;
	      need_updatemark = AY_TRUE;
	    }
	  if(!strcmp(argv[i], "-type"))
	    {
	      if(argi != view->type)
		{
		  ay_viewt_changetype(view, argi);
		}
	      /* this avoids havoc for repeated calls */
	      view->rotx = 0.0;
	      view->roty = 0.0;
	      view->rotz = 0.0;
	    }
	  break;
	case 'u':
	  if(!strcmp(argv[i], "-undokb"))
	    {
	      if(view->enable_undo)
		{
		  /* save current state of view, but only once per
		     keypress-keyrelease-sequence */
		  if(!kbdact_in_progress)
		    {
		      /* undo save */
		      tclargv[0] = arg0;
		      tclargv[1] = arg1;
		      tclargv[2] = argv[i+1];
		      (void)ay_undo_undotcmd(NULL, ay_interp, 3, tclargv);
		    }
		}
	    }
	  if(!strcmp(argv[i], "-upx"))
	    {
	      view->up[0] = argd;
	      view->aligned = AY_FALSE;
	      need_updatemark = AY_TRUE;
	    }
	  if(!strcmp(argv[i], "-upy"))
	    {
	      view->up[1] = argd;
	      view->aligned = AY_FALSE;
	      need_updatemark = AY_TRUE;
	    }
	  if(!strcmp(argv[i], "-upz"))
	    {
	      view->up[2] = argd;
	      view->aligned = AY_FALSE;
	      need_updatemark = AY_TRUE;
	    }
	  if(!strcmp(argv[i], "-updroty"))
	    {
	      /* reset rotx,rotz; recalc roty */
	      view->rotx = 0.0;
	      view->roty = 0.0;
	      view->rotz = 0.0;
	      temp[0] = view->to[0] - view->from[0];
	      temp[1] = view->to[2] - view->from[2];
	      view->roty = 180.0 + AY_R2D(atan2(temp[0], temp[1]));
	      view->drawmark = AY_FALSE;
	      view->aligned = AY_FALSE;
	    }
	  if(!strcmp(argv[i], "-ugrid"))
	    {
	      view->usegrid = argi;
	    }
	  break;
	case 'z':
	  if(!strcmp(argv[i], "-zoom"))
	    {
	      view->zoom = argd;
	      need_updatemark = AY_TRUE;
	    }
	  if(!strcmp(argv[i], "-zrect"))
	    {
	      if(view->rect_xmin < view->rect_xmax)
		{
		  rectw = (int)(view->rect_xmax-view->rect_xmin);
		  dispx = (int)((width/2)-(view->rect_xmax-(rectw/2)));
		}
	      else
		{
		  rectw = (int)(view->rect_xmin-view->rect_xmax);
		  dispx = (int)((width/2)-(view->rect_xmin-(rectw/2)));
		}
	      if(view->rect_ymin < view->rect_ymax)
		{
		  recth = (int)(view->rect_ymax-view->rect_ymin);
		  dispy = (int)((height/2)-(view->rect_ymax-(recth/2)));
		}
	      else
		{
		  recth = (int)(view->rect_ymin-view->rect_ymax);
		  dispy = (int)((height/2)-(view->rect_ymin-(recth/2)));
		}

	      if(!(rectw && recth))
		break;

	      dxw = -(dispx*view->conv_x);
	      dyw = (dispy*view->conv_y);
	      t[0] = view->from[0] - view->to[0];
	      t[1] = view->from[1] - view->to[1];
	      t[2] = view->from[2] - view->to[2];
	      glMatrixMode(GL_MODELVIEW);
	      glPushMatrix();
	       glLoadIdentity();
	       /* check, whether we are able to derive camera
		  orientation angles from t */
	       if(!t[0] && !t[2])
		 {
		   /* no, this (hopefully just) happens with Top views, that
		      are Y-Axis aligned and express a useful angle in
		      view->up */

		   /* XXXX this currently only works for real Y-aligned
		      Top views */
		   t[0] = -view->up[0];
		   t[1] = -view->up[1];
		   t[2] = -view->up[2];
		   roty = AY_R2D(atan2(t[0], t[2]));

		   if(fabs(view->roll) > AY_EPSILON)
		     glRotated(-view->roll, 0.0, 1.0, 0.0);

		   glRotated(roty, 0.0, 1.0, 0.0);

		   glTranslated(dxw, 0.0, -dyw);

		   glRotated(-roty, 0.0, 1.0, 0.0);

		   if(fabs(view->roll) > AY_EPSILON)
		     glRotated(view->roll, 0.0, 1.0, 0.0);
		 }
	       else
		 {
		   /* yes, derive angles from t */
		   roty = AY_R2D(atan2(t[0], t[2]));

		   t2[0] = t[0];
		   t2[1] = t[2];
		   if(t[0] && t[2])
		     rotx = AY_R2D(atan2(t[1], AY_V2LEN(t2)));

		   if(fabs(view->roll) > AY_EPSILON)
		     glRotated(-view->roll, 0.0, 0.0, 1.0);

		   glRotated(roty, 0.0, 1.0, 0.0);
		   glRotated(-rotx, 1.0, 0.0, 0.0);

		   if(view->up[1] > 0.0)
		     glTranslated(dxw, dyw, 0.0);
		   else
		     glTranslated(-dxw, -dyw, 0.0);

		   glRotated(rotx, 1.0, 0.0, 0.0);
		   glRotated(-roty, 0.0, 1.0, 0.0);

		   if(fabs(view->roll) > AY_EPSILON)
		     glRotated(view->roll, 0.0, 0.0, 1.0);
		 }
	       glGetDoublev(GL_MODELVIEW_MATRIX, mm);
	      glPopMatrix();

	      ay_trafo_apply3(view->from, mm);
	      ay_trafo_apply3(view->to, mm);
	      if(rectw > recth)
		{
		  if(width != rectw)
		    view->zoom /= (width/rectw);
		}
	      else
		{
		  if(height != recth)
		    view->zoom /= (height/recth);
		}
	      view->drawmark = AY_FALSE;
	    }
	  break;
	default:
	  break;
	} /* switch */
      i += 2;
  } /* while */

  ay_toglcb_reshape(togl);

  if((view->drawmark && need_updatemark) ||
     (need_updatemark && ay_prefs.globalmark))
    {
      ay_viewt_updatemark(togl, /*local=*/AY_TRUE);
    }

  if(need_redraw)
    {
      ay_toglcb_display(togl);
    }

  view->full_notify = AY_FALSE;
  ay_viewt_uprop(view, AY_TRUE);

  /* save point in time where we return control to Tcl; we use this
     to detect whether the user holds down a key if we are called
     in the future again */
#ifndef AYNOUNISTDH
  t_lastcalled = times(&tbuf);
#endif
#ifdef WIN32
  t_lastcalled = GetTickCount();
#endif

 return TCL_OK;
} /* ay_viewt_setconftcb */


/* ay_viewt_updatemark:
 *  calculate new mark window coordinates, if <local> is true, do it
 *  just for the view <togl>, otherwise and if also the globalmark
 *  preference is enabled, also update all other views;
 *  also updates the global Tcl array "aymark", so that scripts
 *  may access the new mark coordinates
 */
int
ay_viewt_updatemark(struct Togl *togl, int local)
{
 ay_view_object *view = (ay_view_object *)Togl_GetClientData(togl);
 double dummy, mm[16], pm[16];
 int i, vp[4], height = Togl_Height(togl);
 GLint gl_status = GL_TRUE;
 Tcl_Obj *to = NULL;
 Tcl_Interp *interp = ay_interp;

  glFlush();

  glGetIntegerv(GL_VIEWPORT, vp);

  glGetDoublev(GL_PROJECTION_MATRIX, pm);

  glGetDoublev(GL_MODELVIEW_MATRIX, mm);

  switch(view->type)
    {
    case AY_VTFRONT:
    case AY_VTTRIM:
      gl_status = gluProject(view->markworld[0], view->markworld[1], 0,
			     mm, pm, vp,
		 &view->markx, &view->marky, &dummy);
      break;
    case AY_VTSIDE:
      gl_status = gluProject(0, view->markworld[1], view->markworld[2],
			     mm, pm, vp,
		 &view->markx, &view->marky, &dummy);
      break;
    case AY_VTTOP:
      gl_status = gluProject(view->markworld[0], 0, view->markworld[2],
			     mm, pm, vp,
		 &view->markx, &view->marky, &dummy);
      break;
    case AY_VTPERSP:
      gl_status = gluProject(view->markworld[0],
			     view->markworld[1],
			     view->markworld[2],
			     mm, pm, vp,
			     &view->markx, &view->marky, &dummy);
      break;
    default:
      view->drawmark = AY_FALSE;
      break;
    }

  if(gl_status == GL_FALSE)
    {
      view->markx = 0;
      view->marky = 0;
      view->drawmark = AY_FALSE;
      return AY_ERROR;
    }

  view->marky = height - view->marky;

  if(!local && ay_prefs.globalmark)
    {
      ay_viewt_updateglobalmark(togl);
    }

  for(i = 0; i < 3; i++)
    {
      to = Tcl_NewDoubleObj(view->markworld[i]);
      Tcl_ObjSetVar2(interp, aymarkobj, xyzobj[i], to, TCL_LEAVE_ERR_MSG |
		     TCL_GLOBAL_ONLY);
    }

 return AY_OK;
} /* ay_viewt_updatemark */


/** ay_viewt_updateglobalmark:
 *  manage the global mark after change in view \a togl
 *  (copy the mark from the view to all other views)
 */
void
ay_viewt_updateglobalmark(struct Togl *togl)
{
 ay_view_object *view = (ay_view_object *)Togl_GetClientData(togl);
 ay_object *o = ay_root->down;
 ay_view_object *v = NULL;

  while(o && o->next)
    {
      if(o->type == AY_IDVIEW)
	{
	  v = (ay_view_object *)o->refine;
	  if(v != view)
	    {
	      memcpy(v->markworld, view->markworld, 3*sizeof(double));
	      v->drawmark = view->drawmark;
	      Togl_MakeCurrent(v->togl);
	      ay_viewt_updatemark(v->togl, /*local=*/AY_TRUE);
	      ay_toglcb_display(v->togl);
	    }
	}
      o = o->next;
    } /* while */

  Togl_MakeCurrent(togl);

 return;
} /* ay_viewt_updateglobalmark */


/** ay_viewt_getglobalmark:
 *  get the global mark
 *
 * \param[in,out] m pointer where to store the address of the mark
 */
void
ay_viewt_getglobalmark(double **m)
{
 ay_object *o = ay_root->down;
 ay_view_object *v = NULL;

  while(o && o->next)
    {
      if(o->type == AY_IDVIEW)
	{
	  v = (ay_view_object *)o->refine;
	  *m = v->markworld;
	  break;
	}
      o = o->next;
    } /* while */

 return;
} /* ay_viewt_getglobalmark */


/* ay_viewt_fromcamtcb:
 *  copy from/to/up from the selected camera object to view <togl>
 */
int
ay_viewt_fromcamtcb(struct Togl *togl, int argc, char *argv[])
{
 ay_view_object *view = (ay_view_object *)Togl_GetClientData(togl);
 ay_list_object *sel = ay_selection;
 ay_object *o = NULL;
 ay_camera_object *c = NULL;
 char fname[] = "fromcamera";

  if(!sel)
    {
      ay_error(AY_ENOSEL,fname,NULL);
      return TCL_OK;
    }
  o = sel->object;
  if(o->type != AY_IDCAMERA)
    {
      ay_error(AY_ERROR,fname, "Please select a camera!");
      return TCL_OK;
    }
  c = (ay_camera_object *)o->refine;

  memcpy(view->from, c->from, 3*sizeof(double));
  memcpy(view->to, c->to, 3*sizeof(double));
  memcpy(view->up, c->up, 3*sizeof(double));
  view->roll = c->roll;
  view->zoom = c->zoom;

  ay_toglcb_reshape(togl);
  ay_toglcb_display(togl);

  view->full_notify = AY_FALSE;
  ay_viewt_uprop(view, AY_TRUE);

  view->drawmark = AY_FALSE;
  view->aligned = AY_FALSE;

 return TCL_OK;
} /* ay_viewt_fromcamtcb */


/* ay_viewt_tocamtcb:
 *  copy camera settings of view <togl> to the selected camera object
 */
int
ay_viewt_tocamtcb(struct Togl *togl, int argc, char *argv[])
{
 ay_view_object *view = (ay_view_object *)Togl_GetClientData(togl);
 ay_list_object *sel = ay_selection;
 ay_object *o = NULL;
 ay_camera_object *c = NULL;
 char fname[] = "tocamera";

  if(!sel)
    {
      ay_error(AY_ENOSEL,fname,NULL);
      return TCL_OK;
    }
  o = sel->object;
  if(o->type != AY_IDCAMERA)
    {
      ay_error(AY_ERROR,fname, "Please select a camera!");
      return TCL_OK;
    }
  c = (ay_camera_object *)o->refine;

  memcpy(c->from, view->from, 3*sizeof(double));
  memcpy(c->to, view->to, 3*sizeof(double));
  memcpy(c->up, view->up, 3*sizeof(double));
  c->roll = view->roll;
  c->zoom = view->zoom;

 return TCL_OK;
} /* ay_viewt_tocamtcb */


/* ay_viewt_uprop:
 *  update view property
 */
void
ay_viewt_uprop(ay_view_object *view, int notify)
{
 Tcl_Interp *interp = ay_interp;
 ay_list_object *sel = ay_selection;
 ay_object *o = NULL;
 ay_view_object *sview = NULL;

  if(sel)
    {
      if(!sel->next)
	{
	  o = sel->object;
	  if(o->type == AY_IDVIEW)
	    {
	      sview = (ay_view_object *)(o->refine);
	      if(sview == view)
		{
		  ay_prop_gettcmd(NULL, interp, 0, NULL);
		  if(notify)
		    {
		      ay_notify_object(o);
		    }
		} /* if */
	    } /* if */
	} /* if */
    }
  else
    {
      if(notify)
	{
	  o = ay_root->down;
	  while(o && o->next)
	    {
	      if(o->type == AY_IDVIEW)
		{
		  sview = (ay_view_object *)(o->refine);
		  if(sview == view)
		    {
		      ay_notify_object(o);
		    }
		}
	      o = o->next;
	    }
	} /* if */
    } /* if */

 return;
} /* ay_viewt_uprop */


/* ay_viewt_griddify:
 *  snap winx and winy to current grid
 */
int
ay_viewt_griddify(struct Togl *togl, double *winx, double *winy)
{
 ay_view_object *view = (ay_view_object *)Togl_GetClientData(togl);
 ay_object *o = NULL;
 int width = Togl_Width(togl);
 int height = Togl_Height(togl);
 double refx = 0.0, refy = 0.0, refz = 0.0;
 double gdx = 0.0, gdy = 0.0;
 double gridx = view->grid/view->conv_x,
   gridy = view->grid/view->conv_y, gridz = 0.0;
 double ghx = view->grid/view->conv_x/2.0,
   ghy = view->grid/view->conv_y/2.0;
 double m[16] = {0};
 GLdouble mp[16], mm[16];
 GLint vp[4], gl_status = GL_TRUE;

  if(view->grid != 0.0)
    {

      if(!view->local)
	{
	  /* get reference point in window coords */
	  switch(view->type)
	    {
	    case AY_VTFRONT:
	    case AY_VTTRIM:
	      refx = (width/2.0)-(view->from[0]/view->conv_x);
	      refy = (height/2.0)+(view->from[1]/view->conv_y);
	      break;
	    case AY_VTSIDE:
	      refx = (width/2.0)+(view->from[2]/view->conv_x);
	      refy = (height/2.0)+(view->from[1]/view->conv_y);
	      break;
	    case AY_VTTOP:
	      refx = (width/2.0)-(view->from[0]/view->conv_x);
	      refy = (height/2.0)-(view->from[2]/view->conv_y);
	      break;
	    }
	}
      else
	{
	  /* get reference point and grid size in window coords */
	  glGetIntegerv(GL_VIEWPORT, vp);
	  glGetDoublev(GL_PROJECTION_MATRIX, mp);

	  if(ay_currentlevel->object != ay_root)
	    {
	      ay_trafo_concatparent(ay_currentlevel->next);
	    }

	   if(view->aligned && ay_selection)
	     {
	       o = ay_selection->object;

	       ay_trafo_creatematrix(o, m);
	       ay_trafo_multmatrix(mm, m);
	     } /* if */

	  if(GL_FALSE == gluProject(0.0, 0.0, 0.0, mm, mp, vp,
				    &refx, &refy, &refz))
	    {
	      return AY_ERROR;
	    }

	  refy = height-refy;
	  switch(view->type)
	    {
	    case AY_VTFRONT:
	    case AY_VTTRIM:
	      gl_status = gluProject(view->grid, view->grid, 0.0, mm, mp, vp,
				     &gridx, &gridy, &gridz);
	      gridy = height-gridy;
	      break;
	    case AY_VTSIDE:
	      gl_status = gluProject(0.0, view->grid, -view->grid, mm, mp, vp,
				     &gridx, &gridy, &gridz);
	      gridy = height-gridy;
	      break;
	    case AY_VTTOP:
	      gl_status = gluProject(view->grid, 0.0, view->grid, mm, mp, vp,
				     &gridx, &gridy, &gridz);
	      break;
	    }
	  if(gl_status == GL_FALSE)
	    {
	      return AY_ERROR;
	    }
	  gridx = gridx-refx;
	  gridy = -(gridy-refy);

	  ghx = gridx/2.0;
	  ghy = gridy/2.0;
	} /* if */

      /* from grid reference, grid size and current window coordinates
	 compute new window coordinates, that are snapped to the grid;
         snapping to new coordinate values occurs halfway between two
	 grid corners (thus, the use of ghx/ghy) */
      gdx = fmod(*winx-refx, gridx);
      gdy = fmod(*winy-refy, gridy);

      if(*winx>refx)
	{
	  if(gdx >= ghx)
	    *winx += (gridx-gdx);
	  else
	    *winx -= gdx;
	}
      else
	{
	  if(gdx <= -ghx)
	    *winx -= (gridx+gdx);
	  else
	    *winx -= gdx;
	}

      if(*winy>refy)
	{
	  if(gdy >= ghy)
	    *winy += (gridy-gdy);
	  else
	    *winy -= gdy;
	}
      else
	{
	  if(gdy <= -ghy)
	    *winy -= (gridy+gdy);
	  else
	    *winy -= gdy;
	} /* if */

    } /* if */

 return AY_OK;
} /* ay_viewt_griddify */


/* ay_viewt_droptcb:
 *  an object has been dropped onto a view window
 */
int
ay_viewt_droptcb(struct Togl *togl, int argc, char *argv[])
{
 int ay_status = AY_OK;
 char fname[] = "droptcb";
 ay_view_object *view = (ay_view_object *)Togl_GetClientData(togl);
 ay_list_object *sel = ay_selection;
 ay_object *o, *v;
 ay_voidfp *arr = NULL;
 ay_treedropcb *cb = NULL;

  if(!sel)
    return TCL_OK; /* Oops? Should not there be atleast one selected object? */

  o = sel->object;

  switch(o->type)
    {
    case AY_IDLIGHT:
    case AY_IDCAMERA:
      arr = ay_treedropcbt.arr;
      cb = (ay_treedropcb *)(arr[AY_IDVIEW]);
      v = ay_root->down;
      while(v)
	{
	  if(v->type == AY_IDVIEW)
	    if(v->refine == view)
	      break;
	  v = v->next;
	}
      if(v && (v->type == AY_IDVIEW) && (v->refine == view))
	{
	  /* call drop callback */
	  if(cb)
	    {
	      ay_status = cb(v);
	      if((ay_status != AY_OK) && (ay_status != AY_EDONOTLINK))
		{
		  ay_error(AY_ERROR, fname, "Drop callback failed!");
		}
	    }
	}
      else
	{
	  ay_error(AY_ERROR, fname, "Could not find view object!");
	}
      break;
    default:
      ay_viewt_zoomtoobj(togl, argc, argv);
      break;
    } /* switch */

 return TCL_OK;
} /* ay_viewt_droptcb */


/* ay_viewt_setupintview:
 *  set up the view window <o> from data in <vtemp>
 *  (while reading an internal view from a scene file)
 */
void
ay_viewt_setupintview(int viewnum, ay_object *o, ay_view_object *vtemp)
{
 struct Togl *togl;
 Togl_Callback *altdispcb;
 ay_view_object *view = NULL;
 char command[255] = {0}, update_cmd[] = "update";
 int vnum = viewnum-1;

  if(!o || !vtemp)
    return;

  if(o->type == AY_IDVIEW)
    view = (ay_view_object *)o->refine;
  else
    return;

  /* protect togl and altdispcb pointers from the memcpy below */
  togl = view->togl;
  altdispcb = view->altdispcb;
  if(view->bgimage)
    free(view->bgimage);
  memcpy(view, vtemp, sizeof(ay_view_object));
  view->togl = togl;
  view->altdispcb = altdispcb;

  view->bgimage = NULL;
  if(vtemp->bgimage)
    {
      if((view->bgimage = malloc((strlen(vtemp->bgimage)+1)*sizeof(char))))
	{
	  strcpy(view->bgimage, vtemp->bgimage);
	  view->bgimagedirty = AY_TRUE;
	}
    }

  /* notify also includes reshape() and additionally loads the BGImage */
  (void)ay_notify_object(o);

  (void)ay_viewt_makecurtcb(view->togl, 0, NULL);

  /* set various view menu icons */
  sprintf(command,
	  "global ay;viewSetGridIcon [lindex $ay(views) %d] %g\n",
	  vnum, vtemp->grid);

  Tcl_Eval(ay_interp, command);

  sprintf(command,
	  "global ay;viewSetDModeIcon [lindex $ay(views) %d] %d\n",
	  vnum, vtemp->drawmode);

  Tcl_Eval(ay_interp, command);

  sprintf(command,
	  "global ay;viewSetMModeIcon [lindex $ay(views) %d] %d\n",
	  vnum, vtemp->local);

  Tcl_Eval(ay_interp, command);

  Tcl_Eval(ay_interp, update_cmd);

 return;
} /* ay_viewt_setupintview */


/* ay_viewt_markfromsel:
 *  set mark from selected objects cog
 */
int
ay_viewt_markfromsel(struct Togl *togl)
{
 ay_view_object *view = (ay_view_object *)Togl_GetClientData(togl);
 ay_list_object *sel = NULL;
 ay_object *o = NULL;
 int height = Togl_Height(togl);
 double *cogs = NULL, cog[3] = {0};
 GLint vp[4];
 GLdouble mm[16], mp[16], winx, winy, winz;
 int i = 0, a = 0, numo = 0, numu;

  sel = ay_selection;

  if(!sel)
    return AY_ENOSEL;

  while(sel)
    {
      numo++;
      sel = sel->next;
    } /* while */

  if(!(cogs = calloc(numo, 3*sizeof(double))))
    return AY_EOMEM;

  sel = ay_selection;
  while(sel)
    {
      o = sel->object;

      cogs[a]   = o->movx;
      cogs[a+1] = o->movy;
      cogs[a+2] = o->movz;

      a += 3;

      sel = sel->next;
    } /* while */

  if(numo > 1)
    {
      qsort(cogs, numo, 3*sizeof(double), ay_nct_cmppnt);

      numu = numo;
      a = 0;
      for(i = 0; i < numo-1; i++)
	{
	  if(!ay_nct_cmppnt(&(cogs[a]), &(cogs[a+3])))
	    {
	      numu--;
	    }
	  a += 3;
	}
      a = 0;
      /* for the special case of two equal cogs, make
	 sure we have the first of the cogs in cog */
      cog[0] = cogs[a]  /(double)numu;
      cog[1] = cogs[a+1]/(double)numu;
      cog[2] = cogs[a+2]/(double)numu;
      for(i = 1; i < numo; i++)
	{
	  if(ay_nct_cmppnt(&(cogs[a]), &(cogs[a+3])))
	    {
	      cog[0] += cogs[a+3]/(double)numu;
	      cog[1] += cogs[a+4]/(double)numu;
	      cog[2] += cogs[a+5]/(double)numu;
	    }
	  a += 3;
	}
    }
  else
    {
      memcpy(cog, cogs, 3*sizeof(double));
    }

  free(cogs);

  glGetIntegerv(GL_VIEWPORT, vp);

  glGetDoublev(GL_PROJECTION_MATRIX, mp);

  ay_trafo_identitymatrix(mm);
  if(ay_currentlevel->object != ay_root)
    {
      ay_trafo_getparent(ay_currentlevel->next, mm);
    }

  if(GL_FALSE == gluProject(cog[0],cog[1],cog[2],mm,mp,vp,&winx,&winy,&winz))
    {
      return AY_ERROR;
    }

  view->markx = winx;
  view->marky = height-winy;

  AY_APTRAN3(view->markworld, cog, mm);

  view->drawmark = AY_TRUE;

  if(ay_prefs.globalmark)
    {
      ay_viewt_updateglobalmark(togl);
    }

 return AY_OK;
} /* ay_viewt_markfromsel */


/* ay_viewt_markfromselp:
 *  set mark from selected points cog
 */
int
ay_viewt_markfromselp(struct Togl *togl, int mode)
{
 ay_view_object *view = (ay_view_object *)Togl_GetClientData(togl);
 ay_list_object *sel = NULL;
 ay_object *o = NULL;
 int height = Togl_Height(togl);
 double ttcog[3] = {0}, tcog[3] = {0}, cog[3] = {0};
 GLint vp[4];
 GLdouble mm[16], mp[16], winx, winy, winz;
 unsigned int numo = 0;

  sel = ay_selection;
  while(sel)
    {
      o = sel->object;
      if(o->selp)
	{
	  numo++;
	}
      sel = sel->next;
    }

  if(numo == 0)
    return AY_ERROR;

  sel = ay_selection;
  while(sel)
    {
      o = sel->object;
      if(o->selp)
	{
	  (void)ay_selp_getcenter(o->selp, mode, tcog);
	  if(AY_ISTRAFO(o))
	    {
	      ay_trafo_creatematrix(o, mm);
	      AY_APTRAN3(ttcog, tcog, mm);

	      cog[0] += ttcog[0]/(double)numo;
	      cog[1] += ttcog[1]/(double)numo;
	      cog[2] += ttcog[2]/(double)numo;
	    }
	  else
	    {
	      cog[0] += tcog[0]/(double)numo;
	      cog[1] += tcog[1]/(double)numo;
	      cog[2] += tcog[2]/(double)numo;
	    }

	} /* if */
      sel = sel->next;
    } /* while */

  glGetIntegerv(GL_VIEWPORT, vp);

  glGetDoublev(GL_PROJECTION_MATRIX, mp);

  ay_trafo_identitymatrix(mm);
  if(ay_currentlevel->object != ay_root)
    {
      ay_trafo_getparent(ay_currentlevel->next, mm);
    }

  if(GL_FALSE == gluProject(cog[0], cog[1], cog[2], mm, mp, vp,
			    &winx, &winy, &winz))
    {
      return AY_ERROR;
    }

  view->markx = winx;
  view->marky = height-winy;

  AY_APTRAN3(view->markworld, cog, mm);

  view->drawmark = AY_TRUE;

  if(ay_prefs.globalmark)
    {
      ay_viewt_updateglobalmark(togl);
    }

 return AY_OK;
} /* ay_viewt_markfromselp */


/* ay_viewt_saveimgtcb:
 *
 */
int
ay_viewt_saveimgtcb(struct Togl *togl, int argc, char *argv[])
{
 char fname[] = "saveimg", fargs[] = "filename";
 TIFF *tif;
 unsigned int i, w, h, s = 3;
 unsigned char *pixels;
 int dim[4];

  if(argc <= 1)
    {
      ay_error(AY_EARGS, argv[0], fargs);
      return TCL_OK;
    }

  Togl_MakeCurrent(togl);

  glGetIntegerv(GL_VIEWPORT, dim);

  w = dim[2];
  h = dim[3];

  if(!(pixels = calloc(w*h*s, sizeof(unsigned char))))
    {
      ay_error(AY_EOMEM, fname, NULL);
      return TCL_OK;
    }

  glPixelStorei(GL_PACK_ALIGNMENT, 1);
  glReadBuffer(GL_FRONT);

  /* is the buffer, we are reading here ok? */
  glReadPixels(0, 0, w, h, GL_RGB, GL_UNSIGNED_BYTE, pixels);

  tif = TIFFOpen(argv[2], "w");
  if(tif)
    {
      TIFFSetField(tif, TIFFTAG_IMAGEWIDTH, (uint32) w);
      TIFFSetField(tif, TIFFTAG_IMAGELENGTH, (uint32) h);
      TIFFSetField(tif, TIFFTAG_BITSPERSAMPLE, 8);
      TIFFSetField(tif, TIFFTAG_COMPRESSION, COMPRESSION_NONE);
      TIFFSetField(tif, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_RGB);
      TIFFSetField(tif, TIFFTAG_SAMPLESPERPIXEL, s/*3/4*/);
      TIFFSetField(tif, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
      TIFFSetField(tif, TIFFTAG_ROWSPERSTRIP, 1);
      TIFFSetField(tif, TIFFTAG_IMAGEDESCRIPTION,
		   "Created by Ayam.");

      for(i = 0; i < h; i++)
	{
	  if(TIFFWriteScanline(tif, &(pixels[(h-i-1)*w*s]), i, 0) < 0)
	    {
	      TIFFClose(tif);
	      ay_error(AY_ERROR, fname, "Error writing image file.");
	      goto cleanup;
	    }
	}
      TIFFClose(tif);
    }
  else
    {
      ay_error(AY_ERROR, fname, "Error opening image file.");
      goto cleanup;
    }

cleanup:

  if(pixels)
    free(pixels);

 return TCL_OK;
} /* ay_viewt_saveimgtcb */


/* ay_viewt_warpmouse:
 *  arrange to warp the mouse to new coordinates
 */
void
ay_viewt_warpmouse(struct Togl *togl, double *coord, ay_object *o,
		   double *newwinx, double *newwiny)
{
 double winx, winy, dummy, /*rm[16],*/ mm[16], pm[16];
 int vp[4], height = Togl_Height(togl);
 GLint gl_status = GL_TRUE;
 char *cmd = NULL, warpcmd[] = "warpMouse";

  glGetIntegerv(GL_VIEWPORT, vp);

  glGetDoublev(GL_PROJECTION_MATRIX, pm);

  /* coord is in world space, no need to transform... */
/*
  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glLoadIdentity();

   if(ay_currentlevel->object != ay_root)
     {
       ay_trafo_getall(ay_currentlevel->next);
     }

   glTranslated(o->movx, o->movy, o->movz);
   ay_quat_torotmatrix(o->quat, rm);
   glMultMatrixd(rm);
   glScaled(o->scalx, o->scaly, o->scalz);


   glGetDoublev(GL_MODELVIEW_MATRIX, mm);

  glPopMatrix();
*/

  ay_trafo_identitymatrix(mm);

  gl_status = gluProject(coord[0], coord[1], coord[2],
			 mm, pm, vp,
			 &winx, &winy, &dummy);

  if(gl_status == GL_FALSE)
    {
      return;
    }

  if((cmd = calloc(TCL_DOUBLE_SPACE*2+12, sizeof(char))))
    {
      sprintf(cmd, "%s %lg %lg", warpcmd,
	      winx-*newwinx,
	      (height-winy)-*newwiny);

      Tcl_Eval(ay_interp, cmd);

      *newwinx = winx;
      *newwiny = (height - winy);

      free(cmd);
    }

 return;
} /* ay_viewt_warpmouse */


/* ay_viewt_init:
 *  Initialize this module.
 *
 */
void
ay_viewt_init(void)
{
  ayobj = Tcl_NewStringObj("ay",-1);
  cvredrawobj = Tcl_NewStringObj("cVRedraw",-1);
  cvdmodeobj = Tcl_NewStringObj("cVDMode",-1);
  cvdrawselobj = Tcl_NewStringObj("cVDrawSel",-1);
  cvdrawlevelobj = Tcl_NewStringObj("cVDrawLevel",-1);
  cvdrawocsobj = Tcl_NewStringObj("cVDrawOCS",-1);
  cvaalinesobj = Tcl_NewStringObj("cVAALines",-1);
  cvdrawbgobj = Tcl_NewStringObj("cVDrawBG",-1);
  cvdrawgridobj = Tcl_NewStringObj("cVDrawGrid",-1);
  cvusegridobj = Tcl_NewStringObj("cVUseGrid",-1);
  cvmmodeobj = Tcl_NewStringObj("cVMMode",-1);
  cvgridsizeobj = Tcl_NewStringObj("cVGridSize",-1);
  cvfovobj = Tcl_NewStringObj("cVFOV",-1);
  cvtypeobj = Tcl_NewStringObj("cVType",-1);
  cvbgimageobj = Tcl_NewStringObj("cVBGImage",-1);
  cvfromxobj = Tcl_NewStringObj("cVFromX",-1);
  cvfromyobj = Tcl_NewStringObj("cVFromY",-1);
  cvfromzobj = Tcl_NewStringObj("cVFromZ",-1);
  cvtoxobj = Tcl_NewStringObj("cVToX",-1);
  cvtoyobj = Tcl_NewStringObj("cVToY",-1);
  cvtozobj = Tcl_NewStringObj("cVToZ",-1);
  cvzoomobj = Tcl_NewStringObj("cVZoom",-1);
  cvpntsobj = Tcl_NewStringObj("cVPnts",-1);
  cvdrawmarkobj = Tcl_NewStringObj("cVDrawMark",-1);
  cvundoobj = Tcl_NewStringObj("cVUndo",-1);
  aymarkobj = Tcl_NewStringObj("aymark",-1);
  xyzobj[0] = Tcl_NewStringObj("x",-1);
  xyzobj[1] = Tcl_NewStringObj("y",-1);
  xyzobj[2] = Tcl_NewStringObj("z",-1);

 return;
} /* ay_viewt_init */
