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

/* ipatch.c -  Interpolating Patch object */

static char *ay_ipatch_name = "IPatch";

/* functions: */

/* ay_ipatch_createcb:
 *  create callback function of ipatch object
 */
int
ay_ipatch_createcb(int argc, char *argv[], ay_object *o)
{
 int ay_status = AY_OK;
 int tcl_status = TCL_OK;
 char fname[] = "crtipatch";
 char option_handled = AY_FALSE;
 int center = AY_FALSE, createmp = -1;
 int stride = 3, uorder = 4, vorder = 4, width = 4, height = 4;
 int ukt = AY_KTNURB, vkt = AY_KTNURB, optnum = 0, i = 2, j = 0, k = 0;
 int acvlen = 0;
 char **acv = NULL;
 double *cv = NULL;
 double udx = 0.25, udy = 0.0, udz = 0.0;
 double vdx = 0.0, vdy = 0.25, vdz = 0.0;
 double ext = 0.0, s[3] = {0};
 ay_ipatch_object *ip = NULL;

  if(!o)
    return AY_ENULL;

  /* parse args */
  while(i < argc)
    {
      if(i+1 >= argc)
	{
	  ay_error(AY_EOPT, fname, argv[i]);
	  ay_status = AY_ERROR;
	  goto cleanup;
	}

      tcl_status = TCL_OK;
      option_handled = AY_FALSE;
      optnum = i;
      if(argv[i] && argv[i][0] != '\0')
	{
	  switch(argv[i][1])
	    {
	    case 'w':
	      /* -width */
	      tcl_status = Tcl_GetInt(ay_interp, argv[i+1], &width);
	      option_handled = AY_TRUE;
	      break;
	    case 'h':
	      /* -height */
	      tcl_status = Tcl_GetInt(ay_interp, argv[i+1], &height);
	      option_handled = AY_TRUE;
	      break;
	    case 'u':
	      switch(argv[i][2])
		{
		case 'd':
		  switch(argv[i][3])
		    {
		    case 'x':
		      /* -udx */
		      tcl_status = Tcl_GetDouble(ay_interp, argv[i+1], &udx);
		      option_handled = AY_TRUE;
		      break;
		    case 'y':
		      /* -udy */
		      tcl_status = Tcl_GetDouble(ay_interp, argv[i+1], &udy);
		      option_handled = AY_TRUE;
		      break;
		    case 'z':
		      /* -udz */
		      tcl_status = Tcl_GetDouble(ay_interp, argv[i+1], &udz);
		      option_handled = AY_TRUE;
		      break;
		    default:
		      break;
		    } /* switch */
		  break;
		case 'o':
		  /* -uorder */
		  tcl_status = Tcl_GetInt(ay_interp, argv[i+1], &uorder);
		  option_handled = AY_TRUE;
		  break;
		case 'k':
		  /* -uktype */
		  tcl_status = Tcl_GetInt(ay_interp, argv[i+1], &ukt);
		  option_handled = AY_TRUE;
		  break;
		default:
		  break;
		} /* switch */
	      break;
	    case 'v':
	      switch(argv[i][2])
		{
		case 'd':
		  switch(argv[i][3])
		    {
		    case 'x':
		      /* -vdx */
		      tcl_status = Tcl_GetDouble(ay_interp, argv[i+1], &vdx);
		      option_handled = AY_TRUE;
		      break;
		    case 'y':
		      /* -vdy */
		      tcl_status = Tcl_GetDouble(ay_interp, argv[i+1], &vdy);
		      option_handled = AY_TRUE;
		      break;
		    case 'z':
		      /* -vdz */
		      tcl_status = Tcl_GetDouble(ay_interp, argv[i+1], &vdz);
		      option_handled = AY_TRUE;
		      break;
		    default:
		      break;
		    } /* switch */
		  break;
		case 'o':
		  /* -vorder */
		  tcl_status = Tcl_GetInt(ay_interp, argv[i+1], &vorder);
		  option_handled = AY_TRUE;
		  break;
		case 'k':
		  /* -vktype */
		  tcl_status = Tcl_GetInt(ay_interp, argv[i+1], &vkt);
		  option_handled = AY_TRUE;
		  break;
		default:
		  break;
		} /* switch */
	      break;
	    case 'c':
	      switch(argv[i][2])
		{
		case 'n':
		  /* -cn */
		  if(cv)
		    {
		      free(cv);
		      cv = NULL;
		    }
		  tcl_status = ay_tcmd_convdlist(argv[i+1], &acvlen, &cv);
		  option_handled = AY_TRUE;
		  break;
		case 'v':
		  /* -cv */
		  if(Tcl_SplitList(ay_interp, argv[i+1], &acvlen, &acv) ==
		     TCL_OK)
		    {
		      if(cv)
			{
			  free(cv);
			}
		      if(!(cv = calloc(acvlen, sizeof(double))))
			{
			  Tcl_Free((char *) acv);
			  ay_status = AY_EOMEM;
			  goto cleanup;
			}
		      for(j = 0; j < acvlen; j++)
			{
			  tcl_status = Tcl_GetDouble(ay_interp,
						     acv[j], &cv[j]);
			  if(tcl_status != TCL_OK)
			    {
			      break;
			    }
			} /* for */
		      Tcl_Free((char *) acv);
		    }
		  option_handled = AY_TRUE;
		  break;
		case 'e':
		  /* -center */
		  tcl_status = Tcl_GetInt(ay_interp, argv[i+1], &center);
		  option_handled = AY_TRUE;
		  break;
		case 'r':
		  /* -createmp */
		  tcl_status = Tcl_GetBoolean(ay_interp, argv[i+1], &createmp);
		  option_handled = AY_TRUE;
		  break;
		default:
		  break;
		} /* switch */
	      break;
	    default:
	      break;
	    } /* switch */

	  if(option_handled && (tcl_status != TCL_OK))
	    {
	      ay_error(AY_EOPT, fname, argv[i]);
	      ay_status = AY_ERROR;
	      goto cleanup;
	    }

	  i += 2;
	}
      else
	{
	  i++;
	} /* if */

      if(!option_handled)
	{
	  ay_error(AY_EUOPT, fname, argv[optnum]);
	  ay_status = AY_ERROR;
	  goto cleanup;
	}

    } /* while */

  if(uorder <= 0)
    {
      uorder = 4;
    }

  if(vorder <= 0)
    {
      vorder = 4;
    }

  if(width <= 1)
    {
      width = 4;
    }

  if(width < uorder)
    {
      uorder = width;
    }

  if(height <= 1)
    {
      height = 4;
    }

  if(height < vorder)
    {
      vorder = height;
    }

  if(cv)
    {
      /* check length of user provided control point array */
      if(acvlen/stride < width*height)
	{
	  if(acvlen>0)
	    s[0] = cv[0];
	  if(acvlen>1)
	    s[1] = cv[1];
	  if(acvlen>2)
	    s[2] = cv[2];

	  free(cv);
	  cv = NULL;
	  center = AY_FALSE;
	}
    } /* if */

  if(!(ip = calloc(1,sizeof(ay_ipatch_object))))
    {
      return AY_EOMEM;
    }

  ip->width = width;
  ip->height = height;
  ip->order_u = uorder;
  ip->order_v = vorder;

  if(!cv)
    {
      if(!(cv = calloc(width*height*stride,sizeof(double))))
	{
	  free(ip);
	  return AY_EOMEM;
	}

      if(center)
	{
	  if(fabs(udx) > AY_EPSILON)
	    ext = (width-1)*udx;
	  if(fabs(vdx) > AY_EPSILON)
	    ext += (height-1)*vdx;
	  s[0] = -(ext/2.0);
	  ext = 0.0;
	  if(fabs(udy) > AY_EPSILON)
	    ext = (width-1)*udy;
	  if(fabs(vdy) > AY_EPSILON)
	    ext += (height-1)*vdy;
	  s[1] = -(ext/2.0);
	  ext = 0.0;
	  if(fabs(udz) > AY_EPSILON)
	    ext = (width-1)*udz;
	  if(fabs(vdz) > AY_EPSILON)
	    ext += (height-1)*vdz;
	  s[2] = -(ext/2.0);
	}

      k = 0;
      for(i = 0; i < width; i++)
	{
	  for(j = 0; j < height; j++)
	    {
	      cv[k]   = s[0] + (double)j*vdx;
	      cv[k+1] = s[1] + (double)j*vdy;
	      cv[k+2] = s[2] + (double)j*vdz;
	      k += stride;
	    }
	  s[0] += udx;
	  s[1] += udy;
	  s[2] += udz;
	}
    } /* if */

  ip->controlv = cv;

  o->refine = (void *)ip;

  /* prevent cleanup code from doing something harmful */
  cv = NULL;

cleanup:

  if(cv)
    free(cv);

  if(ay_status == AY_EOMEM)
    {
      ay_error(AY_EOMEM, fname, NULL);
      ay_status = AY_ERROR;
    }

 return ay_status;
} /* ay_ipatch_createcb */


/* ay_ipatch_deletecb:
 *  delete callback function of ipatch object
 */
int
ay_ipatch_deletecb(void *c)
{
 ay_ipatch_object *ipatch = NULL;

  if(!c)
    return AY_ENULL;

  ipatch = (ay_ipatch_object *)(c);

  /* free controlv */
  if(ipatch->controlv)
    free(ipatch->controlv);

  /* free NURBS patch(es) */
  if(ipatch->npatch)
    ay_object_deletemulti(ipatch->npatch);

  free(ipatch);

 return AY_OK;
} /* ay_ipatch_deletecb */


/* ay_ipatch_copycb:
 *  copy callback function of ipatch object
 */
int
ay_ipatch_copycb(void *src, void **dst)
{
 ay_ipatch_object *ipatch = NULL, *ipatchsrc = NULL;

  if(!src || !dst)
    return AY_ENULL;

  ipatchsrc = (ay_ipatch_object *)src;

  if(!(ipatch = calloc(1, sizeof(ay_ipatch_object))))
    return AY_EOMEM;

  memcpy(ipatch, src, sizeof(ay_ipatch_object));

  ipatch->controlv = NULL;

  /* copy controlv */
  if(!(ipatch->controlv = calloc(3 * ipatch->width * ipatch->height,
				 sizeof(double))))
    {
      free(ipatch);
      return AY_EOMEM;
    }
  memcpy(ipatch->controlv, ipatchsrc->controlv,
	 3 * ipatch->width * ipatch->height * sizeof(double));

  /* copy NURBS patch(es) */
  if(ipatchsrc->npatch)
    {
      ay_object_copymulti(ipatchsrc->npatch, &(ipatch->npatch));
    }

  *dst = (void *)ipatch;

 return AY_OK;
} /* ay_ipatch_copycb */


/* ay_ipatch_drawcpcb:
 *  internal helper function
 *  draw the control polygon
 */
int
ay_ipatch_drawcpcb(struct Togl *togl, ay_object *o)
{
 ay_ipatch_object *ipatch = NULL;
 double *ver = NULL;
 int i, j, a, width, height;

  if(!o)
    return AY_ENULL;

  ipatch = (ay_ipatch_object *)o->refine;

  if(!ipatch)
    return AY_ENULL;

  width = ipatch->width;
  height = ipatch->height;

  ver = ipatch->controlv;

  a = 0;
  if(ipatch->close_v)
    {
      for(i = 0; i < width; i++)
	{
	  glBegin(GL_LINE_LOOP);
	   for(j = 0; j < height; j++)
	     {
	       glVertex3dv((GLdouble *)&ver[a]);
	       a += 3;
	     }
	  glEnd();
	}
    }
  else
    {
      for(i = 0; i < width; i++)
	{
	  glBegin(GL_LINE_STRIP);
	   for(j = 0; j < height; j++)
	     {
	       glVertex3dv((GLdouble *)&ver[a]);
	       a += 3;
	     } /* for */
	  glEnd();
	} /* for */
    } /* if */

  a = 0;
  if(ipatch->close_u)
    {
      for(j = 0; j < height; j++)
	{
	  a = j * 3;
	  glBegin(GL_LINE_LOOP);
	   for(i = 0; i < width; i++)
	     {
	       glVertex3dv((GLdouble *)&ver[a]);

	       a += (3 * height);
	     }
	  glEnd();
	}
    }
  else
    {
      for(j = 0; j < height; j++)
	{
	  a = j * 3;
	  glBegin(GL_LINE_STRIP);
	   for(i = 0; i < width; i++)
	     {
	       glVertex3dv((GLdouble *)&ver[a]);

	       a += (3 * height);
	     } /* for */
	  glEnd();
	} /* for */
    } /* if */

 return AY_OK;
} /* ay_ipatch_drawcpcb */


/* ay_ipatch_drawcb:
 *  draw (display in an Ayam view window) callback function of ipatch object
 */
int
ay_ipatch_drawcb(struct Togl *togl, ay_object *o)
{
 int display_mode = ay_prefs.np_display_mode;
 ay_ipatch_object *ipatch = NULL;
 ay_object *p = NULL;

  if(!o)
    return AY_ENULL;

  ipatch = (ay_ipatch_object *)o->refine;

  if(ipatch->display_mode != 0)
    {
      display_mode = ipatch->display_mode-1;
    }

  /* draw just the control polygon? */
  if(display_mode == 0)
    {
      /* Yes */
      ay_ipatch_drawcpcb(togl, o);
    }
  else
    {
      /* No, draw the NURBS patch(es), if present */
      p = ipatch->npatch;
      while(p)
	{
	  ay_draw_object(togl, p, AY_FALSE);
	  p = p->next;
	} /* while */
    } /* if */

 return AY_OK;
} /* ay_ipatch_drawcb */


/* ay_ipatch_shadecb:
 *  shade (display in an Ayam view window) callback function of ipatch object
 */
int
ay_ipatch_shadecb(struct Togl *togl, ay_object *o)
{
 ay_ipatch_object *ipatch = NULL;
 ay_object *p = NULL;

  ipatch = (ay_ipatch_object *)o->refine;

  p = ipatch->npatch;
  while(p)
    {
      ay_shade_object(togl, p, AY_FALSE);
      p = p->next;
    } /* while */

 return AY_OK;
} /* ay_ipatch_shadecb */


/* ay_ncurve_drawhcb:
 *  draw handles (in an Ayam view window) callback function of ipatch object
 */
int
ay_ipatch_drawhcb(struct Togl *togl, ay_object *o)
{
 int width = 0, height = 0, i = 0, a = 0;
 ay_ipatch_object *ipatch = (ay_ipatch_object *) o->refine;
 GLdouble *ver = NULL;
 double point_size = ay_prefs.handle_size;

  width = ipatch->width;
  height = ipatch->height;

  ver = ipatch->controlv;

  glPointSize((GLfloat)point_size);

  glBegin(GL_POINTS);
  for(i=0; i<(width*height); i++)
    {
      glVertex3dv((GLdouble *)&ver[a]);
      a += 3;
    }
  glEnd();

  /* draw arrows */
  ay_draw_arrow(togl, &(ver[width*height*3-6]), &(ver[width*height*3-3]));

 return AY_OK;
} /* ay_ipatch_drawhcb */


/* ay_ipatch_getpntcb:
 *  get point (editing and selection) callback function of ipatch object
 */
int
ay_ipatch_getpntcb(int mode, ay_object *o, double *p, ay_pointedit *pe)
{
 ay_ipatch_object *ipatch = NULL;
 ay_point *pnt = NULL, **lastpnt = NULL;
 double min_dist = ay_prefs.pick_epsilon, dist = 0.0;
 double *pecoord = NULL, **pecoords = NULL, *control = NULL, *c;
 int i = 0, j = 0, a = 0, found = AY_FALSE;

  if(!o || ((mode != 3) && (!p || !pe)))
    return AY_ENULL;

  ipatch = (ay_ipatch_object *)(o->refine);

  if(min_dist == 0.0)
    min_dist = DBL_MAX;

  if(pe)
    pe->rational = AY_TRUE;

  switch(mode)
    {
    case 0:
      /* select all points */
      if(!(pe->coords = calloc(ipatch->width * ipatch->height,
			       sizeof(double*))))
	return AY_EOMEM;

      for(i = 0; i < (ipatch->width*ipatch->height); i++)
	{
	  pe->coords[i] = &(ipatch->controlv[a]);
	  a += 3;
	}


      pe->num = ipatch->width * ipatch->height;

      break;
    case 1:
      /* selection based on a single point? */
      control = ipatch->controlv;

      for(i = 0; i < (ipatch->width * ipatch->height); i++)
	{
	  dist = AY_VLEN((p[0] - control[j]),
			 (p[1] - control[j+1]),
			 (p[2] - control[j+2]));

	  if(dist < min_dist)
	    {
	      pecoord = &(control[j]);
	      min_dist = dist;
	    }

	  j += 3;
	}

      if(!pecoord)
	return AY_OK; /* XXXX should this return a 'AY_EPICK' ? */

      if(!found)
	{

	  if(!(pe->coords = calloc(1, sizeof(double*))))
	    return AY_EOMEM;

	  pe->coords[0] = pecoord;
	  pe->num = 1;
	}
      break;
    case 2:
      /* selection based on planes */
      control = ipatch->controlv;
      j = 0;
      a = 0;
      for(i = 0; i < ipatch->width * ipatch->height; i++)
	{
	  c = &(control[j]);

	  /* test point c against the four planes in p */
	  if(((p[0]*c[0] + p[1]*c[1] + p[2]*c[2] + p[3]) < 0.0) &&
	     ((p[4]*c[0] + p[5]*c[1] + p[6]*c[2] + p[7]) < 0.0) &&
	     ((p[8]*c[0] + p[9]*c[1] + p[10]*c[2] + p[11]) < 0.0) &&
	     ((p[12]*c[0] + p[13]*c[1] + p[14]*c[2] + p[15]) < 0.0))
	    {

	      if(!(pecoords = realloc(pecoords, (a+1)*sizeof(double *))))
		return AY_EOMEM;
	      pecoords[a] = &(control[j]);
	      a++;
	    } /* if */

	  j += 3;
	} /* for */

      if(!pecoords)
	return AY_OK; /* XXXX should this return a 'AY_EPICK' ? */

      pe->coords = pecoords;
      pe->num = a;

      break;
    case 3:
      /* rebuild from o->selp */
      pnt = o->selp;
      lastpnt = &o->selp;
      while(pnt)
	{
	  if(pnt->index < (unsigned int)(ipatch->width * ipatch->height))
	    {
	      pnt->point = &(ipatch->controlv[pnt->index*3]);
	      pnt->rational = AY_TRUE;
	      lastpnt = &(pnt->next);
	      pnt = pnt->next;
	    }
	  else
	    {
	      *lastpnt = pnt->next;
	      free(pnt);
	      pnt = *lastpnt;
	    }
	} /* while */
      break;
    default:
      break;
    } /* switch */

 return AY_OK;
} /* ay_ipatch_getpntcb */


/* ay_ipatch_setpropcb:
 *  set property (from Tcl to C context) callback function of ipatch object
 */
int
ay_ipatch_setpropcb(Tcl_Interp *interp, int argc, char *argv[], ay_object *o)
{
 int ay_status = AY_OK;
 char *n1 = "IPatchAttrData";
 char fname[] = "ipatch_setpropcb";
 Tcl_Obj *to = NULL, *toa = NULL, *ton = NULL;
 ay_ipatch_object *ipatch = NULL;
 int new_ktype_u, new_close_u, new_width;
 int new_ktype_v, new_close_v, new_height;
 int update = AY_FALSE;


  if(!o)
    return AY_ENULL;

  ipatch = (ay_ipatch_object *)o->refine;

  toa = Tcl_NewStringObj(n1,-1);
  ton = Tcl_NewStringObj(n1,-1);

  Tcl_SetStringObj(ton,"Width",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp,to, &new_width);

  Tcl_SetStringObj(ton,"Height",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp,to, &new_height);

  Tcl_SetStringObj(ton,"Close_U",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp,to, &new_close_u);

  Tcl_SetStringObj(ton,"Close_V",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp,to, &new_close_v);

  Tcl_SetStringObj(ton,"Order_U",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp,to, &new_close_u);

  Tcl_SetStringObj(ton,"Order_V",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp,to, &new_close_v);

  Tcl_SetStringObj(ton,"Knot-Type_U",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp,to, &new_ktype_u);

  Tcl_SetStringObj(ton,"Knot-Type_V",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp,to, &new_ktype_v);

  ipatch->ktype_u = new_ktype_u;
  ipatch->ktype_v = new_ktype_v;

  ipatch->close_u = new_close_u;
  ipatch->close_v = new_close_v;

  Tcl_SetStringObj(ton,"Tolerance",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetDoubleFromObj(interp,to, &(ipatch->glu_sampling_tolerance));

  Tcl_SetStringObj(ton,"DisplayMode",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp,to, &(ipatch->display_mode));


  Tcl_IncrRefCount(toa);Tcl_DecrRefCount(toa);
  Tcl_IncrRefCount(ton);Tcl_DecrRefCount(ton);


  /* apply changed values to patch */

  /* resize patch */
  if(new_width != ipatch->width && (new_width > 1))
    {
      if(o->selp)
	{
	  ay_selp_clear(o);
	}

      ay_status = ay_npt_resizearrayw(&(ipatch->controlv), 3,
				      ipatch->width, ipatch->height,
				      new_width);

      if(ay_status)
	ay_error(AY_ERROR,fname,"Could not resize patch!");
      else
	ipatch->width = new_width;
    } /* if */

  if(new_height != ipatch->height && (new_height > 1))
    {
      if(o->selp)
	{
	  ay_selp_clear(o);
	}

      ay_status = ay_npt_resizearrayh(&(ipatch->controlv), 3,
				      ipatch->width, ipatch->height,
				      new_height);

      if(ay_status)
	ay_error(AY_ERROR,fname,"Could not resize patch!");
      else
	ipatch->height = new_height;
    } /* if */

  if(update)
    {
      ay_notify_force(o);

      o->modified = AY_TRUE;

      ay_notify_parent();
    }

 return AY_OK;
} /* ay_ipatch_setpropcb */


/* ay_ipatch_getpropcb:
 *  get property (from C to Tcl context) callback function of ipatch object
 */
int
ay_ipatch_getpropcb(Tcl_Interp *interp, int argc, char *argv[], ay_object *o)
{
 char *n1="IPatchAttrData";
 Tcl_Obj *to = NULL, *toa = NULL, *ton = NULL;
 ay_ipatch_object *ipatch = NULL;

  if(!o)
    return AY_ENULL;

  ipatch = (ay_ipatch_object *)(o->refine);

  toa = Tcl_NewStringObj(n1,-1);

  ton = Tcl_NewStringObj(n1,-1);


  Tcl_SetStringObj(ton,"Width",-1);
  to = Tcl_NewIntObj(ipatch->width);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton,"Height",-1);
  to = Tcl_NewIntObj(ipatch->height);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton,"Close_U",-1);
  to = Tcl_NewIntObj(ipatch->close_u);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton,"Close_V",-1);
  to = Tcl_NewIntObj(ipatch->close_v);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton,"Order_U",-1);
  to = Tcl_NewIntObj(ipatch->order_u);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton,"Order_V",-1);
  to = Tcl_NewIntObj(ipatch->order_v);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton,"Knot-Type_U",-1);
  to = Tcl_NewIntObj(ipatch->ktype_u);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);
  Tcl_SetStringObj(ton,"Knot-Type_V",-1);
  to = Tcl_NewIntObj(ipatch->ktype_v);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton,"Tolerance",-1);
  to = Tcl_NewDoubleObj(ipatch->glu_sampling_tolerance);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton,"DisplayMode",-1);
  to = Tcl_NewIntObj(ipatch->display_mode);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);

  ay_prop_getnpinfo(interp, n1, ipatch->npatch);

  Tcl_IncrRefCount(toa);Tcl_DecrRefCount(toa);
  Tcl_IncrRefCount(ton);Tcl_DecrRefCount(ton);

 return AY_OK;
} /* ay_ipatch_getpropcb */


/* ay_ipatch_readcb:
 *  read (from scene file) callback function of ipatch object
 */
int
ay_ipatch_readcb(FILE *fileptr, ay_object *o)
{
 ay_ipatch_object *ipatch = NULL;
 int i, a;

 if(!o)
   return AY_ENULL;

  if(!(ipatch = calloc(1, sizeof(ay_ipatch_object))))
    { return AY_EOMEM; }

  fscanf(fileptr,"%d\n",&ipatch->width);
  fscanf(fileptr,"%d\n",&ipatch->height);
  fscanf(fileptr,"%d\n",&ipatch->close_u);
  fscanf(fileptr,"%d\n",&ipatch->close_v);
  fscanf(fileptr,"%d\n",&ipatch->order_u);
  fscanf(fileptr,"%d\n",&ipatch->order_v);
  fscanf(fileptr,"%d\n",&ipatch->ktype_u);
  fscanf(fileptr,"%d\n",&ipatch->ktype_v);

  if(!(ipatch->controlv = calloc(ipatch->width*ipatch->height*3,
				 sizeof(double))))
    {
      free(ipatch);
      return AY_EOMEM;
    }

  a = 0;
  for(i = 0; i < ipatch->width*ipatch->height; i++)
    {
      fscanf(fileptr,"%lg %lg %lg\n",&(ipatch->controlv[a]),
	     &(ipatch->controlv[a+1]),
	     &(ipatch->controlv[a+2]));
      a += 3;
    }

  fscanf(fileptr,"%lg\n",&(ipatch->glu_sampling_tolerance));
  fscanf(fileptr,"%d\n",&(ipatch->display_mode));

  o->refine = ipatch;

 return AY_OK;
} /* ay_ipatch_readcb */


/* ay_ipatch_writecb:
 *  write (to scene file) callback function of ipatch object
 */
int
ay_ipatch_writecb(FILE *fileptr, ay_object *o)
{
 ay_ipatch_object *ipatch = NULL;
 int i, a;

  if(!o)
    return AY_ENULL;

  ipatch = (ay_ipatch_object *)(o->refine);

  fprintf(fileptr, "%d\n", ipatch->width);
  fprintf(fileptr, "%d\n", ipatch->height);
  fprintf(fileptr, "%d\n", ipatch->close_u);
  fprintf(fileptr, "%d\n", ipatch->close_v);
  fprintf(fileptr, "%d\n", ipatch->order_u);
  fprintf(fileptr, "%d\n", ipatch->order_v);
  fprintf(fileptr, "%d\n", ipatch->ktype_u);
  fprintf(fileptr, "%d\n", ipatch->ktype_v);

  a = 0;
  for(i = 0; i < ipatch->width*ipatch->height; i++)
    {
      fprintf(fileptr,"%g %g %g\n", ipatch->controlv[a],
	      ipatch->controlv[a+1],
	      ipatch->controlv[a+2]);
      a += 3;
    }

  fprintf(fileptr, "%g\n", ipatch->glu_sampling_tolerance);
  fprintf(fileptr, "%d\n", ipatch->display_mode);

 return AY_OK;
} /* ay_ipatch_writecb */


/* ay_ipatch_wribcb:
 *  RIB export callback function of ipatch object
 */
int
ay_ipatch_wribcb(char *file, ay_object *o)
{
 int ay_status = AY_OK;
 ay_ipatch_object *ipatch = NULL;
 ay_object *p;

  if(!o)
    return AY_OK;

  ipatch = (ay_ipatch_object*)(o->refine);
  p = ipatch->npatch;
  while(p)
    {
      ay_wrib_object(file, p);
      p = p->next;
    }

 return ay_status;
} /* ay_ipatch_wribcb */


/* ay_ipatch_bbccb:
 *  bounding box calculation callback function of ipatch object
 */
int
ay_ipatch_bbccb(ay_object *o, double *bbox, int *flags)
{
 double xmin, xmax, ymin, ymax, zmin, zmax;
 double *controlv = NULL;
 int i, a;
 ay_ipatch_object *ipatch = NULL;

  if(!o || !bbox || !flags)
    return AY_ENULL;

  ipatch = (ay_ipatch_object *)o->refine;

  controlv = ipatch->controlv;

  xmin = controlv[0];
  xmax = xmin;
  ymin = controlv[1];
  ymax = ymin;
  zmin = controlv[2];
  zmax = zmin;

  a = 0;
  for(i = 0; i < ipatch->width*ipatch->height; i++)
    {
      if(controlv[a] < xmin)
	xmin = controlv[a];
      if(controlv[a] > xmax)
	xmax = controlv[a];

      if(controlv[a+1] < ymin)
	ymin = controlv[a+1];
      if(controlv[a+1] > ymax)
	ymax = controlv[a+1];

      if(controlv[a+2] < zmin)
	zmin = controlv[a+2];
      if(controlv[a+2] > zmax)
	zmax = controlv[a+2];

      a += 3;
    }

  /* P1 */
  bbox[0] = xmin; bbox[1] = ymax; bbox[2] = zmax;
  /* P2 */
  bbox[3] = xmin; bbox[4] = ymax; bbox[5] = zmin;
  /* P3 */
  bbox[6] = xmax; bbox[7] = ymax; bbox[8] = zmin;
  /* P4 */
  bbox[9] = xmax; bbox[10] = ymax; bbox[11] = zmax;

  /* P5 */
  bbox[12] = xmin; bbox[13] = ymin; bbox[14] = zmax;
  /* P6 */
  bbox[15] = xmin; bbox[16] = ymin; bbox[17] = zmin;
  /* P7 */
  bbox[18] = xmax; bbox[19] = ymin; bbox[20] = zmin;
  /* P8 */
  bbox[21] = xmax; bbox[22] = ymin; bbox[23] = zmax;

 return AY_OK;
} /* ay_ipatch_bbccb */


/* ay_ipatch_notifycb:
 *  notification callback function of ipatch object
 */
int
ay_ipatch_notifycb(ay_object *o)
{
 int ay_status = AY_OK;
 int i, j, a, b;
 ay_object *p = NULL;
 ay_ipatch_object *ip = NULL;
 ay_nurbpatch_object *np = NULL;
 double *cv = NULL, *icv = NULL, *npcv = NULL;

  if(!o)
    return AY_ENULL;

  ip = (ay_ipatch_object *)o->refine;

  if(ip->npatch)
    {
      ay_object_deletemulti(ip->npatch);
      ip->npatch = NULL;
    }

  if(!(cv = calloc(ip->width*ip->height*4, sizeof(double))))
    return AY_EOMEM;

  a = 0;
  b = 0;
  for(i = 0; i < ip->width; i++)
    {
      for(j = 0; j < ip->height; j++)
	{
	  memcpy(&(cv[a]), &(ip->controlv[b]),
		 3*sizeof(double));

	  cv[a+3] = 1.0;

	  a += 4;
	  b += 3;
	}
    }

  ay_npt_create(ip->order_u, ip->order_v, ip->width, ip->height,
		AY_KTCUSTOM, AY_KTCUSTOM, cv, NULL, NULL, &np);

  ay_npt_createnpatchobject(&p);
  p->refine = (void*)np;

  if(ip->width > 2)
    {
      ay_status = ay_npt_interpolateu(np, ip->order_u);

      if(ay_status)
	goto cleanup;
    } /* if*/


  if(ip->height > 2)
    {
     ay_status = ay_npt_interpolatev(np, ip->order_v);

      if(ay_status)
	goto cleanup;
    } /* if*/

  ip->npatch = p;

  /* prevent cleanup code from doing something harmful */
  p = NULL;

cleanup:

  if(p)
    ay_object_deletemulti(p);

 return ay_status;
} /* ay_ipatch_notifycb */


/* ay_ipatch_providecb:
 *  provide callback function of ipatch object
 */
int
ay_ipatch_providecb(ay_object *o, unsigned int type, ay_object **result)
{
 int ay_status = AY_OK;
 char fname[] = "ipatch_providecb";
 ay_ipatch_object *pm = NULL;
 ay_object *new = NULL, **t = NULL, *p = NULL;

  if(!o)
    return AY_ENULL;

  if(!result)
    {
      if(type == AY_IDNPATCH)
	return AY_OK;
      else
	return AY_ERROR;
    }

  pm = (ay_ipatch_object *) o->refine;

  if(type == AY_IDNPATCH)
    {
      t = &(new);

      if(!pm->npatch)
	return AY_ERROR;

      p = pm->npatch;
      while(p)
	{
	  ay_status = ay_object_copy(p, t);
	  if(ay_status)
	    {
	      ay_error(ay_status, fname, NULL);
	      return AY_ERROR;
	    }
	  ay_trafo_copy(o, *t);
	  t = &((*t)->next);
	  p = p->next;
	} /* while */

      /* copy eventually present TP tags */
      ay_npt_copytptag(o, new);

      *result = new;
    } /* if */

 return ay_status;
} /* ay_ipatch_providecb */


/* ay_ipatch_convertcb:
 *  convert callback function of ipatch object
 */
int
ay_ipatch_convertcb(ay_object *o, int in_place)
{
 int ay_status = AY_OK;
 ay_ipatch_object *ipatch = NULL;
 ay_level_object *level = NULL;
 ay_object *l = NULL, *p = NULL, *new = NULL, **next = NULL;

  if(!o)
    return AY_ENULL;

  ipatch = (ay_ipatch_object *) o->refine;
  p = ipatch->npatch;
  if(!in_place)
    {
      while(p)
	{
	  ay_status = ay_object_copy(p, &new);
	  if(new)
	    {
	      ay_trafo_copy(o, new);

	      /* copy eventually present TP tags */
	      ay_npt_copytptag(o, new);

	      ay_status = ay_object_link(new);
	    } /* if */
	  p = p->next;
	} /* while */
    }
  else
    {
      ay_status = ay_object_create(AY_IDLEVEL, &l);
      level = (ay_level_object *)(l->refine);
      level->type = AY_LTLEVEL;
      next = &(l->down);

      while(p)
	{
	  ay_status = ay_object_copy(p, &new);
	  if(new)
	    {
	      ay_trafo_copy(o, new);

	      /* copy eventually present TP tags */
	      ay_npt_copytptag(o, new);

	      *next = new;
	      next = &(new->next);
	    } /* if */
	  p = p->next;
	} /* while */

      if(new)
	{
	  *next = ay_endlevel;
	  ay_object_replace(l, o);
	}
      else
	{
	  ay_object_delete(l);
	} /* if */
    } /* if */

 return ay_status;
} /* ay_ipatch_convertcb */


/* ay_ipatch_init:
 *  initialize the ipatch object module
 */
int
ay_ipatch_init(Tcl_Interp *interp)
{
 int ay_status = AY_OK;

  ay_status = ay_otype_registercore(ay_ipatch_name,
				    ay_ipatch_createcb,
				    ay_ipatch_deletecb,
				    ay_ipatch_copycb,
				    ay_ipatch_drawcb,
				    ay_ipatch_drawhcb,
				    ay_ipatch_shadecb,
				    ay_ipatch_setpropcb,
				    ay_ipatch_getpropcb,
				    ay_ipatch_getpntcb,
				    ay_ipatch_readcb,
				    ay_ipatch_writecb,
				    ay_ipatch_wribcb,
				    ay_ipatch_bbccb,
				    AY_IDIPATCH);

  ay_status = ay_notify_register(ay_ipatch_notifycb, AY_IDIPATCH);

  ay_status = ay_convert_register(ay_ipatch_convertcb, AY_IDIPATCH);

  ay_status = ay_provide_register(ay_ipatch_providecb, AY_IDIPATCH);

 return ay_status;
} /* ay_ipatch_init */

