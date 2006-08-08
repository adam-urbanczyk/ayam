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

/* light.c - light object */

static char *ay_light_name = "Light";


int
ay_light_createcb(int argc, char *argv[], ay_object *o)
{
 ay_light_object *light = NULL;
 char fname[] = "crtlight";

  if(!o)
    return AY_ENULL;

  if(!(light = calloc(1, sizeof(ay_light_object))))
    {
      ay_error(AY_EOMEM, fname, NULL);
      return AY_ERROR;
    }

  light->type = AY_LITCUSTOM;
  light->on = AY_TRUE;
  light->samples = 1;

  light->colr = 255;
  light->colg = 255;
  light->colb = 255;
  light->intensity = 1.0;

  light->cone_angle = AY_D2R(30.0);
  light->cone_delta_angle = AY_D2R(5.0);
  light->beam_distribution = 1.0;

  light->tto[2] = -10.0;

  o->parent = AY_TRUE;
  o->inherit_trafos = AY_TRUE;
  o->refine = light;

 return AY_OK;
} /* ay_light_createcb */


int
ay_light_deletecb(void *c)
{
 ay_light_object *light = NULL;

  if(!c)
    return AY_ENULL;

  light = (ay_light_object *)(c);

  if(light->lshader)
    ay_shader_free(light->lshader);

  free(light);

 return AY_OK;
} /* ay_light_deletecb */


int
ay_light_copycb(void *src, void **dst)
{
 ay_light_object *light = NULL;
 int ay_status = AY_OK;

  if(!src || !dst)
    return AY_ENULL;

  if(!(light = calloc(1, sizeof(ay_light_object))))
    return AY_EOMEM;

  memcpy(light, src, sizeof(ay_light_object));
  /* warning links point to original object*/

  if(((ay_light_object*)src)->lshader)
    {
      ay_status = ay_shader_copy(((ay_light_object*)src)->lshader,
				 &(light->lshader));
    }

  *dst = (void *)light;

 return AY_OK;
} /* ay_light_copycb */


int
ay_light_drawcb(struct Togl *togl, ay_object *o)
{
 ay_light_object *light = NULL;
 ay_shader *shader = NULL;
 ay_shader_arg *sarg = NULL;
 GLdouble from[3] = {0};
 GLdouble to[3] = {0};
 GLdouble w;
 double radius = 0.0, len = 0.0, vd[3] = {0}, va[3] = {0}, vn[3] = {0};
 double quat[4] = {0}, rm[16];
 int has_from = 0, has_to = 0, i, a;
 double c1[24] = {0};
 GLfloat oldcolor[4] = {0.0f,0.0f,0.0f,0.0f};

  w = (GLdouble)(sqrt(2.0)*0.5);

  if(!o)
    return AY_ENULL;

  light = (ay_light_object *)o->refine;

  if(!light)
    return AY_ENULL;


  if(!o->selected)
    {
      /* save current color */
      glGetFloatv(GL_CURRENT_COLOR, oldcolor);
      /* set color for lights */
      glColor3d((GLdouble)ay_prefs.lir, (GLdouble)ay_prefs.lig,
		(GLdouble)ay_prefs.lib);
    }

    switch(light->type)
    {
    case AY_LITPOINT:
      has_from = AY_TRUE;
      from[0] = light->tfrom[0];
      from[1] = light->tfrom[1];
      from[2] = light->tfrom[2];
      break;
    case AY_LITSPOT:
    case AY_LITDISTANT:
      has_from = AY_TRUE;
      from[0] = light->tfrom[0];
      from[1] = light->tfrom[1];
      from[2] = light->tfrom[2];
      has_to = AY_TRUE;
      to[0] = light->tto[0];
      to[1] = light->tto[1];
      to[2] = light->tto[2];
      break;
    case AY_LITCUSTOM:
      if(light->lshader)
	{
	  shader = light->lshader;

	  sarg = shader->arg;
	  while(sarg)
	    {
	      if(!ay_comp_strcase(sarg->name, "from"))
		{
		  has_from = AY_TRUE;
		  from[0] = sarg->val.point[0];
		  from[1] = sarg->val.point[1];
		  from[2] = sarg->val.point[2];
		}
	      if(!ay_comp_strcase(sarg->name, "to"))
		{
		  has_to = AY_TRUE;
		  to[0] = sarg->val.point[0];
		  to[1] = sarg->val.point[1];
		  to[2] = sarg->val.point[2];
		}
	      sarg = sarg->next;
	    } /* while */
	} /* if */
      break;

    } /* switch */

  if(has_from)
    {
      glBegin(GL_LINES);
      glVertex3d(from[0]-(1.0/o->scalx),from[1],from[2]);
      glVertex3d(from[0]+(1.0/o->scalx),from[1],from[2]);
      glVertex3d(from[0],from[1]-(1.0/o->scaly),from[2]);
      glVertex3d(from[0],from[1]+(1.0/o->scaly),from[2]);
      glVertex3d(from[0],from[1],from[2]-(1.0/o->scalz));
      glVertex3d(from[0],from[1],from[2]+(1.0/o->scalz));
      glEnd();

      if(has_to)
	{
	  glBegin(GL_LINES);
	  glVertex3d(from[0],from[1],from[2]);
	  glVertex3d(to[0],to[1],to[2]);
	  glEnd();

	  /* draw direction pointer */
	  ay_draw_arrow(togl, from, to);
	}
    }

  if(light->type == AY_LITSPOT)
    {
      vd[0] = to[0]-from[0];
      vd[1] = to[1]-from[1];
      vd[2] = to[2]-from[2];

      len = AY_V3LEN(vd);

      if(len > AY_EPSILON)
	{

	  radius = tan(light->cone_angle)*len;
	  if(fabs(radius) < AY_EPSILON)
	    radius = 1.0;

	  a = 0;
	  for(i=0;i<8;i++)
	    {
	      c1[a]   = to[0];
	      c1[a+1] = to[1];
	      c1[a+2] = to[2];
	      a+=3;
	    }

	  if(fabs(vd[0]) > AY_EPSILON)
	    {
	      if(fabs(vd[1]) > AY_EPSILON)
		{
		  if(fabs(vd[2]) > AY_EPSILON)
		    { /* not aligned */
		      va[0] = 1.0;
		    }
		  else
		    { /* xy-plane */
		      va[2] = 1.0;
		    }
		}
	      else
		{
		  if(fabs(vd[2]) > AY_EPSILON)
		    { /* xz-plane */
		      va[1] = 1.0;
		    }
		  else
		    { /* x-axis */
		      va[1] = 1.0;
		    }
		}
	    }
	  else
	    {
	      if(fabs(vd[1]) > AY_EPSILON)
		{
		  if(fabs(vd[2]) > AY_EPSILON)
		    { /* yz-plane */
		      va[0] = 1.0;
		    }
		  else
		    { /* y-axis */
		      va[0] = 1.0;
		    }
		}
	      else
		{
		  if(fabs(vd[2]) > AY_EPSILON)
		    { /* z-axis */
		      va[0] = 1.0;
		    }
		}
	    }

	  /* create normal */
	  AY_V3CROSS(vn, vd, va)
	  /* scale normal to radius length */
	  len = AY_V3LEN(vn);
	  AY_V3SCAL(vn, radius/len)

	  c1[0] += vn[0];
	  c1[1] += vn[1];
	  c1[2] += vn[2];

	  /* create a rotation matrix */
	  ay_quat_axistoquat(vd, AY_D2R(45.0), quat);
	  ay_quat_torotmatrix(quat, rm);

	  /* apply matrix to all normals */
	  a = 3;
	  for(i=0;i<7;i++)
	    {
	      ay_trafo_apply3(vn,rm);
	      c1[a]   += vn[0];
	      c1[a+1] += vn[1];
	      c1[a+2] += vn[2];
	      a+=3;
	    }

	  /* draw */
	  a = 0;
	  glBegin(GL_LINES);
	  for(i=0;i<4;i++)
	    {
	      glVertex3d(from[0],from[1],from[2]);
	      glVertex3d(c1[a],c1[a+1],c1[a+2]);
	      a+=6;
	    }
	  glEnd();

	  a = 0;
	  glBegin(GL_LINE_LOOP);
	  for(i=0;i<8;i++)
	    {
	      glVertex3d(c1[a],c1[a+1],c1[a+2]);
	      a+=3;
	    }
	  glEnd();
	} /* if */
    } /* if */

  if(!o->selected)
    {
      /* restore old color */
      glColor3f(oldcolor[0], oldcolor[1], oldcolor[2]);
    }

 return AY_OK;
} /* ay_light_drawcb */


int
ay_light_drawhcb(struct Togl *togl, ay_object *o)
{
 ay_light_object *light = NULL;
 ay_shader *shader = NULL;
 ay_shader_arg *sarg = NULL;
 GLdouble from[3] = {0};
 GLdouble to[3] = {0};
 int has_from = 0, has_to = 0;

  if(!o)
    return AY_ENULL;

  light = (ay_light_object *)o->refine;

  if(!light)
    return AY_ENULL;

  if(light->type == AY_LITCUSTOM)
    {
      if(light->lshader)
	{
	  shader = light->lshader;

	  sarg = shader->arg;
	  while(sarg)
	    {
	      if(!ay_comp_strcase(sarg->name,"from"))
		{
		  has_from = AY_TRUE;
		  from[0] = sarg->val.point[0];
		  from[1] = sarg->val.point[1];
		  from[2] = sarg->val.point[2];
		}
	      if(!ay_comp_strcase(sarg->name,"to"))
		{
		  has_to = AY_TRUE;
		  to[0] = sarg->val.point[0];
		  to[1] = sarg->val.point[1];
		  to[2] = sarg->val.point[2];
		}
	      sarg = sarg->next;
	    } /* while */
	} /* if */
    }
  else
    {
      switch(light->type)
	{
	case AY_LITPOINT:
	  has_from = AY_TRUE;
	  from[0] = light->tfrom[0];
	  from[1] = light->tfrom[1];
	  from[2] = light->tfrom[2];
	  break;
	case AY_LITDISTANT:
	case AY_LITSPOT:
	  has_from = AY_TRUE;
	  from[0] = light->tfrom[0];
	  from[1] = light->tfrom[1];
	  from[2] = light->tfrom[2];
	  has_to = AY_TRUE;
	  to[0] = light->tto[0];
	  to[1] = light->tto[1];
	  to[2] = light->tto[2];
	  break;
	default:
	  break;
	} /* switch */
    } /* if */


  if(has_from)
    {
      glBegin(GL_POINTS);
       glVertex3dv(from);
      glEnd();
    }

  if(has_to)
    {
      glBegin(GL_POINTS);
       glVertex3dv(to);
      glEnd();
    }

 return AY_OK;
} /* ay_light_drawhcb */


int
ay_light_shadecb(struct Togl *togl, ay_object *o)
{
 ay_light_object *light = NULL;

  if(!o)
    return AY_ENULL;

  light = (ay_light_object *)o->refine;

  if(!light)
    return AY_ENULL;

 return AY_OK;
} /* ay_light_shadecb */


int
ay_light_getpntcb(int mode, ay_object *o, double *p)
{
 ay_light_object *light = NULL;
 ay_shader *shader = NULL;
 ay_shader_arg *sarg = NULL;
 double min_dist = ay_prefs.pick_epsilon, dist = 0.0;
 double *pecoord = NULL, **pecoords = NULL, *c;
 int has_from = 0, has_to = 0, numpts = 0, a = 0;

  if(!o || !p)
    return AY_ENULL;

  light = (ay_light_object *)(o->refine);

  if(min_dist == 0.0)
    min_dist = DBL_MAX;

  if(ay_point_edit_coords)
    free(ay_point_edit_coords);

  ay_point_edit_coords = NULL;

  if(light->type == AY_LITCUSTOM)
    {
      if(light->lshader)
	{
	  shader = light->lshader;

	  sarg = shader->arg;
	  while(sarg)
	    {
	      if(!ay_comp_strcase(sarg->name, "from"))
		{
		  has_from = AY_TRUE;
		  light->tfrom[0] = sarg->val.point[0];
		  light->tfrom[1] = sarg->val.point[1];
		  light->tfrom[2] = sarg->val.point[2];
		}
	      if(!ay_comp_strcase(sarg->name, "to"))
		{
		  has_to = AY_TRUE;
		  light->tto[0] = sarg->val.point[0];
		  light->tto[1] = sarg->val.point[1];
		  light->tto[2] = sarg->val.point[2];
		}
	      sarg = sarg->next;
	    } /* while */
	} /* if */
    }
  else
    {
      switch(light->type)
	{
	case AY_LITPOINT:
	  has_from = AY_TRUE;
	  break;
	case AY_LITDISTANT:
	case AY_LITSPOT:
	  has_from = AY_TRUE;
	  has_to = AY_TRUE;
	  break;
	default:
	  break;
	} /* switch */
    } /* if */

  /* select all points? */
  if(mode == 0)
    { /* yes */
      if(has_from)
	numpts++;

      if(has_to)
	numpts++;

      if(numpts > 0)
	{
	  if(!(ay_point_edit_coords = calloc(numpts, sizeof(double*))))
	    return AY_EOMEM;

	  if(has_from)
	    ay_point_edit_coords[0] = light->tfrom;

	  if(has_to)
	    {
	      if(has_from)
		ay_point_edit_coords[1] = light->tto;
	      else
		ay_point_edit_coords[0] = light->tto;
	    }

	  ay_point_edit_coords_homogenous = AY_FALSE;
	  ay_point_edit_coords_number = numpts;
	}
    }
  else
    { /* no */

      /* selection based on a single point? */
      if(mode == 1)
	{ /* yes */

	  if(has_from)
	    {
	      dist = AY_VLEN((p[0] - light->tfrom[0]),
			     (p[1] - light->tfrom[1]),
			     (p[2] - light->tfrom[2]));

	      if(dist < min_dist)
		{
		  pecoord = light->tfrom;
		  min_dist = dist;
		}
	    }
	  if(has_to)
	    {
	      dist = AY_VLEN((p[0] - light->tto[0]),
			     (p[1] - light->tto[1]),
			     (p[2] - light->tto[2]));

	      if(dist < min_dist)
		{
		  pecoord = light->tto;
		  min_dist = dist;
		}
	    }

	  if(!pecoord)
	    return AY_OK; /* XXXX should this return a 'AY_EPICK' ? */

	  if(!(ay_point_edit_coords = calloc(1, sizeof(double*))))
	    return AY_EOMEM;

	  ay_point_edit_coords[0] = pecoord;
	  ay_point_edit_coords_homogenous = AY_FALSE;
	  ay_point_edit_coords_number = 1;

	}
      else
	{ /* no */

	  /* selection based on planes */
	  if(has_from)
	    {
	      c = light->tfrom;
	      /* test point c against the four planes in p */
	      if(((p[0]*c[0] + p[1]*c[1] + p[2]*c[2] + p[3]) < 0.0) &&
		 ((p[4]*c[0] + p[5]*c[1] + p[6]*c[2] + p[7]) < 0.0) &&
		 ((p[8]*c[0] + p[9]*c[1] + p[10]*c[2] + p[11]) < 0.0) &&
		 ((p[12]*c[0] + p[13]*c[1] + p[14]*c[2] + p[15]) < 0.0))
		{

		  if(!(pecoords = realloc(pecoords, (a+1)*sizeof(double *))))
		    return AY_EOMEM;
		  pecoords[a] = c;
		  a++;
		} /* if */
	    } /* if */

	  if(has_to)
	    {
	      c = light->tto;
	      /* test point c against the four planes in p */
	      if(((p[0]*c[0] + p[1]*c[1] + p[2]*c[2] + p[3]) < 0.0) &&
		 ((p[4]*c[0] + p[5]*c[1] + p[6]*c[2] + p[7]) < 0.0) &&
		 ((p[8]*c[0] + p[9]*c[1] + p[10]*c[2] + p[11]) < 0.0) &&
		 ((p[12]*c[0] + p[13]*c[1] + p[14]*c[2] + p[15]) < 0.0))
		{

		  if(!(pecoords = realloc(pecoords, (a+1)*sizeof(double *))))
		    return AY_EOMEM;
		  pecoords[a] = c;
		  a++;
		} /* if */
	    } /* if */

	  if(!pecoords)
	    return AY_OK; /* XXXX should this return a 'AY_EPICK' ? */

	  ay_point_edit_coords_homogenous = AY_FALSE;
	  ay_point_edit_coords = pecoords;
	  ay_point_edit_coords_number = a;

	} /* if */
    } /* if */

 return AY_OK;
} /* ay_light_getpntcb */


/* Tcl -> C! */
int
ay_light_setpropcb(Tcl_Interp *interp, int argc, char *argv[], ay_object *o)
{
 int ay_status = AY_OK;
 char *n1 = "LightAttrData";
 double dtemp = 0.0;
 Tcl_Obj *to = NULL, *toa = NULL, *ton = NULL;
 ay_light_object *light = NULL;

  if(!o)
   return AY_ENULL;

  light = (ay_light_object *)o->refine;

  toa = Tcl_NewStringObj(n1,-1);

  ton = Tcl_NewStringObj("Type",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp, to, &(light->type));

  Tcl_SetStringObj(ton,"IsOn",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp, to, &(light->on));

  Tcl_SetStringObj(ton,"IsLocal",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp, to, &(light->local));

  Tcl_SetStringObj(ton,"Shadows",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp, to, &(light->shadows));

  Tcl_SetStringObj(ton,"Samples",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp, to, &(light->samples));

  Tcl_SetStringObj(ton,"Intensity",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetDoubleFromObj(interp, to, &(light->intensity));

  Tcl_SetStringObj(ton,"Color_R",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp, to, &(light->colr));
  Tcl_SetStringObj(ton,"Color_G",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp, to, &(light->colg));
  Tcl_SetStringObj(ton,"Color_B",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp, to, &(light->colb));

  Tcl_SetStringObj(ton,"ConeAngle",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetDoubleFromObj(interp, to, &(dtemp));

  light->cone_angle = AY_D2R(dtemp);

  Tcl_SetStringObj(ton,"ConeDAngle",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetDoubleFromObj(interp, to, &(dtemp));

  light->cone_delta_angle = AY_D2R(dtemp);

  Tcl_SetStringObj(ton,"BeamDistrib",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetDoubleFromObj(interp, to, &(light->beam_distribution));

  Tcl_SetStringObj(ton,"UseSM",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp, to, &(light->use_sm));

  Tcl_SetStringObj(ton,"SMRes",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp, to, &(light->sm_resolution));

  Tcl_SetStringObj(ton,"From_X",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetDoubleFromObj(interp, to, &(light->tfrom[0]));
  Tcl_SetStringObj(ton,"From_Y",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetDoubleFromObj(interp, to, &(light->tfrom[1]));
  Tcl_SetStringObj(ton,"From_Z",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetDoubleFromObj(interp, to, &(light->tfrom[2]));

  Tcl_SetStringObj(ton,"To_X",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetDoubleFromObj(interp, to, &(light->tto[0]));
  Tcl_SetStringObj(ton,"To_Y",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetDoubleFromObj(interp, to, &(light->tto[1]));
  Tcl_SetStringObj(ton,"To_Z",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetDoubleFromObj(interp, to, &(light->tto[2]));

  Tcl_IncrRefCount(toa);Tcl_DecrRefCount(toa);
  Tcl_IncrRefCount(ton);Tcl_DecrRefCount(ton);

  ay_notify_force(o);

  o->modified = AY_TRUE;
  ay_status = ay_notify_parent();

 return AY_OK;
} /* ay_light_setpropcb */


/* C -> Tcl! */
int
ay_light_getpropcb(Tcl_Interp *interp, int argc, char *argv[], ay_object *o)
{
 char *n1="LightAttrData";
 Tcl_Obj *to = NULL, *toa = NULL, *ton = NULL;
 ay_light_object *light = NULL;

  if(!o)
    return AY_ENULL;

  light = (ay_light_object *)(o->refine);

  toa = Tcl_NewStringObj(n1,-1);

  ton = Tcl_NewStringObj("Type",-1);
  to = Tcl_NewIntObj(light->type);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton,"IsOn",-1);
  to = Tcl_NewIntObj(light->on);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton,"IsLocal",-1);
  to = Tcl_NewIntObj(light->local);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton,"Shadows",-1);
  to = Tcl_NewIntObj(light->shadows);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton,"Samples",-1);
  to = Tcl_NewIntObj(light->samples);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton,"Color_R",-1);
  to = Tcl_NewIntObj(light->colr);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_SetStringObj(ton,"Color_G",-1);
  to = Tcl_NewIntObj(light->colg);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_SetStringObj(ton,"Color_B",-1);
  to = Tcl_NewIntObj(light->colb);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton,"Intensity",-1);
  to = Tcl_NewDoubleObj(light->intensity);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton,"ConeAngle",-1);
  to = Tcl_NewDoubleObj(AY_R2D(light->cone_angle));
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton,"ConeDAngle",-1);
  to = Tcl_NewDoubleObj(AY_R2D(light->cone_delta_angle));
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton,"BeamDistrib",-1);
  to = Tcl_NewDoubleObj(light->beam_distribution);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton,"UseSM",-1);
  to = Tcl_NewIntObj(light->use_sm);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton,"SMRes",-1);
  to = Tcl_NewIntObj(light->sm_resolution);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton,"From_X",-1);
  to = Tcl_NewDoubleObj(light->tfrom[0]);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_SetStringObj(ton,"From_Y",-1);
  to = Tcl_NewDoubleObj(light->tfrom[1]);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_SetStringObj(ton,"From_Z",-1);
  to = Tcl_NewDoubleObj(light->tfrom[2]);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton,"To_X",-1);
  to = Tcl_NewDoubleObj(light->tto[0]);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_SetStringObj(ton,"To_Y",-1);
  to = Tcl_NewDoubleObj(light->tto[1]);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_SetStringObj(ton,"To_Z",-1);
  to = Tcl_NewDoubleObj(light->tto[2]);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_IncrRefCount(toa);Tcl_DecrRefCount(toa);
  Tcl_IncrRefCount(ton);Tcl_DecrRefCount(ton);

 return AY_OK;
} /* ay_light_getpropcb */


int
ay_light_readcb(FILE *fileptr, ay_object *o)
{
 ay_light_object *light = NULL;
 int has_shader = AY_FALSE, ay_status = AY_OK;
 double p[3] = {DBL_MAX, DBL_MAX, DBL_MAX};

  if(!o)
   return AY_ENULL;

  if(!(light = calloc(1, sizeof(ay_light_object))))
    { return AY_EOMEM; }


  fscanf(fileptr,"%d\n",&light->shadows);
  fscanf(fileptr,"%d\n",&light->samples);
  fscanf(fileptr,"%d\n",&light->type);
  fscanf(fileptr,"%d\n",&light->on);
  fscanf(fileptr,"%lg\n",&light->intensity);
  fscanf(fileptr,"%d\n",&light->colr);
  fscanf(fileptr,"%d\n",&light->colg);
  fscanf(fileptr,"%d\n",&light->colb);
  fscanf(fileptr,"%lg\n",&light->cone_angle);
  fscanf(fileptr,"%lg\n",&light->cone_delta_angle);
  fscanf(fileptr,"%lg\n",&light->beam_distribution);
  fscanf(fileptr,"%d\n",&light->use_sm);
  fscanf(fileptr,"%d\n",&light->sm_resolution);
  fscanf(fileptr,"%lg\n",&light->tfrom[0]);
  fscanf(fileptr,"%lg\n",&light->tfrom[1]);
  fscanf(fileptr,"%lg\n",&light->tfrom[2]);
  fscanf(fileptr,"%lg\n",&light->tto[0]);
  fscanf(fileptr,"%lg\n",&light->tto[1]);
  fscanf(fileptr,"%lg\n",&light->tto[2]);
  fscanf(fileptr,"%d\n",&has_shader);

  o->refine = light;

  if(has_shader)
    {
      ay_status = ay_read_shader(fileptr, &light->lshader);
      /* copy shader from/to to light.tfrom light.tto */
      ay_status = ay_pact_getpoint(1, o, p);

    }

  if(ay_read_version >= 5)
    {
      fscanf(fileptr,"%d\n",&light->local);
    }

 return AY_OK;
} /* ay_light_readcb */


int
ay_light_writecb(FILE *fileptr, ay_object *o)
{
 ay_light_object *light = NULL;
 int ay_status = AY_OK;

  if(!o)
    return AY_ENULL;

  light = (ay_light_object *)(o->refine);

  fprintf(fileptr, "%d\n", light->shadows);
  fprintf(fileptr, "%d\n", light->samples);
  fprintf(fileptr, "%d\n", light->type);
  fprintf(fileptr, "%d\n", light->on);
  fprintf(fileptr, "%g\n", light->intensity);
  fprintf(fileptr, "%d\n", light->colr);
  fprintf(fileptr, "%d\n", light->colg);
  fprintf(fileptr, "%d\n", light->colb);

  fprintf(fileptr, "%g\n", light->cone_angle);
  fprintf(fileptr, "%g\n", light->cone_delta_angle);
  fprintf(fileptr, "%g\n", light->beam_distribution);
  fprintf(fileptr, "%d\n", light->use_sm);
  fprintf(fileptr, "%d\n", light->sm_resolution);
  fprintf(fileptr, "%g\n", light->tfrom[0]);
  fprintf(fileptr, "%g\n", light->tfrom[1]);
  fprintf(fileptr, "%g\n", light->tfrom[2]);
  fprintf(fileptr, "%g\n", light->tto[0]);
  fprintf(fileptr, "%g\n", light->tto[1]);
  fprintf(fileptr, "%g\n", light->tto[2]);

  if(light->lshader)
    {
      fprintf(fileptr, "1\n");
      ay_status = ay_write_shader(fileptr, light->lshader);
    }
  else
    {
      fprintf(fileptr, "0\n");
    }

  fprintf(fileptr, "%d\n", light->local);

 return AY_OK;
} /* ay_light_writecb */


/* this callback does nothing
 * the real work of writing light objects to RIB
 * is done in aycore/wrib.c/ay_wrib_lights()
 */
int
ay_light_wribcb(char *file, ay_object *o)
{

  if(!o)
   return AY_ENULL;

 return AY_OK;
} /* ay_light_wribcb */


int
ay_light_bbccb(ay_object *o, double *bbox, int *flags)
{
 double xmin = DBL_MAX, xmax = -DBL_MAX, ymin = DBL_MAX;
 double ymax = -DBL_MAX, zmin = DBL_MAX, zmax = -DBL_MAX;
 double from[3] = {0};
 double to[3] = {0};
 int has_from = 0, has_to = 0;
 ay_light_object *light = NULL;
 ay_shader *shader = NULL;
 ay_shader_arg *sarg = NULL;

  if(!o || !bbox)
    return AY_ENULL;

  light = (ay_light_object *)o->refine;

  switch(light->type)
    {
    case AY_LITPOINT:
      has_from = AY_TRUE;
      from[0] = light->tfrom[0];
      from[1] = light->tfrom[1];
      from[2] = light->tfrom[2];
      break;
    case AY_LITSPOT:
    case AY_LITDISTANT:
      has_from = AY_TRUE;
      from[0] = light->tfrom[0];
      from[1] = light->tfrom[1];
      from[2] = light->tfrom[2];
      has_to = AY_TRUE;
      to[0] = light->tto[0];
      to[1] = light->tto[1];
      to[2] = light->tto[2];
      break;
    case AY_LITCUSTOM:
      if(light->lshader)
	{
	  shader = light->lshader;

	  sarg = shader->arg;
	  while(sarg)
	    {
	      if(!ay_comp_strcase(sarg->name,"from"))
		{
		  has_from = AY_TRUE;
		  from[0] = sarg->val.point[0];
		  from[1] = sarg->val.point[1];
		  from[2] = sarg->val.point[2];
		}
	      if(!ay_comp_strcase(sarg->name,"to"))
		{
		  has_to = AY_TRUE;
		  to[0] = sarg->val.point[0];
		  to[1] = sarg->val.point[1];
		  to[2] = sarg->val.point[2];
		}
	      sarg = sarg->next;
	    } /* while */
	} /* if */
      break;

    } /* switch */

  if(has_from)
    {
      if(from[0] < xmin)
	xmin = from[0];
      if(from[0] > xmax)
	xmax = from[0];

      if(from[1] < ymin)
	ymin = from[1];
      if(from[1] > ymax)
	ymax = from[1];

      if(from[2] < zmin)
	zmin = from[2];
      if(from[2] > zmax)
	zmax = from[2];
    } /* if */

  if(has_to)
    {
      if(to[0] < xmin)
	xmin = to[0];
      if(to[0] > xmax)
	xmax = to[0];

      if(to[1] < ymin)
	ymin = to[1];
      if(to[1] > ymax)
	ymax = to[1];

      if(to[2] < zmin)
	zmin = to[2];
      if(to[2] > zmax)
	zmax = to[2];
    } /* if */

  if(has_from || has_to)
    {
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
    }

 return AY_OK;
} /* ay_light_bbccb */


int
ay_light_notifycb(ay_object *o)
{
 char fname[] = "light_notify";
 ay_light_object *light = NULL;
 ay_shader *shader = NULL;
 ay_shader_arg *sarg = NULL;

  if(!o)
    return AY_ENULL;

  light = (ay_light_object *)(o->refine);

  /* warn user, if light shader exists but type is not custom */
  if((light->type != AY_LITCUSTOM) && light->lshader)
    {
      ay_error(AY_EWARN, fname,
	       "This light will not use the attached light shader!");
    }


  /*
   * no action needed for objects of internal types;
   * but for custom lights, that were modified by single point
   * editing actions (hence the test for o->modified), we need
   * to transfer the point coords from "tto" and "tfrom" to the
   * attached light shader
   */
  if((light->type == AY_LITCUSTOM) && (o->modified))
    {
      o->modified = AY_FALSE;
      if(light->lshader)
	{
	  shader = light->lshader;

	  sarg = shader->arg;
	  while(sarg)
	    {
	      if(!ay_comp_strcase(sarg->name, "from"))
		{
		  sarg->val.point[0] = (float)light->tfrom[0];
		  sarg->val.point[1] = (float)light->tfrom[1];
		  sarg->val.point[2] = (float)light->tfrom[2];
		}
	      if(!ay_comp_strcase(sarg->name, "to"))
		{
		  sarg->val.point[0] = (float)light->tto[0];
		  sarg->val.point[1] = (float)light->tto[1];
		  sarg->val.point[2] = (float)light->tto[2];
		}
	      sarg = sarg->next;
	    } /* while */
	} /* if */
    } /* if */

 return AY_OK;
} /* ay_light_notifycb */


int
ay_light_init(Tcl_Interp *interp)
{
 int ay_status = AY_OK;

  ay_status = ay_otype_registercore(ay_light_name,
				    ay_light_createcb,
				    ay_light_deletecb,
				    ay_light_copycb,
				    ay_light_drawcb,
				    ay_light_drawhcb,
				    ay_light_shadecb,
				    ay_light_setpropcb,
				    ay_light_getpropcb,
				    ay_light_getpntcb,
				    ay_light_readcb,
				    ay_light_writecb,
				    NULL, /* no RIB export */
				    ay_light_bbccb,
				    AY_IDLIGHT);

  ay_status = ay_notify_register(ay_light_notifycb, AY_IDLIGHT);

  ay_matt_nomaterial(AY_IDLIGHT);

 return ay_status;
} /* ay_light_init */

