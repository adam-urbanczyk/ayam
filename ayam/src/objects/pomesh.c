/*
 * Ayam, a free 3D modeler for the RenderMan interface.
 *
 * Ayam is copyrighted 1998-2006 by Randolf Schultz
 * (randolf.schultz@gmail.com) and others.
 *
 * All rights reserved.
 *
 * See the file License for details.
 *
 */

#include "ayam.h"

/* pomesh.c - PolyMesh object */

static char *ay_pomesh_name = "PolyMesh";

/* functions: */

/* ay_pomesh_createcb:
 *  create callback function of pomesh object
 */
int
ay_pomesh_createcb(int argc, char *argv[], ay_object *o)
{
 int ay_status = AY_OK;
 int tcl_status = TCL_OK;
 char fname[] = "pomesh_createcb";
 char option_handled = AY_FALSE;
 char **av;
 int avlen;
 int optnum = 0, i = 2, j = 0;
 unsigned int ui = 0, uj = 0, uk = 0, npolys = 0, tmpui = 0;
 unsigned int *nloops = NULL, *nverts = NULL, *verts = NULL;
 unsigned int nloopslen = 0, nvertslen = 0, vertslen = 0;
 unsigned int totalverts = 0, controlvlen = 0;
 double *controlv = NULL;
 int vnormals = AY_FALSE, stride = 3;
 ay_pomesh_object *pomesh = NULL;

 /*
   -p 1 -cv {0 0 0  1 0 0  0 1 0}
   -p 1 -l {2} -cv {0 0 0  1 0 0  0 1 0  .25 .25 0  .5 .25 0  .25 .5 0}
 */

  /* parse args */
  while(i < argc)
    {
      if(i+1 >= argc)
	{
	  ay_error(AY_EOPT, fname, argv[i]);
	  ay_status = AY_ERROR;
	  goto cleanup;
	}

      tcl_status = TCL_OK;
      ay_status = AY_OK;
      option_handled = AY_FALSE;
      optnum = i;
      if(argv[i] && argv[i][0] != '\0')
	{
	  switch(argv[i][1])
	    {
	    case 'p':
	      /* -polys */
	      ay_status = ay_tcmd_getuint(argv[i+1], &npolys);
	      option_handled = AY_TRUE;
	      break;
	    case 'l':
	      /* -loops */
	      if(Tcl_SplitList(ay_interp, argv[i+1], &avlen, &av) == TCL_OK)
		{
		  if(nloops)
		    {
		      free(nloops);
		    }
		  if(!(nloops = calloc(avlen, sizeof(unsigned int))))
		    {
		      Tcl_Free((char *) av);
		      ay_status = AY_EOMEM;
		      goto cleanup;
		    }
		  for(j = 0; j < avlen; j++)
		    {
		      ay_status = ay_tcmd_getuint(av[j], &tmpui);
		      if(ay_status != AY_OK)
			{
			  break;
			}
		      nloops[j] = tmpui;
		    } /* for */
		  nloopslen = avlen;
		  Tcl_Free((char *) av);
		}
	      option_handled = AY_TRUE;
	      break;
	    case 'n':
	      /* -nverts */
	      if(Tcl_SplitList(ay_interp, argv[i+1], &avlen, &av) == TCL_OK)
		{
		  if(nverts)
		    {
		      free(nverts);
		    }
		  if(!(nverts = calloc(avlen, sizeof(unsigned int))))
		    {
		      Tcl_Free((char *) av);
		      ay_status = AY_EOMEM;
		      goto cleanup;
		    }

		  for(j = 0; j < avlen; j++)
		    {
		      ay_status = ay_tcmd_getuint(av[j], &tmpui);
		      if(ay_status != AY_OK)
			{
			  break;
			}
		      nverts[j] = tmpui;
		    } /* for */
		  nvertslen = avlen;
		  Tcl_Free((char *) av);
		}
	      option_handled = AY_TRUE;
	      break;
	    case 'i':
	      /* -iverts */
	      if(Tcl_SplitList(ay_interp, argv[i+1], &avlen, &av) == TCL_OK)
		{
		  if(verts)
		    {
		      free(verts);
		    }
		  if(!(verts = calloc(avlen, sizeof(unsigned int))))
		    {
		      Tcl_Free((char *) av);
		      ay_status = AY_EOMEM;
		      goto cleanup;
		    }
		  for(j = 0; j < avlen; j++)
		    {
		      ay_status = ay_tcmd_getuint(av[j], &tmpui);
		      if(ay_status != AY_OK)
			{
			  break;
			}
		      verts[j] = tmpui;
		    } /* for */
		  vertslen = avlen;
		  Tcl_Free((char *) av);
		}
	      option_handled = AY_TRUE;
	      break;
	    case 'c':
	      switch(argv[i][2])
		{
		case 'n':
		  /* -cn */
		  if(controlv)
		    {
		      free(controlv);
		      controlv = NULL;
		    }
		  tcl_status = ay_tcmd_convdlist(argv[i+1], &avlen, &controlv);
		  option_handled = AY_TRUE;
		  break;
		case 'v':
		  /* -cv */
		  if(Tcl_SplitList(ay_interp, argv[i+1], &avlen, &av) == TCL_OK)
		    {
		      if(controlv)
			{
			  free(controlv);
			}
		      if(!(controlv = calloc(avlen, sizeof(double))))
			{
			  Tcl_Free((char *) av);
			  ay_status = AY_EOMEM;
			  goto cleanup;
			}
		      for(j = 0; j < avlen; j++)
			{
			  tcl_status = Tcl_GetDouble(ay_interp,
						     av[j], &controlv[j]);
			  if(tcl_status != TCL_OK)
			    {
			      break;
			    }
			} /* for */
		      controlvlen = avlen;
		      Tcl_Free((char *) av);
		    }
		  option_handled = AY_TRUE;
		  break;
		default:
		  break;
		} /* switch */
	      break;
	    case 'v':
	      /* -vnormals */
	      tcl_status = Tcl_GetInt(ay_interp, argv[i+1], &vnormals);
	      option_handled = AY_TRUE;
	      break;
	    default:
	      break;
	    } /* switch */

	  if(option_handled &&
	     ((tcl_status != TCL_OK) || (ay_status != AY_OK)))
	    {
	      ay_error(AY_EOPT, fname, argv[i]);
	      ay_status = AY_ERROR;
	      goto cleanup;
	    }

	  i += 2;
	}
      else
	{
	  i++;
	} /* if */

      if(!option_handled)
	{
	  ay_error(AY_EUOPT, fname, argv[optnum]);
	  ay_status = AY_ERROR;
	  goto cleanup;
	}

    } /* while */

  if(vnormals)
    {
      stride = 6;
    }

  controlvlen /= stride;

  if(npolys > 0)
    {
      /* see if we can automatically fill some arrays */
      if(!nloops)
	{
	  if(!(nloops = calloc(npolys, sizeof(unsigned int))))
	    {
	      ay_status = AY_EOMEM;
	      goto cleanup;
	    }
	  for(ui = 0; ui < npolys; ui++)
	    {
	      nloops[ui] = 1;
	    } /* for */
	  nloopslen = npolys;
	} /* if */

      if(!nverts)
	{
	  uj = 0;
	  for(ui = 0; ui < npolys; ui++)
	    {
	      uj += nloops[ui];
	    }
	  if(!(nverts = calloc(uj, sizeof(unsigned int))))
	    {
	      ay_status = AY_EOMEM;
	      goto cleanup;
	    }
	  for(ui = 0; ui < uj; ui++)
	    {
	      nverts[ui] = 3;
	    } /* for */
	  nvertslen = uj;
	} /* if */

      if(!verts)
	{
	  uk = 0;
	  for(ui = 0; ui < npolys; ui++)
	    {
	      for(uj = 0; uj < nloops[ui]; uj++)
		{
		  totalverts += nverts[uk];
		  uk++;
		}
	    }
	  if(!(verts = calloc(totalverts, sizeof(unsigned int))))
	    {
	      ay_status = AY_EOMEM;
	      goto cleanup;
	    }
	  for(ui = 0; ui < totalverts; ui++)
	    {
	      verts[ui] = ui;
	    } /* for */
	  vertslen = totalverts;
	} /* if */

      if(!controlv)
	{
	  totalverts = 0;
	  uk = 0;
	  for(ui = 0; ui < npolys; ui++)
	    {
	      for(uj = 0; uj < nloops[ui]; uj++)
		{
		  totalverts += nverts[uk];
		  uk++;
		}
	    }
	  if(!(controlv = calloc(totalverts, stride*sizeof(double))))
	    {
	      ay_status = AY_EOMEM;
	      goto cleanup;
	    }
#if 0
	  uj = 0;
	  for(ui = 0; ui < totalverts; ui++)
	    {
	      controlv[uj]   = 0.0;
	      controlv[uj+1] = 0.0;
	      controlv[uj+2] = 0.0;

	      if(vnormals)
		{
		  controlv[uj+5] = 1.0;
		}

	      uj += stride;
	    } /* for */
#endif
	  controlvlen = totalverts;
	} /* if */

    } /* if */

  if(!(pomesh = calloc(1, sizeof(ay_pomesh_object))))
    {
      ay_status = AY_EOMEM;
      goto cleanup;
    }

  if(npolys > 0)
    {
      /* check the parameters */
      if(nloopslen < npolys)
	{
	  ay_error(AY_ERROR, fname, "nloops < npolys");
	  ay_status = AY_ERROR;
	  goto cleanup;
	}
      uj = 0;
      for(ui = 0; ui < npolys; ui++)
	{
	  uj += nloops[ui];
	}
      if(nvertslen < uj)
	{
	  ay_error(AY_ERROR, fname, "nverts < sum(nloops)");
	  ay_status = AY_ERROR;
	  goto cleanup;
	}
      totalverts = 0;
      uk = 0;
      for(ui = 0; ui < npolys; ui++)
	{
	  for(uj = 0; uj < nloops[ui]; uj++)
	    {
	      if(nverts[uk] < 3)
		{
		  ay_error(AY_ERROR, fname,
			   "unsupported number of vertices per loop (<3)");
		  ay_status = AY_ERROR;
		  goto cleanup;
		}
	      totalverts += nverts[uk];
	      uk++;
	    }
	}
      if(vertslen < totalverts)
	{
	  ay_error(AY_ERROR, fname, "verts < sum(nverts)");
	  ay_status = AY_ERROR;
	  goto cleanup;
	}
      for(ui = 0; ui < vertslen; ui++)
	{
	  if(verts[ui] >= controlvlen)
	    {
	      ay_error(AY_ERROR, fname,
	       "vertex index out of bounds (or not enough control points)");
	      ay_status = AY_ERROR;
	      goto cleanup;
	    }
	}
      pomesh->npolys = npolys;
      pomesh->nloops = nloops;
      pomesh->nverts = nverts;
      pomesh->verts = verts;
      pomesh->controlv = controlv;
      pomesh->ncontrols = controlvlen;
      if(vnormals)
	{
	  pomesh->has_normals = AY_TRUE;
	}

      /* prevent cleanup code from doing something harmful */
      nloops = NULL;
      nverts = NULL;
      verts = NULL;
      controlv = NULL;
    } /* if(npolys > 0) */

  o->refine = (void *)pomesh;
  pomesh = NULL;

cleanup:

  if(nloops)
    free(nloops);

  if(nverts)
    free(nverts);

  if(verts)
    free(verts);

  if(controlv)
    free(controlv);

  if(pomesh)
    free(pomesh);

  if(ay_status == AY_EOMEM)
    {
      ay_error(AY_EOMEM, fname, NULL);
      ay_status = AY_ERROR;
    }

 return ay_status;
} /* ay_pomesh_createcb */


/* ay_pomesh_deletecb:
 *  delete callback function of pomesh object
 */
int
ay_pomesh_deletecb(void *c)
{
 ay_pomesh_object *pomesh = NULL;

  if(!c)
    return AY_ENULL;

  pomesh = (ay_pomesh_object *)(c);

  /* free nloops */
  if(pomesh->nloops)
    free(pomesh->nloops);

  /* free nverts */
  if(pomesh->nverts)
    free(pomesh->nverts);

  /* free verts */
  if(pomesh->verts)
    free(pomesh->verts);

  /* free controlv */
  if(pomesh->controlv)
    free(pomesh->controlv);

  /* free normals */
  if(pomesh->face_normals)
    free(pomesh->face_normals);

  free(pomesh);

 return AY_OK;
} /* ay_pomesh_deletecb */


/* ay_pomesh_copycb:
 *  copy callback function of pomesh object
 */
int
ay_pomesh_copycb(void *src, void **dst)
{
 int ay_status = AY_OK;
 ay_pomesh_object *pomesh = NULL, *pomeshsrc = NULL;
 unsigned int i, total_loops = 0, total_verts = 0;
 int stride = 0;

  if(!src || !dst)
    return AY_ENULL;

  pomeshsrc = (ay_pomesh_object *)src;

  if(pomeshsrc->has_normals)
    stride = 6;
  else
    stride = 3;

  if(!(pomesh = malloc(sizeof(ay_pomesh_object))))
    return AY_EOMEM;

  memcpy(pomesh, src, sizeof(ay_pomesh_object));

  pomesh->nloops = NULL;
  pomesh->nverts = NULL;
  pomesh->verts = NULL;
  pomesh->controlv = NULL;
  pomesh->face_normals = NULL;

  /* copy nloops */
  if(pomeshsrc->npolys && pomeshsrc->nloops)
    {
      if(!(pomesh->nloops = malloc(pomeshsrc->npolys * sizeof(unsigned int))))
	{
	  ay_status = AY_EOMEM;
	  goto cleanup;
	}
      memcpy(pomesh->nloops, pomeshsrc->nloops,
	     pomesh->npolys * sizeof(unsigned int));

      for(i = 0; i < pomeshsrc->npolys; i++)
	{
	  total_loops += pomeshsrc->nloops[i];
	} /* for */
    } /* if */

  /* copy nverts */
  if(pomeshsrc->nverts && total_loops)
    {
      if(!(pomesh->nverts = malloc(total_loops * sizeof(unsigned int))))
	{
	  ay_status = AY_EOMEM;
	  goto cleanup;
	}
      memcpy(pomesh->nverts, pomeshsrc->nverts,
	     total_loops * sizeof(unsigned int));

      for(i = 0; i < total_loops; i++)
	{
	  total_verts += pomeshsrc->nverts[i];
	} /* for */
    } /* if */

  /* copy verts */
  if(pomeshsrc->verts && total_verts)
    {
      if(!(pomesh->verts = malloc(total_verts * sizeof(unsigned int))))
	{
	  ay_status = AY_EOMEM;
	  goto cleanup;
	}
      memcpy(pomesh->verts, pomeshsrc->verts,
	     total_verts * sizeof(unsigned int));
    }

  /* copy controlv */
  if(pomeshsrc->controlv)
    {
      if(!(pomesh->controlv = malloc(stride * pomeshsrc->ncontrols *
				     sizeof(double))))
	{
	  ay_status = AY_EOMEM;
	  goto cleanup;
	}
      memcpy(pomesh->controlv, pomeshsrc->controlv,
	     stride * pomesh->ncontrols * sizeof(double));
    }

  /* copy the face normals */
  if(pomeshsrc->face_normals)
    {
      if(!(pomesh->face_normals = malloc(3 * pomeshsrc->npolys *
					 sizeof(double))))
	{
	  ay_status = AY_EOMEM;
	  goto cleanup;
	}
      memcpy(pomesh->face_normals, pomeshsrc->face_normals,
	     3 * pomeshsrc->npolys * sizeof(double));
    }

  *dst = (void *)pomesh;

  /* prevent cleanup code from doing something harmful */
  pomesh = NULL;

cleanup:

  if(pomesh)
    {
      if(pomesh->nloops)
	free(pomesh->nloops);
      if(pomesh->nverts)
	free(pomesh->nverts);
      if(pomesh->verts)
	free(pomesh->verts);
      if(pomesh->controlv)
	free(pomesh->controlv);
      if(pomesh->face_normals)
	free(pomesh->face_normals);
      free(pomesh);
    }

 return ay_status;
} /* ay_pomesh_copycb */


/* ay_pomesh_drawcb:
 *  draw (display in an Ayam view window) callback function of pomesh object
 */
int
ay_pomesh_drawcb(struct Togl *togl, ay_object *o)
{
 ay_pomesh_object *pomesh = NULL;
 int stride = 0;
 unsigned int i = 0, j = 0, k = 0, l = 0, m = 0, n = 0;
 unsigned int a;
#if 0
 double p[3];
#endif

  if(!o)
    return AY_ENULL;

  pomesh = (ay_pomesh_object *)(o->refine);

  if(!pomesh)
    return AY_ENULL;

  if(pomesh->has_normals)
    stride = 6;
  else
    stride = 3;

  for(i = 0; i < pomesh->npolys; i++)
    {
      for(j = 0; j < pomesh->nloops[l]; j++)
	{
	  glBegin(GL_LINE_LOOP);
	   for(k = 0; k < pomesh->nverts[m]; k++)
	    {
	      a = pomesh->verts[n++];
	      glVertex3dv((GLdouble*)(&(pomesh->controlv[a * stride])));
	    }
	  glEnd();
	  m++;
	} /* for */
      l++;
    } /* for */
#if 0
  if(pomesh->has_normals)
    {
      glBegin(GL_LINES);
      a = 0;
      for(i = 0; i < pomesh->ncontrols; i++)
	{
	  glVertex3dv((GLdouble*)(&(pomesh->controlv[a])));
	  memcpy(p, &(pomesh->controlv[a]), 3*sizeof(double));
	  p[0] += pomesh->controlv[a+3]*0.1;
	  p[1] += pomesh->controlv[a+4]*0.1;
	  p[2] += pomesh->controlv[a+5]*0.1;
	  glVertex3dv((GLdouble*)p);
	  a += 6;
	}
      glEnd();
    }
#endif
 return AY_OK;
} /* ay_pomesh_drawcb */


/* ay_pomesh_shadecb:
 *  shade (display in an Ayam view window) callback function of pomesh object
 */
int
ay_pomesh_shadecb(struct Togl *togl, ay_object *o)
{
 int ay_status = AY_OK;
 ay_pomesh_object *pomesh = NULL;
 /*
 int i = 0, j = 0, k = 0, l = 0;
 unsigned int a;
 GLUtesselator *tess;
 */
  if(!o)
    return AY_ENULL;

  pomesh = (ay_pomesh_object *)(o->refine);

  if(!pomesh)
    return AY_ENULL;

  if(1/*o->modified*/)
    {
      ay_status = ay_pomesht_tesselate(pomesh);
    }

 return ay_status;
} /* ay_pomesh_shadecb */


/* ay_pomesh_drawhcb:
 *  draw handles (in an Ayam view window) callback function of pomesh object
 */
int
ay_pomesh_drawhcb(struct Togl *togl, ay_object *o)
{
 unsigned int i;
 int stride;
 double *pnts;
 ay_pomesh_object *pomesh;

  if(!o)
    return AY_ENULL;

  pomesh = (ay_pomesh_object *)(o->refine);

  if(!pomesh)
    return AY_ENULL;

  if(pomesh->has_normals)
    stride = 6;
  else
    stride = 3;

  pnts = pomesh->controlv;

  glBegin(GL_POINTS);
   for(i = 0; i < pomesh->ncontrols; i++)
     {
       glVertex3dv((GLdouble*)pnts);
       pnts += stride;
     }
  glEnd();

 return AY_OK;
} /* ay_pomesh_drawhcb */


/* ay_pomesh_getpntcb:
 *  get point (editing and selection) callback function of pomesh object
 */
int
ay_pomesh_getpntcb(int mode, ay_object *o, double *p, ay_pointedit *pe)
{
 ay_pomesh_object *pomesh = NULL;
 ay_point *pnt = NULL, **lastpnt = NULL;
 double min_dist = ay_prefs.pick_epsilon, dist = 0.0;
 double *pecoord = NULL, **ctmp;
 double *control = NULL, *c = NULL;
 unsigned int i = 0, j = 0, a = 0;
 unsigned int *itmp, peindex = 0;
 int stride = 0;

  if(!o || ((mode != 3) && (!p || !pe)))
    return AY_ENULL;

  pomesh = (ay_pomesh_object *)(o->refine);

  if(!pomesh)
    return AY_ENULL;

  if(pomesh->has_normals)
    stride = 6;
  else
    stride = 3;

  if(min_dist == 0.0)
    min_dist = DBL_MAX;

  if(pe)
    pe->rational = AY_FALSE;

  switch(mode)
    {
    case 0:
      /* select all points */
      if(!(pe->coords = calloc(pomesh->ncontrols, sizeof(double*))))
	return AY_EOMEM;

      if(!(pe->indices = calloc(pomesh->ncontrols,
				sizeof(unsigned int))))
	return AY_EOMEM;

      for(i = 0; i < pomesh->ncontrols; i++)
	{
	  pe->coords[i] = &(pomesh->controlv[a]);
	  pe->indices[i] = i;
	  a += stride;
	} /* for */

      pe->num = pomesh->ncontrols;
      break;
    case 1:
      /* selection based on a single point */
      control = pomesh->controlv;
      for(i = 0; i < pomesh->ncontrols; i++)
	{
	  dist = AY_VLEN((p[0] - control[j]),
			 (p[1] - control[j+1]),
			 (p[2] - control[j+2]));

	  if(dist < min_dist)
	    {
	      min_dist = dist;
	      pecoord = &(control[j]);
	      peindex = i;
	    }

	  j += stride;
	} /* for */

      if(!pecoord)
	return AY_OK; /* XXXX should this return a 'AY_EPICK' ? */

      if(!(pe->coords = calloc(1, sizeof(double*))))
	return AY_EOMEM;
      if(!(pe->indices = calloc(1, sizeof(unsigned int))))
	return AY_EOMEM;

      pe->coords[0] = pecoord;
      pe->indices[0] = peindex;
      pe->num = 1;
      break;
    case 2:
      /* selection based on planes */
      control = pomesh->controlv;
      j = 0;
      a = 0;
      for(i = 0; i < pomesh->ncontrols; i++)
	{
	  c = &(control[j]);

	  /* test point c against the four planes in p */
	  if(((p[0]*c[0] + p[1]*c[1] + p[2]*c[2] + p[3]) < 0.0) &&
	     ((p[4]*c[0] + p[5]*c[1] + p[6]*c[2] + p[7]) < 0.0) &&
	     ((p[8]*c[0] + p[9]*c[1] + p[10]*c[2] + p[11]) < 0.0) &&
	     ((p[12]*c[0] + p[13]*c[1] + p[14]*c[2] + p[15]) < 0.0))
	    {
	      if(!(ctmp = realloc(pe->coords, (a+1)*sizeof(double *))))
		return AY_EOMEM;
	      pe->coords = ctmp;
	      if(!(itmp = realloc(pe->indices, (a+1)*sizeof(unsigned int))))
		return AY_EOMEM;
	      pe->indices = itmp;

	      pe->coords[a] = &(control[j]);
	      pe->indices[a] = i;

	      a++;
	    } /* if */
	  j += stride;
	} /* for */

      pe->num = a;
      break;
    case 3:
      /* rebuild from o->selp */
      pnt = o->selp;
      lastpnt = &o->selp;
      while(pnt)
	{
	  if(pnt->index < pomesh->ncontrols)
	    {
	      pnt->point = &(pomesh->controlv[pnt->index*stride]);
	      pnt->rational = AY_FALSE;
	      lastpnt = &(pnt->next);
	      pnt = pnt->next;
	    }
	  else
	    {
	      *lastpnt = pnt->next;
	      free(pnt);
	      pnt = *lastpnt;
	    }
	}
      break;
    default:
      break;
    } /* if */

 return AY_OK;
} /* ay_pomesh_getpntcb */


/* ay_pomesh_setpropcb:
 *  set property (from Tcl to C context) callback function of pomesh object
 */
int
ay_pomesh_setpropcb(Tcl_Interp *interp, int argc, char *argv[], ay_object *o)
{
 /*int ay_status = AY_OK;*/
 char *n1 = "PolyMeshAttrData";
 /*char fname[] = "pomesh_setpropcb";*/
 Tcl_Obj *to = NULL, *toa = NULL, *ton = NULL;
 ay_pomesh_object *pomesh = NULL;
 int new_type;

  if(!interp || !o)
    return AY_ENULL;

  pomesh = (ay_pomesh_object *)o->refine;

  if(!pomesh)
    return AY_ENULL;

  toa = Tcl_NewStringObj(n1, -1);
  ton = Tcl_NewStringObj(n1, -1);

  Tcl_SetStringObj(ton, "Type", -1);
  to = Tcl_ObjGetVar2(interp, toa, ton, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp, to, &new_type);

  pomesh->type = new_type;

  Tcl_IncrRefCount(toa);Tcl_DecrRefCount(toa);
  Tcl_IncrRefCount(ton);Tcl_DecrRefCount(ton);

  o->modified = AY_TRUE;
  ay_notify_parent();

 return AY_OK;
} /* ay_pomesh_setpropcb */


/* ay_pomesh_getpropcb:
 *  get property (from C to Tcl context) callback function of pomesh object
 */
int
ay_pomesh_getpropcb(Tcl_Interp *interp, int argc, char *argv[], ay_object *o)
{
 char *n1="PolyMeshAttrData";
 Tcl_Obj *to = NULL, *toa = NULL, *ton = NULL;
 ay_pomesh_object *pomesh = NULL;

  if(!interp || !o)
    return AY_ENULL;

  pomesh = (ay_pomesh_object *)(o->refine);

  if(!pomesh)
    return AY_ENULL;

  toa = Tcl_NewStringObj(n1, -1);
  ton = Tcl_NewStringObj(n1, -1);

  Tcl_SetStringObj(ton, "Type", -1);
  to = Tcl_NewIntObj(pomesh->type);
  Tcl_ObjSetVar2(interp, toa, ton, to, TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton, "NPolys", -1);
  to = Tcl_NewIntObj((int)pomesh->npolys);
  Tcl_ObjSetVar2(interp, toa, ton, to, TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton, "NControls", -1);
  to = Tcl_NewIntObj((int)pomesh->ncontrols);
  Tcl_ObjSetVar2(interp, toa, ton, to, TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton, "HasNormals", -1);
  to = Tcl_NewIntObj((int)pomesh->has_normals);
  Tcl_ObjSetVar2(interp, toa, ton, to, TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);

  Tcl_IncrRefCount(toa);Tcl_DecrRefCount(toa);
  Tcl_IncrRefCount(ton);Tcl_DecrRefCount(ton);

 return AY_OK;
} /* ay_pomesh_getpropcb */


/* ay_pomesh_readcb:
 *  read (from scene file) callback function of pomesh object
 */
int
ay_pomesh_readcb(FILE *fileptr, ay_object *o)
{
 int ay_status = AY_OK;
 ay_pomesh_object *pomesh = NULL;
 unsigned int total_loops = 0, total_verts = 0;
 unsigned int i, a;

  if(!fileptr || !o)
   return AY_ENULL;

  if(!(pomesh = calloc(1, sizeof(ay_pomesh_object))))
    { return AY_EOMEM; }

  fscanf(fileptr, "%d\n", &pomesh->type);
  fscanf(fileptr, "%u\n", &pomesh->npolys);
  if(!(pomesh->nloops = calloc(pomesh->npolys, sizeof(unsigned int))))
    { ay_status = AY_EOMEM; goto cleanup; }
  for(i = 0; i < pomesh->npolys; i++)
    {
      fscanf(fileptr, "%u\n", &(pomesh->nloops[i]));
    }

  fscanf(fileptr, "%u\n", &total_loops);
  if(!(pomesh->nverts = calloc(total_loops, sizeof(unsigned int))))
    { ay_status = AY_EOMEM; goto cleanup; }

  for(i = 0; i < total_loops; i++)
    {
      fscanf(fileptr, "%u\n", &(pomesh->nverts[i]));
    }

  fscanf(fileptr, "%u\n", &total_verts);
  if(!(pomesh->verts = calloc(total_verts, sizeof(unsigned int))))
    { ay_status = AY_EOMEM; goto cleanup; }

  for(i = 0; i < total_verts; i++)
    {
      fscanf(fileptr, "%u\n", &(pomesh->verts[i]));
    }

  /* read controlv */
  fscanf(fileptr,"%u\n",&pomesh->ncontrols);
  fscanf(fileptr, "%d\n", &pomesh->has_normals);

  if(!(pomesh->controlv = calloc(pomesh->ncontrols * 3 + (pomesh->has_normals*
				 pomesh->ncontrols * 3), sizeof(double))))
    { ay_status = AY_EOMEM; goto cleanup; }

  if(pomesh->has_normals)
    {
      a = 0;
      for(i = 0; i < pomesh->ncontrols; i++)
	{
	  fscanf(fileptr, "%lg %lg %lg\n", &(pomesh->controlv[a]),
		 &(pomesh->controlv[a+1]),
		 &(pomesh->controlv[a+2]));
	  a += 3;
	  fscanf(fileptr, "%lg %lg %lg\n", &(pomesh->controlv[a]),
		 &(pomesh->controlv[a+1]),
		 &(pomesh->controlv[a+2]));
	  a += 3;
	} /* for */
    }
  else
    {
      a = 0;
      for(i = 0; i < pomesh->ncontrols; i++)
	{
	  fscanf(fileptr, "%lg %lg %lg\n", &(pomesh->controlv[a]),
		 &(pomesh->controlv[a+1]),
		 &(pomesh->controlv[a+2]));
	  a += 3;
	} /* for */
    } /* if */

  /* return result */
  o->refine = pomesh;

  /* prevent cleanup code from doing something harmful */
  pomesh = NULL;

cleanup:

  if(pomesh)
    {
      if(pomesh->nloops)
	free(pomesh->nloops);

      if(pomesh->nverts)
	free(pomesh->nverts);

      if(pomesh->verts)
	free(pomesh->verts);

      if(pomesh->controlv)
	free(pomesh->controlv);

      free(pomesh);
    }

 return ay_status;
} /* ay_pomesh_readcb */


/* ay_pomesh_writecb:
 *  write (to scene file) callback function of pomesh object
 */
int
ay_pomesh_writecb(FILE *fileptr, ay_object *o)
{
 ay_pomesh_object *pomesh = NULL;
 unsigned int total_loops = 0, total_verts = 0;
 unsigned int i = 0, a = 0;

  if(!fileptr || !o)
    return AY_ENULL;

  pomesh = (ay_pomesh_object *)(o->refine);

  if(!pomesh)
    return AY_ENULL;

  fprintf(fileptr, "%d\n", pomesh->type);
  fprintf(fileptr, "%u\n", pomesh->npolys);

  /* write nloops */
  for(i = 0; i < pomesh->npolys; i++)
    {
      fprintf(fileptr, "%u\n", pomesh->nloops[i]);
      total_loops += pomesh->nloops[i];
    } /* for */

  /* write nverts */
  fprintf(fileptr, "%u\n", total_loops);
  for(i = 0; i < total_loops; i++)
    {
      fprintf(fileptr, "%u\n", pomesh->nverts[i]);
      total_verts += pomesh->nverts[i];
    } /* for */

  /* write verts */
  fprintf(fileptr, "%u\n", total_verts);
  for(i = 0; i < total_verts; i++)
    {
      fprintf(fileptr, "%u\n", pomesh->verts[i]);
    } /* for */

  /* write controlv */
  fprintf(fileptr, "%u\n", pomesh->ncontrols);
  fprintf(fileptr, "%d\n", pomesh->has_normals);
  if(pomesh->has_normals)
    {
      a = 0;
      for(i = 0; i < pomesh->ncontrols; i++)
	{
	  fprintf(fileptr,"%g %g %g\n", pomesh->controlv[a],
		  pomesh->controlv[a+1], pomesh->controlv[a+2]);
	  a += 3;
	  fprintf(fileptr,"%g %g %g\n", pomesh->controlv[a],
		  pomesh->controlv[a+1], pomesh->controlv[a+2]);
	  a += 3;
	}
    }
  else
    {
      a = 0;
      for(i = 0; i < pomesh->ncontrols; i++)
	{
	  fprintf(fileptr,"%g %g %g\n", pomesh->controlv[a],
		  pomesh->controlv[a+1], pomesh->controlv[a+2]);
	  a += 3;
	}
    }

 return AY_OK;
} /* ay_pomesh_writecb */


/* ay_pomesh_wribcb:
 *  RIB export callback function of pomesh object
 */
int
ay_pomesh_wribcb(char *file, ay_object *o)
{
 int ay_status = AY_OK;
 ay_pomesh_object *pomesh = NULL;
 RtInt *nloops = NULL, *nverts = NULL, *verts = NULL;
 RtPoint *controls = NULL, *normals = NULL;
 RtToken *tokens = NULL;
 RtPointer *parms = NULL;
 unsigned int i = 0, a = 0;
 unsigned int total_loops = 0, total_verts = 0;
 int stride = 0, j, n = 0, pvc = 0;

  if(!o)
    return AY_OK;

  pomesh = (ay_pomesh_object*)(o->refine);

  if(!pomesh)
    return AY_ENULL;

  if(pomesh->has_normals)
    stride = 6;
  else
    stride = 3;

  if(!(controls = calloc(pomesh->ncontrols, sizeof(RtPoint))))
    return AY_EOMEM;

  a = 0;
  for(i = 0; i < pomesh->ncontrols; i++)
    {
      controls[i][0] = (RtFloat)pomesh->controlv[a];
      controls[i][1] = (RtFloat)pomesh->controlv[a+1];
      controls[i][2] = (RtFloat)pomesh->controlv[a+2];
      a += stride;
    } /* for */

  if(pomesh->has_normals)
    {
      if(!(normals = calloc(pomesh->ncontrols, sizeof(RtPoint))))
	{ ay_status = AY_EOMEM; goto cleanup; }

      a = 3;
      for(i = 0; i < pomesh->ncontrols; i++)
	{
	  normals[i][0] = (RtFloat)pomesh->controlv[a];
	  normals[i][1] = (RtFloat)pomesh->controlv[a+1];
	  normals[i][2] = (RtFloat)pomesh->controlv[a+2];
	  a += stride;
	} /* for */
    } /* if */


  if(!(nloops = calloc(pomesh->npolys, sizeof(RtInt))))
    { ay_status = AY_EOMEM; goto cleanup; }

  for(i = 0; i < pomesh->npolys; i++)
    {
      nloops[i] = (RtInt)(pomesh->nloops[i]);
      total_loops += pomesh->nloops[i];
    } /* for */

  if(!(nverts = calloc(total_loops, sizeof(RtInt))))
    { ay_status = AY_EOMEM; goto cleanup; }

  for(i = 0; i < total_loops; i++)
    {
      nverts[i] = (RtInt)(pomesh->nverts[i]);
      total_verts += pomesh->nverts[i];
    } /* for */

  if(!(verts = calloc(total_verts, sizeof(RtInt))))
    { ay_status = AY_EOMEM; goto cleanup; }

  for(i = 0; i < total_verts; i++)
    {
      verts[i] = (RtInt)(pomesh->verts[i]);
    } /* for */

  /* Do we have any primitive variables? */
  if(!(pvc = ay_pv_count(o)))
    {
      /* No */
      if(pomesh->has_normals)
	RiPointsGeneralPolygons((RtInt)pomesh->npolys, nloops, nverts, verts,
				"P", controls, "N", normals, NULL);
      else
	RiPointsGeneralPolygons((RtInt)pomesh->npolys, nloops, nverts, verts,
				"P", controls, NULL);
    }
  else
    {
      /* Yes, we have primitive variables. */
      if(pomesh->has_normals)
	{
	  if(!(tokens = calloc(pvc+2, sizeof(RtToken))))
	    { ay_status = AY_EOMEM; goto cleanup; }

	  if(!(parms = calloc(pvc+2, sizeof(RtPointer))))
	    { free(tokens); ay_status = AY_EOMEM; goto cleanup; }

	  tokens[0] = "P";
	  parms[0] = (RtPointer)controls;
	  tokens[1] = "N";
	  parms[1] = (RtPointer)normals;

	  n = 2;
	  ay_status = ay_pv_filltokpar(o, AY_TRUE, 2, &n, tokens, parms);
	}
      else
	{
	  if(!(tokens = calloc(pvc+1, sizeof(RtToken))))
	    { ay_status = AY_EOMEM; goto cleanup; }

	  if(!(parms = calloc(pvc+1, sizeof(RtPointer))))
	    { free(tokens); ay_status = AY_EOMEM; goto cleanup; }

	  tokens[0] = "P";
	  parms[0] = (RtPointer)controls;

	  n = 1;
	  ay_status = ay_pv_filltokpar(o, AY_TRUE, 1, &n, tokens, parms);
	} /* if */

      if(!ay_status)
	{
	  RiPointsGeneralPolygonsV((RtInt)pomesh->npolys, nloops, nverts, verts,
				   (RtInt)n, tokens, parms);
	}

      for(j = (pomesh->has_normals?2:1); j < n; j++)
	{
	  if(tokens[j])
	    free(tokens[j]);
	  if(parms[i])
	    free(parms[j]);
	}

      free(tokens);
      free(parms);
    } /* if */

  /* clean up */
cleanup:
  if(controls)
    free(controls);
  if(normals)
    free(normals);
  if(nloops)
    free(nloops);
  if(nverts)
    free(nverts);
  if(verts)
    free(verts);

 return ay_status;
} /* ay_pomesh_wribcb */


/* ay_pomesh_bbccb:
 *  bounding box calculation callback function of pomesh object
 */
int
ay_pomesh_bbccb(ay_object *o, double *bbox, int *flags)
{
 int stride = 0;
 ay_pomesh_object *pomesh = NULL;

  if(!o || !bbox || !flags)
    return AY_ENULL;

  pomesh = (ay_pomesh_object *)o->refine;

  if(!pomesh)
    return AY_ENULL;

  if(pomesh->has_normals)
    stride = 6;
  else
    stride = 3;

 return ay_bbc_fromarr(pomesh->controlv, pomesh->ncontrols,
		       stride, bbox);
} /* ay_pomesh_bbccb */


/* ay_pomesh_notifycb:
 *  notification callback function of pomesh object
 */
int
ay_pomesh_notifycb(ay_object *o)
{
  /* int ay_status = AY_OK;*/
 ay_pomesh_object *pomesh = NULL;

  if(!o)
    return AY_ENULL;

  pomesh = (ay_pomesh_object *) o->refine;

  if(!pomesh)
    return AY_ENULL;

  if(pomesh->face_normals)
    free(pomesh->face_normals);
  pomesh->face_normals = NULL;

 return AY_OK;
} /* ay_pomesh_notifycb */


/* ay_pomesh_convertcb:
 *  convert callback function of pomesh object
 */
int
ay_pomesh_convertcb(ay_object *o, int in_place)
{
 int ay_status = AY_OK;
 ay_pomesh_object *pomesh = NULL;
 ay_object *new = NULL;

  if(!o)
    return AY_ENULL;

  pomesh = (ay_pomesh_object *) o->refine;

  /* first, create new object(s) */
  if(!(new = calloc(1, sizeof(ay_object))))
    { return AY_EOMEM; }

  ay_object_defaults(new);
  new->type = AY_IDSDMESH;

  ay_status = ay_pomesht_tosdmesh(pomesh,
				  ((ay_sdmesh_object**)(void*)&(new->refine)));

  /* second, link new object(s), or replace old object with it/them */
  if(new && new->refine)
    {

      ay_trafo_copy(o, new);

      if(!in_place)
	{
	  ay_object_link(new);
	}
      else
	{
	  ay_object_replace(new, o);
	} /* if */
    } /* if */

 return ay_status;
} /* ay_pomesh_convertcb */


/* ay_pomesh_init:
 *  initialize the pomesh object module
 */
int
ay_pomesh_init(Tcl_Interp *interp)
{
 int ay_status = AY_OK;

  ay_status = ay_otype_registercore(ay_pomesh_name,
				    ay_pomesh_createcb,
				    ay_pomesh_deletecb,
				    ay_pomesh_copycb,
				    ay_pomesh_drawcb,
				    ay_pomesh_drawhcb,
				    ay_pomesh_shadecb,
				    ay_pomesh_setpropcb,
				    ay_pomesh_getpropcb,
				    ay_pomesh_getpntcb,
				    ay_pomesh_readcb,
				    ay_pomesh_writecb,
				    ay_pomesh_wribcb,
				    ay_pomesh_bbccb,
				    AY_IDPOMESH);

  /*
    ay_status = ay_notify_register(ay_pomesh_notifycb, AY_IDPOMESH);
  */

  ay_status += ay_convert_register(ay_pomesh_convertcb, AY_IDPOMESH);

 return ay_status;
} /* ay_pomesh_init */

