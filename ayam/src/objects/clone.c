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

/* clone.c - clone object */

static char *ay_clone_name = "Clone";

int
ay_clone_createcb(int argc, char *argv[], ay_object *o)
{
 char fname[] = "crtclone";
 ay_clone_object *new = NULL;

  if(!o)
    return AY_ENULL;

  if(!(new = calloc(1, sizeof(ay_clone_object))))
    {
      ay_error(AY_EOMEM, fname, NULL);
      return AY_ERROR;
    }

  new->scalx = 1.0;
  new->scaly = 1.0;
  new->scalz = 1.0;

  new->quat[3] = 1.0;

  o->parent = AY_TRUE;

  o->refine = new;

 return AY_OK;
} /* ay_clone_createcb */


int
ay_clone_deletecb(void *c)
{
 ay_clone_object *clone = NULL;
 ay_object *t = NULL;

  if(!c)
    return AY_ENULL;    

  clone = (ay_clone_object *)(c);

  while(clone->clones)
    {
      t = clone->clones;
      clone->clones = t->next;
      free(t);
    }

  free(clone);

 return AY_OK;
} /* ay_clone_deletecb */


int
ay_clone_copycb(void *src, void **dst)
{
 ay_clone_object *clone = NULL, *clonesrc = NULL;

  if(!src || !dst)
    return AY_ENULL;

  clonesrc = (ay_clone_object *)src;

  if(!(clone = calloc(1, sizeof(ay_clone_object))))
    return AY_EOMEM; 

  memcpy(clone, src, sizeof(ay_clone_object)); 

  clone->clones = NULL;

  *dst = (void *)clone;

 return AY_OK;
} /* ay_clone_copycb */


int
ay_clone_drawcb(struct Togl *togl, ay_object *o)
{
 ay_clone_object *cc = NULL;
 ay_object *c = NULL;

  if(!o)
    return AY_ENULL;

  cc = (ay_clone_object *)o->refine;
  
  c = cc->clones;
  while(c)
    {
      ay_draw_object(togl, c, AY_TRUE);
      c = c->next;
    }

 return AY_OK;
} /* ay_clone_drawcb */


int
ay_clone_shadecb(struct Togl *togl, ay_object *o)
{
 ay_clone_object *cc = NULL;
 ay_object *c = NULL;

  if(!o)
    return AY_ENULL;

  cc = (ay_clone_object *)o->refine;
  
  c = cc->clones;
  while(c)
    {
      ay_shade_object(togl, c, AY_FALSE);
      c = c->next;
    }

 return AY_OK;
} /* ay_clone_shadecb */


int
ay_clone_drawhcb(struct Togl *togl, ay_object *o)
{

 return AY_OK;
} /* ay_clone_drawhcb */


int
ay_clone_getpntcb(int mode, ay_object *o, double *p)
{

 return AY_OK;
} /* ay_clone_getpntcb */


/* Tcl -> C! */
int
ay_clone_setpropcb(Tcl_Interp *interp, int argc, char *argv[], ay_object *o)
{
 int ay_status = AY_OK;
 char *n1 = "CloneAttrData";
 /*char fname[] = "clone_setpropcb";*/
 Tcl_Obj *to = NULL, *toa = NULL, *ton = NULL;
 double dtemp;
 double xaxis[3] = {1.0,0.0,0.0};
 double yaxis[3] = {0.0,1.0,0.0};
 double zaxis[3] = {0.0,0.0,1.0};
 double quat[4], drot;
 ay_clone_object *clone = NULL;
 int pasteProp = AY_FALSE;

  if(!o)
    return AY_ENULL;

  clone = (ay_clone_object *)o->refine;
  
  toa = Tcl_NewStringObj(n1,-1);
  ton = Tcl_NewStringObj(n1,-1);

  Tcl_SetStringObj(ton,"NumClones",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp,to, &(clone->numclones));

  if(clone->numclones < 0)
    clone->numclones = 0;

  Tcl_SetStringObj(ton,"Rotate",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp,to, &(clone->rotate));

  Tcl_SetStringObj(toa, n1, -1);
  Tcl_SetStringObj(ton, "Translate_X", -1);
  to = Tcl_ObjGetVar2(interp, toa, ton, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetDoubleFromObj(interp, to, &(clone->movx));
  Tcl_SetStringObj(ton, "Translate_Y", -1);
  to = Tcl_ObjGetVar2(interp, toa, ton, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetDoubleFromObj(interp, to, &(clone->movy));
  Tcl_SetStringObj(ton, "Translate_Z", -1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetDoubleFromObj(interp,to, &(clone->movz));

  Tcl_SetStringObj(ton, "Quat0", -1);
  to = Tcl_ObjGetVar2(interp, toa, ton, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetDoubleFromObj(interp, to, &dtemp);
  clone->quat[0] = dtemp;
  Tcl_SetStringObj(ton, "Quat1", -1);
  to = Tcl_ObjGetVar2(interp, toa, ton, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetDoubleFromObj(interp, to, &dtemp);
  clone->quat[1] = dtemp;
  Tcl_SetStringObj(ton, "Quat2", -1);
  to = Tcl_ObjGetVar2(interp, toa, ton, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetDoubleFromObj(interp, to, &dtemp);
  clone->quat[2] = dtemp;
  Tcl_SetStringObj(ton, "Quat3", -1);
  to = Tcl_ObjGetVar2(interp, toa, ton, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetDoubleFromObj(interp, to, &dtemp);
  clone->quat[3] = dtemp;

  Tcl_SetStringObj(ton, "Rotate_X", -1);
  to = Tcl_ObjGetVar2(interp, toa, ton, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetDoubleFromObj(interp, to, &dtemp);
  if(clone->rotx != dtemp)
    {
      if(!pasteProp)
	{
	  drot = (clone->rotx - dtemp);
	  ay_quat_axistoquat(xaxis, AY_D2R(drot), quat);
	  ay_quat_add(quat, clone->quat, clone->quat);
	}
      clone->rotx = dtemp;
    }
  Tcl_SetStringObj(ton, "Rotate_Y", -1);
  to = Tcl_ObjGetVar2(interp, toa, ton, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetDoubleFromObj(interp, to, &dtemp);
  if(clone->roty != dtemp)
    {
      if(!pasteProp)
	{     
	  drot = (clone->roty - dtemp);
	  ay_quat_axistoquat(yaxis, AY_D2R(drot), quat);
	  ay_quat_add(quat, clone->quat, clone->quat);
	}
      clone->roty = dtemp;
    }
  Tcl_SetStringObj(ton, "Rotate_Z", -1);
  to = Tcl_ObjGetVar2(interp, toa, ton, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetDoubleFromObj(interp, to, &dtemp);
  if(clone->rotz != dtemp)
    {
      if(!pasteProp)
	{
	  drot = (clone->rotz - dtemp);
	  ay_quat_axistoquat(zaxis, AY_D2R(drot), quat);
	  ay_quat_add(quat, clone->quat, clone->quat);
	}
      clone->rotz = dtemp;
    }

  if(clone->rotx == 0.0 && clone->roty == 0.0 && clone->rotz == 0.0)
    {
      clone->quat[0] = 0.0;
      clone->quat[1] = 0.0;
      clone->quat[2] = 0.0;
      clone->quat[3] = 1.0;
    }

  Tcl_SetStringObj(ton, "Scale_X", -1);
  to = Tcl_ObjGetVar2(interp, toa, ton, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetDoubleFromObj(interp, to, &dtemp);
  if(dtemp != 0.0)
    clone->scalx = dtemp;
  Tcl_SetStringObj(ton, "Scale_Y", -1);
  to = Tcl_ObjGetVar2(interp, toa, ton, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetDoubleFromObj(interp, to, &dtemp);
  if(dtemp != 0.0)
    clone->scaly = dtemp;
  Tcl_SetStringObj(ton, "Scale_Z", -1);
  to = Tcl_ObjGetVar2(interp, toa, ton, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetDoubleFromObj(interp, to, &dtemp);
  if(dtemp != 0.0)
    clone->scalz = dtemp;


  Tcl_IncrRefCount(toa);Tcl_DecrRefCount(toa);
  Tcl_IncrRefCount(ton);Tcl_DecrRefCount(ton);

  ay_status = ay_notify_force(o);

  ay_status = ay_notify_parent();

 return AY_OK;
} /* ay_clone_setpropcb */


/* C -> Tcl! */
int
ay_clone_getpropcb(Tcl_Interp *interp, int argc, char *argv[], ay_object *o)
{
 char *n1="CloneAttrData";
 Tcl_Obj *to = NULL, *toa = NULL, *ton = NULL;
 ay_clone_object *clone = NULL;

  if(!o)
    return AY_ENULL;

  clone = (ay_clone_object *)(o->refine);

  toa = Tcl_NewStringObj(n1,-1);

  ton = Tcl_NewStringObj(n1,-1);


  Tcl_SetStringObj(ton,"NumClones",-1);
  to = Tcl_NewIntObj(clone->numclones);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton,"Rotate",-1);
  to = Tcl_NewIntObj(clone->rotate);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton,"Translate_X", -1);
  to = Tcl_NewDoubleObj(clone->movx);
  Tcl_ObjSetVar2(interp, toa, ton, to, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_SetStringObj(ton, "Translate_Y", -1);
  to = Tcl_NewDoubleObj(clone->movy);
  Tcl_ObjSetVar2(interp, toa, ton, to, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_SetStringObj(ton, "Translate_Z", -1);
  to = Tcl_NewDoubleObj(clone->movz);
  Tcl_ObjSetVar2(interp, toa, ton, to, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton, "Quat0", -1);
  to = Tcl_NewDoubleObj(clone->quat[0]);
  Tcl_ObjSetVar2(interp, toa, ton, to, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_SetStringObj(ton, "Quat1", -1);
  to = Tcl_NewDoubleObj(clone->quat[1]);
  Tcl_ObjSetVar2(interp, toa, ton, to, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_SetStringObj(ton, "Quat2", -1);
  to = Tcl_NewDoubleObj(clone->quat[2]);
  Tcl_ObjSetVar2(interp, toa, ton, to, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_SetStringObj(ton, "Quat3", -1);
  to = Tcl_NewDoubleObj(clone->quat[3]);
  Tcl_ObjSetVar2(interp, toa, ton, to, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton, "Rotate_X", -1);
  to = Tcl_NewDoubleObj(clone->rotx);
  Tcl_ObjSetVar2(interp, toa, ton, to, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_SetStringObj(ton, "Rotate_Y", -1);
  to = Tcl_NewDoubleObj(clone->roty);
  Tcl_ObjSetVar2(interp, toa, ton, to, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_SetStringObj(ton, "Rotate_Z", -1);
  to = Tcl_NewDoubleObj(clone->rotz);
  Tcl_ObjSetVar2(interp, toa, ton, to, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton, "Scale_X", -1);
  to = Tcl_NewDoubleObj(clone->scalx);
  Tcl_ObjSetVar2(interp, toa, ton, to, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_SetStringObj(ton, "Scale_Y", -1);
  to = Tcl_NewDoubleObj(clone->scaly);
  Tcl_ObjSetVar2(interp, toa, ton, to, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_SetStringObj(ton, "Scale_Z", -1);
  to = Tcl_NewDoubleObj(clone->scalz);
  Tcl_ObjSetVar2(interp, toa, ton, to, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);


  Tcl_IncrRefCount(toa);Tcl_DecrRefCount(toa);
  Tcl_IncrRefCount(ton);Tcl_DecrRefCount(ton);

 return AY_OK;
} /* ay_clone_getpropcb */


int
ay_clone_readcb(FILE *fileptr, ay_object *o)
{
 ay_clone_object *clone = NULL;

 if(!o)
   return AY_ENULL;

  if(!(clone = calloc(1, sizeof(ay_clone_object))))
    { return AY_EOMEM; }

  fscanf(fileptr,"%d\n",&clone->numclones);

  fscanf(fileptr,"%lg\n",&clone->movx);
  fscanf(fileptr,"%lg\n",&clone->movy);
  fscanf(fileptr,"%lg\n",&clone->movz);

  fscanf(fileptr,"%lg\n",&clone->rotx);
  fscanf(fileptr,"%lg\n",&clone->roty);
  fscanf(fileptr,"%lg\n",&clone->rotz);

  fscanf(fileptr,"%lg\n",&clone->quat[0]);
  fscanf(fileptr,"%lg\n",&clone->quat[1]);
  fscanf(fileptr,"%lg\n",&clone->quat[2]);
  fscanf(fileptr,"%lg\n",&clone->quat[3]);

  fscanf(fileptr,"%lg\n",&clone->scalx);
  fscanf(fileptr,"%lg\n",&clone->scaly);
  fscanf(fileptr,"%lg\n",&clone->scalz);

  if(ay_read_version > 3)
    {
      fscanf(fileptr,"%d\n",&clone->rotate);
    }

  o->refine = clone;

 return AY_OK;
} /* ay_clone_readcb */


int
ay_clone_writecb(FILE *fileptr, ay_object *o)
{
 ay_clone_object *clone = NULL;

  if(!o)
    return AY_ENULL;

  clone = (ay_clone_object *)(o->refine);

  fprintf(fileptr,"%d\n",clone->numclones);

  fprintf(fileptr,"%g\n",clone->movx);
  fprintf(fileptr,"%g\n",clone->movy);
  fprintf(fileptr,"%g\n",clone->movz);

  fprintf(fileptr,"%g\n",clone->rotx);
  fprintf(fileptr,"%g\n",clone->roty);
  fprintf(fileptr,"%g\n",clone->rotz);

  fprintf(fileptr,"%g\n",clone->quat[0]);
  fprintf(fileptr,"%g\n",clone->quat[1]);
  fprintf(fileptr,"%g\n",clone->quat[2]);
  fprintf(fileptr,"%g\n",clone->quat[3]);

  fprintf(fileptr,"%g\n",clone->scalx);
  fprintf(fileptr,"%g\n",clone->scaly);
  fprintf(fileptr,"%g\n",clone->scalz);

  fprintf(fileptr, "%d\n", clone->rotate);

 return AY_OK;
} /* ay_clone_writecb */


int
ay_clone_wribcb(char *file, ay_object *o)
{
 ay_clone_object *cc = NULL;
 ay_object *c = NULL;
 int old_resinstances;

  if(!o)
    return AY_ENULL;

  cc = (ay_clone_object *)o->refine;
  old_resinstances = ay_prefs.resolveinstances;
  ay_prefs.resolveinstances = AY_TRUE;
  
  c = cc->clones;
  while(c)
    {
      ay_wrib_object(file, c);
      c = c->next;
    }

  ay_prefs.resolveinstances = old_resinstances;

 return AY_OK;
} /* ay_clone_wribcb */


int
ay_clone_bbccb(ay_object *o, double *bbox, int *flags)
{
 int ay_status = AY_OK;
 int i, a;
 double xmin = DBL_MAX, xmax = -DBL_MAX, ymin = DBL_MAX;
 double ymax = -DBL_MAX, zmin = DBL_MAX, zmax = -DBL_MAX;
 double bbt[24] = {0};
 ay_clone_object *cc = NULL;
 ay_object *c = NULL;

  if(!o || !flags)
    return AY_ENULL;

  cc = (ay_clone_object *)o->refine;
  
  c = cc->clones;
  if(!c)
    {  /* use the bounding boxes of the child(s) */
      *flags = 2;
      return AY_OK;
    }

  while(c)
    {
      ay_status = ay_bbc_get(c, bbt);

      a = 0;
      for(i = 0; i < 8; i++)
	{
	  if(bbt[a] < xmin)
	    xmin = bbt[a];
	  if(bbt[a] > xmax)
	    xmax = bbt[a];
	  a += 3;
	} /* for */

      a = 1;
      for(i = 0; i < 8; i++)
	{
	  if(bbt[a] < ymin)
	    ymin = bbt[a];
	  if(bbt[a] > ymax)
	    ymax = bbt[a];
	  a += 3;
	} /* for */

      a = 2;
      for(i = 0; i < 8; i++)
	{
	  if(bbt[a] < zmin)
	    zmin = bbt[a];
	  if(bbt[a] > zmax)
	    zmax = bbt[a];
	  a += 3;
	} /* for */

      c = c->next;
    } /* while */

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

 return AY_OK;
} /* ay_clone_bbccb */


int
ay_clone_notifycb(ay_object *o)
{
 int ay_status = AY_OK;
 char fname[] = "clone_notifycb";
 ay_clone_object *clone = NULL;
 ay_object *down = NULL, *newo = NULL, **next = NULL, trafo = {0};
 ay_object *tr;
 int i = 0, tr_iscopy = AY_FALSE;
 double euler[3];

  if(!o)
    return AY_ENULL;    

  clone = (ay_clone_object *)(o->refine);
  while(clone->clones)
    {
      tr = clone->clones;
      clone->clones = tr->next;
      free(tr);
    }
  tr = NULL;

  /* get (first) child object */
  down = o->down;
  if(down->type != AY_IDINSTANCE)
    {
      /* arrange clones along curve? */
      if(down->next)
	{ /* Yes! */

	  /* get trajectory curve */
	  if(down->next->type != AY_IDNCURVE)
	    {
	      ay_status = ay_provide_object(down->next, AY_IDNCURVE, &tr);
	      if(!tr)
		{
		  /* XXXX issue error message? */
		  return AY_OK;
		}
	      else
		{
		  tr_iscopy = AY_TRUE;
		}
	    }
	  else
	    {
	      tr = down->next;
	    }

	  /* create clones */
	  next = &(clone->clones);
	  for(i = 0; i < clone->numclones; i++)
	    {
	      /* create a new instance object */
	      newo = NULL;
	      if(!(newo = calloc(1, sizeof(ay_object))))
		return AY_EOMEM; /* XXXX memory leak! */

	      ay_object_defaults(newo);
	      newo->type = AY_IDINSTANCE;
	      newo->refine = down;

	      /* link new instance object */
	      *next = newo;
	      next = &(newo->next);
	    } /* for */

	  ay_status = ay_nct_arrange(clone->clones, tr, clone->rotate);

	  if(tr_iscopy)
	    {
	      ay_object_delete(tr);
	    }
	}
      else
	{ /* No! */
	  ay_trafo_defaults(&trafo);
	  next = &(clone->clones);
	  for(i = 0; i < clone->numclones; i++)
	    {
	      /* prepare transformation attributes */
	      trafo.movx += clone->movx;
	      trafo.movy += clone->movy;
	      trafo.movz += clone->movz;

	      trafo.scalx *= clone->scalx;
	      trafo.scaly *= clone->scaly;
	      trafo.scalz *= clone->scalz;

	      ay_quat_add(trafo.quat, clone->quat, trafo.quat);
	      ay_quat_toeuler(trafo.quat, euler);

	      trafo.rotx = AY_R2D(euler[0]);
	      trafo.roty = AY_R2D(euler[1]);
	      trafo.rotz = AY_R2D(euler[2]);

	      /* create a new instance object */
	      newo = NULL;
	      if(!(newo = calloc(1, sizeof(ay_object))))
		return AY_EOMEM; /* XXXX memory leak! */

	      ay_object_defaults(newo);
	      newo->type = AY_IDINSTANCE;
	      newo->refine = down;
	      ay_trafo_copy(down, newo);
	      ay_trafo_add(&trafo, newo);

	      /* link new instance */
	      *next = newo;
	      next = &(newo->next);
	    } /* for */
	} /* if */

    }
  else
    {
      ay_error(AY_ERROR, fname, "cannot clone from instance");
    } /* if */

 return AY_OK;
} /* ay_clone_notifycb */


int
ay_clone_convertcb(ay_object *o)
{
 int ay_status = AY_OK;
 ay_clone_object *cc = NULL;
 ay_object *clone = NULL, *newo = NULL, *down = NULL;

  if(!o)
    return AY_ENULL;

  down = o->down;
  if(!down || !down->next)
    return AY_OK;

  cc = (ay_clone_object *) o->refine;

  clone = cc->clones;
  while(clone)
    {
      newo = NULL;
      ay_status = ay_object_copy(down, &newo);
      ay_trafo_copy(clone, newo);
      ay_status = ay_object_link(newo);
      clone = clone->next;
    }

 return ay_status;
} /* ay_clone_convertcb */


int
ay_clone_providecb(ay_object *o, unsigned int type, ay_object **result)
{
 int ay_status = AY_OK;
 ay_clone_object *cc = NULL;
 ay_object *clone = NULL, *down = NULL, *newo = NULL, **next = NULL;

  if(!o || !result)
    return AY_ENULL;

  cc = (ay_clone_object *) o->refine;

  down = o->down;
  if(!down || !down->next)
    return AY_OK;
  
  next = result;

  clone = cc->clones;
  while(clone)
    {
      newo = NULL;

      if(down->type == type)
	{
	  ay_status = ay_object_copy(down, &newo);
	} else {
	  ay_status = ay_provide_object(down, type, &newo);
	} /* if */

      if(newo)
	{
	  ay_trafo_copy(clone, newo);
	  *next = newo;
	  next = &(newo->next);
	} /* if */

      clone = clone->next;
    } /* while */

 return ay_status;
} /* ay_clone_providecb */


int
ay_clone_init(Tcl_Interp *interp)
{
 int ay_status = AY_OK;

  ay_status = ay_otype_registercore(ay_clone_name,
				    ay_clone_createcb,
				    ay_clone_deletecb,
				    ay_clone_copycb,
				    ay_clone_drawcb,
				    NULL, /* no handles */
				    ay_clone_shadecb,
				    ay_clone_setpropcb,
				    ay_clone_getpropcb,
				    ay_clone_getpntcb,
				    ay_clone_readcb,
				    ay_clone_writecb,
				    ay_clone_wribcb,
				    ay_clone_bbccb,
				    AY_IDCLONE);

  ay_status = ay_notify_register(ay_clone_notifycb, AY_IDCLONE);

  ay_status = ay_convert_register(ay_clone_convertcb, AY_IDCLONE);

  ay_status = ay_provide_register(ay_clone_providecb, AY_IDCLONE);

 return ay_status;
} /* ay_clone_init */

