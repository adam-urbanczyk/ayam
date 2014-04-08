/*
 * Ayam, a free 3D modeler for the RenderMan interface.
 *
 * Ayam is copyrighted 1998-2005 by Randolf Schultz
 * (randolf.schultz@gmail.com) and others.
 *
 * All rights reserved.
 *
 * See the file License for details.
 *
 */

#include "ayam.h"

/* root.c - root object */

static char *ay_root_name = "Root";

/* functions: */

/* ay_root_createcb:
 *  create callback function of root object
 */
int
ay_root_createcb(int argc, char *argv[], ay_object *o)
{
 ay_root_object *root = NULL;
 char fname[] = "crtroot";
 ay_riopt *riopt = NULL;

  if(!o)
    return AY_ENULL;

  if(!(root = calloc(1, sizeof(ay_root_object))))
    {
      ay_error(AY_EOMEM, fname, NULL);
      return AY_ERROR;
    }

  if(!(root->riopt = calloc(1, sizeof(ay_riopt))))
    {
      ay_error(AY_EOMEM, fname, NULL);
      free(root);
      return AY_ERROR;
    }

  /* set default ri-options */
  riopt = root->riopt;
  riopt->width = 0;
  riopt->height = 0;
  riopt->Variance = 0.0;
  riopt->Samples_X = 2.0;
  riopt->Samples_Y = 2.0;
  riopt->FilterFunc = 0;
  riopt->FilterWidth = 2.0;
  riopt->FilterHeight = 2.0;
  riopt->ExpGain = 1.0;
  riopt->ExpGamma = 1.0;
  riopt->RGBA_ONE = 255;
  riopt->RGBA_MIN = 0;
  riopt->RGBA_MAX = 255;
  riopt->RGBA_Dither = 0.5;

  riopt->MinSamples = 4;
  riopt->MaxSamples = 64;
  riopt->MaxRayLevel = 4;
  riopt->ShadowBias = 0.01;
  riopt->PRManSpec = 1;

  riopt->RadSteps = 0;
  riopt->PatchSamples = 2;
  riopt->texturemem = 10000;
  riopt->geommem = 40000;

  riopt->use_std_display = AY_TRUE;

  o->parent = AY_TRUE;
  o->inherit_trafos = AY_FALSE;
  o->refine = root;

 return AY_OK;
} /* ay_root_createcb */


/* ay_root_deletecb:
 *  delete callback function of root object
 */
int
ay_root_deletecb(void *c)
{
 ay_root_object *root = NULL;
 ay_riopt *riopt = NULL;

  if(!c)
    return AY_ENULL;

  root = (ay_root_object *)(c);

  riopt = root->riopt;
  if(riopt->textures)
    {
      free(riopt->textures);
      riopt->textures = NULL;
    }
  if(riopt->shaders)
    {
      free(riopt->shaders);
      riopt->shaders = NULL;
    }
  if(riopt->archives)
    {
      free(riopt->archives);
      riopt->archives = NULL;
    }
  if(riopt->procedurals)
    {
      free(riopt->procedurals);
      riopt->procedurals = NULL;
    }
  free(root->riopt);
  free(root);

 return AY_OK;
} /* ay_root_deletecb */


/* ay_root_copycb:
 *  copy callback function of root object
 */
int
ay_root_copycb(void *src, void **dst)
{
 char fname[] = "root_copycb";

  ay_error(AY_ERROR, fname, "can not copy a root object");

 return AY_ERROR;
} /* ay_root_copycb */


/* ay_root_drawcb:
 *  draw (display in an Ayam view window) callback function of root object
 */
int
ay_root_drawcb(struct Togl *togl, ay_object *o)
{

  if(!o)
    return AY_ENULL;

  /* ignore current transformation */
  glLoadIdentity();

  /* draw coordinate system */
  ay_draw_cs(togl, 0);

 return AY_OK;
} /* ay_root_drawcb */


/* ay_root_drawacb:
 *  draw annotations (in an Ayam view window) callback function of root object
 */
int
ay_root_drawacb(struct Togl *togl, ay_object *o)
{

  if(!o)
    return AY_ENULL;

  ay_draw_cs(togl, 1);

 return AY_OK;
} /* ay_root_drawacb */


/* ay_root_shadecb:
 *  shade (display in an Ayam view window) callback function of root object
 */
int
ay_root_shadecb(struct Togl *togl, ay_object *o)
{
  if(!o)
    return AY_ENULL;

  /* XXXX draw no coordinate system in shaded views? */

 return AY_OK;
} /* ay_root_shadecb */


/* ay_root_getpntcb:
 *  get point (editing and selection) callback function of root object
 */
int
ay_root_getpntcb(int mode, ay_object *o, double *p, ay_pointedit *pe)
{
  if(!o || !p || !pe)
    return AY_ENULL;

 return AY_OK;
} /* ay_root_getpntcb */


/* ay_root_setpropcb:
 *  set property (from Tcl to C context) callback function of root object
 */
int
ay_root_setpropcb(Tcl_Interp *interp, int argc, char *argv[], ay_object *o)
{
 char fname[] = "root_setpropcb";
 char *n1 = "RiOptData";
 Tcl_Obj *to = NULL, *toa = NULL, *ton = NULL;
 ay_root_object *root = NULL;
 ay_riopt *riopt = NULL;
 double dtemp = 0.0;
 int itemp = 0;
 char *result;

  if(!interp || !o)
    return AY_ENULL;

  root = (ay_root_object *)o->refine;

  if(!root)
    return AY_ENULL;

  riopt = root->riopt;

  toa = Tcl_NewStringObj(n1, -1);

  ton = Tcl_NewStringObj("Variance", -1);
  to = Tcl_ObjGetVar2(interp, toa, ton, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetDoubleFromObj(interp, to, &dtemp);
  riopt->Variance = dtemp;
  Tcl_SetStringObj(ton, "Width", -1);
  to = Tcl_ObjGetVar2(interp, toa, ton, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp, to, &itemp);
  riopt->width = itemp;
  Tcl_SetStringObj(ton, "Height", -1);
  to = Tcl_ObjGetVar2(interp, toa, ton, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp, to, &itemp);
  riopt->height = itemp;
  Tcl_SetStringObj(ton, "Samples_X", -1);
  to = Tcl_ObjGetVar2(interp, toa, ton, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetDoubleFromObj(interp, to, &dtemp);
  riopt->Samples_X = dtemp;
  Tcl_SetStringObj(ton, "Samples_Y", -1);
  to = Tcl_ObjGetVar2(interp, toa, ton, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetDoubleFromObj(interp, to, &dtemp);
  riopt->Samples_Y = dtemp;
  Tcl_SetStringObj(ton, "FilterFunc", -1);
  to = Tcl_ObjGetVar2(interp, toa, ton, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp, to, &itemp);
  riopt->FilterFunc = (char)itemp;

  Tcl_SetStringObj(ton, "FilterWidth", -1);
  to = Tcl_ObjGetVar2(interp, toa, ton, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetDoubleFromObj(interp, to, &dtemp);
  riopt->FilterWidth = dtemp;
  Tcl_SetStringObj(ton, "FilterHeight", -1);
  to = Tcl_ObjGetVar2(interp, toa, ton, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetDoubleFromObj(interp, to, &dtemp);
  riopt->FilterHeight = dtemp;
  Tcl_SetStringObj(ton, "ExpGain", -1);
  to = Tcl_ObjGetVar2(interp, toa, ton, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetDoubleFromObj(interp, to, &dtemp);
  riopt->ExpGain = dtemp;
  Tcl_SetStringObj(ton, "ExpGamma", -1);
  to = Tcl_ObjGetVar2(interp, toa, ton, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetDoubleFromObj(interp, to, &dtemp);
  riopt->ExpGamma = dtemp;
  Tcl_SetStringObj(ton, "RGBA_ONE", -1);
  to = Tcl_ObjGetVar2(interp, toa, ton, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp, to, &itemp);
  riopt->RGBA_ONE = itemp;
  Tcl_SetStringObj(ton, "RGBA_MIN", -1);
  to = Tcl_ObjGetVar2(interp, toa, ton, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp, to, &itemp);
  riopt->RGBA_MIN = itemp;
  Tcl_SetStringObj(ton, "RGBA_MAX", -1);
  to = Tcl_ObjGetVar2(interp, toa, ton, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp, to, &itemp);
  riopt->RGBA_MAX = itemp;
  Tcl_SetStringObj(ton, "RGBA_Dither", -1);
  to = Tcl_ObjGetVar2(interp, toa, ton, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetDoubleFromObj(interp, to, &dtemp);
  riopt->RGBA_Dither = dtemp;

  Tcl_SetStringObj(ton, "MinSamples", -1);
  to = Tcl_ObjGetVar2(interp, toa, ton, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp, to, &itemp);
  riopt->MinSamples = itemp;
  Tcl_SetStringObj(ton, "MaxSamples", -1);
  to = Tcl_ObjGetVar2(interp, toa, ton, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp, to, &itemp);
  riopt->MaxSamples = itemp;
  Tcl_SetStringObj(ton, "MaxRayLevel", -1);
  to = Tcl_ObjGetVar2(interp, toa, ton, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp, to, &itemp);
  riopt->MaxRayLevel = itemp;
  Tcl_SetStringObj(ton, "ShadowBias", -1);
  to = Tcl_ObjGetVar2(interp, toa, ton, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetDoubleFromObj(interp, to, &dtemp);
  riopt->ShadowBias = dtemp;
  Tcl_SetStringObj(ton, "PRManSpec", -1);
  to = Tcl_ObjGetVar2(interp, toa, ton, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp, to, &itemp);
  riopt->PRManSpec = itemp;

  Tcl_SetStringObj(ton, "RadSteps", -1);
  to = Tcl_ObjGetVar2(interp, toa, ton, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp, to, &itemp);
  riopt->RadSteps = itemp;
  Tcl_SetStringObj(ton, "PatchSamples", -1);
  to = Tcl_ObjGetVar2(interp, toa, ton, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp, to, &itemp);
  riopt->PatchSamples = itemp;

  result = Tcl_GetVar2(interp, n1, "Textures",
		       TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  if(riopt->textures)
    {
      free(riopt->textures);
      riopt->textures = NULL;
    }

  if(!(riopt->textures = calloc(strlen(result)+1, sizeof(char))))
    {
      ay_error(AY_EOMEM, fname, NULL);
      return AY_ERROR;
    }
  strcpy(riopt->textures, result);

  result = Tcl_GetVar2(interp, n1, "Shaders",
		       TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  if(riopt->shaders)
    {
      free(riopt->shaders);
      riopt->shaders = NULL;
    }

  if(!(riopt->shaders = calloc(strlen(result)+1, sizeof(char))))
    {
      ay_error(AY_EOMEM, fname, NULL);
      return AY_ERROR;
    }
  strcpy(riopt->shaders, result);

  result = Tcl_GetVar2(interp, n1, "Archives",
		       TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  if(riopt->archives)
    {
      free(riopt->archives);
      riopt->archives = NULL;
    }

  if(!(riopt->archives = calloc(strlen(result)+1, sizeof(char))))
    {
      ay_error(AY_EOMEM, fname, NULL);
      return AY_ERROR;
    }
  strcpy(riopt->archives, result);

  result = Tcl_GetVar2(interp, n1, "Procedurals",
		       TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  if(riopt->procedurals)
    {
      free(riopt->procedurals);
      riopt->procedurals = NULL;
    }

  if(!(riopt->procedurals = calloc(strlen(result)+1, sizeof(char))))
    {
      ay_error(AY_EOMEM, fname, NULL);
      return AY_ERROR;
    }
  strcpy(riopt->procedurals, result);


  Tcl_SetStringObj(ton, "TextureMem", -1);
  to = Tcl_ObjGetVar2(interp, toa, ton, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp, to, &itemp);
  riopt->texturemem = itemp;

  Tcl_SetStringObj(ton, "GeomMem", -1);
  to = Tcl_ObjGetVar2(interp, toa, ton, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp, to, &itemp);
  riopt->geommem = itemp;

  Tcl_SetStringObj(ton, "StdDisplay", -1);
  to = Tcl_ObjGetVar2(interp, toa, ton, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp, to, &itemp);
  riopt->use_std_display = itemp;

  Tcl_IncrRefCount(toa);Tcl_DecrRefCount(toa);
  Tcl_IncrRefCount(ton);Tcl_DecrRefCount(ton);

 return AY_OK;
} /* ay_root_setpropcb */


/* ay_root_getpropcb:
 *  get property (from C to Tcl context) callback function of root object
 */
int
ay_root_getpropcb(Tcl_Interp *interp, int argc, char *argv[], ay_object *o)
{
 char *n1="RiOptData";
 Tcl_Obj *to = NULL, *toa = NULL, *ton = NULL;
 ay_root_object *root = NULL;
 ay_riopt *riopt = NULL;

  if(!interp || !o)
    return AY_ENULL;

  root = (ay_root_object *)o->refine;

  if(!root)
    return AY_ENULL;

  riopt = root->riopt;

  toa = Tcl_NewStringObj(n1, -1);

  ton = Tcl_NewStringObj("Variance", -1);
  to = Tcl_NewDoubleObj(riopt->Variance);
  Tcl_ObjSetVar2(interp, toa, ton, to, TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);
  Tcl_SetStringObj(ton, "Width", -1);
  to = Tcl_NewIntObj(riopt->width);
  Tcl_ObjSetVar2(interp, toa, ton, to, TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);
  Tcl_SetStringObj(ton, "Height", -1);
  to = Tcl_NewIntObj(riopt->height);
  Tcl_ObjSetVar2(interp, toa, ton, to, TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);
  Tcl_SetStringObj(ton, "Samples_X", -1);
  to = Tcl_NewDoubleObj(riopt->Samples_X);
  Tcl_ObjSetVar2(interp, toa, ton, to, TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);
  Tcl_SetStringObj(ton, "Samples_Y", -1);
  to = Tcl_NewDoubleObj(riopt->Samples_Y);
  Tcl_ObjSetVar2(interp, toa, ton, to, TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);
  Tcl_SetStringObj(ton, "FilterFunc", -1);
  to = Tcl_NewIntObj(riopt->FilterFunc);
  Tcl_ObjSetVar2(interp, toa, ton, to, TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);
  Tcl_SetStringObj(ton, "FilterWidth", -1);
  to = Tcl_NewDoubleObj(riopt->FilterWidth);
  Tcl_ObjSetVar2(interp, toa, ton, to, TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);
  Tcl_SetStringObj(ton, "FilterHeight", -1);
  to = Tcl_NewDoubleObj(riopt->FilterHeight);
  Tcl_ObjSetVar2(interp, toa, ton, to, TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);
  Tcl_SetStringObj(ton, "ExpGain", -1);
  to = Tcl_NewDoubleObj(riopt->ExpGain);
  Tcl_ObjSetVar2(interp, toa, ton, to, TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);
  Tcl_SetStringObj(ton, "ExpGamma", -1);
  to = Tcl_NewDoubleObj(riopt->ExpGamma);
  Tcl_ObjSetVar2(interp, toa, ton, to, TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);
  Tcl_SetStringObj(ton, "RGBA_ONE", -1);
  to = Tcl_NewIntObj((int)riopt->RGBA_ONE);
  Tcl_ObjSetVar2(interp, toa, ton, to, TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);
  Tcl_SetStringObj(ton, "RGBA_MIN", -1);
  to = Tcl_NewIntObj((int)riopt->RGBA_MIN);
  Tcl_ObjSetVar2(interp, toa, ton, to, TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);
  Tcl_SetStringObj(ton, "RGBA_MAX", -1);
  to = Tcl_NewIntObj((int)riopt->RGBA_MAX);
  Tcl_ObjSetVar2(interp, toa, ton, to, TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);
  Tcl_SetStringObj(ton, "RGBA_Dither", -1);
  to = Tcl_NewDoubleObj(riopt->RGBA_Dither);
  Tcl_ObjSetVar2(interp, toa, ton, to, TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton, "MinSamples", -1);
  to = Tcl_NewIntObj(riopt->MinSamples);
  Tcl_ObjSetVar2(interp, toa, ton, to, TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);
  Tcl_SetStringObj(ton, "MaxSamples", -1);
  to = Tcl_NewIntObj(riopt->MaxSamples);
  Tcl_ObjSetVar2(interp, toa, ton, to, TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);
  Tcl_SetStringObj(ton, "MaxRayLevel", -1);
  to = Tcl_NewIntObj(riopt->MaxRayLevel);
  Tcl_ObjSetVar2(interp, toa, ton, to, TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);
  Tcl_SetStringObj(ton, "ShadowBias", -1);
  to = Tcl_NewDoubleObj(riopt->ShadowBias);
  Tcl_ObjSetVar2(interp, toa, ton, to, TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);
  Tcl_SetStringObj(ton, "PRManSpec", -1);
  to = Tcl_NewIntObj(riopt->PRManSpec);
  Tcl_ObjSetVar2(interp, toa, ton, to, TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);
  Tcl_SetStringObj(ton, "RadSteps", -1);
  to = Tcl_NewIntObj(riopt->RadSteps);
  Tcl_ObjSetVar2(interp, toa, ton, to, TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);
  Tcl_SetStringObj(ton, "PatchSamples", -1);
  to = Tcl_NewIntObj(riopt->PatchSamples);
  Tcl_ObjSetVar2(interp, toa, ton, to, TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton, "Textures", -1);
  to = Tcl_NewStringObj(riopt->textures, -1);
  Tcl_ObjSetVar2(interp, toa, ton, to, TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);
  Tcl_SetStringObj(ton, "Shaders", -1);
  to = Tcl_NewStringObj(riopt->shaders, -1);
  Tcl_ObjSetVar2(interp, toa, ton, to, TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);
  Tcl_SetStringObj(ton, "Archives", -1);
  to = Tcl_NewStringObj(riopt->archives, -1);
  Tcl_ObjSetVar2(interp, toa, ton, to, TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);
  Tcl_SetStringObj(ton, "Procedurals", -1);
  to = Tcl_NewStringObj(riopt->procedurals, -1);
  Tcl_ObjSetVar2(interp, toa, ton, to, TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton, "TextureMem", -1);
  to = Tcl_NewIntObj(riopt->texturemem);
  Tcl_ObjSetVar2(interp, toa, ton, to, TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);
  Tcl_SetStringObj(ton, "GeomMem", -1);
  to = Tcl_NewIntObj(riopt->geommem);
  Tcl_ObjSetVar2(interp, toa, ton, to, TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton, "StdDisplay", -1);
  to = Tcl_NewIntObj(riopt->use_std_display);
  Tcl_ObjSetVar2(interp, toa, ton, to, TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);

  Tcl_IncrRefCount(toa);Tcl_DecrRefCount(toa);
  Tcl_IncrRefCount(ton);Tcl_DecrRefCount(ton);

 return AY_OK;
} /* ay_root_getpropcb */


/* ay_root_readcb:
 *  read (from scene file) callback function of root object
 */
int
ay_root_readcb(FILE *fileptr, ay_object *o)
{
 int ay_status = AY_OK;
 ay_root_object *root = NULL;
 int itemp = 0, has_atmosphere = 0, has_imager = 0;
 int fwtemp = 0, fhtemp = 0;
 ay_riopt *riopt = NULL;

  if(!fileptr | !o)
    return AY_ENULL;

  if(!ay_root)
    return AY_ENULL;

  root = (ay_root_object *)(ay_root->refine);

  if(!root)
    return AY_ENULL;

  riopt = root->riopt;
  if(riopt->textures)
    {
      free(riopt->textures);
      riopt->textures = NULL;
    }
  if(riopt->shaders)
    {
      free(riopt->shaders);
      riopt->shaders = NULL;
    }
  if(riopt->archives)
    {
      free(riopt->archives);
      riopt->archives = NULL;
    }
  if(riopt->procedurals)
    {
      free(riopt->procedurals);
      riopt->procedurals = NULL;
    }
  riopt = NULL;

  /* read RiOptions */
  if(!(riopt = calloc(1, sizeof(ay_riopt))))
    { return AY_EOMEM; }

  fscanf(fileptr,"%lg\n",&riopt->Variance);
  fscanf(fileptr,"%lg\n",&riopt->Samples_X);
  fscanf(fileptr,"%lg\n",&riopt->Samples_Y);
  fscanf(fileptr,"%d\n",&itemp);
  riopt->FilterFunc = (char)itemp;
  fscanf(fileptr,"%d\n",&fwtemp/*riopt->FilterWidth*/);
  fscanf(fileptr,"%d\n",&fhtemp/*riopt->FilterHeight*/);
  fscanf(fileptr,"%lg\n",&riopt->ExpGain);
  fscanf(fileptr,"%lg\n",&riopt->ExpGamma);
  fscanf(fileptr,"%lg\n",&riopt->RGBA_ONE);
  fscanf(fileptr,"%lg\n",&riopt->RGBA_MIN);
  fscanf(fileptr,"%lg\n",&riopt->RGBA_MAX);
  fscanf(fileptr,"%lg\n",&riopt->RGBA_Dither);
  fscanf(fileptr,"%d\n",&riopt->MinSamples);
  fscanf(fileptr,"%d\n",&riopt->MaxSamples);
  fscanf(fileptr,"%d\n",&riopt->MaxRayLevel);
  fscanf(fileptr,"%lg\n",&riopt->ShadowBias);
  fscanf(fileptr,"%d\n",&itemp);
  riopt->PRManSpec = itemp;
  fscanf(fileptr,"%d\n",&riopt->RadSteps);
  fscanf(fileptr,"%d",&riopt->PatchSamples);
  (void)fgetc(fileptr);

  ay_status = ay_read_string(fileptr, &(riopt->textures));
  if(ay_status)
    {
      free(riopt);
      return ay_status;
    }
  ay_status = ay_read_string(fileptr, &(riopt->archives));
  if(ay_status)
    {
      if(riopt->textures)
	free(riopt->textures);
      free(riopt);
      return ay_status;
    }
  ay_status = ay_read_string(fileptr, &(riopt->shaders));
  if(ay_status)
    {
      if(riopt->textures)
	free(riopt->textures);
      if(riopt->archives)
	free(riopt->archives);
      free(riopt);
      return ay_status;
    }

  fscanf(fileptr,"%d\n",&riopt->texturemem);
  fscanf(fileptr,"%d\n",&riopt->geommem);

  fscanf(fileptr,"%d\n",&riopt->width);
  fscanf(fileptr,"%d\n",&riopt->height);

  /* read Atmosphere */
  fscanf(fileptr,"%d\n",&has_atmosphere);
  if(has_atmosphere)
    {
      if(root->atmosphere)
	{
	  ay_shader_free(root->atmosphere);
	  root->atmosphere = NULL;
	}
      ay_status = ay_read_shader(fileptr, &(root->atmosphere));
      if(ay_status)
	{
	  ay_shader_free(root->atmosphere);
	  root->atmosphere = NULL;
	}
    }

  /* read Imager */
  fscanf(fileptr,"%d\n",&has_imager);
  if(has_imager)
    {
      if(root->imager)
	{
	  ay_shader_free(root->imager);
	  root->imager = NULL;
	}
      ay_status = ay_read_shader(fileptr, &(root->imager));
      if(ay_status)
	{
	  ay_shader_free(root->imager);
	  root->imager = NULL;
	}
    }

  if(ay_read_version >= 5)
    {
      /* since 1.6 */
      fscanf(fileptr,"%d", &riopt->use_std_display);
      fgetc(fileptr);

      ay_status = ay_read_string(fileptr, &(riopt->procedurals));
      if(ay_status)
	{
	  if(riopt->textures)
	    free(riopt->textures);
	  if(riopt->archives)
	    free(riopt->archives);
	  if(riopt->shaders)
	    free(riopt->shaders);
	  free(riopt);
	  return ay_status;
	}
    }
  else
    {
      riopt->use_std_display = AY_TRUE;
    }

  if(root->riopt)
    free(root->riopt);

  root->riopt = riopt;

  if(ay_read_version >= 10)
    {
      /* since 1.13 */
      fscanf(fileptr,"%lg\n",&riopt->FilterWidth);
      fscanf(fileptr,"%lg\n",&riopt->FilterHeight);
    }
  else
    {
      riopt->FilterWidth = fwtemp;
      riopt->FilterHeight = fhtemp;
    }

  /* link newly read tags to old root object */
  ay_tags_delall(ay_root);
  if(o->tags)
    {
      if(o->tags->type == ay_hc_tagtype)
	{
	  ay_root->tags = o->tags->next;
	  o->tags->next = NULL;
	}
      else
	{
	  ay_root->tags = o->tags;
	  o->tags = NULL;
	}
    }

  /* copy important attributes from o to real ay_root,
     (we leave with EDONOTLINK, which would make them
     disappear else) */
  ay_root->hide = o->hide;
  /* copy name? No, it is not visible to the user anyway, currently... */

  /* arrange for views to be saved */
  ay_prefs.save_rootviews = AY_TRUE;

 return AY_EDONOTLINK;
} /* ay_root_readcb */


/* ay_root_writecb:
 *  write (to scene file) callback function of root object
 */
int
ay_root_writecb(FILE *fileptr, ay_object *o)
{
 ay_root_object *root = NULL;
 ay_riopt *riopt = NULL;

  if(!fileptr | !o)
    return AY_ENULL;

  root = (ay_root_object*)(o->refine);

  if(!root)
    return AY_ENULL;

  /* write RiOptions */
  riopt = root->riopt;

  fprintf(fileptr,"%g\n",riopt->Variance);
  fprintf(fileptr,"%g\n",riopt->Samples_X);
  fprintf(fileptr,"%g\n",riopt->Samples_Y);
  fprintf(fileptr,"%d\n",(int)(riopt->FilterFunc));
  fprintf(fileptr,"%d\n",(int)riopt->FilterWidth);
  fprintf(fileptr,"%d\n",(int)riopt->FilterHeight);
  fprintf(fileptr,"%g\n",riopt->ExpGain);
  fprintf(fileptr,"%g\n",riopt->ExpGamma);
  fprintf(fileptr,"%g\n",riopt->RGBA_ONE);
  fprintf(fileptr,"%g\n",riopt->RGBA_MIN);
  fprintf(fileptr,"%g\n",riopt->RGBA_MAX);
  fprintf(fileptr,"%g\n",riopt->RGBA_Dither);

  fprintf(fileptr,"%d\n",riopt->MinSamples);
  fprintf(fileptr,"%d\n",riopt->MaxSamples);
  fprintf(fileptr,"%d\n",riopt->MaxRayLevel);
  fprintf(fileptr,"%g\n",riopt->ShadowBias);
  fprintf(fileptr,"%d\n",(int)(riopt->PRManSpec));
  fprintf(fileptr,"%d\n",riopt->RadSteps);
  fprintf(fileptr,"%d\n",riopt->PatchSamples);

  if(riopt->textures)
    fprintf(fileptr,"%s\n",riopt->textures);
  else
    fprintf(fileptr,"\n");

  if(riopt->archives)
    fprintf(fileptr,"%s\n",riopt->archives);
  else
    fprintf(fileptr,"\n");

  if(riopt->shaders)
    fprintf(fileptr,"%s\n",riopt->shaders);
  else
    fprintf(fileptr,"\n");

  fprintf(fileptr,"%d\n",riopt->texturemem);
  fprintf(fileptr,"%d\n",riopt->geommem);

  fprintf(fileptr,"%d\n",riopt->width);
  fprintf(fileptr,"%d\n",riopt->height);

  /* write Atmosphere */
  if(root->atmosphere)
    {
      fprintf(fileptr,"1\n");
      ay_write_shader(fileptr, root->atmosphere);
    }
  else
    {
      fprintf(fileptr,"0\n");
    }

  /* write Imager */
  if(root->imager)
    {
      fprintf(fileptr,"1\n");
      ay_write_shader(fileptr, root->imager);
    }
  else
    {
      fprintf(fileptr,"0\n");
    }

  fprintf(fileptr,"%d\n",riopt->use_std_display);

  if(riopt->procedurals)
    fprintf(fileptr,"%s\n",riopt->procedurals);
  else
    fprintf(fileptr,"\n");

  fprintf(fileptr,"%g\n",riopt->FilterWidth);
  fprintf(fileptr,"%g\n",riopt->FilterHeight);

 return AY_OK;
} /* ay_root_writecb */


/* ay_root_wribcb:
 *  RIB export callback function of root object
 */
int
ay_root_wribcb(char *file, ay_object *o)
{
 ay_root_object *root = NULL;
 ay_riopt *riopt = NULL;
 RtFloat fw = 0, fh = 0;
 RtFloat rtftemp = 0.0f;
 RtInt rtitemp = 0;

  if(!o)
   return AY_ENULL;

  root = (ay_root_object*)o->refine;

  if(!root)
    return AY_ENULL;

  riopt = root->riopt;

  /* wrib RiOptions */
  if(riopt->Variance > 0.0)
    RiPixelVariance((RtFloat)riopt->Variance);
  else
    RiPixelSamples((RtFloat)riopt->Samples_X,
		   (RtFloat)riopt->Samples_Y);

  if(riopt->FilterWidth <= 0)
    fw = 2.0f;
  else
    fw = (RtFloat)riopt->FilterWidth;

  if(riopt->FilterHeight <= 0)
    fh = 2.0f;
  else
    fh = (RtFloat)riopt->FilterHeight;

  switch(riopt->FilterFunc)
    {
    case 1:
      RiPixelFilter(RiTriangleFilter, fw, fh);
      break;
    case 2:
      RiPixelFilter(RiCatmullRomFilter, fw, fh);
      break;
    case 3:
      RiPixelFilter(RiBoxFilter, fw, fh);
      break;
    case 4:
      RiPixelFilter(RiSincFilter, fw, fh);
      break;
    default:
      RiPixelFilter(RiGaussianFilter, fw, fh);
    }

  RiExposure((RtFloat)riopt->ExpGain, (RtFloat)riopt->ExpGamma);

  RiQuantize(RI_RGBA, (RtInt)riopt->RGBA_ONE, (RtInt)riopt->RGBA_MIN,
	     (RtInt)riopt->RGBA_MAX, (RtFloat)riopt->RGBA_Dither);

  /* BMRT-Specific */
  if(!ay_prefs.ristandard)
  {
    rtitemp = riopt->MinSamples;
    RiOption((RtToken)"render", (RtToken)"minsamples",
	     (RtPointer)(&rtitemp), RI_NULL);

    rtitemp = riopt->MaxSamples;
    RiOption((RtToken)"render", (RtToken)"maxsamples",
	     (RtPointer)(&rtitemp), RI_NULL);

    rtitemp = riopt->MaxRayLevel;
    RiOption((RtToken)"render", (RtToken)"max_raylevel",
	     (RtPointer)(&rtitemp), RI_NULL);

    rtftemp = (RtFloat)riopt->ShadowBias;
    RiOption((RtToken)"render", (RtToken)"minshadowbias",
	     (RtPointer)(&rtftemp), RI_NULL);

    rtitemp = riopt->PRManSpec;
    RiOption((RtToken)"render", (RtToken)"prmanspecular",
	     (RtPointer)(&rtitemp), RI_NULL);

    rtitemp = riopt->RadSteps;
    RiOption((RtToken)"radiosity", (RtToken)"steps",
	     (RtPointer)(&riopt->RadSteps), RI_NULL);

    rtitemp = riopt->PatchSamples;
    RiOption((RtToken)"radiosity", (RtToken)"minpatchsamples",
	     (RtPointer)(&riopt->PatchSamples), RI_NULL);

    if(riopt->textures)
      if((riopt->textures)[0] != '\0')
	RiOption((RtToken)"searchpath", (RtToken)"texture",
		 (RtPointer)(&riopt->textures), RI_NULL);

    if(riopt->shaders)
      if((riopt->shaders)[0] != '\0')
	RiOption((RtToken)"searchpath", (RtToken)"shader",
		 (RtPointer)(&riopt->shaders), RI_NULL);

    if(riopt->archives)
      if((riopt->archives)[0] != '\0')
	RiOption((RtToken)"searchpath", (RtToken)"archive",
		 (RtPointer)(&riopt->archives), RI_NULL);

    if(riopt->procedurals)
      if((riopt->procedurals)[0] != '\0')
	RiOption((RtToken)"searchpath", (RtToken)"procedural",
		 (RtPointer)(&riopt->procedurals), RI_NULL);

    rtitemp = riopt->texturemem;
    RiOption((RtToken)"limits", (RtToken)"texturememory",
	     &rtitemp, RI_NULL);

    rtitemp = riopt->geommem;
    RiOption((RtToken)"limits", (RtToken)"geommemory",
	     &rtitemp, RI_NULL);
  } /* if */

 return AY_OK;
} /* ay_root_wribcb */


/* ay_root_bbccb:
 *  bounding box calculation callback function of root object
 */
int
ay_root_bbccb(ay_object *o, double *bbox, int *flags)
{
 double min = -2.0, max = 2.0;

  if(!o || !bbox || !flags)
    return AY_ENULL;

  /* exclusive bounding box, discard child(ren) bounding box(es) */
  *flags = 1;

  /* P1 */
  bbox[0] = min; bbox[1] = max; bbox[2] = max;
  /* P2 */
  bbox[3] = min; bbox[4] = max; bbox[5] = min;
  /* P3 */
  bbox[6] = max; bbox[7] = max; bbox[8] = min;
  /* P4 */
  bbox[9] = max; bbox[10] = max; bbox[11] = max;

  /* P5 */
  bbox[12] = min; bbox[13] = min; bbox[14] = max;
  /* P6 */
  bbox[15] = min; bbox[16] = min; bbox[17] = min;
  /* P7 */
  bbox[18] = max; bbox[19] = min; bbox[20] = min;
  /* P8 */
  bbox[21] = max; bbox[22] = min; bbox[23] = max;

 return AY_OK;
} /* ay_root_bbccb */


/* ay_root_init:
 *  initialize the root object module
 */
int
ay_root_init(Tcl_Interp *interp)
{
 int ay_status = AY_OK;

  ay_status = ay_otype_registercore(ay_root_name,
				    ay_root_createcb,
				    ay_root_deletecb,
				    ay_root_copycb,
				    ay_root_drawcb,
				    NULL, /* no handles */
				    NULL, /* no shading! */
				    ay_root_setpropcb,
				    ay_root_getpropcb,
				    NULL, /* no picking! */
				    ay_root_readcb,
				    ay_root_writecb,
				    ay_root_wribcb,
				    ay_root_bbccb,
				    AY_IDROOT);

  ay_status += ay_draw_registerdacb(ay_root_drawacb, AY_IDROOT);

  Tcl_SetVar(interp, "propertyList", "RootAttr", TCL_APPEND_VALUE |
	     TCL_LIST_ELEMENT | TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  /* root objects may not be associated with materials */
  ay_matt_nomaterial(AY_IDROOT);

 return ay_status;
} /* ay_root_init */
