/*
 * Ayam, a free 3D modeler for the RenderMan interface.
 *
 * Ayam is copyrighted 1998-2004 by Randolf Schultz
 * (rschultz@informatik.uni-rostock.de) and others.
 *
 * All rights reserved.
 *
 * See the file License for details.
 *
 */

// aycsg.cpp - Ayam CSG preview plugin

#include <GL/glew.h>
#include "ayam.h"

#include "opencsg.h"
#include "ayCSGPrimitive.h"

#ifdef AYCSGDBG
#include "aycore/ppoh.h"
#endif

// local preprocessor definitions
#define CSGTYPE modified

// global variables
std::vector<OpenCSG::Primitive*> primitives;

OpenCSG::Algorithm algo = OpenCSG::Automatic;

OpenCSG::DepthComplexityAlgorithm depthalgo =
 OpenCSG::NoDepthComplexitySampling;

ay_object *aycsg_root; // the root of the local copy of the object tree

// TM tags are used to store transformation attributes delegated from
// parents (CSG operation objects) to their children (e.g. primitives);
// note that the TM tags are used here in a quite unusual way:
// <tag->name> is used to denote coordinate system flips caused by an
// odd number of scale factors (it is not a pointer!) and
// <tag->val> points to an array of doubles (the transformation matrix
// itself) instead to a string;
// we can do this safely, because no TM tag will escape this module ever
char *aycsg_tm_tagtype;
typedef struct aycsg_taglistelem_s {
  struct aycsg_taglistelem_s *next;
  ay_tag_object *tag;
} aycsg_taglistelem;
// all TM tags created by aycsg are managed using this list
aycsg_taglistelem *aycsg_tmtags;

char aycsg_version_ma[] = AY_VERSIONSTR;
char aycsg_version_mi[] = AY_VERSIONSTRMI;

// prototypes of functions local to this module
int aycsg_rendertcb(struct Togl *togl, int argc, char *argv[]);

int aycsg_drawtoplevelprim(Togl *togl);

int aycsg_flatten(ay_object *t, struct Togl *togl, int parent_csgtype);

void aycsg_clearprimitives();

int aycsg_applyrule1(ay_object *t);
int aycsg_applyrule2(ay_object *t);
int aycsg_applyrule3(ay_object *t);
int aycsg_applyrule4(ay_object *t);
int aycsg_applyrule5(ay_object *t);
int aycsg_applyrule6(ay_object *t);
int aycsg_applyrule7(ay_object *t);
int aycsg_applyrule8(ay_object *t);

int aycsg_normalize(ay_object *t);

int aycsg_removetlu(ay_object *o, ay_object **t);

int aycsg_delegatetrafo(ay_object *o);

int aycsg_delegateall(ay_object *t);

int aycsg_binarify(ay_object *parent, ay_object *left, ay_object **target);

int aycsg_copytree(int sel_only, ay_object *t, int *is_csg,
		   ay_object **target);

void aycsg_cleartree(ay_object *t);

void aycsg_cleartmtags();

extern "C" {
int Aycsg_Init(Tcl_Interp *interp);
}

// functions

#ifdef AYCSGDBG
// aycsg_printppohcb:
//  ppoh callback to print the CSG type (for debugging purposes only)
int
aycsg_printppohcb(ay_object *o, FILE *fileptr, char *prefix)
{
 int ay_status = AY_OK;

  if(o->tags && (o->tags->type == aycsg_tm_tagtype))
    {
      if(prefix)
	{
	  fprintf(fileptr, "%s", prefix);
	}
      fprintf(fileptr, "Has TM tag\n");
    }

  if(o->type == AY_IDLEVEL)
    {

      if(prefix)
	{
	  fprintf(fileptr, "%s", prefix);
	}

      fprintf(fileptr, "CSG-Type: ");

      switch(o->modified)
	{
	case AY_LTEND:
	  fprintf(fileptr, "EndLevel");
	  break;
	case AY_LTLEVEL:
	  fprintf(fileptr, "Level");
	  break;
	case AY_LTUNION:
	  fprintf(fileptr, "Union");
	  break;
	case AY_LTDIFF:
	  fprintf(fileptr, "Difference");
	  break;
	case AY_LTINT:
	  fprintf(fileptr, "Intersection");
	  break;
	case AY_LTPRIM:
	  fprintf(fileptr, "Primitive");
	  break;
	default:
	  break;
	} // switch
      fprintf(fileptr, "\n");
    } // if

 return ay_status;
} // aycsg_printppohcb
#endif


// aycsg_rendertcb:
//  Togl callback that renders CSG in the view pointed to by <togl>
int
aycsg_rendertcb(struct Togl *togl, int argc, char *argv[])
{
 int ay_status = AY_OK;
 // Tcl_Interp *interp = ay_interp;
 ay_view_object *view = NULL;
 int orig_use_materialcolor = ay_prefs.use_materialcolor;
 GLfloat color[4] = {0.0f,0.0f,0.0f,0.0f};
 int is_csg;
#ifdef AYCSGDBG
 ay_printcb *cbv[4];

  cbv[0] = &ay_ppoh_prtype;
  cbv[1] = &ay_ppoh_prtrafos;
  cbv[2] = &aycsg_printppohcb/*ay_ppoh_prflags*/;
  cbv[3] = NULL;
#endif

  // do not use glColor()/glMaterial() while resolving CSG,
  // it is needed by OpenCSG...
  ay_prefs.use_materialcolor = AY_FALSE;

  view = (ay_view_object *)Togl_GetClientData(togl);

  aycsg_clearprimitives();
  aycsg_root = NULL;

  ay_status = aycsg_copytree(view->drawsel,
			     (view->drawsel||view->drawlevel)?
			     ay_currentlevel->object:ay_root->next,
			     &is_csg, &aycsg_root);

  ay_status = aycsg_delegateall(aycsg_root);
  
  ay_status = aycsg_removetlu(aycsg_root, &aycsg_root);
  
#ifdef AYCSGDBG
  ay_ppoh_print(aycsg_root, stdout, 0, cbv);
  /*  aycsg_cleartree(aycsg_root);
  aycsg_root = NULL;
  aycsg_cleartmtags();
  aycsg_tmtags = NULL;
  return TCL_OK;*/
#endif

  ay_status = aycsg_normalize(aycsg_root);

  ay_status = aycsg_flatten(aycsg_root, togl, AY_LTUNION);

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  if(view->drawsel || view->drawlevel)
    {
      glPushMatrix();
      ay_trafo_getall(ay_currentlevel->next);
    }

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

  // fill depth buffer (resolve CSG operations)
  glDisable(GL_LIGHTING);
  OpenCSG::render(primitives, algo, depthalgo);

  // now draw again using existing depth buffer bits and
  // possibly with colors
  glEnable(GL_DITHER);
  glEnable(GL_LIGHTING);
  glLightModelf(GL_LIGHT_MODEL_TWO_SIDE, (GLfloat)1.0);
  glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, GL_TRUE);

  color[0] = (GLfloat)ay_prefs.shr;
  color[1] = (GLfloat)ay_prefs.shg;
  color[2] = (GLfloat)ay_prefs.shb;
  color[3] = (GLfloat)1.0;

  glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, color);
  glMatrixMode(GL_MODELVIEW);

  ay_prefs.use_materialcolor = orig_use_materialcolor;
  glDepthFunc(GL_EQUAL);
   for(std::vector<OpenCSG::Primitive*>::const_iterator i =
	 primitives.begin(); i != primitives.end(); ++i) {
     (*i)->render();
   }
  glDepthFunc(GL_LESS);

  // now draw non-CSG top level primitives
  ay_status = aycsg_drawtoplevelprim(togl);

  if(view->drawsel || view->drawlevel)
    {
      glMatrixMode(GL_MODELVIEW);
      glPopMatrix();
    }

  // swap buffers
  Togl_SwapBuffers(togl);

  // clear local copy of object tree
  aycsg_cleartree(aycsg_root);
  aycsg_root = NULL;

  // clear all TM tags
  aycsg_cleartmtags();
  aycsg_tmtags = NULL;

 return TCL_OK;
} // aycsg_rendertcb


// aycsg_drawtoplevelprim:
//  draw non-CSG primitives in the top level of the scene
int
aycsg_drawtoplevelprim(Togl *togl)
{
 int ay_status = AY_OK;
 ay_object *t = aycsg_root;
 int has_tm = AY_FALSE;

  while(t)
    {
      // is t a primitive?
      if(t->CSGTYPE == AY_LTPRIM)
	{
	  if(t->tags && (t->tags->type == aycsg_tm_tagtype))
	    {
	      has_tm = AY_TRUE;
	      glPushMatrix();
	      glMultMatrixd((GLdouble*)(t->tags->val));
	      if(t->tags->name)
		{
		  glFrontFace(GL_CW);
		}
	    }
	  ay_status = ay_shade_object(togl, t, AY_FALSE);
	  if(has_tm)
	    {
	      if(t->tags->name)
		{
		  glFrontFace(GL_CCW);
		}
	      glPopMatrix();
	    }
	}
      t = t->next;
    } // while

 return ay_status;
} // aycsg_drawtoplevelprim


// aycsg_flatten:
//  _recursively_ convert tree <t> to primitive array for resolving CSG
//  and rendering, <t> should be normalized using aycsg_normalize()
int
aycsg_flatten(ay_object *t, struct Togl *togl, int parent_csgtype)
{
 int ay_status = AY_OK;

  while(t)
    {
      // is t a primitive?
      if((t->CSGTYPE == AY_LTPRIM) && (parent_csgtype != AY_LTUNION))
	{
	  // yes
	  // is this the lowest and leftmost primitive in the tree?
	  // the simple check (t->next) is possible, because the
	  // tree is binary _and_ normalized
	  if(t->next)
	    {
	      // yes, this is always an intersecting primitive
	      primitives.push_back(new OpenCSG::ayCSGPrimitive(t, togl,
						OpenCSG::Intersection, 1));
	    }
	  else
	    {
	      // no, use parent_csgtype to determine CSG operation
	      if(parent_csgtype == AY_LTDIFF)
		{
		  primitives.push_back(new OpenCSG::ayCSGPrimitive(t, togl,
						    OpenCSG::Subtraction, 1));
		}
	      else
		{
		  primitives.push_back(new OpenCSG::ayCSGPrimitive(t, togl,
						    OpenCSG::Intersection, 1));
		} // if
	    } // if

	}
      else
	{
	  // no
	  if((t->type == AY_IDLEVEL) && (t->down))
	    {
	      ay_status = aycsg_flatten(t->down, togl, t->CSGTYPE);
	    }
	} // if

     t = t->next;
    } // while

 return ay_status;
} // aycsg_flatten


// aycsg_clearprimitives:
//  clear the array of CSG primitives created by aycsg_flatten()
void
aycsg_clearprimitives()
{
  for(std::vector<OpenCSG::Primitive*>::const_iterator i =
	primitives.begin(); i != primitives.end(); ++i)
    {
      OpenCSG::ayCSGPrimitive* p =
	static_cast<OpenCSG::ayCSGPrimitive*>(*i);
      delete p;
    }

  primitives.clear();

 return;
} // aycsg_clearprimitives


// The following eight functions implement rules that transform
// a binary tree to disjunctive normal form (sum of products).

// Rule1: X - ( Y U Z ) => ( X - Y ) - Z
int
aycsg_applyrule1(ay_object *t)
{
 ay_object *X, *Y, *Z, *s;

  // 1. check applicability of rule
  if(!t || !t->down || !t->down->next)
    return AY_ERROR;

  if(t->CSGTYPE != AY_LTDIFF)
    return AY_ERROR;

  X = t->down;

  s = t->down->next;

  if(s->CSGTYPE != AY_LTUNION)
    return AY_ERROR;

  if(!s->down)
    return AY_ERROR;

  Y = s->down;
  Z = Y->next;

  // 2. transform hierarchy
  s->CSGTYPE = AY_LTDIFF;
  t->down = s;
  s->down = X;
  X->next = Y;
  Y->next = NULL;
  s->next = Z;

 return AY_OK;
} // aycsg_applyrule1


// Rule2: X O ( Y U Z ) => ( X O Y ) U ( X O Z )
//          t     r            r     t     s
int
aycsg_applyrule2(ay_object *t)
{
 ay_object *X, *Xnew, *Y, *Z, *r, *s;

  // 1. check applicability of rule
  if(!t || !t->down || !t->down->next)
    return AY_ERROR;

  if(t->CSGTYPE != AY_LTINT)
    return AY_ERROR;

  X = t->down;
  r = t->down->next;

  if(r->CSGTYPE != AY_LTUNION)
    return AY_ERROR;

  if(!r->down)
    return AY_ERROR;

  Y = r->down;
  Z = r->down->next;

  // 2. transform hierarchy
  if(!(Xnew = (ay_object*)calloc(1, sizeof(ay_object))))
    return AY_EOMEM;
  memcpy(Xnew, X, sizeof(ay_object));
  if(!(s = (ay_object*)calloc(1, sizeof(ay_object))))
    {free(Xnew); return AY_EOMEM;}
  memcpy(s, t, sizeof(ay_object));

  t->CSGTYPE = AY_LTUNION;
  r->CSGTYPE = AY_LTINT;
  s->CSGTYPE = AY_LTINT;
  t->down = r;
  r->next = s;
  r->down = X;
  X->next = Y;
  Y->next = NULL;
  s->down = Xnew;
  Xnew->next = Z;

 return AY_OK;
} // aycsg_applyrule2


// Rule3: X - ( Y O Z ) => ( X - Y ) U ( X - Z )
//          t     r            r     t     s
int
aycsg_applyrule3(ay_object *t)
{
 ay_object *X, *Xnew, *Y, *Z, *r, *s;

  // 1. check applicability of rule
  if(!t || !t->down || !t->down->next)
    return AY_ERROR;

  if(t->CSGTYPE != AY_LTDIFF)
    return AY_ERROR;

  X = t->down;
  r = t->down->next;

  if(r->CSGTYPE != AY_LTINT)
    return AY_ERROR;

  if(!r->down)
    return AY_ERROR;

  Y = r->down;
  Z = r->down->next;

  // 2. transform hierarchy
  if(!(Xnew = (ay_object*)calloc(1, sizeof(ay_object))))
    return AY_EOMEM;
  memcpy(Xnew, X, sizeof(ay_object));
  if(!(s = (ay_object*)calloc(1, sizeof(ay_object))))
    {free(Xnew); return AY_EOMEM;}
  memcpy(s, t, sizeof(ay_object));

  t->CSGTYPE = AY_LTUNION;
  r->CSGTYPE = AY_LTDIFF;
  s->CSGTYPE = AY_LTDIFF;
  t->down = r;
  r->next = s;
  r->down = X;
  X->next = Y;
  Y->next = NULL;
  s->down = Xnew;
  Xnew->next = Z;

 return AY_OK;
} // aycsg_applyrule3


// Rule4: X O ( Y O Z ) => ( X O Y ) O Z
int
aycsg_applyrule4(ay_object *t)
{
 ay_object *X, *Y, *Z, *s;

  // 1. check applicability of rule
  if(!t || !t->down || !t->down->next)
    return AY_ERROR;

  if(t->CSGTYPE != AY_LTDIFF)
    return AY_ERROR;

  X = t->down;
  s = t->down->next;

  if(s->CSGTYPE != AY_LTINT)
    return AY_ERROR;

  if(!s->down)
    return AY_ERROR;
  Y = s->down;
  Z = s->down->next;

  // 2. transform hierarchy
  t->down = s;
  s->next = Z;
  s->down = X;
  X->next = Y;
  Y->next = NULL;

 return AY_OK;
} // aycsg_applyrule4


// Rule5: X - ( Y - Z ) => ( X - Y ) U ( X O Z )
//          t     r            r     t     s
int
aycsg_applyrule5(ay_object *t)
{
 ay_object *X, *Xnew, *Y, *Z, *r, *s;

  // 1. check applicability of rule
  if(!t || !t->down || !t->down->next)
    return AY_ERROR;

  if(t->CSGTYPE != AY_LTDIFF)
    return AY_ERROR;

  X = t->down;
  r = t->down->next;

  if(r->CSGTYPE != AY_LTDIFF)
    return AY_ERROR;

  if(!r->down)
    return AY_ERROR;

  Y = r->down;
  Z = r->down->next;

  // 2. transform hierarchy
  if(!(Xnew = (ay_object*)calloc(1, sizeof(ay_object))))
    return AY_EOMEM;
  memcpy(Xnew, X, sizeof(ay_object));
  if(!(s = (ay_object*)calloc(1, sizeof(ay_object))))
    {free(Xnew); return AY_EOMEM;}
  memcpy(s, t, sizeof(ay_object));

  t->CSGTYPE = AY_LTUNION;
  r->CSGTYPE = AY_LTDIFF;
  s->CSGTYPE = AY_LTINT;
  t->down = r;
  r->next = s;
  r->down = X;
  X->next = Y;
  Y->next = NULL;
  s->down = Xnew;
  Xnew->next = Z;

 return AY_OK;
} // aycsg_applyrule5


// Rule6: X O ( Y - Z ) => ( X O Y ) - Z
int
aycsg_applyrule6(ay_object *t)
{
 ay_object *X, *Y, *Z, *s;

  // 1. check applicability of rule
  if(!t || !t->down || !t->down->next)
    return AY_ERROR;

  if(t->CSGTYPE != AY_LTINT)
    return AY_ERROR;

  X = t->down;

  s = t->down->next;

  if(s->CSGTYPE != AY_LTDIFF)
    return AY_ERROR;

  if(!s->down)
    return AY_ERROR;

  Y = s->down;
  Z = Y->next;

  // 2. transform hierarchy
  t->CSGTYPE = AY_LTINT;
  t->down = s;
  s->down = X;
  X->next = Y;
  s->next = Z;
  Y->next = NULL;

 return AY_OK;
} // aycsg_applyrule6


// Rule7: ( X U Y ) - Z => ( X - Z ) U ( Y - Z )
//            r     t          r     t     s
int
aycsg_applyrule7(ay_object *t)
{
 ay_object *X, *Y, *Z, *Znew, *r, *s;

  // 1. check applicability of rule
  if(!t || !t->down || !t->down->next)
    return AY_ERROR;

  if(t->CSGTYPE != AY_LTDIFF)
    return AY_ERROR;

  r = t->down;
  Z = t->down->next;

  if(r->CSGTYPE != AY_LTUNION)
    return AY_ERROR;

  if(!r->down)
    return AY_ERROR;

  X = r->down;
  Y = r->down->next;

  // 2. transform hierarchy
  if(!(Znew = (ay_object*)calloc(1, sizeof(ay_object))))
    return AY_EOMEM;
  memcpy(Znew, Z, sizeof(ay_object));
  if(!(s = (ay_object*)calloc(1, sizeof(ay_object))))
    {free(Znew); return AY_EOMEM;}
  memcpy(s, t, sizeof(ay_object));

  t->CSGTYPE = AY_LTUNION;
  r->CSGTYPE = AY_LTDIFF;
  s->CSGTYPE = AY_LTDIFF;
  t->down = r;
  r->next = s;
  r->down = X;
  X->next = Z;
  s->down = Y;
  Y->next = Znew;

 return AY_OK;
} // aycsg_applyrule7


// Rule8: ( X U Y ) O Z => ( X O Z ) U ( Y O Z )
//            r     t          r     t     s
int
aycsg_applyrule8(ay_object *t)
{
 ay_object *X, *Y, *Z, *Znew, *r, *s;

  // 1. check applicability of rule
  if(!t || !t->down || !t->down->next)
    return AY_ERROR;

  if(t->CSGTYPE != AY_LTINT)
    return AY_ERROR;

  r = t->down;
  Z = t->down->next;

  if(r->CSGTYPE != AY_LTUNION)
    return AY_ERROR;

  if(!r->down)
    return AY_ERROR;

  X = r->down;
  Y = r->down->next;

  // 2. transform hierarchy
  if(!(Znew = (ay_object*)calloc(1, sizeof(ay_object))))
    return AY_EOMEM;
  memcpy(Znew, Z, sizeof(ay_object));
  if(!(s = (ay_object*)calloc(1, sizeof(ay_object))))
    {free(Znew); return AY_EOMEM;}
  memcpy(s, t, sizeof(ay_object));

  t->CSGTYPE = AY_LTUNION;
  r->CSGTYPE = AY_LTINT;
  s->CSGTYPE = AY_LTINT;
  t->down = r;
  r->next = s;
  r->down = X;
  X->next = Z;
  s->down = Y;
  Y->next = Znew;

 return AY_OK;
} // aycsg_applyrule8


// aycsg_normalize:
//  convert CSG tree <t> to disjunctive normal (sum of products) form
//  by _recursively_ applying the transformation rules above
int
aycsg_normalize(ay_object *t)
{
 int ay_status = AY_OK;
 int done;

  while(t)
    {
      if((t->CSGTYPE != AY_LTPRIM) && t->down && t->down->next)
	{
	  do
	    {
	      done = AY_FALSE;
	      while(!done)
		{
		  done = AY_TRUE;
		  if(!aycsg_applyrule1(t))
		    done = AY_FALSE;
		  if(!aycsg_applyrule2(t))
		    done = AY_FALSE;
		  if(!aycsg_applyrule3(t))
		    done = AY_FALSE;
		  if(!aycsg_applyrule4(t))
		    done = AY_FALSE;
		  if(!aycsg_applyrule5(t))
		    done = AY_FALSE;
		  if(!aycsg_applyrule6(t))
		    done = AY_FALSE;
		  if(!aycsg_applyrule7(t))
		    done = AY_FALSE;
		  if(!aycsg_applyrule8(t))
		    done = AY_FALSE;
		} // while

	      if(t->down)
		{
		  ay_status = aycsg_normalize(t->down);
		}
	    }
	  while((t->CSGTYPE != AY_LTUNION) && (!(
		(t->down->next->CSGTYPE == AY_LTPRIM) ||
		t->down->CSGTYPE != AY_LTUNION)));

	  ay_status = aycsg_normalize(t->down->next);

	} // if
      t = t->next;
    } // while

 return ay_status;
} // aycsg_normalize


// aycsg_removetlu:
//  remove normal level and union objects in the toplevel of the
//  hierarchy (above all other CSG operations)
int
aycsg_removetlu(ay_object *o, ay_object **t)
{
 int ay_status = AY_OK;
 ay_object *last = NULL;
 //ay_level_object *l = NULL;

  // eliminate unions and levels above CSG trees
  while(o)
    {
      if(o->type == AY_IDLEVEL)
	{
	  //l = (ay_level_object *)(o->refine);
	  //if((l->type == AY_LTLEVEL) || (l->type == AY_LTLUNION))
	  if((o->CSGTYPE == AY_LTUNION) || (o->CSGTYPE == AY_LTPRIM))
	    {
	      // delegate transformation attributes
	      aycsg_delegatetrafo(o);

	      // replace o with its child(ren)
	      if(o->down)
		{
		  last = o->down;
		  while(last->next)
		    {
		      last = last->next;
		    }
		  last->next = o->next;
		}
	      else
		{
		  return AY_ENULL;
		} // if
	      *t = o->down;

	      free(o);
	      o = *t;
	    }
	  else
	    {
	      t = &(o->next);
	      o = o->next;
	    } // if
	}
      else
	{
	  t = &(o->next);
	  o = o->next;
	} // if
    } // while

 return ay_status;
} // aycsg_removetlu


// aycsg_delegatetrafo:
//  delegate transformation attributes from object <o> to its child(ren);
//  in contrast to ay_trafo_delegate(), this function also works properly
//  if the children already have arbitrary transformation attributes by
//  using TM tags to store additional transformations
int
aycsg_delegatetrafo(ay_object *o)
{
 int ay_status = AY_OK;
 ay_object *down = NULL;
 ay_tag_object *tag = NULL;
 aycsg_taglistelem *tle = NULL;
 double tm[16] = {0};

  if(!o)
    return AY_ENULL;

  if(!o->down)
    return AY_ERROR;

  down = o->down;

  if((o->movx != 0.0) || (o->movy != 0.0) || (o->movz != 0.0) ||
     (o->rotx != 0.0) || (o->roty != 0.0) || (o->rotz != 0.0) ||
     (o->scalx != 1.0) || (o->scaly != 1.0) || (o->scalz != 1.0) ||
     (o->quat[0] != 0.0) || (o->quat[1] != 0.0) || (o->quat[2] != 0.0) || 
     (o->quat[3] != 1.0) || (o->tags && (o->tags->type == aycsg_tm_tagtype)))
    {

      while(down)
	{

	  // has <o> a TM tag?
	  if(o->tags && (o->tags->type == aycsg_tm_tagtype))
	    {
	      // yes, copy it first (before the normal transformation
	      // attributes) to <down>

	      // has <down> already a TM tag?
	      if(down->tags && (down->tags->type == aycsg_tm_tagtype))
		{
		  // yes, just multiply new transformations into it
		  /*
		  memcpy(tm, o->tags->val, 16*sizeof(double));
		  ay_trafo_multmatrix4(tm, (double*)down->tags->val);
		  memcpy(down->tags->val, tm, 16*sizeof(double));
		  */
		  ay_trafo_multmatrix4((double*)down->tags->val,
				       (double*)o->tags->val);
		  if(o->tags->name)
		    {
		      if(down->tags->name)
			down->tags->name = (char*)AY_FALSE;
		      else
			down->tags->name = (char*)AY_TRUE;
		    }
		}
	      else
		{
		  // no, create, fill, and link a new tag
		  tag = NULL;
		  if(!(tag = (ay_tag_object*)calloc(1, sizeof(ay_tag_object))))
		    return AY_EOMEM;
		  tag->type = aycsg_tm_tagtype;
		  if(!(tag->val = (char*)calloc(16, sizeof(double))))
		    {
		      free(tag); return AY_EOMEM;
		    }
		  memcpy(tag->val, o->tags->val, 16*sizeof(double));
		  tag->name = (char*)o->tags->name;
		  tle = NULL;
		  if(!(tle = (aycsg_taglistelem*)calloc(1,
			       sizeof(aycsg_taglistelem))))
		    {
		      free(tag->val); free(tag); return AY_EOMEM;
		    }
		  tle->tag = tag;
		  tle->next = aycsg_tmtags;
		  aycsg_tmtags = tle;
		  tag->next = down->tags;
		  down->tags = tag;
		} // if
	    } // if
	  
	  // has <down> already a TM tag?
	  if(down->tags && (down->tags->type == aycsg_tm_tagtype))
	    {
	      // yes, just multiply new transformations into it
	      /*
	      ay_trafo_creatematrix(o, tm);
	      ay_trafo_multmatrix4(tm, (double*)down->tags->val);
	      memcpy(down->tags->val, tm, 16*sizeof(double));
	      */
	      ay_trafo_creatematrix(o, tm);
	      if((o->scalx*o->scaly*o->scalz)<0.0)
		{
		  if(down->tags->name)
		    down->tags->name = (char*)AY_FALSE;
		  else
		    down->tags->name = (char*)AY_TRUE;
		}
	      ay_trafo_multmatrix4((double*)down->tags->val, tm);
	    }
	  else
	    {
	      // no, create, fill, and link a new tag
	      tag = NULL;
	      if(!(tag = (ay_tag_object*)calloc(1, sizeof(ay_tag_object))))
		return AY_EOMEM;
	      tag->type = aycsg_tm_tagtype;
	      if(!(tag->val = (char*)calloc(16, sizeof(double))))
		{
		  free(tag); return AY_EOMEM;
		}
	      ay_trafo_creatematrix(o, (double*)tag->val);
	      if((o->scalx*o->scaly*o->scalz)<0.0)
		{
		 tag->name = (char*)AY_TRUE;
		}
	      tle = NULL;
	      if(!(tle = (aycsg_taglistelem*)calloc(1,
			   sizeof(aycsg_taglistelem))))
		{
		  free(tag->val); free(tag); return AY_EOMEM;
		}
	      tle->tag = tag;
	      tle->next = aycsg_tmtags;
	      aycsg_tmtags = tle;
	      tag->next = down->tags;
	      down->tags = tag;
	    } // if

	  // copy material pointer
	  if(!down->mat)
	    {
	      if(o->mat)
		{
		  down->mat = o->mat;
		} // if
	    } // if
	  
	  down = down->next;
	} // while

      // reset to default trafos
      o->movx = 0.0;
      o->movy = 0.0;
      o->movz = 0.0;

      o->scalx = 1.0;
      o->scaly = 1.0;
      o->scalz = 1.0;

      o->rotx = 0.0;
      o->roty = 0.0;
      o->rotz = 0.0;

      o->quat[0] = 0.0;
      o->quat[1] = 0.0;
      o->quat[2] = 0.0;
      o->quat[3] = 1.0;

      if(o->tags && (o->tags->type == aycsg_tm_tagtype))
	{
	  // no worries, we keep a pointer to the tm tag in aycsg_tmtags...
	  o->tags = o->tags->next;
	} // if
    } // if

 return ay_status;
} // aycsg_delegatetrafo


// aycsg_delegateall:
//  _recursively_ delegate transformation attributes from all CSG-Operation
//  level objects to their children for subtree <t>
int
aycsg_delegateall(ay_object *t)
{
 int ay_status = AY_OK;
 ay_object *o = t;

  if(!t)
    return AY_ENULL;

  while(o)
    {
      if(o->down && (o->type == AY_IDLEVEL)/* && (o->CSGTYPE != AY_LTPRIM)*/)
	{
	  ay_status = aycsg_delegatetrafo(o);
	} // if

      if(o->down)
	{
	  ay_status = aycsg_delegateall(o->down);
	} // if

      o = o->next;
    } // while

 return ay_status;
} // aycsg_delegateall


// aycsg_binarify:
//  _recursively_ convert n-ary subtree below <parent> to binary form,
//  creating copies of <parent> that are inserted in the existing list
//  of children
//  <left>: first new child of the new level
//  <target>: pointer to which new level should be chained
int
aycsg_binarify(ay_object *parent, ay_object *left, ay_object **target)
{
 int ay_status = AY_OK;
 ay_object *tmp = NULL;

  if((!parent) || (!left))
    return AY_ENULL;

  if(!left->next)
    { // nothing to be done? probably called wrongly...
      return AY_OK;
    }

  if(!(tmp = (ay_object*)calloc(1, sizeof(ay_object))))
    return AY_EOMEM;

  memcpy(tmp, parent, sizeof(ay_object));

  tmp->next = NULL;
  tmp->down = left;

  if(left->next->next)
    { // there are (still) more than two children
      // => wee need another intermediate level object
      ay_status = aycsg_binarify(parent, left->next, &(left->next));
    }

  // link the new level to the hierarchy
  *target = tmp;

 return AY_OK;
} // aycsg_binarify


// aycsg_copytree:
//  _recursively_ copies the tree pointed to by <t> into <target>;
//  omits terminating end-level objects!,
//  descends just into level objects, does not copy type specific objects,
//  converts to binary form, informs caller via <is_csg> whether subtree
//  <t> contains CSG operations, removes parents with a single child,
//  if <sel_only> is AY_TRUE: just copies selected objects
int
aycsg_copytree(int sel_only, ay_object *t, int *is_csg, ay_object **target)
{
 int ay_status = AY_OK;
 int lis_csg = 0;
 ay_level_object *l = NULL;
 ay_object *tmp = NULL;

  if(!t)
    return AY_ENULL;

  while(t->next)
    {
      if(t->hide || (sel_only && !t->selected))
	{
	  t = t->next;
	  continue;
	}

      if(!(*target = (ay_object*)calloc(1, sizeof(ay_object))))
	return AY_EOMEM;

      memcpy(*target, t, sizeof(ay_object));
      (*target)->next = NULL;
      (*target)->down = NULL;

      ay_status = AY_OK;

      // we just descend into non-empty level objects
      if((t->type == AY_IDLEVEL) && (t->down))
	{
	  // descend
	  ay_status = aycsg_copytree(AY_FALSE,
				     t->down, &lis_csg, &((*target)->down));
	}

      if(ay_status)
	return ay_status;

      // check for (and discard) level objects with just one child
      if((*target)->type == AY_IDLEVEL)
	{
	  if(((*target)->down) && (!(*target)->down->next))
	    {
	      // t has just one child, and may be discarded
	      aycsg_delegatetrafo(*target);
	      tmp = (*target)->down;
	      free(*target);
	      *target = tmp;
	      t = t->next;
	      target = &((*target)->next);
	      continue;
	    }
	}

      // we use the "modified" (CSGTYPE) flag to remember whether an object is
      // a primitive or a CSG operation and if it is a CSG op. of which type
      // (see ayam.h Level Object SubType Ids)

      if((*target)->type != AY_IDLEVEL)
	{
	  (*target)->CSGTYPE = AY_LTPRIM;
	}
      else
	{
	  l = (ay_level_object *)((*target)->refine);
	  switch(l->type)
	    {
	    case AY_LTUNION:
	      (*target)->CSGTYPE = AY_LTUNION;
	      break;
	    case AY_LTINT:
	      (*target)->CSGTYPE = AY_LTINT;
	      break;
	    case AY_LTDIFF:
	      (*target)->CSGTYPE = AY_LTDIFF;
	      break;
	    case AY_LTPRIM:
	      (*target)->CSGTYPE = AY_LTPRIM;
	      break;
	    default:
	      // CSGTYPE == AY_LTLEVEL
	      if(lis_csg)
		{
		  (*target)->CSGTYPE = AY_LTUNION;
		}
	      else
		{
		  (*target)->CSGTYPE = AY_LTPRIM;
		}
	      break;
	    } // switch

	  if(lis_csg || (l->type > 1) && (l->type < 5))
	    {
	      lis_csg = 1;

	      if((*target)->down && (*target)->down->next &&
		    (*target)->down->next->next)
		{
		  // *target has more than 2 children
		  // => convert to binary tree
		  ay_status = aycsg_binarify(*target, (*target)->down->next,
					     &((*target)->down->next));


		} // if
	    } // if
	} // if


      t = t->next;
      target = &((*target)->next);
    } // while

  *is_csg = lis_csg;

 return ay_status;
} // aycsg_copytree


// aycsg_cleartree:
//  clear the copy of the scene tree created by aycsg_copytree()
void
aycsg_cleartree(ay_object *t)
{
 ay_object *temp = NULL;

  while(t)
    {
      if(t->type == AY_IDLEVEL && t->down)
	{
	  aycsg_cleartree(t->down);
	} // if

      temp = t->next;
      free(t);
      t = temp;
    } // while

 return;
} // aycsg_cleartree


// aycsg_cleartmtags:
//  
void
aycsg_cleartmtags()
{
 aycsg_taglistelem *tle = NULL;

  while(aycsg_tmtags)
    {
      tle = aycsg_tmtags->next;
      
      if(aycsg_tmtags->tag)
	{
	  if(aycsg_tmtags->tag->val)
	    free(aycsg_tmtags->tag->val);
	  free(aycsg_tmtags->tag);
	} // if
      free(aycsg_tmtags);

      aycsg_tmtags = tle;
    } // while

 return;
} // aycsg_cleartmtags


extern "C" {

// Aycsg_Init:
//  initialize aycsg module
//  note: this function _must_ be capitalized exactly this way
//  regardless of the filename of the shared object (see: man n load)!
int
Aycsg_Init(Tcl_Interp *interp)
{
 char fname[] = "Aycsg_Init";
 int err;
 int ay_status = AY_OK;

  // first, check versions
  if(strcmp(ay_version_ma, aycsg_version_ma))
    {
      ay_error(AY_ERROR, fname,
	       "Plugin has been compiled for a different Ayam version!");
      ay_error(AY_ERROR, fname, "It is unsafe to continue! Bailing out...");
      return TCL_OK;
    }

  if(strcmp(ay_version_mi, aycsg_version_mi))
    {
      ay_error(AY_ERROR, fname,
	       "Plugin has been compiled for a different Ayam version!");
      ay_error(AY_ERROR, fname, "However, it is probably safe to continue...");
    }

  aycsg_root = NULL;

  // register TM tag type
  ay_status = ay_tags_register(interp, "TM", &aycsg_tm_tagtype);
  if(ay_status)
    return TCL_OK;

  aycsg_tmtags = NULL;

#ifdef AYCSGDBG
  ay_ppoh_init(interp);
#endif

  // initialize GLEW
  err = glewInit();
  if (GLEW_OK != err) {
    // problem: glewInit failed, something is seriously wrong
    ay_error(AY_ERROR, fname, "GLEW Initialization failed.");
    //ay_error(AY_ERROR, fname, glewGetErrorString(err));
    return TCL_OK;
  }

  // create a new command for all views (Togl widgets)
  Togl_CreateCommand("rendercsg", aycsg_rendertcb);

  ay_error(AY_EOUTPUT, fname, "Plugin 'aycsg' successfully loaded.");

 return TCL_OK;
} // Aycsg_Init

} // extern "C"

// remove local preprocessor definitions
#undef CSGTYPE
