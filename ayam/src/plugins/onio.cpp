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

// onio.cpp - Ayam OpenNURBS (Rhino 3DM) IO plugin

#include "ayam.h"

#include "opennurbs.h"
#include "opennurbs_extensions.h"

// local preprocessor definitions

// global variables

char onio_version_ma[] = AY_VERSIONSTR;
char onio_version_mi[] = AY_VERSIONSTRMI;


// prototypes of functions local to this module

int onio_readnurbssurface(ON_NurbsSurface *p_s);

int onio_readnurbscurve(ON_NurbsCurve *p_c);

int onio_readobject(const ON_Object *p_o);

extern "C" {

int Onio_Init(Tcl_Interp *interp);

}


// functions

// onio_readnurbssurface
//
int
onio_readnurbssurface(ON_NurbsSurface *p_s)
{
 int ay_status = AY_OK;
 int width, height, i, j, a, b, stride;
 double *controlv = NULL;
 double *uknotv = NULL, *vknotv = NULL, *knotv;
 ay_nurbpatch_object *patch = NULL;
 ay_object *newo = NULL;

  // get some info about the surface
  width = p_s->m_cv_count[0];
  height = p_s->m_cv_count[1];

  stride = p_s->m_dim;

  if(p_s->m_is_rat)
    {
      stride++;
    }

  if(stride > 4)
    {
      stride = 4;
    }

  // allocate new (safe) memory
  if(!(controlv = (double*)calloc(width*height*4, sizeof(double))))
    return AY_EOMEM;
  if(!(uknotv = (double*)calloc(width+p_s->m_order[0], sizeof(double))))
    { free(controlv); return AY_EOMEM; }
  if(!(vknotv = (double*)calloc(height+p_s->m_order[1], sizeof(double))))
    { free(controlv); free(uknotv); return AY_EOMEM; }
  
  // copy data into new (safe) memory
  a = 0; b = 0;
  for(i = 0; i < width; i++)
    {
      for(j = 0; j < height; j++)
	{
	  a = (i*height+j)*(p_s->m_dim+p_s->m_is_rat);
	  memcpy(&(controlv[b]), &(p_s->m_cv[a]), stride*sizeof(double));
	  b += 4;
	} // for
    } // for

  // if no weights are in the file, reset them to 1.0
  if(!p_s->m_is_rat)
    {
      b = 3;
      for(i = 0; i < width; i++)
	{
	  for(j = 0; j < height; j++)
	    {
	      controlv[b] = 1.0;
	      b += 4;
	    } // for
	} // for
    } // if
  
  // if weights are in the file but the dimension of the surface is
  // higher than 3, copy the weights in this step
  if(p_s->m_is_rat && (p_s->m_dim > 3))
    {
      b = 3;
      for(i = 0; i < width; i++)
	{
	  for(j = 0; j < height; j++)
	    {
	      a = (i*height+j)*(p_s->m_dim+p_s->m_is_rat);
	      controlv[b] = p_s->m_cv[a+p_s->m_dim];
	      b += 4;
	    } // for
	} // for
    } // if

  // copy the knot vectors
  knotv = p_s->m_knot[0];
  a = 1; b = 0;
  for(i = 0; i < width+p_s->m_order[0]-2; i++)
    {
      uknotv[a] = knotv[b];
      a++; b++;
    } // for
  uknotv[0] = uknotv[1];
  uknotv[width+p_s->m_order[0]-1] = uknotv[width+p_s->m_order[0]-2];

  knotv = p_s->m_knot[1];
  a = 1; b = 0;
  for(i = 0; i < height+p_s->m_order[1]-2; i++)
    {
      vknotv[a] = knotv[b];
      a++; b++;
    } // for
  vknotv[0] = vknotv[1];
  vknotv[height+p_s->m_order[1]-1] = vknotv[height+p_s->m_order[1]-2];

  // now create a NURBPatch object
  ay_status = ay_npt_create(p_s->m_order[0], p_s->m_order[1], width, height,
			    AY_KTCUSTOM, AY_KTCUSTOM,
			    controlv, uknotv, vknotv,
			    &patch);

  if(ay_status)
    { free(controlv); free(uknotv); free(vknotv); return ay_status; }

  if(!(newo = (ay_object*)calloc(1, sizeof(ay_object))))
    { free(controlv); free(uknotv); free(vknotv); return AY_EOMEM; }

  ay_status = ay_object_defaults(newo);

  newo->type = AY_IDNPATCH;
  newo->refine = patch;
  newo->parent = AY_TRUE;
  newo->hide_children = AY_TRUE;
  newo->inherit_trafos = AY_FALSE;

  // link the new patch into the scene hierarchy
  ay_status = ay_object_link(newo);

  if(ay_status)
    ay_status = ay_object_delete(newo);

 return ay_status;
} // onio_readnurbssurface


// onio_readnurbscurve
//
int
onio_readnurbscurve(ON_NurbsCurve *p_c)
{
 int ay_status = AY_OK;
 int length, i, a, b, stride;
 double *controlv = NULL;
 double *knotv = NULL;
 ay_nurbcurve_object *curve = NULL;
 ay_object *newo = NULL;

  // get some info about the curve
  length = p_c->m_cv_count;

  stride = p_c->m_dim;

  if(p_c->m_is_rat)
    {
      stride++;
    }

  if(stride > 4)
    {
      stride = 4;
    }

  // allocate new (safe) memory
  if(!(controlv = (double*)calloc(length*4, sizeof(double))))
    return AY_EOMEM;
  if(!(knotv = (double*)calloc(length+p_c->m_order, sizeof(double))))
    { free(controlv); return AY_EOMEM; }
  
  // copy data into new (safe) memory
  a = 0; b = 0;
  for(i = 0; i < length; i++)
    {
      a = i*(p_c->m_dim+p_c->m_is_rat);
      memcpy(&(controlv[b]), &(p_c->m_cv[a]), stride*sizeof(double));
      b += 4;
    } // for

  // if no weights are in the file, reset them to 1.0
  if(!p_c->m_is_rat)
    {
      b = 3;
      for(i = 0; i < length; i++)
	{

	  controlv[b] = 1.0;
	  b += 4;
	} // for
    } // if
  
  // if weights are in the file but the dimension of the curve is
  // higher than 3, copy the weights in this step
  if(p_c->m_is_rat && (p_c->m_dim > 3))
    {
      b = 3;
      for(i = 0; i < length; i++)
	{
	  a = i*(p_c->m_dim+p_c->m_is_rat);
	  controlv[b] = p_c->m_cv[a+p_c->m_dim];
	  b += 4;
	} // for
    } // if
  
  // copy the knot vector
  a = 1; b = 0;
  for(i = 0; i < length+p_c->m_order-2; i++)
    {
      knotv[a] = p_c->m_knot[b];
      a++; b++;
    } // for
  knotv[0] = knotv[1];
  knotv[length+p_c->m_order-1] = knotv[length+p_c->m_order-2];

  // now create a NURBCurve object
  ay_status = ay_nct_create(p_c->m_order, length, AY_KTCUSTOM, controlv, knotv,
			    &curve);

  if(ay_status)
    { free(controlv); free(knotv); return ay_status; }

  if(!(newo = (ay_object*)calloc(1, sizeof(ay_object))))
    { free(controlv); free(knotv); return AY_EOMEM; }

  ay_status = ay_object_defaults(newo);

  newo->type = AY_IDNCURVE;
  newo->refine = curve;

  // link the new curve into the scene hierarchy
  ay_status = ay_object_link(newo);

  if(ay_status)
    ay_status = ay_object_delete(newo);

 return ay_status;
} // onio_readnurbscurve


// onio_readobject:
//
int
onio_readobject(const ON_Object *p_o)
{
 int ay_status = AY_OK;
 double accuracy = 0.1;
 ON_NurbsSurface p_s;
 ON_NurbsCurve p_c;

  if(!p_o)
    return AY_ENULL;

  switch(p_o->ObjectType())
    {
    case ON::curve_object:
      if(ON_NurbsCurve::Cast(p_o))
	ay_status = onio_readnurbscurve((ON_NurbsCurve*)p_o);
      if(ON_PolylineCurve::Cast(p_o))
	{
	  (ON_PolylineCurve::Cast(p_o))->GetNurbForm(p_c, accuracy, NULL);
	  ay_status = onio_readnurbscurve(&p_c);
	}
      if(ON_LineCurve::Cast(p_o))
	{
	  (ON_LineCurve::Cast(p_o))->GetNurbForm(p_c, accuracy, NULL);
	  ay_status = onio_readnurbscurve(&p_c);
	}
      if(ON_ArcCurve::Cast(p_o))
	{
	  (ON_ArcCurve::Cast(p_o))->GetNurbForm(p_c, accuracy, NULL);
	  ay_status = onio_readnurbscurve(&p_c);
	}
      if(ON_CurveOnSurface::Cast(p_o))
	{
	  (ON_CurveOnSurface::Cast(p_o))->GetNurbForm(p_c, accuracy, NULL);
	  ay_status = onio_readnurbscurve(&p_c);
	}
      break;
    case ON::surface_object:
      if(ON_NurbsSurface::Cast(p_o))
	ay_status = onio_readnurbssurface((ON_NurbsSurface*)p_o);
      if(ON_RevSurface::Cast(p_o))
	{
	  (ON_RevSurface::Cast(p_o))->GetNurbForm(p_s, accuracy);
	  ay_status = onio_readnurbssurface(&p_s);
	}
      if(ON_SumSurface::Cast(p_o))
	{
	  (ON_SumSurface::Cast(p_o))->GetNurbForm(p_s, accuracy);
	  ay_status = onio_readnurbssurface(&p_s);
	}
      if(ON_PlaneSurface::Cast(p_o))
	{
	  (ON_PlaneSurface::Cast(p_o))->GetNurbForm(p_s, accuracy);
	  ay_status = onio_readnurbssurface(&p_s);
	}
      /*
      if(ON_BezierSurface::Cast(p_o))
	{
	  ON_BezierSurface::Cast(p_o).GetNurbForm(p_s);
	  ay_status = onio_readnurbssurface((ON_NurbsSurface*)p_s);
	}
      */
      break;
    default:
      break;
    }

 return ay_status;
} // onio_readobject


int
onio_readtcmd(ClientData clientData, Tcl_Interp *interp,
	      int argc, char *argv[])
{
 int ay_status = AY_OK;
 char fname[] = "onio_read";
 ONX_Model model;

  // check args
  if(argc != 2)
    {
      ay_error(AY_EARGS, fname, "filename");
      return TCL_OK;
    }

  const char *filename = argv[1];

  // open file containing opennurbs archive
  FILE *archive_fp = ON::OpenFile(filename, "rb");
  if(!archive_fp)
    {
      ay_error(AY_EOPENFILE, fname, argv[1]);
      return TCL_OK;
    }

  // create achive object from file pointer
  ON_BinaryFile archive(ON::read3dm, archive_fp);

  // read the contents of the file into "model"
  bool rc = model.Read(archive, NULL/*stderr*/);

  // close the file
  ON::CloseFile(archive_fp);

  // print diagnostic
  if(!rc)
    {
      ay_error(AY_ERROR, fname, "Error reading file!");
    }

  // see if everything is in good shape
  if(!model.IsValid(NULL/*stderr*/))
    {
      ay_error(AY_ERROR, fname, "Model is not valid!");
    }

  for(int i = 0; i < model.m_object_table.Capacity(); ++i)
    {
      if((model.m_object_table[i]).m_object)
	{
	  ay_status = onio_readobject((model.m_object_table[i]).m_object);
	  if(ay_status)
	    {
	      ay_error(ay_status, fname, NULL);
	      ay_error(AY_ERROR, fname,
		       "Failed to read/convert object; will continue!");
	    }
	} // if
    } // for

  // destroy this model
  model.Destroy();


 return TCL_OK;
} // onio_readtcmd


extern "C" {

// Onio_Init | onio_inittcmd:
//  initialize onio module
//  note: this function _must_ be capitalized exactly this way
//  regardless of the filename of the shared object (see: man n load)!
#ifdef ONIOWRAPPED
int
onio_inittcmd(ClientData clientData, Tcl_Interp *interp,
	      int argc, char *argv[])
#else
int
Onio_Init(Tcl_Interp *interp)
#endif
{
 char fname[] = "Onio_Init";
 // int err;
 // int ay_status = AY_OK;

#ifndef ONIOWRAPPED
  // first, check versions
  if(strcmp(ay_version_ma, onio_version_ma))
    {
      ay_error(AY_ERROR, fname,
	       "Plugin has been compiled for a different Ayam version!");
      ay_error(AY_ERROR, fname, "It is unsafe to continue! Bailing out...");
      return TCL_OK;
    }

  if(strcmp(ay_version_mi, onio_version_mi))
    {
      ay_error(AY_ERROR, fname,
	       "Plugin has been compiled for a different Ayam version!");
      ay_error(AY_ERROR, fname, "However, it is probably safe to continue...");
    }
#endif

#ifndef ONIOWRAPPED
  // source onio.tcl, it contains vital Tcl-code
  if((Tcl_EvalFile(interp, "onio.tcl")) != TCL_OK)
     {
       ay_error(AY_ERROR, fname,
		  "Error while sourcing \\\"onio.tcl\\\"!");
       return TCL_OK;
     }
#endif

  // initialize OpenNURBS
  ON::Begin();

  // create new commands for all views (Togl widgets)
  //Togl_CreateCommand("rendercsg", onio_rendertcb);

  // create new Tcl commands to interface with the plugin
  Tcl_CreateCommand(interp, "onioRead", onio_readtcmd,
		    (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);


#ifndef ONIOWRAPPED
  ay_error(AY_EOUTPUT, fname, "Plugin 'onio' successfully loaded.");
#endif

 return TCL_OK;
} // Aycsg_Init | onio_inittcmd

} // extern "C"

// remove local preprocessor definitions

