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

/* viewt.c - view tools */

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
 GLfloat light_posf[4] = { 0.0f,  0.0f, 1000.0f, 1.0f};
 GLfloat light_poss[4] = {1000.0f,  0.0f,  0.0f, 1.0f};
 GLfloat light_post[4] = { 0.0f, 1000.0f,  0.0f, 1.0f};
 GLdouble nearp = view->nearp, farp = view->farp;

  glViewport(0, 0, width, height);

  glMatrixMode(GL_PROJECTION);

  glLoadIdentity();

  if(view->type == AY_VTPERSP)
    {
      if(view->nearp <= 0.0)
	{
	  nearp = 1.0;
	}

      if((view->farp == 0.0) || (view->farp <= nearp))
	{
	  farp = 1000.0;
	}

      glFrustum(-aspect*view->zoom, aspect*view->zoom,
		-1.0*view->zoom, 1.0*view->zoom,
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
   /*  glLoadIdentity();*/
   glRotated(view->rotz, 0.0, 0.0, 1.0);
   glRotated(view->roty, 0.0, 1.0, 0.0);
   glRotated(view->rotx, 1.0, 0.0, 0.0);
   if((view->type == AY_VTPERSP) || (view->type == AY_VTFRONT))
     glLightfv(GL_LIGHT0, GL_POSITION, light_posf);
   if(view->type == AY_VTSIDE)
     glLightfv(GL_LIGHT0, GL_POSITION, light_poss);
   if(view->type == AY_VTTOP)
     glLightfv(GL_LIGHT0, GL_POSITION, light_post);
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
     glRotated(roty, 0.0, 1.0, 0.0);


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
int
ay_viewt_wintoobj(struct Togl *togl, ay_object *o,
		  double winX, double winY,
		  double *objX, double *objY, double *objZ)
{
 int ay_status = AY_OK;
 int height = Togl_Height(togl);
 GLint viewport[4];
 GLdouble modelMatrix[16], projMatrix[16], m[16], winx, winy;
 GLfloat winz = 0.0f;

  if(!o)
    return AY_ENULL;

  winx = winX;
  winy = height - winY;

  glReadPixels((GLint)winx,(GLint)winy,1,1,GL_DEPTH_COMPONENT,GL_FLOAT,&winz);

  /* fprintf(stderr,"WinZ:%g\n",winz); */

  glGetIntegerv(GL_VIEWPORT, viewport);

  ay_viewt_setupprojection(togl);

  glGetDoublev(GL_PROJECTION_MATRIX, projMatrix);

  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();

   ay_trafo_getall(ay_currentlevel->next);

   glTranslated(o->movx, o->movy, o->movz);

   ay_quat_torotmatrix(o->quat, m);
   glMultMatrixd(m);

   glScaled(o->scalx, o->scaly, o->scalz);

   glGetDoublev(GL_MODELVIEW_MATRIX, modelMatrix);

   gluUnProject(winx, winy, (GLdouble)winz, modelMatrix, projMatrix, viewport,
		objX, objY, objZ);

   /*  fprintf(stderr,"ObjX:%g; ObjY:%g; ObjZ:%g\n",*objX, *objY, *objZ); */

  glPopMatrix();

 return ay_status;
} /* ay_viewt_wintoobj */


/* ay_viewt_winrecttoobj:
 * transforms the rectangle formed by the window coordinates
 * winX, winY, winX2, winY2, 
 * to object coordinates (obj[24]!) for the given object *o
 * Assumes the standard transformations are
 * in use!
 */
int
ay_viewt_winrecttoobj(struct Togl *togl, ay_object *o,
		      double winX, double winY, double winX2, double winY2,
		      double *obj)
{
 int ay_status = AY_OK;
 int height = Togl_Height(togl), i, j;
 GLint viewport[4];
 GLdouble modelMatrix[16], projMatrix[16], m[16], win[24];

  if(!togl || !o || !obj)
    return AY_ENULL;

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

  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();

   ay_trafo_getall(ay_currentlevel->next);

   glTranslated(o->movx, o->movy, o->movz);

   ay_quat_torotmatrix(o->quat, m);
   glMultMatrixd(m);

   glScaled(o->scalx, o->scaly, o->scalz);

   glGetDoublev(GL_MODELVIEW_MATRIX, modelMatrix);

   for(i = 0; i < 8; i++)
     {
       gluUnProject(win[i*3], win[i*3+1], win[i*3+2],
		    modelMatrix, projMatrix, viewport,
		    &(obj[i*3]), &(obj[i*3+1]), &(obj[i*3+2]));
     }

  glPopMatrix();

 return ay_status;
} /* ay_viewt_winrecttoobj */


/* ay_viewt_wintoworld:
 *  transforms the window coordinates winX winY
 *  to world coordinates
 */
int
ay_viewt_wintoworld(struct Togl *togl, double winX, double winY,
		    double *worldX, double *worldY, double *worldZ)
{
 int ay_status = AY_OK;
 int height = Togl_Height(togl);
 GLint viewport[4];
 GLdouble modelMatrix[16], projMatrix[16], winx, winy;
 GLfloat winz = 0.0f;

  winx = winX;
  winy = height - winY;

  glReadPixels((GLint)winx,(GLint)winy,1,1,GL_DEPTH_COMPONENT,GL_FLOAT,&winz);

  glGetIntegerv(GL_VIEWPORT, viewport);
  ay_viewt_setupprojection(togl);
  glGetDoublev(GL_PROJECTION_MATRIX, projMatrix);

  glMatrixMode (GL_MODELVIEW);
  glPushMatrix();
   glLoadIdentity();
   glGetDoublev(GL_MODELVIEW_MATRIX, modelMatrix);
  glPopMatrix ();

  gluUnProject(winx, winy, (GLdouble)winz, modelMatrix, projMatrix, viewport,
	       worldX, worldY, worldZ);

 return ay_status;
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
 ay_list_object *sel = ay_selection;
 double bb[24] = {0};
 double xmin = DBL_MAX, xmax = -DBL_MAX, ymin = DBL_MAX;
 double ymax = -DBL_MAX, zmin = DBL_MAX, zmax = -DBL_MAX;
 double cog[3] = {0}, dt[3] = {0}, dt2[3] = {0}, l, lx, ly, lz;
 int i, a;
 GLdouble m[16] = {0}, mt[16] = {0};

  if(sel)
    {
      while(sel)
	{
	  o = sel->object;

	  ay_status = ay_bbc_get(o, bb);
	  if(!ay_status)
	    {
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
	    }

	  sel = sel->next;
	} /* while */

      glMatrixMode(GL_MODELVIEW);
      glPushMatrix();
       glLoadIdentity();
       ay_trafo_getall(ay_currentlevel->next);
       glGetDoublev(GL_MODELVIEW_MATRIX, mt);
      glPopMatrix();

      /* calc center of gravity (middle of bounding box) */
      cog[0] = xmin+((fabs(xmax-xmin))/2.0);
      cog[1] = ymin+((fabs(ymax-ymin))/2.0);
      cog[2] = zmin+((fabs(zmax-zmin))/2.0);

      ay_trafo_apply3(cog, mt);

      dt[0] = view->to[0] - cog[0];
      dt[1] = view->to[1] - cog[1];
      dt[2] = view->to[2] - cog[2];

      /* let the view point to objects COG */
      view->to[0] -= dt[0];
      view->to[1] -= dt[1];
      view->to[2] -= dt[2];
      view->from[0] -= dt[0];
      view->from[1] -= dt[1];
      view->from[2] -= dt[2];

      ay_toglcb_reshape(togl);

      /* calc new zoom factor */
      glMatrixMode (GL_MODELVIEW);
      glPushMatrix();
       glLoadIdentity();
       glRotated(view->rotx, 1.0, 0.0, 0.0);
       glRotated(view->roty, 0.0, 1.0, 0.0);
       glRotated(view->rotz, 0.0, 0.0, 1.0);
       glGetDoublev(GL_MODELVIEW_MATRIX, m);
      glPopMatrix();

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
      
      if(ly>lx)
	l = ly;
      else
	l = lx;
      if(l<lz)
	l = lz;

      if(l < 1E-6 || l > 1E6)
	l = 3.0;

      view->zoom = l*0.9;

      if(view->type == AY_VTPERSP)
	{
	  view->zoom /= 12; 
	  /* normalize */
	  /*
	  dt[0] = view->from[0] - view->to[0];
	  dt[1] = view->from[1] - view->to[1];
	  dt[2] = view->from[2] - view->to[2];
	  l = AY_V3LEN(dt);
	  */
	} /* if */

      if(view->zoom < 1E-6 || view->zoom > 1E6)
	view->zoom = 3.0;

      ay_toglcb_reshape(togl);
      ay_toglcb_display(togl);
      
      ay_viewt_uprop(view);
      view->drawmarker = AY_FALSE;
    } /* if (sel) */

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

  /* get parent objects transformation */
  glMatrixMode (GL_MODELVIEW);
  glPushMatrix();
   glLoadIdentity();
   if(ay_currentlevel->object != ay_root)
     {
       ay_trafo_getallr(ay_currentlevel->next);
     }
   glGetDoublev(GL_MODELVIEW_MATRIX, m);
  glPopMatrix();

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
      
  ay_viewt_uprop(view);
  view->drawmarker = AY_FALSE;

 return TCL_OK;
} /* ay_viewt_align */


/* ay_viewt_makecurtcb:
 *
 */
int
ay_viewt_makecurtcb(struct Togl *togl, int argc, char *argv[])
{
 Tcl_Interp *interp = ay_interp;
 char *n1="ay";
 Tcl_Obj *to = NULL, *toa = NULL, *ton = NULL;
 ay_view_object *view = NULL;

  view = (ay_view_object *)Togl_GetClientData(togl);

  Togl_MakeCurrent(togl);

  toa = Tcl_NewStringObj(n1, -1);
  ton = Tcl_NewStringObj("cVRedraw", -1);
  to = Tcl_NewIntObj(view->redraw);
  Tcl_ObjSetVar2(interp, toa, ton, to, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_SetStringObj(ton, "cVShade", -1);
  to = Tcl_NewIntObj(view->shade);
  Tcl_ObjSetVar2(interp, toa, ton, to, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_SetStringObj(ton, "cVDrawSel", -1);
  to = Tcl_NewIntObj(view->drawsel);
  Tcl_ObjSetVar2(interp, toa, ton, to, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_SetStringObj(ton, "cVDrawLevel", -1);
  to = Tcl_NewIntObj(view->drawlevel);
  Tcl_ObjSetVar2(interp, toa, ton, to, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_SetStringObj(ton, "cVDrawBG", -1);
  to = Tcl_NewIntObj(view->drawbg);
  Tcl_ObjSetVar2(interp, toa, ton, to, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_SetStringObj(ton, "cVDrawGrid", -1);
  to = Tcl_NewIntObj(view->drawgrid);
  Tcl_ObjSetVar2(interp, toa, ton, to, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_SetStringObj(ton, "cVUseGrid", -1);
  to = Tcl_NewIntObj(view->usegrid);
  Tcl_ObjSetVar2(interp, toa, ton, to, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_SetStringObj(ton, "cVLocal", -1);
  to = Tcl_NewIntObj(view->local);
  Tcl_ObjSetVar2(interp, toa, ton, to, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_SetStringObj(ton, "cVGridSize", -1);
  to = Tcl_NewDoubleObj(view->grid);
  Tcl_ObjSetVar2(interp, toa, ton, to, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_SetStringObj(ton, "cVFOV", -1);
  to = Tcl_NewDoubleObj((atan(view->zoom)*180.0)/AY_PI*2.0);
  Tcl_ObjSetVar2(interp, toa, ton, to, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton, "cVType", -1);
  to = Tcl_NewIntObj(view->type);
  Tcl_ObjSetVar2(interp, toa, ton, to, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_IncrRefCount(toa); Tcl_DecrRefCount(toa);
  Tcl_IncrRefCount(ton); Tcl_DecrRefCount(ton);

 return TCL_OK;
} /* ay_viewt_makecurtcb */


/* ay_viewt_changetype:
 *
 */
void
ay_viewt_changetype(ay_view_object *view, int type)
{

  switch(type)
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

  if(type != view->type)
    {
      if(view->type == AY_VTPERSP)
	{
	  view->zoom *= 12.0;
	}
      else
	{
	  if(type == AY_VTPERSP)
	    {
	      view->zoom /= 12.0;
	    }
	} /* if */

    } /* if */

  view->type = type;
  view->drawmarker = AY_FALSE;
  view->aligned = AY_FALSE;

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

  glClearColor((GLfloat)ay_prefs.bgr, (GLfloat)ay_prefs.bgg,
	       (GLfloat)ay_prefs.bgb, (GLfloat)0.0);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  /* draw */
  if(view->shade)
    ay_shade_view(togl);
  else
    ay_draw_view(togl);

  /* XXXX is this really necessary? */ 
  /*glFlush();*/

  Togl_SwapBuffers(togl);

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
 char fname[] = "view_setconf";
 Tcl_Interp *interp = ay_interp;
 ay_view_object *view = (ay_view_object *)Togl_GetClientData(togl);
 ay_object *o = NULL;
 int argi = 0, need_redraw = AY_TRUE;
 double argd = 0.0, rotx = 0.0, roty = 0.0, rotz = 0.0;
 double temp[3] = {0};
 int i = 2;

  Togl_MakeCurrent(togl);

  while(i+1 < argc)
    {
      rotx = 0.0; roty = 0.0; rotz = 0.0;
      Tcl_GetDouble(interp, argv[i+1], &argd);
      /* XXXX this code assumes, every argument is atleast 2 chars long! */
      switch(argv[i][1])
	{
	case 'd':
	  if(!strcmp(argv[i], "-dfromx"))
	    {
	      view->from[0] += argd;
	      view->drawmarker = AY_FALSE;
	      view->aligned = AY_FALSE;
	    }
	  if(!strcmp(argv[i], "-dfromy"))
	    {
	      view->from[1] += argd;
	      view->drawmarker = AY_FALSE;
	      view->aligned = AY_FALSE;
	    }
	  if(!strcmp(argv[i], "-dfromz"))
	    {
	      view->from[2] += argd;
	      view->drawmarker = AY_FALSE;
	      view->aligned = AY_FALSE;
	    }
	  if(!strcmp(argv[i], "-dtox"))
	    {
	      view->to[0] += argd;
	      view->drawmarker = AY_FALSE;
	      view->aligned = AY_FALSE;
	    }
	  if(!strcmp(argv[i], "-dtoy"))
	    {
	      view->to[1] += argd;
	      view->drawmarker = AY_FALSE;
	      view->aligned = AY_FALSE;
	    }
	  if(!strcmp(argv[i], "-dtoz"))
	    {
	      view->to[2] += argd;
	      view->drawmarker = AY_FALSE;
	      view->aligned = AY_FALSE;
	    }
	  if(!strcmp(argv[i], "-drotx"))
	    {
	      roty = argd;
	      view->roty += argd;

	      ay_viewt_rotate(view, 0.0, roty, 0.0);
	      view->drawmarker = AY_FALSE;
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
	      view->drawmarker = AY_FALSE;
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
	      view->drawmarker = AY_FALSE;
	      view->aligned = AY_FALSE;
	    }
	  if(!strcmp(argv[i], "-dzoom"))
	    {
	      view->zoom *= argd;
	      view->drawmarker = AY_FALSE;
	    }
	  if(!strcmp(argv[i], "-dsel"))
	    {
	      Tcl_GetInt(interp, argv[i+1], &argi);
	      view->drawsel = argi;
	    }
	  if(!strcmp(argv[i], "-dlev"))
	    {
	      Tcl_GetInt(interp, argv[i+1], &argi);
	      view->drawlevel = argi;
	    }
	  if(!strcmp(argv[i], "-dbg"))
	    {
	      Tcl_GetInt(interp, argv[i+1], &argi);
	      view->drawbg = argi;
	    }
	  if(!strcmp(argv[i], "-draw"))
	    {
	      Tcl_GetInt(interp, argv[i+1], &argi);
	      view->redraw = argi;
	    }
	  if(!strcmp(argv[i], "-drawg"))
	    {
	      Tcl_GetInt(interp, argv[i+1], &argi);
	      view->drawgrid = argi;
	    }
	  if(!strcmp(argv[i], "-drawh"))
	    {
	      Tcl_GetInt(interp, argv[i+1], &argi);
	      if(argi == view->drawhandles)
		need_redraw = AY_FALSE;
	      view->drawhandles = argi;
	    }
	  break;
	case 'f':
	  if(!strcmp(argv[i], "-fromx"))
	    {
	      view->from[0] = argd;
	      view->drawmarker = AY_FALSE;
	      view->aligned = AY_FALSE;
	    }
	  if(!strcmp(argv[i], "-fromy"))
	    {
	      view->from[1] = argd;
	      view->drawmarker = AY_FALSE;
	      view->aligned = AY_FALSE;
	    }
	  if(!strcmp(argv[i], "-fromz"))
	    {
	      view->from[2] = argd;
	      view->drawmarker = AY_FALSE;
	      view->aligned = AY_FALSE;
	    }
	  if(!strcmp(argv[i], "-fovx"))
	    {
	      Tcl_GetDouble(interp, argv[i+1], &argd);

	      view->zoom = fabs(tan(AY_D2R(argd/2.0)));
	      view->drawmarker = AY_FALSE;
	      view->aligned = AY_FALSE;
	    }
	  break;
	case 'g':
	  if(!strcmp(argv[i], "-grid"))
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
	  if(!strcmp(argv[i], "-gmark"))
	    {
	      Tcl_GetDouble(interp, argv[i+1], &view->markx);
	      Tcl_GetDouble(interp, argv[i+2], &view->marky);
	      if(view->usegrid)
		ay_viewt_griddify(togl, &view->markx, &view->marky);
	      Tcl_GetInt(interp, argv[i+3], &argi);
	      if(view->markx == 0.0 && view->marky == 0 &&
		 argi == view->drawmarker)
		need_redraw = AY_FALSE;
	      view->drawmarker = argi;
	    } /* if */
	  break;
	case 'l':
	  if(!strcmp(argv[i], "-local"))
	    {
	      Tcl_GetInt(interp, argv[i+1], &argi);
	      view->local = argi;
	      view->drawmarker = AY_FALSE;
	    }
	  break;
	case 'm':
	  if(!strcmp(argv[i], "-mark"))
	    {
	      Tcl_GetDouble(interp, argv[i+1], &view->markx);
	      Tcl_GetDouble(interp, argv[i+2], &view->marky);

	      Tcl_GetInt(interp, argv[i+3], &argi);
	      if(view->markx == 0.0 && view->marky == 0 &&
		 argi == view->drawmarker)
		need_redraw = AY_FALSE;
	      view->drawmarker = argi;
	    } /* if */
	  break;
	case 'n':
	  if(!strcmp(argv[i], "-name"))
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
	  if(!strcmp(argv[i], "-pos"))
	    {
	      Tcl_GetInt(interp, argv[i+1], &view->pos_x);
	      Tcl_GetInt(interp, argv[i+2], &view->pos_y);
	    }
#ifdef AYENABLEPPREV
	  if(!strcmp(argv[i], "-pprev"))
	    {
	      Tcl_GetInt(interp, argv[i+1], &argi);
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
	      view->drawmarker = AY_FALSE;
	      view->aligned = AY_FALSE;
	    }
	  if(!strcmp(argv[i], "-rect"))
	    {
	      Tcl_GetDouble(interp, argv[i+1], &view->rect_xmin);
	      Tcl_GetDouble(interp, argv[i+2], &view->rect_ymin);
	      Tcl_GetDouble(interp, argv[i+3], &view->rect_xmax);
	      Tcl_GetDouble(interp, argv[i+4], &view->rect_ymax);
	      
	      Tcl_GetInt(interp, argv[i+5], &view->drawrect);
	    } /* if */
	  break;
	case 's':
	  if(!strcmp(argv[i], "-shade"))
	    {
	      Tcl_GetInt(interp, argv[i+1], &argi);
	      view->shade = argi;
	    }
	  break;
	case 't':
	  if(!strcmp(argv[i], "-tox"))
	    {
	      view->to[0] = argd;
	      view->drawmarker = AY_FALSE;
	      view->aligned = AY_FALSE;
	    }
	  if(!strcmp(argv[i], "-toy"))
	    {
	      view->to[1] = argd;
	      view->drawmarker = AY_FALSE;
	      view->aligned = AY_FALSE;
	    }
	  if(!strcmp(argv[i], "-toz"))
	    {
	      view->to[2] = argd;
	      view->drawmarker = AY_FALSE;
	      view->aligned = AY_FALSE;
	    }
	  if(!strcmp(argv[i], "-type"))
	    {
	      Tcl_GetInt(interp, argv[i+1], &argi);
	      if(argi != view->type)
		{
		  ay_viewt_changetype(view, argi);
		}

	      view->rotx = 0.0;
	      view->roty = 0.0;
	      view->rotz = 0.0;
	      view->drawmarker = AY_FALSE;
	      view->aligned = AY_FALSE;
	    }
	  break;
	case 'u':
	  if(!strcmp(argv[i], "-upx"))
	    {
	      view->up[0] = argd;
	      view->drawmarker = AY_FALSE;
	      view->aligned = AY_FALSE;
	    }
	  if(!strcmp(argv[i], "-upy"))
	    {
	      view->up[1] = argd;
	      view->drawmarker = AY_FALSE;
	      view->aligned = AY_FALSE;
	    }
	  if(!strcmp(argv[i], "-upz"))
	    {
	      view->up[2] = argd;
	      view->drawmarker = AY_FALSE;
	      view->aligned = AY_FALSE;
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
	      view->drawmarker = AY_FALSE;
	      view->aligned = AY_FALSE;
	    }
	  if(!strcmp(argv[i], "-ugrid"))
	    {
	      Tcl_GetInt(interp, argv[i+1], &argi);
	      view->usegrid = argi;
	    }
	  break;
	case 'z':
	  if(!strcmp(argv[i], "-zoom"))
	    {
	      view->zoom = argd;
	      view->drawmarker = AY_FALSE;
	    }
	  break;
	} /* switch */
      i += 2;
  } /* while */

  ay_toglcb_reshape(togl);

  if(need_redraw)
    {
      ay_toglcb_display(togl);
    }

  ay_viewt_uprop(view);

 return TCL_OK;
} /* ay_viewt_setconftcb */


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
  view->roll =  c->roll;
  view->zoom =  c->zoom;

  ay_toglcb_reshape(togl);
  ay_toglcb_display(togl);

  ay_viewt_uprop(view);
  view->drawmarker = AY_FALSE;
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

  memcpy(c->from,view->from,  3*sizeof(double));
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
ay_viewt_uprop(ay_view_object *view)
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
		} /* if */
	    } /* if */
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
 GLint vp[4];

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
	  glMatrixMode(GL_PROJECTION);
	  glGetDoublev(GL_PROJECTION_MATRIX, mp);
	  glMatrixMode(GL_MODELVIEW);
	  glPushMatrix();
	   ay_trafo_getall(ay_currentlevel->next);

	   if(view->aligned && ay_selection)
	     {
	       o = ay_selection->object;
	       glTranslated((GLdouble)o->movx, (GLdouble)o->movy,
			    (GLdouble)o->movz);
	       ay_quat_torotmatrix(o->quat, m);
	       glMultMatrixd((GLdouble*)m);
	  
	       glScaled((GLdouble)o->scalx, (GLdouble)o->scaly,
			(GLdouble)o->scalz);
	     } /* if */

	   glGetDoublev(GL_MODELVIEW_MATRIX, mm);
	  glPopMatrix();
	  gluProject(0.0, 0.0, 0.0, mm, mp, vp, &refx, &refy, &refz);
	  refy = height-refy;
	  switch(view->type)
	    {
	    case AY_VTFRONT:
	    case AY_VTTRIM:
	      gluProject(view->grid, view->grid, 0.0, mm, mp, vp,
			 &gridx, &gridy, &gridz);
	      gridy = height-gridy;
	      break;
	    case AY_VTSIDE:
	      gluProject(0.0, view->grid, -view->grid, mm, mp, vp,
			 &gridx, &gridy, &gridz);
	      gridy = height-gridy;
	      break;
	    case AY_VTTOP:
	      gluProject(view->grid, 0.0, view->grid, mm, mp, vp,
			 &gridx, &gridy, &gridz);
	      break;
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
