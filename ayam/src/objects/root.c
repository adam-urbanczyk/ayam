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

/* root.c - root object */

static char *ay_root_name = "Root";

int
ay_root_createcb(int argc, char *argv[], ay_object *o)
{
 ay_root_object *root = NULL;
 char fname[] = "crtroot";
 ay_riopt_object *riopt = NULL;

  if(!o)
    return AY_ENULL;

  if(!(root = calloc(1, sizeof(ay_root_object))))
    {
      ay_error(AY_EOMEM, fname, NULL);
      return AY_ERROR;
    }

  if(!(root->riopt = calloc(1, sizeof(ay_riopt_object))))
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
  riopt->FilterWidth = 2;
  riopt->FilterHeight = 2;
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


  o->parent = AY_TRUE;
  o->refine = root;

 return AY_OK;
} /* ay_root_createcb */


int
ay_root_deletecb(void *c)
{
 ay_root_object *root = NULL;

  if(!c)
    return AY_ENULL;

  root = (ay_root_object *)(c);

  free(root->riopt);
  free(root);

 return AY_OK;
} /* ay_root_deletecb */


int
ay_root_copycb(void *src, void **dst)
{
 char fname[] = "ay_root_copycb";

  ay_error(AY_ERROR, fname, "can not copy a root object");

 return AY_ERROR;
} /* ay_root_copycb */


int
ay_root_drawcb(struct Togl *togl, ay_object *o)
{
 GLdouble mvm[16], pm[16];
 GLdouble win1x, win1y, win1z, win2x, win2y, win2z, win3x, win3y, win3z;
 GLfloat color[4] = {0.0f,0.0f,0.0f,0.0f};
 GLint vp[4];

  if(!o)
    return AY_ENULL;

  glGetFloatv(GL_CURRENT_COLOR, color);

  /* ignore transformation */
  glLoadIdentity();

  /* draw */
  glBegin(GL_LINES);
   glColor3f((GLfloat)1.0, (GLfloat)0.0, (GLfloat)0.0);
   glVertex3f((GLfloat)0.0, (GLfloat)0.0, (GLfloat)0.0);
   glVertex3f((GLfloat)1.0, (GLfloat)0.0, (GLfloat)0.0);
   glColor3f((GLfloat)0.0, (GLfloat)1.0, (GLfloat)0.0);
   glVertex3f((GLfloat)0.0, (GLfloat)0.0, (GLfloat)0.0);
   glVertex3f((GLfloat)0.0, (GLfloat)1.0, (GLfloat)0.0);
   glColor3f((GLfloat)0.0, (GLfloat)0.0, (GLfloat)1.0);
   glVertex3f((GLfloat)0.0, (GLfloat)0.0, (GLfloat)0.0);
   glVertex3f((GLfloat)0.0, (GLfloat)0.0, (GLfloat)1.0);
  glEnd();

  glColor4fv(color);

  glGetDoublev(GL_MODELVIEW_MATRIX, mvm);
  glGetDoublev(GL_PROJECTION_MATRIX, pm);
  glGetIntegerv(GL_VIEWPORT, vp);
  gluProject((GLdouble)1.0, (GLdouble)0.0,
	     (GLdouble)0.0, mvm, pm, vp,
	     &win1x, &win1y, &win1z);
  gluProject((GLdouble)0.0, (GLdouble)1.0,
	     (GLdouble)0.0, mvm, pm, vp,
	     &win2x, &win2y, &win2z);
  gluProject((GLdouble)0.0, (GLdouble)0.0,
	     (GLdouble)1.0, mvm, pm, vp,
	     &win3x, &win3y, &win3z);

  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
   glLoadIdentity();
   glOrtho(0, Togl_Width(togl), 0, Togl_Height(togl), -100.0, 100.0);

   glMatrixMode(GL_MODELVIEW);
   glPushMatrix();
    glLoadIdentity();
    glTranslated(win1x+6,win1y,0.0);
    glBegin(GL_LINES);
     glVertex3f((GLfloat)0, (GLfloat)0, (GLfloat)0.0);
     glVertex3f((GLfloat)-4, (GLfloat)-4, (GLfloat)0.0);
     glVertex3f((GLfloat)0, (GLfloat)0, (GLfloat)0.0);
     glVertex3f((GLfloat)4, (GLfloat)4, (GLfloat)0.0);
     glVertex3f((GLfloat)0, (GLfloat)0, (GLfloat)0.0);
     glVertex3f((GLfloat)-4, (GLfloat)4, (GLfloat)0.0);
     glVertex3f((GLfloat)0, (GLfloat)0, (GLfloat)0.0);
     glVertex3f((GLfloat)4, (GLfloat)-4, (GLfloat)0.0);
    glEnd();
    glLoadIdentity();
    glTranslated(win2x+6,win2y,0.0);
    glBegin(GL_LINES);
     glVertex3f((GLfloat)0, (GLfloat)0, (GLfloat)0.0);
     glVertex3f((GLfloat)-4, (GLfloat)4, (GLfloat)0.0);
     glVertex3f((GLfloat)0, (GLfloat)0, (GLfloat)0.0);
     glVertex3f((GLfloat)4, (GLfloat)4, (GLfloat)0.0);
     glVertex3f((GLfloat)0, (GLfloat)0, (GLfloat)0.0);
     glVertex3f((GLfloat)-4, (GLfloat)-4, (GLfloat)0.0);
    glEnd();
    glLoadIdentity();
    glTranslated(win3x+6,win3y,0.0);
    glBegin(GL_LINE_STRIP);
     glVertex3f((GLfloat)0, (GLfloat)0, (GLfloat)0.0);
     glVertex3f((GLfloat)-3, (GLfloat)-3, (GLfloat)0.0);
     glVertex3f((GLfloat)4, (GLfloat)-3, (GLfloat)0.0);
    glEnd();
    glBegin(GL_LINE_STRIP);
     glVertex3f((GLfloat)0, (GLfloat)0, (GLfloat)0.0);
     glVertex3f((GLfloat)3, (GLfloat)3, (GLfloat)0.0);
     glVertex3f((GLfloat)-4, (GLfloat)3, (GLfloat)0.0);
    glEnd();
    glPopMatrix();
   glMatrixMode(GL_PROJECTION);
   glPopMatrix();
  glMatrixMode(GL_MODELVIEW);

 return AY_OK;
} /* ay_root_drawcb */


int
ay_root_drawhcb(struct Togl *togl, ay_object *o)
{
  if(!o)
    return AY_ENULL;

 return AY_OK;
} /* ay_root_drawhcb */


int
ay_root_shadecb(struct Togl *togl, ay_object *o)
{
  if(!o)
    return AY_ENULL;

 return AY_OK;
} /* ay_root_shadecb */


int
ay_root_getpntcb(ay_object *o, double *p)
{
  if(!o)
    return AY_ENULL;

 return AY_OK;
} /* ay_root_getpntcb */


/* Tcl -> C! */
int
ay_root_setpropcb(Tcl_Interp *interp, int argc, char *argv[], ay_object *o)
{
 char fname[] = "ay_root_setpropcb";
 char *n1 = "RiOptData";
 Tcl_Obj *to = NULL, *toa = NULL, *ton = NULL;
 ay_root_object *root = NULL;
 ay_riopt_object *riopt = NULL;
 double dtemp = 0.0;
 int itemp = 0;
 char *result;

  if(!o)
    return AY_ENULL;

  root = (ay_root_object *)o->refine;
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
  Tcl_GetIntFromObj(interp, to, &itemp);
  riopt->FilterWidth = itemp;
  Tcl_SetStringObj(ton, "FilterHeight", -1);
  to = Tcl_ObjGetVar2(interp, toa, ton, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp, to, &itemp);
  riopt->FilterHeight = itemp;
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
      return TCL_OK;
    }
  strcpy(riopt->textures, result);

  result = Tcl_GetVar2(interp, n1, "Includes",
		       TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  if(riopt->includes)
    {
      free(riopt->includes);
      riopt->includes = NULL;
    }

  if(!(riopt->includes = calloc(strlen(result)+1, sizeof(char))))
    {
      ay_error(AY_EOMEM, fname, NULL);
      return TCL_OK;
    }
  strcpy(riopt->includes, result);

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
      return TCL_OK;
    }
  strcpy(riopt->shaders, result);


  Tcl_SetStringObj(ton, "TextureMem", -1);
  to = Tcl_ObjGetVar2(interp, toa, ton, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp, to, &itemp);
  riopt->texturemem = itemp;

  Tcl_SetStringObj(ton, "GeomMem", -1);
  to = Tcl_ObjGetVar2(interp, toa, ton, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp, to, &itemp);
  riopt->geommem = itemp;

  Tcl_IncrRefCount(toa);Tcl_DecrRefCount(toa);
  Tcl_IncrRefCount(ton);Tcl_DecrRefCount(ton);

 return AY_OK;
} /* ay_root_setpropcb */


/* C -> Tcl! */
int
ay_root_getpropcb(Tcl_Interp *interp, int argc, char *argv[], ay_object *o)
{
 char *n1="RiOptData";
 Tcl_Obj *to = NULL, *toa = NULL, *ton = NULL;
 ay_root_object *root = NULL;
 ay_riopt_object *riopt = NULL;

  if(!o)
    return AY_ENULL;

  root = (ay_root_object *)o->refine;
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
  to = Tcl_NewIntObj(riopt->FilterWidth);
  Tcl_ObjSetVar2(interp, toa, ton, to, TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);
  Tcl_SetStringObj(ton, "FilterHeight", -1);
  to = Tcl_NewIntObj(riopt->FilterHeight);
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
  Tcl_SetStringObj(ton, "Includes", -1);
  to = Tcl_NewStringObj(riopt->includes, -1);
  Tcl_ObjSetVar2(interp, toa, ton, to, TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);
  Tcl_SetStringObj(ton, "Shaders", -1);
  to = Tcl_NewStringObj(riopt->shaders, -1);
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

  Tcl_IncrRefCount(toa);Tcl_DecrRefCount(toa);
  Tcl_IncrRefCount(ton);Tcl_DecrRefCount(ton);

 return AY_OK;
} /* ay_root_getpropcb */


int
ay_root_readcb(FILE *fileptr, ay_object *o)
{
 int ay_status = AY_OK;
 ay_root_object *root = NULL;
 int read, itemp = 0, has_atmosphere = 0, has_imager = 0;
 ay_riopt_object *riopt = NULL;

  if(!o)
    return AY_ENULL;

  if(!ay_root)
    return AY_ENULL;

  root = (ay_root_object *)(ay_root->refine);

  if(!root)
    return AY_ENULL;

  /* read RiOptions */
  if(!(riopt = calloc(1, sizeof(ay_riopt_object))))
    { return AY_EOMEM; }

  fscanf(fileptr,"%lg\n",&riopt->Variance);
  fscanf(fileptr,"%lg\n",&riopt->Samples_X);
  fscanf(fileptr,"%lg\n",&riopt->Samples_Y);
  fscanf(fileptr,"%d\n",&itemp);
  riopt->FilterFunc = (char)itemp;
  fscanf(fileptr,"%d\n",&riopt->FilterWidth);
  fscanf(fileptr,"%d\n",&riopt->FilterHeight);
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
  read = fgetc(fileptr);

  if(read == '\r')
    fgetc(fileptr);

  if(riopt->textures)
    {
      free(riopt->textures);
      riopt->textures = NULL;
    }
  ay_read_string(fileptr, &(riopt->textures));
  if(riopt->includes)
    {
      free(riopt->includes);
      riopt->includes = NULL;
    }
  ay_read_string(fileptr, &(riopt->includes));
  if(riopt->shaders)
    {
      free(riopt->shaders);
      riopt->shaders = NULL;
    }
  ay_read_string(fileptr, &(riopt->shaders));

  fscanf(fileptr,"%d\n",&riopt->texturemem);
  fscanf(fileptr,"%d\n",&riopt->geommem);

  fscanf(fileptr,"%d\n",&riopt->width);
  fscanf(fileptr,"%d\n",&riopt->height);

  if(root->riopt)
    free(root->riopt);

  root->riopt = riopt;

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
    }

  /* link newly read tags to old root object */
  ay_tags_delall(ay_root);
  if(o->tags)
    {
      ay_root->tags = o->tags;
      o->tags = NULL;
    }

 return AY_EDONOTLINK;
} /* ay_root_readcb */


int
ay_root_writecb(FILE *fileptr, ay_object *o)
{
 ay_root_object *root = NULL;
 ay_riopt_object *riopt = NULL;

  if(!o)
    return AY_ENULL;

  root = (ay_root_object*)(o->refine);
  

  /* write RiOptions */
  riopt = root->riopt;

  fprintf(fileptr,"%g\n",riopt->Variance);
  fprintf(fileptr,"%g\n",riopt->Samples_X);
  fprintf(fileptr,"%g\n",riopt->Samples_Y);
  fprintf(fileptr,"%d\n",(int)(riopt->FilterFunc));
  fprintf(fileptr,"%d\n",riopt->FilterWidth);
  fprintf(fileptr,"%d\n",riopt->FilterHeight);
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

  if(riopt->includes)
    fprintf(fileptr,"%s\n",riopt->includes);
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


 return AY_OK;
} /* ay_root_writecb */


int
ay_root_wribcb(char *file, ay_object *o)
{
 ay_root_object *root = NULL;
 ay_riopt_object *riopt = NULL;
 RtInt fw = 0, fh = 0;
 RtFloat rtftemp = 0.0f;
 RtInt rtitemp = 0;

  if(!o)
   return AY_ENULL;

  root = (ay_root_object*)o->refine;
  riopt = root->riopt;


  /* wrib RiOptions */
  if(riopt->Variance > 0.0)
    RiPixelVariance((RtFloat)riopt->Variance);
  else
    RiPixelSamples((RtFloat)riopt->Samples_X,
		   (RtFloat)riopt->Samples_Y);

  if(riopt->FilterWidth <= 0)
    fw = (RtInt)2;
  else
    fw = (RtInt)riopt->FilterWidth;

  if(riopt->FilterHeight <= 0)
    fh = (RtInt)2;
  else
    fh = (RtInt)riopt->FilterHeight;

  switch(riopt->FilterFunc)
    {
    case 1:
      RiPixelFilter(RiTriangleFilter,(RtFloat)fw,(RtFloat)fh);
      break;
    case 2:
      RiPixelFilter(RiCatmullRomFilter,(RtFloat)fw,(RtFloat)fh);
      break;
    case 3:
      RiPixelFilter(RiBoxFilter,(RtFloat)fw,(RtFloat)fh);
      break;
    case 4:
      RiPixelFilter(RiSincFilter,(RtFloat)fw,(RtFloat)fh);
      break;
    default:
      RiPixelFilter(RiGaussianFilter,(RtFloat)fw,(RtFloat)fh);
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

    if(riopt->includes)
      if((riopt->textures)[0] != '\0')
	RiOption((RtToken)"searchpath", (RtToken)"include",
		 (RtPointer)(&riopt->includes), RI_NULL);

    if(riopt->shaders)
      if((riopt->textures)[0] != '\0')
	RiOption((RtToken)"searchpath", (RtToken)"shader",
		 (RtPointer)(&riopt->shaders), RI_NULL);

    rtitemp = riopt->texturemem;
    RiOption((RtToken)"limits", (RtToken)"texturememory",
	     &rtitemp, RI_NULL);

    rtitemp = riopt->geommem;
    RiOption((RtToken)"limits", (RtToken)"geommemory",
	     &rtitemp, RI_NULL);
  }



 return AY_OK;
} /* ay_root_wribcb */


int
ay_root_bbccb(ay_object *o, double *bbox, int *flags)
{

  if(!o || !bbox)
    return AY_ENULL;

  /* we have no own bounding box, all that counts are the children */
  *flags = 2;

 return AY_OK;
} /* ay_root_bbccb */


int
ay_root_init(Tcl_Interp *interp)
{
 int ay_status = AY_OK;

  ay_status = ay_otype_registercore(ay_root_name,
				    ay_root_createcb,
				    ay_root_deletecb,
				    ay_root_copycb,
				    ay_root_drawcb,
				    NULL, /* no handles! */
				    NULL, /* no shading! */
				    ay_root_setpropcb,
				    ay_root_getpropcb,
				    NULL, /* no picking! */
				    ay_root_readcb,
				    ay_root_writecb,
				    ay_root_wribcb,
				    ay_root_bbccb,
				    AY_IDROOT);

  Tcl_SetVar(interp, "propertyList", "RootAttr", TCL_APPEND_VALUE |
	     TCL_LIST_ELEMENT | TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  ay_matt_nomaterial(AY_IDROOT);

 return ay_status;
} /* ay_root_init */
