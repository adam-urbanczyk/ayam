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

/* ayam.c - the main Ayam module - initialization and global symbols */

/* global symbols */
ay_prefs_object ay_prefs = {0};

Tcl_Interp *ay_interp;
ay_object *ay_root;
ay_object **ay_next;

ay_list_object *ay_selection;

ay_list_object *ay_currentlevel;

ay_object *ay_clipboard;

int ay_point_edit_coords_homogenous;
int ay_point_edit_coords_number;
double **ay_point_edit_coords;
ay_object *ay_point_edit_object;

Tcl_HashTable ay_otypesht;

/* function pointer tables */
ay_table ay_typenamest;

ay_table ay_createcbt;

ay_table ay_deletecbt;

ay_table ay_copycbt;

ay_table ay_drawcbt;

ay_table ay_drawhcbt;

ay_table ay_shadecbt;

ay_table ay_getpropcbt;

ay_table ay_setpropcbt;

ay_table ay_getpntcbt;

ay_table ay_wribcbt;

ay_table ay_readcbt;

ay_table ay_writecbt;

ay_table ay_notifycbt;

ay_table ay_treedropcbt;

ay_table ay_convertcbt;

ay_table ay_providecbt;

ay_table ay_bbccbt;


Tcl_HashTable ay_tagtypesht;

Tcl_HashTable ay_temptagtypesht;

ay_table ay_tagnamest;

ay_table ay_oidptrt;


int ay_errno;

ay_object *ay_last_read_object;

int ay_read_version;

char ay_version_ma[] = AYVERSIONSTR;
char ay_version_mi[] = AYVERSIONSTRMI;


char *ay_instt_oitagtype;

char *ay_riattr_tagtype;

char *ay_riopt_tagtype;

char *ay_tc_tagtype;

char *ay_ridisp_tagtype;

static char *ay_log = "/tmp/ay.log";

/* prototypes for initialization callbacks of additional included modules */
#ifdef AYIDRWRAPPED
  int Idr_Init(Tcl_Interp *interp);
#endif

#ifdef AYMETAWRAPPED
  int Metaobj_Init(Tcl_Interp *interp);
  int Metacomp_Init(Tcl_Interp *interp);
#endif

#ifdef AYRRIBWRAPPED
  int Rrib_Init(Tcl_Interp *interp);
#endif

/* ay_init:
 *  create tables, ay_root
 */
int
ay_init(Tcl_Interp *interp)
{
 int ay_status = AY_OK;
 char fname[] = "ay_init";

  ay_interp = interp;

  /* initialize AY tables */
  Tcl_InitHashTable(&ay_otypesht, TCL_STRING_KEYS);

  if((ay_status = ay_table_init(&ay_typenamest)))
    { ay_error(ay_status, fname, NULL); return AY_ERROR; }

  if((ay_status = ay_table_init(&ay_createcbt)))
    { ay_error(ay_status, fname, NULL); return AY_ERROR; }

  if((ay_status = ay_table_init(&ay_deletecbt)))
    { ay_error(ay_status, fname, NULL); return AY_ERROR; }

  if((ay_status = ay_table_init(&ay_copycbt)))
    { ay_error(ay_status, fname, NULL); return AY_ERROR; }

  if((ay_status = ay_table_init(&ay_drawcbt)))
    { ay_error(ay_status, fname, NULL); return AY_ERROR; }

  if((ay_status = ay_table_init(&ay_drawhcbt)))
    { ay_error(ay_status, fname, NULL); return AY_ERROR; }

  if((ay_status = ay_table_init(&ay_shadecbt)))
    { ay_error(ay_status, fname, NULL); return AY_ERROR; }

  if((ay_status = ay_table_init(&ay_setpropcbt)))
    { ay_error(ay_status, fname, NULL); return AY_ERROR; }

  if((ay_status = ay_table_init(&ay_getpropcbt)))
    { ay_error(ay_status, fname, NULL); return AY_ERROR; }

  if((ay_status = ay_table_init(&ay_getpntcbt)))
    { ay_error(ay_status, fname, NULL); return AY_ERROR; }

  if((ay_status = ay_table_init(&ay_notifycbt)))
    { ay_error(ay_status, fname, NULL); return AY_ERROR; }

  if((ay_status = ay_table_init(&ay_readcbt)))
    { ay_error(ay_status, fname, NULL); return AY_ERROR; }

  if((ay_status = ay_table_init(&ay_writecbt)))
    { ay_error(ay_status, fname, NULL); return AY_ERROR; }

  if((ay_status = ay_table_init(&ay_wribcbt)))
    { ay_error(ay_status, fname, NULL); return AY_ERROR; }

  if((ay_status = ay_table_init(&ay_bbccbt)))
    { ay_error(ay_status, fname, NULL); return AY_ERROR; }

  if((ay_status = ay_table_init(&ay_treedropcbt)))
    { ay_error(ay_status, fname, NULL); return AY_ERROR; }

  if((ay_status = ay_table_init(&ay_convertcbt)))
    { ay_error(ay_status, fname, NULL); return AY_ERROR; }

  if((ay_status = ay_table_init(&ay_providecbt)))
    { ay_error(ay_status, fname, NULL); return AY_ERROR; }

  /* Tags */
  Tcl_InitHashTable(&ay_tagtypesht, TCL_STRING_KEYS);

  Tcl_InitHashTable(&ay_temptagtypesht, TCL_STRING_KEYS);

  /* initialize instancing helper module */
  ay_instt_init(interp);

  /* initialize material object helper module */
  ay_matt_init(interp);

  /* initialize riattribute tag helper module */
  ay_riattr_init(interp);

  /* initialize tc tag helper module */
  ay_tc_init(interp);

  /* initialize rioption tag helper module */
  ay_riopt_init(interp);

  /* initialize wrib module */
  ay_wrib_init(interp);

  /* initialize object comparison helper module */
  if((ay_status = ay_comp_init()))
    { ay_error(ay_status, fname, NULL); return AY_ERROR; }

  /* initialize automatic instancing module */
  if((ay_status = ay_ai_init(interp)))
    { ay_error(ay_status, fname, NULL); return AY_ERROR; }

  /* fill tables (init object types) */
  if((ay_status = ay_root_init(interp)))
    { ay_error(ay_status, fname, NULL); return AY_ERROR; }

  if((ay_status = ay_view_init(interp)))
    { ay_error(ay_status, fname, NULL); return AY_ERROR; }

  if((ay_status = ay_box_init(interp)))
    { ay_error(ay_status, fname, NULL); return AY_ERROR; }

  if((ay_status = ay_bpatch_init(interp)))
    { ay_error(ay_status, fname, NULL); return AY_ERROR; }

  if((ay_status = ay_camera_init(interp)))
    { ay_error(ay_status, fname, NULL); return AY_ERROR; }

  if((ay_status = ay_cap_init(interp)))
    { ay_error(ay_status, fname, NULL); return AY_ERROR; }

  if((ay_status = ay_clone_init(interp)))
    { ay_error(ay_status, fname, NULL); return AY_ERROR; }

  if((ay_status = ay_cone_init(interp)))
    { ay_error(ay_status, fname, NULL); return AY_ERROR; }

  if((ay_status = ay_concatnc_init(interp)))
    { ay_error(ay_status, fname, NULL); return AY_ERROR; }

  if((ay_status = ay_cylinder_init(interp)))
    { ay_error(ay_status, fname, NULL); return AY_ERROR; }

  if((ay_status = ay_disk_init(interp)))
    { ay_error(ay_status, fname, NULL); return AY_ERROR; }

  if((ay_status = ay_extrude_init(interp)))
    { ay_error(ay_status, fname, NULL); return AY_ERROR; }

  if((ay_status = ay_hyperb_init(interp)))
    { ay_error(ay_status, fname, NULL); return AY_ERROR; }

  if((ay_status = ay_icurve_init(interp)))
    { ay_error(ay_status, fname, NULL); return AY_ERROR; }

  if((ay_status = ay_instance_init(interp)))
    { ay_error(ay_status, fname, NULL); return AY_ERROR; }

  if((ay_status = ay_level_init(interp)))
    { ay_error(ay_status, fname, NULL); return AY_ERROR; }

  if((ay_status = ay_light_init(interp)))
    { ay_error(ay_status, fname, NULL); return AY_ERROR; }

  if((ay_status = ay_parab_init(interp)))
    { ay_error(ay_status, fname, NULL); return AY_ERROR; }

  if((ay_status = ay_revolve_init(interp)))
    { ay_error(ay_status, fname, NULL); return AY_ERROR; }

  if((ay_status = ay_riinc_init(interp)))
    { ay_error(ay_status, fname, NULL); return AY_ERROR; }

  if((ay_status = ay_material_init(interp)))
    { ay_error(ay_status, fname, NULL); return AY_ERROR; }

  if((ay_status = ay_ncurve_init(interp)))
    { ay_error(ay_status, fname, NULL); return AY_ERROR; }

  if((ay_status = ay_npatch_init(interp)))
    { ay_error(ay_status, fname, NULL); return AY_ERROR; }

  if((ay_status = ay_pamesh_init(interp)))
    { ay_error(ay_status, fname, NULL); return AY_ERROR; }

  if((ay_status = ay_pomesh_init(interp)))
    { ay_error(ay_status, fname, NULL); return AY_ERROR; }

  if((ay_status = ay_sphere_init(interp)))
    { ay_error(ay_status, fname, NULL); return AY_ERROR; }

  if((ay_status = ay_skin_init(interp)))
    { ay_error(ay_status, fname, NULL); return AY_ERROR; }

  if((ay_status = ay_sweep_init(interp)))
    { ay_error(ay_status, fname, NULL); return AY_ERROR; }

  if((ay_status = ay_torus_init(interp)))
    { ay_error(ay_status, fname, NULL); return AY_ERROR; }

  /* initialize undo system */
  if((ay_status = ay_undo_init(10)))
    { ay_error(ay_status, fname, NULL); return AY_ERROR; }

  /* initialize mops import module */
    if((ay_status = ay_mopsi_init()))
    { ay_error(ay_status, fname, NULL); return AY_ERROR; }

  /* create root object */
  if((ay_status = ay_object_create(AY_IDROOT, &ay_root)))
    { ay_error(ay_status, fname, NULL); return AY_ERROR; }

  /* create terminating level object in current level */
  if((ay_status = ay_object_create(AY_IDLEVEL, &ay_root->next)))
    { ay_error(ay_status, fname, NULL); return AY_ERROR; }

  /* create terminating level object in views level */
  if((ay_status = ay_object_create(AY_IDLEVEL, &ay_root->down)))
    { ay_error(ay_status, fname, NULL); return AY_ERROR; }

  ay_next = &(ay_root->next);

  ay_currentlevel = NULL;
  ay_status = ay_clevel_add(NULL);
  ay_status = ay_clevel_add(ay_root);

  ay_selection = NULL;
  ay_clipboard = NULL;

  ay_prefs.handle_size = 6.0;
  ay_prefs.glu_sampling_tolerance = 30.0;
  ay_prefs.pick_epsilon = 0.2;

  ay_prefs.checklights = AY_TRUE;

  ay_prefs.bgr = 0.5;
  ay_prefs.bgg = 0.5;
  ay_prefs.bgb = 0.5;

  ay_prefs.obr = 0.0;
  ay_prefs.obg = 0.1;
  ay_prefs.obb = 0.45;

  ay_prefs.ser = 1.0;
  ay_prefs.seg = 1.0;
  ay_prefs.seb = 1.0;

  ay_prefs.grr = 0.6;
  ay_prefs.grg = 0.6;
  ay_prefs.grb = 0.6;

  ay_prefs.tpr = 0.5;
  ay_prefs.tpg = 0.0;
  ay_prefs.tpb = 0.0;

  ay_prefs.shr = 0.8;
  ay_prefs.shg = 0.75;
  ay_prefs.shb = 0.65;

  ay_prefs.use_sm = AY_FALSE;
  ay_prefs.edit_snaps_to_grid = AY_TRUE;

  ay_prefs.list_types = AY_TRUE;

  if(!(ay_prefs.logfile = calloc(strlen(ay_log)+1, sizeof(char))))
    return AY_EOMEM;
  strcpy(ay_prefs.logfile, ay_log);

  if(!(ay_prefs.pprender = calloc(3+1, sizeof(char))))
    {
      free(ay_prefs.logfile);
      return AY_EOMEM;
    }
  strcpy(ay_prefs.pprender, "rgl");

  /* no selected points */
  ay_point_edit_object = NULL;
  ay_point_edit_coords = NULL;
  ay_point_edit_coords_homogenous = AY_FALSE;
  ay_point_edit_coords_number = 0;

  /* initialize additional included modules */
#ifdef AYIDRWRAPPED
  if((ay_status = Idr_Init(interp)))
    { ay_error(ay_status, fname, NULL); return AY_ERROR; }
  Tcl_SetVar(interp, "AYIDRWRAPPED", "1", TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
#else
  Tcl_SetVar(interp, "AYIDRWRAPPED", "0", TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
#endif

#ifdef AYMETAWRAPPED
  if((ay_status = Metaobj_Init(interp)))
    { ay_error(ay_status, fname, NULL); return AY_ERROR; }
  if((ay_status = Metacomp_Init(interp)))
    { ay_error(ay_status, fname, NULL); return AY_ERROR; }
  Tcl_SetVar(interp, "AYMETAWRAPPED", "1", TCL_LEAVE_ERR_MSG |
	     TCL_GLOBAL_ONLY);
#else
  Tcl_SetVar(interp, "AYMETAWRAPPED", "0", TCL_LEAVE_ERR_MSG |
	     TCL_GLOBAL_ONLY);
#endif

#ifdef AYRRIBWRAPPED
  if((ay_status = Rrib_Init(interp)))
    { ay_error(ay_status, fname, NULL); return AY_ERROR; }
  Tcl_SetVar(interp, "AYRRIBWRAPPED", "1", TCL_LEAVE_ERR_MSG |
	     TCL_GLOBAL_ONLY);
#else
  Tcl_SetVar(interp, "AYRRIBWRAPPED", "0", TCL_LEAVE_ERR_MSG |
	     TCL_GLOBAL_ONLY);
#endif

 return ay_status;
} /* ay_init */


/*
 * Tcl_AppInit || ay_InitStandalone:
 *
 */
#ifdef AYWRAPPED
int ay_InitStandalone(Tcl_Interp *interp);
int
ay_InitStandalone(Tcl_Interp *interp)
#else
int
Tcl_AppInit(Tcl_Interp *interp)
#endif /* AYWRAPPED */
{
int ay_status = AY_OK;

#ifndef AYWRAPPED
  if(Tcl_Init(interp) == TCL_ERROR)
    return TCL_ERROR;

  if(Tk_Init(interp) == TCL_ERROR)
    return TCL_ERROR;
#endif /* AYWRAPPED */

  if(Togl_Init(interp) == TCL_ERROR)
    return TCL_ERROR;

  Togl_CreateFunc(ay_toglcb_create);
  Togl_DestroyFunc(ay_toglcb_destroy);
  Togl_DisplayFunc(ay_toglcb_display);
  Togl_ReshapeFunc(ay_toglcb_reshape);

  /* clear.c */
  Tcl_CreateCommand(interp, "newScene", ay_clear_scenetcmd,
		     (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  /* clevel.c */
  Tcl_CreateCommand(interp, "goTop", ay_clevel_gotoptcmd,
		     (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "goUp", ay_clevel_gouptcmd,
		     (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "goDown", ay_clevel_godowntcmd,
		     (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "getLevel", ay_clevel_gettcmd,
		     (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  /* clipb.c */
  Tcl_CreateCommand(interp, "copOb", ay_clipb_copytcmd,
		     (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "cutOb", ay_clipb_cuttcmd,
		     (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "pasOb", ay_clipb_pastetcmd,
		     (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "cmovOb", ay_clipb_movetcmd,
		     (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  /* conv.c */
  Tcl_CreateCommand(interp, "convOb", ay_convert_forcetcmd,
		    (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  /* draw.c */

  /* error.c */
  Tcl_CreateCommand(interp, "ayError", ay_error_tcmd,
		     (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  /* instt.c */
  Tcl_CreateCommand(interp, "resolveIn", ay_instt_resolvetcmd,
		     (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  /* mopsi.c */
  Tcl_CreateCommand(interp, "importMops", ay_mopsi_tcmd,
		    (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  /* notify.c */
  Tcl_CreateCommand(interp, "forceNot", ay_notify_forcetcmd,
		    (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  /* object.c */
  Tcl_CreateCommand(interp, "crtOb", ay_object_createtcmd,
		    (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "delOb", ay_object_deletetcmd,
		    (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "nameOb", ay_object_setnametcmd,
		    (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "hasChild", ay_object_haschildtcmd,
		    (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "getType", ay_object_gettypetcmd,
		    (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "getName", ay_object_getnametcmd,
		    (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  /* pomesht.c */
  Tcl_CreateCommand(interp, "mergePo", ay_pomesht_mergetcmd,
		    (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);
  Tcl_CreateCommand(interp, "optiPo", ay_pomesht_optimizetcmd,
		    (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  /* prop.c */
  Tcl_CreateCommand(interp, "setProp", ay_prop_settcmd,
		    (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "getProp", ay_prop_gettcmd,
		    (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "setTrafo", ay_prop_settrafotcmd,
		    (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "getTrafo", ay_prop_gettrafotcmd,
		    (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "setAttr", ay_prop_setattrtcmd,
		    (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "getAttr", ay_prop_getattrtcmd,
		    (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "setMat", ay_prop_setmattcmd,
		    (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "getMat", ay_prop_getmattcmd,
		    (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  /* prefs.c */
  Tcl_CreateCommand(interp, "setPrefs", ay_prefs_settcmd,
		    (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "getPrefs", ay_prefs_gettcmd,
		    (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  /* read.c */
  Tcl_CreateCommand(interp, "replaceScene", ay_read_replacetcmd,
		    (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "insertScene", ay_read_inserttcmd,
		    (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);


  /* sel.c */
  Tcl_CreateCommand(interp, "selOb", ay_sel_setfromlbtcmd,
		    (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "getSel", ay_sel_getseltcmd,
		    (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  /* shader.c */
  Tcl_CreateCommand(interp, "shaderScanSLC", ay_shader_scanslctcmd,
		    (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "shaderScanSLX", ay_shader_scanslxtcmd,
		    (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "shaderSet", ay_shader_settcmd,
		    (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "shaderGet", ay_shader_gettcmd,
		    (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  /* tags.c */
  Tcl_CreateCommand(interp, "tagIsTemp", ay_tags_istemptcmd,
		      (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);
  Tcl_CreateCommand(interp, "setTags", ay_tags_settcmd,
		      (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);
  Tcl_CreateCommand(interp, "addTag", ay_tags_addtcmd,
		      (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);
  Tcl_CreateCommand(interp, "getTags", ay_tags_gettcmd,
		      (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);
  Tcl_CreateCommand(interp, "delTags", ay_tags_deletetcmd,
		      (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  /* tcmd.c */
  Tcl_CreateCommand(interp, "revert", ay_tcmd_reverttcmd,
		    (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "show", ay_tcmd_showtcmd,
		    (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "hide", ay_tcmd_hidetcmd,
		    (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "getVersion", ay_tcmd_getvertcmd,
		    (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "getPnt", ay_tcmd_getpointtcmd,
		    (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "setPnt", ay_tcmd_setpointtcmd,
		    (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  /* tmp.c */
  Tcl_CreateCommand(interp, "tmpGet", ay_tmp_gettcmd,
		    (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  /* trafo.c */
  Tcl_CreateCommand(interp, "delegTrafo", ay_trafo_delegatetcmd,
		    (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "movOb", ay_trafo_movobtcmd,
		    (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "movSel", ay_trafo_movseltcmd,
		    (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "scalOb", ay_trafo_scalobtcmd,
		    (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "scalSel", ay_trafo_scalseltcmd,
		    (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "rotOb", ay_trafo_rotobtcmd,
		    (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "rotSel", ay_trafo_rotseltcmd,
		    (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  /* oact.c */
  Togl_CreateCommand("moveoac", ay_oact_movetcb);

  Togl_CreateCommand("rotoac", ay_oact_rottcb);

  Togl_CreateCommand("rotoaac", ay_oact_rotatcb);

  Togl_CreateCommand("sc1dxoac", ay_oact_sc1DXcb);

  Togl_CreateCommand("sc1dyoac", ay_oact_sc1DYcb);

  Togl_CreateCommand("sc1dzoac", ay_oact_sc1DZcb);

  Togl_CreateCommand("sc2doac", ay_oact_sc2Dcb);

  Togl_CreateCommand("sc3doac", ay_oact_sc3Dcb);

  Togl_CreateCommand("str2doac", ay_oact_str2Dcb);

  /* objsel.c */
  Togl_CreateCommand("processObjSel", ay_objsel_processcb);

  Tcl_CreateCommand(interp, "getNameFromNode", ay_objsel_getnmfrmndtcmd,
		    (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);
  
  /* pact.c */
  Togl_CreateCommand("selpac", ay_pact_seltcb);

  Togl_CreateCommand("deselpac", ay_pact_deseltcb);

  Togl_CreateCommand("insertpac", ay_pact_insertptcb);

  Togl_CreateCommand("deletepac", ay_pact_deleteptcb);

  Togl_CreateCommand("startpepac", ay_pact_startpetcb);

  Togl_CreateCommand("pepac", ay_pact_petcb);

  Togl_CreateCommand("dpepac", ay_pact_pedtcb);

  Togl_CreateCommand("wepac", ay_pact_wetcb);

  Togl_CreateCommand("wrpac", ay_pact_wrtcb);

  /* selp.c */
  Tcl_CreateCommand(interp, "selPoints", ay_selp_selalltcmd,
		    (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "applyTrafo", ay_selp_applytrafotcmd,
		    (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  /* undo.c */
  Tcl_CreateCommand(interp, "undo", ay_undo_undotcmd,
		    (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  /* vact.c */
  Togl_CreateCommand("movevac", ay_vact_movetcb);

  Togl_CreateCommand("zoomvac", ay_vact_zoomtcb);

  Togl_CreateCommand("movezvac", ay_vact_moveztcb);

  /* viewt.c */
  Togl_CreateCommand("redraw", ay_viewt_redrawtcb);

  Togl_CreateCommand("reshape", ay_viewt_reshapetcb);

  Togl_CreateCommand("setconf", ay_viewt_setconftcb);

  Togl_CreateCommand("mc", ay_viewt_makecurtcb);

  Togl_CreateCommand("zoomob", ay_viewt_zoomtoobj);

  Togl_CreateCommand("align", ay_viewt_align);

  Togl_CreateCommand("tocam", ay_viewt_tocamtcb);

  Togl_CreateCommand("fromcam", ay_viewt_fromcamtcb);

  /* wrib.c */
  Togl_CreateCommand("wrib", ay_wrib_cb);

  /* write.c */
  Tcl_CreateCommand(interp, "saveScene", ay_write_scenetcmd,
		    (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);


  /* contrib/tree.c */
  Tcl_CreateCommand(interp, "treeInit", ay_tree_inittcmd,
		    (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  /* NURBS */
  /* nurbs/ict.c */
  Tcl_CreateCommand(interp, "revertIC", ay_ict_reverttcmd,
		    (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);
  /* nurbs/nct.c */
  Tcl_CreateCommand(interp, "revertNC", ay_nct_reverttcmd,
		    (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "refineNC", ay_nct_refinetcmd,
		    (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "clampNC", ay_nct_clamptcmd,
		    (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "elevateNC", ay_nct_elevatetcmd,
		    (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "insknNC", ay_nct_insertkntcmd,
		    (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "collNC", ay_nct_collapsetcmd,
		    (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "explNC", ay_nct_explodetcmd,
		    (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Togl_CreateCommand("finduac", ay_nct_finducb);

  Tcl_CreateCommand(interp, "splitNC", ay_nct_splittcmd,
		    (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "concatNC", ay_nct_concattcmd,
		    (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "crtNCircle", ay_nct_crtncircletcmd,
		    (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "crtNRect", ay_nct_crtrecttcmd,
		    (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "crtClosedBS", ay_nct_crtclosedbsptcmd,
		    (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  /* nurbs/npt.c */

  Tcl_CreateCommand(interp, "crtNSphere", ay_npt_crtnspheretcmd,
		    (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "crtNSphere2", ay_npt_crtnsphere2tcmd,
		    (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "splitNP", ay_npt_splittocurvestcmd,
		    (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "buildNP", ay_npt_buildfromcurvestcmd,
		    (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);



/* inform Tcl-context about compile time configuration: */

#ifndef AYWRAPPED
Tcl_SetVar(interp,"AYWRAPPED", "0", TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
#else
Tcl_SetVar(interp, "AYWRAPPED", "1", TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
#endif /* AYWRAPPED */

#ifndef AYUSEAFFINE
Tcl_SetVar(interp,"AYUSEAFFINE", "0", TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
#else
Tcl_SetVar(interp, "AYUSEAFFINE", "1", TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
#endif /* AYUSEAFFINE */

#ifndef AYUSESLARGS
Tcl_SetVar(interp,"AYUSESLARGS", "0", TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
#else
Tcl_SetVar(interp, "AYUSESLARGS", "1", TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
#endif /* AYUSESLARGS */

#ifndef AYUSESLCARGS
Tcl_SetVar(interp,"AYUSESLCARGS", "0", TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
#else
Tcl_SetVar(interp, "AYUSESLCARGS", "1", TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
#endif /* AYUSESLCARGS */

#ifndef AYUSESLXARGS
Tcl_SetVar(interp,"AYUSESLXARGS", "0", TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
#else
Tcl_SetVar(interp, "AYUSESLXARGS", "1", TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
#endif /* AYUSESLXARGS */

#ifndef AYUSESLOARGS
Tcl_SetVar(interp,"AYUSESLOARGS", "0", TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
#else
Tcl_SetVar(interp, "AYUSESLOARGS", "1", TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
#endif /* AYUSESLOARGS */

#ifndef AYUSESOARGS
Tcl_SetVar(interp,"AYUSESOARGS", "0", TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
#else
Tcl_SetVar(interp, "AYUSESOARGS", "1", TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
#endif /* AYUSESOARGS */

#ifndef AYIDR
Tcl_SetVar(interp,"AYIDR", "0", TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
#else
Tcl_SetVar(interp, "AYIDR", "1", TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
#endif

#ifndef AYENABLEPPREV
Tcl_SetVar(interp,"AYENABLEPPREV", "0", TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
#else
Tcl_SetVar(interp, "AYENABLEPPREV", "1", TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
#endif /* AY_ENABLEPPREV */


  if((ay_status = ay_init(interp)) != AY_OK)
    {
      Tcl_SetResult (interp,
                     "Ayam: Initialization failed! Bailing out!",
                     TCL_STATIC);
      return TCL_ERROR;
    }

 Tcl_SetVar2(interp, "ay", "ay_version", AYVERSIONSTR, TCL_GLOBAL_ONLY |
	     TCL_LEAVE_ERR_MSG);

 return TCL_OK;
} /* Tcl_AppInit */


#ifndef AYWRAPPED
/********************************************/
/*main:                                     */
/********************************************/
int
main (int argc, char **argv)
{
  Tk_Main (argc, argv, Tcl_AppInit);
  return 0;
} /* main */
#endif /* AYWRAPPED */
