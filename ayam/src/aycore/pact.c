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

/* pact.c - single point related interactive actions */

/* prototypes of functions local to this module: */
int ay_pact_insertnc(ay_nurbcurve_object *curve,
		     double objX, double objY, double objZ);

int ay_pact_insertic(ay_icurve_object *curve,
		     double objX, double objY, double objZ);

int ay_pact_deletenc(ay_nurbcurve_object *curve,
		     double objX, double objY, double objZ);

int ay_pact_deleteic(ay_icurve_object *icurve,
		     double objX, double objY, double objZ);

/* functions: */

/* ay_pact_getpoint:
 *   calls object type specific getpoint
 *   functions to let the object decide
 *   which point should be edited in single
 *   point editing mode or for which point
 *   the weight should be changed
*/
int
ay_pact_getpoint(int mode, ay_object *o, double *obj)
{
 int ay_status = AY_OK;
 void **arr = NULL;
 ay_getpntcb *cb = NULL;

  if(!o)
    return AY_ENULL;

  arr = ay_getpntcbt.arr;
  cb = (ay_getpntcb *)(arr[o->type]);
  if(cb)
    ay_status = cb(mode, o, obj);

 return ay_status;
} /* ay_pact_getpoint */


/* ay_pact_seltcb:
 *  select/tag single points of an object
 */
int
ay_pact_seltcb(struct Togl *togl, int argc, char *argv[])
{
 int ay_status = AY_OK;
 char fname[] = "selpac";
 Tcl_Interp *interp = Togl_Interp (togl);
 /*  ay_view_object *view = Togl_GetClientData(togl);*/
 double height = Togl_Height(togl);
 int i, have_it;
 double winX = 0.0, winY = 0.0, winX2 = 0.0, winY2 = 0.0, dtemp = 0.0;
 double obj[24] = {0}, pe[16] = {0};
 /*  double pickepsilon = ay_prefs.pick_epsilon;*/
 ay_point_object *newp = NULL, *point = NULL, *last = NULL;
 int multiple = AY_FALSE;
 ay_list_object *sel = ay_selection;
 ay_object *o = NULL;
 GLfloat pixel[3] = {0.0f,0.0f,0.0f};

  if(!ay_selection)
    {
      ay_error(AY_ENOSEL, fname, NULL);
      return TCL_OK;
    }

  Tcl_GetDouble(interp, argv[2], &winX);
  Tcl_GetDouble(interp, argv[3], &winY);

  if(argc > 4)
    {
      multiple = AY_TRUE;
      Tcl_GetDouble(interp, argv[4], &winX2);
      Tcl_GetDouble(interp, argv[5], &winY2);

    }
  else
    {
      winX2 = winX;
      winY2 = winY;
    }

  if(winX2 < winX)
    {
      dtemp = winX2;
      winX2 = winX;
      winX = dtemp;
    }

  if(winY2 < winY)
    {
      dtemp = winY2;
      winY2 = winY;
      winY = dtemp;
    }

  while(sel)
    {
      o = sel->object;
      if(!o)
	return TCL_OK;

      if(!multiple)
	{
	  glReadPixels((GLint)(winX),(GLint)(height-winY), 1, 1,
		       GL_RGB, GL_FLOAT, &pixel);
	  ay_status = ay_viewt_wintoobj(togl, o, winX, winY,
					&(obj[0]), &(obj[1]), &(obj[2]));
	  
	  ay_status = ay_pact_getpoint(1, o, obj);
	}
      else
	{
	
	  ay_status = ay_viewt_winrecttoobj(togl, o, winX, winY, winX2, winY2,
					    obj);
	  
	  /* create plane equation coefficients */
	  ay_trafo_pointstoplane(obj[0], obj[1], obj[2],
				 obj[3], obj[4], obj[5],
				 obj[12], obj[13], obj[14],
				 &(pe[0]), &(pe[1]), &(pe[2]), &(pe[3]));

	  ay_trafo_pointstoplane(obj[3], obj[4], obj[5],
				 obj[9], obj[10], obj[11],
				 obj[15], obj[16], obj[17],
				 &(pe[4]), &(pe[5]), &(pe[6]), &(pe[7]));

	  ay_trafo_pointstoplane(obj[6], obj[7], obj[8],
				 obj[18], obj[19], obj[20],
				 obj[9], obj[10], obj[11],
				 &(pe[8]), &(pe[9]), &(pe[10]), &(pe[11]));

	  ay_trafo_pointstoplane(obj[0], obj[1], obj[2],
				 obj[12], obj[13], obj[14],
				 obj[6], obj[7], obj[8],
				 &(pe[12]), &(pe[13]), &(pe[14]), &(pe[15]));

	  ay_status = ay_pact_getpoint(2, o, pe);

	} /* if */

      if(ay_point_edit_coords)
	{
	  have_it = 0;
	  for(i = 0; i < ay_point_edit_coords_number; i++)
	    {
	      /* do we have that point selected already? */
	      point = o->selp;
	      last = NULL;
	      have_it = 0;
	      while(point)
		{
		  if(point->point == ay_point_edit_coords[i])
		    {
		      /* we have that point already, so we delete
			 it from the selection if we are not in
			 multiple selection mode */
		      if(!multiple)
			{
			  if(last)
			    last->next = point->next;
			  else
			    o->selp = point->next;

			  free(point);
			} /* if */
		      have_it = 1;
		      point = NULL; /* break loop */
		    } /* if */

		  last = point;
		  if(point)
		    point = point->next;
		} /* while */

	      if(!have_it)
		{
		  /* create new point object */
		  newp = NULL;
		  if(!(newp = calloc(1, sizeof(ay_point_object))))
		    {
		      ay_error(AY_EOMEM, fname, NULL);
		      return TCL_OK;
		    }

		  newp->next = o->selp;
		  o->selp = newp;
		  newp->point = ay_point_edit_coords[i];
		  newp->homogenous = ay_point_edit_coords_homogenous;
		}

	    } /* for */
	} /* if */

      sel = sel->next;
    } /* while */

 return TCL_OK;
} /* ay_pact_seltcb */


/* ay_pact_deseltcb
 *
 */
int
ay_pact_deseltcb(struct Togl *togl, int argc, char *argv[])
{
 ay_list_object *sel = ay_selection;

  while(sel)
    {
      
      ay_selp_clear(sel->object);

      sel = sel->next;
    } /* while */

 return TCL_OK;
} /* ay_pact_deseltcb */


/* ay_pact_flashpoint:
 *
 */
int
ay_pact_flashpoint(int ignore_old)
{
 int cont, penumber = ay_point_edit_coords_number;
 static int have_old_flashed_point = AY_FALSE;
 static double old_obj[3] = {0};

#ifdef GL_VERSION_1_1
  cont = AY_FALSE;
  if(have_old_flashed_point && ay_point_edit_coords)
    {
      if((ay_point_edit_coords[penumber-1][0] == old_obj[0]) &&
	 (ay_point_edit_coords[penumber-1][1] == old_obj[1]) &&
	 (ay_point_edit_coords[penumber-1][2] == old_obj[2]))
	{
	  cont = AY_TRUE;
	}
    }
  if(!cont || ignore_old)
    {
      glDrawBuffer(GL_FRONT);
      glEnable(GL_COLOR_LOGIC_OP);
      glLogicOp(GL_XOR);
      glColor3d((GLdouble)ay_prefs.sxr, (GLdouble)ay_prefs.sxg,
		(GLdouble)ay_prefs.sxb);
      glDisable(GL_DEPTH_TEST);
      glMatrixMode(GL_MODELVIEW);
      glPushMatrix();
       glLoadIdentity();
       ay_trafo_getall(ay_currentlevel->next);
       glBegin(GL_POINTS);
       /* clear old point? */
       if(have_old_flashed_point && !ignore_old)
	 {
	   glVertex3d(old_obj[0], old_obj[1], old_obj[2]);
	   have_old_flashed_point = AY_FALSE;
	 }
       /*
	 glEnd();
	 glFlush();
	 glBegin(GL_POINTS);
       */
       /* draw new point? */
       if(ay_point_edit_coords)
	 {
	   glVertex3d(ay_point_edit_coords[penumber-1][0],
		      ay_point_edit_coords[penumber-1][1],
		      ay_point_edit_coords[penumber-1][2]);
	   
	   old_obj[0] = ay_point_edit_coords[penumber-1][0];
	   old_obj[1] = ay_point_edit_coords[penumber-1][1];
	   old_obj[2] = ay_point_edit_coords[penumber-1][2];
	   have_old_flashed_point = AY_TRUE;
	 }
       else
	 {
	   have_old_flashed_point = AY_FALSE;
	 }
       glEnd();
      glPopMatrix();
      glEnable(GL_DEPTH_TEST);
      glDisable(GL_COLOR_LOGIC_OP);
      glDrawBuffer(GL_BACK);
    } /* if */
#endif
 return AY_OK;
} /* ay_pact_flashpoint */


/* ay_pact_startpetcb:
 *  prepares everything for the point editing
 *  modes
 */
int
ay_pact_startpetcb(struct Togl *togl, int argc, char *argv[])
{
 int ay_status = AY_OK;
 char fname[] = "pointEdit";
 Tcl_Interp *interp = Togl_Interp (togl);
 /*  ay_view_info *view = Togl_GetClientData(togl);*/
 double winX = 0.0, winY = 0.0;
 /*  double pickepsilon = ay_prefs.pick_epsilon;*/
 double obj[3] = {0};
 ay_list_object *sel = ay_selection;
 int penumber = 0, *tmpi;
 double **pecoords = NULL, **tmp = NULL;
 ay_object **tmpo = NULL;


  if(ay_pe_numcpo)
    free(ay_pe_numcpo);
  ay_pe_numcpo = NULL;

  if(ay_pe_homcpo)
    free(ay_pe_homcpo);
  ay_pe_homcpo = NULL;

  if(ay_pe_objects)
    free(ay_pe_objects);
  ay_pe_objects = NULL;

  ay_pe_objectslen = 0;

  Tcl_GetDouble(interp, argv[2], &winX);
  Tcl_GetDouble(interp, argv[3], &winY);

  while(sel)
    {
      ay_status = ay_viewt_wintoobj(togl, sel->object, winX, winY,
				    &(obj[0]), &(obj[1]), &(obj[2]));

      ay_status = ay_pact_getpoint(1, sel->object, obj);
      if(ay_point_edit_coords)
	{
	  if(!(tmp = realloc(pecoords, (ay_point_edit_coords_number +
					penumber)*sizeof(double*))))
	    {
	      ay_error(AY_EOMEM, fname, NULL);
	      free(pecoords);
	      return TCL_OK;
	    }
	  else
	    {
	      pecoords = tmp;
	      memcpy(&(pecoords[penumber]), ay_point_edit_coords,
		     ay_point_edit_coords_number*sizeof(double*));
	      penumber += ay_point_edit_coords_number;
	    }

	  ay_point_edit_object = sel->object;

	  /* remember number of picked points of current object */
	  if(!(tmpi = realloc(ay_pe_numcpo, (ay_pe_objectslen+1)*
			      sizeof(int))))
	    {
	      ay_error(AY_EOMEM, fname, NULL);
	      return TCL_OK;
	    }
	  else
	    {
	      ay_pe_numcpo = tmpi;
	      ay_pe_numcpo[ay_pe_objectslen] = ay_point_edit_coords_number;
	    }

	  /* remember homogenous state of current object */
	  if(!(tmpi = realloc(ay_pe_homcpo, (ay_pe_objectslen+1)*
			      sizeof(int))))
	    {
	      ay_error(AY_EOMEM, fname, NULL);
	      return TCL_OK;
	    }
	  else
	    {
	      ay_pe_homcpo = tmpi;
	      ay_pe_homcpo[ay_pe_objectslen] = ay_point_edit_coords_hom;
	    }

	  /* remember pointer to current object */
	  if(!(tmpo = realloc(ay_pe_objects, (ay_pe_objectslen+1)*
			      sizeof(ay_object*))))
	    {
	      ay_error(AY_EOMEM, fname, NULL);
	      return TCL_OK;
	    }
	  else
	    {
	      ay_pe_objects = tmpo;
	      ay_pe_objects[ay_pe_objectslen] = sel->object;
	    }

	  ay_pe_objectslen++;
	  
	}

      sel = sel->next;
    } /* while */

  ay_point_edit_coords_number = penumber;
  if(ay_point_edit_coords)
    free(ay_point_edit_coords);
  ay_point_edit_coords = pecoords;

  if(argc > 4)
    {
      if(argc > 5)
	ay_status = ay_pact_flashpoint(AY_TRUE);
      else
	ay_status = ay_pact_flashpoint(AY_FALSE);
    } /* if */

 return TCL_OK;
} /* ay_pact_startpetcb */


/* ay_pact_pedtcb:
 *  single point direct edit callback
 *
 */
int
ay_pact_pedtcb(struct Togl *togl, int argc, char *argv[])
{
 int ay_status = AY_OK;
 Tcl_Interp *interp = Togl_Interp (togl);
 ay_view_object *view = (ay_view_object *)Togl_GetClientData(togl);
 double winX = 0.0, winY = 0.0;
 double obj[3] = {0};
 char *n1 = "editPointDarray", fname[] = "editPointDirect";
 Tcl_Obj *to = NULL, *toa = NULL, *ton = NULL;
 int i, changed = AY_FALSE, local = 0;
 double *coords, wcoords[4];
 ay_list_object *sel = NULL;
 ay_object *o = NULL;


  sel = ay_selection;
  if(!sel)
    {
      ay_error(AY_ENOSEL, "pointEditD", NULL);
      return TCL_OK;
    }

  if(argc < 1)
    {
      return TCL_OK;
    }

  if(!strcmp(argv[2], "-start"))
    {
      o = sel->object;

      Tcl_GetDouble(interp, argv[3], &winX);
      Tcl_GetDouble(interp, argv[4], &winY);

      ay_status = ay_viewt_wintoobj(togl, o, winX, winY,
				    &(obj[0]), &(obj[1]), &(obj[2]));

      ay_status = ay_pact_getpoint(1, o, obj);

      ay_point_edit_object = o;

      if(ay_point_edit_coords)
	{
	  toa = Tcl_NewStringObj(n1, -1);
	  ton = Tcl_NewStringObj(n1, -1);

	  coords = ay_point_edit_coords[0];

	  Tcl_SetStringObj(ton,"lx",-1);
	  to = Tcl_NewDoubleObj(coords[0]);
	  Tcl_ObjSetVar2(interp, toa, ton, to,
			 TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

	  Tcl_SetStringObj(ton,"ly",-1);
	  to = Tcl_NewDoubleObj(coords[1]);
	  Tcl_ObjSetVar2(interp, toa, ton, to,
			 TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

	  Tcl_SetStringObj(ton,"lz",-1);
	  to = Tcl_NewDoubleObj(coords[2]);
	  Tcl_ObjSetVar2(interp, toa, ton, to,
			 TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

	  if(ay_point_edit_coords_homogenous)
	    {
	      Tcl_SetStringObj(ton,"lw",-1);
	      to = Tcl_NewDoubleObj(coords[3]);
	      Tcl_ObjSetVar2(interp, toa, ton, to,
			     TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
	    } /* if */


	  if(ay_point_edit_coords_homogenous)
	    {
	      memcpy(wcoords, coords, 4*sizeof(double));
	    }
	  else
	    {
	      memcpy(wcoords, coords, 3*sizeof(double));
	      wcoords[3] = 1.0;
	    }

	  ay_trafo_applyall(ay_currentlevel->next, o, wcoords);

	  Tcl_SetStringObj(ton,"wx",-1);
	  to = Tcl_NewDoubleObj(wcoords[0]);
	  Tcl_ObjSetVar2(interp, toa, ton, to,
			 TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

	  Tcl_SetStringObj(ton,"wy",-1);
	  to = Tcl_NewDoubleObj(wcoords[1]);
	  Tcl_ObjSetVar2(interp, toa, ton, to,
			 TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

	  Tcl_SetStringObj(ton,"wz",-1);
	  to = Tcl_NewDoubleObj(wcoords[2]);
	  Tcl_ObjSetVar2(interp, toa, ton, to,
			 TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

	  if(ay_point_edit_coords_homogenous)
	    {
	      Tcl_SetStringObj(ton,"ww",-1);
	      to = Tcl_NewDoubleObj(wcoords[3]);
	      Tcl_ObjSetVar2(interp, toa, ton, to,
			     TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
	    } /* if */

	  Tcl_SetStringObj(ton,"valid",-1);
	  to = Tcl_NewIntObj(1);
	  Tcl_ObjSetVar2(interp, toa, ton, to,
			 TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
	  Tcl_IncrRefCount(toa); Tcl_DecrRefCount(toa);
	  Tcl_IncrRefCount(ton); Tcl_DecrRefCount(ton);
	} /* if */
      return TCL_OK;
    } /* if */

  if(!strcmp(argv[2], "-apply"))
    {
      o = sel->object;

      toa = Tcl_NewStringObj(n1, -1);
      ton = Tcl_NewStringObj(n1, -1);
      Tcl_SetStringObj(ton,"changed",-1);
      to = Tcl_ObjGetVar2(interp, toa, ton,
			  TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
      Tcl_GetIntFromObj(interp, to, &changed);

      if(changed)
	{
	  if(!ay_point_edit_coords)
	    {
	      ay_error(AY_ERROR, fname, "Lost pointer to selected points!");
	      Tcl_IncrRefCount(toa); Tcl_DecrRefCount(toa);
	      Tcl_IncrRefCount(ton); Tcl_DecrRefCount(ton);
	      return TCL_OK;
	    } /* if */
	  
	  for(i = 0; i < ay_point_edit_coords_number; i++)
	    {
	      coords = ay_point_edit_coords[i];

	      Tcl_SetStringObj(ton,"x",-1);
	      to = Tcl_ObjGetVar2(interp, toa, ton,
				  TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
	      Tcl_GetDoubleFromObj(interp, to, &coords[0]);

	      Tcl_SetStringObj(ton,"y",-1);
	      to = Tcl_ObjGetVar2(interp, toa, ton,
				  TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
	      Tcl_GetDoubleFromObj(interp, to, &coords[1]);

	      Tcl_SetStringObj(ton,"z",-1);
	      to = Tcl_ObjGetVar2(interp, toa, ton,
				  TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
	      Tcl_GetDoubleFromObj(interp, to, &coords[2]);

	      if(ay_point_edit_coords_homogenous)
		{
		  Tcl_SetStringObj(ton,"w",-1);
		  to = Tcl_ObjGetVar2(interp, toa, ton,
				      TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
		  Tcl_GetDoubleFromObj(interp, to, &coords[3]);
		} /* if */

	      Tcl_SetStringObj(ton,"local",-1);
	      to = Tcl_ObjGetVar2(interp, toa, ton,
				  TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
	      Tcl_GetIntFromObj(interp, to, &local);

	      if(!local)
		{
		  ay_trafo_applyalli(ay_currentlevel->next, o, coords);
		}

	    } /* for */

	  ay_point_edit_object->modified = AY_TRUE;
	  ay_notify_force(ay_point_edit_object);
	  ay_status = ay_notify_parent();
	  view->drawmarker = AY_FALSE;
	} /* if */

      Tcl_IncrRefCount(toa); Tcl_DecrRefCount(toa);
      Tcl_IncrRefCount(ton); Tcl_DecrRefCount(ton);
      
      return TCL_OK;
    } /* if */

 return TCL_OK;
} /* ay_pact_pedtcb */


/* ay_pact_insertnc:
 *
 *
 */
int
ay_pact_insertnc(ay_nurbcurve_object *curve,
		 double objX, double objY, double objZ)
{

 int ay_status = AY_OK;
 char fname[] = "insert_pointnc";
 double *cv = NULL;
 int i = 0, j = 0, k = 0, index = -1;
 double min_distance = ay_prefs.pick_epsilon, distance = 0.0;
 double *newcontrolv = NULL, *oldcontrolv = NULL, *newknotv = NULL;
 int inserted, sections = 0, section;

 if(!curve)
   return AY_ENULL;

  cv = curve->controlv;

  if(min_distance == 0.0)
    min_distance = DBL_MAX;

  for(i = 0; i < curve->length; i++)
    {
      distance = AY_VLEN((objX - curve->controlv[j]),
			 (objY -  curve->controlv[j+1]),
			 (objZ -  curve->controlv[j+2]));

      if(distance < min_distance)
	{
	  index = i;
	  min_distance = distance;
	}

      j += 4;
    } /* for */

  /* no point picked? */
  if(index == -1)
    {
      return AY_OK;
    }

  if(curve->closed)
    {
      /*
       * if the curve is closed we simply insert the new point
       * in the controlvector between the picked point and the
       * next different
       */
      oldcontrolv = curve->controlv;

      if(index == curve->length-1)
	index--;

      curve->length++;
      if(!(newcontrolv = calloc(curve->length*4, sizeof(double))))
	{
	  curve->length--;
	  return AY_EOMEM;
	}

      j = 0;
      inserted = AY_FALSE;
      for(i = 0; i < (curve->length-1); i++)
	{
	  if(i >= index && !inserted)
	    {
	      memcpy(&(newcontrolv[j*4]), &(oldcontrolv[i*4]),
		     4*sizeof(double));
		  
	      if(oldcontrolv[i*4] != oldcontrolv[(i+1)*4] ||
		 oldcontrolv[i*4+1] != oldcontrolv[(i+1)*4+1] ||
		 oldcontrolv[i*4+2] != oldcontrolv[(i+1)*4+2])
		{
		  newcontrolv[(j+1)*4] = oldcontrolv[i*4] +
		    ((oldcontrolv[(i+1)*4] - oldcontrolv[i*4])/2.0);

		  newcontrolv[(j+1)*4+1] = oldcontrolv[i*4+1] +
		    ((oldcontrolv[(i+1)*4+1] - oldcontrolv[i*4+1])/2.0);

		  newcontrolv[(j+1)*4+2] = oldcontrolv[i*4+2] +
		    ((oldcontrolv[(i+1)*4+2] - oldcontrolv[i*4+2])/2.0);

		  newcontrolv[(j+1)*4+3] = oldcontrolv[i*4+3] +
		    ((oldcontrolv[(i+1)*4+3] - oldcontrolv[i*4+3])/2.0);

		  j++;
		  inserted = AY_TRUE;
		}

	    }
	  else
	    {
	      memcpy(&(newcontrolv[j*4]), &(oldcontrolv[i*4]),
		     4*sizeof(double));
	    }
	  j++;
	} /* for */

      if(!inserted)
	{
	  free(newcontrolv);
	  curve->length--;
	  ay_error(AY_ERROR, fname, "Cannot insert point here!");
	  return AY_ERROR;
	}

      free(curve->controlv);
      curve->controlv = newcontrolv;

      if(curve->knot_type == AY_KTCUSTOM)
	{
	  if(!(newknotv = calloc(curve->length+curve->order, sizeof(double))))
	    {
	      curve->length--;
	      return AY_EOMEM;
	    }

	  i = 0;
	  for(j = curve->order; j < curve->length-1; j++)
	    {
	      if(curve->knotv[j] != curve->knotv[j+1])
		{
		  sections++;
		}
	    }

	  section = sections*index/(curve->length-1);

	  k = 0; i = 0;
	  for(j = 0; j < curve->length+curve->order-1; j++)
	    {
	      
	      if((j >= curve->order-1) &&
		 (curve->knotv[j] != curve->knotv[j+1]))
		{
		  newknotv[k] = curve->knotv[j];
		  k++;
		  if(i == section)
		    {
		      newknotv[k] = curve->knotv[j]+
			((curve->knotv[j+1]-curve->knotv[j])/2.0);
		      k++;
		    }
		  i++;
		}
	      else
		{
		  newknotv[k] = curve->knotv[j];
		  k++;
		}
	      
	    } /* for */

	  free(curve->knotv);
	  curve->knotv = newknotv;

	  /*
	  ay_error(AY_EWARN,fname, "Changed knot type to B-Spline!");
	  curve->knot_type = AY_KTBSPLINE;
	  */
	}

      if(curve->knot_type != AY_KTCUSTOM)
	{
	  ay_status = ay_knots_createnc(curve);
	}
      ay_status = ay_nct_close(curve);
    }
  else
    { /* curve is not closed */

      curve->length++;
      if(!(newcontrolv = calloc(curve->length*4, sizeof(double))))
	{
	  curve->length--;
	  return AY_EOMEM;
	}

      j = 0; k = 0;
      for(i = 0; i < curve->length-1; i++)
	{
	  if(i == index)
	    {
	      /* insert point here */
	      memcpy(&newcontrolv[k],&curve->controlv[j],4*sizeof(double));

	      k += 4;
	      if(i == curve->length-2)
		{ /* the new point is the new last point */
		  newcontrolv[k] = curve->controlv[j] +
		    ((curve->controlv[j]-curve->controlv[j-4])/2.0);
		  newcontrolv[k+1] = curve->controlv[j+1]+
		    ((curve->controlv[j+1]-curve->controlv[j+1-4])/2.0);
		  newcontrolv[k+2] = curve->controlv[j+2]+
		    ((curve->controlv[j+2]-curve->controlv[j+2-4])/2.0);
		  newcontrolv[k+3] = curve->controlv[j+3]+
		    ((curve->controlv[j+3]-curve->controlv[j+3-4])/2.0);
		}
	      else
		{
		  newcontrolv[k] = curve->controlv[j] +
		    ((curve->controlv[j+4]-curve->controlv[j])/2.0);
		  newcontrolv[k+1] = curve->controlv[j+1]+
		    ((curve->controlv[j+1+4]-curve->controlv[j+1])/2.0);
		  newcontrolv[k+2] = curve->controlv[j+2]+
		    ((curve->controlv[j+2+4]-curve->controlv[j+2])/2.0);
		  newcontrolv[k+3] = curve->controlv[j+3]+
		    ((curve->controlv[j+3+4]-curve->controlv[j+3])/2.0);
		} /* if */
	    }
	  else
	    {
	      memcpy(&newcontrolv[k],&curve->controlv[j],4*sizeof(double));
	    } /* if */
		  
	  k += 4;
	  j += 4;
	} /* for */

      free(curve->controlv);
      curve->controlv = newcontrolv;

      if(curve->knot_type == AY_KTCUSTOM)
	{
	  if(!(newknotv = calloc(curve->length+curve->order, sizeof(double))))
	    {
	      curve->length--;
	      return AY_EOMEM;
	    }

	  /* count sections */
	  i = 0;
	  for(j = curve->order; j < curve->length-1; j++)
	    {
	      if(curve->knotv[j] != curve->knotv[j+1])
		{
		  sections++;
		}
	    }

	  section = sections*index/(curve->length-1);

	  k = 0; i = 0;
	  for(j = 0; j < curve->length+curve->order-2; j++)
	    {
	      
	      if((j >= curve->order-1) &&
		 (curve->knotv[j] != curve->knotv[j+1]))
		{
		  newknotv[k] = curve->knotv[j];
		  k++;
		  if(i == section)
		    {
		      newknotv[k] = curve->knotv[j]+
			((curve->knotv[j+1]-curve->knotv[j])/2.0);
		      k++;
		    }
		  i++;
		}
	      else
		{
		  newknotv[k] = curve->knotv[j];
		  k++;
		}
	      
	    } /* for */

	  newknotv[curve->length+curve->order-1] =
	    curve->knotv[curve->length+curve->order-2];

	  free(curve->knotv);
	  curve->knotv = newknotv;

	  /*	  
	    ay_error(AY_EWARN,fname, "Changed knot type to NURB!");
	    curve->knot_type = AY_KTNURB;
	  */
	} /* if */

      if(curve->knot_type != AY_KTCUSTOM)
	{
	  ay_status = ay_knots_createnc(curve);
	}
    } /* if */

  ay_status = ay_nct_recreatemp(curve);
       
  ay_status = ay_notify_parent();

 return ay_status;
} /* ay_pact_insertnc */


/* ay_pact_insertic:
 *
 *
 */
int
ay_pact_insertic(ay_icurve_object *curve,
		 double objX, double objY, double objZ)
{

 int ay_status = AY_OK;
 char fname[] = "insert_pointic";
 double *cv = NULL;
 int i = 0, j = 0, index = -1;
 double min_distance = ay_prefs.pick_epsilon, distance = 0.0;
 double *newcontrolv = NULL, *oldcontrolv = NULL;
 int inserted;

 if(!curve)
   return AY_ENULL;

  cv = curve->controlv;

  if(min_distance == 0.0)
    min_distance = DBL_MAX;

  for(i = 0; i < curve->length; i++)
    {
      distance = AY_VLEN((objX - curve->controlv[j]),
			 (objY - curve->controlv[j+1]),
			 (objZ - curve->controlv[j+2]));

      if(distance < min_distance)
	{
	  index = i;
	  min_distance = distance;
	}

      j += 3;
    } /* for */

  /* no point picked? */
  if(index == -1)
    {
      return AY_OK;
    }

  /*
   * we simply insert the new point in the controlvector
   * between the picked point and the next different
   */
  oldcontrolv = curve->controlv;

  if(!curve->closed)
    {
      if(index == curve->length-1)
	{
	  index--;
	}
    }
  else
    {
      if(index == curve->length-1)
	{
	  curve->length++;
	  if(!(newcontrolv = calloc(curve->length*3, sizeof(double))))
	    {
	      curve->length--;
	      return AY_EOMEM;
	    }
	  memcpy(newcontrolv, oldcontrolv,
		 (curve->length-1)*3*sizeof(double));
	  j = (curve->length-1)*3;
	  i = (curve->length-2)*3;
	  newcontrolv[j] = oldcontrolv[i] +
	    ((oldcontrolv[0] - oldcontrolv[i])/2.0);

	  newcontrolv[j+1] = oldcontrolv[i+1] +
	    ((oldcontrolv[1] - oldcontrolv[i+1])/2.0);

	  newcontrolv[j+2] = oldcontrolv[i+2] +
	    ((oldcontrolv[2] - oldcontrolv[i+2])/2.0);

	  free(curve->controlv);
	  curve->controlv = newcontrolv;
	  return ay_status; /* XXXX early exit */
	}

    }
  curve->length++;
  if(!(newcontrolv = calloc(curve->length*3, sizeof(double))))
    {
      curve->length--;
      return AY_EOMEM;
    }

  j = 0;
  inserted = AY_FALSE;
  for(i = 0; i < (curve->length-1); i++)
    {
      if(i >= index && !inserted)
	{
	  memcpy(&(newcontrolv[j*3]), &(oldcontrolv[i*3]),
		 3*sizeof(double));
		  
	  if(oldcontrolv[i*3] != oldcontrolv[(i+1)*3] ||
	     oldcontrolv[i*3+1] != oldcontrolv[(i+1)*3+1] ||
	     oldcontrolv[i*3+2] != oldcontrolv[(i+1)*3+2])
	    {
	      newcontrolv[(j+1)*3] = oldcontrolv[i*3] +
		((oldcontrolv[(i+1)*3] - oldcontrolv[i*3])/2.0);

	      newcontrolv[(j+1)*3+1] = oldcontrolv[i*3+1] +
		((oldcontrolv[(i+1)*3+1] - oldcontrolv[i*3+1])/2.0);

	      newcontrolv[(j+1)*3+2] = oldcontrolv[i*3+2] +
		((oldcontrolv[(i+1)*3+2] - oldcontrolv[i*3+2])/2.0);

	      newcontrolv[(j+1)*3+3] = oldcontrolv[i*3+3] +
		((oldcontrolv[(i+1)*3+3] - oldcontrolv[i*3+3])/2.0);

	      j++;
	      inserted = AY_TRUE;
	    }

	}
      else
	{
	  memcpy(&(newcontrolv[j*3]), &(oldcontrolv[i*3]),
		 3*sizeof(double));
	}
      j++;
    } /* for */

  if((curve->closed) && (index == curve->length-2))
    {

      inserted = AY_TRUE;
    }

  if(!inserted)
    {
      free(newcontrolv);
      curve->length--;
      ay_error(AY_ERROR, fname, "Cannot insert point here!");
      return AY_ERROR;
    }


  free(curve->controlv);
  curve->controlv = newcontrolv;

 return ay_status;
} /* ay_pact_insertic */


/* ay_pact_insertptcb:
 *  insert point callback
 *
 */
int
ay_pact_insertptcb(struct Togl *togl, int argc, char *argv[])
{
 int ay_status = AY_OK;
 Tcl_Interp *interp = Togl_Interp (togl);
 double winX = 0.0, winY = 0.0, objX = 0.0, objY = 0.0, objZ = 0.0;
 char fname[] = "insert_point";

  if(ay_selection)
    {
      ay_selp_clear(ay_selection->object);

      Tcl_GetDouble(interp, argv[2], &winX);
      Tcl_GetDouble(interp, argv[3], &winY);


      ay_status = ay_viewt_wintoobj(togl, ay_selection->object,
				    winX, winY,
				    &objX,&objY,&objZ);

      switch(ay_selection->object->type)
	{
	case AY_IDNCURVE:
	  ay_status = ay_pact_insertnc((ay_nurbcurve_object *)
				       (ay_selection->object->refine),
				       objX, objY, objZ);
	  if(ay_status)
	    ay_error(ay_status, fname, "Error while inserting point!");
	  break;
	case AY_IDICURVE:
	  ay_status = ay_pact_insertic((ay_icurve_object *)
				       (ay_selection->object->refine),
				       objX, objY, objZ);
	  if(ay_status)
	    ay_error(ay_status, fname, "Error while inserting point!");

	  ay_status = ay_notify_force(ay_selection->object);

	  break;
	default:
	  break;
	}

       ay_status = ay_notify_parent();
    }
  else
    ay_error(AY_ENOSEL, fname, NULL);

 return TCL_OK;
} /* ay_pact_insertptcb */


/* ay_pact_deletenc:
 *
 */
int
ay_pact_deletenc(ay_nurbcurve_object *curve,
		 double objX, double objY, double objZ)
{
 int ay_status = AY_OK;
 char fname[] = "delete_point";
 double *cv = NULL;
 int i=0, j=0, k=0, index = -1;
 double min_distance = ay_prefs.pick_epsilon, distance = 0.0;
 double *newcontrolv = NULL, *newknotv = NULL;


  if(!curve)
    return AY_ENULL;
  cv = curve->controlv;

  if(min_distance == 0.0)
    min_distance = DBL_MAX;

  index = -1;
  for(i = 0; i < curve->length; i++)
    {
      distance = AY_VLEN((objX - cv[j]),
			 (objY -  cv[j+1]),
			 (objZ -  cv[j+2]));

      if(distance < min_distance)
	{
	  index = i;
	  min_distance = distance;
	}

      j += 4;
    } /* for */

  if((index != -1) && (curve->length-1 < 2))
    {
      ay_error(AY_ERROR, fname, "need atleast two points");
      return TCL_OK;
    }

  if((curve->closed) &&
     (index > (curve->length-curve->order)))
    {
      index = curve->order-(curve->length-index)-1;
    }

  /* create new curve */
  if(index != -1)
    {
      curve->length--;
      if(!(newcontrolv = calloc(curve->length*4, sizeof(double))))
	return AY_EOMEM;

      /* copy controlv */
      j = 0;
      k = 0;
      for(i=0; i<=curve->length; i++)
	{
	  if(i != index)
	    {
	      newcontrolv[k] = curve->controlv[j];
	      newcontrolv[k+1] = curve->controlv[j+1];
	      newcontrolv[k+2] = curve->controlv[j+2];
	      newcontrolv[k+3] = curve->controlv[j+3];
	      k += 4;
	    }
		
	  j += 4;
	} /* for */

      free(curve->controlv);
      curve->controlv = newcontrolv;

      if(curve->knot_type == AY_KTBEZIER)
	curve->order--;

      if(curve->length < curve->order)
	curve->order = curve->length;

      /* generate new knots? */
      if(curve->knot_type == AY_KTCUSTOM)
	{
	  if(!(newknotv = calloc(curve->order+curve->length,
				 sizeof(double))))
	    { return AY_EOMEM; }

	  memcpy(newknotv, curve->knotv, (curve->length)*sizeof(double));
	  memcpy(&(newknotv[curve->length]),
		 &(curve->knotv[curve->length+1]),
		 curve->order*sizeof(double));
	  /*
	    for(i=0;i<curve->length+curve->order;i++)
	    {
	    newknotv[i] = curve->knotv[i];
	    }
	  */
	  free(curve->knotv);
	  curve->knotv = newknotv;
	}
      else
	{
	  ay_status = ay_knots_createnc(curve);
	}

    } /* if */

  if(curve->closed)
    {
      ay_status = ay_nct_close(curve);
      if(ay_status) 
	{
	  ay_error(ay_status, fname, "cannot close this curve");
	  curve->closed = AY_FALSE;
	}
    }
      
  ay_status = ay_nct_recreatemp(curve);

  ay_status = ay_notify_parent();

 return AY_OK;
} /* ay_pact_deletenc */


/* ay_pact_deleteic:
 *
 */
int
ay_pact_deleteic(ay_icurve_object *icurve,
		 double objX, double objY, double objZ)
{
 char fname[] = "delete_point";
 double *cv = NULL;
 int i=0, j=0, k=0, index = -1;
 double min_distance = ay_prefs.pick_epsilon, distance = 0.0;
 double *newcontrolv = NULL;


  if(!icurve)
    return AY_ENULL;

  cv = icurve->controlv;

  if(min_distance == 0.0)
    min_distance = DBL_MAX;

  index = -1;
  for(i = 0; i < icurve->length; i++)
    {
      distance = AY_VLEN((objX - cv[j]),
			 (objY -  cv[j+1]),
			 (objZ -  cv[j+2]));

      if(distance < min_distance)
	{
	  index = i;
	  min_distance = distance;
	}

      j += 3;
    } /* for */

  if((index != -1) && (icurve->length-1 < 3))
    {
      ay_error(AY_ERROR, fname, "need atleast three points");
      return TCL_OK;
    }

  /* create new icurve */
  if(index != -1)
    {
      icurve->length--;
      if(!(newcontrolv = calloc(icurve->length*3, sizeof(double))))
	return AY_EOMEM;

      /* copy controlv */
      j = 0;
      k = 0;
      for(i=0; i<=icurve->length; i++)
	{
	  if(i != index)
	    {
	      newcontrolv[k] = icurve->controlv[j];
	      newcontrolv[k+1] = icurve->controlv[j+1];
	      newcontrolv[k+2] = icurve->controlv[j+2];

	      k += 3;
	    }
		
	  j += 3;
	} /* for */

      free(icurve->controlv);
      icurve->controlv = newcontrolv;

   } /* if */

 return AY_OK;
} /* ay_pact_deleteic */


/* ay_pact_deleteptcb:
 *  delete point callback
 *
 */
int
ay_pact_deleteptcb(struct Togl *togl, int argc, char *argv[])
{
 int ay_status = AY_OK;
 Tcl_Interp *interp = Togl_Interp (togl);
 double winX = 0.0, winY = 0.0, objX = 0.0, objY = 0.0, objZ = 0.0;
 char fname[] = "delete_point";

  if(ay_selection)
    {
      ay_selp_clear(ay_selection->object);

      Tcl_GetDouble(interp, argv[2], &winX);
      Tcl_GetDouble(interp, argv[3], &winY);


      ay_status = ay_viewt_wintoobj(togl, ay_selection->object,
				    winX, winY,
				    &objX,&objY,&objZ);

      switch(ay_selection->object->type)
	{
	case AY_IDNCURVE:
	  ay_status = ay_pact_deletenc((ay_nurbcurve_object *)
				       (ay_selection->object->refine),
				       objX, objY, objZ);
	  if(ay_status)
	    ay_error(ay_status, fname, "Error while deleting point!");
	  break;
	case AY_IDICURVE:
	  ay_status = ay_pact_deleteic((ay_icurve_object *)
				       (ay_selection->object->refine),
				       objX, objY, objZ);
	  if(ay_status)
	    ay_error(ay_status, fname, "Error while deleting point!");

	  ay_status = ay_notify_force(ay_selection->object);

	  break;
	default:
	  break;
	}

       ay_status = ay_notify_parent();
    }
  else
    ay_error(AY_ENOSEL, fname, NULL);

 return TCL_OK;
} /* ay_pact_deleteptcb */


/* ay_pact_griddify:
 *  helper for ay_pact_petcb()
 *  snap value n to a grid sized by grid
 */
void
ay_pact_griddify(double *n, double grid)
{
 double m;

  m = fmod(*n, grid); 

  if(fabs(m) > 1.0e-05)
    {
      if(*n>0.0)
	{
	  if(m < (grid/2.0))
	    {
	      *n -= m;
	    }
	  else
	    {
	      *n += grid-m;
	    }
	}
      else
	{
	  if(m < -(grid/2.0))
	    {
	      *n -= grid+m;
	    }
	  else
	    {
	      *n -= m;
	    } /* if */
	} /* if */
    } /* if */

 return;
} /* ay_pact_griddify */


/* ay_pact_petcb:
 *  action callback for single point editing
 */
int
ay_pact_petcb(struct Togl *togl, int argc, char *argv[])
{
 int ay_status = AY_OK;
 char fname[] = "edit_points";
 Tcl_Interp *interp = Togl_Interp (togl);
 ay_view_object *view = (ay_view_object *)Togl_GetClientData(togl);
 /*ay_list_object *sel = ay_selection;*/
 static double oldwinx = 0.0, oldwiny = 0.0;
 double winx = 0.0, winy = 0.0;
 double movX, movY, movZ, dx=0.0, dy=0.0, dz=0.0, *coords;
 double euler[3] = {0};
 int i = 0, j, k = 0, redraw = AY_FALSE;
 static GLdouble m[16] = {0};
 /*GLdouble mo[16] = {0};*/
 ay_object *o = ay_point_edit_object;

  if(!o)
    return TCL_OK;

  for(j = 0; j < ay_pe_objectslen; j++)
    {
      o = ay_pe_objects[j];

      glMatrixMode(GL_MODELVIEW);
      glPushMatrix();

      glScaled (1.0/o->scalx, 1.0/o->scaly, 1.0/o->scalz);
      if(!view->aligned)
	{
	  ay_quat_toeuler(o->quat, euler);
	  glRotated(AY_R2D(euler[0]), 0.0, 0.0, 1.0);
	  glRotated(AY_R2D(euler[1]), 0.0, 1.0, 0.0);
	  glRotated(AY_R2D(euler[2]), 1.0, 0.0, 0.0);
	}
      glTranslated(-o->movx, -o->movy, -o->movz);

      if(!view->local)
	{
	  ay_trafo_getalli(ay_currentlevel->next);
	}
      else
	{
	  ay_trafo_getallis(ay_currentlevel->next);
	}

      glGetDoublev(GL_MODELVIEW_MATRIX, m);
      glPopMatrix();

      if(argc >= 4)
	{
	  if(!strcmp(argv[2],"-winxy"))
	    {
	      Tcl_GetDouble(interp, argv[3], &winx);
	      Tcl_GetDouble(interp, argv[4], &winy);
	      
	      if(view->usegrid)
		{
		  ay_viewt_griddify(togl, &winx, &winy);
		}
	    }
	  else
	    {
	      if(!strcmp(argv[2],"-start"))
		{
		  Tcl_GetDouble(interp, argv[3], &winx);
		  Tcl_GetDouble(interp, argv[4], &winy);

		  if(view->usegrid)
		    {
		      ay_viewt_griddify(togl, &winx, &winy);
		    }

		  oldwinx = winx;
		  oldwiny = winy;

		  /* snap selected points to grid coordinates */
		  if(ay_point_edit_coords)
		    {
		      if(view->usegrid && ay_prefs.edit_snaps_to_grid &&
			 (view->grid != 0.0))
			{
			  for(i = 0; i < ay_pe_numcpo[j]; i++)
			    {
			      coords = ay_point_edit_coords[k]; 
			      k++;
			      if(!view->local)
				{
				  ay_trafo_applyall(ay_currentlevel->next, o,
						    coords);
				} /* if */
			      if(ay_prefs.snap3d)
				{
				  ay_pact_griddify(&(coords[0]), view->grid);
				  ay_pact_griddify(&(coords[1]), view->grid);
				  ay_pact_griddify(&(coords[2]), view->grid);
				}
			      else
				{
				  switch(view->type)
				    {
				    case AY_VTFRONT:
				    case AY_VTTRIM:
				      ay_pact_griddify(&(coords[0]),
						       view->grid);
				      ay_pact_griddify(&(coords[1]),
						       view->grid);
				      break;
				    case AY_VTSIDE:
				      ay_pact_griddify(&(coords[1]),
						       view->grid);
				      ay_pact_griddify(&(coords[2]),
						       view->grid);
				      break;
				    case AY_VTTOP:
				      ay_pact_griddify(&(coords[0]),
						       view->grid);
				      ay_pact_griddify(&(coords[2]),
						       view->grid);
				      break;
				    default:
				      break;
				    } /* switch */
				} /* if */
			      if(!view->local)
				{
				  ay_trafo_applyalli(ay_currentlevel->next, o,
						     coords);
				} /* if */
			    } /* for */
			  /* XXXX this redraw is only necessary if coords are
			     really changed by the snap to grid operation */
			  ay_toglcb_display(togl);
			} /* if */
		    } /* if */
		  return TCL_OK;
		} /* if */
	    } /* if */
	}
      else
	{
	  ay_error(AY_EARGS, fname, NULL);
	  return TCL_OK;
	} /* if */

      dx = -(oldwinx - winx) * view->conv_x;
      dy = (oldwiny - winy) * view->conv_y;

      /* Side or Top view? */
      if(view->type == AY_VTSIDE)
	{
	  dz = -dx;
	  dx = 0.0;
	}

      if(view->type == AY_VTTOP)
	{
	  dx = dx;
	  dz = -dy;
	  dy = 0.0;
	}

      movX = m[0]*dx+m[4]*dy+m[8]*dz;
      movY = m[1]*dx+m[5]*dy+m[9]*dz;
      movZ = m[2]*dx+m[6]*dy+m[10]*dz;

      if(ay_point_edit_coords)
	{
	  for(i = 0; i < ay_pe_numcpo[j]; i++)
	    {
	      coords = ay_point_edit_coords[k]; 
	      k++;
	      coords[0] += movX;
	      coords[1] += movY;
	      coords[2] += movZ;
	    } /* for */

	  if((fabs(movX) > AY_EPSILON)||
	     (fabs(movY) > AY_EPSILON)||
	     (fabs(movZ) > AY_EPSILON))
	    {
	      o->modified = AY_TRUE;
	      ay_notify_force(o);
	      redraw = AY_TRUE;
	    } /* if */
	} /* if */
    } /* for */

  if(redraw)
    {
      if(!ay_prefs.lazynotify)
	{
	  ay_status = ay_notify_parent();
	} /* if */
      ay_toglcb_display(togl);
      ay_status = ay_pact_flashpoint(AY_TRUE);
    } /* if */

  oldwinx = winx;
  oldwiny = winy;

 return TCL_OK;
} /* ay_pact_petcb */


/* ay_pact_wetcb: 
 *  action callback for single point weight editing
 */
int
ay_pact_wetcb(struct Togl *togl, int argc, char *argv[])
{
 int ay_status = AY_OK;
 char fname[] = "weight_edit";
 Tcl_Interp *interp = Togl_Interp (togl);
 /*  ay_view_info *view = Togl_GetClientData(togl);*/
 double dx, winx = 0.0, new_weight, *coords;
 static double oldwinx = 0.0;
 static double olddx = 0.0;
 int i = 0;

  /* parse args */
  if(argc == 4)
    {
      if(!strcmp(argv[2],"-winx"))
	Tcl_GetDouble(interp, argv[3], &winx);
      else
	if(!strcmp(argv[2],"-start"))
	  {
	    Tcl_GetDouble(interp, argv[3], &winx);
	    oldwinx = winx;
	    olddx = 0.0;
	  }
    }
  else
    {
      ay_error(AY_EARGS, fname, NULL);
      return TCL_OK;
    }

  if(!ay_selection)
    return TCL_OK;

  dx = oldwinx - winx;
  oldwinx = winx;

  if(ay_point_edit_coords)
    {
      for(i = 0; i < ay_point_edit_coords_number; i++)
	{
	  coords = ay_point_edit_coords[i]; 
	  if(ay_point_edit_coords_homogenous)
	    {
	      new_weight = coords[3];
	      if(dx>0.0)
		{
		  new_weight *= 1.1;
		}
	      else
		{
		  new_weight *= 0.9;
		}
	      coords[3] = new_weight;
	    }
	  else
	    {
	      ay_error(AY_ERROR, fname, "Point is not homogenous!");
	    } /* if */
	} /* for */

      if(ay_point_edit_coords_homogenous)
	{
	  ay_point_edit_object->modified = AY_TRUE;
	  ay_notify_force(ay_selection->object);

	  if(!ay_prefs.lazynotify)
	    ay_status = ay_notify_parent();
	} /* if */
    } /* if */

 return TCL_OK;
} /* ay_pact_wetcb */


/* ay_pact_wrtcb: 
 *  this action callback resets all weights
 */
int
ay_pact_wrtcb(struct Togl *togl, int argc, char *argv[])
{
 int ay_status = AY_OK;
 char fname[] = "reset_weights";
 double p[3], *coords;
 int i;
 ay_object *o = NULL;
 ay_list_object *sel = ay_selection;


 while(sel)
   {
     o = sel->object;

     if(ay_point_edit_coords)
       {
	 free(ay_point_edit_coords);
	 ay_point_edit_coords = NULL;
       }

     ay_status = ay_pact_getpoint(0, o, p);

     if(ay_status)
       {
	 ay_error(AY_ERROR, fname, NULL);
       }

     if(ay_point_edit_coords && ay_point_edit_coords_homogenous)
       {
	 for(i = 0; i < ay_point_edit_coords_number; i++)
	   {
	     coords = ay_point_edit_coords[i]; 
	     coords[3] = 1.0;
	   } /* for */

	 o->modified = AY_TRUE;
	 ay_notify_force(o);
       }
     else
       {
	 ay_error(AY_ERROR, fname, "No points (with weights) found!");
       } /* if */

     sel = sel->next;
   } /* while */
 

  ay_status = ay_notify_parent();

 return TCL_OK;
} /* ay_pact_wrtcb */
