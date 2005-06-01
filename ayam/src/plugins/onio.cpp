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

typedef int (onio_writecb) (ay_object *o, ONX_Model *p_m, double *m);


// global variables

char onio_version_ma[] = AY_VERSIONSTR;
char onio_version_mi[] = AY_VERSIONSTRMI;

static Tcl_HashTable onio_write_ht;

ay_object *onio_lrobject = NULL;

static double tm[16] = {0}; // current transformation matrix

int onio_importcurves = AY_TRUE;
int onio_ignorefirsttrim = AY_TRUE;
int onio_exportcurves = AY_TRUE;
int onio_expsphereasbrep = AY_TRUE;
int onio_expcylinderasbrep = AY_TRUE;
int onio_expconeasbrep = AY_TRUE;
int onio_exptorusasbrep = AY_TRUE;
int onio_expselected = AY_FALSE;
int onio_expobeynoexport = AY_TRUE;
int onio_expignorehidden = AY_TRUE;

double onio_accuracy = 1.0e-12;


// prototypes of functions local to this module

int onio_transposetm(double *m1, double *m2);

int onio_getnurbsurfobj(ay_object *o, ON_NurbsSurface **pp_n, double *m);

int onio_writename(ay_object *o, ONX_Model_Object& p_mo);

int onio_prependname(ay_object *o, ONX_Model_Object& p_mo);

int onio_writenpatch(ay_object *o, ONX_Model *p_m, double *m);

int onio_get2dcurveobj(ay_object *o, ON_NurbsCurve **pp_c);

int onio_addtrim(ay_object *o, ON_BrepLoop::TYPE ltype,
		 ON_BrepTrim::TYPE ttype,
		 ON_Brep *p_b, ON_BrepFace *p_f);

bool onio_isboundingloop(ay_object *o);

int onio_writetrimmednpatch(ay_object *o, ONX_Model *p_m, double *m);

int onio_writenpconvertible(ay_object *o, ONX_Model *p_m, double *m);

int onio_writencurve(ay_object *o, ONX_Model *p_m, double *m);

int onio_writencconvertible(ay_object *o, ONX_Model *p_m, double *m);

int onio_writelevel(ay_object *o, ONX_Model *p_m, double *m);

int onio_writeclone(ay_object *o, ONX_Model *p_m, double *m);

int onio_writeinstance(ay_object *o, ONX_Model *p_m, double *m);

int onio_writescript(ay_object *o, ONX_Model *p_m, double *m);

int onio_writesphere(ay_object *o, ONX_Model *p_m, double *m);

int onio_writecylinder(ay_object *o, ONX_Model *p_m, double *m);

int onio_writecone(ay_object *o, ONX_Model *p_m, double *m);

int onio_writetorus(ay_object *o, ONX_Model *p_m, double *m);

int onio_writebox(ay_object *o, ONX_Model *p_m, double *m);

int onio_writeobject(ay_object *o, ONX_Model *p_m);

int onio_writetcmd(ClientData clientData, Tcl_Interp *interp,
		   int argc, char *argv[]);

int onio_registerwritecb(char *type, onio_writecb *cb);

/////////////////////////////////////////////////////////////////////////

static int onio_w2c_size(int, const wchar_t*);

static int onio_w2c(int,        // w_count = number of wide chars to convert
                const wchar_t*, // source wide char string
                int,            // c_count,
                char*           // array of at least c_count+1 characters
                );

int onio_readnurbssurface(ON_NurbsSurface *p_s);

int onio_readnurbscurve(ON_NurbsCurve *p_c);

int onio_getncurvefromcurve(const ON_Curve *p_o, double accuracy,
			    ON_NurbsCurve** pp_c);

int onio_readbrep(ON_Brep *p_b, double accuracy);

int onio_readobject(ONX_Model *p_m, const ON_Object *p_o, double accuracy);

int onio_readlayer(ONX_Model &model, int li, double accuracy);

int onio_readname(ay_object *o, ON_3dmObjectAttributes *attr);

int onio_readtcmd(ClientData clientData, Tcl_Interp *interp,
		  int argc, char *argv[]);

extern "C" {

int Onio_Init(Tcl_Interp *interp);

} // extern "C"


// functions

// onio_transposetm:
//  transpose 4x4 transformation matrix in <m1> from Ayam style to OpenNURBS
//  style, return result in <m2>
int
onio_transposetm(double *m1, double *m2)
{

  if(!m1 || !m2)
    return AY_ENULL;

  m2[0] = m1[0];
  m2[1] = m1[4];
  m2[2] = m1[8];
  m2[3] = m1[12];

  m2[4] = m1[1];
  m2[5] = m1[5];
  m2[6] = m1[9];
  m2[7] = m1[13];

  m2[8] = m1[2];
  m2[9] = m1[6];
  m2[10] = m1[10];
  m2[11] = m1[14];

  m2[12] = m1[3];
  m2[13] = m1[7];
  m2[14] = m1[11];
  m2[15] = m1[15];

 return AY_OK;
} // onio_transposetm


// onio_getnurbsurfobj:
//
int
onio_getnurbsurfobj(ay_object *o, ON_NurbsSurface **pp_n, double *m)
{
 int ay_status = AY_OK;
 int i, j, a, stride = 4;
 double *cv;
 ay_nurbpatch_object *np = NULL;
 ON_NurbsSurface *p_n = NULL;

  if(!o || !pp_n || !m)
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
  cv = p_n->m_cv;
  for(i = 0; i < np->width; i++)
    {
      for(j = 0; j < np->height; j++)
	{
	  p_n->SetCV(i, j, ON::homogeneous_rational, &(np->controlv[a]));
	  ay_trafo_apply4(cv, m);
	  cv += 4;
	  a += stride;
	} // for
    } // for

  // return result
  *pp_n = p_n;

 return ay_status;
} // onio_getnurbsurfobj


// onio_writename:
//
int
onio_writename(ay_object *o, ONX_Model_Object& p_mo)
{

  if(!o->name || (strlen(o->name) == 0))
    return AY_OK;
  
  ON_wString *p_name = new ON_wString(o->name);
  p_mo.m_attributes.m_name = *p_name;
  delete p_name;

 return AY_OK;
} // onio_writename


// onio_prependname:
//
int
onio_prependname(ay_object *o, ONX_Model_Object& p_mo)
{

  if(!o->name || (strlen(o->name) == 0))
    return AY_OK;
  
  ON_wString *p_name = new ON_wString(o->name);

  if(p_mo.m_attributes.m_name.IsEmpty())
    p_mo.m_attributes.m_name = *p_name;
  else
    p_mo.m_attributes.m_name = *p_name + '/' + p_mo.m_attributes.m_name;

  delete p_name;

 return AY_OK;
} // onio_prependname


// onio_writenpatch:
//
int
onio_writenpatch(ay_object *o, ONX_Model *p_m, double *m)
{
 int ay_status = AY_OK;
 ON_NurbsSurface *p_n = NULL;

  if(!o || !p_m || !m)
    return AY_ENULL;

  // is this patch trimmed?
  if(o->down && o->down->next)
    {
      // yes
      ay_status = onio_writetrimmednpatch(o, p_m, m);
      return ay_status;
    }

  ay_status = onio_getnurbsurfobj(o, &p_n, m);
  if(p_n)
    {
      ONX_Model_Object& mo = p_m->m_object_table.AppendNew();
      mo.m_object = p_n;
      mo.m_bDeleteObject = true;

      onio_writename(o, mo);

    } // if

 return ay_status;
} // onio_writenpatch


// onio_get2dcurveobj:
//  propagate changes to onio_writencurve()
int
onio_get2dcurveobj(ay_object *o, ON_NurbsCurve **pp_c)
{
 int ay_status = AY_OK;
 ay_nurbcurve_object *nc = NULL;
 double cv[4], m[16];
 int i, a, stride = 4;
 ON_NurbsCurve *p_c = NULL;

  if(!o)
    return AY_ENULL;

  nc = (ay_nurbcurve_object *)o->refine;
  p_c = new ON_NurbsCurve(2, true, nc->order, nc->length);

  // copy knots, ignoring "superfluous"/"phantom" end knots
  for(i = 0; i < nc->order+nc->length-2; i++)
    {
      p_c->SetKnot(i, nc->knotv[i+1]);
    }

  ay_trafo_creatematrix(o, m);

  // copy control points
  a = 0;
  for(i = 0; i < nc->length; i++)
    {
      memcpy(cv, &(nc->controlv[a]), stride*sizeof(double));
      ay_trafo_apply3(cv, m);
      cv[2] = cv[3];
      p_c->SetCV(i, ON::homogeneous_rational, cv);
      a += stride;
    }

  // return result
  *pp_c = p_c;

 return ay_status;
} // onio_get2dcurveobj


// onio_addtrim:
//
int
onio_addtrim(ay_object *o, ON_BrepLoop::TYPE ltype, ON_BrepTrim::TYPE ttype,
	     ON_Brep *p_b, ON_BrepFace *p_f)
{
 int ay_status = AY_OK;
 char fname[] = "onio_addtrim";
 ON_NurbsCurve c, c2, *p_c = NULL;
 ON_Curve *p_curve = NULL;
 unsigned int c2i, c3i;
 double tolerance = onio_accuracy;

  if(!o || !p_b)
    return AY_ENULL;

  if(o->type == AY_IDNCURVE)
    {
      onio_get2dcurveobj(o, &p_c);
      if(p_c)
	{
	  c2i = p_b->m_C2.Count();
	  p_b->m_C2.Append(p_c);
	  c3i = p_b->m_C3.Count();
	  c = *p_c;

	  const ON_Surface *pSurface = p_f->SurfaceOf();
	  p_curve = pSurface->Pushup(c, tolerance);
	  if(p_curve == NULL)
	    {
	      ay_error(AY_ERROR, fname, "pushup failed");
	      return AY_ERROR;
	    }

	  p_curve->GetNurbForm(c2, tolerance, NULL);
	  p_c = new ON_NurbsCurve(c2);
	  p_b->m_C3.Append(p_c);

	  delete p_curve;

	  ON_BrepVertex& v1 = p_b->NewVertex(p_c->PointAtStart());
	  v1.m_tolerance = 0.0;
	  // no need to create a second vertex for a closed curve...
	  //ON_BrepVertex& v2 = p_b->NewVertex(p_c->PointAtEnd());
	  //v2.m_tolerance = 0.0;

	  ON_BrepEdge& edge = p_b->NewEdge(v1, v1, c3i);
	  edge.m_tolerance = 0.0;

	  ON_BrepLoop& loop = p_b->NewLoop(ltype);
	  loop.m_fi = p_f->m_face_index;
	  if(ON_BrepLoop::outer == ltype)
	    {
	      // the index of the outer loop is always
	      // in face.m_li[0]
	      p_f->m_li.Insert(0,loop.m_loop_index);
	    }
	  else
	    {
	      p_f->m_li.Append(loop.m_loop_index);
	    }

	  ON_BrepTrim& trim = p_b->NewTrim(edge, false, loop, c2i);
	  trim.m_type = ttype;
	  trim.m_tolerance[0] = 0.0;
	  trim.m_tolerance[1] = 0.0;

	} // if
    } // if

 return ay_status;
} // onio_addtrim


// onio_isboundingloop:
//
bool
onio_isboundingloop(ay_object *o)
{
 ay_nurbcurve_object *nc = NULL;

  if(!o)
    return false;

  if(!o->type == AY_IDNCURVE)
    return false;

  nc = (ay_nurbcurve_object*)o->refine;

  if(nc->order != 2 || nc->length != 4)
    return false;

  // XXXX add check control points

 return true;
} // onio_isboundingloop


// onio_writetrimmednpatch:
//
int
onio_writetrimmednpatch(ay_object *o, ONX_Model *p_m, double *m)
{
 int ay_status = AY_OK;
 ay_object *down;
 ay_nurbpatch_object *np = NULL;
 ON_NurbsSurface s, *p_s = NULL;
 ON_Plane plane, *p_p = NULL;
 ON_PlaneSurface ps, *p_ps = NULL;
 ON_Brep *p_b = NULL;
 ON_BrepFace *p_f = NULL;
 double tolerance = onio_accuracy;
 ON_BrepLoop::TYPE ltype = ON_BrepLoop::inner;

  if(!o || !p_m || !m)
    return AY_ENULL;

  np = (ay_nurbpatch_object*)(o->refine);

  ay_status = onio_getnurbsurfobj(o, &p_s, m);
  if(p_s == NULL)
    return ay_status;

  if((np->width == 2) && (np->height == 2) &&
     (p_s->IsPlanar(&plane, tolerance)))
    {
      // assume, that we have here an Ayam cap surface that can be converted
      // to a rectangular PlaneSurface instead of a NurbsSurface
      p_p = new ON_Plane();
      *p_p = plane;
      p_ps = new ON_PlaneSurface();
      ON_Interval ext;
      // XXXX the use of ControlPolygonLength() further assumes that the
      // quadrilateral we have here is a rectangle (what it surely does
      // not have to be!)
      double w = p_s->ControlPolygonLength(0);
      double minx = -w/2.0, maxx = w/2.0;
      ext.Set(minx, maxx);
      p_ps->SetExtents(0, ext, false);
      minx = np->uknotv[np->uorder-1];
      maxx = np->uknotv[np->width];
      p_ps->SetDomain(0, minx, maxx);

      double h = p_s->ControlPolygonLength(1);
      double miny = -h/2.0, maxy = h/2.0;
      ext.Set(miny, maxy);
      p_ps->SetExtents(1, ext, false);
      miny = np->vknotv[np->vorder-1];
      maxy = np->vknotv[np->height];
      p_ps->SetDomain(1, miny, maxy);

      p_ps->m_plane = *p_p;

      ps = *p_ps;
      p_b = new ON_Brep();
      // create new face from surface (creates a bounding trimloop as well!)
      p_f = p_b->NewFace(ps);

      // cleanup
      delete p_s;
      p_s = NULL;
      delete p_p;
      p_p = NULL;
      delete p_ps;
      p_ps = NULL;
    }
  else
    {
      s = *p_s;
      p_b = new ON_Brep();
      // create new face from surface (creates a bounding trimloop as well!)
      p_f = p_b->NewFace(s);
    } // if

  down = o->down;

  // remove earlier created bounding trimloop because we cut away the
  // outside with our own trims?
  if(down && !onio_isboundingloop(down))
    {
      // Yes.
      p_b->DeleteLoop(p_b->m_L[0], true);
      p_b->Compact();
      ltype = ON_BrepLoop::outer;
    }

  while(down && down->next)
    {
      if(!onio_isboundingloop(down))
	{
	  ay_status = onio_addtrim(down,
				   ltype,
				   ON_BrepTrim::boundary,
				   p_b, p_f);
	  // XXXX check ay_status
	  ltype = ON_BrepLoop::inner;
	} // if

      down = down->next;
    } // while

  ONX_Model_Object& mo = p_m->m_object_table.AppendNew();
  mo.m_object = p_b;
  mo.m_bDeleteObject = true;

  onio_writename(o, mo);

 return ay_status;
} // onio_writetrimmednpatch


// onio_writenpconvertible:
//
int
onio_writenpconvertible(ay_object *o, ONX_Model *p_m, double *m)
{
 int ay_status = AY_OK;
 ay_object *p = NULL, *t = NULL;

  if(!o || !p_m || !m)
    return AY_ENULL;

  ay_status = ay_provide_object(o, AY_IDNPATCH, &p);
  if(p)
    {
      int first = p_m->m_object_table.Count();
      t = p;
      while(t)
	{
	  if(t->type == AY_IDNPATCH)
	    {
	      // do not use m but tm because m already contains the
	      // transformations of o and the provided objects (p)
	      // do so as well
	      // ay_status = onio_writenpatch(t, p_m, tm);

	      ay_status = onio_writeobject(t, p_m);

	    } // if
	  t = t->next;
	} // while

      ay_status = ay_object_deletemulti(p);

      int last = p_m->m_object_table.Count();
      for(int i = first; i < last; i++)
	onio_writename(o, p_m->m_object_table[i]);

      return AY_OK;
    } // if

 return ay_status;
} // onio_writenpconvertible


// onio_writencurve:
//  propagate changes to onio_get2dcurveobj()
int
onio_writencurve(ay_object *o, ONX_Model *p_m, double *m)
{
 int ay_status = AY_OK;
 int i, a, stride = 4;
 double *cv;
 ay_nurbcurve_object *nc = NULL;
 ON_NurbsCurve *p_c = NULL;

  if(!o || !p_m || !m)
    return AY_ENULL;

  if(!onio_exportcurves)
    return AY_OK;

  nc = (ay_nurbcurve_object *)o->refine;
  p_c = new ON_NurbsCurve(3, true, nc->order, nc->length);

  // copy knots, ignoring "superfluous"/"phantom" end knots
  for(i = 0; i < nc->order+nc->length-2; i++)
    p_c->SetKnot(i, nc->knotv[i+1]);

  // copy control points
  a = 0;
  cv = p_c->m_cv;
  for(i = 0; i < nc->length; i++)
    {
      p_c->SetCV(i, ON::homogeneous_rational, &(nc->controlv[a]));
      ay_trafo_apply4(cv, m);
      cv += 4;
      a += stride;
    }

  ONX_Model_Object& mo = p_m->m_object_table.AppendNew();
  mo.m_object = p_c;
  mo.m_bDeleteObject = true;

  onio_writename(o, mo);

 return ay_status;
} // onio_writencurve


// onio_writencconvertible:
//
int
onio_writencconvertible(ay_object *o, ONX_Model *p_m, double *m)
{
 int ay_status = AY_OK;
 ay_object *p = NULL, *t = NULL;

  if(!o || !p_m || !m)
    return AY_ENULL;

  if(!onio_exportcurves)
    return AY_OK;

  ay_status = ay_provide_object(o, AY_IDNCURVE, &p);
  if(p)
    {
      int first = p_m->m_object_table.Count();
      t = p;
      while(t)
	{
	  if(t->type == AY_IDNCURVE)
	    {
	      // do not use m but tm because m already contains the
	      // transformations of o and the provided objects (p)
	      // do so as well
	      // ay_status = onio_writencurve(t, p_m, tm);

	      ay_status = onio_writeobject(t, p_m);

	    } // if
	  t = t->next;
	} // while

      ay_status = ay_object_deletemulti(p);

      int last = p_m->m_object_table.Count();
      for(int i = first; i < last; i++)
	onio_writename(o, p_m->m_object_table[i]);

      return AY_OK;
    } // if

 return ay_status;
} // onio_writencconvertible


// onio_writelevel:
//
int
onio_writelevel(ay_object *o, ONX_Model *p_m, double *m)
{
 int ay_status = AY_OK;
 ay_object *down = NULL;
 ay_level_object *l = NULL;
 double m1[16] = {0};

  if(!o || !p_m || !m)
    return AY_ENULL;

  l = (ay_level_object *)o->refine;

  if(l->type == AY_LTEND)
    return AY_OK;

  if(o->down && o->down->next)
    {
      memcpy(m1, tm, 16*sizeof(double));
      memcpy(tm, m, 16*sizeof(double));

      int first = p_m->m_object_table.Count();

      down = o->down;
      while(down->next)
	{
	  ay_status = onio_writeobject(down, p_m);
	  down = down->next;
	}

      int last = p_m->m_object_table.Count();
      for(int i = first; i < last; i++)
	onio_prependname(o, p_m->m_object_table[i]);

      memcpy(tm, m1, 16*sizeof(double));
    } // if

 return ay_status;
} // onio_writelevel


// onio_writeclone:
//
int
onio_writeclone(ay_object *o, ONX_Model *p_m, double *m)
{
 int ay_status = AY_OK;
 ay_clone_object *cl = NULL;
 ay_object *clone;

  if(!o || !p_m || !m)
    return AY_ENULL;

  cl = (ay_clone_object *)o->refine;

  clone = cl->clones;

  while(clone)
    {
      ay_status = onio_writeobject(clone, p_m);

      clone = clone->next;
    } // while

 return ay_status;
} // onio_writeclone


// onio_writeinstance:
//
int
onio_writeinstance(ay_object *o, ONX_Model *p_m, double *m)
{
 int ay_status = AY_OK;
 ay_object *orig, tmp = {0};

  if(!o || !p_m || !m)
    return AY_ENULL;

  orig = (ay_object *)o->refine;

  ay_trafo_copy(orig, &tmp);
  ay_trafo_copy(o, orig);
  ay_status = onio_writeobject(orig, p_m);
  ay_trafo_copy(&tmp, orig);

 return ay_status;
} // onio_writeinstance


// onio_writescript:
//
int
onio_writescript(ay_object *o, ONX_Model *p_m, double *m)
{
 int ay_status = AY_OK;
 ay_script_object *sc = NULL;
 ay_object *cm;

  if(!o || !p_m || !m)
    return AY_ENULL;

  sc = (ay_script_object *)o->refine;

  if(((sc->type == 1) || (sc->type == 2)) && (sc->cm_objects))
    {
      cm = sc->cm_objects;
      while(cm)
	{
	  ay_status = onio_writeobject(cm, p_m);

	  cm = cm->next;
	} // while
    } // if

 return ay_status;
} // onio_writescript


// onio_writesphere:
//
int
onio_writesphere(ay_object *o, ONX_Model *p_m, double *m)
{
 int ay_status = AY_OK;
 double tm[16] = {0};
 ay_sphere_object *sphere = NULL;
 ON_Sphere *p_sp = NULL;
 ON_3dPoint center(0.0, 0.0, 0.0);

  if(!o || !p_m || !m)
    return AY_ENULL;

  sphere = (ay_sphere_object *)o->refine;

  onio_transposetm(m, tm);

  ON_Xform xform(tm);

  p_sp = new ON_Sphere(center, sphere->radius);

  if(p_sp)
    {
      if(!onio_expsphereasbrep)
	{
	  ay_status = onio_writenpconvertible(o, p_m, m);
	  /*
	  ON_NurbsSurface su, *p_su = NULL;

	  p_sp->GetNurbForm(su);

	  su.Transform(xform);

	  p_su = new ON_NurbsSurface(su);
	  if(p_su)
	    {
	      ONX_Model_Object& mo = p_m->m_object_table.AppendNew();
	      mo.m_object = p_su;
	      mo.m_bDeleteObject = true;
	      
	      onio_writename(o, mo);

	    } // if
	  */
	}
      else
	{
	  ON_Sphere sp = *p_sp;
	  ON_Brep *p_b = ON_BrepSphere(sp, NULL);

	  if(p_b)
	    {
	      p_b->Transform(xform);
	      ONX_Model_Object& mo = p_m->m_object_table.AppendNew();
	      mo.m_object = p_b;
	      mo.m_bDeleteObject = true;

	      onio_writename(o, mo);

	    } // if
	} // if
      delete p_sp;
    } // if

 return ay_status;
} // onio_writesphere


// onio_writecylinder:
//
int
onio_writecylinder(ay_object *o, ONX_Model *p_m, double *m)
{
 int ay_status = AY_OK;
 double tm[16] = {0};
 ay_cylinder_object *cylinder = NULL;
 ON_Cylinder *p_cy = NULL;


  if(!o || !p_m || !m)
    return AY_ENULL;

  cylinder = (ay_cylinder_object *)o->refine;
  double zmin = (cylinder->zmin<cylinder->zmax)?cylinder->zmin:cylinder->zmax;
  double zmax = (cylinder->zmin>cylinder->zmax)?cylinder->zmin:cylinder->zmax;

  ON_3dPoint center(0.0, 0.0, zmin);

  ON_Circle circle(center, cylinder->radius);

  onio_transposetm(m, tm);

  ON_Xform xform(tm);

  p_cy = new ON_Cylinder(circle, zmax-zmin);

  if(p_cy)
    {
      if(!onio_expcylinderasbrep)
        {
	  ay_status = onio_writenpconvertible(o, p_m, m);
	  /*
          ON_NurbsSurface su, *p_su = NULL;

          p_cy->GetNurbForm(su);

          su.Transform(xform);

          p_su = new ON_NurbsSurface(su);
          if(p_su)
            {
              ONX_Model_Object& mo = p_m->m_object_table.AppendNew();
              mo.m_object = p_su;
              mo.m_bDeleteObject = true;

	      onio_writename(o, mo);

	    } // if
	  */
        }
      else
        {
          ON_Cylinder cy = *p_cy;
          ON_Brep *p_b = ON_BrepCylinder(cy, cylinder->closed,
                                         cylinder->closed, NULL);

          if(p_b)
            {
              p_b->Transform(xform);
              ONX_Model_Object& mo = p_m->m_object_table.AppendNew();
              mo.m_object = p_b;
              mo.m_bDeleteObject = true;

	      onio_writename(o, mo);

            } // if
        } // if
      delete p_cy;
    } // if

 return ay_status;
} // onio_writecylinder


// onio_writecone:
//
int
onio_writecone(ay_object *o, ONX_Model *p_m, double *m)
{
 int ay_status = AY_OK;
 double tm[16] = {0};
 ay_cone_object *cone = NULL;
 ON_Cone *p_co = NULL;

  if(!o || !p_m || !m)
    return AY_ENULL;

  cone = (ay_cone_object *)o->refine;

  ON_3dPoint center(0.0, 0.0, cone->height);
  ON_3dVector normal(0.0, 0.0, -1.0);
  ON_Plane plane(center, normal);

  onio_transposetm(m, tm);

  ON_Xform xform(tm);

  p_co = new ON_Cone(plane, cone->height, cone->radius);

  if(p_co)
    {
      if(!onio_expconeasbrep)
        {
	  ay_status = onio_writenpconvertible(o, p_m, m);
	  /*
          ON_NurbsSurface su, *p_su = NULL;

          p_co->GetNurbForm(su);

          su.Transform(xform);

          p_su = new ON_NurbsSurface(su);
          if(p_su)
            {
              ONX_Model_Object& mo = p_m->m_object_table.AppendNew();
              mo.m_object = p_su;
              mo.m_bDeleteObject = true;

	      onio_writename(o, mo);

	    } // if
	  */ 
        }
      else
        {
          ON_Cone co = *p_co;
          ON_Brep *p_b = ON_BrepCone(co, cone->closed, NULL);

          if(p_b)
            {
              p_b->Transform(xform);
              ONX_Model_Object& mo = p_m->m_object_table.AppendNew();
              mo.m_object = p_b;
              mo.m_bDeleteObject = true;

	      onio_writename(o, mo);

            } // if
        } // if
      delete p_co;
    } // if

 return ay_status;
} // onio_writecone


// onio_writetorus:
//
int
onio_writetorus(ay_object *o, ONX_Model *p_m, double *m)
{
 int ay_status = AY_OK;
 double tm[16] = {0};
 ay_torus_object *torus = NULL;
 ON_Torus *p_to = NULL;

  if(!o || !p_m || !m)
    return AY_ENULL;

  torus = (ay_torus_object *)o->refine;

  ON_3dPoint center(0.0, 0.0, 0.0);
  ON_3dVector normal(0.0, 0.0, 1.0);
  ON_Plane plane(center, normal);

  onio_transposetm(m, tm);

  ON_Xform xform(tm);

  p_to = new ON_Torus(plane, torus->majorrad, torus->minorrad);

  if(p_to)
    {
      if(!onio_exptorusasbrep)
	{
	  ay_status = onio_writenpconvertible(o, p_m, m);
	  /*
	  ON_NurbsSurface su, *p_su = NULL;

	  p_to->GetNurbForm(su);

	  su.Transform(xform);

	  p_su = new ON_NurbsSurface(su);
	  if(p_su)
	    {
	      ONX_Model_Object& mo = p_m->m_object_table.AppendNew();
	      mo.m_object = p_su;
	      mo.m_bDeleteObject = true;

	      onio_writename(o, mo);

	    } // if
	  */
	}
      else
	{
	  ON_Torus to = *p_to;
	  ON_Brep *p_b = ON_BrepTorus(to, NULL);

	  if(p_b)
	    {
	      p_b->Transform(xform);
	      ONX_Model_Object& mo = p_m->m_object_table.AppendNew();
	      mo.m_object = p_b;
	      mo.m_bDeleteObject = true;

	      onio_writename(o, mo);

	    } // if
	} // if

      delete p_to;
    } // if

 return ay_status;
} // onio_writetorus


// onio_writebox:
//
int
onio_writebox(ay_object *o, ONX_Model *p_m, double *m)
{
 int ay_status = AY_OK;
 double tm[16] = {0};
 ay_box_object *box = NULL;
 ON_3dPoint corners[8];

  if(!o || !p_m || !m)
    return AY_ENULL;

  box = (ay_box_object *)o->refine;

  onio_transposetm(m, tm);

  ON_Xform xform(tm);

  corners[0].x = -(box->width/2.0);
  corners[0].y = -(box->height/2.0);
  corners[0].z =  (box->length/2.0);

  corners[1].x =  (box->width/2.0);
  corners[1].y = -(box->height/2.0);
  corners[1].z =  (box->length/2.0);

  corners[2].x =  (box->width/2.0);
  corners[2].y = -(box->height/2.0);
  corners[2].z = -(box->length/2.0);

  corners[3].x = -(box->width/2.0);
  corners[3].y = -(box->height/2.0);
  corners[3].z = -(box->length/2.0);


  corners[4].x = -(box->width/2.0);
  corners[4].y =  (box->height/2.0);
  corners[4].z =  (box->length/2.0);

  corners[5].x =  (box->width/2.0);
  corners[5].y =  (box->height/2.0);
  corners[5].z =  (box->length/2.0);

  corners[6].x =  (box->width/2.0);
  corners[6].y =  (box->height/2.0);
  corners[6].z = -(box->length/2.0);

  corners[7].x = -(box->width/2.0);
  corners[7].y =  (box->height/2.0);
  corners[7].z = -(box->length/2.0);

  ON_Brep *p_b = ON_BrepBox(corners, NULL);

  if(p_b)
    {
      p_b->Transform(xform);
      ONX_Model_Object& mo = p_m->m_object_table.AppendNew();
      mo.m_object = p_b;
      mo.m_bDeleteObject = true;

      onio_writename(o, mo);

    } // if

 return ay_status;
} // onio_writebox


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
 double m1[16] = {0}, m2[16];
 ay_tag_object *t = NULL;

  if(!o || !p_m)
    return AY_ENULL;

  if(onio_expselected && !o->selected)
    return AY_OK;

  if(onio_expobeynoexport && o->tags)
    {
      t = o->tags;
      while(t)
	{
	  if(t->type == ay_noexport_tagtype)
	    {
	      return AY_OK;
	    } // if
	  t = t->next;
	} // while
    } // if

  if(onio_expignorehidden && o->hide)
    return AY_OK;

  if((entry = Tcl_FindHashEntry(ht, (char *)(o->type))))
    {
      cb = (onio_writecb*)Tcl_GetHashValue(entry);
      if(cb)
	{
	  if((o->movx != 0.0) || (o->movy != 0.0) || (o->movz != 0.0) ||
	     (o->rotx != 0.0) || (o->roty != 0.0) || (o->rotz != 0.0) ||
	     (o->scalx != 1.0) || (o->scaly != 1.0) || (o->scalz != 1.0) ||
	     (o->quat[0] != 0.0) || (o->quat[1] != 0.0) ||
	     (o->quat[2] != 0.0) || (o->quat[3] != 1.0))
	    {
	      ay_trafo_creatematrix(o, m1);
	      memcpy(m2, tm, 16*sizeof(double));
	      ay_trafo_multmatrix4(m2, m1);
	      ay_status = cb(o, p_m, m2);
	    }
	  else
	    {
	      ay_status = cb(o, p_m, tm);
	    } // if

	  if(ay_status)
	    {
	      ay_error(AY_ERROR, fname, "Error exporting object.");
	      ay_status = AY_OK;
	    } // if
	} // if
    }
  else
    {
      sprintf(err, "Cannot export objects of type: %s.",
	      ay_object_gettypename(o->type));
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
 int t, i = 2, version = 3;
 ay_object *o = ay_root;
 ONX_Model model;
 const ON_Layer *p_layer = NULL;
 ON_3dmObjectAttributes attribs;
 //ON_TextLog& error_log;

  // check args
  if(argc < 2)
    {
      ay_error(AY_EARGS, fname, "filename");
      return TCL_OK;
    }

  // parse args
  filename = argv[1];

  while(i+1 < argc)
    {
      if(!strcmp(argv[i], "-c"))
	{
	  sscanf(argv[i+1], "%d", &onio_exportcurves);
	}
      else
      if(!strcmp(argv[i], "-q"))
	{
	  sscanf(argv[i+1], "%d", &t);
	  onio_expsphereasbrep = t;
	  onio_expcylinderasbrep = t;
	  onio_expconeasbrep = t;
	  onio_exptorusasbrep = t;
	}
      else
      if(!strcmp(argv[i], "-s"))
	{
	  sscanf(argv[i+1], "%d", &onio_expselected);
	}
      else
      if(!strcmp(argv[i], "-o"))
	{
	  sscanf(argv[i+1], "%d", &onio_expobeynoexport);
	}
      else
      if(!strcmp(argv[i], "-i"))
	{
	  sscanf(argv[i+1], "%d", &onio_expignorehidden);
	}
      i+=2;
    } // while

  // open the file for writing
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

  // fill object table
  while(o)
    {
      ay_trafo_identitymatrix(tm);

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
      return AY_ERROR; // name already registered
    }
  else
    {
      // create new entry
      entry = Tcl_CreateHashEntry(ht, name, &new_item);
      Tcl_SetHashValue(entry, (char*)cb);
    }

 return ay_status;
} // onio_registerwritecb


////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////


// onio_w2c_size:
// gets minimum "c_count" arg for onio_w2c() below
// taken from opennurbs_wstring.cpp
static int onio_w2c_size( int w_count, const wchar_t* w )
{
  // returns number of bytes used in wide conversion.  Does not
  // include NULL terminator.
  int rc = 0;
  if ( w ) {
	  rc = on_WideCharToMultiByte(w, w_count, NULL, 0);
    if ( rc < 0 )
      rc = 0;
  }
  return rc;
} // onio_w2c_size


// onio_w2c:
// convert wide chars to ASCII chars
// taken from opennurbs_wstring.cpp
static int onio_w2c( int w_count,
                const wchar_t* w,
                int c_count,
                char* c // array of at least c_count+1 characters
                )
{
  int rc = 0;
  if ( c )
    c[0] = 0;
  // returns length of converted c[]
  if ( c_count > 0 && c ) {
    c[0] = 0;
    if ( w ) {
	    rc = on_WideCharToMultiByte(w, w_count, c, c_count);
      if ( rc > 0 && rc <= c_count )
        c[rc] = 0;
      else {
        c[c_count] = 0;
        rc = 0;
      }
    }
  }
  return rc;
} // onio_w2c


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

  ay_object_crtendlevel(&(newo->down));

  // link the new patch into the scene hierarchy
  ay_status = ay_object_link(newo);

  if(ay_status)
    ay_status = ay_object_delete(newo);
  else
    onio_lrobject = newo;

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
    }
  else
    {
      if(p_c->m_dim == 2)
	{
	  b = 2;
	  for(i = 0; i < length; i++)
	    {
	      controlv[b+1] = controlv[b];
	      controlv[b] = 0.0;
	      b += 4;
	    } // for
	} // if
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
  else
    onio_lrobject = newo;

 return ay_status;
} // onio_readnurbscurve


// onio_getncurvefromcurve:
//
int
onio_getncurvefromcurve(const ON_Curve *p_o, double accuracy,
			ON_NurbsCurve **pp_c)
{
 ON_NurbsCurve c;
 int handled = AY_FALSE;

  if(!p_o || ! pp_c)
   return AY_ENULL;

  if(ON_NurbsCurve::Cast(p_o))
    {
      (ON_NurbsCurve::Cast(p_o))->GetNurbForm(c, accuracy, NULL);
      handled = AY_TRUE;
    }
  if(ON_PolylineCurve::Cast(p_o))
    {
      if((ON_PolylineCurve::Cast(p_o))->GetNurbForm(c, accuracy, NULL))
	{
	  handled = AY_TRUE;
	}
      else
	{
	  return AY_ERROR;
	}
    }
  if(ON_PolyCurve::Cast(p_o))
    {
      if((ON_PolyCurve::Cast(p_o))->GetNurbForm(c, accuracy, NULL))
	{
	  handled = AY_TRUE;
	}
      else
	{
	  return AY_ERROR;
	}
    }
  if(ON_LineCurve::Cast(p_o))
    {
      if((ON_LineCurve::Cast(p_o))->GetNurbForm(c, accuracy, NULL))
	{
	  handled = AY_TRUE;
	}
      else
	{
	  return AY_ERROR;
	}
    }
  if(ON_ArcCurve::Cast(p_o))
    {
      if((ON_ArcCurve::Cast(p_o))->GetNurbForm(c, accuracy, NULL))
	{
	  handled = AY_TRUE;
	}
      else
	{
	  return AY_ERROR;
	}
    }
  if(ON_CurveOnSurface::Cast(p_o))
    {
      if((ON_CurveOnSurface::Cast(p_o))->GetNurbForm(c, accuracy, NULL))
      {
	 handled = AY_TRUE;
	}
      else
	{
	  return AY_ERROR;
	}
    }

  if(!handled)
    return AY_ERROR;

  // return result
  *pp_c = new ON_NurbsCurve();
  **pp_c = c;

 return AY_OK;
} // onio_getncurvefromcurve


// onio_readbrep:
//
int
onio_readbrep(ON_Brep *p_b, double accuracy)
{
 int ay_status = AY_OK;
 char fname[] = "onio_readbrep";
 int i;
 ON_NurbsSurface s;
 const ON_Surface* p_s = NULL;
 ay_object *lo = NULL, *o, *lf;
 ay_level_object *level = NULL;


  for(i = 0; i < p_b->m_F.Count(); ++i)
    {
      // XXXX should put multiple faces in one level to apply transformations
      // more easily!
      const ON_BrepFace& face = p_b->m_F[i];

      if(face.m_si < 0 || face.m_si >= p_b->m_S.Count())
	{
	  // invalid brep
	  ay_error(AY_ERROR, fname, "invalid brep (wrong surface index)");
	  return AY_ERROR;
	}

      p_s = p_b->m_S[face.m_si];
      if(!p_s)
	{
	  // invalid brep
	  ay_error(AY_ERROR, fname, "invalid brep (surface not found)");
	  return AY_ERROR;
	} // if

      if(p_s->GetNurbForm(s, accuracy))
	{
	  ay_status = onio_readnurbssurface(&s);
	  lf = onio_lrobject;
	}
      else
	{
	  ay_error(AY_ERROR, fname,
	  "Unable to convert brep face; continuing with next face.");
	  continue;
	}

      if(ay_status)
	return ay_status;

      // loop_count = number of trimming loops on this face (>=1)
      const int loop_count = face.m_li.Count();

      int fli; // face's loop index
      for(fli = 0; fli < loop_count; fli++)
	{
	  if(onio_ignorefirsttrim && fli == 0)
	    continue;

	  const int li = face.m_li[fli]; // li = brep loop index
	  const ON_BrepLoop& loop = p_b->m_L[li];

	  // loop_edge_count = number of trimming edges in this loop
	  const int loop_trim_count = loop.m_ti.Count();

	  // do we need to create a level object?
	  if(loop_trim_count > 1)
	    {
	      // yes

	      if(ay_next == &(onio_lrobject->next))
		{
		  ay_next = &(onio_lrobject->down);
		}

	      if(!(level = (ay_level_object *)calloc(1,
			    sizeof(ay_level_object))))
		{
		  return AY_EOMEM;
		}

	      level->type = AY_LTLEVEL;

	      if(!(lo = (ay_object *) calloc(1, sizeof(ay_object))))
		{
		  return AY_EOMEM;
		}

	      lo->type = AY_IDLEVEL;
	      lo->refine = level;
	      lo->parent = AY_TRUE;
	      lo->inherit_trafos = AY_TRUE;

	      ay_status = ay_object_crtendlevel(&(lo->down));

	      ay_status = ay_object_link(lo);

	      ay_next = &(lo->down);
	    } // if

	  int lti; // loop's trim index
	  for(lti = 0; lti < loop_trim_count; lti++)
	    {
	      const int ti = loop.m_ti[lti]; // ti = brep trim index
	      const ON_BrepTrim& trim = p_b->m_T[ti];

	      //////////////////////////////////////////////////////
	      // 2d trimming information
	      //
	      // Each trim has a 2d parameter space curve.
	      ON_Curve* p_c = NULL;
	      const int c2i = trim.m_c2i; // c2i = brep 2d curve index
	      if(c2i < 0 || c2i >= p_b->m_C2.Count())
		{
		  // invalid brep m_T[ti].m_c2i
		  ay_error(AY_ERROR, fname, "invalid brep (2dcurve index)");
		  continue;
		  //return AY_ERROR;
		}

	      p_c = p_b->m_C2[c2i];
	      if(!p_c)
		{
		  // invalid brep m_C2[c2i] is NULL
		  ay_error(AY_ERROR, fname,
			   "invalid brep (cannot find 2dcurve)");
		  continue;
		  //return AY_ERROR;
		}

	      // add trim curve to Ayam NURBSPatch object
	      if((loop_trim_count < 2) && (ay_next == &(onio_lrobject->next)))
		{
		  ay_next = &(onio_lrobject->down);
		}

	      ON_NurbsCurve* p_nc = NULL;
	      ay_status = onio_getncurvefromcurve(p_c, accuracy, &p_nc);
	      if(ay_status)
		{
		  ay_error(AY_ERROR, fname,
		      "Unable to convert trim curve; continuing with next.");
		  continue;
		  //return AY_ERROR;
		}

	      ay_status = onio_readnurbscurve(p_nc);

	      delete p_nc;

	      if(ay_status)
		{
		  ay_error(AY_ERROR, fname,
		      "Unable to convert trim curve; continuing with next.");
		  continue;
		  //return AY_ERROR;
		}

	      // XXXX do we need to decode the topology?

	      //////////////////////////////////////////////////////
	      // topology and 3d geometry information
	      //

	      // Trim starts at v0 and ends at v1.  When the trim
	      // is a loop or on a singular surface side, v0i and v1i
	      // will be equal.
	      //const int v0i = trim.m_vi[0]; // v0i = brep vertex index
	      //const int v1i = trim.m_vi[1]; // v1i = brep vertex index
	      //const ON_BrepVertex& v0 = p_b->m_V[v0i];
	      //const ON_BrepVertex& v1 = p_b->m_V[v1i];
	      // The vX.m_ei[] array contains the p_b->m_E[] indices of
	      // the edges that begin or end at vX.
#if 0
	      const int ei = trim.m_ei;
	      if(ei == -1)
		{
		  // This trim lies on a portion of a singular surface side.
		  // The vertex indices are still valid and will be equal.
		}
	      else
		{
		  // If trim.m_bRev3d is FALSE, the orientations of the 3d edge
		  // and the 3d curve obtained by composing the surface and 2d
		  // curve agree.
		  //
		  // If trim.m_bRev3d is TRUE, the orientations of the 3d edge
		  // and the 3d curve obtained by composing the surface and 2d
		  // curve are opposite.
		  const ON_BrepEdge& edge = p_b->m_E[ei];
		  const int c3i = edge.m_c3i;
		  const ON_Curve* p3dCurve = NULL;

		  if(c3i < 0 || c3i >= p_b->m_C3.Count())
		    {
		      // invalid brep m_E[%d].m_c3i
		      return AY_ERROR;
		    }
		  else
		    {
		      p3dCurve = p_b->m_C3[c3i];
		      if(!p3dCurve)
			{
			  // invalid brep m_C3[%d] is NULL
			  return AY_ERROR;
			}
		    } // if

		  // The edge.m_ti[] array contains the p_b->m_T[] indices
		  // for the other trims that are joined to this edge.
		} // if
#endif

	    } // for
	  // do we need to repair ay_next because we created a level?
	  if(loop_trim_count > 1)
	    {
	      // yes
	      ay_next = &(lo->next);
	    } // if
	} // for
      ay_next = &(lf->next);
    } // for

  if(onio_lrobject && onio_lrobject->down)
    {
      o = onio_lrobject->down;

      while(o->next)
	o = o->next;

      ay_status = ay_object_crtendlevel(&(o->next));
    } // if

 return ay_status;
} // onio_readbrep


// onio_readreference:
//
int
onio_readreference(ONX_Model *p_m, ON_InstanceRef *p_r, double accuracy)
{
 int ay_status = AY_OK;

  if(!p_m || !p_r)
    return AY_ENULL;

  ON_UUID uuid = p_r->m_instance_definition_uuid;
  if(p_m->m_object_table[p_m->ObjectIndex(uuid)].m_object)
    {
      ON_Object *p_o =
     (p_m->m_object_table[p_m->ObjectIndex(uuid)].m_object)->DuplicateObject();
      if(p_o && ON_Geometry::Cast(p_o))
	{
	  ((ON_Geometry*)p_o)->Transform(p_r->m_xform);
	  ay_status = onio_readobject(p_m, p_o, accuracy);
	} // if
      if(p_o)
	delete p_o;
    } // if

 return ay_status;
} // onio_readreference


// onio_readobject:
//
int
onio_readobject(ONX_Model *p_m, const ON_Object *p_o, double accuracy)
{
 int ay_status = AY_OK;
 ON_NurbsSurface s;
 ON_NurbsCurve c;

  if(!p_m || !p_o)
    return AY_ENULL;

  switch(p_o->ObjectType())
    {
    case ON::curve_object:
      if(onio_importcurves)
	{
	  if(ON_NurbsCurve::Cast(p_o))
	    ay_status = onio_readnurbscurve((ON_NurbsCurve*)p_o);
	  if(ON_PolylineCurve::Cast(p_o))
	    {
	      if((ON_PolylineCurve::Cast(p_o))->GetNurbForm(c,
							    accuracy, NULL))
		{
		  ay_status = onio_readnurbscurve(&c);
		}
	      else
		{
		  return AY_ERROR;
		}
	    }
	  if(ON_PolyCurve::Cast(p_o))
	    {
	      if((ON_PolyCurve::Cast(p_o))->GetNurbForm(c, accuracy, NULL))
		{
		  ay_status = onio_readnurbscurve(&c);
		}
	      else
		{
		  return AY_ERROR;
		}
	    }
	  if(ON_LineCurve::Cast(p_o))
	    {
	      if((ON_LineCurve::Cast(p_o))->GetNurbForm(c, accuracy, NULL))
		{
		  ay_status = onio_readnurbscurve(&c);
		}
	      else
		{
		  return AY_ERROR;
		}
	    }
	  if(ON_ArcCurve::Cast(p_o))
	    {
	      if((ON_ArcCurve::Cast(p_o))->GetNurbForm(c, accuracy, NULL))
		{
		  ay_status = onio_readnurbscurve(&c);
		}
	      else
		{
		  return AY_ERROR;
		}
	    }
	  if(ON_CurveOnSurface::Cast(p_o))
	    {
	      if((ON_CurveOnSurface::Cast(p_o))->GetNurbForm(c,
							     accuracy, NULL))
		{
		  ay_status = onio_readnurbscurve(&c);
		}
	      else
		{
		  return AY_ERROR;
		}
	    } // if
	} // if
      break;
    case ON::surface_object:
      if(ON_NurbsSurface::Cast(p_o))
	ay_status = onio_readnurbssurface((ON_NurbsSurface*)p_o);
      if(ON_RevSurface::Cast(p_o))
	{
	  if((ON_RevSurface::Cast(p_o))->GetNurbForm(s, accuracy))
	    {
	      ay_status = onio_readnurbssurface(&s);
	    }
	  else
	    {
	      return AY_ERROR;
	    }
	}
      if(ON_SumSurface::Cast(p_o))
	{
	  if((ON_SumSurface::Cast(p_o))->GetNurbForm(s, accuracy))
	    {
	      ay_status = onio_readnurbssurface(&s);
	    }
	  else
	    {
	      return AY_ERROR;
	    }
	}
      if(ON_PlaneSurface::Cast(p_o))
	{
	  if((ON_PlaneSurface::Cast(p_o))->GetNurbForm(s, accuracy))
	    {
	      ay_status = onio_readnurbssurface(&s);
	    }
	  else
	    {
	      return AY_ERROR;
	    }
	}
      /*
      if(ON_BezierSurface::Cast(p_o))
	{
	  if((ON_BezierSurface::Cast(p_o)->GetNurbForm(s, accuracy))
            {
	      ay_status = onio_readnurbssurface(&s);
            {
	  else
	    {
	      return AY_ERROR;
	    }
	}
      */
      break;
    case ON::brep_object:
      ay_status = onio_readbrep((ON_Brep *)p_o, accuracy);
      break;
    case ON::instance_reference:
      ay_status = onio_readreference(p_m, (ON_InstanceRef *)p_o, accuracy);
    default:
      break;
    } // switch

 return ay_status;
} // onio_readobject


// onio_readlayer:
//
int
onio_readlayer(ONX_Model &model, int li, double accuracy)
{
 int ay_status = AY_OK;
 int i;
 char fname[] = "onio_readlayer";
 ON_Layer *layer;
 ON_3dmObjectAttributes *attr;
 ay_object *newo = NULL;
 ay_level_object *newlevel = NULL;

  if((li < 0) || (li > model.m_layer_table.Capacity()))
    {
      ay_error(AY_ERROR, fname, "layer index invalid");
      // XXXX inform user about real model.m_layer_table.Capacity()
      return AY_ERROR;
    }

  layer = &(model.m_layer_table[li]);
  if(!layer)
    return AY_ENULL;

  // create level object, named as the layer
  if(!(newo = (ay_object*)calloc(1, sizeof(ay_object))))
    return AY_EOMEM;
  if(!(newlevel = (ay_level_object*)calloc(1, sizeof(ay_level_object))))
    return AY_EOMEM;

  ay_object_defaults(newo);
  newo->type = AY_IDLEVEL;
  newo->inherit_trafos = AY_TRUE;
  newo->parent = AY_TRUE;
  newo->refine = newlevel;
  ay_status = ay_object_crtendlevel(&(newo->down));

  ay_status = ay_object_link(newo);

  ay_next = &(newo->down);

  // read layer name
  if(layer->LayerName() && (layer->LayerName().Length() > 0))
    {
      int length = layer->LayerName().Length();
      int clength = onio_w2c_size(length, layer->LayerName());
      if((newo->name = (char*)calloc(clength+1, sizeof(char))))
	{
	  onio_w2c(length, layer->LayerName(), clength, newo->name);
	} // if

      // repair name (convert " " to "_")
      char *c = newo->name;
      while(c && *c != '\0')
	{
	  if(*c == ' ')
	    *c = '_';
	  c++;
	} // while
    } // if

  // read objects from layer
  for(i = 0; i < model.m_object_table.Capacity(); ++i)
    {
      if((model.m_object_table[i]).m_object)
	{
	  attr = &((model.m_object_table[i]).m_attributes);
	  if(attr->m_layer_index == li)
	    {
	      ay_status = onio_readobject(&model,
					  (model.m_object_table[i]).m_object,
					  accuracy);
	      if(ay_status)
		{
		  ay_error(ay_status, fname, NULL);
		  ay_error(AY_ERROR, fname,
		       "Failed to read/convert object; continuing with next!");
		} // if

	      // read object name
	      ay_status = onio_readname(onio_lrobject,
			           &((model.m_object_table[i]).m_attributes));
	    } // if
	} // if
    } // for

  ay_next = &(newo->next);

 return ay_status;
} // onio_readlayer


// onio_readname:
//
int
onio_readname(ay_object *o, ON_3dmObjectAttributes *attr)
{
 int ay_status = AY_OK;

  if(!o || !attr)
    return AY_OK;

  if(o->name)
    free(o->name);
  o->name = NULL;

  if(attr->m_name && (attr->m_name.Length() > 0))
    {
      int length = attr->m_name.Length();
      int clength = onio_w2c_size(length, attr->m_name);
      if((o->name = (char*)calloc(clength+1, sizeof(char))))
	{
	  onio_w2c(length, attr->m_name, clength, o->name);
	} // if
      //else
      // XXXX should return AY_EOMEM

      // repair name (convert " " to "_")
      char *c = o->name;
      while(c && *c != '\0')
	{
	  if(*c == ' ')
	    *c = '_';
	  c++;
	} // while
    } // if

 return ay_status;
} // onio_readname


// onio_readtcmd:
//
int
onio_readtcmd(ClientData clientData, Tcl_Interp *interp,
	      int argc, char *argv[])
{
 int ay_status = AY_OK;
 char fname[] = "onio_read";
 ONX_Model model;
 char *minus;
 int i = 2, sframe = -1, eframe = -1;
 double accuracy = 0.1;

  onio_importcurves = AY_TRUE;

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
	  sscanf(argv[i+1], "%d", &onio_importcurves);
	}
      else
      if(!strcmp(argv[i], "-i"))
	{
	  sscanf(argv[i+1], "%d", &onio_ignorefirsttrim);
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
		     (minus = strchr((const char*)(&(argv[i+1][1])), '-')))
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

  onio_lrobject = NULL;
  if(sframe == -1)
    {
      for(i = 0; i < model.m_object_table.Capacity(); ++i)
	{
	  if((model.m_object_table[i]).m_object)
	    {
	      ay_status = onio_readobject(&model,
					  (model.m_object_table[i]).m_object,
					  accuracy);
	      if(ay_status)
		{
		  ay_error(ay_status, fname, NULL);
		  ay_error(AY_ERROR, fname,
		      "Failed to read/convert object; continuing with next!");
		} // if

	      // read object name
	      ay_status = onio_readname(onio_lrobject,
			           &((model.m_object_table[i]).m_attributes));

	    } // if
	} // for
    }
  else
    {
      if(eframe != -1)
	{
	  for(i = sframe; i <= eframe; i++)
	    {
	      ay_status = onio_readlayer(model, i, accuracy);
	    } // for
	}
      else
	{
	  ay_status = onio_readlayer(model, sframe, accuracy);
	} // if
    } // if


  // destroy this model
  model.Destroy();

 return TCL_OK;
} // onio_readtcmd


extern "C" {

// Onio_Init:
//  initialize onio module
//  note: this function _must_ be capitalized exactly this way
//  regardless of the filename of the shared object (see: man n load)!
int
Onio_Init(Tcl_Interp *interp)
{
 int ay_status = AY_OK;
 char fname[] = "Onio_Init";
 // int err;
 // int ay_status = AY_OK;

#ifndef AYONIOWRAPPED
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
#endif // !AYONIOWRAPPED

#ifndef AYONIOWRAPPED
  // source onio.tcl, it contains vital Tcl-code
  if((Tcl_EvalFile(interp, "onio.tcl")) != TCL_OK)
     {
       ay_error(AY_ERROR, fname,
		  "Error while sourcing \\\"onio.tcl\\\"!");
       return TCL_OK;
     }
#endif // !AYONIOWRAPPED

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

  ay_status = onio_registerwritecb((char *)(AY_IDREVOLVE),
				   onio_writenpconvertible);

  ay_status = onio_registerwritecb((char *)(AY_IDSWEEP),
				   onio_writenpconvertible);

  ay_status = onio_registerwritecb((char *)(AY_IDSKIN),
				   onio_writenpconvertible);

  ay_status = onio_registerwritecb((char *)(AY_IDGORDON),
				   onio_writenpconvertible);

  ay_status = onio_registerwritecb((char *)(AY_IDBIRAIL1),
				   onio_writenpconvertible);

  ay_status = onio_registerwritecb((char *)(AY_IDBIRAIL2),
				   onio_writenpconvertible);

  ay_status = onio_registerwritecb((char *)(AY_IDCAP),
				   onio_writenpconvertible);

  ay_status = onio_registerwritecb((char *)(AY_IDBPATCH),
				   onio_writenpconvertible);

  ay_status = onio_registerwritecb((char *)(AY_IDPAMESH),
				   onio_writenpconvertible);

  ay_status = onio_registerwritecb((char *)(AY_IDTEXT),
				   onio_writenpconvertible);


  ay_status = onio_registerwritecb((char *)(AY_IDNCURVE),
				   onio_writencurve);

  ay_status = onio_registerwritecb((char *)(AY_IDICURVE),
				   onio_writencconvertible);

  ay_status = onio_registerwritecb((char *)(AY_IDCONCATNC),
				   onio_writencconvertible);

  ay_status = onio_registerwritecb((char *)(AY_IDEXTRNC),
				   onio_writencconvertible);

  ay_status = onio_registerwritecb((char *)(AY_IDLEVEL),
				   onio_writelevel);

  ay_status = onio_registerwritecb((char *)(AY_IDCLONE),
				   onio_writeclone);

  ay_status = onio_registerwritecb((char *)(AY_IDINSTANCE),
				   onio_writeinstance);

  ay_status = onio_registerwritecb((char *)(AY_IDSCRIPT),
				   onio_writescript);

  ay_status = onio_registerwritecb((char *)(AY_IDSPHERE),
				   onio_writesphere);

  ay_status = onio_registerwritecb((char *)(AY_IDCYLINDER),
				   onio_writecylinder);

  ay_status = onio_registerwritecb((char *)(AY_IDCONE),
				   onio_writecone);

  ay_status = onio_registerwritecb((char *)(AY_IDTORUS),
				   onio_writetorus);

  ay_status = onio_registerwritecb((char *)(AY_IDBOX),
				   onio_writebox);

  ay_status = onio_registerwritecb((char *)(AY_IDDISK),
				   onio_writenpconvertible);

  ay_status = onio_registerwritecb((char *)(AY_IDHYPERBOLOID),
				   onio_writenpconvertible);

  ay_status = onio_registerwritecb((char *)(AY_IDPARABOLOID),
				   onio_writenpconvertible);


#ifndef AYONIOWRAPPED
  ay_error(AY_EOUTPUT, fname, "Plugin 'onio' successfully loaded.");
#endif // !AYONIOWRAPPED

 return TCL_OK;
} // Aycsg_Init | onio_inittcmd

} // extern "C"
