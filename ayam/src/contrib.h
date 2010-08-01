#ifndef __contrib_h__
#define __contrib_h__
/*
 * Ayam, a free 3D modeler for the RenderMan interface.
 *
 * Ayam is copyrighted 1998-2001 by Randolf Schultz
 * (randolf.schultz@gmail.com) and others.
 *
 * All rights reserved.
 *
 * See the file License for details.
 *
 */

#include "ayam.h"

/* contrib.h */


/* ai.c */

/** initialize automatic instancing module
 */
int ay_ai_init(Tcl_Interp *interp);


/* sm.c */

/** stack of transformations */
typedef struct ay_sm_trafostack_s
{
  struct ay_sm_trafostack_s *next; /**< next transformation */

  /** \name translation attributes */
  /*@{*/
  double movx, movy, movz;
  /*@}*/

  /** \name orientation attributes */
  /*@{*/
  double rotx, roty, rotz;
  /*@}*/

  /** \name scale attributes */
  /*@{*/
  double scalx, scaly, scalz;
  /*@}*/

  double quat[4]; /**< quaternion attribute */
} ay_sm_trafostack;

/** search scene for lightsources and create their shadowmaps
 */
void ay_sm_wriballsm(char *file, char *objfile, ay_object *o,
		     ay_sm_trafostack *trafo,
		     int rwidth, int rheight,
		     int selonly);


/* tree.c */

/** get node description from object
 */
int ay_tree_crtnodefromobj(ay_object *o, ay_object *l, int d,
			   char **node, char **ins, int *found);

/** create current level list from node description
 */
void ay_tree_getclevel(char *node);

/** get object from node description
 */
ay_object *ay_tree_getobject(char *node);

/** register a tree drop callback
 */
int ay_tree_registerdrop(ay_treedropcb  *cb, unsigned int type_id);

/** Tcl command to select objects from node descriptions
 */
int ay_tree_selecttcmd(ClientData clientData, Tcl_Interp *interp,
		       int argc, char *argv[]);

/** initialize tree management module
 */
int ay_tree_init(Tcl_Interp *interp);


/* objsel.c */

/** Togl action callback for object picking
 */
int ay_objsel_processcb(struct Togl *togl, int argc, char *argv[]);

/** Tcl command to get the name of an object from a node description
 */
int ay_objsel_getnmfrmndtcmd(ClientData clientData, Tcl_Interp *interp,
			     int argc, char *argv[]);

/*! \file contrib.h \brief Ayam contrib API */

#endif /* __contrib_h__ */
