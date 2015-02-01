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


/** ay_trafo_getparent:
 *  Accumulate all parent transformations starting from specified
 *  level up to ay_root (unless a parent stops inheritance of the
 *  transformation attributes).
 *
 * \param[in] lo current level
 * \param[in,out] tm transformation matrix (double[16]) to process
 */
void
ay_trafo_getparent(ay_list_object *lo, double *tm)
{
 ay_object *o = NULL;
 double m[16];

  if(!lo || !tm)
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
      ay_trafo_getparent(lo->next->next, tm);
    }

  if((o != ay_root) && o->down && AY_ISTRAFO(o))
    {
      ay_trafo_translatematrix(o->movx, o->movy, o->movz, tm);
      ay_quat_torotmatrix(o->quat, m);
      ay_trafo_multmatrix(tm, m);
      ay_trafo_scalematrix(o->scalx, o->scaly, o->scalz, tm);
    }

 return;
} /* ay_trafo_getparent */


/** ay_trafo_getparentinv:
 *  Accumulate all inverse parent transformations starting from specified
 *  level up to ay_root (unless a parent stops inheritance of the
 *  transformation attributes).
 *
 * \param[in] lo current level
 * \param[in,out] tm transformation matrix (double[16]) to process
 */
void
ay_trafo_getparentinv(ay_list_object *lo, double *tm)
{
 ay_object *o = NULL;
 double quat[4], m[16];

  if(!lo || !tm)
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

  if((o != ay_root) && o->down && AY_ISTRAFO(o))
    {
      ay_trafo_scalematrix(1.0/o->scalx, 1.0/o->scaly, 1.0/o->scalz, tm);
      memcpy(quat, o->quat, 4*sizeof(double));
      ay_quat_inv(quat);
      ay_quat_torotmatrix(quat, m);
      ay_trafo_multmatrix(tm, m);
      ay_trafo_translatematrix(-o->movx, -o->movy, -o->movz, tm);
    }

  if(lo->next)
   {
     ay_trafo_getparentinv(lo->next->next, tm);
   }

 return;
} /* ay_trafo_getparentinv */


/** ay_trafo_getsomeparent:
 *  Accumulate some parent transformations starting from specified
 *  level up to ay_root (unless a parent stops inheritance of the
 *  transformation attributes).
 *
 * \param[in] lo current level
 * \param[in] what specifies which transformation attributes to get
 * \param[in,out] tm transformation matrix (double[16]) to process
 */
void
ay_trafo_getsomeparent(ay_list_object *lo, int what, double *tm)
{
 ay_object *o = NULL;
 double m[16];

  if(!lo || !tm)
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
      ay_trafo_getsomeparent(lo->next->next, what, tm);
    }

  if((o != ay_root) && o->down && AY_ISTRAFO(o))
    {
      if(what & AY_MOV)
	ay_trafo_translatematrix(o->movx, o->movy, o->movz, tm);
      if(what & AY_ROT)
	{
	  ay_quat_torotmatrix(o->quat, m);
	  ay_trafo_multmatrix(tm, m);
	}
      if(what & AY_SCA)
	ay_trafo_scalematrix(o->scalx, o->scaly, o->scalz, tm);
    }

 return;
} /* ay_trafo_getsomeparent */


/* ay_trafo_getsomeparentinv:
 *  Accumulate some inverse parent transformations starting from specified
 *  level up to ay_root (unless a parent stops inheritance of the
 *  transformation attributes).
 *
 * \param[in] lo current level
 * \param[in] what specifies which transformation attributes to get
 * \param[in,out] tm transformation matrix (double[16]) to process
 */
void
ay_trafo_getsomeparentinv(ay_list_object *lo, int what, double *tm)
{
 ay_object *o = NULL;
 double quat[4], m[16];

  if(!lo || !tm)
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

  if((o != ay_root) && o->down && AY_ISTRAFO(o))
    {
      if(what & AY_SCA)
	ay_trafo_scalematrix(1.0/o->scalx, 1.0/o->scaly, 1.0/o->scalz, tm);
      if(what & AY_ROT)
	{
	  memcpy(quat, o->quat, 4*sizeof(double));
	  ay_quat_inv(quat);
	  ay_quat_torotmatrix(quat, m);
	  ay_trafo_multmatrix(tm, m);
	}
      if(what & AY_MOV)
	ay_trafo_translatematrix(-o->movx, -o->movy, -o->movz, tm);
    }

  if(lo->next)
   {
     ay_trafo_getsomeparentinv(lo->next->next, what, tm);
   }

 return;
} /* ay_trafo_getsomeparentinv */


/* ay_trafo_concatparent:
 *
 */
void
ay_trafo_concatparent(ay_list_object *lo)
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
      ay_trafo_concatparent(lo->next->next);
    }

  if((o != ay_root) && o->down)
    {
      glTranslated((GLdouble)o->movx, (GLdouble)o->movy, (GLdouble)o->movz);
      ay_quat_torotmatrix(o->quat, m);
      glMultMatrixd((GLdouble *)m);
      glScaled((GLdouble)o->scalx, (GLdouble)o->scaly, (GLdouble)o->scalz);
    }

 return;
} /* ay_trafo_concatparent */


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
      return AY_EWARN;
    }

  ay_quat_toeuler(o->quat, euler);

  child = o->down;
  while(child)
    {
      if(child == ay_endlevel)
	{
	  return AY_OK;
	}

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
 *  Delegate transformations of selected objects to their
 *  respective child objects.
 *  Implements the \a delegTrafo scripting interface command.
 *  See also the corresponding section in the \ayd{scdelegtrafo}.
 *
 *  \returns TCL_OK in any case.
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
      if(ay_status > 1)
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
 double tm[16], m[16];

  if(!p)
    {
      return;
    }

  ay_trafo_identitymatrix(tm);
  if(lo && lo->object != ay_root)
    ay_trafo_getparent(lo, tm);
  if(o)
    {
      ay_trafo_translatematrix(o->movx, o->movy, o->movz, tm);
      ay_quat_torotmatrix(o->quat, m);
      ay_trafo_multmatrix(tm, m);
      ay_trafo_scalematrix(o->scalx, o->scaly, o->scalz, tm);
    }
  ay_trafo_apply3(p,tm);

 return;
} /* ay_trafo_applyall */


/* ay_trafo_applyalli:
 *
 */
void
ay_trafo_applyalli(ay_list_object *lo, ay_object *o, double *p)
{
 double tm[16], m[16], quat[4];

  if(!p)
    {
      return;
    }

  ay_trafo_identitymatrix(tm);
  if(o)
    {
      ay_trafo_scalematrix(1.0/o->scalx, 1.0/o->scaly, 1.0/o->scalz, tm);
      memcpy(quat, o->quat, 4*sizeof(double));
      ay_quat_inv(quat);
      ay_quat_torotmatrix(quat, m);
      ay_trafo_multmatrix(tm, m);
      ay_trafo_translatematrix(-o->movx, -o->movy, -o->movz, tm);
    }
  if(lo && lo->object != ay_root)
    ay_trafo_getparentinv(lo, tm);

  ay_trafo_apply3(p,tm);

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
 *  Translate selected objects.
 *  Implements the \a movOb scripting interface command.
 *  See also the corresponding section in the \ayd{scmovob}.
 *
 *  \returns TCL_OK in any case.
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
 *  Translate selected points.
 *  Implements the \a movPnts scripting interface command.
 *  See also the corresponding section in the \ayd{scmovpnts}.
 *
 *  \returns TCL_OK in any case.
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
 *  Scale selected objects.
 *  Implements the \a scalOb scripting interface command.
 *  See also the corresponding section in the \ayd{scscalob}.
 *
 *  \returns TCL_OK in any case.
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
 *  Scale selected points.
 *  Implements the \a scalPnts scripting interface command.
 *  See also the corresponding section in the \ayd{scscalpnts}.
 *
 *  \returns TCL_OK in any case.
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
 *  Rotate selected objects.
 *  Implements the \a rotOb scripting interface command.
 *  See also the corresponding section in the \ayd{scrotob}.
 *
 *  \returns TCL_OK in any case.
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
 *  Rotate selected points.
 *  Implements the \a rotPnts scripting interface command.
 *  See also the corresponding section in the \ayd{scrotpnts}.
 *
 *  \returns TCL_OK in any case.
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


/* ay_trafo_multmatrix:
 *  multiply transformation matrices <m1> and <m2> (do M1.M2),
 *  put result into <m1>
 */
void
ay_trafo_multmatrix(double *m1, double *m2)
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
} /* ay_trafo_multmatrix */


/* ay_trafo_invmatrix:
 *  invert _transformation_ matrix \a m, put result into <mi>;
 *  borrowed from Mesa3.2.1/matrix.c which in turn borrowed it
 *  from Graphics Gems II;
 *  with additional writes to allow working with completely
 *  uninitialized mi matrices
 */
int
ay_trafo_invmatrix(double *m, double *mi)
{
 double pos, neg, t;
 double det;

  if(!m || !mi)
    return AY_ENULL;

   /* Calculate the determinant of upper left 3x3 submatrix and
    * determine if the matrix is singular.
    */
   pos = neg = 0.0;
   t = AY_M44(m,0,0) * AY_M44(m,1,1) * AY_M44(m,2,2);
   if (t >= 0.0) pos += t; else neg += t;

   t = AY_M44(m,1,0) * AY_M44(m,2,1) * AY_M44(m,0,2);
   if (t >= 0.0) pos += t; else neg += t;

   t = AY_M44(m,2,0) * AY_M44(m,0,1) * AY_M44(m,1,2);
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

   AY_M44(mi,0,3) = 0;

   AY_M44(mi,1,0) = (- (AY_M44(m,1,0)*AY_M44(m,2,2) -
			AY_M44(m,2,0)*AY_M44(m,1,2) )*det);
   AY_M44(mi,1,1) = (  (AY_M44(m,0,0)*AY_M44(m,2,2) -
			AY_M44(m,2,0)*AY_M44(m,0,2) )*det);
   AY_M44(mi,1,2) = (- (AY_M44(m,0,0)*AY_M44(m,1,2) -
			AY_M44(m,1,0)*AY_M44(m,0,2) )*det);

   AY_M44(mi,1,3) = 0;

   AY_M44(mi,2,0) = (  (AY_M44(m,1,0)*AY_M44(m,2,1) -
			AY_M44(m,2,0)*AY_M44(m,1,1) )*det);
   AY_M44(mi,2,1) = (- (AY_M44(m,0,0)*AY_M44(m,2,1) -
			AY_M44(m,2,0)*AY_M44(m,0,1) )*det);
   AY_M44(mi,2,2) = (  (AY_M44(m,0,0)*AY_M44(m,1,1) -
			AY_M44(m,1,0)*AY_M44(m,0,1) )*det);

   AY_M44(mi,2,3) = 0;

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
} /* ay_trafo_invmatrix */


/* ay_trafo_invgenmatrix:
 *  invert _generic_ matrix \a m, put result into <mi>;
 *  code borrowed from Mesa/SGI GLU
 */
int
ay_trafo_invgenmatrix(double *m, double *mi)
{
 double inv[16], det;
 int i;

  if(!m || !mi)
    return AY_ENULL;

  inv[0] = m[5]  * m[10] * m[15] -
    m[5]  * m[11] * m[14] -
    m[9]  * m[6]  * m[15] +
    m[9]  * m[7]  * m[14] +
    m[13] * m[6]  * m[11] -
    m[13] * m[7]  * m[10];

  inv[4] = -m[4]  * m[10] * m[15] +
    m[4]  * m[11] * m[14] +
    m[8]  * m[6]  * m[15] -
    m[8]  * m[7]  * m[14] -
    m[12] * m[6]  * m[11] +
    m[12] * m[7]  * m[10];

  inv[8] = m[4]  * m[9] * m[15] -
    m[4]  * m[11] * m[13] -
    m[8]  * m[5] * m[15] +
    m[8]  * m[7] * m[13] +
    m[12] * m[5] * m[11] -
    m[12] * m[7] * m[9];

  inv[12] = -m[4]  * m[9] * m[14] +
    m[4]  * m[10] * m[13] +
    m[8]  * m[5] * m[14] -
    m[8]  * m[6] * m[13] -
    m[12] * m[5] * m[10] +
    m[12] * m[6] * m[9];

  inv[1] = -m[1]  * m[10] * m[15] +
    m[1]  * m[11] * m[14] +
    m[9]  * m[2] * m[15] -
    m[9]  * m[3] * m[14] -
    m[13] * m[2] * m[11] +
    m[13] * m[3] * m[10];

  inv[5] = m[0]  * m[10] * m[15] -
    m[0]  * m[11] * m[14] -
    m[8]  * m[2] * m[15] +
    m[8]  * m[3] * m[14] +
    m[12] * m[2] * m[11] -
    m[12] * m[3] * m[10];

  inv[9] = -m[0]  * m[9] * m[15] +
    m[0]  * m[11] * m[13] +
    m[8]  * m[1] * m[15] -
    m[8]  * m[3] * m[13] -
    m[12] * m[1] * m[11] +
    m[12] * m[3] * m[9];

  inv[13] = m[0]  * m[9] * m[14] -
    m[0]  * m[10] * m[13] -
    m[8]  * m[1] * m[14] +
    m[8]  * m[2] * m[13] +
    m[12] * m[1] * m[10] -
    m[12] * m[2] * m[9];

  inv[2] = m[1]  * m[6] * m[15] -
    m[1]  * m[7] * m[14] -
    m[5]  * m[2] * m[15] +
    m[5]  * m[3] * m[14] +
    m[13] * m[2] * m[7] -
    m[13] * m[3] * m[6];

  inv[6] = -m[0]  * m[6] * m[15] +
    m[0]  * m[7] * m[14] +
    m[4]  * m[2] * m[15] -
    m[4]  * m[3] * m[14] -
    m[12] * m[2] * m[7] +
    m[12] * m[3] * m[6];

  inv[10] = m[0]  * m[5] * m[15] -
    m[0]  * m[7] * m[13] -
    m[4]  * m[1] * m[15] +
    m[4]  * m[3] * m[13] +
    m[12] * m[1] * m[7] -
    m[12] * m[3] * m[5];

  inv[14] = -m[0]  * m[5] * m[14] +
    m[0]  * m[6] * m[13] +
    m[4]  * m[1] * m[14] -
    m[4]  * m[2] * m[13] -
    m[12] * m[1] * m[6] +
    m[12] * m[2] * m[5];

  inv[3] = -m[1] * m[6] * m[11] +
    m[1] * m[7] * m[10] +
    m[5] * m[2] * m[11] -
    m[5] * m[3] * m[10] -
    m[9] * m[2] * m[7] +
    m[9] * m[3] * m[6];

  inv[7] = m[0] * m[6] * m[11] -
    m[0] * m[7] * m[10] -
    m[4] * m[2] * m[11] +
    m[4] * m[3] * m[10] +
    m[8] * m[2] * m[7] -
    m[8] * m[3] * m[6];

  inv[11] = -m[0] * m[5] * m[11] +
    m[0] * m[7] * m[9] +
    m[4] * m[1] * m[11] -
    m[4] * m[3] * m[9] -
    m[8] * m[1] * m[7] +
    m[8] * m[3] * m[5];

  inv[15] = m[0] * m[5] * m[10] -
    m[0] * m[6] * m[9] -
    m[4] * m[1] * m[10] +
    m[4] * m[2] * m[9] +
    m[8] * m[1] * m[6] -
    m[8] * m[2] * m[5];

  det = m[0] * inv[0] + m[1] * inv[4] + m[2] * inv[8] + m[3] * inv[12];

  if(det*det < 1e-25)
    return AY_ERROR;

  det = 1.0 / det;

  for(i = 0; i < 16; i++)
    {
      mi[i] = inv[i] * det;
    }

 return AY_OK;
} /* ay_trafo_invgenmatrix */


/* ay_trafo_creatematrix:
 *  initialize transformation matrix \a m from transformation
 *  attributes of <o>
 */
void
ay_trafo_creatematrix(ay_object *o, double *m)
{
 double *q, mr[16];

  if(!o || !m)
    return;

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
  ay_trafo_multmatrix(m, mr);

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

 return;
} /* ay_trafo_creatematrix */


/* ay_trafo_decomposematrix:
 *  decompose transformation matrix \a m to transformation attributes of <o>;
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


/** ay_trafo_identitymatrix:
 *  initialize transformation matrix \a m to identity
 *
 * \param[in,out] m matrix to process
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


/** ay_trafo_translatematrix:
 *  add a translation to the transformation matrix \a m
 *
 * \param[in] x translation in x
 * \param[in] y translation in y
 * \param[in] z translation in z
 * \param[in,out] m matrix to process
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


/** ay_trafo_scalematrix:
 *  add a scale transformation to transformation matrix \a m
 *
 * \param[in] x scale factor
 * \param[in] y scale factor
 * \param[in] z scale factor
 * \param[in,out] m matrix to process
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


/** ay_trafo_rotatematrix:
 *  add a rotation to the transformation matrix \a m;
 *  code taken from Mesa (Erich Boleyn (erich@uruk.org))
 *
 * \param[in] angle rotation angle (in degrees)
 * \param[in] x x component of rotation axis
 * \param[in] y y component of rotation axis
 * \param[in] z z component of rotation axis
 * \param[in,out] m matrix to process
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
    *  like so: Rz * Ry * T * Ry' * Rz'. T is the final rotation
    *  (which is about the X-axis), and the two composite transforms
    *  Ry' * Rz' and Rz * Ry are (respectively) the rotations necessary
    *  from the arbitrary axis to the X-axis then back. They are
    *  all elementary rotations.
    *
    *  Rz' is a rotation about the Z-axis, to bring the axis vector
    *  into the x-z plane. Then Ry' is applied, rotating about the
    *  Y-axis to bring the axis vector parallel with the X-axis. The
    *  rotation about the X-axis is then performed. Ry and Rz are
    *  simply the respective inverse transforms to bring the arbitrary
    *  axis back to it's original orientation. The first transforms
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
    *  is used below. It may be that any primary axis chosen will give the
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

   ay_trafo_multmatrix(m, t);

 return;
} /* ay_trafo_rotatematrix */


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


/** ay_trafo_normalize:
 *  Normalize transformation attributes.
 *
 * \param[in,out] o object to process
 * \param[in] digits number of significant figures to keep
 */
void
ay_trafo_normalize(ay_object *o, int digits)
{
 double euler[3];

  if(!o)
    return;

  if(digits <= 0)
    return;

  o->movx = ay_trafo_round(o->movx, digits);
  o->movy = ay_trafo_round(o->movy, digits);
  o->movz = ay_trafo_round(o->movz, digits);

  o->scalx = ay_trafo_round(o->scalx, digits);
  o->scaly = ay_trafo_round(o->scaly, digits);
  o->scalz = ay_trafo_round(o->scalz, digits);

  if(fabs(o->quat[0]) < pow(10, -digits))
    o->quat[0] = 0.0;
  if(fabs(o->quat[1]) < pow(10, -digits))
    o->quat[1] = 0.0;
  if(fabs(o->quat[2]) < pow(10, -digits))
    o->quat[2] = 0.0;
  if(fabs(o->quat[3]) < pow(10, -digits))
    o->quat[3] = 0.0;

  ay_quat_toeuler(o->quat, euler);

  o->rotx = AY_R2D(euler[0]);
  o->roty = AY_R2D(euler[1]);
  o->rotz = AY_R2D(euler[2]);

  o->rotx = ay_trafo_round(o->rotx, digits);
  o->roty = ay_trafo_round(o->roty, digits);
  o->rotz = ay_trafo_round(o->rotz, digits);

 return;
} /* ay_trafo_normalize */


/** ay_trafo_normalizetcmd:
 *  Normalize transformation attributes of selected objects.
 *
 *  Implements the \a normTrafos scripting interface command.
 *  Also implements the \a normPnts scripting interface command.
 *  See also the corresponding section in the \ayd{scnormtrafo}.
 *
 *  \returns TCL_OK in any case.
 */
int
ay_trafo_normalizetcmd(ClientData clientData, Tcl_Interp *interp,
		       int argc, char *argv[])
{
 ay_list_object *sel = ay_selection;
 int pnts = AY_FALSE;

  if(!sel)
    {
      ay_error(AY_ENOSEL, argv[0], NULL);
      return TCL_OK;
    }

  if(argv[0][4] == 'P')
    pnts = AY_TRUE;

  while(sel)
    {
      if(pnts)
	{
	  ay_selp_normalize(sel->object, ay_prefs.normalizedigits);
	}
      else
	{
	  ay_trafo_normalize(sel->object, ay_prefs.normalizedigits);
	}
      sel = sel->next;
    } /* while */

 return TCL_OK;
} /* ay_trafo_normalizetcmd */


/** ay_trafo_round:
 * Rounds double value to specified number of significant figures.
 * Always rounds away from zero, so -2.6 to 1 sig fig will become -3.0.
 *
 * \param[in] value double to normalize
 * \param[in] digits number of digits (should be in the range 1 - 15)
 *
 * \returns normalized value
*/
double ay_trafo_round(double value, int digits)
{
 double a, b;
 long int i;
 int neg = 0;

  if(!value)
    return 0.0;

  if(value < 0.0)
    {
      value = -value;
      neg = 1;
    }

  if(value < pow(10, -digits))
    return 0.0;

  i = digits - log10(value);

  if(i)
    {
      a = pow(10.0, (double)i);
    }
  else
    {
      a = 1.0;
    }

  b = value * a;
  i = b + 0.5;
  value = i / a;

 return neg ? -value : value;
} /* ay_trafo_round */


/** ay_trafo_getall:
 * Concatenates all parent transformations and object transformations
 * onto a given matrix.
 *
 * \param[in] lo current level
 * \param[in] o object
 * \param[in, out] tm pointer to transformation matrix (double[16]) to fill
 */
void
ay_trafo_getall(ay_list_object *lo, ay_object *o, double *tm)
{
 double m[16];

  if(lo && lo->object != ay_root)
    {
      ay_trafo_getparent(lo->next, tm);
    }
  ay_trafo_translatematrix(o->movx, o->movy, o->movz, tm);
  ay_quat_torotmatrix(o->quat, m);
  ay_trafo_multmatrix(tm, m);
  ay_trafo_scalematrix(o->scalx, o->scaly, o->scalz, tm);

 return;
} /* ay_trafo_getall */
