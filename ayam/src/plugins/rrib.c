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


/* functions: */

RtVoid ay_rrib_RiSphere(RtFloat radius, RtFloat zmin, RtFloat zmax,
			RtFloat thetamax,
			RtInt n, RtToken tokens[], RtPointer parms[])
{
 ay_sphere_object s;
 ay_object *o = NULL;
 int ay_status = AY_OK;

  s.closed = AY_FALSE;
  s.radius = (double)radius;
  s.zmin = (double)zmin;
  s.zmax = (double)zmax;
  s.thetamax = (double)thetamax;

  ay_rrib_co.refine = (void *)(&s);
  ay_rrib_co.type = AY_IDSPHERE;
  ay_status = ay_object_copy(&ay_rrib_co, &o);
  ay_status = ay_object_link(o);

 return;
} /* ay_rrib_RiSphere */


RtVoid ay_rrib_RiCylinder(RtFloat radius, RtFloat zmin, RtFloat zmax,
			  RtFloat thetamax,
			  RtInt n, RtToken tokens[], RtPointer parms[])
{ 
 ay_cylinder_object c;
 ay_object *o = NULL;
 int ay_status = AY_OK;

  c.closed = AY_FALSE;
  c.radius = (double)radius;
  c.zmin = (double)zmin;
  c.zmax = (double)zmax;
  c.thetamax = (double)thetamax;

  ay_rrib_co.refine = (void *)(&c);
  ay_rrib_co.type = AY_IDCYLINDER;
  ay_status = ay_object_copy(&ay_rrib_co, &o);
  ay_status = ay_object_link(o);

 return;
} /* ay_rrib_RiCylinder */


RtVoid ay_rrib_RiDisk(RtFloat height, RtFloat radius, RtFloat thetamax,
		      RtInt n, RtToken tokens[], RtPointer parms[])
{ 
 ay_disk_object d;
 ay_object *o = NULL;
 int ay_status = AY_OK;

  d.height = (double)height;
  d.radius = (double)radius;
  d.thetamax = (double)thetamax;

  ay_rrib_co.refine = (void *)(&d);
  ay_rrib_co.type = AY_IDDISK;
  ay_status = ay_object_copy(&ay_rrib_co, &o);
  ay_status = ay_object_link(o);

 return;
} /* ay_rrib_RiDisk */


RtVoid ay_rrib_RiCone(RtFloat height, RtFloat radius, RtFloat thetamax,
		      RtInt n, RtToken tokens[], RtPointer parms[])
{ 
 ay_cone_object c;
 ay_object *o = NULL;
 int ay_status = AY_OK;

  c.closed = AY_FALSE;
  c.height = (double)height;
  c.radius = (double)radius;
  c.thetamax = (double)thetamax;

  ay_rrib_co.refine = (void *)(&c);
  ay_rrib_co.type = AY_IDCONE;
  ay_status = ay_object_copy(&ay_rrib_co, &o);
  ay_status = ay_object_link(o);

 return;
} /* ay_rrib_RiCone */



RtVoid ay_rrib_RiParaboloid(RtFloat rmax, 
			    RtFloat zmin, RtFloat zmax, RtFloat thetamax,
			    RtInt n, RtToken tokens[], RtPointer parms[])
{ 
 ay_paraboloid_object p;
 ay_object *o = NULL;
 int ay_status = AY_OK;

  p.closed = AY_FALSE;
  p.rmax = (double)rmax;
  p.zmin = (double)zmin;
  p.zmax = (double)zmax;
  p.thetamax = (double)thetamax;

  ay_rrib_co.refine = (void *)(&p);
  ay_rrib_co.type = AY_IDPARABOBOLOID;
  ay_status = ay_object_copy(&ay_rrib_co, &o);
  ay_status = ay_object_link(o);

 return;
} /* ay_rrib_RiParaboloid */


RtVoid ay_rrib_RiHyperboloid(RtPoint point1, RtPoint point2, RtFloat thetamax,
			     RtInt n, RtToken tokens[], RtPointer parms[])
{ 
 ay_hyperboloid_object h;
 ay_object *o = NULL;
 int ay_status = AY_OK;

  h.closed = AY_FALSE;
  h.p1[0] = (double)(point1[0]);
  h.p1[1] = (double)(point1[1]);
  h.p1[2] = (double)(point1[2]);

  h.p2[0] = (double)(point2[0]);
  h.p2[1] = (double)(point2[1]);
  h.p2[2] = (double)(point2[2]);

  h.thetamax = (double)thetamax;

  ay_rrib_co.refine = (void *)(&h);
  ay_rrib_co.type = AY_IDHYPERBOLOID;
  ay_status = ay_object_copy(&ay_rrib_co, &o);
  ay_status = ay_object_link(o);

 return;
} /* ay_rrib_RiHyperboloid */


RtVoid ay_rrib_RiTorus(RtFloat majorradius, RtFloat minorradius, 
		       RtFloat phimin, RtFloat phimax, RtFloat thetamax, 
		       RtInt n, RtToken tokens[], RtPointer parms[])
{ 
 ay_torus_object t;
 ay_object *o = NULL;
 int ay_status = AY_OK;

  t.closed = AY_FALSE;
  t.majorrad = (double)majorradius;
  t.minorrad = (double)minorradius;
  t.phimin = (double)phimin;
  t.phimax = (double)phimax;
  t.thetamax = (double)thetamax;

  ay_rrib_co.refine = (void *)(&t);
  ay_rrib_co.type = AY_IDTORUS;
  ay_status = ay_object_copy(&ay_rrib_co, &o);
  ay_status = ay_object_link(o);

 return;
} /* ay_rrib_RiTorus */


RtVoid ay_rrib_RiNuPatch(RtInt nu, RtInt uorder, RtFloat uknot[], 
			 RtFloat umin, RtFloat umax, 
			 RtInt nv, RtInt vorder, RtFloat vknot[],
			 RtFloat vmin, RtFloat vmax,
			 RtInt n, RtToken tokens[], RtPointer parms[])
{
 ay_nurbpatch_object np;
 ay_object *o = NULL;
 int ay_status = AY_OK;
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
    return;
  
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
	    }

	  p += (nv*4);
	  pp += stride;
	}
    }



  ay_rrib_co.refine = (void *)(&np);
  ay_rrib_co.type = AY_IDNPATCH;
  ay_status = ay_object_copy(&ay_rrib_co, &o);
  ay_status = ay_object_link(o);

 return;
} /* ay_rrib_RiNuPatch */


RtVoid ay_rrib_RiTrimCurve(RtInt nloops, RtInt ncurves[], RtInt order[], 
			   RtFloat knot[], RtFloat min[], RtFloat max[], 
			   RtInt n[], RtFloat u[], RtFloat v[], RtFloat w[])
{

} /* ay_rrib_RiTrimCurve */


RtLightHandle ay_rrib_RiAreaLightSourceV( RtToken name, 
			     RtInt n, RtToken tokens[], RtPointer parms[] )
{
   (void)name; (void)n; (void)tokens; (void)parms;
   
   /* Fake it. */
   /*return (RtLightHandle)LastLightHandle++;*/
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
}


RtVoid ay_rrib_RiAttributeEnd( void )
{
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
   (void)frame;
}


RtVoid ay_rrib_RiFrameEnd( void )
{ 
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


RtLightHandle ay_rrib_RiLightSourceV( RtToken name, 
                              RtInt n, RtToken tokens[], RtPointer parms[] )
{
   (void)name; (void)n; (void)tokens; (void)parms;

   /* Fake it. */
   /*   return (RtLightHandle)LastLightHandle++;*/
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
   (void)angle; (void)dx; (void)dy; (void)dz; 
}


RtVoid ay_rrib_RiScale( RtFloat dx, RtFloat dy, RtFloat dz )
{ 
   (void)dx; (void)dy; (void)dz;
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
}


RtVoid ay_rrib_RiTransformEnd( void )
{ 
}


RtVoid ay_rrib_RiTranslate( RtFloat dx, RtFloat dy, RtFloat dz )
{ 
   (void)dx; (void)dy; (void)dz;
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

int
ay_rrib_initgprims()
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


 return ay_status;
}


int
ay_rrib_readrib(char *filename, int frame)
{
 RIB_HANDLE  rib;

  ay_object_defaults(&ay_rrib_co);

  /* Table included in libraries. */ 
  rib = RibOpen( filename, kRIB_LAST_RI, gRibNopRITable);


  /* fill table */
  
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
 int ay_status = AY_OK;

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

  ay_status = ay_rrib_initgprims();

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
