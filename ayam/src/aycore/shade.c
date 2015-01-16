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

/* prototypes of functions local to this module: */
int ay_shade_cleansil(struct Togl *togl, int selection, unsigned char *sil);

void ay_shade_thinsil(int w, int h, unsigned char *src);

unsigned char *ay_shade_detectsil(struct Togl *togl, int selection);


/** ay_shade_cleansil:
 *  remove all silhouette points from the direct vicinity
 *  of already drawn wire points
 */
int
ay_shade_cleansil(struct Togl *togl, int selection, unsigned char *sil)
{
 int i, j, w, h, d;
 unsigned char *wires, *w1, *w2, *w3, *p;
 unsigned char col[3];
 int clear = AY_FALSE;
 ay_object *o = ay_root;
 ay_list_object *sel = ay_selection;
 ay_view_object *view = (ay_view_object *)Togl_GetClientData(togl);
 int hascol = AY_FALSE;

  w = Togl_Width(togl);
  h = Togl_Height(togl);

  if(!(wires = malloc(w*h*4*sizeof(unsigned char))))
    return hascol;

  glDepthMask(GL_FALSE);

  if(view->drawlevel || view->type == AY_VTTRIM)
    {
      o = ay_currentlevel->object;
      glPushMatrix();
      if(ay_currentlevel->object != ay_root)
	{
	  ay_trafo_concatparent(ay_currentlevel->next);
	}
    }

  if(selection)
    {
      glPushMatrix();
       if(!view->drawlevel)
	 {
	   if(ay_currentlevel->object != ay_root)
	     {
	       ay_trafo_concatparent(ay_currentlevel->next);
	     }
	 }
       glLineWidth((GLfloat)ay_prefs.sellinewidth);
       col[0] = (unsigned char)(ay_prefs.ser*255);
       col[1] = (unsigned char)(ay_prefs.seg*255);
       col[2] = (unsigned char)(ay_prefs.seb*255);
       glColor3f((GLfloat)ay_prefs.ser, (GLfloat)ay_prefs.seg,
		 (GLfloat)ay_prefs.seb);
       while(sel)
	 {
	   ay_draw_object(togl, sel->object, AY_TRUE);
	   sel = sel->next;
	 }
      glLineWidth((GLfloat)ay_prefs.linewidth);
      glPopMatrix();
    }
  else
    {
      glLineWidth((GLfloat)ay_prefs.linewidth);
      col[0] = (unsigned char)(ay_prefs.obr*255);
      col[1] = (unsigned char)(ay_prefs.obg*255);
      col[2] = (unsigned char)(ay_prefs.obb*255);
      glColor3f((GLfloat)ay_prefs.obr, (GLfloat)ay_prefs.obg,
	    (GLfloat)ay_prefs.obb);
      while(o->next)
	{
	  ay_draw_object(togl, o, AY_FALSE);
	  o = o->next;
	}
    }

  if(view->drawlevel || view->type == AY_VTTRIM)
    {
      glMatrixMode(GL_MODELVIEW);
      glPopMatrix();
    }

  glDepthMask(GL_TRUE);

  glFlush();
  glReadPixels(0, 0, w, h, GL_RGBA, GL_UNSIGNED_BYTE, wires);
  glClear(GL_COLOR_BUFFER_BIT);

  for(i = 1; i < h-1; i++)
    {
      w1 = &(wires[(i-1)*4*w]);
      w2 = w1+w*4;
      w3 = w2+w*4;
      p = &(sil[i*4*w+4]);
      for(j = 1; j < w-1; j++)
	{
	  d = abs(col[0]-p[0])+abs(col[1]-p[1])+abs(col[2]-p[2]);
	  if(d < 3)
	    {
	      hascol = AY_TRUE;
	      clear = AY_FALSE;
	      while(1)
		{
		  d = abs(col[0]-w1[0])+abs(col[1]-w1[1])+abs(col[2]-w1[2]);
		  if(d < 3)
		    { clear = AY_TRUE; break; }
		  d = abs(col[0]-w2[0])+abs(col[1]-w2[1])+abs(col[2]-w2[2]);
		  if(d < 3)
		    { clear = AY_TRUE; break; }
		  d = abs(col[0]-w3[0])+abs(col[1]-w3[1])+abs(col[2]-w3[2]);
		  if(d < 3)
		    { clear = AY_TRUE; break; }
		  d = abs(col[0]-w1[4])+abs(col[1]-w1[5])+abs(col[2]-w1[6]);
		  if(d < 3)
		    { clear = AY_TRUE; break; }
		  /* omitting central pixel */
		  d = abs(col[0]-w3[4])+abs(col[1]-w3[5])+abs(col[2]-w3[6]);
		  if(d < 3)
		    { clear = AY_TRUE; break; }
		  d = abs(col[0]-w1[8])+abs(col[1]-w1[9])+abs(col[2]-w1[10]);
		  if(d < 3)
		    { clear = AY_TRUE; break; }
		  d = abs(col[0]-w2[8])+abs(col[1]-w2[9])+abs(col[2]-w2[10]);
		  if(d < 3)
		    { clear = AY_TRUE; break; }
		  d = abs(col[0]-w3[8])+abs(col[1]-w3[9])+abs(col[2]-w3[10]);
		  if(d < 3)
		    { clear = AY_TRUE; break; }
		  break;
		}
	      if(clear)
		memset(p, 0, 4*sizeof(unsigned char));
	    }
	  w1 += 4;
	  w2 += 4;
	  w3 += 4;
	  p += 4;
	}
    }

  free(wires);

 return hascol;
} /* ay_shade_cleansil */


/** ay_shade_thinsil:
 * execute morphological thinning to reduce the line width coming
 * from the Sobel line detector
 */
void
ay_shade_thinsil(int w, int h, unsigned char *src)
{
 unsigned char *srcp;
 int x, y, num, finished;
 int nw, no, ne, we, ea, sw, so, se;
 static int erasetable[256] = {
   0,0,1,1,0,0,1,1,
   1,1,0,1,1,1,0,1,
   1,1,0,0,1,1,1,1,
   0,0,0,0,0,0,0,1,

   0,0,1,1,0,0,1,1,
   1,1,0,1,1,1,0,1,
   1,1,0,0,1,1,1,1,
   0,0,0,0,0,0,0,1,

   1,1,0,0,1,1,0,0,
   0,0,0,0,0,0,0,0,
   0,0,0,0,0,0,0,0,
   0,0,0,0,0,0,0,0,

   1,1,0,0,1,1,0,0,
   1,1,0,1,1,1,0,1,
   0,0,0,0,0,0,0,0,
   0,0,0,0,0,0,0,0,

   0,0,1,1,0,0,1,1,
   1,1,0,1,1,1,0,1,
   1,1,0,0,1,1,1,1,
   0,0,0,0,0,0,0,1,

   0,0,1,1,0,0,1,1,
   1,1,0,1,1,1,0,1,
   1,1,0,0,1,1,1,1,
   0,0,0,0,0,0,0,0,

   1,1,0,0,1,1,0,0,
   0,0,0,0,0,0,0,0,
   1,1,0,0,1,1,1,1,
   0,0,0,0,0,0,0,0,

   1,1,0,0,1,1,0,0,
   1,1,0,1,1,1,0,0,
   1,1,0,0,1,1,1,0,
   1,1,0,0,1,0,0,0
 };

  finished = AY_FALSE;
  while(!finished)
    {
      finished = AY_TRUE;
      for(y = 1; y < h-1; y++)
	{
	  srcp = src+y*w;
	  x = 1;
	  while(x < w-1)
	    {
	      if(*(srcp+x) == 0)
		{
		  we = *(srcp+x-1);
		  ea = *(srcp+x+1);
		  if((we == 255) || (ea == 255))
		    {
		      nw = *(srcp+x+w-1);
		      no = *(srcp+x+w);
		      ne = *(srcp+x+w+1);
		      sw = *(srcp+x-w-1);
		      so = *(srcp+x-w);
		      se = *(srcp+x-w+1);
		      num = nw/255+no/255*2+ne/255*4+we/255*8+ea/255*16+
			sw/255*32+so/255*64+se/255*128;
		      if(erasetable[num] == 1)
			{
			  *(srcp+x) = 255;
			  finished = AY_FALSE;
			  x++;
			}
		    }
		}
	      x++;
	    }
	}

      for(x = 1; x < w-1; x++)
	{
	  y = 1;
	  while(y < h-1)
	    {
	      srcp = src+y*w;
	      if(*(srcp+x) == 0)
		{
		  no = *(srcp+x+w);
		  so = *(srcp+x-w);
		  if((no == 255) || (so == 255))
		    {
		      nw = *(srcp+x+w-1);
		      ne = *(srcp+x+w+1);
		      we = *(srcp+x-1);
		      ea = *(srcp+x+1);
		      sw = *(srcp+x-w-1);
		      se = *(srcp+x-w+1);
		      num = nw/255+no/255*2+ne/255*4+we/255*8+ea/255*16+
			sw/255*32+so/255*64+se/255*128;
		      if(erasetable[num] == 1)
			{
			  *(srcp+x) = 255;
			  finished = AY_FALSE;
			  y++;
			}
		    }
		}
	      y++;
	    }
	}
    }

 return;
} /* ay_shade_thinsil */


/** ay_shade_detectsil:
 * Create a silhouette texture by shading all objects in a special
 * lighting setup and detect edges in the resulting z- and
 * color-buffer data.
 *
 * \param togl view to use
 * \param selection if AY_TRUE create the silhouettes for
 *  the selected objects only
 *
 * \returns silhouette texture or NULL (in case of an error)
 */
unsigned char *
ay_shade_detectsil(struct Togl *togl, int selection)
{
 ay_view_object *view = (ay_view_object *)Togl_GetClientData(togl);
 ay_list_object *sel = ay_selection;
 ay_object *o = ay_root;
 int i, j, k, w, h;
 float ex, ey, e;
 float *d1, *d2, *d3, thresh = 0.01f;
 unsigned char *c1, *c2, *c3;
 GLfloat light_pos[] = { 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f };
 GLfloat red[] = {1.0f, 0.0f, 0.0f, 1.0f};
 GLfloat green[] = {0.0f, 1.0f, 0.0f, 1.0f};
 GLfloat blue[] = {0.0f, 0.0f, 1.0f, 1.0f};
 GLfloat white[] = {1.0f, 1.0f, 1.0f, 1.0f};
 unsigned char color[4];
 GLfloat sx[9] = {-1,0,1,-2,0,2,-1,0,1}, sy[9]={-1,-2,-1,0,0,0,1,2,1};
 GLfloat *depthimg = NULL;
 int use_materialcolor;
 unsigned char *silimg = NULL, *edges = NULL, *s, *t;

  if(selection && !sel)
    return NULL;

  w = Togl_Width(togl);
  h = Togl_Height(togl);

  if(!(silimg = calloc(w*h*4, sizeof(unsigned char))))
    goto cleanup;

  if(!(edges = malloc(w*h*sizeof(unsigned char))))
    goto cleanup;

  memset(edges, 255, w*h*sizeof(unsigned char));

  /* setup lighting */
  glDisable(GL_DITHER);
  glDisable(GL_LIGHTING);
  if(ay_prefs.sdmode > 1)
    {
      glClearColor((GLfloat)1.0f, (GLfloat)1.0f,
		   (GLfloat)1.0f, (GLfloat)0.0f);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

      glEnable(GL_LIGHTING);
      glLightModelf(GL_LIGHT_MODEL_TWO_SIDE, (GLfloat)1.0);
      glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, GL_TRUE);
      glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, white);

      glMatrixMode(GL_MODELVIEW);
      glPushMatrix();
       glRotated(view->rotz, 0.0, 0.0, 1.0);
       glRotated(view->roty, 0.0, 1.0, 0.0);
       glRotated(view->rotx, 1.0, 0.0, 0.0);
      glPopMatrix();

      glEnable(GL_LIGHT1);
      glEnable(GL_LIGHT2);
      glEnable(GL_LIGHT3);
      glEnable(GL_LIGHT4);
      glEnable(GL_LIGHT5);

      glLightfv(GL_LIGHT0, GL_POSITION, &(light_pos[0]));
      glLightfv(GL_LIGHT0, GL_DIFFUSE, red);
      glLightfv(GL_LIGHT1, GL_POSITION, &(light_pos[1]));
      glLightfv(GL_LIGHT1, GL_DIFFUSE, green);
      glLightfv(GL_LIGHT2, GL_POSITION, &(light_pos[2]));
      glLightfv(GL_LIGHT2, GL_DIFFUSE, blue);

      light_pos[2] = -1.0f;
      red[0] = -1.0f;
      green[1] = -1.0f;
      blue[2] = -1.0f;
      glLightfv(GL_LIGHT3, GL_POSITION, &(light_pos[0]));
      glLightfv(GL_LIGHT3, GL_DIFFUSE, red);
      glLightfv(GL_LIGHT4, GL_POSITION, &(light_pos[1]));
      glLightfv(GL_LIGHT4, GL_DIFFUSE, green);
      glLightfv(GL_LIGHT5, GL_POSITION, &(light_pos[2]));
      glLightfv(GL_LIGHT5, GL_DIFFUSE, blue);
      use_materialcolor = ay_prefs.use_materialcolor;
      ay_prefs.use_materialcolor = AY_FALSE;
    }

  /* shade all objects */
  glMatrixMode(GL_MODELVIEW);

  if(view->drawlevel)
    {
      o = ay_currentlevel->object;
      glPushMatrix();
      if(ay_currentlevel->object != ay_root)
	{
	  ay_trafo_concatparent(ay_currentlevel->next);
	}
    }

  if(!selection && !view->drawsel)
    {
      while(o)
	{
	  if(!o->selected)
	    ay_shade_object(togl, o, AY_FALSE);
	  o = o->next;
	} /* while */
    } /* if */
  else
    {
      if(sel)
	{
	  /* shade the selected objects */
	  glPushMatrix();
	  if(!view->drawlevel)
	    {
	      if(ay_currentlevel->object != ay_root)
		{
		  ay_trafo_concatparent(ay_currentlevel->next);
		}
	    }
	  while(sel)
	    {
	      ay_shade_object(togl, sel->object, AY_FALSE);
	      sel = sel->next;
	    }
	  glPopMatrix();
	} /* if */
    }
  if(view->drawlevel)
    {
      glMatrixMode(GL_MODELVIEW);
      glPopMatrix();
    }

  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  glPixelStorei(GL_PACK_ALIGNMENT, 1);

  glFlush();

  if(ay_prefs.sdmode != 2)
    {
      if(!(depthimg = calloc(w*h, sizeof(GLfloat))))
	goto cleanup;

      /* process z-buffer */
      glReadPixels(0, 0, w, h, GL_DEPTH_COMPONENT, GL_FLOAT, depthimg);

      /* detect edges in z-buffer data using a simple sobel filter */
      for(i = 1; i < h-1; i++)
	{
	  s = &(edges[i*w+1]);
	  d2 = &(depthimg[i*w]);
	  d1 = d2-w;
	  d3 = d2+w;
	  for(j = 1; j < w-1; j++)
	    {
	      ex = 0;
	      ex += d1[0]*sx[0];
	      ex += d2[0]*sx[1];
	      ex += d3[0]*sx[2];

	      ex += d1[1]*sx[3];
	      ex += d2[1]*sx[4];
	      ex += d3[1]*sx[5];

	      ex += d1[2]*sx[6];
	      ex += d2[2]*sx[7];
	      ex += d3[2]*sx[8];

	      ey = 0;
	      ey += d1[0]*sy[0];
	      ey += d2[0]*sy[1];
	      ey += d3[0]*sy[2];

	      ey += d1[1]*sy[3];
	      ey += d2[1]*sy[4];
	      ey += d3[1]*sy[5];

	      ey += d1[2]*sy[6];
	      ey += d2[2]*sy[7];
	      ey += d3[2]*sy[8];

	      e = (float)sqrt(ex*ex+ey*ey);

	      if(e > thresh)
		{
		  *s = 0;
		}

	      d1++;
	      d2++;
	      d3++;
	      s++;
	    }
	}
    } /* if ay_prefs.sdmode != 2 (not color only) */

  if(ay_prefs.sdmode > 1)
    {
      /* process color-buffer */
      glReadPixels(0, 0, w, h, GL_RGBA, GL_UNSIGNED_BYTE, silimg);

      /* detect edges in color data using a simple sobel filter, separately
	 on each color channel */
      for(i = 1; i < h-1; i++)
	{
	  s = &(edges[i*w+1]);
	  c2 = &(silimg[i*4*w]);
	  c1 = c2-4*w;
	  c3 = c2+4*w;
	  for(j = 1; j < w-1; j++)
	    {
	      for(k = 0; k < 3; k++)
		{
		  if(*s)
		    {
		      ex = 0;
		      ex += c1[0]*sx[0];
		      ex += c2[0]*sx[1];
		      ex += c3[0]*sx[2];

		      ex += c1[4]*sx[3];
		      ex += c2[4]*sx[4];
		      ex += c3[4]*sx[5];

		      ex += c1[8]*sx[6];
		      ex += c2[8]*sx[7];
		      ex += c3[8]*sx[8];

		      ey = 0;
		      ey += c1[0]*sy[0];
		      ey += c2[0]*sy[1];
		      ey += c3[0]*sy[2];

		      ey += c1[4]*sy[3];
		      ey += c2[4]*sy[4];
		      ey += c3[4]*sy[5];

		      ey += c1[8]*sy[6];
		      ey += c2[8]*sy[7];
		      ey += c3[8]*sy[8];

		      e = (float)sqrt(ex*ex+ey*ey);

		      if(e > 300)
			{
			  *s = 0;
			}
		    }
		  c1++;
		  c2++;
		  c3++;
		}

	      c1++;
	      c2++;
	      c3++;
	      s++;
	    }
	}
    } /* if sdmode > 1 (color or both) */

  /*
    the sobel creates 2 pixel wide lines, so we need to process them by
    morphological thinning (unless the normal line width is already 2!)
  */
  if(selection)
    {
      if(ay_prefs.sellinewidth < 1.5)
	ay_shade_thinsil(w, h, edges);
    }
  else
    {
      if(ay_prefs.linewidth < 1.5)
	ay_shade_thinsil(w, h, edges);
    }

  /* create the silhouette texture */
  if(selection)
    {
      color[0] = (unsigned char)(ay_prefs.ser*255);
      color[1] = (unsigned char)(ay_prefs.seg*255);
      color[2] = (unsigned char)(ay_prefs.seb*255);
    }
  else
    {
      color[0] = (unsigned char)(ay_prefs.obr*255);
      color[1] = (unsigned char)(ay_prefs.obg*255);
      color[2] = (unsigned char)(ay_prefs.obb*255);
    }
  color[3] = 255;

  memset(silimg, 0, w*h*4*sizeof(unsigned char));

  for(i = 1; i < h-1; i++)
    {
      s = &(edges[i*w+1]);
      t = &(silimg[i*w*4+4]);
      for(j = 1; j < w-1; j++)
	{
	  if(!*s)
	    {
	      memcpy(t, color, 4*sizeof(unsigned char));
	    }
	  /*
	  else
	    {
	      memset(t, 0, 4*sizeof(unsigned char));
	    }
	  */
	  s++;
	  t += 4;
	}
    }

cleanup:

  if(ay_prefs.sdmode > 1)
    {
      ay_prefs.use_materialcolor = use_materialcolor;
      /* reset light configuration */
      ay_viewt_setupprojection(togl);
      glLightfv(GL_LIGHT0, GL_DIFFUSE, white);
      glDisable(GL_LIGHT1);
      glDisable(GL_LIGHT2);
      glDisable(GL_LIGHT3);
      glDisable(GL_LIGHT4);
      glDisable(GL_LIGHT5);
      glClearColor((GLfloat)ay_prefs.bgr, (GLfloat)ay_prefs.bgg,
		   (GLfloat)ay_prefs.bgb, (GLfloat)1.0);
    }

  glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

  if(depthimg)
    free(depthimg);
  if(edges)
    free(edges);

 return silimg;
} /* ay_shade_detectsil */


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
  if((o->scalx*o->scaly*o->scalz) < 0.0)
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
 ay_list_object *sel = ay_selection;
 ay_object *o = ay_root;
 ay_voidfp *arr = NULL;
 ay_drawcb *cb = NULL;
 ay_point *point = NULL;
 GLfloat color[4] = {0.0f,0.0f,0.0f,0.0f};
 double m[16];
 unsigned char *sil = NULL, *silsel = NULL;

  if(view->dirty)
    {
      ay_toglcb_reshape(togl);
      view->dirty = AY_FALSE;
    }

  if(view->drawmode == AY_DMWIREHIDDEN && !view->action_state &&
     ay_prefs.sdmode != 0)
    {
      if(!view->drawsel)
	{
	  sil = ay_shade_detectsil(togl, AY_FALSE);
	}
      if(sel)
	silsel = ay_shade_detectsil(togl, AY_TRUE);
    }

  glDisable(GL_LIGHTING);

  /* draw background image */
  if(view->drawbgimage)
    {
      ay_draw_bgimage(togl);
    }

  /* draw grid (but not in hidden wire mode) */
  if(view->drawgrid && view->drawmode < 3)
    {
      ay_draw_grid(togl);
    }

  /* when drawing mode is shade+draw, "set back" the shaded
     surfaces a bit so that we can draw on top of them later */
  if(view->drawmode > 1)
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

  if(view->drawmode < 3)
    {
      glEnable(GL_DITHER);
      glEnable(GL_LIGHTING);
      glLightModelf(GL_LIGHT_MODEL_TWO_SIDE, (GLfloat)1.0);
      glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, GL_TRUE);

      color[0] = (GLfloat)ay_prefs.shr;
      color[1] = (GLfloat)ay_prefs.shg;
      color[2] = (GLfloat)ay_prefs.shb;
      color[3] = (GLfloat)1.0;

      glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, color);
    }
  else
    {
      /* drawmode is AY_DMWIREHIDDEN, do not paint, but
	 update the z-buffer */
      glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
    }

  glMatrixMode(GL_MODELVIEW);

  if(view->drawlevel)
    {
      o = ay_currentlevel->object;
      glPushMatrix();
      if(ay_currentlevel->object != ay_root)
	{
	  ay_trafo_concatparent(ay_currentlevel->next);
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

  if(view->drawmode == AY_DMWIREHIDDEN)
    {
      glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
      if(sil)
	{
	  if(!view->antialiaslines && (ay_prefs.linewidth < 1.5))
	    if(!ay_shade_cleansil(togl, AY_FALSE, sil))
	      {
		free(sil);
		sil = NULL;
	      }
	}
      if(!view->drawsel && silsel)
	{
	  if(!view->antialiaslines && (ay_prefs.sellinewidth < 1.5))
	    if(!ay_shade_cleansil(togl, AY_TRUE, silsel))
	      {
		free(silsel);
		silsel = NULL;
	      }
	}
      if(sil)
	ay_draw_silhouettes(togl, sil);
      if(silsel)
	ay_draw_silhouettes(togl, silsel);
    } /* if drawmode is wirehidden */

  if(sel)
    {
      /* process the selected objects */
      glPushMatrix();
      if(!view->drawlevel)
	{
	  if(ay_currentlevel->object != ay_root)
	    {
	      ay_trafo_concatparent(ay_currentlevel->next);
	    }
	}

      /* all objects have been shaded already, if
         DrawSelectionOnly is _not_ enabled */
      if(view->drawsel)
	{
	  if(view->drawmode == AY_DMWIREHIDDEN)
	    {
	      glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
	    }

	  while(sel)
	    {
	      ay_shade_object(togl, sel->object, AY_FALSE);
	      sel = sel->next;
	    } /* while */

	  if(view->drawmode == AY_DMWIREHIDDEN)
	    {
	      glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	      if(silsel)
		{
		  if(!view->antialiaslines && (ay_prefs.sellinewidth < 1.5))
		    if(ay_shade_cleansil(togl, AY_TRUE, silsel))
		      ay_draw_silhouettes(togl, silsel);
		}
	    }
	} /* if draw selection only */

      /* draw handles of selected objects */
      if(view->drawhandles && (view->drawmode != AY_DMSHADE))
	{
	  /* let all handles appear "on top" of current drawing
	  * Do we really want this? In a shaded view, the user expects
	  * probably removal of hidden bits. On the other hand, he
	  * might not be able to reach all handles he wants to then,
	  * when modelling in a shaded view. So, for now, hide the
	  * handles in perspective views, as those are thought for
	  * review, not modelling anyway.
	  */
	  if(view->type != AY_VTPERSP && view->drawmode != AY_DMWIREHIDDEN)
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
  glDisable(GL_DITHER);

  /* draw marked point in space */
  if(view->drawmark)
    {
      ay_draw_mark(togl);
    } /* if */

  if(view->drawlevel)
    {
      glMatrixMode(GL_MODELVIEW);
      glPopMatrix();
    }

  /* shade and draw? */
  if(view->drawmode > 1)
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
  else
    {
      glLineWidth((GLfloat)1.0f);
    }

  if(sil)
    free(sil);
  if(silsel)
    free(silsel);

 return AY_OK;
} /* ay_shade_view */

