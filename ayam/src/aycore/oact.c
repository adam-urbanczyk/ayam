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

/* oact.c - object related interactive actions */

/* ay_oact_movetcb:
 *
 */
int
ay_oact_movetcb(struct Togl *togl, int argc, char *argv[])
{
 int ay_status = AY_OK;
 Tcl_Interp *interp = Togl_Interp(togl);
 ay_view_object *view = (ay_view_object *)Togl_GetClientData(togl);
 ay_point_object *point = NULL;
 static double oldwinx = 0.0, oldwiny = 0.0;
 double winx = 0.0, winy = 0.0;
 double dx = 0, dy = 0, dz = 0;
 double v1[3] = {0}, v2[3] = {0};
 double euler[3] = {0};
 GLdouble mm[16];
 static GLdouble m[16] = {0};
 ay_list_object *sel = ay_selection;
 ay_object *o = NULL;
 char fname[] = "move_object";

 if(view->type == AY_VTPERSP)
    {
      ay_error(AY_ERROR, fname, "Operation not allowed in perspective views.");
      return TCL_OK;
    }

  /* parse args */
  if(argc >= 4)
    {
      if(!strcmp(argv[2],"-winxy"))
      {
	Tcl_GetDouble(interp, argv[3], &winx);
	Tcl_GetDouble(interp, argv[4], &winy);

	if(view->usegrid)
	  {
	    ay_viewt_griddify(togl, &winx, &winy);
	  }
      }
      else
	if(!strcmp(argv[2],"-start"))
	  {

	    if(!ay_selection)
	      {
		ay_error(AY_ENOSEL, fname, NULL);
		return TCL_OK;
	      }

	    Tcl_GetDouble(interp, argv[3], &winx);
	    Tcl_GetDouble(interp, argv[4], &winy);

	    if(view->usegrid)
	      {
		ay_viewt_griddify(togl, &winx, &winy);
	      }

	    oldwinx = winx;
	    oldwiny = winy;

	    /* get real direction of current level coordinate system */
	    glMatrixMode (GL_MODELVIEW);
	    glPushMatrix();
	    /*	    if(view->type != AY_VTTRIM)*/
	    if(!view->local)
	      {
		if(ay_currentlevel && ay_currentlevel->next)
		  {
		    ay_trafo_getallisr(ay_currentlevel->next);
		  }
	      
	      }
	    else
	      {
		if(ay_currentlevel && ay_currentlevel->next)
		  {
		    ay_trafo_getallis(ay_currentlevel->next);
		  }
	      }
	    glGetDoublev(GL_MODELVIEW_MATRIX, m);
	    glPopMatrix();

	  }
    }
  else
    {
      ay_error(AY_EARGS, fname,
		 "\\[-start winx winy|-winxy winx winy\\]");
      return TCL_OK;
    }


  if(!ay_selection)
    {
      return TCL_OK;
    }


  /* bail out, if we stay in the same grid cell */
  if((oldwinx == winx) && (oldwiny == winy))
    return TCL_OK;

  /* calc dx, dy, dz */
  dx = -(oldwinx - winx) * view->conv_x;
  dy = (oldwiny - winy) * view->conv_y;

   if((view->type == AY_VTFRONT) || (view->type == AY_VTTRIM))
    {
      v2[0]=dx;
      v2[1]=dy;
      v2[2]=0.0;
      AY_APTRAN3(v1,v2,m)
      dx = v1[0];
      dy = v1[1];
      dz = v1[2];
    }


  /* Side or Top view? */
  if(view->type == AY_VTSIDE)
    {
      v2[0]=0.0;
      v2[1]=dy;
      v2[2]=-dx;
      AY_APTRAN3(v1,v2,m)
      dx = v1[0];
      dy = v1[1];
      dz = v1[2];
    }
  if(view->type == AY_VTTOP)
    {
      v2[0]=dx;
      v2[1]=0.0;
      v2[2]=-dy;
      AY_APTRAN3(v1,v2,m)
      dx = v1[0];
      dy = v1[1];
      dz = v1[2];
    }

 
  /* move the objects / selected points */

  while(sel)
    {
      o = sel->object;
      if(!o)
	{
	  ay_error(AY_ENULL, fname, NULL);
	  return TCL_OK;
	}

      if(sel->object->selp)
	{
	  glMatrixMode(GL_MODELVIEW);
	  glPushMatrix();
	   glScaled (1.0/o->scalx, 1.0/o->scaly, 1.0/o->scalz);
	   if(!view->aligned)
	     {
	       ay_quat_toeuler(o->quat, euler);
	       glRotated(AY_R2D(euler[0]), 0.0, 0.0, 1.0);
	       glRotated(AY_R2D(euler[1]), 0.0, 1.0, 0.0);
	       glRotated(AY_R2D(euler[2]), 1.0, 0.0, 0.0);
	     }
	   glGetDoublev(GL_MODELVIEW_MATRIX, mm);
	  glPopMatrix();

	  v2[0]=dx;
	  v2[1]=dy;
	  v2[2]=dz;
	  AY_APTRAN3(v1,v2,mm)
	  dx = v1[0];
	  dy = v1[1];
	  dz = v1[2];

	  point = sel->object->selp;
	  while(point)
	    {
	      point->point[0]+=dx;
	      point->point[1]+=dy;
	      point->point[2]+=dz;

	      point = point->next;
	    }

	  o->modified = AY_TRUE;
	  ay_notify_force(o);

	}
      else
	{
	  o = sel->object;
	  if(o)
	    {
	      o->movx += dx; 
	      o->movy += dy; 
	      o->movz += dz; 
	    } /* if */
	} /* if */

      sel = sel->next;
    } /* while */


  oldwinx = winx;
  oldwiny = winy;


  if((fabs(dx) > AY_EPSILON) ||
     (fabs(dy) > AY_EPSILON) ||
     (fabs(dz) > AY_EPSILON))
    {
      if(!ay_prefs.lazynotify)
	{
	  ay_status = ay_notify_parent();
	}

      ay_toglcb_display(togl);
    } /* if */


 return TCL_OK;
} /* ay_oact_movetcb */


/* ay_oact_rottcb:
 *
 */
int
ay_oact_rottcb(struct Togl *togl, int argc, char *argv[])
{
 Tcl_Interp *interp = Togl_Interp (togl);
 ay_view_object *view = (ay_view_object *)Togl_GetClientData(togl);
 static double oldwinx = 0.0, oldwiny = 0.0;
 double height = Togl_Height(togl);
 double winx = 0.0, winy = 0.0, tpoint[4] = {0};
 double angle = 0.0;
 double xaxis[3]={1.0,0.0,0.0};
 double yaxis[3]={0.0,1.0,0.0};
 double zaxis[3]={0.0,0.0,1.0};
 double quat[4];
 double v1[2], v2[2];
 double alpha, beta;
 GLdouble mp[16], mm[16], owinx, owiny, owinz;
 GLint vp[4];
 ay_list_object *sel = ay_selection;
 ay_object *o = NULL;
 ay_point_object *point = NULL;
 char fname[] = "rotate_object";
 
 if(view->type == AY_VTPERSP)
    {
      ay_error(AY_ERROR, fname, "Operation not allowed in perspective views.");
      return TCL_OK;
    }

  /* parse args */
  if(argc == 5)
    {
      if(!strcmp(argv[2],"-winxy"))
      {
	Tcl_GetDouble(interp, argv[3], &winx);
	Tcl_GetDouble(interp, argv[4], &winy);

	if(view->usegrid)
	  {
	    ay_viewt_griddify(togl,&winx,&winy);
	  }

      }
      else
	if(!strcmp(argv[2],"-start"))
	  {

	    if(!ay_selection)
	      {
		ay_error(AY_ENOSEL, fname, NULL);
		return TCL_OK;
	      }

	    Tcl_GetDouble(interp, argv[3], &winx);
	    Tcl_GetDouble(interp, argv[4], &winy);

	    if(view->usegrid)
	      {
		ay_viewt_griddify(togl,&winx,&winy);
	      }

	    oldwinx = winx;
	    oldwiny = winy;
	  }
    }
  else
    {
      ay_error(AY_EARGS, fname,
		 "\\[-start winx winy|-winxy winx winy\\]");
      return TCL_OK;
    }

  if(!ay_selection)
    {
      return TCL_OK;
    }

  if((oldwinx == winx) && (oldwiny == winy))
    return TCL_OK;

  glGetIntegerv(GL_VIEWPORT, vp);
  glGetDoublev(GL_PROJECTION_MATRIX, mp);
  glMatrixMode (GL_MODELVIEW);

  /* rotate the object(s) */
  while(sel)
    {
      o = sel->object;
      if(o)
	{
	  glPushMatrix();
 	   ay_trafo_getall(ay_currentlevel->next);
	   glTranslated(o->movx, o->movy, o->movz);
	   glScaled (o->scalx, o->scaly, o->scalz);
	   glGetDoublev(GL_MODELVIEW_MATRIX, mm);
	  glPopMatrix();
	  gluProject(0.0,0.0,0.0,mm,mp,vp,&owinx,&owiny,&owinz);
    
	  owiny = height-owiny;
	    
	  v1[0] = oldwinx-owinx;
	  v1[1] = oldwiny-owiny;

	  if((v1[0]==0.0)&&(v1[1]==0.0))
	    break;
	  alpha = AY_R2D(acos(v1[0]/AY_V2LEN(v1)));
	  if(v1[1]<0.0)
	    alpha = 360.0-alpha;
	     
	  v2[0] = winx-owinx;
	  v2[1] = winy-owiny;
	  if((v2[0]==0.0)&&(v2[1]==0.0))
	    break;
	  beta = AY_R2D(acos(v2[0]/AY_V2LEN(v2)));
	  if(v2[1]<0.0)
	    beta = 360.0-beta;

	  angle = beta - alpha;

	  if(o->selp)
	    {
	      point = o->selp;

	      glPushMatrix();
	      glLoadIdentity();

	      switch(view->type)
		{
		case AY_VTSIDE:
		  /* rotate about x */
		  glRotated(-angle,1.0,0.0,0.0);
		  break;
		case AY_VTFRONT:
		case AY_VTTRIM:
		  /* rotate about z */
		  glRotated(-angle,0.0,0.0,1.0);
		  break;
		case AY_VTTOP:
		  /* rotate about y */
		  glRotated(-angle,0.0,1.0,0.0);
		  break;
		default:
		  break;
		} /* switch */

	      glGetDoublev(GL_MODELVIEW_MATRIX, mm);

	      glPopMatrix();
	      
	      while(point)
		{
		  AY_APTRAN3(tpoint,point->point,mm);
		  memcpy(point->point,tpoint,3*sizeof(double));

		  point = point->next;
		}

	      o->modified = AY_TRUE;
	      ay_notify_force(o);

	    }
	  else
	    switch(view->type)
	      {
	      case AY_VTSIDE:
		/* rotate about x */

		o->rotx += angle;
		if(o->rotx >= 360.0) o->rotx -= 360.0;
		if(o->rotx <= -360.0) o->rotx += 360.0;

		ay_quat_axistoquat(xaxis, AY_D2R(angle), quat);
		ay_quat_add(quat, o->quat, o->quat);
		break;
	      case AY_VTFRONT:
	      case AY_VTTRIM:
		/* rotate about z */

		o->rotz += angle;
		if(o->rotz >= 360.0) o->rotz -= 360.0;
		if(o->rotz <= -360.0) o->rotz += 360.0;

		ay_quat_axistoquat(zaxis, AY_D2R(angle), quat);
		ay_quat_add(quat, o->quat, o->quat);
		break;
	      case AY_VTTOP:
		/* rotate about y */

		o->roty -= angle;
		if(o->roty >= 360.0) o->roty -= 360.0;
		if(o->roty <= -360.0) o->roty += 360.0;

		ay_quat_axistoquat(yaxis, AY_D2R(angle), quat);
		ay_quat_add(quat, o->quat, o->quat);
		break;
	      default:
		break;
	      } /* switch */

	} /* if */
      
      sel = sel->next;
    } /* while */

  if(!ay_prefs.lazynotify)
    ay_notify_parent();

  ay_toglcb_display(togl);

  oldwinx = winx;
  oldwiny = winy;

 return TCL_OK;
} /* ay_oact_rottcb */


/* ay_oact_rotatcb
 *
 */
int
ay_oact_rotatcb(struct Togl *togl, int argc, char *argv[])
{
 Tcl_Interp *interp = Togl_Interp (togl);
 ay_view_object *view = (ay_view_object *)Togl_GetClientData(togl);
 static double oldwinx = 0.0, oldwiny = 0.0;
 static double oldfromx = 0.0, oldfromy = 0.0, oldfromz = 0.0;
 double height = Togl_Height(togl);
 double winx = 0.0, winy = 0.0, ax=0.0, ay=0.0, ay2=0.0, axo=0.0, ayo=0.0;
 double dummy;
 double angle = 0.0, tpoint[4] = {0};
 double xaxis[3]={1.0,0.0,0.0};
 double yaxis[3]={0.0,1.0,0.0};
 double zaxis[3]={0.0,0.0,1.0};
 double dx,dy,dax,day,zangle,yangle,xangle;
 double quat[4];
 double v1[3] = {0}, v2[3] = {0}, v3[3];
 double alpha, beta;
 GLdouble m[16];
 GLdouble mp[16], mm[16], owinx = 0.0, owiny = 0.0, owinz = 0.0;
 GLint vp[4];
 ay_list_object *sel = ay_selection;
 ay_object *o = NULL;
 ay_point_object *point = NULL;
 char fname[] = "rotate_object_about";


  /* parse args */
  if(argc == 7)
    {
      if(!strcmp(argv[2],"-winxy"))
      {
	if(!view->drawmarker)
	  {
	    /* if view->drawmarker is not enabled some other action
	       changed view trafos so that the point is not valid
	       anymore and we should request a new point */
	    ay_error(AY_ERROR, fname,
		  "Lost point to rotate about. Please restart this action!");
	    return TCL_OK;
	  }

	if(oldfromx != view->from[0] ||
	   oldfromy != view->from[1] ||
	   oldfromz != view->from[2])
	  {
	    /* if some other action changed view trafos the point
	       to rotate about is not valid anymore and we should
	       request a new point */
	    ay_error(AY_ERROR, fname,
		   "Lost point to rotate about. Please restart this action!");
	    view->drawmarker = AY_FALSE;
	    return TCL_OK;
	  }

	Tcl_GetDouble(interp, argv[3], &winx);
	Tcl_GetDouble(interp, argv[4], &winy);
	Tcl_GetDouble(interp, argv[5], &ax);
	Tcl_GetDouble(interp, argv[6], &ay);

	if(view->usegrid)
	  {
	    ay_viewt_griddify(togl, &winx, &winy);
	    ay_viewt_griddify(togl, &ax, &ay);
	  }
      }
      else
	if(!strcmp(argv[2],"-start"))
	  {
	    
	    if(!ay_selection)
	      {
		ay_error(AY_ENOSEL, fname, NULL);
		return TCL_OK;
	      }

	    Tcl_GetDouble(interp, argv[3], &winx);
	    Tcl_GetDouble(interp, argv[4], &winy);
	    Tcl_GetDouble(interp, argv[5], &ax);
	    Tcl_GetDouble(interp, argv[6], &ay);

	    if(view->usegrid)
	      {
		ay_viewt_griddify(togl, &winx, &winy);
		ay_viewt_griddify(togl, &ax, &ay);

		view->markx = ax;
		view->marky = ay;

	      }

	    oldwinx = winx;
	    oldwiny = winy;
	    
	  }
    }
  else
    {

      oldfromx = view->from[0];
      oldfromy = view->from[1];
      oldfromz = view->from[2];

      /*ay_error(AY_EARGS, fname, "-start winx winy|-winxy winx winy");*/
      return TCL_OK;
    }

  if(!ay_selection)
    {
      return TCL_OK;
    }

  if((oldwinx == winx) && (oldwiny == winy))
    return TCL_OK;

  glGetIntegerv(GL_VIEWPORT, vp);
  glGetDoublev(GL_PROJECTION_MATRIX, mp);
  glMatrixMode (GL_MODELVIEW);

  ay2 = ay;

  /* rotate the object(s) */
  while(sel)
    {
      o = sel->object;
      if(o)
	{
	  ay = ay2;
	  glPushMatrix();
	  if(ay_currentlevel->next)
	    ay_trafo_getall(ay_currentlevel->next);
	   glTranslated(o->movx, o->movy, o->movz);
	   /*ay_build_rotmatrix(m, o->quat);
	     glMultMatrixf(&m[0][0]);*/
	   glScaled (o->scalx, o->scaly, o->scalz);
	   glGetDoublev(GL_MODELVIEW_MATRIX, mm);
	  glPopMatrix();
	  gluProject(0.0,0.0,0.0,mm,mp,vp,&owinx,&owiny,&owinz);

	  owiny = height-owiny;

	  dx = owinx;
	  dy = owiny;
	  dax = dx - ax;
	  day = dy - ay;

	  owinx -= (owinx - ax);
	  owiny -= (owiny - ay);

	  v1[0] = oldwinx-owinx;
	  v1[1] = oldwiny-owiny;

	  if((v1[0]==0.0)&&(v1[1]==0.0))
	    break;
	  alpha = AY_R2D(acos(v1[0]/AY_V2LEN(v1)));
	  if(v1[1]<0.0)
	    alpha = 360.0-alpha;
	     
	  v2[0] = winx-owinx;
	  v2[1] = winy-owiny;
	  if((v2[0]==0.0)&&(v2[1]==0.0))
	    break;
	  beta = AY_R2D(acos(v2[0]/AY_V2LEN(v2)));
	  if(v2[1]<0.0)
	    beta = 360.0-beta;

	  angle = beta - alpha;

	  switch(view->type)
	    {
	    case AY_VTSIDE:
	      /* rotate about x */

	      /* get old rotation about X */
	      glPushMatrix();
	      ay_quat_torotmatrix(o->quat, m);
	      glMultMatrixd(m);
	      glGetDoublev(GL_MODELVIEW_MATRIX, mm);
	      glPopMatrix();

	      AY_APTRAN3(v3,zaxis,mm);
	      v2[0] = v3[2];
	      v2[1] = v3[1];
	      xangle = AY_R2D(acos(v2[0]/AY_V2LEN(v2)));
	      if(v2[1]<0.0)
		xangle = 360.0-xangle;

	      glPushMatrix();
	      if(ay_currentlevel->next)
		ay_trafo_getall(ay_currentlevel->next);
	      glTranslated(o->movx, o->movy, o->movz);
	      
	      glRotated(xangle, 1.0, 0.0, 0.0);

	      /*	      glScaled (o->scalx, o->scaly, o->scalz);*/
	      glGetDoublev(GL_MODELVIEW_MATRIX, mm);
	      glPopMatrix();
	      /*
	      gluProject(0.0,0.0,0.0,mm,mp,vp,&owinx,&owiny,&owinz);
	      */
	      ay = height-ay;
	      
	      gluUnProject(ax,ay,owinz,mm,mp,vp,&dummy,&ayo,&axo);

	      glPushMatrix();
	      glLoadIdentity();

	      glRotated(xangle, 1.0, 0.0, 0.0);

	      glTranslated(0.0, -ayo, -axo);
	      glRotated(-angle,1.0,0.0,0.0);
	      glTranslated(0.0, ayo, axo);

	      glGetDoublev(GL_MODELVIEW_MATRIX, mm);
	      glPopMatrix();

	      v1[0] = 0.0;
	      v1[1] = 0.0;
	      v1[2] = 0.0;

	      AY_APTRAN3(v2,v1,mm);

	      break;
	    case AY_VTFRONT:
	    case AY_VTTRIM:
	      /* rotate about z */

	      /* get old rotation about Z */
	      glPushMatrix();
	      ay_quat_torotmatrix(o->quat, m);
	      glMultMatrixd(m);
	      glGetDoublev(GL_MODELVIEW_MATRIX, mm);
	      glPopMatrix();

	      AY_APTRAN3(v3,xaxis,mm);
	      v2[0] = v3[0];
	      v2[1] = v3[1];
	      zangle = AY_R2D(acos(v2[0]/AY_V2LEN(v2)));
	      if(v2[1]<0.0)
		zangle = 360.0-zangle;

	      glPushMatrix();
	      if(ay_currentlevel->next)
		ay_trafo_getall(ay_currentlevel->next);
	      glTranslated(o->movx, o->movy, o->movz);
	      
	      glRotated(zangle, 0.0, 0.0, 1.0);

	      /*	      glScaled (o->scalx, o->scaly, o->scalz);*/
	      glGetDoublev(GL_MODELVIEW_MATRIX, mm);
	      glPopMatrix();
	      /*
	      gluProject(0.0,0.0,0.0,mm,mp,vp,&owinx,&owiny,&owinz);
	      */
	      ay = height-ay;
	      
	      gluUnProject(ax,ay,owinz,mm,mp,vp,&axo,&ayo,&dummy);

	      glPushMatrix();
	      glLoadIdentity();

	      glRotated(zangle, 0.0, 0.0, 1.0);

	      glTranslated(-axo, -ayo, 0.0);
	      glRotated(-angle,0.0,0.0,1.0);
	      glTranslated(axo, ayo, 0.0);

	      glGetDoublev(GL_MODELVIEW_MATRIX, mm);
	      glPopMatrix();

	      v1[0] = 0.0;
	      v1[1] = 0.0;

	      AY_APTRAN3(v2,v1,mm);

	      break;
	    case AY_VTTOP:
	      /* rotate about y */

	      /* get old rotation about Y */

	      glPushMatrix();
	      ay_quat_torotmatrix(o->quat, m);
	      glMultMatrixd(m);
	      glGetDoublev(GL_MODELVIEW_MATRIX, mm);
	      glPopMatrix();

	      AY_APTRAN3(v3,xaxis,mm);
	      v2[0] = v3[0];
	      v2[1] = v3[2];
	      yangle = AY_R2D(acos(v2[0]/AY_V2LEN(v2)));
	      if(v2[1]<0.0)
		yangle = 360.0-yangle;

	      glPushMatrix();
	      if(ay_currentlevel->next)
		ay_trafo_getall(ay_currentlevel->next);
	      glTranslated(o->movx, o->movy, o->movz);
	      
	      glRotated(yangle, 0.0, 1.0, 0.0);

	      /*	      glScaled (o->scalx, o->scaly, o->scalz);*/
	      glGetDoublev(GL_MODELVIEW_MATRIX, mm);
	      glPopMatrix();
	      /*
	      gluProject(0.0,0.0,0.0,mm,mp,vp,&owinx,&owiny,&owinz);
	      */
	      ay = height-ay;
	      
	      gluUnProject(ax,ay,owinz,mm,mp,vp,&axo,&dummy,&ayo);

	      glPushMatrix();
	      glLoadIdentity();

	      glRotated(yangle, 0.0, 1.0, 0.0);

	      glTranslated(-axo, 0.0, -ayo);
	      glRotated(-angle,0.0,1.0,0.0);
	      glTranslated(axo, 0.0, ayo);

	      glGetDoublev(GL_MODELVIEW_MATRIX, mm);
	      glPopMatrix();

	      v1[0] = 0.0;
	      v1[1] = 0.0;
	      v1[2] = 0.0;

	      AY_APTRAN3(v2,v1,mm);

	      break;
	    default:
	      break;
	    } /* switch */


	  if(o->selp)
	    {
	      point = o->selp;

	      glPushMatrix();
	      glLoadIdentity();
	      switch(view->type)
		{
		case AY_VTSIDE:
		  /* rotate about x */
		  glRotated(-angle,1.0,0.0,0.0);
		  glTranslated(0.0,-v2[1],-v2[2]);
		  
		  break;
		case AY_VTFRONT:
		case AY_VTTRIM:
		  /* rotate about z */
		  glRotated(-angle,0.0,0.0,1.0);
		  glTranslated(-v2[0],-v2[1],0.0);

		  break;
		case AY_VTTOP:
		  /* rotate about y */
		  glRotated(-angle,0.0,1.0,0.0);
		  glTranslated(-v2[0],0.0,-v2[2]);
	      
		  break;
		default:
		  break;
		} /* switch */

	      glGetDoublev(GL_MODELVIEW_MATRIX, mm);
	      glPopMatrix();

	      while(point)
		{
		  AY_APTRAN3(tpoint,point->point,mm);
		  memcpy(point->point,tpoint,3*sizeof(double));

		  point = point->next;
		}

	      o->modified = AY_TRUE;
	      ay_notify_force(o);

	    }
	  else
	    {
	      switch(view->type)
		{
		case AY_VTSIDE:
		  /* rotate about x */

	      
		  o->movz -= v2[2];
		  o->movy -= v2[1];
	
		  o->rotx += angle;
		  if(o->rotx >= 360.0) o->rotx -= 360.0;
		  if(o->rotx <= -360.0) o->rotx += 360.0;

		  ay_quat_axistoquat(xaxis, AY_D2R(angle), quat);
		  ay_quat_add(quat, o->quat, o->quat);
		  break;
		case AY_VTFRONT:
		case AY_VTTRIM:
		  /* rotate about z */

		  o->movx -= v2[0];
		  o->movy -= v2[1];
	      
		  o->rotz += angle;
		  if(o->rotz >= 360.0) o->rotz -= 360.0;
		  if(o->rotz <= -360.0) o->rotz += 360.0;

		  ay_quat_axistoquat(zaxis, AY_D2R(angle), quat);
		  ay_quat_add(quat, o->quat, o->quat);

		  break;
		case AY_VTTOP:
		  /* rotate about y */
	    
		  o->movx -= v2[0];
		  o->movz -= v2[2];

		  o->roty -= angle;
		  if(o->roty >= 360.0) o->roty -= 360.0;
		  if(o->roty <= -360.0) o->roty += 360.0;

		  ay_quat_axistoquat(yaxis, AY_D2R(angle), quat);
		  ay_quat_add(quat, o->quat, o->quat);
		  break;
		default:
		  break;
		} /* switch */

	    } /* else */

	} /* if */
      
      sel = sel->next;
    } /* while */

  if(!ay_prefs.lazynotify)
    ay_notify_parent();

  ay_toglcb_display(togl);

  oldwinx = winx;
  oldwiny = winy;

 return TCL_OK;
} /* ay_oact_rotatcb */


/* ay_oact_sc1DXcb:
 *
 */
int
ay_oact_sc1DXcb(struct Togl *togl, int argc, char *argv[])
{
 Tcl_Interp *interp = Togl_Interp (togl);
 ay_view_object *view = (ay_view_object *)Togl_GetClientData(togl);
 double height = Togl_Height(togl);
 static double oldwinx = 0.0, oldwiny = 0.0;
 double winx = 0.0, winy = 0.0, dscalx = 1.0;
 double tpoint[4]={0}, t1, t2, v1[2], v2[2];
 GLdouble vx[3], alpha, beta, gamma;
 GLdouble mp[16], mm[16], owinx, owiny, owinz;
 GLdouble m[16];
 GLint vp[4];
 ay_list_object *sel = ay_selection;
 ay_object *o = NULL;
 ay_point_object *point = NULL;
 char fname[] = "scale1DX_object";

 if(view->type == AY_VTPERSP)
    {
      ay_error(AY_ERROR, fname, "Operation not allowed in perspective views.");
      return TCL_OK;
    }

  /* parse args */
  if(argc >= 4)
    {
      if(!strcmp(argv[2],"-winxy"))
      {
	Tcl_GetDouble(interp, argv[3], &winx);
	Tcl_GetDouble(interp, argv[4], &winy);
	if(view->usegrid)
	  {
	    ay_viewt_griddify(togl,&winx,&winy);
	  }

      }
      else
	if(!strcmp(argv[2],"-start"))
	  {
	    
	    if(!ay_selection)
	      {
		ay_error(AY_ENOSEL, fname, NULL);
		return TCL_OK;
	      }

	    Tcl_GetDouble(interp, argv[3], &winx);
	    Tcl_GetDouble(interp, argv[4], &winy);
	    
	    if(view->usegrid)
	      {
		ay_viewt_griddify(togl,&winx,&winy);
	      }
	    
	    oldwinx = winx;
	    oldwiny = winy;

	  }

    }
  else
    {
      ay_error(AY_EARGS, fname,
		 "\\[-start $winx $winy|-winxy $winx $winy\\]");
      return TCL_OK;
    }

  if(!ay_selection)
    {
      return TCL_OK;
    }

  if((oldwinx == winx) && (oldwiny == winy))
    return TCL_OK;

  glGetIntegerv(GL_VIEWPORT, vp);
  glGetDoublev(GL_PROJECTION_MATRIX, mp);
  glMatrixMode(GL_MODELVIEW);

  /* scale the object(s) / selected points */
  while(sel)
    {
      o = sel->object;
      if(o)
	{
	  glPushMatrix();

	  if(view->type != AY_VTTRIM)
	    {
	      ay_trafo_getall(ay_currentlevel->next);
	    }

	   glTranslated(o->movx, o->movy, o->movz);
	   ay_quat_torotmatrix(o->quat, m);
	   glMultMatrixd(m);
	   glScaled (o->scalx, o->scaly, o->scalz);
	    
	   glGetDoublev(GL_MODELVIEW_MATRIX, mm);
	  glPopMatrix();
	  gluProject(0.0,0.0,0.0,mm,mp,vp,&owinx,&owiny,&owinz);
	  gluProject(1.0,0.0,0.0,mm,mp,vp,&vx[0],&vx[1],&vx[2]);

	  owiny = height - owiny;
	  vx[1] = height - vx[1];

	  vx[0] -= owinx;
	  vx[1] -= owiny;

	  if((vx[0]!=0.0)||(vx[1]!=0.0))
	    {

	      alpha = AY_R2D(acos(vx[0]/AY_V2LEN(vx)));
	      if(vx[1]>0.0)
		alpha = 360.0-alpha;

	      v1[0] = (oldwinx-owinx);
	      v1[1] = (oldwiny-owiny);

	      v2[0] = (winx-owinx);
 	      v2[1] = (winy-owiny);
	      
	      beta = AY_R2D(acos(v1[0]/AY_V2LEN(v1)));
	      if(v1[1]<0.0)
		beta = 360.0-beta;

	      gamma = AY_R2D(acos(v2[0]/AY_V2LEN(v2)));
	      if(v1[1]<0.0)
		gamma = 360.0-gamma;

	      t1 = cos(AY_D2R(alpha)+AY_D2R(beta))*AY_V2LEN(v1);
	      t2 = cos(AY_D2R(alpha)+AY_D2R(gamma))*AY_V2LEN(v2);

	      if(t1)
		dscalx = t2/t1;
	      else
		dscalx = 1.0;
	    }
	  else
	    dscalx = 1.0;
	      
	  if(o->selp)
	    {
	      point = o->selp;
	      
	      glPushMatrix();
	       glLoadIdentity();

	       if(!view->local)
		 ay_trafo_getallisr(ay_currentlevel->next);

	       glScaled(dscalx,1.0,1.0);

	       if(!view->local)
		 ay_trafo_getallsr(ay_currentlevel->next);

	       glGetDoublev(GL_MODELVIEW_MATRIX, mm);
	      glPopMatrix();

	      while(point)
		{
		  AY_APTRAN3(tpoint,point->point,mm);
		  memcpy(point->point,tpoint,3*sizeof(double));

		  point = point->next;
		}

	      o->modified = AY_TRUE;	      
	      ay_notify_force(o);

	    }
	  else
	    {
	      o->scalx *= dscalx;
	    }


	}
      
      sel = sel->next;
    }

  oldwinx = winx;
  oldwiny = winy;
  
  if(!ay_prefs.lazynotify)
    ay_notify_parent();

  ay_toglcb_display(togl);

 return TCL_OK;
} /* ay_oact_sc1DXcb */


/* ay_oact_sc1DYcb:
 *
 */
int
ay_oact_sc1DYcb(struct Togl *togl, int argc, char *argv[])
{
 Tcl_Interp *interp = Togl_Interp (togl);
 ay_view_object *view = (ay_view_object *)Togl_GetClientData(togl);
 double height = Togl_Height(togl);
 static double oldwinx = 0.0, oldwiny = 0.0;
 double winx = 0.0, winy = 0.0, dscaly = 1.0;
 double tpoint[4]={0}, t1, t2, v1[2], v2[2];
 GLdouble vy[3], alpha, beta, gamma;
 GLdouble mp[16], mm[16], owinx, owiny, owinz;
 GLdouble m[16];
 GLint vp[4];
 ay_list_object *sel = ay_selection;
 ay_object *o = NULL;
 ay_point_object *point = NULL;
 char fname[] = "scale1DY_object";

 if(view->type == AY_VTPERSP)
    {
      ay_error(AY_ERROR, fname, "Operation not allowed in perspective views.");
      return TCL_OK;
    }

  /* parse args */
  if(argc >= 4)
    {
      if(!strcmp(argv[2],"-winxy"))
      {
	Tcl_GetDouble(interp, argv[3], &winx);
	Tcl_GetDouble(interp, argv[4], &winy);
	if(view->usegrid)
	  {
	    ay_viewt_griddify(togl,&winx,&winy);
	  }	
      }
      else
	if(!strcmp(argv[2],"-start"))
	  {
	    
	    if(!ay_selection)
	      {
		ay_error(AY_ENOSEL, fname, NULL);
		return TCL_OK;
	      }

	    Tcl_GetDouble(interp, argv[3], &winx);
	    Tcl_GetDouble(interp, argv[4], &winy);
	    if(view->usegrid)
	      {
		ay_viewt_griddify(togl,&winx,&winy);
	      }

	    oldwinx = winx;
	    oldwiny = winy;
	  }

    }
  else
    {
      ay_error(AY_EARGS, fname,
		 "\\[-start $winx $winy|-winxy $winx $winy\\]");
      return TCL_OK;
    }

  if(!ay_selection)
    {
      return TCL_OK;
    }

  if((oldwinx == winx) && (oldwiny == winy))
    return TCL_OK;

  glGetIntegerv(GL_VIEWPORT, vp);
  glGetDoublev(GL_PROJECTION_MATRIX, mp);
  glMatrixMode(GL_MODELVIEW);

  /* scale the object(s) / selected points */
  while(sel)
    {
      o = sel->object;
      if(o)
	{
	  glPushMatrix();
	  if(view->type != AY_VTTRIM)
	    ay_trafo_getall(ay_currentlevel->next);
	   glTranslated(o->movx, o->movy, o->movz);
	   ay_quat_torotmatrix(o->quat, m);
	   glMultMatrixd(m);
	   glScaled (o->scalx, o->scaly, o->scalz);
	   glGetDoublev(GL_MODELVIEW_MATRIX, mm);
	  glPopMatrix();
	  gluProject(0.0,0.0,0.0,mm,mp,vp,&owinx,&owiny,&owinz);
	  gluProject(0.0,1.0,0.0,mm,mp,vp,&vy[0],&vy[1],&vy[2]);

	  vy[1] = height - vy[1];
	  owiny = height - owiny;
	 
	  vy[0] -= owinx;
	  vy[1] -= owiny;

	  if((vy[0]!=0.0)||(vy[1]!=0.0))
	    {
	      alpha = AY_R2D(acos(vy[0]/AY_V2LEN(vy)));
	      if(vy[1]>0.0)
		alpha = 360.0-alpha;

	      v1[0] = (oldwinx-owinx);
	      v1[1] = (oldwiny-owiny);

	      v2[0] = (winx-owinx);
	      v2[1] = (winy-owiny);
	      
	      beta = AY_R2D(acos(v1[0]/AY_V2LEN(v1)));
	      if(v1[1]<0.0)
		beta = 360.0-beta;

	      gamma = AY_R2D(acos(v2[0]/AY_V2LEN(v2)));
	      if(v1[1]<0.0)
		gamma = 360.0-gamma;

	      t1 = cos(AY_D2R(alpha)+AY_D2R(beta))*AY_V2LEN(v1);
	      t2 = cos(AY_D2R(alpha)+AY_D2R(gamma))*AY_V2LEN(v2);

	      if(t1)
		dscaly = t2/t1;
	      else
		dscaly = 1.0;
	    }
	  else
	    dscaly = 1.0;
	      
	  if(o->selp)
	    {
	      point = o->selp;
	      
	      glPushMatrix();
	       glLoadIdentity();

	       if(!view->local)
		 ay_trafo_getallisr(ay_currentlevel->next);

	       glScaled(1.0,dscaly,1.0);

	       if(!view->local)
		 ay_trafo_getallsr(ay_currentlevel->next);

	       glGetDoublev(GL_MODELVIEW_MATRIX, mm);
	      glPopMatrix();

	      while(point)
		{
		  AY_APTRAN3(tpoint,point->point,mm);
		  memcpy(point->point,tpoint,3*sizeof(double));

		  point = point->next;
		}

	      o->modified = AY_TRUE;	
	      ay_notify_force(o);

	    }
	  else
	    {
	      o->scaly *= dscaly;
	    }
	}
      
      sel = sel->next;
    }

  oldwinx = winx;
  oldwiny = winy;

  if(!ay_prefs.lazynotify)
    ay_notify_parent();

  ay_toglcb_display(togl);

 return TCL_OK;
} /* ay_oact_sc1DYcb */


/* ay_oact_sc1DZcb:
 *
 */
int
ay_oact_sc1DZcb(struct Togl *togl, int argc, char *argv[])
{
 Tcl_Interp *interp = Togl_Interp (togl);
 ay_view_object *view = (ay_view_object *)Togl_GetClientData(togl);
 double height = Togl_Height(togl);
 static double oldwinx = 0.0, oldwiny = 0.0;
 double winx = 0.0, winy = 0.0, dscalz = 1.0;
 double tpoint[4]={0}, t1, t2, v1[2], v2[2];
 GLdouble vz[3], alpha, beta, gamma;
 GLdouble mp[16], mm[16], owinx, owiny, owinz;
 GLdouble m[16];
 GLint vp[4];
 ay_list_object *sel = ay_selection;
 ay_object *o = NULL;
 ay_point_object *point = NULL;
 char fname[] = "scale1DZ_object";

 if(view->type == AY_VTPERSP)
    {
      ay_error(AY_ERROR, fname, "Operation not allowed in perspective views.");
      return TCL_OK;
    }

  /* parse args */
  if(argc >= 4)
    {
      if(!strcmp(argv[2],"-winxy"))
      {
	Tcl_GetDouble(interp, argv[3], &winx);
	Tcl_GetDouble(interp, argv[4], &winy);

	if(view->usegrid)
	  {
	    ay_viewt_griddify(togl,&winx,&winy);
	  }	
      }
      else
	if(!strcmp(argv[2],"-start"))
	  {
	    
	    if(!ay_selection)
	      {
		ay_error(AY_ENOSEL, fname, NULL);
		return TCL_OK;
	      }

	    Tcl_GetDouble(interp, argv[3], &winx);
	    Tcl_GetDouble(interp, argv[4], &winy);

	    if(view->usegrid)
	      {
		ay_viewt_griddify(togl,&winx,&winy);
	      }

	    oldwinx = winx;
	    oldwiny = winy;
	  }

    }
  else
    {
      ay_error(AY_EARGS, fname,
		 "\\[-start $winx $winy|-winxy $winx $winy\\]");
      return TCL_OK;
    }

  if(!ay_selection)
    {
      return TCL_OK;
    }

  if((oldwinx == winx) && (oldwiny == winy))
    return TCL_OK;

  glGetIntegerv(GL_VIEWPORT, vp);
  glGetDoublev(GL_PROJECTION_MATRIX, mp);
  glMatrixMode(GL_MODELVIEW);

  /* scale the object(s) / selected points */
  while(sel)
    {
      o = sel->object;
      if(o)
	{
	  glPushMatrix();
	  if(view->type != AY_VTTRIM)
	    ay_trafo_getall(ay_currentlevel->next);
	   glTranslated(o->movx, o->movy, o->movz);
	   ay_quat_torotmatrix(o->quat, m);
	   glMultMatrixd(m);
	   glScaled (o->scalx, o->scaly, o->scalz);
	   glGetDoublev(GL_MODELVIEW_MATRIX, mm);
	  glPopMatrix();
	  gluProject(0.0,0.0,0.0,mm,mp,vp,&owinx,&owiny,&owinz);
	  gluProject(0.0,0.0,1.0,mm,mp,vp,&vz[0],&vz[1],&vz[2]);

	  vz[1] = height - vz[1];
	  owiny = height - owiny;
	 
	  vz[0] -= owinx;
	  vz[1] -= owiny;

	  if((vz[0]!=0.0)||(vz[1]!=0.0))
	    {
	      alpha = AY_R2D(acos(vz[0]/AY_V2LEN(vz)));
	      if(vz[1]>0.0)
		alpha = 360.0-alpha;

	      v1[0] = (oldwinx-owinx);
	      v1[1] = (oldwiny-owiny);

	      v2[0] = (winx-owinx);
	      v2[1] = (winy-owiny);
	      
	      beta = AY_R2D(acos(v1[0]/AY_V2LEN(v1)));
	      if(v1[1]<0.0)
		beta = 360.0-beta;

	      gamma = AY_R2D(acos(v2[0]/AY_V2LEN(v2)));
	      if(v1[1]<0.0)
		gamma = 360.0-gamma;

	      t1 = cos(AY_D2R(alpha)+AY_D2R(beta))*AY_V2LEN(v1);
	      t2 = cos(AY_D2R(alpha)+AY_D2R(gamma))*AY_V2LEN(v2);

	      if(t1)
		dscalz = t2/t1;
	      else
		dscalz = 1.0;
	    }
	  else
	    dscalz = 1.0;
	      
	  if(o->selp)
	    {
	      point = o->selp;
	      
	      glPushMatrix();
	       glLoadIdentity();

	       if(!view->local)
		 ay_trafo_getallisr(ay_currentlevel->next);

	       glScaled(1.0,1.0,dscalz);

	       if(!view->local)
		 ay_trafo_getallsr(ay_currentlevel->next);

	       glGetDoublev(GL_MODELVIEW_MATRIX, mm);
	      glPopMatrix();

	      while(point)
		{
		  AY_APTRAN3(tpoint,point->point,mm);
		  memcpy(point->point,tpoint,3*sizeof(double));

		  point = point->next;
		}

	      o->modified = AY_TRUE;
	      ay_notify_force(o);

	    }
	  else
	    {
	      o->scalz *= dscalz;
	    }
	}
      
      sel = sel->next;
    }

  oldwinx = winx;
  oldwiny = winy;

  if(!ay_prefs.lazynotify)
    ay_notify_parent();

  ay_toglcb_display(togl);

 return TCL_OK;
} /* ay_oact_sc1DXcb */

#if 0
/* selected points only */
int
ay_oact_sc1DXWcb(struct Togl *togl, int argc, char *argv[])
{
 Tcl_Interp *interp = Togl_Interp (togl);
 ay_view_object *view = Togl_GetClientData(togl);
 static double oldwinx = 0.0, oldwiny = 0.0;
 double winx = 0.0, winy = 0.0, dscalx = 0.0;
 double tpoint[4]={0};
 GLdouble mp[16], mm[16], owinx, owiny, owinz;
 GLdouble m[16];
 GLint vp[4];
 ay_list_object *sel = ay_selection;
 ay_object *o = NULL;
 ay_point_object *point = NULL;
 char fname[] = "scale1DX_world";
 double euler[3] = {0};

  if(view->type == AY_VTPERSP)
    {
      ay_error(AY_ERROR, fname, "Operation not allowed in perspective views.");
      return TCL_OK;
    }

  /* parse args */
  if(argc >= 4)
    {
      if(!strcmp(argv[2],"-winxy"))
      {
	Tcl_GetDouble(interp, argv[3], &winx);
	Tcl_GetDouble(interp, argv[4], &winy);
	if(view->grid != 0.0)
	  {
	    ay_viewt_griddify(togl,&winx,&winy);
	  }	
      }
      else
	if(!strcmp(argv[2],"-start"))
	  {
	    
	    if(!ay_selection)
	      {
		ay_error(AY_ENOSEL, fname, NULL);
		return TCL_OK;
	      }

	    Tcl_GetDouble(interp, argv[3], &winx);
	    Tcl_GetDouble(interp, argv[4], &winy);
	    if(view->grid != 0.0)
	      {
		ay_viewt_griddify(togl,&winx,&winy);
	      }

	    oldwinx = winx;
	    oldwiny = winy;
	  }

    }
  else
    {
      ay_error(AY_EARGS, fname,
		 "\\[-start $winx $winy|-winxy $winx $winy\\]");
      return TCL_OK;
    }

  if(!ay_selection)
    {
      return TCL_OK;
    }

  if((oldwinx == winx) && (oldwiny == winy))
    return TCL_OK;

  glGetIntegerv(GL_VIEWPORT, vp);
  glGetDoublev(GL_PROJECTION_MATRIX, mp);
  glMatrixMode(GL_MODELVIEW);

  /* scale the selected points */
  while(sel)
    {
      o = sel->object;
      if(o->selp)
	{
	  glPushMatrix();
	  if(view->type != AY_VTTRIM)
	    ay_trafo_getall(ay_currentlevel->next);
	   glTranslated(o->movx, o->movy, o->movz);
	   ay_quat_torotmatrix(o->quat, m);
	   glMultMatrixd(m);
	   glScaled (o->scalx, o->scaly, o->scalz);
	   glGetDoublev(GL_MODELVIEW_MATRIX, mm);
	  glPopMatrix();
	  gluProject(0.0,0.0,0.0,mm,mp,vp,&owinx,&owiny,&owinz);

	  if(view->type == AY_VTSIDE)
	    {
	      dscalx = 1.0;
	    }
	  else
	    {
	      if(oldwinx-owinx)
		{
		  dscalx = (winx-owinx)/(oldwinx-owinx);
		}
	      else
		{
		  dscalx = 1.0;
		}
	    }
	 
	  point = o->selp;
	      
	  glPushMatrix();
	   glLoadIdentity();

	   ay_trafo_getallisr(ay_currentlevel->next);
	   
	   ay_quat_to_euler(o->quat,euler);
	   glRotatef(AY_R2D(euler[0]), 0.0, 0.0, 1.0);
	   glRotatef(AY_R2D(euler[1]), 0.0, 1.0, 0.0);
	   glRotatef(AY_R2D(euler[2]), 1.0, 0.0, 0.0);
	   glScaled(dscalx,1.0,1.0);
	   glRotatef(AY_R2D(-euler[2]), 1.0, 0.0, 0.0);
	   glRotatef(AY_R2D(-euler[1]), 0.0, 1.0, 0.0);
	   glRotatef(AY_R2D(-euler[0]), 0.0, 0.0, 1.0);
	   ay_trafo_getallsr(ay_currentlevel->next);

	   glGetDoublev(GL_MODELVIEW_MATRIX, mm);
	  glPopMatrix();

	  while(point)
	    {
	      AY_APTRAN3(tpoint,point->point,mm);
	      memcpy(point->point,tpoint,3*sizeof(double));

	      point = point->next;
	    }

	  o->modified = AY_TRUE;
	  
	}
     
      sel = sel->next;
    }

  oldwinx = winx;
  oldwiny = winy;

  if(!ay_prefs.lazynotify)
    ay_notify_parent();

  ay_toglcb_display(togl);

 return TCL_OK;
} /* ay_oact_sc1DXWcb */


/* selected points only */
int
ay_oact_sc1DYWcb(struct Togl *togl, int argc, char *argv[])
{
 Tcl_Interp *interp = Togl_Interp (togl);
 ay_view_object *view = Togl_GetClientData(togl);
 double height = Togl_Height(togl);
 static double oldwinx = 0.0, oldwiny = 0.0;
 double winx = 0.0, winy = 0.0, dscaly = 1.0;
 double tpoint[4]={0};
 GLdouble mp[16], mm[16], owinx, owiny, owinz;
 GLdouble m[16];
 GLint vp[4];
 ay_list_object *sel = ay_selection;
 ay_object *o = NULL;
 ay_point_object *point = NULL;
 char fname[] = "scale1DY_world";
 double euler[3] = {0};

  if(view->type == AY_VTPERSP)
    {
      ay_error(AY_ERROR, fname, "Operation not allowed in perspective views.");
      return TCL_OK;
    }

  /* parse args */
  if(argc >= 4)
    {
      if(!strcmp(argv[2],"-winxy"))
      {
	Tcl_GetDouble(interp, argv[3], &winx);
	Tcl_GetDouble(interp, argv[4], &winy);
	if(view->grid != 0.0)
	  {
	    ay_viewt_griddify(togl,&winx,&winy);
	  }	
      }
      else
	if(!strcmp(argv[2],"-start"))
	  {
	    
	    if(!ay_selection)
	      {
		ay_error(AY_ENOSEL, fname, NULL);
		return TCL_OK;
	      }

	    Tcl_GetDouble(interp, argv[3], &winx);
	    Tcl_GetDouble(interp, argv[4], &winy);
	    if(view->grid != 0.0)
	      {
		ay_viewt_griddify(togl,&winx,&winy);
	      }

	    oldwinx = winx;
	    oldwiny = winy;
	  }

    }
  else
    {
      ay_error(AY_EARGS, fname,
		 "\\[-start $winx $winy|-winxy $winx $winy\\]");
      return TCL_OK;
    }

  if(!ay_selection)
    {
      return TCL_OK;
    }

  if((oldwinx == winx) && (oldwiny == winy))
    return TCL_OK;

  glGetIntegerv(GL_VIEWPORT, vp);
  glGetDoublev(GL_PROJECTION_MATRIX, mp);
  glMatrixMode(GL_MODELVIEW);


  /* scale the selected points */
  while(sel)
    {
      o = sel->object;
      if(o->selp)
	{
	  glPushMatrix();
	  if(view->type != AY_VTTRIM)
	    ay_trafo_getall(ay_currentlevel->next);
	   glTranslated(o->movx, o->movy, o->movz);
	   ay_quat_torotmatrix(o->quat, m);
	   glMultMatrixd(m);
	   glScaled (o->scalx, o->scaly, o->scalz);
	   glGetDoublev(GL_MODELVIEW_MATRIX, mm);
	  glPopMatrix();
	  gluProject(0.0,0.0,0.0,mm,mp,vp,&owinx,&owiny,&owinz);

	  owiny = height - owiny;
	  if(view->type == AY_VTTOP)
	    {
	      dscaly = 1.0;
	    }
	  else
	    {
	      if(oldwiny-owiny)
		{
		  dscaly = (winy-owiny)/(oldwiny-owiny);
		}
	      else
		{
		  dscaly = 1.0;
		}
	    }
	 
	  point = o->selp;
	      
	  glPushMatrix();
	   glLoadIdentity();

	   ay_trafo_getallisr(ay_currentlevel->next);

	   ay_quat_to_euler(o->quat,euler);
	   glRotatef(AY_R2D(euler[0]), 0.0, 0.0, 1.0);
	   glRotatef(AY_R2D(euler[1]), 0.0, 1.0, 0.0);
	   glRotatef(AY_R2D(euler[2]), 1.0, 0.0, 0.0);
	   glScaled(1.0,dscaly,1.0);
	   glRotatef(AY_R2D(-euler[2]), 1.0, 0.0, 0.0);
	   glRotatef(AY_R2D(-euler[1]), 0.0, 1.0, 0.0);
	   glRotatef(AY_R2D(-euler[0]), 0.0, 0.0, 1.0);
	   ay_trafo_getallsr(ay_currentlevel->next);

	   glGetDoublev(GL_MODELVIEW_MATRIX, mm);
	  glPopMatrix();

	  while(point)
	    {
	      AY_APTRAN3(tpoint,point->point,mm);
	      memcpy(point->point,tpoint,3*sizeof(double));

	      point = point->next;
	    }

	  o->modified = AY_TRUE;

	}

      sel = sel->next;
    }

  oldwinx = winx;
  oldwiny = winy;

  if(!ay_prefs.lazynotify)
    ay_notify_parent();

  ay_toglcb_display(togl);

 return TCL_OK;
} /* ay_oact_sc1DYWcb */


/* selected points only */
int
ay_oact_sc1DZWcb(struct Togl *togl, int argc, char *argv[])
{
 Tcl_Interp *interp = Togl_Interp (togl);
 ay_view_object *view = Togl_GetClientData(togl);
 double height = Togl_Height(togl);
 static double oldwinx = 0.0, oldwiny = 0.0;
 double winx = 0.0, winy = 0.0, dscalz=1.0;
 double tpoint[4]={0};
 GLdouble mp[16], mm[16], owinx, owiny, owinz;
 GLdouble m[16];
 GLint vp[4];
 ay_list_object *sel = ay_selection;
 ay_object *o = NULL;
 ay_point_object *point = NULL;
 char fname[] = "scale1DZ_world";
 double euler[3] = {0};

  if(view->type == AY_VTPERSP)
    {
      ay_error(AY_ERROR, fname, "Operation not allowed in perspective views.");
      return TCL_OK;
    }

  /* parse args */
  if(argc >= 4)
    {
      if(!strcmp(argv[2],"-winxy"))
      {
	Tcl_GetDouble(interp, argv[3], &winx);
	Tcl_GetDouble(interp, argv[4], &winy);
	if(view->grid != 0.0)
	  {
	    ay_viewt_griddify(togl,&winx,&winy);
	  }	
      }
      else
	if(!strcmp(argv[2],"-start"))
	  {
	    
	    if(!ay_selection)
	      {
		ay_error(AY_ENOSEL, fname, NULL);
		return TCL_OK;
	      }

	    Tcl_GetDouble(interp, argv[3], &winx);
	    Tcl_GetDouble(interp, argv[4], &winy);
	    if(view->grid != 0.0)
	      {
		ay_viewt_griddify(togl,&winx,&winy);
	      }

	    oldwinx = winx;
	    oldwiny = winy;
	  }

    }
  else
    {
      ay_error(AY_EARGS, fname,
		 "\\[-start $winx $winy|-winxy $winx $winy\\]");
      return TCL_OK;
    }

  if(!ay_selection)
    {
      return TCL_OK;
    }

  if((oldwinx == winx) && (oldwiny == winy))
    return TCL_OK;

  glGetIntegerv(GL_VIEWPORT, vp);
  glGetDoublev(GL_PROJECTION_MATRIX, mp);
  glMatrixMode(GL_MODELVIEW);


  /* scale the selected points */
  while(sel)
    {
      o = sel->object;
      if(o->selp)
	{
	  glPushMatrix();
	  if(view->type != AY_VTTRIM)
	    ay_trafo_getall(ay_currentlevel->next);
	   glTranslated(o->movx, o->movy, o->movz);
	   ay_quat_torotmatrix(o->quat, m);
	   glMultMatrixd(m);
	   glScaled (o->scalx, o->scaly, o->scalz);
	   glGetDoublev(GL_MODELVIEW_MATRIX, mm);
	  glPopMatrix();
	  gluProject(0.0,0.0,0.0,mm,mp,vp,&owinx,&owiny,&owinz);

	  owiny = height - owiny;
	  if(view->type == AY_VTFRONT || view->type == AY_VTTRIM)
	    {
	      dscalz = 1.0;
	    }
	  else
	    {
	      if(view->type == AY_VTTOP)
		{
		  if(oldwiny-owiny)
		    dscalz = (winy-owiny)/(oldwiny-owiny);
		  else
		    dscalz = 1.0;
		}
	      else
		{
		  if(view->type == AY_VTSIDE)
		    {
		      if(oldwinx-owinx)
			{
			  dscalz = (winx-owinx)/(oldwinx-owinx);
			}
		      else
			{
			  dscalz = 1.0;
			}
		    }
		}
	    }

	  point = o->selp;
	      
	  glPushMatrix();
	   glLoadIdentity();

	   ay_trafo_getallisr(ay_currentlevel->next);

	   ay_quat_to_euler(o->quat,euler);
	   glRotatef(AY_R2D(euler[0]), 0.0, 0.0, 1.0);
	   glRotatef(AY_R2D(euler[1]), 0.0, 1.0, 0.0);
	   glRotatef(AY_R2D(euler[2]), 1.0, 0.0, 0.0);
	   glScaled(1.0,1.0,dscalz);
	   glRotatef(AY_R2D(-euler[2]), 1.0, 0.0, 0.0);
	   glRotatef(AY_R2D(-euler[1]), 0.0, 1.0, 0.0);
	   glRotatef(AY_R2D(-euler[0]), 0.0, 0.0, 1.0);
	   ay_trafo_getallsr(ay_currentlevel->next);
	   
	   glGetDoublev(GL_MODELVIEW_MATRIX, mm);
	  glPopMatrix();

	  while(point)
	    {
	      AY_APTRAN3(tpoint,point->point,mm);
	      memcpy(point->point,tpoint,3*sizeof(double));

	      point = point->next;
	    }

	  o->modified = AY_TRUE;
	  
	}

      sel = sel->next;
    }

  oldwinx = winx;
  oldwiny = winy;

  if(!ay_prefs.lazynotify)
    ay_notify_parent();

  ay_toglcb_display(togl);

 return TCL_OK;
} /* ay_oact_sc1DZWcb */
#endif

/* ay_oact_sc2Dcb:
 *
 */
int
ay_oact_sc2Dcb(struct Togl *togl, int argc, char *argv[])
{
 Tcl_Interp *interp = Togl_Interp (togl);
 ay_view_object *view = (ay_view_object *)Togl_GetClientData(togl);
 double height = Togl_Height(togl);
 static double oldwinx = 0.0, oldwiny = 0.0;
 double winx = 0.0, winy = 0.0, dscal = 1.0;
 double tpoint[4]={0}, t1, t2, v1[2], v2[2];
 GLdouble mp[16], mm[16], owinx, owiny, owinz;
 GLdouble m[16];
 GLint vp[4];
 ay_list_object *sel = ay_selection;
 ay_object *o = NULL;
 ay_point_object *point = NULL;
 char fname[] = "scale2D_object";

  if(view->type == AY_VTPERSP)
    {
      ay_error(AY_ERROR, fname, "Operation not allowed in perspective views.");
      return TCL_OK;
    }

  /* parse args */
  if(argc >= 4)
    {
      if(!strcmp(argv[2],"-winxy"))
      {
	Tcl_GetDouble(interp, argv[3], &winx);
	Tcl_GetDouble(interp, argv[4], &winy);
	if(view->usegrid)
	  {
	    ay_viewt_griddify(togl,&winx,&winy);
	  }	
      }
      else
	if(!strcmp(argv[2],"-start"))
	  {

	    if(!ay_selection)
	      {
		ay_error(AY_ENOSEL, fname, NULL);
		return TCL_OK;
	      }

	    Tcl_GetDouble(interp, argv[3], &winx);
	    Tcl_GetDouble(interp, argv[4], &winy);
	    if(view->usegrid)
	      {
		ay_viewt_griddify(togl,&winx,&winy);
	      }

	    oldwinx = winx;
	    oldwiny = winy;
	  }

    }
  else
    {
      ay_error(AY_EARGS, fname,
		 "\\[-start $winx $winy|-winxy $winx $winy\\]");
      return TCL_OK;
    }

  if(!ay_selection)
    {
      return TCL_OK;
    }

  if((oldwinx == winx) && (oldwiny == winy))
    return TCL_OK;

  glGetIntegerv(GL_VIEWPORT, vp);
  glGetDoublev(GL_PROJECTION_MATRIX, mp);
  glMatrixMode(GL_MODELVIEW);

  /* scale the object(s) / selected points */
  while(sel)
    {
      o = sel->object;
      if(o)
	{
	  glPushMatrix();
	  if(view->type != AY_VTTRIM)
	    ay_trafo_getall(ay_currentlevel->next);
	   glTranslated(o->movx, o->movy, o->movz);
	   ay_quat_torotmatrix(o->quat, m);
	   glMultMatrixd(m);
	   glScaled (o->scalx, o->scaly, o->scalz);
	   glGetDoublev(GL_MODELVIEW_MATRIX, mm);
	  glPopMatrix();
	  gluProject(0.0,0.0,0.0,mm,mp,vp,&owinx,&owiny,&owinz);

	  owiny = height - owiny;
	 
	  v1[0] = (oldwinx-owinx);
	  v1[1] = (oldwiny-owiny);

	  v2[0] = (winx-owinx);
	  v2[1] = (winy-owiny);
	      
	  t1 = AY_V2LEN(v1);
	  t2 = AY_V2LEN(v2);

	  if(t1)
	    dscal = t2/t1;
	  else
	    dscal = 1.0;
	      
	  if(o->selp)
	    {
	      point = o->selp;
	      
	      glPushMatrix();
	       glLoadIdentity();

	       if(!view->local)
		 ay_trafo_getallisr(ay_currentlevel->next);

	       switch(view->type)
		 {
		 case AY_VTFRONT:
		 case AY_VTTRIM:
		   glScaled(dscal,dscal,1.0);
		   break;
		 case AY_VTSIDE:
		   glScaled(1.0,dscal,dscal);
		   break;
		 case AY_VTTOP:
		   glScaled(dscal,1.0,dscal);
		   break;
		 default:
		   break;
		 }

	       if(!view->local)
		 ay_trafo_getallsr(ay_currentlevel->next);

	       glGetDoublev(GL_MODELVIEW_MATRIX, mm);
	      glPopMatrix();

	      while(point)
		{
		  AY_APTRAN3(tpoint,point->point,mm);
		  memcpy(point->point,tpoint,3*sizeof(double));

		  point = point->next;
		}

	      o->modified = AY_TRUE;
	      ay_notify_force(o);

	    }
	  else
	    {
	       switch(view->type)
		 {
		 case AY_VTFRONT:
		 case AY_VTTRIM:
		   o->scalx *= dscal;
		   o->scaly *= dscal;
		   break;
		 case AY_VTSIDE:
		   o->scaly *= dscal;
		   o->scalz *= dscal;
		   break;
		 case AY_VTTOP:
		   o->scalx *= dscal;
		   o->scalz *= dscal;
		   break;
		 default:
		   break;
		 }

	    }
	}
      
      sel = sel->next;
    }

  oldwinx = winx;
  oldwiny = winy;

  if(!ay_prefs.lazynotify)
    ay_notify_parent();

  ay_toglcb_display(togl);

 return TCL_OK;
} /* ay_oact_sc2Dcb */


/* ay_oact_sc3Dcb:
 *
 */
int
ay_oact_sc3Dcb(struct Togl *togl, int argc, char *argv[])
{
 Tcl_Interp *interp = Togl_Interp (togl);
 ay_view_object *view = (ay_view_object *)Togl_GetClientData(togl);
 double height = Togl_Height(togl);
 static double oldwinx = 0.0, oldwiny = 0.0;
 double winx = 0.0, winy = 0.0, dscal = 1.0;
 double tpoint[4]={0}, t1, t2, v1[2], v2[2];
 GLdouble mp[16], mm[16], owinx, owiny, owinz;
 GLdouble m[16];
 GLint vp[4];
 ay_list_object *sel = ay_selection;
 ay_object *o = NULL;
 ay_point_object *point = NULL;
 char fname[] = "scale3D_object";

  if(view->type == AY_VTPERSP)
    {
      ay_error(AY_ERROR, fname, "Operation not allowed in perspective views.");
      return TCL_OK;
    }

  /* parse args */
  if(argc >= 4)
    {
      if(!strcmp(argv[2],"-winxy"))
      {
	Tcl_GetDouble(interp, argv[3], &winx);
	Tcl_GetDouble(interp, argv[4], &winy);
	if(view->usegrid)
	  {
	    ay_viewt_griddify(togl,&winx,&winy);
	  }	
      }
      else
	if(!strcmp(argv[2],"-start"))
	  {

	    if(!ay_selection)
	      {
		ay_error(AY_ENOSEL, fname, NULL);
		return TCL_OK;
	      }

	    Tcl_GetDouble(interp, argv[3], &winx);
	    Tcl_GetDouble(interp, argv[4], &winy);
	    if(view->usegrid)
	      {
		ay_viewt_griddify(togl,&winx,&winy);
	      }

	    oldwinx = winx;
	    oldwiny = winy;
	  }

    }
  else
    {
      ay_error(AY_EARGS, fname,
		 "\\[-start $winx $winy|-winxy $winx $winy\\]");
      return TCL_OK;
    }

  if(!ay_selection)
    {
      return TCL_OK;
    }

  if((oldwinx == winx) && (oldwiny == winy))
    return TCL_OK;

  glGetIntegerv(GL_VIEWPORT, vp);
  glGetDoublev(GL_PROJECTION_MATRIX, mp);
  glMatrixMode(GL_MODELVIEW);

  /* scale the object(s) / selected points */
  while(sel)
    {
      o = sel->object;
      if(o)
	{
	  glPushMatrix();
	  if(view->type != AY_VTTRIM)
	    ay_trafo_getall(ay_currentlevel->next);
	   glTranslated(o->movx, o->movy, o->movz);
	   ay_quat_torotmatrix(o->quat, m);
	   glMultMatrixd(m);
	   glScaled (o->scalx, o->scaly, o->scalz);
	   glGetDoublev(GL_MODELVIEW_MATRIX, mm);
	  glPopMatrix();
	  gluProject(0.0,0.0,0.0,mm,mp,vp,&owinx,&owiny,&owinz);

	  owiny = height - owiny;
	 
	  v1[0] = (oldwinx-owinx);
	  v1[1] = (oldwiny-owiny);

	  v2[0] = (winx-owinx);
	  v2[1] = (winy-owiny);
	      
	  t1 = AY_V2LEN(v1);
	  t2 = AY_V2LEN(v2);

	  if(t1)
	    dscal = t2/t1;
	  else
	    dscal = 1.0;
	      
	  if(o->selp)
	    {
	      point = o->selp;
	      
	      glPushMatrix();
	       glLoadIdentity();

	       if(!view->local)
		 ay_trafo_getallisr(ay_currentlevel->next);

	       glScaled(dscal,dscal,dscal);

	       if(!view->local)
		 ay_trafo_getallsr(ay_currentlevel->next);

	       glGetDoublev(GL_MODELVIEW_MATRIX, mm);
	      glPopMatrix();

	      while(point)
		{
		  AY_APTRAN3(tpoint,point->point,mm);
		  memcpy(point->point,tpoint,3*sizeof(double));

		  point = point->next;
		}

	      o->modified = AY_TRUE;
	      ay_notify_force(o);

	    }
	  else
	    {
	      o->scalx *= dscal;
	      o->scaly *= dscal;
	      o->scalz *= dscal;
	    }
	}
      
      sel = sel->next;
    }

  oldwinx = winx;
  oldwiny = winy;

  if(!ay_prefs.lazynotify)
    ay_notify_parent();

  ay_toglcb_display(togl);

 return TCL_OK;
} /* ay_oact_sc3Dcb */


/* ay_oact_str2Dcb:
 *
 */
int
ay_oact_str2Dcb(struct Togl *togl, int argc, char *argv[])
{
 ay_view_object *view = (ay_view_object *)Togl_GetClientData(togl);

 switch(view->type)
   {
   case AY_VTFRONT:
   case AY_VTTRIM:
     ay_oact_sc1DXcb(togl, argc, argv);
     ay_oact_sc1DYcb(togl, argc, argv);
     break;
   case AY_VTSIDE:
     ay_oact_sc1DZcb(togl, argc, argv);
     ay_oact_sc1DYcb(togl, argc, argv);
     break;
   case AY_VTTOP:
     ay_oact_sc1DXcb(togl, argc, argv);
     ay_oact_sc1DZcb(togl, argc, argv);
     break;

   }

 return TCL_OK;
} /* ay_oact_sc3Dcb */


