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

/* objsel.c - select objects on a viewport */


/* ay_objsel_process_hits: process datas returned by the selection mode of
 * OpenGL in order to find which objects have been selected
 */
void
ay_objsel_process_hits (GLint hits, GLuint buffer[], char *var)
{
  char fname[] = "objsel_process_hits";
  char *node = NULL, *tmp = NULL;
  int i = 0, size = 0;
  GLint *ptr = NULL;

  ptr = (GLint *) buffer;

  /* Allocate memory to store temporary datas */
  tmp = (char*) malloc (256 * sizeof (char));
  if (!tmp)
    {
      ay_error (AY_EOMEM, fname, NULL);
      return;
    }

  /* Allocate memory to store the node address */
  node = (char*) malloc (256 * sizeof (char));
  if (!node)
    {
      free (tmp);
      ay_error (AY_EOMEM, fname, NULL);
      return;
    }
  size = 256;
  node[0] = 0;
  
  for (i = 0; i < hits; i++)
    {
      ay_object *o = NULL;
      GLuint object_name = 0;
      int n = 0;
      unsigned int j = 0;
      GLuint name;
      float z1 = 0.0f, z2 = 0.0f;

      name = *(ptr++);
      z1 = (float) *(ptr++) / 0x7fffffff;
      z2 = (float) *(ptr++) / 0x7fffffff;
      /*printf("%f %f\n", z1, z2);*/
      o = ay_root;
      strncat (node, "root", size);

      for (j = 0; j < name; j++)
	{
	  n = 0;
	  object_name = *(ptr++);

	  /* Skip the root object from the list */
	  if (object_name == ay_root->glname) {
	    ptr += name - j - 1;
	    break;
	  }

	  if (object_name)
	    {
	      while (o->next)
		{
		  if (o->glname == object_name)
		    break;

		  o = o->next;
		  n++;
		}

	      /* If we have reached the end of the current level
	       * (i.e. o->next == NULL) then we must verify that the object
	       * is the one we are looking for else we leave...
	       */
	      if (o->glname != object_name)
	        {
		  o = NULL;
		  node[0] = 0;
		  break;
		}

	      /* Goes down one level */
	      if ((j != name - 1) && (o->down))
		o = o->down;

	      /* Stores the level number */
	      sprintf (tmp, ":%d", n);

	      /* Allocate more memory to node if needed */
	      if ((strlen (node) + strlen (tmp) + 10) > size)
		{
		  size += 256;
		  node = (char*) realloc (node, size * sizeof (char));
		  if (!node)
		    {
		      free (node);
		      free (tmp);
		      ay_error (AY_EOMEM, fname, NULL);
		      return;
		    }
		}

	      /* Adds the level number to the node */
	      strncat (node, tmp, size);
	    } /* if */
	} /* for */

      strncat(node, " ", size);
      
      /* If OpenGL found that the grid or the coordinate system are in the
       * selection then a stand-alone "root " (i.e. an empty node) is generated
       * because the grid and the coord. system are not AYAM objects. When this
       * happens we have to remove this extra "root ".
       */
      if (strlen(node) >= 5)
        {
          if (!strncmp (&node[strlen(node) - 5], "root ", 5))
	    node[strlen(node) - 5] = 0;
	}
    } /* for */

  /* Remove the space at the end of 'node' */
  if (node[strlen(node) - 1] == ' ')
    node[strlen(node) - 1] = 0;

  /* Verify that 'tmp' is big enough to store the Tcl command */
  if ((strlen (node) + 15) > 256)
    {
      tmp = (char*) realloc (tmp, (strlen (node) + 15) * sizeof (char));
      if (!tmp)
	{
	  free (node);
	  free (tmp);
	  ay_error (AY_EOMEM, fname, NULL);
	  return;
	}
    }

  /* Store the result in the Tcl variable : node */
  sprintf (tmp, "set %s \"%s\"", var, node);
  Tcl_Eval (ay_interp, tmp);

  free (node);
  free (tmp);
} /* ay_objsel_process_hits */


/* ay_objsel_processcb:
 *
 */
int
ay_objsel_processcb (struct Togl *togl, int argc, char *argv[])
{
 ay_view_object *view = (ay_view_object *) Togl_GetClientData (togl);
 /* char fname[] = "objsel_process"; */
 ay_object *o = ay_root;
 int ay_status = AY_OK;
 int width = Togl_Width (togl);
 int height = Togl_Height (togl);
 GLdouble aspect = ((GLdouble) width) / ((GLdouble) height);
 Tcl_Interp *interp = ay_interp;
 double x1 = 0.0, y1 = 0.0, x2 = 0.0, y2 = 0.0;
 double x = 0.0, y = 0.0, boxw = 0.0, boxh = 0.0;
 GLuint selectBuf[1024];
 GLint hits;
 GLint viewport[4];

  Tcl_GetDouble (interp, argv[3], &x1);
  Tcl_GetDouble (interp, argv[4], &y1);
  
  if (argc == 7)
    {
      Tcl_GetDouble (interp, argv[5], &x2);
      Tcl_GetDouble (interp, argv[6], &y2);
      
      x = (x1 + x2) / 2.0;
      y = (y1 + y2) / 2.0;
      boxh = abs(y2 - y1);
      boxw = abs(x2 - x1);
    }
  else
    {
     Tcl_Obj *toa = NULL, *ton = NULL, *to = NULL;
     char *part1 = "ayprefs", *part2 = "PickTolerance";
     double tolerance = 0.0;
      
      toa = Tcl_NewStringObj(part1, -1);
      ton = Tcl_NewStringObj(part2, -1);
      to = Tcl_ObjGetVar2(interp, toa, ton,
		TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

      tolerance = atof(Tcl_GetStringFromObj(to, NULL));
      x = x1;
      y = y1;
      boxh = tolerance;
      boxw = tolerance;
      
      Tcl_IncrRefCount(toa); Tcl_DecrRefCount(toa);
      Tcl_IncrRefCount(ton); Tcl_DecrRefCount(ton);
    }

  glGetIntegerv (GL_VIEWPORT, viewport);

  glSelectBuffer (1024, selectBuf);
  glRenderMode (GL_SELECT);

  glInitNames ();
  glPushName (0);

  glMatrixMode (GL_PROJECTION);
  glPushMatrix ();
  glLoadIdentity ();
  gluPickMatrix (x, (GLdouble) (viewport[3] - y), boxw, boxh, viewport);

  /* Setup projection code from viewt.c */
  if (view->type == AY_VTPERSP)
    glFrustum (-aspect * view->zoom, aspect * view->zoom,
	       -1.0 * view->zoom, 1.0 * view->zoom, 1, 1000.0);
  else
    glOrtho (-aspect * view->zoom, aspect * view->zoom,
	     -1.0 * view->zoom, 1.0 * view->zoom, -100.0, 100.0);

  if (view->roll != 0.0)
    glRotated (view->roll, 0.0, 0.0, 1.0);
  if (view->type != AY_VTTOP)
    gluLookAt (view->from[0], view->from[1], view->from[2],
	       view->to[0], view->to[1], view->to[2],
	       view->up[0], view->up[1], view->up[2]);
  else
    gluLookAt (view->from[0], view->from[1], view->from[2],
	       view->to[0], view->to[1], view->to[2],
	       view->up[0], view->up[1], view->up[2]);

  glMatrixMode (GL_MODELVIEW);

  ay_current_glname = 0;

  if(!view->shade)
    {
      while(o->next)
	{
	  ay_status = ay_draw_object(togl, o, 2);
	  o = o->next;
	}
    }
  else
    {
      glDisable(GL_LIGHTING);
      while(o->next)
	{
	  ay_status = ay_shade_object(togl, o, AY_TRUE);
	  o = o->next;
	}
      glEnable(GL_LIGHTING);
    }
    
  glMatrixMode (GL_PROJECTION);
  glPopMatrix ();
  glFinish ();

  hits = glRenderMode (GL_RENDER);

  ay_objsel_process_hits (hits, selectBuf, argv[2]);

  return TCL_OK;
} /* ay_objsel_processcb */


/* ay_objsel_getnmfrmndtcmd:
 * returns the name of an object designated by its node (in tree format)
 */
int
ay_objsel_getnmfrmndtcmd(ClientData clientData, Tcl_Interp *interp,
		  int argc, char *argv[])
{
 char fname[] = "objsel_getnmfrmnd";
 ay_object *o = NULL;
 Tcl_DString ds;
 
  if (argc != 3)
    {
      ay_error(AY_EARGS, fname, "varname string");
      return TCL_OK;
    }
  
  o = ay_tree_getobject(argv[2]);
  
  Tcl_DStringInit(&ds);
  Tcl_DStringAppend(&ds, ay_object_getname(o), -1);
  
  if ((o->name) || (o->type == AY_IDINSTANCE))
    {
      if (ay_prefs.list_types)
        {
	  Tcl_DStringAppend(&ds, "(", -1);
	  Tcl_DStringAppend(&ds, ay_object_gettypename(o->type), -1);
	  Tcl_DStringAppend(&ds, ")", -1);
	}
    }
  
  Tcl_SetVar(interp, argv[1], Tcl_DStringValue(&ds), TCL_LEAVE_ERR_MSG);
  Tcl_DStringFree(&ds);

  return TCL_OK;
}
