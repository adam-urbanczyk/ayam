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

/* prefs.c - functions for handling of preference settings */

/* C -> Tcl! */
int
ay_prefs_gettcmd(ClientData clientData, Tcl_Interp *interp,
		 int argc, char *argv[])
{
  /* char fname[] = "getPrefs";*/
 char *n1="ayprefs";
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

  Tcl_SetStringObj(ton, "UndoLevels", -1);
  to = Tcl_NewIntObj(ay_prefs.undo_levels);
  Tcl_ObjSetVar2(interp, toa, ton, to, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton, "Snap3D", -1);
  to = Tcl_NewIntObj(ay_prefs.snap3d);
  Tcl_ObjSetVar2(interp, toa, ton, to, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton, "Tolerance", -1);
  to = Tcl_NewDoubleObj(ay_prefs.glu_sampling_tolerance);
  Tcl_ObjSetVar2(interp, toa, ton, to, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton, "NPDisplayMode", -1);
  to = Tcl_NewIntObj(ay_prefs.np_display_mode);
  Tcl_ObjSetVar2(interp, toa, ton, to, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton, "NCDisplayMode", -1);
  to = Tcl_NewIntObj(ay_prefs.nc_display_mode);
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

  Tcl_SetStringObj(ton, "PPRender", -1);
  to = Tcl_NewStringObj(ay_prefs.pprender, -1);
  Tcl_ObjSetVar2(interp, toa, ton, to, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton, "SMethod", -1);
  to = Tcl_NewIntObj(ay_prefs.smethod);
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

  Tcl_IncrRefCount(toa);Tcl_DecrRefCount(toa);
  Tcl_IncrRefCount(ton);Tcl_DecrRefCount(ton);

 return TCL_OK;
} /* ay_prefs_gettcmd */


/* Tcl -> C! */
int
ay_prefs_settcmd(ClientData clientData, Tcl_Interp *interp,
		 int argc, char *argv[])
{
  /* char fname[] = "setPrefs";*/
 char *n1="ayprefs", *n2 = "ayprefse";
 Tcl_Obj *to = NULL, *toa = NULL, *ton = NULL;
 int itemp = 0, ay_status = AY_OK, qf = 0;
 double dtemp = 0.0;
 char fname[] = "set_prefs";
 char *str = NULL;
 char *ucargs[3] = {0}, ucarg1[] = "clear";

  /* Modeling */
  toa = Tcl_NewStringObj(n1, -1);
  ton = Tcl_NewStringObj("PickEpsilon", -1);
  to = Tcl_ObjGetVar2(interp, toa, ton, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetDoubleFromObj(interp, to, &dtemp);

  if(dtemp > 0.0)
    {
      ay_prefs.pick_epsilon = dtemp;
    }
  else
    {
      ay_error(AY_ERROR, fname,
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

  Tcl_SetStringObj(ton, "HandleSize", -1);
  to = Tcl_ObjGetVar2(interp, toa, ton, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetDoubleFromObj(interp, to, &ay_prefs.handle_size);

  Tcl_SetStringObj(ton, "LazyNotify", -1);
  to = Tcl_ObjGetVar2(interp, toa, ton, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp, to, &ay_prefs.lazynotify);

  Tcl_SetStringObj(ton, "CompleteNotify", -1);
  to = Tcl_ObjGetVar2(interp, toa, ton, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp, to, &ay_prefs.completenotify);

  Tcl_SetStringObj(ton, "EditSnaps", -1);
  to = Tcl_ObjGetVar2(interp, toa, ton, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp, to, &ay_prefs.edit_snaps_to_grid);

  Tcl_SetStringObj(ton, "UndoLevels", -1);
  to = Tcl_ObjGetVar2(interp, toa, ton, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp, to, &itemp);

  if(itemp != ay_prefs.undo_levels)
    {

      if((itemp != -1) && (itemp < 2)/* && (itemp < 1000)*/)
	{
	  itemp = 2;

	  ay_error(AY_ERROR, fname,
	   "Illegal value for UndoLevels (should be >2 or -1), reset to 2.");

	  to = Tcl_NewIntObj(itemp);
	  Tcl_ObjSetVar2(interp, toa, ton, to,
			 TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
	  Tcl_SetStringObj(toa, n2, -1);
	  Tcl_ObjSetVar2(interp, toa, ton, to,
			 TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
	  Tcl_SetStringObj(toa, n1, -1);
	}


      /*      ay_status = ay_undo_clear();*/
      ucargs[1] = ucarg1;
      ay_undo_undotcmd(clientData, interp, 2, ucargs);

      ay_status = ay_undo_init(itemp);
      if(ay_status)
	{
	  ay_error(ay_status, fname, NULL);
	  return TCL_OK;
	}

      ay_prefs.undo_levels = itemp;

    } /* if */

  Tcl_SetStringObj(ton, "Snap3D", -1);
  to = Tcl_ObjGetVar2(interp, toa, ton, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp, to, &ay_prefs.snap3d);

  Tcl_SetStringObj(ton, "Tolerance", -1);
  to = Tcl_ObjGetVar2(interp, toa, ton, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetDoubleFromObj(interp, to, &ay_prefs.glu_sampling_tolerance);

  qf = ay_stess_GetQF(ay_prefs.glu_sampling_tolerance);

  if(qf < 1)
    {
      qf = 1;
    }
  ay_prefs.stess_qf = qf;


  Tcl_SetStringObj(ton, "NPDisplayMode", -1);
  to = Tcl_ObjGetVar2(interp, toa, ton, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp, to, &ay_prefs.np_display_mode);

  Tcl_SetStringObj(ton, "NCDisplayMode", -1);
  to = Tcl_ObjGetVar2(interp, toa, ton, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp, to, &ay_prefs.nc_display_mode);

  Tcl_SetStringObj(ton, "UseMatColor", -1);
  to = Tcl_ObjGetVar2(interp, toa, ton, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp, to, &ay_prefs.use_materialcolor);

  Tcl_SetStringObj(ton, "Background_R", -1);
  to = Tcl_ObjGetVar2(interp, toa, ton, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp, to, &itemp);
  if(itemp < 0) itemp = 0;
  if(itemp > 255) itemp = 255;
  ay_prefs.bgr = (double)(itemp/255.0);

  Tcl_SetStringObj(ton, "Background_G", -1);
  to = Tcl_ObjGetVar2(interp, toa, ton, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp, to, &itemp);
  if(itemp < 0) itemp = 0;
  if(itemp > 255) itemp = 255;
  ay_prefs.bgg = (double)(itemp/255.0);

  Tcl_SetStringObj(ton, "Background_B", -1);
  to = Tcl_ObjGetVar2(interp, toa, ton, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp, to, &itemp);
  if(itemp < 0) itemp = 0;
  if(itemp > 255) itemp = 255;
  ay_prefs.bgb = (double)(itemp/255.0);

  Tcl_SetStringObj(ton, "Object_R", -1);
  to = Tcl_ObjGetVar2(interp, toa, ton, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp, to, &itemp);
  if(itemp < 0) itemp = 0;
  if(itemp > 255) itemp = 255;
  ay_prefs.obr = (double)(itemp/255.0);

  Tcl_SetStringObj(ton, "Object_G", -1);
  to = Tcl_ObjGetVar2(interp, toa, ton, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp, to, &itemp);
  if(itemp < 0) itemp = 0;
  if(itemp > 255) itemp = 255;
  ay_prefs.obg = (double)(itemp/255.0);

  Tcl_SetStringObj(ton, "Object_B", -1);
  to = Tcl_ObjGetVar2(interp, toa, ton, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp, to, &itemp);
  if(itemp < 0) itemp = 0;
  if(itemp > 255) itemp = 255;
  ay_prefs.obb = (double)(itemp/255.0);


  Tcl_SetStringObj(ton, "Selection_R", -1);
  to = Tcl_ObjGetVar2(interp, toa, ton, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp, to, &itemp);
  if(itemp < 0) itemp = 0;
  if(itemp > 255) itemp = 255;
  ay_prefs.ser = (double)(itemp/255.0);

  Tcl_SetStringObj(ton, "Selection_G", -1);
  to = Tcl_ObjGetVar2(interp, toa, ton, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp, to, &itemp);
  if(itemp < 0) itemp = 0;
  if(itemp > 255) itemp = 255;
  ay_prefs.seg = (double)(itemp/255.0);

  Tcl_SetStringObj(ton, "Selection_B", -1);
  to = Tcl_ObjGetVar2(interp, toa, ton, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp, to, &itemp);
  if(itemp < 0) itemp = 0;
  if(itemp > 255) itemp = 255;
  ay_prefs.seb = (double)(itemp/255.0);


  Tcl_SetStringObj(ton, "Grid_R", -1);
  to = Tcl_ObjGetVar2(interp, toa, ton, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp, to, &itemp);
  if(itemp < 0) itemp = 0;
  if(itemp > 255) itemp = 255;
  ay_prefs.grr = (double)(itemp/255.0);

  Tcl_SetStringObj(ton, "Grid_G", -1);
  to = Tcl_ObjGetVar2(interp, toa, ton, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp, to, &itemp);
  if(itemp < 0) itemp = 0;
  if(itemp > 255) itemp = 255;
  ay_prefs.grg = (double)(itemp/255.0);

  Tcl_SetStringObj(ton, "Grid_B", -1);
  to = Tcl_ObjGetVar2(interp, toa, ton, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp, to, &itemp);
  if(itemp < 0) itemp = 0;
  if(itemp > 255) itemp = 255;
  ay_prefs.grb = (double)(itemp/255.0);


  Tcl_SetStringObj(ton, "Tag_R", -1);
  to = Tcl_ObjGetVar2(interp, toa, ton, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp, to, &itemp);
  if(itemp < 0) itemp = 0;
  if(itemp > 255) itemp = 255;
  ay_prefs.tpr = (double)(itemp/255.0);

  Tcl_SetStringObj(ton, "Tag_G", -1);
  to = Tcl_ObjGetVar2(interp, toa, ton, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp, to, &itemp);
  if(itemp < 0) itemp = 0;
  if(itemp > 255) itemp = 255;
  ay_prefs.tpg = (double)(itemp/255.0);

  Tcl_SetStringObj(ton, "Tag_B", -1);
  to = Tcl_ObjGetVar2(interp, toa, ton, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp, to, &itemp);
  if(itemp < 0) itemp = 0;
  if(itemp > 255) itemp = 255;
  ay_prefs.tpb = (double)(itemp/255.0);


  Tcl_SetStringObj(ton, "Shade_R", -1);
  to = Tcl_ObjGetVar2(interp, toa, ton, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp, to, &itemp);
  if(itemp < 0) itemp = 0;
  if(itemp > 255) itemp = 255;
  ay_prefs.shr = (double)(itemp/255.0);

  Tcl_SetStringObj(ton, "Shade_G", -1);
  to = Tcl_ObjGetVar2(interp, toa, ton, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp, to, &itemp);
  if(itemp < 0) itemp = 0;
  if(itemp > 255) itemp = 255;
  ay_prefs.shg = (double)(itemp/255.0);

  Tcl_SetStringObj(ton, "Shade_B", -1);
  to = Tcl_ObjGetVar2(interp, toa, ton, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp, to, &itemp);
  if(itemp < 0) itemp = 0;
  if(itemp > 255) itemp = 255;
  ay_prefs.shb = (double)(itemp/255.0);

  Tcl_SetStringObj(ton, "Light_R", -1);
  to = Tcl_ObjGetVar2(interp, toa, ton, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp, to, &itemp);
  if(itemp < 0) itemp = 0;
  if(itemp > 255) itemp = 255;
  ay_prefs.lir = (double)(itemp/255.0);

  Tcl_SetStringObj(ton, "Light_G", -1);
  to = Tcl_ObjGetVar2(interp, toa, ton, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp, to, &itemp);
  if(itemp < 0) itemp = 0;
  if(itemp > 255) itemp = 255;
  ay_prefs.lig = (double)(itemp/255.0);

  Tcl_SetStringObj(ton, "Light_B", -1);
  to = Tcl_ObjGetVar2(interp, toa, ton, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp, to, &itemp);
  if(itemp < 0) itemp = 0;
  if(itemp > 255) itemp = 255;
  ay_prefs.lib = (double)(itemp/255.0);

  Tcl_SetStringObj(ton, "SelXOR_R", -1);
  to = Tcl_ObjGetVar2(interp, toa, ton, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp, to, &itemp);
  if(itemp < 0) itemp = 0;
  if(itemp > 255) itemp = 255;
  ay_prefs.sxr = (double)(itemp/255.0);

  Tcl_SetStringObj(ton, "SelXOR_G", -1);
  to = Tcl_ObjGetVar2(interp, toa, ton, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp, to, &itemp);
  if(itemp < 0) itemp = 0;
  if(itemp > 255) itemp = 255;
  ay_prefs.sxg = (double)(itemp/255.0);

  Tcl_SetStringObj(ton, "SelXOR_B", -1);
  to = Tcl_ObjGetVar2(interp, toa, ton, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp, to, &itemp);
  if(itemp < 0) itemp = 0;
  if(itemp > 255) itemp = 255;
  ay_prefs.sxb = (double)(itemp/255.0);


  /* RIB */
  Tcl_SetStringObj(ton, "ResInstances", -1);
  to = Tcl_ObjGetVar2(interp, toa, ton, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp, to, &ay_prefs.resolveinstances);

  Tcl_SetStringObj(ton, "CheckLights", -1);
  to = Tcl_ObjGetVar2(interp, toa, ton, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp, to, &ay_prefs.checklights);

  Tcl_SetStringObj(ton, "DefaultMat", -1);
  to = Tcl_ObjGetVar2(interp, toa, ton, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp, to, &ay_prefs.defaultmat);

  Tcl_SetStringObj(ton, "RIStandard", -1);
  to = Tcl_ObjGetVar2(interp, toa, ton, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp, to, &ay_prefs.ristandard);

  Tcl_SetStringObj(ton, "WriteIdent", -1);
  to = Tcl_ObjGetVar2(interp, toa, ton, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp, to, &ay_prefs.writeident);

  Tcl_SetStringObj(ton, "ShadowMaps", -1);
  to = Tcl_ObjGetVar2(interp, toa, ton, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp, to, &ay_prefs.use_sm);

  Tcl_SetStringObj(ton, "ExcludeHidden", -1);
  to = Tcl_ObjGetVar2(interp, toa, ton, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp, to, &ay_prefs.excludehidden);

  /* Misc */
  Tcl_SetStringObj(ton, "ListTypes", -1);
  to = Tcl_ObjGetVar2(interp, toa, ton, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp, to, &ay_prefs.list_types);

  Tcl_SetStringObj(ton, "MarkHidden", -1);
  to = Tcl_ObjGetVar2(interp, toa, ton, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp, to, &ay_prefs.mark_hidden);

  Tcl_SetStringObj(ton, "Logging", -1);
  to = Tcl_ObjGetVar2(interp, toa, ton, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp, to, &ay_prefs.writelog);

  Tcl_SetStringObj(ton, "LogFile", -1);
  to = Tcl_ObjGetVar2(interp, toa, ton, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  str = Tcl_GetStringFromObj(to, NULL);

  if(str)
    {
      if(ay_prefs.logfile)
	free(ay_prefs.logfile);
      ay_prefs.logfile = NULL;

      if(!(ay_prefs.logfile = calloc(strlen(str)+1, sizeof(char))))
	return AY_EOMEM;

      strcpy(ay_prefs.logfile, str);
    }

  Tcl_SetStringObj(ton, "PPRender", -1);
  to = Tcl_ObjGetVar2(interp, toa, ton, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  str = Tcl_GetStringFromObj(to, NULL);

  if(str)
    {
      if(ay_prefs.pprender)
	free(ay_prefs.pprender);
      ay_prefs.pprender = NULL;

      if(!(ay_prefs.pprender = calloc(strlen(str)+1, sizeof(char))))
	return AY_EOMEM;

      strcpy(ay_prefs.pprender, str);
    } /* if */


  Tcl_SetStringObj(ton, "SMethod", -1);
  to = Tcl_ObjGetVar2(interp, toa, ton, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp, to, &ay_prefs.smethod);

  Tcl_SetStringObj(ton, "SParamU", -1);
  to = Tcl_ObjGetVar2(interp, toa, ton, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetDoubleFromObj(interp, to, &ay_prefs.sparamu);

  Tcl_SetStringObj(ton, "SParamV", -1);
  to = Tcl_ObjGetVar2(interp, toa, ton, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetDoubleFromObj(interp, to, &ay_prefs.sparamv);

  Tcl_SetStringObj(ton, "WarnUnknownTag", -1);
  to = Tcl_ObjGetVar2(interp, toa, ton, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp, to, &ay_prefs.wutag);

  Tcl_IncrRefCount(toa);Tcl_DecrRefCount(toa);
  Tcl_IncrRefCount(ton);Tcl_DecrRefCount(ton);

 return TCL_OK;
} /* ay_prefs_settcmd */
