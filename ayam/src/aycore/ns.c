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

#include "ayam.h"

/* ns.c - Notify Script tag helpers */


/* prototypes of functions local to this module */

Tk_RestrictAction ay_ns_restrictall(ClientData clientData, XEvent *eventPtr);


/* functions: */

/* ay_ns_restrictall:
 *  
 */
Tk_RestrictAction
ay_ns_restrictall(ClientData clientData,
		      XEvent *eventPtr)
{
  return TK_DEFER_EVENT;
} /* ay_ns_restrictall */


/* ay_ns_execute:
 *  
 */
int
ay_ns_execute(ay_object *o, char *script)
{
 int ay_status = AY_OK, result = TCL_OK;
 char fname[] = "ns_execute";
 static int sema = 0;
 ay_list_object *l = NULL, *old_sel = NULL;
 ClientData old_restrictcd;

  if(!o || !script)
    return AY_ENULL;

  /* this semaphor protects ourselves from running in an endless
     recursive loop should the script modify our child objects */
  if(sema)
    {
      return AY_OK;
    }
  else
    {
      sema = 1;
    } /* if */

  old_sel = ay_selection;
  ay_selection = NULL;

  ay_sel_add(o);

  Tk_RestrictEvents(ay_ns_restrictall, NULL, &old_restrictcd);
  result = Tcl_GlobalEval(ay_interp, script);
  Tk_RestrictEvents(NULL, NULL, &old_restrictcd);

  if(result == TCL_ERROR)
    {
      ay_error(AY_ERROR, fname, "Script failed!");
    }

  /* restore old selection */
  while(ay_selection)
    {
      l = ay_selection->next;
      /*
      if(ay_selection->object)
	ay_selection->object->selected = AY_FALSE;
      */
      free(ay_selection);
      ay_selection = l;
    } /* while */

  ay_selection = old_sel;

  sema = 0;

 return AY_OK;
}/* ay_ns_execute */


/* ay_ns_init:
 *  initialize ns module by registering the NS tag type
 */
void
ay_ns_init(Tcl_Interp *interp)
{

  /* register NS tag type */
  ay_tags_register(interp, "NS", &ay_ns_tagtype);

 return;
} /* ay_ns_init */
