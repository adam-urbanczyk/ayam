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

/* trafo.c - functions for handling of linear transformations */

/* ay_trafo_apply3:
 *  apply transformations in transformation matrix m[16] to point c[3]
 */
void
ay_trafo_apply3(double *c, double *m)
{
 double result[3] = {0};

  result[0] = m[0]*c[0] + m[4]*c[1] + m[8] *c[2] + m[12]*1.0;
  result[1] = m[1]*c[0] + m[5]*c[1] + m[9] *c[2] + m[13]*1.0;
  result[2] = m[2]*c[0] + m[6]*c[1] + m[10]*c[2] + m[14]*1.0;

  memcpy(c, result, (size_t)(3*sizeof(double)));

 return;
} /* ay_trafo_apply3 */


/* ay_trafo_apply3v:
 *  apply transformations in transformation matrix m[16] to
 *  vector c[clen*stride] (stride >= 3)
 */
void
ay_trafo_apply3v(double *c, unsigned int clen, unsigned int stride, double *m)
{
 double result[3];
 unsigned int i, j = 0;

  for(i = 0; i < clen; i++)
    {
      result[0] = m[0]*c[j] + m[4]*c[j+1] + m[8] *c[j+2] + m[12]*1.0;
      result[1] = m[1]*c[j] + m[5]*c[j+1] + m[9] *c[j+2] + m[13]*1.0;
      result[2] = m[2]*c[j] + m[6]*c[j+1] + m[10]*c[j+2] + m[14]*1.0;

      memcpy(&(c[j]), result, (size_t)(3*sizeof(double)));

      j += stride;
    }

 return;
} /* ay_trafo_apply3v */


/* ay_trafo_apply4:
 *  apply transformations in transformation matrix m[16] to point c[4]
 */
void
ay_trafo_apply4(double *c, double *m)
{
 double result[4] = {0};

  result[0] = m[0]*c[0] + m[4]*c[1] + m[8] *c[2] + m[12]*c[3];
  result[1] = m[1]*c[0] + m[5]*c[1] + m[9] *c[2] + m[13]*c[3];
  result[2] = m[2]*c[0] + m[6]*c[1] + m[10]*c[2] + m[14]*c[3];
  result[3] = m[3]*c[0] + m[7]*c[1] + m[11]*c[2] + m[15]*c[3];

  memcpy(c, result, (size_t)(4*sizeof(double)));

 return;
} /* ay_trafo_apply4 */


/* ay_trafo_apply4v:
 *  apply transformations in transformation matrix m[16] to
 *  vector c[clen*stride] (stride >= 4)
 */
void
ay_trafo_apply4v(double *c, unsigned int clen, unsigned int stride, double *m)
{
 double result[4];
 unsigned int i, j = 0;

  for(i = 0; i < clen; i++)
    {
      result[0] = m[0]*c[j] + m[4]*c[j+1] + m[8] *c[j+2] + m[12]*c[j+3];
      result[1] = m[1]*c[j] + m[5]*c[j+1] + m[9] *c[j+2] + m[13]*c[j+3];
      result[2] = m[2]*c[j] + m[6]*c[j+1] + m[10]*c[j+2] + m[14]*c[j+3];
      result[3] = m[3]*c[j] + m[7]*c[j+1] + m[11]*c[j+2] + m[15]*c[j+3];

      memcpy(&(c[j]), result, (size_t)(4*sizeof(double)));

      j += stride;
    }

 return;
} /* ay_trafo_apply4v */


/* ay_trafo_getall:
 *
 */
void
ay_trafo_getall(ay_list_object *lo)
{
 ay_object *o = NULL;
 double m[16];

  if(!lo)
    {
      return;
    }

  o = lo->object;

  if(!o)
    {
      return;
    }

  if(!o->inherit_trafos)
    {
      return;
    }

  if(lo->next)
    {
      ay_trafo_getall(lo->next->next);
    }

  if((o != ay_root) && o->down)
    {
      glTranslated((GLdouble)o->movx, (GLdouble)o->movy, (GLdouble)o->movz);
      ay_quat_torotmatrix(o->quat, m);
      glMultMatrixd((GLdouble *)m);
      glScaled((GLdouble)o->scalx, (GLdouble)o->scaly, (GLdouble)o->scalz);
    }

 return;
} /* ay_trafo_getall */


/* ay_trafo_getalli:
 *
 */
void
ay_trafo_getalli(ay_list_object *lo)
{
 ay_object *o = NULL;
 double euler[3] = {0};

  if(!lo)
    {
      return;
    }

  o = lo->object;

  if(!o)
    {
      return;
    }

  if(!o->inherit_trafos)
    {
      return;
    }

  if((o != ay_root) && o->down)
    {
      glScaled((GLdouble)(1.0/o->scalx), (GLdouble)(1.0/o->scaly),
	       (GLdouble)(1.0/o->scalz));

      ay_quat_toeuler(o->quat, euler);
      glRotated((GLdouble)AY_R2D(euler[0]), (GLdouble)0.0, (GLdouble)0.0,
		(GLdouble)1.0);
      glRotated((GLdouble)AY_R2D(euler[1]), (GLdouble)0.0, (GLdouble)1.0,
		(GLdouble)0.0);
      glRotated((GLdouble)AY_R2D(euler[2]), (GLdouble)1.0, (GLdouble)0.0,
		(GLdouble)0.0);

      glTranslated((GLdouble)-o->movx, (GLdouble)-o->movy, (GLdouble)-o->movz);
    }

  if(lo->next)
   {
     ay_trafo_getalli(lo->next->next);
   }

 return;
} /* ay_trafo_getalli */


/* ay_trafo_getalls:
 *
 */
void
ay_trafo_getalls(ay_list_object *lo)
{
 ay_object *o = NULL;

  if(!lo)
    {
      return;
    }

  o = lo->object;

  if(!o)
    {
      return;
    }

  if(!o->inherit_trafos)
    {
      return;
    }

  if(lo->next)
    {
      ay_trafo_getalls(lo->next->next);
    }

  if((o != ay_root) && o->down)
    {
      glScaled((GLdouble)o->scalx, (GLdouble)o->scaly, (GLdouble)o->scalz);
    }

 return;
} /* ay_trafo_getalls */


/* ay_trafo_getallis:
 *
 */
void
ay_trafo_getallis(ay_list_object *lo)
{
 ay_object *o = NULL;

  if(!lo)
    {
      return;
    }

  o = lo->object;

  if(!o)
    {
      return;
    }

  if(!o->inherit_trafos)
    {
      return;
    }

  if((o != ay_root) && o->down)
    {
      glScaled((GLdouble)(1.0/o->scalx), (GLdouble)(1.0/o->scaly),
	       (GLdouble)(1.0/o->scalz));
    }

  if(lo->next)
   {
     ay_trafo_getallis(lo->next->next);
   }

 return;
} /* ay_trafo_getallis */


/* ay_trafo_getallsr:
 *
 */
void
ay_trafo_getallsr(ay_list_object *lo)
{
 ay_object *o = NULL;
 double m[16] = {0};

  if(!lo)
    {
      return;
    }

  o = lo->object;

  if(!o)
    {
      return;
    }

  if(!o->inherit_trafos)
    {
      return;
    }

  if(lo->next)
   {
     ay_trafo_getallsr(lo->next->next);
   }

  if((o != ay_root) && o->down)
    {
      glScaled(o->scalx, o->scaly, o->scalz);
      ay_quat_torotmatrix(o->quat, m);
      glMultMatrixd((GLdouble *)m);
    }

 return;
} /* ay_trafo_getallsr */


/* ay_trafo_getallisr:
 *
 */
void
ay_trafo_getallisr(ay_list_object *lo)
{
 ay_object *o = NULL;
 double euler[3] = {0};

  if(!lo)
    {
      return;
    }

  o = lo->object;

  if(!o)
    {
      return;
    }

  if(!o->inherit_trafos)
    {
      return;
    }

  if((o != ay_root) && o->down)
    {
      glScaled((GLdouble)(1.0/o->scalx), (GLdouble)(1.0/o->scaly),
	       (GLdouble)(1.0/o->scalz));

      ay_quat_toeuler(o->quat, euler);
      glRotated((GLdouble)AY_R2D(euler[0]), (GLdouble)0.0, (GLdouble)0.0,
		(GLdouble)1.0);
      glRotated((GLdouble)AY_R2D(euler[1]), (GLdouble)0.0, (GLdouble)1.0,
		(GLdouble)0.0);
      glRotated((GLdouble)AY_R2D(euler[2]), (GLdouble)1.0, (GLdouble)0.0,
		(GLdouble)0.0);

    }

  if(lo->next)
   {
     ay_trafo_getallisr(lo->next->next);
   }

 return;
} /* ay_trafo_getallisr */


/* ay_trafo_getallr:
 *
 */
void
ay_trafo_getallr(ay_list_object *lo)
{
 ay_object *o = NULL;
 double m[16] = {0};

  if(!lo)
    {
      return;
    }

  o = lo->object;

  if(!o)
    {
      return;
    }

  if(!o->inherit_trafos)
    {
      return;
    }

  if(lo->next)
   {
     ay_trafo_getallr(lo->next->next);
   }

  if((o != ay_root) && o->down)
    {
      ay_quat_torotmatrix(o->quat, m);
      glMultMatrixd((GLdouble *)m);
    }

 return;
} /* ay_trafo_getallr */


/* ay_trafo_getallir:
 *
 */
void
ay_trafo_getallir(ay_list_object *lo)
{
 ay_object *o = NULL;
 double euler[3] = {0};

  if(!lo)
    {
      return;
    }

  o = lo->object;

  if(!o)
    {
      return;
    }

  if(!o->inherit_trafos)
    {
      return;
    }

  if((o != ay_root) && o->down)
    {
      ay_quat_toeuler(o->quat, euler);
      glRotated((GLdouble)AY_R2D(euler[0]), (GLdouble)0.0, (GLdouble)0.0,
		(GLdouble)1.0);
      glRotated((GLdouble)AY_R2D(euler[1]), (GLdouble)0.0, (GLdouble)1.0,
		(GLdouble)0.0);
      glRotated((GLdouble)AY_R2D(euler[2]), (GLdouble)1.0, (GLdouble)0.0,
		(GLdouble)0.0);
    }

  if(lo->next)
   {
     ay_trafo_getallir(lo->next->next);
   }

 return;
} /* ay_trafo_getallir */


/* ay_trafo_delegate:
 *  delegate all transformations to child objects of object <o>,
 *  resetting its own transformations
 */
int
ay_trafo_delegate(ay_object *o)
{
 ay_object *child = NULL;
 double euler[3] = {0};
 /*
 double quat[4] = {0};
 double xaxis[3] = {1.0, 0.0, 0.0};
 double yaxis[3] = {0.0, 1.0, 0.0};
 double zaxis[3] = {0.0, 0.0, 1.0};
 */
 double m[16] = {0}, v1[4] = {0};

  if(!o)
    {
      return AY_ENULL;
    }

  if(!o->down)
    {
      return AY_ERROR;
    }

  ay_quat_toeuler(o->quat, euler);

  child = o->down;
  while(child)
    {

      if((o->quat[0] != 0.0) || (o->quat[1] != 0.0) ||
	 (o->quat[2] != 0.0) || (o->quat[3] != 1.0))
	{
	  if((fabs(child->movx) > AY_EPSILON) ||
	     (fabs(child->movy) > AY_EPSILON) ||
	     (fabs(child->movz) > AY_EPSILON))
	    {

	      v1[0] = child->movx;
	      v1[1] = child->movy;
	      v1[1] = child->movy;

	      ay_quat_torotmatrix(o->quat, m);
	      ay_trafo_apply3(v1, m);

	      child->movx = v1[0];
	      child->movy = v1[1];
	      child->movz = v1[2];
	    } /* if */
	} /* if */

      child->movx += o->movx;
      child->movy += o->movy;
      child->movz += o->movz;

      child->scalx *= o->scalx;
      child->scaly *= o->scaly;
      child->scalz *= o->scalz;

      child->rotx -= AY_R2D(euler[2]);
      child->roty -= AY_R2D(euler[1]);
      child->rotz -= AY_R2D(euler[0]);
      /*
      ay_quat_axistoquat(xaxis, euler[2], quat);
      ay_quat_add(quat, child->quat, child->quat);

      ay_quat_axistoquat(yaxis, euler[1], quat);
      ay_quat_add(quat, child->quat, child->quat);

      ay_quat_axistoquat(zaxis, euler[0], quat);
      ay_quat_add(quat, child->quat, child->quat);
      */

      ay_quat_add(o->quat, child->quat, child->quat);

      child = child->next;
    } /* while */

  o->movx = 0.0;
  o->movy = 0.0;
  o->movz = 0.0;

  o->scalx = 1.0;
  o->scaly = 1.0;
  o->scalz = 1.0;

  o->rotx = 0.0;
  o->roty = 0.0;
  o->rotz = 0.0;

  o->quat[0] = 0.0;
  o->quat[1] = 0.0;
  o->quat[2] = 0.0;
  o->quat[3] = 1.0;

 return AY_OK;
} /* ay_trafo_delegate */


/* ay_trafo_delegatetcmd:
 *
 */
int
ay_trafo_delegatetcmd(ClientData clientData, Tcl_Interp *interp,
		      int argc, char *argv[])
{
 int ay_status = AY_OK;
 ay_list_object *sel = ay_selection;

  if(!sel)
    {
      ay_error(AY_ENOSEL, argv[0], NULL);
      return TCL_OK;
    }

  while(sel)
    {
      ay_status = ay_trafo_delegate(sel->object);
      if(ay_status)
	{
	  ay_error(ay_status, argv[0], NULL);
	  return TCL_OK;
	}

      sel = sel->next;
    } /* while */

 return TCL_OK;
} /* ay_trafo_delegatetcmd */


/* ay_trafo_applyall:
 *
 */
void
ay_trafo_applyall(ay_list_object *lo, ay_object *o, double *p)
{
 GLdouble m[16];
 double rm[16];

  if(!o || !p)
    {
      return;
    }

  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
   ay_trafo_getall(lo);

   glTranslated(o->movx, o->movy, o->movz);
   ay_quat_torotmatrix(o->quat, rm);
   glMultMatrixd((GLdouble *)rm);
   glScaled(o->scalx, o->scaly, o->scalz);
   glGetDoublev(GL_MODELVIEW_MATRIX, m);
  glPopMatrix();

  ay_trafo_apply3(p,m);

 return;
} /* ay_trafo_applyall */


/* ay_trafo_applyalli:
 *
 */
void
ay_trafo_applyalli(ay_list_object *lo, ay_object *o, double *p)
{
 GLdouble m[16];
 double euler[3] = {0};

  if(!o || !p)
    {
      return;
    }

  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
   glScaled(1.0/o->scalx, 1.0/o->scaly, 1.0/o->scalz);
   ay_quat_toeuler(o->quat, euler);
   glRotated((GLdouble)AY_R2D(euler[0]), (GLdouble)0.0, (GLdouble)0.0,
	     (GLdouble)1.0);
   glRotated((GLdouble)AY_R2D(euler[1]), (GLdouble)0.0, (GLdouble)1.0,
	     (GLdouble)0.0);
   glRotated((GLdouble)AY_R2D(euler[2]), (GLdouble)1.0, (GLdouble)0.0,
	     (GLdouble)0.0);
   glTranslated(-o->movx, -o->movy, -o->movz);
   ay_trafo_getalli(lo);
   glGetDoublev(GL_MODELVIEW_MATRIX, m);
  glPopMatrix();

  ay_trafo_apply3(p,m);

 return;
} /* ay_trafo_applyalli */


/* ay_trafo_copy:
 *
 */
void
ay_trafo_copy(ay_object *src, ay_object *dst)
{

  if(!src || !dst) 
    {
      return;
    }

  dst->movx = src->movx;
  dst->movy = src->movy;
  dst->movz = src->movz;

  dst->scalx = src->scalx;
  dst->scaly = src->scaly;
  dst->scalz = src->scalz;

  dst->rotx = src->rotx;
  dst->roty = src->roty;
  dst->rotz = src->rotz;

  memcpy(dst->quat, src->quat, 4*sizeof(double));

 return;
} /* ay_trafo_copy */


/* ay_trafo_add:
 *
 */
void
ay_trafo_add(ay_object *src, ay_object *dst)
{
 double euler[3];

  if(!src || !dst) 
    {
      return;
    }

  dst->movx += src->movx;
  dst->movy += src->movy;
  dst->movz += src->movz;

  dst->scalx *= src->scalx;
  dst->scaly *= src->scaly;
  dst->scalz *= src->scalz;

  ay_quat_add(src->quat, dst->quat, dst->quat);
  ay_quat_toeuler(dst->quat, euler);

  dst->rotx = AY_R2D(euler[0]);
  dst->roty = AY_R2D(euler[1]);
  dst->rotz = AY_R2D(euler[2]);

 return;
} /* ay_trafo_add */


/* ay_trafo_defaults:
 *
 */
void
ay_trafo_defaults(ay_object *o)
{

  if(!o)
    {
      return;
    }

  o->movx = 0.0;
  o->movy = 0.0;
  o->movz = 0.0;

  o->scalx = 1.0;
  o->scaly = 1.0;
  o->scalz = 1.0;

  o->rotx = 0.0;
  o->roty = 0.0;
  o->rotz = 0.0;

  o->quat[0] = 0.0;
  o->quat[1] = 0.0;
  o->quat[2] = 0.0;
  o->quat[3] = 1.0;

 return;
} /* ay_trafo_defaults */


/* ay_trafo_movobtcmd:
 *
 */
int
ay_trafo_movobtcmd(ClientData clientData, Tcl_Interp *interp,
		   int argc, char *argv[])
{
 int tcl_status = TCL_OK;
 double dx = 0, dy = 0, dz = 0;
 ay_list_object *sel = ay_selection;
 ay_object *o = NULL;

  if(argc != 4)
    {
      ay_error(AY_EARGS, argv[0], "%dx %dy %dz");
      return TCL_OK;
    }

  tcl_status = Tcl_GetDouble(interp, argv[1], &dx);
  AY_CHTCLERRRET(tcl_status, argv[0], interp);
  tcl_status = Tcl_GetDouble(interp, argv[2], &dy);
  AY_CHTCLERRRET(tcl_status, argv[0], interp);
  tcl_status = Tcl_GetDouble(interp, argv[3], &dz);
  AY_CHTCLERRRET(tcl_status, argv[0], interp);

  while(sel)
    {
      o = sel->object;

      o->movx += dx;
      o->movy += dy;
      o->movz += dz;
      o->modified = AY_TRUE;

      sel = sel->next;
    }

  ay_notify_parent();

 return TCL_OK;
} /* ay_trafo_movobtcmd */


/* ay_trafo_movpntstcmd:
 *
 */
int
ay_trafo_movpntstcmd(ClientData clientData, Tcl_Interp *interp,
		     int argc, char *argv[])
{
 int tcl_status = TCL_OK;
 double dx = 0, dy = 0, dz = 0;
 ay_list_object *sel = ay_selection;
 ay_object *o = NULL;
 ay_point *point = NULL;
 GLdouble mm[16];
 double tpoint[4] = {0};
 int notify_parent = AY_FALSE;

  if(argc != 4)
    {
      ay_error(AY_EARGS, argv[0], "%dx %dy %dz");
      return TCL_OK;
    }

  tcl_status = Tcl_GetDouble(interp, argv[1], &dx);
  AY_CHTCLERRRET(tcl_status, argv[0], interp);
  tcl_status = Tcl_GetDouble(interp, argv[2], &dy);
  AY_CHTCLERRRET(tcl_status, argv[0], interp);
  tcl_status = Tcl_GetDouble(interp, argv[3], &dz);
  AY_CHTCLERRRET(tcl_status, argv[0], interp);

  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
   glTranslated(dx,dy,dz);
   glGetDoublev(GL_MODELVIEW_MATRIX, mm);
  glPopMatrix();

  while(sel)
    {
      o = sel->object;

      if(o->selp)
	{
	  if(!o->selp->readonly)
	    {
	      point = o->selp;
	      while(point)
		{
		  AY_APTRAN3(tpoint,point->point,mm);
		  memcpy(point->point,tpoint,3*sizeof(double));

		  point = point->next;
		}
	      o->modified = AY_TRUE;
	      notify_parent = AY_TRUE;
	    } /* if */
	} /* if */

      sel = sel->next;
    }

  if(notify_parent)
    ay_notify_parent();

 return TCL_OK;
} /* ay_trafo_movpntstcmd */


/* ay_trafo_scalobtcmd:
 *
 */
int
ay_trafo_scalobtcmd(ClientData clientData, Tcl_Interp *interp,
		    int argc, char *argv[])
{
 int tcl_status = TCL_OK;
 double dx = 0, dy = 0, dz = 0;
 ay_list_object *sel = ay_selection;
 ay_object *o = NULL;

  if(argc != 4)
    {
      ay_error(AY_EARGS, argv[0], "%dx %dy %dz");
      return TCL_OK;
    }

  tcl_status = Tcl_GetDouble(interp, argv[1], &dx);
  AY_CHTCLERRRET(tcl_status, argv[0], interp);
  tcl_status = Tcl_GetDouble(interp, argv[2], &dy);
  AY_CHTCLERRRET(tcl_status, argv[0], interp);
  tcl_status = Tcl_GetDouble(interp, argv[3], &dz);
  AY_CHTCLERRRET(tcl_status, argv[0], interp);

  if(dx == 0.0)
    dx = 1.0;
  if(dy == 0.0)
    dy = 1.0;
  if(dz == 0.0)
    dz = 1.0;

  while(sel)
    {
      o = sel->object;

      o->scalx *= dx;
      o->scaly *= dy;
      o->scalz *= dz;
      o->modified = AY_TRUE;

      sel = sel->next;
    }

  ay_notify_parent();

 return TCL_OK;
} /* ay_trafo_scalobtcmd */


/* ay_trafo_scalpntstcmd:
 *
 */
int
ay_trafo_scalpntstcmd(ClientData clientData, Tcl_Interp *interp,
		      int argc, char *argv[])
{
 int tcl_status = TCL_OK;
 double dx = 0, dy = 0, dz = 0;
 ay_list_object *sel = ay_selection;
 ay_object *o = NULL;
 ay_point *point = NULL;
 GLdouble mm[16];
 double tpoint[4] = {0};
 int notify_parent = AY_FALSE;

  if(argc != 4)
    {
      ay_error(AY_EARGS, argv[0], "%dx %dy %dz");
      return TCL_OK;
    }

  tcl_status = Tcl_GetDouble(interp, argv[1], &dx);
  AY_CHTCLERRRET(tcl_status, argv[0], interp);
  tcl_status = Tcl_GetDouble(interp, argv[2], &dy);
  AY_CHTCLERRRET(tcl_status, argv[0], interp);
  tcl_status = Tcl_GetDouble(interp, argv[3], &dz);
  AY_CHTCLERRRET(tcl_status, argv[0], interp);

  if(dx == 0.0)
    dx = 1.0;
  if(dy == 0.0)
    dy = 1.0;
  if(dz == 0.0)
    dz = 1.0;

  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
   glScaled(dx, dy, dz);
   glGetDoublev(GL_MODELVIEW_MATRIX, mm);
  glPopMatrix();

  while(sel)
    {
      o = sel->object;

      if(o->selp)
	{
	  if(!o->selp->readonly)
	    {
	      point = o->selp;
	      while(point)
		{
		  AY_APTRAN3(tpoint, point->point, mm);
		  memcpy(point->point, tpoint, 3*sizeof(double));

		  point = point->next;
		}
	      o->modified = AY_TRUE;
	      notify_parent = AY_TRUE;
	    } /* if */
	} /* if */

      sel = sel->next;
    } /* while */

  if(notify_parent)
    ay_notify_parent();

 return TCL_OK;
} /* ay_trafo_scalpntstcmd */


/* ay_trafo_rotobtcmd:
 *
 */
int
ay_trafo_rotobtcmd(ClientData clientData, Tcl_Interp *interp,
		   int argc, char *argv[])
{
 int tcl_status = TCL_OK;
 double dx = 0, dy = 0, dz = 0;
 ay_list_object *sel = ay_selection;
 ay_object *o = NULL;
 double xaxis[3]={1.0,0.0,0.0};
 double yaxis[3]={0.0,1.0,0.0};
 double zaxis[3]={0.0,0.0,1.0};
 double quat[4];

  if(argc != 4)
    {
      ay_error(AY_EARGS, argv[0], "%dx %dy %dz");
      return TCL_OK;
    }

  tcl_status = Tcl_GetDouble(interp, argv[1], &dx);
  AY_CHTCLERRRET(tcl_status, argv[0], interp);
  tcl_status = Tcl_GetDouble(interp, argv[2], &dy);
  AY_CHTCLERRRET(tcl_status, argv[0], interp);
  tcl_status = Tcl_GetDouble(interp, argv[3], &dz);
  AY_CHTCLERRRET(tcl_status, argv[0], interp);

  while(sel)
    {
      o = sel->object;

      o->rotx += dx;
      if(dx != 0.0)
	{
	  ay_quat_axistoquat(xaxis, dx*AY_PI/180.0, quat);
	  ay_quat_add(quat, o->quat, o->quat);
	}
      o->roty += dy;
      if(dy != 0.0)
	{
	  ay_quat_axistoquat(yaxis, dy*AY_PI/180.0, quat);
	  ay_quat_add(quat, o->quat, o->quat);
	}
      o->rotz += dz;
      if(dz != 0.0)
	{
	  ay_quat_axistoquat(zaxis, dz*AY_PI/180.0, quat);
	  ay_quat_add(quat, o->quat, o->quat);
	}
      o->modified = AY_TRUE;

      sel = sel->next;
    } /* while */

  ay_notify_parent();

 return TCL_OK;
} /* ay_trafo_rotobtcmd */


/* ay_trafo_rotpntstcmd:
 *
 */
int
ay_trafo_rotpntstcmd(ClientData clientData, Tcl_Interp *interp,
		     int argc, char *argv[])
{
 int tcl_status = TCL_OK;
 double dx = 0, dy = 0, dz = 0;
 ay_list_object *sel = ay_selection;
 ay_object *o = NULL;
 ay_point *point = NULL;
 GLdouble mm[16];
 double tpoint[4] = {0};
 int notify_parent = AY_FALSE;

  if(argc != 4)
    {
      ay_error(AY_EARGS, argv[0], "%dx %dy %dz");
      return TCL_OK;
    }

  tcl_status = Tcl_GetDouble(interp, argv[1], &dx);
  AY_CHTCLERRRET(tcl_status, argv[0], interp);
  tcl_status = Tcl_GetDouble(interp, argv[2], &dy);
  AY_CHTCLERRRET(tcl_status, argv[0], interp);
  tcl_status = Tcl_GetDouble(interp, argv[3], &dz);
  AY_CHTCLERRRET(tcl_status, argv[0], interp);

  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
   glRotated(dx, 1.0, 0.0, 0.0);
   glRotated(dy, 0.0, 1.0, 0.0);
   glRotated(dz, 0.0, 0.0, 1.0);
   glGetDoublev(GL_MODELVIEW_MATRIX, mm);
  glPopMatrix();

  while(sel)
    {
      o = sel->object;

      if(o->selp)
	{
	  if(!o->selp->readonly)
	    {
	      point = o->selp;
	      while(point)
		{
		  AY_APTRAN3(tpoint, point->point, mm);
		  memcpy(point->point, tpoint, 3*sizeof(double));

		  point = point->next;
		}
	      o->modified = AY_TRUE;
	      notify_parent = AY_TRUE;
	    } /* if */
	} /* if */

      sel = sel->next;
    } /* while */

  if(notify_parent)
    ay_notify_parent();

 return TCL_OK;
} /* ay_trafo_rotpntstcmd */


/* ay_trafo_multmatrix4:
 *  multiply transformation matrices <m1> and <m2> (do M1.M2),
 *  put result into <m1>
 */
void
ay_trafo_multmatrix4(double *m1, double *m2)
{
 double mt[16] = {0}, t;
 int i, j, k;

  for(j = 0; j < 4; j++)
    {
      for(i = 0; i < 4; i++)
	{
	  t = 0.0;
	  for(k = 0; k < 4; k++)
	    {
	      t += m1[k*4+i] * m2[j*4+k];
	    }
	  mt[j*4+i] = t;
	}
    }

  memcpy(m1, mt, 16*sizeof(double));

 return;
} /* ay_trafo_multmatrix4 */


/* ay_trafo_invmatrix4:
 *  invert transformation matrix <m>, put result into <mi>;
 *  borrowed from Mesa3.2.1/matrix.c which in turn borrowed it
 *  from Graphics Gems II
 */
int
ay_trafo_invmatrix4(double *m, double *mi)
{
 double pos, neg, t;
 double det;

  if(!m || !mi)
    return AY_ENULL;

   /* Calculate the determinant of upper left 3x3 submatrix and
    * determine if the matrix is singular.
    */
   pos = neg = 0.0;
   t =  AY_M44(m,0,0) * AY_M44(m,1,1) * AY_M44(m,2,2);
   if (t >= 0.0) pos += t; else neg += t;

   t =  AY_M44(m,1,0) * AY_M44(m,2,1) * AY_M44(m,0,2);
   if (t >= 0.0) pos += t; else neg += t;

   t =  AY_M44(m,2,0) * AY_M44(m,0,1) * AY_M44(m,1,2);
   if (t >= 0.0) pos += t; else neg += t;

   t = -AY_M44(m,2,0) * AY_M44(m,1,1) * AY_M44(m,0,2);
   if (t >= 0.0) pos += t; else neg += t;

   t = -AY_M44(m,1,0) * AY_M44(m,0,1) * AY_M44(m,2,2);
   if (t >= 0.0) pos += t; else neg += t;

   t = -AY_M44(m,0,0) * AY_M44(m,2,1) * AY_M44(m,1,2);
   if (t >= 0.0) pos += t; else neg += t;

   det = pos + neg;

   if (det*det < 1e-25)
      return AY_ERROR;

   det = 1.0 / det;
   AY_M44(mi,0,0) = (  (AY_M44(m,1,1)*AY_M44(m,2,2) -
			AY_M44(m,2,1)*AY_M44(m,1,2) )*det);
   AY_M44(mi,0,1) = (- (AY_M44(m,0,1)*AY_M44(m,2,2) -
			AY_M44(m,2,1)*AY_M44(m,0,2) )*det);
   AY_M44(mi,0,2) = (  (AY_M44(m,0,1)*AY_M44(m,1,2) -
			AY_M44(m,1,1)*AY_M44(m,0,2) )*det);
   AY_M44(mi,1,0) = (- (AY_M44(m,1,0)*AY_M44(m,2,2) -
			AY_M44(m,2,0)*AY_M44(m,1,2) )*det);
   AY_M44(mi,1,1) = (  (AY_M44(m,0,0)*AY_M44(m,2,2) -
			AY_M44(m,2,0)*AY_M44(m,0,2) )*det);
   AY_M44(mi,1,2) = (- (AY_M44(m,0,0)*AY_M44(m,1,2) -
			AY_M44(m,1,0)*AY_M44(m,0,2) )*det);
   AY_M44(mi,2,0) = (  (AY_M44(m,1,0)*AY_M44(m,2,1) -
			AY_M44(m,2,0)*AY_M44(m,1,1) )*det);
   AY_M44(mi,2,1) = (- (AY_M44(m,0,0)*AY_M44(m,2,1) -
			AY_M44(m,2,0)*AY_M44(m,0,1) )*det);
   AY_M44(mi,2,2) = (  (AY_M44(m,0,0)*AY_M44(m,1,1) -
			AY_M44(m,1,0)*AY_M44(m,0,1) )*det);

   /* Do the translation part */
   AY_M44(mi,0,3) = - (AY_M44(m,0,3) * AY_M44(mi,0,0) +
		     AY_M44(m,1,3) * AY_M44(mi,0,1) +
		     AY_M44(m,2,3) * AY_M44(mi,0,2) );
   AY_M44(mi,1,3) = - (AY_M44(m,0,3) * AY_M44(mi,1,0) +
		     AY_M44(m,1,3) * AY_M44(mi,1,1) +
		     AY_M44(m,2,3) * AY_M44(mi,1,2) );
   AY_M44(mi,2,3) = - (AY_M44(m,0,3) * AY_M44(mi,2,0) +
		     AY_M44(m,1,3) * AY_M44(mi,2,1) +
		     AY_M44(m,2,3) * AY_M44(mi,2,2) );

   AY_M44(mi,3,3) = 1;

 return AY_OK;
} /* ay_trafo_invmatrix4 */



/* ay_trafo_creatematrix:
 *  initialize transformation matrix <m> from transformation
 *  attributes of <o>
 */
int
ay_trafo_creatematrix(ay_object *o, double *m)
{
 double *q, mr[16];

  if(!o || !m)
    return AY_ENULL;

  memset(m, 0, 16*sizeof(double));
  m[0] = 1.0;
  m[5] = 1.0;
  m[10] = 1.0;
  m[12] = o->movx;
  m[13] = o->movy;
  m[14] = o->movz;
  m[15] = 1.0;

  /* if(o->quat[4]!=1.0...)*/
  q = o->quat;
  memset(mr, 0, 16*sizeof(double));
  mr[0] = (1.0 - 2.0 * (q[1] * q[1] + q[2] * q[2]));
  mr[1] = 2.0 * (q[0] * q[1] - q[2] * q[3]);
  mr[2] = 2.0 * (q[2] * q[0] + q[1] * q[3]);

  mr[4] = 2.0 * (q[0] * q[1] + q[2] * q[3]);
  mr[5] = (1.0 - 2.0 * (q[2] * q[2] + q[0] * q[0]));
  mr[6] = 2.0 * (q[1] * q[2] - q[0] * q[3]);

  mr[8] = 2.0 * (q[2] * q[0] - q[1] * q[3]);
  mr[9] = 2.0 * (q[1] * q[2] + q[0] * q[3]);
  mr[10] = (1.0 - 2.0 * (q[1] * q[1] + q[0] * q[0]));
  mr[15] = 1.0;
  ay_trafo_multmatrix4(m, mr);

  m[0] *= o->scalx;
  m[1] *= o->scalx;
  m[2] *= o->scalx;
  m[3] *= o->scalx;
  m[4] *= o->scaly;
  m[5] *= o->scaly;
  m[6] *= o->scaly;
  m[7] *= o->scaly;
  m[8]  *= o->scalz;
  m[9]  *= o->scalz;
  m[10] *= o->scalz;
  m[11] *= o->scalz;

 return AY_OK;
} /* ay_trafo_creatematrix */


/* ay_trafo_decomposematrix:
 *  decompose transformation matrix <m> to transformation attributes of <o>;
 *  Note well: shear components are completely ignored;
 *  Matrix decomposition code borrowed from Graphics Gems II unmatrix.c.
 */
void
ay_trafo_decomposematrix(double *m, ay_object *o)
{
 double v1[3], v2[3], v3[3], v4[3];
 double sx, sy, sz;
 double rx, ry, rz;
 int i;
 double axis[3], quat[4] = {0};

  if(!m || !o)
    return;

  o->movx = 0.0;
  o->movy = 0.0;
  o->movz = 0.0;
  o->scalx = 1.0;
  o->scaly = 1.0;
  o->scalz = 1.0;
  o->quat[0] = 0.0;
  o->quat[1] = 0.0;
  o->quat[2] = 0.0;
  o->quat[3] = 1.0;
  o->rotx = 0.0;
  o->roty = 0.0;
  o->rotz = 0.0;

  quat[3] = 1.0;

  if(fabs(m[15]) <= AY_EPSILON)
    {
      return;
    }

  /* normalize matrix */
  for(i = 0; i < 16; i++)
    {
      m[i] /= m[15];
    }

  /* decompose matrix */

  /* get translation */
  o->movx = (double)m[12];
  o->movy = (double)m[13];
  o->movz = (double)m[14];

  /* get row vectors containing scale&rotation */
  v1[0] = (double)m[0];
  v1[1] = (double)m[1];
  v1[2] = (double)m[2];

  v2[0] = (double)m[4];
  v2[1] = (double)m[5];
  v2[2] = (double)m[6];

  v3[0] = (double)m[8];
  v3[1] = (double)m[9];
  v3[2] = (double)m[10];

  /* get scale */
  sx = AY_V3LEN(v1);
  sy = AY_V3LEN(v2);
  sz = AY_V3LEN(v3);

  /* normalize row vectors */
  if(fabs(sx) > AY_EPSILON)
    {
      o->scalx *= sx;
      AY_V3SCAL(v1, 1.0/sx);
    }
  if(fabs(sy) > AY_EPSILON)
    {
      o->scaly *= sy;
      AY_V3SCAL(v2, 1.0/sy);
    }
  if(fabs(sz) > AY_EPSILON)
    {
      o->scalz *= sz;
      AY_V3SCAL(v3, 1.0/sz);
    }

  /*
   * Check for a coordinate system flip.  If the determinant
   * is -1, then negate the matrix and the scaling factors.
   */
  AY_V3CROSS(v4, v2, v3)
  if(AY_V3DOT(v1, v4) < 0)
    {
      /*ay_error(AY_EWARN, fname, "Coordinate system flip detected!");*/

      o->scalx *= -1.0;
      o->scaly *= -1.0;
      o->scalz *= -1.0;

      for(i = 0; i < 3; i++)
	{
	  v1[i] *= -1;
	}
      for(i = 0; i < 3; i++)
	{
	  v2[i] *= -1;
	}
      for(i = 0; i < 3; i++)
	{
	  v3[i] *= -1;
	}
    } /* if */

  /* now get rotation */
  ry = asin(-v1[2]);
  if(cos(ry) != 0)
    {
      rx = atan2(v2[2], v3[2]);
      rz = atan2(v1[1], v1[0]);
    }
  else
    {
      rx = atan2(v2[0], v2[1]);
      rz = 0;
    }

  if(fabs(rx) > AY_EPSILON)
    {
      axis[0] = 1.0;
      axis[1] = 0.0;
      axis[2] = 0.0;
      quat[0] = 0.0;
      quat[1] = 0.0;
      quat[2] = 0.0;
      quat[3] = 1.0;
      ay_quat_axistoquat(axis, -rx, quat);
      ay_quat_add(quat, o->quat, o->quat);
      o->rotx = AY_R2D(rx);
    }

  if(fabs(ry) > AY_EPSILON)
    {
      axis[0] = 0.0;
      axis[1] = 1.0;
      axis[2] = 0.0;
      quat[0] = 0.0;
      quat[1] = 0.0;
      quat[2] = 0.0;
      quat[3] = 1.0;
      ay_quat_axistoquat(axis, -ry, quat);
      ay_quat_add(quat, o->quat, o->quat);
      o->roty = AY_R2D(ry);
    }

  if(fabs(rz) > AY_EPSILON)
    {
      axis[0] = 0.0;
      axis[1] = 0.0;
      axis[2] = 1.0;
      quat[0] = 0.0;
      quat[1] = 0.0;
      quat[2] = 0.0;
      quat[3] = 1.0;
      ay_quat_axistoquat(axis, -rz, quat);
      ay_quat_add(quat, o->quat, o->quat);
      o->rotz = AY_R2D(rz);
    }

 return;
} /* ay_trafo_decomposematrix */


/* ay_trafo_identitymatrix:
 *  initialize transformation matrix <m> to identity
 */
void
ay_trafo_identitymatrix(double *m)
{

  memset(m, 0, 16*sizeof(double));
  m[0] = 1.0;
  m[5] = 1.0;
  m[10] = 1.0;
  m[15] = 1.0;

 return;
} /* ay_trafo_identitymatrix */


/* ay_trafo_translatematrix:
 *  add a translation to transformation matrix <m>
 */
void
ay_trafo_translatematrix(double x, double y, double z, double *m)
{

  m[12] = m[0] * x + m[4] * y + m[8]  * z + m[12];
  m[13] = m[1] * x + m[5] * y + m[9]  * z + m[13];
  m[14] = m[2] * x + m[6] * y + m[10] * z + m[14];
  m[15] = m[3] * x + m[7] * y + m[11] * z + m[15];

 return;
} /* ay_trafo_translatematrix */


/* ay_trafo_scalematrix:
 *  add a scale transformation to transformation matrix m
 */
void
ay_trafo_scalematrix(double x, double y, double z, double *m)
{

  m[0] *= x;   m[4] *= y;   m[8]  *= z;
  m[1] *= x;   m[5] *= y;   m[9]  *= z;
  m[2] *= x;   m[6] *= y;   m[10] *= z;
  m[3] *= x;   m[7] *= y;   m[11] *= z;

 return;
} /* ay_trafo_scalematrix */


/* ay_trafo_rotatematrix:
 *  add a rotation of angle <angle> (expressed in degrees)
 *  about axis <x>,<y>,<z> to transformation matrix <m>;
 *  code taken from Mesa (Erich Boleyn (erich@uruk.org))
 */
void
ay_trafo_rotatematrix(double angle, double x, double y, double z, double *m)
{
 double t[16], mag, s, c;
 double xx, yy, zz, xy, yz, zx, xs, ys, zs, one_c;

   s = sin(AY_D2R(angle));
   c = cos(AY_D2R(angle));

   mag = sqrt( x*x + y*y + z*z );

   if (mag /*== 0.0*/ < AY_EPSILON) {
      /* do nothing */
     return;
   }

   x /= mag;
   y /= mag;
   z /= mag;

   /*
    *  Arbitrary axis rotation matrix.
    *
    *  This is composed of 5 matrices, Rz, Ry, T, Ry', Rz', multiplied
    *  like so:  Rz * Ry * T * Ry' * Rz'.  T is the final rotation
    *  (which is about the X-axis), and the two composite transforms
    *  Ry' * Rz' and Rz * Ry are (respectively) the rotations necessary
    *  from the arbitrary axis to the X-axis then back.  They are
    *  all elementary rotations.
    *
    *  Rz' is a rotation about the Z-axis, to bring the axis vector
    *  into the x-z plane.  Then Ry' is applied, rotating about the
    *  Y-axis to bring the axis vector parallel with the X-axis.  The
    *  rotation about the X-axis is then performed.  Ry and Rz are
    *  simply the respective inverse transforms to bring the arbitrary
    *  axis back to it's original orientation.  The first transforms
    *  Rz' and Ry' are considered inverses, since the data from the
    *  arbitrary axis gives you info on how to get to it, not how
    *  to get away from it, and an inverse must be applied.
    *
    *
    *  The basic calculation used is to recognize that the arbitrary
    *  axis vector (x, y, z), since it is of unit length, actually
    *  represents the sines and cosines of the angles to rotate the
    *  X-axis to the same orientation, with theta being the angle about
    *  Z and phi the angle about Y (in the order described above)
    *  as follows:
    *
    *  cos ( theta ) = x / sqrt ( 1 - z^2 )
    *  sin ( theta ) = y / sqrt ( 1 - z^2 )
    *
    *  cos ( phi ) = sqrt ( 1 - z^2 )
    *  sin ( phi ) = z
    *
    *  Note that cos ( phi ) can further be inserted to the above
    *  formulas:
    *
    *  cos ( theta ) = x / cos ( phi )
    *  sin ( theta ) = y / sin ( phi )
    *
    *
    *  ...etc.  Because of those relations and the standard trigonometric
    *  relations, it is possible to reduce the transforms down to what
    *  is used below.  It may be that any primary axis chosen will give the
    *  same results (modulo a sign convention) using this method.
    *
    *  Particularly nice is to notice that all divisions that might
    *  have caused trouble when parallel to certain planes or
    *  axis go away with care paid to reducing the expressions.
    *  After checking, it does perform correctly under all cases, since
    *  in all the cases of division where the denominator would have
    *  been zero, the numerator would have been zero as well, giving
    *  the expected result.
    */

   xx = x * x;
   yy = y * y;
   zz = z * z;
   xy = x * y;
   yz = y * z;
   zx = z * x;
   xs = x * s;
   ys = y * s;
   zs = z * s;
   one_c = 1.0 - c;

   AY_M44(t,0,0) = (one_c * xx) + c;
   AY_M44(t,0,1) = (one_c * xy) - zs;
   AY_M44(t,0,2) = (one_c * zx) + ys;
   AY_M44(t,0,3) = 0.0;

   AY_M44(t,1,0) = (one_c * xy) + zs;
   AY_M44(t,1,1) = (one_c * yy) + c;
   AY_M44(t,1,2) = (one_c * yz) - xs;
   AY_M44(t,1,3) = 0.0;

   AY_M44(t,2,0) = (one_c * zx) - ys;
   AY_M44(t,2,1) = (one_c * yz) + xs;
   AY_M44(t,2,2) = (one_c * zz) + c;
   AY_M44(t,2,3) = 0.0;

   AY_M44(t,3,0) = 0.0;
   AY_M44(t,3,1) = 0.0;
   AY_M44(t,3,2) = 0.0;
   AY_M44(t,3,3) = 1.0;

   ay_trafo_multmatrix4(m, t);

 return;
} /* ay_trafo_rotatematrix */


/* ay_trafo_pointstoplane:
 *  convert 3 points to plane equation coefficients
 */
void
ay_trafo_pointstoplane(double x1, double y1, double z1,
		       double x2, double y2, double z2,
		       double x3, double y3, double z3,
		       double *A, double *B, double *C, double *D)
{

  *A = y1 * (z2 - z3) + y2 * (z3 - z1) + y3 * (z1 - z2);
  *B = z1 * (x2 - x3) + z2 * (x3 - x1) + z3 * (x1 - x2);
  *C = x1 * (y2 - y3) + x2 * (y3 - y1) + x3 * (y1 - y2);
  *D = -(x1 * (y2 * z3 - y3 * z2) + x2 * (y3 * z1 - y1 * z3) +
	 x3 * (y1 * z2 - y2 * z1));

 return;
} /* ay_trafo_pointstoplane */


/* ay_trafo_scalecog:
 *
 */
void
ay_trafo_scalecog(double scale, double *cv, int len, int stride)
{
 int i;
 double *p, cog[3] = {0};

  if(!cv)
    return;

  p = cv;
  for(i = 0; i < len; i++)
    {
      cog[0] += (p[0]/len);
      cog[1] += (p[1]/len);
      cog[2] += (p[2]/len);

      p += stride;
    } /* for */

  p = cv;
  for(i = 0; i < len; i++)
    {
      p[0] -= cog[0];
      p[1] -= cog[1];
      p[2] -= cog[2];

      p[0] *= scale;
      p[1] *= scale;
      p[2] *= scale;

      p[0] += cog[0];
      p[1] += cog[1];
      p[2] += cog[2];

      p += stride;
    } /* for */

 return;
} /* ay_trafo_scalecog */

