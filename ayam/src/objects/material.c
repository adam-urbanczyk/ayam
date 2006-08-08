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

/* material.c - material object */

static char *ay_material_name = "Material";

int
ay_material_createcb(int argc, char *argv[], ay_object *o)
{
 ay_mat_object *material = NULL;
 char fname[] = "crtmaterial";
 int ay_status = AY_OK;
 size_t stringlen = 0;

  if(!o)
    return AY_ENULL;

  if(argc < 3)
    {
      ay_error(AY_EARGS, fname, "name");
      return AY_ERROR;
    }

  stringlen = strlen(argv[2]);
  if(stringlen > 0)
    {
      if(!(material = calloc(1, sizeof(ay_mat_object))))
	{
	  ay_error(AY_EOMEM, fname, NULL);
	  return AY_ERROR;
	}

      ay_status = ay_matt_registermaterial(argv[2], material);
      if(ay_status)
	{
	  ay_error(AY_ERROR, fname, "Material already registered!");
	  ay_error(AY_ERROR, fname, "Please choose another name!");
	  free(material);
	  return AY_ERROR;
	}


      if(!(o->name = calloc(stringlen+1, sizeof(char))))
	{
	  ay_error(AY_EOMEM, fname, NULL);
	  free(material);
	  return AY_ERROR;
	}
      strcpy(o->name, argv[2]);

      material->nameptr = &(o->name);
      material->refcountptr = &(o->refcount);
      material->objptr = o;

      /* default color & opacity values */
      material->colr = 220;
      material->colg = 220;
      material->colb = 220;

      material->opr = 255;
      material->opg = 255;
      material->opb = 255;

      material->camera = 1;
      material->reflection = 1;
      material->shadow = 1;
      material->shading_rate = 1.0;

      o->refine = material;
    }
  else
    {
      ay_error(AY_ERROR, fname, "Invalid material name!");
      return AY_ERROR;
    }

 return AY_OK;
} /* ay_material_createcb */


int
ay_material_deletecb(void *c)
{
 ay_mat_object *material = NULL;
 char *name = NULL;

  if(!c)
    return AY_ENULL;

  material = (ay_mat_object *)(c);

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

  if(material->nameptr)
    {
      name = *material->nameptr;
      if(ay_matt_isregistered(name))
	{
	  /* remove all references to this material */
	  ay_matt_removerefs(ay_root, material);

	  ay_matt_removerefs(ay_clipboard, material);

	  /* additionally, de-register this material */
	  if(name)
	    ay_matt_deregister(name);
	}
    }

  free(material);

 return AY_OK;
} /* ay_material_deletecb */


int
ay_material_copycb(void *src, void **dst)
{
 ay_mat_object *material = NULL, *srcm;
 int ay_status = AY_OK;

  if(!src || !dst)
    return AY_ENULL;

  if(!(material = calloc(1, sizeof(ay_mat_object))))
    return AY_EOMEM;

  memcpy(material, src, sizeof(ay_mat_object));
  /* Warning! Links point to original object! */

  material->nameptr = NULL;
  material->refcountptr = NULL;
  material->objptr = NULL;
  *dst = (void *)material;
  srcm = (ay_mat_object *)src;

  if(srcm->sshader)
    {
      ay_status = ay_shader_copy(srcm->sshader, &(material->sshader));
      if(ay_status)
	{
	  material->sshader = NULL;
	  return ay_status;
	}
    }
  if(srcm->dshader)
    {
      ay_status = ay_shader_copy(srcm->dshader, &(material->dshader));
      if(ay_status)
	{
	  material->dshader = NULL;
	  return ay_status;
	}
    }
  if(srcm->ishader)
    {
      ay_status = ay_shader_copy(srcm->ishader, &(material->ishader));
      if(ay_status)
	{
	  material->ishader = NULL;
	  return ay_status;
	}
    }
  if(srcm->eshader)
    {
      ay_status = ay_shader_copy(srcm->eshader, &(material->eshader));
      if(ay_status)
	{
	  material->eshader = NULL;
	  return ay_status;
	}
    }

 return AY_OK;
} /* ay_material_copycb */


int
ay_material_drawcb(struct Togl *togl, ay_object *o)
{
 ay_mat_object *material = NULL;

  if(!o)
    return AY_ENULL;

  material = (ay_mat_object *)o->refine;

  if(!material)
    return AY_ENULL;

 return AY_OK;
} /* ay_material_drawcb */


int
ay_material_shadecb(struct Togl *togl, ay_object *o)
{
 ay_mat_object *material = NULL;

  if(!o)
    return AY_ENULL;

  material = (ay_mat_object *)o->refine;

  if(!material)
    return AY_ENULL;

 return AY_OK;
} /* ay_material_shadecb */


/* Tcl -> C! */
int
ay_material_setpropcb(Tcl_Interp *interp, int argc, char *argv[], ay_object *o)
{
 int ay_status = AY_OK;
 char *n1 = "RiAttrData", *n2 = "MaterialAttrData";
 Tcl_Obj *to = NULL, *toa = NULL, *ton = NULL;
 ay_mat_object *material = NULL, *oldmat = NULL;
 int newname_status = AY_OK, oldname_status = AY_OK;
 char *newname = NULL, *oldname = NULL, *string = NULL;
 int stringlen = 0;
 char fname[] = "setmaterial";

  if(!o)
    return AY_ENULL;

  material = (ay_mat_object *)o->refine;

  toa = Tcl_NewStringObj(n1,-1);

  ton = Tcl_NewStringObj("Color_R",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp,to, &material->colr);
  Tcl_SetStringObj(ton,"Color_G",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp,to, &material->colg);
  Tcl_SetStringObj(ton,"Color_B",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp,to, &material->colb);

  Tcl_SetStringObj(ton,"Opacity_R",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp,to, &material->opr);
  Tcl_SetStringObj(ton,"Opacity_G",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp,to, &material->opg);
  Tcl_SetStringObj(ton,"Opacity_B",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp,to, &material->opb);

  /* shading */
  Tcl_SetStringObj(ton,"ShadingRate",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetDoubleFromObj(interp,to, &material->shading_rate);

  Tcl_SetStringObj(ton,"Interpolation",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp,to, &material->shading_interpolation);

  Tcl_SetStringObj(ton,"Sides",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp,to, &material->sides);

  /* displacement */
  Tcl_SetStringObj(ton,"TrueDisp",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp,to, &material->true_displacement);

  Tcl_SetStringObj(ton,"BoundCoord",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp,to, &material->dbound);

  Tcl_SetStringObj(ton,"BoundVal",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetDoubleFromObj(interp,to, &material->dbound_val);

  /* shadows */
  Tcl_SetStringObj(ton,"CastShadows",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp, to, &(material->cast_shadows));

  /* visibility */
  Tcl_SetStringObj(ton,"Camera",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp, to, &(material->camera));

  Tcl_SetStringObj(ton,"Reflection",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp, to, &(material->reflection));

  Tcl_SetStringObj(ton,"Shadow",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp, to, &(material->shadow));

  /* get new name */
  Tcl_SetStringObj(toa, n2, -1);
  Tcl_SetStringObj(ton,"Materialname",-1);
  oldname = o->name;

  to = Tcl_ObjGetVar2(interp, toa, ton, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  string = Tcl_GetStringFromObj(to, &stringlen);
  if(!string)
    {
      ay_error(AY_ENULL, fname, NULL);
      return TCL_OK;
    }

  if(stringlen > 0)
    {
      if(!(newname = calloc(stringlen+1, sizeof(char))))
	{
	  ay_error(AY_EOMEM, fname, NULL);
	  return TCL_OK;
	}
      strcpy(newname, string);
    }

  if(newname != NULL)
    {
      /* Is the new name different from the old one? */
      if(strcmp(newname, oldname))
	{
	  /* Yes, register new name. */
	  newname_status = ay_matt_registermaterial(newname, material);
	  if(!newname_status)
	    {
	      /* New name is successfully registered! */

	      /* Now correctly dispose the old name. */
	      if(oldname)
		oldname_status = ay_matt_isregistered(oldname);
	      else
		oldname_status = AY_FALSE;

	      if(oldname_status != AY_FALSE)
		{
		  /* old name is already registered, de-register it;
		     but only if this really is the registered material
		     object (there can be materials with the same name) */
		  ay_status = ay_matt_getmaterial(oldname, &oldmat);
		  if(oldmat == material)
		    {
		      ay_matt_deregister(oldname);
		    }
		}

	      if(o->name)
		{
		  free(o->name);
		}
	      /* finally, set new name */
	      o->name = newname;
	    }
	  else
	    {
	      /* new name is already registered! */
	      ay_error(AY_ERROR, fname,
		       "Material name is already registered!");
	      ay_error(AY_ERROR, fname, "Falling back to old name!");
	      free(newname);
	    } /* if */
	}
      else
	{
	  /* No, newname == oldname, thus discard new name. */
	  free(newname);
	} /* if */
    } /* if */

  Tcl_IncrRefCount(toa);Tcl_DecrRefCount(toa);
  Tcl_IncrRefCount(ton);Tcl_DecrRefCount(ton);

 return AY_OK;
} /* ay_material_setpropcb */


/* C -> Tcl! */
int
ay_material_getpropcb(Tcl_Interp *interp, int argc, char *argv[], ay_object *o)
{
 char *n1 = "RiAttrData", *n2 = "MaterialAttrData";
 int ay_status = AY_OK;
 Tcl_Obj *to = NULL, *toa = NULL, *ton = NULL;
 ay_mat_object *material = NULL, *mat = NULL;
 char *name = NULL;

  if(!o)
    return AY_ENULL;

  material = (ay_mat_object *)(o->refine);

  toa = Tcl_NewStringObj(n1,-1);

  ton = Tcl_NewStringObj("Color_R",-1);
  to = Tcl_NewIntObj(material->colr);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_SetStringObj(ton,"Color_G",-1);
  to = Tcl_NewIntObj(material->colg);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_SetStringObj(ton,"Color_B",-1);
  to = Tcl_NewIntObj(material->colb);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton,"Opacity_R",-1);
  to = Tcl_NewIntObj(material->opr);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_SetStringObj(ton,"Opacity_G",-1);
  to = Tcl_NewIntObj(material->opg);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_SetStringObj(ton,"Opacity_B",-1);
  to = Tcl_NewIntObj(material->opb);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  /* shading */
  Tcl_SetStringObj(ton,"ShadingRate",-1);
  to = Tcl_NewDoubleObj(material->shading_rate);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton,"Interpolation",-1);
  to = Tcl_NewIntObj(material->shading_interpolation);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton,"Sides",-1);
  to = Tcl_NewIntObj(material->sides);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  /* displacement */
  Tcl_SetStringObj(ton,"TrueDisp",-1);
  to = Tcl_NewIntObj(material->true_displacement);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton,"BoundCoord",-1);
  to = Tcl_NewIntObj(material->dbound);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton,"BoundVal",-1);
  to = Tcl_NewDoubleObj(material->dbound_val);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  /* shadows */
  Tcl_SetStringObj(ton,"CastShadows",-1);
  to = Tcl_NewIntObj(material->cast_shadows);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  /* visibility */
  Tcl_SetStringObj(ton,"Camera",-1);
  to = Tcl_NewIntObj(material->camera);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton,"Reflection",-1);
  to = Tcl_NewIntObj(material->reflection);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton,"Shadow",-1);
  to = Tcl_NewIntObj(material->shadow);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  /* repair nameptr if it has not been set, after e.g.
     a clipboard operation */
  if(!material->nameptr)
    {
      material->nameptr = &(o->name);
    }
  /* repair refcountptr if it has not been set, after e.g.
     a clipboard operation */
  if(!material->refcountptr)
    {
      material->refcountptr = &(o->refcount);
      /*      o->refcount = 0;*/
    }

  /* repair objptr if it has not been set, after e.g.
     a clipboard operation */
  if(!material->objptr)
    {
      material->objptr = o;
    }

  Tcl_SetStringObj(toa,n2,-1);
  Tcl_SetStringObj(ton,"RefCount",-1);
  to = Tcl_NewIntObj(o->refcount);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton,"Registered",-1);
  ay_status = ay_matt_getmaterial(o->name, &mat);
  if(mat == material)
    {
      to = Tcl_NewStringObj("Yes.", -1);
    }
  else
    {
      to = Tcl_NewStringObj("No, please rename!", -1);
    }
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  name = *material->nameptr;
  Tcl_SetStringObj(ton,"Materialname",-1);
  to = Tcl_NewStringObj(name, -1);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_IncrRefCount(toa);Tcl_DecrRefCount(toa);
  Tcl_IncrRefCount(ton);Tcl_DecrRefCount(ton);

 return AY_OK;
} /* ay_material_getpropcb */


int
ay_material_readcb(FILE *fileptr, ay_object *o)
{
 ay_mat_object *material = NULL;
 int ay_status = AY_OK;
 char fname[] = "material_read";
 int has_shader = 0;

  if(!o)
    return AY_ENULL;

  if(!(material = calloc(1, sizeof(ay_mat_object))))
    { return AY_EOMEM; }

  fscanf(fileptr,"%d\n",&material->registered);

  fscanf(fileptr,"%d\n",&material->colr);
  fscanf(fileptr,"%d\n",&material->colg);
  fscanf(fileptr,"%d\n",&material->colb);

  fscanf(fileptr,"%d\n",&material->opr);
  fscanf(fileptr,"%d\n",&material->opg);
  fscanf(fileptr,"%d\n",&material->opb);

  fscanf(fileptr,"%lg\n",&material->shading_rate);
  fscanf(fileptr,"%d\n",&material->shading_interpolation);

  fscanf(fileptr,"%d\n",&material->dbound);
  fscanf(fileptr,"%lg\n",&material->dbound_val);

  fscanf(fileptr,"%d\n",&material->true_displacement);

  fscanf(fileptr,"%d\n",&material->cast_shadows);

  fscanf(fileptr,"%d\n",&material->camera);
  fscanf(fileptr,"%d\n",&material->reflection);
  fscanf(fileptr,"%d\n",&material->shadow);
  fscanf(fileptr,"%d\n",&has_shader);
  if(has_shader)
    {
      ay_status = ay_read_shader(fileptr, &material->sshader);
    }
  has_shader = 0;
  fscanf(fileptr,"%d\n",&has_shader);
  if(has_shader)
    {
      ay_status = ay_read_shader(fileptr, &material->dshader);
    }
  has_shader = 0;
  fscanf(fileptr,"%d\n",&has_shader);
  if(has_shader)
    {
      ay_status = ay_read_shader(fileptr, &material->ishader);
    }
  has_shader = 0;
  fscanf(fileptr,"%d\n",&has_shader);
  if(has_shader)
    {
      ay_status = ay_read_shader(fileptr, &material->eshader);
    }

  if(ay_read_version >= 4)
    {
      fscanf(fileptr,"%d\n",&material->sides);
    }

  material->nameptr = &(o->name);
  material->refcountptr = &(o->refcount);
  material->objptr = o;

  if(material->registered)
    {
      ay_status = ay_matt_registermaterial(o->name, material);
    }

  if(ay_status)
    {
      ay_error(AY_ERROR,fname, "Could not register material:");
      ay_error(AY_ERROR,fname, o->name);
    }

  o->refine = material;

 return AY_OK;
} /* ay_material_readcb */


int
ay_material_writecb(FILE *fileptr, ay_object *o)
{
 ay_mat_object *material = NULL;
 int ay_status = AY_OK;

  if(!o)
    return AY_ENULL;

  material = (ay_mat_object *)(o->refine);

  fprintf(fileptr, "%d\n", material->registered);

  fprintf(fileptr, "%d\n", material->colr);
  fprintf(fileptr, "%d\n", material->colg);
  fprintf(fileptr, "%d\n", material->colb);

  fprintf(fileptr, "%d\n", material->opr);
  fprintf(fileptr, "%d\n", material->opg);
  fprintf(fileptr, "%d\n", material->opb);

  fprintf(fileptr, "%g\n", material->shading_rate);
  fprintf(fileptr, "%d\n", material->shading_interpolation);

  fprintf(fileptr, "%d\n", material->dbound);
  fprintf(fileptr, "%g\n", material->dbound_val);

  fprintf(fileptr, "%d\n", material->true_displacement);

  fprintf(fileptr, "%d\n", material->cast_shadows);

  fprintf(fileptr, "%d\n", material->camera);
  fprintf(fileptr, "%d\n", material->reflection);
  fprintf(fileptr, "%d\n", material->shadow);

  if(material->sshader)
    {
      fprintf(fileptr, "1\n");
      ay_status = ay_write_shader(fileptr, material->sshader);
    }
  else
    {
      fprintf(fileptr, "0\n");
    }

  if(material->dshader)
    {
      fprintf(fileptr, "1\n");
      ay_status = ay_write_shader(fileptr, material->dshader);
    }
  else
    {
      fprintf(fileptr, "0\n");
    }

  if(material->ishader)
    {
      fprintf(fileptr, "1\n");
      ay_status = ay_write_shader(fileptr, material->ishader);
    }
  else
    {
      fprintf(fileptr, "0\n");
    }

  if(material->eshader)
    {
      fprintf(fileptr, "1\n");
      ay_status = ay_write_shader(fileptr, material->eshader);
    }
  else
    {
      fprintf(fileptr, "0\n");
    }

  fprintf(fileptr, "%d\n", material->sides);

 return AY_OK;
} /* ay_material_writecb */


/* This callback does nothing! Material objects are
 *  written to RIB via aycore/matt.c/ay_matt_wrib()
 */
int
ay_material_wribcb(char *file, ay_object *o)
{

  if(!o)
   return AY_ENULL;

 return AY_OK;
} /* ay_material_wribcb */


int
ay_material_bbccb(ay_object *o, double *bbox, int *flags)
{

  if(!o || !bbox)
    return AY_ENULL;

  /* we have no own bbox */
  *flags = 2;

 return AY_OK;
} /* ay_material_bbccb */


/* ay_material_dropcb:
 *  objects are dropped onto a material object
 *  make those objects of this material
 */
int
ay_material_dropcb(ay_object *o)
{
 char fname[] = "material_drop";
 int ay_status = AY_OK;
 ay_list_object *sel = ay_selection;
 ay_object *s = NULL;
 ay_mat_object *mat, *oldmat, *regmat = NULL;
 unsigned int *refcountptr;
 int success = AY_FALSE;
 char arg1[] = "save", arg2[] = "MatDrop";
 char *argv[3] = {0};

  if(!sel)
    {
      ay_error(AY_ENOSEL, fname, NULL);
      return AY_EDONOTLINK;
    }

  mat = (ay_mat_object *)o->refine;

  ay_status = ay_matt_getmaterial(o->name, &regmat);
  if(regmat != mat)
    {
      ay_error(AY_ERROR, fname, "Material is not registered!");
      ay_error(AY_ERROR, fname, "Please rename it to register!");
      return AY_EDONOTLINK;
    }

  /* undo save */
  /* no need to fake a selection here, because all view
     objects are saved by each undo operation anyway */
  argv[1] = arg1;
  argv[2] = arg2;
  ay_status = ay_undo_undotcmd(NULL, ay_interp, 3, argv);

  while(sel)
    {
      s = sel->object;

      if(ay_matt_mayhavematerial(s->type))
	{
	  if(s->mat)
	    {
	      oldmat = s->mat;
	      refcountptr = oldmat->refcountptr;
	      (*refcountptr)--;
	    }
	  s->mat = mat;
	  o->refcount++;
	  success = AY_TRUE;
	}

      sel = sel->next;
    }

  if(success)
    {
      ay_error(AY_EOUTPUT, fname, "Object(s) changed material to:");
      ay_error(AY_EOUTPUT, fname, *(mat->nameptr));
    }

 return AY_EDONOTLINK;
} /* ay_material_dropcb */


int
ay_material_init(Tcl_Interp *interp)
{
 int ay_status = AY_OK;

  ay_status = ay_otype_registercore(ay_material_name,
				    ay_material_createcb,
				    ay_material_deletecb,
				    ay_material_copycb,
				    ay_material_drawcb,
				    NULL, /* no points to edit */
				    ay_material_shadecb,
				    ay_material_setpropcb,
				    ay_material_getpropcb,
				    NULL, /* No Picking! */
				    ay_material_readcb,
				    ay_material_writecb,
				    NULL, /* no RIB export */
				    ay_material_bbccb,
				    AY_IDMATERIAL);

  /* register drop callback */
  ay_status = ay_tree_registerdrop(ay_material_dropcb, AY_IDMATERIAL);

  ay_matt_nomaterial(AY_IDMATERIAL);

 return ay_status;
} /* ay_material_init */

