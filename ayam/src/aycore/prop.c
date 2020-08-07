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

/* prop.c - functions for handling of general properties */

/** ay_prop_gettcmd:
 *  Get the object type specific property data from the C context
 *  and put it into the respective data array in the Tcl context.
 *
 *  C -> Tcl!
 *
 *  Implements the \a getProp scripting interface command.
 *  See also the corresponding section in the \ayd{scgetprop}.
 *
 *  \returns TCL_OK in any case.
 */
int
ay_prop_gettcmd(ClientData clientData, Tcl_Interp *interp,
		int argc, char *argv[])
{
 int ay_status = AY_OK;
 ay_list_object *sel = ay_selection;
 ay_object *o = NULL;
 ay_propcb *cb = NULL;
 ay_voidfp *arr = NULL;

  if(!sel)
    {
      ay_error(AY_ENOSEL, argv[0], NULL);
      return TCL_OK;
    }

  o = sel->object;

  arr = ay_getpropcbt.arr;
  cb = (ay_propcb *)(arr[o->type]);
  if(cb)
    {
      ay_status = cb(interp, argc, argv, o);
    }

  if(ay_status)
    {
      ay_error(AY_ERROR, argv[0], "getprop callback failed...");
    }

 return TCL_OK;
} /* ay_prop_gettcmd */


/** ay_prop_settcmd:
 *  Set the object type specific property data from the Tcl context
 *  to the C context.
 *
 *  Tcl -> C!
 *
 *  Implements the \a setProp scripting interface command.
 *  See also the corresponding section in the \ayd{scsetprop}.
 *
 *  \returns TCL_OK in any case.
 */
int
ay_prop_settcmd(ClientData clientData, Tcl_Interp *interp,
		int argc, char *argv[])
{
 int ay_status = AY_OK;
 ay_list_object *sel = ay_selection;
 ay_object *o = NULL;
 ay_propcb *cb = NULL;
 ay_voidfp *arr = NULL;

  if(!sel)
    {
      ay_error(AY_ENOSEL, argv[0], NULL);
      return TCL_OK;
    }

  o = sel->object;

  arr = ay_setpropcbt.arr;
  cb = (ay_propcb *)(arr[o->type]);
  if(cb)
    {
      ay_status = cb(interp, argc, argv, o);
    }

  if(ay_status)
    {
      ay_error(AY_ERROR, argv[0], "setprop callback failed...");
    }

 return TCL_OK;
} /* ay_prop_settcmd */


/** ay_prop_gettrafotcmd:
 *  Get the transformation property data from the C context
 *  and put it into the respective data array in the Tcl context.
 *
 *  C -> Tcl!
 *
 *  Implements the \a getTrafo scripting interface command.
 *  See also the corresponding section in the \ayd{scgettrafo}.
 *
 *  \returns TCL_OK in any case.
 */
int
ay_prop_gettrafotcmd(ClientData clientData, Tcl_Interp *interp,
		     int argc, char *argv[])
{
 ay_list_object *sel = ay_selection;
 ay_object *o = NULL;
 char *arr = "transfPropData", *quatstr = NULL;
 int len;

  if(!sel)
    {
      ay_error(AY_ENOSEL, argv[0], NULL);
      return TCL_OK;
    }

  if(!(quatstr = calloc(TCL_DOUBLE_SPACE*4+10, sizeof(char))))
    return TCL_OK;

  o = sel->object;

  Tcl_SetVar2Ex(interp, arr, "Translate_X", Tcl_NewDoubleObj(o->movx),
		TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_SetVar2Ex(interp, arr, "Translate_Y", Tcl_NewDoubleObj(o->movy),
		TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_SetVar2Ex(interp, arr, "Translate_Z", Tcl_NewDoubleObj(o->movz),
		TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetVar2Ex(interp, arr, "Quat0", Tcl_NewDoubleObj(o->quat[0]),
		TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_SetVar2Ex(interp, arr, "Quat1", Tcl_NewDoubleObj(o->quat[1]),
		TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_SetVar2Ex(interp, arr, "Quat2", Tcl_NewDoubleObj(o->quat[2]),
		TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_SetVar2Ex(interp, arr, "Quat3", Tcl_NewDoubleObj(o->quat[3]),
		TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  len = sprintf(quatstr, "[%.2lg, %.2lg, %.2lg, %.2lg]",
		o->quat[0], o->quat[1], o->quat[2], o->quat[3]);
  Tcl_SetVar2Ex(interp, arr, "Quaternion", Tcl_NewStringObj(quatstr, len),
		TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  len = sprintf(quatstr, "[%lg, %lg, %lg, %lg]",
		o->quat[0], o->quat[1], o->quat[2], o->quat[3]);
  Tcl_SetVar2Ex(interp, arr, "QuaternionBall", Tcl_NewStringObj(quatstr, len),
		TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetVar2Ex(interp, arr, "Rotate_X", Tcl_NewDoubleObj(o->rotx),
		TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_SetVar2Ex(interp, arr, "Rotate_Y", Tcl_NewDoubleObj(o->roty),
		TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_SetVar2Ex(interp, arr, "Rotate_Z", Tcl_NewDoubleObj(o->rotz),
		TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetVar2Ex(interp, arr, "Scale_X", Tcl_NewDoubleObj(o->scalx),
		TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_SetVar2Ex(interp, arr, "Scale_Y", Tcl_NewDoubleObj(o->scaly),
		TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_SetVar2Ex(interp, arr, "Scale_Z", Tcl_NewDoubleObj(o->scalz),
		TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  free(quatstr);

 return TCL_OK;
} /* ay_prop_gettrafotcmd */


/** ay_prop_settrafotcmd:
 *  Set the transformation property data from the Tcl context
 *  to the C context.
 *
 *  Tcl -> C!
 *
 *  Implements the \a setTrafo scripting interface command.
 *  See also the corresponding section in the \ayd{scsettrafo}.
 *
 *  \returns TCL_OK in any case.
 */
int
ay_prop_settrafotcmd(ClientData clientData, Tcl_Interp *interp,
		     int argc, char *argv[])
{
 ay_list_object *sel = ay_selection;
 ay_object *o = NULL;
 char *arr = "transfPropData";
 Tcl_Obj *to = NULL;
 double dtemp;
 double xaxis[3] = {1.0,0.0,0.0};
 double yaxis[3] = {0.0,1.0,0.0};
 double zaxis[3] = {0.0,0.0,1.0};
 double quat[4], drot;
 int pasteProp = 0, notify_parent = AY_FALSE;

  if(!sel)
    {
      ay_error(AY_ENOSEL, argv[0], NULL);
      return TCL_OK;
    }

  o = sel->object;

  to = Tcl_GetVar2Ex(interp, "ay", "pasteProp",
		     TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  if(to)
    Tcl_GetIntFromObj(interp, to, &pasteProp);

  to = Tcl_GetVar2Ex(interp, arr, "Translate_X",
		     TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetDoubleFromObj(interp, to, &dtemp);
  if(dtemp != o->movx)
    {
      o->movx = dtemp;
      notify_parent = AY_TRUE;
    }

  to = Tcl_GetVar2Ex(interp, arr, "Translate_Y",
		     TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetDoubleFromObj(interp, to, &dtemp);
  if(dtemp != o->movy)
    {
      o->movy = dtemp;
      notify_parent = AY_TRUE;
    }

  to = Tcl_GetVar2Ex(interp, arr, "Translate_Z",
		     TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetDoubleFromObj(interp,to, &dtemp);
  if(dtemp != o->movz)
    {
      o->movz = dtemp;
      notify_parent = AY_TRUE;
    }

  to = Tcl_GetVar2Ex(interp, arr, "Quat0",
		     TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetDoubleFromObj(interp, to, &dtemp);
  if(dtemp != o->quat[0])
    {
      o->quat[0] = dtemp;
      notify_parent = AY_TRUE;
    }
  to = Tcl_GetVar2Ex(interp, arr, "Quat1",
		     TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetDoubleFromObj(interp, to, &dtemp);
  if(dtemp != o->quat[1])
    {
      o->quat[1] = dtemp;
      notify_parent = AY_TRUE;
    }
  to = Tcl_GetVar2Ex(interp, arr, "Quat2",
		     TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetDoubleFromObj(interp, to, &dtemp);
  if(dtemp != o->quat[2])
    {
      o->quat[2] = dtemp;
      notify_parent = AY_TRUE;
    }
  to = Tcl_GetVar2Ex(interp, arr, "Quat3",
		     TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetDoubleFromObj(interp, to, &dtemp);
  if(dtemp != o->quat[3])
    {
      o->quat[3] = dtemp;
      notify_parent = AY_TRUE;
    }

  to = Tcl_GetVar2Ex(interp, arr, "Rotate_X",
		     TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetDoubleFromObj(interp, to, &dtemp);
  if(o->rotx != dtemp)
    {
      if(!pasteProp)
	{
	  drot = (o->rotx - dtemp);
	  ay_quat_axistoquat(xaxis, AY_D2R(drot), quat);
	  ay_quat_add(quat, o->quat, o->quat);
	}
      notify_parent = AY_TRUE;
      o->rotx = dtemp;
    }
  to = Tcl_GetVar2Ex(interp, arr, "Rotate_Y",
		     TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetDoubleFromObj(interp, to, &dtemp);
  if(o->roty != dtemp)
    {
      if(!pasteProp)
	{
	  drot = (o->roty - dtemp);
	  ay_quat_axistoquat(yaxis, AY_D2R(drot), quat);
	  ay_quat_add(quat, o->quat, o->quat);
	}
      notify_parent = AY_TRUE;
      o->roty = dtemp;
    }
  to = Tcl_GetVar2Ex(interp, arr, "Rotate_Z",
		     TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetDoubleFromObj(interp, to, &dtemp);
  if(o->rotz != dtemp)
    {
      if(!pasteProp)
	{
	  drot = (o->rotz - dtemp);
	  ay_quat_axistoquat(zaxis, AY_D2R(drot), quat);
	  ay_quat_add(quat, o->quat, o->quat);
	}
      notify_parent = AY_TRUE;
      o->rotz = dtemp;
    }

  if(o->rotx == 0.0 && o->roty == 0.0 && o->rotz == 0.0)
    {
      o->quat[0] = 0.0;
      o->quat[1] = 0.0;
      o->quat[2] = 0.0;
      o->quat[3] = 1.0;
    }

  to = Tcl_GetVar2Ex(interp, arr, "Scale_X",
		     TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetDoubleFromObj(interp, to, &dtemp);
  if(dtemp != 0.0)
    {
      o->scalx = dtemp;
      notify_parent = AY_TRUE;
    }
  to = Tcl_GetVar2Ex(interp, arr, "Scale_Y",
		     TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetDoubleFromObj(interp, to, &dtemp);
  if(dtemp != 0.0)
    {
      o->scaly = dtemp;
      notify_parent = AY_TRUE;
    }
  to = Tcl_GetVar2Ex(interp, arr, "Scale_Z",
		     TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetDoubleFromObj(interp, to, &dtemp);
  if(dtemp != 0.0)
    {
      o->scalz = dtemp;
      notify_parent = AY_TRUE;
    }

  if(notify_parent)
    {
      o->modified = AY_TRUE;
      ay_notify_parent();
    }

 return TCL_OK;
} /* ay_prop_settrafotcmd */


/** ay_prop_getattrtcmd:
 *  Get the attributes property data from the C context
 *  and put it into the respective data array in the Tcl context.
 *
 *  C -> Tcl!
 *
 *  Implements the \a getAttr scripting interface command.
 *  See also the corresponding section in the \ayd{scgetattr}.
 *
 *  \returns TCL_OK in any case.
 */
int
ay_prop_getattrtcmd(ClientData clientData, Tcl_Interp *interp,
		    int argc, char *argv[])
{
 ay_list_object *sel = ay_selection;
 ay_object *o = NULL;
 char *arr = "attrPropData";

  if(!sel)
    {
      ay_error(AY_ENOSEL, argv[0], NULL);
      return TCL_OK;
    }

  o = sel->object;

  Tcl_SetVar2Ex(interp, arr, "Objectname", Tcl_NewStringObj(o->name, -1),
		TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetVar2Ex(interp, arr, "Hide", Tcl_NewIntObj(o->hide),
		TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetVar2Ex(interp, arr, "HideChildren", Tcl_NewIntObj(o->hide_children),
		TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetVar2Ex(interp, arr, "HideChildren", Tcl_NewIntObj(o->refcount),
		TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

 return TCL_OK;
} /* ay_prop_getattrtcmd */


/** ay_prop_setattrtcmd:
 *  Set the attributes property data from the Tcl context
 *  to the C context.
 *
 *  Tcl -> C!
 *
 *  Implements the \a setAttr scripting interface command.
 *  See also the corresponding section in the \ayd{scsetattr}.
 *
 *  \returns TCL_OK in any case.
 */
int
ay_prop_setattrtcmd(ClientData clientData, Tcl_Interp *interp,
		    int argc, char *argv[])
{
 ay_list_object *sel = ay_selection;
 ay_object *o = NULL;
 char *arr = "attrPropData";
 Tcl_Obj *to = NULL;
 char *string = NULL;
 int stringlen;

  if(!sel)
    {
      ay_error(AY_ENOSEL, argv[0], NULL);
      return TCL_OK;
    }

  o = sel->object;

  to = Tcl_GetVar2Ex(interp, arr, "Objectname",
		     TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  string = Tcl_GetStringFromObj(to, &stringlen);
  if(!string)
    {
      ay_error(AY_ENULL, argv[0], NULL);
      return TCL_OK;
    }
  if(o->name)
    {
      free(o->name);
      o->name = NULL;
    }
  if(stringlen > 0)
    {
      if(!(o->name = calloc(stringlen+1, sizeof(char))))
	{
	  ay_error(AY_EOMEM, argv[0], NULL);
	  return TCL_OK;
	}
      strcpy(o->name, string);
    }

  to = Tcl_GetVar2Ex(interp, arr, "Hide",
		     TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp, to, &(o->hide));

  to = Tcl_GetVar2Ex(interp, arr, "HideChildren",
		     TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp, to, &(o->hide_children));

  ay_notify_parent();

 return TCL_OK;
} /* ay_prop_setattrtcmd */


/** ay_prop_getmattcmd:
 *  Get the material property data from the C context
 *  and put it into the respective data array in the Tcl context.
 *
 *  C -> Tcl!
 *
 *  Implements the \a getMat scripting interface command.
 *  See also the corresponding section in the \ayd{scgetmat}.
 *
 *  \returns TCL_OK in any case.
 */
int
ay_prop_getmattcmd(ClientData clientData, Tcl_Interp *interp,
		   int argc, char *argv[])
{
 ay_list_object *sel = ay_selection;
 ay_object *o = NULL;
 char *arr = "matPropData";
 ay_mat_object *material = NULL;
 char *matname = NULL;

  if(!sel)
    {
      ay_error(AY_ENOSEL, argv[0], NULL);
      return TCL_OK;
    }

  o = sel->object;

  /* clear variable */
  Tcl_SetVar2Ex(interp, arr, "Materialname", Tcl_NewStringObj("", -1),
		TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  material = o->mat;

  if(material)
    {
      if(material->nameptr)
	{
	  matname = *(material->nameptr);
	  Tcl_SetVar2Ex(interp, arr, "Materialname",
			Tcl_NewStringObj(matname, -1),
			TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
	}
    }

 return TCL_OK;
} /* ay_prop_getmattcmd */


/** ay_prop_setmattcmd:
 *  Set the material property data from the Tcl context
 *  to the C context.
 *
 *  Tcl -> C!
 *
 *  Implements the \a setMat scripting interface command.
 *  See also the corresponding section in the \ayd{scsetmat}.
 *
 *  \returns TCL_OK in any case.
 */
int
ay_prop_setmattcmd(ClientData clientData, Tcl_Interp *interp,
		   int argc, char *argv[])
{
 int ay_status = AY_OK;
 ay_list_object *sel = ay_selection;
 ay_object *o = NULL, *m = NULL;
 char *arr = "matPropData";
 Tcl_Obj *to = NULL;
 char *string = NULL;
 int stringlen;
 ay_mat_object *material = NULL;

  if(!sel)
    {
      ay_error(AY_ENOSEL, argv[0], NULL);
      return TCL_OK;
    }

  o = sel->object;

  to = Tcl_GetVar2Ex(interp, arr, "Materialname",
		     TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  string = Tcl_GetStringFromObj(to, &stringlen);
  if(!string)
    {
      ay_error(AY_ENULL, argv[0], NULL);
      return TCL_OK;
    }

  if(o->mat)
    {
      m = o->mat->objptr;
      if(m)
	m->refcount--;
      else
	ay_error(AY_ERROR, argv[0], "could not decrease material refcount!");
    }

  o->mat = NULL;

  if(stringlen > 0)
    {
      ay_status = ay_matt_getmaterial(string, &material);
      if(!ay_status)
	{
	  o->mat = material;
	  m = material->objptr;
	  if(m)
	    m->refcount++;
	  else
	    ay_error(AY_ERROR, argv[0],
		     "could not increase material refcount!");
	}
      else
	{
	  ay_error(AY_ERROR, argv[0], "material is not registered");
	}
    }

  ay_notify_parent();

 return TCL_OK;
} /* ay_prop_setmattcmd */


/* ay_prop_getnpinfo:
 *  compile info string for NPInfo property field
 */
int
ay_prop_getnpinfo(Tcl_Interp *interp, char *arr, ay_object *o)
{
 Tcl_Obj *to = NULL, *tob = NULL;
 char buffer[128], buffer2[128], buffer3[64], buffer4[64];
 char buffer5[64];
 ay_nurbpatch_object *np = NULL;
 Tcl_DString ds;
 int len;

  if(!interp || !arr)
    return AY_ENULL;

  if(o && o->type == AY_IDNPATCH)
    {
      np = (ay_nurbpatch_object *)(o->refine);
      /*"40 x 20, 4, 4, 3 (NURB), 3 (NURB)"*/
      len = sprintf(buffer/*, sizeof(buffer)*/, "%d x %d, %d, %d, %d, %d",
	       np->width, np->height, np->uorder, np->vorder,
	       np->uknot_type, np->vknot_type);

      to = Tcl_NewStringObj(buffer, len);
    }
  else
    {
      to = Tcl_NewStringObj("n/a", 3);
    } /* if */

  Tcl_SetVar2Ex(interp, arr, "NPInfo", to,
		TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  if(interp == ay_safeinterp)
    {
      return AY_OK;
    }

  /* set Balloon info text */
  if(np)
    {
      Tcl_DStringInit(&ds);

      len = sprintf(buffer2/*, sizeof(buffer)*/,
		    "Width: %d\nHeight: %d\nOrder_U: %d\nOrder_V: %d\n",
		    np->width, np->height, np->uorder, np->vorder);

      Tcl_DStringAppend(&ds, buffer2, len);

      sprintf(buffer3/*, sizeof(buffer)*/, "Knot-Type_U: ");
      switch(np->uknot_type)
	{
	case AY_KTBEZIER:
	  sprintf(&(buffer3[13])/*, sizeof(buffer)*/, "Bezier\n");
	  break;
	case AY_KTBSPLINE:
	  sprintf(&(buffer3[13])/*, sizeof(buffer)*/, "B-Spline\n");
	  break;
	case AY_KTNURB:
	  sprintf(&(buffer3[13])/*, sizeof(buffer)*/, "NURB\n");
	  break;
	case AY_KTCUSTOM:
	  sprintf(&(buffer3[13])/*, sizeof(buffer)*/, "CUSTOM\n");
	  break;
	case AY_KTCHORDAL:
	  sprintf(&(buffer3[13])/*, sizeof(buffer)*/, "Chordal\n");
	  break;
	case AY_KTCENTRI:
	  sprintf(&(buffer3[13])/*, sizeof(buffer)*/, "Centripetal\n");
	  break;
	default:
	  sprintf(&(buffer3[13])/*, sizeof(buffer)*/, "Unknown\n");
	  break;
	}

      Tcl_DStringAppend(&ds, buffer3, -1);

      sprintf(buffer4/*, sizeof(buffer)*/, "Knot-Type_V: ");
      switch(np->vknot_type)
	{
	case AY_KTBEZIER:
	  sprintf(&(buffer4[13])/*, sizeof(buffer)*/, "Bezier\n");
	  break;
	case AY_KTBSPLINE:
	  sprintf(&(buffer4[13])/*, sizeof(buffer)*/, "B-Spline\n");
	  break;
	case AY_KTNURB:
	  sprintf(&(buffer4[13])/*, sizeof(buffer)*/, "NURB\n");
	  break;
	case AY_KTCUSTOM:
	  sprintf(&(buffer4[13])/*, sizeof(buffer)*/, "CUSTOM\n");
	  break;
	case AY_KTCHORDAL:
	  sprintf(&(buffer4[13])/*, sizeof(buffer)*/, "Chordal\n");
	  break;
	case AY_KTCENTRI:
	  sprintf(&(buffer4[13])/*, sizeof(buffer)*/, "Centripetal\n");
	  break;
	default:
	  sprintf(&(buffer4[13])/*, sizeof(buffer)*/, "Unknown\n");
	  break;
	}

      Tcl_DStringAppend(&ds, buffer4, -1);

      sprintf(buffer5/*, sizeof(buffer)*/, "Is_Rational: ");

      if(np->is_rat)
	{
	  sprintf(&(buffer5[13])/*, sizeof(buffer)*/, "Yes");
	}
      else
	{
	  sprintf(&(buffer5[13])/*, sizeof(buffer)*/, "No");
	}

      Tcl_DStringAppend(&ds, buffer5, -1);

      tob = Tcl_NewStringObj(Tcl_DStringValue(&ds), -1);

      Tcl_DStringFree(&ds);
    }
  else
    {
      tob = Tcl_NewStringObj("n/a", 3);
    } /* if o is NPatch */

  Tcl_SetVar2Ex(interp, arr, "NPInfoBall", tob, TCL_LEAVE_ERR_MSG |
		TCL_GLOBAL_ONLY);

 return AY_OK;
} /* ay_prop_getnpinfo */


/* ay_prop_getncinfo:
 *  compile info string for NCInfo property field
 */
int
ay_prop_getncinfo(Tcl_Interp *interp, char *arr, ay_object *o)
{
 Tcl_Obj *to = NULL, *tob = NULL;
 char buffer[256], buffer2[128], buffer3[64], buffer4[64];
 char buffer5[64];
 ay_nurbcurve_object *nc = NULL;
 Tcl_DString ds;
 int len;

  if(!interp || !arr)
    return AY_ENULL;

  if(o && o->type == AY_IDNCURVE)
    {
      nc = (ay_nurbcurve_object *)(o->refine);
      /*"40, 4, NURB"*/
      len = sprintf(buffer/*, sizeof(buffer)*/, "%d, %d, %d",
		    nc->length, nc->order, nc->knot_type);

      to = Tcl_NewStringObj(buffer, len);
    }
  else
    {
      to = Tcl_NewStringObj("n/a", 3);
    } /* if */

  Tcl_SetVar2Ex(interp, arr, "NCInfo", to, TCL_LEAVE_ERR_MSG |
		TCL_GLOBAL_ONLY);

  if(interp == ay_safeinterp)
    {
      return AY_OK;
    }

  /* set Balloon info text */
  if(nc)
    {
      Tcl_DStringInit(&ds);

      len = sprintf(buffer2/*, sizeof(buffer)*/,
		    "Length: %d\nOrder: %d\n",
		    nc->length, nc->order);

      Tcl_DStringAppend(&ds, buffer2, len);

      sprintf(buffer3/*, sizeof(buffer)*/, "Knot-Type: ");
      switch(nc->knot_type)
	{
	case AY_KTBEZIER:
	  sprintf(&(buffer3[11])/*, sizeof(buffer)*/, "Bezier\n");
	  break;
	case AY_KTBSPLINE:
	  sprintf(&(buffer3[11])/*, sizeof(buffer)*/, "B-Spline\n");
	  break;
	case AY_KTNURB:
	  sprintf(&(buffer3[11])/*, sizeof(buffer)*/, "NURB\n");
	  break;
	case AY_KTCUSTOM:
	  sprintf(&(buffer3[11])/*, sizeof(buffer)*/, "CUSTOM\n");
	  break;
	case AY_KTCHORDAL:
	  sprintf(&(buffer3[11])/*, sizeof(buffer)*/, "Chordal\n");
	  break;
	case AY_KTCENTRI:
	  sprintf(&(buffer3[11])/*, sizeof(buffer)*/, "Centripetal\n");
	  break;
	default:
	  sprintf(&(buffer3[11])/*, sizeof(buffer)*/, "Unknown\n");
	  break;
	}

      Tcl_DStringAppend(&ds, buffer3, -1);

      sprintf(buffer4/*, sizeof(buffer)*/, "Is_Rational: ");

      if(nc->is_rat)
	{
	  sprintf(&(buffer4[13])/*, sizeof(buffer)*/, "Yes\n");
	}
      else
	{
	  sprintf(&(buffer4[13])/*, sizeof(buffer)*/, "No\n");
	}

      Tcl_DStringAppend(&ds, buffer4, -1);

      sprintf(buffer5/*, sizeof(buffer)*/, "Type: ");

      switch(nc->type)
	{
	case AY_CTOPEN:
	  sprintf(&(buffer5[6])/*, sizeof(buffer)*/, "Open");
	  break;
	case AY_CTCLOSED:
	  sprintf(&(buffer5[6])/*, sizeof(buffer)*/, "Closed");
	  break;
	case AY_CTPERIODIC:
	  sprintf(&(buffer5[6])/*, sizeof(buffer)*/, "Periodic");
	  break;
	default:
	  break;
	} /* switch */

      Tcl_DStringAppend(&ds, buffer5, -1);

      tob = Tcl_NewStringObj(Tcl_DStringValue(&ds), Tcl_DStringLength(&ds));

      Tcl_DStringFree(&ds);
    }
  else
    {
      tob = Tcl_NewStringObj("n/a", -1);
    } /* if o is NCurve */

  Tcl_SetVar2Ex(interp, arr, "NCInfoBall", tob, TCL_LEAVE_ERR_MSG |
		TCL_GLOBAL_ONLY);

 return AY_OK;
} /* ay_prop_getncinfo */

