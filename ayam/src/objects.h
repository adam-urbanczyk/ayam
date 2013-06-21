#ifndef __objects_h__
#define __objects_h__
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

/* objects.h - prototypes of core object initializers */

/* acurve.c */
int ay_acurve_init(Tcl_Interp *interp);

/* bevel.c */
int ay_bevel_init(Tcl_Interp *interp);

/* birail1.c */
int ay_birail1_init(Tcl_Interp *interp);

/* birail2.c */
int ay_birail2_init(Tcl_Interp *interp);

/* box.c */
int ay_box_init(Tcl_Interp *interp);

/* bpatch.c */
int ay_bpatch_init(Tcl_Interp *interp);

/* camera.c */
int ay_camera_init(Tcl_Interp *interp);

/* cap.c */
int ay_cap_init(Tcl_Interp *interp);

/* clone.c */
int ay_clone_init(Tcl_Interp *interp);

/* cone.c */
int ay_cone_init(Tcl_Interp *interp);

/* concatnc.c */
int ay_concatnc_init(Tcl_Interp *interp);

/* concatnp.c */
int ay_concatnp_init(Tcl_Interp *interp);

/* cylinder.c */
int ay_cylinder_init(Tcl_Interp *interp);

/* disk.c */
int ay_disk_init(Tcl_Interp *interp);

/* extrnc.c */
int ay_extrnc_init(Tcl_Interp *interp);

/* extrnp.c */
int ay_extrnp_init(Tcl_Interp *interp);

/* extrude.c */
int ay_extrude_init(Tcl_Interp *interp);

/* gordon.c */
int ay_gordon_init(Tcl_Interp *interp);

/* hyperb.c */
int ay_hyperb_init(Tcl_Interp *interp);

/* icurve.c */
int ay_icurve_init(Tcl_Interp *interp);

/* instance.c */
int ay_instance_init(Tcl_Interp *interp);

/* ipatch.c */
int ay_ipatch_init(Tcl_Interp *interp);

/* level.c */
int ay_level_init(Tcl_Interp *interp);

/* light.c */
void ay_light_getfromto(ay_light_object *light, double *from, double *to,
			int *has_from, int *has_to);

int ay_light_init(Tcl_Interp *interp);

/* material.c */
int ay_material_init(Tcl_Interp *interp);

/* ncircle.c */
int ay_ncircle_init(Tcl_Interp *interp);

/* ncurve.c */
int ay_ncurve_init(Tcl_Interp *interp);

/* npatch.c */
int ay_npatch_init(Tcl_Interp *interp);

/* offnc.c */
int ay_offnc_init(Tcl_Interp *interp);

/* offnp.c */
int ay_offnp_init(Tcl_Interp *interp);

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

/* riproc.c */
int ay_riproc_init(Tcl_Interp *interp);

/* script.c */
int ay_script_init(Tcl_Interp *interp);

/* sdmesh.c */
int ay_sdmesh_init(Tcl_Interp *interp);

/* select.c */
int ay_select_init(Tcl_Interp *interp);

/* sphere.c */
int ay_sphere_init(Tcl_Interp *interp);

/* skin.c */
int ay_skin_init(Tcl_Interp *interp);

/* sweep.c */
int ay_sweep_init(Tcl_Interp *interp);

/* swing.c */
int ay_swing_init(Tcl_Interp *interp);

/* text.c */
int ay_text_init(Tcl_Interp *interp);

/* torus.c */
int ay_torus_init(Tcl_Interp *interp);

/* trim.c */
int ay_trim_init(Tcl_Interp *interp);

/* view.c */
int ay_view_init(Tcl_Interp *interp);

/*! \file objects.h \brief Ayam object initializers */

#endif /* __objects_h__ */
