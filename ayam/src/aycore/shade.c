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

/* shade.c - functions for drawing a shaded scene using OpenGL */

/* ay_shade_object:
 *  shade a single object and children
 */
void
ay_shade_object(struct Togl *togl, ay_object *o, int push_name)
{
 int ay_status = AY_OK;
 char fname[] = "shade_object";
 ay_voidfp *arr = NULL;
 ay_drawcb *cb = NULL;
 ay_object *down;
 double m[16];
 GLfloat oldcolor[4] = {0.0f,0.0f,0.0f,0.0f}, color[4] = {0.0f,0.0f,0.0f,0.0f};
 ay_object *mo = NULL;
 int reset_color = AY_FALSE, toggled_normals = AY_FALSE;
 /* static*/ int cw = AY_TRUE;
 GLint ff = GL_CCW;

  if(o->hide)
    {
      o->glname = 0;
      return;
    }

  /* if an odd number of scale factors are negative
     swap front and back faces */
  if((o->scalx*o->scaly*o->scalz)<0.0)
    {
      glGetIntegerv(GL_FRONT_FACE, &ff);
      if(ff == GL_CW)
	{
	  glFrontFace(GL_CCW);
	  cw = AY_FALSE;
	}
      else
	{
	  glFrontFace(GL_CW);
	}
      toggled_normals = AY_TRUE;
      /*
      if(cw)
	cw = AY_FALSE;
      else
	cw = AY_TRUE;
      if(cw)
	glFrontFace(GL_CW);
      else
	glFrontFace(GL_CCW);
      */
    }

  glPushMatrix();

   glTranslated((GLdouble)o->movx, (GLdouble)o->movy, (GLdouble)o->movz);
   ay_quat_torotmatrix(o->quat, m);
   glMultMatrixd((GLdouble *)m);
   glScaled((GLdouble)o->scalx, (GLdouble)o->scaly, (GLdouble)o->scalz);

   if(push_name)
     {
       o->glname = ++ay_current_glname;

       glPushName(o->glname);
     }
   else
     {
       if(ay_prefs.use_materialcolor)
	 {
	   if(o->type == AY_IDINSTANCE)
	     mo = (ay_object *)o->refine;
	   else
	     mo = o;

	   if(mo->mat)
	     {
	       if(mo->mat->colr != -1)
		 {
		   reset_color = AY_TRUE;
		   glGetMaterialfv(GL_FRONT, GL_AMBIENT, oldcolor);

		   color[0] = (GLfloat)(mo->mat->colr/255.0);
		   color[1] = (GLfloat)(mo->mat->colg/255.0);
		   color[2] = (GLfloat)(mo->mat->colb/255.0);
		   color[3] = (GLfloat)1.0;
		   glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE,
				color);
		 } /* if */
	     } /* if */
	 } /* if */
     } /* if */

   arr = ay_shadecbt.arr;
   cb = (ay_drawcb *)(arr[o->type]);
   if(cb)
     ay_status = cb(togl, o);

   if(ay_status)
     {
       ay_error(AY_ERROR, fname, "shade callback failed");
       goto cleanup;
     }

   if(!o->hide_children)
     {
       down = o->down;
       while(down)
	 {
	   ay_shade_object(togl, down, push_name);
	   down = down->next;
	 } /* while */
     } /* if */

cleanup:

   if(push_name)
     {
       glPopName();
     }

  glPopMatrix();

  if(toggled_normals)
    {
      if(cw)
	{
	  glFrontFace(GL_CCW);
	}
      else
	{
	  glFrontFace(GL_CW);
	}
      /*
      if(cw)
	glFrontFace(GL_CCW);
      else
	glFrontFace(GL_CW);

      if(cw)
	cw = AY_FALSE;
      else
	cw = AY_TRUE;
      */
    }

  if(reset_color)
    {
      glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, oldcolor);
    }

 return;
} /* ay_shade_object */


/* ay_shade_view:
 *  shade a view
 */
int
ay_shade_view(struct Togl *togl)
{
 int ay_status = AY_OK;
 char fname[] = "shade_view";
 ay_view_object *view = (ay_view_object *)Togl_GetClientData(togl);
 int width = Togl_Width(togl), height = Togl_Height(togl);
 ay_list_object *sel = ay_selection;
 ay_object *o = ay_root;
 ay_voidfp *arr = NULL;
 ay_drawcb *cb = NULL;
 ay_point *point = NULL;
 GLfloat color[4] = {0.0f,0.0f,0.0f,0.0f};
 double m[16];

  if(view->dirty)
    {
      ay_toglcb_reshape(togl);
      view->dirty = AY_FALSE;
    }

  glDisable(GL_LIGHTING);

  /* draw background image */
  if(view->drawbgimage)
    {
      ay_draw_bgimage(togl);
    }

  /* draw grid */
  if(view->drawgrid)
    {
      ay_draw_grid(togl);
    }

  /* when drawing mode is shade+draw, set back the shaded
     surfaces a bit so that we can draw on top later */
  if(view->shade > 1)
    {
#ifdef GL_VERSION_1_1
      glEnable(GL_POLYGON_OFFSET_FILL);
      /*glPolygonOffset(1.0, 1.0);*/
      glPolygonOffset((GLfloat)ay_prefs.polyoffset0/*0.5f*/,
		      (GLfloat)ay_prefs.polyoffset1/*0.5f*/);
#else
      glEnable(GL_POLYGON_OFFSET_EXT);
      glPolygonOffsetEXT(1.0, 0.000002);
#endif
    }

  glEnable(GL_DITHER);
  glEnable(GL_LIGHTING);
  glLightModelf(GL_LIGHT_MODEL_TWO_SIDE, (GLfloat)1.0);
  glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, GL_TRUE);

  color[0] = (GLfloat)ay_prefs.shr;
  color[1] = (GLfloat)ay_prefs.shg;
  color[2] = (GLfloat)ay_prefs.shb;
  color[3] = (GLfloat)1.0;

  glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, color);
  glMatrixMode(GL_MODELVIEW);

  if(view->drawlevel)
    {
      o = ay_currentlevel->object;
      glPushMatrix();
      if(ay_currentlevel->object != ay_root)
	{
	  ay_trafo_getall(ay_currentlevel->next);
	}
    }

  if(!view->drawsel)
    {
      while(o)
	{
	  ay_shade_object(togl, o, AY_FALSE);
	  o = o->next;
	} /* while */
    } /* if */

  if(sel)
    {
      /* process the selected objects */
      glPushMatrix();
      if(!view->drawlevel)
	{
	  if(ay_currentlevel->object != ay_root)
	    {
	      ay_trafo_getall(ay_currentlevel->next);
	    }
	}

      /* all objects have been shaded already, if
         DrawSelectionOnly is _not_ enabled */
      if(view->drawsel)
	{
	  while(sel)
	    {
	      ay_shade_object(togl, sel->object, AY_FALSE);
	      sel = sel->next;
	    } /* while */
	} /* if */

      /* draw handles of selected objects */
      if(view->drawhandles && view->shade < 2)
	{
	  /* let all handles appear "on top" of current drawing
	  * Do we really want this? In a shaded view, the user expects
	  * probably removal of hidden bits. On the other hand, he
	  * might not be able to reach all handles he wants to then,
	  * when modelling in a shaded view. So, for now, hide the
	  * handles in perspective views, as those are thought for
	  * review, not modelling anyway.
	  */
	  if(view->type != AY_VTPERSP)
	    {
	      glClear(GL_DEPTH_BUFFER_BIT);
	    }

	  glDisable(GL_LIGHTING);

	  /* set size of points */
	  glPointSize((GLfloat)ay_prefs.handle_size);

	  /* set color for selected objects */
	  glColor3f((GLfloat)ay_prefs.ser, (GLfloat)ay_prefs.seg,
		    (GLfloat)ay_prefs.seb);

	  arr = ay_drawhcbt.arr;
	  sel = ay_selection;

	  while(sel)
	    {
	      o = sel->object;

	      if(!o->hide)
		{
		  glPushMatrix();
		   glTranslated((GLdouble)o->movx, (GLdouble)o->movy,
				(GLdouble)o->movz);
		   ay_quat_torotmatrix(o->quat, m);
		   glMultMatrixd((GLdouble*)m);
		   glScaled((GLdouble)o->scalx, (GLdouble)o->scaly,
			    (GLdouble)o->scalz);

		   cb = (ay_drawcb *)(arr[o->type]);

		   if(cb)
		     {
		       ay_status = cb(togl, o);
		       if(ay_status)
			 {
			   ay_error(ay_status, fname,
				    "draw handle callback failed");
			 }
		     }

		  glPopMatrix();
		} /* if */
	      sel = sel->next;
	    } /* while */

	  /* draw selected points */
	  if(view->type != AY_VTPERSP)
	    {
	      glDisable(GL_DEPTH_TEST);
	    }
	  else
	    {
	      glDepthRange(0.0, 0.9999999);
	      glDepthFunc(GL_LEQUAL);
	    }

	  /* set color for selected points */
	  glColor3f((GLfloat)ay_prefs.tpr, (GLfloat)ay_prefs.tpg,
		    (GLfloat)ay_prefs.tpb);

	  sel = ay_selection;
	  while(sel)
	    {
	      o = sel->object;

	      if(!o->hide && o->selp)
		{
		  glPushMatrix();
		   glTranslated((GLdouble)o->movx, (GLdouble)o->movy,
				(GLdouble)o->movz);
		   ay_quat_torotmatrix(o->quat, m);
		   glMultMatrixd((GLdouble*)m);
		   glScaled((GLdouble)o->scalx, (GLdouble)o->scaly,
			    (GLdouble)o->scalz);

		   point = o->selp;
		   glBegin(GL_POINTS);
		    while(point)
		      {
			glVertex3d((GLdouble)point->point[0],
				   (GLdouble)point->point[1],
				   (GLdouble)point->point[2]);

			point = point->next;
		      }
		   glEnd();

		  glPopMatrix();
		}
	      sel = sel->next;
	    } /* while */

	  /* set color for selected objects */
	  glColor3f((GLfloat)ay_prefs.ser, (GLfloat)ay_prefs.seg,
		    (GLfloat)ay_prefs.seb);

	  if(view->type != AY_VTPERSP)
	    {
	      glEnable(GL_DEPTH_TEST);
	    }
	  else
	    {
	      glDepthRange(0.0, 1.0);
	      glDepthFunc(GL_LESS);
	    }
	} /* if */
      glPopMatrix();
    } /* if */

  glDisable(GL_LIGHTING);

  /* draw marked point in space */
  if(view->drawmark)
    {
      glColor3f((GLfloat)ay_prefs.tpr, (GLfloat)ay_prefs.tpg,
		(GLfloat)ay_prefs.tpb);
      glDisable(GL_DEPTH_TEST);
      glMatrixMode(GL_PROJECTION);
      glPushMatrix();
       glLoadIdentity();
       glOrtho(0, width, 0, height, -100.0, 100.0);
       glMatrixMode(GL_MODELVIEW);
       glPushMatrix();
        glLoadIdentity();
        glBegin(GL_LINES);
	 glVertex3d(view->markx-3.0, height-view->marky, 0.0);
	 glVertex3d(view->markx+4.0, height-view->marky, 0.0);
	 glVertex3d(view->markx, height-view->marky+3.0, 0.0);
	 glVertex3d(view->markx, height-view->marky-4.0, 0.0);
	glEnd();
       glPopMatrix();
       glMatrixMode(GL_PROJECTION);
      glPopMatrix();
      glEnable(GL_DEPTH_TEST);
    } /* if */

  if(view->drawlevel)
    {
      glMatrixMode(GL_MODELVIEW);
      glPopMatrix();
    }

  glDisable(GL_DITHER);

  /* shade and draw? */
  if(view->shade > 1)
    {
#ifdef GL_VERSION_1_1
      glDisable(GL_POLYGON_OFFSET_FILL);
      glPolygonOffset(0.0, 0.0);
#else
      glDisable(GL_POLYGON_OFFSET_EXT);
      glPolygonOffsetEXT(0.0, 0.0);
#endif

      ay_draw_view(togl, AY_TRUE);
    }

 return AY_OK;
} /* ay_shade_view */

