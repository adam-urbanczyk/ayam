#ifndef __ppoh_h__
#define __ppoh_h__
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

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* ppoh.h - pretty print object hierarchies (for debugging purposes) */

typedef int (ay_printcb) (ay_object *o, FILE *fileptr, char *prefix);

int ay_ppoh_prtype(ay_object *o, FILE *fileptr, char *prefix);

int ay_ppoh_prtrafos(ay_object *o, FILE *fileptr, char *prefix);

int ay_ppoh_prflags(ay_object *o, FILE *fileptr, char *prefix);

int ay_ppoh_print(ay_object *o, FILE *fileptr, int indent, ay_printcb **cbv);

int ay_ppoh_printtcmd(ClientData clientData, Tcl_Interp *interp,
		      int argc, char *argv[]);

int ay_ppoh_init(Tcl_Interp *interp);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __ppoh_h__ */
