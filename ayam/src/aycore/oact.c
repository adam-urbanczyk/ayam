/*
 * Ayam, a free 3D modeler for the RenderMan interface.
 *
 * Ayam is copyrighted 1998-2001 by Randolf Schultz
 * (randolf.schultz@gmail.com) and others.
 *
 * All rights reserved.
 *
 * See the file License for details.
 *
 */

#include "ayam.h"

/* oact.c - object related interactive actions */


/* ay_oact_parseargs:
 *  helper for all actions; check view type, check object selection,
 *  parse args
 */
int
ay_oact_parseargs(struct Togl *togl, int argc, char *argv[], char *fname,
		  double *winx, double *winy, double *owinx, double *owiny)
{
 Tcl_Interp *interp = Togl_Interp(togl);
 ay_view_object *view = (ay_view_object *)Togl_GetClientData(togl);

  if(view->type == AY_VTPERSP)
    {
      ay_error(AY_ERROR, fname, "Operation not allowed in perspective views.");
      return AY_ERROR;
    }

  if(argc >= 4)
    {
      if(argv[2][0] && (argv[2][1] == 'w') /*!strcmp(argv[2],"-winxy")*/)
	{
	  if(!ay_selection)
	    {
	      /* avoid too many error messages: return silently */
	      return AY_ERROR;
	    }

	  Tcl_GetDouble(interp, argv[3], winx);
	  Tcl_GetDouble(interp, argv[4], winy);

	  if(view->usegrid)
	    {
	      ay_viewt_griddify(togl, winx, winy);
	    }
	}
      else
	{
	  if(argv[2][0] && (argv[2][1] == 's') /*!strcmp(argv[2],"-start")*/)
	    {
	      if(!ay_selection)
		{
		  ay_error(AY_ENOSEL, fname, NULL);
		  return AY_ERROR;
		}

	      Tcl_GetDouble(interp, argv[3], winx);
	      Tcl_GetDouble(interp, argv[4], winy);
	      if(view->usegrid)
		{
		  ay_viewt_griddify(togl, winx, winy);
		}

	      *owinx = *winx;
	      *owiny = *winy;
	    }
	}
    }
  else
    {
      ay_error(AY_EARGS, fname,
		 "\\[-start $winx $winy|-winxy $winx $winy\\]");
      return TCL_OK;
    }

 return AY_OK;
} /* ay_oact_parseargs */


/* ay_oact_movetcb:
 *
 */
int
ay_oact_movetcb(struct Togl *togl, int argc, char *argv[])
{
 int ay_status = AY_OK;
 Tcl_Interp *interp = Togl_Interp(togl);
 ay_view_object *view = (ay_view_object *)Togl_GetClientData(togl);
 ay_point *point = NULL;
 static double oldwinx = 0.0, oldwiny = 0.0;
 static GLdouble m[16] = {0};
 static int rest = 0;
 double winx = 0.0, winy = 0.0;
 double dx = 0, dy = 0, dz = 0;
 double sdx = 0, sdy = 0, sdz = 0;
 double v1[3] = {0}, v2[3] = {0};
 double euler[3] = {0};
 GLdouble mm[16];
 ay_list_object *sel = ay_selection;
 ay_object *o = NULL;
 char fname[] = "move_act";

  /* parse args */
  ay_status = ay_oact_parseargs(togl, argc, argv, fname,
				&winx, &winy, &oldwinx, &oldwiny);

  if(ay_status)
    {
      return TCL_OK;
    }

  /* if first argument is -start ... */
  if((argc >= 4) && argv[2][0] && (argv[2][1] == 's'))
    {
      /* if we have the restrict argument ... */
      if(argc > 5)
	{
	  /* get restrict axis */
	  Tcl_GetInt(interp, argv[5], &rest);
	}
      else
	{
	  rest = 0;
	}
    }

  /* bail out, as long as we stay in the same grid cell */
  if((oldwinx == winx) && (oldwiny == winy))
    {
      return TCL_OK;
    }

  /* get real direction of current level coordinate system */
  glMatrixMode(GL_MODELVIEW);
   glPushMatrix();
   /*	    if(view->type != AY_VTTRIM)*/
   if(!view->local)
     {
       if(ay_currentlevel->object != ay_root)
	 {
	   ay_trafo_getallisr(ay_currentlevel->next);
	 }
     }
   else
     {
       if(ay_currentlevel->object != ay_root)
	 {
	   ay_trafo_getallis(ay_currentlevel->next);
	 }
     } /* if */
   glGetDoublev(GL_MODELVIEW_MATRIX, m);
  glPopMatrix();

  /* calc dx, dy, dz */
  dx = -(oldwinx - winx) * view->conv_x;
  dy = (oldwiny - winy) * view->conv_y;

  /* modify dx/dy/dz according to view type */
  if((view->type == AY_VTFRONT) || (view->type == AY_VTTRIM))
    {
      /* restrict to X? */
      if(rest == 1)
	{
	  dy = 0.0;
	}
      /* restrict to Y? */
      if(rest == 2)
	{
	  dx = 0.0;
	}
      /* restrict to Z? */
      if(rest == 3)
	{
	  dx = 0.0;
	  dy = 0.0;
	}
      v2[0] = dx;
      v2[1] = dy;
      v2[2] = 0.0;
      AY_APTRAN3(v1, v2, m)
      dx = v1[0];
      dy = v1[1];
      dz = v1[2];
    }

  if(view->type == AY_VTSIDE)
    {
      /* restrict to Z? */
      if(rest == 3)
	{
	  dy = 0.0;
	}
      /* restrict to Y? */
      if(rest == 2)
	{
	  dx = 0.0;
	}
      /* restrict to X? */
      if(rest == 1)
	{
	  dx = 0.0;
	  dy = 0.0;
	}
      v2[0] = 0.0;
      v2[1] = dy;
      v2[2] = -dx;
      AY_APTRAN3(v1, v2, m)
      dx = v1[0];
      dy = v1[1];
      dz = v1[2];
    }

  if(view->type == AY_VTTOP)
    {
      /* restrict to X? */
      if(rest == 1)
	{
	  dy = 0.0;
	}
      /* restrict to Z? */
      if(rest == 3)
	{
	  dx = 0.0;
	}
      /* restrict to Y? */
      if(rest == 2)
	{
	  dx = 0.0;
	  dy = 0.0;
	}
      v2[0] = dx;
      v2[1] = 0.0;
      v2[2] = -dy;
      AY_APTRAN3(v1, v2, m)
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

      if(view->transform_points)
	{
	  if(o->selp && (!o->selp->readonly))
	    {
	      glMatrixMode(GL_MODELVIEW);
	      glPushMatrix();
	       glScaled(1.0/o->scalx, 1.0/o->scaly, 1.0/o->scalz);
	       if(!view->aligned)
		 {
		   ay_quat_toeuler(o->quat, euler);
		   glRotated(AY_R2D(euler[0]), 0.0, 0.0, 1.0);
		   glRotated(AY_R2D(euler[1]), 0.0, 1.0, 0.0);
		   glRotated(AY_R2D(euler[2]), 1.0, 0.0, 0.0);
		 }
	       glGetDoublev(GL_MODELVIEW_MATRIX, mm);
	      glPopMatrix();

	      v2[0] = dx;
	      v2[1] = dy;
	      v2[2] = dz;
	      AY_APTRAN3(v1, v2, mm)
	      sdx = v1[0];
	      sdy = v1[1];
	      sdz = v1[2];

	      point = sel->object->selp;
	      while(point)
		{
		  point->point[0] += sdx;
		  point->point[1] += sdy;
		  point->point[2] += sdz;

		  point = point->next;
		} /* while */

	      o->modified = AY_TRUE;
	      ay_notify_force(o);
	    }
	}
      else
	{
	  o = sel->object;
	  if(o)
	    {
	      o->movx += dx;
	      o->movy += dy;
	      o->movz += dz;
	      o->modified = AY_TRUE;
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
	  ay_notify_parent();
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
 int ay_status = AY_OK;
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
 ay_point *point = NULL;
 char fname[] = "rotate_act";

  /* parse args */
  ay_status = ay_oact_parseargs(togl, argc, argv, fname,
				&winx, &winy, &oldwinx, &oldwiny);

  if(ay_status)
    {
      return TCL_OK;
    }

  /* bail out, as long as we stay in the same grid cell */
  if((oldwinx == winx) && (oldwiny == winy))
    {
      return TCL_OK;
    }

  glGetIntegerv(GL_VIEWPORT, vp);
  glGetDoublev(GL_PROJECTION_MATRIX, mp);
  glMatrixMode(GL_MODELVIEW);

  /* rotate the object(s) */
  while(sel)
    {
      o = sel->object;

      /* so that we may use continue; */
      sel = sel->next;

      if(o)
	{
	  glPushMatrix();
	   if(ay_currentlevel->object != ay_root)
	     {
	       ay_trafo_getall(ay_currentlevel->next);
	     }
	   glTranslated(o->movx, o->movy, o->movz);
	   glScaled(o->scalx, o->scaly, o->scalz);
	   glGetDoublev(GL_MODELVIEW_MATRIX, mm);
	  glPopMatrix();
	  if(GL_FALSE == gluProject(0.0,0.0,0.0,mm,mp,vp,&owinx,&owiny,&owinz))
	    {
	      return TCL_OK;
	    }

	  owiny = height-owiny;

	  v1[0] = oldwinx-owinx;
	  v1[1] = oldwiny-owiny;
	  /* bail out, if we get too near the mark */
	  if((fabs(v1[0]) < AY_EPSILON) && (fabs(v1[1]) < AY_EPSILON))
	    continue;
	  alpha = AY_R2D(acos(v1[0]/AY_V2LEN(v1)));
	  if(v1[1] < 0.0)
	    alpha = 360.0-alpha;

	  v2[0] = winx-owinx;
	  v2[1] = winy-owiny;
	  /* bail out, if we get too near the mark */
	  if((fabs(v2[0]) < AY_EPSILON) && (fabs(v2[1]) < AY_EPSILON))
	    continue;

	  beta = AY_R2D(acos(v2[0]/AY_V2LEN(v2)));
	  if(v2[1]<0.0)
	    beta = 360.0-beta;

	  angle = beta - alpha;

	  if(view->transform_points)
	    {
	      if(o->selp && (!o->selp->readonly))
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
		} /* if */
	    }
	  else
	    {
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
	      o->modified = AY_TRUE;
	    } /* if */

	} /* if */

    } /* while */

  if(!ay_prefs.lazynotify)
    {
      ay_notify_parent();
    }

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
 int ay_status = AY_OK;
 ay_view_object *view = (ay_view_object *)Togl_GetClientData(togl);
 static double oldwinx = 0.0, oldwiny = 0.0;
 double winx = 0.0, winy = 0.0;
 double ax = 0.0, ay = 0.0;
 double angle = 0.0, tpoint[4] = {0};
 double xaxis[3] = {1.0,0.0,0.0};
 double yaxis[3] = {0.0,1.0,0.0};
 double zaxis[3] = {0.0,0.0,1.0};
 double /*dx, dy, dax, day,*/ zangle, yangle, xangle;
 double quat[4];
 double v1[3] = {0}, v2[3] = {0}, v3[3], v4[3];
 double alpha, beta;
 GLdouble m[16], mm[16], mmi[16];
 ay_list_object *sel = ay_selection;
 ay_object *o = NULL;
 ay_point *point = NULL;
 char fname[] = "rotateA_act";

  /* parse args */
  ay_status = ay_oact_parseargs(togl, argc, argv, fname,
				&winx, &winy, &oldwinx, &oldwiny);

  if(ay_status)
    {
      return TCL_OK;
    }

  if(!view->drawmark)
    {
      /* if view->drawmark is not enabled some other action
	 changed view trafos so that the point is not valid
	 anymore and we should request a new point */
      ay_error(AY_ERROR, fname,
	       "Lost mark. Please restart this action!");
      return TCL_OK;
    }

  /* bail out, as long as we stay in the same grid cell */
  if((oldwinx == winx) && (oldwiny == winy))
    {
      return TCL_OK;
    }

  /* calculate rotation angle from window coordinates
     of picked points (old/start and current from drag)
     and point to rotate about */
  ax = view->markx;
  ay = view->marky;

  v1[0] = oldwinx - ax;
  v1[1] = oldwiny - ay;
  /* bail out, if we get too near the mark */
  if((fabs(v1[0]) < AY_EPSILON) && (fabs(v1[1]) < AY_EPSILON))
    {
      return TCL_OK;
    }
  alpha = AY_R2D(acos(v1[0]/AY_V2LEN(v1)));
  if(v1[1] < 0.0)
    alpha = 360.0 - alpha;

  v2[0] = winx - ax;
  v2[1] = winy - ay;
  if((fabs(v2[0]) < AY_EPSILON) && (fabs(v2[1]) < AY_EPSILON))
    {
      return TCL_OK;
    }
  beta = AY_R2D(acos(v2[0]/AY_V2LEN(v2)));
  if(v2[1] < 0.0)
    beta = 360.0 - beta;

  angle = beta - alpha;

  /* rotate the selected object(s)/selected points */
  glMatrixMode(GL_MODELVIEW);
  while(sel)
    {
      o = sel->object;
      if(o)
	{
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

	      AY_APTRAN3(v3, zaxis, mm);
	      v2[0] = v3[2];
	      v2[1] = v3[1];
	      xangle = AY_R2D(acos(v2[0]/AY_V2LEN(v2)));
	      if(v2[1]>0.0)
		xangle = 360.0-xangle;

	      /* transform ax/ay to object space */
	      glPushMatrix();
	       if(ay_currentlevel->object != ay_root)
		 {
		   ay_trafo_getall(ay_currentlevel->next);
		 }
	       glTranslated(o->movx, o->movy, o->movz);
	       glRotated(xangle, 1.0, 0.0, 0.0);
	       if(view->transform_points)
		 {
		   glScaled(o->scalx, o->scaly, o->scalz);
		 }
	       glGetDoublev(GL_MODELVIEW_MATRIX, mm);
	      glPopMatrix();
	      ay_trafo_invmatrix4(mm, mmi);
	      AY_APTRAN3(v4, view->markworld, mmi);

	      glPushMatrix();
	       glLoadIdentity();
	       glRotated(xangle, 1.0, 0.0, 0.0);
	       glTranslated(0.0, -v4[1], -v4[2]);
	       glRotated(-angle,1.0,0.0,0.0);
	       glTranslated(0.0, v4[1], v4[2]);
	       glGetDoublev(GL_MODELVIEW_MATRIX, mm);
	      glPopMatrix();

	      memset(v1, 0, 3*sizeof(double));

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

	      /* transform ax/ay to object space */
	      glPushMatrix();
	       if(ay_currentlevel->object != ay_root)
		{
		  ay_trafo_getall(ay_currentlevel->next);
		}
	       glTranslated(o->movx, o->movy, o->movz);
	       glRotated(zangle, 0.0, 0.0, 1.0);
	       if(view->transform_points)
		 {
		   glScaled(o->scalx, o->scaly, o->scalz);
		 }
	       glGetDoublev(GL_MODELVIEW_MATRIX, mm);
	      glPopMatrix();

	      ay_trafo_invmatrix4(mm,mmi);
	      AY_APTRAN3(v4,view->markworld,mmi);

	      glPushMatrix();
	       glLoadIdentity();
	       glRotated(zangle, 0.0, 0.0, 1.0);
	       glTranslated(-v4[0], -v4[1], 0.0);
	       glRotated(-angle,0.0,0.0,1.0);
	       glTranslated(v4[0], v4[1], 0.0);

	       glGetDoublev(GL_MODELVIEW_MATRIX, mm);
	      glPopMatrix();

	      memset(v1, 0, 3*sizeof(double));

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
	      if(v2[1]>0.0)
		yangle = 360.0-yangle;

	      /* transform ax/ay to object space */
	      glPushMatrix();
	       if(ay_currentlevel->object != ay_root)
		 {
		   ay_trafo_getall(ay_currentlevel->next);
		 }
	       glTranslated(o->movx, o->movy, o->movz);
	       glRotated(yangle, 0.0, 1.0, 0.0);
	       if(view->transform_points)
		 {
		   glScaled(o->scalx, o->scaly, o->scalz);
		 }
	       glGetDoublev(GL_MODELVIEW_MATRIX, mm);
	      glPopMatrix();
	      ay_trafo_invmatrix4(mm, mmi);
	      AY_APTRAN3(v4, view->markworld, mmi);

	      glPushMatrix();
	       glLoadIdentity();
	       glRotated(yangle, 0.0, 1.0, 0.0);
	       glTranslated(-v4[0], 0.0, -v4[2]);
	       glRotated(-angle,0.0,1.0,0.0);
	       glTranslated(v4[0], 0.0, v4[2]);
	       glGetDoublev(GL_MODELVIEW_MATRIX, mm);
	      glPopMatrix();

	      memset(v1, 0, 3*sizeof(double));

	      AY_APTRAN3(v2,v1,mm);
	      break;
	    default:
	      break;
	    } /* switch */

	  if(view->transform_points)
	    {
	      if(o->selp && (!o->selp->readonly))
		{
		  point = o->selp;

		  glPushMatrix();
		   glLoadIdentity();
		   switch(view->type)
		     {
		     case AY_VTSIDE:
		       /* rotate about x */
		       glTranslated(0.0,-v2[1],-v2[2]);
		       glRotated(-angle,1.0,0.0,0.0);
		       break;
		     case AY_VTFRONT:
		     case AY_VTTRIM:
		       /* rotate about z */
		       glTranslated(-v2[0],-v2[1],0.0);
		       glRotated(-angle,0.0,0.0,1.0);
		       break;
		     case AY_VTTOP:
		       /* rotate about y */
		       glTranslated(-v2[0],0.0,-v2[2]);
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
		} /* if */
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
	      o->modified = AY_TRUE;
	    } /* if */
	} /* if */

      sel = sel->next;
    } /* while */

  if(!ay_prefs.lazynotify)
    {
      ay_notify_parent();
    }

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
 int ay_status = AY_OK;
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
 ay_point *point = NULL;
 char fname[] = "scale1DX_act";

  /* parse args */
  ay_status = ay_oact_parseargs(togl, argc, argv, fname,
				&winx, &winy, &oldwinx, &oldwiny);

  if(ay_status)
    {
      return TCL_OK;
    }

  /* bail out, as long as we stay in the same grid cell */
  if((oldwinx == winx) && (oldwiny == winy))
    {
      return TCL_OK;
    }

  glGetIntegerv(GL_VIEWPORT, vp);
  glGetDoublev(GL_PROJECTION_MATRIX, mp);
  glMatrixMode(GL_MODELVIEW);

  /* scale the object(s) / selected points */
  while(sel)
    {
      o = sel->object;

      /* so that we may use continue; */
      sel = sel->next;

      if(o)
	{
	  glPushMatrix();

	   if(view->type != AY_VTTRIM)
	     {
	       if(ay_currentlevel->object != ay_root)
		 {
		   ay_trafo_getall(ay_currentlevel->next);
		 }
	     }

	   glTranslated(o->movx, o->movy, o->movz);
	   ay_quat_torotmatrix(o->quat, m);
	   glMultMatrixd(m);
	   glScaled(o->scalx, o->scaly, o->scalz);

	   glGetDoublev(GL_MODELVIEW_MATRIX, mm);
	  glPopMatrix();
	  if(GL_FALSE == gluProject(0.0,0.0,0.0,mm,mp,vp,&owinx,&owiny,&owinz))
	    {
	      return TCL_OK;
	    }
	  if(GL_FALSE == gluProject(1.0,0.0,0.0,mm,mp,vp,&vx[0],&vx[1],&vx[2]))
	    {
	      return TCL_OK;
	    }

	  owiny = height - owiny;
	  vx[1] = height - vx[1];

	  vx[0] -= owinx;
	  vx[1] -= owiny;

	  if((fabs(vx[0]) > AY_EPSILON) || (fabs(vx[1]) > AY_EPSILON))
	    {
	      alpha = AY_R2D(acos(vx[0]/AY_V2LEN(vx)));
	      if(vx[1] > 0.0)
		alpha = 360.0-alpha;

	      v1[0] = (oldwinx-owinx);
	      v1[1] = (oldwiny-owiny);
	      /* bail out, if we get too near the origin */
	      if((fabs(v1[0]) < AY_EPSILON) && (fabs(v1[1]) < AY_EPSILON))
		continue;

	      v2[0] = (winx-owinx);
 	      v2[1] = (winy-owiny);
	      /* bail out, if we get too near the origin */
	      if((fabs(v2[0]) < AY_EPSILON) && (fabs(v2[1]) < AY_EPSILON))
		continue;

	      beta = AY_R2D(acos(v1[0]/AY_V2LEN(v1)));
	      if(v1[1] < 0.0)
		beta = 360.0-beta;

	      gamma = AY_R2D(acos(v2[0]/AY_V2LEN(v2)));
	      if(v2[1] < 0.0)
		gamma = 360.0-gamma;

	      t1 = cos(AY_D2R(alpha)+AY_D2R(beta))*AY_V2LEN(v1);
	      t2 = cos(AY_D2R(alpha)+AY_D2R(gamma))*AY_V2LEN(v2);

	      if(fabs(t2) > AY_EPSILON && fabs(t1) > AY_EPSILON)
		dscalx = t2/t1;
	      else
		dscalx = 1.0;

	    }
	  else
	    dscalx = 1.0;

	  if(view->transform_points)
	    {
	      if(o->selp && (!o->selp->readonly))
		{
		  point = o->selp;

		  glPushMatrix();
		   glLoadIdentity();

		   if(!view->local)
		     {
		       if(ay_currentlevel->object != ay_root)
			 {
			   ay_trafo_getallisr(ay_currentlevel->next);
			 }
		     }

		   glScaled(dscalx,1.0,1.0);

		   if(!view->local)
		     {
		       if(ay_currentlevel->object != ay_root)
			 {
			   ay_trafo_getallsr(ay_currentlevel->next);
			 }
		     }

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
		} /* if */
	    }
	  else
	    {
	      o->scalx *= dscalx;
	      o->modified = AY_TRUE;
	    } /* if */
	} /* if */

    } /* while */

  oldwinx = winx;
  oldwiny = winy;

  if(!ay_prefs.lazynotify)
    {
      ay_notify_parent();
    }

  ay_toglcb_display(togl);

 return TCL_OK;
} /* ay_oact_sc1DXcb */


/* ay_oact_sc1DYcb:
 *
 */
int
ay_oact_sc1DYcb(struct Togl *togl, int argc, char *argv[])
{
 int ay_status = AY_OK;
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
 ay_point *point = NULL;
 char fname[] = "scale1DY_act";

  /* parse args */
  ay_status = ay_oact_parseargs(togl, argc, argv, fname,
				&winx, &winy, &oldwinx, &oldwiny);

  if(ay_status)
    {
      return TCL_OK;
    }

  /* bail out, as long as we stay in the same grid cell */
  if((oldwinx == winx) && (oldwiny == winy))
    {
      return TCL_OK;
    }

  glGetIntegerv(GL_VIEWPORT, vp);
  glGetDoublev(GL_PROJECTION_MATRIX, mp);
  glMatrixMode(GL_MODELVIEW);

  /* scale the object(s) / selected points */
  while(sel)
    {
      o = sel->object;

      /* so that we may use continue; */
      sel = sel->next;

      if(o)
	{
	  glPushMatrix();
	   if(view->type != AY_VTTRIM)
	     {
	       if(ay_currentlevel->object != ay_root)
		 {
		  ay_trafo_getall(ay_currentlevel->next);
		 }
	     }
	   glTranslated(o->movx, o->movy, o->movz);
	   ay_quat_torotmatrix(o->quat, m);
	   glMultMatrixd(m);
	   glScaled(o->scalx, o->scaly, o->scalz);
	   glGetDoublev(GL_MODELVIEW_MATRIX, mm);
	  glPopMatrix();

	  if(GL_FALSE == gluProject(0.0,0.0,0.0,mm,mp,vp,&owinx,&owiny,&owinz))
	    {
	      return TCL_OK;
	    }
	  if(GL_FALSE == gluProject(0.0,1.0,0.0,mm,mp,vp,&vy[0],&vy[1],&vy[2]))
	    {
	      return TCL_OK;
	    }

	  vy[1] = height - vy[1];
	  owiny = height - owiny;

	  vy[0] -= owinx;
	  vy[1] -= owiny;

	  if((fabs(vy[0]) > AY_EPSILON) || (fabs(vy[1]) > AY_EPSILON))
	    {
	      alpha = AY_R2D(acos(vy[0]/AY_V2LEN(vy)));
	      if(vy[1]>0.0)
		alpha = 360.0-alpha;

	      v1[0] = (oldwinx-owinx);
	      v1[1] = (oldwiny-owiny);
	      /* bail out, if we get too near the origin */
	      if((fabs(v1[0]) < AY_EPSILON) && (fabs(v1[1]) < AY_EPSILON))
		continue;

	      v2[0] = (winx-owinx);
	      v2[1] = (winy-owiny);
	      /* bail out, if we get too near the origin */
	      if((fabs(v2[0]) < AY_EPSILON) && (fabs(v2[1]) < AY_EPSILON))
		continue;

	      beta = AY_R2D(acos(v1[0]/AY_V2LEN(v1)));
	      if(v1[1]<0.0)
		beta = 360.0-beta;

	      gamma = AY_R2D(acos(v2[0]/AY_V2LEN(v2)));
	      if(v2[1]<0.0)
		gamma = 360.0-gamma;

	      t1 = cos(AY_D2R(alpha)+AY_D2R(beta))*AY_V2LEN(v1);
	      t2 = cos(AY_D2R(alpha)+AY_D2R(gamma))*AY_V2LEN(v2);

	      if(fabs(t2) > AY_EPSILON && fabs(t1) > AY_EPSILON)
		dscaly = t2/t1;
	      else
		dscaly = 1.0;
	    }
	  else
	    dscaly = 1.0;

	  if(view->transform_points)
	    {
	      if(o->selp && (!o->selp->readonly))
		{
		  point = o->selp;

		  glPushMatrix();
		   glLoadIdentity();

		   if(!view->local)
		     {
		       if(ay_currentlevel->object != ay_root)
			 {
			   ay_trafo_getallisr(ay_currentlevel->next);
			 }
		     }

		   glScaled(1.0,dscaly,1.0);

		   if(!view->local)
		     {
		       if(ay_currentlevel->object != ay_root)
			 {
			   ay_trafo_getallsr(ay_currentlevel->next);
			 }
		     }

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
		} /* if */
	    }
	  else
	    {
	      o->scaly *= dscaly;
	      o->modified = AY_TRUE;
	    } /* if */
	} /* if */

    } /* while */

  oldwinx = winx;
  oldwiny = winy;

  if(!ay_prefs.lazynotify)
    {
      ay_notify_parent();
    }

  ay_toglcb_display(togl);

 return TCL_OK;
} /* ay_oact_sc1DYcb */


/* ay_oact_sc1DZcb:
 *
 */
int
ay_oact_sc1DZcb(struct Togl *togl, int argc, char *argv[])
{
 int ay_status = AY_OK;
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
 ay_point *point = NULL;
 char fname[] = "scale1DZ_act";

  /* parse args */
  ay_status = ay_oact_parseargs(togl, argc, argv, fname,
				&winx, &winy, &oldwinx, &oldwiny);

  if(ay_status)
    {
      return TCL_OK;
    }

  /* bail out, as long as we stay in the same grid cell */
  if((oldwinx == winx) && (oldwiny == winy))
    {
      return TCL_OK;
    }

  glGetIntegerv(GL_VIEWPORT, vp);
  glGetDoublev(GL_PROJECTION_MATRIX, mp);
  glMatrixMode(GL_MODELVIEW);

  /* scale the object(s) / selected points */
  while(sel)
    {
      o = sel->object;

      /* so that we may use continue; */
      sel = sel->next;

      if(o)
	{
	  glPushMatrix();
	   if(view->type != AY_VTTRIM)
	     {
	       if(ay_currentlevel->object != ay_root)
		 {
		   ay_trafo_getall(ay_currentlevel->next);
		 }
	     }
	   glTranslated(o->movx, o->movy, o->movz);
	   ay_quat_torotmatrix(o->quat, m);
	   glMultMatrixd(m);
	   glScaled(o->scalx, o->scaly, o->scalz);
	   glGetDoublev(GL_MODELVIEW_MATRIX, mm);
	  glPopMatrix();
	  if(GL_FALSE == gluProject(0.0,0.0,0.0,mm,mp,vp,&owinx,&owiny,&owinz))
	    {
	      return TCL_OK;
	    }
	  if(GL_FALSE == gluProject(0.0,0.0,1.0,mm,mp,vp,&vz[0],&vz[1],&vz[2]))
	    {
	      return TCL_OK;
	    }

	  vz[1] = height - vz[1];
	  owiny = height - owiny;

	  vz[0] -= owinx;
	  vz[1] -= owiny;

	  if((fabs(vz[0]) > AY_EPSILON) || (fabs(vz[1]) > AY_EPSILON))
	    {
	      alpha = AY_R2D(acos(vz[0]/AY_V2LEN(vz)));
	      if(vz[1]>0.0)
		alpha = 360.0-alpha;

	      v1[0] = (oldwinx-owinx);
	      v1[1] = (oldwiny-owiny);
	      /* bail out, if we get too near the origin */
	      if((fabs(v1[0]) < AY_EPSILON) && (fabs(v1[1]) < AY_EPSILON))
		continue;

	      v2[0] = (winx-owinx);
	      v2[1] = (winy-owiny);
	      /* bail out, if we get too near the origin */
	      if((fabs(v2[0]) < AY_EPSILON) && (fabs(v2[1]) < AY_EPSILON))
		continue;

	      beta = AY_R2D(acos(v1[0]/AY_V2LEN(v1)));
	      if(v1[1]<0.0)
		beta = 360.0-beta;

	      gamma = AY_R2D(acos(v2[0]/AY_V2LEN(v2)));
	      if(v2[1]<0.0)
		gamma = 360.0-gamma;

	      t1 = cos(AY_D2R(alpha)+AY_D2R(beta))*AY_V2LEN(v1);
	      t2 = cos(AY_D2R(alpha)+AY_D2R(gamma))*AY_V2LEN(v2);

	      if(fabs(t2) > AY_EPSILON && fabs(t1) > AY_EPSILON)
		dscalz = t2/t1;
	      else
		dscalz = 1.0;
	    }
	  else
	    dscalz = 1.0;

	  if(view->transform_points)
	    {
	      if(o->selp && (!o->selp->readonly))
		{
		  point = o->selp;

		  glPushMatrix();
		   glLoadIdentity();

		   if(!view->local)
		     {
		       if(ay_currentlevel->object != ay_root)
			 {
			   ay_trafo_getallisr(ay_currentlevel->next);
			 }
		     }

		   glScaled(1.0,1.0,dscalz);

		   if(!view->local)
		     {
		       if(ay_currentlevel->object != ay_root)
			 {
			   ay_trafo_getallsr(ay_currentlevel->next);
			 }
		     }

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
		} /* if */
	    }
	  else
	    {
	      o->scalz *= dscalz;
	      o->modified = AY_TRUE;
	    } /* if */
	} /* if */

    } /* while */

  oldwinx = winx;
  oldwiny = winy;

  if(!ay_prefs.lazynotify)
    {
      ay_notify_parent();
    }

  ay_toglcb_display(togl);

 return TCL_OK;
} /* ay_oact_sc1DZcb */


/* ay_oact_sc2Dcb:
 *
 */
int
ay_oact_sc2Dcb(struct Togl *togl, int argc, char *argv[])
{
 int ay_status = AY_OK;
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
 ay_point *point = NULL;
 char fname[] = "scale2D_act";

  /* parse args */
  ay_status = ay_oact_parseargs(togl, argc, argv, fname,
				&winx, &winy, &oldwinx, &oldwiny);

  if(ay_status)
    {
      return TCL_OK;
    }

  /* bail out, as long as we stay in the same grid cell */
  if((oldwinx == winx) && (oldwiny == winy))
    {
      return TCL_OK;
    }

  glGetIntegerv(GL_VIEWPORT, vp);
  glGetDoublev(GL_PROJECTION_MATRIX, mp);
  glMatrixMode(GL_MODELVIEW);

  /* scale the object(s) / selected points */
  while(sel)
    {
      o = sel->object;

      /* so that we may use continue; */
      sel = sel->next;

      if(o)
	{
	  glPushMatrix();
	   if(view->type != AY_VTTRIM)
	     {
	       if(ay_currentlevel->object != ay_root)
		 {
		   ay_trafo_getall(ay_currentlevel->next);
		 }
	     }
	   glTranslated(o->movx, o->movy, o->movz);
	   ay_quat_torotmatrix(o->quat, m);
	   glMultMatrixd(m);
	   glScaled(o->scalx, o->scaly, o->scalz);
	   glGetDoublev(GL_MODELVIEW_MATRIX, mm);
	  glPopMatrix();
	  if(GL_FALSE == gluProject(0.0,0.0,0.0,mm,mp,vp,&owinx,&owiny,&owinz))
	    {
	      return TCL_OK;
	    }

	  owiny = height - owiny;

	  v1[0] = (oldwinx-owinx);
	  v1[1] = (oldwiny-owiny);
	  if((fabs(v1[0])<AY_EPSILON)&&(fabs(v1[1])<AY_EPSILON))
	    continue;

	  v2[0] = (winx-owinx);
	  v2[1] = (winy-owiny);
	  if((fabs(v2[0])<AY_EPSILON)&&(fabs(v2[1])<AY_EPSILON))
	    continue;

	  t1 = AY_V2LEN(v1);
	  t2 = AY_V2LEN(v2);

	  if(fabs(t2) > AY_EPSILON && fabs(t1) > AY_EPSILON)
	    dscal = t2/t1;
	  else
	    dscal = 1.0;

	  if(view->transform_points)
	    {
	      if(o->selp && (!o->selp->readonly))
		{
		  point = o->selp;

		  glPushMatrix();
		   glLoadIdentity();

		   if(!view->local)
		     {
		       if(ay_currentlevel->object != ay_root)
			 {
			   ay_trafo_getallisr(ay_currentlevel->next);
			 }
		     }

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
		     {
		       if(ay_currentlevel->object != ay_root)
			 {
			   ay_trafo_getallsr(ay_currentlevel->next);
			 }
		     }

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
		} /* if */
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
	      o->modified = AY_TRUE;
	    } /* if */
	} /* if */

    } /* while */

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
 int ay_status = AY_OK;
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
 ay_point *point = NULL;
 char fname[] = "scale3D_act";

  /* parse args */
  ay_status = ay_oact_parseargs(togl, argc, argv, fname,
				&winx, &winy, &oldwinx, &oldwiny);

  if(ay_status)
    {
      return TCL_OK;
    }

  /* bail out, as long as we stay in the same grid cell */
  if((oldwinx == winx) && (oldwiny == winy))
    {
      return TCL_OK;
    }

  glGetIntegerv(GL_VIEWPORT, vp);
  glGetDoublev(GL_PROJECTION_MATRIX, mp);
  glMatrixMode(GL_MODELVIEW);

  /* scale the object(s) / selected points */
  while(sel)
    {
      o = sel->object;

      /* so that we may use continue; */
      sel = sel->next;

      if(o)
	{
	  glPushMatrix();
	   if(view->type != AY_VTTRIM)
	     {
	       if(ay_currentlevel->object != ay_root)
		 {
		   ay_trafo_getall(ay_currentlevel->next);
		 }
	     }
	   glTranslated(o->movx, o->movy, o->movz);
	   ay_quat_torotmatrix(o->quat, m);
	   glMultMatrixd(m);
	   glScaled(o->scalx, o->scaly, o->scalz);
	   glGetDoublev(GL_MODELVIEW_MATRIX, mm);
	  glPopMatrix();
	  if(GL_FALSE == gluProject(0.0,0.0,0.0,mm,mp,vp,&owinx,&owiny,&owinz))
	    {
	      return TCL_OK;
	    }

	  owiny = height - owiny;

	  v1[0] = (oldwinx-owinx);
	  v1[1] = (oldwiny-owiny);
	  if((fabs(v1[0])<AY_EPSILON)&&(fabs(v1[1])<AY_EPSILON))
	    continue;

	  v2[0] = (winx-owinx);
	  v2[1] = (winy-owiny);
	  if((fabs(v2[0])<AY_EPSILON)&&(fabs(v2[1])<AY_EPSILON))
	    continue;

	  t1 = AY_V2LEN(v1);
	  t2 = AY_V2LEN(v2);

	  if(fabs(t2) > AY_EPSILON && fabs(t1) > AY_EPSILON)
	    dscal = t2/t1;
	  else
	    dscal = 1.0;

	  if(view->transform_points)
	    {
	      if(o->selp && (!o->selp->readonly))
		{
		  point = o->selp;

		  glPushMatrix();
		   glLoadIdentity();

		   if(!view->local)
		     {
		       if(ay_currentlevel->object != ay_root)
			 {
			   ay_trafo_getallisr(ay_currentlevel->next);
			 }
		     }

		   glScaled(dscal,dscal,dscal);

		   if(!view->local)
		     {
		       if(ay_currentlevel->object != ay_root)
			 {
			   ay_trafo_getallsr(ay_currentlevel->next);
			 }
		     }

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
		} /* if */
	    }
	  else
	    {
	      o->scalx *= dscal;
	      o->scaly *= dscal;
	      o->scalz *= dscal;
	      o->modified = AY_TRUE;
	    } /* if */
	} /* if */

    } /* while */

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
 int ay_status = AY_OK;
 char fname[] = "stretch2D_act";
 double d = 0.0;
 ay_view_object *view = (ay_view_object *)Togl_GetClientData(togl);

  /* parse args */
  ay_status = ay_oact_parseargs(togl, argc, argv, fname,
				&d, &d, &d, &d);

  if(ay_status)
    {
      return TCL_OK;
    }

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
    default:
      break;
    }

 return TCL_OK;
} /* ay_oact_str2Dcb */


/* ay_oact_sc1DXAcb:
 *
 */
int
ay_oact_sc1DXAcb(struct Togl *togl, int argc, char *argv[])
{
 int ay_status = AY_OK;
 ay_view_object *view = (ay_view_object *)Togl_GetClientData(togl);
 double height = Togl_Height(togl);
 static double oldwinx = 0.0, oldwiny = 0.0;
 double winx = 0.0, winy = 0.0, dscalx = 1.0;
 double ax = 0.0, ay = 0.0;
 double al = 0.0, mov = 0.0, oldmov = 0.0;
 double tpoint[4]={0}, t1, t2, v1[2], v2[2];
 double xaxis[3]={1.0,0.0,0.0}, v3[3];
 GLdouble vx[3], a[3], alpha, beta, gamma;
 GLdouble mp[16], mm[16], mmi[16], owinx, owiny, owinz;
 GLdouble m[16];
 GLint vp[4];
 ay_list_object *sel = ay_selection;
 ay_object *o = NULL;
 ay_point *point = NULL;
 char fname[] = "scale1DXA_act";

  /* parse args */
  ay_status = ay_oact_parseargs(togl, argc, argv, fname,
				&winx, &winy, &oldwinx, &oldwiny);

  if(ay_status)
    {
      return TCL_OK;
    }

  if(!view->drawmark)
    {
      /* if view->drawmark is not enabled some other action
	 changed view trafos so that the point is not valid
	 anymore and we should request a new point */
      ay_error(AY_ERROR, fname,
	       "Lost mark. Please restart this action!");
      return TCL_OK;
    }

  /* bail out, as long as we stay in the same grid cell */
  if((oldwinx == winx) && (oldwiny == winy))
    {
      return TCL_OK;
    }

  ax = view->markx;
  ay = view->marky;

  glGetIntegerv(GL_VIEWPORT, vp);
  glGetDoublev(GL_PROJECTION_MATRIX, mp);
  glMatrixMode(GL_MODELVIEW);

  /* scale the object(s) / selected points */
  while(sel)
    {
      o = sel->object;

      /* so that we may use continue; */
      sel = sel->next;

      if(o)
	{
	  glPushMatrix();
	  if(view->type != AY_VTTRIM)
	    {
	      if(ay_currentlevel->object != ay_root)
		{
		  ay_trafo_getall(ay_currentlevel->next);
		}
	    }

	   glTranslated(o->movx, o->movy, o->movz);
	   ay_quat_torotmatrix(o->quat, m);
	   glMultMatrixd(m);
	   glScaled(o->scalx, o->scaly, o->scalz);

	   glGetDoublev(GL_MODELVIEW_MATRIX, mm);
	  glPopMatrix();
	  if(GL_FALSE == gluProject(0.0,0.0,0.0,mm,mp,vp,&owinx,&owiny,&owinz))
	    {
	      return TCL_OK;
	    }
	  if(GL_FALSE == gluProject(1.0,0.0,0.0,mm,mp,vp,&vx[0],&vx[1],&vx[2]))
	    {
	      return TCL_OK;
	    }
	  owiny = height - owiny;
	  vx[1] = height - vx[1];

	  vx[0] -= owinx;
	  vx[1] -= owiny;

	  if((fabs(vx[0]) > AY_EPSILON) || (fabs(vx[1]) > AY_EPSILON))
	    {
	      alpha = AY_R2D(acos(vx[0]/AY_V2LEN(vx)));
	      if(vx[1]>0.0)
		alpha = 360.0-alpha;

	      owinx = ax;
	      owiny = ay;

	      v1[0] = (oldwinx-owinx);
	      v1[1] = (oldwiny-owiny);
	      /* bail out, if we get too near the mark */
	      if((fabs(v1[0])<AY_EPSILON)&&(fabs(v1[1])<AY_EPSILON))
		continue;

	      v2[0] = (winx-owinx);
 	      v2[1] = (winy-owiny);
	      /* bail out, if we get too near the mark */
	      if((fabs(v2[0])<AY_EPSILON)&&(fabs(v2[1])<AY_EPSILON))
		continue;

	      beta = AY_R2D(acos(v1[0]/AY_V2LEN(v1)));
	      if(v1[1]<0.0)
		beta = 360.0-beta;

	      gamma = AY_R2D(acos(v2[0]/AY_V2LEN(v2)));
	      if(v2[1]<0.0)
		gamma = 360.0-gamma;

	      t1 = cos(AY_D2R(alpha)+AY_D2R(beta))*AY_V2LEN(v1);
	      t2 = cos(AY_D2R(alpha)+AY_D2R(gamma))*AY_V2LEN(v2);

	      if(fabs(t2) > AY_EPSILON && fabs(t1) > AY_EPSILON)
		dscalx = t2/t1;
	      else
		dscalx = 1.0;
	    }
	  else
	    dscalx = 1.0;

	  /* transform mark from world to current level space */
	  glPushMatrix();
	  if(view->type != AY_VTTRIM)
	    {
	      if(ay_currentlevel->object != ay_root)
		{
		  ay_trafo_getall(ay_currentlevel->next);
		}
	    }

	  glGetDoublev(GL_MODELVIEW_MATRIX, mm);
	  glPopMatrix();

	  ay_trafo_invmatrix4(mm, mmi);
	  AY_APTRAN3(a, view->markworld, mmi);

	  ay_quat_torotmatrix(o->quat, mm);
	  AY_APTRAN3(v3, xaxis, mm);

	  oldmov = v3[0]*o->movx+v3[1]*o->movy+v3[2]*o->movz;
	  al = v3[0]*a[0]+v3[1]*a[1]+v3[2]*a[2];

	  mov = al - ((o->scalx*(dscalx))*((al-oldmov)/o->scalx));

	  if(view->transform_points)
	    {
	      if(o->selp && (!o->selp->readonly))
		{
		  point = o->selp;

		  glPushMatrix();
		   glLoadIdentity();

		   if(!view->local)
		     {
		       if(ay_currentlevel->object != ay_root)
			 {
			   ay_trafo_getallisr(ay_currentlevel->next);
			 }
		     }

		   glTranslated(mov-o->movx,0.0,0.0);
		   glScaled(dscalx,1.0,1.0);

		   if(!view->local)
		     {
		       if(ay_currentlevel->object != ay_root)
			 {
			   ay_trafo_getallsr(ay_currentlevel->next);
			 }
		     }

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
		} /* if */
	    }
	  else
	    {
	      mov = oldmov - mov;

	      o->movx -= v3[0]*mov;
	      o->movy -= v3[1]*mov;
	      o->movz -= v3[2]*mov;

	      o->scalx *= dscalx;

	      o->modified = AY_TRUE;
	    } /* if */
	} /* if */

    } /* while */

  oldwinx = winx;
  oldwiny = winy;

  if(!ay_prefs.lazynotify)
    ay_notify_parent();

  ay_toglcb_display(togl);

 return TCL_OK;
} /* ay_oact_sc1DXAcb */


/* ay_oact_sc1DYAcb:
 *
 */
int
ay_oact_sc1DYAcb(struct Togl *togl, int argc, char *argv[])
{
 int ay_status = AY_OK;
 ay_view_object *view = (ay_view_object *)Togl_GetClientData(togl);
 double height = Togl_Height(togl);
 static double oldwinx = 0.0, oldwiny = 0.0;
 double winx = 0.0, winy = 0.0, dscaly = 1.0;
 double ax = 0.0, ay = 0.0;
 double al = 0.0, mov = 0.0, oldmov = 0.0;
 double tpoint[4]={0}, t1, t2, v1[2], v2[2];
 double yaxis[3]={0.0,1.0,0.0}, v3[3];
 GLdouble vy[3], a[3], alpha, beta, gamma;
 GLdouble mp[16], mm[16], mmi[16], owinx, owiny, owinz;
 GLdouble m[16];
 GLint vp[4];
 ay_list_object *sel = ay_selection;
 ay_object *o = NULL;
 ay_point *point = NULL;
 char fname[] = "scale1DYA_act";

  /* parse args */
  ay_status = ay_oact_parseargs(togl, argc, argv, fname,
				&winx, &winy, &oldwinx, &oldwiny);

  if(ay_status)
    {
      return TCL_OK;
    }

  if(!view->drawmark)
    {
      /* if view->drawmark is not enabled some other action
	 changed view trafos so that the point is not valid
	 anymore and we should request a new point */
      ay_error(AY_ERROR, fname,
	       "Lost mark. Please restart this action!");
      return TCL_OK;
    }

  /* bail out, as long as we stay in the same grid cell */
  if((oldwinx == winx) && (oldwiny == winy))
    {
      return TCL_OK;
    }

  ax = view->markx;
  ay = view->marky;

  glGetIntegerv(GL_VIEWPORT, vp);
  glGetDoublev(GL_PROJECTION_MATRIX, mp);
  glMatrixMode(GL_MODELVIEW);

  /* scale the object(s) / selected points */
  while(sel)
    {
      o = sel->object;

      /* so that we may use continue; */
      sel = sel->next;

      if(o)
	{
	  glPushMatrix();
	   if(view->type != AY_VTTRIM)
	     {
	       if(ay_currentlevel->object != ay_root)
		 {
		   ay_trafo_getall(ay_currentlevel->next);
		 }
	     }

	   glTranslated(o->movx, o->movy, o->movz);
	   ay_quat_torotmatrix(o->quat, m);
	   glMultMatrixd(m);
	   glScaled(o->scalx, o->scaly, o->scalz);

	   glGetDoublev(GL_MODELVIEW_MATRIX, mm);
	  glPopMatrix();
	  if(GL_FALSE == gluProject(0.0,0.0,0.0,mm,mp,vp,&owinx,&owiny,&owinz))
	    {
	      return TCL_OK;
	    }
	  if(GL_FALSE == gluProject(0.0,1.0,0.0,mm,mp,vp,&vy[0],&vy[1],&vy[2]))
	    {
	      return TCL_OK;
	    }
	  owiny = height - owiny;
	  vy[1] = height - vy[1];

	  vy[0] -= owinx;
	  vy[1] -= owiny;

	  if((fabs(vy[0]) > AY_EPSILON) || (fabs(vy[1]) > AY_EPSILON))
	    {
	      alpha = AY_R2D(acos(vy[0]/AY_V2LEN(vy)));
	      if(vy[1]>0.0)
		alpha = 360.0-alpha;

	      owinx = ax;
	      owiny = ay;

	      v1[0] = (oldwinx-owinx);
	      v1[1] = (oldwiny-owiny);
	      /* bail out, if we get too near the mark */
	      if((fabs(v1[0])<AY_EPSILON)&&(fabs(v1[1])<AY_EPSILON))
		continue;

	      v2[0] = (winx-owinx);
 	      v2[1] = (winy-owiny);
	      /* bail out, if we get too near the mark */
	      if((fabs(v2[0])<AY_EPSILON)&&(fabs(v2[1])<AY_EPSILON))
		continue;

	      beta = AY_R2D(acos(v1[0]/AY_V2LEN(v1)));
	      if(v1[1]<0.0)
		beta = 360.0-beta;

	      gamma = AY_R2D(acos(v2[0]/AY_V2LEN(v2)));
	      if(v2[1]<0.0)
		gamma = 360.0-gamma;

	      t1 = cos(AY_D2R(alpha)+AY_D2R(beta))*AY_V2LEN(v1);
	      t2 = cos(AY_D2R(alpha)+AY_D2R(gamma))*AY_V2LEN(v2);

	      if(fabs(t2) > AY_EPSILON && fabs(t1) > AY_EPSILON)
		dscaly = t2/t1;
	      else
		dscaly = 1.0;
	    }
	  else
	    dscaly = 1.0;

	  /* transform mark from world to current level space */
	  glPushMatrix();
	  if(view->type != AY_VTTRIM)
	    {
	      if(ay_currentlevel->object != ay_root)
		{
		  ay_trafo_getall(ay_currentlevel->next);
		}
	    }

	  glGetDoublev(GL_MODELVIEW_MATRIX, mm);
	  glPopMatrix();

	  ay_trafo_invmatrix4(mm, mmi);
	  AY_APTRAN3(a, view->markworld, mmi);

	  ay_quat_torotmatrix(o->quat, mm);
	  AY_APTRAN3(v3, yaxis, mm);

	  oldmov = v3[0]*o->movx+v3[1]*o->movy+v3[2]*o->movz;
	  al = v3[0]*a[0]+v3[1]*a[1]+v3[2]*a[2];

	  mov = al - ((o->scaly*(dscaly))*((al-oldmov)/o->scaly));

	  if(view->transform_points)
	    {
	      if(o->selp && (!o->selp->readonly))
		{
		  point = o->selp;

		  glPushMatrix();
		   glLoadIdentity();

		   if(!view->local)
		     {
		       if(ay_currentlevel->object != ay_root)
			 {
			   ay_trafo_getallisr(ay_currentlevel->next);
			 }
		     }

		   glTranslated(0.0,mov-o->movy,0.0);
		   glScaled(1.0,dscaly,1.0);

		   if(!view->local)
		     {
		       if(ay_currentlevel->object != ay_root)
			 {
			   ay_trafo_getallsr(ay_currentlevel->next);
			 }
		     }

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
		} /* if */
	    }
	  else
	    {
	      mov = oldmov - mov;

	      o->movx -= v3[0]*mov;
	      o->movy -= v3[1]*mov;
	      o->movz -= v3[2]*mov;

	      o->scaly *= dscaly;

	      o->modified = AY_TRUE;
	    } /* if */
	} /* if */

    } /* while */

  oldwinx = winx;
  oldwiny = winy;

  if(!ay_prefs.lazynotify)
    ay_notify_parent();

  ay_toglcb_display(togl);

 return TCL_OK;
} /* ay_oact_sc1DYAcb */


/* ay_oact_sc1DZAcb:
 *
 */
int
ay_oact_sc1DZAcb(struct Togl *togl, int argc, char *argv[])
{
 int ay_status = AY_OK;
 ay_view_object *view = (ay_view_object *)Togl_GetClientData(togl);
 double height = Togl_Height(togl);
 static double oldwinx = 0.0, oldwiny = 0.0;
 double winx = 0.0, winy = 0.0, dscalz = 1.0;
 double ax = 0.0, ay = 0.0;
 double al = 0.0, mov = 0.0, oldmov = 0.0;
 double tpoint[4]={0}, t1, t2, v1[2], v2[2];
 double zaxis[3]={0.0,0.0,1.0}, v3[3];
 GLdouble vz[3], a[3], alpha, beta, gamma;
 GLdouble mp[16], mm[16], mmi[16], owinx, owiny, owinz;
 GLdouble m[16];
 GLint vp[4];
 ay_list_object *sel = ay_selection;
 ay_object *o = NULL;
 ay_point *point = NULL;
 char fname[] = "scale1DZA_act";

  /* parse args */
  ay_status = ay_oact_parseargs(togl, argc, argv, fname,
				&winx, &winy, &oldwinx, &oldwiny);

  if(ay_status)
    {
      return TCL_OK;
    }

  if(!view->drawmark)
    {
      /* if view->drawmark is not enabled some other action
	 changed view trafos so that the point is not valid
	 anymore and we should request a new point */
      ay_error(AY_ERROR, fname,
	       "Lost mark. Please restart this action!");
      return TCL_OK;
    }

  /* bail out, as long as we stay in the same grid cell */
  if((oldwinx == winx) && (oldwiny == winy))
    {
      return TCL_OK;
    }

  ax = view->markx;
  ay = view->marky;

  glGetIntegerv(GL_VIEWPORT, vp);
  glGetDoublev(GL_PROJECTION_MATRIX, mp);
  glMatrixMode(GL_MODELVIEW);

  /* scale the object(s) / selected points */
  while(sel)
    {
      o = sel->object;

      /* so that we may use continue; */
      sel = sel->next;

      if(o)
	{
	  glPushMatrix();
	   if(view->type != AY_VTTRIM)
	     {
	       if(ay_currentlevel->object != ay_root)
		 {
		   ay_trafo_getall(ay_currentlevel->next);
		 }
	     }

	   glTranslated(o->movx, o->movy, o->movz);
	   ay_quat_torotmatrix(o->quat, m);
	   glMultMatrixd(m);
	   glScaled(o->scalx, o->scaly, o->scalz);

	   glGetDoublev(GL_MODELVIEW_MATRIX, mm);
	  glPopMatrix();
	  if(GL_FALSE == gluProject(0.0,0.0,0.0,mm,mp,vp,&owinx,&owiny,&owinz))
	    {
	      return TCL_OK;
	    }
	  if(GL_FALSE == gluProject(0.0,0.0,1.0,mm,mp,vp,&vz[0],&vz[1],&vz[2]))
	    {
	      return TCL_OK;
	    }
	  owiny = height - owiny;
	  vz[1] = height - vz[1];

	  vz[0] -= owinx;
	  vz[1] -= owiny;

	  if((fabs(vz[0]) > AY_EPSILON) || (fabs(vz[1]) > AY_EPSILON))
	    {
	      alpha = AY_R2D(acos(vz[0]/AY_V2LEN(vz)));
	      if(vz[1]>0.0)
		alpha = 360.0-alpha;

	      owinx = ax;
	      owiny = ay;

	      v1[0] = (oldwinx-owinx);
	      v1[1] = (oldwiny-owiny);
	      /* bail out, if we get too near the mark */
	      if((fabs(v1[0])<AY_EPSILON)&&(fabs(v1[1])<AY_EPSILON))
		continue;

	      v2[0] = (winx-owinx);
 	      v2[1] = (winy-owiny);
	      /* bail out, if we get too near the mark */
	      if((fabs(v2[0])<AY_EPSILON)&&(fabs(v2[1])<AY_EPSILON))
		continue;

	      beta = AY_R2D(acos(v1[0]/AY_V2LEN(v1)));
	      if(v1[1]<0.0)
		beta = 360.0-beta;

	      gamma = AY_R2D(acos(v2[0]/AY_V2LEN(v2)));
	      if(v2[1]<0.0)
		gamma = 360.0-gamma;

	      t1 = cos(AY_D2R(alpha)+AY_D2R(beta))*AY_V2LEN(v1);
	      t2 = cos(AY_D2R(alpha)+AY_D2R(gamma))*AY_V2LEN(v2);

	      if(fabs(t2) > AY_EPSILON && fabs(t1) > AY_EPSILON)
		dscalz = t2/t1;
	      else
		dscalz = 1.0;
	    }
	  else
	    dscalz = 1.0;

	  /* transform mark from world to current level space */
	  glPushMatrix();
	  if(view->type != AY_VTTRIM)
	    {
	      if(ay_currentlevel->object != ay_root)
		{
		  ay_trafo_getall(ay_currentlevel->next);
		}
	    }

	  glGetDoublev(GL_MODELVIEW_MATRIX, mm);
	  glPopMatrix();

	  ay_trafo_invmatrix4(mm, mmi);
	  AY_APTRAN3(a, view->markworld, mmi);

	  ay_quat_torotmatrix(o->quat, mm);
	  AY_APTRAN3(v3, zaxis, mm);

	  oldmov = v3[0]*o->movx+v3[1]*o->movy+v3[2]*o->movz;
	  al = v3[0]*a[0]+v3[1]*a[1]+v3[2]*a[2];

	  mov = al - ((o->scalz*(dscalz))*((al-oldmov)/o->scalz));

	  if(view->transform_points)
	    {
	      if(o->selp && (!o->selp->readonly))
		{
		  point = o->selp;

		  glPushMatrix();
		   glLoadIdentity();

		   if(!view->local)
		     {
		       if(ay_currentlevel->object != ay_root)
			 {
			   ay_trafo_getallisr(ay_currentlevel->next);
			 }
		     }

		   glTranslated(0.0,0.0,mov-o->movz);
		   glScaled(1.0,1.0,dscalz);

		   if(!view->local)
		     {
		       if(ay_currentlevel->object != ay_root)
			 {
			   ay_trafo_getallsr(ay_currentlevel->next);
			 }
		     }

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
		} /* if */
	    }
	  else
	    {
	      mov = oldmov - mov;

	      o->movx -= v3[0]*mov;
	      o->movy -= v3[1]*mov;
	      o->movz -= v3[2]*mov;

	      o->scalz *= dscalz;

	      o->modified = AY_TRUE;
	    } /* if */
	} /* if */

    } /* while */

  oldwinx = winx;
  oldwiny = winy;

  if(!ay_prefs.lazynotify)
    ay_notify_parent();

  ay_toglcb_display(togl);

 return TCL_OK;
} /* ay_oact_sc1DZAcb */


/* ay_oact_sc2DAcb:
 *
 */
int
ay_oact_sc2DAcb(struct Togl *togl, int argc, char *argv[])
{
 int ay_status = AY_OK;
 ay_view_object *view = (ay_view_object *)Togl_GetClientData(togl);
 static double oldwinx = 0.0, oldwiny = 0.0;
 double ax = 0.0, ay = 0.0;
 double winx = 0.0, winy = 0.0, dscal = 1.0;
 double tpoint[4]={0}, t1, t2, v1[2], v2[2];
 GLdouble a[3], mm[16], mmi[16];
 ay_list_object *sel = ay_selection;
 ay_object *o = NULL;
 ay_point *point = NULL;
 char fname[] = "scale2DA_act";

  /* parse args */
  ay_status = ay_oact_parseargs(togl, argc, argv, fname,
				&winx, &winy, &oldwinx, &oldwiny);

  if(ay_status)
    {
      return TCL_OK;
    }

  if(!view->drawmark)
    {
      /* if view->drawmark is not enabled some other action
	 changed view trafos so that the point is not valid
	 anymore and we should request a new point */
      ay_error(AY_ERROR, fname,
	       "Lost mark. Please restart this action!");
      return TCL_OK;
    }

  /* bail out, as long as we stay in the same grid cell */
  if((oldwinx == winx) && (oldwiny == winy))
    {
      return TCL_OK;
    }

  /* calculate scale factor from the relative lengths
     of the vectors (oldpickedpoint-mark) and (pickedpoint-mark) */
  ax = view->markx;
  ay = view->marky;

  v1[0] = (oldwinx-ax);
  v1[1] = (oldwiny-ay);
  /* bail out, if we get too near the mark */
  if((fabs(v1[0])<AY_EPSILON)&&(fabs(v1[1])<AY_EPSILON))
    return TCL_OK;

  v2[0] = (winx-ax);
  v2[1] = (winy-ay);
  /* bail out, if we get too near the mark */
  if((fabs(v2[0])<AY_EPSILON)&&(fabs(v2[1])<AY_EPSILON))
    return TCL_OK;

  t1 = AY_V2LEN(v1);
  t2 = AY_V2LEN(v2);

  if(fabs(t2) > AY_EPSILON && fabs(t1) > AY_EPSILON)
    dscal = t2/t1;
  else
    dscal = 1.0;

  /* transform mark from world to current level space */
  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  if(view->type != AY_VTTRIM)
    {
      if(ay_currentlevel->object != ay_root)
	{
	  ay_trafo_getall(ay_currentlevel->next);
	}
    }

  glGetDoublev(GL_MODELVIEW_MATRIX, mm);
  glPopMatrix();

  ay_trafo_invmatrix4(mm, mmi);
  AY_APTRAN3(a, view->markworld, mmi);

  /* scale the object(s) / selected points */
  while(sel)
    {
      o = sel->object;

      /* so that we may use continue; */
      sel = sel->next;

      if(o)
	{
	  if(view->transform_points)
	    {
	      if(o->selp && (!o->selp->readonly))
		{
		  point = o->selp;

		  glPushMatrix();
		   glLoadIdentity();

		   if(!view->local)
		     {
		       if(ay_currentlevel->object != ay_root)
			 {
			   ay_trafo_getallisr(ay_currentlevel->next);
			 }
		     }

		   glTranslated(-o->movx,-o->movy,-o->movz);
		   glTranslated(a[0],a[1],a[2]);
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
		   glTranslated(-a[0],-a[1],-a[2]);
		   glTranslated(o->movx,o->movy,o->movz);

		   if(!view->local)
		     {
		       if(ay_currentlevel->object != ay_root)
			 {
			   ay_trafo_getallsr(ay_currentlevel->next);
			 }
		     }

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
		} /* if */
	    }
	  else
	    {
	      switch(view->type)
		{
		case AY_VTFRONT:
		case AY_VTTRIM:
		  o->movx = o->scalx*dscal*((o->movx-a[0])/o->scalx)+a[0];
		  o->movy = o->scaly*dscal*((o->movy-a[1])/o->scaly)+a[1];
		  o->scalx *= dscal;
		  o->scaly *= dscal;
		  break;
		case AY_VTSIDE:
		  o->movy = o->scaly*dscal*((o->movy-a[1])/o->scaly)+a[1];
		  o->movz = o->scalz*dscal*((o->movz-a[2])/o->scalz)+a[2];
		  o->scaly *= dscal;
		  o->scalz *= dscal;
		  break;
		case AY_VTTOP:
		  o->movx = o->scalx*dscal*((o->movx-a[0])/o->scalx)+a[0];
		  o->movz = o->scalz*dscal*((o->movz-a[2])/o->scalz)+a[2];
		  o->scalx *= dscal;
		  o->scalz *= dscal;
		  break;
		default:
		  break;
		}
	      o->modified = AY_TRUE;
	    } /* if */
	} /* if */

    } /* while */

  oldwinx = winx;
  oldwiny = winy;

  if(!ay_prefs.lazynotify)
    ay_notify_parent();

  ay_toglcb_display(togl);

 return TCL_OK;
} /* ay_oact_sc2DAcb */


/* ay_oact_str2DAcb:
 *
 */
int
ay_oact_str2DAcb(struct Togl *togl, int argc, char *argv[])
{
 int ay_status = AY_OK;
 char fname[] = "stretch2DA_act";
 double d = 0.0;
 ay_view_object *view = (ay_view_object *)Togl_GetClientData(togl);

  /* parse args */
  ay_status = ay_oact_parseargs(togl, argc, argv, fname,
				&d, &d, &d, &d);

  if(ay_status)
    {
      return TCL_OK;
    }

  if(!view->drawmark)
    {
      /* if view->drawmark is not enabled some other action
	 changed view trafos so that the point is not valid
	 anymore and we should request a new point */
      ay_error(AY_ERROR, fname,
	       "Lost mark. Please restart this action!");
      return TCL_OK;
    }

  switch(view->type)
    {
    case AY_VTFRONT:
    case AY_VTTRIM:
      ay_oact_sc1DXAcb(togl, argc, argv);
      ay_oact_sc1DYAcb(togl, argc, argv);
      break;
    case AY_VTSIDE:
      ay_oact_sc1DZAcb(togl, argc, argv);
      ay_oact_sc1DYAcb(togl, argc, argv);
      break;
    case AY_VTTOP:
      ay_oact_sc1DXAcb(togl, argc, argv);
      ay_oact_sc1DZAcb(togl, argc, argv);
      break;
    default:
      break;
    }

 return TCL_OK;
} /* ay_oact_str2DAcb */


/* ay_oact_sc3DAcb:
 *
 */
int
ay_oact_sc3DAcb(struct Togl *togl, int argc, char *argv[])
{
 int ay_status = AY_OK;
 ay_view_object *view = (ay_view_object *)Togl_GetClientData(togl);
 static double oldwinx = 0.0, oldwiny = 0.0;
 double ax = 0.0, ay = 0.0;
 double winx = 0.0, winy = 0.0, dscal = 1.0;
 double tpoint[4]={0}, t1, t2, v1[2], v2[2];
 GLdouble a[3], mm[16], mmi[16];
 ay_list_object *sel = ay_selection;
 ay_object *o = NULL;
 ay_point *point = NULL;
 char fname[] = "scale3DA_act";

  if(!view->drawmark)
    {
      /* if view->drawmark is not enabled some other action
	 changed view trafos so that the point is not valid
	 anymore and we should request a new point */
      ay_error(AY_ERROR, fname,
	       "Lost mark. Please restart this action!");
      return TCL_OK;
    }

  /* parse args */
  ay_status = ay_oact_parseargs(togl, argc, argv, fname,
				&winx, &winy, &oldwinx, &oldwiny);

  if(ay_status)
    return TCL_OK;

  /* bail out, as long as we stay in the same grid cell */
  if((oldwinx == winx) && (oldwiny == winy))
    return TCL_OK;

  ax = view->markx;
  ay = view->marky;

  /* calculate scale factor from the relative lengths
     of the vectors (oldpickedpoint-mark) and (pickedpoint-mark) */
  v1[0] = (oldwinx-ax);
  v1[1] = (oldwiny-ay);

  /* bail out, if we get too near the mark */
  if((fabs(v1[0]) < AY_EPSILON) && (fabs(v1[1]) < AY_EPSILON))
    return TCL_OK;

  v2[0] = (winx-ax);
  v2[1] = (winy-ay);

  /* bail out, if we get too near the mark */
  if((fabs(v2[0]) < AY_EPSILON) && (fabs(v2[1]) < AY_EPSILON))
    return TCL_OK;

  t1 = AY_V2LEN(v1);
  t2 = AY_V2LEN(v2);

  if(fabs(t2) > AY_EPSILON && fabs(t1) > AY_EPSILON)
    dscal = t2/t1;
  else
    dscal = 1.0;

  /* transform mark from world to current level space */
  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
   if(view->type != AY_VTTRIM)
     {
       if(ay_currentlevel->object != ay_root)
	 {
	   ay_trafo_getall(ay_currentlevel->next);
	 }
     }
   glGetDoublev(GL_MODELVIEW_MATRIX, mm);
  glPopMatrix();

  ay_trafo_invmatrix4(mm, mmi);
  AY_APTRAN3(a, view->markworld, mmi);

  /* scale the object(s) / selected points */
  while(sel)
    {
      o = sel->object;

      /* so that we may use continue; */
      sel = sel->next;

      if(o)
	{
	  if(view->transform_points)
	    {
	      if(o->selp && (!o->selp->readonly))
		{
		  point = o->selp;

		  glPushMatrix();
		   glLoadIdentity();

		   if(!view->local)
		     {
		       if(ay_currentlevel->object != ay_root)
			 {
			   ay_trafo_getallisr(ay_currentlevel->next);
			 }
		     }

		   glTranslated(-o->movx,-o->movy,-o->movz);
		   glTranslated(a[0],a[1],a[2]);
		   glScaled(dscal,dscal,dscal);
		   glTranslated(-a[0],-a[1],-a[2]);
		   glTranslated(o->movx,o->movy,o->movz);

		   if(!view->local)
		     {
		       if(ay_currentlevel->object != ay_root)
			 {
			   ay_trafo_getallsr(ay_currentlevel->next);
			 }
		     }

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
		} /* if */
	    }
	  else
	    {
	      o->movx = o->scalx*dscal*((o->movx-a[0])/o->scalx)+a[0];
	      o->movy = o->scaly*dscal*((o->movy-a[1])/o->scaly)+a[1];
	      o->movz = o->scalz*dscal*((o->movz-a[2])/o->scalz)+a[2];
	      o->scalx *= dscal;
	      o->scaly *= dscal;
	      o->scalz *= dscal;

	      o->modified = AY_TRUE;
	    } /* if */
	} /* if */
    } /* while */

  oldwinx = winx;
  oldwiny = winy;

  if(!ay_prefs.lazynotify)
    {
      ay_notify_parent();
    }

  ay_toglcb_display(togl);

 return TCL_OK;
} /* ay_oact_sc3DAcb */
