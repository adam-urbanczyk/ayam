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

/* instance.c - instance object */

static char *ay_instance_name = "Instance";


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

  /* XXXX create a blacklist data structure for this test */
  if((target->type == AY_IDROOT) || (target->type == AY_IDVIEW) ||
     (target->type == AY_IDLIGHT) || (target->type == AY_IDINSTANCE)
     || (target->type == AY_IDMATERIAL))
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


int
ay_instance_drawcb(struct Togl *togl, ay_object *o)
{
 char fname[] = "instance_draw";
 int ay_status = AY_OK;
 void **arr = NULL;
 ay_object *t = NULL, *down = NULL;
 ay_drawcb *cb = NULL;

  if(!o)
    return AY_ENULL;

  t = (ay_object *)o->refine;

  glPushMatrix();

   arr = ay_drawcbt.arr;
   cb = (ay_drawcb *)(arr[t->type]);

   if(cb)
     ay_status = cb(togl, t);

   if(ay_status)
     {
       ay_error(AY_ERROR, fname, "draw callback failed");
       
       glPopMatrix();
       return AY_ERROR;
     }

   if(!t->inherit_trafos)
     glLoadIdentity();

   if(!t->hide_children)
     {
       down = t->down;
       while(down)
	 {
	   ay_status = ay_draw_object(togl, down, AY_TRUE);
	   down = down->next;
	 } /* while */
     }

  glPopMatrix();

 return AY_OK;
} /* ay_instance_drawcb */


int
ay_instance_drawhcb(struct Togl *togl, ay_object *o)
{
  if(!o)
    return AY_ENULL;

 return AY_OK;
} /* ay_instance_drawhcb */


int
ay_instance_shadecb(struct Togl *togl, ay_object *o)
{
 char fname[] = "instance_shade";
 int ay_status = AY_OK;
 void **arr = NULL;
 ay_object *t = NULL, *down = NULL;
 ay_drawcb *cb = NULL;

  if(!o)
    return AY_ENULL;

  t = (ay_object *)o->refine;

  glPushMatrix();

   arr = ay_shadecbt.arr;
   cb = (ay_drawcb *)(arr[t->type]);

   if(cb)
     ay_status = cb(togl, t);

   if(ay_status)
     {
       ay_error(AY_ERROR, fname, "draw callback failed");
       
       glPopMatrix();
       return AY_ERROR;
     }

   if(!t->inherit_trafos)
     glLoadIdentity();

   if(!t->hide_children)
     {
       down = t->down;
       while(down)
	 {
	   ay_status = ay_shade_object(togl, down, AY_FALSE);
	   down = down->next;
	 } /* while */
     }

  glPopMatrix();

 return AY_OK;
} /* ay_instance_shadecb */


int
ay_instance_getpntcb(int mode, ay_object *o, double *p)
{

  if(!o)
    return AY_ENULL;

 return AY_OK;
} /* ay_instance_getpntcb */


/* Tcl -> C! */
int
ay_instance_setpropcb(Tcl_Interp *interp, int argc, char *argv[], ay_object *o)
{

  if(!o)
    return AY_ENULL;

 return AY_OK;
} /* ay_instance_setpropcb */


/* C -> Tcl! */
int
ay_instance_getpropcb(Tcl_Interp *interp, int argc, char *argv[], ay_object *o)
{

  if(!o)
    return AY_ENULL;

 return AY_OK;
} /* ay_instance_getpropcb */


int
ay_instance_readcb(FILE *fileptr, ay_object *o)
{

 return AY_OK;
} /* ay_instance_readcb */


int
ay_instance_writecb(FILE *fileptr, ay_object *o)
{

 return AY_OK;
} /* ay_instance_writecb */


int
ay_instance_wribcb(char *file, ay_object *o)
{
 int ay_status = AY_OK;
 char *iafilename = NULL;
 int found = AY_FALSE;
 ay_object *orig = NULL, *down = NULL;
 ay_tag_object *tag = NULL;
 void **arr = NULL;
 ay_wribcb *cb = NULL;
 ay_level_object *l = NULL, *ld = NULL;

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
	      if(tag->type == ay_instt_oitagtype)
		{
		  if(!file)
		    {
		      RiReadArchive(tag->val,(RtVoid*)RI_NULL,RI_NULL);
		    }
		  else
		    {
		      if(!(iafilename = calloc(1,
					   strlen(tag->val)+strlen(file)+2)))
			return AY_EOMEM;
		      strcpy(iafilename,file);
		      iafilename[strlen(file)] = '-';
		      strcpy(&(iafilename[strlen(file)+1]),tag->val);
		      RiReadArchive(iafilename,(RtVoid*)RI_NULL,RI_NULL);
		      free(iafilename);
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
	  ld = NULL;
	  if(down->type == AY_IDLEVEL)
	    ld = (ay_level_object*)down->refine;

	  if(!ld || (ld && (ld->type != AY_LTUNION) &&
		     (ld->type != AY_LTDIFF) &&
		     (ld->type != AY_LTINT)))
	    {
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

	  if(!ld || (ld && (ld->type != AY_LTUNION) &&
		     (ld->type != AY_LTDIFF) &&
		     (ld->type != AY_LTINT)))
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


int
ay_instance_bbccb(ay_object *o, double *bbox, int *flags)
{
 char fname[] = "instance_bbc";
 int ay_status = AY_OK;
 void **arr = NULL;
 ay_object *t = NULL, *d = NULL;
 ay_bbccb *cb = NULL;
 double m[16];
 double bbt[24] = {0};
 int i, a;
 double xmin = DBL_MAX, xmax = -DBL_MAX, ymin = DBL_MAX;
 double ymax = -DBL_MAX, zmin = DBL_MAX, zmax = -DBL_MAX;

  if(!o || !bbox || !flags)
    return AY_ENULL;

  /* get transformations of o */
  ay_trafo_creatematrix(o, m);

  t = (ay_object *)o->refine;

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
 * propagate changes to this function to ay_instt_resolve()!
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
 ay_object *inext = NULL;

  if(i->type != AY_IDINSTANCE)
    return AY_ERROR;

  orig = (ay_object *)i->refine;
  if(!orig)
    return AY_ERROR;


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

  /* free some data that may be accidentally attached to the
     instance object (by errors in other parts of the core?) */
  if(i->selp)
    ay_selp_clear(i);

  ay_tags_delall(i);

  /* copy data from original object via temp object to instance object */
  ay_status = ay_object_copy(orig, &temp);
  if(ay_status)
    return ay_status;

  memcpy(i, temp, sizeof(ay_object));

  /* repair pointers */
  i->next = inext;
  i->name = iname;

  /* free temporary object */
  if(temp->name)
    free(temp->name);
  if(temp)
    free(temp);

  /* use transformation attributes from instance, not from original */
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

  orig->refcount--;

 return ay_status;
} /* ay_instance_convertcb */


int
ay_instance_providecb(ay_object *o, unsigned int type, ay_object **result)
{
 int ay_status = AY_OK;
 ay_object *i = NULL;

  if(!o || !result)
    return AY_ENULL;

  i = (ay_object *) o->refine;

  if(i->type == type)
    {

      ay_status = ay_object_copy(i, result);
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
	  ay_trafo_copy(o, *result);
	}
    }

 return ay_status;
} /* ay_instance_providecb */


int
ay_instance_init(Tcl_Interp *interp)
{
 int ay_status = AY_OK;

  ay_status = ay_otype_registercore(ay_instance_name,
				    ay_instance_createcb,
				    ay_instance_deletecb,
				    ay_instance_copycb,
				    ay_instance_drawcb,
				    NULL, /* no handles! */
				    ay_instance_shadecb,
				    ay_instance_setpropcb,
				    ay_instance_getpropcb,
				    NULL, /* no picking! */
				    ay_instance_readcb,
				    ay_instance_writecb,
				    ay_instance_wribcb,
				    ay_instance_bbccb,
				    AY_IDINSTANCE);


  ay_status = ay_convert_register(ay_instance_convertcb, AY_IDINSTANCE);

  ay_status = ay_provide_register(ay_instance_providecb, AY_IDINSTANCE);

  ay_matt_nomaterial(AY_IDINSTANCE);

 return ay_status;
} /* ay_instance_init */
