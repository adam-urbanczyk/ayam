#include <GL/glew.h>
#include "ayam.h"

#include "opencsg.h"
#include "ayCSGPrimitive.h"

// local preprocessor definitions
#define CSGTYPE modified

// global variables
std::vector<OpenCSG::Primitive*> primitives;

OpenCSG::Algorithm algo = OpenCSG::Automatic;

OpenCSG::DepthComplexityAlgorithm depthalgo =
 OpenCSG::NoDepthComplexitySampling;

ay_object *aycsg_root;

char ayslb_version_ma[] = AY_VERSIONSTR;
char ayslb_version_mi[] = AY_VERSIONSTRMI;

// prototypes of functions local to this module
int aycsg_rendertcb(struct Togl *togl, int argc, char *argv[]);

int aycsg_flatten(ay_object *t);

int aycsg_applyrule1(ay_object *t);
int aycsg_applyrule2(ay_object *t);
int aycsg_applyrule3(ay_object *t);
int aycsg_applyrule4(ay_object *t);
int aycsg_applyrule5(ay_object *t);
int aycsg_applyrule6(ay_object *t);
int aycsg_applyrule7(ay_object *t);
int aycsg_applyrule8(ay_object *t);

int aycsg_normalize(ay_object *t);

int aycsg_binarify(ay_object *parent, ay_object *left, ay_object **target);

int aycsg_copytree(ay_object *t, int *is_csg, ay_object **target);

void aycsg_cleartree(ay_object *t);

void aycsg_clearprimitives();

extern "C" {
int Aycsg_Init(Tcl_Interp *interp);
}

// functions

int
aycsg_rendertcb(struct Togl *togl, int argc, char *argv[])
{
 int ay_status = AY_OK;
 // Tcl_Interp *interp = ay_interp;
 // ay_view_object *view = NULL;
 int orig_use_materialcolor = ay_prefs.use_materialcolor;
 GLfloat color[4] = {0.0f,0.0f,0.0f,0.0f};
 int is_csg;

  /* do not use glColor/glMaterial while drawing, it is needed by OpenCSG... */
  ay_prefs.use_materialcolor = AY_FALSE;

  aycsg_clearprimitives();

  ay_status = aycsg_copytree(ay_root->next, &is_csg, &aycsg_root);

  ay_status = aycsg_normalize(aycsg_root);

  ay_status = aycsg_flatten(aycsg_root);

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
  
  /* fill depth buffer (resolve CSG operations) */
  glDisable(GL_LIGHTING);
  OpenCSG::render(primitives, algo, depthalgo);

  /* now draw again using existing depth buffer bits and
     possibly with colors */
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
   for (std::vector<OpenCSG::Primitive*>::const_iterator i =
	  primitives.begin(); i != primitives.end(); ++i) {
     (*i)->render();
   }
  glDepthFunc(GL_LESS);

  /* swap buffers */
  Togl_SwapBuffers(togl);

  aycsg_cleartree(aycsg_root);

 return TCL_OK;
} // aycsg_rendertcb


// aycsg_flatten:
//  convert tree to primitive array for rendering
int
aycsg_flatten(ay_object *t)
{
 int ay_status = AY_OK;
 
  while(t)
    {
      // is t a primitive?
      if(t->modified)
	{
	  // No.
	  if(t->modified == 1)
	    {
	      primitives.push_back(new OpenCSG::ayCSGPrimitive(t,
						 OpenCSG::Intersection, 1));
	    }
	  else
	    {
	      primitives.push_back(new OpenCSG::ayCSGPrimitive(t,
						 OpenCSG::Subtraction, 1));
	    }

	  if((t->type == AY_IDLEVEL) && (t->down))
	    {
	      ay_status = aycsg_flatten(t->down);
	    }
	} // if

     t = t->next;
    } // while

 return ay_status;
} // aycsg_flatten


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
//  by recursively applying the transformation rules above
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


// aycsg_binarify:
//  convert n-ary subtree below <parent> to binary form, creating copies
//  of <parent> that are inserted in the existing list of children
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
//  recursively copies the tree pointed to by <t> into <target>;
//  omits terminating end-level objects!,
//  descends just into level objects, does not copy type specific objects,
//  converts to binary form, informs caller via <is_csg> whether subtree
//  <t> contains CSG operations
int
aycsg_copytree(ay_object *t, int *is_csg, ay_object **target)
{
 int ay_status = AY_OK;
 int lis_csg = 0;
 ay_level_object *l = NULL;
 ay_object *tmp = NULL;

  if(!t)
    return AY_ENULL;
  
  while(t->next)
    {
      if(!(*target = (ay_object*)calloc(1, sizeof(ay_object))))
	return AY_EOMEM;
      
      memcpy(*target, t, sizeof(ay_object));

      ay_status = AY_OK;

      // we just descend into non-empty level objects
      if((t->type == AY_IDLEVEL) && (t->down))
	{
	  // descend
	  ay_status = aycsg_copytree(t->down, &lis_csg, &((*target)->down));
	}

      if(ay_status)
	return ay_status;

      // check for (and discard) level objects with just one child
      if((*target)->type == AY_IDLEVEL)
	{
	  if(((*target)->down) && (!(*target)->down->next))
	    {
	      // t has just one child, and may be discarded
	      ay_trafo_delegate(*target);
	      tmp = (*target)->down;
	      free(*target);
	      *target = tmp;
	      t = t->next;
	      target = &((*target)->next);
	      continue;
	    }
	}
      
      // we use the "modified" flag to remember whether an object is
      // a primitive (flag == 0) or a CSG op and if it is a CSG op,
      // of which type: intersection (flag == 1) or subtraction (flag == 2)
      // or union (flag == 3)

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

	  if((l->type > 1) && (l->type < 5))
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


void
aycsg_cleartree(ay_object *t)
{
 ay_object *temp = NULL;

  while(t)
    {
      if((t->type == AY_IDLEVEL) && t->down)
	{
	  aycsg_cleartree(t->down);
	}
      temp =  t->next;
      free(temp);
      t = t->next;
    } // while

 return;
} // aycsg_cleartree


void
aycsg_clearprimitives()
{
  for (std::vector<OpenCSG::Primitive*>::const_iterator i =
	 primitives.begin(); i != primitives.end(); ++i)
    {
      OpenCSG::ayCSGPrimitive* p = 
	static_cast<OpenCSG::ayCSGPrimitive*>(*i);
      delete p;
    }
  
  primitives.clear();

 return;
} // aycsg_clearprimitives

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

  // first, check versions
  if(strcmp(ay_version_ma, ayslb_version_ma))
    {
      ay_error(AY_ERROR, fname,
	       "Plugin has been compiled for a different Ayam version!");
      ay_error(AY_ERROR, fname, "It is unsafe to continue! Bailing out...");
      return TCL_OK;
    }

  if(strcmp(ay_version_mi, ayslb_version_mi))
    {
      ay_error(AY_ERROR, fname,
	       "Plugin has been compiled for a different Ayam version!");
      ay_error(AY_ERROR, fname, "However, it is probably safe to continue...");
    }

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
