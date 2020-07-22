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

void ay_prefs_setcolor(Tcl_Interp *interp, char *arr, char *var,
		       double *r, double *g, double *b);

/* C -> Tcl! */
int
ay_prefs_gettcmd(ClientData clientData, Tcl_Interp *interp,
		 int argc, char *argv[])
{
 char *arr = "ayprefs";

  Tcl_SetVar2Ex(interp, arr, "PickEpsilon",
		Tcl_NewDoubleObj(ay_prefs.pick_epsilon),
		TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetVar2Ex(interp, arr, "HandleSize",
		Tcl_NewDoubleObj(ay_prefs.handle_size),
		TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetVar2Ex(interp, arr, "LazyNotify",
		Tcl_NewIntObj(ay_prefs.lazynotify),
		TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetVar2Ex(interp, arr, "CompleteNotify",
		Tcl_NewIntObj(ay_prefs.completenotify),
		TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetVar2Ex(interp, arr, "EditSnaps",
		Tcl_NewIntObj(ay_prefs.edit_snaps_to_grid),
		TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetVar2Ex(interp, arr, "RationalPoints",
		Tcl_NewIntObj(ay_prefs.rationalpoints),
		TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetVar2Ex(interp, arr, "GlobalMark",
		Tcl_NewIntObj(ay_prefs.globalmark),
		TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetVar2Ex(interp, arr, "CreateAtMark",
		Tcl_NewIntObj(ay_prefs.createatmark),
		TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetVar2Ex(interp, arr, "NormalizeTrafos",
		Tcl_NewIntObj(ay_prefs.normalizetrafos),
		TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetVar2Ex(interp, arr, "NormalizeMark",
		Tcl_NewIntObj(ay_prefs.normalizemark),
		TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetVar2Ex(interp, arr, "NormalizeDigits",
		Tcl_NewIntObj(ay_prefs.normalizedigits),
		TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetVar2Ex(interp, arr, "UndoLevels",
		Tcl_NewIntObj(ay_prefs.undo_levels),
		TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetVar2Ex(interp, arr, "Snap3D",
		Tcl_NewIntObj(ay_prefs.snap3d),
		TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetVar2Ex(interp, arr, "AllowWarp",
		Tcl_NewIntObj(ay_prefs.allow_warp),
		TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetVar2Ex(interp, arr, "ConvResetDisplay",
		Tcl_NewIntObj(ay_prefs.conv_reset_display),
		TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetVar2Ex(interp, arr, "Tolerance",
		Tcl_NewDoubleObj(ay_prefs.glu_sampling_tolerance),
		TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetVar2Ex(interp, arr, "ToleranceA",
		Tcl_NewDoubleObj(ay_prefs.glu_sampling_tolerance_a),
		TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetVar2Ex(interp, arr, "NPDisplayMode",
		Tcl_NewIntObj(ay_prefs.np_display_mode),
		TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetVar2Ex(interp, arr, "NCDisplayMode",
		Tcl_NewIntObj(ay_prefs.nc_display_mode),
		TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetVar2Ex(interp, arr, "NPDisplayModeA",
		Tcl_NewIntObj(ay_prefs.np_display_mode_a),
		TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetVar2Ex(interp, arr, "NCDisplayModeA",
		Tcl_NewIntObj(ay_prefs.nc_display_mode_a),
		TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetVar2Ex(interp, arr, "AvoidPwlCurve",
		Tcl_NewIntObj(ay_prefs.glu_avoid_pwlcurve),
		TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetVar2Ex(interp, arr, "UseMatColor",
		Tcl_NewIntObj(ay_prefs.use_materialcolor),
		TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetVar2Ex(interp, arr, "Background_R",
		Tcl_NewIntObj((int)(ay_prefs.bgr*255)),
		TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetVar2Ex(interp, arr, "Background_G",
		Tcl_NewIntObj((int)(ay_prefs.bgg*255)),
		TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetVar2Ex(interp, arr, "Background_B",
		Tcl_NewIntObj((int)(ay_prefs.bgb*255)),
		TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetVar2Ex(interp, arr, "Object_R",
		Tcl_NewIntObj((int)(ay_prefs.obr*255)),
		TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetVar2Ex(interp, arr, "Object_G",
		Tcl_NewIntObj((int)(ay_prefs.obg*255)),
		TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetVar2Ex(interp, arr, "Object_B",
		Tcl_NewIntObj((int)(ay_prefs.obb*255)),
		TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetVar2Ex(interp, arr, "Selection_R",
		Tcl_NewIntObj((int)(ay_prefs.ser*255)),
		TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetVar2Ex(interp, arr, "Selection_G",
		Tcl_NewIntObj((int)(ay_prefs.seg*255)),
		TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetVar2Ex(interp, arr, "Selection_B",
		Tcl_NewIntObj((int)(ay_prefs.seb*255)),
		TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetVar2Ex(interp, arr, "Grid_R",
		Tcl_NewIntObj((int)(ay_prefs.grr*255)),
		TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetVar2Ex(interp, arr, "Grid_G",
		Tcl_NewIntObj((int)(ay_prefs.grg*255)),
		TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetVar2Ex(interp, arr, "Grid_B",
		Tcl_NewIntObj((int)(ay_prefs.grb*255)),
		TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetVar2Ex(interp, arr, "Tag_R",
		Tcl_NewIntObj((int)(ay_prefs.tpr*255)),
		TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetVar2Ex(interp, arr, "Tag_G",
		Tcl_NewIntObj((int)(ay_prefs.tpg*255)),
		TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetVar2Ex(interp, arr, "Tag_B",
		Tcl_NewIntObj((int)(ay_prefs.tpb*255)),
		TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetVar2Ex(interp, arr, "Shade_R",
		Tcl_NewIntObj((int)(ay_prefs.shr*255)),
		TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetVar2Ex(interp, arr, "Shade_G",
		Tcl_NewIntObj((int)(ay_prefs.shg*255)),
		TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetVar2Ex(interp, arr, "Shade_B",
		Tcl_NewIntObj((int)(ay_prefs.shb*255)),
		TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetVar2Ex(interp, arr, "Light_R",
		Tcl_NewIntObj((int)(ay_prefs.lir*255)),
		TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetVar2Ex(interp, arr, "Light_G",
		Tcl_NewIntObj((int)(ay_prefs.lig*255)),
		TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetVar2Ex(interp, arr, "Light_B",
		Tcl_NewIntObj((int)(ay_prefs.lib*255)),
		TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetVar2Ex(interp, arr, "SelXOR_R",
		Tcl_NewIntObj((int)(ay_prefs.sxr*255)),
		TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetVar2Ex(interp, arr, "SelXOR_G",
		Tcl_NewIntObj((int)(ay_prefs.sxg*255)),
		TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetVar2Ex(interp, arr, "SelXOR_B",
		Tcl_NewIntObj((int)(ay_prefs.sxb*255)),
		TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetVar2Ex(interp, arr, "CullFaces",
		Tcl_NewIntObj(ay_prefs.cullfaces),
		TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetVar2Ex(interp, arr, "ResInstances",
		Tcl_NewIntObj(ay_prefs.resolveinstances),
		TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetVar2Ex(interp, arr, "CheckLights",
		Tcl_NewIntObj(ay_prefs.checklights),
		TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetVar2Ex(interp, arr, "DefaultMat",
		Tcl_NewIntObj(ay_prefs.defaultmat),
		TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetVar2Ex(interp, arr, "RIStandard",
		Tcl_NewIntObj(ay_prefs.ristandard),
		TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetVar2Ex(interp, arr, "WriteIdent",
		Tcl_NewIntObj(ay_prefs.writeident),
		TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetVar2Ex(interp, arr, "ShadowMaps",
		Tcl_NewIntObj(ay_prefs.use_sm),
		TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetVar2Ex(interp, arr, "ExcludeHidden",
		Tcl_NewIntObj(ay_prefs.excludehidden),
		TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetVar2Ex(interp, arr, "SingleWindow",
		Tcl_NewIntObj(ay_prefs.single_window),
		TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetVar2Ex(interp, arr, "ListTypes",
		Tcl_NewIntObj(ay_prefs.list_types),
		TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetVar2Ex(interp, arr, "MarkHidden",
		Tcl_NewIntObj(ay_prefs.mark_hidden),
		TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetVar2Ex(interp, arr, "Logging",
		Tcl_NewIntObj(ay_prefs.writelog),
		TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetVar2Ex(interp, arr, "LogFile",
		Tcl_NewStringObj(ay_prefs.logfile, -1),
		TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetVar2Ex(interp, arr, "ErrorLevel",
		Tcl_NewIntObj(ay_prefs.errorlevel),
		TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetVar2Ex(interp, arr, "PPRender",
		Tcl_NewIntObj((int)ay_prefs.pprender),
		TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetVar2Ex(interp, arr, "SMethod",
		Tcl_NewIntObj(ay_prefs.smethod),
		TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetVar2Ex(interp, arr, "SParamU",
		Tcl_NewDoubleObj(ay_prefs.sparamu),
		TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetVar2Ex(interp, arr, "SParamV",
		Tcl_NewDoubleObj(ay_prefs.sparamv),
		TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetVar2Ex(interp, arr, "WarnUnknownTag",
		Tcl_NewIntObj(ay_prefs.wutag),
		TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetVar2Ex(interp, arr, "WarnPnts",
		Tcl_NewIntObj(ay_prefs.warnpnts),
		TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetVar2Ex(interp, arr, "LineWidth",
		Tcl_NewDoubleObj(ay_prefs.linewidth),
		TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetVar2Ex(interp, arr, "SelLineWidth",
		Tcl_NewDoubleObj(ay_prefs.sellinewidth),
		TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetVar2Ex(interp, arr, "AALineWidth",
		Tcl_NewDoubleObj(ay_prefs.aalinewidth),
		TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetVar2Ex(interp, arr, "AASelLineWidth",
		Tcl_NewDoubleObj(ay_prefs.aasellinewidth),
		TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetVar2Ex(interp, arr, "AAFudge",
		Tcl_NewDoubleObj(ay_prefs.aafudge),
		TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetVar2Ex(interp, arr, "SDMode",
		Tcl_NewIntObj(ay_prefs.sdmode),
		TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetVar2Ex(interp, arr, "PolyOffset0",
		Tcl_NewDoubleObj(ay_prefs.polyoffset0),
		TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetVar2Ex(interp, arr, "PolyOffset1",
		Tcl_NewDoubleObj(ay_prefs.polyoffset1),
		TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetVar2Ex(interp, arr, "PVTexCoordName",
		Tcl_NewStringObj(ay_prefs.texcoordname, -1),
		TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetVar2Ex(interp, arr, "PVNormalName",
		Tcl_NewStringObj(ay_prefs.normalname, -1),
		TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetVar2Ex(interp, arr, "PVTangentName",
		Tcl_NewStringObj(ay_prefs.tangentname, -1),
		TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetVar2Ex(interp, arr, "PVColorName",
		Tcl_NewStringObj(ay_prefs.colorname, -1),
		TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetVar2Ex(interp, arr, "DisableFailedScripts",
		Tcl_NewIntObj(ay_prefs.disablefailedscripts),
		TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

 return TCL_OK;
} /* ay_prefs_gettcmd */


/* Tcl -> C! */
int
ay_prefs_settcmd(ClientData clientData, Tcl_Interp *interp,
		 int argc, char *argv[])
{
 char *arr = "ayprefs", *arre = "ayprefse";
 char *cvtags = NULL, *tagname;
 unsigned int *tmp, tagtype;
 Tcl_HashEntry *entry = NULL;
 Tcl_Obj *to = NULL;
 double dtemp = 0.0;
 size_t arglen = 0;
 int setall = AY_TRUE, i = 1, itemp = 0, ay_status = AY_OK, qf = 0;
 char *ucargs[3] = {0}, ucarg0[] = "undo", ucarg1[] = "clear";
 char colbg[] = "Background_*";
 char colgr[] = "Grid_*";
 char colli[] = "Light_*";
 char colob[] = "Object_*";
 char colsx[] = "SelXOR_*";
 char colsh[] = "Shade_*";
 char colse[] = "Selection_*";
 char colta[] = "Tag_*";

 if(argc > 1)
   {
     setall = AY_FALSE;
   }

  do
  {
    if(!setall)
      arglen = strlen(argv[i]);

    if(setall || (argv[i][0] == 'A'))
      {
	if(setall || arglen == 9)
	  {
	    to = Tcl_GetVar2Ex(interp, arr, "AllowWarp",
			       TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
	    Tcl_GetIntFromObj(interp, to, &(ay_prefs.allow_warp));
	  }
	if(setall || arglen == 13)
	  {
	    to = Tcl_GetVar2Ex(interp, arr, "AvoidPwlCurve",
			       TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
	    Tcl_GetIntFromObj(interp, to, &(ay_prefs.glu_avoid_pwlcurve));
	  }
	if(setall || arglen == 11)
	  {
	    to = Tcl_GetVar2Ex(interp, arr, "AALineWidth",
			       TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
	    Tcl_GetDoubleFromObj(interp, to, &(ay_prefs.aalinewidth));
	  }
	if(setall || arglen == 14)
	  {
	    to = Tcl_GetVar2Ex(interp, arr, "AASelLineWidth",
			       TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
	    Tcl_GetDoubleFromObj(interp, to, &(ay_prefs.aasellinewidth));
	  }
	if(setall || arglen == 7)
	  {
	    to = Tcl_GetVar2Ex(interp, arr, "AAFudge",
			       TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
	    Tcl_GetDoubleFromObj(interp, to, &(ay_prefs.aafudge));
	  }
      } /* A... */

    if(setall || (argv[i][0] == 'B'))
      {
	ay_prefs_setcolor(interp, arr, colbg, &(ay_prefs.bgr),
			    &(ay_prefs.bgg), &(ay_prefs.bgb));
      } /* B... */

    if(setall || (argv[i][0] == 'C'))
      {
	to = Tcl_GetVar2Ex(interp, arr, "CompleteNotify",
			   TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
	Tcl_GetIntFromObj(interp, to, &(ay_prefs.completenotify));

	to = Tcl_GetVar2Ex(interp, arr, "CreateAtMark",
			   TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
	Tcl_GetIntFromObj(interp, to, &(ay_prefs.createatmark));

	to = Tcl_GetVar2Ex(interp, arr, "ConvResetDisplay",
			   TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
	Tcl_GetIntFromObj(interp, to, &(ay_prefs.conv_reset_display));

	to = Tcl_GetVar2Ex(interp, arr, "CheckLights",
			   TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
	Tcl_GetIntFromObj(interp, to, &(ay_prefs.checklights));

	if((ay_status = ay_tcmd_getstring(interp, arr, "ConvertTags", &cvtags)))
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
		    tagtype = *((unsigned int*)Tcl_GetHashValue(entry));

		    if(!(tmp = realloc(ay_prefs.converttags,
			 (ay_prefs.converttagslen+1)*sizeof(unsigned int))))
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

	to = Tcl_GetVar2Ex(interp, arr, "CullFaces",
			   TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
	Tcl_GetIntFromObj(interp, to, &(ay_prefs.cullfaces));
      } /* C... */

    if(setall || (argv[i][0] == 'D'))
      {
	to = Tcl_GetVar2Ex(interp, arr, "DefaultMat",
			   TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
	Tcl_GetIntFromObj(interp, to, &(ay_prefs.defaultmat));

	to = Tcl_GetVar2Ex(interp, arr, "DisableFailedScripts",
			   TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
	Tcl_GetIntFromObj(interp, to, &(ay_prefs.disablefailedscripts));
      } /* D... */

    if(setall || (argv[i][0] == 'E'))
      {
	to = Tcl_GetVar2Ex(interp, arr, "ErrorLevel",
			   TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
	Tcl_GetIntFromObj(interp, to, &(itemp));
	ay_prefs.errorlevel = (char)itemp;

	to = Tcl_GetVar2Ex(interp, arr, "EditSnaps",
			   TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
	Tcl_GetIntFromObj(interp, to, &(ay_prefs.edit_snaps_to_grid));

	to = Tcl_GetVar2Ex(interp, arr, "ExcludeHidden",
			   TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
	Tcl_GetIntFromObj(interp, to, &(ay_prefs.excludehidden));
      } /* E... */

    if(setall || (argv[i][0] == 'G'))
      {
	to = Tcl_GetVar2Ex(interp, arr, "GlobalMark",
			   TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
	Tcl_GetIntFromObj(interp, to, &(ay_prefs.globalmark));

	if(setall || (arglen == 4))
	  ay_prefs_setcolor(interp, arr, colgr, &(ay_prefs.grr),
			    &(ay_prefs.grg), &(ay_prefs.grb));
      } /* G... */

    if(setall || (argv[i][0] == 'H'))
      {
	to = Tcl_GetVar2Ex(interp, arr, "HandleSize",
			   TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
	Tcl_GetDoubleFromObj(interp, to, &(ay_prefs.handle_size));
      } /* H... */

    if(setall || (argv[i][0] == 'K'))
      {

      } /* K... */

    if(setall || (argv[i][0] == 'L'))
      {
	to = Tcl_GetVar2Ex(interp, arr, "LazyNotify",
			   TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
	Tcl_GetIntFromObj(interp, to, &(ay_prefs.lazynotify));

	to = Tcl_GetVar2Ex(interp, arr, "LineWidth",
			   TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
	Tcl_GetDoubleFromObj(interp, to, &(ay_prefs.linewidth));

	to = Tcl_GetVar2Ex(interp, arr, "Logging",
			   TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
	Tcl_GetIntFromObj(interp, to, &(ay_prefs.writelog));

	if((ay_status = ay_tcmd_getstring(interp, arr, "LogFile",
					  &(ay_prefs.logfile))))
	  goto cleanup;

	to = Tcl_GetVar2Ex(interp, arr, "ListTypes",
			   TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
	Tcl_GetIntFromObj(interp, to, &(ay_prefs.list_types));

	if(setall || arglen == 5)
	  ay_prefs_setcolor(interp, arr, colli, &(ay_prefs.lir),
			    &(ay_prefs.lig), &(ay_prefs.lib));
      } /* L... */

    if(setall || (argv[i][0] == 'M'))
      {
	to = Tcl_GetVar2Ex(interp, arr, "MarkHidden",
			   TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
	Tcl_GetIntFromObj(interp, to, &(ay_prefs.mark_hidden));
      } /* M... */

    if(setall || (argv[i][0] == 'N'))
      {
	to = Tcl_GetVar2Ex(interp, arr, "NormalizeTrafos",
			   TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
	Tcl_GetIntFromObj(interp, to, &(ay_prefs.normalizetrafos));

	to = Tcl_GetVar2Ex(interp, arr, "NormalizeMark",
			   TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
	Tcl_GetIntFromObj(interp, to, &(ay_prefs.normalizemark));

	to = Tcl_GetVar2Ex(interp, arr, "NormalizeDigits",
			   TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
	Tcl_GetIntFromObj(interp, to, &(ay_prefs.normalizedigits));

	to = Tcl_GetVar2Ex(interp, arr, "NPDisplayMode",
			   TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
	Tcl_GetIntFromObj(interp, to, &(ay_prefs.np_display_mode));

	to = Tcl_GetVar2Ex(interp, arr, "NCDisplayMode",
			   TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
	Tcl_GetIntFromObj(interp, to, &(ay_prefs.nc_display_mode));

	to = Tcl_GetVar2Ex(interp, arr, "NPDisplayModeA",
			   TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
	Tcl_GetIntFromObj(interp, to, &(ay_prefs.np_display_mode_a));

	to = Tcl_GetVar2Ex(interp, arr, "NCDisplayModeA",
			   TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
	Tcl_GetIntFromObj(interp, to, &(ay_prefs.nc_display_mode_a));
      } /* N... */

    if(setall || (argv[i][0] == 'O'))
      {
	ay_prefs_setcolor(interp, arr, colob, &(ay_prefs.obr),
			  &(ay_prefs.obg), &(ay_prefs.obb));
      } /* O... */

    if(setall || (argv[i][0] == 'P'))
      {
	if(setall || arglen == 11)
	  {
	    to = Tcl_GetVar2Ex(interp, arr, "PickEpsilon",
			       TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
	    Tcl_GetDoubleFromObj(interp, to, &(dtemp));

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

		Tcl_SetVar2Ex(interp, arr, "PickEpsilon", to,
			      TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
		Tcl_SetVar2Ex(interp, arre, "PickEpsilon", to,
			      TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
	      }
	  }

	if(setall || arglen == 13)
	  {
	    to = Tcl_GetVar2Ex(interp, arr, "PickTolerance",
			       TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
	    Tcl_GetDoubleFromObj(interp, to, &(ay_prefs.object_pick_epsilon));
	  }

	to = Tcl_GetVar2Ex(interp, arr, "PolyOffset0",
			   TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
	Tcl_GetDoubleFromObj(interp, to, &(ay_prefs.polyoffset0));

	to = Tcl_GetVar2Ex(interp, arr, "PolyOffset1",
			   TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
	Tcl_GetDoubleFromObj(interp, to, &(ay_prefs.polyoffset1));

	if((ay_status = ay_tcmd_getstring(interp, arr, "PVTexCoordName",
					  &(ay_prefs.texcoordname))))
	  goto cleanup;

	if((ay_status = ay_tcmd_getstring(interp, arr, "PVNormalName",
					  &(ay_prefs.normalname))))
	  goto cleanup;

	if((ay_status = ay_tcmd_getstring(interp, arr, "PVTangentName",
					  &(ay_prefs.tangentname))))
	  goto cleanup;

	if((ay_status = ay_tcmd_getstring(interp, arr, "PVColorName",
					  &(ay_prefs.colorname))))
	  goto cleanup;

	if((ay_status = ay_tcmd_getstring(interp, arr, "PVOpacityName",
					  &(ay_prefs.opacityname))))
	  goto cleanup;

	if((ay_status = ay_tcmd_getstring(interp, arr, "PPRender",
					  &(ay_prefs.pprender))))
	  goto cleanup;
      } /* P... */

    if(setall || (argv[i][0] == 'Q'))
      {

      } /* Q... */

    if(setall || (argv[i][0] == 'R'))
      {
	to = Tcl_GetVar2Ex(interp, arr, "RationalPoints",
			   TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
	Tcl_GetIntFromObj(interp, to, &(ay_prefs.rationalpoints));

	to = Tcl_GetVar2Ex(interp, arr, "ResInstances",
			   TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
	Tcl_GetIntFromObj(interp, to, &(ay_prefs.resolveinstances));

	to = Tcl_GetVar2Ex(interp, arr, "RIStandard",
			   TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
	Tcl_GetIntFromObj(interp, to, &(ay_prefs.ristandard));
      } /* R... */

    if(setall || (argv[i][0] == 'S'))
      {
	to = Tcl_GetVar2Ex(interp, arr, "SelLineWidth",
			   TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
	Tcl_GetDoubleFromObj(interp, to, &(ay_prefs.sellinewidth));

	to = Tcl_GetVar2Ex(interp, arr, "SDMode",
			   TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
	Tcl_GetIntFromObj(interp, to, &(ay_prefs.sdmode));

	to = Tcl_GetVar2Ex(interp, arr, "SMethod",
			   TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
	Tcl_GetIntFromObj(interp, to, &(ay_prefs.smethod));

	ay_prefs.smethod++;

	to = Tcl_GetVar2Ex(interp, arr, "SParamU",
			   TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
	Tcl_GetDoubleFromObj(interp, to, &(ay_prefs.sparamu));

	to = Tcl_GetVar2Ex(interp, arr, "SParamV",
			   TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
	Tcl_GetDoubleFromObj(interp, to, &(ay_prefs.sparamv));

	to = Tcl_GetVar2Ex(interp, arr, "SingleWindow",
			   TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
	Tcl_GetIntFromObj(interp, to, &(ay_prefs.single_window));

	to = Tcl_GetVar2Ex(interp, arr, "ShadowMaps",
			   TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
	Tcl_GetIntFromObj(interp, to, &(ay_prefs.use_sm));

	ay_prefs_setcolor(interp, arr, colsx, &(ay_prefs.sxr),
			  &(ay_prefs.sxg), &(ay_prefs.sxb));

	ay_prefs_setcolor(interp, arr, colsh, &(ay_prefs.shr),
			  &(ay_prefs.shg), &(ay_prefs.shb));

	ay_prefs_setcolor(interp, arr, colse, &(ay_prefs.ser),
			  &(ay_prefs.seg), &(ay_prefs.seb));

	to = Tcl_GetVar2Ex(interp, arr, "Snap3D",
			   TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
	Tcl_GetIntFromObj(interp, to, &(ay_prefs.snap3d));
      } /* S... */

    if(setall || (argv[i][0] == 'T'))
      {
	if(setall || arglen == 3)
	  {
	    ay_prefs_setcolor(interp, arr, colta, &(ay_prefs.tpr),
			      &(ay_prefs.tpg), &(ay_prefs.tpb));
	  }

	to = Tcl_GetVar2Ex(interp, arr, "Tolerance",
			   TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
	Tcl_GetDoubleFromObj(interp, to, &(ay_prefs.glu_sampling_tolerance));

	qf = ay_stess_GetQF(ay_prefs.glu_sampling_tolerance);

	if(qf < 1)
	  {
	    qf = 1;
	  }
	ay_prefs.stess_qf = qf;

	to = Tcl_GetVar2Ex(interp, arr, "ToleranceA",
			   TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
	Tcl_GetDoubleFromObj(interp, to, &(ay_prefs.glu_sampling_tolerance_a));
      } /* T... */

    if(setall || (argv[i][0] == 'U'))
      {
	to = Tcl_GetVar2Ex(interp, arr, "UndoLevels",
			   TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
	Tcl_GetIntFromObj(interp, to, &(itemp));

	if(itemp != ay_prefs.undo_levels)
	  {
	    if(itemp < 0)
	      itemp = 0;

	    Tcl_SetVar2Ex(interp, arr, "UndoLevels",
			  Tcl_NewIntObj(itemp),
			  TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

	    Tcl_SetVar2Ex(interp, arre, "UndoLevels",
			  Tcl_NewIntObj(itemp),
			  TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

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

	to = Tcl_GetVar2Ex(interp, arr, "UseMatColor",
			   TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
	Tcl_GetIntFromObj(interp, to, &(ay_prefs.use_materialcolor));
      } /* U... */

    if(setall || (argv[i][0] == 'W'))
      {
	to = Tcl_GetVar2Ex(interp, arr, "WriteIdent",
			   TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
	Tcl_GetIntFromObj(interp, to, &(ay_prefs.writeident));

	to = Tcl_GetVar2Ex(interp, arr, "WarnUnknownTag",
			   TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
	Tcl_GetIntFromObj(interp, to, &(ay_prefs.wutag));

	to = Tcl_GetVar2Ex(interp, arr, "WarnPnts",
			   TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
	Tcl_GetIntFromObj(interp, to, &(ay_prefs.warnpnts));
      } /* W... */

    i++;
  }
  while(i <= argc);

cleanup:

  if(ay_status)
    {
      ay_error(ay_status, argv[0], NULL);
    }

 return TCL_OK;
} /* ay_prefs_settcmd */


/** ay_prefs_setcolor:
 * Helper to get a color value from Tcl.
 *
 * \param[in] interp Tcl interpreter to use
 * \param[in] arr string with array name (e.g. "ayprefs")
 * \param[in,out] var base color variable name in arr (e.g. "Light_*")
 * \param[in,out] r where to store the red component
 * \param[in,out] g where to store the green component
 * \param[in,out] b where to store the blue component
 */
void
ay_prefs_setcolor(Tcl_Interp *interp, char *arr, char *var,
		  double *r, double *g, double *b)
{
 Tcl_Obj *to;
 size_t l;
 int i, itemp[3];
 char apps[3] = {'R', 'G', 'B'};

  l = strlen(var)-1;
  for(i = 0; i < 3; i++)
    {
      var[l] = apps[i];
      to = Tcl_GetVar2Ex(interp, arr, var,
			 TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
      Tcl_GetIntFromObj(interp, to, &(itemp[i]));
      if(itemp[i] < 0)
	itemp[i] = 0;
      if(itemp[i] > 255)
	itemp[i] = 255;
    }

  *r = itemp[0]/255.0;
  *g = itemp[1]/255.0;
  *b = itemp[2]/255.0;

 return;
} /* ay_prefs_setcolor */
