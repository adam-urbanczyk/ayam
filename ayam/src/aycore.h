#ifndef __aycore_h__
#define __aycore_h__
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

/* aycore.h - prototypes of core */

/* bbc.c */
int ay_bbc_get(ay_object *o, double *bbox);

/* clear.c */
int ay_clear_scene(void);

int ay_clear_scenetcmd(ClientData clientData, Tcl_Interp * interp,
		       int argc, char *argv[]);

/* clevel.c */
int ay_clevel_find(ay_object *c, ay_object *o, int *found);

int ay_clevel_add(ay_object *o);

int ay_clevel_del(void);

int ay_clevel_delall(void);

int ay_clevel_gotoptcmd(ClientData clientData, Tcl_Interp *interp,
			int argc, char *argv[]);

int ay_clevel_gouptcmd(ClientData clientData, Tcl_Interp *interp,
		       int argc, char *argv[]);

int ay_clevel_godowntcmd(ClientData clientData, Tcl_Interp *interp,
			 int argc, char *argv[]);

int ay_clevel_gettcmd(ClientData clientData, Tcl_Interp *interp,
		      int argc, char *argv[]);


/* clipb.c */
int ay_clipb_copytcmd(ClientData clientData, Tcl_Interp *interp,
		      int argc, char *argv[]);

int ay_clipb_cuttcmd(ClientData clientData, Tcl_Interp *interp,
		     int argc, char *argv[]);

int ay_clipb_pastetcmd(ClientData clientData, Tcl_Interp *interp,
		       int argc, char *argv[]);

int ay_clipb_movetcmd(ClientData clientData, Tcl_Interp *interp,
		      int argc, char *argv[]);

/* comp.c */
int ay_comp_strcase(char *s1, char *s2);

int ay_comp_register(ay_comparecb  *compcb, unsigned int type_id);

int ay_comp_init(void);

int ay_comp_objects(ay_object *o1, ay_object *o2);

int ay_comp_trafos(ay_object *o1, ay_object *o2);

int ay_comp_tags(ay_object *o1, ay_object *o2);

/* convert.c */
int ay_convert_register(ay_convertcb  *convcb, unsigned int type_id);

int ay_convert_force(ay_object *o, int in_place);

int ay_convert_forcetcmd(ClientData clientData, Tcl_Interp * interp,
			 int argc, char *argv[]);

/* draw.c */
void ay_draw_selp(ay_object *o);

int ay_draw_object(struct Togl *togl, ay_object *o, int selected);

int ay_draw_view(struct Togl *togl, int draw_offset);

void ay_draw_grid(struct Togl *togl);

void ay_draw_arrow(struct Togl *togl, double *from, double *to);

void ay_draw_bgimage(struct Togl *togl);

void ay_draw_needredraw(ay_list_object *oldsel, ay_list_object *newsel,
			int *result);

void ay_draw_trimview(void);

void ay_draw_rectangle(int winwidth, int winheight,
		       double xmin, double ymin,
		       double xmax, double ymax);

/* error.c */
void ay_error(int code, char *where, char *what);

int ay_error_tcmd(ClientData clientData, Tcl_Interp *interp,
		  int argc, char *argv[]);

void ay_error_glucb(GLenum err);

/* geom.c */
int ay_geom_intersectlines3D(double *p1, double *t1,
			     double *p2, double *t2,
			     double *p);

void ay_geom_calcnfrom3(double *p1, double *p2, double *p3, double *n);

/* instt.c */
int ay_instt_createoidht(ay_object *o);

int ay_instt_connect(ay_object *o, ay_object **last);

int ay_instt_createoid(char **dest);

int ay_instt_createorigids(ay_object *o);

int ay_instt_createinstanceids(ay_object *o);

int ay_instt_clearoidtags(ay_object *o);

int ay_instt_findinstance(ay_object *r, ay_object *o);

int ay_instt_checkclipboard(ay_object *o);

int ay_instt_wribiarchives(char *file, ay_object *o);

int ay_instt_resolve(ay_object *i);

int ay_instt_resolvetcmd(ClientData clientData, Tcl_Interp *interp,
			 int argc, char *argv[]);

int ay_instt_checkinstance(ay_object *o, ay_object *target,
			   ay_object *instance, int *check_this_tree);

int ay_instt_check(ay_object *o, ay_object *target);

int ay_instt_getmaster(ay_object *o, ay_object *i, ay_object **r);

int ay_instt_getmastertcmd(ClientData clientData, Tcl_Interp *interp,
			   int argc, char *argv[]);

void ay_instt_init(Tcl_Interp *interp);

/* interpol.c */
int ay_interpol_1DA4D(double p, int len, double *st, double *en, double *ta);

int ay_interpol_1DA1D(double p, int len, double *st, double *en, double *ta);

int ay_interpol_trafos(double p, ay_object *o1, ay_object *o2, ay_object *ta);

int ay_interpol_ncurves(double p, ay_object *c1, ay_object *c2,
			ay_object **ta);

/* mopsi.c */
int ay_mopsi_tcmd(ClientData clientData, Tcl_Interp * interp,
		  int argc, char *argv[]);

int ay_mopsi_register(char *tname, ay_mopsicb *cb);

int ay_mopsi_init(void);


/* notify.c */
int ay_notify_register(ay_notifycb *notcb, unsigned int type_id);

int ay_notify_parent(void);

int ay_notify_force(ay_object *o);

int ay_notify_forceparent(ay_object *o, int silent);

int ay_notify_forcetcmd(ClientData clientData, Tcl_Interp *interp,
			int argc, char *argv[]);

/* oact.c */
int ay_oact_movetcb(struct Togl *togl, int argc, char *argv[]);

int ay_oact_rottcb(struct Togl *togl, int argc, char *argv[]);

int ay_oact_rotatcb(struct Togl *togl, int argc, char *argv[]);

int ay_oact_sc1DXcb(struct Togl *togl, int argc, char *argv[]);

int ay_oact_sc1DYcb(struct Togl *togl, int argc, char *argv[]);

int ay_oact_sc1DZcb(struct Togl *togl, int argc, char *argv[]);

int ay_oact_sc2Dcb(struct Togl *togl, int argc, char *argv[]);

int ay_oact_sc3Dcb(struct Togl *togl, int argc, char *argv[]);

int ay_oact_str2Dcb(struct Togl *togl, int argc, char *argv[]);

/* object.c */
int ay_object_defaults(ay_object *o);

int ay_object_create(unsigned int index, ay_object **o);

int ay_object_createargs(unsigned int index, int argc, char **argv,
			 ay_object **o);

int ay_object_createtcmd(ClientData clientData, Tcl_Interp *interp,
			 int argc, char *argv[]);

int ay_object_delete(ay_object *o);

int ay_object_deletemulti(ay_object *o);

int ay_object_deletetcmd(ClientData clientData, Tcl_Interp *interp,
			 int argc, char *argv[]);

int ay_object_link(ay_object *o);

int ay_object_unlink(ay_object *o);

char *ay_object_gettypename(unsigned int index);

char *ay_object_getname(ay_object *o);

int ay_object_setnametcmd(ClientData clientData, Tcl_Interp *interp,
			  int argc, char *argv[]);

int ay_object_copy(ay_object *src, ay_object **dst);

int ay_object_copymulti(ay_object *src, ay_object **dst);

int ay_object_haschildtcmd(ClientData clientData, Tcl_Interp *interp,
			   int argc, char *argv[]);

int ay_object_gettypetcmd(ClientData clientData, Tcl_Interp *interp,
			  int argc, char *argv[]);

int ay_object_getnametcmd(ClientData clientData, Tcl_Interp *interp,
			  int argc, char *argv[]);

int ay_object_crtendlevel(ay_object **o);

int ay_object_deleteinstances(ay_object **o);

int ay_object_replace(ay_object *src, ay_object *dst);

int ay_object_ccp(ay_object *o);

/* objio.c */
int ay_objio_writescene(char *filename, int selected);

int ay_objio_init(Tcl_Interp *interp);

/* otype.c */
int ay_otype_register(char *name,
		      ay_createcb  *crtcb,
		      ay_deletecb  *delcb,
		      ay_copycb    *copycb,
		      ay_drawcb    *drawcb,
		      ay_drawcb    *drawhcb,
		      ay_drawcb    *shadecb,
		      ay_propcb    *setpropcb,
		      ay_propcb    *getpropcb,
		      ay_getpntcb  *getpntcb,
		      ay_readcb    *readcb,
		      ay_writecb   *writecb,
		      ay_wribcb    *wribcb,
		      ay_bbccb     *bbccb,
		      unsigned int *type_index);

int ay_otype_registercore(char *name,
			  ay_createcb  *crtcb,
			  ay_deletecb  *delcb,
			  ay_copycb    *copycb,
			  ay_drawcb    *drawcb,
			  ay_drawcb    *drawhcb,
			  ay_drawcb    *shadecb,
			  ay_propcb    *setpropcb,
			  ay_propcb    *getpropcb,
			  ay_getpntcb  *getpntcb,
			  ay_readcb    *readcb,
			  ay_writecb   *writecb,
			  ay_wribcb    *wribcb,
			  ay_bbccb     *bbccb,
			  unsigned int type_index);

/* matt.c */
void ay_matt_init(Tcl_Interp *interp);

int ay_matt_registermaterial(char *name, ay_mat_object *mat);

int ay_matt_isregistered(char *name);

int ay_matt_deregister(char *name);

int ay_matt_getmaterial(char *name, ay_mat_object **material);

int ay_matt_removeallrefs(ay_object *o);

int ay_matt_removerefs(ay_object *o, ay_mat_object *material);

int ay_matt_removecliprefs(ay_object *o);

int ay_matt_connect(ay_object *o);

int ay_matt_creatematerialids(ay_object *o);

int ay_matt_clearmaterialids(ay_object *o);

int ay_matt_wrib(char *file, ay_mat_object *m);

void ay_matt_nomaterial(unsigned int type);

int ay_matt_mayhavematerial(unsigned int type);

/* pact.c */
int ay_pact_getpoint(int mode, ay_object *o, double *obj);

void ay_pact_pedclear(ay_object *o);

int ay_pact_seltcb(struct Togl *togl, int argc, char *argv[]);

int ay_pact_deseltcb(struct Togl *togl, int argc, char *argv[]);

int ay_pact_insertptcb(struct Togl *togl, int argc, char *argv[]);

int ay_pact_deleteptcb(struct Togl *togl, int argc, char *argv[]);

int ay_pact_startpetcb(struct Togl *togl, int argc, char *argv[]);

int ay_pact_pedtcb(struct Togl *togl, int argc, char *argv[]);

void ay_pact_griddify(double *n, double grid);

int ay_pact_petcb(struct Togl *togl, int argc, char *argv[]);

int ay_pact_wetcb(struct Togl *togl, int argc, char *argv[]);

int ay_pact_wrtcb(struct Togl *togl, int argc, char *argv[]);

/* pomesht.c */
int ay_pomesht_tesselate(ay_pomesh_object *pomesh);

int ay_pomesht_merge(ay_list_object *list, ay_object **result);

int ay_pomesht_mergetcmd(ClientData clientData, Tcl_Interp * interp,
			 int argc, char *argv[]);

int ay_pomesht_optimizecoords(ay_pomesh_object *pomesh, int ignore_normals);

int ay_pomesht_optimizetcmd(ClientData clientData, Tcl_Interp * interp,
			    int argc, char *argv[]);

/* prefs.c */
int ay_prefs_gettcmd(ClientData clientData, Tcl_Interp *interp,
		    int argc, char *argv[]);

int ay_prefs_settcmd(ClientData clientData, Tcl_Interp *interp,
		    int argc, char *argv[]);

/* prop.c */
int ay_prop_gettcmd(ClientData clientData, Tcl_Interp *interp,
		    int argc, char *argv[]);

int ay_prop_settcmd(ClientData clientData, Tcl_Interp *interp,
		    int argc, char *argv[]);

int ay_prop_gettrafotcmd(ClientData clientData, Tcl_Interp *interp,
			 int argc, char *argv[]);

int ay_prop_settrafotcmd(ClientData clientData, Tcl_Interp *interp,
			 int argc, char *argv[]);

int ay_prop_getattrtcmd(ClientData clientData, Tcl_Interp *interp,
			int argc, char *argv[]);

int ay_prop_setattrtcmd(ClientData clientData, Tcl_Interp *interp,
			int argc, char *argv[]);

int ay_prop_getmattcmd(ClientData clientData, Tcl_Interp *interp,
			int argc, char *argv[]);

int ay_prop_setmattcmd(ClientData clientData, Tcl_Interp *interp,
			int argc, char *argv[]);

/* provide.c */
int ay_provide_register(ay_providecb  *provcb, unsigned int type_id);

int ay_provide_object(ay_object *o, unsigned int type, ay_object **result);

/* pv.c */
int ay_pv_filltokpar(ay_object *o, int declare, int start,
		     int *added, RtToken tokens[], RtPointer parms[]);

int ay_pv_count(ay_object *o);

void ay_pv_init(Tcl_Interp *interp);

/* quat.c */
void ay_quat_axistoquat(double a[3], double phi, double q[4]);

void ay_quat_norm(double q[4]);

void ay_quat_add(double q1[4], double q2[4], double dest[4]);

void ay_quat_torotmatrix(double q[4], double m[16]);

void ay_quat_toeuler(double q[4], double euler[3]);

int ay_quat_slerp(double time, double q1[4], double q2[4], double *r);

double ay_quat_dot(double q1[4], double q2[4]);

/* read.c */
int ay_read_string(FILE *fileptr, char **result);

int ay_read_unistring(FILE *fileptr, Tcl_UniChar **result);

int ay_read_skip(FILE *fileptr);

int ay_read_header(FILE *fileptr);

int ay_read_attributes(FILE *fileptr, ay_object *o);

int ay_read_shader(FILE *fileptr, ay_shader **result);

int ay_read_object(FILE *fileptr);

int ay_read_scene(Tcl_Interp *interp, char *filename, int insert);

int ay_read_replacetcmd(ClientData clientData, Tcl_Interp * interp,
			int argc, char *argv[]);

int ay_read_inserttcmd(ClientData clientData, Tcl_Interp * interp,
		       int argc, char *argv[]);

/* riattr.c */
int ay_riattr_wrib(ay_object *o);

void ay_riattr_init(Tcl_Interp *interp);

/* riopt.c */
int ay_riopt_wrib(ay_object *o);

void ay_riopt_init(Tcl_Interp *interp);

/* sdmesht.c */
int ay_sdmesht_tesselate(ay_sdmesh_object *sdmesh);

/* sel.c */
int ay_sel_free(int clear_selflag);

int ay_sel_add(ay_object *o);

int ay_sel_setfromlbtcmd(ClientData clientData, Tcl_Interp *interp,
			 int argc, char *argv[]);

int ay_sel_getseltcmd(ClientData clientData, Tcl_Interp *interp,
		      int argc, char *argv[]);

/* selp.c */
void ay_selp_clear(ay_object *o);

int ay_selp_selall(ay_object *o);

int ay_selp_selalltcmd(ClientData clientData, Tcl_Interp *interp,
		       int argc, char *argv[]);

int ay_selp_applytrafotcmd(ClientData clientData, Tcl_Interp *interp,
			   int argc, char *argv[]);

int ay_selp_invert(ay_object *o);

int ay_selp_inverttcmd(ClientData clientData, Tcl_Interp *interp,
		       int argc, char *argv[]);

/* shade.c */
int ay_shade_object(struct Togl *togl, ay_object *o, int push_name);

int ay_shade_view(struct Togl *togl);

/* shader.c */
#ifdef AYUSESLCARGS
int ay_shader_scanslcsarg(SLC_VISSYMDEF *symbol, Tcl_DString *ds);
#endif

int ay_shader_scanslctcmd(ClientData clientData, Tcl_Interp *interp,
			  int argc, char *argv[]);
#ifdef AYUSESLXARGS
int ay_shader_scanslxsarg(SLX_VISSYMDEF *symbol, Tcl_DString *ds);
#endif

int ay_shader_scanslxtcmd(ClientData clientData, Tcl_Interp *interp,
			  int argc, char *argv[]);

int ay_shader_free(ay_shader *shader);

int ay_shader_copyarg(ay_shader_arg *source, ay_shader_arg **dest);

int ay_shader_copy(ay_shader *source, ay_shader **dest);

int ay_shader_wrib(ay_shader *shader, int type, RtLightHandle *light_handle);

int ay_shader_gettcmd(ClientData clientData, Tcl_Interp * interp,
		      int argc, char *argv[]);

int ay_shader_settcmd(ClientData clientData, Tcl_Interp * interp,
		      int argc, char *argv[]);


/* table.c */
int ay_table_init(ay_table *table);

int ay_table_additem(ay_table *table, void *newitem,
		     unsigned int index, int policy);

/* tags.c */
int ay_tags_delall(ay_object *o);

int ay_tags_free(ay_tag_object *tag);

int ay_tags_copy(ay_tag_object *source, ay_tag_object **dest);

int ay_tags_copyall(ay_object *src, ay_object *dst);

int ay_tags_temp(Tcl_Interp *interp, char *name, int set, int *result);

int ay_tags_istemptcmd(ClientData clientData, Tcl_Interp * interp,
		       int argc, char *argv[]);

int ay_tags_append(ay_object *o, ay_tag_object *tag);

int ay_tags_register(Tcl_Interp *interp, char *name, char **result);

int ay_tags_settcmd(ClientData clientData, Tcl_Interp * interp,
		    int argc, char *argv[]);

int ay_tags_addtcmd(ClientData clientData, Tcl_Interp * interp,
		    int argc, char *argv[]);

int ay_tags_gettcmd(ClientData clientData, Tcl_Interp * interp,
		    int argc, char *argv[]);

int ay_tags_deletetcmd(ClientData clientData, Tcl_Interp * interp,
		       int argc, char *argv[]);

int ay_tags_parseplist(char *str, int declare, RtInt *argc, RtToken **tokensr,
		       RtPointer **valuesr);

/* tc.c */
int ay_tc_wrib(ay_object *o);

int ay_tc_wribindex(ay_object *o, int index);

int ay_tc_count(ay_object *o);

void ay_tc_init(Tcl_Interp *interp);

/* tcmd.c */
int ay_tcmd_reverttcmd(ClientData clientData, Tcl_Interp *interp,
		       int argc, char *argv[]);

void ay_tcmd_showall(ay_object *o);

int ay_tcmd_showtcmd(ClientData clientData, Tcl_Interp *interp,
		     int argc, char *argv[]);

void ay_tcmd_hideall(ay_object *o);

int ay_tcmd_hidetcmd(ClientData clientData, Tcl_Interp *interp,
		     int argc, char *argv[]);

int ay_tcmd_getvertcmd(ClientData clientData, Tcl_Interp *interp,
		       int argc, char *argv[]);

int ay_tcmd_getbppntfromindex(ay_bpatch_object *patch, int index,
			      double **p);

int ay_tcmd_getpointtcmd(ClientData clientData, Tcl_Interp *interp,
			 int argc, char *argv[]);

int ay_tcmd_setpointtcmd(ClientData clientData, Tcl_Interp *interp,
			 int argc, char *argv[]);

int ay_tcmd_getglerrortcmd(ClientData clientData, Tcl_Interp *interp,
			   int argc, char *argv[]);

#ifdef AYENABLEWAIT
int ay_tcmd_waitpidtcmd(ClientData clientData, Tcl_Interp *interp,
			int argc, char *argv[]);
#endif /* AYENABLEWAIT */

#ifdef AYENABLEFEXIT
int ay_tcmd_fastexittcmd(ClientData clientData, Tcl_Interp *interp,
			 int argc, char *argv[]);
#endif /* AYENABLEFEXIT */

int ay_tcmd_withobtcmd(ClientData clientData, Tcl_Interp *interp,
		       int argc, char *argv[]);

/* tgui.c */
int ay_tgui_init(Tcl_Interp *interp);

/* tmp.c */
int ay_tmp_gettcmd(ClientData clientData, Tcl_Interp *interp,
		   int argc, char *argv[]);

/* toglcb.c */
void ay_toglcb_create(struct Togl *togl);

void ay_toglcb_destroy(struct Togl *togl);

void ay_toglcb_reshape(struct Togl *togl);

void ay_toglcb_display(struct Togl *togl);


/* trafo.c */
void ay_trafo_apply3(double *c, double *m);

void ay_trafo_apply4(double *c, double *m);

void ay_trafo_getall(ay_list_object *lo);

void ay_trafo_getalli(ay_list_object *lo);

void ay_trafo_getalls(ay_list_object *lo);

void ay_trafo_getallis(ay_list_object *lo);

void ay_trafo_getallsr(ay_list_object *lo);

void ay_trafo_getallisr(ay_list_object *lo);

void ay_trafo_getallr(ay_list_object *lo);

void ay_trafo_getallir(ay_list_object *lo);

int ay_trafo_delegate(ay_object *o);

int ay_trafo_delegatetcmd(ClientData clientData, Tcl_Interp *interp,
			  int argc, char *argv[]);

void ay_trafo_applyall(ay_list_object *lo, ay_object *o, double *p);

void ay_trafo_applyalli(ay_list_object *lo, ay_object *o, double *p);

void ay_trafo_copy(ay_object *src, ay_object *dst);

void ay_trafo_add(ay_object *src, ay_object *dst);

void ay_trafo_defaults(ay_object *o);

int ay_trafo_movobtcmd(ClientData clientData, Tcl_Interp * interp,
		       int argc, char *argv[]);

int ay_trafo_movseltcmd(ClientData clientData, Tcl_Interp * interp,
		       int argc, char *argv[]);

int ay_trafo_scalobtcmd(ClientData clientData, Tcl_Interp * interp,
		    int argc, char *argv[]);

int ay_trafo_scalseltcmd(ClientData clientData, Tcl_Interp * interp,
			 int argc, char *argv[]);

int ay_trafo_rotobtcmd(ClientData clientData, Tcl_Interp * interp,
		       int argc, char *argv[]);

int ay_trafo_rotseltcmd(ClientData clientData, Tcl_Interp * interp,
			int argc, char *argv[]);

int ay_trafo_multmatrix4(double *m1, double *m2);

int ay_trafo_invmatrix4(double *m, double *mi);

int ay_trafo_apply(ay_object *o, double *p, int stride, int reusem);

int ay_trafo_creatematrix(ay_object *o, double *m);

void ay_trafo_identitymatrix(double *m);

void ay_trafo_translatematrix(double x, double y, double z, double *m);

void ay_trafo_scalematrix(double x, double y, double z, double *m);

void ay_trafo_rotatematrix(double angle, double x, double y, double z,
			   double *m);

void ay_trafo_pointstoplane(double x1, double y1, double z1,
			    double x2, double y2, double z2,
			    double x3, double y3, double z3,
			    double *A, double *B, double *C, double *D);
/* undo.c */
int ay_undo_init(int buffer_size);

int ay_undo_clear(void);

int ay_undo_clearobj(ay_object *o);

int ay_undo_undotcmd(ClientData clientData, Tcl_Interp *interp,
		     int argc, char *argv[]);

/* vact.c */
int ay_vact_movetcb(struct Togl *togl, int argc, char *argv[]);

int ay_vact_zoomtcb(struct Togl *togl, int argc, char *argv[]);

int ay_vact_moveztcb(struct Togl *togl, int argc, char *argv[]);


/* viewt.c */
void ay_viewt_setupprojection(struct Togl *togl);

void ay_viewt_rotate(ay_view_object *view, double rotx, double roty,
		     double rotz);

int ay_viewt_wintoobj(struct Togl *togl, ay_object *o,
		      double winX, double winY,
		      double *objX, double *objY, double *objZ);

int ay_viewt_winrecttoobj(struct Togl *togl, ay_object *o,
			  double winX, double winY, double winX2, double winY2,
			  double *obj);

int ay_viewt_wintoworld(struct Togl *togl, double winX, double winY,
			double *worldX, double *worldY, double *worldZ);

int ay_viewt_zoomtoobj(struct Togl *togl, int argc, char *argv[]);

int ay_viewt_align(struct Togl *togl, int argc, char *argv[]);

void ay_viewt_alignlocal();

int ay_viewt_makecurtcb(struct Togl *togl, int argc, char *argv[]);

void ay_viewt_changetype(ay_view_object *view, int type);

int ay_viewt_reshapetcb(struct Togl *togl, int argc, char *argv[]);

int ay_viewt_redrawtcb(struct Togl *togl, int argc, char *argv[]);

int ay_viewt_setconftcb(struct Togl *togl, int argc, char *argv[]);

int ay_viewt_fromcamtcb(struct Togl *togl, int argc, char *argv[]);

int ay_viewt_tocamtcb(struct Togl *togl, int argc, char *argv[]);

void ay_viewt_uprop(ay_view_object *view);

int ay_viewt_griddify(struct Togl *togl, double *winx, double *winy);

int ay_viewt_droptcb(struct Togl *togl, int argc, char *argv[]);

/* w32t.c */
#ifdef WIN32
int ay_w32t_w32killtcmd(ClientData clientData, Tcl_Interp *interp,
					  int argc, char *argv[]);
#endif

/* wrib.c */
int ay_wrib_noexport(ay_object *o);

void ay_wrib_aimz(RtPoint direction);

void ay_wrib_placecamera(RtPoint position, RtPoint direction, double roll);

int ay_wrib_rioptions(void);

int ay_wrib_checklights(ay_object *o);

int ay_wrib_lights(char *file, ay_object *o);

void ay_wrib_rootsh(int imager);

void ay_wrib_defmat(char *file);

int ay_wrib_trafos(ay_object *o);

int ay_wrib_object(char *file, ay_object *o);

int ay_wrib_refobject(char *file, ay_object *o);

int ay_wrib_scene(char *file, char *image, int temp, double *from, double *to,
		  double roll, double zoom, double nearp, double farp,
		  int width, int height, int type);

int ay_wrib_cb(struct Togl *togl, int argc, char *argv[]);

void ay_wrib_displaytags(void);

void ay_wrib_hidertags(void);

int ay_wrib_tcmd(ClientData clientData, Tcl_Interp * interp,
		 int argc, char *argv[]);

#ifdef AYENABLEPPREV
int ay_wrib_pprevdraw(ay_view_object *view);

int ay_wrib_pprevopen(ay_view_object *view);

void ay_wrib_pprevclose();
#endif

void ay_wrib_init(Tcl_Interp *interp);

/* write.c */
int ay_write_header(FILE *fileptr);

int ay_write_attributes(FILE *fileptr, ay_object *o);

int ay_write_shader(FILE *fileptr, ay_shader *shader);

int ay_write_object(FILE *fileptr, ay_object *o);

int ay_write_scene(char *fname, int selected);

int ay_write_scenetcmd(ClientData clientData, Tcl_Interp * interp,
		       int argc, char *argv[]);


#endif /* __aycore_h__ */
