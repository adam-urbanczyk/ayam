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
 *  if selected is false, selected objects
 *  will not be drawn!
 */
int
ay_draw_object(struct Togl *togl, ay_object *o, int selected)
{
 int ay_status = AY_OK;
 char fname[] = "draw_object";
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
   
  o->glname = ++ay_current_glname;
 
  glPushMatrix();

   glTranslated((GLdouble)o->movx, (GLdouble)o->movy, (GLdouble)o->movz);
   ay_quat_torotmatrix(o->quat, m);
   glMultMatrixd((GLdouble*)m);
   glScaled((GLdouble)o->scalx, (GLdouble)o->scaly, (GLdouble)o->scalz);
  
   arr = ay_drawcbt.arr;
   cb = (ay_drawcb *)(arr[o->type]);

   if (selected != AY_TRUE)
     glPushName(o->glname);

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

   if(!o->hidechilds && o->down)
     {
       down = o->down;
       while(down->next)
	 {
	   ay_status = ay_draw_object(togl, down, selected);
	   down = down->next;
	 } /* while */
     }

  if (selected != AY_TRUE)
    glPopName();

  glPopMatrix();

 return AY_OK;
} /* ay_draw_object */


/* ay_draw_view:
 *  draw a view
 */
int
ay_draw_view(struct Togl *togl)
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

  if(view->drawlevel)
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

      while(sel)
	{
	  ay_status = ay_draw_object(togl, sel->object, AY_TRUE);
	  sel = sel->next;
	} /* while */

      /* let all handles appear "on top" of current drawing */
      glClear(GL_DEPTH_BUFFER_BIT);

      /* draw handles of selected objects */
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

      glPopMatrix();
    } /* if */


  /* draw rectangle */
  if(view->drawrect)
    {
      glColor3d((GLdouble)ay_prefs.tpr, (GLdouble)ay_prefs.tpg,
		(GLdouble)ay_prefs.tpb);

      glMatrixMode(GL_PROJECTION);
      glPushMatrix();
       glLoadIdentity();
       glOrtho(0, width, 0, height, -100.0, 100.0);
       glMatrixMode(GL_MODELVIEW);
       glPushMatrix();
        glLoadIdentity();
        glBegin(GL_LINE_LOOP);
	 glVertex3d(view->rect_xmin, height-view->rect_ymin, 0.0);
	 glVertex3d(view->rect_xmax, height-view->rect_ymin, 0.0);
	 glVertex3d(view->rect_xmax, height-view->rect_ymax, 0.0);
	 glVertex3d(view->rect_xmin, height-view->rect_ymax, 0.0);
	glEnd();
       glPopMatrix();
       glMatrixMode(GL_PROJECTION);
      glPopMatrix();
    } /* if */

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


  if(view->drawlevel)
    {
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
 int width = Togl_Width(togl);
 int height = Togl_Height(togl);
 double aspect = (double)width / (double)height;
 double grid = view->grid;
 double j;
 double minwinx = 0.0, minwiny = 0.0, maxwinx = 0.0, maxwiny = 0.0;

  if(grid < 1E-6)
    return;

  switch(view->type)
    {
    case AY_VTFRONT:
    case AY_VTTRIM:
      if(aspect > 1.0)
	{
	  minwinx = floor(((-aspect * view->zoom) - fabs(view->from[0]))/
			  (view->grid<1.0?view->grid:1.0/view->grid));
	  minwiny = floor(((-view->zoom) - fabs(view->from[1]))/
			  (view->grid<1.0?view->grid:1.0/view->grid));
	}
      else
	{
	  minwiny = floor(((view->zoom/-aspect) - fabs(view->from[1]))/
			  (view->grid<1.0?view->grid:1.0/view->grid));
	  minwinx = floor(((-view->zoom) - fabs(view->from[0]))/
			  (view->grid<1.0?view->grid:1.0/view->grid));
	}
      break;
    case AY_VTSIDE:
      if(aspect > 1.0)
	{
	  minwinx = floor(((-aspect * view->zoom) - fabs(view->from[2]))/
			  (view->grid<1.0?view->grid:1.0/view->grid));
	  minwiny = floor(((-view->zoom) - fabs(view->from[1]))/
			  (view->grid<1.0?view->grid:1.0/view->grid));
	}
      else
	{
	  minwiny = floor(((view->zoom/-aspect) - fabs(view->from[1]))/
			  (view->grid<1.0?view->grid:1.0/view->grid));
	  minwinx = floor(((-view->zoom) - fabs(view->from[2]))/
			  (view->grid<1.0?view->grid:1.0/view->grid));
	}
      break;
    case AY_VTTOP:
      if(aspect > 1.0)
	{
	  minwinx = floor(((-aspect * view->zoom) - fabs(view->from[0]))/
			  (view->grid<1.0?view->grid:1.0/view->grid));
	  minwiny = floor(((-view->zoom) - fabs(view->from[2]))/
			  (view->grid<1.0?view->grid:1.0/view->grid));
	}
      else
	{
	  minwiny = floor(((view->zoom/-aspect) - fabs(view->from[2]))/
			  (view->grid<1.0?view->grid:1.0/view->grid));
	  minwinx = floor(((-view->zoom) - fabs(view->from[0]))/
			  (view->grid<1.0?view->grid:1.0/view->grid));
	}
      break;
    }

  maxwinx = -minwinx;
  maxwiny = -minwiny;

  
  if(view->local)
    {
      glPushMatrix();
      if(ay_currentlevel && ay_currentlevel->next)
	{
	  ay_trafo_getall(ay_currentlevel->next);
	}
    }

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

  if(view->local)
    {
      glPopMatrix();
    }

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

  if(view->bgimage)
    {
      glDisable(GL_DEPTH_TEST);
      glEnable(GL_TEXTURE_2D);
      glEnable(GL_BLEND);
      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
      glMatrixMode(GL_PROJECTION);
      glPushMatrix();
       glLoadIdentity();
       glOrtho(0.0, (GLdouble) w, 0.0, (GLdouble) h, -1.0, 1.0);

       glMatrixMode(GL_MODELVIEW);
       glPushMatrix();
       glLoadIdentity();
       glColor3d((GLdouble)ay_prefs.ser, (GLdouble)ay_prefs.seg,
		 (GLdouble)ay_prefs.seb);
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
