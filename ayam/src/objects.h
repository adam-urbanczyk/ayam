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

/* objects.h - prototypes of core objects */

/* box.c */
int ay_box_init(Tcl_Interp *interp);

/* bpatch.c */
int ay_bpatch_init(Tcl_Interp *interp);

/* camera.c */
int ay_camera_init(Tcl_Interp *interp);

/* camera.c */
int ay_cap_init(Tcl_Interp *interp);

/* clone.c */
int ay_clone_init(Tcl_Interp *interp);

/* cone.c */
int ay_cone_init(Tcl_Interp *interp);

/* concatnc.c */
int ay_concatnc_init(Tcl_Interp *interp);

/* cylinder.c */
int ay_cylinder_init(Tcl_Interp *interp);

/* disk.c */
int ay_disk_init(Tcl_Interp *interp);

/* extrude.c */
int ay_extrude_init(Tcl_Interp *interp);

/* hyperb.c */
int ay_hyperb_init(Tcl_Interp *interp);

/* icurve.c */
int ay_icurve_init(Tcl_Interp *interp);

/* instance.c */
int ay_instance_init(Tcl_Interp *interp);

/* level.c */
int ay_level_init(Tcl_Interp *interp);

/* light.c */
int ay_light_init(Tcl_Interp *interp);

/* material.c */
int ay_material_init(Tcl_Interp *interp);

/* ncurve.c */
int ay_ncurve_init(Tcl_Interp *interp);

/* npatch.c */
int ay_npatch_init(Tcl_Interp *interp);

/* pamesh.c */
int ay_pamesh_init(Tcl_Interp *interp);

/* pomesh.c */
int ay_pomesh_init(Tcl_Interp *interp);

/* parab.c */
int ay_parab_init(Tcl_Interp *interp);

/* root.c */
int ay_root_init(Tcl_Interp *interp);

/* revolve.c */
int ay_revolve_init(Tcl_Interp *interp);

/* riinc.c */
int ay_riinc_init(Tcl_Interp *interp);

/* sphere.c */
int ay_sphere_init(Tcl_Interp *interp);

/* skin.c */
int ay_skin_init(Tcl_Interp *interp);

/* sweep.c */
int ay_sweep_init(Tcl_Interp *interp);

/* torus.c */
int ay_torus_init(Tcl_Interp *interp);

/* view.c */
int ay_view_init(Tcl_Interp *interp);
