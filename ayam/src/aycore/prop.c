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

/* prop.c - functions for handling of general properties */

/* C -> Tcl! */
int
ay_prop_gettcmd(ClientData clientData, Tcl_Interp *interp,
		int argc, char *argv[])
{
 int ay_status = AY_OK;
 char fname[] = "getProp";
 ay_list_object *sel = ay_selection;
 ay_object *o = NULL;
 ay_propcb *cb = NULL;
 void **arr = NULL;

  if(!sel)
    {
      ay_error(AY_ENOSEL, fname, NULL);
      return TCL_OK;
    }

  o = sel->object;
  if(!o)
    {
      ay_error(AY_ENULL, fname, NULL);
      return TCL_OK;
    }

  arr = ay_getpropcbt.arr;
  cb = (ay_propcb *)(arr[o->type]);
  if(cb)
    ay_status = cb(interp, argc, argv, o);

  if(ay_status)
    ay_error(AY_ERROR, fname, "getprop callback failed...");


 return TCL_OK;
} /* ay_prop_gettcmd */


/* Tcl -> C! */
int
ay_prop_settcmd(ClientData clientData, Tcl_Interp *interp,
		int argc, char *argv[])
{
 int ay_status = AY_OK;
 char fname[] = "setProp";
 ay_list_object *sel = ay_selection;
 ay_object *o = NULL;
 ay_propcb *cb = NULL;
 void **arr = NULL;

  if(!sel)
    {
      ay_error(AY_ENOSEL, fname, NULL);
      return TCL_OK;
    }

  o = sel->object;
  if(!o)
    {
      ay_error(AY_ENULL, fname, NULL);
      return TCL_OK;
    }

  /* get callback from table */
  arr = ay_setpropcbt.arr;
  cb = (ay_propcb *)(arr[o->type]);
  if(cb)
    ay_status = cb(interp, argc, argv, o);

  if(ay_status)
    ay_error(AY_ERROR, fname, "setprop callback failed...");

 return TCL_OK;
} /* ay_prop_settcmd */


/* C -> Tcl! */
int
ay_prop_gettrafotcmd(ClientData clientData, Tcl_Interp *interp,
		int argc, char *argv[])
{
 char fname[] = "getTrafo";
 ay_list_object *sel = ay_selection;
 ay_object *o = NULL;
 char *n1="transfPropData";
 Tcl_Obj *to = NULL, *toa = NULL, *ton = NULL;

  if(!sel)
    {
      ay_error(AY_ENOSEL, fname, NULL);
      return TCL_OK;
    }

  o = sel->object;
  if(!o)
    {
      ay_error(AY_ENULL, fname, NULL);
      return TCL_OK;
    }

  toa = Tcl_NewStringObj(n1, -1);
  ton = Tcl_NewStringObj("Translate_X", -1);
  to = Tcl_NewDoubleObj(o->movx);
  Tcl_ObjSetVar2(interp, toa, ton, to, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_SetStringObj(ton, "Translate_Y", -1);
  to = Tcl_NewDoubleObj(o->movy);
  Tcl_ObjSetVar2(interp, toa, ton, to, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_SetStringObj(ton, "Translate_Z", -1);
  to = Tcl_NewDoubleObj(o->movz);
  Tcl_ObjSetVar2(interp, toa, ton, to, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton, "Quat0", -1);
  to = Tcl_NewDoubleObj(o->quat[0]);
  Tcl_ObjSetVar2(interp, toa, ton, to, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_SetStringObj(ton, "Quat1", -1);
  to = Tcl_NewDoubleObj(o->quat[1]);
  Tcl_ObjSetVar2(interp, toa, ton, to, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_SetStringObj(ton, "Quat2", -1);
  to = Tcl_NewDoubleObj(o->quat[2]);
  Tcl_ObjSetVar2(interp, toa, ton, to, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_SetStringObj(ton, "Quat3", -1);
  to = Tcl_NewDoubleObj(o->quat[3]);
  Tcl_ObjSetVar2(interp, toa, ton, to, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton, "Rotate_X", -1);
  to = Tcl_NewDoubleObj(o->rotx);
  Tcl_ObjSetVar2(interp, toa, ton, to, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_SetStringObj(ton, "Rotate_Y", -1);
  to = Tcl_NewDoubleObj(o->roty);
  Tcl_ObjSetVar2(interp, toa, ton, to, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_SetStringObj(ton, "Rotate_Z", -1);
  to = Tcl_NewDoubleObj(o->rotz);
  Tcl_ObjSetVar2(interp, toa, ton, to, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_SetStringObj(ton, "Scale_X", -1);
  to = Tcl_NewDoubleObj(o->scalx);
  Tcl_ObjSetVar2(interp, toa, ton, to, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_SetStringObj(ton, "Scale_Y", -1);
  to = Tcl_NewDoubleObj(o->scaly);
  Tcl_ObjSetVar2(interp, toa, ton, to, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_SetStringObj(ton, "Scale_Z", -1);
  to = Tcl_NewDoubleObj(o->scalz);
  Tcl_ObjSetVar2(interp, toa, ton, to, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_IncrRefCount(toa);Tcl_DecrRefCount(toa);
  Tcl_IncrRefCount(ton);Tcl_DecrRefCount(ton);


 return TCL_OK;
} /* ay_prop_gettrafotcmd */


/* Tcl -> C! */
int
ay_prop_settrafotcmd(ClientData clientData, Tcl_Interp *interp,
		int argc, char *argv[])
{
 char fname[] = "setTrafo";
 ay_list_object *sel = ay_selection;
 ay_object *o = NULL;
 char *n1="transfPropData";
 Tcl_Obj *to = NULL, *toa = NULL, *ton = NULL;
 double dtemp;
 double xaxis[3] = {1.0,0.0,0.0};
 double yaxis[3] = {0.0,1.0,0.0};
 double zaxis[3] = {0.0,0.0,1.0};
 double quat[4], drot;
 int pasteProp = 0;

  if(!sel)
    {
      ay_error(AY_ENOSEL, fname, NULL);
      return TCL_OK;
    }

  o = sel->object;
  if(!o)
    {
      ay_error(AY_ENULL, fname, NULL);
      return TCL_OK;
    }

  toa = Tcl_NewStringObj("ay", -1);
  ton = Tcl_NewStringObj("pasteProp", -1);
  to = Tcl_ObjGetVar2(interp, toa, ton, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp, to, &pasteProp);

  Tcl_SetStringObj(toa, n1, -1);
  Tcl_SetStringObj(ton, "Translate_X", -1);
  to = Tcl_ObjGetVar2(interp, toa, ton, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetDoubleFromObj(interp, to, &(o->movx));
  Tcl_SetStringObj(ton, "Translate_Y", -1);
  to = Tcl_ObjGetVar2(interp, toa, ton, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetDoubleFromObj(interp, to, &(o->movy));
  Tcl_SetStringObj(ton, "Translate_Z", -1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetDoubleFromObj(interp,to, &(o->movz));

  Tcl_SetStringObj(ton, "Quat0", -1);
  to = Tcl_ObjGetVar2(interp, toa, ton, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetDoubleFromObj(interp, to, &dtemp);
  o->quat[0] = dtemp;
  Tcl_SetStringObj(ton, "Quat1", -1);
  to = Tcl_ObjGetVar2(interp, toa, ton, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetDoubleFromObj(interp, to, &dtemp);
  o->quat[1] = dtemp;
  Tcl_SetStringObj(ton, "Quat2", -1);
  to = Tcl_ObjGetVar2(interp, toa, ton, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetDoubleFromObj(interp, to, &dtemp);
  o->quat[2] = dtemp;
  Tcl_SetStringObj(ton, "Quat3", -1);
  to = Tcl_ObjGetVar2(interp, toa, ton, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetDoubleFromObj(interp, to, &dtemp);
  o->quat[3] = dtemp;

  Tcl_SetStringObj(ton, "Rotate_X", -1);
  to = Tcl_ObjGetVar2(interp, toa, ton, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetDoubleFromObj(interp, to, &dtemp);
  if(o->rotx != dtemp)
    {
      if(!pasteProp)
	{
	  drot = (o->rotx - dtemp);
	  ay_quat_axistoquat(xaxis, AY_D2R(drot), quat);
	  ay_quat_add(quat, o->quat, o->quat);
	}
      o->rotx = dtemp;
    }
  Tcl_SetStringObj(ton, "Rotate_Y", -1);
  to = Tcl_ObjGetVar2(interp, toa, ton, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetDoubleFromObj(interp, to, &dtemp);
  if(o->roty != dtemp)
    {
      if(!pasteProp)
	{
	  drot = (o->roty - dtemp);
	  ay_quat_axistoquat(yaxis, AY_D2R(drot), quat);
	  ay_quat_add(quat, o->quat, o->quat);
	}
      o->roty = dtemp;
    }
  Tcl_SetStringObj(ton, "Rotate_Z", -1);
  to = Tcl_ObjGetVar2(interp, toa, ton, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetDoubleFromObj(interp, to, &dtemp);
  if(o->rotz != dtemp)
    {
      if(!pasteProp)
	{
	  drot = (o->rotz - dtemp);
	  ay_quat_axistoquat(zaxis, AY_D2R(drot), quat);
	  ay_quat_add(quat, o->quat, o->quat);
	}
      o->rotz = dtemp;
    }

  if(o->rotx == 0.0 && o->roty == 0.0 && o->rotz == 0.0)
    {
      o->quat[0] = 0.0;
      o->quat[1] = 0.0;
      o->quat[2] = 0.0;
      o->quat[3] = 1.0;
    }

  Tcl_SetStringObj(ton, "Scale_X", -1);
  to = Tcl_ObjGetVar2(interp, toa, ton, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetDoubleFromObj(interp, to, &dtemp);
  if(dtemp != 0.0)
    o->scalx = dtemp;
  Tcl_SetStringObj(ton, "Scale_Y", -1);
  to = Tcl_ObjGetVar2(interp, toa, ton, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetDoubleFromObj(interp, to, &dtemp);
  if(dtemp != 0.0)
    o->scaly = dtemp;
  Tcl_SetStringObj(ton, "Scale_Z", -1);
  to = Tcl_ObjGetVar2(interp, toa, ton, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetDoubleFromObj(interp, to, &dtemp);
  if(dtemp != 0.0)
    o->scalz = dtemp;

  Tcl_IncrRefCount(toa);Tcl_DecrRefCount(toa);
  Tcl_IncrRefCount(ton);Tcl_DecrRefCount(ton);

  o->modified = AY_TRUE;

  ay_notify_parent();

 return TCL_OK;
} /* ay_prop_settrafotcmd */


/* C -> Tcl! */
int
ay_prop_getattrtcmd(ClientData clientData, Tcl_Interp *interp,
		    int argc, char *argv[])
{
 char fname[] = "getAttr";
 ay_list_object *sel = ay_selection;
 ay_object *o = NULL;
 char *n1="attrPropData";
 Tcl_Obj *to = NULL, *toa = NULL, *ton = NULL;

  if(!sel)
    {
      ay_error(AY_ENOSEL, fname, NULL);
      return TCL_OK;
    }

  o = sel->object;
  if(!o)
    {
      ay_error(AY_ENULL, fname, NULL);
      return TCL_OK;
    }

  toa = Tcl_NewStringObj(n1, -1);
  ton = Tcl_NewStringObj("Objectname", -1);
  to = Tcl_NewStringObj(o->name, -1);
  Tcl_ObjSetVar2(interp, toa, ton, to, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton, "Hide", -1);
  to = Tcl_NewIntObj(o->hide);
  Tcl_ObjSetVar2(interp, toa, ton, to, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton, "HideChildren", -1);
  to = Tcl_NewIntObj(o->hide_children);
  Tcl_ObjSetVar2(interp, toa, ton, to, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton, "RefCount", -1);
  to = Tcl_NewIntObj(o->refcount);
  Tcl_ObjSetVar2(interp, toa, ton, to, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);


  Tcl_IncrRefCount(toa);Tcl_DecrRefCount(toa);
  Tcl_IncrRefCount(ton);Tcl_DecrRefCount(ton);

 return TCL_OK;
} /* ay_prop_getattrtcmd */


/* Tcl -> C! */
int
ay_prop_setattrtcmd(ClientData clientData, Tcl_Interp *interp,
		    int argc, char *argv[])
{
 char fname[] = "setAttr";
 ay_list_object *sel = ay_selection;
 ay_object *o = NULL;
 char *n1="attrPropData";
 Tcl_Obj *to = NULL, *toa = NULL, *ton = NULL;
 char *string = NULL;
 int stringlen;

  if(!sel)
    {
      ay_error(AY_ENOSEL, fname, NULL);
      return TCL_OK;
    }

  o = sel->object;
  if(!o)
    {
      ay_error(AY_ENULL, fname, NULL);
      return TCL_OK;
    }

  toa = Tcl_NewStringObj(n1, -1);
  ton = Tcl_NewStringObj("Objectname", -1);
  to = Tcl_ObjGetVar2(interp, toa, ton, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  string = Tcl_GetStringFromObj(to, &stringlen);
  if(!string)
    {
      ay_error(AY_ENULL, fname, NULL);
      return TCL_OK;
    }
  if(o->name)
    { free(o->name); o->name = NULL; }
  if(stringlen > 0)
    {
      if(!(o->name = calloc(stringlen+1, sizeof(char))))
	{
	  ay_error(AY_EOMEM, fname, NULL);
	  return TCL_OK;
	}
      strcpy(o->name, string);
    }

  Tcl_SetStringObj(ton, "Hide", -1);
  to = Tcl_ObjGetVar2(interp, toa, ton, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp, to, &o->hide);

  Tcl_SetStringObj(ton, "HideChildren", -1);
  to = Tcl_ObjGetVar2(interp, toa, ton, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp, to, &o->hide_children);

  Tcl_IncrRefCount(toa);Tcl_DecrRefCount(toa);
  Tcl_IncrRefCount(ton);Tcl_DecrRefCount(ton);

  ay_notify_parent();

 return TCL_OK;
} /* ay_prop_setattrtcmd */


/* C -> Tcl! */
int
ay_prop_getmattcmd(ClientData clientData, Tcl_Interp *interp,
		    int argc, char *argv[])
{
 char fname[] = "getMat";
 ay_list_object *sel = ay_selection;
 ay_object *o = NULL;
 char *n1="matPropData";
 Tcl_Obj *to = NULL, *toa = NULL, *ton = NULL;
 ay_mat_object *material = NULL;
 char *matname = NULL;

  if(!sel)
    {
      ay_error(AY_ENOSEL, fname, NULL);
      return TCL_OK;
    }

  o = sel->object;
  if(!o)
    {
      ay_error(AY_ENULL, fname, NULL);
      return TCL_OK;
    }

  toa = Tcl_NewStringObj(n1, -1);
  ton = Tcl_NewStringObj("Materialname", -1);
  /* clear variable */
  to = Tcl_NewStringObj("", -1);
  Tcl_ObjSetVar2(interp, toa, ton, to, TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);

  material = o->mat;

  if(material)
    {
      if(material->nameptr)
	{

	  matname = *(material->nameptr);
	  to = Tcl_NewStringObj(matname, -1);
	  Tcl_ObjSetVar2(interp, toa, ton, to, TCL_LEAVE_ERR_MSG |
			 TCL_GLOBAL_ONLY);
	}
    }

  Tcl_IncrRefCount(toa);Tcl_DecrRefCount(toa);
  Tcl_IncrRefCount(ton);Tcl_DecrRefCount(ton);

 return TCL_OK;
} /* ay_prop_getmattcmd */


/* Tcl -> C! */
int
ay_prop_setmattcmd(ClientData clientData, Tcl_Interp *interp,
		    int argc, char *argv[])
{
 char fname[] = "setMat";
 int ay_status = AY_OK;
 ay_list_object *sel = ay_selection;
 ay_object *o = NULL, *m = NULL;
 char *n1="matPropData";
 Tcl_Obj *to = NULL, *toa = NULL, *ton = NULL;
 char *string = NULL;
 int stringlen;
 ay_mat_object *material = NULL;

  if(!sel)
    {
      ay_error(AY_ENOSEL, fname, NULL);
      return TCL_OK;
    }

  o = sel->object;
  if(!o)
    {
      ay_error(AY_ENULL, fname, NULL);
      return TCL_OK;
    }

  toa = Tcl_NewStringObj(n1, -1);
  ton = Tcl_NewStringObj("Materialname", -1);
  to = Tcl_ObjGetVar2(interp, toa, ton, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  string = Tcl_GetStringFromObj(to, &stringlen);
  if(!string)
    {
      ay_error(AY_ENULL, fname, NULL);
      return TCL_OK;
    }

  if(o->mat)
    {
      m = o->mat->objptr;
      if(m)
	m->refcount--;
      else
	ay_error(AY_ERROR, fname, "could not decrease material refcount!");
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
	    ay_error(AY_ERROR, fname, "could not increase material refcount!");
	}
      else
	{
	  ay_error(AY_ERROR, fname, "material is not registered");
	}
    }

  Tcl_IncrRefCount(toa);Tcl_DecrRefCount(toa);
  Tcl_IncrRefCount(ton);Tcl_DecrRefCount(ton);

  ay_notify_parent();

 return TCL_OK;
} /* ay_prop_setmattcmd */


int
ay_prop_getnpinfo(Tcl_Interp *interp, char *n1, ay_object *o)
{
 Tcl_Obj *to = NULL, *toa = NULL, *ton = NULL, *to2 = NULL;
 char buffer[128], buffer2[128], buffer3[64], buffer4[64];
 char buffer5[64];
 ay_nurbpatch_object *np = NULL;

  if(!interp || !n1)
    return AY_ENULL;

  toa = Tcl_NewStringObj(n1, -1);

  ton = Tcl_NewStringObj("NPInfo", -1);

  if(o && o->type == AY_IDNPATCH)
    {
      np = (ay_nurbpatch_object *)(o->refine);
      /*"40 x 20, 4, 4, 3 (NURB), 3 (NURB)"*/
      sprintf(buffer/*, sizeof(buffer)*/, "%d x %d, %d, %d, %d, %d",
	       np->width, np->height, np->uorder, np->vorder,
	       np->uknot_type, np->vknot_type);

      to = Tcl_NewStringObj(buffer, -1);
    }
  else
    {
      to = Tcl_NewStringObj("n/a", -1);
    } /* if */

  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);

  /* set Balloon info text */
  Tcl_SetStringObj(ton, "NPInfoBall", -1);
  if(o && o->type == AY_IDNPATCH)
    {
      np = (ay_nurbpatch_object *)(o->refine);

      sprintf(buffer2/*, sizeof(buffer)*/,
	      "Width: %d\nHeight: %d\nOrder_U: %d\nOrder_V: %d\n",
	      np->width, np->height, np->uorder, np->vorder);

      to2 = Tcl_NewStringObj(buffer2, -1);

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
	}

      Tcl_AppendToObj(to2, buffer3, -1);

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
	}

      Tcl_AppendToObj(to2, buffer4, -1);

      sprintf(buffer5/*, sizeof(buffer)*/, "Is_Rational: ");

      if(np->is_rat)
	{
	  sprintf(&(buffer5[13])/*, sizeof(buffer)*/, "Yes");
	}
      else
	{
	  sprintf(&(buffer5[13])/*, sizeof(buffer)*/, "No");
	}

      Tcl_AppendToObj(to2, buffer5, -1);

    }
  else
    {
      to2 = Tcl_NewStringObj("n/a", -1);
    } /* if */

  Tcl_ObjSetVar2(interp,toa,ton,to2,TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);

  Tcl_IncrRefCount(toa);Tcl_DecrRefCount(toa);
  Tcl_IncrRefCount(ton);Tcl_DecrRefCount(ton);

 return AY_OK;
} /* ay_prop_getnpinfo */


int
ay_prop_getncinfo(Tcl_Interp *interp, char *n1, ay_object *o)
{
 Tcl_Obj *to = NULL, *toa = NULL, *ton = NULL;
 char buffer[256];
 ay_nurbcurve_object *nc = NULL;

  if(!interp || !n1)
    return AY_ENULL;

  toa = Tcl_NewStringObj(n1, -1);

  ton = Tcl_NewStringObj("NCInfo", -1);

  if(o && o->type == AY_IDNCURVE)
    {
      nc = (ay_nurbcurve_object *)(o->refine);
      /*"40, 4, NURB"*/
      sprintf(buffer/*, sizeof(buffer)*/, "%d, %d, %d",
	       nc->length, nc->order, nc->knot_type);

      to = Tcl_NewStringObj(buffer, -1);
    }
  else
    {
      to = Tcl_NewStringObj("n/a", -1);
    } /* if */

  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);

  Tcl_IncrRefCount(toa);Tcl_DecrRefCount(toa);
  Tcl_IncrRefCount(ton);Tcl_DecrRefCount(ton);

 return AY_OK;
} /* ay_prop_getncinfo */

