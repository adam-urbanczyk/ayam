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

/* idr.c - Importance Driven Rendering for Ayam */

/* initial implementation for The Mops by H. Birkholz */
/* adapted for Ayam by R. Schultz */

#include "ayam.h"
#include "tiffio.h"

char idr_ay_version_ma[] = AYVERSIONSTR;
char idr_ay_version_mi[] = AYVERSIONSTRMI;

/* IDR tag type (visible to the user) */
static char *idr_idrtagtype;
static char *idr_idrtagname = "IDR";

/* internal IDR tag type used to specify Importance */
static char *idr_iidrtagtype;
static char *idr_iidrtagname = "IIDR";

/* internal IDR tag type used to specify important regions in image space */
static char *idr_ridrtagtype;
static char *idr_ridrtagname = "RIDR";

/* internal IDR tag type used to specify important regions in object space */
static char *idr_r3idrtagtype;
static char *idr_r3idrtagname = "R3IDR";


/* internal IDR tag type used to mark changed objects */
static char *idr_cidrtagtype;
static char *idr_cidrtagname = "CIDR";

/* internal IDR tag type used to mark objects in "check change" buffer */
static char *idr_ccidrtagtype;
static char *idr_ccidrtagname = "CCIDR";

/* functions local to this module */
int idr_clear_importance(ay_object *o);
int idr_copy_importance(ay_object *o, char *from_type);

/* list structure for picture parts */
typedef struct idr_picpart_s {
  struct idr_picpart_s *next;
  int left, top, right, bottom;
  char *RIBFile;
  char *ImageFile;
  int alpha; /* alpha value for this part, 0 for object masks */
  uint32 *rgba_result;  /* resulting image data */
} idr_picpart;

idr_picpart *idr_partlist = NULL, **idr_partlist_next;

/* global variables */

uint32 *idr_picture_buf = NULL;  /* the result picture is stored here */

int idr_picture_width, idr_picture_height;  /* dimension of target picture */

int idr_qlevels; /* how many quality/importance levels are to be rendered? */

char *idr_base; /* basename for all idr related temporary files */

static ay_drawcb *idr_root_drawcb = NULL;

char idr_window_path[256];

int weight_r;  /* one start of rendrib costs as much time as
		  rendering weight_r pixels; set via tcl-interface  */

typedef struct idr_param_s {
  double importance;
  int halfres;
  int exclude;
  int optimizebb;
} idr_param;

/* changed objects buffer */
static ay_object *idr_selected = NULL;


/* prototypes */
int idr_get2dbbc(ay_object *o, int *left, int *right,
		 int *bottom, int *top);

/* functions */


/* idr_save_selected:
 *
 */
int
idr_save_selected()
{
 ay_list_object *sel = ay_selection;
 ay_tag_object *t = NULL, *tccidr = NULL;
 ay_object *o = NULL, *n = NULL, **p = NULL;
 int i = 0;
 int ay_status = AY_OK;

  o = idr_selected;

  while(o)
    {
      n = o->next;
      ay_object_delete(o);
      o = n;
    }

  idr_selected = NULL;
  p = &(idr_selected);
  while(sel)
    {
      o = sel->object;

      if(o->type != AY_IDROOT &&
	 o->type != AY_IDVIEW)
	{
	  ay_status = AY_OK;
	  ay_status = ay_object_copy(o, p);
	  if(!ay_status)
	    {
	      n = *p;
	      p = &(n->next);

	      tccidr = NULL;
	      t = o->tags;
	      while(t)
		{
		  if(t->type == idr_ccidrtagtype)
		    {
		      tccidr = t;
		      t = NULL;
		    }
		  else
		    {
		      t = t->next;
		    } /* if */
		}

	      if(tccidr)
		{
		  sprintf(tccidr->val, "%d", i);
		}
	      else
		{
		  t = NULL;
		  /* create new tag */
		  if(!(t = calloc(1, sizeof(ay_tag_object))))
		    return AY_EOMEM;

		  /* fill new tag */
		  if(!(t->name = calloc(4, sizeof(char))))
		    return AY_EOMEM;
		  strcpy(t->name, idr_ccidrtagname);
		  t->type = idr_ccidrtagtype;

		  if(!(t->val = calloc(64, sizeof(char))))
		    return AY_EOMEM;

		  sprintf(t->val, "%d", i);
	  
		  /* link new tag */
		  t->next = o->tags;
		  o->tags = t;
		} /* if */
	    }
	  else
	    {
	      *p = NULL;
	    } /* if */
	} /* if */
      i++;
      sel = sel->next;
    }

 return AY_OK;
} /* idr_save_selected */


/* idr_assign_impsel:
 *
 */
int
idr_assign_impsel()
{
 ay_list_object *sel = ay_selection;
 ay_tag_object *t = NULL, *told = NULL, *tnew = NULL;
 ay_object *o = NULL;

  while(sel)
    {
      o = sel->object;
      t = o->tags;
      /* find old IIDR tag */
      while(t)
	{
	  if(t->type == idr_iidrtagtype)
	    {
	      told = t;
	      t = NULL;
	    }
	  else
	    {
	      t = t->next;
	    }
	}

      if(told)
	{
	  /* set new tag value */
	  if(told->val)
	    {
	      free(told->val);
	      told->val = NULL;
	    }

	  if(!(told->val = calloc(64, sizeof(char))))
	    return AY_EOMEM;

	  sprintf(told->val, "%g", 1.0);
	  
	}
      else
	{
	  /* create new tag */
	  if(!(tnew = calloc(1, sizeof(ay_tag_object))))
	    return AY_EOMEM;

	  /* fill new tag */
	  if(!(tnew->name = calloc(4, sizeof(char))))
	    return AY_EOMEM;
	  strcpy(tnew->name, idr_iidrtagname);
	  tnew->type = idr_iidrtagtype;

	  if(!(tnew->val = calloc(64, sizeof(char))))
	    return AY_EOMEM;

	  sprintf(tnew->val, "%g", 1.0);
	  
	  /* link new tag */
	  tnew->next = o->tags;
	  o->tags = tnew;
	}

      sel = sel->next;
    }

 return AY_OK;
} /* idr_assign_impsel */


/* idr_clear_changed:
 *  clear internal (changed) importance tags from scene below o
 */
int
idr_clear_changed(ay_object *o)
{
 int ay_status = AY_OK;
 ay_object *down = NULL;
 ay_tag_object *t = NULL, **tprev = NULL;

 if(o)
   {
     if(o->down)
       {
	 down = o->down;
	 while(down->next)
	   {
	     ay_status = idr_clear_changed(down);
	     down = down->next;
	   }
       }
     
     /* find tag */
     t = o->tags;
     tprev = &(o->tags);
     while(t)
       {
	 if(t->type == idr_cidrtagtype)
	   {
	     /* delete found tag */
	     if(t->val)
	       {
		 free(t->val);
	       }
	     if(t->name)
	       {
		 free(t->name);
	       }
	     *tprev = t->next;
	     free(t);
	     t = NULL;
	   }
	 else
	   {
	     tprev = &(t->next);
	     t = t->next;
	   } /* if */
       } /* while */

   } /* if */

 return AY_OK;
} /* idr_clear_changed */


/* idr_assign_impchanged:
 *  assign importance to changed objects
 */
int
idr_assign_impchanged()
{
 ay_tag_object *t = NULL, *told = NULL, *tnew = NULL;
 ay_list_object *sel = ay_selection;
 ay_object *o = NULL, *s = NULL;
 int changed = AY_FALSE;
 int i = 0, j = 0;

  if(!idr_selected)
    return AY_OK;

  while(sel)
    {
      o = sel->object;

     /* find CCIDR tag, to get the right index of copied object
	in save selected buffer */
      t = o->tags;
      while(t)
	{
	  if(t->type == idr_ccidrtagtype)
	    {
	      sscanf(t->val, "%d", &i);
	    }
	  t = t->next;
	} /* while */

      /* get i'th object from save selected buffer */
      s = idr_selected;
      for(j = 0; j < i; j++)
	{
	  s = s->next;
	  if(!s)
	    {
	      break;
	    }
	} /* for */

      if(!s)
	{
	  return AY_OK;
	}

      /* sanity check */
      if(o->type == s->type)
	{
	  if(!ay_comp_trafos(o, s))
	    {
	      changed = AY_TRUE;
	    }
	  else
	    {
	      if(!ay_comp_objects(o, s))
		{
		  changed = AY_TRUE;
		}
	    }
	}
      else
	{
	  /* oops? wrong index? */
	  return AY_OK;
	}

      if(changed)
	{
	  t = o->tags;
	  /* find old CIDR tag */
	  while(t)
	    {
	      if(t->type == idr_cidrtagtype)
		{
		  told = t;
		  t = NULL;
		}
	      else
		{
		  t = t->next;
		}
	    } /* while */

	  if(told)
	    {
	      /* set new tag value */
	      if(told->val)
		{
		  free(told->val);
		  told->val = NULL;
		}

	      if(!(told->val = calloc(64, sizeof(char))))
		return AY_EOMEM;

	      sprintf(told->val, "%g", 1.0);
	 
	    }
	  else
	    {
	      /* create new tag */
	      if(!(tnew = calloc(1, sizeof(ay_tag_object))))
		return AY_EOMEM;

	      /* fill new tag */
	      if(!(tnew->name = calloc(4, sizeof(char))))
		return AY_EOMEM;
	      strcpy(tnew->name, idr_cidrtagname);
	      tnew->type = idr_cidrtagtype;

	      if(!(tnew->val = calloc(64, sizeof(char))))
		return AY_EOMEM;
	      
	      sprintf(tnew->val, "%g", 1.0);
	  
	      /* link new tag */
	      tnew->next = o->tags;
	      o->tags = tnew;
	    } /* if */
	} /* if */

      sel = sel->next;
    } /* while */

 return AY_OK;
} /* idr_assign_impchanged */


/* idr_3dreg_topart:
 *  
 */
int
idr_3dreg_topart(ay_object *o, ay_view_object *view, idr_picpart **partlist)
{
 int ay_status = AY_OK;
 double bb[24] = {0};
 ay_tag_object *t = NULL;
 int left, top, right, bottom;
 idr_picpart *part = NULL;
 double i = 0.0;
 int width, height;
 struct Togl *togl = NULL;

  if(!o)
    {
      return AY_ENULL;
    }

  togl = view->togl;
  width = Togl_Width (togl);
  height = Togl_Height (togl);
  /* find 3DREG tag of this object */
  t = o->tags;
  while(t)
    {
      if(t->type == idr_r3idrtagtype)
	{
	  sscanf(t->val, "%lg", &i);
	  t = NULL;
	}
      else
	{
	  t = t->next;
	}
    }

  ay_status = ay_bbc_get(o, bb);

  if(ay_status)
    {
      return ay_status;
    }
  idr_get2dbbc(o, &left, &right, &bottom, &top);

  left -= width/2;
  if(left < -width/2)
    left = -width/2;

  right -= width/2;
  if(right > width/2)
    right = width/2;

  bottom -= height/2;
  if(bottom < -height/2)
    bottom = -height/2;

  top -= height/2;
  if(top > height/2)
    top = height/2;

  if((left < right) && (bottom < top))
    {
      /* if resulting box has a size, add it to partlist */
      if(!(part = calloc(1, sizeof(idr_picpart))))
      {
	return AY_EOMEM;
      }
      
      part->next = *partlist;
      *partlist = part;
      part->left = left;
      part->right = right;
      part->bottom = bottom;
      part->top = top;
    } /* if */

 return AY_OK;
} /* idr_3dreg_topart */



/* idr_assign_importance:
 *  mode 0: selection -> importance == 1.0
 */
int
idr_assign_importance(int mode, ay_view_object *view)
{
 int ay_status = AY_OK;
 ay_object *o = NULL;

 switch(mode)
   {
   case 0:
     /* manual, use importance from IDR tags */
     /* first clear internal importance tags from scene */
     o = ay_root;
     while(o->next)
       {
	 ay_status = idr_clear_importance(o);
	 o = o->next;
       }
     /* now copy importance from IDR tags to IIDR tags */
     o = ay_root;
     while(o->next)
       {
	 ay_status = idr_copy_importance(o, idr_idrtagtype);
	 o = o->next;
       }
     break;
   case 1:
     /* use importance from selection */
     /* first clear importance tags from scene */
     o = ay_root;
     while(o/*->next*/)
       {
	 ay_status = idr_clear_importance(o);
	 o = o->next;
       }
     /* now assign importance based on selection */
     ay_status = idr_assign_impsel();
     break;
   case 2:
     /* use importance from changed objects */

     ay_status = idr_assign_impchanged();

     /* first clear importance tags from scene */
     o = ay_root;
     while(o->next)
       {
	 ay_status = idr_clear_importance(o);
	 o = o->next;
       }
     /* now copy importance from CIDR tags to IIDR tags */
     o = ay_root;
     while(o->next)
       {
	 ay_status = idr_copy_importance(o, idr_cidrtagtype);
	 o = o->next;
       }

     /* clear information about changed objects, */
     /* from scene: */
      o = ay_root;
      while(o->next)
	{
	  ay_status = idr_clear_changed(o);
	  o = o->next;
	}
      /* and clipboard: */
      o = ay_clipboard;
      if(o)
	{
	  while(o->next)
	    {
	      ay_status = idr_clear_changed(o);
	      o = o->next;
	    }
	}


      /* save state of currently selected objects */
      ay_status = idr_save_selected();

     break;
   case 3:
     /* use 2D regions */
     /* clear importance tags from scene */
     o = ay_root;
     while(o->next)
       {
	 ay_status = idr_clear_importance(o);
	 o = o->next;
       }
     break;
   case 4:
     /* use 3D regions */
     /* clear importance tags from scene */
     o = ay_root;
     while(o->next)
       {
	 ay_status = idr_clear_importance(o);
	 o = o->next;
       }

     break;
   default:
     break;
   } /* switch */

 return AY_OK;
} /* idr_assign_importance */


/* idr_clear_importance:
 *  clear internal importance tags from scene below o
 */
int
idr_clear_importance(ay_object *o)
{
 int ay_status = AY_OK;
 ay_object *down = NULL;
 ay_tag_object *t = NULL, **tprev = NULL;

 if(o)
   {
     if(o->down)
       {
	 down = o->down;
	 while(down->next)
	   {
	     ay_status = idr_clear_importance(down);
	     down = down->next;
	   }
       }
     
     /* find tag */
     t = o->tags;
     tprev = &(o->tags);
     while(t)
       {
	 if(t->type == idr_iidrtagtype)
	   {
	     /* delete found tag */
	     if(t->val)
	       {
		 free(t->val);
	       }
	     if(t->name)
	       {
		 free(t->name);
	       }
	     *tprev = t->next;
	     free(t);
	     t = NULL;
	   }
	 else
	   {
	     tprev = &(t->next);
	     t = t->next;
	   } /* if */
       } /* while */

   } /* if */

 return AY_OK;
} /* idr_clear_importance */


/* idr_copy_importance:
 *  copy importance tags from XIDR type (from_type) to IIDR tag types
 */
int
idr_copy_importance(ay_object *o, char *from_type)
{
 int ay_status = AY_OK;
 double dtemp = 0.0;
 ay_object *down = NULL;
 ay_tag_object *t = NULL;
 ay_tag_object *tidr = NULL, *tiidr = NULL;

 if(o)
   {
     if(o->down)
       {
	 down = o->down;
	 while(down->next)
	   {
	     ay_status = idr_copy_importance(down, from_type);
	     down = down->next;
	   }
       }
     
     /* find IDR tag */
     t = o->tags;
     while(t)
       {
	 if(t->type == from_type)
	   {
	     tidr = t;
	     t = NULL;
	   }
	 else
	   {
	     t = t->next;
	   } /* if */
       } /* while */

     if(tidr)
       {
	 /* find target tag */
	 t = o->tags;
	 while(t)
	   {
	     if(t->type == idr_iidrtagtype)
	       {
		 tiidr = t;
		 t = NULL;
	       }
	     else
	       {
		 t = t->next;
	       } /* if */
	   } /* while */

	 if(!tiidr)
	   { /* create new iidrt tag */

	     if(!(tiidr = calloc(1, sizeof(ay_tag_object))))
	       return AY_EOMEM;

	     /* fill new tag */
	     if(!(tiidr->name = calloc(5, sizeof(char))))
	       return AY_EOMEM;
	     sprintf(tiidr->name, idr_iidrtagname);
	     tiidr->type = idr_iidrtagtype;

	     if(!(tiidr->val = calloc(64, sizeof(char))))
	       return AY_EOMEM;

	     sscanf(tidr->val, "%lg", &dtemp);
	     sprintf(tiidr->val, "%g", dtemp);
	  
	     /* link new tag */
	     tiidr->next = o->tags;
	     o->tags = tiidr;

	   }
	 else
	   { /* just copy data from idr to iidr */

	     if(tiidr->val)
	       {
		 free(tiidr->val);
		 tiidr->val = NULL;
	       }
	     	     
	     if(!(tiidr->val = calloc(64, sizeof(char))))
	       return AY_EOMEM;

	     sscanf(tidr->val, "%lg", &dtemp);
	     sprintf(tiidr->val, "%g", dtemp);
	  

	   }



       } /* if */

   } /* if */

 return AY_OK;
} /* idr_copy_importance */


/* idr_show_parts:
 *  for debugging only, create a IDR-Tag from parts in partlist
 */
int
idr_show_parts(struct Togl *togl, idr_picpart *partlist)
{
  /* int ay_status = AY_OK;*/
 ay_tag_object *t = NULL;
 idr_picpart *part = NULL;
 char buf[255];
 ay_object *v = NULL;
 ay_view_object *v1 = NULL;
 char *rvname = NULL;
 int width = Togl_Width(togl);
 int height = Togl_Height(togl);

  v1 = Togl_GetClientData(togl);

  v = ay_root->down;
  while(v->next)
    {
      if(v1 == (ay_view_object*)(v->refine))
	{
	  rvname = v->name;
	}
      v = v->next;
    }

 
 part = partlist;
 while(part)
   {
     /* create new ridrt tag */
     if(!(t = calloc(1, sizeof(ay_tag_object))))
       return AY_EOMEM;

     /* fill new tag */
     if(!(t->name = calloc(5, sizeof(char))))
       return AY_EOMEM;
     sprintf(t->name, idr_ridrtagname);
     t->type = idr_ridrtagtype;

     if(!(t->val = calloc(64, sizeof(char))))
       return AY_EOMEM;

     sprintf(buf, "%s %d %d %d %d 1.0", rvname,
	     part->left+(width/2), (part->bottom+(height/2)), part->right+(width/2), (part->top+(height/2)));

     if(!(t->val = calloc(strlen(buf)+1, sizeof(char))))
       return AY_EOMEM;

     strcpy(t->val, buf);

     /* link new tag */
     t->next = ay_root->tags;
     ay_root->tags = t;

     part = part->next;
   }
 return AY_OK;
} /* idr_show_parts */


/*
 * idr_wrib_object:
 *  replaces ay_wrib_object in case of IDR rendering
 *  In:
 *  o: object to be written
 *  file: RIB filename
 *  importance: write objects with this importance value;
 *              objects with other or none value,
 *              will be enclosed in RiMatte(1); RiMatte(0); calls
 *  exclude: objects with other importance will simply be excluded
 *  pimportance: importance of parent object, if defined (!= -1.0)
 *               and o does not define an importance, we assume the
 *               parent importance as current importance of o
 */
int 
idr_wrib_object(ay_object *o, char *file, double importance, int exclude,
		double pimportance)
{
 int ay_status = AY_OK;
 ay_object *down = NULL;
 void **arr = NULL;
 ay_wribcb *cb = NULL;
 ay_level_object *l = NULL;
 char *parname = "name";
 ay_tag_object *tag = NULL;
 double oimportance = -1.0;

  if(!o)
    return AY_OK;

  if(ay_prefs.excludehidden && o->hide)
    return AY_OK;

  tag = o->tags;
  while(tag)
    {
      if(tag->type == idr_iidrtagtype)
	{
	  sscanf(tag->val, "%lg", &oimportance);
	  tag = NULL;
	}
      else
	{
	  tag = tag->next;
	}
    } /* while */

  if(pimportance != -1.0)
    {
      /* use parent object importance for this object (if not
       defined locally) */
      if(oimportance == -1.0)
	{
	  oimportance = pimportance;
	}
    }
  else
    {
      /* assume importance of 0.0 for objects in top level
	 of the scene with undefined importance */
      if(oimportance == -1.0)
	{
	  oimportance = 0.0;
	}
    }

  if(importance == oimportance)
    {
      if(oimportance != pimportance)
	{
	  RiMatte(0);
	}
    }
  else
    {
      if(!exclude)
	{
	  if(oimportance != pimportance)
	    {
	      RiMatte(1);
	    }
	}
      else
	{
	  /*
	    we may not just bail out here, because
	    lower level objects may redefine importance again,
	    so that we have to descent
	  */
	  if(o->down)
	    {
	      down = o->down;

	      if(o->type == AY_IDLEVEL)
		{
		  l = (ay_level_object*)o->refine;
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
		      RiSolidBegin(RI_PRIMITIVE);
		      break;
		    default:
		      break;
		    }
		}

	      while(down->next)
		{
		  ay_status = idr_wrib_object(down, file, importance, exclude,
					      oimportance);
		  if(ay_status)
		    return ay_status;
	      
		  down = down->next;
		} /* while */

	      if(l)
		{
		  if(l->type > 1)
		    {
		      RiSolidEnd();
		    } /* if */
		} /* if */

	    } /* if */

	  return AY_OK; /* XXXX early exit! */
	} /* if */
    } /* if */

  if(!ay_prefs.resolveinstances && (o->type != AY_IDMATERIAL) &&
     o->refcount)
    {
      ay_status = ay_wrib_refobject(file, o);

      if(importance == oimportance)
	{
	  if(oimportance != pimportance)
	    RiMatte(1);
	}
      else
	{
	  if(oimportance != pimportance)
	    RiMatte(0);
	}

      return ay_status; /* XXXX early exit! */
    }


  arr = ay_wribcbt.arr;
  cb = (ay_wribcb *)(arr[o->type]);

  if(cb)
    {
      /* write name */
      if(ay_prefs.writeident)
	{
	  if(o->name)
	    {
	      if(!ay_prefs.ristandard)
		{
		  RiDeclare(parname, "string");
		  RiAttribute("identifier", parname,
			      (RtPointer)&o->name, RI_NULL);
		  
		}
	      else
		{
		  RiArchiveRecord(RI_COMMENT, o->name);
		}
	    }
	}
      else
	{
	  if(o->name)
	    {
	      RiArchiveRecord(RI_COMMENT, o->name);
	    }
	}
	
      RiAttributeBegin();
      RiTransformBegin();

      /* write transformations */
      ay_wrib_trafos(o);

      /* write material information */
      if(o->type != AY_IDINSTANCE)
	{
	  if(o->mat)
	    {
	      ay_status = ay_matt_wrib(file, o->mat);
	    }

	  /* write tags */
	  if(o->tags)
	    {
	      ay_status = ay_riattr_wrib(o);
	    }

	}


    } /* if */

  if(o->down)
    {
      down = o->down;

      if(o->type == AY_IDLEVEL)
	{
	  l = (ay_level_object*)o->refine;
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
	      RiSolidBegin(RI_PRIMITIVE);
	      break;
	    default:
	      break;
	    }
	}

      while(down->next)
	{
	  ay_status = idr_wrib_object(down, file, importance, exclude,
				      oimportance);
	  if(ay_status)
	    return ay_status;
	      
	  down = down->next;
	} /* while */

      if(l)
	{
	  if(l->type > 1)
	    {
	      RiSolidEnd();
	    }
	}

    } /* if */

  if(cb)
    {
      ay_status = cb(file, o);

      RiTransformEnd();
      RiAttributeEnd();
    } /* if */

  if(importance == oimportance)
    {
      if(oimportance != pimportance)
	RiMatte(1);
    }
  else
    {
      if(oimportance != pimportance)
	RiMatte(0);
    }

 return ay_status;
} /* idr_wrib_object */

#if 0
/*
 * idr_wrib_object:
 *  replaces ay_wrib_object in case of IDR rendering
 *  In:
 *  o: object to be written
 *  file: RIB filename
 *  mode: 3 states used by IDR rendering
 *        00 - rgl mode (don't render selected objects)
 *        01 - rendrib mode (unmask unselected objects by RiMatte)
 *        02 - child mode (don't check selection)
 */
int 
idr_wrib_object(ay_object *o, char *file, int mode)
{
 int ay_status = AY_OK;
 ay_list_object *sel = ay_selection;
 ay_object *down = NULL;
 void **arr = NULL;
 ay_wribcb *cb = NULL;
 ay_level_object *l = NULL;
 char *parname = "name";

  if(!o)
    return AY_OK;

  if(mode != 2)
    {
      /* not in child mode */
      while((sel) && (sel->object != o))
	{
          sel = sel->next;
	}
      if((sel) && (mode == 0))
	  return AY_OK;  /* don't render high quality parts in rgl */
      if((sel == NULL) && (mode == 1))
	  RiMatte(1);
      else
	  RiMatte(0);
    }


  if(!ay_prefs.resolveinstances && (o->type != AY_IDMATERIAL) &&
     o->refcount)
    {
      ay_status = ay_wrib_refobject(file, o);

      if(mode != 2)
	{
	  if((sel == NULL) && (mode == 1))
	      RiMatte(0);     
	}
      return ay_status; /* XXXX early exit! */
    }


  arr = ay_wribcbt.arr;
  cb = (ay_wribcb *)(arr[o->type]);

  if(cb)
    {
      /* write name */
      if(ay_prefs.writeident)
	{
	  if(o->name)
	    {
	      if(!ay_prefs.ristandard)
		{
		  RiDeclare(parname, "string");
		  RiAttribute("identifier", parname,
			      (RtPointer)&o->name, RI_NULL);
		  
		}
	      else
		{
		  RiArchiveRecord(RI_COMMENT, o->name);
		}
	    }
	}
      else
	{
	  if(o->name)
	    {
	      RiArchiveRecord(RI_COMMENT, o->name);
	    }
	}
	
      RiAttributeBegin();
      RiTransformBegin();

      /* write transformations */
      ay_wrib_trafos(o);

      /* write material information */
      if(o->type != AY_IDINSTANCE)
	{
	  if(o->mat)
	    {
	      ay_status = ay_matt_wrib(file, o->mat);
	    }

	  /* write tags */
	  if(o->tags)
	    {
	      ay_status = ay_riattr_wrib(o);
	    }

	}


    } /* if */

  if(o->down)
    {
      down = o->down;

      if(o->type == AY_IDLEVEL)
	{
	  l = (ay_level_object*)o->refine;
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
	      RiSolidBegin(RI_PRIMITIVE);
	      break;
	    default:
	      break;
	    }
	}

      while(down->next)
	{
	  ay_status = ay_wrib_object(file, down);
	  if(ay_status)
	    return ay_status;
	      
	  down = down->next;
	} /* while */

      if(l)
	{
	  if(l->type > 1)
	    {
	      RiSolidEnd();
	    }
	}

    } /* if */

  if(cb)
    {
      ay_status = cb(file, o);

      RiTransformEnd();
      RiAttributeEnd();
    } /* if */

  if(mode != 2)
    {
      /* end RiMatte(1) */
      if((sel == NULL) && (mode == 1))
	  RiMatte(0);     
    }
 return ay_status;
} /* idr_wrib_object */
#endif

/*
 * idr_wrib_scene:
 *  replaces ay_wrib_scene in case of IDR rendering
 *  In:
 *  file: RIB filename
 *  image: TIFF filename for result picture
 *  importance: write just object with this importance
 *  exclude: exclude objects with other importance value?
 *  from: source point of the camera
 *  to: target point of the camera
 *  roll, zoom: roll angle and zoom factor of the camera
 *  width, height: dimension of the full image
 *  type: type of projection
 *  left, bottom, right, top: region of the full image to be rendered
 */
int
idr_wrib_scene(char *file, char *image, double importance, int exclude,
	       double *from,
	       double *to,
	       double roll, double zoom,
	       int width, int height, int type,
	       int left, int bottom, int right, int top)
{
 int ay_status = AY_OK;
 ay_object *o = ay_root;
 RtPoint f, t, d;
 RtFloat aspect = 1.0, swleft, swright, swtop, swbot;
 double fakt;
 /* RtFloat bias0 = 0.5, bias1 = 0.5;*/
 RtFloat fov = (RtFloat)90.0;

 if(!ay_prefs.resolveinstances)
  {
    /* reset oid counter */
    ay_instt_createoid(NULL);
    /* create OI tags for all original (referenced) objects */
    ay_status = ay_instt_createorigids(o);
    /* create OI tags for all instance (referencing) objects */
    ay_status = ay_instt_createinstanceids(o);
    /* write archive files for all original (referenced) objects */
    ay_status = ay_instt_wribiarchives(file, o);
  }

  aspect = (RtFloat)(width/((double)height));
  /* assemble args */
  f[0] = (RtFloat) from[0];
  f[1] = (RtFloat) from[1];
  f[2] = (RtFloat) from[2];

  t[0] = (RtFloat) to[0];
  t[1] = (RtFloat) to[1];
  t[2] = (RtFloat) to[2];

  d[0] = (RtFloat)(to[0] - from[0]);
  d[1] = (RtFloat)(to[1] - from[1]);
  d[2] = (RtFloat)(to[2] - from[2]);



  if(!file) /* dump .rib to stdout? */
    RiBegin(RI_NULL);
  else
    RiBegin(file);

  /* write shadow maps */
  if(ay_prefs.use_sm)
    {
      ay_prefs.wrib_sm = AY_TRUE;
      /* XXXX what are these good for?
       * do we need them ?
       */
      /*
      RiOption((RtToken)"shadow", (RtToken)"bias0",
	     (RtPointer)(&bias0), RI_NULL);
      RiOption((RtToken)"shadow", (RtToken)"bias1",
	     (RtPointer)(&bias1), RI_NULL);
      */
      ay_sm_wriballsm(file, ay_root->next, NULL, width, height);
      ay_prefs.wrib_sm = AY_FALSE;
    }


  if(!image) /* render to image file or to frame buffer? */
    RiDisplay(RI_NULL, RI_FRAMEBUFFER, RI_RGBA, RI_NULL);
  else
    RiDisplay(image, RI_FILE, RI_RGBA, RI_NULL);

  /* write imager shader */
  ay_wrib_rootsh(AY_TRUE);

  /* Camera! */
  RiFormat(right-left, top-bottom, -1.0);
  /*  RiFormat(width, height, -1.0);*/
  if(type != AY_VTPERSP)
    RiProjection("orthographic", RI_NULL);
  else
    RiProjection("perspective", "fov", (RtPointer)&fov, RI_NULL);

  /*RiFrameAspectRatio((RtFloat)aspect);*/
  RiFrameAspectRatio((RtFloat)fabs(((double)right-left)/(bottom-top)));

  aspect = (RtFloat)(width/((double)height));
  swleft = -aspect;
  swright =  aspect;
  swbot = -1.0;
  swtop = 1.0;

  fakt = width/((double)(swright-swleft));
  /* inverse calculation for exact pixel matching */
  swleft = left/fakt;
  swright = right/fakt;
  swtop = top/fakt;
  swbot = bottom/fakt;

  RiScreenWindow((RtFloat)swleft*zoom, (RtFloat)swright*zoom,
		 (RtFloat)swbot*zoom, (RtFloat)swtop*zoom);

  RiIdentity();

  /* check for lights */
  if(ay_prefs.checklights)
    {
      if(!ay_wrib_checklights(ay_root))
	{ /* no lights in scene -> add a distant headlight */
	  RiArchiveRecord(RI_COMMENT, "Default Distant Headlight");
	  RiLightSource("distantlight", RI_NULL);
	}
    }

  /* convert rh to lh */
  RiArchiveRecord(RI_COMMENT, "rh->lh");
  RiScale(-1.0, 1.0, 1.0);
  RiArchiveRecord(RI_COMMENT, "Camera!");
  ay_wrib_placecamera(f, d, roll);

  /* wrib RiOptions */
  ay_status = ay_wrib_rioptions();

  /* wrib root RiOption tags*/
  ay_status = ay_riopt_wrib(ay_root);

  RiWorldBegin();

  /* Lights! */
  RiArchiveRecord(RI_COMMENT, "Lights!");
  ay_status = ay_wrib_lights(file, ay_root->next);

  RiIdentity();

  /* Action! */
  RiArchiveRecord(RI_COMMENT, "Action!");

  /* write atmosphere shader */
  ay_wrib_rootsh(AY_FALSE);

  /* write default material */
  ay_wrib_defmat(file);

  o = ay_root->next;
  while(o->next)
    {
      ay_status = idr_wrib_object(o, file, importance, exclude, -1.0);
      o = o->next;
    }
  RiWorldEnd();

  /* Cut! */
  RiEnd();

 return ay_status;
} /* idr_wrib_scene */


/*
 * idr_translate_points:
 *  translate points from world to camera coordinate system
 *  In:
 *  position, direction, roll: camera parameters
 *  src: pointer to incoming world points
 *  num: number of points to translate
 *  Out:
 *  dst: pointer to resulting camera points
 */
int
idr_translate_points(RtPoint position, RtPoint direction, double roll,
		     double *src, double *dst, int num)
{
 double x, y, z, s, c;
 double rotx, roty;
 int n;

  /* aim along z axis */
  /* calculate the two left angles */
  roty = atan2(direction[0], direction[2]);  
  rotx = atan2(direction[1],
	       sqrt(direction[2]*direction[2]+direction[0]*direction[0]));  
  for (n = 0; n < num; n++)
    {
      x = -(src[n*3+0]-position[0]);
      y = src[n*3+1]-position[1];
      z = src[n*3+2]-position[2];
      /* rotation around x, y and z axis */

      s = sin(-roty); c = cos(-roty);
      dst[n*3+0] = x*c - z*s;
      dst[n*3+2] = x*s + z*c;
  
      s = sin(-rotx); c = cos(-rotx);
      dst[n*3+1] = y*c + dst[n*3+2]*s;
      z = -y*s + dst[n*3+2]*c;

      s = sin(-roll*AY_PI/180); c = cos(-roll*AY_PI/180);
      x = dst[n*3+0]*c + dst[n*3+1]*s;
      y = -dst[n*3+0]*s + dst[n*3+1]*c;

      dst[n*3+0] = x; dst[n*3+1] = y; dst[n*3+2] = z;
    }
 return AY_OK;
} /* idr_translate_points */


/*
 * idr_get2dbbc:

 */
int
idr_get2dbbc(ay_object *o, int *left, int *right,
	     int *bottom, int *top)
{
 GLdouble mvm[16], pm[16], winx, winy, winz;
 GLint vp[4];
 double bb[24] = {0};
 double xmin = DBL_MAX, ymin = DBL_MAX;
 double xmax = -DBL_MAX, ymax = -DBL_MAX;
 int i;

  glGetDoublev(GL_MODELVIEW_MATRIX, mvm);
  glGetDoublev(GL_PROJECTION_MATRIX, pm);
  glGetIntegerv(GL_VIEWPORT, vp);

  ay_bbc_get(o, bb);
  for(i=0;i<24;i+=3)
    {
      gluProject((GLdouble)bb[i], (GLdouble)bb[i+1],
		 (GLdouble)bb[i+2], mvm, pm, vp,
		 &winx, &winy, &winz);

      if(winx < xmin)
	xmin = winx;
      if(winx > xmax)
	xmax = winx;
      if(winy < ymin)
	ymin = winy;
      if(winy > ymax)
	ymax = winy;

    } /* for */

  *left = (int)xmin;
  *right = (int)xmax;
  *bottom = (int)ymin;
  *top = (int)ymax;

 return AY_OK;
} /* idr_get2dbbc */


/*
 * idr_optbbcombine:
 *  combines all bounding boxes in part list
 *  In:
 *  part: pointer to a pointer to a partlist
 */
void
idr_optbbcombine(idr_picpart **part)
{
 int minx = INT_MAX, miny = INT_MAX;
 int maxx = INT_MIN, maxy = INT_MIN;
 idr_picpart *p = NULL, *pn = NULL;
 int alpha = 0;

  p = *part;

  while(p)
    {
      pn = p->next;

      if(p->left < minx)
	minx = p->left;

      if(p->right > maxx)
	maxx = p->right;

      if(p->bottom < miny)
	miny = p->bottom;

      if(p->top > maxy)
	maxy = p->top;

      alpha += p->alpha;

      free(p);
      p = pn;
    }

  *part = NULL;
  p = NULL;
  if(!(p = calloc(1, sizeof(idr_picpart))))
    return;

  p->left = minx;
  p->right = maxx;
  p->bottom = miny;
  p->top = maxy;
  if(alpha)
    p->alpha = 1;
  *part = p;

 return;
} /* idr_optbbcombine */


/*
 * idr_checkboxes:
 *  calculate costs for combining two bounding boxes
 *  In:
 *  part1, part2: the bounding boxes
 *  wasted: rendering costs without combining this two boxes
 *  Out:
 *  return: new rendering costs
 */
int
idr_checkboxes(idr_picpart *part1, idr_picpart *part2,
	       idr_picpart *opartlist, idr_picpart *tpartlist,
	       int wasted)
{
 int l, b, r, t;
 int lc, bc, rc, tc;
 int f, flag;
 idr_picpart *part;
 
  if(part1->left < part2->left)
    l = part1->left;
  else
    l = part2->left;

  if(part1->bottom < part2->bottom)
    b = part1->bottom;
  else
    b = part2->bottom;
  if(part1->right > part2->right)
    r = part1->right;
  else
    r = part2->right;
  if(part1->top > part2->top)
    t = part1->top;
  else
    t = part2->top;

  /* when combining two boxes we save one complete rendering step */
  wasted -= weight_r;

  /* count pixels to render in new bounding box with the disjunct boxes
     in partlist */
  part = opartlist;
  f = 0;
  while(part)
    {
      if(part->left < l)
	lc = l;
      else
	lc = part->left;
      if(part->bottom < b)
	bc = b;
      else
	bc = part->bottom;
      if(part->right > r)
	rc = r;
      else
	rc = part->right;
      if(part->top > t)
	tc = t;
      else
	tc = part->top;
      if((lc < rc) && (bc < tc))
	f += (rc-lc)*(tc-bc);
      part = part->next;
    }
  /* add number of pixels that don't belong to the original boxes to cost */
  wasted += (r-l)*(t-b)-f;

  /* calculate weight */
  part = tpartlist;
  /* eliminate overlapping regions, allow even splitting */
  while(part)
    {
      if((part != part1) && (part != part2))
	{
	  /* do parts overlap each other? */
	  if(part->left < l)
	    lc = l;
	  else
	    lc = part->left;
	  if(part->bottom < b)
	    bc = b;
	  else
	    bc = part->bottom;
	  if(part->right > r)
	    rc = r;
	  else
	    rc = part->right;
	  if(part->top > t)
	    tc = t;
	  else
	    tc = part->top;
	  if((lc < rc) && (bc < tc))
	    {
	      /* parts overlap */
	      flag = 0;
	      if(l > part->left)
		flag|= 1;
	      if(b > part->bottom)
		flag|= 2;
	      if(r < part->right)
		flag|= 4;
	      if(t < part->top)
		flag|= 8;
	      switch (flag)
		{
		case 0:  /* new box covers whole new one */
		  /* we save a whole rendering step */
		  wasted-= weight_r;
		  break;

		case 1:
		case 2:
		case 4:
		case 8:  /* new box covers part of the old one */
		  break;

		case 3:
		case 5:
		case 6:
		case 9:
		case 10:  /* new box covers part of the old one */
		case 12:  /* but old one must be divided to remove
			     overlapping */
		  /* !!don't divide if cost rise to much */
		  if((rc-lc)*(tc-bc) > weight_r)
		    wasted += weight_r;
		  else
		    wasted += (rc-lc)*(tc-bc);
		  break;

		default:
		  /* we should never arrive here !! */
		  break;
		} /* switch */
	    } /* if */
	} /* if */
      part = part->next;
    } /* while */

 return wasted;
} /* idr_checkboxes */


/*
 * idr_combineboxes:
 *  combines two bounding boxes and removes overlapping if neccessary
 *  In:
 *  part1, part2: the two boxes
 */
void
idr_combineboxes(idr_picpart *part1, idr_picpart *part2,
		 idr_picpart *tpartlist)
{
 idr_picpart *part, *lastpart, *nextpart;
 int l, b, r, t, flag;
 
  /* find box before part2 in bounding box list */
  part = part1;
  while(part->next != part2)
      part = part->next;
  /* unlink part2 from bounding box list */
  part->next = part2->next;
  /* combine bounding boxes */
  if(part1->left > part2->left)
      part1->left = part2->left;
  if(part1->bottom > part2->bottom)
      part1->bottom = part2->bottom;
  if(part1->right < part2->right)
      part1->right = part2->right;
  if(part1->top < part2->top)
      part1->top = part2->top;
  /* delete part2 from memory */
  free(part2);
  
  /* test box against all other boxes in tpartlist to prevent overlapping */
  lastpart = NULL;
  part = tpartlist;
  while(part)
    {
      if(part != part1)
        {
	  if(part->left > part1->left)
	      l = part->left;
	  else
	      l = part1->left;
	  if(part->bottom > part1->bottom)
	      b = part->bottom;
	  else
	      b = part1->bottom;
	  if(part->right < part1->right)
	      r = part->right;
	  else
	      r = part1->right;
	  if(part->top < part1->top)
	      t = part->top;
	  else
	      t = part1->top;
	  if((l < r) && (b < t))
	    {
	      /* parts overlap each other */
	      flag = 0;
	      if(part1->left > part->left)
		  flag|= 1;
	      if(part1->bottom > part->bottom)
		  flag|= 2;
	      if(part1->right < part->right)
		  flag|= 4;
	      if(part1->top < part->top)
		  flag|= 8;
	      switch(flag)
	        {
		  case 0:  /* part lies complete within part1 */
		      if(lastpart)
		        {
			  lastpart->next = part->next;
			  free(part);
			  part = lastpart->next;
		        }
		      else
		        {
			  lastpart = part->next;
			  free(part);
			  part = lastpart;
			  lastpart = NULL;
			  tpartlist = part;  /* changed first list element */
		        }
		      break;

		  case 1:  /* cut part at the right side */
		      part->right = l;
		      break;
		  case 2:  /* cut part at the top side */
		      part->top = b;
		      break;
		  case 4:  /* cut part at the left side */
		      part->left = r;
		      break;
		  case 8:  /* cut part at the bottom side */
		      part->bottom = t;
		      break;

		  case 3:
		  case 5:
		  case 6:
		  case 9:
		  case 10:  /* new box covers part of the old one */
		  case 12:  /* but old one must be divided */
		      if((r-l)*(t-b) > weight_r)
		        {
			  nextpart = calloc(1, sizeof(idr_picpart));
			  nextpart->next = part->next;
			  part->next = nextpart;
			  switch(flag)
			    {
			      case 3:
				  nextpart->left = l;
				  nextpart->right = part->right;
				  nextpart->bottom = part->bottom;
				  nextpart->top = b;
				  part->right = l;
				  break;
			      case 5:
				  nextpart->left = r;
				  nextpart->right = part->right;
				  nextpart->bottom = part->bottom;
				  nextpart->top = part->bottom;
				  part->right = l;
				  break;
			      case 6:
				  nextpart->left = part->left;
				  nextpart->right = r;
				  nextpart->bottom = part->bottom;
				  nextpart->top = b;
				  part->left = r;
				  break;
			      case 9:
				  nextpart->left = l;
				  nextpart->right = part->right;
				  nextpart->bottom = t;
				  nextpart->top = part->top;
				  part->right = l;
				  break;
			      case 10:
				  nextpart->left = part->left;
				  nextpart->right = part->right;
				  nextpart->bottom = t;
				  nextpart->top = part->top;
				  part->top = b;
				  break;
			      case 12:
				  nextpart->left = part->left;
				  nextpart->right = r;
				  nextpart->bottom = t;
				  nextpart->top = part->top;
				  part->left = r;
				  break;
			    } /* switch */
			  /* jump over the second tile of part */
			  lastpart = nextpart;
			  part = nextpart->next;
		        } /* if */
		      else
		        {
			  lastpart = part;
			  part = part->next;
		        }
		      break;
	        } /* switch */
	    } /* if */
	  else
	    {
	      lastpart = part;
              part = part->next;
	    }
        } /* if */
      else
        {
	  lastpart = part;
          part = part->next;
        }
    } /* while */

 return;
} /* idr_combineboxes */


/*
 * idr_removeoverlappingboxes:
 *  create disjunct list of bounding boxes by removing overlapping
 *  In:
 *  part: points the start of the list of bounding boxes
 */
void
idr_removeoverlappingboxes(idr_picpart **part)
{
 idr_picpart *src, *dst, *lastsrc, *lastdst;
 idr_picpart *add, *new_src;
 int l, b, r, t;
 int l1, b1, r1, t1;
 int del_src, del_dst; 
 int flag, flag2, bp;
 unsigned char div_flags[] = { 0xDB, 0x00, 0x5A, 0x00, 0x00, 0x00, 0x7E };

  lastsrc = NULL;
  src = *part;
  /* check each part against each other */
  while(src)
    {
      del_src = 0;
      lastdst = NULL;
      dst = *part;
      while((dst) && (del_src == 0))
        {
	  if(dst != src)
	    {
	      del_dst = 0;
	      if(src->left > dst->left)
	          l = src->left;
	      else
	          l = dst->left;
	      if(src->bottom > dst->bottom)
	          b = src->bottom;
	      else
	          b = dst->bottom;
	      if(src->right < dst->right)
	          r = src->right;
	      else
	          r = dst->right;
	      if(src->top < dst->top)
	          t = src->top;
	      else
	          t = dst->top;
	      if((l < r) && (b < t))
	        {
		  /* parts overlap each other */
		  flag = 0;
		  if(dst->left > src->left)
		      flag|= 1;
		  if(dst->bottom > src->bottom)
		      flag|= 2;
		  if(dst->right < src->right)
		      flag|= 4;
		  if(dst->top < src->top)
		      flag|= 8;

		  switch(flag)
		    {
		      case 0:  /* delete src */
			  if(lastsrc)
			      lastsrc->next = src->next;
			  else
			      *part = src->next;
			  free(src);
			  if(lastsrc)
			      src = lastsrc->next;
			  else
			      src = *part;
			  del_src = 1;
			  break;

		      case 3:
		      case 5:
		      case 9:  /* split src and dst */
			  flag2 = 0;
			  /* check if comparation in other direction produces
			     better result */
			  if(src->left > dst->left)
			      flag2|= 1;
			  if(src->bottom > dst->bottom)
			      flag2|= 2;
			  if(src->right < dst->right)
			      flag2|= 4;
			  if(src->top < dst->top)
			      flag2|= 8;
			  if((flag2 != 0) && (flag2 != 1) && (flag2 != 2) &&
			     (flag2 != 4) && (flag2 != 8))
			    {
			      if(src->left < dst->left)	
				  l1 = src->left;
			      else
				  l1 = dst->left;
			      if(src->bottom < dst->bottom)
				  b1 = src->bottom;
			      else
				  b1 = dst->bottom;
			      if(src->right > dst->right)
				  r1 = src->right;
			      else
				  r1 = dst->right;
			      if(src->top > dst->top)
				  t1 = src->top;
			      else
			  	t1 = dst->top;
			      bp = 0;
			      src->left = l;
			      src->bottom = b;
			      src->right = r;
			      src->top = t;
			      new_src = NULL;
			      while(bp < 8)
				{
				  if(div_flags[flag-3]&(1<<bp))
				    {
				      add = calloc(1, sizeof(idr_picpart));
				      switch(bp)
					{
					  case 0:
					  case 3:
					  case 5:
					      add->left = l1;
					      add->right = l;
					      break;
					  case 1:
					  case 6:
					      add->left = l;
					      add->right = r;
					      break;
					  default:  /*  2, 4, 7 */
					      add->left = r;
					      add->right = r1;
					      break;
					}
				      switch(bp)
					{
					  case 0:
					  case 1:
					  case 2:
					      add->bottom = b1;
					      add->top = b;
					      break;
					  case 3:
					  case 4:
					      add->bottom = b;
					      add->top = t;
					      break;
					  default:  /*  5, 6, 7 */
					      add->bottom = t;
					      add->top = t1;
					      break;
					}
				      if((add->left == add->right) ||
					 (add->bottom == add->top))
					{
					  /* should never happen */
					  free(add);
					}
				      else
					{
					  add->next = new_src;
					  new_src = add;
					}
				    } /* if */
				  bp++;
				} /* while */
			      if(new_src) /* should always be */
				{
				  if(lastdst)
				      lastdst->next = dst->next;
				  else
				      *part = dst->next;
				  free(dst);
				  /* insert new_src after src */
				  dst = new_src;
				  while(dst->next)
				      dst = dst->next;
				  dst->next = src->next;
				  src->next = new_src;
				  if (lastdst)
				      dst = lastdst->next;
				  else
				      dst = *part;
				  del_dst = 1;
				  del_src = 1;
				}
			    } /* if */
			  break;

		      case 1:  /* cut src box */
			  src->right = dst->left;
			  break;
		      case 2:  /* cut src box */
			  src->top = dst->bottom;
			  break;
		      case 4:  /* cut src box */
			  src->left = dst->right;
			  break;
		      case 8:  /* cut src box */
			  src->bottom = dst->top;
			  break;
		    } /* switch */
		} /* if */
	      if(del_dst == 0)
		{
		  lastdst = dst;
		  dst = dst->next;
		}
	    } /* if */
	  else
	    {
	      lastdst = dst;
	      dst = dst->next;
	    }
      }
      if(del_src == 0)
        {
	  lastsrc = src;
	  src = src->next;
        }
    } /* while */

 return;
} /* idr_removeoverlappingboxes */


/*
 * idr_get2dbbclist:
 *
 */
int
idr_get2dbbclist(idr_picpart **partlist, double importance,
		 int width, int height, ay_object *o)
{
 int ay_status = AY_OK;
 ay_tag_object *t = NULL;
 double i = 0.0;
 int left, top, right, bottom;
 idr_picpart *part = NULL;
 double bb[24] = {0};

  while(o->next)
    {
      if(o->down)
	{
	  ay_status = idr_get2dbbclist(partlist, importance, width,
				       height, o->down);
	}

      i = 0.0;
      /* find importance of this object */
      t = o->tags;
      while(t)
	{
	  if(t->type == idr_iidrtagtype)
	    {
	      sscanf(t->val, "%lg", &i);
	      t = NULL;
	    }
	  else
	    {
	      t = t->next;
	    }
	}

      if(i == importance)
	{
	  ay_bbc_get(o, bb);
	  idr_get2dbbc(o, &left, &right, &bottom, &top);

	  left -= width/2;
	  if(left < -width/2)
	    left = -width/2;

	  right -= width/2;
	  if(right > width/2)
	    right = width/2;

	  bottom -= height/2;
	  if(bottom < -height/2)
	    bottom = -height/2;

	  top -= height/2;
	  if(top > height/2)
	    top = height/2;

	  if((left < right) && (bottom < top))
	    {
	      /* if resulting box has a size, add it to partlist */
	      part = calloc(1, sizeof(idr_picpart));
	      part->next = *partlist;
	      *partlist = part;
	      part->left = left;
	      part->right = right;
	      part->bottom = bottom;
	      part->top = top;
	    } /* if */
	} /* if */

      o = o->next;
    } /* while */

 return ay_status;
} /* idr_get2dbbclist */


/*
 * idr_getpartsfromimpreg:
 *
 */
int
idr_getpartsfromimpreg(idr_picpart **partlist, struct Togl *togl,
		       double importance,
		       int width, int height)
{
 int ay_status = AY_OK;
 ay_tag_object *t = NULL, *told = NULL, *tnew = NULL;
 int left, top, right, bottom;
 idr_picpart *part = NULL;
 ay_object *v = NULL;
 ay_view_object *v1 = NULL;
 double X1, Y1, X2, Y2, imp;
 char vname[255], *rvname = NULL;
 int found = AY_FALSE;
 ay_object *o = NULL;

  v1 = Togl_GetClientData(togl);

  v = ay_root->down;
  while(v->next)
    {
      if(v1 == (ay_view_object*)(v->refine))
	{
	  rvname = v->name;
	}
      v = v->next;
    }

  t = ay_root->tags;
  while(t)
    {
      if(t->type == idr_ridrtagtype)
	{
	  sscanf(t->val,"%s %lg %lg %lg %lg %lg", vname,
		 &X1, &Y1, &X2, &Y2, &imp);

	  /* are we in the right window? */
	  if(!strcmp(vname, rvname))
	    { /* yes */

	      if(imp == importance)
		{
		  left = (int)X1;
		  right = (int)X2;
		  bottom = (int)Y2;
		  top = (int)Y1;

		  left -= width/2;
		  if(left < -width/2)
		    left = -width/2;

		  right -= width/2;
		  if(right > width/2)
		    right = width/2;

		  bottom -= height/2;
		  if(bottom < -height/2)
		    bottom = -height/2;

		  top -= height/2;
		  if(top > height/2)
		    top = height/2;

		  if((left < right) && (bottom < top))
		    {
		      /* if resulting box has a size, add it to partlist */
		      part = calloc(1, sizeof(idr_picpart));
		      part->next = *partlist;
		      *partlist = part;
		      part->left = left;
		      part->right = right;
		      part->bottom = bottom;
		      part->top = top;
		      part->alpha = 1;
		      found = AY_TRUE;
		    } /* if */
		} /* if */
	     
	    } /* if */

	} /* if */

      t = t->next;
    } /* while */


  if(found)
    {
      /*
	Yes, we actually have atleast one picture part to
	render. We need to fake importance tags for all
	toplevel objects in order to render them all.
      */
      /* first clear old IIDR tags */
      o = ay_root;
      while(o->next)
	{
	  ay_status = idr_clear_importance(o);
	  o = o->next;
	}
      /* now create/fake importance tags */
      o = ay_root;
      while(o->next)
	{
	  t = o->tags;

	  /* find old IIDR tag */
	  while(t)
	    {
	      if(t->type == idr_iidrtagtype)
		{
		  told = t;
		  t = NULL;
		}
	      else
		{
		  t = t->next;
		}
	    } /* while */

	  if(told)
	    {
	      /* set new tag value */
	      if(told->val)
		{
		  free(told->val);
		  told->val = NULL;
		}

	      if(!(told->val = calloc(64, sizeof(char))))
		return AY_EOMEM;
	      
	      sprintf(told->val, "%g", importance);
	  
	    }
	  else
	    {
	      /* create new tag */
	      if(!(tnew = calloc(1, sizeof(ay_tag_object))))
		return AY_EOMEM;

	      /* fill new tag */
	      if(!(tnew->name = calloc(4, sizeof(char))))
		return AY_EOMEM;
	      strcpy(tnew->name, idr_iidrtagname);
	      tnew->type = idr_iidrtagtype;

	      if(!(tnew->val = calloc(64, sizeof(char))))
		return AY_EOMEM;

	      sprintf(tnew->val, "%g", importance);
	  
	      /* link new tag */
	      tnew->next = o->tags;
	      o->tags = tnew;
	    }

	  o = o->next;
	} /* while */
    } /* if */

 return ay_status;
} /* idr_getpartsfromimpreg */

/*
 * idr_getpartsfrom3dimpreg:
 *
 */
int
idr_getpartsfrom3dimpreg(idr_picpart **partlist, struct Togl *togl,
		       double importance,
		       int width, int height)
{
 int ay_status = AY_OK;
 ay_tag_object *t = NULL, *told = NULL, *tnew = NULL;
 ay_view_object *view = NULL;
 ay_object *o = NULL;

  view = Togl_GetClientData(togl);


  if(ay_root->down)
    {
      o = ay_root->down;
	 
      while(o->next)
	{
	  if(o->type != AY_IDVIEW)
	    {
	      idr_3dreg_topart(o, view, partlist);
	    }
	  o = o->next;
	} /* while */
    } /* if */

  if(partlist)
    {
      /*
	Yes, we actually have atleast one picture part to
	render. We need to fake importance tags for all
	toplevel objects in order to render them all.
      */
      /* first clear old IIDR tags */
      o = ay_root;
      while(o->next)
	{
	  ay_status = idr_clear_importance(o);
	  o = o->next;
	}
      /* now create/fake importance tags */
      o = ay_root;
      while(o->next)
	{
	  t = o->tags;

	  /* find old IIDR tag */
	  while(t)
	    {
	      if(t->type == idr_iidrtagtype)
		{
		  told = t;
		  t = NULL;
		}
	      else
		{
		  t = t->next;
		}
	    } /* while */

	  if(told)
	    {
	      /* set new tag value */
	      if(told->val)
		{
		  free(told->val);
		  told->val = NULL;
		}

	      if(!(told->val = calloc(64, sizeof(char))))
		return AY_EOMEM;
	      
	      sprintf(told->val, "%g", importance);
	  
	    }
	  else
	    {
	      /* create new tag */
	      if(!(tnew = calloc(1, sizeof(ay_tag_object))))
		return AY_EOMEM;

	      /* fill new tag */
	      if(!(tnew->name = calloc(4, sizeof(char))))
		return AY_EOMEM;
	      strcpy(tnew->name, idr_iidrtagname);
	      tnew->type = idr_iidrtagtype;

	      if(!(tnew->val = calloc(64, sizeof(char))))
		return AY_EOMEM;

	      sprintf(tnew->val, "%g", importance);
	  
	      /* link new tag */
	      tnew->next = o->tags;
	      o->tags = tnew;
	    }

	  o = o->next;
	} /* while */
    } /* if */

 return ay_status;
} /* idr_getpartsfrom3dimpreg */


/*
 * idr_getpartlist:
 *
 */
int
idr_getpartlist(idr_picpart **partlist, struct Togl *togl,
		int idrmode, double importance,
		int width, int height, ay_object *o)
{
 int ay_status = AY_OK;

 switch(idrmode)
   {
   case 0:
   case 1:
   case 2:
     ay_status = idr_get2dbbclist(partlist, importance, width, height, o);
     break;
   case 3:
     ay_status = idr_getpartsfromimpreg(partlist, togl, importance,
					width, height);
   break;
   case 4:
     ay_status = idr_getpartsfrom3dimpreg(partlist, togl, importance,
					  width, height);
     break;
   default:
     break;
   }


 return ay_status;
} /* idr_getpartlist */




/*
 * idr_wrib_tcb:
 *  Togl callback that write ribs for selected view (Togl)
 *  In:
 *  togl: Togl window structure
 *  argc: number of arguments
 *  argv: argument values
 */
int
idr_wrib_tcb(struct Togl *togl, int argc, char *argv[])
{
 int ay_status = AY_OK;
 Tcl_Interp *interp = Togl_Interp (togl);
 Tcl_Obj *to = NULL, *toa = NULL, *ton = NULL;
 char fname[] = "idr_wrib";
 int width = Togl_Width (togl);
 int height = Togl_Height (togl);
 int left, top, right, bottom;
 int i1, i2, i1opt, i2opt, i, k, krit;
 int wasted;
 idr_picpart *part, *optpart, *nextpart;
 idr_picpart *tpartlist = NULL, *opartlist = NULL;
 char buf[256];
 ay_view_object *view = Togl_GetClientData(togl);
 int num;
 int use_current_bg = 0, qlevels = 0, idrmode = 0;
 idr_param *params = NULL;
 char *idrbase = NULL;
 ay_riopt_object *rioptions = NULL;
 ay_root_object *root = NULL;


  root = (ay_root_object *)ay_root->refine;
  rioptions = root->riopt;

  /* save window path for access to view window */
  strcpy(idr_window_path, argv[0]);

  /* get global variables */
  toa = Tcl_NewStringObj("idrprefs",-1);
  ton = Tcl_NewStringObj("UseCurrentBG",-1);
  to = Tcl_ObjGetVar2(interp, toa, ton, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp, to, &use_current_bg);

  Tcl_SetStringObj(ton, "Weight_R",-1);
  to = Tcl_ObjGetVar2(interp, toa, ton, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp, to, &weight_r);

  Tcl_SetStringObj(ton, "QLevels",-1);
  to = Tcl_ObjGetVar2(interp, toa, ton, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp, to, &qlevels);

  Tcl_SetStringObj(ton, "Mode",-1);
  to = Tcl_ObjGetVar2(interp, toa, ton, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp, to, &idrmode);

  Tcl_SetStringObj(ton, "IDRBase",-1);
  to = Tcl_ObjGetVar2(interp, toa, ton, TCL_LEAVE_ERR_MSG |
		      TCL_GLOBAL_ONLY);
  if(!((idrbase = calloc(strlen(Tcl_GetStringFromObj(to, NULL))+1,
			 sizeof(char)))))
  {
    ay_error(AY_EOMEM, fname, NULL);
    return TCL_OK;
  }
  strcpy(idrbase, Tcl_GetStringFromObj(to, NULL));


  if(qlevels < 2)
    return TCL_OK;

  if(!(params = calloc(qlevels, sizeof(idr_param))))
    {
      ay_error(AY_EOMEM, fname, NULL);
      return TCL_OK;
    }


  /* assign importance */
  ay_status = idr_assign_importance(idrmode, view);
  if(ay_status)
    {
      ay_error(ay_status, fname, NULL);
      return TCL_OK;
    }

  for(i = 0; i < qlevels; i++)
    {
      sprintf(buf, "HalfRes%d", i);
      Tcl_SetStringObj(ton, buf, -1);
      to = Tcl_ObjGetVar2(interp, toa, ton, TCL_LEAVE_ERR_MSG |
			  TCL_GLOBAL_ONLY);
      Tcl_GetIntFromObj(interp, to, &((params[i]).halfres));

      sprintf(buf, "ExcludeOther%d", i);
      Tcl_SetStringObj(ton, buf, -1);
      to = Tcl_ObjGetVar2(interp, toa, ton, TCL_LEAVE_ERR_MSG |
			  TCL_GLOBAL_ONLY);
      Tcl_GetIntFromObj(interp, to, &((params[i]).exclude));

      sprintf(buf, "OptimizeBB%d", i);
      Tcl_SetStringObj(ton, buf, -1);
      to = Tcl_ObjGetVar2(interp, toa, ton, TCL_LEAVE_ERR_MSG |
			  TCL_GLOBAL_ONLY);
      Tcl_GetIntFromObj(interp, to, &((params[i]).optimizebb));

      sprintf(buf, "Importance%d", i);
      Tcl_SetStringObj(ton, buf, -1);
      to = Tcl_ObjGetVar2(interp, toa, ton, TCL_LEAVE_ERR_MSG |
			  TCL_GLOBAL_ONLY);
      Tcl_GetDoubleFromObj(interp, to, &((params[i]).importance));      
    } /* for */

  /* adjust roll, if up vector points down */
  if(view->up[1] < 0.0)
    view->roll += 180.0;

  /* free all old content from global partlist */
  while(idr_partlist)
    {
      nextpart = idr_partlist->next;

      if(idr_partlist->RIBFile)
	free(idr_partlist->RIBFile);
      if(idr_partlist->ImageFile)
	free(idr_partlist->ImageFile);

      free(idr_partlist);
      idr_partlist = nextpart;
    }
  idr_partlist = NULL;
  idr_partlist_next = NULL;

  /* store values for global access */
  idr_picture_width = width;
  idr_picture_height = height;

  /* write RIBs */
  for(i = 0; i < qlevels; i++)
    {
      if((i == 0) && (use_current_bg))
	break;

      sprintf(buf, "Files%d", i);

      Tcl_SetVar2(ay_interp, "idrprefs", buf, "",
		  TCL_GLOBAL_ONLY | TCL_LEAVE_ERR_MSG);

      if((params[i]).halfres)
        {
	  /* use half resolution */
	  rioptions->Samples_X /= 2.0;
	  rioptions->Samples_Y /= 2.0;
        }

      left = -width/2;
      bottom = -height/2;
      right = -width/2+width;
      top = -height/2+height;
        

      switch((params[i]).optimizebb)
	{
	case 0:
	  { /* no bounding boxes */

	    /* construct picture part data structure */
	    part = calloc(1, sizeof(idr_picpart));
	    part->left = -width/2;
	    part->bottom = -height/2;
	    part->right = -width/2+width;
	    part->top = -height/2+height;

	    if(idrmode>2)
	      part->alpha = 1;

	    /* construct RIBFile/ImageFile */
	    sprintf(buf, "%s_%d_0.rib", idrbase, i);
	    if(!(part->RIBFile = calloc(strlen(buf)+1, sizeof(char))))
	      {
		ay_error(AY_EOMEM, fname, NULL);
		return TCL_OK;
	      }
	    strcpy(part->RIBFile, buf);

	    sprintf(buf, "%s_%d_0.tiff", idrbase, i);
	    if(!(part->ImageFile = calloc(strlen(buf)+1, sizeof(char))))
	      {
		ay_error(AY_EOMEM, fname, NULL);
		return TCL_OK;
	      }
	    strcpy(part->ImageFile, buf);

	    /* link picture part data structure */
	    if(idr_partlist_next)
	      {
		*idr_partlist_next = part;
	      }
	    else
	      {
		idr_partlist = part;
	      }
	    idr_partlist_next = &part->next;
  
	    /* write RIB */
	    idr_wrib_scene(part->RIBFile, part->ImageFile,
			   (params[i]).importance, (params[i]).exclude,
			   view->from, view->to, view->roll,
			   view->zoom, width, height,
			   view->type, -width/2, -height/2,
			   -width/2+width, -height/2+height);

	    sprintf(buf, "Files%d", i);

	    Tcl_SetVar2(ay_interp, "idrprefs", buf, part->RIBFile,
			TCL_APPEND_VALUE | TCL_GLOBAL_ONLY |
			TCL_LIST_ELEMENT | TCL_LEAVE_ERR_MSG);


	  }
	  break;
	case 1:
	  { /* just combine all parts to one */
	    
	    idr_getpartlist(&tpartlist, togl, idrmode, params[i].importance,
			    width, height, ay_root);
	    
	    idr_optbbcombine(&tpartlist);
	    /* link tpartlist to global picture part list */
	    if(idr_partlist_next)
	      {
		*idr_partlist_next = tpartlist;
	      }
	    else
	      {
		idr_partlist = tpartlist;
	      }
	    part = tpartlist;
	    tpartlist = NULL;
	    idr_partlist_next = &(part->next);

	    /* construct RIBFile/ImageFile */
	    sprintf(buf, "%s_%d_0.rib", idrbase, i);
	    if(!(part->RIBFile = calloc(strlen(buf)+1, sizeof(char))))
	      {
		ay_error(AY_EOMEM, fname, NULL);
		return TCL_OK;
	      }
	    strcpy(part->RIBFile, buf);
	    
	    sprintf(buf, "%s_%d_0.tiff", idrbase, i);
	    if(!(part->ImageFile = calloc(strlen(buf)+1, sizeof(char))))
	      {
		ay_error(AY_EOMEM, fname, NULL);
		return TCL_OK;
	      }
	    strcpy(part->ImageFile, buf);

	    idr_wrib_scene(part->RIBFile, part->ImageFile, 
			   (params[i]).importance, (params[i]).exclude,
			   view->from, view->to, view->roll,
			   view->zoom, width, height,
			   view->type, part->left, part->bottom,
			   part->right, part->top);

	    sprintf(buf, "Files%d", i);

	    Tcl_SetVar2(ay_interp, "idrprefs", buf, part->RIBFile,
			TCL_APPEND_VALUE | TCL_GLOBAL_ONLY |
			TCL_LIST_ELEMENT | TCL_LEAVE_ERR_MSG);

	  }
	  break;
	case 2:
	  { /* full bounding box optimization */
	    tpartlist = NULL;
	    /* first we create a list with a bounding box for
	       objects with current importance */
	    idr_getpartlist(&tpartlist, togl, idrmode, params[i].importance,
			     width, height, ay_root);

	    /* remove overlapping from bounding box list */
	    idr_removeoverlappingboxes(&tpartlist);

	    /* copy partlist */
	    part = tpartlist;
	    while(part)
	      {
		if(opartlist)
		  {
		    optpart->next = calloc(1, sizeof(idr_picpart));
		    optpart = optpart->next;
		  }
		else
		  {
		    opartlist = calloc(1, sizeof(idr_picpart));
		    optpart = opartlist;
		  }
		memcpy(optpart, part, sizeof(idr_picpart));
		part = part->next;
	      } /* while */

	    /* calculate wasted time as the number of bounding boxes
	     (except the first one) multiplied with weight_r */
	    if(tpartlist)
	      {
		part = tpartlist->next;
		wasted = 0;
		while(part)
		  {
		    wasted += weight_r;
		    part = part->next;
		  }
	      }

	    /* try to combine as max boxes as possible in tpartlist */
	    if(tpartlist)
	      {
		do
		  {
		    krit = wasted;
		    i1 = 0;
		    optpart = tpartlist;

		    while(optpart->next)
		      {
			part = optpart->next;
			i2 = 0;
			while(part)
			  {
			    k = idr_checkboxes(optpart, part,
					       opartlist, tpartlist,
					       wasted);
			    if(k < krit)
			      {
				krit = k;
				i1opt = i1;
				i2opt = i2;
			      }
			    i2++;
			    part = part->next;
			  } /* while */
			i1++;
			optpart = optpart->next;
		      } /* while */

		    if(krit < wasted)
		      {
			/* found a combineable pair of boxes */
			optpart = tpartlist;
			while(i1opt)
			  {
			    optpart = optpart->next;
			    i1opt--;
			  }
			part = optpart->next;
			while(i2opt)
			  {
			    part = part->next;
			    i2opt--;
			  }
			idr_combineboxes(optpart, part, tpartlist);
			krit = wasted;
		      }
		    else
		      {
			krit = -1;
		      }
		  } while (krit != -1);
	      } /* if */

	    /* write RIBs */
	    part = tpartlist;
	    num = 0;
	    while(part)
	      {
		/* construct RIBFile/ImageFile */
		sprintf(buf, "%s_%d_%d.rib", idrbase, i, num);
		if(!(part->RIBFile = calloc(strlen(buf)+1, sizeof(char))))
		  {
		    ay_error(AY_EOMEM, fname, NULL);
		    return TCL_OK;
		  }
		strcpy(part->RIBFile, buf);

		sprintf(buf, "%s_%d_%d.tiff", idrbase, i, num);
		if(!(part->ImageFile = calloc(strlen(buf)+1, sizeof(char))))
		  {
		    ay_error(AY_EOMEM, fname, NULL);
		    return TCL_OK;
		  }
		strcpy(part->ImageFile, buf);

		idr_wrib_scene(part->RIBFile, part->ImageFile, 
			       (params[i]).importance, (params[i]).exclude,
			       view->from, view->to, view->roll,
			       view->zoom, width, height,
			       view->type, part->left, part->bottom,
			       part->right, part->top);

		sprintf(buf, "Files%d", i);

		Tcl_SetVar2(ay_interp, "idrprefs", buf, part->RIBFile,
			    TCL_APPEND_VALUE | TCL_GLOBAL_ONLY |
			    TCL_LIST_ELEMENT | TCL_LEAVE_ERR_MSG);

		/* for important regions-mode set alpha
		   of this part to opaqe */
		if(idrmode>2)
		  {
		    part->alpha = 1;
		  }
		part = part->next;
		num++;
	      } /* while */

	    /* link tpartlist to global picture part list */
	    if(idr_partlist_next)
	      {
		*idr_partlist_next = tpartlist;
	      }
	    else
	      {
		idr_partlist = tpartlist;
	      }
	    part = tpartlist;
	    while(part->next)
	      {
		part = part->next;
	      }
	    idr_partlist_next = &part->next;

	    tpartlist = NULL;

	    /* free copy of tpartlist */
	    while(opartlist)
	      {
		nextpart = opartlist->next;
		free(opartlist);
		opartlist = nextpart;
	      }
	    opartlist = NULL;

	  }
	  break;
	default:
	  break;
	} /* switch */

      if((params[i]).halfres)
        {
	  rioptions->Samples_X *= 2.0;
	  rioptions->Samples_Y *= 2.0;
	}
    } /* for i<qlevels */

  if(view->up[1] < 0.0)
    view->roll -= 180.0;

  /* free params */
  free(params);

  /* free Tcl objects */
  Tcl_IncrRefCount(toa);Tcl_DecrRefCount(toa);
  Tcl_IncrRefCount(ton);Tcl_DecrRefCount(ton);

 return TCL_OK;
} /* idr_wrib_tcb */


/*
 * idr_char2hex:
 *  convert unsigned char to hex-string
 *  In:
 *  c: byte to convert
 *  h: destination pointer
 */
void
idr_char2hex(unsigned char c, char *h)
{
 static char digits[] = { '0', '1', '2', '3', '4', '5', '6', '7',
			  '8', '9', 'a', 'b', 'c', 'd', 'e', 'f' };
  h[0] = digits[c>>4];
  h[1] = digits[c&15];
} /* idr_char2hex */


/*
 * idr_read_tiff:
 *  reads a tiff file via libtiff
 *  In:
 *  name: Tiff filename
 *  buf: RGBA-Pixel buffer
 *  interp: the Tcl interpreter
 *  Out:
 *  width, height: dimensions of read tiff
 */
int
idr_read_tiff(char *name, uint32 *buf, int *width, int *height,
	      Tcl_Interp *interp)
{
 TIFF* tif;
 unsigned int bo = 1;  /* test byte order */
 uint32 w, h, c;
 unsigned char *r, b;
 char fname[] = "read_tiff";

  tif = TIFFOpen(name, "r");
  if(tif)
    {
      TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, &w);
      TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &h);
      (*width) = w; (*height) = h;
      if(TIFFReadRGBAImage(tif, w, h, buf, 0) == 0)
        {
	  /* Error while reading TIFF */
	  ay_error(AY_ERROR, fname, "TIFFReadRGBAImage() failed.");
	  TIFFClose(tif);
	  return AY_FALSE;
        }
      else
        {
	  /* check byte order */
	  r = (unsigned char *)&bo;
	  if(((r[0] == 0) && (TIFFIsByteSwapped(tif) == 0)) ||
	     ((r[0] == 1) && (TIFFIsByteSwapped(tif) != 0)))
	    {
	      /* byte order must be corrected: we need intel format */
	      for(c = 0; c < w*h; c++)
	        {
		  r = (unsigned char *)&buf[c];
		  b = r[0];
		  r[0] = r[3];
		  r[3] = b;
		  b = r[1];
		  r[1] = r[2];
		  r[2] = b;
	        }
	    }
        }
      TIFFClose(tif);
    }
  else
    {
      /* Unable to open TIFF */
      ay_error(AY_ERROR, fname, "TIFFOpen() failed.");
      return AY_FALSE;
    }

 return AY_TRUE;
} /* idr_read_tiff */


/*
 * idr_combine_pics:
 *  combine pictures using alpha channel information
 *  In:
 *  db: destination pixel buffer
 *  dw, dh: dimension of db
 *  sb: source pixel buffer
 *  sw, sh: dimension of sb
 *  l, b: target position (left, bottom) of sb in db
 *  part_alpha: if != 0 ignore alpha
 */
void
idr_combine_pics(uint32 *db, int dw, int dh, uint32 *sb, int sw, int sh,
		 int l, int b, int part_alpha)
{
 int x, y, z, a;
 unsigned char *pixel_src, *pixel_dst;
  l+= dw/2;
  b+= dh/2; 
  for(y = 0; y < sh; y++)
    {
      for (x = 0; x < sw; x++)
        {
	  if((x+l < dw) && (y+b < dh))
	    {
	      pixel_src = (char *)&sb[y*sw+x];
	      if(part_alpha)
		{
		  a = 255;
		}
	      else
		{
		  a = pixel_src[3]; /* alpha value */
		}
	      pixel_dst = (char *)&db[(y+b)*dw+(x+l)];
	      for(z = 0; z < 3; z++)
		{
	          pixel_dst[z] = ((255-a)*pixel_dst[z]+a*pixel_src[z])/256;
		}
	    }
        }
    }
} /* idr_combine_pics */


/*
 * idr_combineresultstcmd:
 *  Tcl command, that creates a Tk photostring picture from low and high
 *  quality parts (high memory usage!!)
 *  In:
 *  clientData, interp: Tcl stuff
 *  argc: number of arguments
 *  argv: values of arguments
 */
int
idr_combineresultstcmd(ClientData clientData, Tcl_Interp *interp,
		       int argc, char *argv[])
{
 char strbuf[32];
 char *string, *bytes;
 int bpl;
 int num;
 int w, h, r, c;
 idr_picpart *part;
 char fname[] = "idr_combineresults";

  if(argc != 4)
    {
      ay_error(AY_EARGS, fname, "w h pdata");
      return TCL_OK;
    }

  if(idr_picture_buf)
      free(idr_picture_buf);

  /* we save the final result into idr_picture_buf */
  if(!(idr_picture_buf =
       (uint32*)_TIFFmalloc(idr_picture_width*idr_picture_height*
			    sizeof(uint32))))
    {
      ay_error(AY_EOMEM, fname, NULL);
      return TCL_OK;
    }

  part = idr_partlist;
  num = 0;
  
  while(part)
    {
      part->rgba_result = (uint32*)_TIFFmalloc(idr_picture_width*
					       idr_picture_height*
					       sizeof (uint32));

      if(!idr_read_tiff(part->ImageFile, part->rgba_result,
			&w, &h, interp))
	{
	  ay_error(AY_ERROR, fname, "Error reading:");
	  ay_error(AY_ERROR, fname, part->ImageFile);
	  return TCL_OK;
	}

      idr_combine_pics(idr_picture_buf, idr_picture_width,
		       idr_picture_height, part->rgba_result,
		       w, h, part->left, part->bottom, part->alpha);

      free(part->rgba_result);

      part = part->next;
    } /* while */


  /* return width and height to tcl script */
  sprintf(strbuf, "%d", idr_picture_width);
  Tcl_SetVar(interp, argv[1], strbuf, TCL_LEAVE_ERR_MSG);
  sprintf(strbuf, "%d", idr_picture_height);
  Tcl_SetVar(interp, argv[2], strbuf, TCL_LEAVE_ERR_MSG);

  Tcl_UnsetVar(interp, argv[3], TCL_LEAVE_ERR_MSG);
  /* convert raster data to Tcl photo string */
  bpl = idr_picture_width*8+4;
  /* bytes per line: {_#rrggbb_#rrggbb_..._#rrggbb_}_ */
  string = calloc(1, idr_picture_height*bpl+1);
  if(string)
    {
      /* got TIFF, so put in Tcl list of list of colors */
      for(r = 0; r < idr_picture_height; r++)
	{
	  /* row by row */
	  /* create inner Tcl-String without Tcl_SetVar
	     because of speed :-( */
	  string[r*bpl] = '{';
	  string[r*bpl+1] = ' ';
	  for(c = 0; c < idr_picture_width; c++)
	    {
	      /* column by column */
	      bytes = (char *)&idr_picture_buf[(idr_picture_height-r-1)*
					      idr_picture_width+c];
	      string[r*bpl+c*8+2] = '#';
	      idr_char2hex(bytes[0], &string[r*bpl+c*8+3]);
	      idr_char2hex(bytes[1], &string[r*bpl+c*8+5]);
	      idr_char2hex(bytes[2], &string[r*bpl+c*8+7]);
	      string[r*bpl+c*8+9] = ' ';
	    }
	  string[r*bpl+bpl-2] = '}';
	  string[r*bpl+bpl-1] = ' ';
	}
      string[idr_picture_height*bpl-1] = 0;
      Tcl_SetVar(interp, argv[3], string, TCL_LEAVE_ERR_MSG);
      free(string);
    }
  else
    {
      /* Out of memory */
      ay_error(AY_EOMEM, fname, NULL);
    }
 
  /* free Tcl objects */
  /*
  Tcl_IncrRefCount(toa);Tcl_DecrRefCount(toa);
  Tcl_IncrRefCount(ton);Tcl_DecrRefCount(ton);
  */

 return TCL_OK;
} /* idr_combineresultstcmd */


/*
 * idr_writetifftcmd:
 *  tcl command to write rendering result to disk (via libtiff)
 *  In:
 *  clientdata, interp: Tcl stuff
 *  argc: number of arguments
 *  argv: values of arguments
 */
int
idr_writetifftcmd(ClientData clientData, Tcl_Interp *interp,
		    int argc, char *argv[])
{
 TIFF *tif;
 int i;
 char fname[] = "write_tiff";

  if(idr_picture_buf == NULL)
    {
      ay_error(AY_ERROR, fname, "No picture to be saved!");
      return TCL_OK;
    }
  if(argc != 2)
    {
      ay_error(AY_EARGS, fname, "filename");
      return TCL_OK;
    }
  tif = TIFFOpen(argv[1], "w");
  if(tif)
    {
      TIFFSetField(tif, TIFFTAG_IMAGEWIDTH, (uint32) idr_picture_width);
      TIFFSetField(tif, TIFFTAG_IMAGELENGTH, (uint32) idr_picture_height);
      TIFFSetField(tif, TIFFTAG_BITSPERSAMPLE, 8);
      TIFFSetField(tif, TIFFTAG_COMPRESSION, COMPRESSION_NONE);
      TIFFSetField(tif, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_RGB);
      TIFFSetField(tif, TIFFTAG_SAMPLESPERPIXEL, 4);
      TIFFSetField(tif, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
      TIFFSetField(tif, TIFFTAG_ROWSPERSTRIP, 1);
      TIFFSetField(tif, TIFFTAG_IMAGEDESCRIPTION,
		   "Created by Ayam-IDR Plugin.");
      for(i = idr_picture_height - 1; i >= 0; i--)
        {
	  if (TIFFWriteScanline(tif, 
	  	&idr_picture_buf[idr_picture_width*(idr_picture_height-i-1)],
		i, 0) < 0)
	    {
	      TIFFClose(tif);
	      ay_error(AY_ERROR, fname, "Error while writing TIFF!");
	      return TCL_OK;
	    }
        }
      TIFFClose(tif);
    }

 return TCL_OK;
} /* idr_writetifftcmd */


/*
 * idr_draw_cb
 *  draw function for a custom object, used to display results/bounding boxes
 *  within the ay views
 *  In:
 *  togl: Togl structure of the window to draw in
 *  o: pointer to the custom object (not used)
 */
int
idr_draw_cb(struct Togl *togl, ay_object *o)
{
 idr_picpart *part;
 GLdouble RasterPos[4];
 GLboolean Blend, DepthTest;
 GLint BlendSrc, BlendDst;
 char *name;
 int showresult;
 Tcl_Obj *to = NULL, *toa = NULL, *ton = NULL;
 
  /* Do are we in the right window ? */
  name = Tk_PathName(Togl_TkWin(togl));
  if(strcmp(name, idr_window_path) == 0)
    {
      glMatrixMode(GL_PROJECTION);
      glPushMatrix();
      glLoadIdentity();
      glOrtho(0, Togl_Width(togl), 0, Togl_Height(togl), -100.0, 100.0);
      glMatrixMode(GL_MODELVIEW);
      glPushMatrix();
      glLoadIdentity();
      /* draw in camera coordinate system */
      toa = Tcl_NewStringObj("idr",-1);
      ton = Tcl_NewStringObj("ShowResult",-1);
      to = Tcl_ObjGetVar2(Togl_Interp(togl), toa, ton, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
      Tcl_GetIntFromObj(Togl_Interp(togl), to, &showresult);
      if(showresult)
        {
	  /* show rendering result */
	  glGetBooleanv(GL_DEPTH_TEST, &DepthTest);
	  glDisable(GL_DEPTH_TEST);
	  glGetBooleanv(GL_BLEND, &Blend);
	  glEnable(GL_BLEND);
	  glGetIntegerv(GL_BLEND_SRC, &BlendSrc);
	  glGetIntegerv(GL_BLEND_DST, &BlendDst);
	  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	  glGetDoublev(GL_CURRENT_RASTER_POSITION, RasterPos);

	  part = idr_partlist;
	  while(part)
	    {
	      if(part->rgba_result)
	        {
		  glRasterPos2i(part->left+Togl_Width(togl)/2,
				part->bottom+Togl_Height(togl)/2);
		  glDrawPixels(part->right-part->left, part->top-part->bottom,
			       GL_RGBA, GL_UNSIGNED_BYTE, part->rgba_result);
	        }
	      part = part->next;
	    }
	  glRasterPos4dv(RasterPos);
	  glBlendFunc(BlendSrc, BlendDst);
	  if(DepthTest)
	      glEnable(GL_DEPTH_TEST);
	  if(Blend == GL_FALSE)
	      glDisable(GL_DEPTH_TEST);
        }
      else
        {
	  /* show bounding boxes */
	  glColor3f(1.0, 0, 0);
	  part = idr_partlist;
	  while(part)
	    {
	      glBegin(GL_LINE_LOOP);
	      glVertex3d(part->left+Togl_Width(togl)/2+1,
			 part->bottom+Togl_Height(togl)/2+1, 1);
	      glVertex3d(part->right+Togl_Width(togl)/2-1,
			 part->bottom+Togl_Height(togl)/2+1, 1);
	      glVertex3d(part->right+Togl_Width(togl)/2-1,
			 part->top+Togl_Height(togl)/2-1, 1);
	      glVertex3d(part->left+Togl_Width(togl)/2+1,
			 part->top+Togl_Height(togl)/2-1, 1);
	      glEnd();
	      part = part->next;
	    }
	  /*
	  glColor3f(1.0, 0, 1.0);
	  part = optpartlist;
	  while(part)
	    {
	      glBegin(GL_LINE_LOOP);
	      glVertex3d(part->left+Togl_Width(togl)/2,
			 part->bottom+Togl_Height(togl)/2, 1);
	      glVertex3d(part->right+Togl_Width(togl)/2,
			 part->bottom+Togl_Height(togl)/2, 1);
	      glVertex3d(part->right+Togl_Width(togl)/2,
			 part->top+Togl_Height(togl)/2, 1);
	      glVertex3d(part->left+Togl_Width(togl)/2,
			 part->top+Togl_Height(togl)/2, 1);
	      glEnd();
	      part = part->next;
	    }
	  */
        }
      glPopMatrix();
      glMatrixMode(GL_PROJECTION);
      glPopMatrix();
      glMatrixMode(GL_MODELVIEW);
    }
 return AY_OK;
} /* idr_draw_cb */


/* idr_delregion:
 *  delete important regions of view togl
 *  In:
 *  vname: view
 */
int
idr_delregion(char *vname)
{
  /* char fname[] = "idr_delregion";*/
 ay_tag_object *t = NULL, **tlast = NULL;
 double X1, Y1, X2, Y2, imp;
 char tvname[255];

  t = ay_root->tags;
  tlast = &(ay_root->tags);
  while(t)
    {

      if(t->type == idr_ridrtagtype)
	{

	  sscanf(t->val,"%s %lg %lg %lg %lg %lg", tvname,
		 &X1, &Y1, &X2, &Y2, &imp);

	  if(!strcmp(tvname, vname))
	    {
	      *tlast = t->next;
	      if(t->name)
		free(t->name);
	      if(t->val)
		free(t->val);
	      free(t);
	      t = *tlast;
	    }
	  else
	    {
	      tlast = &(t->next);
	      t = t->next;
	    }
	}
      else
	{
	  tlast = &(t->next);
	  t = t->next;
	}
    }

 return TCL_OK;
} /* idr_delregion */

/* idr_defregion_tcb:
 *  Togl callback that allows to define important regions
 *  In:
 *  togl: Togl window structure
 *  argc: number of arguments
 *  argv: argument values
 */
int
idr_defregion_tcb(struct Togl *togl, int argc, char *argv[])
{
 char fname[] = "idr_defregion";
 int ay_status = AY_OK;
 Tcl_Interp *interp = Togl_Interp(togl);
 ay_view_object *view = Togl_GetClientData(togl);
 int height = Togl_Height(togl);
 /*
 int width = Togl_Width (togl);
 int left, top, right, bottom;
 */
 double winX, winY, winX2, winY2, dtemp, imp;
 char buffer[255], *vname;
 ay_tag_object *tnew = NULL;
 ay_object *o = ay_root, *v = NULL;
 int del = AY_FALSE;

  if(argc > 5)
    {
      Tcl_GetDouble(interp, argv[2], &winX);
      Tcl_GetDouble(interp, argv[3], &winY);
      Tcl_GetDouble(interp, argv[4], &winX2);
      Tcl_GetDouble(interp, argv[5], &winY2);
      Tcl_GetDouble(interp, argv[6], &imp);
    }
  else
    {
      if(argc == 3)
	{
	  del = AY_TRUE;
	}
      else
	{
	  ay_error(AY_EARGS, fname, "winx winy winx2 winy2 importance");
	  return TCL_OK;
	}
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

  /* find view */
  v = o->down;
  while(v->next)
    {
      if(view == (ay_view_object *)(v->refine))
	{
	  vname = v->name;	  
	}

      v = v->next;
    } /* while */

  if(del)
    {
      if(vname)
	ay_status = idr_delregion(vname);
      return TCL_OK;
    }

  /* revert y */
  winY = height-winY;
  winY2 = height-winY2;


  /* create new tag */
  if(!(tnew = calloc(1, sizeof(ay_tag_object))))
    {
      ay_error(AY_EOMEM, fname, NULL);
      return TCL_OK;
    }

  /* fill new tag */
  if(!(tnew->name = calloc(5, sizeof(char))))
    {
      ay_error(AY_EOMEM, fname, NULL);
      return TCL_OK;
    }

  strcpy(tnew->name, idr_ridrtagname);
  tnew->type = idr_ridrtagtype;

  sprintf(buffer, "%s %g %g %g %g %g", vname, winX, winY, winX2, winY2, imp);

  if(!(tnew->val = calloc(strlen(buffer)+1, sizeof(char))))
    {
      ay_error(AY_EOMEM, fname, NULL);
      return TCL_OK;
    }

  strcpy(tnew->val, buffer);

  /* link new tag */
  tnew->next = o->tags;
  o->tags = tnew;

 return TCL_OK;
} /* idr_defregion_tcb */


/*
 * idr_drawroot_cb
 *  new draw callback for root object, looks for RIDR tags
 *  (important regions) and draws them, then jumps into original
 *  draw root callback
 *  In:
 *  togl: Togl structure of the window to draw in
 *  o: pointer to the root object
 */
int
idr_drawroot_cb(struct Togl *togl, ay_object *o)
{
 ay_view_object *v1 = NULL;
 ay_tag_object *t = NULL;
 ay_object *v = NULL;
 double X1, Y1, X2, Y2, imp;
 char vname[255], *rvname = NULL;
 int width = Togl_Width(togl);
 int height = Togl_Height(togl);

 t = ay_root->tags;

 v1 = Togl_GetClientData(togl);

 v = ay_root->down;
 while(v->next)
   {
     if(v1 == (ay_view_object*)(v->refine))
	{
	  rvname = v->name;
	}
     v = v->next;
   }

 while(t)
   {
     if(t->type == idr_ridrtagtype)
       {
	 sscanf(t->val,"%s %lg %lg %lg %lg %lg", vname,
		&X1, &Y1, &X2, &Y2, &imp);

	 /* are we in the right window? */
	 if(!strcmp(vname, rvname))
	   { /* yes */
	     glDisable(GL_DEPTH_TEST);
	     glMatrixMode(GL_PROJECTION);
	     glPushMatrix();
	      glLoadIdentity();
	      glOrtho(0, width, 0, height, -100.0, 100.0);
	      glMatrixMode(GL_MODELVIEW);
	      glPushMatrix();
	       glLoadIdentity();
	       glBegin(GL_LINE_LOOP);
	        glVertex3d(X1, Y1, 0.0);
		glVertex3d(X1, Y2, 0.0);
		glVertex3d(X2, Y2, 0.0);
		glVertex3d(X2, Y1, 0.0);
	       glEnd();
	      glPopMatrix();
	     glMatrixMode(GL_PROJECTION);
	    glPopMatrix();
	    glEnable(GL_DEPTH_TEST);
	    glMatrixMode(GL_MODELVIEW);
	   } /* if */
       } /* if */
      t = t->next;
   } /* while */

  /* call old draw_root callback */
  if(idr_root_drawcb)
    {
      idr_root_drawcb(togl, o);
    }

 return AY_OK;
} /* idr_drawroot_cb */


/* ay_tree_selecttcmd:
 *  select objects in the tree view
 *  replaces ay_tree_selecttcmd()
 */
int
idr_tree_selecttcmd(ClientData clientData, Tcl_Interp *interp,
		   int argc, char *argv[])
{
 int ay_status = AY_OK;
 /* char fname[] = "idr_treeSelect";*/
 ay_object *o;
 int i;

  /* assign importance to changed objects */
  ay_status = idr_assign_impchanged();

  /* free old selection */
  ay_status = ay_sel_free();

  /* update current level */
  if (argc > 1)
    {
      ay_tree_getclevel(argv[1]);
    }
  else
    {
      return TCL_OK;
    }

  /* now, add selected objects */
  i = 1;
  while (i != argc) {
      o = ay_tree_getobject(argv[i]);
      if(o)
	ay_sel_add(o);
      i++;
  }

 /* save newly selected objects for later comparison */
  ay_status = idr_save_selected();

 return TCL_OK;
} /* idr_tree_selecttcmd */


/*
 * Idr_Init
 *  setup tcl commands, togl callback and custom object
 *  In:
 *  interp: the Tcl intrepreter
 */
int
Idr_Init(Tcl_Interp *interp)
{
 char fname[] = "Idr_Init";
 int ay_status = AY_OK;
 char success_cmd[] =
   "puts stdout \"Importance Driven Rendering available now ...\"";
 void **arr = NULL;

  /* first check versions */
  if(strcmp(ay_version_ma, idr_ay_version_ma))
    {
      ay_error(AY_ERROR, fname,
	       "Plugin has been compiled for a different Ayam version!");
      ay_error(AY_ERROR, fname, "It is unsafe to continue! Bailing out...");
      return TCL_OK;
    }

  if(strcmp(ay_version_mi, idr_ay_version_mi))
    {
      ay_error(AY_ERROR, fname,
	       "Plugin has been compiled for a different Ayam version!");
      ay_error(AY_ERROR, fname, "However, it is probably safe to continue...");
    }

  /* Create Togl commands */
  Togl_CreateCommand("idr_wrib", idr_wrib_tcb);

  Togl_CreateCommand("idr_defreg", idr_defregion_tcb);

  /* Create Tcl commands */
  Tcl_CreateCommand(interp, "idrCombineResults", idr_combineresultstcmd,
		    (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "idrsaveResult", idr_writetifftcmd,
		    (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);
  
  Tcl_CreateCommand(interp, "idr_treeSelect", idr_tree_selecttcmd,
		    (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);
  

  /* register IDR tag type */
  ay_status = ay_tags_register(interp, idr_idrtagname, &idr_idrtagtype);
  if(ay_status)
    {
      ay_error(AY_ERROR, fname, "Error registering IDR tag type!");
      return TCL_OK;
    }

  /* register internal IDR tag types */
  ay_status = ay_tags_register(interp, idr_iidrtagname, &idr_iidrtagtype);
  if(ay_status)
    {
      ay_error(AY_ERROR, fname, "Error registering IIDR tag type!");
      return TCL_OK;
    }

  ay_status = ay_tags_register(interp, idr_ridrtagname, &idr_ridrtagtype);
  if(ay_status)
    {
      ay_error(AY_ERROR, fname, "Error registering RIDR tag type!");
      return TCL_OK;
    }

  ay_status = ay_tags_register(interp, idr_r3idrtagname, &idr_r3idrtagtype);
  if(ay_status)
    {
      ay_error(AY_ERROR, fname, "Error registering R3IDR tag type!");
      return TCL_OK;
    }

  ay_status = ay_tags_register(interp, idr_cidrtagname, &idr_cidrtagtype);
  if(ay_status)
    {
      ay_error(AY_ERROR, fname, "Error registering CIDR tag type!");
      return TCL_OK;
    }
  ay_status = ay_tags_register(interp, idr_ccidrtagname, &idr_ccidrtagtype);
  if(ay_status)
    {
      ay_error(AY_ERROR, fname, "Error registering CCIDR tag type!");
      return TCL_OK;
    }

  /* dirty hack! overwrite root-object drawing routine */
  arr = ay_drawcbt.arr;
  idr_root_drawcb = arr[ay_root->type];
  arr[ay_root->type] = idr_drawroot_cb;

  /* this object type allows us to draw within the ay view windows */
#if 0
  ay_register_custom_objecttype(
			     "idr_GL",
			     NULL,
			     idr_draw_cb, 
			     NULL,
			     idr_draw_cb,
			     NULL,
			     NULL,
			     NULL,
			     NULL,
			     NULL,
			     NULL,
			     NULL,
			     NULL
			     );
#endif

  /* load idr GUI extensions */
  if((Tcl_EvalFile(interp, "idr.tcl")) != TCL_OK)
    {
      ay_error(AY_ERROR, fname,
	       "Error while sourcing \\\"idr.tcl\\\"!");
      return TCL_OK;
    }

  Tcl_Eval(interp, success_cmd);

 return TCL_OK;
} /* Idr_Init */
