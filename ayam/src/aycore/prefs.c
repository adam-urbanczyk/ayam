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

/* prefs.c - functions for handling of preference settings */

void ay_prefs_setcolor(Tcl_Interp *interp, Tcl_Obj *arr, char *var,
		       double *r, double *g, double *b);

/* C -> Tcl! */
int
ay_prefs_gettcmd(ClientData clientData, Tcl_Interp *interp,
		 int argc, char *argv[])
{
 char *n1 = "ayprefs";
 Tcl_Obj *to = NULL, *toa = NULL, *ton = NULL;

  /* Modeling */
  toa = Tcl_NewStringObj(n1, -1);
  ton = Tcl_NewStringObj("PickEpsilon", -1);
  to = Tcl_NewDoubleObj(ay_prefs.pick_epsilon);
  Tcl_ObjSetVar2(interp, toa, ton, to, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton, "HandleSize", -1);
  to = Tcl_NewDoubleObj(ay_prefs.handle_size);
  Tcl_ObjSetVar2(interp, toa, ton, to, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton, "LazyNotify", -1);
  to = Tcl_NewIntObj(ay_prefs.lazynotify);
  Tcl_ObjSetVar2(interp, toa, ton, to, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton, "CompleteNotify", -1);
  to = Tcl_NewIntObj(ay_prefs.completenotify);
  Tcl_ObjSetVar2(interp, toa, ton, to, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton, "EditSnaps", -1);
  to = Tcl_NewIntObj(ay_prefs.edit_snaps_to_grid);
  Tcl_ObjSetVar2(interp, toa, ton, to, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton, "RationalPoints", -1);
  to = Tcl_NewIntObj(ay_prefs.rationalpoints);
  Tcl_ObjSetVar2(interp, toa, ton, to, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton, "GlobalMark", -1);
  to = Tcl_NewIntObj(ay_prefs.globalmark);
  Tcl_ObjSetVar2(interp, toa, ton, to, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton, "CreateAtMark", -1);
  to = Tcl_NewIntObj(ay_prefs.createatmark);
  Tcl_ObjSetVar2(interp, toa, ton, to, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton, "NormalizeTrafos", -1);
  to = Tcl_NewIntObj(ay_prefs.normalizetrafos);
  Tcl_ObjSetVar2(interp, toa, ton, to, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton, "NormalizeDigits", -1);
  to = Tcl_NewIntObj(ay_prefs.normalizedigits);
  Tcl_ObjSetVar2(interp, toa, ton, to, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton, "UndoLevels", -1);
  to = Tcl_NewIntObj(ay_prefs.undo_levels);
  Tcl_ObjSetVar2(interp, toa, ton, to, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton, "Snap3D", -1);
  to = Tcl_NewIntObj(ay_prefs.snap3d);
  Tcl_ObjSetVar2(interp, toa, ton, to, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton, "AllowWarp", -1);
  to = Tcl_NewIntObj(ay_prefs.allow_warp);
  Tcl_ObjSetVar2(interp, toa, ton, to, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton, "ConvResetDisplay", -1);
  to = Tcl_NewIntObj(ay_prefs.conv_reset_display);
  Tcl_ObjSetVar2(interp, toa, ton, to, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton, "Tolerance", -1);
  to = Tcl_NewDoubleObj(ay_prefs.glu_sampling_tolerance);
  Tcl_ObjSetVar2(interp, toa, ton, to, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton, "ToleranceA", -1);
  to = Tcl_NewDoubleObj(ay_prefs.glu_sampling_tolerance_a);
  Tcl_ObjSetVar2(interp, toa, ton, to, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton, "NPDisplayMode", -1);
  to = Tcl_NewIntObj(ay_prefs.np_display_mode);
  Tcl_ObjSetVar2(interp, toa, ton, to, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton, "NCDisplayMode", -1);
  to = Tcl_NewIntObj(ay_prefs.nc_display_mode);
  Tcl_ObjSetVar2(interp, toa, ton, to, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton, "NPDisplayModeA", -1);
  to = Tcl_NewIntObj(ay_prefs.np_display_mode_a);
  Tcl_ObjSetVar2(interp, toa, ton, to, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton, "NCDisplayModeA", -1);
  to = Tcl_NewIntObj(ay_prefs.nc_display_mode_a);
  Tcl_ObjSetVar2(interp, toa, ton, to, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton, "AvoidPwlCurve", -1);
  to = Tcl_NewIntObj(ay_prefs.glu_avoid_pwlcurve);
  Tcl_ObjSetVar2(interp, toa, ton, to, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton, "UseMatColor", -1);
  to = Tcl_NewIntObj(ay_prefs.use_materialcolor);
  Tcl_ObjSetVar2(interp, toa, ton, to, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton, "Background_R", -1);
  to = Tcl_NewIntObj((int)(ay_prefs.bgr*255));
  Tcl_ObjSetVar2(interp, toa, ton, to, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton, "Background_G", -1);
  to = Tcl_NewIntObj((int)(ay_prefs.bgg*255));
  Tcl_ObjSetVar2(interp, toa, ton, to, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton, "Background_B", -1);
  to = Tcl_NewIntObj((int)(ay_prefs.bgb*255));
  Tcl_ObjSetVar2(interp, toa, ton, to, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);


  Tcl_SetStringObj(ton, "Object_R", -1);
  to = Tcl_NewIntObj((int)(ay_prefs.obr*255));
  Tcl_ObjSetVar2(interp, toa, ton, to, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton, "Object_G", -1);
  to = Tcl_NewIntObj((int)(ay_prefs.obg*255));
  Tcl_ObjSetVar2(interp, toa, ton, to, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton, "Object_B", -1);
  to = Tcl_NewIntObj((int)(ay_prefs.obb*255));
  Tcl_ObjSetVar2(interp, toa, ton, to, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);


  Tcl_SetStringObj(ton, "Selection_R", -1);
  to = Tcl_NewIntObj((int)(ay_prefs.ser*255));
  Tcl_ObjSetVar2(interp, toa, ton, to, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton, "Selection_G", -1);
  to = Tcl_NewIntObj((int)(ay_prefs.seg*255));
  Tcl_ObjSetVar2(interp, toa, ton, to, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton, "Selection_B", -1);
  to = Tcl_NewIntObj((int)(ay_prefs.seb*255));
  Tcl_ObjSetVar2(interp, toa, ton, to, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);


  Tcl_SetStringObj(ton, "Grid_R", -1);
  to = Tcl_NewIntObj((int)(ay_prefs.grr*255));
  Tcl_ObjSetVar2(interp, toa, ton, to, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton, "Grid_G", -1);
  to = Tcl_NewIntObj((int)(ay_prefs.grg*255));
  Tcl_ObjSetVar2(interp, toa, ton, to, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton, "Grid_B", -1);
  to = Tcl_NewIntObj((int)(ay_prefs.grb*255));
  Tcl_ObjSetVar2(interp, toa, ton, to, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);


  Tcl_SetStringObj(ton, "Tag_R", -1);
  to = Tcl_NewIntObj((int)(ay_prefs.tpr*255));
  Tcl_ObjSetVar2(interp, toa, ton, to, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton, "Tag_G", -1);
  to = Tcl_NewIntObj((int)(ay_prefs.tpg*255));
  Tcl_ObjSetVar2(interp, toa, ton, to, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton, "Tag_B", -1);
  to = Tcl_NewIntObj((int)(ay_prefs.tpb*255));
  Tcl_ObjSetVar2(interp, toa, ton, to, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);


  Tcl_SetStringObj(ton, "Shade_R", -1);
  to = Tcl_NewIntObj((int)(ay_prefs.shr*255));
  Tcl_ObjSetVar2(interp, toa, ton, to, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton, "Shade_G", -1);
  to = Tcl_NewIntObj((int)(ay_prefs.shg*255));
  Tcl_ObjSetVar2(interp, toa, ton, to, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton, "Shade_B", -1);
  to = Tcl_NewIntObj((int)(ay_prefs.shb*255));
  Tcl_ObjSetVar2(interp, toa, ton, to, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton, "Light_R", -1);
  to = Tcl_NewIntObj((int)(ay_prefs.lir*255));
  Tcl_ObjSetVar2(interp, toa, ton, to, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton, "Light_G", -1);
  to = Tcl_NewIntObj((int)(ay_prefs.lig*255));
  Tcl_ObjSetVar2(interp, toa, ton, to, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton, "Light_B", -1);
  to = Tcl_NewIntObj((int)(ay_prefs.lib*255));
  Tcl_ObjSetVar2(interp, toa, ton, to, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton, "SelXOR_R", -1);
  to = Tcl_NewIntObj((int)(ay_prefs.sxr*255));
  Tcl_ObjSetVar2(interp, toa, ton, to, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton, "SelXOR_G", -1);
  to = Tcl_NewIntObj((int)(ay_prefs.sxg*255));
  Tcl_ObjSetVar2(interp, toa, ton, to, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton, "SelXOR_B", -1);
  to = Tcl_NewIntObj((int)(ay_prefs.sxb*255));
  Tcl_ObjSetVar2(interp, toa, ton, to, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton, "CullFaces", -1);
  to = Tcl_NewIntObj(ay_prefs.cullfaces);
  Tcl_ObjSetVar2(interp, toa, ton, to, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);


  /* RIB */
  Tcl_SetStringObj(ton, "ResInstances", -1);
  to = Tcl_NewIntObj(ay_prefs.resolveinstances);
  Tcl_ObjSetVar2(interp, toa, ton, to, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton, "CheckLights", -1);
  to = Tcl_NewIntObj(ay_prefs.checklights);
  Tcl_ObjSetVar2(interp, toa, ton, to, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton, "DefaultMat", -1);
  to = Tcl_NewIntObj(ay_prefs.defaultmat);
  Tcl_ObjSetVar2(interp, toa, ton, to, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton, "RIStandard", -1);
  to = Tcl_NewIntObj(ay_prefs.ristandard);
  Tcl_ObjSetVar2(interp, toa, ton, to, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton, "WriteIdent", -1);
  to = Tcl_NewIntObj(ay_prefs.writeident);
  Tcl_ObjSetVar2(interp, toa, ton, to, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton, "ShadowMaps", -1);
  to = Tcl_NewIntObj(ay_prefs.use_sm);
  Tcl_ObjSetVar2(interp, toa, ton, to, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton, "ExcludeHidden", -1);
  to = Tcl_NewIntObj(ay_prefs.excludehidden);
  Tcl_ObjSetVar2(interp, toa, ton, to, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  /* Misc */
  Tcl_SetStringObj(ton, "SingleWindow", -1);
  to = Tcl_NewIntObj(ay_prefs.single_window);
  Tcl_ObjSetVar2(interp, toa, ton, to, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton, "ListTypes", -1);
  to = Tcl_NewIntObj(ay_prefs.list_types);
  Tcl_ObjSetVar2(interp, toa, ton, to, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton, "MarkHidden", -1);
  to = Tcl_NewIntObj(ay_prefs.mark_hidden);
  Tcl_ObjSetVar2(interp, toa, ton, to, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton, "Logging", -1);
  to = Tcl_NewIntObj(ay_prefs.writelog);
  Tcl_ObjSetVar2(interp, toa, ton, to, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton, "LogFile", -1);
  to = Tcl_NewStringObj(ay_prefs.logfile, -1);
  Tcl_ObjSetVar2(interp, toa, ton, to, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton, "ErrorLevel", -1);
  to = Tcl_NewIntObj((int)ay_prefs.errorlevel);
  Tcl_ObjSetVar2(interp, toa, ton, to, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton, "PPRender", -1);
  to = Tcl_NewStringObj(ay_prefs.pprender, -1);
  Tcl_ObjSetVar2(interp, toa, ton, to, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton, "SMethod", -1);
  to = Tcl_NewIntObj(ay_prefs.smethod - 1);
  Tcl_ObjSetVar2(interp, toa, ton, to, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton, "SParamU", -1);
  to = Tcl_NewDoubleObj(ay_prefs.sparamu);
  Tcl_ObjSetVar2(interp, toa, ton, to, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton, "SParamV", -1);
  to = Tcl_NewDoubleObj(ay_prefs.sparamv);
  Tcl_ObjSetVar2(interp, toa, ton, to, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton, "WarnUnknownTag", -1);
  to = Tcl_NewIntObj(ay_prefs.wutag);
  Tcl_ObjSetVar2(interp, toa, ton, to, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton, "LineWidth", -1);
  to = Tcl_NewDoubleObj(ay_prefs.linewidth);
  Tcl_ObjSetVar2(interp, toa, ton, to, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton, "SelLineWidth", -1);
  to = Tcl_NewDoubleObj(ay_prefs.sellinewidth);
  Tcl_ObjSetVar2(interp, toa, ton, to, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton, "AALineWidth", -1);
  to = Tcl_NewDoubleObj(ay_prefs.aalinewidth);
  Tcl_ObjSetVar2(interp, toa, ton, to, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton, "AASelLineWidth", -1);
  to = Tcl_NewDoubleObj(ay_prefs.aasellinewidth);
  Tcl_ObjSetVar2(interp, toa, ton, to, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton, "AAFudge", -1);
  to = Tcl_NewDoubleObj(ay_prefs.aafudge);
  Tcl_ObjSetVar2(interp, toa, ton, to, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton, "SDMode", -1);
  to = Tcl_NewIntObj(ay_prefs.sdmode);
  Tcl_ObjSetVar2(interp, toa, ton, to, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton, "PolyOffset0", -1);
  to = Tcl_NewDoubleObj(ay_prefs.polyoffset0);
  Tcl_ObjSetVar2(interp, toa, ton, to, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton, "PolyOffset1", -1);
  to = Tcl_NewDoubleObj(ay_prefs.polyoffset1);
  Tcl_ObjSetVar2(interp, toa, ton, to, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton, "PVTexCoordName", -1);
  to = Tcl_NewStringObj(ay_prefs.texcoordname, -1);
  Tcl_ObjSetVar2(interp, toa, ton, to, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton, "PVNormalName", -1);
  to = Tcl_NewStringObj(ay_prefs.normalname, -1);
  Tcl_ObjSetVar2(interp, toa, ton, to, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton, "PVTangentName", -1);
  to = Tcl_NewStringObj(ay_prefs.tangentname, -1);
  Tcl_ObjSetVar2(interp, toa, ton, to, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton, "PVColorName", -1);
  to = Tcl_NewStringObj(ay_prefs.colorname, -1);
  Tcl_ObjSetVar2(interp, toa, ton, to, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton, "DisableFailedScripts", -1);
  to = Tcl_NewIntObj(ay_prefs.disablefailedscripts);
  Tcl_ObjSetVar2(interp, toa, ton, to, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_IncrRefCount(toa);Tcl_DecrRefCount(toa);
  Tcl_IncrRefCount(ton);Tcl_DecrRefCount(ton);

 return TCL_OK;
} /* ay_prefs_gettcmd */


/* Tcl -> C! */
int
ay_prefs_settcmd(ClientData clientData, Tcl_Interp *interp,
		 int argc, char *argv[])
{
 char *n1 = "ayprefs", *n2 = "ayprefse";
 char *cvtags = NULL, *tagname, *tagtype, **tmp;
 Tcl_HashEntry *entry = NULL;
 Tcl_Obj *to = NULL, *toa = NULL, *ton = NULL;
 double dtemp = 0.0;
 int setall = AY_TRUE, arglen = 0, i = 1, itemp = 0, ay_status = AY_OK, qf = 0;
 char *ucargs[3] = {0}, ucarg0[] = "undo", ucarg1[] = "clear";

 if(argc > 1)
   {
     setall = AY_FALSE;
   }

  do
  {
    toa = Tcl_NewStringObj(n1, -1);
    ton = Tcl_NewStringObj("", -1);

    if(!setall)
      arglen = strlen(argv[i]);

    if(setall || (argv[i][0] == 'A'))
      {
	if(setall || arglen == 9)
	  {
	    Tcl_SetStringObj(ton, "AllowWarp", 9);
	    to = Tcl_ObjGetVar2(interp, toa, ton,
				TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
	    Tcl_GetIntFromObj(interp, to, &ay_prefs.allow_warp);
	  }
	if(setall || arglen == 13)
	  {
	    Tcl_SetStringObj(ton, "AvoidPwlCurve", 13);
	    to = Tcl_ObjGetVar2(interp, toa, ton,
				TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
	    Tcl_GetIntFromObj(interp, to, &ay_prefs.glu_avoid_pwlcurve);
	  }
	if(setall || arglen == 11)
	  {
	    Tcl_SetStringObj(ton, "AALineWidth", 11);
	    to = Tcl_ObjGetVar2(interp, toa, ton,
				TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
	    Tcl_GetDoubleFromObj(interp, to, &ay_prefs.aalinewidth);
	  }
	if(setall || arglen == 14)
	  {
	    Tcl_SetStringObj(ton, "AASelLineWidth", 14);
	    to = Tcl_ObjGetVar2(interp, toa, ton,
				TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
	    Tcl_GetDoubleFromObj(interp, to, &ay_prefs.aasellinewidth);
	  }
	if(setall || arglen == 7)
	  {
	    Tcl_SetStringObj(ton, "AAFudge", 7);
	    to = Tcl_ObjGetVar2(interp, toa, ton,
				TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
	    Tcl_GetDoubleFromObj(interp, to, &ay_prefs.aafudge);
	  }
      } /* A... */

    if(setall || (argv[i][0] == 'B'))
      {
	ay_prefs_setcolor(interp, toa, "Background", &(ay_prefs.bgr),
			    &(ay_prefs.bgg), &(ay_prefs.bgb));
      } /* B... */

    if(setall || (argv[i][0] == 'C'))
      {
	Tcl_SetStringObj(ton, "CompleteNotify", 14);
	to = Tcl_ObjGetVar2(interp, toa, ton,
			    TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
	Tcl_GetIntFromObj(interp, to, &ay_prefs.completenotify);

	Tcl_SetStringObj(ton, "CreateAtMark", 12);
	to = Tcl_ObjGetVar2(interp, toa, ton,
			    TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
	Tcl_GetIntFromObj(interp, to, &ay_prefs.createatmark);

	Tcl_SetStringObj(ton, "ConvResetDisplay", 16);
	to = Tcl_ObjGetVar2(interp, toa, ton,
			    TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
	Tcl_GetIntFromObj(interp, to, &ay_prefs.conv_reset_display);

	Tcl_SetStringObj(ton, "CheckLights", 11);
	to = Tcl_ObjGetVar2(interp, toa, ton,
			    TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
	Tcl_GetIntFromObj(interp, to, &ay_prefs.checklights);

	Tcl_SetStringObj(ton, "ConvertTags", 11);
	if((ay_status = ay_tcmd_getstring(interp, toa, ton, &cvtags)))
	  goto cleanup;

	if(ay_prefs.converttags)
	  free(ay_prefs.converttags);
	ay_prefs.converttags = NULL;
	ay_prefs.converttagslen = 0;

	if(cvtags)
	  {
	    tagname = strtok(cvtags, ",");
	    while(tagname)
	      {
		if((entry = Tcl_FindHashEntry(&ay_tagtypesht, tagname)))
		  {
		    tagtype = (char*)Tcl_GetHashValue(entry);

		    if(!(tmp = realloc(ay_prefs.converttags,
				 (ay_prefs.converttagslen+1)*sizeof(char*))))
		      goto cleanup;
		    ay_prefs.converttags = tmp;
		    ay_prefs.converttags[ay_prefs.converttagslen] = tagtype;
		    ay_prefs.converttagslen++;

		    if(tagtype == ay_cp_tagtype)
		      ay_prefs.conv_keep_caps = AY_TRUE;

		    if(tagtype == ay_bp_tagtype)
		      ay_prefs.conv_keep_bevels = AY_TRUE;
		  }
		tagname = strtok(NULL, ",");
	      }
	    free(cvtags);
	  }


	Tcl_SetStringObj(ton, "CullFaces", 9);
	to = Tcl_ObjGetVar2(interp, toa, ton,
			    TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
	Tcl_GetIntFromObj(interp, to, &ay_prefs.cullfaces);
      } /* C... */

    if(setall || (argv[i][0] == 'D'))
      {
	Tcl_SetStringObj(ton, "DefaultMat", 10);
	to = Tcl_ObjGetVar2(interp, toa, ton,
			    TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
	Tcl_GetIntFromObj(interp, to, &ay_prefs.defaultmat);
	Tcl_SetStringObj(ton, "DisableFailedScripts", 20);
	to = Tcl_ObjGetVar2(interp, toa, ton,
			    TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
	Tcl_GetIntFromObj(interp, to, &ay_prefs.disablefailedscripts);
      } /* D... */

    if(setall || (argv[i][0] == 'E'))
      {
	Tcl_SetStringObj(ton, "ErrorLevel", 10);
	to = Tcl_ObjGetVar2(interp, toa, ton,
			    TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
	Tcl_GetIntFromObj(interp, to, &itemp);
	ay_prefs.errorlevel = itemp;

	Tcl_SetStringObj(ton, "EditSnaps", 9);
	to = Tcl_ObjGetVar2(interp, toa, ton,
			    TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
	Tcl_GetIntFromObj(interp, to, &ay_prefs.edit_snaps_to_grid);

	Tcl_SetStringObj(ton, "ExcludeHidden", 13);
	to = Tcl_ObjGetVar2(interp, toa, ton,
			    TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
	Tcl_GetIntFromObj(interp, to, &ay_prefs.excludehidden);
      } /* E... */

    if(setall || (argv[i][0] == 'G'))
      {
	Tcl_SetStringObj(ton, "GlobalMark", 10);
	to = Tcl_ObjGetVar2(interp, toa, ton,
			    TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
	Tcl_GetIntFromObj(interp, to, &ay_prefs.globalmark);

	if(setall || (arglen == 4))
	  ay_prefs_setcolor(interp, toa, "Grid", &(ay_prefs.grr),
			    &(ay_prefs.grg), &(ay_prefs.grb));

      } /* G... */

    if(setall || (argv[i][0] == 'H'))
      {
	Tcl_SetStringObj(ton, "HandleSize", 10);
	to = Tcl_ObjGetVar2(interp, toa, ton,
			    TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
	Tcl_GetDoubleFromObj(interp, to, &ay_prefs.handle_size);
      } /* H... */

    if(setall || (argv[i][0] == 'K'))
      {

      } /* K... */

    if(setall || (argv[i][0] == 'L'))
      {
	Tcl_SetStringObj(ton, "LazyNotify", 10);
	to = Tcl_ObjGetVar2(interp, toa, ton,
			    TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
	Tcl_GetIntFromObj(interp, to, &ay_prefs.lazynotify);

	Tcl_SetStringObj(ton, "LineWidth", 9);
	to = Tcl_ObjGetVar2(interp, toa, ton,
			    TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
	Tcl_GetDoubleFromObj(interp, to, &ay_prefs.linewidth);

	Tcl_SetStringObj(ton, "Logging", 7);
	to = Tcl_ObjGetVar2(interp, toa, ton,
			    TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
	Tcl_GetIntFromObj(interp, to, &ay_prefs.writelog);

	Tcl_SetStringObj(ton, "LogFile", 7);
	if((ay_status = ay_tcmd_getstring(interp, toa, ton,
					  &(ay_prefs.logfile))))
	  goto cleanup;

	Tcl_SetStringObj(ton, "ListTypes", 9);
	to = Tcl_ObjGetVar2(interp, toa, ton,
			    TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
	Tcl_GetIntFromObj(interp, to, &ay_prefs.list_types);

	if(setall || arglen == 5)
	  ay_prefs_setcolor(interp, toa, "Light", &(ay_prefs.lir),
			    &(ay_prefs.lig), &(ay_prefs.lib));
      } /* L... */

    if(setall || (argv[i][0] == 'M'))
      {
	Tcl_SetStringObj(ton, "MarkHidden", 10);
	to = Tcl_ObjGetVar2(interp, toa, ton,
			    TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
	Tcl_GetIntFromObj(interp, to, &ay_prefs.mark_hidden);
      } /* M... */

    if(setall || (argv[i][0] == 'N'))
      {
	Tcl_SetStringObj(ton, "NormalizeTrafos", 15);
	to = Tcl_ObjGetVar2(interp, toa, ton,
			    TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
	Tcl_GetIntFromObj(interp, to, &ay_prefs.normalizetrafos);

	Tcl_SetStringObj(ton, "NormalizeDigits", 15);
	to = Tcl_ObjGetVar2(interp, toa, ton,
			    TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
	Tcl_GetIntFromObj(interp, to, &ay_prefs.normalizedigits);

	Tcl_SetStringObj(ton, "NPDisplayMode", 13);
	to = Tcl_ObjGetVar2(interp, toa, ton,
			    TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
	Tcl_GetIntFromObj(interp, to, &ay_prefs.np_display_mode);

	Tcl_SetStringObj(ton, "NCDisplayMode", 13);
	to = Tcl_ObjGetVar2(interp, toa, ton,
			    TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
	Tcl_GetIntFromObj(interp, to, &ay_prefs.nc_display_mode);

	Tcl_SetStringObj(ton, "NPDisplayModeA", 14);
	to = Tcl_ObjGetVar2(interp, toa, ton,
			    TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
	Tcl_GetIntFromObj(interp, to, &ay_prefs.np_display_mode_a);

	Tcl_SetStringObj(ton, "NCDisplayModeA", 14);
	to = Tcl_ObjGetVar2(interp, toa, ton,
			    TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
	Tcl_GetIntFromObj(interp, to, &ay_prefs.nc_display_mode_a);
      } /* N... */

    if(setall || (argv[i][0] == 'O'))
      {
	ay_prefs_setcolor(interp, toa, "Object", &(ay_prefs.obr),
			  &(ay_prefs.obg), &(ay_prefs.obb));
      } /* O... */

    if(setall || (argv[i][0] == 'P'))
      {
	if(setall || arglen == 11)
	  {
	    Tcl_SetStringObj(ton, "PickEpsilon", 11);
	    to = Tcl_ObjGetVar2(interp, toa, ton,
				TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
	    Tcl_GetDoubleFromObj(interp, to, &dtemp);

	    if(dtemp > 0.0)
	      {
		ay_prefs.pick_epsilon = dtemp;
	      }
	    else
	      {
		ay_error(AY_ERROR, argv[0],
		    "Illegal value for PickEpsilon (should be >0.0), reset.");
		if(ay_prefs.pick_epsilon > 0.0)
		  {
		    to = Tcl_NewDoubleObj(ay_prefs.pick_epsilon);
		  }
		else
		  {
		    ay_prefs.pick_epsilon = 0.2;
		    to = Tcl_NewDoubleObj(0.2);
		  }
		Tcl_ObjSetVar2(interp, toa, ton, to,
			       TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
		Tcl_SetStringObj(toa, n2, -1);
		Tcl_ObjSetVar2(interp, toa, ton, to,
			       TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
		Tcl_SetStringObj(toa, n1, -1);
	      }
	  }

	if(setall || arglen == 13)
	  {
	    Tcl_SetStringObj(ton, "PickTolerance", 13);
	    to = Tcl_ObjGetVar2(interp, toa, ton,
				TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
	    Tcl_GetDoubleFromObj(interp, to, &ay_prefs.object_pick_epsilon);
	  }

	Tcl_SetStringObj(ton, "PolyOffset0", 11);
	to = Tcl_ObjGetVar2(interp, toa, ton,
			    TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
	Tcl_GetDoubleFromObj(interp, to, &ay_prefs.polyoffset0);

	Tcl_SetStringObj(ton, "PolyOffset1", 11);
	to = Tcl_ObjGetVar2(interp, toa, ton,
			    TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
	Tcl_GetDoubleFromObj(interp, to, &ay_prefs.polyoffset1);

	Tcl_SetStringObj(ton, "PVTexCoordName", 14);
	if((ay_status = ay_tcmd_getstring(interp, toa, ton,
					  &(ay_prefs.texcoordname))))
	  goto cleanup;

	Tcl_SetStringObj(ton, "PVNormalName", 12);
	if((ay_status = ay_tcmd_getstring(interp, toa, ton,
					  &(ay_prefs.normalname))))
	  goto cleanup;

	Tcl_SetStringObj(ton, "PVTangentName", 13);
	if((ay_status = ay_tcmd_getstring(interp, toa, ton,
					  &(ay_prefs.tangentname))))
	  goto cleanup;

	Tcl_SetStringObj(ton, "PVColorName", 11);
	if((ay_status = ay_tcmd_getstring(interp, toa, ton,
					  &(ay_prefs.colorname))))
	  goto cleanup;

	Tcl_SetStringObj(ton, "PVOpacityName", 13);
	if((ay_status = ay_tcmd_getstring(interp, toa, ton,
					  &(ay_prefs.opacityname))))
	  goto cleanup;


	Tcl_SetStringObj(ton, "PPRender", 8);
	if((ay_status = ay_tcmd_getstring(interp, toa, ton,
					  &(ay_prefs.pprender))))
	  goto cleanup;
      } /* P... */


    if(setall || (argv[i][0] == 'Q'))
      {

      } /* Q... */

    if(setall || (argv[i][0] == 'R'))
      {
	Tcl_SetStringObj(ton, "RationalPoints", 14);
	to = Tcl_ObjGetVar2(interp, toa, ton,
			    TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
	Tcl_GetIntFromObj(interp, to, &ay_prefs.rationalpoints);

	Tcl_SetStringObj(ton, "ResInstances", 12);
	to = Tcl_ObjGetVar2(interp, toa, ton,
			    TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
	Tcl_GetIntFromObj(interp, to, &ay_prefs.resolveinstances);

	Tcl_SetStringObj(ton, "RIStandard", 10);
	to = Tcl_ObjGetVar2(interp, toa, ton,
			    TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
	Tcl_GetIntFromObj(interp, to, &ay_prefs.ristandard);
      } /* R... */


    if(setall || (argv[i][0] == 'S'))
      {
	Tcl_SetStringObj(ton, "SelLineWidth", 12);
	to = Tcl_ObjGetVar2(interp, toa, ton,
			    TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
	Tcl_GetDoubleFromObj(interp, to, &ay_prefs.sellinewidth);

	Tcl_SetStringObj(ton, "SDMode", 6);
	to = Tcl_ObjGetVar2(interp, toa, ton,
			    TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
	Tcl_GetIntFromObj(interp, to, &ay_prefs.sdmode);

	Tcl_SetStringObj(ton, "SMethod", 7);
	to = Tcl_ObjGetVar2(interp, toa, ton,
			    TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
	Tcl_GetIntFromObj(interp, to, &ay_prefs.smethod);

	ay_prefs.smethod++;

	Tcl_SetStringObj(ton, "SParamU", 7);
	to = Tcl_ObjGetVar2(interp, toa, ton,
			    TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
	Tcl_GetDoubleFromObj(interp, to, &ay_prefs.sparamu);

	Tcl_SetStringObj(ton, "SParamV", 7);
	to = Tcl_ObjGetVar2(interp, toa, ton,
			    TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
	Tcl_GetDoubleFromObj(interp, to, &ay_prefs.sparamv);

	Tcl_SetStringObj(ton, "SingleWindow", 12);
	to = Tcl_ObjGetVar2(interp, toa, ton,
			    TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
	Tcl_GetIntFromObj(interp, to, &ay_prefs.single_window);

	Tcl_SetStringObj(ton, "ShadowMaps", 10);
	to = Tcl_ObjGetVar2(interp, toa, ton,
			    TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
	Tcl_GetIntFromObj(interp, to, &ay_prefs.use_sm);

	ay_prefs_setcolor(interp, toa, "SelXOR", &(ay_prefs.sxr),
			  &(ay_prefs.sxg), &(ay_prefs.sxb));

	ay_prefs_setcolor(interp, toa, "Shade", &(ay_prefs.shr),
			  &(ay_prefs.shg), &(ay_prefs.shb));

	ay_prefs_setcolor(interp, toa, "Selection", &(ay_prefs.ser),
			  &(ay_prefs.seg), &(ay_prefs.seb));

	Tcl_SetStringObj(ton, "Snap3D", 6);
	to = Tcl_ObjGetVar2(interp, toa, ton,
			    TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
	Tcl_GetIntFromObj(interp, to, &ay_prefs.snap3d);
      } /* S... */

    if(setall || (argv[i][0] == 'T'))
      {
	if(setall || arglen == 3)
	  {
	    ay_prefs_setcolor(interp, toa, "Tag", &(ay_prefs.tpr),
			      &(ay_prefs.tpg), &(ay_prefs.tpb));
	  }

	Tcl_SetStringObj(ton, "Tolerance", 9);
	to = Tcl_ObjGetVar2(interp, toa, ton,
			    TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
	Tcl_GetDoubleFromObj(interp, to, &ay_prefs.glu_sampling_tolerance);

	qf = ay_stess_GetQF(ay_prefs.glu_sampling_tolerance);

	if(qf < 1)
	  {
	    qf = 1;
	  }
	ay_prefs.stess_qf = qf;

	Tcl_SetStringObj(ton, "ToleranceA", 10);
	to = Tcl_ObjGetVar2(interp, toa, ton,
			    TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
	Tcl_GetDoubleFromObj(interp, to, &ay_prefs.glu_sampling_tolerance_a);
      } /* T... */

    if(setall || (argv[i][0] == 'U'))
      {
	Tcl_SetStringObj(ton, "UndoLevels", 10);
	to = Tcl_ObjGetVar2(interp, toa, ton,
			    TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
	Tcl_GetIntFromObj(interp, to, &itemp);

	if(itemp != ay_prefs.undo_levels)
	  {
	    if(itemp < 0)
	      itemp = 0;

	    to = Tcl_NewIntObj(itemp);
	    Tcl_ObjSetVar2(interp, toa, ton, to,
			   TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
	    Tcl_SetStringObj(toa, n2, -1);
	    Tcl_ObjSetVar2(interp, toa, ton, to,
			   TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
	    Tcl_SetStringObj(toa, n1, -1);

	    /*      ay_status = ay_undo_clear();*/
	    ucargs[0] = ucarg0;
	    ucargs[1] = ucarg1;
	    ay_undo_undotcmd(clientData, interp, 2, ucargs);

	    ay_status = ay_undo_init(itemp);
	    if(ay_status)
	      {
		ay_error(ay_status, argv[0], NULL);
		return TCL_OK;
	      }

	    ay_prefs.undo_levels = itemp;
	  } /* if */

	Tcl_SetStringObj(ton, "UseMatColor", 11);
	to = Tcl_ObjGetVar2(interp, toa,
			    ton, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
	Tcl_GetIntFromObj(interp, to, &ay_prefs.use_materialcolor);
      } /* U... */

    if(setall || (argv[i][0] == 'W'))
      {
	Tcl_SetStringObj(ton, "WriteIdent", 10);
	to = Tcl_ObjGetVar2(interp, toa, ton,
			    TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
	Tcl_GetIntFromObj(interp, to, &ay_prefs.writeident);

	Tcl_SetStringObj(ton, "WarnUnknownTag", 14);
	to = Tcl_ObjGetVar2(interp, toa, ton,
			    TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
	Tcl_GetIntFromObj(interp, to, &ay_prefs.wutag);
      } /* W... */

    i++;
  }
  while(i <= argc);

cleanup:

  Tcl_IncrRefCount(toa);Tcl_DecrRefCount(toa);
  Tcl_IncrRefCount(ton);Tcl_DecrRefCount(ton);

  if(ay_status)
    {
      ay_error(ay_status, argv[0], NULL);
    }

 return TCL_OK;
} /* ay_prefs_settcmd */


/** ay_prefs_setcolor:
 * Helper to get a color value from Tcl.
 *
 * \param interp Tcl interpreter to use
 * \param arr string object with array name (ayprefs)
 * \param var base color variable name in arr (e.g. Light)
 * \param r where to store the red component
 * \param g where to store the green component
 * \param b where to store the blue component
 */
void
ay_prefs_setcolor(Tcl_Interp *interp, Tcl_Obj *arr, char *var,
		  double *r, double *g, double *b)
{
 Tcl_Obj *to, *ton;
 int i, itemp[3], l;
 char *apps[3] = {"_R", "_G", "_B"};

  l = strlen(var);
  ton = Tcl_NewStringObj(var, l);
  for(i = 0; i < 3; i++)
    {
      Tcl_AppendToObj(ton, apps[i], 2);
      to = Tcl_ObjGetVar2(interp, arr, ton,
			  TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
      Tcl_GetIntFromObj(interp, to, &(itemp[i]));
      if(itemp[i] < 0)
	itemp[i] = 0;
      if(itemp[i] > 255)
	itemp[i] = 255;
      Tcl_SetObjLength(ton, l);
    }

  *r = itemp[0]/255.0;
  *g = itemp[1]/255.0;
  *b = itemp[2]/255.0;

 return;
} /* ay_prefs_setcolor */
