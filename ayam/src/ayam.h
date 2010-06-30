#ifndef __ayam_h__
#define __ayam_h__
/*
 * Ayam, a free 3D modeler for the RenderMan interface.
 *
 * Ayam is copyrighted 1998-2002 by Randolf Schultz
 * (randolf.schultz@gmail.com) and others.
 *
 * All rights reserved.
 *
 * See the file License for details.
 *
 */

/* ayam.h - the main Ayam header */


/* Includes */
#include <limits.h>
#include <float.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#undef WIN32_LEAN_AND_MEAN
#endif /* WIN32 */

#ifdef __cplusplus
 extern "C" {
#endif /* __cplusplus */

#ifdef AYWITHAQUA
  #include <OpenGL/glu.h>
#else
  #include <GL/glu.h>
#endif /* AYWITHAQUA */

/*
#ifdef AYUSEORIGTOGL
*/
#include <togl.h>
/*
#else
#include <aytogl.h>
#endif*/
/* AYUSEORIGTOGL */

#ifdef AYUSEAFFINE
#include <ributil.h>
#endif /* AYUSEAFFINE */

#ifdef AYUSEBMRTRIBOUT
#include <ri.h>
#endif

#ifdef AYUSEAQSISRIB
#include <ri.h>
#endif

#ifdef AYUSESLARGS
#include <sl.h>
#endif /* AYUSESLARGS */

#ifdef AYUSESLCARGS
#include <slc.h>
#endif /* AYUSESLCARGS */

#ifdef AYUSESLOARGS
#include <slo.h>
#endif /* AYUSESLOARGS */

#ifdef AYUSESOARGS
#include <so.h>
#endif /* AYUSESOARGS */

#ifdef AYUSESLXARGS
#include <slx.h>
#endif /* AYUSESLXARGS */

#define AYGLUCBTYPE

#ifdef WIN32
 #undef AYGLUCBTYPE
 #define AYGLUCBTYPE (GLUnurbsErrorProc)
 #ifdef AYMESASGIGLU
  #undef AYGLUCBTYPE
  #define AYGLUCBTYPE (_GLUfuncptr)
 #endif
 #ifdef AYUSESUPERGLU
  #undef AYGLUCBTYPE
  #define AYGLUCBTYPE
 #endif
#endif


/* Ayam Object Structure */

/** Ayam object */
typedef struct ay_object_s {
  struct ay_object_s *next;  /**< next object in same hierarchie-level */
  struct ay_object_s *down;  /**< children of this object */

  /** the type of the object (AY_ID*) */
  unsigned int type;

  /** the name of the object */
  char *name;

  /** a "name" for OpenGL selection */
  unsigned int glname;

  /** how many references of this object exist? */
  unsigned int refcount;

  /** is this object currently selected? */
  int selected;

  /** is this object modified by an editing action? */
  int modified;

  /** does this object allow children? */
  int parent;

  /** do children inherit the transformation attributes? */
  int inherit_trafos;

  /* Visibility */
  /** is this object hidden? */
  int hide;
  /** should the children of this object be hidden? */
  int hide_children;

  /** \name translation attributes */
  /*@{*/
  double movx, movy, movz;
  /*@}*/

  /** \name orientation attributes */
  /*@{*/
  double rotx, roty, rotz;
  /*@}*/

  /** \name scale attributes */
  /*@{*/
  double scalx, scaly, scalz;
  /*@}*/

  double quat[4]; /**< quaternion attribute */

#if 0
  struct ay_trafo_s *trafo; /**< transformations of this object */
#endif
  struct ay_point_s *selp; /**< selected points of this object */

  struct ay_tag_s *tags; /**< tags of this object */

  struct ay_mat_object_s *mat; /**< material of this object */

  void *refine; /**< type specific object (e.g. ay_sphere_object) */
} ay_object;


/** Ayam object list element */
typedef struct ay_list_object_s
{
  struct ay_list_object_s *next; /**< next list element */
  ay_object *object; /**< Ayam object */
} ay_list_object;


/* Shaders */

/** RenderMan shader parameter */
typedef struct ay_shader_arg_s
{
  struct ay_shader_arg_s *next; /**< next parameter */
  char *name; /**< name of parameter */
  int type; /**< type of parameter (AY_SA*)
	      (color point vector scalar string) */
  union {
    float color[3];
    float point[4];
    float matrix[16];
    float scalar;
    char *string;
  } val; /**< value of parameter */
} ay_shader_arg;


/** RenderMan shader */
typedef struct ay_shader_s
{
  struct ay_shader_s *next; /**< next shader */
  int type; /**< shader type (AY_ST*)
	      (light volume surface displacement transformation) */
  char *name; /**< name of shader */
  ay_shader_arg *arg; /**< shader parameters */
} ay_shader;


/** Material Object */
typedef struct ay_mat_object_s {

  int registered; /**< is this material unique? */

  char **nameptr; /**< pointer to name (stored in corresponding ay_object) */
  unsigned int *refcountptr; /**< pointer to reference counter */
  ay_object *objptr; /**< pointer to corresponding ay_object */

  /* RiStandard (3.1) Attributes */

  /** \name Color */
  /*@{*/
  int colr, colg, colb;
  /*@}*/

  /** \name Opacity */
  /*@{*/
  int opr, opg, opb;
  /*@}*/

  /* Matte */
  int matte; /**< matte: 0 - no, 1 - yes */

  /* Shading */
  double shading_rate; /**< shading rate */
  int shading_interpolation; /**< interpolation: 0 - constant, 1 - smooth */

  /* Displacement */
  double dbound_val; /**< displacement bound value */
  int dbound; /**< coordinate system for bound value */

  /* Sidedness */
  int sides; /**< sidedness: 0 - two-sided, 1 - one-sided */

  /* Shaders */
  /** surface shader */
  ay_shader *sshader;
  /** displacement shader */
  ay_shader *dshader;
  /** interior shader */
  ay_shader *ishader;
  /** exterior shader */
  ay_shader *eshader;

  /* BMRT Specific Attributes */
   /* Radiosity */
    /* Average Color */
    int avr, avg, avb, ava;
    /* Emitted Color */
    int emr, emg, emb, ema;
    /* Specular Color */
    int spr, spg, spb, spa;
    /* Meshing */
    double patch_size, elem_size, min_size;
    /* Calculation */
    int zonal; /* dontset, none, zonal_receives, zonal_shoots, full_zonal */
    /* Caustics */
    int has_caustics; /* no, yes */

   /* Shadows */
   int cast_shadows; /* Os, none, opaque, surface */

   /* Displacements */
   int true_displacement; /* no, yes */

   /* Visibility */
   int camera; /* yes, no */
   int reflection; /* yes, no */
   int shadow; /* yes, no */

} ay_mat_object;

/** RenderMan interface options */
typedef struct ay_riopt_s
{
  double Variance;
  double Samples_X;
  double Samples_Y;
  char FilterFunc;
  double FilterWidth;
  double FilterHeight;
  double ExpGain;
  double ExpGamma;
  double RGBA_ONE;
  double RGBA_MIN;
  double RGBA_MAX;
  double RGBA_Dither;

  int MinSamples;
  int MaxSamples;
  int MaxRayLevel;
  double ShadowBias;
  char PRManSpec;
  int RadSteps;
  int PatchSamples;

  char *textures;
  char *shaders;
  char *archives;
  char *procedurals;

  int texturemem;
  int geommem;

  int width;
  int height;

  int use_std_display;
} ay_riopt;


/** Root object */
typedef struct ay_root_object_s
{
  ay_riopt  *riopt; /**< RenderMan interface options of the current scene */
  ay_shader *imager; /**< imager shader */
  ay_shader *atmosphere; /**< atmosphere shader */
} ay_root_object;


/** NURBS curve object */
typedef struct ay_nurbcurve_object_s
{
  int type; /**< curve type (AY_CTOPEN, AY_CTCLOSED, AY_CTPERIODIC) */
  int length; /**< curve length */
  int order; /**< curve order */
  int knot_type; /**< knot type (AY_KT*) */
  int is_rat; /**< is any weight != 1.0 */
  double *controlv; /**< control points [length*4] */
  double *knotv; /**< knot vector [length+order]*/

  double glu_sampling_tolerance;
  int display_mode; /**< drawing mode */

  GLUnurbsObj *no; /**< GLU NURBS object */

  /* stess */
  int tesslen;
  double *tessv;
  int tessqf;

  /* multiple points */
  int createmp;
  struct ay_mpoint_s *mpoints;
} ay_nurbcurve_object;


/* a tesselated NURBS patch point */
typedef struct ay_stess_uvp_s {
  struct ay_stess_uvp_s *next;
  int type;    /* 0 - original point, 1 - trimloop point */
  int dir;     /* direction of associated trimcurve, 0 - cw, 1 - ccw */
  double u, v; /* associated parametric values of this point */
  double C[6]; /* geometric coordinates and normal of this point */
} ay_stess_uvp;


/* a complete tesselation */
typedef struct ay_stess_s {
  /* untrimmed patch */
  int tessw, tessh;
  double *tessv; /* [tessw*tessh] */

  /* trimmed patch */
  /* number of arrays of tesselated points */
  int upslen, vpslen;
  /* arrays of lists of tesselated points */
  ay_stess_uvp **ups, **vps; /* [upslen], [vpslen] */
  /* first trim is oriented clockwise */
  int ft_cw;
  /* number of tesselated trim curves */
  int tcslen;
  /* tesselated trim curves */
  double **tcs; /* [tcslen][tcslens[i]] */
  /* length and direction of trim curves */
  int *tcslens, *tcsdirs; /* [tcslen] */
  double ud, vd;
} ay_stess;


/** NURBS patch object */
typedef struct ay_nurbpatch_object_s
{
  int width, height;
  int uorder, vorder;
  int uknot_type; /* AY_KTBEZIER, AY_KTBSPLINE, AY_KTNURB, AY_KTCUSTOM */
  int vknot_type; /* AY_KTBEZIER, AY_KTBSPLINE, AY_KTNURB, AY_KTCUSTOM */
  /*int closedu, closedv;*/ /* unused */
  int is_rat;

  double *controlv; /**< control points [width*height*4] */
  double *uknotv;
  double *vknotv;
  /*double *texv;*/ /* unused */

  /* GLU */
  GLUnurbsObj *no;

  double glu_sampling_tolerance;
  int display_mode;

  /* stess */
  int tessqf;
  ay_stess *stess;

  /* multiple points */
  int createmp;
  struct ay_mpoint_s *mpoints;
} ay_nurbpatch_object;


/** PatchMesh object */
typedef struct ay_pamesh_object_s {
  int width, height;
  int close_u, close_v;
  double *controlv; /**< control points [width*height*4] */
  int type; /* AY_PTBILINEAR, AY_PTBICUBIC */
  int btype_u; /* AY_BTBEZIER, AY_BTBSPLINE, AY_BTCATMULLROM, AY_BTHERMITE,
		  AY_BTCUSTOM */
  int btype_v; /* AY_BTBEZIER, AY_BTBSPLINE, AY_BTCATMULLROM, AY_BTHERMITE,
		  AY_BTCUSTOM */
  int ustep;
  double *ubasis; /* [16], only in use for btype_u == AY_BTCUSTOM */
  int vstep;
  double *vbasis; /* [16], only in use for btype_v == AY_BTCUSTOM */

  /** cached NURBS patch representation */
  ay_object *npatch;

  double glu_sampling_tolerance;
  int display_mode;
} ay_pamesh_object;


/** PolyMesh object */
typedef struct ay_pomesh_object_s {
  int type; /**< unused */

  unsigned int npolys; /**< total number of polygons */
  unsigned int *nloops; /**< loops per polygon [npolys] */
  unsigned int *nverts; /**< verts per loop[<sum of all elements of nloops>] */
  unsigned int *verts; /**< [<sum of all elements of nverts>] */

  unsigned int ncontrols; /**< total number of control points */
  int has_normals; /**< vertex normals? 0 - No, stride=3; 1 - Yes, stride=6 */
  double *controlv; /**< control points [ncontrols * stride] */
} ay_pomesh_object;


/** SubdivisionMesh object */
typedef struct ay_sdmesh_object_s {
  int scheme; /**< subdivision scheme (AY_SDSCATMULL, AY_SDSLOOP) */

  unsigned int nfaces; /**< total number of faces */
  unsigned int *nverts; /**< number of vertices per face [nfaces] */
  unsigned int *verts; /**< vertex indices [<sum of all elements of nverts>] */
  unsigned int ntags; /**< total number of tags */
  int *tags; /**< [ntags] (AY_SDTHOLE, AY_SDTCORNER, AY_SDTCREASE, AY_SDTIB) */
  unsigned int *nargs; /**< number of arguments per tag [ntags * 2] */
  int *intargs; /**< integer args [<sum of all even elements of nargs>] */
  double *floatargs; /**< float args [<sum of all uneven elements of nargs>] */

  unsigned int ncontrols; /**< total number of control points */
  double *controlv; /**< control points [ncontrols * 3] */
} ay_sdmesh_object;


/** Gordon object */
typedef struct ay_gordon_object_s {
  int wcc; /**< watch (and automatically correct) parameter curves? */
  int uorder; /**< desired order for u dimension */
  int vorder; /**< desired order for v dimension */

  /** cached caps and bevel objects */
  ay_object *caps_and_bevels;

  /** cached NURBS patch representation */
  ay_object *npatch;

  double glu_sampling_tolerance;
  int display_mode;
} ay_gordon_object;


/** Text object */
typedef struct ay_text_object_s
{
  char *fontname; /**< filename of TrueType font description file */
  Tcl_UniChar *unistring; /**< UNICODE string of text */
  double height; /**< height of letters (Z) */
  int revert; /**< revert curves (to fix trim problems)? */
  int has_upper_cap; /**< create upper cap? */
  int has_lower_cap; /**< create lower cap? */

  /** cached NURBS patch representation */
  ay_object *npatch;

  double glu_sampling_tolerance;
  int display_mode;
} ay_text_object;


/** Lightsource object */
typedef struct ay_light_object_s
{
  int type; /**< light type: custom, point, spot, or distant (AY_LT*) */
  int on; /**< light state: 0 off, 1 on */
  int local; /**< light only local objects (same level and below)? */
  RtLightHandle light_handle; /**< RI handle of local lights */
  int shadows; /**< light casts shadows? */
  int samples; /**< number of samples (for area lights), default: 1 */
  int colr, colg, colb;
  double intensity; /**< intensity of light source */
  double cone_angle; /**< size of spot light cone */
  double cone_delta_angle; /**< size of spot light penumbra */
  double beam_distribution; /**< intensity distribution in spot light beam */
  int use_sm; /**< create shadow map? */
  int sm_resolution; /**< shadow map resolution */
  ay_shader *lshader; /**< light shader (for custom lights only!) */
  double tfrom[3]; /**< light source position */
  double tto[3]; /**< light source aim point */
} ay_light_object;


/** Level object */
typedef struct ay_level_object_s
{
  int type; /**< type of level (AY_LT*) */
} ay_level_object;


/** Box object */
typedef struct ay_box_object_s
{
  double width; /**< width of box (X) */
  double length; /**< length of box (Z) */
  double height; /**< height of box (Y) */
} ay_box_object;


/** Bilinear patch object */
typedef struct ay_bpatch_object_s
{
  double p1[3]; /**< point 1 */
  double p2[3]; /**< point 2 */
  double p3[3]; /**< point 3 */
  double p4[3]; /**< point 4 */
} ay_bpatch_object;


/** Sphere object */
typedef struct ay_sphere_object_s
{
  char closed; /**< create missing cap surfaces? */
  char is_simple; /**< is thetamax 360.0 and are zmin/zmax >= radius? */
  double radius; /**< radius of sphere */
  double zmin; /**< delimit sphere on z axis */
  double zmax; /**< delimit sphere on z axis */
  double thetamax; /**< angle of revolution (degrees) */
} ay_sphere_object;


/** Cone object */
typedef struct ay_cone_object_s
{
  char closed; /**< create missing cap surfaces? */
  char is_simple; /**< is thetamax 360.0? */
  double radius; /**< radius of cone */
  double height; /**< height of cone */
  double thetamax; /**< angle of revolution (degrees) */
} ay_cone_object;


/** Disk object */
typedef struct ay_disk_object_s
{
  char is_simple; /**< is thetamax 360.0? */
  double radius; /**< radius of disk */
  double height; /**< displacement of disk on z axis */
  double thetamax; /**< angle of revolution (degrees) */
} ay_disk_object;


/** Cylinder object */
typedef struct ay_cylinder_object_s
{
  char closed; /**< create missing cap surfaces? */
  char is_simple; /**< is thetamax 360.0? */
  double radius; /**< radius of cylinder */
  double zmin; /**< delimit cylinder on z axis */
  double zmax; /**< delimit cylinder on z axis */
  double thetamax; /**< angle of revolution (degrees) */
} ay_cylinder_object;


/** Hyperboloid object */
typedef struct ay_hyperboloid_s
{
  char closed; /**< create missing cap surfaces? */
  double p1[3]; /**< point 1 */
  double p2[3]; /**< point 2 */
  double thetamax; /**< angle of revolution (degrees) */
} ay_hyperboloid_object;


/** Paraboloid object */
typedef struct ay_paraboloid_object_s
{
  char closed; /**< create missing cap surfaces? */
  double rmax; /**< radius on base */
  double zmin; /**< delimit paraboloid on z axis */
  double zmax; /**< delimit paraboloid on z axis */
  double thetamax; /**< angle of revolution (degrees) */
} ay_paraboloid_object;


/** Torus object */
typedef struct ay_torus_object_s
{
  char closed; /**< create missing cap surfaces? */
  double majorrad, minorrad;
  double phimin, phimax;
  double thetamax; /**< angle of revolution (degrees) */
} ay_torus_object;


/** Interpolating curve object */
typedef struct ay_icurve_object_s
{
  int type; /**< interpolation mode (C2-cubic or global) */
  int length; /**< number of data points */
  int order; /**< desired order of NURBS curve */
  int derivs; /**< have end derivatives? */
  int param_type;  /**< parameterization (chordal or centripetal) */
  double sdlen; /**< start derivative length */
  double edlen; /**< end derivative length */

  double *controlv; /**< data points [length*3] */
  double sderiv[3]; /**< start derivative */
  double ederiv[3]; /**< end derivative */

  /** cached NURBS curve representation */
  ay_object *ncurve;

  double glu_sampling_tolerance;
  int display_mode;
} ay_icurve_object;


/** Approximating curve object */
typedef struct ay_acurve_object_s
{
  int length; /**< number of data points */
  int alength; /**< desired number of NURBS control points */
  int closed; /**< create closed curve? */
  int symmetric; /**< create symmetric curve? */
  int order; /**< desired order of NURBS curve */

  double *controlv; /**< data points [length*3] */

  /** cached NURBS curve representation */
  ay_object *ncurve;

  double glu_sampling_tolerance;
  int display_mode;
} ay_acurve_object;


/** Concatenate curves object */
typedef struct ay_concatnc_object_s
{
  int closed; /**< create closed curve? */
  int fillgaps; /**< create fillets? */
  int revert; /**< revert created curve? */
  int knot_type; /**< knot type of created curve */
  double ftlength; /**< length of fillet end tangents */

  /** cached NURBS curve representation */
  ay_object *ncurve;

  double glu_sampling_tolerance;
  int display_mode;
} ay_concatnc_object;


/** Offset curves object */
typedef struct ay_offnc_object_s
{
  int mode; /**< offset mode */
  int revert; /**< revert created curve? */
  double offset; /**< offset/distance value */

  /** cached NURBS curve representation */
  ay_object *ncurve;

  double glu_sampling_tolerance;
  int display_mode;
} ay_offnc_object;


/** Cap surface object */
typedef struct ay_cap_object_s
{
  int type; /**< cap type (0 - trim, 1 - gordon) */

  /** cached NURBS patch representation */
  ay_object *npatch;

  double glu_sampling_tolerance;
  int display_mode;
} ay_cap_object;


/** Bevel surface object */
typedef struct ay_bevel_object_s
{
  int has_cap; /**< add cap surface? */

  /** cached NURBS patch representation */
  ay_object *npatch;

  double glu_sampling_tolerance;
  int display_mode;
} ay_bevel_object;


/** Clone object */
typedef struct ay_clone_object_s
{
  int numclones; /**< number of clones to create */
  int rotate; /**< rotate clones (perpendicular to trajectory)? */
  int mirror; /**< enable mirror mode? (0 - no, 1,2,3 - yes (YZ,XZ,XY)) */

  /* transformations */
  double movx, movy, movz;
  double rotx, roty, rotz;
  double scalx, scaly, scalz;
  double quat[4]; /* quaternion */

  /** cached clones */
  ay_object *clones;
} ay_clone_object;


/** Camera object */
typedef struct ay_camera_object_s
{
  double from[3]; /**< viewpoint */
  double to[3]; /**< aim point */
  double up[3]; /**< up vector */
  double roll; /**< roll angle */
  double zoom; /**< zoom factor */
  double nearp; /**< near clipping plane */
  double farp; /**< far clipping plane */
} ay_camera_object;


/** RenderMan Interface Bytestream include object */
typedef struct ay_riinc_object_s
{
  double width, length, height;
  char *file; /**< filename of include file */
} ay_riinc_object;


/** RenderMan Interface procedural object */
typedef struct ay_riproc_object_s
{
  int type;
  double minx, miny, minz, maxx, maxy, maxz;
  char *file;
  char *data;
} ay_riproc_object;


/** Surface of revolution object */
typedef struct ay_revolve_object_s
{
  double thetamax; /**< angle of revolution (degrees) */
  int sections; /**< number of sections in u direction */
  int order; /**< desired order in u direction */
  int has_upper_cap; /**< create upper cap? */
  ay_object *upper_cap; /**< cached upper cap */
  int has_lower_cap; /**< create lower cap? */
  ay_object *lower_cap; /**< cached lower cap */
  int has_start_cap; /**< create start cap (at theta 0)? */
  ay_object *start_cap; /**< cached start cap */
  int has_end_cap; /**< create end cap (at thetamax)? */
  ay_object *end_cap; /**< cached end cap */

  /** cached NURBS patch representation */
  ay_object *npatch;

  double glu_sampling_tolerance;
  int display_mode;
} ay_revolve_object;


/** Extrusion surface object */
typedef struct ay_extrude_object_s
{
  double height; /**< height of extrusion */
  int has_upper_cap; /**< create upper cap? */
  int has_lower_cap; /**< create lower cap? */

  /** cached caps and bevel objects */
  ay_object *caps_and_bevels;

  /** cached NURBS patch representation */
  ay_object *npatch;

  double glu_sampling_tolerance;
  int display_mode;
} ay_extrude_object;


/** Swept surface object */
typedef struct ay_sweep_object_s
{
  int rotate; /**< rotate sections (perpendicular to trajectory)? */
  int interpolate; /**< interpolate sections? */
  int close; /**< create periodic (closed) surface? */
  int sections; /**< desired number of sections (0 - automatic) */
  int has_start_cap; /**< create start cap? */
  int has_end_cap; /**< create end cap? */

  /** cached caps and bevel objects */
  ay_object *caps_and_bevels;

  /** cached NURBS patch representation */
  ay_object *npatch;

  double glu_sampling_tolerance;
  int display_mode;
} ay_sweep_object;


/** Swung surface object */
typedef struct ay_swing_object_s
{
  int has_upper_cap; /**< create upper cap? */
  ay_object *upper_cap; /**< cached upper cap */
  int has_lower_cap; /**< create lower cap? */
  ay_object *lower_cap; /**< cached lower cap */
  int has_start_cap; /**< create start cap? */
  ay_object *start_cap; /**< cached start cap */
  int has_end_cap; /**< create end cap? */
  ay_object *end_cap; /**< cached end cap */

  /** cached NURBS patch representation */
  ay_object *npatch;

  double glu_sampling_tolerance;
  int display_mode;
} ay_swing_object;


/** Birail surface object (from three curves) */
typedef struct ay_birail1_object_s
{
  int close; /**< unused */
  int sections; /**< number of sections in the birailed surface (U) */
  int has_start_cap; /**< create start cap? */
  int has_end_cap; /**< create end cap? */

  /** cached caps and bevel objects */
  ay_object *caps_and_bevels;

  /** cached NURBS patch representation */
  ay_object *npatch;

  double glu_sampling_tolerance;
  int display_mode;
} ay_birail1_object;


/** Birail surface object (from four curves) */
typedef struct ay_birail2_object_s
{
  int close; /**< unused */
  int sections; /**< number of sections in the birailed surface (U) */
  int interpolctrl;  /**< use interpolation control curve? */
  int has_start_cap; /**< create start cap? */
  int has_end_cap; /**< create end cap? */

  /** cached caps and bevel objects */
  ay_object *caps_and_bevels;

  /** cached NURBS patch representation */
  ay_object *npatch;

  double glu_sampling_tolerance;
  int display_mode;
} ay_birail2_object;


/** Skinned surface object (Loft) */
typedef struct ay_skin_object_s
{
  int interpolate; /**< interpolate all curves? */
  int uorder; /**< desired order in u direction */
  int uknot_type; /**< desired knot type in u direction */
  double uknotv; /**< desired knot vector in u direction */
  int has_start_cap; /**< create start cap? */
  int has_end_cap; /**< create end cap? */

  /** cached caps and bevel objects */
  ay_object *caps_and_bevels;

  /** cached NURBS patch representation */
  ay_object *npatch;

  double glu_sampling_tolerance;
  int display_mode;
} ay_skin_object;


/** Extract curve from surface object */
typedef struct ay_extrnc_object_s
{
  int side; /**< which curve to extract? (0,1,2,3 - boundary; 4,5 - inner) */
  int pnum; /**< select patch from multiple provided */
  int revert; /**< revert extracted curve? */
  int relative; /**< interpret parameter value in relative way? */
  double parameter; /**< parameter value (u/v) */

  /** cached NURBS curve representation */
  ay_object *ncurve;

  double glu_sampling_tolerance;
  int display_mode;
} ay_extrnc_object;


/** Extract surface from surface object */
typedef struct ay_extrnp_object_s
{
  int pnum; /**< select patch from multiple provided */
  int relative; /**< interpret parameter values in relative way? */
  double umin, umax, vmin, vmax;

  /** cached NURBS patch representation */
  ay_object *npatch;

  double glu_sampling_tolerance;
  int display_mode;
} ay_extrnp_object;


/** Concatenate surfaces object */
typedef struct ay_concatnp_object_s
{
  int type;
  int revert;
  int knot_type;

  int fillgaps;
  double ftlength;

  /** cached NURBS patch representation */
  ay_object *npatch;

  double glu_sampling_tolerance;
  int display_mode;
} ay_concatnp_object;


/** Offset surface object */
typedef struct ay_offnp_object_s
{
  int mode;
  double offset;

  /** cached NURBS patch representation */
  ay_object *npatch;

  double glu_sampling_tolerance;
  int display_mode;
} ay_offnp_object;


/** Circle object */
typedef struct ay_ncircle_object_s
{
  double radius; /**< radius of circle/arc */
  double tmin; /**< start angle of circle/arc */
  double tmax; /**< end angle of circle/arc */

  /** cached NURBS curve representation */
  ay_object *ncurve;

  double glu_sampling_tolerance;
  int display_mode;
} ay_ncircle_object;


/** Script object */
typedef struct ay_script_object_s
{
  char *script; /**< the script text (Tcl) */
  int active; /**< activate/run the script? (0 - Inactive, 1 - Active) */
  int type; /**< type of script (0 - Run, 1 - Create, 2 - Modify) */
  ay_object *cm_objects; /**< created or modified objects */

  int modified; /**< need to recompile the script? */
  Tcl_Obj *cscript; /**< cache compiled script */

  int paramslen; /**< number of saved script parameters */
  Tcl_Obj **params; /**< save script parameters */
} ay_script_object;


/** View object */
typedef struct ay_view_object_s
{
  struct Togl *togl; /**< pointer to corresponding Togl widget structure */
  int type; /**< view type (AY_VT*) (Persp., Front, Side, Top, Trim) */
  double grid; /**< gridsize, 0.0 == no grid */

  int local; /* editing takes place in local space, not world space? */
  int aligned; /* view is aligned to object-space of selected object? */

  int drawsel; /**< draw selected objects (and their children) only? */
  int drawlevel; /**< draw current level (and below) only? */
  int redraw; /**< automatic redraw? */
  int drawgrid; /**< draw grid? */
  int usegrid; /**< use (snap to) grid? */
  int shade; /**< shade view? */
  int antialiaslines; /**< use antialiasing for line drawing? */

  /*#ifdef AY_ENABLEPPREV*/
  int ppreview; /**< create a permanent preview? */
  /*#endif*/

  int drawobjectcs; /**< draw object coordinate system? */

  /* camera */
  double from[3]; /**< viewpoint */
  double to[3]; /**< aim point */
  double up[3]; /**< up vector */
  double roll; /**< roll angle */
  double zoom; /**< zoom factor */
  double nearp; /**< near clipping plane */
  double farp; /**< far clipping plane */

  /* temporarily in use for rotation with cursor keys */
  double rotx, roty, rotz;

  /* how to get from win to world coordinates (in parallel views!) */
  double conv_x, conv_y;

  /* rectangle, currently used to draw
     a rectangle while drag-selection of points */
  double rect_xmin, rect_xmax, rect_ymin, rect_ymax;
  int drawrect;

  /* mark a point in space */
  double markworld[3];
  double markx;
  double marky;
  int drawmark;

  /* position of the view window on the screen */
  int pos_x, pos_y;

  /* is the view window iconified? */
  int isicon;

  /* is a modelling action active that needs to display handles? */
  int drawhandles;

  /* need to call reshape before drawing? */
  int dirty;

  /* background image */
  char *bgimage;
  int bgimagedirty;
  int drawbgimage;

  /* geometry for background image */
  int bgwidth, bgheight;
  int bguorder, bgvorder;
  float *bgknotv, *bgcv;

  /** unique identifier, for plugins (e.g. AyCSG) that tie
     exclusive resources (e.g. offscreen buffers) to views */
  int id;

  /** alternative display callback, for plugins that like to take
     over drawing (e.g. AyCSG) */
  Togl_Callback *altdispcb;
} ay_view_object;


/** Select object */
typedef struct ay_select_object_s
{
  char *indices; /**< indices of objects to be selected */
  int length; /**< length of seli array */
  int *seli; /**< cache of object indices */
} ay_select_object;


/** Trim surface object */
typedef struct ay_trim_object_s
{
  int patchnum; /**< select patch from multiple provided */

  /** cached NURBS patch representation */
  ay_object *npatch;
} ay_trim_object;


/** User preferences */
typedef struct ay_preferences_s
{
  int list_types;
  int mark_hidden;
  int single_window;

  /* modelling prefs */
  int edit_snaps_to_grid;
  double pick_epsilon;
  int lazynotify;
  int completenotify;
  int undo_levels;
  int snap3d;
  int globalmark;

  /* RIB export prefs */
  int resolveinstances;
  int checklights;
  int ristandard;
  int use_sm;
  int defaultmat; /* 0 no, 1 matte, 2 "default" */
  int writeident;
  int excludehidden;

  /* Mops Import prefs */
  int mopsiresetdisplaymode;
  int mopsiresettolerance;

  /* drawing */
  double handle_size; /* size of points */

  double bgr, bgg, bgb; /* background color */
  double obr, obg, obb; /* object color */
  double ser, seg, seb; /* selection color */
  double grr, grg, grb; /* grid color */
  double tpr, tpg, tpb; /* tagged points color */
  double shr, shg, shb; /* default shade color */
  double lir, lig, lib; /* default light color */
  double sxr, sxg, sxb; /* default drag selection rectangle color (XOR) */

  int use_materialcolor;

  double linewidth;
  double sellinewidth;

  /* error handling */
  char onerror; /* 0 stop, 1 continue */
  int writelog;
  char *logfile;

  /* state of the RIB exporter */
  int wrib_sm;
  int wrib_em;
  int wrib_archives;

  /* rendering quality */
  double glu_sampling_tolerance;
  int np_display_mode;
  int nc_display_mode;
  int glu_cache_float; /* unused */

  int stess_qf;

  /* sampling mode/quality for NURBS -> PolyMesh conversion */
  int smethod;
  double sparamu;
  double sparamv;

  /* control warnings about unknown tag types */
  int wutag;

  /* parameters for glPolygonOffset */
  double polyoffset0;
  double polyoffset1;

  /* save root & views with the currently open scene? */
  int save_rootviews;

  /* is a permanent preview window open? */
  int pprev_open;
  char *pprender;

  /* PV tag names */
  char *texcoordname;
  char *normalname;
  char *colorname;

} ay_preferences;


/** selected points */
typedef struct ay_point_s
{
  struct ay_point_s *next;
  int homogenous; /* AY_TRUE, AY_FALSE */
  double *point;
  unsigned int index;
} ay_point;


/** multiple points */
typedef struct ay_mpoint_s
{
  struct ay_mpoint_s *next;
  int multiplicity;
  double **points;
  unsigned int *indices;
} ay_mpoint;


/** point edit helper */
typedef struct ay_pointedit_s
{
  unsigned int num;
  double **coords;
  unsigned int *indices;
  int homogenous;
} ay_pointedit;


/** Tag, attach arbitrary information to objects */
typedef struct ay_tag_s
{
  struct ay_tag_s *next;
  char *name;
  char *type;
  char *val;
} ay_tag;


/** transformation attributes */
typedef struct ay_trafo_s
{
  double movx, movy, movz;
  double rotx, roty, rotz;
  double scalx, scaly, scalz;
  double quat[4]; /* quaternion */
} ay_trafo;


/* avoid the use of "void *" to store function pointers */
typedef void (*ay_voidfp)(void);

/** callback table */
typedef struct ay_ftable_s
{
  unsigned int size;
  ay_voidfp *arr;
} ay_ftable;

/* Callbacks */

/** Create callback, think constructor */
typedef int (ay_createcb) (int argc, char *argv[], ay_object *o);

/** Delete callback, think destructor */
typedef int (ay_deletecb) (void *c);

/** Copy callback, think copy constructor */
typedef int (ay_copycb) (void *src, void **dst);

/** Draw callback */
typedef int (ay_drawcb) (struct Togl *togl,  ay_object *o);

/** Get/Set properties callback */
typedef int (ay_propcb) (Tcl_Interp *interp, int argc, char *argv[],
			 ay_object *o);

/** Select editable points callback */
typedef int (ay_getpntcb) (int mode, ay_object *o, double *p,
			   ay_pointedit *pe);

/** RIB export callback */
typedef int (ay_wribcb) (char *file, ay_object *o);

/** Read (from Ayam scene file) callback */
typedef int (ay_readcb) (FILE *fileptr, ay_object *o);

/** Write (to Ayam scene file) callback */
typedef int (ay_writecb) (FILE *fileptr, ay_object *o);

/** Notification (update after changes to children) callback  */
typedef int (ay_notifycb) (ay_object *o);

/** Tree drop (update after Drag and Drop operation in TreeView) callback */
typedef int (ay_treedropcb) (ay_object *o);

/** Compare callback */
typedef int (ay_comparecb) (ay_object *o1, ay_object *o2);

/** Convert callback */
typedef int (ay_convertcb) (ay_object *o, int in_place);

/** Provide callback */
typedef int (ay_providecb) (ay_object *o, unsigned int type,
			    ay_object **result);

/** Calculate bounding box callback */
typedef int (ay_bbccb) (ay_object *o, double *bbox, int *flags);


/* Globals */

/** Main Ayam Tcl interpreter */
extern Tcl_Interp *ay_interp;

/** Safe Tcl interpreter (e.g. for Script object scripts */
extern Tcl_Interp *ay_safeinterp;

/** user preferences */
extern ay_preferences ay_prefs;

/** pointer to the root object */
extern ay_object *ay_root;

/** pointer to pointer (to some objects ->next or ->down slot) where
    the next object will be linked to */
extern ay_object **ay_next;

/** current view */
extern ay_view_object *ay_currentview;

/** current object selection */
extern ay_list_object *ay_selection;

/** current level */
extern ay_list_object *ay_currentlevel;

/** object clipboard */
extern ay_object *ay_clipboard;

/** table of registered object types */
extern Tcl_HashTable ay_otypesht;

/** table of registered object type names */
extern Tcl_HashTable ay_typenamesht;

/* function pointer tables (object callbacks) */
/** all registered create callbacks */
extern ay_ftable ay_createcbt;
/** all registered delete callbacks */
extern ay_ftable ay_deletecbt;
/** all registered copy callbacks */
extern ay_ftable ay_copycbt;
/** all registered draw callbacks */
extern ay_ftable ay_drawcbt;
/** all registered draw handles callbacks */
extern ay_ftable ay_drawhcbt;
/** all registered shade callbacks */
extern ay_ftable ay_shadecbt;
/** all registered get properties callbacks */
extern ay_ftable ay_getpropcbt;
/** all registered set properties callbacks */
extern ay_ftable ay_setpropcbt;
/** all registered get points callbacks */
extern ay_ftable ay_getpntcbt;
/** all registered RIB export callbacks */
extern ay_ftable ay_wribcbt;
/** all registered read callbacks */
extern ay_ftable ay_readcbt;
/** all registered write callbacks */
extern ay_ftable ay_writecbt;
/** all registered notify callbacks */
extern ay_ftable ay_notifycbt;
/** all registered bounding box calculation callbacks */
extern ay_ftable ay_bbccbt;

/** all registered tree drop callbacks */
extern ay_ftable ay_treedropcbt;

/** table of registered tag types */
extern Tcl_HashTable ay_tagtypesht;

/** table of temporary tag types */
extern Tcl_HashTable ay_temptagtypesht;

/** all registered conversion callbacks */
extern ay_ftable ay_convertcbt;

/** all registered provide callbacks */
extern ay_ftable ay_providecbt;

/** global error number */
extern int ay_errno;

extern ay_object *ay_last_read_object;

extern int ay_read_version;

extern int ay_read_viewnum;

/** major Ayam version number */
extern char ay_version_ma[];
/** minor Ayam version number */
extern char ay_version_mi[];

/* internal tags */
extern char *ay_oi_tagtype;
extern char *ay_oi_tagname;
extern char *ay_riattr_tagtype;
extern char *ay_riattr_tagname;
extern char *ay_riopt_tagtype;
extern char *ay_riopt_tagname;
extern char *ay_tc_tagtype;
extern char *ay_tc_tagname;
extern char *ay_pv_tagtype;
extern char *ay_pv_tagname;
extern char *ay_ridisp_tagtype;
extern char *ay_ridisp_tagname;
extern char *ay_rihider_tagtype;
extern char *ay_rihider_tagname;
extern char *ay_noexport_tagtype;
extern char *ay_noexport_tagname;
extern char *ay_tp_tagtype;
extern char *ay_tp_tagname;
extern char *ay_bns_tagtype;
extern char *ay_bns_tagname;
extern char *ay_ans_tagtype;
extern char *ay_ans_tagname;
extern char *ay_dbns_tagtype;
extern char *ay_dbns_tagname;
extern char *ay_dans_tagtype;
extern char *ay_dans_tagname;
extern char *ay_umm_tagtype;
extern char *ay_umm_tagname;
extern char *ay_vmm_tagtype;
extern char *ay_vmm_tagname;
extern char *ay_bp_tagtype;
extern char *ay_bp_tagname;
extern char *ay_np_tagtype;
extern char *ay_np_tagname;
extern char *ay_rp_tagtype;
extern char *ay_rp_tagname;
extern char *ay_hc_tagtype;
extern char *ay_hc_tagname;

extern unsigned int ay_current_glname;
extern int ay_wrib_framenum;
extern unsigned int ay_current_primlevel;

/* Definitions */

/** \name The Truth */
/*@{*/
#define AY_TRUE      1
#define AY_FALSE     0
/*@}*/

/** \name Return/Error Codes */
/*@{*/
#define AY_OK          0 /* everything all right */
#define AY_EWARN       1 /* warning */
#define AY_ERROR       2 /* unspecified error */
#define AY_EFLUSH      3 /* flush error messages */
#define AY_EOUTPUT     4 /* used for unspecific output */
#define AY_EOMEM       5 /* out of memory */
#define AY_EOPENFILE  10 /* error opening file */
#define AY_ECLOSEFILE 11 /* error closing file */
#define AY_EFORMAT    12 /* wrong file format  */
#define AY_EUEOF      13 /* unexpected EOF read */
#define AY_EEOF       14 /* EOF read */
#define AY_EDONOTLINK 15 /* Do not link read object! */
#define AY_ENOSEL     20 /* nothing selected */
#define AY_EARGS      21 /* missing or wrong args */
#define AY_EOPT       22 /* missing or malformed option value */
#define AY_EUOPT      23 /* unknown option */
#define AY_EWTYPE     24 /* object is of wrong type */
#define AY_ETYPE      30 /* type exists */
#define AY_ENTYPE     31 /* type does not exist */
#define AY_EREF       40 /* reference counter not zero */
#define AY_ENULL      50 /* illegal zero pointer encountered */
/*@}*/

/** \name Object Type Ids */
/*@{*/
#define AY_IDROOT           0
#define AY_IDNPATCH         1
#define AY_IDNCURVE         2
#define AY_IDLEVEL          3
#define AY_IDLIGHT          4
#define AY_IDBOX            5
#define AY_IDBPATCH         6
#define AY_IDVIEW           7
#define AY_IDCAMERA         8
#define AY_IDINSTANCE       9
#define AY_IDSPHERE        10
#define AY_IDDISK          11
#define AY_IDCONE          12
#define AY_IDCYLINDER      13
#define AY_IDPARABOLOID    14
#define AY_IDHYPERBOLOID   15
#define AY_IDTORUS         16
#define AY_IDRIINC         17
#define AY_IDMATERIAL      18
#define AY_IDICURVE        19
#define AY_IDREVOLVE       20
#define AY_IDEXTRUDE       21
#define AY_IDSWEEP         22
#define AY_IDSKIN          23
#define AY_IDCAP           24
#define AY_IDPAMESH        25
#define AY_IDPOMESH        26
#define AY_IDCONCATNC      27
#define AY_IDCLONE         28
#define AY_IDSDMESH        29
#define AY_IDGORDON        30
#define AY_IDTEXT          31
#define AY_IDBIRAIL1       32
#define AY_IDBIRAIL2       33
#define AY_IDEXTRNC        34
#define AY_IDSCRIPT        35
#define AY_IDRIPROC        36
#define AY_IDBEVEL         37
#define AY_IDNCIRCLE       38
#define AY_IDSWING         39
#define AY_IDSELECT        40
#define AY_IDEXTRNP        41
#define AY_IDOFFNC         42
#define AY_IDACURVE        43
#define AY_IDTRIM          44
#define AY_IDCONCATNP      45
#define AY_IDOFFNP         46

#define AY_IDLAST          50
/*@}*/

/** \name Level Object SubType Ids */
/*@{*/
#define AY_LTEND    0
#define AY_LTLEVEL  1
#define AY_LTUNION  2
#define AY_LTDIFF   3
#define AY_LTINT    4
#define AY_LTPRIM   5
/*@}*/

/** \name View Object SubType Ids */
/*@{*/
#define AY_VTFRONT  0
#define AY_VTSIDE   1
#define AY_VTTOP    2
#define AY_VTPERSP  3
#define AY_VTTRIM   4
/*@}*/

/** \name Shader Types */
/*@{*/
#define AY_STSURFACE        0
#define AY_STDISPLACEMENT   1
#define AY_STVOLUME         2
#define AY_STLIGHT          3
#define AY_STIMAGER         4
#define AY_STTRANSFORMATION 5

#define AY_STAREALIGHT      6
#define AY_STINTERIOR       7
#define AY_STEXTERIOR       8
#define AY_STATMOSPHERE     9
/*@}*/

/** \name Shader Argument Types */
/*@{*/
#define AY_SASCALAR  0
#define AY_SAPOINT   1
#define AY_SANORMAL  2
#define AY_SAVECTOR  3
#define AY_SACOLOR   4
#define AY_SASTRING  5
#define AY_SAMATRIX  6
/*@}*/

/** \name Light Source Types */
/*@{*/
#define AY_LITCUSTOM    0
#define AY_LITPOINT     1
#define AY_LITDISTANT   2
#define AY_LITSPOT      3
/*@}*/

/** \name NURBS Curve Types */
/*@{*/
#define AY_CTOPEN     0
#define AY_CTCLOSED   1
#define AY_CTPERIODIC 2
/*@}*/

/** \name Knot Vector Types */
/*@{*/
#define AY_KTBEZIER    0
#define AY_KTBSPLINE   1
#define AY_KTNURB      2
#define AY_KTCUSTOM    3
#define AY_KTCHORDAL   4
#define AY_KTCENTRI    5
/*@}*/

/** \name Patch Mesh Types */
/*@{*/
#define AY_PTBILINEAR  0
#define AY_PTBICUBIC   1
/*@}*/

/** \name Basis Matrix Types */
/*@{*/
#define AY_BTBEZIER     0
#define AY_BTBSPLINE    1
#define AY_BTCATMULLROM 2
#define AY_BTHERMITE    3
#define AY_BTCUSTOM     4
/*@}*/

/** \name Subdivision Schemes */
/*@{*/
#define AY_SDSCATMULL    0
#define AY_SDSLOOP       1
/*@}*/

/** \name Subdivision Tag Types */
/*@{*/
#define AY_SDTHOLE    0
#define AY_SDTCORNER  1
#define AY_SDTCREASE  2
#define AY_SDTIB      3
/*@}*/

/** \name Procedural Object Types */
/*@{*/
#define AY_PRTDREADA  0 /**< Delayed Read Archive */
#define AY_PRTRUNPROG 1 /**< Run Program */
#define AY_PRTDYNLOAD 2 /**< Dynamic Load */
/*@}*/


/* size of arrows */
#define AY_POINTER 8

/* to avoid direct comparison of doubles with 0.0 */
#define AY_EPSILON 1.0e-06

/** \name Directions */
/*@{*/
#define AY_NORTH  0
#define AY_EAST   1
#define AY_SOUTH  2
#define AY_WEST   3
/*@}*/

/** \name Transcendent Tools */
/*@{*/
#ifdef M_PI
 #define AY_PI M_PI
 #define AY_HALFPI (M_PI/2.0)
#else
 #define AY_PI 3.1415926535897932384626433
 #define AY_HALFPI (3.1415926535897932384626433/2.0)
#endif

#define AY_D2R(x) ((x)*AY_PI/180.0)

#define AY_R2D(x) ((x)*180.0/AY_PI)

#define AY_COT(x) (cos(x)/sin(x))
/*@}*/

/** \name Basic Vector Arithmetic */
/*@{*/
#define AY_VLEN(x,y,z) sqrt((x*x)+(y*y)+(z*z))

#define AY_V3LEN(v) sqrt((v[0]*v[0])+(v[1]*v[1])+(v[2]*v[2]))

#define AY_V2LEN(v) sqrt((v[0]*v[0])+(v[1]*v[1]))

#define AY_V3ZERO(v) {v[0]=0.0; v[1]=0.0; v[2]=0.0;}

#define AY_V3SUB(r,v1,v2) {r[0]=v1[0]-v2[0];r[1]=v1[1]-v2[1];r[2]=v1[2]-v2[2];}

#define AY_V3ADD(r,v1,v2) {r[0]=v1[0]+v2[0];r[1]=v1[1]+v2[1];r[2]=v1[2]+v2[2];}

#define AY_V3MUL(r,v1,v2) {r[0]=v1[0]*v2[0];r[1]=v1[1]*v2[1];r[2]=v1[2]*v2[2];}

#define AY_V3CROSS(r,v1,v2) {r[0]=(v1[1] * v2[2]) - (v1[2] * v2[1]);r[1]=(v1[2] * v2[0]) - (v1[0] * v2[2]);r[2]=(v1[0] * v2[1]) - (v1[1] * v2[0]);}

#define AY_V3SCAL(v,f) {(v[0])*=(f);(v[1])*=(f);(v[2])*=(f);}

#define AY_V2SCAL(v,f) {(v[0])*=(f);(v[1])*=(f);}

/* XXXX is this nesting dangerous? wrong? */
#define AY_V3NORM(v) {AY_V3SCAL((v),(1.0/(AY_V3LEN(v))));}

#define AY_V2NORM(v) {AY_V2SCAL((v),(1.0/(AY_V2LEN(v))));}

#define AY_V3DOT(v1,v2) (v1[0]*v2[0] + v1[1]*v2[1] + v1[2]*v2[2])

#define AY_V2DOT(v1,v2) (v1[0]*v2[0] + v1[1]*v2[1])

#define AY_V4COMP(v1, v2) ((fabs(v1[0]-v2[0]) < AY_EPSILON) &&\
			   (fabs(v1[1]-v2[1]) < AY_EPSILON) &&\
			   (fabs(v1[2]-v2[2]) < AY_EPSILON) &&\
                           (fabs(v1[3]-v2[3]) < AY_EPSILON))

#define AY_V3COMP(v1, v2) ((fabs(v1[0]-v2[0]) < AY_EPSILON) &&\
			   (fabs(v1[1]-v2[1]) < AY_EPSILON) &&\
			   (fabs(v1[2]-v2[2]) < AY_EPSILON))

#define AY_V2COMP(v1, v2) ((fabs(v1[0]-v2[0]) < AY_EPSILON) &&\
			   (fabs(v1[1]-v2[1]) < AY_EPSILON))

/*@}*/

/* Warning: v1 and v2 must be different locations in memory! */
#define AY_APTRAN4(v1,v2,m) {v1[0]=v2[0]*m[0]+v2[1]*m[4]+v2[2]*m[8]+v2[3]*m[12];v1[1]=v2[0]*m[1]+v2[1]*m[5]+v2[2]*m[9]+v2[3]*m[13];v1[2]=v2[0]*m[2]+v2[1]*m[6]+v2[2]*m[10]+v2[3]*m[14];v1[3]=v2[0]*m[3]+v2[1]*m[7]+v2[2]*m[11]+v2[3]*m[15];}

/* Warning: v1 and v2 must be different locations in memory! */
#define AY_APTRAN3(v1,v2,m) {v1[0]=v2[0]*m[0]+v2[1]*m[4]+v2[2]*m[8]+1.0*m[12];v1[1]=v2[0]*m[1]+v2[1]*m[5]+v2[2]*m[9]+1.0*m[13];v1[2]=v2[0]*m[2]+v2[1]*m[6]+v2[2]*m[10]+1.0*m[14];}

#define AY_M44(m,r,c) ((m)[(c)*4+(r)])

#define AY_ISTRAFO(o) ((fabs(o->movx) > AY_EPSILON) ||\
		       (fabs(o->movy) > AY_EPSILON) ||\
		       (fabs(o->movz) > AY_EPSILON) ||\
		       (fabs(o->quat[0]) > AY_EPSILON) ||\
		       (fabs(o->quat[1]) > AY_EPSILON) ||\
		       (fabs(o->quat[2]) > AY_EPSILON) ||\
		       (fabs(1.0 - o->quat[3]) > AY_EPSILON) ||\
		       (fabs(1.0 - o->scalx) > AY_EPSILON) ||\
		       (fabs(1.0 - o->scaly) > AY_EPSILON) ||\
		       (fabs(1.0 - o->scalz) > AY_EPSILON))


/** \name Version Strings and Numbers */
/*@{*/
#define AY_VERSIONSTR "1.17"
#define AY_VERSIONSTRMI "0"

#define AY_VERSIONMA 1
#define AY_VERSION   17
#define AY_VERSIONMI 0
/*@}*/

/* Ayam API */
#include "aycore.h"
#include "nurbs.h"
#include "objects.h"
#include "contrib.h"

/** \file ayam.h \brief main Ayam header */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __ayam_h__ */

