/*
 * Ayam, a free 3D modeler for the RenderMan interface.
 *
 * Ayam is copyrighted 1998-2004 by Randolf Schultz
 * (rschultz@informatik.uni-rostock.de) and others.
 *
 * All rights reserved.
 *
 * See the file License for details.
 *
 */

#include "ayam.h"
#include "tti.h"

/* text.c - text object */

static char *ay_text_name = "Text";

int
ay_text_createcb(int argc, char *argv[], ay_object *o)
{
 ay_text_object *text = NULL;
 char fname[] = "crttext";

  if(!(text = calloc(1, sizeof(ay_text_object))))
    {
      ay_error(AY_EOMEM, fname, NULL);
      return AY_ERROR;
    }

  text->height = 1.0;

  o->refine = text;

 return AY_OK;
} /* ay_text_createcb */


int
ay_text_deletecb(void *c)
{
 ay_text_object *text = NULL;

  if(!c)
    return AY_ENULL; 

  text = (ay_text_object *)(c);

  if(text->npatch)
    ay_object_deletemulti(text->npatch);
   
  free(text);

 return AY_OK;
} /* ay_text_deletecb */


int
ay_text_copycb(void *src, void **dst)
{
 ay_text_object *tdst = NULL, *tsrc = NULL;

  tsrc = (ay_text_object *)src;
  tdst = (ay_text_object *)dst;

  if(!(tdst = calloc(1, sizeof(ay_text_object))))
    return AY_EOMEM; 

  memcpy(tdst, tsrc, sizeof(ay_text_object)); 

  tdst->npatch = NULL;
  tdst->fontname = NULL;
  tdst->string = NULL;

  if(tsrc->fontname)
    {
      if(!(tdst->fontname = calloc(strlen(tsrc->fontname)+1, sizeof(char))))
	{
	  free(tdst);
	  return AY_EOMEM;
	}
      strcpy(tdst->fontname, tsrc->fontname);
    }

  if(tsrc->string)
    {
      if(!(tdst->string = calloc(strlen(tsrc->string)+1, sizeof(char))))
	{
	  if(tdst->fontname)
	    free(tdst->fontname);
	  free(tdst);
	  return AY_EOMEM;
	}
      strcpy(tdst->string, tsrc->string);
    }
  
  *dst = tdst;

 return AY_OK;
} /* ay_text_copycb */


int
ay_text_drawcb(struct Togl *togl, ay_object *o)
{
 ay_text_object *text = NULL;
 ay_object *npatch;

  if(!o)
    return AY_ENULL; 

  text = (ay_text_object *)(o->refine);
  
  npatch = text->npatch;

  while(npatch)
    {
      ay_draw_object(togl, npatch, AY_TRUE);
      npatch = npatch->next; 
    }

 return AY_OK;
} /* ay_text_drawcb */


int
ay_text_shadecb(struct Togl *togl, ay_object *o)
{
 ay_text_object *text = NULL;
 ay_object *npatch;

  if(!o)
    return AY_ENULL; 

  text = (ay_text_object *)(o->refine);
  
  npatch = text->npatch;

  while(npatch)
    {
      ay_shade_object(togl, npatch, AY_TRUE);
      npatch = npatch->next; 
    }

 return AY_OK;
} /* ay_text_shadecb */


/* Tcl -> C! */
int
ay_text_setpropcb(Tcl_Interp *interp, int argc, char *argv[], ay_object *o)
{
 int ay_status = AY_OK;
 char fname[] = "setProp";
 char *n1 = "TextAttrData";
 char *result;
 Tcl_Obj *to = NULL, *toa = NULL, *ton = NULL;
 ay_text_object *text = NULL;

  if(!o)
    return AY_ENULL;

  text = (ay_text_object *)o->refine;
  
  result = Tcl_GetVar2(interp, n1, "FontName",
		       TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  if(text->fontname)
    {
      free(text->fontname);
      text->fontname = NULL;
    }

  if(!(text->fontname = calloc(strlen(result)+1, sizeof(char))))
    {
      ay_error(AY_EOMEM, fname, NULL);
      return TCL_OK;
    }
  strcpy(text->fontname, result);

  result = Tcl_GetVar2(interp, n1, "String",
		       TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  if(text->string)
    {
      free(text->string);
      text->string = NULL;
    }

  if(!(text->string = calloc(strlen(result)+1, sizeof(char))))
    {
      ay_error(AY_EOMEM, fname, NULL);
      return TCL_OK;
    }
  strcpy(text->string, result);

  toa = Tcl_NewStringObj(n1,-1);
  ton = Tcl_NewStringObj(n1,-1);

  Tcl_SetStringObj(ton,"Height",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetDoubleFromObj(interp,to, &(text->height));

  Tcl_SetStringObj(ton,"UpperCap",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp,to, &(text->has_upper_cap));

  Tcl_SetStringObj(ton,"LowerCap",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp,to, &(text->has_lower_cap));


  Tcl_SetStringObj(ton,"DisplayMode",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp,to, &(text->glu_display_mode));

  Tcl_SetStringObj(ton,"Tolerance",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetDoubleFromObj(interp,to, &(text->glu_sampling_tolerance));

  Tcl_IncrRefCount(toa);Tcl_DecrRefCount(toa);
  Tcl_IncrRefCount(ton);Tcl_DecrRefCount(ton);

  ay_status = ay_notify_force(o);

  ay_status = ay_notify_parent();

 return AY_OK;
} /* ay_text_setpropcb */


/* C -> Tcl! */
int
ay_text_getpropcb(Tcl_Interp *interp, int argc, char *argv[], ay_object *o)
{
 int ay_status = AY_OK;
 char *n1="TextAttrData";
 Tcl_Obj *to = NULL, *toa = NULL, *ton = NULL;
 ay_text_object *text = NULL;

  if(!o)
    return AY_ENULL;

  text = (ay_text_object *)o->refine;

  toa = Tcl_NewStringObj(n1,-1);

  ton = Tcl_NewStringObj("FontName",-1);
  to = Tcl_NewStringObj(text->fontname, -1);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton,"String",-1);
  to = Tcl_NewStringObj(text->string, -1);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton,"Height",-1);
  to = Tcl_NewDoubleObj(text->height);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton,"UpperCap",-1);
  to = Tcl_NewIntObj(text->has_upper_cap);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton,"LowerCap",-1);
  to = Tcl_NewIntObj(text->has_lower_cap);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton,"DisplayMode",-1);
  to = Tcl_NewIntObj(text->glu_display_mode);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton,"Tolerance",-1);
  to = Tcl_NewDoubleObj(text->glu_sampling_tolerance);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);

  Tcl_IncrRefCount(toa);Tcl_DecrRefCount(toa);
  Tcl_IncrRefCount(ton);Tcl_DecrRefCount(ton);

 return AY_OK;
} /* ay_text_getpropcb */


int
ay_text_readcb(FILE *fileptr, ay_object *o)
{
 ay_text_object *bp = NULL;

  if(!o)
   return AY_ENULL;

  if(!(bp = calloc(1, sizeof(ay_text_object))))
    { return AY_EOMEM; }

  ay_read_string(fileptr, &(bp->fontname));

  ay_read_string(fileptr, &(bp->string));

  fscanf(fileptr,"%lg\n",&text->height);
  fscanf(fileptr,"%d\n",&text->has_upper_cap);
  fscanf(fileptr,"%d\n",&text->has_lower_cap);
  fscanf(fileptr,"%d\n",&text->glu_display_mode);
  fscanf(fileptr,"%lg\n",&text->glu_sampling_tolerance);

  o->refine = bp;

 return AY_OK;
} /* ay_text_readcb */


int
ay_text_writecb(FILE *fileptr, ay_object *o)
{
 ay_text_object *text = NULL;

  if(!o)
    return AY_ENULL;

  text = (ay_text_object *)(o->refine);

  fprintf(fileptr, "%s\n", text->fontname);
  fprintf(fileptr, "%s\n", text->string);
  fprintf(fileptr, "%d\n", text->height);
  fprintf(fileptr, "%d\n", text->has_upper_cap);
  fprintf(fileptr, "%d\n", text->has_lower_cap);
  fprintf(fileptr, "%d\n", text->glu_display_mode);
  fprintf(fileptr, "%g\n", text->glu_sampling_tolerance);

 return AY_OK;
} /* ay_text_writecb */


int
ay_text_wribcb(char *file, ay_object *o)
{
 ay_text_object *text = NULL;
 ay_object *p = NULL;

  if(!o)
   return AY_ENULL;

  text = (ay_text_object*)o->refine;

  p = text->npatch;
  while(p)
    {
      ay_wrib_object(file, p);
      p = p->next;
    }

 return AY_OK;
} /* ay_text_wribcb */


int
ay_text_bbccb(ay_object *o, double *bbox, int *flags)
{
 ay_text_object *text = NULL;
 ay_object *p = NULL;

  if(!o || !bbox)
    return AY_ENULL;

  text = (ay_text_object *)o->refine; 
  if(text->npatch)
    {
      p = text->npatch;

      /*
      while(p->next)
	{
	  p = p->next;
	}
      */

      ay_bbc_get(p, bbox);
      *flags = 1;
    }
  else
    {
      /* invalid/nonexisting bbox */
      *flags = 2;
    }

 return AY_OK;
} /* ay_text_bbccb */


int
ay_text_notifycb(ay_object *o)
{
 int ay_status = AY_OK;
 int tti_status = 0;
 ay_text_object *text = NULL;
 ay_tti_letter letter = {0};
 ay_tti_outline *outline;
 ay_object *curves = NULL, *newcurve = NULL, **nextcurve = NULL;
 ay_object *patch, **nextnpatch;
 ay_object ext = {0}, endlevel = {0};
 ay_extrude_object extrude = {0};
 char *c;
 int i;

  if(!o)
    return AY_ENULL;

  text = (ay_text_object *)o->refine;

  if(text->npatch)
    ay_status = ay_object_deletemulti(text->npatch);
  text->npatch = NULL;
  nextnpatch = &(text->npatch);

  if(!text->fontname || text->fontname[0] == '\0' ||
     !text->string || text->string[0] == '\0')
    return AY_OK;

  ext.refine = &extrude;
  extrude.height = 1.0;
  extrude.has_lower_cap = text->has_lower_cap;
  extrude.has_upper_cap = text->has_upper_cap;
  extrude.glu_display_mode = text->glu_display_mode;
  extrude.glu_sampling_tolerance = text->glu_sampling_tolerance;

  ext.type = AY_IDEXTRUDE;
  
  c = text->string;
  while(*c != '\0')
    {
      tti_status = ay_tti_getcurves(text->fontname, *c, &letter);

      if(letter.numoutlines > 0)
	{
	  nextcurve = &curves;
	  for(i = 0; i < letter.numoutlines; i++)
	    {
	      outline = &((letter.outlines)[i]);
	      newcurve = NULL;
	      ay_status = ay_tti_outlinetoncurve(outline, &newcurve);

	      if(outline->filled)
		{
		  /* this outline is a true outline */

		  /* if there are already outlines (and possibly holes)
		     in <curves> we need to create patches from them now */
		  if(curves)
		    {
		      *nextcurve = &endlevel;
		      ext.down = curves;
		      extrude.npatch = NULL;
		      extrude.upper_cap = NULL;
		      extrude.lower_cap = NULL;
		      extrude.upper_bevels = NULL;
		      extrude.lower_bevels = NULL;
		      ay_notify_force(&ext);
		      
		      if(extrude.npatch)
			{
			  *nextnpatch = extrude.npatch;
			  patch = extrude.npatch;
			  while(patch)
			    {
			      nextnpatch = &(patch->next);
			      patch = patch->next;
			    } /* while */
			} /* if */

		      if(extrude.upper_cap)
			{
			  *nextnpatch = extrude.upper_cap;
			  nextnpatch = &(extrude.upper_cap->next);
			} /* if */

		      if(extrude.lower_cap)
			{
			  *nextnpatch = extrude.lower_cap;
			  nextnpatch = &(extrude.lower_cap->next);
			} /* if */

		      if(extrude.upper_bevels)
			{
			  *nextnpatch = extrude.upper_bevels;
			  patch = extrude.upper_bevels;
			  while(patch)
			    {
			      nextnpatch = &(patch->next);
			      patch = patch->next;
			    } /* while */
			} /* if */

		      if(extrude.lower_bevels)
			{
			  *nextnpatch = extrude.lower_bevels;
			  patch = extrude.lower_bevels;
			  while(patch)
			    {
			      nextnpatch = &(patch->next);
			      patch = patch->next;
			    } /* while */
			} /* if */

		      *nextcurve = NULL;
		      ay_object_deletemulti(curves);
		      curves = NULL;
		    } /* if */

		  *nextcurve = newcurve;
		}
	      else
		{
		  /* this "outline" is a hole which we just append */
		  *nextcurve = newcurve;
		} /* if */

	      nextcurve = &(newcurve->next);

	      if((i == letter.numoutlines-1) && (curves))
		{
		  /* end of loop reached, but there are still unconverted
		     outlines in <curves> => convert them to patches now */
		  *nextcurve = &endlevel;
		  ext.down = curves;
		  extrude.npatch = NULL;
		  extrude.upper_cap = NULL;
		  extrude.lower_cap = NULL;
		  extrude.upper_bevels = NULL;
		  extrude.lower_bevels = NULL;
		  ay_notify_force(&ext);

		  if(extrude.npatch)
		    {
		      *nextnpatch = extrude.npatch;
		      patch = extrude.npatch;
		      while(patch)
			{
			  nextnpatch = &(patch->next);
			  patch = patch->next;
			} /* while */
		    } /* if */

		  if(extrude.upper_cap)
		    {
		      *nextnpatch = extrude.upper_cap;
		      nextnpatch = &(extrude.upper_cap->next);
		    } /* if */

		  if(extrude.lower_cap)
		    {
		      *nextnpatch = extrude.lower_cap;
		      nextnpatch = &(extrude.lower_cap->next);
		    } /* if */

		  if(extrude.upper_bevels)
		    {
		      *nextnpatch = extrude.upper_bevels;
		      patch = extrude.upper_bevels;
		      while(patch)
			{
			  nextnpatch = &(patch->next);
			  patch = patch->next;
			} /* while */
		    } /* if */

		  if(extrude.lower_bevels)
		    {
		      *nextnpatch = extrude.lower_bevels;
		      patch = extrude.lower_bevels;
		      while(patch)
			{
			  nextnpatch = &(patch->next);
			  patch = patch->next;
			} /* while */
		    } /* if */

		  *nextcurve = NULL;
		  ay_object_deletemulti(curves);
		  curves = NULL;
		} /* if */
	    } /* for */
	} /* if */


      c++;
    } /* while */

 return AY_OK;
} /* ay_text_notifycb */

int
ay_text_convertcb(ay_object *o, int in_place)
{
 int ay_status = AY_OK;
 ay_text_object *t = NULL;
 ay_object *new = NULL;

  if(!o)
    return AY_ENULL;

  /* first, create new objects */

  t = (ay_text_object *) o->refine;

  if(t->npatch)
    {
      ay_status = ay_object_copy(t->npatch, &new);
      ay_trafo_copy(o, new);
      new->hide_children = AY_TRUE;
      new->parent = AY_TRUE;
      ay_object_crtendlevel(&(new->down));
      ay_object_link(new);
    }

 return AY_OK;
} /* ay_text_convertcb */


int
ay_text_providecb(ay_object *o, unsigned int type, ay_object **result)
{
 return AY_OK;
} /* ay_text_providecb */


int
ay_text_init(Tcl_Interp *interp)
{
 int ay_status = AY_OK;

 ay_status = ay_otype_registercore(ay_text_name,
				   ay_text_createcb,
				   ay_text_deletecb,
				   ay_text_copycb,
				   ay_text_drawcb,
				   NULL,
				   ay_text_shadecb,
				   ay_text_setpropcb,
				   ay_text_getpropcb,
				   NULL,
				   ay_text_readcb,
				   ay_text_writecb,
				   ay_text_wribcb,
				   ay_text_bbccb,
				   AY_IDTEXT);

  Tcl_SetVar(interp, "propertyList", "TextAttr", TCL_APPEND_VALUE |
	     TCL_LIST_ELEMENT | TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  ay_status = ay_notify_register(ay_text_notifycb, AY_IDTEXT);

  ay_status = ay_convert_register(ay_text_convertcb, AY_IDTEXT);

  ay_status = ay_provide_register(ay_text_providecb, AY_IDTEXT);

 return ay_status;
} /* ay_text_init */
