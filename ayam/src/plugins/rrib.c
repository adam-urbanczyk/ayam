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
/* as these would clash with the Affine includes above */

#ifdef AYUSEBMRTRIBOUT
#undef AYUSEBMRTRIBOUT
#endif

#ifdef AYUSEAFFINE
#undef AYUSEAFFINE
#endif

#include <ayam.h>

#define _RIPRIV_FUNC_TYPES

/* global variables for this module: */

char ay_rrib_version_ma[] = AY_VERSIONSTR;
char ay_rrib_version_mi[] = AY_VERSIONSTRMI;

/* current object:
 * all object reading functions enter data to this object,
 * link object type specific data to it, then copy it
 * into the Ayam scene using ay_rrib_linkobject()
 */
ay_object ay_rrib_co;

/* current material object */
ay_mat_object ay_rrib_cm;

/* attribute state stack */
typedef struct ay_rrib_attrstate_s {
  struct ay_rrib_attrstate_s *next;

  char *identifier_name;

  ay_object *trimcurves;

  int light_shadows;
  int light_samples;

  int read_arealight_geom;

  /* RiStandard (3.1) Attributes */

  /* Color */
  int colr, colg, colb;

  /* Opacity */
  int opr, opg, opb; 

  /* Matte */
  int matte; /* no, yes */

  /* Shading */
  double shading_rate;
  int shading_interpolation; /* constant, smooth */

  /* Displacement */
  double dbound_val; /* displacement bound value */
  int dbound; /* coordinate system for bound value */

  /* Sidedness */
  int sides; /* two-sided, one-sided */

  /* Shaders */
  /* Surface */
  ay_shader *sshader;
  /* Displacement */
  ay_shader *dshader;
  /* Interior */
  ay_shader *ishader;
  /* Exterior */
  ay_shader *eshader;

  /* BMRT Specific Attributes */  
   /* Shadows */
   int cast_shadows; /* Os, none, opaque, surface */

   /* Displacements */
   int true_displacement; /* no, yes */

   /* Visibility */
   int camera; /* yes, no */
   int reflection; /* yes, no */
   int shadow; /* yes, no */

  /* all other RiAttributes */
  ay_tag_object *tags;

  int btype_u, btype_v;
  RtBasis *ubasisptr;
  RtBasis *vbasisptr;
  RtInt ustep;
  RtInt vstep;

  RtFloat s1, s2, s3, s4, t1, t2, t3, t4;

} ay_rrib_attrstate;

ay_rrib_attrstate *ay_rrib_cattributes;

/* transformation stack */
typedef struct ay_rrib_trafostate_s {
  struct ay_rrib_trafostate_s *next;

  double m[16];

} ay_rrib_trafostate;

ay_rrib_trafostate *ay_rrib_ctrafos;


/* current frame */
int ay_rrib_cframe;

/* last read object */
ay_object *ay_rrib_lrobject;

/* light handle */
int ay_rrib_clighthandle;

/* first read light */
ay_object *ay_rrib_flobject;

/* object handle */
int ay_rrib_cobjecthandle;

/* objects (with handles):
 * objects defined in the RIB with RiObject will be linked
 * to these pointers for later use with RiObjectInstance
 */
ay_list_object *ay_rrib_objects;
ay_list_object *ay_rrib_lastobject;
/* temporary space for ay_next while reading objects between
   RiObjectBegin/End */
ay_object **ay_rrib_aynext;

/* fov */
double ay_rrib_fov;

/* cliping planes */
double ay_rrib_near, ay_rrib_far;

/* image size */
int width, height;

/* material */
int ay_rrib_lastmaterialnum;

/* import options */
int ay_rrib_readframe; /* number of frame to read */
int ay_rrib_readoptions; /* read RiOptions */
int ay_rrib_readcamera; /* read (create) the Camera */
int ay_rrib_readlights; /* read lights */
int ay_rrib_readmaterial; /* read material and attributes */
int ay_rrib_readmateriali; /* read material and attributes (internal) */
int ay_rrib_readpartial; /* read partial RIB (e.g. without WorldBegin/End) */
int ay_rrib_errorlevel; /* 0: silence, 1: errors, 2: warnings, 3: all */

/* grib is used by Affine to specify the current RIB */
/* ay_rrib_RiReadArchive() keeps a copy of this on the stack
   when recursively descending into other RIBs */
PRIB_INSTANCE grib;

/* the following tables are used by Affine to parse vectors of points */ 
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

enum {
   NTBL_N,
   NTBL_Nn,
   NTBL_LAST
};

char N[] = {
    0,  1 , 'N',
    2,  2 ,'\0',  0,
    0,  3 ,'n','\0',  1,
};


/* prototypes of functions local to this module: */

RtVoid ay_rrib_RiSphere(RtFloat radius, RtFloat zmin, RtFloat zmax,
			RtFloat thetamax,
			RtInt n, RtToken tokens[], RtPointer parms[]);

RtVoid ay_rrib_RiCylinder(RtFloat radius, RtFloat zmin, RtFloat zmax,
			  RtFloat thetamax,
			  RtInt n, RtToken tokens[], RtPointer parms[]);

RtVoid ay_rrib_RiDisk(RtFloat height, RtFloat radius, RtFloat thetamax,
		      RtInt n, RtToken tokens[], RtPointer parms[]);

RtVoid ay_rrib_RiCone(RtFloat height, RtFloat radius, RtFloat thetamax,
		      RtInt n, RtToken tokens[], RtPointer parms[]);

RtVoid ay_rrib_RiParaboloid(RtFloat rmax, 
			    RtFloat zmin, RtFloat zmax, RtFloat thetamax,
			    RtInt n, RtToken tokens[], RtPointer parms[]);

RtVoid ay_rrib_RiHyperboloid(RtPoint point1, RtPoint point2, RtFloat thetamax,
			     RtInt n, RtToken tokens[], RtPointer parms[]);

RtVoid ay_rrib_RiTorus(RtFloat majorradius, RtFloat minorradius, 
		       RtFloat phimin, RtFloat phimax, RtFloat thetamax, 
		       RtInt n, RtToken tokens[], RtPointer parms[]);

RtVoid ay_rrib_RiNuPatch(RtInt nu, RtInt uorder, RtFloat uknot[],
			 RtFloat umin, RtFloat umax,
			 RtInt nv, RtInt vorder, RtFloat vknot[],
			 RtFloat vmin, RtFloat vmax,
			 RtInt n, RtToken tokens[], RtPointer parms[]);

RtVoid ay_rrib_RiTrimCurve(RtInt nloops, RtInt ncurves[], RtInt order[],
			   RtFloat knot[], RtFloat min[], RtFloat max[],
			   RtInt n[], RtFloat u[], RtFloat v[], RtFloat w[]);

RtLightHandle ay_rrib_RiLightSource(RtToken name,
				RtInt n, RtToken tokens[], RtPointer parms[]);

RtLightHandle ay_rrib_RiAreaLightSource(RtToken name,
				RtInt n, RtToken tokens[], RtPointer parms[]);

RtVoid ay_rrib_RiAtmosphere(RtToken name,
			    RtInt n, RtToken tokens[], RtPointer parms[]);

RtVoid ay_rrib_RiAttribute(RtToken name,
			   RtInt n, RtToken tokens[], RtPointer parms[]);

RtVoid ay_rrib_RiAttributeBegin(void);

RtVoid ay_rrib_RiAttributeEnd(void);


RtVoid ay_rrib_RiBasis(RtBasis ubasis, RtInt ustep,
		       RtBasis vbasis, RtInt vstep);

RtVoid ay_rrib_RiBound(RtBound bound);

RtVoid ay_rrib_RiClipping(RtFloat hither, RtFloat yon);

RtVoid ay_rrib_RiColor(RtColor color);

RtVoid ay_rrib_RiColorSamples(RtInt n, RtFloat nRGB[], RtFloat RGBn[]);

RtVoid ay_rrib_RiCoordinateSystem(RtToken space);

RtVoid ay_rrib_RiCoordSysTransform(RtToken space);

RtVoid ay_rrib_RiCropWindow(RtFloat xmin, RtFloat xmax, 
			    RtFloat ymin, RtFloat ymax);

RtVoid ay_rrib_RiCurves(RtToken type, RtInt ncurves, RtInt nvertices[], 
		  RtToken wrap, 
		  RtInt n, RtToken tokens[], RtPointer parms[]);

RtToken ay_rrib_RiDeclare(char *name, char *declaration);

RtVoid ay_rrib_RiDeformation(RtToken name,
			     RtInt n, RtToken tokens[], RtPointer parms[]);

RtVoid ay_rrib_RiDepthOfField(RtFloat fstop, RtFloat focallength, 
			      RtFloat focaldistance);

RtVoid ay_rrib_RiDetail(RtBound bound);

RtVoid ay_rrib_RiDetailRange(RtFloat minvisible, RtFloat lowertransition, 
			     RtFloat uppertransition, RtFloat maxvisible);

RtVoid ay_rrib_RiDisplacement(RtToken name,
			      RtInt n, RtToken tokens[], RtPointer parms[]);

RtVoid ay_rrib_RiDisplay(char *name, RtToken type, RtToken mode,
			 RtInt n, RtToken tokens[], RtPointer parms[]);

RtVoid ay_rrib_RiErrorHandler(RtErrorHandler handler);

RtVoid ay_rrib_RiExposure(RtFloat gain, RtFloat gamma);

RtVoid ay_rrib_RiExterior(RtToken name,
			  RtInt n, RtToken tokens[], RtPointer parms[]);

RtVoid ay_rrib_RiFormat(RtInt xres, RtInt yres, RtFloat aspect);

RtVoid ay_rrib_RiFrameAspectRatio(RtFloat aspect);

RtVoid ay_rrib_RiFrameBegin(RtInt frame);

RtVoid ay_rrib_RiFrameEnd(void);

RtVoid ay_rrib_RiGeometricApproximation(RtToken type, RtFloat value);

RtVoid ay_rrib_RiGeometricRepresentation(RtToken type);

RtVoid ay_rrib_RiGeometry(RtToken type,
			  RtInt n, RtToken tokens[], RtPointer parms[]);

RtVoid ay_rrib_RiHider(RtToken type,
		       RtInt n, RtToken tokens[], RtPointer parms[]);

RtVoid ay_rrib_RiIdentity(void);

RtVoid ay_rrib_RiIlluminate(RtLightHandle light, RtBoolean onoff);

RtVoid ay_rrib_RiImager(RtToken name,
			RtInt n, RtToken tokens[], RtPointer parms[]);

RtVoid ay_rrib_RiImplicit(RtInt a, RtInt b[], RtInt c, RtFloat d[],
			 RtInt e, RtFloat f[], RtInt g, RtFloat h[],
			 RtInt n, RtToken tokens[], RtPointer parms[]);

RtVoid ay_rrib_RiInterior(RtToken name, 
			  RtInt n, RtToken tokens[], RtPointer parms[]);

RtVoid ay_rrib_RiMakeBump(char *picturename, char *texturename, 
		RtToken swrap, RtToken twrap,
		RtFilterFunc filterfunc, RtFloat swidth, RtFloat twidth,
		RtInt n, RtToken tokens[], RtPointer parms[]);


RtVoid ay_rrib_RiMakeCubeFaceEnvironment(char *px, char *nx, char *py,
					 char *ny, char *pz, char *nz,
					 char *texturename,
					 RtFloat fov,
					 RtFilterFunc filterfunc,
					 RtFloat swidth, RtFloat twidth,
					 RtInt n,
					 RtToken tokens[], RtPointer parms[]);

RtVoid ay_rrib_RiMakeLatLongEnvironment(char *picturename, char *texturename, 
					RtFilterFunc filterfunc,
					RtFloat swidth, RtFloat twidth,
					RtInt n,
					RtToken tokens[], RtPointer parms[]);

RtVoid ay_rrib_RiMakeShadow(char *picturename, char *texturename,
			    RtInt n, RtToken tokens[], RtPointer parms[]);

RtVoid ay_rrib_RiMakeTexture(char *picturename, char *texturename,
			     RtToken swrap, RtToken twrap,
			     RtFilterFunc filterfunc,
			     RtFloat swidth, RtFloat twidth,
			     RtInt n, RtToken tokens[], RtPointer parms[]);

RtVoid ay_rrib_RiMatte(RtBoolean onoff);

RtVoid ay_rrib_RiMotionBegin(RtInt n, RtFloat times[]);

RtVoid ay_rrib_RiMotionEnd(void);

RtVoid ay_rrib_RiNuCurves(RtInt ncurves, RtInt nvertices[], RtInt order[],
			  RtFloat knot[], RtFloat min[], RtFloat max[],
			  RtInt n, RtToken tokens[], RtPointer parms[]);

RtObjectHandle ay_rrib_RiObjectBegin(void);

RtVoid ay_rrib_RiObjectEnd(void);

RtVoid ay_rrib_RiObjectInstance(RtObjectHandle handle);

RtVoid ay_rrib_RiOpacity(RtColor color);


RtVoid ay_rrib_RiOption(RtToken name,
			RtInt n, RtToken tokens[], RtPointer parms[]);

RtVoid ay_rrib_RiOrientation(RtToken orientation);


RtVoid ay_rrib_RiPatch(RtToken type,
		       RtInt n, RtToken tokens[], RtPointer parms[]);

RtVoid ay_rrib_RiPatchMesh(RtToken type, RtInt nu, RtToken uwrap, 
			   RtInt nv, RtToken vwrap, 
			   RtInt n, RtToken tokens[], RtPointer parms[]);

RtVoid ay_rrib_RiPerspective(RtFloat fov);

RtVoid ay_rrib_RiPixelFilter(RtFilterFunc filterfunc, 
			     RtFloat xwidth, RtFloat ywidth);

RtVoid ay_rrib_RiPixelSamples(RtFloat xsamples, RtFloat ysamples);

RtVoid ay_rrib_RiPixelVariance(RtFloat variation);

RtVoid ay_rrib_RiPoints(RtInt npoints,
			RtInt n, RtToken tokens[], RtPointer parms[]);

RtVoid ay_rrib_RiPointsGeneralPolygons(RtInt npolys, RtInt nloops[],
				RtInt nvertices[], RtInt vertices[],
				RtInt n, RtToken tokens[], RtPointer parms[]);

RtVoid ay_rrib_RiGeneralPolygon(RtInt nloops, RtInt nvertices[],
				RtInt n, RtToken tokens[], RtPointer parms[]);

RtVoid
ay_rrib_RiPointsPolygons(RtInt npolys, RtInt nvertices[], RtInt vertices[],
			 RtInt n, RtToken tokens[], RtPointer parms[]);

RtVoid ay_rrib_RiPolygon(RtInt nvertices,
			 RtInt n, RtToken tokens[], RtPointer parms[]);

RtVoid ay_rrib_RiProjection(RtToken name, 
			    RtInt n, RtToken tokens[], RtPointer parms[]);

RtVoid ay_rrib_RiQuantize(RtToken type, RtInt one, 
			  RtInt min, RtInt max, RtFloat ampl);

RtVoid ay_rrib_RiReadArchive(RtToken name,
			     RtVoid (*callback)( RtToken, char*, char* ),
			     RtInt n, RtToken tokens[], RtPointer parms[]);

RtVoid ay_rrib_RiRelativeDetail(RtFloat relativedetail);

RtVoid ay_rrib_RiResource(RtToken handle, RtToken type,
			  RtInt n, RtToken tokens[], RtPointer parms[]);

RtVoid ay_rrib_RiReverseOrientation(void);

RtVoid ay_rrib_RiConcatTransform(RtMatrix transform);

RtVoid ay_rrib_RiRotate(RtFloat angle, RtFloat dx, RtFloat dy, RtFloat dz);

RtVoid ay_rrib_RiScale(RtFloat dx, RtFloat dy, RtFloat dz);

RtVoid ay_rrib_RiTransform(RtMatrix transform);

RtVoid ay_rrib_RiTranslate(RtFloat dx, RtFloat dy, RtFloat dz);

RtVoid ay_rrib_RiScreenWindow(RtFloat left, RtFloat right,
			      RtFloat bottom, RtFloat top);

RtVoid ay_rrib_RiShadingInterpolation(RtToken type);

RtVoid ay_rrib_RiShadingRate(RtFloat size);

RtVoid ay_rrib_RiShutter(RtFloat min, RtFloat max);

RtVoid ay_rrib_RiSides(RtInt sides);

RtVoid ay_rrib_RiSkew(RtFloat angle, RtFloat dx1, RtFloat dy1, RtFloat dz1,
		      RtFloat dx2, RtFloat dy2, RtFloat dz2);

RtVoid ay_rrib_RiSolidBegin(RtToken operation);

RtVoid ay_rrib_RiSolidEnd(void);

RtVoid ay_rrib_RiSurface(RtToken name,
			 RtInt n, RtToken tokens[], RtPointer parms[]);

RtVoid ay_rrib_RiSubdivisionMesh(RtToken scheme, RtInt nfaces,
				 RtInt nvertices[], RtInt vertices[],
				 RtInt ntags, RtToken tags[],
				 RtInt nargs[],
				 RtInt intargs[], RtFloat floatargs[],
				 RtInt n, RtToken tokens[], RtPointer parms[]);

RtVoid ay_rrib_RiTextureCoordinates(RtFloat s1, RtFloat t1,
				    RtFloat s2, RtFloat t2,
				    RtFloat s3, RtFloat t3,
				    RtFloat s4, RtFloat t4);
RtVoid ay_rrib_RiTransformBegin(void);

RtVoid ay_rrib_RiTransformEnd(void);

RtVoid ay_rrib_RiWorldBegin(void);

RtVoid ay_rrib_RiWorldEnd(void);

RtVoid ay_rrib_RiBegin(RtToken name);

RtVoid ay_rrib_RiEnd(void);

RtVoid ay_rrib_RiArchiveRecord(RtToken type, char *format, char *s);

RtVoid ay_rrib_RiProcedural(RtPointer data, RtBound bound,
			    RtVoid (*subdivfunc)(RtPointer, RtFloat),
			    RtVoid (*freefunc)(RtPointer));

RtPoint* ay_rrib_RiTransformPoints(RtToken fromspace, RtToken tospace,
				   RtInt n, RtPoint points[]);

RtVoid ay_rrib_RiErrorIgnore(RtInt code, RtInt severity, char *msg);

RtVoid ay_rrib_RiErrorPrint(RtInt code, RtInt severity, char *msg);

RtVoid ay_rrib_RiErrorAbort(RtInt code, RtInt severity, char *msg);

RtFloat ay_rrib_RiBoxFilter(RtFloat x, RtFloat y,
			    RtFloat xwidth, RtFloat ywidth);

RtFloat ay_rrib_RiTriangleFilter(RtFloat x, RtFloat y, 
				 RtFloat xwidth, RtFloat ywidth);

RtFloat ay_rrib_RiCatmullRomFilter(RtFloat x, RtFloat y, 
				   RtFloat xwidth, RtFloat ywidth);

RtFloat ay_rrib_RiGaussianFilter(RtFloat x, RtFloat y, 
				 RtFloat xwidth, RtFloat ywidth);

RtFloat ay_rrib_RiSincFilter(RtFloat x, RtFloat y,
			     RtFloat xwidth, RtFloat ywidth);

RtVoid ay_rrib_Ri_version(RtFloat version);

RtVoid ay_rrib_RiProcDelayedReadArchive(RtPointer data, RtFloat detail);

RtVoid ay_rrib_RiProcRunProgram(RtPointer data, RtFloat detail);

RtVoid ay_rrib_RiProcDynamicLoad(RtPointer data, RtFloat detail);

/**/

void ay_rrib_readshader(char *sname, int stype,
			RtInt n, RtToken tokens[], RtPointer parms[],
			ay_shader **result);

void ay_rrib_readparams(int n, RtToken tokens[], RtPointer parms[],
			Tcl_DString *ds);

void ay_rrib_readtag(char *tagtype, char *tagname, char *name,
		     int i, RtToken tokens[], RtPointer parms[],
		     ay_tag_object **destination);

void ay_rrib_fixname(char *name);

void ay_rrib_initgeneral(void);

void ay_rrib_cleargeneral(void);

void ay_rrib_initoptions(void);

void ay_rrib_initgprims(void);

void ay_rrib_cleargprims(void);

void ay_rrib_pushattribs(void);

void ay_rrib_popattribs(void);

void ay_rrib_pushtrafos(void);

void ay_rrib_poptrafos(void);

void ay_rrib_trafotoobject(ay_object *o, double *transform);

int ay_rrib_comptags(ay_object *o1, ay_object *o2);

void ay_rrib_linkmaterial(ay_object *o);

void ay_rrib_linktexcoord(ay_object *o);

void ay_rrib_linkobject(void *object, int type);

int ay_rrib_printerror(RIB_HANDLE rib, int code, int severity,
		       PRIB_ERROR error);

int ay_rrib_readrib(char *filename, int frame, int read_camera,
		    int read_options,
		    int read_lights, int read_material, int read_partial,
		    int error_level);

/* functions: */

RtVoid
ay_rrib_RiSphere(RtFloat radius, RtFloat zmin, RtFloat zmax,
		 RtFloat thetamax,
		 RtInt n, RtToken tokens[], RtPointer parms[])
{
 ay_sphere_object s;

  s.is_simple = AY_FALSE;
  s.closed = AY_FALSE;
  s.radius = (double)radius;
  s.zmin = (double)zmin;
  s.zmax = (double)zmax;
  s.thetamax = (double)thetamax;

  ay_rrib_linkobject((void *)(&s), AY_IDSPHERE);

 return;
} /* ay_rrib_RiSphere */


RtVoid
ay_rrib_RiCylinder(RtFloat radius, RtFloat zmin, RtFloat zmax,
		   RtFloat thetamax,
		   RtInt n, RtToken tokens[], RtPointer parms[])
{
 ay_cylinder_object c;

  c.is_simple = AY_FALSE;
  c.closed = AY_FALSE;
  c.radius = (double)radius;
  c.zmin = (double)zmin;
  c.zmax = (double)zmax;
  c.thetamax = (double)thetamax;

  ay_rrib_linkobject((void *)(&c), AY_IDCYLINDER);

 return;
} /* ay_rrib_RiCylinder */


RtVoid
ay_rrib_RiDisk(RtFloat height, RtFloat radius, RtFloat thetamax,
	       RtInt n, RtToken tokens[], RtPointer parms[])
{
 ay_disk_object d;

  d.is_simple = AY_FALSE;
  d.height = (double)height;
  d.radius = (double)radius;
  d.thetamax = (double)thetamax;

  ay_rrib_linkobject((void *)(&d), AY_IDDISK);

 return;
} /* ay_rrib_RiDisk */


RtVoid
ay_rrib_RiCone(RtFloat height, RtFloat radius, RtFloat thetamax,
	       RtInt n, RtToken tokens[], RtPointer parms[])
{
 ay_cone_object c;

  c.is_simple = AY_FALSE;
  c.closed = AY_FALSE;
  c.height = (double)height;
  c.radius = (double)radius;
  c.thetamax = (double)thetamax;

  ay_rrib_linkobject((void *)(&c), AY_IDCONE);

 return;
} /* ay_rrib_RiCone */



RtVoid
ay_rrib_RiParaboloid(RtFloat rmax, 
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


RtVoid
ay_rrib_RiHyperboloid(RtPoint point1, RtPoint point2, RtFloat thetamax,
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


RtVoid
ay_rrib_RiTorus(RtFloat majorradius, RtFloat minorradius, 
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


RtVoid
ay_rrib_RiNuPatch(RtInt nu, RtInt uorder, RtFloat uknot[],
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

  memset(&np, '\0', sizeof(ay_nurbpatch_object));
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
  ay_rrib_co.hide_children = AY_TRUE;
  ay_rrib_linkobject((void *)(&np), AY_IDNPATCH);
  ay_rrib_co.hide_children = AY_FALSE;
  ay_rrib_co.parent = AY_FALSE;
  ay_object_delete(ay_rrib_co.down);
  ay_rrib_co.down = NULL;

  free(np.uknotv);
  free(np.vknotv);
  free(np.controlv);

 return;
} /* ay_rrib_RiNuPatch */


RtVoid
ay_rrib_RiTrimCurve(RtInt nloops, RtInt ncurves[], RtInt order[],
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
 char fname[] = "ay_rrib_RiLightSource";

  if(!ay_rrib_readlights)
    return((RtLightHandle)(ay_rrib_clighthandle++));

  /* load some defaults */
  l.type = AY_LITCUSTOM;
  l.lshader = NULL;
  l.intensity = 1.0;
  l.tfrom[0] = 0.0;
  l.tfrom[1] = 0.0;
  l.tfrom[2] = 0.0;
  l.on = AY_TRUE;
  l.use_sm = AY_FALSE;
  l.tto[0] = 0.0;
  l.tto[1] = 0.0;
  l.tto[2] = 1.0;

  l.colr = 255;
  l.colg = 255;
  l.colb = 255;

  l.cone_angle = 30.0;
  l.cone_delta_angle = 5.0;
  l.beam_distribution = 2.0;
  l.sm_resolution = 0;

  if(ay_rrib_cattributes->light_samples != -1)
    l.samples = ay_rrib_cattributes->light_samples;
  else
    l.samples = 1;

  if(ay_rrib_cattributes->light_shadows != -1)
    l.shadows = ay_rrib_cattributes->light_shadows;
  else
    l.shadows = 0;


  /* check for default light source types first */
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

  if(l.type != AY_LITCUSTOM)
    {
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
    }
  else
    {
      ay_rrib_readshader(name, AY_STLIGHT, n, tokens, parms, &(l.lshader));
    }

  ay_rrib_co.parent = AY_TRUE;
  ay_status = ay_object_crtendlevel(&(ay_rrib_co.down));
  if(ay_status)
    {
      ay_error(AY_ERROR, fname,
	   "Could not create terminating level object, scene is corrupt now!");
    }

  ay_rrib_co.refine = (void *)(&l);
  ay_rrib_co.type = AY_IDLIGHT;

  ay_rrib_trafotoobject(&ay_rrib_co, ay_rrib_ctrafos->m);

  ay_status = ay_object_copy(&ay_rrib_co, &o);
  ay_status = ay_object_link(o);
  ay_rrib_lrobject = o;
  ay_rrib_co.parent = AY_FALSE;
  ay_object_delete(ay_rrib_co.down);
  ay_rrib_co.down = NULL;

  if(l.lshader)
    {
      ay_shader_free(l.lshader);
      l.lshader = NULL;
    }
  
  if(ay_rrib_clighthandle == 1)
    {
      ay_rrib_flobject = o;
    }

 return((RtLightHandle)(ay_rrib_clighthandle++));
} /* ay_rrib_RiLightSource */


RtLightHandle
ay_rrib_RiAreaLightSource(RtToken name,
			  RtInt n, RtToken tokens[], RtPointer parms[])
{
 RtLightHandle lh;

  if(!ay_rrib_readlights)
    return((RtLightHandle)(ay_rrib_clighthandle++));

  /* first, read area light as normal light source */
  lh = ay_rrib_RiLightSource(name, n, tokens, parms);

  /* then, prepare everything to read arealight geometry */
  ay_rrib_cattributes->read_arealight_geom = 1;

 return(lh);
} /* ay_rrib_RiAreaLightSource */


RtVoid
ay_rrib_RiAtmosphere(RtToken name,
		     RtInt n, RtToken tokens[], RtPointer parms[])
{
 int ay_status = AY_OK;
 ay_root_object *root = NULL;

  root = (ay_root_object *)ay_root->refine;

  if(root->atmosphere)
    ay_status = ay_shader_free(root->atmosphere);
  root->atmosphere = NULL;

  ay_rrib_readshader(name, AY_STATMOSPHERE, n, tokens, parms,
		     &(root->atmosphere));

 return;
} /* ay_rrib_RiAtmosphere */


RtVoid
ay_rrib_RiAttribute(RtToken name,
		    RtInt n, RtToken tokens[], RtPointer parms[])
{
 int i, itemp, attribute_handled = AY_FALSE;
 char *stemp = NULL;
 char fname[] = "ay_rrib_RiAttribute";

  if(!strcmp(name,"identifier"))
    {
      for(i = 0; i < n; i++)
	{
	  if(!strcmp(tokens[0], "name"))
	    {
	      stemp = *((char **)(parms[i]));
	      if(ay_rrib_cattributes->identifier_name)
		free(ay_rrib_cattributes->identifier_name);
	      if(!(ay_rrib_cattributes->identifier_name =
		   calloc(strlen(stemp)+1, sizeof(char))))
		{
		  ay_error(AY_EOMEM, fname, NULL);
		  return;
		}
	      strcpy(ay_rrib_cattributes->identifier_name, stemp);
	      ay_rrib_fixname(ay_rrib_cattributes->identifier_name);
	      attribute_handled = AY_TRUE;
	    } /* if */
	  if(!attribute_handled)
	    {
	      ay_rrib_readtag(ay_riattr_tagtype, "RiAttribute", name,
			      i, tokens, parms, &(ay_rrib_cattributes->tags));
	    } /* if */
	} /* for */
      return;
    } /* if */

  if(!strcmp(name,"light"))
    {
      for(i = 0; i < n; i++)
	{
	  if(!strcmp(tokens[i], "shadows"))
	    {
	      if(!strcmp(*((char **)(parms[i])), "on"))
		ay_rrib_cattributes->light_shadows = AY_TRUE;
	      else
		ay_rrib_cattributes->light_shadows = AY_FALSE;
	      attribute_handled = AY_TRUE;
	    }
	  if(!strcmp(tokens[i], "nsamples"))
	    {

	      ay_rrib_cattributes->light_samples =
		(int)(*((RtInt *)(parms[i])));
	      attribute_handled = AY_TRUE;
	    }
	  if(!attribute_handled)
	    {
	      ay_rrib_readtag(ay_riattr_tagtype, "RiAttribute", name,
			      i, tokens, parms, &(ay_rrib_cattributes->tags));
	    }
	} /* for */
      return;
    } /* if */

  if(!strcmp(name,"render"))
    {
      for(i = 0; i < n; i++)
	{
	  if(!strcmp(tokens[i], "truedisplacement"))
	    {
	      ay_rrib_cattributes->true_displacement =
		(int)(*((RtInt *)(parms[i])));
	      attribute_handled = AY_TRUE;
	    }
	  if(!strcmp(tokens[i], "cast_shadows"))
	    {
	      itemp = 0;
	      if(!strcmp(*((char **)(parms[i])), "none"))
		{
		  itemp = 1;
		}
	      if(!strcmp(*((char **)(parms[i])), "opaque"))
		{
		  itemp = 2;
		}
	      if(!strcmp(*((char **)(parms[i])), "shader"))
		{
		  itemp = 3;
		}
	      ay_rrib_cattributes->cast_shadows = itemp;
	      attribute_handled = AY_TRUE;
	    } /* if */
	  if(!strcmp(tokens[i], "visibility"))
	    {
	      itemp = (int)(*((RtInt *)(parms[i])));
	      if(itemp-4 >= 0)
		{
		  ay_rrib_cattributes->shadow = AY_TRUE;
		  itemp -= 4;
		}
	      if(itemp-2 >= 0)
		{
		  ay_rrib_cattributes->reflection = AY_TRUE;
		  itemp -= 2;
		}
	      if(itemp-1 >= 0)
		{
		  ay_rrib_cattributes->camera = AY_TRUE;
		}
	      attribute_handled = AY_TRUE;
	    } /* if */
	  if(!attribute_handled)
	    {
	      ay_rrib_readtag(ay_riattr_tagtype, "RiAttribute", name,
			      i, tokens, parms, &(ay_rrib_cattributes->tags));
	    }
	} /* for */
      return;
    } /* if */

  if(!strcmp(name,"displacementbound"))
    {
      for(i = 0; i < n; i++)
	{
	  if(!strcmp(tokens[i], "coordinatesystem"))
	    {
	      itemp = 0;
	      /* XXXX is this complete? */
	      if(!strcmp(*((char **)(parms[i])), "camera"))
		{
		  itemp = 2;
		}
	      if(!strcmp(*((char **)(parms[i])), "shader"))
		{
		  itemp = 1;
		}
	      if(!strcmp(*((char **)(parms[i])), "object"))
		{
		  itemp = 0;
		}
	      ay_rrib_cattributes->dbound = itemp;
	      attribute_handled = AY_TRUE;
	    } /* if */
	  if(!strcmp(tokens[i], "sphere"))
	    {
	      ay_rrib_cattributes->dbound_val =
		(double)(*((RtFloat *)(parms[i])));
	      attribute_handled = AY_TRUE;
	    }
	  if(!attribute_handled)
	    {
	      ay_rrib_readtag(ay_riattr_tagtype, "RiAttribute", name,
			      i, tokens, parms, &(ay_rrib_cattributes->tags));
	    }
	} /* for */
      return;
    } /* if */

  for(i = 0; i < n; i++)
    {
      ay_rrib_readtag(ay_riattr_tagtype, "RiAttribute", name,
		      i, tokens, parms, &(ay_rrib_cattributes->tags));
    }

 return;
} /* ay_rrib_RiAttribute */


RtVoid
ay_rrib_RiAttributeBegin(void)
{

  ay_rrib_pushattribs();
  ay_rrib_pushtrafos();

} /* ay_rrib_RiAttributeBegin */


RtVoid
ay_rrib_RiAttributeEnd(void)
{

  ay_rrib_poptrafos();
  ay_rrib_popattribs();

} /* ay_rrib_RiAttributeEnd */



RtVoid
ay_rrib_RiBasis(RtBasis ubasis, RtInt ustep,
		RtBasis vbasis, RtInt vstep)
{
 int ubasis_custom = AY_TRUE;
 int vbasis_custom = AY_TRUE;

  ay_rrib_cattributes->ustep = ustep;
  ay_rrib_cattributes->vstep = vstep;

  if(ubasis == RiBezierBasis)
    {
      ay_rrib_cattributes->btype_u = AY_BTBEZIER;
      ubasis_custom = AY_FALSE;
    }

  if(ubasis == RiBSplineBasis)
    {
      ay_rrib_cattributes->btype_u = AY_BTBSPLINE;
      ubasis_custom = AY_FALSE;
    }

  if(ubasis == RiCatmullRomBasis)
    {
      ay_rrib_cattributes->btype_u = AY_BTCATMULLROM;
      ubasis_custom = AY_FALSE;
    }

  if(ubasis == RiHermiteBasis)
    {
      ay_rrib_cattributes->btype_u = AY_BTHERMITE;
      ubasis_custom = AY_FALSE;
    }

  if(vbasis == RiBezierBasis)
    {
      ay_rrib_cattributes->btype_v = AY_BTBEZIER;
      vbasis_custom = AY_FALSE;
    }

  if(vbasis == RiBSplineBasis)
    {
      ay_rrib_cattributes->btype_v = AY_BTBSPLINE;
      vbasis_custom = AY_FALSE;
    }

  if(vbasis == RiCatmullRomBasis)
    {
      ay_rrib_cattributes->btype_v = AY_BTCATMULLROM;
      vbasis_custom = AY_FALSE;
    }

  if(vbasis == RiHermiteBasis)
    {
      ay_rrib_cattributes->btype_v = AY_BTHERMITE;
      vbasis_custom = AY_FALSE;
    }

 return;
} /* ay_rrib_RiBasis */


RtVoid
ay_rrib_RiBound(RtBound bound)
{
   (void)bound;
} /* ay_rrib_RiBound */


RtVoid
ay_rrib_RiClipping(RtFloat hither, RtFloat yon)
{
  ay_rrib_near = (double)hither;
  ay_rrib_far = (double)yon;

 return;
} /* ay_rrib_RiClipping */


RtVoid
ay_rrib_RiColor(RtColor color)
{

  ay_rrib_cattributes->colr = (int)(color[0]*255);
  ay_rrib_cattributes->colg = (int)(color[1]*255);
  ay_rrib_cattributes->colb = (int)(color[2]*255);

 return;
} /* ay_rrib_RiColor */


RtVoid
ay_rrib_RiColorSamples(RtInt n, RtFloat nRGB[], RtFloat RGBn[])
{
   (void)n; (void)nRGB; (void)RGBn;
} /* ay_rrib_RiColorSamples */


RtVoid
ay_rrib_RiCoordinateSystem(RtToken space)
{
   (void)space; 
} /* ay_rrib_RiCoordinateSystem */


RtVoid
ay_rrib_RiCoordSysTransform(RtToken space)
{
   (void)space; 
} /* ay_rrib_RiCoordSysTransform */


RtVoid
ay_rrib_RiCropWindow(RtFloat xmin, RtFloat xmax, 
		     RtFloat ymin, RtFloat ymax)
{
   (void)xmin; (void)xmax; (void)ymin; (void)ymax; 
} /* ay_rrib_RiCropWindow */


RtVoid
ay_rrib_RiCurves(RtToken type, RtInt ncurves, RtInt nvertices[], 
		 RtToken wrap, 
		 RtInt n, RtToken tokens[], RtPointer parms[])
{
   (void)type; (void)ncurves; (void)nvertices; (void)wrap;
   (void)n; (void)tokens; (void)parms; 
} /* ay_rrib_RiCurves */


RtToken
ay_rrib_RiDeclare(char *name, char *declaration)
{
/*
 char *newname = NULL;
 char fname[] = "ay_rrib_RiDeclare";

  if(!(newname = calloc(strlen(name)+1, sizeof(char))))
    return NULL;
  strcpy(newname, name);
 
  if(!RibDeclare(grib, newname, declaration))
    {
      ay_error(AY_ERROR, fname, "Declaration failed!");
    }
*/
 return name;
} /* ay_rrib_RiDeclare */


RtVoid
ay_rrib_RiDeformation(RtToken name,
		      RtInt n, RtToken tokens[], RtPointer parms[])
{
  (void)name; (void)n; (void)tokens; (void)parms;
} /* ay_rrib_RiDeformation */


RtVoid
ay_rrib_RiDepthOfField(RtFloat fstop, RtFloat focallength, 
		       RtFloat focaldistance)
{
  (void)fstop; (void)focallength; (void)focaldistance;
} /* ay_rrib_RiDepthOfField */


RtVoid
ay_rrib_RiDetail(RtBound bound)
{
  (void)bound;
} /* ay_rrib_RiDetail */


RtVoid
ay_rrib_RiDetailRange(RtFloat minvisible, RtFloat lowertransition, 
		      RtFloat uppertransition, RtFloat maxvisible)
{
   (void)minvisible; (void)lowertransition; 
   (void)uppertransition; (void)maxvisible;
} /* ay_rrib_RiDetailRange */




RtVoid
ay_rrib_RiDisplacement(RtToken name,
		       RtInt n, RtToken tokens[], RtPointer parms[])
{
 int ay_status = AY_OK;

  if(ay_rrib_cattributes->dshader)
    ay_status = ay_shader_free(ay_rrib_cattributes->dshader);
  ay_rrib_cattributes->dshader = NULL;

  ay_rrib_readshader(name, AY_STDISPLACEMENT, n, tokens, parms,
		     &(ay_rrib_cattributes->dshader));

 return;
} /* ay_rrib_RiDisplacement */


RtVoid
ay_rrib_RiDisplay(char *name, RtToken type, RtToken mode,
		  RtInt n, RtToken tokens[], RtPointer parms[])
{
 int ay_status = AY_OK;
 ay_tag_object *nt = NULL;
 char tname[] = "RiDisplay";
 Tcl_DString ds;

  if(!(nt = calloc(1, sizeof(ay_tag_object))))
    {
      return;
    }

  if(!(nt->name = calloc(strlen(tname)+1, sizeof(char))))
    {
      free(nt); return;
    }

  strcpy(nt->name, tname);
  nt->type = ay_ridisp_tagtype;
  Tcl_DStringInit(&ds);
  Tcl_DStringAppend(&ds, name, -1);
  Tcl_DStringAppend(&ds, ",", -1);
  Tcl_DStringAppend(&ds, type, -1);
  Tcl_DStringAppend(&ds, ",", -1);
  Tcl_DStringAppend(&ds, mode, -1);

  /* parse parameters, if there are any */
  if(n > 0)
    {
      ay_rrib_readparams(n, tokens, parms, &ds);

    } /* if */
  
  if(!(nt->val = calloc(strlen(Tcl_DStringValue(&ds))+1, sizeof(char))))
    {
      free(nt->name);
      free(nt);
      Tcl_DStringFree(&ds);
      return;
    } /* if */

  strcpy(nt->val, Tcl_DStringValue(&ds));

  /* append tag to root object */
  ay_status = ay_tags_append(ay_root, nt);

  /* clean up */
  Tcl_DStringFree(&ds);

 return;
} /* ay_rrib_RiDisplay */


RtVoid
ay_rrib_RiErrorHandler(RtErrorHandler handler)
{
   (void)handler;
} /* ay_rrib_RiErrorHandler */


RtVoid
ay_rrib_RiExposure(RtFloat gain, RtFloat gamma)
{
 ay_riopt_object *riopt = NULL;
 ay_root_object *root = NULL;

  root = (ay_root_object *)ay_root->refine;
  riopt = root->riopt;

  riopt->ExpGain = (double)gain;
  riopt->ExpGamma = (double)gamma;

 return;
} /* ay_rrib_RiExposure */


RtVoid
ay_rrib_RiExterior(RtToken name,
		   RtInt n, RtToken tokens[], RtPointer parms[])
{
 int ay_status = AY_OK;

  if(ay_rrib_cattributes->eshader)
    ay_status = ay_shader_free(ay_rrib_cattributes->eshader);
  ay_rrib_cattributes->eshader = NULL;

  ay_rrib_readshader(name, AY_STEXTERIOR, n, tokens, parms,
		     &(ay_rrib_cattributes->eshader));

 return;
} /* ay_rrib_RiExterior */


RtVoid
ay_rrib_RiFormat(RtInt xres, RtInt yres, RtFloat aspect)
{
 ay_riopt_object *riopt = NULL;
 ay_root_object *root = NULL;

  root = (ay_root_object *)ay_root->refine;
  riopt = root->riopt;

  riopt->width = (int)xres;
  riopt->height = (int)yres;

 return;
} /* ay_rrib_RiFormat */


RtVoid
ay_rrib_RiFrameAspectRatio(RtFloat aspect)
{
  (void)aspect; 
} /* ay_rrib_RiFrameAspectRatio */


RtVoid
ay_rrib_RiFrameBegin(RtInt frame)
{

  ay_rrib_pushattribs();

  if((!ay_rrib_readpartial) && (ay_rrib_readframe != -1))
    {
      if(frame == ay_rrib_readframe)
	{
	  ay_rrib_cframe = frame;
	  ay_rrib_initgprims();
	  ay_rrib_initgeneral();
	}
      else
	{
	  ay_rrib_cleargprims();
	  ay_rrib_cleargeneral();
	} /* if */
    } /* if */

 return;
} /* ay_rrib_RiFrameBegin */


RtVoid
ay_rrib_RiFrameEnd( void )
{

  ay_rrib_popattribs();

  if(ay_rrib_readframe != -1 || !ay_rrib_readpartial)
    {
      if(ay_rrib_cframe == ay_rrib_readframe)
	{
	  ay_rrib_cleargprims();
	  ay_rrib_cleargeneral();
	}
    }

 return;
} /* ay_rrib_RiFrameEnd */


RtVoid
ay_rrib_RiGeometricApproximation(RtToken type, RtFloat value)
{
   (void)type; (void)value;
} /* ay_rrib_RiGeometricApproximation */


RtVoid
ay_rrib_RiGeometricRepresentation(RtToken type)
{
   (void)type;
} /* ay_rrib_RiGeometricRepresentation */


RtVoid
ay_rrib_RiGeometry(RtToken type,
		   RtInt n, RtToken tokens[], RtPointer parms[])
{
   (void)type; (void)n; (void)tokens; (void)parms;
} /* ay_rrib_RiGeometry */


RtVoid
ay_rrib_RiHider(RtToken type,
		RtInt n, RtToken tokens[], RtPointer parms[])
{
 int ay_status = AY_OK;
 ay_tag_object *nt = NULL;
 char tname[] = "RiHider";
 Tcl_DString ds;

  if(!(nt = calloc(1, sizeof(ay_tag_object))))
    {
      return;
    }

  if(!(nt->name = calloc(strlen(tname)+1, sizeof(char))))
    {
      free(nt); return;
    }

  strcpy(nt->name, tname);
  nt->type = ay_rihider_tagtype;
  Tcl_DStringInit(&ds);
  Tcl_DStringAppend(&ds, type, -1);

  /* parse parameters, if there are any */
  if(n > 0)
    {
      ay_rrib_readparams(n, tokens, parms, &ds);

      if(!(nt->val = calloc(strlen(Tcl_DStringValue(&ds))+1, sizeof(char))))
	{
	  free(nt->name);
	  free(nt);
	  Tcl_DStringFree(&ds);
	  return;
	}
      strcpy(nt->val, Tcl_DStringValue(&ds));
    } /* if */

  /* append tag to root object */
  ay_status = ay_tags_append(ay_root, nt);

  /* clean up */
  Tcl_DStringFree(&ds);

 return;
} /* ay_rrib_RiHider */


RtVoid
ay_rrib_RiIdentity(void)
{
 int i;

  for(i = 0; i < 16; i++)
    {
      ay_rrib_ctrafos->m[i] = 0.0;
    }

  ay_rrib_ctrafos->m[0] = 1.0;
  ay_rrib_ctrafos->m[5] = 1.0;
  ay_rrib_ctrafos->m[10] = 1.0;
  ay_rrib_ctrafos->m[15] = 1.0;

 return;
} /* ay_rrib_RiIdentity */


RtVoid
ay_rrib_RiIlluminate(RtLightHandle light, RtBoolean onoff)
{
 ay_object *o = NULL;
 ay_light_object *l = NULL;
 int i = 0;

  o = ay_rrib_flobject;
  while(o->next && i < (int)light)
    {
      if(o->type == AY_IDLIGHT)
	{
	  l = (ay_light_object *)o->refine;
	  if(onoff)
	    {
	      l->on = AY_TRUE;
	    }
	  else
	    {
	      l->on = AY_FALSE;
	    } /* if */
	  i++;
	} /* if */
      o = o->next;
    } /* while */

 return;
} /* ay_rrib_RiIlluminate */


RtVoid
ay_rrib_RiImager(RtToken name,
		 RtInt n, RtToken tokens[], RtPointer parms[])
{
 int ay_status = AY_OK;
 ay_root_object *root = NULL;

  root = (ay_root_object *)ay_root->refine;

  if(root->imager)
    ay_status = ay_shader_free(root->imager);
  root->imager = NULL;

  ay_rrib_readshader(name, AY_STIMAGER, n, tokens, parms,
		     &(root->imager));

 return;
} /* ay_rrib_RiImager */


RtVoid
ay_rrib_RiImplicit(RtInt a, RtInt b[], RtInt c, RtFloat d[],
		   RtInt e, RtFloat f[], RtInt g, RtFloat h[],
		   RtInt n, RtToken tokens[], RtPointer parms[])
{
   (void)a; (void)b; (void)c; (void)d; (void)e; (void)f; (void)g; (void)h;
   (void)n; (void)tokens; (void)parms;
} /* ay_rrib_RiImplicit */


RtVoid
ay_rrib_RiInterior(RtToken name, 
		   RtInt n, RtToken tokens[], RtPointer parms[])
{
 int ay_status = AY_OK;

  if(ay_rrib_cattributes->ishader)
    ay_status = ay_shader_free(ay_rrib_cattributes->ishader);
  ay_rrib_cattributes->ishader = NULL;

  ay_rrib_readshader(name, AY_STINTERIOR, n, tokens, parms,
		     &(ay_rrib_cattributes->ishader));

 return;
} /* ay_rrib_RiInterior */


RtVoid
ay_rrib_RiMakeBump(char *picturename, char *texturename,
		   RtToken swrap, RtToken twrap,
		   RtFilterFunc filterfunc, RtFloat swidth, RtFloat twidth,
		   RtInt n, RtToken tokens[], RtPointer parms[])
{
   (void)picturename; (void)texturename; 
   (void)swrap; (void)twrap; 
   (void)filterfunc; (void)swidth; (void)twidth; 
   (void)n; (void)tokens; (void)parms;
} /* ay_rrib_RiMakeBump */


RtVoid
ay_rrib_RiMakeCubeFaceEnvironment(char *px, char *nx, char *py, char *ny,
				  char *pz, char *nz, char *texturename,
				  RtFloat fov,
				  RtFilterFunc filterfunc,
				  RtFloat swidth, RtFloat twidth,
				  RtInt n,
				  RtToken tokens[], RtPointer parms[])
{
   (void)px; (void)nx; (void)py; (void)ny; 
   (void)pz; (void)nz; (void)texturename; 
   (void)fov; 
   (void)filterfunc; 
   (void)swidth; (void)twidth; 
   (void)n; (void)tokens; (void)parms;
} /* ay_rrib_RiMakeCubeFaceEnvironment */


RtVoid
ay_rrib_RiMakeLatLongEnvironment(char *picturename, char *texturename,
				 RtFilterFunc filterfunc,
				 RtFloat swidth, RtFloat twidth,
				 RtInt n,
				 RtToken tokens[], RtPointer parms[])
{
   (void)picturename; (void)texturename; 
   (void)filterfunc; 
   (void)swidth; (void)twidth; 
   (void)n; (void)tokens; (void)parms;
} /* ay_rrib_RiMakeLatLongEnvironment */


RtVoid
ay_rrib_RiMakeShadow(char *picturename, char *texturename,
		     RtInt n, RtToken tokens[], RtPointer parms[])
{
   (void)picturename; (void)texturename; 
   (void)n; (void)tokens; (void)parms;
} /* ay_rrib_RiMakeShadow */


RtVoid
ay_rrib_RiMakeTexture(char *picturename, char *texturename,
		      RtToken swrap, RtToken twrap,
		      RtFilterFunc filterfunc,
		      RtFloat swidth, RtFloat twidth,
		      RtInt n, RtToken tokens[], RtPointer parms[])
{
   (void)picturename; (void)texturename; 
   (void)swrap; (void)twrap; 
   (void)filterfunc; 
   (void)swidth; (void)twidth; 
   (void)n; (void)tokens; (void)parms;
} /* ay_rrib_RiMakeTexture */


RtVoid
ay_rrib_RiMatte(RtBoolean onoff)
{

  if(onoff)
    {
      ay_rrib_cattributes->matte = AY_TRUE;
    }
  else
    {
      ay_rrib_cattributes->matte = AY_FALSE;
    }

 return;
} /* ay_rrib_RiMatte */


RtVoid
ay_rrib_RiMotionBegin(RtInt n, RtFloat times[])
{
   (void)n; (void)times;
} /* ay_rrib_RiMotionBegin */


RtVoid ay_rrib_RiMotionEnd(void)
{

} /* ay_rrib_RiMotionEnd */


RtVoid
ay_rrib_RiNuCurves(RtInt ncurves, RtInt nvertices[], RtInt order[],
		   RtFloat knot[], RtFloat min[], RtFloat max[], 
		   RtInt n, RtToken tokens[], RtPointer parms[])
{
   (void)ncurves; (void)nvertices; (void)order; (void)knot; 
   (void)min; (void)max; 
   (void)n; (void)tokens; (void)parms;
} /* ay_rrib_RiNuCurves */



RtObjectHandle
ay_rrib_RiObjectBegin(void)
{
 ay_list_object *new = NULL;
 char fname[] = "ay_rrib_RiObjectBegin";

  if(!(new = calloc(1, sizeof(ay_list_object))))
    {
      ay_error(AY_EOMEM, fname, NULL);
      return((RtObjectHandle)(ay_rrib_cobjecthandle++));
    }

  if(ay_rrib_lastobject)
    {
      ay_rrib_lastobject->next = new;
    }
  else
    {
      ay_rrib_objects = new;
    }

  ay_rrib_lastobject = new;

  ay_rrib_aynext = ay_next;
  ay_next = &(new->object);

  ay_rrib_pushtrafos();
  ay_rrib_RiIdentity();
  ay_rrib_pushattribs();

 return((RtObjectHandle)(ay_rrib_cobjecthandle++));
} /* ay_rrib_RiObjectBegin */


RtVoid
ay_rrib_RiObjectEnd(void)
{

  /* stop linking read objects to object handle */
  ay_next = ay_rrib_aynext;

  ay_rrib_poptrafos();
  ay_rrib_popattribs();

 return;
} /* ay_rrib_RiObjectEnd */


RtVoid
ay_rrib_RiObjectInstance(RtObjectHandle handle)
{
 int ay_status = AY_OK;
 ay_list_object *l = NULL;
 ay_object *o = NULL, *c = NULL;
 char fname[] = "ay_rrib_RiObjectInstance";
 int i = 1, j = 0;
 double m[16], mt[16];
 double quat[4];
 double axis[3];

  if((int)handle > ay_rrib_cobjecthandle)
    {
      ay_error(AY_ERROR, fname, "undefined object handle");
      return;
    }

  l = ay_rrib_objects;
  while(l->next && (i < (int)handle))
    {
      i++;
      l = l->next;
    } /* while */

  /* copy objects from object handle to scene */
  /* XXXX this could be smarter by not copying all the time
     but creating instances for the second and next calls to
     ObjectInstance; however, problematic is:
     a) how to distinguish between first and next calls
     b) how to deal with multiple objects in one object handle
     c) how to remember where the instances should point to
  */
  if((i == (int)handle) && l)
    {
      o = l->object;
      while(o)
	{
	  c = NULL;
	  ay_status = ay_object_copy(o, &c);
	  if(!ay_status)
	    {
	      /* XXXX should we rather concatenate the current transformations
		 to the transformations of the objects in object handle? */

	      ay_rrib_trafotoobject(c, ay_rrib_ctrafos->m);
	      /*
	      for(j = 0; j < 16; j++)
		{
		  m[j] = 0.0;
		}
	      m[0] = o->scalx;
	      m[5] = o->scaly;
	      m[10] = o->scalz;
	      m[15] = 1.0;

	      m[3] = o->movx;
	      m[7] = o->movy;
	      m[11] = o->movz;

	      if(fabs(o->rotx) > AY_EPSILON)
		{
		  axis[0] = 1.0;
		  axis[1] = 0.0;
		  axis[2] = 0.0;
		  ay_quat_axistoquat(axis, AY_D2R(o->rotx), quat);
		  ay_quat_torotmatrix(quat, mt);
		  ay_trafo_multmatrix4(m, mt);
		}
	      if(fabs(o->roty) > AY_EPSILON)
		{
		  axis[0] = 0.0;
		  axis[1] = 1.0;
		  axis[2] = 0.0;
		  ay_quat_axistoquat(axis, AY_D2R(o->roty), quat);
		  ay_quat_torotmatrix(quat, mt);
		  ay_trafo_multmatrix4(m, mt);
		}
	      if(fabs(o->rotz) > AY_EPSILON)
		{
		  axis[0] = 0.0;
		  axis[1] = 0.0;
		  axis[2] = 1.0;
		  ay_quat_axistoquat(axis, AY_D2R(o->rotz), quat);
		  ay_quat_torotmatrix(quat, mt);
		  ay_trafo_multmatrix4(m, mt);
		}
		ay_trafo_multmatrix4(m, ay_rrib_ctrafos->m);
		ay_rrib_trafotoobject(c, m);
	      */
	      ay_object_link(c);
	    } /* if */
	  o = o->next;
	} /* while */
    } /* if */

 return;
} /* ay_rrib_RiObjectInstance */


RtVoid
ay_rrib_RiOpacity(RtColor color)
{

  ay_rrib_cattributes->opr = (int)(color[0]*255);
  ay_rrib_cattributes->opg = (int)(color[1]*255);
  ay_rrib_cattributes->opb = (int)(color[2]*255);

 return;
} /* ay_rrib_RiOpacity */


RtVoid
ay_rrib_RiOption(RtToken name,
		 RtInt n, RtToken tokens[], RtPointer parms[])
{
 int i, option_handled = AY_FALSE;
 char fname[] = "ay_rrib_RiOption";
 ay_riopt_object *riopt = NULL;
 ay_root_object *root = NULL;
 char *stemp;

  root = (ay_root_object *)ay_root->refine;
  riopt = root->riopt;

  if(!strcmp(name,"limits"))
    {
      for(i = 0; i < n; i++)
	{

	  if(!strcmp(tokens[i], "texturememory"))
	    {
	      riopt->texturemem = (int)(*((RtInt *)(parms[i])));
	      option_handled = AY_TRUE;
	    }
	  if(!strcmp(tokens[i], "geommemory"))
	    {
	      riopt->geommem = (int)(*((RtInt *)(parms[i])));
	      option_handled = AY_TRUE;
	    }
	  if(!option_handled)
	    {
	      ay_rrib_readtag(ay_riopt_tagtype, "RiOption", name,
			      i, tokens, parms, &(ay_root->tags));
	    }
	} /* for */
      return;
    } /* if */

  if(!strcmp(name,"radiosity"))
    {
      for(i = 0; i < n; i++)
	{

	  if(!strcmp(tokens[i], "steps"))
	    {
	      riopt->RadSteps = (int)(*((RtInt *)(parms[i])));
	      option_handled = AY_TRUE;
	    }
	  if(!strcmp(tokens[i], "minpatchsamples"))
	    {
	      riopt->PatchSamples = (int)(*((RtInt *)(parms[i])));
	      option_handled = AY_TRUE;
	    }
	  if(!option_handled)
	    {
	      ay_rrib_readtag(ay_riopt_tagtype, "RiOption", name,
			      i, tokens, parms, &(ay_root->tags));
	    }
	} /* for */
      return;
    } /* if */

  if(!strcmp(name,"render"))
    {
      for(i = 0; i < n; i++)
	{
	  if(!strcmp(tokens[i], "prmanspecular"))
	    {
	      riopt->PRManSpec = (char)(*((RtInt *)(parms[i])));
	      option_handled = AY_TRUE;
	    }
	  if(!strcmp(tokens[i], "minshadowbias"))
	    {
	      riopt->ShadowBias = (double)(*((RtFloat *)(parms[i])));
	      option_handled = AY_TRUE;
	    }
	  if(!strcmp(tokens[i], "max_raylevel"))
	    {
	      riopt->MaxRayLevel = (int)(*((RtInt *)(parms[i])));
	      option_handled = AY_TRUE;
	    }
	  if(!strcmp(tokens[i], "minsamples"))
	    {
	      riopt->MinSamples = (int)(*((RtInt *)(parms[i])));
	      option_handled = AY_TRUE;
	    }
	  if(!strcmp(tokens[i], "maxsamples"))
	    {
	      riopt->MaxSamples = (int)(*((RtInt *)(parms[i])));
	      option_handled = AY_TRUE;
	    }
	  if(!option_handled)
	    {
	      ay_rrib_readtag(ay_riopt_tagtype, "RiOption", name,
			      i, tokens, parms, &(ay_root->tags));
	    }
	} /* for */
      return;
    } /* if */

  if(!strcmp(name,"searchpath"))
    {
      for(i = 0; i < n; i++)
	{

	  if(!strcmp(tokens[i], "shader"))
	    {
	      stemp = *((char **)(parms[i]));
	      if(riopt->shaders)
		free(riopt->shaders);
	      riopt->shaders = NULL;
	      if(!(riopt->shaders = calloc(strlen(stemp)+1,
					   sizeof(char))))
		{
		  ay_error(AY_EOMEM, fname, NULL);
		  return;
		}
	      strcpy(riopt->shaders, stemp);
	      option_handled = AY_TRUE;
	    }
	  if(!strcmp(tokens[i], "include"))
	    {
	      stemp = *((char **)(parms[i]));
	      if(riopt->includes)
		free(riopt->includes);
	      riopt->includes = NULL;
	      if(!(riopt->includes = calloc(strlen(stemp)+1,
					   sizeof(char))))
		{
		  ay_error(AY_EOMEM, fname, NULL);
		  return;
		}
	      strcpy(riopt->includes, stemp);
	      option_handled = AY_TRUE;
	    }
	  if(!strcmp(tokens[i], "texture"))
	    {
	      stemp = *((char **)(parms[i]));
	      if(riopt->textures)
		free(riopt->textures);
	      riopt->textures = NULL;
	      if(!(riopt->textures = calloc(strlen(stemp)+1,
					   sizeof(char))))
		{
		  ay_error(AY_EOMEM, fname, NULL);
		  return;
		}
	      strcpy(riopt->textures, stemp);
	      option_handled = AY_TRUE;
	    }
	} /* for */
      return;
    } /* if */

  for(i = 0; i < n; i++)
    {
      ay_rrib_readtag(ay_riopt_tagtype, "RiOption", name,
		      i, tokens, parms, &(ay_root->tags));
    }

 return;
} /* ay_rrib_RiOption */


RtVoid
ay_rrib_RiOrientation(RtToken orientation)
{
 return;
} /* ay_rrib_RiOrientation */


RtVoid
ay_rrib_RiPatch(RtToken type, 
		RtInt n, RtToken tokens[], RtPointer parms[])
{
 ay_bpatch_object bp;
 int i = 0, stride = 4;
 RtPointer tokensfound[PPWTBL_LAST];
 RtFloat *pw = NULL;

 if(!strcmp(type, RI_BICUBIC))
   {
     ay_rrib_RiPatchMesh(RI_BICUBIC, 4, (RtToken)"no",
			 4, (RtToken)"no", n, tokens, parms);
     return;
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
	  return;
	}
    } /* if */

  /* XXXX divide by w? */

  i = 0;
  bp.p1[0] = pw[i];
  bp.p1[1] = pw[i+1];
  bp.p1[2] = pw[i+2];
  i += stride;

  bp.p2[0] = pw[i];
  bp.p2[1] = pw[i+1];
  bp.p2[2] = pw[i+2];
  i += stride;

  bp.p4[0] = pw[i];
  bp.p4[1] = pw[i+1];
  bp.p4[2] = pw[i+2];
  i += stride;

  bp.p3[0] = pw[i];
  bp.p3[1] = pw[i+1];
  bp.p3[2] = pw[i+2];


  ay_rrib_linkobject((void *)(&bp), AY_IDBPATCH);

 return;
} /* ay_rrib_RiPatch */


RtVoid
ay_rrib_RiPatchMesh(RtToken type, RtInt nu, RtToken uwrap,
		    RtInt nv, RtToken vwrap,
		    RtInt n, RtToken tokens[], RtPointer parms[])
{
 int ay_status = AY_OK;
 ay_pamesh_object pm;
 int i = 0, j = 0, stride = 4, detail = 0;
 double *p = NULL;
 RtPointer tokensfound[PPWTBL_LAST];
 RtFloat *pp = NULL, *pw = NULL;

  memset(&pm, '\0', sizeof(ay_pamesh_object));
  pm.glu_sampling_tolerance = 0.0;
  pm.glu_display_mode = 0;
  pm.npatch = NULL;

  if(!strcmp(type, RI_BILINEAR))
    pm.type = AY_PTBILINEAR;
  else
    pm.type = AY_PTBICUBIC;

  pm.width = (int)nu;
  if(!strcmp(uwrap, RI_PERIODIC))
    pm.close_u = AY_TRUE;
  else
    pm.close_u = AY_FALSE;

  pm.height = (int)nv;
  if(!strcmp(vwrap, RI_PERIODIC))
    pm.close_v = AY_TRUE;
  else
    pm.close_v = AY_FALSE;

  pm.btype_u = ay_rrib_cattributes->btype_u;
  pm.btype_v = ay_rrib_cattributes->btype_v;

  pm.ustep = ay_rrib_cattributes->ustep;
  pm.vstep = ay_rrib_cattributes->vstep;

  pm.ubasis = NULL;
  pm.vbasis = NULL;

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
	  if(pm.ubasis)
	    free(pm.ubasis);
	  if(pm.vbasis)
	    free(pm.vbasis);
	  return;
	} /* if */
    } /* if */
  
  if(!(pm.controlv = calloc(nu*nv*4, sizeof(double))))
    {
      if(pm.ubasis)
	free(pm.ubasis);
      if(pm.vbasis)
	free(pm.vbasis);
      return;
    } /* if */

  pp = pw;
  for(i = 0; i < nv; i++)
    {
      p = &(pm.controlv[i*4]);
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

    if(!ay_pmt_valid(&pm, &detail))
      {
	ay_status = ay_pmt_tonpatch(&pm, &(pm.npatch));
      }

  ay_rrib_linkobject((void *)(&pm), AY_IDPAMESH);

  ay_object_deletemulti(pm.npatch);
  pm.npatch = NULL;

  free(pm.controlv);
  pm.controlv = NULL;

 return;
} /* ay_rrib_RiPatchMesh */


RtVoid
ay_rrib_RiPerspective(RtFloat fov)
{
   ay_rrib_fov = (double)fov;
   ay_rrib_RiIdentity();
 return;
} /* ay_rrib_RiPerspective */


RtVoid
ay_rrib_RiPixelFilter(RtFilterFunc filterfunc, 
		      RtFloat xwidth, RtFloat ywidth)
{
 ay_riopt_object *riopt = NULL;
 ay_root_object *root = NULL;

  root = (ay_root_object *)ay_root->refine;
  riopt = root->riopt;

  riopt->FilterWidth = (double)xwidth;
  riopt->FilterHeight = (double)ywidth;

 return;
} /* ay_rrib_RiPixelFilter */


RtVoid
ay_rrib_RiPixelSamples(RtFloat xsamples, RtFloat ysamples)
{
 ay_riopt_object *riopt = NULL;
 ay_root_object *root = NULL;

  root = (ay_root_object *)ay_root->refine;
  riopt = root->riopt;

  riopt->Samples_X = (double)xsamples;
  riopt->Samples_Y = (double)ysamples;

 return;
} /* ay_rrib_RiPixelSamples */


RtVoid
ay_rrib_RiPixelVariance(RtFloat variation)
{
 ay_riopt_object *riopt = NULL;
 ay_root_object *root = NULL;

  root = (ay_root_object *)ay_root->refine;
  riopt = root->riopt;

  riopt->Variance = (double)variation;

 return;
} /* ay_rrib_RiPixelVariance */


RtVoid
ay_rrib_RiPoints(RtInt npoints, 
		 RtInt n, RtToken tokens[], RtPointer parms[])
{
   (void)npoints;
   (void)n; (void)tokens; (void)parms; 
} /* ay_rrib_RiPoints */


RtVoid
ay_rrib_RiPointsGeneralPolygons(RtInt npolys, RtInt nloops[],
				RtInt nvertices[], RtInt vertices[],
				RtInt n, RtToken tokens[], RtPointer parms[])
{
 ay_pomesh_object pm = {0};
 int i = 0, a = 0, b = 0, stride = 0;
 unsigned int total_loops = 0, total_verts = 0, nc_needed = 0;
 RtPointer tokensfound[PPWTBL_LAST];
 RtPointer ntokensfound[NTBL_LAST];
 RtFloat *pp = NULL, *pw = NULL;
 double *normalv = NULL;

  memset(&pm, '\0', sizeof(ay_pomesh_object));
  pm.npolys = npolys;

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
	  return;
	} /* if */
    } /* if */

  if(!(pm.nloops = calloc(npolys, sizeof(unsigned int))))
    return;
  for(i = 0; i < npolys; i++)
    {
      pm.nloops[i] = (unsigned int) nloops[i];
      total_loops += nloops[i];
    } /* for */

  if(!(pm.nverts = calloc(total_loops, sizeof(unsigned int))))
    { free(pm.nloops); return; }
  for(i = 0; i < total_loops; i++)
    {
      pm.nverts[i] = (unsigned int) nvertices[i];
      total_verts += nvertices[i];
    } /* for */

  if(!(pm.verts = calloc(total_verts, sizeof(unsigned int))))
    { free(pm.nloops); free(pm.nverts); return; }
  for(i = 0; i < total_verts; i++)
    {
      pm.verts[i] = (unsigned int) vertices[i];
      /* get biggest vertice index, this is the number of controlvs we need */
      if(nc_needed < (unsigned int)(vertices[i]))
	{
	  nc_needed = (unsigned int)(vertices[i]);
	}
    } /* for */
  nc_needed++;

  if(!(pm.controlv = calloc(nc_needed*3, sizeof(double))))
    { free(pm.nloops); free(pm.nverts); free(pm.verts); return; }
  a = 0;
  pp = pw;
  for(i = 0; i < nc_needed; i++)
    {
      pm.controlv[a]   = (double)(pp[0]);
      pm.controlv[a+1] = (double)(pp[1]);
      pm.controlv[a+2] = (double)(pp[2]);
      a += 3;
      pp += stride;
    } /* for */
  pm.ncontrols = nc_needed;

  RibGetUserParameters(N, NTBL_LAST, n, tokens, parms, ntokensfound);
  if(ntokensfound[NTBL_N])
    {
      pm.has_normals = AY_TRUE;
      if(!(normalv = calloc(nc_needed*6, sizeof(double))))
	{
	  free(pm.nloops); free(pm.nverts); free(pm.verts); free(pm.controlv);
	  return;
	} /* if */
      pw = (RtFloat*)ntokensfound[NTBL_N];
      pp = pw;
      a = 0; b = 0;
      for(i = 0; i < nc_needed; i++)
	{
	  normalv[a]   = pm.controlv[b];
	  normalv[a+1] = pm.controlv[b+1];
	  normalv[a+2] = pm.controlv[b+2];
	  normalv[a+3] = (double)(pp[0]);
	  normalv[a+4] = (double)(pp[1]);
	  normalv[a+5] = (double)(pp[2]);

	  a += 6;
	  b += 3;
	  pp += 3;
	} /* for */

      free(pm.controlv);
      pm.controlv = normalv;

    } /* if */

  /* now link the object to the scene */
  ay_rrib_linkobject((void *)(&pm), AY_IDPOMESH);

  free(pm.nloops);
  free(pm.nverts);
  free(pm.verts);
  free(pm.controlv);

 return;
} /* ay_rrib_RiPointsGeneralPolygons */


RtVoid
ay_rrib_RiGeneralPolygon(RtInt nloops, RtInt nvertices[],
			 RtInt n, RtToken tokens[], RtPointer parms[])
{
 RtInt *vertices = NULL, total_verts = 0;
 int i;

  for(i = 0; i < nloops; i++)
    {
      total_verts += nvertices[i];
    }

  if(!(vertices = calloc(total_verts, sizeof(RtInt))))
    return;

  for(i = 0; i < total_verts; i++)
    {
      vertices[i] = i;
    }

  ay_rrib_RiPointsGeneralPolygons(1, &nloops, nvertices, vertices,
				  n, tokens, parms);

  /* clean up */
  free(vertices);

 return;
} /* ay_rrib_RiGeneralPolygon */


RtVoid
ay_rrib_RiPointsPolygons(RtInt npolys, RtInt nvertices[], RtInt vertices[],
			 RtInt n, RtToken tokens[], RtPointer parms[])
{
 RtInt *nloops = NULL;
 int i;

  if(!(nloops = calloc(npolys, sizeof(RtInt))))
    return;

  for(i = 0; i < npolys; i++)
    {
      nloops[i] = 1;
    }

  ay_rrib_RiPointsGeneralPolygons(npolys, nloops, nvertices, vertices,
				  n, tokens, parms);

  /* clean up */
  free(nloops);

 return;
} /* ay_rrib_RiPointsPolygons */


RtVoid
ay_rrib_RiPolygon(RtInt nvertices,
		  RtInt n, RtToken tokens[], RtPointer parms[])
{
 RtInt *vertices = NULL;
 int i, j = 1;

  if(!(vertices = calloc(nvertices, sizeof(RtInt))))
    return;

  for(i = 0; i < nvertices; i++)
    {
      vertices[i] = i;
    }

  ay_rrib_RiPointsGeneralPolygons(1, &j, &nvertices, vertices,
				  n, tokens, parms);

  /* clean up */
  free(vertices);

 return;
} /* ay_rrib_RiPolygon */


RtVoid
ay_rrib_RiProjection(RtToken name, 
		     RtInt n, RtToken tokens[], RtPointer parms[])
{

  if(!strcmp(name, "perspective"))
    {
      /* perspective projection */
      ay_rrib_fov = 90.0;
      /* get fov (if specified) */
      if(n > 0)
	{
	  if(!strcmp(tokens[0], "fov"))
	    ay_rrib_fov = (double)(*((RtFloat*)(parms[0])));
	}
    }
  else
    {
      /* assume parallel projection */
    }

  /* start reading camera transformations, thus set current trafos to
     default values */
  ay_rrib_RiIdentity();

 return;
} /* ay_rrib_RiProjection */


RtVoid
ay_rrib_RiQuantize(RtToken type, RtInt one, 
		   RtInt min, RtInt max, RtFloat ampl)
{
 ay_riopt_object *riopt = NULL;
 ay_root_object *root = NULL;

  root = (ay_root_object *)ay_root->refine;
  riopt = root->riopt;

  /* XXXX type? */

  riopt->RGBA_ONE = (double)one;
  riopt->RGBA_MIN = (double)min;
  riopt->RGBA_MAX = (double)max;
  riopt->RGBA_Dither = (double)ampl;

 return;
} /* ay_rrib_RiQuantize */


RtVoid
ay_rrib_RiReadArchive(RtToken name,
		      RtVoid (*callback)( RtToken, char*, char* ),
		      RtInt n, RtToken tokens[], RtPointer parms[])
{
 RIB_HANDLE rib = NULL, oldgrib;
 char fname[] = "ay_rrib_RiReadArchive";

  oldgrib = grib;

  rib = RibOpenSubfile(grib, name);

  if(rib)
    {
      grib = (PRIB_INSTANCE)rib;      

      RibRead(rib);

      RibClose(rib);
    }
  else
    {
      ay_error(AY_EOPENFILE, fname, name);
    } /* if */

  grib = oldgrib;

 return;
} /* ay_rrib_RiReadArchive */


RtVoid
ay_rrib_RiRelativeDetail(RtFloat relativedetail)
{
 return;
} /* ay_rrib_RiRelativeDetail */


RtVoid
ay_rrib_RiResource(RtToken handle, RtToken type,
		   RtInt n, RtToken tokens[], RtPointer parms[])
{
 return;
} /* ay_rrib_RiResource */


RtVoid
ay_rrib_RiReverseOrientation(void)
{
 return;
} /* ay_rrib_RiReverseOrientation */


RtVoid
ay_rrib_RiConcatTransform(RtMatrix transform)
{
 int i, j, k;
 double m[16];

 k = 0;
 for(i = 0; i < 4; i++)
   {
     for(j = 0; j < 4; j++)
       {
	 m[k] = (double)transform[i][j];
	 k++;
       }
   }

  ay_trafo_multmatrix4(ay_rrib_ctrafos->m, m);

 return;
} /* ay_rrib_RiConcatTransform */


RtVoid
ay_rrib_RiRotate(RtFloat angle, RtFloat dx, RtFloat dy, RtFloat dz)
{
 double quat[4];
 double axis[3];
 double m[16];

  axis[0] = (double)dx;
  axis[1] = (double)dy;
  axis[2] = (double)dz;
  ay_quat_axistoquat(axis, -AY_D2R((double)angle), quat);
  ay_quat_torotmatrix(quat, m);
  ay_trafo_multmatrix4(ay_rrib_ctrafos->m, m);

 return;
} /* ay_rrib_RiRotate */


RtVoid
ay_rrib_RiScale(RtFloat dx, RtFloat dy, RtFloat dz)
{
 double m[16] = {0};

  m[0] = (double)dx;
  m[5] = (double)dy;
  m[10] = (double)dz;
  m[15] = 1.0;

  ay_trafo_multmatrix4(ay_rrib_ctrafos->m, m);

 return;
} /* ay_rrib_RiScale */


RtVoid
ay_rrib_RiTransform(RtMatrix transform)
{
 int i, j, k;

  k = 0;
  for(i=0;i<4;i++)
    {
      for(j=0;j<4;j++)
	{
	  ay_rrib_ctrafos->m[k] = transform[i][j];
	  k++;
	}
    }

 return;
} /* ay_rrib_RiTransform */


RtVoid
ay_rrib_RiTranslate(RtFloat dx, RtFloat dy, RtFloat dz)
{
 double m[16] = {0};
 
  m[0] = 1.0;
  m[5] = 1.0;
  m[10] = 1.0;
  m[15] = 1.0;
  /*
  m[3] = (double)dx;
  m[7] = (double)dy;
  m[11] = (double)dz;
  */
  m[12] = (double)dx;
  m[13] = (double)dy;
  m[14] = (double)dz;

  ay_trafo_multmatrix4(ay_rrib_ctrafos->m, m);

 return;
} /* ay_rrib_RiTranslate */


RtVoid
ay_rrib_RiScreenWindow(RtFloat left, RtFloat right,
		       RtFloat bottom, RtFloat top)
{
 return;
} /* ay_rrib_RiScreenWindow */


RtVoid
ay_rrib_RiShadingInterpolation(RtToken type)
{

  if(!strcmp(type, "smooth"))
    ay_rrib_cattributes->shading_interpolation = 1;
  else
    ay_rrib_cattributes->shading_interpolation = 0;

 return;
} /* ay_rrib_RiShadingInterpolation */


RtVoid
ay_rrib_RiShadingRate(RtFloat size)
{

  ay_rrib_cattributes->shading_rate = (double)size;

 return;
} /* ay_rrib_RiShadingRate */


RtVoid
ay_rrib_RiShutter(RtFloat min, RtFloat max)
{
 return;
} /* ay_rrib_RiShutter */


RtVoid
ay_rrib_RiSides(RtInt sides)
{

  ay_rrib_cattributes->sides = (int)sides;

 return;
} /* ay_rrib_RiSides */


RtVoid
ay_rrib_RiSkew(RtFloat angle, RtFloat dx1, RtFloat dy1, RtFloat dz1,
	       RtFloat dx2, RtFloat dy2, RtFloat dz2)
{
   (void)angle; 
   (void)dx1; (void)dy1; (void)dz1; (void)dx2; (void)dy2; (void)dz2;
} /* ay_rrib_RiSkew */


RtVoid
ay_rrib_RiSolidBegin(RtToken operation)
{
 ay_level_object l;

  l.type = AY_LTLEVEL;

  if(!strcmp(operation,"primitive"))
    l.type = AY_LTPRIM;

  if(!strcmp(operation,"union"))
    l.type = AY_LTUNION;

  if(!strcmp(operation,"difference"))
    l.type = AY_LTDIFF;

  if(!strcmp(operation,"intersection"))
    l.type = AY_LTINT;
 
  ay_rrib_co.parent = AY_TRUE;
  ay_rrib_linkobject((void *)(&l), AY_IDLEVEL);
  ay_rrib_co.parent = AY_FALSE;
  ay_object_delete(ay_rrib_co.down);
  ay_rrib_co.down = NULL;

  ay_clevel_add(ay_rrib_lrobject);
  ay_clevel_add(ay_rrib_lrobject->down);
  ay_next = &(ay_rrib_lrobject->down);

  ay_rrib_pushtrafos();
  ay_rrib_RiIdentity();

 return;
} /* ay_rrib_RiSolidBegin */


RtVoid
ay_rrib_RiSolidEnd(void)
{

  ay_clevel_del();
  ay_next = &(ay_currentlevel->object->next);
  ay_clevel_del();

  ay_rrib_poptrafos();

 return;
} /* ay_rrib_RiSolidEnd */


RtVoid
ay_rrib_RiSurface(RtToken name, 
		  RtInt n, RtToken tokens[], RtPointer parms[])
{
 int ay_status = AY_OK;

  if(ay_rrib_cattributes->sshader)
    ay_status = ay_shader_free(ay_rrib_cattributes->sshader);
  ay_rrib_cattributes->sshader = NULL;

  ay_rrib_readshader(name, AY_STSURFACE, n, tokens, parms,
		     &(ay_rrib_cattributes->sshader));

 return;
} /* ay_rrib_RiSurface */


RtVoid
ay_rrib_RiSubdivisionMesh(RtToken scheme, RtInt nfaces,
			  RtInt nvertices[], RtInt vertices[],
			  RtInt ntags, RtToken tags[],
			  RtInt nargs[],
			  RtInt intargs[], RtFloat floatargs[],
			  RtInt n, RtToken tokens[], RtPointer parms[])
{
 ay_sdmesh_object sm;
 int i = 0, a = 0, stride = 0;
 unsigned int total_verts = 0, total_intargs = 0, total_floatargs = 0;
 unsigned int nc_needed = 0;
 RtPointer tokensfound[PPWTBL_LAST];
 RtFloat *pp = NULL, *pw = NULL;
 RtToken ccscheme = "catmullclark";

  memset(&sm, '\0', sizeof(ay_sdmesh_object));
  if(!(strcmp(scheme, ccscheme)))
    scheme = AY_SDSCATMULL;
  sm.nfaces = nfaces;

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
	  return;
	} /* if */
    } /* if */

  if(!(sm.nverts = calloc(nfaces, sizeof(unsigned int))))
    return;
  for(i = 0; i < nfaces; i++)
    {
      sm.nverts[i] = (unsigned int)(nvertices[i]);
      total_verts += nvertices[i];
    } /* for */

  if(!(sm.verts = calloc(total_verts, sizeof(unsigned int))))
    { free(sm.nverts); return; }
  for(i = 0; i < total_verts; i++)
    {
      sm.verts[i] = (unsigned int) vertices[i];
      /* get biggest vertice index, this is the number of controlvs we need */
      if(nc_needed < (unsigned int)(vertices[i]))
	{
	  nc_needed = (unsigned int)(vertices[i]);
	}
    } /* for */
  nc_needed++;

  if(!(sm.controlv = calloc(nc_needed*3, sizeof(double))))
    { free(sm.nverts); free(sm.verts); return; }
  a = 0;
  pp = pw;
  for(i = 0; i < nc_needed; i++)
    {
      sm.controlv[a]   = (double)(pp[0]);
      sm.controlv[a+1] = (double)(pp[1]);
      sm.controlv[a+2] = (double)(pp[2]);
      a += 3;
      pp += stride;
    } /* for */
  sm.ncontrols = nc_needed;


  /* get tags */
  sm.ntags = ntags;
  if(ntags > 0)
    {
      if(!(sm.tags = calloc(ntags, sizeof(int))))
	{ free(sm.nverts); free(sm.verts); free(sm.controlv); return;}
      for(i = 0; i < ntags; i++)
	{
	  /*
	  switch(tags[i])
	    {
	    case RI_HOLE:
	      sm.tags[i] = AY_SDTHOLE;
	      break;
	    case RI_CREASE:
	      sm.tags[i] = AY_SDTCREASE;
	      break;
	    case RI_CORNER:
	      sm.tags[i] = AY_SDTCORNER;
	      break;
	    case RI_INTERPOLATEBOUNDARY:
	      sm.tags[i] = AY_SDTIB;
	      break;
	    default:
	      break;
	    }
	  */
	  if(!(strcmp(tags[i], "hole")))
	    sm.tags[i] = AY_SDTHOLE;
	  if(!(strcmp(tags[i], "crease")))
	    sm.tags[i] = AY_SDTCREASE;
	  if(!(strcmp(tags[i], "corner")))
	    sm.tags[i] = AY_SDTCORNER;
	  if(!(strcmp(tags[i], "interpolateboundary")))
	    sm.tags[i] = AY_SDTIB;
	} /* for */

      if(!(sm.nargs = calloc(2 * ntags, sizeof(unsigned int))))
	{
	  free(sm.nverts); free(sm.verts); free(sm.controlv); free(sm.tags);
	  return;
	}
      for(i = 0; i < (2 * ntags); i++)
	{
	  sm.nargs[i] = (unsigned int)(nargs[i]);
	}
      for(i = 0; i < (2 * ntags); i += 2)
	{
	  total_intargs += (unsigned int)(nargs[i]);
	  total_floatargs += (unsigned int)(nargs[i+1]);
	}
      if(!(sm.intargs = calloc(total_intargs, sizeof(int))))
	{
	  free(sm.nverts); free(sm.verts); free(sm.controlv); free(sm.tags);
	  free(sm.nargs);
	  return;
	}
      for(i = 0; i < total_intargs; i++)
	{
	  sm.intargs[i] = (int)(intargs[i]);
	}
      if(!(sm.floatargs = calloc(total_floatargs, sizeof(double))))
	{
	  free(sm.nverts); free(sm.verts); free(sm.controlv); free(sm.tags);
	  free(sm.nargs); if(sm.intargs){free(sm.intargs);}
	  return;
	}
      for(i = 0; i < total_floatargs; i++)
	{
	  sm.floatargs[i] = (double)(floatargs[i]);
	}
    } /* if */

  /* now link the object to the scene */
  ay_rrib_linkobject((void *)(&sm), AY_IDSDMESH);

  /* clean up */
  free(sm.nverts);
  free(sm.verts);
  free(sm.controlv);
  if(ntags > 0)
    {
      free(sm.tags);
      free(sm.nargs);
      if(intargs)
	free(sm.intargs);
      if(floatargs)
	free(sm.floatargs);
    }

 return;
} /* ay_rrib_RiSubdivisionMesh */


RtVoid
ay_rrib_RiTextureCoordinates(RtFloat s1, RtFloat t1,
                             RtFloat s2, RtFloat t2,
                             RtFloat s3, RtFloat t3,
                             RtFloat s4, RtFloat t4)
{

  ay_rrib_cattributes->s1 = s1;
  ay_rrib_cattributes->s2 = s2;
  ay_rrib_cattributes->s3 = s3;
  ay_rrib_cattributes->s4 = s4;
  
  ay_rrib_cattributes->t1 = t1;
  ay_rrib_cattributes->t2 = t2;
  ay_rrib_cattributes->t3 = t3;
  ay_rrib_cattributes->t4 = t4;

 return;
} /* ay_rrib_RiTextureCoordinates */


RtVoid
ay_rrib_RiTransformBegin(void)
{

  ay_rrib_pushtrafos();

 return;
} /* ay_rrib_RiTransformBegin */


RtVoid
ay_rrib_RiTransformEnd(void)
{

  ay_rrib_poptrafos();

 return;
} /* ay_rrib_RiTransformEnd */


RtVoid
ay_rrib_RiWorldBegin(void)
{
 int ay_status = AY_OK;
 ay_camera_object c;
 double mi[16];
 char fname[] = "ay_rrib_RiWorldBegin";
 /* ay_level_object l;*/

  if(ay_rrib_readcamera)
    {
      c.from[0] = 0.0;
      c.from[1] = 0.0;
      c.from[2] = -10.0;
      c.to[0] = 0.0;
      c.to[1] = 0.0;
      c.to[2] = 0.0;
      c.up[0] = 0.0;
      c.up[1] = 10.0;
      c.up[2] = 0.0;
      c.roll = 0.0;

      c.nearp = 0.0;
      if(ay_rrib_near != 0.0)
	c.nearp = ay_rrib_near;

      c.farp = 0.0;
      if(ay_rrib_far != 0.0)
	c.farp = ay_rrib_far;

      if(fabs(ay_rrib_fov) > AY_EPSILON)
	{
	  c.zoom = fabs(tan(AY_D2R(ay_rrib_fov/2.0)));
	}
      else
	{
	  c.zoom = 1.0;
	}

      ay_status = ay_trafo_invmatrix4(ay_rrib_ctrafos->m, mi);
      if(ay_status)
	{
	  ay_error(AY_ERROR, fname, "Could not invert camera transformation.");
	  ay_rrib_RiIdentity();
	  return;
	}


      ay_trafo_apply3(c.from, mi);
      ay_trafo_apply3(c.to, mi);
      ay_trafo_apply3(c.up, mi);

      ay_rrib_RiIdentity();

      ay_rrib_linkobject((void *)(&c), AY_IDCAMERA);

      /*
	l.type = AY_LTLEVEL;
	ay_rrib_co.parent = AY_TRUE;
	ay_rrib_linkobject((void *)(&l), AY_IDLEVEL);
	ay_rrib_co.parent = AY_FALSE;
	ay_object_delete(ay_rrib_co.down);
	ay_rrib_co.down = NULL;
	ay_rrib_RiIdentity();
      */
    } /* if */

  /* start reading materials */
  if(ay_rrib_readmaterial)
    {
      ay_rrib_readmateriali = 1;
    }

  /* start reading gprims if all frames are to be read */
  if(ay_rrib_readframe == -1)
    ay_rrib_initgprims();

 return;
} /* ay_rrib_RiWorldBegin */


RtVoid
ay_rrib_RiWorldEnd(void)
{

  ay_rrib_cleargprims();

 return;
} /* ay_rrib_RiWorldEnd */


RtVoid
ay_rrib_RiBegin(RtToken name)
{
   /*
   LastObjectHandle = 1;
   LastLightHandle = 1;
   */
 return;
} /* ay_rrib_RiBegin */


RtVoid
ay_rrib_RiEnd(void)
{
 return;
} /* ay_rrib_RiEnd */


RtVoid
ay_rrib_RiArchiveRecord(RtToken type, char *format, char *s)
{
 return;
} /* ay_rrib_RiArchiveRecord */


RtVoid
ay_rrib_RiProcedural(RtPointer data, RtBound bound,
		     RtVoid (*subdivfunc)(RtPointer, RtFloat),
		     RtVoid (*freefunc)(RtPointer))
{
 return;
} /* ay_rrib_RiProcedural */


RtPoint*
ay_rrib_RiTransformPoints(RtToken fromspace, RtToken tospace,
			  RtInt n, RtPoint points[])
{
 return NULL;
} /* ay_rrib_RiTransformPoints */


RtVoid
ay_rrib_RiErrorIgnore(RtInt code, RtInt severity, char *msg)
{

  printf("RiErrorIgnore\n");

 return;
} /* ay_rrib_RiErrorIgnore */


RtVoid
ay_rrib_RiErrorPrint(RtInt code, RtInt severity, char *msg)
{

  printf("RiErrorPrint\n");

 return;
} /* ay_rrib_RiErrorPrint */


RtVoid
ay_rrib_RiErrorAbort(RtInt code, RtInt severity, char *msg)
{

  printf("RiErrorAbort\n");

 return;
} /* ay_rrib_RiErrorAbort */


RtFloat
ay_rrib_RiBoxFilter(RtFloat x, RtFloat y,
		    RtFloat xwidth, RtFloat ywidth)
{
 ay_riopt_object *riopt = NULL;
 ay_root_object *root = NULL;

  root = (ay_root_object *)ay_root->refine;
  riopt = root->riopt;

  riopt->FilterFunc = 3;
  riopt->FilterWidth = (double)xwidth;
  riopt->FilterHeight = (double)ywidth;

 return 0.0;
} /* ay_rrib_RiBoxFilter */


RtFloat
ay_rrib_RiTriangleFilter(RtFloat x, RtFloat y, 
			 RtFloat xwidth, RtFloat ywidth)
{
 ay_riopt_object *riopt = NULL;
 ay_root_object *root = NULL;

  root = (ay_root_object *)ay_root->refine;
  riopt = root->riopt;

  riopt->FilterFunc = 1;
  riopt->FilterWidth = (double)xwidth;
  riopt->FilterHeight = (double)ywidth;

 return 0.0;
} /* ay_rrib_RiTriangleFilter */


RtFloat
ay_rrib_RiCatmullRomFilter(RtFloat x, RtFloat y, 
			   RtFloat xwidth, RtFloat ywidth)
{
 ay_riopt_object *riopt = NULL;
 ay_root_object *root = NULL;

  root = (ay_root_object *)ay_root->refine;
  riopt = root->riopt;

  riopt->FilterFunc = 2;
  riopt->FilterWidth = (double)xwidth;
  riopt->FilterHeight = (double)ywidth;

 return 0.0;
} /* ay_rrib_RiCatmullRomFilter */


RtFloat
ay_rrib_RiGaussianFilter(RtFloat x, RtFloat y, 
			 RtFloat xwidth, RtFloat ywidth)
{
 ay_riopt_object *riopt = NULL;
 ay_root_object *root = NULL;

  root = (ay_root_object *)ay_root->refine;
  riopt = root->riopt;

  riopt->FilterFunc = 0;
  riopt->FilterWidth = (double)xwidth;
  riopt->FilterHeight = (double)ywidth;

 return 0.0;
} /* ay_rrib_RiGaussianFilter */


RtFloat
ay_rrib_RiSincFilter(RtFloat x, RtFloat y,
		     RtFloat xwidth, RtFloat ywidth)
{
 ay_riopt_object *riopt = NULL;
 ay_root_object *root = NULL;

  root = (ay_root_object *)ay_root->refine;
  riopt = root->riopt;

  riopt->FilterFunc = 4;
  riopt->FilterWidth = (double)xwidth;
  riopt->FilterHeight = (double)ywidth;

 return 0.0;
} /* ay_rrib_RiSincFilter */


RtVoid
ay_rrib_Ri_version(RtFloat version)
{
 return;
} /* ay_rrib_Ri_version */


RtVoid
ay_rrib_RiProcDelayedReadArchive(RtPointer data, RtFloat detail)
{
 return;
} /* ay_rrib_RiProcDelayedReadArchive */


RtVoid
ay_rrib_RiProcRunProgram(RtPointer data, RtFloat detail)
{
 return;
} /* ay_rrib_RiProcRunProgram */


RtVoid
ay_rrib_RiProcDynamicLoad(RtPointer data, RtFloat detail)
{
 return;
} /* ay_rrib_RiProcDynamicLoad */


void
ay_rrib_readshader(char *sname, int stype,
		   RtInt n, RtToken tokens[], RtPointer parms[],
		   ay_shader **result)
{
 int i = 0, j = 0, k = 0;
 ay_shader *s = NULL;
 ay_shader_arg *sarg = NULL, **nextsarg = NULL;
 RIB_HASHHND ht = NULL;
 PRIB_HASHATOM  p = NULL;
 int type, link;
 char fname[] = "ay_rrib_readshader";
 double dtemp = 0.0;
 char *stemp = NULL;
 RtColor *col;
 RtPoint *pnt;
 RtMatrix *mat;
 RtString *str;
 char *token = NULL;

  if(!(s = calloc(1, sizeof(ay_shader))))
    {
      ay_error(AY_EOMEM, fname, NULL);
      return;
    }

  s->type = stype;

  if(!(s->name = calloc(strlen(sname) + 1, sizeof(char))))
    {
      ay_error(AY_EOMEM, fname, NULL);
      free(s);
      return;
    }

  strcpy(s->name, sname);

  nextsarg = &(s->arg);

  ht = RibGetHashHandle(grib);

  for(i = 0; i < n; i++)
    {

      if(strchr(tokens[i], ' '))
	{
	  token = strchr(tokens[i], ' ')+1;
	}
      else
	{
	  token = tokens[i];
	}

      p = NULL;
      p = RibFindMatch(ht, kRIB_HASH_VARIABLE,
		       kRIB_UNKNOWNCLASS | kRIB_UNKNOWNTYPE,
		       (void*)token);

      if(p)
	{
	  type = kRIB_TYPE_MASK & p->code;
	  link = AY_FALSE;
	  if(p->with.n == 1)
	    {
	      link = AY_TRUE;

	      if(!(sarg = calloc(1, sizeof(ay_shader_arg))))
		return;

	      switch(type)
		{
		case kRIB_INTTYPE:
		case kRIB_FLOATTYPE:
		  sarg->type = AY_SASCALAR;
		  dtemp = (double)(*((RtFloat *)(parms[i])));
		  sarg->val.scalar = dtemp;
		  break;
		case kRIB_STRINGTYPE:
		  sarg->type = AY_SASTRING;
		  str = (RtString *)(parms[i]);
		  stemp = NULL;
		  if(!(stemp = calloc(strlen(*str)+1, sizeof(char))))
		    {
		      link = AY_FALSE;
		      break;
		    }
		  strcpy(stemp, *str);
		  sarg->val.string = stemp;
		  break;
		case kRIB_COLORTYPE:
		  sarg->type = AY_SACOLOR;
		  col = (RtColor *)(parms[i]);
		  sarg->val.color[0] = (float)((*col)[0]);
		  sarg->val.color[1] = (float)((*col)[1]);
		  sarg->val.color[2] = (float)((*col)[2]);
		  break;
		  
		case kRIB_POINTTYPE:
		  sarg->type = AY_SAPOINT;
		  pnt = (RtPoint *)(parms[i]);
		  sarg->val.point[0] = (float)((*pnt)[0]);
		  sarg->val.point[1] = (float)((*pnt)[1]);
		  sarg->val.point[2] = (float)((*pnt)[2]);
		  sarg->val.point[3] = (float)1.0;
		  break;
		case kRIB_NORMALTYPE:
		  sarg->type = AY_SANORMAL;
		  pnt = (RtPoint *)(parms[i]);
		  sarg->val.point[0] = (float)((*pnt)[0]);
		  sarg->val.point[1] = (float)((*pnt)[1]);
		  sarg->val.point[2] = (float)((*pnt)[2]);
		  sarg->val.point[3] = (float)1.0;
		  break;
		case kRIB_VECTORTYPE:
		  sarg->type = AY_SAVECTOR;
		  pnt = (RtPoint *)(parms[i]);
		  sarg->val.point[0] = (float)((*pnt)[0]);
		  sarg->val.point[1] = (float)((*pnt)[1]);
		  sarg->val.point[2] = (float)((*pnt)[2]);
		  sarg->val.point[3] = (float)1.0;
		  break;
		case kRIB_MATRIXTYPE:
		  sarg->type = AY_SAMATRIX;
		  mat = (RtMatrix *)(parms[i]);
		  for(j = 0; j < 4; j++)
		    {
		      for(k = 0; k < 4; k++)
			{
			  sarg->val.matrix[j*4+k] = (float)((*mat)[j][k]);
			} /* for */
		    } /* for */
		  break;
		default:
		  ay_error(AY_ERROR, fname,
			   "Skipping parameter of unknown type:");
		  ay_error(AY_ERROR, fname, tokens[i]);
		  link = AY_FALSE;
		  break;
		} /* switch */
	    

	      /* link argument to shader */
	      if(link)
		{
		  stemp = NULL;
		  if(!(stemp = calloc(strlen(token)+1, sizeof(char))))
		    {
		      if(sarg->type == AY_SASTRING && sarg->val.string)
			free(sarg->val.string);
		      free(sarg);
		    }
		  else
		    {
		      strcpy(stemp, token);
		      sarg->name = stemp;
		      *nextsarg = sarg;
		      nextsarg = &(sarg->next);
		    }
		}
	      else
		{
		  free(sarg);
		}
	    }
	  else
	    {
	      ay_error(AY_ERROR, fname, "Skipping array parameter:");
	      ay_error(AY_ERROR, fname, tokens[i]);
	    }
	}
      else
	{
	  ay_error(AY_ERROR, fname, "Skipping undeclared token:");
	  ay_error(AY_ERROR, fname, tokens[i]);
	}

      
    } /* for */

  *result = s;

 return;
} /* ay_rrib_readshader */


void
ay_rrib_readparams(int n, RtToken tokens[], RtPointer parms[],
		   Tcl_DString *ds)
{
 int i, type;
 RIB_HASHHND ht = NULL;
 PRIB_HASHATOM  p = NULL;
 char fname[] = "ay_rrib_readparams";
 RtColor *col;
 RtPoint *pnt;
 char *valstr = NULL, valbuf[255], typechar;
 char *token = NULL;

  ht = RibGetHashHandle(grib);

  for(i = 0; i < n; i++)
    {

      if(strchr(tokens[i], ' '))
	{
	  token = strchr(tokens[i], ' ')+1;
	}
      else
	{
	  token = tokens[i];
	}

      p = NULL;
      p = RibFindMatch(ht, kRIB_HASH_VARIABLE,
		       kRIB_UNKNOWNCLASS | kRIB_UNKNOWNTYPE,
		       (void*)token);

      if(p)
	{
	  type = kRIB_TYPE_MASK & p->code;

	  switch(type)
	    {
	    case kRIB_INTTYPE:
	      typechar = 'i';
	      sprintf(valbuf,"%d", (int)(*((RtInt *)(parms[i]))));
	      valstr = valbuf;
	      break;
	    case kRIB_FLOATTYPE:
	      typechar = 'f';
	      sprintf(valbuf,"%f", (float)(*((RtFloat *)(parms[i]))));
	      valstr = valbuf;
	      break;
	    case kRIB_STRINGTYPE:
	      typechar = 's';
	      break;
	    case kRIB_COLORTYPE:
	      typechar = 'c';
	      col = (RtColor *)(parms[i]);
	      sprintf(valbuf,"%f,%f,%f", (float)((*col)[0]),(float)((*col)[1]),
		      (float)((*col)[2]));
	      valstr = valbuf;
	      break;		  
	    case kRIB_POINTTYPE:
	      typechar = 'p';
	      pnt = (RtPoint *)(parms[i]);
	      sprintf(valbuf,"%f,%f,%f", (float)((*pnt)[0]),(float)((*pnt)[1]),
		      (float)((*pnt)[2]));
	      break;
	    default:
	      typechar = 'u';
	      ay_error(AY_ERROR, fname,
		       "Skipping parameter of unknown type:");
	      ay_error(AY_ERROR, fname, tokens[i]);
	      break;
	    } /* switch */

	  if(typechar != 'u')
	    {
	      Tcl_DStringAppend(ds, ",", -1);
	      Tcl_DStringAppend(ds, token, -1);
	      Tcl_DStringAppend(ds, ",", -1);
	      Tcl_DStringAppend(ds, &typechar, 1);
	      Tcl_DStringAppend(ds, ",", -1);
	      if(typechar == 's')
		{
		  Tcl_DStringAppend(ds, *((char **)(parms[i])), -1);
		}
	      else
		{
		  Tcl_DStringAppend(ds, valstr, -1);
		} /* if */
	    } /* if */
	}
      else
	{
	  ay_error(AY_ERROR, fname, "Skipping undeclared token:");
	  ay_error(AY_ERROR, fname, tokens[i]);
	} /* if */
    } /* for */

 return;
} /* ay_rrib_readparams */


void
ay_rrib_readtag(char *tagtype, char *tagname, char *name,
		int i, RtToken tokens[], RtPointer parms[],
		ay_tag_object **destination)
{
 int type, found = AY_FALSE;
 ay_tag_object *n = NULL, *ot;
 Tcl_DString ds;
 RIB_HASHHND ht = NULL;
 PRIB_HASHATOM  p = NULL;
 char fname[] = "ay_rrib_readtag";
 RtColor *col;
 RtPoint *pnt;
 char *valstr = NULL, valbuf[255], typechar;
 char *token = NULL;

  if(!destination)
   return;

  if(!(n = calloc(1, sizeof(ay_tag_object))))
    {
      return;
    }

  if(!(n->name = calloc(strlen(tagname)+1, sizeof(char))))
    {
      return;
    }

  strcpy(n->name, tagname);

  n->type = tagtype;

  ht = RibGetHashHandle(grib);

  if(strchr(tokens[i], ' '))
    {
      token = strchr(tokens[i], ' ')+1;
    }
  else
    {
      token = tokens[i];
    }

  p = NULL;
  p = RibFindMatch(ht, kRIB_HASH_VARIABLE,
		   kRIB_UNKNOWNCLASS | kRIB_UNKNOWNTYPE,
		   (void*)token);

  if(p)
    {
      type = kRIB_TYPE_MASK & p->code;

      switch(type)
	{
	case kRIB_INTTYPE:
	  typechar = 'i';
	  sprintf(valbuf,"%d", (int)(*((RtInt *)(parms[i]))));
	  valstr = valbuf;
	  break;
	case kRIB_FLOATTYPE:
	  typechar = 'f';
	  sprintf(valbuf,"%f", (float)(*((RtFloat *)(parms[i]))));
	  valstr = valbuf;
	  break;
	case kRIB_STRINGTYPE:
	  typechar = 's';
	  break;
	case kRIB_COLORTYPE:
	  typechar = 'c';
	  col = (RtColor *)(parms[i]);
	  sprintf(valbuf,"%f,%f,%f", (float)((*col)[0]),(float)((*col)[1]),
		  (float)((*col)[2]));
	  valstr = valbuf;
	  break;		  
	case kRIB_POINTTYPE:
	  typechar = 'p';
	  pnt = (RtPoint *)(parms[i]);
	  sprintf(valbuf,"%f,%f,%f", (float)((*pnt)[0]),(float)((*pnt)[1]),
		  (float)((*pnt)[2]));
	  break;
	default:
	  ay_error(AY_ERROR, fname,
		   "Skipping parameter of unknown type:");
	  ay_error(AY_ERROR, fname, tokens[i]);
	  free(n->name);
	  free(n);
	  return;
	  break;
	} /* switch */
    }
  else
    {
      ay_error(AY_ERROR, fname, "Skipping undeclared token:");
      ay_error(AY_ERROR, fname, tokens[i]);
      free(n->name);
      free(n);
      return;
    }

  Tcl_DStringInit(&ds);

  Tcl_DStringAppend(&ds, name, -1);
  Tcl_DStringAppend(&ds, ",", -1);
  Tcl_DStringAppend(&ds, token, -1);
  Tcl_DStringAppend(&ds, ",", -1);
  Tcl_DStringAppend(&ds, &typechar, 1);
  Tcl_DStringAppend(&ds, ",", -1);

  if(typechar == 's')
    {
      Tcl_DStringAppend(&ds, *((char **)(parms[i])), -1);
    }
  else
    {
      Tcl_DStringAppend(&ds, valstr, -1);
    }

  if(!(n->val = calloc(strlen(Tcl_DStringValue(&ds))+1, sizeof(char))))
    {
      free(n->name);
      free(n);
      Tcl_DStringFree(&ds);
      return;
    }

  strcpy(n->val, Tcl_DStringValue(&ds));

  Tcl_DStringFree(&ds);

  /* find eventually existing tag with the same name and replace it */
  if(*destination)
    {
      found = AY_FALSE;
      ot = *destination;
      while(ot && !found)
	{
	  if(ot->type == tagtype)
	    {
	      if(!strncmp(ot->val, n->val, strlen(name)))
		{
		  found = AY_TRUE;
		  free(ot->val);
		  ot->val = n->val;
		  free(n->name);
		  free(n);
		}
	    }
	  ot = ot->next;
	}

      if(!found)
	{
	  n->next = *destination;
	  *destination = n;
	}

    }
  else
    {
      n->next = *destination;
      *destination = n;
    }

 return;
} /* ay_rrib_readtag */


/* avoid names with spaces or other characters that may be
   (mis)interpreted by Tcl */
void
ay_rrib_fixname(char *name)
{

  if(name)
    {
      while(*name != '\0')
	{
	  if(*name == ' ')
	    *name = '_';

	  if((*name == '[') || (*name == '{'))
	    *name = '(';

	  if((*name == ']') || (*name == '}'))
	    *name = ')';

	  name++;
	} /* while */
    } /* if */

 return;
} /* ay_rrib_fixname */


void
ay_rrib_initgeneral(void)
{

  gRibNopRITable[kRIB_WORLDBEGIN] = (PRIB_RIPROC)ay_rrib_RiWorldBegin;
  gRibNopRITable[kRIB_PROJECTION] = (PRIB_RIPROC)ay_rrib_RiProjection;

  gRibNopRITable[kRIB_TRANSFORM] = (PRIB_RIPROC)ay_rrib_RiTransform;
  gRibNopRITable[kRIB_TRANSFORMBEGIN] = (PRIB_RIPROC)ay_rrib_RiTransformBegin;
  gRibNopRITable[kRIB_TRANSFORMEND] = (PRIB_RIPROC)ay_rrib_RiTransformEnd;
  gRibNopRITable[kRIB_CONCATTRANSFORM] =
    (PRIB_RIPROC)ay_rrib_RiConcatTransform;
  gRibNopRITable[kRIB_IDENTITY] = (PRIB_RIPROC)ay_rrib_RiIdentity;
  gRibNopRITable[kRIB_TRANSLATE] = (PRIB_RIPROC)ay_rrib_RiTranslate;
  gRibNopRITable[kRIB_ROTATE] = (PRIB_RIPROC)ay_rrib_RiRotate;
  gRibNopRITable[kRIB_SCALE] = (PRIB_RIPROC)ay_rrib_RiScale;

  gRibNopRITable[kRIB_ATTRIBUTEBEGIN] = (PRIB_RIPROC)ay_rrib_RiAttributeBegin;
  gRibNopRITable[kRIB_ATTRIBUTEEND] = (PRIB_RIPROC)ay_rrib_RiAttributeEnd;
  gRibNopRITable[kRIB_ATTRIBUTE] = (PRIB_RIPROC)ay_rrib_RiAttribute;
  gRibNopRITable[kRIB_COLOR] = (PRIB_RIPROC)ay_rrib_RiColor;
  gRibNopRITable[kRIB_OPACITY] = (PRIB_RIPROC)ay_rrib_RiOpacity;
  gRibNopRITable[kRIB_SHADINGRATE] = (PRIB_RIPROC)ay_rrib_RiShadingRate;
  gRibNopRITable[kRIB_SHADINGINTERPOLATION] =
    (PRIB_RIPROC)ay_rrib_RiShadingInterpolation;
  gRibNopRITable[kRIB_SIDES] = (PRIB_RIPROC)ay_rrib_RiSides;
  gRibNopRITable[kRIB_TEXTURECOORDINATES] =
    (PRIB_RIPROC)ay_rrib_RiTextureCoordinates;

  gRibNopRITable[kRIB_SURFACE] = (PRIB_RIPROC)ay_rrib_RiSurface;
  gRibNopRITable[kRIB_DISPLACEMENT] = (PRIB_RIPROC)ay_rrib_RiDisplacement;
  gRibNopRITable[kRIB_INTERIOR] = (PRIB_RIPROC)ay_rrib_RiInterior;
  gRibNopRITable[kRIB_EXTERIOR] = (PRIB_RIPROC)ay_rrib_RiExterior;

  gRibNopRITable[kRIB_LIGHTSOURCE] = (PRIB_RIPROC)ay_rrib_RiLightSource;
  gRibNopRITable[kRIB_AREALIGHTSOURCE] =
    (PRIB_RIPROC)ay_rrib_RiAreaLightSource;
  gRibNopRITable[kRIB_ILLUMINATE] = (PRIB_RIPROC)ay_rrib_RiIlluminate;

  gRibNopRITable[kRIB_OBJECTBEGIN] = (PRIB_RIPROC)ay_rrib_RiObjectBegin;
  gRibNopRITable[kRIB_OBJECTEND] = (PRIB_RIPROC)ay_rrib_RiObjectEnd;
  gRibNopRITable[kRIB_READARCHIVE] = (PRIB_RIPROC)ay_rrib_RiReadArchive;

  gRibNopRITable[kRIB_CLIPPING] = (PRIB_RIPROC)ay_rrib_RiClipping;

 return;
} /* ay_rrib_initgeneral */


void
ay_rrib_cleargeneral(void)
{

  gRibNopRITable[kRIB_WORLDBEGIN] = (PRIB_RIPROC)RiNopWorldBegin;
  gRibNopRITable[kRIB_PROJECTION] = (PRIB_RIPROC)RiNopProjectionV;

  gRibNopRITable[kRIB_TRANSFORM] = (PRIB_RIPROC)RiNopTransform;
  gRibNopRITable[kRIB_TRANSFORMBEGIN] = (PRIB_RIPROC)RiNopTransformBegin;
  gRibNopRITable[kRIB_TRANSFORMEND] = (PRIB_RIPROC)RiNopTransformEnd;
  gRibNopRITable[kRIB_CONCATTRANSFORM] =
    (PRIB_RIPROC)RiNopConcatTransform;
  gRibNopRITable[kRIB_IDENTITY] = (PRIB_RIPROC)RiNopIdentity;
  gRibNopRITable[kRIB_TRANSLATE] = (PRIB_RIPROC)RiNopTranslate;
  gRibNopRITable[kRIB_ROTATE] = (PRIB_RIPROC)RiNopRotate;
  gRibNopRITable[kRIB_SCALE] = (PRIB_RIPROC)RiNopScale;


  gRibNopRITable[kRIB_ATTRIBUTEBEGIN] = (PRIB_RIPROC)RiNopAttributeBegin;
  gRibNopRITable[kRIB_ATTRIBUTEEND] = (PRIB_RIPROC)RiNopAttributeEnd;
  gRibNopRITable[kRIB_ATTRIBUTE] = (PRIB_RIPROC)RiNopAttributeV;
  gRibNopRITable[kRIB_COLOR] = (PRIB_RIPROC)RiNopColor;
  gRibNopRITable[kRIB_OPACITY] = (PRIB_RIPROC)RiNopOpacity;
  gRibNopRITable[kRIB_SHADINGRATE] = (PRIB_RIPROC)RiNopShadingRate;
  gRibNopRITable[kRIB_SHADINGINTERPOLATION] =
    (PRIB_RIPROC)RiNopShadingInterpolation;
  gRibNopRITable[kRIB_SIDES] = (PRIB_RIPROC)RiNopSides;
  gRibNopRITable[kRIB_TEXTURECOORDINATES] =
    (PRIB_RIPROC)RiNopTextureCoordinates;

  gRibNopRITable[kRIB_SURFACE] = (PRIB_RIPROC)RiNopSurfaceV;
  gRibNopRITable[kRIB_DISPLACEMENT] = (PRIB_RIPROC)RiNopDisplacementV;
  gRibNopRITable[kRIB_INTERIOR] = (PRIB_RIPROC)RiNopInteriorV;
  gRibNopRITable[kRIB_EXTERIOR] = (PRIB_RIPROC)RiNopExteriorV;

  gRibNopRITable[kRIB_LIGHTSOURCE] = (PRIB_RIPROC)RiNopLightSourceV;
  gRibNopRITable[kRIB_AREALIGHTSOURCE] =
    (PRIB_RIPROC)RiNopAreaLightSourceV;
  gRibNopRITable[kRIB_ILLUMINATE] = (PRIB_RIPROC)RiNopIlluminate;

  gRibNopRITable[kRIB_OBJECTBEGIN] = (PRIB_RIPROC)RiNopObjectBegin;
  gRibNopRITable[kRIB_OBJECTEND] = (PRIB_RIPROC)RiNopObjectEnd;
  gRibNopRITable[kRIB_READARCHIVE] = (PRIB_RIPROC)RiNopReadArchiveV;

  gRibNopRITable[kRIB_CLIPPING] = (PRIB_RIPROC)RiNopClipping;

 return;
} /* ay_rrib_cleargeneral */


void
ay_rrib_initoptions(void)
{

  gRibNopRITable[kRIB_OPTION] = (PRIB_RIPROC)ay_rrib_RiOption;
  gRibNopRITable[kRIB_ATMOSPHERE] = (PRIB_RIPROC)ay_rrib_RiAtmosphere;
  gRibNopRITable[kRIB_IMAGER] = (PRIB_RIPROC)ay_rrib_RiImager;
  gRibNopRITable[kRIB_EXPOSURE] = (PRIB_RIPROC)ay_rrib_RiExposure;
  gRibNopRITable[kRIB_FORMAT] = (PRIB_RIPROC)ay_rrib_RiFormat;
  gRibNopRITable[kRIB_PIXELSAMPLES] = (PRIB_RIPROC)ay_rrib_RiPixelSamples;
  gRibNopRITable[kRIB_PIXELVARIANCE] = (PRIB_RIPROC)ay_rrib_RiPixelVariance;
  gRibNopRITable[kRIB_QUANTIZE] = (PRIB_RIPROC)ay_rrib_RiQuantize;
  gRibNopRITable[kRIB_DISPLAY] = (PRIB_RIPROC)ay_rrib_RiDisplay;
  gRibNopRITable[kRIB_HIDER] = (PRIB_RIPROC)ay_rrib_RiHider;


  gRibNopRITable[kRIB_BOXFILTER] =
    (PRIB_RIPROC)ay_rrib_RiBoxFilter;
  gRibNopRITable[kRIB_TRIANGLEFILTER] =
    (PRIB_RIPROC)ay_rrib_RiTriangleFilter;
  gRibNopRITable[kRIB_GAUSSIANFILTER] =
    (PRIB_RIPROC)ay_rrib_RiGaussianFilter;
  gRibNopRITable[kRIB_SINCFILTER] =
    (PRIB_RIPROC)ay_rrib_RiSincFilter;
  gRibNopRITable[kRIB_CATMULLROMFILTER] =
    (PRIB_RIPROC)ay_rrib_RiCatmullRomFilter;

 return;
} /* ay_rrib_initoptions */


void
ay_rrib_initgprims(void)
{

  gRibNopRITable[kRIB_SPHERE] = (PRIB_RIPROC)ay_rrib_RiSphere;
  gRibNopRITable[kRIB_CYLINDER] = (PRIB_RIPROC)ay_rrib_RiCylinder;
  gRibNopRITable[kRIB_DISK] = (PRIB_RIPROC)ay_rrib_RiDisk;
  gRibNopRITable[kRIB_CONE] = (PRIB_RIPROC)ay_rrib_RiCone;
  gRibNopRITable[kRIB_PARABOLOID] = (PRIB_RIPROC)ay_rrib_RiParaboloid;
  gRibNopRITable[kRIB_HYPERBOLOID] = (PRIB_RIPROC)ay_rrib_RiHyperboloid;
  gRibNopRITable[kRIB_TORUS] = (PRIB_RIPROC)ay_rrib_RiTorus;
  gRibNopRITable[kRIB_NUPATCH] = (PRIB_RIPROC)ay_rrib_RiNuPatch;
  gRibNopRITable[kRIB_TRIMCURVE] = (PRIB_RIPROC)ay_rrib_RiTrimCurve;

  gRibNopRITable[kRIB_PATCH] = (PRIB_RIPROC)ay_rrib_RiPatch;
  gRibNopRITable[kRIB_PATCHMESH] = (PRIB_RIPROC)ay_rrib_RiPatchMesh;
  gRibNopRITable[kRIB_BASIS] = (PRIB_RIPROC)ay_rrib_RiBasis;

  gRibNopRITable[kRIB_POINTSGENERALPOLYGONS] =
    (PRIB_RIPROC)ay_rrib_RiPointsGeneralPolygons;
  gRibNopRITable[kRIB_POINTSPOLYGONS] =
    (PRIB_RIPROC)ay_rrib_RiPointsPolygons;
  gRibNopRITable[kRIB_GENERALPOLYGON] =
    (PRIB_RIPROC)ay_rrib_RiGeneralPolygon;
  gRibNopRITable[kRIB_POLYGON] =
    (PRIB_RIPROC)ay_rrib_RiPolygon;
  gRibNopRITable[kRIB_SUBDIVISIONMESH] =
    (PRIB_RIPROC)ay_rrib_RiSubdivisionMesh;

  gRibNopRITable[kRIB_SOLIDBEGIN] = (PRIB_RIPROC)ay_rrib_RiSolidBegin;
  gRibNopRITable[kRIB_SOLIDEND] = (PRIB_RIPROC)ay_rrib_RiSolidEnd;

  gRibNopRITable[kRIB_OBJECTINSTANCE] = (PRIB_RIPROC)ay_rrib_RiObjectInstance;

 return;
} /* ay_rrib_initgprims */


void
ay_rrib_cleargprims(void)
{

  gRibNopRITable[kRIB_SPHERE] = (PRIB_RIPROC)RiNopSphereV;
  gRibNopRITable[kRIB_CYLINDER] = (PRIB_RIPROC)RiNopCylinderV;
  gRibNopRITable[kRIB_DISK] = (PRIB_RIPROC)RiNopDiskV;
  gRibNopRITable[kRIB_CONE] = (PRIB_RIPROC)RiNopConeV;
  gRibNopRITable[kRIB_PARABOLOID] = (PRIB_RIPROC)RiNopParaboloidV;
  gRibNopRITable[kRIB_HYPERBOLOID] = (PRIB_RIPROC)RiNopHyperboloidV;
  gRibNopRITable[kRIB_TORUS] = (PRIB_RIPROC)RiNopTorusV;
  gRibNopRITable[kRIB_NUPATCH] = (PRIB_RIPROC)RiNopNuPatchV;
  gRibNopRITable[kRIB_TRIMCURVE] = (PRIB_RIPROC)RiNopTrimCurve;

  gRibNopRITable[kRIB_PATCH] = (PRIB_RIPROC)RiNopPatchV;
  gRibNopRITable[kRIB_PATCHMESH] = (PRIB_RIPROC)RiNopPatchMeshV;
  gRibNopRITable[kRIB_BASIS] = (PRIB_RIPROC)RiNopBasis;

  gRibNopRITable[kRIB_POINTSGENERALPOLYGONS] =
    (PRIB_RIPROC)RiNopPointsGeneralPolygonsV;
  gRibNopRITable[kRIB_POINTSPOLYGONS] =
    (PRIB_RIPROC)RiNopPointsPolygonsV;
  gRibNopRITable[kRIB_GENERALPOLYGON] =
    (PRIB_RIPROC)RiNopGeneralPolygonV;
  gRibNopRITable[kRIB_POLYGON] =
    (PRIB_RIPROC)RiNopPolygonV;
  gRibNopRITable[kRIB_SUBDIVISIONMESH] =
    (PRIB_RIPROC)RiNopSubdivisionMeshV;

  gRibNopRITable[kRIB_SOLIDBEGIN] = (PRIB_RIPROC)RiNopSolidBegin;
  gRibNopRITable[kRIB_SOLIDEND] = (PRIB_RIPROC)RiNopSolidEnd;

  gRibNopRITable[kRIB_OBJECTINSTANCE] = (PRIB_RIPROC)RiNopObjectInstance;

 return;
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
      newstate->identifier_name = NULL;
      newstate->trimcurves = NULL;
      newstate->sshader = NULL;
      newstate->dshader = NULL;
      newstate->ishader = NULL;
      newstate->eshader = NULL;
      newstate->tags = NULL;

      if(ay_rrib_cattributes->sshader)
	{
	  ay_status = ay_shader_copy(ay_rrib_cattributes->sshader,
				     &(newstate->sshader));
	}

      if(ay_rrib_cattributes->dshader)
	{
	  ay_status = ay_shader_copy(ay_rrib_cattributes->dshader,
				     &(newstate->dshader));
	}

      if(ay_rrib_cattributes->ishader)
	{
	  ay_status = ay_shader_copy(ay_rrib_cattributes->ishader,
				     &(newstate->ishader));
	}

      if(ay_rrib_cattributes->eshader)
	{
	  ay_status = ay_shader_copy(ay_rrib_cattributes->eshader,
				     &(newstate->eshader));
	}
#if 0
      /* copy trimcurves */
      if(ay_rrib_cattributes->trimcurves)
	{
	  /* XXXX Bug: this simple copy means, only the first trimcurve
	     survives a BeginAttribute! */
	  ay_status = ay_object_copy(ay_rrib_cattributes->trimcurves,
				     &(newstate->trimcurves));


	}
#endif
    }
  else
    {
      /* there was no old attribute state, so we fill the very first
	 with the default values */
      newstate->light_samples = -1;
      newstate->light_shadows = -1;
      newstate->shading_rate = 1.0;
      newstate->colr = -1;
      newstate->opr = -1;
      newstate->camera = 1;
      newstate->reflection = 1;
      newstate->shadow = 1;
      newstate->s1 = 0.0f;
      newstate->t1 = 0.0f;
      newstate->s2 = 1.0f;
      newstate->t2 = 0.0f;
      newstate->s3 = 0.0f;
      newstate->t3 = 1.0f;
      newstate->s4 = 1.0f;
      newstate->t4 = 1.0f;
    } /* if */

  /* link new state to stack */
  newstate->next = ay_rrib_cattributes;
  ay_rrib_cattributes = newstate;

  /* did we just read an RiAttribute after an AreaLight? */
  if(ay_rrib_cattributes->read_arealight_geom > 0)
    {
      /* Yes. Start reading geometric primitives of the arealight. */
      ay_rrib_cattributes->read_arealight_geom++;
      ay_rrib_initgprims();

      /* find light source and start adding next objects as childs */
      /* XXXX assume, the area light source is the last object we read */
      ay_status = ay_clevel_add(ay_rrib_lrobject);
      ay_status = ay_clevel_add(ay_rrib_lrobject->down);
      ay_next = &(ay_rrib_lrobject->down);
    } /* if */

 return;
} /* ay_rrib_pushattribs */


void
ay_rrib_popattribs(void)
{
 ay_rrib_attrstate *nextstate = NULL;
 ay_tag_object *tag = NULL;
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

  if(ay_rrib_cattributes->identifier_name)
    {
      free(ay_rrib_cattributes->identifier_name);
    }

  if(ay_rrib_cattributes->sshader)
    {
      ay_shader_free(ay_rrib_cattributes->sshader);
      ay_rrib_cattributes->sshader = NULL;
    }

  if(ay_rrib_cattributes->dshader)
    {
      ay_shader_free(ay_rrib_cattributes->dshader);
      ay_rrib_cattributes->dshader = NULL;
    }

  if(ay_rrib_cattributes->ishader)
    {
      ay_shader_free(ay_rrib_cattributes->ishader);
      ay_rrib_cattributes->ishader = NULL;
    }

  if(ay_rrib_cattributes->eshader)
    {
      ay_shader_free(ay_rrib_cattributes->eshader);
      ay_rrib_cattributes->eshader = NULL;
    }

  if(ay_rrib_cattributes->tags)
    {
      while(ay_rrib_cattributes->tags)
	{
	  tag = ay_rrib_cattributes->tags;
	  ay_rrib_cattributes->tags = tag->next;
	  ay_tags_free(tag);
	}
    } /* if */

  free(ay_rrib_cattributes);

  ay_rrib_cattributes = nextstate;
  if(ay_rrib_cattributes)
    {
      if(ay_rrib_cattributes->read_arealight_geom > 0)
	{
	  /*      ay_rrib_cattributes->read_arealight_geom--;*/
	  if(ay_rrib_cattributes->read_arealight_geom == 1)
	    {
	      /* found matching AttributeEnd, stop reading geometry
		 if not reading anything anyway (ay_rrib_readpartial == 1) */
	      if(!ay_rrib_readpartial)
		{
		  ay_rrib_cleargprims();
		}
	      ay_rrib_cattributes->read_arealight_geom = 0;
	      /* go up in the scene hierarchy */
	      ay_clevel_del();
	      ay_next = &(ay_currentlevel->object->next);
	      ay_clevel_del();
	    } /* if */
	} /* if */
    } /* if */

 return;
} /* ay_rrib_popattribs */


void
ay_rrib_pushtrafos(void)
{
 ay_rrib_trafostate *newstate = NULL;
 char fname[] = "ay_rrib_pushtrafos";
 int i;

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
      /* there was no old transformation state, so we fill the very first
	 with the default values */
      for(i = 0; i < 16; i++)
	{
	  newstate->m[i] = 0.0;
	}

      newstate->m[0] = 1.0;
      newstate->m[5] = 1.0;
      newstate->m[10] = 1.0;
      newstate->m[15] = 1.0;

    } /* if */

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


/*
 * Matrix Decomposition Code borrowed from Graphics Gems II unmatrix.c
 */
void
ay_rrib_trafotoobject(ay_object *o, double *transform)
{
 double v1[3], v2[3], v3[3], v4[3];
 double sx, sy, sz;
 double rx, ry, rz;
 int i;
 double axis[3], quat[4] = {0};
 char fname[] = "ay_rrib_trafotoobject";

  o->scalx = 1.0;
  o->scaly = 1.0;
  o->scalz = 1.0;
  o->quat[0] = 0.0;
  o->quat[1] = 0.0;
  o->quat[2] = 0.0;
  o->quat[3] = 1.0;
  o->rotx = 0.0;
  o->roty = 0.0;
  o->rotz = 0.0;

  quat[3] = 1.0;

  if(fabs(transform[15]) <= AY_EPSILON )
    return;

  /* normalize matrix */
  for(i = 0; i < 16; i++)
      transform[i] /= transform[15];

  /* decompose matrix */

  /* get translation */
  o->movx = (double)transform[12];
  o->movy = (double)transform[13];
  o->movz = (double)transform[14];
  
  /* get row vectors containing scale&rotation */
  v1[0] = (double)transform[0];
  v1[1] = (double)transform[1];
  v1[2] = (double)transform[2];

  v2[0] = (double)transform[4];
  v2[1] = (double)transform[5];
  v2[2] = (double)transform[6];

  v3[0] = (double)transform[8];
  v3[1] = (double)transform[9];
  v3[2] = (double)transform[10];

  /* get scale */
  sx = AY_V3LEN(v1);
  sy = AY_V3LEN(v2);
  sz = AY_V3LEN(v3);

  /* normalize row vectors */
  if(fabs(sx) > AY_EPSILON)
    {
      o->scalx *= sx;
      AY_V3SCAL(v1, 1.0/sx);
    }
  if(fabs(sy) > AY_EPSILON)
    {
      o->scaly *= sy;
      AY_V3SCAL(v2, 1.0/sy);
    }
  if(fabs(sz) > AY_EPSILON)
    {
      o->scalz *= sz;
      AY_V3SCAL(v3, 1.0/sz);
    }

  /*
   * Check for a coordinate system flip.  If the determinant
   * is -1, then negate the matrix and the scaling factors.
   */
  AY_V3CROSS(v4, v2, v3)
  if(AY_V3DOT(v1, v4) < 0)
    {
      ay_error(AY_EWARN, fname, "Coordinate system flip detected!");
      
      o->scalx *= -1.0;
      o->scaly *= -1.0;
      o->scalz *= -1.0;
      
      for ( i = 0; i < 3; i++ )
	{
	  v1[i] *= -1;
	}
      for ( i = 0; i < 3; i++ )
	{
	  v2[i] *= -1;
	}
      for ( i = 0; i < 3; i++ )
	{
	  v3[i] *= -1;
	}
    }

  /* now get rotation */
  ry = asin(-v1[2]);
  if(cos(ry) != 0)
    {
      rx = atan2(v2[2], v3[2]);
      rz = atan2(v1[1], v1[0]);
    }
  else
    {
      rx = atan2(v2[0], v2[1]);
      rz = 0;
    }

  if(fabs(rx) > AY_EPSILON)
    {
      axis[0] = 1.0;
      axis[1] = 0.0;
      axis[2] = 0.0;
      quat[0] = 0.0;
      quat[1] = 0.0;
      quat[2] = 0.0;
      quat[3] = 1.0;
      ay_quat_axistoquat(axis, -rx, quat);
      ay_quat_add(quat, o->quat, o->quat);
      o->rotx = AY_R2D(rx);
    }

  if(fabs(ry) > AY_EPSILON)
    {
      axis[0] = 0.0;
      axis[1] = 1.0;
      axis[2] = 0.0;
      quat[0] = 0.0;
      quat[1] = 0.0;
      quat[2] = 0.0;
      quat[3] = 1.0;
      ay_quat_axistoquat(axis, -ry, quat);
      ay_quat_add(quat, o->quat, o->quat);
      o->roty = AY_R2D(ry);
    }

  if(fabs(rz) > AY_EPSILON)
    {
      axis[0] = 0.0;
      axis[1] = 0.0;
      axis[2] = 1.0;
      quat[0] = 0.0;
      quat[1] = 0.0;
      quat[2] = 0.0;
      quat[3] = 1.0;
      ay_quat_axistoquat(axis, -rz, quat);
      ay_quat_add(quat, o->quat, o->quat);
      o->rotz = AY_R2D(rz);
    }

 return;
} /* ay_rrib_trafotoobject */


int
ay_rrib_comptags(ay_object *o1, ay_object *o2)
{
 ay_tag_object *t1, *t2, *t, *tc;
 int i = 0, j = 0, found = AY_FALSE;

  t1 = o1->tags;
  t2 = o2->tags;

  if(!t1 && !t2)
    return AY_TRUE;

  /* count RiAttribute tags of o1 and o2 */
  t = t1;
  while(t)
    {
      if(t->type == ay_riattr_tagtype)
	i++;

      t = t->next;
    } /* while */
  t = t2;
  while(t)
    {
      if(t->type == ay_riattr_tagtype)
	j++;

      t = t->next;
    } /* while */

  if(i != j)
    return AY_FALSE;

  /* find matching tags for all RiAttribute tags of o1 */
  t = t1;
  while(t)
    {
      if(t->type == ay_riattr_tagtype)
	{
	  found = AY_FALSE;
	  tc = t2;
	  while(tc && !found)
	    {
	      if(tc->type == ay_riattr_tagtype)
		{
		  /* XXXX this simple string comparison should be changed
		     to a scanf(), compare int/float operation, but there
		     are so many RiAttribute sub types... */
		  if(!strcmp(t->val, tc->val))
		    found = AY_TRUE;
		} /* if */
	      tc = tc->next;
	    } /* while */

	  if(!found)
	    return AY_FALSE;

	} /* if */

      t = t->next;
    } /* while */

 return AY_TRUE;
} /* ay_rrib_comptags */


void
ay_rrib_linkmaterial(ay_object *o)
{
 int ay_status = AY_OK;
 char *fname = "ay_rrib_linkmaterial";
 ay_object *m = NULL, *oldm = NULL, **old_aynext;
 ay_mat_object *mat, *linkwith = NULL;
 ay_rrib_attrstate *attr = ay_rrib_cattributes;
 char materialname[255] = "";
 int found = AY_FALSE;
 
  /* create a material object from current attribute state */
  if(!(mat = calloc(1, sizeof(ay_mat_object))))
    {
      ay_error(AY_EOMEM, fname, NULL);
      return;
    }

  mat->colr = attr->colr;
  mat->colg = attr->colg;
  mat->colb = attr->colb;
  mat->opr = attr->opr;
  mat->opg = attr->opg;
  mat->opb = attr->opb;

  mat->shading_rate = attr->shading_rate;
  mat->shading_interpolation = attr->shading_interpolation;
  mat->sides = attr->sides;
  mat->dbound_val = attr->dbound_val;
  mat->dbound = attr->dbound;

  if(attr->sshader)
    ay_shader_copy(attr->sshader, &(mat->sshader));

  if(attr->dshader)
    ay_shader_copy(attr->dshader, &(mat->dshader));

  if(attr->ishader)
    ay_shader_copy(attr->ishader, &(mat->ishader));

  if(attr->eshader)
    ay_shader_copy(attr->eshader, &(mat->eshader));
  /*
  mat->avr = attr->avr;
  mat->avg = attr->avg;
  mat->avb = attr->avb;
  mat->ava = attr->ava;

  mat->emr = attr->emr;
  mat->emg = attr->emg;
  mat->emb = attr->emb;
  mat->ema = attr->ema;

  mat->spr = attr->spr;
  mat->spg = attr->spg;
  mat->spb = attr->spb;
  mat->spa = attr->spa;

  mat->patch_size = attr->patch_size;
  mat->elem_size = attr->elem_size;
  mat->min_size = attr->min_size;
  mat->zonal = attr->zonal;
  mat->has_caustics = attr->has_caustics;
  */
  mat->cast_shadows = attr->cast_shadows;

  mat->true_displacement = attr->true_displacement;

  mat->camera = attr->camera;
  mat->reflection = attr->reflection;
  mat->shadow = attr->shadow;

  if(!(m = calloc(1, sizeof(ay_object))))
    {
      if(mat->sshader)
	free(mat->sshader);
      if(mat->dshader)
	free(mat->dshader);
      if(mat->ishader)
	free(mat->ishader);
      if(mat->eshader)
	free(mat->eshader);
      free(mat);
      return;
    }

  m->refine = mat;
  m->type = AY_IDMATERIAL;
  mat->nameptr = &(m->name);
  mat->refcountptr = &(m->refcount);
  mat->objptr = m;

  /* compare this object with all existing materials
     in the first level ("Materials") before registering
     a new material */
  if(ay_root->next && ay_root->next->down)
    {
      oldm = ay_root->next->down;
      while(oldm && !found)
	{
	  if(oldm->type == AY_IDMATERIAL)
	    {
	      /* do not let accidentally set transformations
		  make the comparison fail */
	      ay_trafo_copy(oldm, m);
	      /* compare objects */
	      if(ay_comp_objects(oldm, m))
		{
		  /* now compare the tags of the materials */
		  if(ay_rrib_comptags(oldm, m))
		    {
		      found = AY_TRUE;
		      linkwith = (ay_mat_object *)(oldm->refine);
		    }
		} /* if */
	    } /* if */

	  oldm = oldm->next;
	} /* while */
    } /* if */

  if(linkwith == NULL)
    {
      /* no matching material found in scene, create new */
      ay_status = AY_ERROR;
      while((ay_status == AY_ERROR) && (ay_rrib_lastmaterialnum < INT_MAX))
	{
	  ay_rrib_lastmaterialnum++;

	  sprintf(materialname, "mat%d", ay_rrib_lastmaterialnum);

	  ay_status = ay_matt_registermaterial(materialname, mat);
	} /* while */

      if(ay_status)
	{
	  ay_error(AY_ERROR, fname, "Could not register material name.");
	  ay_object_delete(m);
	  return;
	}

      if(!(m->name = calloc(strlen(materialname)+1, sizeof(char))))
	{
	  ay_error(AY_EOMEM, fname, NULL);
	  ay_object_delete(m);
	  return;
	}
      
      strcpy(m->name, materialname);

      /* link new material object to scene */
      if(ay_root->next && ay_root->next->down)
	{
	  old_aynext = ay_next;
	  ay_next = &(ay_root->next->down);
	  ay_status = ay_object_link(m);
	  ay_next = old_aynext;
	}
      else
	{
	  ay_error(AY_ERROR, fname, "Material level does not exist?");
	  ay_object_delete(m);
	  return;
	}
      
      linkwith = mat;
    } /* if */

  /* increase the reference counter */
  (*(linkwith->refcountptr))++;

  /* dispose unlinked material object */
  if(linkwith->objptr != m)
    {
      ay_object_delete(m);
    }

  o->mat = linkwith;

 return;
} /* ay_rrib_linkmaterial */


void
ay_rrib_linktexcoord(ay_object *o)
{
 ay_tag_object *nt = NULL;
 char buf[255], tagname[] = "TC";

  if((AY_EPSILON > fabs(ay_rrib_cattributes->s1)) &&
     (AY_EPSILON > fabs(ay_rrib_cattributes->t1)) &&
     (AY_EPSILON > fabs(ay_rrib_cattributes->s2-1.0f)) &&
     (AY_EPSILON > fabs(ay_rrib_cattributes->t2)) &&
     (AY_EPSILON > fabs(ay_rrib_cattributes->s3)) &&
     (AY_EPSILON > fabs(ay_rrib_cattributes->t3-1.0f)) &&
     (AY_EPSILON > fabs(ay_rrib_cattributes->s4-1.0f)) &&
     (AY_EPSILON > fabs(ay_rrib_cattributes->t4-1.0f)))
    return;

  if(!(nt = calloc(1, sizeof(ay_tag_object))))
    return;

  nt->type = ay_tc_tagtype;

  if(!(nt->name = calloc(strlen(tagname)+1, sizeof(char))))
    {
      free(nt);
      return;
    }

  strcpy(nt->name, tagname);

  sprintf(buf,"%g,%g,%g,%g,%g,%g,%g,%g",
	  ay_rrib_cattributes->s1, ay_rrib_cattributes->t1,
	  ay_rrib_cattributes->s2, ay_rrib_cattributes->t2,
	  ay_rrib_cattributes->s3, ay_rrib_cattributes->t3,
	  ay_rrib_cattributes->s4, ay_rrib_cattributes->t4);

  if(!(nt->val = calloc(strlen(buf)+1, sizeof(char))))
    {
      free(nt->name); free(nt);
      return;
    }

  strcpy(nt->val, buf);

  /* link tag to object */
  nt->next = o->tags;
  o->tags = nt;

 return;
} /* ay_rrib_linktexcoord */


void
ay_rrib_linkobject(void *object, int type)
{
 ay_object *o = NULL, *t = NULL;
 int ay_status = AY_OK;
 char *fname = "ay_rrib_linkobject";

  ay_rrib_co.refine = object;
  ay_rrib_co.type = type;

  ay_rrib_trafotoobject(&ay_rrib_co, ay_rrib_ctrafos->m);

  if(type == AY_IDNPATCH)
    {
      if(ay_rrib_cattributes->trimcurves)
	{
	  ay_rrib_co.down = ay_rrib_cattributes->trimcurves;
	  t = ay_rrib_co.down;
	  while(t->next)
	    t = t->next;
	  ay_status = ay_object_crtendlevel(&(t->next));
	}
    }

  /*if(ay_rrib_readname)*/
  if(ay_rrib_cattributes->identifier_name)
    {
      ay_rrib_co.name = ay_rrib_cattributes->identifier_name;
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

  if(ay_rrib_readmateriali)
    {
      ay_rrib_linkmaterial(o);
      ay_rrib_linktexcoord(o);
    }

  ay_rrib_co.name = NULL;

  if(type == AY_IDNPATCH)
    {
      if(ay_rrib_cattributes->trimcurves)
	{
	  ay_object_delete(t->next);
	  t->next = NULL;
	  ay_rrib_co.down = NULL;
	}
    }

 return;
} /* ay_rrib_linkobject */


int
ay_rrib_printerror(RIB_HANDLE rib, int code, int severity, PRIB_ERROR error)
{
 char buf[255];
 char fname[] = "RRIB";
 char *s = NULL;

  s = RibGetErrorMsg(code);
  if(s && strlen(s) < 200)
    {
      sprintf(buf,"code: %d (%s)", code, s);
    }
  else
    {
      sprintf(buf,"code: %d (no description available)", code);
    }

  switch(severity)
    {
    case 0:
      /* info */
      if(ay_rrib_errorlevel > 2)
	ay_error(AY_EOUTPUT, fname, buf);
      break;
    case 1:
      /* warning */
      if(ay_rrib_errorlevel > 1)
	ay_error(AY_EWARN, fname, buf);
      break;
    case 2:
      /* error */
      if(ay_rrib_errorlevel > 0)
	ay_error(AY_ERROR, fname, buf);
      break;
    case 3:
      /* severe */
      ay_error(AY_ERROR, fname, buf);
      break;
    }

  /* print some more detailed error to stderr */
  RibDefaultErrorHandler(grib, code, severity, error);

  if(severity == 3)
    {
      ay_error(AY_ERROR, fname, "Severe error encountered, bailing out.");
      RibEnd(grib);
    }

 return kRIB_OK;
} /* ay_rrib_printerror */


int
ay_rrib_readrib(char *filename, int frame, int read_camera, int read_options,
		int read_lights, int read_material, int read_partial,
		int error_level)
{
 RIB_HANDLE rib = NULL;
 ay_list_object *tl = NULL;

  /* initialize global variables */
  memset(&ay_rrib_co, '\0', sizeof(ay_object));
  ay_object_defaults(&ay_rrib_co);

  ay_rrib_clighthandle = 1;
  ay_rrib_flobject = NULL;
  ay_rrib_cobjecthandle = 1;
  ay_rrib_objects = NULL;
  ay_rrib_lastobject = NULL;
  ay_rrib_lastmaterialnum = 0;
  ay_rrib_readmateriali = 0;
  ay_rrib_errorlevel = 1;

  /* default fov */
  ay_rrib_fov = 45.0;

  /* default clipping planes */
  ay_rrib_near = 0.0;
  ay_rrib_far = 0.0;

  /* initialize trafo and attribute state stacks */
  ay_rrib_ctrafos = NULL;
  ay_rrib_pushtrafos();
  ay_rrib_cattributes = NULL;
  ay_rrib_pushattribs();

  /* get import options */
  ay_rrib_readframe = frame;
  ay_rrib_readcamera = read_camera;
  ay_rrib_readoptions = read_options;
  ay_rrib_readlights = read_lights;
  ay_rrib_readmaterial = read_material;
  ay_rrib_readpartial = read_partial;
  ay_rrib_errorlevel = error_level;


  gRibNopRITable[kRIB_FRAMEBEGIN] = (PRIB_RIPROC)ay_rrib_RiFrameBegin;
  gRibNopRITable[kRIB_FRAMEEND] = (PRIB_RIPROC)ay_rrib_RiFrameEnd;
  gRibNopRITable[kRIB_DECLARE] = (PRIB_RIPROC)ay_rrib_RiDeclare;

  if(read_partial)
    {
      ay_rrib_initgeneral();
      ay_rrib_initgprims();
      ay_rrib_readmateriali = ay_rrib_readmaterial; 
    }
  else
    {
      if(frame == -1)
	{
	  ay_rrib_initgeneral();
	}
    } /* if */

  if(ay_rrib_readoptions)
    {
      ay_rrib_initoptions();
    }

  rib = RibOpen(filename, kRIB_LAST_RI, gRibNopRITable);

  if(rib)
    {
      grib = (PRIB_INSTANCE)rib;      

      RibSetErrorHandler(rib, ay_rrib_printerror);

      RibRead(rib);

      RibClose(rib);
    }

  /* free temporary objects (if any) */
  while(ay_rrib_objects)
    {
      tl = ay_rrib_objects->next;
      ay_object_deletemulti(ay_rrib_objects->object);
      free(ay_rrib_objects);
      ay_rrib_objects = tl;
    } /* while */

  /* free data from temporary object */
  if(ay_rrib_co.name)
    free(ay_rrib_co.name);
  ay_rrib_co.name = NULL;

  if(ay_rrib_co.tags)
    ay_tags_delall(&ay_rrib_co);
  ay_rrib_co.tags = NULL;

  /* clean up attribute and trafo stacks */
  while(ay_rrib_ctrafos)
    ay_rrib_poptrafos();

  while(ay_rrib_cattributes)
    ay_rrib_popattribs();

 return AY_OK;
} /* ay_rrib_readrib */


int
ay_rrib_readribtcmd(ClientData clientData, Tcl_Interp *interp,
		     int argc, char *argv[])
{
 int ay_status = AY_OK;
 char fname[] = "rrib";
 int frame = 0, read_camera = 1, read_options = 1, read_lights = 1;
 int read_material = 1, read_partial = 0, error_level = 1;
 int i = 2;
 ay_object *o, *n = NULL, **old_aynext;
 ay_level_object *l = NULL;

  if(argc < 2)
    {
      ay_error(AY_EARGS, fname, "filename \\[-f framenumber\\]!");
      return TCL_OK;
    }

  frame = -1;

  while(i+1 < argc)
    {
      if(!strcmp(argv[i], "-f"))
	{
	  sscanf(argv[i+1], "%d", &frame);
	}
      else
      if(!strcmp(argv[i], "-c"))
	{
	  sscanf(argv[i+1], "%d", &read_camera);
	}
      else
      if(!strcmp(argv[i], "-o"))
	{
	  sscanf(argv[i+1], "%d", &read_options);
	}
      else
      if(!strcmp(argv[i], "-l"))
	{
	  sscanf(argv[i+1], "%d", &read_lights);
	}
      else
      if(!strcmp(argv[i], "-m"))
	{
	  sscanf(argv[i+1], "%d", &read_material);
	}
      else
      if(!strcmp(argv[i], "-p"))
	{
	  sscanf(argv[i+1], "%d", &read_partial);
	}
      else
      if(!strcmp(argv[i], "-e"))
	{
	  sscanf(argv[i+1], "%d", &error_level);
	}

	i+=2;
  } /* while */

  /* create Materials level, if it does not exist */
  if(read_material)
    {
      o = ay_root->next;

      if(o->type != AY_IDLEVEL || !o->name || strcmp(o->name,"Materials"))
	{
      
	  if(!(n = calloc(1, sizeof(ay_object))))
	    {
	      ay_error(AY_EOMEM, fname, NULL);
	      return TCL_OK;
	    } /* if */

	  if(!(n->name = calloc(10, sizeof(ay_object))))
	    {
	      free(n);
	      ay_error(AY_EOMEM, fname, NULL);
	      return TCL_OK;
	    } /* if */

	  strcpy(n->name, "Materials");

	  if(!(l = calloc(1, sizeof(ay_level_object))))
	    {
	      free(n->name); free(n);
	      ay_error(AY_EOMEM, fname, NULL);
	      return TCL_OK;
	    } /* if */
	  ay_status = ay_object_crtendlevel(&(n->down));
	  l->type = AY_LTLEVEL;

	  n->type = AY_IDLEVEL;
	  ay_object_defaults(n);
	  n->parent = AY_TRUE;
	  n->refine = l;
      
	  old_aynext = ay_next;
	  ay_next = &(ay_root->next);
	  ay_object_link(n);
	  if(old_aynext)
	    {
	      if(*old_aynext != n)
		ay_next = old_aynext;
	      else
		ay_next = &(n->next);
	    }
	} /* if */
    } /* if */

  ay_status = ay_rrib_readrib(argv[1], frame, read_camera, read_options,
			      read_lights, read_material, read_partial,
			      error_level);
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

#ifndef AYRRIBWRAPPED
  /* source rrib.tcl, it contains Tcl-code for menu entries */
  if((Tcl_EvalFile(interp, "rrib.tcl")) != TCL_OK)
     {
       ay_error(AY_ERROR, fname,
		  "Error while sourcing \\\"rrib.tcl\\\"!");
       return TCL_OK;
     }
#endif

  ay_error(AY_EOUTPUT, fname, "RIB import plugin successfully loaded.");

 return TCL_OK;
} /* Rrib_Init */
