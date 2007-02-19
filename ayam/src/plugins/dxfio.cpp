/*
 * Ayam, a free 3D modeler for the RenderMan interface.
 *
 * Ayam is copyrighted 1998-2007 by Randolf Schultz
 * (rschultz@informatik.uni-rostock.de) and others.
 *
 * All rights reserved.
 *
 * See the file License for details.
 *
 */

// dxfio.cpp - Ayam DXF Import/Export Plugin

#include "ayam.h"

// Dime includes
#include <dime/Base.h>
#include <dime/Basic.h>
#include <dime/Input.h>
#include <dime/Output.h>
#include <dime/Model.h>
#include <dime/State.h>
#include <dime/entities/Entity.h>
#include <dime/entities/Arc.h>
#include <dime/entities/Block.h>
#include <dime/entities/Circle.h>
#include <dime/entities/Ellipse.h>
#include <dime/entities/Line.h>
#include <dime/entities/UnknownEntity.h>
#include <dime/util/Linear.h>
#include <stdio.h>

// local types


// global variables

char dxfio_version_ma[] = AY_VERSIONSTR;
char dxfio_version_mi[] = AY_VERSIONSTRMI;

static Tcl_HashTable dxfio_write_ht;

ay_object *dxfio_lrobject = NULL;

static double tm[16] = {0}; // current transformation matrix

int dxfio_importcurves = AY_TRUE;
int dxfio_exportcurves = AY_TRUE;
int dxfio_expselected = AY_FALSE;
int dxfio_expobeynoexport = AY_TRUE;
int dxfio_expignorehidden = AY_TRUE;
int dxfio_exptoplevellayers = AY_TRUE;

int dxfio_currentlayer = 0;

double dxfio_rescaleknots = 0.0;

double dxfio_accuracy = 1.0e-12;

double dxfio_scalefactor = 1.0;

char dxfio_stagnamedef[] = "mys";
char *dxfio_stagname = dxfio_stagnamedef;
char dxfio_ttagnamedef[] = "myt";
char *dxfio_ttagname = dxfio_ttagnamedef;

// prototypes of functions local to this module
int dxfio_readarc(const class dimeState *state,
		  class dimeArc *arc,
		  void *clientdata);

int dxfio_readblock(const class dimeState *state,
		    class dimeBlock *block,
		    void *clientdata);

int dxfio_readcircle(const class dimeState *state,
		     class dimeCircle *circle,
		     void *clientdata);

int dxfio_readellipse(const class dimeState *state,
		      class dimeEllipse *ellipse,
		      void *clientdata);

int dxfio_readline(const class dimeState *state,
		   class dimeLine *line,
		   void *clientdata);

int dxfio_readprogressdcb(float progress, void *clientdata);

bool dxfio_readentitydcb(const class dimeState *state,
			 class dimeEntity *entity,
			 void *clientdata);

int dxfio_readtcmd(ClientData clientData, Tcl_Interp *interp,
		   int argc, char *argv[]);

extern "C" {
#ifdef WIN32
  __declspec( dllexport )
#endif
  int Dxfio_Init(Tcl_Interp *interp);
} // extern "C"

// implementation of functions

// dxfio_readarc:
//
int
dxfio_readarc(const class dimeState *state,
		 class dimeArc *arc,
		 void *clientdata)
{
 int ay_status = AY_OK;
 ay_object *newo = NULL;
 ay_ncircle_object *newc = NULL;
 dimeVec3f center;

  if(!(newo = (ay_object*)calloc(1, sizeof(ay_object))))
    { return AY_EOMEM; }

  if(!(newc = (ay_ncircle_object*)calloc(1, sizeof(ay_ncircle_object))))
    { free(newo); return AY_EOMEM; }

  newc->radius = arc->getRadius();
  newc->tmin = arc->getStartAngle();
  newc->tmax = arc->getEndAngle();

  ay_status = ay_object_defaults(newo);

  newo->type = AY_IDNCIRCLE;
  newo->refine = newc;
  arc->getCenter(center);
  newo->movx = center[0];
  newo->movy = center[1];
  newo->movz = center[2];

  ay_status = ay_notify_force(newo);

  // link the new arc into the scene hierarchy
  ay_status = ay_object_link(newo);

 return ay_status;
} // dxfio_readarc


// dxfio_readblock:
//
int
dxfio_readblock(const class dimeState *state,
		class dimeBlock *block,
		void *clientdata)
{
 int ay_status = AY_OK;
 ay_object *newo = NULL;
 ay_level_object *newl = NULL;

  if(!(newo = (ay_object*)calloc(1, sizeof(ay_object))))
    { return AY_EOMEM; }

  if(!(newl = (ay_level_object*)calloc(1, sizeof(ay_level_object))))
    { free(newo); return AY_EOMEM; }

  ay_status = ay_object_defaults(newo);

  newo->type = AY_IDLEVEL;
  newo->refine = newl;

  // link the new block into the scene hierarchy
  ay_status = ay_object_link(newo);

 return ay_status;
} // dxfio_readblock


// dxfio_readcircle:
//
int
dxfio_readcircle(const class dimeState *state,
		 class dimeCircle *circle,
		 void *clientdata)
{
 int ay_status = AY_OK;
 ay_object *newo = NULL;
 ay_ncircle_object *newc = NULL;

  if(!(newo = (ay_object*)calloc(1, sizeof(ay_object))))
    { return AY_EOMEM; }

  if(!(newc = (ay_ncircle_object*)calloc(1, sizeof(ay_ncircle_object))))
    { free(newo); return AY_EOMEM; }

  newc->radius = circle->getRadius();
  newc->tmax = 360.0;
  ay_status = ay_object_defaults(newo);

  newo->type = AY_IDNCIRCLE;
  newo->refine = newc;

  newo->movx = circle->getCenter()[0];
  newo->movy = circle->getCenter()[1];
  newo->movz = circle->getCenter()[2];

  ay_status = ay_notify_force(newo);

  // link the new circle into the scene hierarchy
  ay_status = ay_object_link(newo);

 return ay_status;
} // dxfio_readcircle


// dxfio_readellipse:
//
int
dxfio_readellipse(const class dimeState *state,
		 class dimeEllipse *ellipse,
		 void *clientdata)
{
 int ay_status = AY_OK;
 double zaxis[3] = {0.0, 0.0, 1.0}, maja[2];
 ay_object *newo = NULL;
 ay_ncircle_object *newc = NULL;

  if(!(newo = (ay_object*)calloc(1, sizeof(ay_object))))
    { return AY_EOMEM; }

  if(!(newc = (ay_ncircle_object*)calloc(1, sizeof(ay_ncircle_object))))
    { free(newo); return AY_EOMEM; }

  maja[0] = ellipse->getMajorAxisEndpoint()[0];
  maja[1] = ellipse->getMajorAxisEndpoint()[1];

  newc->radius = AY_V2LEN(maja);
  newc->tmin = AY_R2D(ellipse->getStartParam());
  newc->tmax = AY_R2D(ellipse->getEndParam());

  ay_status = ay_object_defaults(newo);

  newo->type = AY_IDNCIRCLE;
  newo->refine = newc;

  newo->movx = ellipse->getCenter()[0];
  newo->movy = ellipse->getCenter()[1];
  newo->movz = ellipse->getCenter()[2];

  newo->scalx = ellipse->getMinorMajorRatio();

  if(fabs(maja[0]) > AY_EPSILON)
    {
      newo->rotz = AY_R2D(atan(maja[1]/maja[0]));
      ay_quat_axistoquat(zaxis, atan(maja[1]/maja[0]), newo->quat);
    }
  else
    {
      if(maja[1] < 0.0)
	{
	  newo->rotz = 90.0;
	  ay_quat_axistoquat(zaxis, AY_HALFPI, newo->quat);
	}
      else
	{
	  newo->rotz = 270.0;
	  ay_quat_axistoquat(zaxis, AY_PI+AY_HALFPI, newo->quat);
	}
    } // if

  ay_status = ay_notify_force(newo);

  // link the new circle/ellipse into the scene hierarchy
  ay_status = ay_object_link(newo);

 return ay_status;
} // dxfio_readellipse


// dxfio_readline:
//
int
dxfio_readline(const class dimeState *state,
	       class dimeLine *line,
	       void *clientdata)
{
 int ay_status = AY_OK;
 double *newcv = NULL;
 ay_object *newo = NULL;
 
  if(!(newo = (ay_object*)calloc(1, sizeof(ay_object))))
    { return AY_EOMEM; }

  if(!(newcv = (double*)calloc(2*4, sizeof(double))))
    { free(newo); return AY_EOMEM; }

  newcv[0] = (line->getCoords(0))[0];
  newcv[1] = (line->getCoords(0))[1];
  newcv[2] = (line->getCoords(0))[2];
  newcv[3] = 1.0;

  newcv[4] = (line->getCoords(1))[0];
  newcv[5] = (line->getCoords(1))[1];
  newcv[6] = (line->getCoords(1))[2];
  newcv[7] = 1.0;

  ay_nct_create(2, 2, AY_KTNURB, newcv, NULL,
		(ay_nurbcurve_object**)&(newo->refine));

  ay_status = ay_object_defaults(newo);

  newo->type = AY_IDNCURVE;

  // link the new curve/line into the scene hierarchy
  ay_status = ay_object_link(newo);

 return ay_status;
} // dxfio_readline


// dxfio_readentitydcb:
//  Dime entity traversal callback
bool
dxfio_readentitydcb(const class dimeState *state,
		    class dimeEntity *entity,
		    void *clientdata)
{
 char fname[] = "dxfio_readentity";

  if(!state || ! entity)
    return true;

  switch(entity->typeId())
    {
    case dimeBase::dime3DFaceType:
      break;
    case dimeBase::dimeArcType:
      dxfio_readarc(state, (dimeArc*)entity, clientdata);
      break;
    case dimeBase::dimeCircleType:
      dxfio_readcircle(state, (dimeCircle*)entity, clientdata);
      break;
    case dimeBase::dimeEllipseType:
      dxfio_readellipse(state, (dimeEllipse*)entity, clientdata);
      break;
    case dimeBase::dimeLineType:
      dxfio_readline(state, (dimeLine*)entity, clientdata);
      break;
    case dimeBase::dimeUnknownEntityType:
      //if(dxfio_errorlevel > 1)
      ay_error(AY_EWARN, fname, "Skipping entity of unknown type.");
      break;
    default:
      break;
    } // switch

 return true;
} // dxfio_readentitydcb


// dxfio_readprogressdcb:
//
int
dxfio_readprogressdcb(float progress, void *clientdata)
{
 static float oldprogress = 0.0f;
 char progressstr[32];
 char arrname[] = "dxfio_options", varname[] = "Progress";

  if(clientdata != NULL)
    {
      oldprogress = 0.0f;
      return 1;
    }

  if(progress-oldprogress > 0.05)
    {
      sprintf(progressstr, "%d", (int)(progress*50.0f));

      Tcl_SetVar2(ay_interp, arrname, varname, progressstr,
		  TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
      while(Tcl_DoOneEvent(TCL_DONT_WAIT)){};

      oldprogress = progress;
    } // if

  return 1;
} // dxfio_readprogressdcb


// dxfio_readtcmd:
//  Tcl command to read DXF files using Dime
int
dxfio_readtcmd(ClientData clientData, Tcl_Interp *interp,
	       int argc, char *argv[])
{
 int ay_status = AY_OK;
 char fname[] = "dxfio_read";
 char *minus, lineerrstr[64];
 int i = 2, sframe = -1, eframe = -1;
 double accuracy = 0.1;
 char arrname[] = "dxfio_options", varname[] = "Progress";

  dxfio_importcurves = AY_TRUE;
  dxfio_rescaleknots = 0.0;
  dxfio_scalefactor = 1.0;
  // reset internal progress counter
  dxfio_readprogressdcb(0.0f, (void*)1);

  // check args
  if(argc < 2)
    {
      ay_error(AY_EARGS, fname, "filename");
      return TCL_OK;
    }

  // parse args
  const char *filename = argv[1];

  while(i+1 < argc)
    {
      if(!strcmp(argv[i], "-a"))
	{
	  sscanf(argv[i+1], "%lg", &accuracy);
	}
      else
      if(!strcmp(argv[i], "-c"))
	{
	  sscanf(argv[i+1], "%d", &dxfio_importcurves);
	}
      else
      if(!strcmp(argv[i], "-r"))
	{
	  sscanf(argv[i+1], "%lg", &dxfio_rescaleknots);
	}
      else
      if(!strcmp(argv[i], "-f"))
	{
	  sscanf(argv[i+1], "%lg", &dxfio_scalefactor);
	}
      else
      if(!strcmp(argv[i], "-t"))
	{
	  dxfio_stagname = argv[i+1];
	  dxfio_ttagname = argv[i+2];
	  i++;
	}
      else
      if(!strcmp(argv[i], "-l"))
	{
	  if(argv[i+1])
	    {
	      if(*argv[i+1] != '-')
		{
		  sscanf(argv[i+1], "%d", &sframe);
		  eframe = sframe;
		  if((strlen(argv[i+1]) > 3) &&
		     (minus = strchr(/*(const char*)*/(&(argv[i+1][1])), '-')))
		    {
		      minus++;
		      if(*minus != '\0')
			{
			  sscanf(minus, "%d", &eframe);
			}
		      else
			{
			  ay_error(AY_ERROR, fname,
	    "could not parse layer range, specify it as: startindex-endindex");
			  return TCL_OK;
			} // if
		    } // if
		} // if
	    } // if
	} // if
      i += 2;
    } // while


  // open file for reading
  dimeInput in;

  if(!in.setFile(filename))
    {
      ay_error(AY_EOPENFILE, fname, argv[1]);
      return TCL_OK;
    }

  in.setCallback(&dxfio_readprogressdcb, NULL);

  // try reading the file
  dimeModel model;

  if(!model.read(&in))
    {
      int line = in.getFilePosition();
      sprintf(lineerrstr, "DXF read error in line: %d", line);
      ay_error(AY_ERROR, fname, lineerrstr);
      return TCL_OK;
    }

  // set progress
  Tcl_SetVar2(ay_interp, arrname, varname, "50",
	      TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  while(Tcl_DoOneEvent(TCL_DONT_WAIT)){};

  // get all entities
  dxfio_lrobject = NULL;
  model.traverseEntities(&dxfio_readentitydcb, NULL,
			 false, true, false);

  // set progress
  Tcl_SetVar2(ay_interp, arrname, varname, "100",
	      TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  while(Tcl_DoOneEvent(TCL_DONT_WAIT)){};

  dxfio_stagname = dxfio_stagnamedef;
  dxfio_ttagname = dxfio_ttagnamedef;

 return TCL_OK;
} // dxfio_readtcmd


extern "C" {

/* note: this function _must_ be capitalized exactly this way
 * regardless of the filename of the shared object (see: man n load)!
 */
#ifdef WIN32
__declspec( dllexport ) int
Dxfio_Init(Tcl_Interp *interp)
#else
int
Dxfio_Init(Tcl_Interp *interp)
#endif // WIN32
{
 char fname[] = "dxfio_init";
 // int ay_status = AY_OK;

  if(Tcl_InitStubs(interp, "8.2", 0) == NULL)
    {
      return TCL_ERROR;
    }

  /* first, check versions */
  if(strcmp(ay_version_ma, dxfio_version_ma))
    {
      ay_error(AY_ERROR, fname,
	       "Plugin has been compiled for a different Ayam version!");
      ay_error(AY_ERROR, fname, "It is unsafe to continue! Bailing out...");
      return TCL_OK;
    }

  if(strcmp(ay_version_mi, dxfio_version_mi))
    {
      ay_error(AY_ERROR, fname,
	       "Plugin has been compiled for a different Ayam version!");
      ay_error(AY_ERROR, fname, "However, it is probably safe to continue...");
    }

#ifndef AYDXFIOWRAPPED
  // source dxfio.tcl, it contains vital Tcl-code
  if((Tcl_EvalFile(interp, "dxfio.tcl")) != TCL_OK)
     {
       ay_error(AY_ERROR, fname,
		  "Error while sourcing \\\"dxfio.tcl\\\"!");
       return TCL_OK;
     }
#endif // !AYDXFIOWRAPPED

  // create new Tcl commands to interface with the plugin
  Tcl_CreateCommand(interp, "dxfioRead", dxfio_readtcmd,
		    (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);
  /*
  Tcl_CreateCommand(interp, "dxfioWrite", dxfio_writetcmd,
		    (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);
  */
  /* init hash table for write callbacks */
  Tcl_InitHashTable(&dxfio_write_ht, TCL_ONE_WORD_KEYS);

  /* fill hash table */


  ay_error(AY_EOUTPUT, fname, "Plugin 'dxfio' successfully loaded.");

 return TCL_OK;
} /* Dxfio_Init */

} // extern "C"
