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

/* undo.c - undo/redo related functions */

/* types local to this module */
typedef struct ay_undo_object_s
{
  char *operation;
  ay_object *objects;
  ay_list_object *references;
} ay_undo_object;

/* prototypes of functions local to this module */
int ay_undo_deletemulti(ay_object *o);

int ay_undo_clearuo(ay_undo_object *uo);

int ay_undo_copymat(ay_mat_object *src, ay_mat_object *dst);

int ay_undo_copyview(ay_view_object *src, ay_view_object *dst);

int ay_undo_copyroot(ay_root_object *src, ay_root_object *dst);

int ay_undo_copyselp(ay_object *src, ay_object *dst);

int ay_undo_copy(ay_undo_object *uo);

int ay_undo_redo(void);

int ay_undo_undo(void);

int ay_undo_save(void);

int ay_undo_copysave(ay_object *src, ay_object **dst);

/* global variables */
static ay_undo_object *undo_buffer;

static int undo_current;

static int undo_buffer_size;

static int undo_last_op; /* last undo operation: -1: no op,
			    1-4: see mode variable in ay_undo_undotcmd() */

static char *undo_saved_op; /* name of saved modelling operation */

/* functions */

/* ay_undo_init:
 *  initialize the undo system by allocating the undo buffer 
 */
int
ay_undo_init(int buffer_size)
{

  if(undo_buffer)
    free(undo_buffer);
  undo_buffer = NULL;

  if(buffer_size <= 0)
    {
      undo_buffer_size = -1;
      return AY_OK;
    }

  if(!(undo_buffer = calloc(buffer_size, sizeof(ay_undo_object))))
    return AY_EOMEM;
  
  undo_current = -1;
  undo_buffer_size = buffer_size;

  undo_last_op = -1; /* no op */

  if(undo_saved_op)
    free(undo_saved_op);
  undo_saved_op = NULL;

 return AY_OK;
} /* ay_undo_init */


/* ay_undo_deletemulti:
 *  delete some connected objects
 */
int
ay_undo_deletemulti(ay_object *o)
{
 int ay_status = AY_OK;
 ay_object *next = NULL, *d = NULL;
 ay_view_object *view = NULL;
 ay_mat_object *material = NULL;
 ay_deletecb *dcb = NULL;
 void **arr = NULL;

  if(!o)
    return AY_ENULL;
  
  d = o;
  while(d)
    {
      next = d->next;
      switch(d->type)
	{
	case AY_IDVIEW:
	  view = (ay_view_object *)d->refine;
	  if(view->bgimage)
	    free(view->bgimage);
	  free(view);
	  /* delete selected points */
	  if(d->selp)
	    ay_selp_clear(d);
	  /* delete tags */
	  ay_tags_delall(d);
	  /* free name */
	  if(d->name)
	    free(d->name);
	  /* now, free generic object */
	  free(d);
	  break;
	case AY_IDINSTANCE:
	  /* delete selected points */
	  if(d->selp)
	    ay_selp_clear(d);
	  /* delete tags */
	  ay_tags_delall(d);
	  /* free name */
	  if(d->name)
	    free(d->name);
	  /* now, free generic object */
	  free(d);
	  break;
	case AY_IDMATERIAL:
	  material = (ay_mat_object *)(d->refine);

	  if(material->sshader)
	    {
	      ay_shader_free(material->sshader);
	      material->sshader = NULL;
	    }
	  if(material->dshader)
	    {
	      ay_shader_free(material->dshader);
	      material->dshader = NULL;
	    }
	  if(material->ishader)
	    {
	      ay_shader_free(material->ishader);
	      material->ishader = NULL;
	    }
	  if(material->eshader)
	    {
	      ay_shader_free(material->eshader);
	      material->eshader = NULL;
	    }

	  free(material);

	  /* delete selected points */
	  if(d->selp)
	    ay_selp_clear(d);
	  /* delete tags */
	  ay_tags_delall(d);
	  /* free name */
	  if(d->name)
	    free(d->name);
	  /* now, free generic object */
	  free(d);
	  break;
	default:
	  arr = ay_deletecbt.arr;
	  dcb = (ay_deletecb*)(arr[d->type]);
	  if(dcb)
	    ay_status = dcb(d->refine);
	  
	  if(ay_status)
	    {
	      return ay_status;
	    }
	  /* delete selected points */
	  if(d->selp)
	    ay_selp_clear(d);
	  /* delete tags */
	  ay_tags_delall(d);
	  /* free name */
	  if(d->name)
	    free(d->name);
	  /* now, free generic object */
	  free(d);

	  break;
	} /* switch */
      d = next;
    } /* while */

 return AY_OK;
} /* ay_undo_deletemulti */


/* ay_undo_clearuo:
 *  clear a single undo object
 */
int
ay_undo_clearuo(ay_undo_object *uo)
{
 ay_list_object *lo = NULL;

  while(uo->references)
    {
      lo = uo->references->next;
      free(uo->references);
      uo->references = lo;
    }

  if(uo->objects)
    {
      ay_undo_deletemulti(uo->objects);
    }
  uo->objects = NULL;

  if(uo->operation)
    free(uo->operation);
  uo->operation = NULL;

 return AY_OK;
} /* ay_undo_clearuo */


/* ay_undo_copymat:
 */
int
ay_undo_copymat(ay_mat_object *src, ay_mat_object *dst)
{
 int ay_status = AY_OK;
 char **onameptr;
 unsigned int *orefcountptr;
 ay_object *oobjptr;
 int oregistered;

  if(dst->sshader)
    {
      ay_status = ay_shader_free(dst->sshader);
      dst->sshader = NULL;
    }

  if(dst->dshader)
    {
      ay_status = ay_shader_free(dst->dshader);
      dst->dshader = NULL;
    }

  if(dst->ishader)
    {
      ay_status = ay_shader_free(dst->ishader);
      dst->ishader = NULL;
    }

  if(dst->eshader)
    {
      ay_status = ay_shader_free(dst->eshader);
      dst->eshader = NULL;
    }

  oregistered = dst->registered;
  onameptr = dst->nameptr;
  orefcountptr = dst->refcountptr;
  oobjptr = dst->objptr;

  memcpy(dst, src, sizeof(ay_mat_object));

  dst->registered = oregistered;
  dst->nameptr = onameptr;
  dst->refcountptr = orefcountptr;
  dst->objptr = oobjptr;

  if(src->sshader)
    {
      ay_status = ay_shader_copy(src->sshader, &(dst->sshader));
    }

  if(src->dshader)
    {
      ay_status = ay_shader_copy(src->dshader, &(dst->dshader));
    }

  if(src->ishader)
    {
      ay_status = ay_shader_copy(src->ishader, &(dst->ishader));
    }

  if(src->eshader)
    {
      ay_status = ay_shader_copy(src->eshader, &(dst->eshader));
    }


 return AY_OK;
}/* ay_undo_copymat */


/* ay_undo_copyview:
 */
int
ay_undo_copyview(ay_view_object *src, ay_view_object *dst)
{
 struct Togl *otogl;

  otogl = dst->togl;

  memcpy(dst, src, sizeof(ay_view_object));

  dst->togl = otogl;
  dst->dirty = AY_TRUE;
  dst->bgimage = NULL;

  /* copy BGImage */
  if(src->bgimage)
    {
      if(!(dst->bgimage = calloc(strlen(src->bgimage)+1, sizeof(char))))
	{
	  return AY_EOMEM;
	}
      strcpy(dst->bgimage, src->bgimage);
    }

 return AY_OK;
}/* ay_undo_copyview */


/* ay_undo_copyroot:
 */
int
ay_undo_copyroot(ay_root_object *src, ay_root_object *dst)
{
 int ay_status = AY_OK;
 ay_riopt_object *srcriopt = NULL, *dstriopt = NULL;

  if(dst->imager)
    {
      ay_status = ay_shader_free(dst->imager);
      dst->imager = NULL;
    }

  if(src->imager)
    {
      ay_status = ay_shader_copy(src->imager, &(dst->imager));
    }

  if(dst->atmosphere)
    {
      ay_status = ay_shader_free(dst->atmosphere);
      dst->atmosphere = NULL;
    }

  if(src->atmosphere)
    {
      ay_status = ay_shader_copy(src->atmosphere, &(dst->atmosphere));
    }

  /* copy RiOptions */
  srcriopt = src->riopt;
  dstriopt = dst->riopt;
  if(dstriopt)
    {
      if(dstriopt->textures)
	{
	  free(dstriopt->textures);
	  dstriopt->textures = NULL;
	}
      if(dstriopt->shaders)
	{
	  free(dstriopt->shaders);
	  dstriopt->shaders = NULL;
	}
      if(dstriopt->archives)
	{
	  free(dstriopt->archives);
	  dstriopt->archives = NULL;
	}
      if(dstriopt->procedurals)
	{
	  free(dstriopt->procedurals);
	  dstriopt->procedurals = NULL;
	}
      free(dst->riopt);
    }
  dst->riopt = NULL;
  if(!(dst->riopt = calloc(1, sizeof(ay_riopt_object))))
    return AY_EOMEM;

  memcpy(dst->riopt, src->riopt, sizeof(ay_riopt_object));
  dstriopt = dst->riopt;

  if(srcriopt->textures)
    {
      dstriopt->textures = NULL;
      if(!(dstriopt->textures = calloc(strlen(srcriopt->textures)+1,
				       sizeof(char))))
	return AY_EOMEM;
      strcpy(dstriopt->textures, srcriopt->textures);
    }

  if(srcriopt->shaders)
    {
      dstriopt->shaders = NULL;
      if(!(dstriopt->shaders = calloc(strlen(srcriopt->shaders)+1,
				       sizeof(char))))
	return AY_EOMEM;
      strcpy(dstriopt->shaders, srcriopt->shaders);
    }

  if(srcriopt->archives)
    {
      dstriopt->archives = NULL;
      if(!(dstriopt->archives = calloc(strlen(srcriopt->archives)+1,
				       sizeof(char))))
	return AY_EOMEM;
      strcpy(dstriopt->archives, srcriopt->archives);
    }

  if(srcriopt->procedurals)
    {
      dstriopt->procedurals = NULL;
      if(!(dstriopt->procedurals = calloc(strlen(srcriopt->procedurals)+1,
				       sizeof(char))))
	return AY_EOMEM;
      strcpy(dstriopt->procedurals, srcriopt->procedurals);
    }

 return AY_OK;
}/* ay_undo_copyroot */


/* ay_undo_copyselp:
 */
int
ay_undo_copyselp(ay_object *src, ay_object *dst)
{
 ay_point_object *p = NULL, *n = NULL, **last = NULL;

  if(dst->selp)
    {
      ay_selp_clear(dst);
    }

  p = src->selp;
  last = &(dst->selp);
  while(p)
    {
      n = NULL;
      if(!(n = calloc(1, sizeof(ay_point_object))))
	return AY_EOMEM;

      memcpy(n, p, sizeof(ay_point_object));
      *last = n;
      last = &(n->next);

      p = p->next;
    }

 return AY_OK;
}/* ay_undo_copyselp */


/* ay_undo_copy:
 *  copy objects from undo object uo
 *  back to scene (in-place) using references
 *  information
 */
int
ay_undo_copy(ay_undo_object *uo)
{
 int ay_status = AY_OK;
 ay_object *o = NULL, *c = NULL, *m = NULL;
 ay_list_object *r = NULL;
 void **arr = NULL;
 ay_deletecb *dcb = NULL;
 ay_copycb *ccb = NULL;
 char view_repairtitle_cmd[] = "viewRepairTitle ", buf[64];
 char view_setgridicon_cmd[] = "viewSetGridIcon .";
 char view_setmodeicon_cmd[] = "viewSetModeIcon .";
 Tcl_DString ds;
 int notify = AY_TRUE;

  if(!uo)
    return AY_OK;

  r = uo->references;
  c = uo->objects;
  while(r)
    {

      o = r->object;

      if(!o)
	{
	  r = r->next;
	  c = c->next;
	  break;
	}

      /* copy name */
      if(o->type != AY_IDMATERIAL)
	{

	  if(o->name)
	    {
	      free(o->name);
	      o->name = NULL;
	    }
	  if(c->name)
	    {
	      if(!(o->name = calloc(strlen(c->name)+1, sizeof(char))))
		return AY_EOMEM;
	      strcpy(o->name, c->name);
	    }

	}

      /* copy tags */
      if(o->tags)
	{
	  ay_status = ay_tags_delall(o);
	}
      ay_status = ay_tags_copyall(c, o);

      /* copy type specific part */
      switch(c->type)
	{
	case AY_IDVIEW:
	  ay_status = ay_undo_copyview((ay_view_object *)(c->refine),
				       (ay_view_object *)(o->refine));

	  if(ay_status)
	    {
	      return ay_status;
	    }

	  /* repair title of view window */
	  Tcl_DStringInit(&ds);
	  Tcl_DStringAppend(&ds, view_repairtitle_cmd, -1);
	  Tcl_DStringAppend(&ds, o->name, -1);
	  sprintf(buf, " %d", ((ay_view_object *)(c->refine))->type);
	  Tcl_DStringAppend(&ds, buf, -1);
	  Tcl_Eval(ay_interp, Tcl_DStringValue(&ds));
	  Tcl_DStringFree(&ds);
	  /* set grid icon of view window */
	  Tcl_DStringInit(&ds);
	  Tcl_DStringAppend(&ds, view_setgridicon_cmd, -1);
	  Tcl_DStringAppend(&ds, o->name, -1);
	  memset(buf, 0, sizeof(buf));
	  sprintf(buf, " %g", ((ay_view_object *)(c->refine))->grid);
	  Tcl_DStringAppend(&ds, buf, -1);
	  Tcl_Eval(ay_interp, Tcl_DStringValue(&ds));
	  Tcl_DStringFree(&ds);
	  /* set mode icon of view window */
	  Tcl_DStringInit(&ds);
	  Tcl_DStringAppend(&ds, view_setmodeicon_cmd, -1);
	  Tcl_DStringAppend(&ds, o->name, -1);
	  memset(buf, 0, sizeof(buf));
	  sprintf(buf, " %d", ((ay_view_object *)(c->refine))->shade);
	  Tcl_DStringAppend(&ds, buf, -1);
	  Tcl_Eval(ay_interp, Tcl_DStringValue(&ds));
	  Tcl_DStringFree(&ds);

	  break;
	case AY_IDROOT:
	  ay_status = ay_undo_copyroot((ay_root_object *)(c->refine),
				       (ay_root_object *)(o->refine));

	  if(ay_status)
	    {
	      return ay_status;
	    }
	  break;
	case AY_IDINSTANCE:
	  o->refine = c->refine;
	  break;
	case AY_IDMATERIAL:
	  if(o->name)
	    {
	      ay_matt_deregister(o->name);
	      free(o->name);
	      o->name = NULL;
	    }
	  if(c->name)
	    {
	      ay_matt_registermaterial(c->name,
				       (ay_mat_object *)(o->refine));
	      if(!(o->name = calloc(strlen(c->name)+1, sizeof(char))))
		return AY_EOMEM;
	      strcpy(o->name, c->name);
	    }
	    
	  ay_status = ay_undo_copymat((ay_mat_object *)(c->refine),
				      (ay_mat_object *)(o->refine));

	  if(ay_status)
	    {
	      return ay_status;
	    }
	  break;
	default:
	  arr = ay_deletecbt.arr;
	  dcb = (ay_deletecb*)(arr[c->type]);
	  if(dcb)
	    ay_status = dcb(o->refine);

	  arr = ay_copycbt.arr;
	  ccb = (ay_copycb*)(arr[c->type]);
	  if(ccb)
	    ay_status = ccb(c->refine, &(o->refine));

	  if(ay_status)
	    {
	      return ay_status;
	    }
	  break;
	} /* switch */

      /* copy trafos */
      ay_trafo_copy(c, o);

      if(o->type != AY_IDMATERIAL)
	{
	  /* copy material */
	  if(!o->mat && c->mat)
	    {
	      m = c->mat->objptr;
	      m->refcount++;
	    }
	  if(o->mat && !c->mat)
	    {
	      m = o->mat->objptr;
	      m->refcount--;
	    }
	  if(o->mat && c->mat)
	    {
	      m = o->mat->objptr;
	      m->refcount--;
	      m = c->mat->objptr;
	      m->refcount++;
	    }

	  o->mat = c->mat;
	}

      /* copy selected points? No! */
      /*ay_status = ay_undo_copyselp(c, o);*/
      ay_selp_clear(o);
      
      /* copy all other attributes */
      o->type = c->type;
      /*o->selected = c->selected;*/
      o->modified = c->modified;
      o->parent = c->parent;
      o->inherit_trafos = c->inherit_trafos;
      o->hide = c->hide;
      o->hide_children = c->hide_children;

      if((c->type != AY_IDVIEW) &&
	 (c->type != AY_IDROOT) && notify)
	{
	  ay_notify_forceparent(o);
	  notify = AY_FALSE;
	}

      c = c->next;
      r = r->next;
    }


 return AY_OK;
} /* ay_undo_copy */


/* ay_undo_redo:
 *  
 */
int
ay_undo_redo(void)
{
 char fname[] = "redo";
 int ay_status = AY_OK;

  if(undo_current == (undo_buffer_size-1))
    {
      ay_error(AY_ERROR, fname, "No further redo info available!");
      return AY_ERROR;
    }

  if((undo_buffer[undo_current+1]).references == NULL)
    {
      ay_error(AY_ERROR, fname, "No further redo info available!");
      return AY_ERROR;
    }

  if(undo_last_op == 0)
    { /* if last op was an undo, we jump an extra step */
      undo_current++;
    }

  undo_current++;

  ay_status = ay_undo_copy(&(undo_buffer[undo_current]));

  undo_last_op = 1;

 return AY_OK;
} /* ay_undo_redo */


/* ay_undo_undo:
 *  
 */
int
ay_undo_undo(void)
{
 char fname[] = "undo";
 int ay_status = AY_OK;

  if(undo_current < 0)
    {
      ay_error(AY_ERROR, fname, "No further undo info available!");
      return AY_ERROR;
    }

  if(undo_last_op == 2)
    { /* if last op was a save, we need to save current state too,
         to allow the user to get back to current state with redo */
      ay_status = ay_undo_save();
      undo_current--;
    }

  if(undo_last_op == 1)
    { /* if last operation was a redo, we jump an extra step */
      undo_current--;
    }

  ay_status = ay_undo_copy(&(undo_buffer[undo_current]));

  undo_current--;

  if(undo_current < 0)
    undo_current = -1;

  undo_last_op = 0;

 return AY_OK;
} /* ay_undo_undo */


/* ay_undo_copysave:
 *  copy object to undo buffer
 */
int
ay_undo_copysave(ay_object *src, ay_object **dst)
{
 int ay_status = AY_OK;
 char fname[] = "undo_copysave";
 ay_object *new = NULL;
 void **arr = NULL;
 ay_copycb *cb = NULL;
 ay_view_object *srcview = NULL, *dstview = NULL;
 ay_root_object *root = NULL;

  if(!src || !dst)
    return AY_ENULL;

  /* copy generic object */
  if(!(new = calloc(1, sizeof(ay_object))))
    return AY_EOMEM;

  *dst = new;

  memcpy(new, src, sizeof(ay_object));
  /* danger! links point to original hierarchy */

  new->next = NULL;
  new->down = NULL;
  new->selp = NULL;
  new->tags = NULL;
  /*  new->mat = NULL;*/

  /*if(src->type != AY_IDMATERIAL)*/
  new->refcount = 0;

  /* copy type specific part */
  switch(src->type)
    {
    case AY_IDVIEW:
      srcview = (ay_view_object *)(src->refine);
      
      if(!(new->refine = calloc(1, sizeof(ay_view_object))))
	return AY_EOMEM;
      dstview = (ay_view_object *)(new->refine);

      memcpy(dstview, srcview, sizeof(ay_view_object));
      dstview->bgimage = NULL;

      /* copy BGImage */
      if(srcview->bgimage)
	{
	  if(!(dstview->bgimage = calloc(strlen(srcview->bgimage)+1,
					 sizeof(char))))
	    {
	      return AY_EOMEM;
	    }

	  strcpy(dstview->bgimage, srcview->bgimage);
	}
      break;
    case AY_IDROOT:
      root = (ay_root_object *)(src->refine);

      if(!(new->refine = calloc(1, sizeof(ay_root_object))))
	return AY_EOMEM;

      ay_status = ay_undo_copyroot((ay_root_object *)(src->refine),
				   (ay_root_object *)(new->refine));

      if(ay_status)
	{
	  return ay_status;
	}
      break;
    case AY_IDINSTANCE:
      new->refine = src->refine;
      break;
    default:
      arr = ay_copycbt.arr;
      cb = (ay_copycb*)(arr[src->type]);
      if(cb)
	ay_status = cb(src->refine, &(new->refine));

      if(ay_status)
	{
	  ay_error(AY_ERROR, fname, "copy callback failed");
	  free(new);
	  return AY_ERROR;
	}
      break;
    } /* switch */

  /* copy name */
  if(src->name)
    {
      if(!(new->name = calloc(strlen(src->name)+1, sizeof(char))))
	{
	  free(new);
	  return AY_EOMEM;
	}

      strcpy(new->name, src->name);
    }

  /* copy tags */
  ay_status = ay_tags_copyall(src, new);

  new->modified = AY_TRUE;

 return AY_OK;
} /* ay_undo_copysave */


/* ay_undo_save:
 *  
 */
int
ay_undo_save(void)
{
 int ay_status = AY_OK;
 ay_undo_object *uo = NULL, *uo2 = NULL;
 ay_list_object *sel = ay_selection, *r = NULL, *lastr = NULL;
 ay_object **nexto = NULL, *view = NULL;
 int i;

  /* we always save all views now */
  if((!sel) && (!ay_root->down->next))
    return AY_OK;

  /* check, whether we operate on top of undo buffer */
  if(undo_current+1 == undo_buffer_size)
    {
      /* yes, we need to clear the oldest saved state */
      /* and then shift all states, so that the top  */
      /* of the undo buffer is empty again */

      /* clear */
      uo = &(undo_buffer[0]);
      ay_status = ay_undo_clearuo(uo);
      /* shift */
      for(i = 0; i < undo_buffer_size-1; i++)
	{
	  uo = &(undo_buffer[i]);
	  uo2 = &(undo_buffer[i+1]);

	  uo->objects = uo2->objects;
	  uo->references = uo2->references;
	}
      uo = &(undo_buffer[undo_buffer_size-1]);
      uo->objects = NULL;
      uo->references = NULL;
    }
  else
    {
      undo_current++;
      /* to avoid, that the user gets (after a undo/redo)
       * into an old sequence of changed states using
       * redo after a save, we clear the undo buffer
       * now partially
       */
      if((undo_last_op == 0) || (undo_last_op == 1))
	{

	  for(i = undo_current; i < undo_buffer_size; i++)
	    {
	      uo = &(undo_buffer[i]);
	      ay_status = ay_undo_clearuo(uo);
	    }
	}

    } /* if */

  uo = &(undo_buffer[undo_current]);
  /* check, whether the current undo slot contains saved objects */
  if(uo->objects)
    { /* yes, free them */
      ay_status = ay_undo_clearuo(uo);
    }

  /* link name of saved modelling operation to this undo object */
  uo->operation = undo_saved_op;
  undo_saved_op = NULL;

  /* finally, we may copy all currently selected objects 
   * and references to the original objects to the undo buffer
   */

  sel = ay_selection;
  nexto = &(uo->objects);
  while(sel)
    {
      /* XXXX remove this test, if copying of
       * root objects (if only for undo)
       * is implemented

      if(sel->object->type != AY_IDROOT)
	{
       */
	  /* copy reference */
	  r = NULL;
	  if(!(r = calloc(1,sizeof(ay_list_object))))
	    {
	      return AY_EOMEM;
	    }

	  if(uo->references)
	    {
	      lastr->next = r;
	    }
	  else
	    {
	      uo->references = r;
	    }

	  lastr = r;

	  r->object = sel->object;

	  /* copy object */
     	  ay_status = ay_undo_copysave(sel->object, nexto);
	  if(ay_status)
	    return ay_status;

	  nexto = &((*nexto)->next);
	  /*}*/
      sel = sel->next;
    } /* while */

  /* save all views */
  view = ay_root->down;
  while(view->next)
    {
      /* copy reference */
      r = NULL;
      if(!(r = calloc(1,sizeof(ay_list_object))))
	{
	  return AY_EOMEM;
	}

      if(uo->references)
	{
	  lastr->next = r;
	}
      else
	{
	  uo->references = r;
	}
      
      lastr = r;

      r->object = view;

      /* copy object */
      ay_status = ay_undo_copysave(view, nexto);
      if(ay_status)
	return ay_status;

      nexto = &((*nexto)->next);
      view = view->next;
    } /* while */

  undo_last_op = 2;

 return AY_OK;
} /* ay_undo_save */


/* ay_undo_clear:
 *  clear all undo information
 */
int
ay_undo_clear(void)
{
 int ay_status = AY_OK;
 int i = 0;
 ay_undo_object *uo = NULL;

  for(i = 0; i < undo_buffer_size-1; i++)
    {
      uo = &(undo_buffer[i]);
      ay_status = ay_undo_clearuo(uo);
    }

  undo_current = -1;
  undo_last_op = -1; /* no op */

  if(undo_saved_op)
    free(undo_saved_op);
  undo_saved_op = NULL;

 return AY_OK;
} /* ay_undo_clear */


/* ay_undo_undotcmd:
 *
 */
int
ay_undo_undotcmd(ClientData clientData, Tcl_Interp *interp,
		 int argc, char *argv[])
{
 int ay_status = AY_OK;
 char fname[] = "undo";
 int mode = 0; /* default mode is "undo" */
 char *a = "ay", *n = "sc", *v = "1";
 char *n2 = "uc", *n3 = "undoo", *n4 = "redoo", v2[64];
 char vnone[] = "none", vnull[] = "null";
 static int uc = 0;

  /* parse args */
  if(argc > 1)
    {
      if(!strcmp(argv[1], "redo"))
	{
	  mode = 1;
	}
      else
	{
	  if(!strcmp(argv[1], "save"))
	    {
	      mode = 2;
	    }
	  else
	    {
	      if(!strcmp(argv[1], "clear"))
		{
		  mode = 3;
		}
	      else
		{
		  ay_error(AY_EARGS, fname, "redo|save|clear");
		  return TCL_OK;
		} /* if */
	    } /* if */
	} /* if */
    } /* if */

  /* protect undo code from too small buffers */
  if(undo_buffer_size < 2)
    {
      if(mode == 2)
	{
	  /* set scene changed flag */
	  Tcl_SetVar2(interp, a, n, v, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
	}
      return TCL_OK;
    } /* if */

  switch(mode)
    {
    case 0:
      ay_status = ay_undo_undo();
      if(!ay_status)
	{
	  uc--;
	  if(undo_current+1 > 0)
	    {
	      if((undo_buffer[undo_current+1]).operation)
		Tcl_SetVar2(interp, a, n3,
			    (undo_buffer[undo_current+1]).operation,
			    TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
	      else
		Tcl_SetVar2(interp, a, n3, vnull,
			    TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
	    }
	  else
	    {
	      Tcl_SetVar2(interp, a, n3, vnone,
			  TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
	    }
	  if(undo_current+2 < undo_buffer_size)
	    {
	      if((undo_buffer[undo_current+2]).operation)
		Tcl_SetVar2(interp, a, n4,
			    (undo_buffer[undo_current+2]).operation,
			    TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
	      else
		Tcl_SetVar2(interp, a, n4, vnull,
			    TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
	    }
	}
      break;
    case 1:
      ay_status = ay_undo_redo();
      if(!ay_status)
	{
	  uc++;
	  if(undo_current > -1)
	    {
	      if((undo_buffer[undo_current]).operation)
		Tcl_SetVar2(interp, a, n3,
			    (undo_buffer[undo_current]).operation,
			    TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
	      else
		Tcl_SetVar2(interp, a, n3, vnull,
			    TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
	    }
	  if(undo_current+1 < undo_buffer_size)
	    {
	      if((undo_buffer[undo_current+1]).operation)
		Tcl_SetVar2(interp, a, n4,
			    (undo_buffer[undo_current+1]).operation,
			    TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
	      else
		Tcl_SetVar2(interp, a, n4, vnull,
			    TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
	    }
	  else
	    {
	      Tcl_SetVar2(interp, a, n4, vnone,
			  TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
	    }
	}
      break;
    case 2:
      ay_status = ay_undo_save();
      uc++;
      /* save name of modelling operation in progress */
      if(argc > 2)
	{
	  if(undo_saved_op)
	    free(undo_saved_op);
	  
	  undo_saved_op = NULL;
	  
	  if(!(undo_saved_op = calloc(strlen(argv[2]) + 1,
				      sizeof(char))))
	    {
	      ay_error(AY_EOMEM, fname, NULL);
	      return TCL_OK;
	    }
	  else
	    {
	      strcpy(undo_saved_op, argv[2]);
	    }
	}
      /* set scene changed flag */
      Tcl_SetVar2(interp, a, n, v, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
      if(undo_saved_op)
	Tcl_SetVar2(interp, a, n3, undo_saved_op,
		    TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
      Tcl_SetVar2(interp, a, n4, vnone, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
      break;
    case 3:
      ay_status = ay_undo_clear();
      uc = 0;
      Tcl_SetVar2(interp, a, n3, vnone, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
      Tcl_SetVar2(interp, a, n4, vnone, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
      break;
    default:
      break;
    } /* switch */

  if(uc > undo_buffer_size)
    uc = undo_buffer_size;

  if((uc < 1) && (mode != 3))
    uc = 1;

  /* set (user visible) undo buffer pointer */
  sprintf(v2, "%d", uc);
  Tcl_SetVar2(interp, a, n2, v2, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

 return TCL_OK;
} /* ay_undo_undotcmd */

