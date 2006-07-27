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

#include <stdarg.h>
#include <iostream>

#include "ayam.h"
#include <opennurbs.h>
#include <opennurbs_extensions.h>


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
int onio_exptoplevellayers = AY_TRUE;

int onio_currentlayer = 0;

double onio_rescaleknots = 0.0;

double onio_accuracy = 1.0e-12;

double onio_scalefactor = 1.0;

char onio_stagnamedef[] = "mys";
char *onio_stagname = onio_stagnamedef;
char onio_ttagnamedef[] = "myt";
char *onio_ttagname = onio_ttagnamedef;


// prototypes of functions local to this module

unsigned int onio_count(ay_object *o);

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

int onio_writepomesh(ay_object *o, ONX_Model *p_m, double *m);

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


// count objects to be exported
//
//
unsigned int
onio_count(ay_object *o)
{
 unsigned int lcount = 0;
 int lasttype = -1;
 Tcl_HashTable *ht = &onio_write_ht;
 Tcl_HashEntry *entry = NULL;
 onio_writecb *cb = NULL;

  if(!o)
    return 0;

  while(o->next)
    {
      if(lasttype != (int)o->type)
	{
	  entry = NULL;
	  if((entry = Tcl_FindHashEntry(ht, (char *)(o->type))))
	    {
	      cb = (onio_writecb*)Tcl_GetHashValue(entry);
	    }
	  else
	    {
	      cb = NULL;
	    }
	  lasttype = o->type;
	} // if

      if(o->down && o->down->next && (cb != onio_writenpconvertible))
	lcount += onio_count(o->down);

      if(cb != NULL)
	lcount++;

      o = o->next;
    } // while

 return lcount;
} // onio_count


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
 double *cv, umin, umax, vmin, vmax;
 ay_nurbpatch_object *np = NULL;
 ON_NurbsSurface *p_n = NULL;

  if(!o || !pp_n || !m)
    return AY_ENULL;

  np = (ay_nurbpatch_object *)o->refine;
  p_n = new ON_NurbsSurface(3, np->is_rat?true:false, np->uorder, np->vorder,
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
	  if(np->is_rat)
	    {
	      p_n->SetCV(i, j, ON::homogeneous_rational, &(np->controlv[a]));
	      ay_trafo_apply4(cv, m);
	      cv += 4;
	    }
	  else
	    {
	      p_n->SetCV(i, j, ON::not_rational, &(np->controlv[a]));
	      ay_trafo_apply3(cv, m);
	      cv += 3;
	    }
	  a += stride;
	} // for
    } // for

  ay_knots_getuminmax(o, np->uorder, np->width+np->uorder, np->uknotv,
		      &umin, &umax);
  if((umin > np->uknotv[np->uorder]) || (umax < np->uknotv[np->width]))
    {
      ON_Interval interval = ON_Interval(umin, umax);
      p_n->Trim(0, interval);
    }

  ay_knots_getvminmax(o, np->vorder, np->height+np->vorder, np->vknotv,
		      &vmin, &vmax);
  if((vmin > np->vknotv[np->vorder]) || (vmax < np->vknotv[np->height]))
    {
      ON_Interval interval = ON_Interval(vmin, vmax);
      p_n->Trim(1, interval);
    }

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

      mo.m_attributes.m_layer_index = onio_currentlayer;

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
 double cv[4], m[16], umin, umax;
 int i, a, stride = 4;
 ON_NurbsCurve *p_c = NULL;

  if(!o)
    return AY_ENULL;

  nc = (ay_nurbcurve_object *)o->refine;
  p_c = new ON_NurbsCurve(2, nc->is_rat?true:false, nc->order, nc->length);

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
      memcpy(cv, &(nc->controlv[a]), (nc->is_rat?4:3)*sizeof(double));
      ay_trafo_apply3(cv, m);
      if(nc->is_rat)
	{
	  cv[2] = cv[3];
	  p_c->SetCV(i, ON::homogeneous_rational, cv);
	}
      else
	{
	  p_c->SetCV(i, ON::not_rational, cv);
	}
      a += stride;
    } // for

  ay_knots_getuminmax(o, nc->order, nc->length+nc->order, nc->knotv,
		      &umin, &umax);

  if((umin > nc->knotv[nc->order]) || (umax < nc->knotv[nc->length]))
    {
      ON_Interval interval = ON_Interval(umin, umax);
      p_c->Trim(interval);
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
 ON_NurbsCurve nc1, nc2, *p_nc = NULL;
 ON_Curve *p_curve = NULL;
 unsigned int c2i, c3i;
 double tolerance = onio_accuracy;
 static int addtoloop = AY_FALSE, firstinloop = AY_FALSE,
   lastinloop = AY_FALSE, firstloopvertex, prevloopvertex;
 static ON_BrepLoop *p_loop = NULL;

  if(!o || !p_b || !p_f)
    return AY_ENULL;

  if(o->type == AY_IDLEVEL)
    {
      // add a trim loop consisting of multiple consecutive trim curves
      o = o->down;
      addtoloop = AY_TRUE;
      firstinloop = AY_TRUE;

      ON_BrepLoop& loop = p_b->NewLoop(ltype);
      p_loop = &loop;
      loop.m_fi = p_f->m_face_index;
      if(ON_BrepLoop::outer == ltype)
	{
	  // the index of the outer loop is always
	  // in face.m_li[0]
	  p_f->m_li.Insert(0, loop.m_loop_index);
	}
      else
	{
	  p_f->m_li.Append(loop.m_loop_index);
	}

      while(o->next)
	{
	  if(o->next && !o->next->next)
	    lastinloop = AY_TRUE;

	  ay_status = onio_addtrim(o, ltype, ttype, p_b, p_f);

	  firstinloop = AY_FALSE;
	  lastinloop = AY_FALSE;

	  o = o->next;
	} // while

      addtoloop = AY_FALSE;
      return ay_status;
    } // if

  if(o->type == AY_IDNCURVE)
    {
      onio_get2dcurveobj(o, &p_nc);
      if(p_nc)
	{
	  c2i = p_b->m_C2.Count();
	  p_b->m_C2.Append(p_nc);
	  c3i = p_b->m_C3.Count();
	  nc1 = *p_nc;

	  const ON_Surface *p_s = p_f->SurfaceOf();
	  p_curve = p_s->Pushup(nc1, tolerance);
	  if(p_curve)
	    {
	      // pushup succeeded, add trim to BRep
	      p_curve->GetNurbForm(nc2, tolerance, NULL);
	      p_nc = new ON_NurbsCurve(nc2);
	      p_b->m_C3.Append(p_nc);

	      delete p_curve;

	      if(!addtoloop)
		{
		  // no need to create a second vertex for a closed curve...
		  ON_BrepVertex& v1 = p_b->NewVertex(p_nc->PointAtStart());
		  v1.m_tolerance = 0.0;

		  ON_BrepEdge& edge = p_b->NewEdge(v1, v1, c3i);
		  edge.m_tolerance = tolerance;

		  ON_BrepLoop& loop = p_b->NewLoop(ltype);
		  loop.m_fi = p_f->m_face_index;
		  if(ON_BrepLoop::outer == ltype)
		    {
		      // the index of the outer loop is always
		      // in face.m_li[0]
		      p_f->m_li.Insert(0, loop.m_loop_index);
		    }
		  else
		    {
		      p_f->m_li.Append(loop.m_loop_index);
		    }

		  ON_BrepTrim& trim = p_b->NewTrim(edge, false, loop, c2i);
		  trim.m_type = ttype;
		  trim.m_tolerance[0] = 0.0;
		  trim.m_tolerance[1] = 0.0;
		}
	      else
		{
		  ON_BrepVertex *p_v1 = NULL, *p_v2 = NULL;
		  if(!firstinloop)
		    {
		      // for all curves in the loop except for the first,
		      // the first vertex is the last vertex of the
		      // previous curve in the loop
		      p_v1 = &(p_b->m_V[prevloopvertex]);
		    }
		  else
		    {
		      firstloopvertex = p_b->m_V.Count();
		      p_v1 = &(p_b->NewVertex(p_nc->PointAtStart()));
		      p_v1->m_tolerance = 0.0;
		    }
		  if(!lastinloop)
		    {
		      prevloopvertex = p_b->m_V.Count();
		      p_v2 = &(p_b->NewVertex(p_nc->PointAtEnd()));
		      p_v2->m_tolerance = 0.0;
		    }
		  else
		    {
		      p_v2 = &(p_b->m_V[firstloopvertex]);
		    }

		  ON_BrepEdge& edge = p_b->NewEdge(*p_v1, *p_v2, c3i);
		  edge.m_tolerance = 0.0;

		  ON_BrepTrim& trim = p_b->NewTrim(edge, false, *p_loop, c2i);
		  trim.m_type = ttype;
		  trim.m_tolerance[0] = 0.0;
		  trim.m_tolerance[1] = 0.0;
		} // if
	    }
	  else
	    {
	      //ay_error(AY_EWARN, fname, "pushup failed, creating polygonal representation of trimcurve...");

	      // pushup failed, create a polygonal 3D representation
	      // of the trim curve
	      bool done = false;
	      int i, j, evalhint = 0, evalhint2[2] = {0, 0}, qf = 2;
	      double u, ud, *st = NULL, *stt = NULL, cv[3];

	      ON_3dPointArray *p_p3darr = new ON_3dPointArray();

	      while((!done) || (qf == 100))
		{
		  // set up array of sample points
		  ud = (double)(p_nc->m_knot[p_nc->m_cv_count-1] -
			 p_nc->m_knot[p_nc->m_order-2]) /
		       (double)((4+p_nc->m_cv_count)*qf);
		  if(st)
		    free(st);
		  st = NULL;
		  stt = NULL;
		  i = 0;
		  for(u = p_nc->m_knot[p_nc->m_order-2];
		      u <= p_nc->m_knot[p_nc->m_cv_count-1];
		      u += ud)
		    {
		      i++;
		      stt = NULL;
		      if(!(stt = (double*)realloc(st, i*2*sizeof(double))))
			{
			  if(st)
			    free(st);
			  return AY_EOMEM;
			}
		      st = stt;

		      p_nc->Evaluate(u, 0, 2, &(st[(i-1)*2]), 0, &evalhint);
		    } // for

		  // evaluate surface at sample points
		  ON_NurbsSurface nsurf;
		  p_s->GetNurbForm(nsurf, tolerance);

		  p_p3darr->SetCapacity(i);

		  for(j = 0; j < i; j++)
		    {
		      nsurf.Evaluate(st[j*2], st[j*2+1], 0, 3, cv, 0,
				     evalhint2);
		      ON_3dPoint pnt(cv);

		      p_p3darr->Insert(j, pnt);
		    }

		  if(!addtoloop)
		    {
		      // check closedness of sample points array
		      if(((*p_p3darr)[0]).DistanceTo((*p_p3darr)[j]) <
			 tolerance)
			{
			  done = true;
			}
		      else
			{
			  done = false;
			  qf++;
			  //p_p3darr->Destroy();
			} // if
		    }
		  else
		    {
		      // check how we attach to last trim
		      if(qf<10)
			{
			  qf = 10;
			  done = false;
			}
		      else
			{
			  done = true;
			}
		    } // if
		} // while

	      ON_3dPointArray p3darr = *p_p3darr;
	      ON_Polyline *p_pl = new ON_Polyline(p3darr);
	      //delete p_p3darr;

	      // get rid of double points
	      p_pl->Clean(AY_EPSILON);

	      // create a polylinecurve
	      ON_PolylineCurve *p_plc = new ON_PolylineCurve();
	      p_plc->m_dim = 3;
	      p_plc->m_t.Reserve(p_pl->PointCount());
	      p_plc->m_t.SetCount(p_pl->PointCount());
	      // set parametric values of polylinecurve
	      for(j = 0; j < p_pl->PointCount(); j++)
		{
		  p_plc->m_t[j] = (double)j;
		}
	      p_plc->m_pline = *p_pl;

	      // add polylinecurve to brep
	      p_b->m_C3.Append(p_plc);

	      if(!addtoloop)
		{
		  // no need to create a second vertex for a closed curve...
		  ON_BrepVertex& v1 = p_b->NewVertex(p_plc->PointAtStart());
		  v1.m_tolerance = 0.0;

		  ON_BrepEdge& edge = p_b->NewEdge(v1, v1, c3i);
		  edge.m_tolerance = 0.0;

		  ON_BrepLoop& loop = p_b->NewLoop(ltype);
		  loop.m_fi = p_f->m_face_index;
		  if(ON_BrepLoop::outer == ltype)
		    {
		      // the index of the outer loop is always
		      // in face.m_li[0]
		      p_f->m_li.Insert(0, loop.m_loop_index);
		    }
		  else
		    {
		      p_f->m_li.Append(loop.m_loop_index);
		    }

		  ON_BrepTrim& trim = p_b->NewTrim(edge, false, loop, c2i);
		  trim.m_type = ttype;
		  trim.m_tolerance[0] = 0.0;
		  trim.m_tolerance[1] = 0.0;
		}
	      else
		{
		  ON_BrepVertex *p_v1 = NULL, *p_v2 = NULL;
		  if(!firstinloop)
		    {
		      // for all curves in the loop except for the first,
		      // the first vertex is the last vertex of the
		      // previous curve in the loop
		      p_v1 = &(p_b->m_V[prevloopvertex]);
		    }
		  else
		    {
		      firstloopvertex = p_b->m_V.Count();
		      p_v1 = &(p_b->NewVertex(p_plc->PointAtStart()));
		      p_v1->m_tolerance = 0.0;
		    }
		  if(!lastinloop)
		    {
		      prevloopvertex = p_b->m_V.Count();
		      p_v2 = &(p_b->NewVertex(p_plc->PointAtEnd()));
		      p_v2->m_tolerance = 0.0;
		    }
		  else
		    {
		      p_v2 = &(p_b->m_V[firstloopvertex]);
		    }
		  ON_BrepEdge& edge = p_b->NewEdge(*p_v1, *p_v2, c3i);
		  edge.m_tolerance = 0.0;

		  ON_BrepTrim& trim = p_b->NewTrim(edge, false, *p_loop, c2i);
		  trim.m_type = ttype;
		  trim.m_tolerance[0] = 0.0;
		  trim.m_tolerance[1] = 0.0;
		} // if
	    } // if
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

  if(!(o->type == AY_IDNCURVE))
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
    } // if

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

  mo.m_attributes.m_layer_index = onio_currentlayer;

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
 double *cv, umin, umax;
 ay_nurbcurve_object *nc = NULL;
 ON_NurbsCurve *p_c = NULL;

  if(!o || !p_m || !m)
    return AY_ENULL;

  if(!onio_exportcurves)
    return AY_OK;

  nc = (ay_nurbcurve_object *)o->refine;
  p_c = new ON_NurbsCurve(3, nc->is_rat?true:false, nc->order, nc->length);

  // copy knots, ignoring "superfluous"/"phantom" end knots
  for(i = 0; i < nc->order+nc->length-2; i++)
    p_c->SetKnot(i, nc->knotv[i+1]);

  // copy control points
  a = 0;
  cv = p_c->m_cv;
  for(i = 0; i < nc->length; i++)
    {
      if(nc->is_rat)
	{
	  p_c->SetCV(i, ON::homogeneous_rational, &(nc->controlv[a]));
	  ay_trafo_apply4(cv, m);
	  cv += 4;
	}
      else
	{
	  p_c->SetCV(i, ON::not_rational, &(nc->controlv[a]));
	  ay_trafo_apply3(cv, m);
	  cv += 3;
	}
      a += stride;
    } // for

  ay_knots_getuminmax(o, nc->order, nc->length+nc->order, nc->knotv,
		      &umin, &umax);

  if((umin > nc->knotv[nc->order]) || (umax < nc->knotv[nc->length]))
    {
      ON_Interval interval = ON_Interval(umin, umax);
      p_c->Trim(interval);
    }

  ONX_Model_Object& mo = p_m->m_object_table.AppendNew();
  mo.m_object = p_c;
  mo.m_bDeleteObject = true;

  mo.m_attributes.m_layer_index = onio_currentlayer;

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


// onio_writepomesh:
//
int
onio_writepomesh(ay_object *o, ONX_Model *p_m, double *m)
{
 int ay_status = AY_OK;
 ay_object *to = NULL;
 ay_list_object *li = NULL, **nextli = NULL, *lihead = NULL;
 ay_pomesh_object *pm;
 int stride = 3, have_mys = AY_FALSE, have_myt = AY_FALSE;
 unsigned int myslen = 0, mytlen = 0;
 double *mysarr = NULL, *mytarr = NULL;
 ay_tag_object mystag = {NULL, ay_pv_tagtype, onio_stagname};
 ay_tag_object myttag = {NULL, ay_pv_tagtype, onio_ttagname};
 ay_tag_object *tag;
 ON_Mesh *p_mesh = NULL;
 BOOL has_texcoords = false;
 BOOL has_vnormals = false;
 unsigned int a, f = 0, i, j, k, p = 0, q = 0, r = 0;

  if(!o || !p_m || !m)
    return AY_ENULL;

  pm = (ay_pomesh_object *)(o->refine);

  if(pm->has_normals)
    has_vnormals = TRUE;

  if(o->tags)
    {
      if(!(mystag.val = (char*)calloc(strlen(onio_stagname)+2,sizeof(char))))
	return AY_EOMEM;
      if(!(myttag.val = (char*)calloc(strlen(onio_ttagname)+2,sizeof(char))))
	return AY_EOMEM;
      strcpy(mystag.val, onio_stagname);
      mystag.val[strlen(onio_stagname)] = ',';
      strcpy(myttag.val, onio_ttagname);
      myttag.val[strlen(onio_ttagname)] = ',';
      tag = o->tags;
      while(tag)
	{
	  if((tag->type == ay_pv_tagtype) && ay_pv_cmpname(tag, &mystag))
	    {
	      have_mys = AY_TRUE;

	      ay_status = ay_pv_convert(tag, &myslen, (void**)&mysarr);
	    }
	  if((tag->type == ay_pv_tagtype) && ay_pv_cmpname(tag, &myttag))
	    {
	      have_myt = AY_TRUE;

	      ay_status = ay_pv_convert(tag, &mytlen, (void**)&mytarr);
	    }
	  tag = tag->next;
	} // while
      free(mystag.val);
      free(myttag.val);
    } // if

  p_mesh = new ON_Mesh(pm->npolys, pm->ncontrols, has_vnormals, has_texcoords);


  // set vertex coordinates and normals
  a = 0;
  if(pm->has_normals)
    stride += 3;
  for(i = 0; i < pm->ncontrols; i++)
    {
      ON_3dPoint pt;
      pt.x = pm->controlv[a];
      pt.y = pm->controlv[a+1];
      pt.z = pm->controlv[a+2];
      p_mesh->SetVertex(i, pt);
      if(pm->has_normals)
	{
	  ON_3dVector ve;
	  ve.x = pm->controlv[a];
	  ve.y = pm->controlv[a+1];
	  ve.z = pm->controlv[a+2];
	  p_mesh->SetVertexNormal(i, ve);
	} // if
      a += stride;
    } // for

  // set faces
  for(i = 0; i < pm->npolys; i++)
    {
      if(pm->nloops[i] == 1)
	{
	  // this face has just one loop (no hole)
	  // XXXX this "for" unneeded?
	  for(j = 0; j < pm->nloops[p]; j++)
	    {
	      if(pm->nverts[q] == 3)
		{
		  // this is a triangle
		  p_mesh->SetTriangle(f, pm->verts[r], pm->verts[r+1],
				      pm->verts[r+2]);
		  f++;
		  r += 3;
		} // if
	      if(pm->nverts[q] == 4)
		{
		  // this is a quad
		  p_mesh->SetQuad(f, pm->verts[r], pm->verts[r+1],
				  pm->verts[r+2], pm->verts[r+3]);
		  f++;
		  r += 4;
		} // if
	      if(pm->nverts[q] > 4)
		{
		  // this is not a triangle or quad => tesselate it
		  // create new object (for the tesselated face)
		  li = NULL;
		  if(!(li = (ay_list_object*)
		       calloc(1, sizeof(ay_list_object))))
		    return AY_EOMEM;
		  to = NULL;
		  if(!(to = (ay_object*)calloc(1, sizeof(ay_object))))
		    return AY_EOMEM;
		  li->object = to;

		  ay_object_defaults(to);

		  to->type = AY_IDPOMESH;

		  ay_status = ay_tess_pomeshf(pm, i, q, r, AY_FALSE,
					  (ay_pomesh_object **)&(to->refine));

		  // temporarily save the tesselated face
		  if(nextli)
		    {
		      *nextli = li;
		    }
		  else
		    {
		      lihead = li;
		    }
		  nextli = &(li->next);

		  // advance index r
		  for(k = 0; k < pm->nverts[q]; k++)
		    {
		      r++;
		    }
		}
	      q++;
	    } // for
	}
      else
	{
	  // this face has more than one loop (hole(s)) => tesselate it

	  // create new object (for the tesselated face)
	  li = NULL;
	  if(!(li = (ay_list_object*)calloc(1, sizeof(ay_list_object))))
	    return AY_EOMEM;
	  to = NULL;
	  if(!(to = (ay_object*)calloc(1, sizeof(ay_object))))
	    return AY_EOMEM;
	  li->object = to;

	  ay_object_defaults(to);

	  to->type = AY_IDPOMESH;

	  ay_status = ay_tess_pomeshf(pm, i, q, r, AY_FALSE,
				      (ay_pomesh_object **)&(to->refine));

	  // temporarily save the tesselated face
	  if(nextli)
	    {
	      *nextli = li;
	    }
	  else
	    {
	      lihead = li;
	    }
	  nextli = &(li->next);

	  // advance indices r and q
	  for(j = 0; j < pm->nloops[p]; j++)
	    {
	      for(k = 0; k < pm->nverts[q]; k++)
		{
		  r++;
		}
	      q++;
	    } // for
	} // if
      p++;
    } // for


  // append p_mesh to object table
  ONX_Model_Object& mo = p_m->m_object_table.AppendNew();
  mo.m_object = p_mesh;
  mo.m_bDeleteObject = true;

  mo.m_attributes.m_layer_index = onio_currentlayer;

  onio_writename(o, mo);

  // write tesselated face(s)
  if(lihead && lihead->next)
    {
      to = NULL;
      ay_status = ay_pomesht_merge(AY_FALSE, lihead, &to);
      if(to)
	{
	  ay_status = ay_pomesht_optimizecoords(
				     (ay_pomesh_object*)to->refine, AY_FALSE);
	  ay_object_defaults(to);
	  to->type = AY_IDPOMESH;
	  // ay_trafo_copy(o, to);
	  onio_writepomesh(lihead->object, p_m, m);
	  ay_object_delete(to);
	}
    }
  else
    {
      if(lihead)
	onio_writepomesh(lihead->object, p_m, m);
    } // if

  while(lihead)
    {
      ay_object_delete(lihead->object);
      li = lihead->next;
      free(lihead);
      lihead = li;
    } // while

 return ay_status;
} // onio_writepomesh


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

	      mo.m_attributes.m_layer_index = onio_currentlayer;

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

	      mo.m_attributes.m_layer_index = onio_currentlayer;

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

	      mo.m_attributes.m_layer_index = onio_currentlayer;

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

	      mo.m_attributes.m_layer_index = onio_currentlayer;

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

	      mo.m_attributes.m_layer_index = onio_currentlayer;

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

	      mo.m_attributes.m_layer_index = onio_currentlayer;

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

	      mo.m_attributes.m_layer_index = onio_currentlayer;

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

	      mo.m_attributes.m_layer_index = onio_currentlayer;

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

      mo.m_attributes.m_layer_index = onio_currentlayer;

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
 int t, i = 2, version = 3, li;
 ay_object *o;
 char aname[] = "onio_options", vname1[] = "Progress";
 ONX_Model model;
 const ON_Layer *p_layer = NULL;
 //ON_3dmObjectAttributes attribs;
 //ON_TextLog& error_log;

  // set default parameters
  onio_scalefactor = 1.0;

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
      else
      if(!strcmp(argv[i], "-l"))
	{
	  sscanf(argv[i+1], "%d", &onio_exptoplevellayers);
	}
      else
      if(!strcmp(argv[i], "-f"))
	{
	  sscanf(argv[i+1], "%lg", &onio_scalefactor);
	}
      else
      if(!strcmp(argv[i], "-t"))
	{
	  onio_stagname = argv[i+1];
	  onio_ttagname = argv[i+2];
	  i++;
	}
      i += 2;
    } // while

  // open the file for writing
  fp = ON::OpenFile(filename, "wb");

  if(!fp)
    {
      ay_error(AY_EOPENFILE, fname, argv[1]);

      onio_stagname = onio_stagnamedef;
      onio_ttagname = onio_ttagnamedef;

      return TCL_OK;
    }

  // some notes
  //model.m_properties.m_Notes.m_notes = sNotes;
  //model.m_properties.m_Notes.m_bVisible = (model.m_properties.m_Notes.m_notes.Length() > 0);

  // set revision history information
  model.m_properties.m_RevisionHistory.NewRevision();

  // set application information
  model.m_properties.m_Application.m_application_name = "Ayam";
  model.m_properties.m_Application.m_application_URL =
    "http://www.ayam3d.org/";
  model.m_properties.m_Application.m_application_details =
    "onio (OpenNURBS) plugin";

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
  // Each object in the object table (written below)
  // should be on a defined layer.  There should be
  // at least one layer with layer index 0 in every file.

  // layer table indices begin at 0
  ON_Layer default_layer;
  default_layer.SetLayerIndex(0);
  default_layer.SetLayerName("Default");
  p_layer = &default_layer;
  model.m_layer_table.Append(p_layer[0]);

  // create layers from top level objects?
  if(onio_exptoplevellayers)
    {
      li = 0;
      o = ay_root->next;
      while(o)
	{
	  if((o->type == AY_IDLEVEL) && o->next)
	    {
	      if(li >= 1)
		{
		  ON_Layer layer;
		  layer.SetLayerIndex(li);
		  if(o->name && strlen(o->name))
		    layer.SetLayerName(o->name);
		  else
		    layer.SetLayerName("Unnamed_Layer");
		  p_layer = new ON_Layer(layer);
		  model.m_layer_table.Append(p_layer[0]);
		}
	      li++;
	    } // if
	  o = o->next;
	} // while
    } // if

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
  li = 0;
  o = ay_root->next;
  while(o)
    {
      ay_trafo_identitymatrix(tm);

      if(onio_scalefactor != 1.0)
	ay_trafo_scalematrix(onio_scalefactor, onio_scalefactor,
			     onio_scalefactor, tm);

      onio_currentlayer = 0;

      if(onio_exptoplevellayers && (o->type == AY_IDLEVEL) && o->next)
	{
	  onio_currentlayer = li;
	  li++;
	}

      ay_status = onio_writeobject(o, &model);

      o = o->next;
    } // while

  // archive to write to
  ON_BinaryFile archive(ON::write3dm, fp);

  // set uuid's, indices, etc.
  model.Polish();

  // set progress
  Tcl_SetVar2(ay_interp, aname, vname1, "50",
	      TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  while(Tcl_DoOneEvent(TCL_DONT_WAIT)){};

  // write model to archive
  bool ok = model.Write(archive,
                        version,
                        __FILE__ " onio_writetcmd() " __DATE__,
                        NULL/*&error_log*/);
  if(!ok)
    {
      ay_error(AY_ERROR, fname, "Error writing file!");
    }

  // close the file
  ON::CloseFile(fp);

  // set progress
  Tcl_SetVar2(ay_interp, aname, vname1, "100",
	      TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  while(Tcl_DoOneEvent(TCL_DONT_WAIT)){};

  onio_stagname = onio_stagnamedef;
  onio_ttagname = onio_ttagnamedef;

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

  // rescale knots to safe distance?
  if(onio_rescaleknots != 0.0)
    {
      ay_knots_rescaletomindist(width+p_s->m_order[0], uknotv,
				onio_rescaleknots);

      ay_knots_rescaletomindist(height+p_s->m_order[1], vknotv,
				onio_rescaleknots);
    }

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

  // rescale knots to safe distance?
  if(onio_rescaleknots != 0.0)
    ay_knots_rescaletomindist(length+p_c->m_order, knotv,
			      onio_rescaleknots);

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
 ay_object *olo = NULL, *lo = NULL, *o, *lf;
 ay_level_object *level = NULL;

  if(p_b->m_F.Count() > 1)
    {
      if(!(level = (ay_level_object *)calloc(1, sizeof(ay_level_object))))
	{
	  return AY_EOMEM;
	}

      level->type = AY_LTLEVEL;

      if(!(olo = (ay_object *) calloc(1, sizeof(ay_object))))
	{
	  free(level); return AY_EOMEM;
	}
      ay_object_defaults(olo);
      olo->type = AY_IDLEVEL;
      olo->refine = level;
      olo->parent = AY_TRUE;
      olo->inherit_trafos = AY_TRUE;
     
      ay_status = ay_object_crtendlevel(&(olo->down));
     
      ay_status = ay_object_link(olo);

      ay_next = &(olo->down);
    } // if

  for(i = 0; i < p_b->m_F.Count(); ++i)
    {
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
	      ay_object_defaults(lo);
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

      if(onio_lrobject && onio_lrobject->down)
	{
	  o = onio_lrobject->down;

	  while(o->next)
	    o = o->next;
	} // if

      ay_next = &(lf->next);
      onio_lrobject = lf;
    } // for

  if(p_b->m_F.Count() > 1)
    {
      ay_next = &(olo->next);
      onio_lrobject = olo;
    }

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


// onio_readmesh:
//
int
onio_readmesh(ON_Mesh *p_m, double accuracy)
{
 int ay_status = AY_OK;
 char fname[] = "onio_readmesh";
 int j;
 unsigned int i, a, stride = 3, *nloops = NULL, *nverts = NULL, *verts = NULL;
 unsigned int tnverts = 0;
 double *controlv = NULL, *stexc = NULL, *ttexc = NULL;
 ay_pomesh_object *po = NULL;
 ay_object *newo = NULL;

  if(!p_m)
    return AY_ENULL;

  if(!(po = (ay_pomesh_object*)calloc(1, sizeof(ay_pomesh_object))))
    { ay_status = AY_EOMEM; goto cleanup; }

  po->ncontrols = p_m->m_V.Capacity();

  if(p_m->m_N.Capacity() != 0)
    {
      stride += 3;
      po->has_normals = AY_TRUE;
    }

  if(!(controlv = (double*)calloc(po->ncontrols*stride, sizeof(double))))
    { ay_status = AY_EOMEM; goto cleanup; }

  // copy control points (and normals)
  a = 0;
  for(i = 0; i < po->ncontrols; i++)
    {
      ON_3fPoint pt = p_m->m_V[(int)i];
      controlv[a]   = pt.x;
      controlv[a+1] = pt.y;
      controlv[a+2] = pt.z;
      if(stride > 3)
	{
	  ON_3fVector ve = p_m->m_N[(int)i];
	  controlv[a+3] = ve.x;
	  controlv[a+4] = ve.y;
	  controlv[a+5] = ve.z;
	} // if
      a += stride;
    } // for

  po->controlv = controlv;

  // copy texture coordinates (if present)
  if(p_m->m_T.Capacity() != 0)
    {
      if(!(stexc = (double*)calloc(p_m->m_T.Capacity(), sizeof(double))))
	{ ay_status = AY_EOMEM; goto cleanup; }
      if(!(ttexc = (double*)calloc(p_m->m_T.Capacity(), sizeof(double))))
	{ ay_status = AY_EOMEM; goto cleanup; }

      for(j = 0; j < p_m->m_T.Capacity(); j++)
	{
	  stexc[j] = p_m->m_T[j].x;
	  ttexc[j] = p_m->m_T[j].y;
	}
    } // if

  // copy faces
  po->npolys = p_m->m_F.Capacity();

  if(!(nloops = (unsigned int*)calloc(po->npolys, sizeof(unsigned int))))
    { ay_status = AY_EOMEM; goto cleanup; }
  po->nloops = nloops;
  for(i = 0; i < po->npolys; i++)
    nloops[i] = 1;

  if(!(nverts = (unsigned int*)calloc(po->npolys, sizeof(unsigned int))))
    { ay_status = AY_EOMEM; goto cleanup; }
  po->nverts = nverts;
  for(i = 0; i < po->npolys; i++)
    {
      if(p_m->m_F[(int)i].IsTriangle())
	{
	  nverts[(int)i] = 3;
	  tnverts += 3;
	}
      else
	{
	  nverts[(int)i] = 4;
	  tnverts += 4;
	} // if
    } // for

  if(!(verts = (unsigned int*)calloc(tnverts, sizeof(unsigned int))))
    { ay_status = AY_EOMEM; goto cleanup; }
  po->verts = verts;
  a = 0;
  for(i = 0; i < po->npolys; i++)
    {
      verts[a] = p_m->m_F[(int)i].vi[0];
      a++;
      verts[a] = p_m->m_F[(int)i].vi[1];
      a++;
      verts[a] = p_m->m_F[(int)i].vi[2];
      a++;
      if(p_m->m_F[(int)i].IsQuad())
	{
	  verts[a] = p_m->m_F[(int)i].vi[3];
	  a++;
	} // if
    } // for

  // create object
  if(!(newo = (ay_object*)calloc(1, sizeof(ay_object))))
    { ay_status = AY_EOMEM; goto cleanup; }

  ay_status = ay_object_defaults(newo);

  newo->type = AY_IDPOMESH;
  newo->refine = po;

  // link texture coordinates as PV tags
  if(stexc)
    {
      ay_status = ay_pv_add(newo, "mys", "varying", 0, p_m->m_T.Capacity(),
			    (void*)stexc);
      ay_status = ay_pv_add(newo, "mys", "varying", 0, p_m->m_T.Capacity(),
			    (void*)stexc);
    }

  // link the new PolyMesh into the scene hierarchy
  ay_status = ay_object_link(newo);

  if(ay_status)
    ay_status = ay_object_delete(newo);
  else
    onio_lrobject = newo;

  po = NULL;
  controlv = NULL;
  newo = NULL;
  nloops = NULL;
  nverts = NULL;
  verts = NULL;

cleanup:
  if(po)
    free(po);
  if(controlv)
    free(controlv);
  if(stexc)
    free(stexc);
  if(ttexc)
    free(ttexc);
  if(newo)
    free(newo);
  if(nloops)
    free(nloops);
  if(nverts)
    free(nverts);
  if(verts)
    free(verts);

 return ay_status;
} // onio_readmesh


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
    case ON::mesh_object:
      ay_status = onio_readmesh((ON_Mesh *)p_o, accuracy);
      break;
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
		}
	      else
		{
		  // read object name
		  ay_status = onio_readname(onio_lrobject,
			           &((model.m_object_table[i]).m_attributes));
		  // apply scale factor
		  if(onio_scalefactor != 1.0)
		    {
		      onio_lrobject->scalx *= onio_scalefactor;
		      onio_lrobject->scaly *= onio_scalefactor;
		      onio_lrobject->scalz *= onio_scalefactor;
		    }
		} // if
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
 char aname[] = "onio_options", vname1[] = "Progress";

  onio_importcurves = AY_TRUE;
  onio_rescaleknots = 0.0;
  onio_scalefactor = 1.0;

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
      if(!strcmp(argv[i], "-r"))
	{
	  sscanf(argv[i+1], "%lg", &onio_rescaleknots);
	}
      else
      if(!strcmp(argv[i], "-f"))
	{
	  sscanf(argv[i+1], "%lg", &onio_scalefactor);
	}
      else
      if(!strcmp(argv[i], "-t"))
	{
	  onio_stagname = argv[i+1];
	  onio_ttagname = argv[i+2];
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

  // set progress
  Tcl_SetVar2(ay_interp, aname, vname1, "50",
	      TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  while(Tcl_DoOneEvent(TCL_DONT_WAIT)){};

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
		}
	      else
		{
		  // read object name
		  ay_status = onio_readname(onio_lrobject,
			           &((model.m_object_table[i]).m_attributes));
		  // apply scale factor
		  if(onio_scalefactor != 1.0)
		    {
		      onio_lrobject->scalx *= onio_scalefactor;
		      onio_lrobject->scaly *= onio_scalefactor;
		      onio_lrobject->scalz *= onio_scalefactor;
		    } // if
		} // if
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

  // set progress
  Tcl_SetVar2(ay_interp, aname, vname1, "100",
	      TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  while(Tcl_DoOneEvent(TCL_DONT_WAIT)){};

  onio_stagname = onio_stagnamedef;
  onio_ttagname = onio_ttagnamedef;

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

  ay_status = onio_registerwritecb((char *)(AY_IDPOMESH),
				   onio_writepomesh);


#ifndef AYONIOWRAPPED
  ay_error(AY_EOUTPUT, fname, "Plugin 'onio' successfully loaded.");
#endif // !AYONIOWRAPPED

 return TCL_OK;
} // Aycsg_Init | onio_inittcmd

} // extern "C"
