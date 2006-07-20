/*
 * Ayam, a free 3D modeler for the RenderMan interface.
 *
 * Ayam is copyrighted 1998-2005 by Randolf Schultz
 * (rschultz@informatik.uni-rostock.de) and others.
 *
 * All rights reserved.
 *
 * See the file License for details.
 *
 */

#include "ayam.h"

unsigned int ay_current_glname = 0;

/* draw.c - functions for drawing a scene using OpenGL */

/* ay_draw_selp:
 *  draw selected points of object o (if there are any)
 */
void
ay_draw_selp(ay_object *o)
{
 ay_point_object *point = NULL;

  if(o->selp)
    {
      glDisable(GL_DEPTH_TEST);

      /* set color for selected points */
      glColor3d((GLdouble)ay_prefs.tpr, (GLdouble)ay_prefs.tpg,
		(GLdouble)ay_prefs.tpb);

      point = o->selp;
      glBegin(GL_POINTS);

       while(point)
	 {
	   glVertex3d((GLdouble)point->point[0], (GLdouble)point->point[1],
		      (GLdouble)point->point[2]);

	   point = point->next;
	 }

       /* set color for selected objects */
       glColor3d((GLdouble)ay_prefs.ser, (GLdouble)ay_prefs.seg,
		 (GLdouble)ay_prefs.seb);
      glEnd();

      glEnable(GL_DEPTH_TEST);
    }

 return;
} /* ay_draw_selp */


/* ay_draw_object:
 *  draw a single object o (and children) in view togl
 *  o if selected is AY_FALSE, selected objects
 *    will not be drawn!
 *  o if selected is 2, a name will be pushed onto OpenGLs
 *    name stack additionally.
 */
int
ay_draw_object(struct Togl *togl, ay_object *o, int selected)
{
 int ay_status = AY_OK;
 char fname[] = "draw_object";
 ay_view_object *view = (ay_view_object *)Togl_GetClientData(togl);
 void **arr = NULL;
 ay_drawcb *cb = NULL;
 ay_object *down;
 double m[16];

  if(selected == AY_FALSE)
    if(o->selected)
      return AY_OK;

  if(o->hide)
    {
      o->glname = 0;
      return AY_OK;
    }

  glPushMatrix();

   glTranslated((GLdouble)o->movx, (GLdouble)o->movy, (GLdouble)o->movz);
   ay_quat_torotmatrix(o->quat, m);
   glMultMatrixd((GLdouble*)m);
   glScaled((GLdouble)o->scalx, (GLdouble)o->scaly, (GLdouble)o->scalz);

   if(selected == AY_TRUE)
     {
       if(view->drawobjectcs)
	 {
	   ay_draw_cs(togl);
	 }
     }

   arr = ay_drawcbt.arr;
   cb = (ay_drawcb *)(arr[o->type]);

   if(selected == 2)
     {
       o->glname = ++ay_current_glname;
       glPushName(o->glname);
     }

   if(cb)
     ay_status = cb(togl, o);

   if(ay_status)
     {
       ay_error(AY_ERROR, fname, "draw callback failed");

       glPopMatrix();
       return AY_ERROR;
     }

   if(!o->inherit_trafos)
     glLoadIdentity();

   if(!o->hide_children && o->down)
     {
       down = o->down;
       while(down->next)
	 {
	   ay_status = ay_draw_object(togl, down, selected);
	   down = down->next;
	 } /* while */
     }

  if(selected == 2)
    {
      glPopName();
    }

  glPopMatrix();

 return AY_OK;
} /* ay_draw_object */


/* ay_draw_view:
 *  draw a view
 */
int
ay_draw_view(struct Togl *togl, int draw_offset)
{
 int ay_status = AY_OK;
 /* char fname[] = "draw_view";*/
 ay_view_object *view = (ay_view_object *)Togl_GetClientData(togl);
 int width = Togl_Width(togl), height = Togl_Height(togl);
 ay_list_object *sel = ay_selection;
 ay_object *o = ay_root;
 void **arr = NULL;
 ay_drawcb *cb = NULL;
 double m[16];

  glDisable(GL_LIGHTING);
  glMatrixMode(GL_MODELVIEW);

  if(!draw_offset)
    {
      if(view->dirty)
	{
	  ay_toglcb_reshape(togl);
	  view->dirty = AY_FALSE;
	}

      if(view->drawbg)
	{
	  ay_draw_bgimage(togl);
	}

      /* draw grid */
      if(view->drawgrid)
	{
	  ay_draw_grid(togl);
	}
    }

  /* draw bounds of parametric space of current NURBS patch */
  if(view->type == AY_VTTRIM)
    {
      ay_draw_trimview();
    }

  if(view->drawlevel || view->type == AY_VTTRIM)
    {
      o = ay_currentlevel->object;
      glPushMatrix();
      ay_trafo_getall(ay_currentlevel->next);
    }

  /* set color for deselected objects */
  glColor3d((GLdouble)ay_prefs.obr, (GLdouble)ay_prefs.obg,
	    (GLdouble)ay_prefs.obb);

  if(!view->drawsel)
    {
      while(o->next)
	{
	  ay_status = ay_draw_object(togl, o, AY_FALSE);
	  o = o->next;
	} /* while */
    } /* if */

  /* draw selected objects */
  if(sel)
    {
      /* set color for selected objects */
      glColor3d((GLdouble)ay_prefs.ser, (GLdouble)ay_prefs.seg,
		(GLdouble)ay_prefs.seb);

      glPushMatrix();
      if(!view->drawlevel)
	ay_trafo_getall(ay_currentlevel->next);

      /* let selected objects appear "on top" of current drawing */
      if(!draw_offset)
	{
	  glDisable(GL_DEPTH_TEST);
	}
      else
	{
	  glDepthFunc(GL_LEQUAL);
	} /* if */

       while(sel)
	 {
	   ay_status = ay_draw_object(togl, sel->object, AY_TRUE);
	   sel = sel->next;
	 } /* while */
      if(!draw_offset)
	{
	  glEnable(GL_DEPTH_TEST);
	}
      else
	{
	  glDepthFunc(GL_LESS);
	} /* if */

      /* draw handles of selected objects */
      if(view->drawhandles)
	{
	  /* let all handles appear "on top" of current drawing;     */
	  /* we cannot use the glDisable(GL_DEPTH_TEST);-method here */
	  /* because we need the Z-values for vertice picking...     */
	  glClear(GL_DEPTH_BUFFER_BIT);

	  /* set size of points */
	  glPointSize((GLfloat)ay_prefs.handle_size);

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
		    }

		  /* draw selected points */
		  if(o->selp)
		    ay_draw_selp(o);

		  glPopMatrix();
		}
	      sel = sel->next;
	    } /* while */
	} /* if */
      glPopMatrix();
    } /* if */

  if(!draw_offset)
    {
      /* draw marker */
      if(view->drawmarker)
	{
	  glColor3d((GLdouble)ay_prefs.tpr, (GLdouble)ay_prefs.tpg,
		    (GLdouble)ay_prefs.tpb);
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
    } /* if */

  if(view->drawlevel || view->type == AY_VTTRIM)
    {
      glMatrixMode(GL_MODELVIEW);
      glPopMatrix();
    }

 return AY_OK;
} /* ay_draw_view */


/* ay_draw_grid:
 *  draw the current grid
 */
void
ay_draw_grid(struct Togl *togl)
{
 ay_view_object *view = (ay_view_object *)Togl_GetClientData(togl);
 ay_object *o;
 int width = Togl_Width(togl);
 int height = Togl_Height(togl);
 double aspect = (double)width / (double)height;
 double grid = view->grid;
 double j, dx, dy;
 double minwinx = 0.0, minwiny = 0.0, maxwinx = 0.0, maxwiny = 0.0;
 double m[16];
 GLdouble mp[16], mm[16], owinx, owiny, owinz, gwinx, gwiny, gwinz;
 GLint vp[4];

  if(view->local && view->type != AY_VTPERSP)
    {
      glGetIntegerv(GL_VIEWPORT, vp);
      glMatrixMode(GL_PROJECTION);
      glGetDoublev(GL_PROJECTION_MATRIX, mp);
      glMatrixMode(GL_MODELVIEW);
      glPushMatrix();
       if(ay_currentlevel && ay_currentlevel->next)
	 {
	   ay_trafo_getall(ay_currentlevel->next);
	 }
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

      gluProject(0.0, 0.0, 0.0, mm, mp, vp, &owinx, &owiny, &owinz);

      switch(view->type)
	{
	case AY_VTFRONT:
	case AY_VTTRIM:
	  gluProject(view->grid, view->grid, 0.0, mm, mp, vp,
		     &gwinx, &gwiny, &gwinz);
	  break;
	case AY_VTSIDE:
	  gluProject(0.0, view->grid, view->grid, mm, mp, vp,
		     &gwinx, &gwiny, &gwinz);
	  break;
	case AY_VTTOP:
	  gluProject(view->grid, 0.0, view->grid, mm, mp, vp,
		     &gwinx, &gwiny, &gwinz);
	  break;
	}

      dx = fabs(gwinx-owinx);
      if(dx < 1.0)
	dx = 1.0;

      dy = fabs(gwiny-owiny);
      if(dy < 1.0)
	dy = 1.0;

      glColor3d((GLdouble)ay_prefs.grr, (GLdouble)ay_prefs.grg,
		(GLdouble)ay_prefs.grb);

      glMatrixMode(GL_PROJECTION);
      glPushMatrix();
       glLoadIdentity();
       glOrtho(0, width, 0, height, -100.0, 100.0);
       glMatrixMode(GL_MODELVIEW);
       glPushMatrix();
        glLoadIdentity();
	glBegin(GL_LINES);
	for(j = owinx; j < width; j += dx)
	  {
	    glVertex3d(j,    0.0, 0.0);
	    glVertex3d(j, height, 0.0);
	  }
	for(j = owinx; j > 0.0; j -= dx)
	  {
	    glVertex3d(j,    0.0, 0.0);
	    glVertex3d(j, height, 0.0);
	  }
	for(j = owiny; j < height; j += dy)
	  {
	    glVertex3d(  0.0, j, 0.0);
	    glVertex3d(width, j, 0.0);
	  }
	for(j = owiny; j > 0.0; j -= dy)
	  {
	    glVertex3d(  0.0, j, 0.0);
	    glVertex3d(width, j, 0.0);
	  }
	glEnd();
       glPopMatrix();
       glMatrixMode(GL_PROJECTION);
      glPopMatrix();
      glMatrixMode(GL_MODELVIEW);
    }
  else
    {

      if(grid < 1E-6)
	return;

      switch(view->type)
	{
	case AY_VTFRONT:
	case AY_VTTRIM:
	  if(aspect > 1.0)
	    {
	      minwinx = ((-aspect * view->zoom) + view->from[0]) - grid;
	      minwiny = ((-view->zoom) + view->from[1]) - grid;
	    }
	  else
	    {
	      minwiny = ((view->zoom/-aspect) + view->from[1]) - grid;
	      minwinx = ((-view->zoom) + view->from[0] - grid);
	    }
	  break;
	case AY_VTSIDE:
	  if(aspect > 1.0)
	    {
	      minwinx =  ((-aspect * view->zoom) + view->from[2]) - grid;
	      minwiny = ((-view->zoom) + view->from[1]) - grid;
	    }
	  else
	    {
	      minwiny = ((view->zoom/-aspect) + view->from[1]) - grid;
	      minwinx = ((-view->zoom) + view->from[2] - grid);
	    }
	  break;
	case AY_VTTOP:
	  if(aspect > 1.0)
	    {
	      minwinx = ((-aspect * view->zoom) + view->from[0]) - grid;
	      minwiny = ((-view->zoom) + view->from[2]) - grid;
	    }
	  else
	    {
	      minwiny = ((-aspect * view->zoom) + view->from[2]) - grid;
	      minwinx = ((-view->zoom) + view->from[0] - grid);
	    }
	  break;
	}

      if(aspect > 1.0)
	{
	  maxwinx = minwinx + 2*((aspect * view->zoom)) + grid;
	  maxwiny = minwiny + 2*(view->zoom) + grid;
	}
      else
	{
	  maxwinx = minwinx + 2*((view->zoom/aspect)) + grid;
	  maxwiny = minwiny + 2*(-view->zoom) + grid;
	}

      /*printf("%g %g %g %g\n",minwinx,maxwinx,minwiny,maxwiny);*/
      glColor3d((GLdouble)ay_prefs.grr, (GLdouble)ay_prefs.grg,
		(GLdouble)ay_prefs.grb);

      glBegin(GL_LINES);

      switch(view->type)
	{
	case AY_VTFRONT:
	case AY_VTTRIM:
	  /* vertical lines */
	  for(j = 0.0; j < maxwinx; j += view->grid)
	    {
	      glVertex3d((GLdouble)j, (GLdouble)minwiny, (GLdouble)0.0);
	      glVertex3d((GLdouble)j, (GLdouble)maxwiny, (GLdouble)0.0);
	    }
	  for(j = (0.0-view->grid); j > minwinx; j -= view->grid)
	    {
	      glVertex3d((GLdouble)j, (GLdouble)minwiny, (GLdouble)0.0);
	      glVertex3d((GLdouble)j, (GLdouble)maxwiny, (GLdouble)0.0);
	    }

	  /* horizontal lines */
	  for(j = 0.0; j < maxwiny; j += view->grid)
	    {
	      glVertex3d((GLdouble)minwinx, (GLdouble)j, (GLdouble)0.0);
	      glVertex3d((GLdouble)maxwinx, (GLdouble)j, (GLdouble)0.0);
	    }
	  for(j = (0.0-view->grid); j > minwiny; j -= view->grid)
	    {
	      glVertex3d((GLdouble)minwinx, (GLdouble)j, (GLdouble)0.0);
	      glVertex3d((GLdouble)maxwinx, (GLdouble)j, (GLdouble)0.0);
	    }
	  break;
	case AY_VTSIDE:
	  /* vertical lines */
	  for(j = 0.0; j < maxwinx; j += view->grid)
	    {
	      glVertex3d((GLdouble)0.0, (GLdouble)minwiny, (GLdouble)j);
	      glVertex3d((GLdouble)0.0, (GLdouble)maxwiny, (GLdouble)j);
	    }
	  for(j = (0.0-view->grid); j > minwinx; j -= view->grid)
	    {
	      glVertex3d((GLdouble)0.0, (GLdouble)minwiny, (GLdouble)j);
	      glVertex3d((GLdouble)0.0, (GLdouble)maxwiny, (GLdouble)j);
	    }

	  /* horizontal lines */
	  for(j = 0.0; j < maxwiny; j += view->grid)
	    {
	      glVertex3d((GLdouble)0.0, (GLdouble)j, (GLdouble)minwinx);
	      glVertex3d((GLdouble)0.0, (GLdouble)j, (GLdouble)maxwinx);
	    }
	  for(j = (0.0-view->grid); j > minwiny; j -= view->grid)
	    {
	      glVertex3d((GLdouble)0.0, (GLdouble)j, (GLdouble)minwinx);
	      glVertex3d((GLdouble)0.0, (GLdouble)j, (GLdouble)maxwinx);
	    }
	  break;
	case AY_VTTOP:
	  /* vertical lines */
	  for(j = 0.0; j < maxwinx; j += view->grid)
	    {
	      glVertex3d((GLdouble)j, (GLdouble)0.0, (GLdouble)minwiny);
	      glVertex3d((GLdouble)j, (GLdouble)0.0, (GLdouble)maxwiny);
	    }
	  for(j = (0.0-view->grid); j > minwinx; j -= view->grid)
	    {
	      glVertex3d((GLdouble)j, (GLdouble)0.0, (GLdouble)minwiny);
	      glVertex3d((GLdouble)j, (GLdouble)0.0, (GLdouble)maxwiny);
	    }

	  /* horizontal lines */
	  for(j = 0.0; j < maxwiny; j += view->grid)
	    {
	      glVertex3d((GLdouble)minwinx, (GLdouble)0.0, (GLdouble)j);
	      glVertex3d((GLdouble)maxwinx, (GLdouble)0.0, (GLdouble)j);
	    }
	  for(j = (0.0-view->grid); j > minwiny; j -= view->grid)
	    {
	      glVertex3d((GLdouble)minwinx, (GLdouble)0.0, (GLdouble)j);
	      glVertex3d((GLdouble)maxwinx, (GLdouble)0.0, (GLdouble)j);
	    }
	  break;
	case AY_VTPERSP:
	  minwinx = -(25.0 * view->grid);
	  minwiny = -(25.0 * view->grid);
	  maxwinx = (25.0 * view->grid);
	  maxwiny = (25.0 * view->grid);
	  /* draw lines */
	  for(j = -25.0 * view->grid; j < maxwinx; j += view->grid)
	    {
	      /* vertical */
	      glVertex3d((GLdouble)j, (GLdouble)0.0, (GLdouble)minwiny);
	      glVertex3d((GLdouble)j, (GLdouble)0.0, (GLdouble)maxwiny);
	      /* horizontal */
	      glVertex3d((GLdouble)minwinx, (GLdouble)0.0, (GLdouble)j);
	      glVertex3d((GLdouble)maxwinx, (GLdouble)0.0, (GLdouble)j);
	    }
	  break;
	}

      glEnd();
    } /* if */

  glClear(GL_DEPTH_BUFFER_BIT);

 return;
} /* ay_draw_grid */


/* ay_draw_arrow:
 *  draw an arrow
 */
void
ay_draw_arrow(struct Togl *togl, double *from, double *to)
{
 GLdouble mvm[16], pm[16], win1x, win1y, win1z, win2x, win2y, win2z;
 GLdouble p1x, p1y, p2x, p2y, alpha;
 GLint vp[4];

  glDisable(GL_DEPTH_TEST);
  glGetDoublev(GL_MODELVIEW_MATRIX, mvm);
  glGetDoublev(GL_PROJECTION_MATRIX, pm);
  glGetIntegerv(GL_VIEWPORT, vp);

  gluProject((GLdouble)to[0], (GLdouble)to[1],
	     (GLdouble)to[2], mvm, pm, vp,
	     &win1x, &win1y, &win1z);

  gluProject((GLdouble)from[0], (GLdouble)from[1],
	     (GLdouble)from[2], mvm, pm, vp,
	     &win2x, &win2y, &win2z);

  p1x = win1x+AY_POINTER;
  p1y = win1y+AY_POINTER;

  p2x = win1x+AY_POINTER;
  p2y = win1y-AY_POINTER;

  if(win2x-win1x != 0.0)
    {
      alpha = AY_R2D(atan((win2y-win1y)/(win2x-win1x)));

      if(win2x<win1x)
	alpha = 180.0+alpha;
    }
  else
    {
      if(win2y>win1y)
	alpha = 90.0;
      else
	alpha = -90.0;
    }

  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
   glLoadIdentity();
   glOrtho(0, Togl_Width(togl), 0, Togl_Height(togl), -100.0, 100.0);

   glMatrixMode(GL_MODELVIEW);
   glPushMatrix();
    glLoadIdentity();
    glTranslated(win1x,win1y,0.0);
    glRotated(alpha,0.0,0.0,1.0);
    glTranslated(-win1x,-win1y,0.0);
    glBegin(GL_LINE_STRIP);
     glVertex3d(p1x,p1y,win1z);
     glVertex3d(win1x,win1y,win1z);
     glVertex3d(p2x,p2y,win1z);
    glEnd();

    glPopMatrix();
   glMatrixMode(GL_PROJECTION);
   glPopMatrix();
  glMatrixMode(GL_MODELVIEW);
  glEnable(GL_DEPTH_TEST);

 return;
} /* ay_draw_arrow */


/* ay_draw_bgimage:
 *  draw background image (if there is one)
 */
void
ay_draw_bgimage(struct Togl *togl)
{
 ay_view_object *view = (ay_view_object *)Togl_GetClientData(togl);
 GLint w = Togl_Width(togl),h = Togl_Height(togl);
 GLfloat color[4] = {0.0f,0.0f,0.0f,0.0f};

  if(view->bgimage)
    {
      glDisable(GL_DEPTH_TEST);
      glEnable(GL_TEXTURE_2D);
      glEnable(GL_BLEND);
      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
      glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
      glMatrixMode(GL_PROJECTION);
      glPushMatrix();
       glLoadIdentity();
       glOrtho(0.0, (GLdouble) w, 0.0, (GLdouble) h, -1.0, 1.0);

       glMatrixMode(GL_MODELVIEW);
       glPushMatrix();
       glLoadIdentity();

       glColor3d((GLdouble)ay_prefs.bgr, (GLdouble)ay_prefs.bgg,
		 (GLdouble)ay_prefs.bgb);

       color[0] = (GLfloat)ay_prefs.bgr;
       color[1] = (GLfloat)ay_prefs.bgg;
       color[2] = (GLfloat)ay_prefs.bgb;
       glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, color);

       glBegin(GL_QUADS);
        glTexCoord2i(0, 0);
	glVertex3i(0, 0, 0);

        glTexCoord2i(0, 1);
	glVertex3i(0, h, 0);

        glTexCoord2i(1, 1);
	glVertex3i(w, h, 0);

        glTexCoord2i(1, 0);
	glVertex3i(w, 0, 0);
       glEnd();

     glPopMatrix();
     glMatrixMode(GL_PROJECTION);
     glPopMatrix();
     glMatrixMode(GL_MODELVIEW);
     glDisable(GL_BLEND);
     glEnable(GL_DEPTH_TEST);
     glDisable(GL_TEXTURE_2D);

    } /* if */

 return;
} /* ay_draw_bgimage */


/* ay_draw_needredraw:
 *  For two lists of objects in the scene, decide whether a redraw
 *  is necessary when the objects in oldsel are deselected and the
 *  objects in newsel are selected (if e.g. oldsel contains a material
 *  and newsel a hidden object, no redraw is necessary, because nothing
 *  would change in any view anyway).
 *  Both lists may be empty.
 */
void
ay_draw_needredraw(ay_list_object *oldsel, ay_list_object *newsel,
		   int *result)
{
 int oldseldrawn = AY_FALSE, newseldrawn = AY_FALSE;
 ay_object *o = NULL;
 ay_list_object *s = NULL, *os = NULL, *ns = NULL;

  *result = AY_TRUE;

  /* check, whether any objects in old selection were actually drawn */
  s = oldsel;
  while(s)
    {
      o = s->object;
      if(o->type != AY_IDMATERIAL)
	{
	  if(!o->hide)
	    {
	      oldseldrawn = AY_TRUE;
	    }
	}
      s = s->next;
    }

  /* check, whether any objects in new selection will actually be drawn */
  s = newsel;
  while(s)
    {
      o = s->object;
      if(o->type != AY_IDMATERIAL)
	{
	  if(!o->hide)
	    {
	      newseldrawn = AY_TRUE;
	    }
	}
      s = s->next;
    }

  /* if oldsel was invisible and newsel too => we need no redraw */
  if(!oldseldrawn && !newseldrawn)
    {
      *result = AY_FALSE;
      return;
    }

  /* check for identical selections */
  if(oldsel && newsel)
    {
      if(oldsel->next || newsel->next)
	{
	  /* check multiple object selections */
	  os = oldsel;
	  ns = newsel;
	  /* XXXX we would rather need to find matching objects
	     for all objects of <os> in <ns> and check for objects
	     without matches here, but this simple check should
	     be sufficient this time */
	  while(os && ns)
	    {
	      /* compare objects */
	      if(os->object != ns->object)
		{
		  return;
		}
	      os = os->next;
	      ns = ns->next;
	      /* check for premature end of one of both lists */
	      if(((!os) && ns) || (os && (!ns)))
		{
		  return;
		}
	    } /* while */
	  *result = AY_FALSE;
	}
      else
	{
	  /* check single object selections */
	  if(oldsel->object == newsel->object)
	    {
	      *result = AY_FALSE;
	      return;
	    }
	} /* if */
    } /* if */

 return;
} /* ay_draw_needredraw */


/* ay_draw_trimview:
 *  draw a view of type Trim by drawing the
 *  bounds of parametric space of current NURBS patch as rectangle
 */
void
ay_draw_trimview(void)
{
 ay_nurbpatch_object *patch = NULL;
 GLfloat minx, maxx, miny, maxy;

 /* check, whether we are not in the root level */
 if(ay_currentlevel && ay_currentlevel->object != ay_root)
   {
     /* check, whether the current level is inside a NURBPatch object */
     if(ay_currentlevel->next &&
	(ay_currentlevel->next->object->type == AY_IDNPATCH))
       {
	 /* it is; get the patch */
	 patch = (ay_nurbpatch_object *)
	   (ay_currentlevel->next->object->refine);
       }

     /* if the first test failed, we can still be inside a level
	inside a NURBPatch object (a trimloop level), are we? */
     if(!patch)
       {
	 if(ay_currentlevel->next && ay_currentlevel->next->next &&
	    ay_currentlevel->next->next->next &&
	    ay_currentlevel->next->next->next->object &&
	    (ay_currentlevel->next->next->next->object->type == AY_IDNPATCH))
	   {

	     /* we are; get the patch */
	     patch = (ay_nurbpatch_object *)
	       (ay_currentlevel->next->next->next->object->refine);
	   }
       }

     /* draw the bounds of the parametric space of the patch as rectangle */
     if(patch)
       {
	 minx = (GLfloat)((patch->uknotv)[0]);
	 miny = (GLfloat)((patch->vknotv)[0]);

	 maxx = (GLfloat)((patch->uknotv)[patch->width + patch->uorder - 1]);
	 maxy = (GLfloat)((patch->vknotv)[patch->height + patch->vorder - 1]);

	 glColor3d((GLdouble)ay_prefs.ser, (GLdouble)ay_prefs.seg,
		   (GLdouble)ay_prefs.seb);

	 glBegin(GL_LINE_LOOP);
	  glVertex3f(minx, miny, 0.0);
	  glVertex3f(maxx, miny, 0.0);
	  glVertex3f(maxx, maxy, 0.0);
	  glVertex3f(minx, maxy, 0.0);
	 glEnd();

	 glColor3d((GLdouble)ay_prefs.obr, (GLdouble)ay_prefs.obg,
		   (GLdouble)ay_prefs.obb);

       } /* if */
   } /* if */

 return;
} /* ay_draw_trimview */


/* ay_draw_rectangle:
 *
 */
void
ay_draw_rectangle(int winwidth, int winheight,
		  double xmin, double ymin, double xmax, double ymax)
{

  glDisable(GL_DEPTH_TEST);
  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
   glLoadIdentity();
   glOrtho(0, winwidth, 0, winheight, -100.0, 100.0);
   glMatrixMode(GL_MODELVIEW);
   glPushMatrix();
    glLoadIdentity();
    glBegin(GL_LINE_LOOP);
     glVertex3d(xmin, winheight-ymin, 0.0);
     glVertex3d(xmax, winheight-ymin, 0.0);
     glVertex3d(xmax, winheight-ymax, 0.0);
     glVertex3d(xmin, winheight-ymax, 0.0);
    glEnd();
   glPopMatrix();
   glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  glEnable(GL_DEPTH_TEST);

 return;
} /* ay_draw_rectangle */


/* ay_draw_cs:
 *  draw an object coordinate system consisting of three color
 *  coded and labelled axes
 */
void
ay_draw_cs(struct Togl *togl)
{
 GLdouble mvm[16], pm[16];
 GLdouble win1x, win1y, win1z, win2x, win2y, win2z, win3x, win3y, win3z;
 GLfloat color[4] = {0.0f,0.0f,0.0f,0.0f};
 GLint vp[4];

  glGetFloatv(GL_CURRENT_COLOR, color);

  /* draw */
  glBegin(GL_LINES);
   glColor3f((GLfloat)1.0, (GLfloat)0.0, (GLfloat)0.0);
   glVertex3f((GLfloat)0.0, (GLfloat)0.0, (GLfloat)0.0);
   glVertex3f((GLfloat)1.0, (GLfloat)0.0, (GLfloat)0.0);
   glColor3f((GLfloat)0.0, (GLfloat)1.0, (GLfloat)0.0);
   glVertex3f((GLfloat)0.0, (GLfloat)0.0, (GLfloat)0.0);
   glVertex3f((GLfloat)0.0, (GLfloat)1.0, (GLfloat)0.0);
   glColor3f((GLfloat)0.0, (GLfloat)0.0, (GLfloat)1.0);
   glVertex3f((GLfloat)0.0, (GLfloat)0.0, (GLfloat)0.0);
   glVertex3f((GLfloat)0.0, (GLfloat)0.0, (GLfloat)1.0);
  glEnd();

  glColor4fv(color);

  glGetDoublev(GL_MODELVIEW_MATRIX, mvm);
  glGetDoublev(GL_PROJECTION_MATRIX, pm);
  glGetIntegerv(GL_VIEWPORT, vp);
  gluProject((GLdouble)1.0, (GLdouble)0.0,
	     (GLdouble)0.0, mvm, pm, vp,
	     &win1x, &win1y, &win1z);
  gluProject((GLdouble)0.0, (GLdouble)1.0,
	     (GLdouble)0.0, mvm, pm, vp,
	     &win2x, &win2y, &win2z);
  gluProject((GLdouble)0.0, (GLdouble)0.0,
	     (GLdouble)1.0, mvm, pm, vp,
	     &win3x, &win3y, &win3z);

  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
   glLoadIdentity();
   glOrtho(0, Togl_Width(togl), 0, Togl_Height(togl), -100.0, 100.0);

   glMatrixMode(GL_MODELVIEW);
   glPushMatrix();
    glLoadIdentity();
    glTranslated(((int)win1x)+6.0+0.375, ((int)win1y)+0.375, 0.0);
    /* draw X */
    glBegin(GL_LINE_STRIP);
     glVertex3i(-3, -3, 0);
     glVertex3i(0, 0, 0);
     glVertex3i(-4, 4, 0);
    glEnd();
    glBegin(GL_LINE_STRIP);
     glVertex3i(3, 3, 0);
     glVertex3i(0, 0, 0);
     glVertex3i(4, -4, 0);
    glEnd();
    glLoadIdentity();
    glTranslated(((int)win2x)+6.0+0.375, ((int)win2y)+0.375, 0.0);
    /* draw Y */
    glBegin(GL_LINES);
     glVertex3i(0, 0, 0);
     glVertex3i(-4, 4, 0);
     glVertex3i(0, 0, 0);
     glVertex3i(4, 4, 0);
     glVertex3i(0, 0, 0);
     glVertex3i(-4, -4, 0);
    glEnd();
    glLoadIdentity();
    glTranslated(((int)win3x)+6+0.375, ((int)win3y)+0.375, 0.0);
    /* draw Z */
    glBegin(GL_LINE_STRIP);
     glVertex3i(0, 0, 0);
     glVertex3i(-3, -3, 0);
     glVertex3i(4, -3, 0);
    glEnd();
    glBegin(GL_LINE_STRIP);
     glVertex3i(0, 0, 0);
     glVertex3i(3, 3, 0);
     glVertex3i(-4, 3, 0);
    glEnd();
    glPopMatrix();
   glMatrixMode(GL_PROJECTION);
   glPopMatrix();
  glMatrixMode(GL_MODELVIEW);

 return;
} /* ay_draw_cs */
