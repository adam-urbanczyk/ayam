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

/* selp.c - selected points related functions */

/* ay_selp_clear:
 *  clear list of selected points from object o
 */
void
ay_selp_clear(ay_object *o)
{
 ay_point_object *next = NULL, *p = NULL;

  if(!o)
    return;

  p = o->selp;

  while(p)
    {
      next = p->next;
      free(p);
      p = next;
    }

  o->selp = NULL;

 return;
} /* ay_selp_clear */


/* ay_selp_selall:
 *  select (tag) all points
 */
int
ay_selp_selall(ay_object *o)
{
 char fname[] = "selp_selall";
 int ay_status = AY_OK;
 double p[3] = {DBL_MIN, DBL_MIN, DBL_MIN};
 int i = 0;
 ay_point_object *newp = NULL;

  if(!o)
    return AY_ENULL;

  ay_selp_clear(o);
  ay_status = ay_pact_getpoint(0, o, p);

  for(i = 0; i < ay_point_edit_coords_number; i++)
    {
      newp = NULL;
      if(!(newp = calloc(1, sizeof(ay_point_object))))
	{
	  ay_error(AY_EOMEM, fname, NULL);
	  return AY_ERROR;
	}

      newp->next = o->selp;
      o->selp = newp;
      newp->point = ay_point_edit_coords[i];
      newp->homogenous = ay_point_edit_coords_homogenous;
    } /* for */

 return AY_OK;
} /* ay_selp_selall */


/* ay_selp_selalltcmd:
 *  select all editable points of selected objects
 */
int
ay_selp_selalltcmd(ClientData clientData, Tcl_Interp *interp,
		   int argc, char *argv[])
{
 char fname[] = "selAllPoints";
 int ay_status = AY_OK;
 ay_object *o = NULL;
 ay_list_object *sel = ay_selection;


  while(sel)
    {
      o = sel->object;

      ay_status = ay_selp_selall(o);

      if(ay_status)
	{
	  ay_error(AY_ERROR, fname, NULL);
	  return TCL_OK;
	}

      sel = sel->next;
    } /* while */

 return TCL_OK;
} /* ay_selp_selalltcmd */


/* ay_selp_applytrafotcmd:
 *  apply transformation to editable points of selected objects
 */
int
ay_selp_applytrafotcmd(ClientData clientData, Tcl_Interp *interp,
		       int argc, char *argv[])
{
 char fname[] = "applyTrafo";
 int ay_status = AY_OK;
 ay_object *o = NULL;
 ay_list_object *sel = ay_selection;
 ay_point_object *bak = NULL, *p = NULL;
 int all = AY_FALSE;
 double m[16];

 if(argc < 2)
   {
     ay_error(AY_EARGS, fname, "sel|all");
     return TCL_OK;
   }

 if(!strcmp(argv[1],"all"))
   all = AY_TRUE;

  while(sel)
    {
      o = sel->object;

      if(all)
	{
	  bak = o->selp;
	  o->selp = NULL;
	  ay_status = ay_selp_selall(o);
	}

      /* create trafo */
      glPushMatrix();
       glTranslated((GLdouble)o->movx, (GLdouble)o->movy, (GLdouble)o->movz);
       ay_quat_torotmatrix(o->quat, m);
       glMultMatrixd((GLdouble*)m);
       glScaled((GLdouble)o->scalx, (GLdouble)o->scaly, (GLdouble)o->scalz);
       glGetDoublev(GL_MODELVIEW_MATRIX, (GLdouble*)m);
      glPopMatrix();

      /* transform points */
      p = o->selp;
      while(p)
	{
	  if(p->homogenous)
	    ay_trafo_apply4(p->point, m);
	  else
	    ay_trafo_apply3(p->point, m);

	  p = p->next;
	}

      /* clear trafo */
      o->movx = 0.0;
      o->movy = 0.0;
      o->movz = 0.0;

      o->rotx = 0.0;
      o->roty = 0.0;
      o->rotz = 0.0;

      o->scalx = 1.0;
      o->scaly = 1.0;
      o->scalz = 1.0;


      o->quat[0] = 0.0;
      o->quat[1] = 0.0;
      o->quat[2] = 0.0;
      o->quat[3] = 1.0;

      /* restore old selected points */
      if(all)
	{
	  ay_selp_clear(o);
	  o->selp = bak;
	}

      sel = sel->next;
    } /* while */

 return TCL_OK;
} /* ay_selp_applytrafotcmd */


/* ay_selp_invert:
 *  invert list of selected points for object o
 */
int
ay_selp_invert(ay_object *o)
{
 int ay_status = AY_OK;
 ay_point_object *t = NULL, *p = NULL, **last2 = NULL, *p2 = NULL;

  if(!o)
    return AY_ENULL;

  if(!o->selp)
    {
      ay_status = ay_selp_selall(o);
      return ay_status;
    }

  /* save old selection of points to p */
  p = o->selp;

  /* select all points */
  o->selp = NULL;
  ay_status = ay_selp_selall(o);
  if(ay_status)
    return ay_status;

  /* remove all points, that are in p (old selection), from p2 (new
     selection); additionally, clear p */
  while(p)
    {
      last2 = &(o->selp);
      p2 = o->selp;
      while(p2)
	{
	  if(p->point == p2->point)
	    {
	      *last2 = p2->next;
	      free(p2);
	      /* XXXX should be unneeded, because of the break below: */
	      p2 = *last2;
	      break;
	    }
	  else
	    {
	      last2 = &(p2->next);
	      p2 = p2->next;
	    } /* if */
	} /* while */
      t = p;
      p = p->next;
      free(t);
    } /* while */

 return AY_OK;
} /* ay_selp_invert */


/* ay_selp_inverttcmd:
 *  invert selection of editable points of selected objects
 */
int
ay_selp_inverttcmd(ClientData clientData, Tcl_Interp *interp,
		   int argc, char *argv[])
{
 char fname[] = "invSelPoints";
 int ay_status = AY_OK;
 ay_object *o = NULL;
 ay_list_object *sel = ay_selection;

  while(sel)
    {
      o = sel->object;

      ay_status = ay_selp_invert(o);

      if(ay_status)
	{
	  ay_error(AY_ERROR, fname, NULL);
	  return TCL_OK;
	}

      sel = sel->next;
    } /* while */

 return TCL_OK;
} /* ay_selp_inverttcmd */
