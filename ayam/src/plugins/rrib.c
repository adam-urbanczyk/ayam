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

/* rrib.c - RIB import plugin */

/*
cc -o rrib rrib.c -I/home/randi/sdk/affine0008/include/
-L/home/randi/sdk/affine0008/lib/ -lribrdr -lribhash -lribnop -lm -ldl -lz
*/

#include <ribrdr.h>
#include <ribnop.h>

/* force ayam.h to _not_ include ri.h from BMRT or ributil.h from Affine */
#ifdef AYUSEBMRTRIBOUT
#undef AYUSEBMRTRIBOUT
#endif

#ifdef AYUSEAFFINE
#undef AYUSEAFFINE
#endif

#include <ayam.h>

#define _RIPRIV_FUNC_TYPES

/* global variables for this module: */

char ay_rrib_version_ma[] = AYVERSIONSTR;
char ay_rrib_version_mi[] = AYVERSIONSTRMI;

/* current object */
ay_object ay_rrib_co;

/* current material object */
ay_mat_object ay_rrib_cm;

/* attribute state stack */
typedef struct ay_rrib_attrstate_s {
  struct ay_rrib_attrstate_s *next;
  ay_object *trimcurves;

} ay_rrib_attrstate;

ay_rrib_attrstate *ay_rrib_cattributes;

/* transformation stack */
typedef struct ay_rrib_trafostate_s {
  struct ay_rrib_trafostate_s *next;

  double movx, movy, movz;
  double scalx, scaly, scalz;
  double quat[4];

} ay_rrib_trafostate;

ay_rrib_trafostate *ay_rrib_ctrafos;


/* current frame */
int ay_rrib_cframe;

/* last read object */
ay_object *ay_rrib_lrobject;

/* light handle */
int ay_rrib_clighthandle;

/* import options */
int ay_rrib_readframe;
int ay_rrib_readoptions;
int ay_rrib_readcamera;
int ay_rrib_readlights;
int ay_rrib_readobjects;


/* how to sort through the data */
enum {
   PPWTBL_P,
   PPWTBL_PW,
   PPWTBL_LAST
};

/* 
 * The following table was created originally with the following command:
 *
 *            tokentbl -s ppw.asc Ppw
 */
char Ppw[] = {
    0,  1 , 'P',
    2,  2 ,'\0',      PPWTBL_P,
    0,  3 ,'w','\0',  PPWTBL_PW
};

/* prototypes of functions local to this module: */

RtVoid ay_rrib_RiSphere(RtFloat radius, RtFloat zmin, RtFloat zmax,
			RtFloat thetamax,
			RtInt n, RtToken tokens[], RtPointer parms[]);

void ay_rrib_pushattribs(void);
void ay_rrib_popattribs(void);
void ay_rrib_pushtrafos(void);
void ay_rrib_poptrafos(void);

void ay_rrib_initgeneral(void);
int ay_rrib_initgprims(void);
int ay_rrib_cleargprims(void);

void ay_rrib_linkobject(void *object, int type);

/* functions: */

RtVoid ay_rrib_RiSphere(RtFloat radius, RtFloat zmin, RtFloat zmax,
			RtFloat thetamax,
			RtInt n, RtToken tokens[], RtPointer parms[])
{
 ay_sphere_object s;

  s.closed = AY_FALSE;
  s.radius = (double)radius;
  s.zmin = (double)zmin;
  s.zmax = (double)zmax;
  s.thetamax = (double)thetamax;

  ay_rrib_linkobject((void *)(&s), AY_IDSPHERE);

 return;
} /* ay_rrib_RiSphere */


RtVoid ay_rrib_RiCylinder(RtFloat radius, RtFloat zmin, RtFloat zmax,
			  RtFloat thetamax,
			  RtInt n, RtToken tokens[], RtPointer parms[])
{ 
 ay_cylinder_object c;

  c.closed = AY_FALSE;
  c.radius = (double)radius;
  c.zmin = (double)zmin;
  c.zmax = (double)zmax;
  c.thetamax = (double)thetamax;

  ay_rrib_linkobject((void *)(&c), AY_IDCYLINDER);

 return;
} /* ay_rrib_RiCylinder */


RtVoid ay_rrib_RiDisk(RtFloat height, RtFloat radius, RtFloat thetamax,
		      RtInt n, RtToken tokens[], RtPointer parms[])
{ 
 ay_disk_object d;

  d.height = (double)height;
  d.radius = (double)radius;
  d.thetamax = (double)thetamax;

  ay_rrib_linkobject((void *)(&d), AY_IDDISK);

 return;
} /* ay_rrib_RiDisk */


RtVoid ay_rrib_RiCone(RtFloat height, RtFloat radius, RtFloat thetamax,
		      RtInt n, RtToken tokens[], RtPointer parms[])
{ 
 ay_cone_object c;

  c.closed = AY_FALSE;
  c.height = (double)height;
  c.radius = (double)radius;
  c.thetamax = (double)thetamax;

  ay_rrib_linkobject((void *)(&c), AY_IDCONE);

 return;
} /* ay_rrib_RiCone */



RtVoid ay_rrib_RiParaboloid(RtFloat rmax, 
			    RtFloat zmin, RtFloat zmax, RtFloat thetamax,
			    RtInt n, RtToken tokens[], RtPointer parms[])
{ 
 ay_paraboloid_object p;

  p.closed = AY_FALSE;
  p.rmax = (double)rmax;
  p.zmin = (double)zmin;
  p.zmax = (double)zmax;
  p.thetamax = (double)thetamax;

  ay_rrib_linkobject((void *)(&p), AY_IDPARABOBOLOID);

 return;
} /* ay_rrib_RiParaboloid */


RtVoid ay_rrib_RiHyperboloid(RtPoint point1, RtPoint point2, RtFloat thetamax,
			     RtInt n, RtToken tokens[], RtPointer parms[])
{ 
 ay_hyperboloid_object h;

  h.closed = AY_FALSE;
  h.p1[0] = (double)(point1[0]);
  h.p1[1] = (double)(point1[1]);
  h.p1[2] = (double)(point1[2]);

  h.p2[0] = (double)(point2[0]);
  h.p2[1] = (double)(point2[1]);
  h.p2[2] = (double)(point2[2]);

  h.thetamax = (double)thetamax;

  ay_rrib_linkobject((void *)(&h), AY_IDHYPERBOLOID);

 return;
} /* ay_rrib_RiHyperboloid */


RtVoid ay_rrib_RiTorus(RtFloat majorradius, RtFloat minorradius, 
		       RtFloat phimin, RtFloat phimax, RtFloat thetamax, 
		       RtInt n, RtToken tokens[], RtPointer parms[])
{ 
 ay_torus_object t;

  t.closed = AY_FALSE;
  t.majorrad = (double)majorradius;
  t.minorrad = (double)minorradius;
  t.phimin = (double)phimin;
  t.phimax = (double)phimax;
  t.thetamax = (double)thetamax;

  ay_rrib_linkobject((void *)(&t), AY_IDTORUS);

 return;
} /* ay_rrib_RiTorus */


RtVoid ay_rrib_RiNuPatch(RtInt nu, RtInt uorder, RtFloat uknot[], 
			 RtFloat umin, RtFloat umax, 
			 RtInt nv, RtInt vorder, RtFloat vknot[],
			 RtFloat vmin, RtFloat vmax,
			 RtInt n, RtToken tokens[], RtPointer parms[])
{
 ay_nurbpatch_object np;
 int i = 0, j = 0, stride = 4;
 double *p = NULL;
 RtPointer tokensfound[PPWTBL_LAST];
 RtFloat *pp = NULL, *pw = NULL;


  np.glu_sampling_tolerance = 0.0;
  np.glu_display_mode = 0;
  np.width = (int)nu;
  np.uorder = (int)uorder;
  np.height = (int)nv;
  np.vorder = (int)vorder;

  np.uknot_type = AY_KTCUSTOM;
  np.vknot_type = AY_KTCUSTOM;

  if(!(np.uknotv = calloc(nu+uorder, sizeof(double))))
    return;

  for(i = 0; i < nu+uorder; i++)
    {
      np.uknotv[i] = (double)(uknot[i]);
    }


  if(!(np.vknotv = calloc(nv+vorder, sizeof(double))))
    return;

  for(i = 0; i < nv+vorder; i++)
    {
      np.vknotv[i] = (double)(vknot[i]);
    }

  RibGetUserParameters(Ppw, PPWTBL_LAST, n, tokens, parms, tokensfound);
  if(tokensfound[PPWTBL_PW])
    {
      pw = (RtFloat*)tokensfound[PPWTBL_PW];
      stride = 4;
    }
  else
    {
      if(tokensfound[PPWTBL_P])
	{
	  pw = (RtFloat*)tokensfound[PPWTBL_P];
	  stride = 3;
	}
      else
	{
	  free(np.uknotv);
	  free(np.vknotv);
	  return;
	}
    }
  
  if(!(np.controlv = calloc(nu*nv*4, sizeof(double))))
    {
      free(np.uknotv);
      free(np.vknotv);
      return;
    }

  pp = pw;
  for(i = 0; i < nv; i++)
    {
      p = &(np.controlv[i*4]);
      for(j = 0; j < nu; j++)
	{
	  p[0] = (double)(pp[0]);
	  p[1] = (double)(pp[1]);
	  p[2] = (double)(pp[2]);

	  if(stride == 4)
	    {	  
	      p[3] = (double)(pp[3]);
	    }
	  else
	    {
	      p[3] = 1.0;
	    } /* if */

	  p += (nv*4);
	  pp += stride;
	} /* for */
    } /* for */

  ay_rrib_co.parent = AY_TRUE;
  ay_rrib_linkobject((void *)(&np), AY_IDNPATCH);
  ay_rrib_co.parent = AY_FALSE;

  free(np.uknotv);
  free(np.vknotv);
  free(np.controlv);

 return;
} /* ay_rrib_RiNuPatch */


RtVoid ay_rrib_RiTrimCurve(RtInt nloops, RtInt ncurves[], RtInt order[], 
			   RtFloat knot[], RtFloat min[], RtFloat max[], 
			   RtInt n[], RtFloat u[], RtFloat v[], RtFloat w[])
{
 int i = 0, j = 0, k = 0, l = 0;
 RtInt *orderptr = NULL, *nptr = NULL;
 RtFloat *knotptr = NULL, *uptr = NULL, *vptr = NULL, *wptr = NULL;
 ay_nurbcurve_object *nc = NULL;
 ay_object *o = NULL, *level = NULL, **ncinloop = NULL;
 int ay_status = AY_OK;


  nptr = n;
  orderptr = order;
  knotptr = knot;
  uptr = u;
  vptr = v;
  wptr = w;
  
  for(i = 0; i < nloops; i++)
   {
     if(ncurves[i] > 1)
       { /* read trim loop */
	 /* create level */
	 level = NULL;
	 if(!(level = calloc(1, sizeof(ay_object))))
	   return;
	 ay_object_defaults(level);
	 level->type = AY_IDLEVEL;
	 level->parent = AY_TRUE;
	 if(!(level->refine = calloc(1, sizeof(ay_level_object))))
	   return;
	 ((ay_level_object *)(level->refine))->type = AY_LTLEVEL;

	 ay_status = ay_object_crtendlevel(&(level->down));
	 ncinloop = &(level->down);


	 /* read loops curves */
	 for(j = 0; j < ncurves[i]; j++)
	   {
	     if(!(nc = calloc(1, sizeof(ay_nurbcurve_object))))
	       return;

	     nc->length = (int)*nptr;
	     nc->order = (int)*orderptr;

	     if(!(nc->knotv = calloc((*nptr + *orderptr), sizeof(double))))
	       return;

	     for(k = 0; k < (*nptr + *orderptr); k++)
	       {
		 nc->knotv[k] = (double)*knotptr;
		 knotptr++;
	       }
	     
	     if(!(nc->controlv = calloc((*nptr * 4), sizeof(double))))
	       return;

	     l = 0;
	     for(k = 0; k < *nptr; k++)
	       {
		 nc->controlv[l] = (double)*uptr;
		 nc->controlv[l+1] = (double)*vptr;
		 nc->controlv[l+3] = (double)*wptr;
		 l += 4;
		 uptr++;
		 vptr++;
		 wptr++;
	       } /* for */

	     /* link trimcurve */
	     o = NULL;
	     if(!(o = calloc(1, sizeof(ay_object))))
	       return;
	     ay_object_defaults(o);
	     o->type = AY_IDNCURVE;
	     o->refine = (void *)nc;

	     o->next = *ncinloop;
	     *ncinloop = o;
	     ncinloop = &(o->next);

	     orderptr++;
	     nptr++;
	   } /* for */

	 /* link level */
	 level->next = ay_rrib_cattributes->trimcurves;
	 ay_rrib_cattributes->trimcurves = level;


       }
     else
       { /* read single trimcurve */
	 if(!(nc = calloc(1, sizeof(ay_nurbcurve_object))))
	   return;

	 nc->length = (int)*nptr;
	 nc->order = (int)*orderptr;

	 if(!(nc->knotv = calloc((*nptr + *orderptr), sizeof(double))))
	   return;

	 for(k = 0; k < (*nptr + *orderptr); k++)
	   {
	     nc->knotv[k] = (double)*knotptr;
	     knotptr++;
	   }

	 if(!(nc->controlv = calloc((*nptr * 4), sizeof(double))))
	   return;

	 l = 0;
	 for(k = 0; k < *nptr; k++)
	   {
	     nc->controlv[l] = (double)*uptr;
	     nc->controlv[l+1] = (double)*vptr;
	     nc->controlv[l+3] = (double)*wptr;
	     l += 4;
	     uptr++;
	     vptr++;
	     wptr++;
	   } /* for */

	 /* link trimcurve */
	 o = NULL;
	 if(!(o = calloc(1, sizeof(ay_object))))
	   return;
	 ay_object_defaults(o);
	 o->type = AY_IDNCURVE;
	 o->refine = (void *)nc;
	 o->next = ay_rrib_cattributes->trimcurves;
	 ay_rrib_cattributes->trimcurves = o;

	 orderptr++;
	 nptr++;
       } /* if */
   } /* for */

 return;
} /* ay_rrib_RiTrimCurve */




RtLightHandle
ay_rrib_RiLightSource(RtToken name, 
		      RtInt n, RtToken tokens[], RtPointer parms[])
{
 ay_light_object l;
 ay_object *o = NULL;
 int ay_status = AY_OK;
 int i = 0;
 RtPoint *pnt = NULL;
 RtColor *col = NULL;

  l.type = AY_LITCUSTOM;
  l.lshader = NULL;
  l.intensity = 1.0;
  l.tfrom[0] = 0.0;
  l.tfrom[1] = 0.0;
  l.tfrom[2] = 0.0;

  l.tto[0] = 0.0;
  l.tto[1] = 0.0;
  l.tto[2] = 1.0;

  l.colr = 255;
  l.colg = 255;
  l.colb = 255;

  l.cone_angle = 30.0;
  l.cone_delta_angle = 5.0;
  l.beam_distribution = 2.0;

  printf("%s\n",name);


  if(!strcmp(name, "spotlight"))
    {
      l.type = AY_LITSPOT;
    }


  if(!strcmp(name, "distantlight"))
    {
      l.type = AY_LITDISTANT;
    }

  if(!strcmp(name, "pointlight"))
    {
      l.type = AY_LITPOINT;
    }


  for(i = 0; i < n; i++)
    {
      if(!strcmp(tokens[i],"intensity"))
	{
	  l.intensity = (double)(*((float*)(parms[i])));
	}
      if(!strcmp(tokens[i],"from"))
	{
	  pnt = (RtPoint*)(parms[i]);
	  l.tfrom[0] = (double)((*pnt)[0]);
	  l.tfrom[1] = (double)((*pnt)[1]);
	  l.tfrom[2] = (double)((*pnt)[2]);
	}
      if(!strcmp(tokens[i],"to"))
	{
	  pnt = (RtPoint*)(parms[i]);
	  l.tto[0] = (double)((*pnt)[0]);
	  l.tto[1] = (double)((*pnt)[1]);
	  l.tto[2] = (double)((*pnt)[2]);
	}
      if(!strcmp(tokens[i],"lightcolor"))
	{
	  col = (RtColor*)(parms[i]);
	  l.colr = (int)((*col)[0])*255.0;
	  l.colg = (int)((*col)[1])*255.0;
	  l.colb = (int)((*col)[2])*255.0;
	}

    } /* for */

  ay_rrib_co.refine = (void *)(&l);
  ay_rrib_co.type = AY_IDLIGHT;
  ay_status = ay_object_copy(&ay_rrib_co, &o);
  ay_status = ay_object_link(o);

 return((RtLightHandle)(ay_rrib_clighthandle++));
} /* ay_rrib_RiLightSource */



RtLightHandle
ay_rrib_RiAreaLightSource(RtToken name, 
			  RtInt n, RtToken tokens[], RtPointer parms[])
{
 

 return ((RtLightHandle)(ay_rrib_clighthandle++));
}


RtVoid ay_rrib_RiAtmosphereV( RtToken name, 
                      RtInt n, RtToken tokens[], RtPointer parms[] )
{
   (void)name; (void)n; (void)tokens; (void)parms;
}


RtVoid ay_rrib_RiAttributeV( RtToken name,
                     RtInt n, RtToken tokens[], RtPointer parms[] )
{
   (void)name; (void)n; (void)tokens; (void)parms;
}


RtVoid ay_rrib_RiAttributeBegin( void )
{

  ay_rrib_pushattribs();

}


RtVoid ay_rrib_RiAttributeEnd( void )
{

  ay_rrib_popattribs();

}


RtVoid ay_rrib_RiBasis( RtBasis ubasis, RtInt ustep, RtBasis vbasis, RtInt vstep )
{
   (void)ubasis; (void)ustep; (void)vbasis; (void)vstep; 
}


RtVoid ay_rrib_RiBound( RtBound bound )
{ 
   (void)bound;
}


RtVoid ay_rrib_RiClipping( RtFloat hither, RtFloat yon )
{ 
   (void)hither; (void)yon; 
}


RtVoid ay_rrib_RiColor( RtColor color )
{ 
   (void)color;
}


RtVoid ay_rrib_RiColorSamples( RtInt n, RtFloat nRGB[], RtFloat RGBn[] )
{ 
   (void)n; (void)nRGB; (void)RGBn;
}


RtVoid ay_rrib_RiConcatTransform( RtMatrix transform)
{ 
   (void)transform;
}



RtVoid ay_rrib_RiCoordinateSystem( RtToken space )
{ 
   (void)space; 
}


RtVoid ay_rrib_RiCoordSysTransform( RtToken space )
{ 
   (void)space; 
}


RtVoid ay_rrib_RiCropWindow( RtFloat xmin, RtFloat xmax, 
		    RtFloat ymin, RtFloat ymax )
{ 
   (void)xmin; (void)xmax; (void)ymin; (void)ymax; 
}


RtVoid ay_rrib_RiCurves( RtToken type, RtInt ncurves, RtInt nvertices[], 
            RtToken wrap, ... )
{
   (void)type; (void)ncurves; (void)nvertices; (void)wrap;
}


RtVoid ay_rrib_RiCurvesV( RtToken type, RtInt ncurves, RtInt nvertices[], 
             RtToken wrap, 
             RtInt n, RtToken tokens[], RtPointer parms[] )
{
   (void)type; (void)ncurves; (void)nvertices; (void)wrap;
   (void)n; (void)tokens; (void)parms; 
}



RtToken ay_rrib_RiDeclare( char *name, char *declaration )
{
   (void)declaration;

   return name;
}


RtVoid ay_rrib_RiDeformationV( RtToken name,
		      RtInt n, RtToken tokens[], RtPointer parms[] )
{ 
   (void)name; (void)n; (void)tokens; (void)parms;
}


RtVoid ay_rrib_RiDepthOfField( RtFloat fstop, RtFloat focallength, 
		      RtFloat focaldistance )
{ 
   (void)fstop; (void)focallength; (void)focaldistance;
}


RtVoid ay_rrib_RiDetail( RtBound bound )
{ 
   (void)bound;
}


RtVoid ay_rrib_RiDetailRange( RtFloat minvisible, RtFloat lowertransition, 
		     RtFloat uppertransition, RtFloat maxvisible )
{ 
   (void)minvisible; (void)lowertransition; 
   (void)uppertransition; (void)maxvisible;
}




RtVoid ay_rrib_RiDisplacementV( RtToken name,
		       RtInt n, RtToken tokens[], RtPointer parms[] )
{ 
   (void)name; (void)n; (void)tokens; (void)parms;
}


RtVoid ay_rrib_RiDisplayV( char *name, RtToken type, RtToken mode, 
		  RtInt n, RtToken tokens[], RtPointer parms[] )
{ 
   (void)name; (void)type; (void)mode; (void)n; (void)tokens; (void)parms;
}


RtVoid ay_rrib_RiErrorHandler( RtErrorHandler handler )
{ 
   (void)handler;
}


RtVoid ay_rrib_RiExposure( RtFloat gain, RtFloat gamma )
{
   (void)gain; (void)gamma;
}


RtVoid ay_rrib_RiExteriorV( RtToken name, 
		   RtInt n, RtToken tokens[], RtPointer parms[] )
{ 
   (void)name; (void)n; (void)tokens; (void)parms;
}


RtVoid ay_rrib_RiFormat( RtInt xres, RtInt yres, RtFloat aspect )
{ 
   (void)xres; (void)yres; (void)aspect;
}


RtVoid ay_rrib_RiFrameAspectRatio( RtFloat aspect )
{
   (void)aspect; 
}


RtVoid ay_rrib_RiFrameBegin( RtInt frame )
{ 
 int ay_status = AY_OK;

   if(ay_rrib_readframe != -1)
     {
       if(frame == ay_rrib_readframe)
	 {
	   ay_rrib_cframe = frame;
	   ay_status = ay_rrib_initgprims();
	 }
       else
	 {
	   ay_status = ay_rrib_cleargprims();
	 }
     }

 return;
}


RtVoid ay_rrib_RiFrameEnd( void )
{ 
 int ay_status = AY_OK;

  if(ay_rrib_readframe != -1)
    {
      if(ay_rrib_cframe == ay_rrib_readframe)
	{
	  ay_status = ay_rrib_cleargprims();
	}
    }

 return;
}


RtVoid ay_rrib_RiGeneralPolygonV( RtInt nloops, RtInt nvertices[],
                          RtInt n, RtToken tokens[], RtPointer parms[] )
{ 
   (void)nloops; (void)nvertices; (void)n; (void)tokens; (void)parms;
}


RtVoid ay_rrib_RiGeometricApproximation( RtToken type, RtFloat value )
{ 
   (void)type; (void)value;
}


RtVoid ay_rrib_RiGeometricRepresentation( RtToken type )
{ 
   (void)type;
}


RtVoid ay_rrib_RiGeometryV( RtToken type, 
		   RtInt n, RtToken tokens[], RtPointer parms[] )
{ 
   (void)type; (void)n; (void)tokens; (void)parms;
}


RtVoid ay_rrib_RiHiderV( RtToken type,
		RtInt n, RtToken tokens[], RtPointer parms[] )
{ 
   (void)type; (void)n; (void)tokens; (void)parms;
}



RtVoid ay_rrib_RiIdentity( void )
{

  ay_rrib_ctrafos->movx = 0.0;
  ay_rrib_ctrafos->movy = 0.0;
  ay_rrib_ctrafos->movz = 0.0;

  ay_rrib_ctrafos->scalx = 1.0;
  ay_rrib_ctrafos->scaly = 1.0;
  ay_rrib_ctrafos->scalz = 1.0;

  ay_rrib_ctrafos->quat[0] = 0.0;
  ay_rrib_ctrafos->quat[1] = 0.0;
  ay_rrib_ctrafos->quat[2] = 0.0;
  ay_rrib_ctrafos->quat[3] = 1.0;

 return;
}


RtVoid ay_rrib_RiIlluminate( RtLightHandle light, RtBoolean onoff )
{ 
   (void)light; (void)onoff;
}


RtVoid ay_rrib_RiImagerV( RtToken name,
                 RtInt n, RtToken tokens[], RtPointer parms[] )
{ 
   (void)name; (void)n; (void)tokens; (void)parms;
}


RtVoid ay_rrib_RiImplicitV( RtInt a, RtInt b[], RtInt c, RtFloat d[],
		       RtInt e, RtFloat f[], RtInt g, RtFloat h[],
		       RtInt n, RtToken tokens[], RtPointer parms[] )
{
   (void)a; (void)b; (void)c; (void)d; (void)e; (void)f; (void)g; (void)h;
   (void)n; (void)tokens; (void)parms;
}


RtVoid ay_rrib_RiInteriorV( RtToken name, 
		   RtInt n, RtToken tokens[], RtPointer parms[] )
{ 
   (void)name; (void)n; (void)tokens; (void)parms;
}

RtVoid ay_rrib_RiMakeBumpV( char *picturename, char *texturename, 
		   RtToken swrap, RtToken twrap,
		   RtFilterFunc filterfunc, RtFloat swidth, RtFloat twidth,
		   RtInt n, RtToken tokens[], RtPointer parms[] )
{ 
   (void)picturename; (void)texturename; 
   (void)swrap; (void)twrap; 
   (void)filterfunc; (void)swidth; (void)twidth; 
   (void)n; (void)tokens; (void)parms;
}


RtVoid ay_rrib_RiMakeCubeFaceEnvironmentV( char *px, char *nx, char *py, char *ny,
				  char *pz, char *nz, char *texturename, 
				  RtFloat fov,
				  RtFilterFunc filterfunc, 
				  RtFloat swidth, RtFloat twidth,
				  RtInt n, 
				  RtToken tokens[], RtPointer parms[] )
{ 
   (void)px; (void)nx; (void)py; (void)ny; 
   (void)pz; (void)nz; (void)texturename; 
   (void)fov; 
   (void)filterfunc; 
   (void)swidth; (void)twidth; 
   (void)n; (void)tokens; (void)parms;
}


RtVoid ay_rrib_RiMakeLatLongEnvironmentV( char *picturename, char *texturename, 
				 RtFilterFunc filterfunc,
				 RtFloat swidth, RtFloat twidth,
				 RtInt n, 
				 RtToken tokens[], RtPointer parms[] )
{ 
   (void)picturename; (void)texturename; 
   (void)filterfunc; 
   (void)swidth; (void)twidth; 
   (void)n; (void)tokens; (void)parms;
}


RtVoid ay_rrib_RiMakeShadowV( char *picturename, char *texturename,
		     RtInt n, RtToken tokens[], RtPointer parms[] )
{ 
   (void)picturename; (void)texturename; 
   (void)n; (void)tokens; (void)parms;
}


RtVoid ay_rrib_RiMakeTextureV( char *picturename, char *texturename, 
		      RtToken swrap, RtToken twrap,
		      RtFilterFunc filterfunc, 
		      RtFloat swidth, RtFloat twidth,
		      RtInt n, RtToken tokens[], RtPointer parms[] )
{ 
   (void)picturename; (void)texturename; 
   (void)swrap; (void)twrap; 
   (void)filterfunc; 
   (void)swidth; (void)twidth; 
   (void)n; (void)tokens; (void)parms;
}


RtVoid ay_rrib_RiMatte( RtBoolean onoff )
{ 
   (void)onoff;
}


RtVoid ay_rrib_RiMotionBeginV( RtInt n, RtFloat times[] )
{ 
   (void)n; (void)times;
}


RtVoid ay_rrib_RiMotionEnd( void )
{ 
}


RtVoid ay_rrib_RiNuCurvesV( RtInt ncurves, RtInt nvertices[], RtInt order[],
		       RtFloat knot[], RtFloat min[], RtFloat max[], 
		       RtInt n, RtToken tokens[], RtPointer parms[] )
{
   (void)ncurves; (void)nvertices; (void)order; (void)knot; 
   (void)min; (void)max; 
   (void)n; (void)tokens; (void)parms;
}



RtObjectHandle ay_rrib_RiObjectBegin( void )
{
   /* Fake it. */
  /*   return (RtObjectHandle)LastObjectHandle++;*/
}


RtVoid ay_rrib_RiObjectEnd( void )
{ 
}


RtVoid ay_rrib_RiObjectInstance( RtObjectHandle handle )
{ 
   (void)handle;
}


RtVoid ay_rrib_RiOpacity( RtColor color)
{ 
   (void)color;
}


RtVoid ay_rrib_RiOptionV( RtToken name, 
		 RtInt n, RtToken tokens[], RtPointer parms[] )
{ 
   (void)name; (void)n; (void)tokens; (void)parms;
}


RtVoid ay_rrib_RiOrientation( RtToken orientation )
{ 
   (void)orientation;
}

RtVoid ay_rrib_RiPatchV( RtToken type, 
		RtInt n, RtToken tokens[], RtPointer parms[] )
{ 
   (void)type; (void)n; (void)tokens; (void)parms;
}


RtVoid ay_rrib_RiPatchMeshV( RtToken type, RtInt nu, RtToken uwrap, 
		    RtInt nv, RtToken vwrap, 
		    RtInt n, RtToken tokens[], RtPointer parms[] )
{ 
   (void)type; (void)nu; (void)uwrap; (void)nv; (void)vwrap; 
   (void)n; (void)tokens; (void)parms;
}


RtVoid ay_rrib_RiPerspective( RtFloat fov )
{
   (void)fov;
}


RtVoid ay_rrib_RiPixelFilter( RtFilterFunc filterfunc, 
		     RtFloat xwidth, RtFloat ywidth )
{ 
   (void)filterfunc; (void)xwidth; (void)ywidth;
}


RtVoid ay_rrib_RiPixelSamples( RtFloat xsamples, RtFloat ysamples )
{ 
   (void)xsamples; (void)ysamples;
}


RtVoid ay_rrib_RiPixelVariance( RtFloat variation )
{ 
   (void)variation;
}


RtVoid ay_rrib_RiPointsV( RtInt npoints, 
		    RtInt n, RtToken tokens[], RtPointer parms[] )
{
   (void)npoints;
   (void)n; (void)tokens; (void)parms; 
}


RtVoid ay_rrib_RiPointsGeneralPolygonsV( RtInt npolys, RtInt nloops[], 
				RtInt nvertices[], RtInt vertices[], 
				RtInt n, RtToken tokens[], RtPointer parms[] )
{ 
   (void)npolys; (void)nloops; (void)nvertices; (void)vertices; 
   (void)n; (void)tokens; (void)parms;
}


RtVoid ay_rrib_RiPointsPolygonsV( RtInt npolys, RtInt nvertices[], RtInt vertices[],
			 RtInt n, RtToken tokens[], RtPointer parms[] )
{ 
   (void)npolys; (void)nvertices; (void)vertices; 
   (void)n; (void)tokens; (void)parms;
}


RtVoid ay_rrib_RiPolygonV( RtInt nvertices,
		  RtInt n, RtToken tokens[], RtPointer parms[] )
{ 
   (void)nvertices; (void)n; (void)tokens; (void)parms;
}


RtVoid ay_rrib_RiProjectionV( RtToken name, 
		     RtInt n, RtToken tokens[], RtPointer parms[] )
{ 
   (void)name; (void)n; (void)tokens; (void)parms;
}


RtVoid ay_rrib_RiQuantize( RtToken type, RtInt one, 
		  RtInt min, RtInt max, RtFloat ampl )
{ 
   (void)type; (void)one; (void)min; (void)max; (void)ampl;
}


RtVoid ay_rrib_RiReadArchiveV( RtToken name, 
                       RtVoid (*callback)( RtToken, char*, char* ),
                       RtInt n, RtToken tokens[], RtPointer parms[] )
{ 
   (void)name; (void)callback;
   (void)n; (void)tokens; (void)parms;
}


RtVoid ay_rrib_RiRelativeDetail( RtFloat relativedetail )
{ 
   (void)relativedetail;
}


RtVoid ay_rrib_RiResourceV( RtToken handle, RtToken type,
		RtInt n, RtToken tokens[], RtPointer parms[] )
{ 
   (void)handle; (void)type;
   (void)n; (void)tokens; (void)parms;
}


RtVoid ay_rrib_RiReverseOrientation( void )
{ 
}


RtVoid ay_rrib_RiRotate( RtFloat angle, RtFloat dx, RtFloat dy, RtFloat dz )
{ 
 double quat[4];
 double axis[3];

  axis[0] = (double)dx;
  axis[1] = (double)dy;
  axis[2] = (double)dz;
  ay_quat_axistoquat(axis, AY_D2R((double)angle), quat);
  ay_quat_add(ay_rrib_ctrafos->quat, quat, ay_rrib_ctrafos->quat);

 return;
}


RtVoid ay_rrib_RiScale( RtFloat dx, RtFloat dy, RtFloat dz )
{

  ay_rrib_ctrafos->scalx *= (double)dx;
  ay_rrib_ctrafos->scaly *= (double)dy;
  ay_rrib_ctrafos->scalz *= (double)dz;

 return;
}


RtVoid ay_rrib_RiScreenWindow( RtFloat left, RtFloat right, 
		      RtFloat bottom, RtFloat top )
{ 
   (void)left; (void)right; (void)bottom; (void)top;
}


RtVoid ay_rrib_RiShadingInterpolation( RtToken type )
{ 
   (void)type;
}


RtVoid ay_rrib_RiShadingRate( RtFloat size )
{ 
   (void)size;
}


RtVoid ay_rrib_RiShutter( RtFloat min, RtFloat max )
{ 
   (void)min; (void)max;
}


RtVoid ay_rrib_RiSides( RtInt sides )
{ 
   (void)sides;
}


RtVoid ay_rrib_RiSkew( RtFloat angle, RtFloat dx1, RtFloat dy1, RtFloat dz1,
	      RtFloat dx2, RtFloat dy2, RtFloat dz2 )
{ 
   (void)angle; 
   (void)dx1; (void)dy1; (void)dz1; (void)dx2; (void)dy2; (void)dz2;
}


RtVoid ay_rrib_RiSolidBegin( RtToken operation )
{ 
   (void)operation;
}


RtVoid ay_rrib_RiSolidEnd( void )
{ 
}


RtVoid ay_rrib_RiSphereV( RtFloat radius, RtFloat zmin, RtFloat zmax, 
		 RtFloat thetamax,
		 RtInt n, RtToken tokens[], RtPointer parms[] )
{ 
   (void)radius; (void)zmin; (void)zmax; (void)thetamax;
   (void)n; (void)tokens; (void)parms;
}


RtVoid ay_rrib_RiSurfaceV( RtToken name, 
                   RtInt n, RtToken tokens[], RtPointer parms[] )
{ 
   (void)name; (void)n; (void)tokens; (void)parms;
}


RtVoid ay_rrib_RiSubdivisionMeshV( RtToken scheme, RtInt nfaces, 
			      RtInt nvertices[], RtInt vertices[],
			      RtInt ntags, RtToken tags[],
			      RtInt nargs[], 
			      RtInt intargs[], RtFloat floatargs[],
			      RtInt n, RtToken tokens[], RtPointer parms[] )
{ 
   (void)scheme;  (void)nfaces;
   (void)nvertices; (void)vertices;
   (void)ntags; (void)tags;
   (void)nargs; (void)intargs; (void)floatargs;
   (void)n; (void)tokens; (void)parms;
}


RtVoid ay_rrib_RiTextureCoordinates( RtFloat s1, RtFloat t1, 
                             RtFloat s2, RtFloat t2,
                             RtFloat s3, RtFloat t3, 
                             RtFloat s4, RtFloat t4 )
{ 
   (void)s1; (void)t1; 
   (void)s2; (void)t2; 
   (void)s3; (void)t3; 
   (void)s4; (void)t4;
}



RtVoid ay_rrib_RiTransform( RtMatrix transform )
{ 
   (void)transform;
}


RtVoid ay_rrib_RiTransformBegin( void )
{

  ay_rrib_pushtrafos();

}


RtVoid ay_rrib_RiTransformEnd( void )
{

  ay_rrib_poptrafos();

}


RtVoid ay_rrib_RiTranslate( RtFloat dx, RtFloat dy, RtFloat dz )
{ 

  ay_rrib_ctrafos->movx += (double)dx;
  ay_rrib_ctrafos->movy += (double)dy;
  ay_rrib_ctrafos->movz += (double)dz;

 return;
}




RtVoid ay_rrib_RiWorldBegin( void )
{
}


RtVoid ay_rrib_RiWorldEnd( void )
{ 
}


RtVoid ay_rrib_RiBegin( RtToken name )
{ 
   (void)name;
   /*
   LastObjectHandle = 1;
   LastLightHandle = 1;
   */
}


RtVoid ay_rrib_RiEnd( void )
{ 
}


RtVoid ay_rrib_RiArchiveRecord( RtToken type, char *format, char *s )
{ 
   (void)type; (void)format; (void)s;
}


RtVoid ay_rrib_RiProcedural( RtPointer data, RtBound bound,
		       RtVoid (*subdivfunc)(RtPointer, RtFloat),
		       RtVoid (*freefunc)(RtPointer) )
{
   (void)data; (void)bound; (void)subdivfunc; (void)freefunc;

   return;
}


RtPoint* ay_rrib_RiTransformPoints( RtToken fromspace, RtToken tospace,
			      RtInt n, RtPoint points[] )
{
   (void)fromspace; (void)tospace; (void)n; (void)points;

   return NULL;
}


RtVoid ay_rrib_RiErrorIgnore( RtInt code, RtInt severity, char *msg )
{
   (void)code; (void)severity; (void)msg;

   return;
}


RtVoid ay_rrib_RiErrorPrint( RtInt code, RtInt severity, char *msg )
{
   (void)code; (void)severity; (void)msg;

   return;
}

RtVoid ay_rrib_RiErrorAbort( RtInt code, RtInt severity, char *msg )
{
   (void)code; (void)severity; (void)msg;

   return;
}


RtFloat ay_rrib_RiBoxFilter( RtFloat x, RtFloat y, RtFloat xwidth, RtFloat ywidth )
{
   (void)x; (void)y; (void)xwidth; (void)ywidth;

   return 0.0;
}


RtFloat ay_rrib_RiTriangleFilter( RtFloat x, RtFloat y, 
			 RtFloat xwidth, RtFloat ywidth )
{
   (void)x; (void)y; (void)xwidth; (void)ywidth;

   return 0.0;
}


RtFloat ay_rrib_RiCatmullRomFilter( RtFloat x, RtFloat y, 
			   RtFloat xwidth, RtFloat ywidth )
{
   (void)x; (void)y; (void)xwidth; (void)ywidth;

   return 0.0;
}


RtFloat ay_rrib_RiGaussianFilter( RtFloat x, RtFloat y, 
			 RtFloat xwidth, RtFloat ywidth )
{
   (void)x; (void)y; (void)xwidth; (void)ywidth;

   return 0.0;
}


RtFloat ay_rrib_RiSincFilter( RtFloat x, RtFloat y, RtFloat xwidth, RtFloat ywidth )
{
   (void)x; (void)y; (void)xwidth; (void)ywidth;

   return 0.0;
}


RtVoid ay_rrib_Ri_version( RtFloat version )
{
   (void)version;
}


RtVoid ay_rrib_RiProcDelayedReadArchive( RtPointer data, RtFloat detail )
{
   (void)data;
   (void)detail;
}


RtVoid ay_rrib_RiProcRunProgram( RtPointer data, RtFloat detail )
{
   (void)data;
   (void)detail;
}

RtVoid ay_rrib_RiProcDynamicLoad( RtPointer data, RtFloat detail )
{
   (void)data;
   (void)detail;
}


void
ay_rrib_initgeneral(void)
{
 int ay_status = AY_OK;

  gRibNopRITable[kRIB_TRANSFORMBEGIN] = (PRIB_RIPROC)ay_rrib_RiTransformBegin;
  gRibNopRITable[kRIB_TRANSFORMEND] = (PRIB_RIPROC)ay_rrib_RiTransformEnd;
  gRibNopRITable[kRIB_IDENTITY] = (PRIB_RIPROC)ay_rrib_RiIdentity;
  gRibNopRITable[kRIB_TRANSLATE] = (PRIB_RIPROC)ay_rrib_RiTranslate;
  gRibNopRITable[kRIB_ROTATE] = (PRIB_RIPROC)ay_rrib_RiRotate;
  gRibNopRITable[kRIB_SCALE] = (PRIB_RIPROC)ay_rrib_RiScale;


  gRibNopRITable[kRIB_ATTRIBUTEBEGIN] = (PRIB_RIPROC)ay_rrib_RiAttributeBegin;
  gRibNopRITable[kRIB_ATTRIBUTEEND] = (PRIB_RIPROC)ay_rrib_RiAttributeEnd;

 return;
}

int
ay_rrib_initgprims(void)
{
 int ay_status = AY_OK;

  gRibNopRITable[kRIB_SPHERE] = (PRIB_RIPROC)ay_rrib_RiSphere;
  gRibNopRITable[kRIB_CYLINDER] = (PRIB_RIPROC)ay_rrib_RiCylinder;
  gRibNopRITable[kRIB_DISK] = (PRIB_RIPROC)ay_rrib_RiDisk;
  gRibNopRITable[kRIB_CONE] = (PRIB_RIPROC)ay_rrib_RiCone;
  gRibNopRITable[kRIB_PARABOLOID] = (PRIB_RIPROC)ay_rrib_RiParaboloid;
  gRibNopRITable[kRIB_HYPERBOLOID] = (PRIB_RIPROC)ay_rrib_RiHyperboloid;
  gRibNopRITable[kRIB_TORUS] = (PRIB_RIPROC)ay_rrib_RiTorus;
  gRibNopRITable[kRIB_NUPATCH] = (PRIB_RIPROC)ay_rrib_RiNuPatch;
  gRibNopRITable[kRIB_TRIMCURVE] = (PRIB_RIPROC)ay_rrib_RiTrimCurve;

  gRibNopRITable[kRIB_LIGHTSOURCE] = (PRIB_RIPROC)ay_rrib_RiLightSource;


 return ay_status;
} /* ay_rrib_initgprims */

int
ay_rrib_cleargprims(void)
{
 int ay_status = AY_OK;

  gRibNopRITable[kRIB_SPHERE] = (PRIB_RIPROC)RiNopSphereV;
  gRibNopRITable[kRIB_CYLINDER] = (PRIB_RIPROC)RiNopCylinderV;
  gRibNopRITable[kRIB_DISK] = (PRIB_RIPROC)RiNopDiskV;
  gRibNopRITable[kRIB_CONE] = (PRIB_RIPROC)RiNopConeV;
  gRibNopRITable[kRIB_PARABOLOID] = (PRIB_RIPROC)RiNopParaboloidV;
  gRibNopRITable[kRIB_HYPERBOLOID] = (PRIB_RIPROC)RiNopHyperboloidV;
  gRibNopRITable[kRIB_TORUS] = (PRIB_RIPROC)RiNopTorusV;
  gRibNopRITable[kRIB_NUPATCH] = (PRIB_RIPROC)RiNopNuPatchV;
  gRibNopRITable[kRIB_TRIMCURVE] = (PRIB_RIPROC)RiNopTrimCurve;

 return ay_status;
} /* ay_rrib_cleargprims */


void
ay_rrib_pushattribs(void)
{
 ay_rrib_attrstate *newstate = NULL;
 char fname[] = "ay_rrib_pushattribs";
 int ay_status = AY_OK;

  if(!(newstate = calloc(1, sizeof(ay_rrib_attrstate))))
    {
      ay_error(AY_EOMEM, fname, NULL);
      return;
    }

  /* copy old into new state */
  if(ay_rrib_cattributes)
    {
      memcpy(newstate, ay_rrib_cattributes, sizeof(ay_rrib_attrstate));

      newstate->trimcurves = NULL;
#if 0
      if(ay_rrib_cattributes->trimcurves)
	{
	  /* XXXX Bug: this simple copy means, only the first trimcurve
	     survives a BeginAttribute! */
	  ay_status = ay_object_copy(ay_rrib_cattributes->trimcurves,
				     &(newstate->trimcurves));


	}
#endif
    }

  /* link new state to stack */
  newstate->next = ay_rrib_cattributes;
  ay_rrib_cattributes = newstate;

 return;
} /* ay_rrib_pushattribs */


void
ay_rrib_popattribs(void)
{
 ay_rrib_attrstate *nextstate = NULL;
 char fname[] = "ay_rrib_popattribs";

  if(!ay_rrib_cattributes)
    {
      ay_error(AY_ERROR, fname, "No states left!");
      return;
    }

  nextstate = ay_rrib_cattributes->next;

  /* free toplevel state */

  if(ay_rrib_cattributes->trimcurves)
    {
      ay_object_deletemulti(ay_rrib_cattributes->trimcurves);
      ay_rrib_cattributes->trimcurves = NULL;
    }

  free(ay_rrib_cattributes);

  ay_rrib_cattributes = nextstate;

 return;
} /* ay_rrib_popattribs */


void
ay_rrib_pushtrafos(void)
{
 ay_rrib_trafostate *newstate = NULL;
 char fname[] = "ay_rrib_pushtrafos";
 int ay_status = AY_OK;

  if(!(newstate = calloc(1, sizeof(ay_rrib_trafostate))))
    {
      ay_error(AY_EOMEM, fname, NULL);
      return;
    }

  /* copy old into new state or initialize first state */
  if(ay_rrib_ctrafos)
    {
      memcpy(newstate, ay_rrib_ctrafos, sizeof(ay_rrib_trafostate));
    }
  else
    {
      newstate->scalx = 1.0;
      newstate->scaly = 1.0;
      newstate->scalz = 1.0;
      newstate->quat[3] = 1.0; 
    }
  /* link new state to stack */
  newstate->next = ay_rrib_ctrafos;
  ay_rrib_ctrafos = newstate;

 return;
} /* ay_rrib_pushtrafos */


void
ay_rrib_poptrafos(void)
{
 ay_rrib_trafostate *nextstate = NULL;
 char fname[] = "ay_rrib_poptrafos";

  if(!ay_rrib_ctrafos)
    {
      ay_error(AY_ERROR, fname, "No states left!");
      return;
    }

  nextstate = ay_rrib_ctrafos->next;

  /* free toplevel state */

  free(ay_rrib_ctrafos);

  ay_rrib_ctrafos = nextstate;

 return;
} /* ay_rrib_poptrafos */


void
ay_rrib_linkobject(void *object, int type)
{
 ay_object *o = NULL, *t = NULL;
 double euler[3] = {0};
 int ay_status = AY_OK;
 char *fname = "ay_rrib_linkobject";

  ay_rrib_co.refine = object;
  ay_rrib_co.type = type;

  ay_rrib_co.movx = ay_rrib_ctrafos->movx;
  ay_rrib_co.movy = ay_rrib_ctrafos->movy;
  ay_rrib_co.movz = ay_rrib_ctrafos->movz;

  ay_rrib_co.scalx = ay_rrib_ctrafos->scalx;
  ay_rrib_co.scaly = ay_rrib_ctrafos->scaly;
  ay_rrib_co.scalz = ay_rrib_ctrafos->scalz;

  ay_rrib_co.quat[0] = ay_rrib_ctrafos->quat[0];
  ay_rrib_co.quat[1] = ay_rrib_ctrafos->quat[1];
  ay_rrib_co.quat[2] = ay_rrib_ctrafos->quat[2];
  ay_rrib_co.quat[3] = ay_rrib_ctrafos->quat[3];

  ay_quat_toeuler(ay_rrib_ctrafos->quat, euler);
  ay_rrib_co.rotx = AY_R2D(euler[2]);
  ay_rrib_co.roty = AY_R2D(euler[1]);
  ay_rrib_co.rotz = AY_R2D(euler[0]);

  if(type == AY_IDNPATCH)
    {
      if(ay_rrib_cattributes->trimcurves)
	{
	  ay_rrib_co.down = ay_rrib_cattributes->trimcurves;
	  t = ay_rrib_co.down;
	  while(t->next)
	    t = t->next;
	  ay_status = ay_object_crtendlevel(&(t->next));
	  ay_rrib_cattributes->trimcurves = NULL;
	}

    }


  if(ay_rrib_co.parent && (!ay_rrib_co.down))
    {
      ay_status = ay_object_crtendlevel(&(ay_rrib_co.down));
      if(ay_status)
	{
	  ay_error(AY_ERROR, fname,
          "Could not create terminating level object, scene is corrupt now!");
	}
    }

  ay_status = ay_object_copy(&ay_rrib_co, &o);
  ay_status = ay_object_link(o);

  ay_rrib_lrobject = o;

 return;
} /* ay_rrib_linkobject */


int
ay_rrib_readrib(char *filename, int frame)
{
 int ay_status = AY_OK;
 RIB_HANDLE rib;
       
  ay_object_defaults(&ay_rrib_co);

  ay_rrib_clighthandle = 1;

  ay_rrib_readframe = frame;

  if(frame == -1)
    {
      ay_status = ay_rrib_initgprims();
    }

  ay_rrib_initgeneral();

  ay_rrib_ctrafos = NULL;
  ay_rrib_pushtrafos();
  ay_rrib_cattributes = NULL;
  ay_rrib_pushattribs();

  /* Table included in libraries. */ 
  rib = RibOpen( filename, kRIB_LAST_RI, gRibNopRITable);
  RibRead(rib);
  RibClose(rib);

 return AY_OK;
} /* ay_rrib_readrib */


int
ay_rrib_readribtcmd(ClientData clientData, Tcl_Interp *interp,
		     int argc, char *argv[])
{
 int ay_status = AY_OK;
 char fname[] = "rrib";
 int frame = 0;

  if(argc < 2)
    {
      ay_error(AY_EARGS, fname, "filename \\[framenumber\\]!");
      return TCL_OK;
    }

  if(argc > 2)
    {
      sscanf(argv[2], "%d", &frame);
    }
  else
    {
      frame = -1;
    }

  ay_status = ay_rrib_readrib(argv[1], frame);
  if(ay_status)
    {
      ay_error(AY_ERROR, fname, NULL);
    }

 return TCL_OK;
} /* ay_rrib_readribtcmd */

int
Rrib_Init(Tcl_Interp *interp)
{
 char fname[] = "rrib_init";
 /* int ay_status = AY_OK;*/

  /* first, check versions */
  if(strcmp(ay_version_ma, ay_rrib_version_ma))
    {
      ay_error(AY_ERROR, fname,
	       "Plugin has been compiled for a different Ayam version!");
      ay_error(AY_ERROR, fname, "It is unsafe to continue! Bailing out...");
      return TCL_OK;
    }

  if(strcmp(ay_version_mi, ay_rrib_version_mi))
    {
      ay_error(AY_ERROR, fname,
	       "Plugin has been compiled for a different Ayam version!");
      ay_error(AY_ERROR, fname, "However, it is probably safe to continue...");
    }

  /* register some C-functions as Tcl-Commands */
  Tcl_CreateCommand (interp, "rrib",
		     ay_rrib_readribtcmd,
		     (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  /* source rrib.tcl, it contains Tcl-code for menu entries */
  if((Tcl_EvalFile(interp, "rrib.tcl")) != TCL_OK)
     {
       ay_error(AY_ERROR, fname,
		  "Error while sourcing \\\"rrib.tcl\\\"!");
       return TCL_OK;
     }


  ay_error(AY_EOUTPUT, fname,
	   "RIB import plugin successfully loaded.");


 return TCL_OK;
} /* Rrib_Init */
