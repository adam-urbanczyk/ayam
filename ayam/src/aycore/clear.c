/*
 * Ayam, a free 3D modeler for the RenderMan interface.
 *
 * Ayam is copyrighted 1998-2001 by Randolf Schultz
 * (rschultz@informatik.uni-rostock.de) and others.
 *
 * All rights reserved.
 *
 * See the file License for details.
 *
 */

#include "ayam.h"

/* clear.c - clear scenes */

/* ay_clear_scene:
 *
 */
int
ay_clear_scene(void)
{
 int ay_status = AY_OK;
 char fname[] = "clear_scene";
 int result;
 ay_object *o = ay_root, *o2 = NULL;
 ay_root_object *root = NULL;
 char *ucargs[3] = {0}, ucarg1[] = "clear";

  /* for all referenced objects in scene:
   *  - check if there are references to them in the clipboard,
   *    if yes, cancel operation!
   */
  ay_status = ay_instt_checkclipboard(ay_root);
  if(ay_status)
    {
      ay_error(AY_ERROR, fname,
	       "There are referencing objects in the clipboard!");
      ay_error(AY_ERROR, fname,
	       "Use menu: \\\"Special/Paste (Move)\\\" first!");
      return AY_ERROR;
    }

  /* clear undo buffer */
  /*ay_status = ay_undo_clear();*/
  ucargs[1] = ucarg1;
  ay_undo_undotcmd(NULL, ay_interp, 2, ucargs);


  /* remove all instance objects from the scene */
  ay_status = ay_object_deleteinstances(&(ay_root->next));
  o = ay_root;

  /* for all material objects in scene:
   * - remove references to them from clipboard
   * - additionally, deregister those materials
   *   (this keeps them from trying to remove references to
   *    them while delete operation is in progress, which
   *    would make crash us badly)
   */
  ay_status = ay_matt_removecliprefs(ay_root);

  /* remove all references from normal objects to material objects */
  ay_status = ay_matt_removeallrefs(ay_root);

  /* remove all objects now, no unlink necessary */
  o = o->next;
  while(o->next)
    {
      o2 = o->next;

      ay_status = ay_object_delete(o);

      if(ay_status != AY_OK)
	printf("%s: Memory leaked!\n", fname);

      /* XXXX ignore errors about reference counts */
      o = o2;
    }
  ay_root->next = o;

  /* clear selection */
  ay_status = ay_sel_free(AY_FALSE);
  if(ay_status)
    {
      return ay_status;
    }

  /* clear clevel */
  result = ay_clevel_gotoptcmd(NULL, ay_interp, 0, NULL);

  /* free shaders from root object */
  root = (ay_root_object *)ay_root->refine;
  if(root->atmosphere)
    {
      ay_shader_free(root->atmosphere);
      root->atmosphere = NULL;
    }

  if(root->imager)
    {
      ay_shader_free(root->imager);
      root->imager = NULL;
    }

 return AY_OK;
} /* ay_clear_scene */


/* ay_clear_scenetcmd:
 *
 */
int
ay_clear_scenetcmd(ClientData clientData, Tcl_Interp * interp,
		   int argc, char *argv[])
{
 int ay_status = AY_OK;
 char fname[] = "newScene";

  ay_status = ay_clear_scene();
  if(ay_status)
    {
      ay_error(ay_status, fname, "Could not clear scene!");
    }

 return TCL_OK;
} /* ay_clear_scenetcmd */
