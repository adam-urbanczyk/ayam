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

/* ai.c - Automatic Instancing for Ayam */

/* original implementation for The Mops by H. Birkholz */
/* adapted for Ayam by R. Schultz */

/* XXXX change all sprintf()/Tcl_Eval()s to ay_error() calls? */

#include "ayam.h"

/* variables local to this module */

int comp_true, comp_false;

int ay_ai_ignoretags;
int ay_ai_ignoremat;

/* prototypes of functions local to this module */

int ay_ai_instanceabletype(unsigned int type);

int ay_ai_compchildren(ay_object *o1, ay_object *o2);

int ay_ai_instanceobject(ay_object *inst, ay_object *ref);

int ay_ai_createinstances(ay_object *ref, ay_object *o);

int ay_ai_makeinstances(ay_object *o, ay_object *instance_root);

int ay_ai_resolveinstances(ay_object *o);

int ay_ai_resolveinstancestcmd(ClientData clientData, Tcl_Interp *interp,
			       int argc, char *argv[]);

int ay_ai_countobjects(ay_object *o);

int ay_ai_makeinstancestcmd(ClientData clientData, Tcl_Interp *interp,
			    int argc, char *argv[]);

/* functions: */

/* ay_ai_instanceabletype:
 *  check for type if it is instanceable, return AY_TRUE if yes
 *  AY_FALSE if not
 */
int
ay_ai_instanceabletype(unsigned int type)
{

  if((type == AY_IDINSTANCE) ||
     (type == AY_IDROOT) ||
     (type == AY_IDLIGHT) ||
     (type == AY_IDMATERIAL) ||
     (type == AY_IDVIEW))
    return AY_FALSE;

 return AY_TRUE;
} /* ay_ai_instanceabletype */


/* ay_ai_compchildren:
 *  all children of "o2" must be instances of the children of o1 or
 *  the same instances as in o1's children
 */
int
ay_ai_compchildren(ay_object *o1, ay_object *o2)
{
 ay_object *c_o1, *c_o2;

  c_o1 = o1->down;
  c_o2 = o2->down;

  /* no children? */
  if(c_o1 && c_o2)
    {
      if(!c_o1->next && !c_o2->next)
	return AY_TRUE;
    }
  else
    {
      return AY_FALSE;
    }

  while((c_o1->next) && (c_o2->next) &&
	(((c_o2->type == AY_IDINSTANCE) && (c_o2->refine == c_o1)) ||
	((c_o1->type == AY_IDINSTANCE) && (c_o1->refine == c_o2->refine))) &&
	ay_comp_trafos(c_o1, c_o2))
    {
      c_o1 = c_o1->next;
      c_o2 = c_o2->next;
    }
  if((c_o1->next == NULL) && (c_o2->next == NULL))
    return AY_TRUE;
  else
    return AY_FALSE;
} /* ay_comp_children */


/* ay_ai_instanceobject:
 *  free all object specific memory and create instance
 */
int
ay_ai_instanceobject(ay_object *inst, ay_object *ref)
{
 int ay_status = AY_OK;
 ay_object *sub = NULL, *sub2 = NULL;
 void **arr = NULL;
 ay_deletecb *cb = NULL;
 ay_mat_object *mat = NULL;
 unsigned int *refcountptr;

  if(inst)
    {
      if(inst->down)
	{
	  sub = inst->down;
	  while(sub)
	    {
	      sub2 = sub->next;
	      ay_status = ay_object_delete(sub);
	      /* XXXX what, if it failed? */
	      sub = sub2;
	    }
	}

      /* delete the real object & attributes */
      /* code taken from aycore/object.c/ay_object_delete() */
      if(inst->refcount > 0)
	return AY_EREF;

      arr = ay_deletecbt.arr;
      cb = (ay_deletecb *)(arr[inst->type]);
      if(cb)
	ay_status = cb(inst->refine);

      if(ay_status)
	return ay_status;

      /* delete selected points */
      if(inst->selp)
	ay_selp_clear(inst);

      /* delete tags */
      ay_tags_delall(inst);

      /* remove reference to material */
      if(inst->mat)
	{
	  mat = inst->mat;
	  refcountptr = mat->refcountptr;
	  (*refcountptr)--;
	}

      /* free name */
      if(inst->name)
	{
	  free(inst->name);
	  inst->name = NULL;
	}

      /* finally, make the instance */
      inst->down = NULL;
      inst->selp = NULL;
      inst->name = NULL;
      inst->tags = NULL;
      inst->refine = ref;
      inst->type = AY_IDINSTANCE;
      ref->refcount++;
  }

 return ay_status;
} /* ay_ai_instanceobject */


/* ay_ai_createinstances:
 *  find matching objects and create instances
 *  for now check all objects in tree each time
 */
int
ay_ai_createinstances(ay_object *ref, ay_object *o)
{
 int ret = 0;

  while(o)
    {
      /* do not create instance of reference object itself */
      if(o != ref)
	{

	  if(o->down)
	    ret += ay_ai_createinstances(ref, o->down);

	  /* easy/fast checks first */
	  if((ref->type == o->type) && (o->refcount == 0))
	    {
	      /* could become an instance, check it out */
	      if((ay_comp_objects(ref, o)) &&
		 ((ay_ai_ignoretags) || (ay_comp_tags(ref, o))) &&
		 ((ay_ai_ignoremat) || (ref->mat == o->mat)) &&
		 (((ref->down == NULL) && (o->down == NULL)) ||
		  (ay_ai_compchildren(ref, o))))
		{
		  /* create instance */
		  ay_ai_instanceobject(o, ref);
		  ret++;
		} /* if */

	    } /* if */

      } /* if */

      o = o->next;
  } /* while */

 return ret;
} /* ay_ai_createinstances */


/* ay_ai_makeinstances:
 * find identical objects and create instances
 */
int
ay_ai_makeinstances(ay_object *o, ay_object *instance_root)
{
 int ret = 0;

  while(o->next)
    {
      if(o->down && (o->type != AY_IDNPATCH))
	{
          ret += ay_ai_makeinstances(o->down, instance_root);
	}

      /* now create instances of "o", if "o" isn´t an instance itself */
      if(o->type != AY_IDINSTANCE)
	{
          ret += ay_ai_createinstances(o, instance_root);
	}

      o = o->next;
    } /* while */

 return ret;
} /* ay_ai_makeinstances */


/* ay_ai_resolveinstances:
 *  this recursive procedure should replace each instance
 *  in the scene by the real object
 */
int
ay_ai_resolveinstances(ay_object *o)
{
 int ret = 0;

  while(o)
    {
      if(o->type == AY_IDINSTANCE)
	{
	  ay_instt_resolve(o);
	  ret++;
	}
      if(o->down)
	ret += ay_ai_resolveinstances(o->down);
      o = o->next;
    } /* while */

 return ret;
} /* ay_ai_resolveinstances */


/* ay_ai_resolveinstancestcmd:
 *  Tcl command to resolve instances
 */
int
ay_ai_resolveinstancestcmd(ClientData clientData, Tcl_Interp *interp,
			   int argc, char *argv[])
{
 char done_cmd[128];

  sprintf(done_cmd, "puts stdout \"ai: %d instances resolved.\"",
	  ay_ai_resolveinstances(ay_currentlevel->object));
  Tcl_Eval(interp, done_cmd);

 return TCL_OK;
} /* ay_ai_resolveinstancestcmd */


/* ay_ai_countobjects:
 * count scene objects for tests
 */
int
ay_ai_countobjects(ay_object *o)
{
 int ret = 0;

  while(o->next)
    {
      if(o->down)
	ret += ay_ai_countobjects(o->down);
      ret++;

      o = o->next;
    } /* while */

 return ret;
} /* ay_ai_countobjects */


/* ay_ai_makeinstancestcmd:
 * Tcl command for automatic instancing
 */
int
ay_ai_makeinstancestcmd(ClientData clientData, Tcl_Interp *interp,
			int argc, char *argv[])
{
 int ay_status = AY_OK;
 char done_cmd[64];
 Tcl_Obj *to = NULL, *toa = NULL, *ton = NULL;
 int dummy = 0;

  /* get ignore flags */
  toa = Tcl_NewStringObj("aiprefs",-1);
  ton = Tcl_NewStringObj("IgnoreTags",-1);
  to = Tcl_ObjGetVar2(interp, toa, ton, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp, to, &ay_ai_ignoretags);

  Tcl_SetStringObj(ton, "IgnoreMat",-1);
  to = Tcl_ObjGetVar2(interp, toa, ton, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp, to, &ay_ai_ignoremat);


  sprintf(done_cmd, "puts stdout \"ai: %d Objects\"",
  	  ay_ai_countobjects(ay_root));
  Tcl_Eval(interp, done_cmd);

  dummy = ay_ai_resolveinstances(ay_currentlevel->object);

  comp_true = 0;
  comp_false = 0;
  sprintf(done_cmd, "puts stdout \"ai: %d instances created.\"",
	  ay_ai_makeinstances(ay_currentlevel->object,
			    ay_currentlevel->object));
  Tcl_Eval(interp, done_cmd);
  /*
  sprintf(done_cmd, "puts stdout \"%d * true, %d * false\"",
  	  comp_true, comp_false);
  Tcl_Eval(interp, done_cmd);
  */
  Tcl_IncrRefCount(toa);Tcl_DecrRefCount(toa);
  Tcl_IncrRefCount(ton);Tcl_DecrRefCount(ton);

  /* clear all cached pointers to scene hierarchy */
  ay_status = ay_object_ccp(NULL);

 return TCL_OK;
} /* ay_ai_makeinstancestcmd */




#if 0
/* the following 2 procedures provides visualisation of one reference object
   and its instances within the tree view widget */

int ay_ai_get_nodes(char *node, ay_object *o, ay_object *ref,
		char *refvar, char *instvar, Tcl_Interp * interp) {
 int p, cnt;

  cnt = 0;
  p = strlen(node);
  while(o) {
      sprintf(&node[p], ":%d", cnt);
      if(o == ref) {
	  /* found the reference object; return in refvar */
	  Tcl_SetVar(interp, refvar, node, TCL_LEAVE_ERR_MSG);
      }
      if((o->type == AY_OTINSTANCE) && (o->object == ref)) {
	  /* found an instance of ref; return in instvar */
	  Tcl_SetVar(interp, instvar, node, TCL_APPEND_VALUE |
					    TCL_LIST_ELEMENT |
					    TCL_LEAVE_ERR_MSG);
      }
      if(o->down)
	  ay_ai_get_nodes(node, o->down, ref, refvar, instvar, interp);
      cnt++;
      o = o->next;
  }
  node[p] = 0;
}

int ay_ai_get_instances_list_tcmd(ClientData clientData, Tcl_Interp *interp,
			       int argc, char *argv[]) {
 ay_object *ref, *o;
 char buf[256], c;
 char fname[] = "ai_getInstancesList";
 char *string;
 int i, p, cnt;

  if(argc != 4) {
      ay_error(interp, AY_ERROR, fname, "Wrong number of arguments (3)!");
      return TCL_OK;
  }
  /* first we search the selected object arg[1] in the tree structure */
  string = argv[1];
  if(memcmp(string, "root:", 5)) {
      ay_error(interp, AY_ERROR, fname, "first arg should be node!");
      return TCL_OK;
  }

  Tcl_SetVar(interp, argv[2], "", TCL_LEAVE_ERR_MSG);
  Tcl_SetVar(interp, argv[3], "", TCL_LEAVE_ERR_MSG);
  o = NULL;
  i = 5;  /* point the first number in node string */
  while(string[i] != 0) {
      if(o)
	  o = o->down;
      else
	  o = ay_root;
      p = i;
      while((string[p] != 0) && (string[p] != ':'))
	  p++;
      c = string[p];
      string[p] = 0;
      cnt = strtol(&string[i], NULL, 10);
      string[p] = c;
      if(c != 0)
	  i = p+1;
      else
	  i = p;

      while((o) && (cnt)) {
	  o = o->next;
	  cnt--;
      }
      if(o == NULL) {
	  ay_error(interp, AY_ERROR, fname, "Error in tree structure!");
	  return TCL_OK;
      }
  }

  /* o should point the selected object */
  if(o->type != AY_OTINSTANCE)
      ref = o;
  else
      ref = o->object;


  strcpy(buf, "root");
  ay_ai_get_nodes(buf, ay_root, ref, argv[3], argv[2], interp);

  return TCL_OK;
}


/* the following 2 procedures provides visualisation of all instanced objects
   within the tree view widget */

int
ay_ai_get_refobjects(char *node, ay_object *o, char *objvar,
		     Tcl_Interp *interp)
{
 int p, cnt;

  cnt = 0;
  p = strlen(node);
  while(o) {
      sprintf(&node[p], ":%d", cnt);
      if(o->refcount) {
	  /* found an instance of ref; return in instvar */
	  Tcl_SetVar(interp, objvar, node, TCL_APPEND_VALUE |
					    TCL_LIST_ELEMENT |
					    TCL_LEAVE_ERR_MSG);
      }
      if(o->down)
	  ay_ai_get_refobjects(node, o->down, objvar, interp);
      cnt++;
      o = o->next;
  }
  node[p] = 0;

 return AY_OK;
} /* ay_ai_get_refobjects */


int
ay_ai_get_refobjects_list_tcmd(ClientData clientData, Tcl_Interp *interp,
			       int argc, char *argv[])
{
 ay_object *ref, *o;
 char buf[256], c;
 char fname[] = "ai_getRefObjectsList";

  if(argc != 2)
    {
      ay_error(interp, AY_ERROR, fname, "Wrong number of arguments (1)!");
      return TCL_OK;
    }

  strcpy(buf, "root");
  ay_ai_get_refobjects(buf, ay_root, argv[1], interp);

 return TCL_OK;
}

#endif

/* ay_ai_init:
 * initialize automatic instancing module
 */
int
ay_ai_init(Tcl_Interp *interp)
{
  /* char fname[] = "Ai_Init";*/

   /* Create Tcl commands */
  Tcl_CreateCommand(interp, "ai_makeInstances", ay_ai_makeinstancestcmd,
		    (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "ai_resolveInstances", ay_ai_resolveinstancestcmd,
		    (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);
  /*
  Tcl_CreateCommand(interp, "ai_getInstancesList", ay_ai_getinstanceslisttcmd,
		    (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);
  Tcl_CreateCommand(interp, "ai_getRefObjectsList",
                    ay_ai_getrefobjectslisttcmd,
                    (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);
  */

 return AY_OK;
} /* ay_ai_init */

