#ifndef __contrib_h__
#define __contrib_h__
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

/* contrib.h */

/* ai.c */
int ay_ai_init(Tcl_Interp *interp);

/* sm.c */
typedef struct ay_sm_trafostack_s
{
  struct ay_sm_trafostack_s *next;
  double movx, movy, movz;
  double rotx, roty, rotz;
  double quat[4];
  double scalx, scaly, scalz;
} ay_sm_trafostack;

void ay_sm_wriballsm(char *file, ay_object *o, ay_sm_trafostack *trafo,
		     int rwidth, int rheight);

/* tree.c */
void ay_tree_getclevel(char *node);

ay_object *ay_tree_getobject(char *node);

int ay_tree_registerdrop(ay_treedropcb  *cb, unsigned int type_id);

int ay_tree_inittcmd(ClientData clientData, Tcl_Interp *interp,
		     int argc, char *argv[]);

/* objsel.c */
int ay_objsel_processcb(struct Togl *togl, int argc, char *argv[]);

int ay_objsel_getnametcmd(ClientData clientData, Tcl_Interp *interp,
			  int argc, char *argv[]);

int ay_objsel_getnmfrmndtcmd(ClientData clientData, Tcl_Interp *interp,
		  int argc, char *argv[]);

#endif /* __contrib_h__ */
