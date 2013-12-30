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

/* instance.c - instance object */

static char *ay_instance_name = "Instance";

int ay_instance_hasrptrafo(ay_object *o);

/* functions: */

/* ay_instance_createcb:
 *  create callback function of instance object
 */
int
ay_instance_createcb(int argc, char *argv[], ay_object *o)
{
 char fname[] = "instance_create";
 ay_list_object *sel = ay_selection;
 ay_object *target = NULL;

  if(!o)
    return AY_ENULL;

  if(!sel)
    return AY_ENOSEL;

  target = sel->object;

  if(target->type == AY_IDINSTANCE)
    {
      target = (ay_object *)target->refine;
    }

  /* XXXX create a blacklist data structure for this test */
  if((target->type == AY_IDROOT) || (target->type == AY_IDVIEW) ||
     (target->type == AY_IDLIGHT) || (target->type == AY_IDMATERIAL))
    {
      ay_error(AY_ERROR, fname,
	       "cannot create instance of this type of object");
      return AY_ERROR;
    }

  /* XXXX check for overflow */
  target->refcount++;

  o->refine = target;

 return AY_OK;
} /* ay_instance_createcb */


/* ay_instance_deletecb:
 *  delete callback function of instance object
 */
int
ay_instance_deletecb(void *c)
{
 ay_object *o = NULL;

  if(!c)
    return AY_OK;

  o = (ay_object *)(c);

  o->refcount--;

 return AY_OK;
} /* ay_instance_deletecb */


/* ay_instance_copycb:
 *  copy callback function of instance object
 */
int
ay_instance_copycb(void *src, void **dst)
{
 ay_object *target = NULL;

  target = (ay_object *)(src);

  /* XXXX check for overflow */
  target->refcount++;

  *dst = target;

 return AY_OK;
} /* ay_instance_copycb */


/* ay_instance_drawcb:
 *  draw (display in an Ayam view window) callback function of instance object
 */
int
ay_instance_drawcb(struct Togl *togl, ay_object *o)
{
 char fname[] = "instance_draw";
 int ay_status = AY_OK;
 ay_voidfp *arr = NULL;
 ay_object *m = NULL, *down = NULL;
 ay_drawcb *cb = NULL;
 double tm[16];

  if(!o)
    return AY_ENULL;

  m = (ay_object *)o->refine;

  glPushMatrix();

   if(o->tags && ay_instance_hasrptrafo(o))
     {
       glLoadIdentity();

       glTranslated((GLdouble)m->movx, (GLdouble)m->movy, (GLdouble)m->movz);
       ay_quat_torotmatrix(m->quat, tm);
       glMultMatrixd((GLdouble*)tm);
       glScaled((GLdouble)m->scalx, (GLdouble)m->scaly, (GLdouble)m->scalz);
     }

   /* draw the master */
   arr = ay_drawcbt.arr;
   cb = (ay_drawcb *)(arr[m->type]);

   if(cb)
     ay_status = cb(togl, m);

   if(ay_status)
     {
       ay_error(AY_ERROR, fname, "draw callback failed");

       glPopMatrix();
       return AY_ERROR;
     }

   /* draw the children of the master */
   if(m->down && m->down->next)
     {
       if(!m->inherit_trafos)
	 glLoadIdentity();

       if(!m->hide_children)
	 {
	   down = m->down;
	   while(down)
	     {
	       ay_draw_object(togl, down, AY_TRUE);
	       down = down->next;
	     } /* while */
	 }
     }

  glPopMatrix();

 return AY_OK;
} /* ay_instance_drawcb */


/* ay_instance_shadecb:
 *  shade (display in an Ayam view window) callback function of instance object
 */
int
ay_instance_shadecb(struct Togl *togl, ay_object *o)
{
 char fname[] = "instance_shade";
 int ay_status = AY_OK;
 ay_voidfp *arr = NULL;
 ay_object *m = NULL, *down = NULL;
 ay_drawcb *cb = NULL;
 double tm[16];

  if(!o)
    return AY_ENULL;

  m = (ay_object *)o->refine;

  glPushMatrix();

   if(o->tags && ay_instance_hasrptrafo(o))
     {
       glLoadIdentity();

       glTranslated((GLdouble)m->movx, (GLdouble)m->movy, (GLdouble)m->movz);
       ay_quat_torotmatrix(m->quat, tm);
       glMultMatrixd((GLdouble*)tm);
       glScaled((GLdouble)m->scalx, (GLdouble)m->scaly, (GLdouble)m->scalz);
     }

   /* shade the master */
   arr = ay_shadecbt.arr;
   cb = (ay_drawcb *)(arr[m->type]);

   if(cb)
     ay_status = cb(togl, m);

   if(ay_status)
     {
       ay_error(AY_ERROR, fname, "draw callback failed");

       glPopMatrix();
       return AY_ERROR;
     }

   /* shade the children of the master */
   if(m->down && m->down->next)
     {
       if(!m->inherit_trafos)
	 glLoadIdentity();

       if(!m->hide_children)
	 {
	   down = m->down;
	   while(down)
	     {
	       ay_shade_object(togl, down, AY_FALSE);
	       down = down->next;
	     } /* while */
	 }
     }

  glPopMatrix();

 return AY_OK;
} /* ay_instance_shadecb */


/* ay_instance_drawhcb:
 *  draw handles (in an Ayam view window) callback function of instance object
 */
int
ay_instance_drawhcb(struct Togl *togl, ay_object *o)
{
 char fname[] = "instance_drawh";
 int ay_status = AY_OK;
 ay_voidfp *arr = NULL;
 ay_object *m = NULL;
 ay_drawcb *cb = NULL;
 double tm[16];

  if(!o)
    return AY_ENULL;

  m = (ay_object *)o->refine;

  glPushMatrix();

   if(o->tags && ay_instance_hasrptrafo(o))
     {
       glLoadIdentity();

       glTranslated((GLdouble)m->movx, (GLdouble)m->movy, (GLdouble)m->movz);
       ay_quat_torotmatrix(m->quat, tm);
       glMultMatrixd((GLdouble*)tm);
       glScaled((GLdouble)m->scalx, (GLdouble)m->scaly, (GLdouble)m->scalz);
     }

   glColor3f((GLfloat)ay_prefs.obr, (GLfloat)ay_prefs.obg,
	     (GLfloat)ay_prefs.obb);

   arr = ay_drawhcbt.arr;
   cb = (ay_drawcb *)(arr[m->type]);

   if(cb)
     ay_status = cb(togl, m);

   if(ay_status)
     {
       ay_error(AY_ERROR, fname, "draw handles callback failed");

       glPopMatrix();
       return AY_ERROR;
     }

  glPopMatrix();

 return AY_OK;
} /* ay_instance_drawhcb */


/* ay_instance_getpntcb:
 *  get point (editing and selection) callback function of instance object
 */
int
ay_instance_getpntcb(int mode, ay_object *o, double *p, ay_pointedit *pe)
{
 int ay_status = AY_OK;
 ay_voidfp *arr = NULL;
 ay_object *m = NULL;
 ay_getpntcb *cb = NULL;
 ay_point *origselp = NULL;

  if(!o)
    return AY_ENULL;

  m = (ay_object *)o->refine;

  arr = ay_getpntcbt.arr;
  cb = (ay_getpntcb *)(arr[m->type]);

  if(cb)
    {
      /*
       * when repairing pointers from our selected points (mode 3)
       * temporarily replace the masters selp with our own so that
       * the getpnt callback does the right thing (repairing _our_ selps)
       */
      if(mode == 3 && o->selp)
	{
	  origselp = m->selp;
	  m->selp = o->selp;
	}

      ay_status = cb(mode, m, p, pe);

      if(mode == 3 && o->selp)
	{
	  m->selp = origselp;
	}

      if((!ay_status) && (mode != 3) && pe && pe->num)
	{
	  /* add NO tag to master, add NM tag to instance */
	  ay_status = ay_tags_addnonm(o, (ay_object*)o->refine);
	  if(ay_status)
	    {
	      ay_pact_clearpointedit(pe);
	    }
	  /* since we currently can not issue a notification on our
	     master (and its parents) in a meaningful way we rather
	     forbid point editing */
	  pe->readonly = AY_TRUE;
	} /* if */

      if(mode == 3 && !o->selp)
	{
	  /* if we get here, the point selection has been removed
	     => also clear the NO/MN tags */
	  ay_status = ay_tags_remnonm(o, (ay_object*)o->refine);
	}
    } /* if */

 return ay_status;
} /* ay_instance_getpntcb */


/* ay_instance_setpropcb:
 *  set property (from Tcl to C context) callback function of instance object
 */
int
ay_instance_setpropcb(Tcl_Interp *interp, int argc, char *argv[], ay_object *o)
{

  if(!o)
    return AY_ENULL;

 return AY_OK;
} /* ay_instance_setpropcb */


/* ay_instance_getpropcb:
 *  get property (from C to Tcl context) callback function of instance object
 */
int
ay_instance_getpropcb(Tcl_Interp *interp, int argc, char *argv[], ay_object *o)
{

  if(!o)
    return AY_ENULL;

 return AY_OK;
} /* ay_instance_getpropcb */


/* ay_instance_readcb:
 *  read (from scene file) callback function of instance object
 */
int
ay_instance_readcb(FILE *fileptr, ay_object *o)
{

 return AY_OK;
} /* ay_instance_readcb */


/* ay_instance_writecb:
 *  write (to scene file) callback function of instance object
 */
int
ay_instance_writecb(FILE *fileptr, ay_object *o)
{

 return AY_OK;
} /* ay_instance_writecb */


/* ay_instance_wribcb:
 *  RIB export callback function of instance object
 */
int
ay_instance_wribcb(char *file, ay_object *o)
{
 int ay_status = AY_OK;
 char *iafilename = NULL;
 int found = AY_FALSE;
 int down_is_prim = AY_FALSE;
 ay_object *orig = NULL, *down = NULL;
 ay_tag *tag = NULL;
 ay_voidfp *arr = NULL;
 ay_wribcb *cb = NULL;
 ay_level_object *l = NULL;

  orig = (ay_object *)o->refine;

  if(!orig)
    return AY_ERROR;

  arr = ay_wribcbt.arr;
  cb = (ay_wribcb *)(arr[orig->type]);

  if(cb)
    {
      if(!ay_prefs.resolveinstances)
	{
	  tag = orig->tags;
	  while(tag && !found)
	    {
	      if(tag->type == ay_oi_tagtype)
		{
		  if(!file)
		    {
		      RiReadArchive(tag->val,(RtVoid*)RI_NULL,RI_NULL);
		    }
		  else
		    {
		      iafilename = ay_wrib_geniafilename(file, tag->val);
		      if(iafilename)
			{
			  RiReadArchive(iafilename,(RtVoid*)RI_NULL,RI_NULL);
			  free(iafilename);
			}
		      else
			return AY_ERROR;
		    }
		  found = AY_TRUE;
		}
	      tag = tag->next;
	    }

	  if(!found)
	    return AY_ERROR; /* This should never happen! */

	  return AY_OK; /* XXXX early exit! */
	}
      else
	{
	  ay_status = cb(file, orig);
	}
    } /* if(cb) */

  if(orig->down && orig->down->next)
    {
      down = orig->down;
      l = NULL;
      if(orig->type == AY_IDLEVEL)
	{
	  l = (ay_level_object*)orig->refine;
	  switch(l->type)
	    {
	    case AY_LTUNION:
	      RiSolidBegin(RI_UNION);
	      break;
	    case AY_LTDIFF:
	      RiSolidBegin(RI_DIFFERENCE);
	      break;
	    case AY_LTINT:
	      RiSolidBegin(RI_INTERSECTION);
	      break;
	    case AY_LTPRIM:
	      if(!ay_current_primlevel)
		{
		  RiSolidBegin(RI_PRIMITIVE);
		}
	      ay_current_primlevel++;
	      break;
	    default:
	      break;
	    } /* switch */
	} /* if */

      while(down->next)
	{
	  down_is_prim = AY_FALSE;
	  if(ay_wrib_isprimitive(down))
	    {
	      down_is_prim = AY_TRUE;
	      if(l && ((l->type == AY_LTUNION) ||
		       (l->type == AY_LTDIFF) ||
		       (l->type == AY_LTINT)))
		{
		  if(!ay_current_primlevel)
		    {
		      RiSolidBegin(RI_PRIMITIVE);
		    }
		  ay_current_primlevel++;
		} /* if */
	    } /* if */

	  ay_status = ay_wrib_object(file, down);

	  if(down_is_prim)
	    {
	      if(l && ((l->type == AY_LTUNION) ||
		       (l->type == AY_LTDIFF) ||
		       (l->type == AY_LTINT)))
		{
		  ay_current_primlevel--;
		  if(!ay_current_primlevel)
		    {
		      RiSolidEnd();
		    }
		} /* if */
	    } /* if */

	  down = down->next;
	} /* while */

      if(l)
	{
	  if(l->type > 1)
	    {
	      if(l->type == AY_LTPRIM)
		{
		  ay_current_primlevel--;
		  if(!ay_current_primlevel)
		    {
		      RiSolidEnd();
		    }
		}
	      else
		{
		  RiSolidEnd();
		} /* if */
	    } /* if */
	} /* if */
    } /* if */

  /* if(orig->type == AY_OTLIGHT)
     ay_status = ay_wrib_lights(o, file, AY_FALSE);*/

 return ay_status;
} /* ay_instance_wribcb */


/* ay_instance_bbccb:
 *  bounding box calculation callback function of instance object
 */
int
ay_instance_bbccb(ay_object *o, double *bbox, int *flags)
{
 char fname[] = "instance_bbc";
 int ay_status = AY_OK;
 ay_voidfp *arr = NULL;
 ay_object *t = NULL, *d = NULL;
 ay_bbccb *cb = NULL;
 double m[16];
 double bbt[24] = {0};
 int i, a;
 double xmin = DBL_MAX, xmax = -DBL_MAX, ymin = DBL_MAX;
 double ymax = -DBL_MAX, zmin = DBL_MAX, zmax = -DBL_MAX;

  if(!o || !bbox || !flags)
    return AY_ENULL;

  t = (ay_object *)o->refine;

  /* get transformations */
  if(o->tags && ay_instance_hasrptrafo(o))
    ay_trafo_creatematrix(t, m);
  else
    ay_trafo_creatematrix(o, m);

  /* get bounding boxes of children of t (if any) */
  if(t->down)
    {

      d = t->down;
      while(d->next)
	{
	  ay_status = ay_bbc_get(d, bbt);
	  if(ay_status)
	    {
	      return ay_status;
	    }

	  /* apply transformations */
	  if(t->inherit_trafos)
	    {
	      a = 0;
	      for(i = 0; i < 8; i++)
		{
		  ay_trafo_apply3(&(bbt[a]), m);
		  a += 3;
		}
	    }

	  a = 0;
	  for(i = 0; i < 8; i++)
	    {
	      if(bbt[a] < xmin)
		xmin = bbt[a];
	      if(bbt[a] > xmax)
		xmax = bbt[a];
	      a += 3;
	    }

	  a = 1;
	  for(i = 0; i < 8; i++)
	    {
	      if(bbt[a] < ymin)
		ymin = bbt[a];
	      if(bbt[a] > ymax)
		ymax = bbt[a];
	      a += 3;
	    }

	  a = 2;
	  for(i = 0; i < 8; i++)
	    {
	      if(bbt[a] < zmin)
		zmin = bbt[a];
	      if(bbt[a] > zmax)
		zmax = bbt[a];
	      a += 3;
	    }

	  d = d->next;
	} /* while */
    } /* if */

  /* now get bb of t */
   arr = ay_bbccbt.arr;
   cb = (ay_bbccb *)(arr[t->type]);

   if(cb)
     ay_status = cb(t, bbt, flags);

   if(ay_status)
     {
       ay_error(AY_ERROR, fname, "bbc callback failed");
       return AY_ERROR;
     }

  if(*flags == 1)
    { /* exclusive bounding box, discard children bounding box */
      xmin = DBL_MAX; xmax = -DBL_MAX; ymin = DBL_MAX;
      ymax = -DBL_MAX; zmin = DBL_MAX; zmax = -DBL_MAX;
    }

  if(*flags != 2)
    { /* bounding box of object t is not marked invalid/non-existent */
      /* thus, merge bounding box of object t with children bounding box */

      /* apply transformations */
      a = 0;
      for(i = 0; i < 8; i++)
	{
	  ay_trafo_apply3(&(bbt[a]), m);
	  a += 3;
	}

      a = 0;
      for(i = 0; i < 8; i++)
	{
	  if(bbt[a] < xmin)
	    xmin = bbt[a];
	  if(bbt[a] > xmax)
	    xmax = bbt[a];
	  a += 3;
	}

      a = 1;
      for(i = 0; i < 8; i++)
	{
	  if(bbt[a] < ymin)
	    ymin = bbt[a];
	  if(bbt[a] > ymax)
	    ymax = bbt[a];
	  a += 3;
	}

      a = 2;
      for(i = 0; i < 8; i++)
	{
	  if(bbt[a] < zmin)
	    zmin = bbt[a];
	  if(bbt[a] > zmax)
	    zmax = bbt[a];
	  a += 3;
	}
    } /* if */

  /* fill in results */
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

  *flags = 3;

 return AY_OK;
} /* ay_instance_bbccb */


/* ay_instance_convertcb:
 *  convert callback function of instance object
 *  propagate changes to this function to ay_instt_resolve()!
 */
int
ay_instance_convertcb(ay_object *i, int in_place)
{
 int ay_status = AY_OK;
 char *iname = NULL;
 double movx, movy, movz;
 double rotx, roty, rotz;
 double scalx, scaly, scalz;
 double quat[4];
 ay_object *orig = NULL, *temp = NULL;
 ay_object *inext = NULL, *new = NULL;

  if(i->type != AY_IDINSTANCE)
    return AY_ERROR;

  orig = (ay_object *)i->refine;
  if(!orig)
    return AY_ERROR;

  if(in_place)
    {
      /* convert the instance to a normal object "in place" */
      movx = i->movx;
      movy = i->movy;
      movz = i->movz;

      rotx = i->rotx;
      roty = i->roty;
      rotz = i->rotz;

      scalx = i->scalx;
      scaly = i->scaly;
      scalz = i->scalz;

      quat[0] = i->quat[0];
      quat[1] = i->quat[1];
      quat[2] = i->quat[2];
      quat[3] = i->quat[3];

      inext = i->next;
      iname = i->name;

      if(i->selp)
	{
	  ay_selp_clear(i);
	  ay_tags_remnonm(i, orig);
	}

      if(i->tags)
	{
	  ay_tags_delall(i);
	}

      /* copy data from original object via temp object to instance object */
      ay_status = ay_object_copy(orig, &temp);
      if(ay_status || !temp)
	return ay_status;

      memcpy(i, temp, sizeof(ay_object));

      /* repair pointers */
      i->next = inext;
      i->name = iname;

      /* free temporary object */
      if(temp->name)
	free(temp->name);

      free(temp);

      /* use transformation attributes from instance, not from original */
      if(!i->tags || !ay_instance_hasrptrafo(i))
	{
	  i->movx = movx;
	  i->movy = movy;
	  i->movz = movz;

	  i->rotx = rotx;
	  i->roty = roty;
	  i->rotz = rotz;

	  i->scalx = scalx;
	  i->scaly = scaly;
	  i->scalz = scalz;

	  i->quat[0] = quat[0];
	  i->quat[1] = quat[1];
	  i->quat[2] = quat[2];
	  i->quat[3] = quat[3];
	}

      orig->refcount--;
    }
  else
    {
      /* create a copy of instanced object with transformation
	 attributes from the instance object */
      ay_status = ay_object_copy(orig, &new);
      if(new)
	{
	  if(!i->tags || !ay_instance_hasrptrafo(i))
	    {
	      ay_trafo_copy(i, new);
	    }
	  ay_object_link(new);
	} /* if */
    } /* if */

 return ay_status;
} /* ay_instance_convertcb */


/* ay_instance_providecb:
 *  provide callback function of instance object
 */
int
ay_instance_providecb(ay_object *o, unsigned int type, ay_object **result)
{
 int ay_status = AY_OK;
 ay_object *i = NULL;

  if(!o)
    return AY_ENULL;

  if(!result)
    {
      i = (ay_object *) o->refine;
      if(i->type == type)
	{
	  return AY_OK;
	}
      else
	{
	  return(ay_provide_object(i, type, NULL));
	}
    }

  i = (ay_object *) o->refine;

  if(i->type == type)
    {

      ay_status = ay_object_copy(i, result);
      if(!o->tags || !ay_instance_hasrptrafo(o))
	ay_trafo_copy(o, *result);

    }
  else
    {
      /*
	if object type of instanced object does not
	match the wanted type, we go the extra mile
	and try to get the wanted type of object
	from the instanced object instead
      */
      ay_status = ay_provide_object(i, type, result);

      if(*result)
	{
	  /*
	    we got it, copy transformation from instance
	    to result
	  */
	  if(!o->tags || !ay_instance_hasrptrafo(o))
	    ay_trafo_copy(o, *result);
	}
    } /* if */

 return ay_status;
} /* ay_instance_providecb */


/* ay_instance_hasrptrafo:
 *  has object <o> a RP tag for the transformations property
 */
int
ay_instance_hasrptrafo(ay_object *o)
{
 ay_tag *t = NULL;

  if(!o)
    return AY_FALSE;

  /* find RP tag */
  t = o->tags;
  while(t)
    {
      if(t->type == ay_rp_tagtype)
	{
	  if(t->val && strstr(t->val, "Transformations"))
	    {
	      return AY_TRUE;
	    }
	}
      t = t->next;
    }

 return AY_FALSE;
} /* ay_instance_hasrptrafo */


/* ay_instance_notifycb:
 *  notification callback function of instance object
 */
int
ay_instance_notifycb(ay_object *o)
{
 int ay_status = AY_OK;

  if(!o)
    return AY_OK;

  /* recover selected points */
  if(o->selp)
    {
      ay_instance_getpntcb(3, o, NULL, NULL);
    }

 return ay_status;
} /* ay_instance_notifycb */


/* ay_instance_init:
 *  initialize the instance object module
 */
int
ay_instance_init(Tcl_Interp *interp)
{
 int ay_status = AY_OK;

  ay_status = ay_otype_registercore(ay_instance_name,
				    ay_instance_createcb,
				    ay_instance_deletecb,
				    ay_instance_copycb,
				    ay_instance_drawcb,
				    ay_instance_drawhcb,
				    ay_instance_shadecb,
				    ay_instance_setpropcb,
				    ay_instance_getpropcb,
				    ay_instance_getpntcb,
				    ay_instance_readcb,
				    ay_instance_writecb,
				    ay_instance_wribcb,
				    ay_instance_bbccb,
				    AY_IDINSTANCE);


  ay_status += ay_notify_register(ay_instance_notifycb, AY_IDINSTANCE);

  ay_status += ay_convert_register(ay_instance_convertcb, AY_IDINSTANCE);

  ay_status += ay_provide_register(ay_instance_providecb, AY_IDINSTANCE);

  /* instances may not be associated with materials */
  ay_matt_nomaterial(AY_IDINSTANCE);

 return ay_status;
} /* ay_instance_init */
