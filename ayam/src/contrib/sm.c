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

/* sm.c - ShadowMap generation */

#if 0
typedef struct ay_prman_data_s {
  int flags;  /* Bit 0: shadowmaps on(1)/off  Bit 1: environmentmaps on(1)/off Bit 2: shaders off(1)/on*/
  int shadowmap;
  int environmentmap;
} ay_prman_data;

ay_prman_data prmandata;
#endif

#define SMEPSILON 1.0e-05


char *resolutions = NULL;

/* prototypes */
void ay_sm_dotrafos(ay_sm_trafostack *trafo, double *px, double *py,
		    double *pz);

void ay_sm_dotrafosinv(ay_sm_trafostack *trafo, double *px, double *py,
		       double *pz);

void ay_sm_wribsm(char *file, ay_sm_trafostack *trafo, ay_object *light);

void ay_sm_getresolution(int index, int *width, int *height,
			 int rwidth, int rheight);


void ay_sm_aimz(double *direction);

void ay_sm_placecamera(double *position, double *direction, double roll);


/* ay_sm_aimz:
 *
 */
void
ay_sm_aimz(double *direction)
{
 double xzlen = 0.0, yzlen = 0.0, xrot = 0.0,yrot = 0.0;
 double tmp;
 
  if((direction[0] == 0.0) && (direction[1] == 0.0) && (direction[2] == 0.0))
    return;

  xzlen = sqrt(direction[0]*direction[0] + direction[2]*direction[2]);
  if(xzlen == 0.0)
    {
      yrot = (direction[1] < 0.0) ? 180.0 : 0.0;
    }
  else
    {
      tmp = direction[2]/xzlen;

      if((tmp>1.0)||(tmp<-1.0))
	{
	  fprintf(stderr,"gotcha!\n");
	  fprintf(stderr,"%g %g\n",direction[0],direction[2]);
	}
      yrot = AY_R2D(acos((fabs(tmp)<=1.0?tmp:(tmp<-1.0?-1.0:1.0))));
    }
 
  yzlen = sqrt(direction[1]*direction[1] + xzlen*xzlen);
  if(yzlen != 0.0)
    {
      xrot = AY_R2D(acos(xzlen/yzlen));
    }
  else
    {
      xrot = 0.0;
    }

  if(direction[1] > 0.0)
    {
      if(fabs(xrot) > SMEPSILON)
	RiRotate((RtFloat)xrot, (RtFloat)1.0,(RtFloat)0.0,(RtFloat)0.0);
    }
  else
    {
      if(fabs(xrot) > SMEPSILON)
	RiRotate((RtFloat)-xrot, (RtFloat)1.0,(RtFloat)0.0,(RtFloat)0.0);
    }

  if(direction[0] < 0.0)
    {
      if(fabs(yrot) > SMEPSILON)
	RiRotate((RtFloat)yrot, (RtFloat)0.0,(RtFloat)1.0,(RtFloat)0.0);
    }
  else
    {
      if(fabs(yrot) > SMEPSILON)
	RiRotate((RtFloat)-yrot, (RtFloat)0.0,(RtFloat)1.0,(RtFloat)0.0);
    }

 return;
} /* ay_sm_aimz */


/* ay_sm_placecamera:
 *
 */
void
ay_sm_placecamera(double *position, double *direction, double roll)
{
  /* XXXX RiIdentity(); */
  if(fabs(roll) > SMEPSILON)
    RiRotate((RtFloat)-roll, (RtFloat)0.0, (RtFloat)0.0, (RtFloat)1.0);

  ay_sm_aimz(direction);

  if((fabs(position[0]) > SMEPSILON) || (fabs(position[1]) > SMEPSILON) ||
     (fabs(position[2]) > SMEPSILON))
    RiTranslate((RtFloat) ((fabs(position[0]) > SMEPSILON) ? -position[0] :
		 0.0),
		(RtFloat) ((fabs(position[1]) > SMEPSILON) ? -position[1] :
		 0.0),
		(RtFloat) ((fabs(position[2]) > SMEPSILON) ? -position[2] :
		0.0));

 return;
} /* ay_sm_placecamera */


/* ay_sm_dotrafos:
 *  where is a worldpoint after all translations?
 */
void
ay_sm_dotrafos(ay_sm_trafostack *trafo, double *px, double *py, double *pz)
{
 double v1[3] = {0};
 double mr[16];
 GLdouble m[16];

 v1[0] = *px;
 v1[1] = *py;
 v1[2] = *pz;

 while(trafo)
   {

     glMatrixMode (GL_MODELVIEW);
     glPushMatrix();
      glLoadIdentity();
      glTranslated(trafo->movx, trafo->movy, trafo->movz);

      ay_quat_torotmatrix(trafo->quat, mr);
      glMultMatrixd(mr);

      glScaled (trafo->scalx, trafo->scaly, trafo->scalz);
      glGetDoublev(GL_MODELVIEW_MATRIX, m);
     glPopMatrix();

     ay_trafo_apply3(v1, m);

     trafo = trafo->next;
   }

 *px = v1[0];
 *py = v1[1];
 *pz = v1[2];

 return;
} /* ay_sm_dotrafos */


/* ay_sm_dotrafosinv:
 *  where is a worldpoint after all translations inverse?
 */
void
ay_sm_dotrafosinv(ay_sm_trafostack *trafo, double *px, double *py, double *pz)
{
 double v1[3] = {0};
 double euler[3];
 GLdouble m[16];

  v1[0] = *px;
  v1[1] = *py;
  v1[2] = *pz;

  if(trafo->next)
   ay_sm_dotrafosinv(trafo->next, px, py, pz);

  glMatrixMode (GL_MODELVIEW);
  glPushMatrix();
   glLoadIdentity();
   glScaled (1.0/trafo->scalx, 1.0/trafo->scaly, 1.0/trafo->scalz);

   ay_quat_toeuler(trafo->quat, euler);
   glRotated((GLdouble)AY_R2D(euler[0]), (GLdouble)0.0, (GLdouble)0.0,
	     (GLdouble)1.0);
   glRotated((GLdouble)AY_R2D(euler[1]), (GLdouble)0.0, (GLdouble)1.0,
	     (GLdouble)0.0);
   glRotated((GLdouble)AY_R2D(euler[2]), (GLdouble)1.0, (GLdouble)0.0,
	     (GLdouble)0.0);
   
   glTranslated(-trafo->movx, -trafo->movy, -trafo->movz);
   glGetDoublev(GL_MODELVIEW_MATRIX, m);
  glPopMatrix();

  ay_trafo_apply3(v1, m);

  *px = v1[0];
  *py = v1[1];
  *pz = v1[2];

 return;
} /* ay_sm_dotrafosinv */


/* ay_sm_wribsm:
 *  render a z-buffer file used to create shadows spot and
 *  distant lights
 */
void
ay_sm_wribsm(char *file, ay_sm_trafostack *trafo, ay_object *light)
{
  /* ay_object *o = ay_root->next;*/
 ay_light_object *l = NULL;
 double euler[3], d[3];

 /*  RiFrameBegin(1);*/
  RiIdentity();
  RiScale((RtFloat)-1.0, (RtFloat)1.0, (RtFloat)1.0);
  /*RiRotate(RtFloat)180, RtFloat)0, RtFloat)1, RtFloat)0);*/

  /* move light according to its from-/to-settings */
  l = (ay_light_object *)light->refine;
  d[0] = l->tto[0]-l->tfrom[0];
  d[1] = l->tto[1]-l->tfrom[1];
  d[2] = l->tto[2]-l->tfrom[2];
  ay_sm_placecamera(l->tfrom, d, 0.0);

  /* move light to origin via trafo structure */
  while (trafo) {
     if ((trafo->scalx != 1.0) || (trafo->scaly != 1.0) ||
	 (trafo->scalz != 1.0))
       RiScale((RtFloat)(1.0/trafo->scalx), (RtFloat)(1.0/trafo->scaly),
	 (RtFloat)(1.0/trafo->scalz));

     if(trafo->quat[0] != 0.0 || trafo->quat[1] != 0.0 ||
	trafo->quat[2] != 0.0 || trafo->quat[3] != 1.0)
       {
	 ay_quat_toeuler(trafo->quat, euler);
	 if(fabs(euler[2]) > 1.0e-05)
	   RiRotate((RtFloat)AY_R2D(euler[2]),
		    (RtFloat)1.0, (RtFloat)0.0, (RtFloat)0.0);
	 if(fabs(euler[1]) > 1.0e-05)
	   RiRotate((RtFloat)AY_R2D(euler[1]),
		    (RtFloat)0.0, (RtFloat)1.0, (RtFloat)0.0);
	 if(fabs(euler[0]) > 1.0e-05)
	   RiRotate((RtFloat)AY_R2D(euler[0]),
		    (RtFloat)0.0, (RtFloat)0.0, (RtFloat)1.0);
       }

     if ((trafo->movx != 0) || (trafo->movy != 0) || (trafo->movz != 0))
       RiTranslate((RtFloat)(-trafo->movx), (RtFloat)(-trafo->movy),
	 (RtFloat)(-trafo->movz));
     trafo = trafo->next;
  }

  RiWorldBegin();
  RiIdentity();
  /* place the objects relative to the centered light */
  RiReadArchive(file, (RtVoid*)RI_NULL, RI_NULL);
  /* was:
  while (o) {
      ay_wrib_object(file, o);
      o = o->next;
  }
  */
  RiWorldEnd();
  /*  RiFrameEnd();*/

 return;
} /* ay_sm_wribsm */


/* ay_sm_getresolution:
 *  return the selected shadowmap resolution, or if selected auto
 *  compute good resolution
 */
void
ay_sm_getresolution(int index, int *width, int *height,
		    int rwidth, int rheight)
{
 int p = 0, m, w, h;

  if (index > 1) {
      /* predefined resolution; look up resolution table */
      *width = 256; *height = 256;  /* default values */
      if (resolutions) {
	  while ((resolutions[p] != 0) && (index != 1)) {
	      while ((resolutions[p] != 0) && (resolutions[p] != ' '))
		  p++;
	      index--;
	      if (resolutions[p] != 0)
		  p++;
	  }
	  if (index == 1) {
	      /* read width and height from Tcl-string */
	      if (resolutions[p] != 0)
		  p--;
	      p--;
	      w = 0; h = 0;
	      m = 1;
	      while ((p >= 0) && (resolutions[p] >= '0') &&
		     (resolutions[p] <= '9')) {
		  h+= m*(resolutions[p]-'0');
		  m*= 10;
		  p--;
	      }

	      while ((p >= 0) && ((resolutions[p] < '0') ||
				  (resolutions[p] > '9')))
		  p--;

	      m = 1;
	      while ((p >= 0) && (resolutions[p] >= '0') &&
		     (resolutions[p] <= '9')) {
		  w+= m*(resolutions[p]-'0');
		  m*= 10;
		  p--;
	      }
	      if ((w > 1) && (h > 1) && (w < 65536) && (h < 65536)) {
		  /* calculate next matching power of two for w and h */
		  *width = 1; *height = 1;
		  while (*width < w)
		      *width = *width<<1;
		  while (*height < h)
		      *height = *height<<1;
	      }
	  }
      }
  }
  else {
      /* automatic resolution; for now the next higher power of two than
	 the rendering resolution */
      *width = 1; *height = 1;
      while (*width < rwidth)
          *width = *width<<1;
      while (*height < rheight)
          *height = *height<<1;
      if (*width > *height)
          *height = *width;
      else
          *width = *height;
  }

 return;
} /* ay_sm_getresolution */


/* ay_sm_wriballsm:
 *  search whole tree for lightsources and create their
 *  shadowmaps if selected
 */
void
ay_sm_wriballsm(char *file, char *objfile, ay_object *o,
		ay_sm_trafostack *trafo,
		int rwidth, int rheight)
{
 ay_light_object *light = NULL;
 ay_sm_trafostack *newtrafo;
 RtFloat fov;
 /*double bb[24]; int i;*/
 ay_object *d;
 char *zname = NULL, *shdname = NULL;
 /*double xmin = DBL_MAX, ymin = DBL_MAX, xmax = -DBL_MAX, ymax = -DBL_MAX;*/
 double px, py, pz;
 int width, height, filelen = 0;
 static int countsm = 0;

  if(!o)
    return;

  filelen = strlen(file);

  if(!(zname = calloc(filelen+64, sizeof(char))))
    return;
  if(!(shdname = calloc(filelen+64, sizeof(char))))
    return;

  if(!trafo)
    countsm = 0;

  newtrafo = calloc(1, sizeof(ay_sm_trafostack));
  newtrafo->next = trafo;
  trafo = newtrafo;
  while (o) {
      trafo->movx = o->movx;
      trafo->movy = o->movy;
      trafo->movz = o->movz;
      trafo->rotx = o->rotx;
      trafo->roty = o->roty;
      trafo->rotz = o->rotz;
      memcpy(trafo->quat, o->quat, 4*sizeof(double));
      trafo->scalx = o->scalx;
      trafo->scaly = o->scaly;
      trafo->scalz = o->scalz;
      
      if (o->down)
	  ay_sm_wriballsm(file, objfile, o->down, trafo, rwidth, rheight);

      if (o->type == AY_IDLIGHT) {
	  light = (ay_light_object *)o->refine;
	  if ((light->on) && (light->type != AY_LITCUSTOM) &&
	      (light->use_sm))
	    {

	      countsm++;
 
	      if(light->sm_resolution == 0)
		{
		  width = 256;
		  height = 256;
		}
	      else
		{
		  width = light->sm_resolution;
		  height = light->sm_resolution;
		}

	      switch (light->type) {
	      case AY_LITPOINT:
		  /* render six shadowmaps: z+, x+, z-, x-, y+, y- */
		  fov = (RtFloat)95.0;  /* 95 degrees suggested in Pixar's
					   application notes */
		  px = 0.0; py = 0.0; pz = 0.0;
		  ay_sm_dotrafos(trafo, &px, &py, &pz);
		  /*
		  ay_sm_getresolution(light->sm_resolution,
				      &width, &height, rwidth, rheight);
		  */
		  sprintf(zname, "%s.point%d_z+.z", file, countsm);
		  sprintf(shdname, "%s.point%d_z+.shd", file, countsm);
		  RiDisplay(zname, "zfile", "z", RI_NULL);
		  RiFormat(width, height, (RtFloat)-1.0);
		  RiProjection("perspective", "fov", &fov, RI_NULL);
		  /* looking along positive z axis */
		  /* transform lightsource to origin */
		  RiFrameBegin((RtFloat)1.0);
		  RiIdentity();
		  RiScale((RtFloat)-1.0, (RtFloat)1.0, (RtFloat)1.0);
		  RiTranslate((RtFloat)-px, (RtFloat)-py, (RtFloat)-pz);
		  RiWorldBegin();
		  RiIdentity();
		  /* place the objects relative to the centered light */
		  d = ay_root->next; 
		  while (d) {
		      ay_wrib_object(file, d);
		      d = d->next;
		  }
		  RiWorldEnd();
		  RiFrameEnd();
                  RiMakeShadow(zname, shdname, RI_NULL);

		  sprintf(zname, "%s.point%d_x+.z", file, countsm);
		  sprintf(shdname, "%s.point%d_x+.shd", file, countsm);
		  RiDisplay(zname, "zfile", "z", RI_NULL);
		  RiFormat(width, height, (RtFloat)-1.0);
		  RiProjection("perspective", "fov", &fov, RI_NULL);
		  /* transform lightsource to origin */
		  RiFrameBegin((RtFloat)1.0);
		  RiIdentity();
		  RiScale((RtFloat)-1.0, (RtFloat)1.0, (RtFloat)1.0);
		  RiRotate((RtFloat)-90, (RtFloat)0, (RtFloat)1, (RtFloat)0);
		  RiTranslate((RtFloat)-px, (RtFloat)-py, (RtFloat)-pz);
		  RiWorldBegin();
		  RiIdentity();
		  /* place the objects relative to the centered light */
		  RiReadArchive(objfile, (RtVoid*)RI_NULL, RI_NULL);
		  /* was:
		  d = ay_root->next; 
		  while (d) {
		      ay_wrib_object(file, d);
		      d = d->next;
		  }
		  */
		  RiWorldEnd();
		  RiFrameEnd();
                  RiMakeShadow(zname, shdname, RI_NULL);

		  sprintf(zname, "%s.point%d_z-.z", file, countsm);
		  sprintf(shdname, "%s.point%d_z-.shd", file, countsm);
		  RiDisplay(zname, "zfile", "z", RI_NULL);
		  RiFormat(width, height, (RtFloat)-1.0);
		  RiProjection("perspective", "fov", &fov, RI_NULL);
		  /* transform lightsource to origin */
		  RiFrameBegin((RtFloat)1.0);
		  RiIdentity();
		  RiScale((RtFloat)-1.0, (RtFloat)1.0, (RtFloat)1.0);
		  RiRotate((RtFloat)180, (RtFloat)0, (RtFloat)1, (RtFloat)0);
		  RiTranslate((RtFloat)-px, (RtFloat)-py, (RtFloat)-pz);
		  RiWorldBegin();
		  RiIdentity();
		  /* place the objects relative to the centered light */
		  RiReadArchive(objfile, (RtVoid*)RI_NULL, RI_NULL);
		  /* was:
		  d = ay_root->next; 
		  while (d) {
		      ay_wrib_object(file, d);
		      d = d->next;
		  }
		  */
		  RiWorldEnd();
		  RiFrameEnd();
                  RiMakeShadow(zname, shdname, RI_NULL);

		  sprintf(zname, "%s.point%d_x-.z", file, countsm);
		  sprintf(shdname, "%s.point%d_x-.shd", file, countsm);
		  RiDisplay(zname, "zfile", "z", RI_NULL);
		  RiFormat(width, height, (RtFloat)-1.0);
		  RiProjection("perspective", "fov", &fov, RI_NULL);
		  /* transform lightsource to origin */
		  RiFrameBegin((RtFloat)1);
		  RiIdentity();
		  RiScale((RtFloat)-1.0, (RtFloat)1.0, (RtFloat)1.0);
		  RiRotate((RtFloat)90, (RtFloat)0, (RtFloat)1, (RtFloat)0);
		  RiTranslate((RtFloat)-px, (RtFloat)-py, (RtFloat)-pz);
		  RiWorldBegin();
		  RiIdentity();
		  /* place the objects relative to the centered light */
		  RiReadArchive(objfile, (RtVoid*)RI_NULL, RI_NULL);
		  /* was:
		  d = ay_root->next; 
		  while (d) {
		      ay_wrib_object(file, d);
		      d = d->next;
		  }
		  */
		  RiWorldEnd();
		  RiFrameEnd();
                  RiMakeShadow(zname, shdname, RI_NULL);

		  sprintf(zname, "%s.point%d_y+.z", file, countsm);
		  sprintf(shdname, "%s.point%d_y+.shd", file, countsm);
		  RiDisplay(zname, "zfile", "z", RI_NULL);
		  RiFormat(width, height, (RtFloat)-1.0);
		  RiProjection("perspective", "fov", &fov, RI_NULL);
		  /* transform lightsource to origin */
		  RiFrameBegin(1);
		  RiIdentity();
		  RiScale((RtFloat)-1.0, (RtFloat)1.0, (RtFloat)1.0);
		  RiRotate((RtFloat)90, (RtFloat)1, (RtFloat)0, (RtFloat)0);
		  RiTranslate((RtFloat)-px, (RtFloat)-py, (RtFloat)-pz);
		  RiWorldBegin();
		  RiIdentity();
		  /* place the objects relative to the centered light */
		  RiReadArchive(objfile, (RtVoid*)RI_NULL, RI_NULL);
		  /* was:
		  d = ay_root->next; 
		  while (d) {
		      ay_wrib_object(file, d);
		      d = d->next;
		  }
		  */
		  RiWorldEnd();
		  RiFrameEnd();
                  RiMakeShadow(zname, shdname, RI_NULL);

		  sprintf(zname, "%s.point%d_y-.z", file, countsm);
		  sprintf(shdname, "%s.point%d_y-.shd", file, countsm);
		  RiDisplay(zname, "zfile", "z", RI_NULL);
		  RiFormat(width, height, (RtFloat)-1.0);
		  RiProjection("perspective", "fov", &fov, RI_NULL);
		  /* transform lightsource to origin */
		  RiFrameBegin(1);
		  RiIdentity();
		  RiScale((RtFloat)-1.0, (RtFloat)1.0, (RtFloat)1.0);
		  RiRotate((RtFloat)-90.0, (RtFloat)1, (RtFloat)0, (RtFloat)0);
		  RiTranslate((RtFloat)-px, (RtFloat)-py, (RtFloat)-pz);
		  RiWorldBegin();
		  RiIdentity();
		  /* place the objects relative to the centered light */
		  RiReadArchive(objfile, (RtVoid*)RI_NULL, RI_NULL);
		  /* was:
		  d = ay_root->next; 
		  while (d) {
		      ay_wrib_object(file, d);
		      d = d->next;
		  }
		  */
		  RiWorldEnd();
		  RiFrameEnd();
                  RiMakeShadow(zname, shdname, RI_NULL);
		  break;

	      case AY_LITSPOT:
		  /* render shadowmap for spotlight */
		  sprintf(zname, "%s.spot%d.z", file, countsm);
		  sprintf(shdname, "%s.spot%d.shd", file, countsm);

		  RiDisplay(zname, "zfile", "z", RI_NULL);
		  /* Camera! */
		  /*
		  ay_sm_getresolution(light->sm_resolution,
				      &width, &height, rwidth, rheight);
		  */
		  RiFormat(width, height, (RtFloat)-1.0);
		  fov = light->cone_angle*360/AY_PI;
		  RiProjection("perspective", "fov", &fov, RI_NULL);
		  RiFrameAspectRatio((RtFloat)1.0);
		  RiScreenWindow((RtFloat)-1, (RtFloat)1,
				 (RtFloat)-1, (RtFloat)1);
		  /* transform lightsource to origin */
		  /* was: ay_sm_wribsm(file, trafo, o);*/
		  ay_sm_wribsm(objfile, trafo, o);
                  RiMakeShadow(zname, shdname, RI_NULL);
		  break;

	      case AY_LITDISTANT:
		  /* render shadowmap for distant light */
		  /* first get bounding box of scene */
#if 0
		  d = ay_root->next;
		  while (d) {
		      if ((d->type != AY_IDLIGHT) && (d->type != AY_IDROOT)
			  /*&& (d->type < AY_OTATTRIB)*/) {
			  ay_bbc_get(d, bb);
			  for (i = 0; i < 8; i++) {
			      ay_sm_dotrafosinv(trafo, &bb[i*3],
						&bb[i*3+1], &bb[i*3+2]);
			      if (bb[i*3] < xmin)
			          xmin = bb[i*3];
			      if (bb[i*3+1] < ymin)
			          ymin = bb[i*3+1];

			      if (bb[i*3] > xmax)
			          xmax = bb[i*3];
			      if (bb[i*3+1] > ymax)
			          ymax = bb[i*3+1];
			  }
		      }
		      d = d->next;
		  }
		  if ((xmin < xmax) && (ymin < ymax)) {
#endif
		      sprintf(zname, "%s.dist%d.z", file, countsm);
		      sprintf(shdname, "%s.dist%d.shd", file, countsm);
		      RiDisplay(zname, "zfile", "z", RI_NULL);
		      /* Camera! */
		      /*
		      ay_sm_getresolution(light->sm_resolution,
					  &width, &height, rwidth, rheight);
		      */
		      RiFormat(width, height, (RtFloat)-1.0);
		      RiProjection("orthographic", RI_NULL);
		      RiFrameAspectRatio((RtFloat)1.0);
		      /*RiScreenWindow(xmin, xmax, ymin, ymax);*/
		      /*RiScreenWindow(-1,1,-1,1);*/
		      /* transform lightsource to origin */
		      /* was: ay_sm_wribsm(file, trafo, o);*/
		      ay_sm_wribsm(objfile, trafo, o);
		      RiMakeShadow(zname, shdname, RI_NULL);
		      /*}*/
		  break;
	      }
	  }
      }
      o = o->next;
  }
  newtrafo = trafo;
  trafo = trafo->next;
  free(newtrafo);

 return;
} /* ay_sm_wriballsm */
