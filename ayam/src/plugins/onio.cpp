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


// local types

typedef int (onio_writecb) (ay_object *o, ONX_Model *p_m);


// global variables

char onio_version_ma[] = AY_VERSIONSTR;
char onio_version_mi[] = AY_VERSIONSTRMI;

static Tcl_HashTable onio_write_ht;


// prototypes of functions local to this module

int onio_writenpatch(ay_object *o, ONX_Model *p_m);

int onio_writenpconvertible(ay_object *o, ONX_Model *p_m);

int onio_writencurve(ay_object *o, ONX_Model *p_m);

int onio_writencconvertible(ay_object *o, ONX_Model *p_m);

int onio_writeobject(ay_object *o, ONX_Model *p_m);

int onio_writetcmd(ClientData clientData, Tcl_Interp *interp,
		   int argc, char *argv[]);

int onio_registerwritecb(char *type, onio_writecb *cb);

/////////////////////////////////////////////////////////////////////////

int onio_readnurbssurface(ON_NurbsSurface *p_s);

int onio_readnurbscurve(ON_NurbsCurve *p_c);

int onio_readobject(const ON_Object *p_o);

int onio_readtcmd(ClientData clientData, Tcl_Interp *interp,
		  int argc, char *argv[]);

extern "C" {

int Onio_Init(Tcl_Interp *interp);

}


// functions


// onio_writenpatch:
//
int
onio_writenpatch(ay_object *o, ONX_Model *p_m)
{
 int ay_status = AY_OK;
 int i, j, a, stride = 4;
 ay_nurbpatch_object *np = NULL;
 ON_NurbsSurface *p_n = NULL;

  if(!o || !p_m)
    return AY_ENULL;

  np = (ay_nurbpatch_object *)o->refine;
  p_n = new ON_NurbsSurface(3, true, np->uorder, np->vorder,
			    np->width, np->height);

  // copy knots, ignoring "superfluous"/"phantom" end knots
  for(i = 0; i < np->uorder+np->width-2; i++)
    p_n->SetKnot(0, i, np->uknotv[i+1]);
  for(i = 0; i < np->vorder+np->height-2; i++)
    p_n->SetKnot(1, i, np->vknotv[i+1]);

  // copy control points
  a = 0;
  for(i = 0; i < np->width; i++)
    {
      for(j = 0; j < np->height; j++)
	{
	  p_n->SetCV(i, j, ON::homogeneous_rational, &(np->controlv[a]));
	  a += stride;
	}
    }

  ONX_Model_Object& mo = p_m->m_object_table.AppendNew();
  mo.m_object = p_n;
  mo.m_bDeleteObject = false;
  /*
    if(object_attributes)
    mo.m_attributes = object_attributes[i];
  */

 return ay_status;
} // onio_writenpatch


// onio_writenpconvertible:
//
int
onio_writenpconvertible(ay_object *o, ONX_Model *p_m)
{
 int ay_status = AY_OK;
 ay_object *p = NULL, *t = NULL;

  if(!o || !p_m)
    return AY_ENULL;

  ay_status = ay_provide_object(o, AY_IDNPATCH, &p);
  if(p)
    {
      t = p;
      while(t)
	{
	  if(t->type == AY_IDNPATCH)
	    {
	      ay_status = onio_writenpatch(t, p_m);
	    } /* if */
	  t = t->next;
	} /* while */

      ay_status = ay_object_deletemulti(p);

      return AY_OK;
    } /* if */

 return ay_status;
} // onio_writenpconvertible


// onio_writencurve:
//
int
onio_writencurve(ay_object *o, ONX_Model *p_m)
{
 int ay_status = AY_OK;
 int i, a, stride = 4;
 ay_nurbcurve_object *nc = NULL;
 ON_NurbsCurve *p_c = NULL;

  if(!o || !p_m)
    return AY_ENULL;

  nc = (ay_nurbcurve_object *)o->refine;
  p_c = new ON_NurbsCurve(3, true, nc->order, nc->length);

  // copy knots, ignoring "superfluous"/"phantom" end knots
  for(i = 0; i < nc->order+nc->length-2; i++)
    p_c->SetKnot(i, nc->knotv[i+1]);
 
  // copy control points
  a = 0;
  for(i = 0; i < nc->length; i++)
    {
      p_c->SetCV(i, ON::homogeneous_rational, &(nc->controlv[a]));
      a += stride;
    }

  ONX_Model_Object& mo = p_m->m_object_table.AppendNew();
  mo.m_object = p_c;
  mo.m_bDeleteObject = false;

  /*
    if(object_attributes)
    mo.m_attributes = object_attributes[i];
  */

 return ay_status;
} // onio_writencurve


// onio_writencconvertible:
//
int
onio_writencconvertible(ay_object *o, ONX_Model *p_m)
{
 int ay_status = AY_OK;
 ay_object *p = NULL, *t = NULL;

  if(!o || !p_m)
    return AY_ENULL;

  ay_status = ay_provide_object(o, AY_IDNCURVE, &p);
  if(p)
    {
      t = p;
      while(t)
	{
	  if(t->type == AY_IDNCURVE)
	    {
	      ay_status = onio_writencurve(t, p_m);
	    } /* if */
	  t = t->next;
	} /* while */

      ay_status = ay_object_deletemulti(p);

      return AY_OK;
    } /* if */

 return ay_status;
} // onio_writencconvertible


// onio_writeobject:
//
int
onio_writeobject(ay_object *o, ONX_Model *p_m)
{
 int ay_status = AY_OK;
 char fname[] = "onio_writeobject";
 Tcl_HashTable *ht = &onio_write_ht;
 Tcl_HashEntry *entry = NULL;
 char err[255];
 onio_writecb *cb = NULL;

  if(!o || !p_m)
    return AY_ENULL;

  if((entry = Tcl_FindHashEntry(ht, (char *)(o->type))))
    {
      cb = (onio_writecb*)Tcl_GetHashValue(entry);
      if(cb)
	{
	  ay_status = cb(o, p_m);
	  if(ay_status)
	    {
	      ay_error(AY_ERROR, fname, "Error exporting object.");
	      ay_status = AY_OK;
	    }
	}
    }
  else
    {
      snprintf(err, 254, "No callback registered for this type: %d.",
	       o->type);
      ay_error(AY_EWARN, fname, err);
    } // if

 return ay_status;
} // onio_writeobject


// onio_writetcmd:
//
int
onio_writetcmd(ClientData clientData, Tcl_Interp *interp,
	       int argc, char *argv[])
{
 int ay_status = AY_OK;
 char fname[] = "onio_write";
 FILE *fp = NULL;
 const char *filename = NULL;
 int version = 3;
 ay_object *o = ay_root;
 ONX_Model model;
 const ON_Layer *p_layer = NULL;
 ON_3dmObjectAttributes attribs;
 //ON_TextLog& error_log;


  // parse args
  if(argc < 2)
    {
      ay_error(AY_EARGS, fname, "filename");
      return TCL_OK;
    }

  filename = argv[1];
  fp = ON::OpenFile(filename, "wb");

  if(!fp)
    {
      ay_error(AY_EOPENFILE, fname, argv[1]);
      return TCL_OK;
    }

  // some notes
  //model.m_properties.m_Notes.m_notes = sNotes;
  //model.m_properties.m_Notes.m_bVisible = (model.m_properties.m_Notes.m_notes.Length() > 0);

  // set revision history information
  model.m_properties.m_RevisionHistory.NewRevision();

  // set application information
  model.m_properties.m_Application.m_application_name = "Ayam";
  model.m_properties.m_Application.m_application_URL = "http://www.ayam3d.org/";
  model.m_properties.m_Application.m_application_details = "onio (OpenNURBS) plugin";

  /*
  if( 0 != settings)
    model.m_settings = *settings;

  if(0 != material && material_count > 0)
  {
    model.m_material_table.Reserve(material_count);
    for (i = 0; i < material_count; i++)
      model.m_material_table.Append(material[i]);
  }
  */
  // layer table
  {
    // Each object in the object table (written below)
    // should be on a defined layer.  There should be
    // at least one layer with layer index 0 in every file.

    // layer table indices begin at 0
    ON_Layer default_layer;
    default_layer.SetLayerIndex(0);
    default_layer.SetLayerName("Default");
    p_layer = &default_layer;
    model.m_layer_table.Append(p_layer[0]);
  }

  // light table
  /*
  if (0 != light && light_count > 0)
  {
    for (i = 0; i < light_count; i++)
    {
      ONX_Model_RenderLight& mrl = model.m_light_table.AppendNew();
      mrl.m_light = light[i];
      if ( light_attributes )
        mrl.m_attributes = light_attributes[i];
    }
  }
  */

  // object table
  // get object attributes and make sure layer and material indices are legit
  while(o)
    {
      ay_status = onio_writeobject(o, &model);

      o = o->next;
    } // while

  // archive to write to
  ON_BinaryFile archive(ON::write3dm, fp);

  // set uuid's, indices, etc.
  model.Polish();

  // write model to archive
  bool ok = model.Write(archive,
                        version,
                        __FILE__ " onio_writetcmd() " __DATE__,
                        NULL/*&error_log*/);
  if(!ok)
    {
      ay_error(AY_ERROR, fname, "Error writing file!");
    }


  ON::CloseFile(fp);

 return TCL_OK;
} // onio_writetcmd


// onio_registerwritecb:
//
int
onio_registerwritecb(char *name, onio_writecb *cb)
{
 int ay_status = AY_OK;
 int new_item = 0;
 Tcl_HashEntry *entry = NULL;
 Tcl_HashTable *ht = &onio_write_ht;

  if(!cb)
    return AY_ENULL;

  if((entry = Tcl_FindHashEntry(ht, name)))
    {
      return AY_ERROR; /* name already registered */
    }
  else
    {
      /* create new entry */
      entry = Tcl_CreateHashEntry(ht, name, &new_item);
      Tcl_SetHashValue(entry, (char*)cb);
    }

 return ay_status;
} // onio_registerwritecb


////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////


// onio_readnurbssurface:
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


// onio_readnurbscurve:
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
    } // switch

 return ay_status;
} // onio_readobject


// onio_readtcmd:
//
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
 int ay_status = AY_OK;
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

  Tcl_CreateCommand(interp, "onioWrite", onio_writetcmd,
		    (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  // init hash table for write callbacks
  Tcl_InitHashTable(&onio_write_ht, TCL_ONE_WORD_KEYS);

  // fill hash table
  ay_status = onio_registerwritecb((char *)(AY_IDNPATCH),
				   onio_writenpatch);

  ay_status = onio_registerwritecb((char *)(AY_IDEXTRUDE),
				   onio_writenpconvertible);


  ay_status = onio_registerwritecb((char *)(AY_IDNCURVE),
				   onio_writencurve);

  ay_status = onio_registerwritecb((char *)(AY_IDICURVE),
				   onio_writencconvertible);


#ifndef ONIOWRAPPED
  ay_error(AY_EOUTPUT, fname, "Plugin 'onio' successfully loaded.");
#endif

 return TCL_OK;
} // Aycsg_Init | onio_inittcmd

} // extern "C"
