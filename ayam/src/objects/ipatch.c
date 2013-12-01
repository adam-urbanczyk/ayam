/*
 * Ayam, a free 3D modeler for the RenderMan interface.
 *
 * Ayam is copyrighted 1998-2011 by Randolf Schultz
 * (randolf.schultz@gmail.com) and others.
 *
 * All rights reserved.
 *
 * See the file License for details.
 *
 */

#include "ayam.h"

/* ipatch.c -  Interpolating Patch object */

static char *ay_ipatch_name = "IPatch";

void ay_ipatch_drawcp(ay_ipatch_object *ipatch);

void ay_ipatch_drawders(ay_ipatch_object *ipatch);

/* functions: */

/* ay_ipatch_createcb:
 *  create callback function of ipatch object
 */
int
ay_ipatch_createcb(int argc, char *argv[], ay_object *o)
{
 int ay_status = AY_OK;
 int tcl_status = TCL_OK;
 char fname[] = "crtipatch";
 char dererror[] = "Not enough derivatives provided.";
 char option_handled = AY_FALSE;
 int center = AY_FALSE, createmp = -1;
 int stride = 3, uorder = 4, vorder = 4, width = 4, height = 4;
 int uclosed = AY_FALSE, vclosed = AY_FALSE;
 int ukt = 0, vkt = 0, optnum = 0, i = 2, j = 0, k = 0;
 int deriv_u = AY_FALSE, deriv_v = AY_FALSE;
 int sdulen = 0, sdvlen = 0, edulen = 0, edvlen = 0;
 int acvlen = 0, *dlen;
 char **acv = NULL;
 double *cv = NULL;
 double udx = 0.25, udy = 0.0, udz = 0.0;
 double vdx = 0.0, vdy = 0.25, vdz = 0.0;
 double ext = 0.0, s[3] = {0};
 double sdlen_u = 0.125, edlen_u = 0.125;
 double sdlen_v = 0.125, edlen_v = 0.125;
 double *sderiv_u = NULL, *ederiv_u = NULL;
 double *sderiv_v = NULL, *ederiv_v = NULL;
 double **deriv;
 ay_ipatch_object *ip = NULL;

  if(!argv || !o)
    return AY_ENULL;

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
      option_handled = AY_FALSE;
      optnum = i;
      if(argv[i] && argv[i][0] != '\0')
	{
	  switch(argv[i][1])
	    {
	    case 'w':
	      /* -width */
	      tcl_status = Tcl_GetInt(ay_interp, argv[i+1], &width);
	      option_handled = AY_TRUE;
	      break;
	    case 'h':
	      /* -height */
	      tcl_status = Tcl_GetInt(ay_interp, argv[i+1], &height);
	      option_handled = AY_TRUE;
	      break;
	    case 'u':
	      switch(argv[i][2])
		{
		case 'c':
		  /* -uclosed */
		  tcl_status = Tcl_GetInt(ay_interp, argv[i+1], &uclosed);
		  option_handled = AY_TRUE;
		  break;
		case 'd':
		  switch(argv[i][3])
		    {
		    case 'x':
		      /* -udx */
		      tcl_status = Tcl_GetDouble(ay_interp, argv[i+1], &udx);
		      option_handled = AY_TRUE;
		      break;
		    case 'y':
		      /* -udy */
		      tcl_status = Tcl_GetDouble(ay_interp, argv[i+1], &udy);
		      option_handled = AY_TRUE;
		      break;
		    case 'z':
		      /* -udz */
		      tcl_status = Tcl_GetDouble(ay_interp, argv[i+1], &udz);
		      option_handled = AY_TRUE;
		      break;
		    default:
		      break;
		    } /* switch */
		  break;
		case 'o':
		  /* -uorder */
		  tcl_status = Tcl_GetInt(ay_interp, argv[i+1], &uorder);
		  option_handled = AY_TRUE;
		  break;
		case 'k':
		  /* -uktype */
		  tcl_status = Tcl_GetInt(ay_interp, argv[i+1], &ukt);
		  option_handled = AY_TRUE;
		  break;
		default:
		  break;
		} /* switch */
	      break;
	    case 'v':
	      switch(argv[i][2])
		{
		case 'c':
		  /* -vclosed */
		  tcl_status = Tcl_GetInt(ay_interp, argv[i+1], &vclosed);
		  option_handled = AY_TRUE;
		  break;
		case 'd':
		  switch(argv[i][3])
		    {
		    case 'x':
		      /* -vdx */
		      tcl_status = Tcl_GetDouble(ay_interp, argv[i+1], &vdx);
		      option_handled = AY_TRUE;
		      break;
		    case 'y':
		      /* -vdy */
		      tcl_status = Tcl_GetDouble(ay_interp, argv[i+1], &vdy);
		      option_handled = AY_TRUE;
		      break;
		    case 'z':
		      /* -vdz */
		      tcl_status = Tcl_GetDouble(ay_interp, argv[i+1], &vdz);
		      option_handled = AY_TRUE;
		      break;
		    default:
		      break;
		    } /* switch */
		  break;
		case 'o':
		  /* -vorder */
		  tcl_status = Tcl_GetInt(ay_interp, argv[i+1], &vorder);
		  option_handled = AY_TRUE;
		  break;
		case 'k':
		  /* -vktype */
		  tcl_status = Tcl_GetInt(ay_interp, argv[i+1], &vkt);
		  option_handled = AY_TRUE;
		  break;
		default:
		  break;
		} /* switch */
	      break;
	    case 'c':
	      switch(argv[i][2])
		{
		case 'n':
		  /* -cn */
		  if(cv)
		    {
		      free(cv);
		      cv = NULL;
		    }
		  tcl_status = ay_tcmd_convdlist(argv[i+1], &acvlen, &cv);
		  option_handled = AY_TRUE;
		  break;
		case 'v':
		  /* -cv */
		  if(Tcl_SplitList(ay_interp, argv[i+1], &acvlen, &acv) ==
		     TCL_OK)
		    {
		      if(cv)
			{
			  free(cv);
			}
		      if(!(cv = calloc(acvlen, sizeof(double))))
			{
			  Tcl_Free((char *) acv);
			  ay_status = AY_EOMEM;
			  goto cleanup;
			}
		      for(j = 0; j < acvlen; j++)
			{
			  tcl_status = Tcl_GetDouble(ay_interp,
						     acv[j], &cv[j]);
			  if(tcl_status != TCL_OK)
			    {
			      break;
			    }
			} /* for */
		      Tcl_Free((char *) acv);
		    }
		  option_handled = AY_TRUE;
		  break;
		case 'e':
		  /* -center */
		  tcl_status = Tcl_GetInt(ay_interp, argv[i+1], &center);
		  option_handled = AY_TRUE;
		  break;
		case 'r':
		  /* -createmp */
		  tcl_status = Tcl_GetBoolean(ay_interp, argv[i+1], &createmp);
		  option_handled = AY_TRUE;
		  break;
		default:
		  break;
		} /* switch */
	      break;
	    case 'e':
	      if(argv[i][2] == 'd')
		{
		  switch(argv[i][3])
		    {
		    case 'e':
		      /* -ederiv_u/-ederiv_v */
		      if(strlen(argv[i]) == 9)
			{
			  if(argv[i][8] == 'u')
			    {
			      deriv = &ederiv_u;
			      dlen = &edulen;
			    }
			  else
			    {
			      deriv = &ederiv_v;
			      dlen = &edvlen;
			    }

			  if(Tcl_SplitList(ay_interp, argv[i+1], &acvlen,
					   &acv) == TCL_OK)
			    {
			      if(*deriv)
				{
				  free(*deriv);
				}
			      if(!(*deriv = calloc(acvlen, sizeof(double))))
				{
				  Tcl_Free((char *) acv);
				  ay_status = AY_EOMEM;
				  goto cleanup;
				}
			      for(j = 0; j < acvlen; j++)
				{
				  tcl_status = Tcl_GetDouble(ay_interp,
							acv[j], &(*deriv)[j]);
				  if(tcl_status != TCL_OK)
				    {
				      break;
				    }
				} /* for */
			      *dlen = acvlen;
			      Tcl_Free((char *) acv);
			    } /* if */
			  option_handled = AY_TRUE;
			} /* if */
		      break;
		    case 'l':
		      /* -edlen_u/-edlen_v */
		      if(strlen(argv[i]) == 8)
			{
			  if(argv[i][7] == 'u')
			    tcl_status = Tcl_GetDouble(ay_interp, argv[i+1],
						       &edlen_u);
			  else
			    tcl_status = Tcl_GetDouble(ay_interp, argv[i+1],
						       &edlen_v);
			  option_handled = AY_TRUE;
			}
		      break;

		    default:
		      break;
		    } /* switch */
		} /* if */
	      break;
	    case 'd':
	      if(argv[i][2] == 'e' && strlen(argv[i]) == 8)
		{
		  /* -deriv_u/-deriv_v */
		  if(argv[i][7] == 'u')
		    tcl_status = Tcl_GetInt(ay_interp, argv[i+1],
					    &deriv_u);
		  else
		    tcl_status = Tcl_GetInt(ay_interp, argv[i+1],
					    &deriv_v);
		  option_handled = AY_TRUE;
		}
	      break;
	    case 's':
	      if(argv[i][2] == 'd')
		{
		  switch(argv[i][3])
		    {
		    case 'e':
		      /* -sderiv_u/-sderiv_v */
		      if(strlen(argv[i]) == 9)
			{
			  if(argv[i][8] == 'u')
			    {
			      deriv = &sderiv_u;
			      dlen = &sdulen;
			    }
			  else
			    {
			      deriv = &sderiv_v;
			      dlen = &sdvlen;
			    }

			  if(Tcl_SplitList(ay_interp, argv[i+1], &acvlen,
					   &acv) == TCL_OK)
			    {
			      if(*deriv)
				{
				  free(*deriv);
				}
			      if(!(*deriv = calloc(acvlen, sizeof(double))))
				{
				  Tcl_Free((char *) acv);
				  ay_status = AY_EOMEM;
				  goto cleanup;
				}
			      for(j = 0; j < acvlen; j++)
				{
				  tcl_status = Tcl_GetDouble(ay_interp,
							acv[j], &(*deriv)[j]);
				  if(tcl_status != TCL_OK)
				    {
				      break;
				    }
				} /* for */
			      Tcl_Free((char *) acv);
			      *dlen = acvlen;
			    } /* if */
			  option_handled = AY_TRUE;
			} /* if */
		      break;
		    case 'l':
		      /* -sdlen_u/-sdlen_v */
		      if(strlen(argv[i]) == 8)
			{
			  if(argv[i][7] == 'u')
			    tcl_status = Tcl_GetDouble(ay_interp, argv[i+1],
						       &sdlen_u);
			  else
			    tcl_status = Tcl_GetDouble(ay_interp, argv[i+1],
						       &sdlen_v);
			  option_handled = AY_TRUE;
			}
		      break;
		    default:
		      break;
		    } /* switch */
		} /* if */
	      break;
	    default:
	      break;
	    } /* switch */

	  if(option_handled && (tcl_status != TCL_OK))
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

  /* check and correct width/height/uorder/vorder */
  if(width <= 1)
    {
      width = 4;
    }

  if(uorder < 2)
    {
      uorder = 2;
    }

  if(uorder > width)
    {
      uorder = (width < 4)?width:4;
    }

  if(height <= 1)
    {
      height = 4;
    }

  if(vorder < 2)
    {
      vorder = 2;
    }

  if(vorder > height)
    {
      vorder = (height < 4)?height:4;
    }

  if(cv)
    {
      /* check length of user provided control point array */
      if(acvlen/stride < width*height)
	{
	  if(acvlen > 0)
	    s[0] = cv[0];
	  if(acvlen > 1)
	    s[1] = cv[1];
	  if(acvlen > 2)
	    s[2] = cv[2];

	  free(cv);
	  cv = NULL;
	  center = AY_FALSE;
	}
    } /* if */

  /* check presence and lengths of user provided derivatives arrays */
  if(((deriv_u == 2) && (!sderiv_u || !ederiv_u)) ||
     ((deriv_v == 2) && (!sderiv_v || !ederiv_v)) ||
     (sderiv_u && (sdulen/stride < height)) ||
     (ederiv_u && (edulen/stride < height)) ||
     (sderiv_v && (sdvlen/stride < width)) ||
     (ederiv_v && (edvlen/stride < width)))
    {
      ay_error(AY_ERROR, fname, dererror);
      ay_status = AY_ERROR;
      goto cleanup;
    }

  /* now create the ipatch */
  if(!(ip = calloc(1, sizeof(ay_ipatch_object))))
    {
      ay_status = AY_EOMEM;
      goto cleanup;
    }

  ip->width = width;
  ip->height = height;
  ip->order_u = uorder;
  ip->order_v = vorder;
  ip->close_u = uclosed;
  ip->close_v = vclosed;

  if(ukt == 0)
    ip->ktype_u = AY_KTCHORDAL;
  else
    if(ukt == 1)
      ip->ktype_u = AY_KTCENTRI;
    else
      ip->ktype_u = AY_KTUNIFORM;
  if(vkt == 0)
    ip->ktype_v = AY_KTCHORDAL;
  else
    if(ukt == 1)
      ip->ktype_v = AY_KTCENTRI;
    else
      ip->ktype_u = AY_KTUNIFORM;

  ip->sdlen_u = sdlen_u;
  ip->edlen_u = edlen_u;
  ip->sdlen_v = sdlen_v;
  ip->edlen_v = edlen_v;

  ip->derivs_u = deriv_u;
  ip->derivs_v = deriv_v;

  ip->sderiv_u = sderiv_u;
  ip->ederiv_u = ederiv_u;
  ip->sderiv_v = sderiv_v;
  ip->ederiv_v = ederiv_v;

  if(!cv)
    {
      if(!(cv = calloc(width*height*stride,sizeof(double))))
	{
	  ay_status = AY_EOMEM;
	  goto cleanup;
	}

      if(center)
	{
	  if(fabs(udx) > AY_EPSILON)
	    ext = (width-1)*udx;
	  if(fabs(vdx) > AY_EPSILON)
	    ext += (height-1)*vdx;
	  s[0] = -(ext/2.0);
	  ext = 0.0;
	  if(fabs(udy) > AY_EPSILON)
	    ext = (width-1)*udy;
	  if(fabs(vdy) > AY_EPSILON)
	    ext += (height-1)*vdy;
	  s[1] = -(ext/2.0);
	  ext = 0.0;
	  if(fabs(udz) > AY_EPSILON)
	    ext = (width-1)*udz;
	  if(fabs(vdz) > AY_EPSILON)
	    ext += (height-1)*vdz;
	  s[2] = -(ext/2.0);
	}

      k = 0;
      for(i = 0; i < width; i++)
	{
	  for(j = 0; j < height; j++)
	    {
	      cv[k]   = s[0] + (double)j*vdx;
	      cv[k+1] = s[1] + (double)j*vdy;
	      cv[k+2] = s[2] + (double)j*vdz;
	      k += stride;
	    }
	  s[0] += udx;
	  s[1] += udy;
	  s[2] += udz;
	}
    } /* if */

  ip->controlv = cv;

  o->refine = (void *)ip;

  ay_notify_object(o);

  /* prevent cleanup code from doing something harmful */
  ip = NULL;
  cv = NULL;
  sderiv_u = NULL;
  ederiv_u = NULL;
  sderiv_v = NULL;
  ederiv_v = NULL;

cleanup:

  if(ip)
    free(ip);

  if(cv)
    free(cv);

  if(sderiv_u)
    free(sderiv_u);

  if(ederiv_u)
    free(ederiv_u);

  if(sderiv_v)
    free(sderiv_v);

  if(ederiv_v)
    free(ederiv_v);

  if(ay_status == AY_EOMEM)
    {
      ay_error(AY_EOMEM, fname, NULL);
      ay_status = AY_ERROR;
    }

 return ay_status;
} /* ay_ipatch_createcb */


/* ay_ipatch_deletecb:
 *  delete callback function of ipatch object
 */
int
ay_ipatch_deletecb(void *c)
{
 ay_ipatch_object *ipatch = NULL;

  if(!c)
    return AY_ENULL;

  ipatch = (ay_ipatch_object *)(c);

  /* free controlv */
  if(ipatch->controlv)
    free(ipatch->controlv);

  /* free derivatives */
  if(ipatch->sderiv_u)
    free(ipatch->sderiv_u);
  if(ipatch->ederiv_u)
    free(ipatch->ederiv_u);
  if(ipatch->sderiv_v)
    free(ipatch->sderiv_v);
  if(ipatch->ederiv_v)
    free(ipatch->ederiv_v);

  /* free NURBS patch(es) */
  if(ipatch->npatch)
    (void)ay_object_deletemulti(ipatch->npatch, AY_FALSE);

  if(ipatch->caps_and_bevels)
    (void)ay_object_deletemulti(ipatch->caps_and_bevels, AY_FALSE);

  free(ipatch);

 return AY_OK;
} /* ay_ipatch_deletecb */


/* ay_ipatch_copycb:
 *  copy callback function of ipatch object
 */
int
ay_ipatch_copycb(void *src, void **dst)
{
 int ay_status = AY_OK;
 ay_ipatch_object *ipatch = NULL, *ipatchsrc = NULL;

  if(!src || !dst)
    return AY_ENULL;

  ipatchsrc = (ay_ipatch_object *)src;

  if(!(ipatch = malloc(sizeof(ay_ipatch_object))))
    return AY_EOMEM;

  memcpy(ipatch, src, sizeof(ay_ipatch_object));

  ipatch->controlv = NULL;

  /* copy controlv */
  if(!(ipatch->controlv = malloc(3 * ipatch->width * ipatch->height *
				 sizeof(double))))
    {
      ay_status = AY_EOMEM;
      goto cleanup;
    }
  memcpy(ipatch->controlv, ipatchsrc->controlv,
	 3 * ipatch->width * ipatch->height * sizeof(double));

  /* copy derivatives */
  if(ipatch->derivs_u)
    {
      if(!(ipatch->sderiv_u = malloc(3 * ipatch->height *
				     sizeof(double))))
	{
	  ay_status = AY_EOMEM;
	  goto cleanup;
	}
      memcpy(ipatch->sderiv_u, ipatchsrc->sderiv_u,
	     3 * ipatch->height * sizeof(double));

      if(!(ipatch->ederiv_u = malloc(3 * ipatch->height *
				     sizeof(double))))
	{
	  ay_status = AY_EOMEM;
	  goto cleanup;
	}
      memcpy(ipatch->ederiv_u, ipatchsrc->ederiv_u,
	     3 * ipatch->height * sizeof(double));
    }
  else
    {
      ipatch->sderiv_u = NULL;
      ipatch->ederiv_u = NULL;
    }

  if(ipatch->derivs_v)
    {
      if(!(ipatch->sderiv_v = malloc(3 * ipatch->width *
				     sizeof(double))))
	{
	  ay_status = AY_EOMEM;
	  goto cleanup;
	}
      memcpy(ipatch->sderiv_v, ipatchsrc->sderiv_v,
	     3 * ipatch->width * sizeof(double));

      if(!(ipatch->ederiv_v = malloc(3 * ipatch->width *
				     sizeof(double))))
	{
	  ay_status = AY_EOMEM;
	  goto cleanup;
	}
      memcpy(ipatch->ederiv_v, ipatchsrc->ederiv_v,
	     3 * ipatch->width * sizeof(double));
    }
  else
    {
      ipatch->sderiv_v = NULL;
      ipatch->ederiv_v = NULL;
    }

  ipatch->npatch = NULL;
  ipatch->caps_and_bevels = NULL;

  *dst = (void *)ipatch;

  /* prevent cleanup code from doing something harmful */
  ipatch = NULL;

cleanup:

  if(ipatch)
    {
      if(ipatch->controlv)
	free(ipatch->controlv);

      if(ipatch->sderiv_u)
	free(ipatch->sderiv_u);
      if(ipatch->ederiv_u)
	free(ipatch->ederiv_u);
      if(ipatch->sderiv_v)
	free(ipatch->sderiv_v);
      if(ipatch->ederiv_v)
	free(ipatch->ederiv_v);

      free(ipatch);
    }

 return ay_status;
} /* ay_ipatch_copycb */


/* ay_ipatch_drawcp:
 *  internal helper function
 *  draw the control polygon
 */
void
ay_ipatch_drawcp(ay_ipatch_object *ipatch)
{
 double *ver = NULL;
 int i, j, a, width, height;

  if(!ipatch)
    return;

  width = ipatch->width;
  height = ipatch->height;

  ver = ipatch->controlv;

  a = 0;
  if(ipatch->close_v)
    {
      for(i = 0; i < width; i++)
	{
	  glBegin(GL_LINE_LOOP);
	   for(j = 0; j < height; j++)
	     {
	       glVertex3dv((GLdouble *)&ver[a]);
	       a += 3;
	     }
	  glEnd();
	}
    }
  else
    {
      for(i = 0; i < width; i++)
	{
	  glBegin(GL_LINE_STRIP);
	   for(j = 0; j < height; j++)
	     {
	       glVertex3dv((GLdouble *)&ver[a]);
	       a += 3;
	     } /* for */
	  glEnd();
	} /* for */
    } /* if */

  if(ipatch->close_u)
    {
      for(j = 0; j < height; j++)
	{
	  a = j * 3;
	  glBegin(GL_LINE_LOOP);
	   for(i = 0; i < width; i++)
	     {
	       glVertex3dv((GLdouble *)&ver[a]);

	       a += (3 * height);
	     }
	  glEnd();
	}
    }
  else
    {
      for(j = 0; j < height; j++)
	{
	  a = j * 3;
	  glBegin(GL_LINE_STRIP);
	   for(i = 0; i < width; i++)
	     {
	       glVertex3dv((GLdouble *)&ver[a]);

	       a += (3 * height);
	     } /* for */
	  glEnd();
	} /* for */
    } /* if */

 return;
} /* ay_ipatch_drawcp */


/* ay_ipatch_drawders:
 *  internal helper function
 *  draw the derivatives
 */
void
ay_ipatch_drawders(ay_ipatch_object *ipatch)
{
 double *cv = NULL, *der = NULL;
 int i, a, b, width, height;

  if(!ipatch)
    return;

  width = ipatch->width;
  height = ipatch->height;

  cv = ipatch->controlv;

  if(ipatch->derivs_u > 1)
    {
      if(ipatch->sderiv_u)
	{
	  a = 0;
	  b = 0;
	  der = ipatch->sderiv_u;
	  glBegin(GL_LINES);
	   for(i = 0; i < height; i++)
	     {
	       glVertex3dv((GLdouble *)&cv[a]);
	       glVertex3dv((GLdouble *)&der[b]);
	       a += 3;
	       b += 3;
	     }
	  glEnd();
	} /* if */
      if(ipatch->ederiv_u)
	{
	  if(ipatch->close_u)
	    a = 0;
	  else
	    a = (width-1)*height*3;
	  b = 0;
	  der = ipatch->ederiv_u;
	  glBegin(GL_LINES);
	   for(i = 0; i < height; i++)
	     {
	       glVertex3dv((GLdouble *)&cv[a]);
	       glVertex3dv((GLdouble *)&der[b]);
	       a += 3;
	       b += 3;
	     }
	  glEnd();
	} /* if */
    } /* if */

  if(ipatch->derivs_v > 1)
    {
      if(ipatch->sderiv_v)
	{
	  a = 0;
	  b = 0;
	  der = ipatch->sderiv_v;

	  glBegin(GL_LINES);
	   for(i = 0; i < width; i++)
	     {
	       glVertex3dv((GLdouble *)&cv[a]);
	       glVertex3dv((GLdouble *)&der[b]);
	       a += height*3;
	       b += 3;
	     }
	  glEnd();
	} /* if */
      if(ipatch->ederiv_v)
	{
	  if(ipatch->close_u)
	    a = 0;
	  else
	    a = (height-1)*3;
	  b = 0;
	  der = ipatch->ederiv_v;
	  glBegin(GL_LINES);
	   for(i = 0; i < width; i++)
	     {
	       glVertex3dv((GLdouble *)&cv[a]);
	       glVertex3dv((GLdouble *)&der[b]);
	       a += height*3;
	       b += 3;
	     }
	  glEnd();
	} /* if */
    } /* if */

 return;
} /* ay_ipatch_drawders */


/* ay_ipatch_drawcb:
 *  draw (display in an Ayam view window) callback function of ipatch object
 */
int
ay_ipatch_drawcb(struct Togl *togl, ay_object *o)
{
 int display_mode = ay_prefs.np_display_mode;
 ay_ipatch_object *ipatch = NULL;
 ay_object *p = NULL;

  if(!o)
    return AY_ENULL;

  ipatch = (ay_ipatch_object *)o->refine;

  if(!ipatch)
    return AY_ENULL;

  if(ipatch->display_mode != 0)
    {
      display_mode = ipatch->display_mode-1;
    }

  /* draw just the control polygon? */
  if(display_mode == 0)
    {
      /* Yes */
      ay_ipatch_drawcp(ipatch);
    }
  else
    {
      /* No, draw the NURBS patch(es), if present */
      p = ipatch->npatch;
      while(p)
	{
	  ay_draw_object(togl, p, AY_FALSE);
	  p = p->next;
	} /* while */
    } /* if */

  /* draw the derivatives */
  if(ipatch->derivs_u > 1 || ipatch->derivs_v > 1)
    {
      ay_ipatch_drawders(ipatch);
    }

  if(ipatch->caps_and_bevels)
    {
      p = ipatch->caps_and_bevels;
      while(p)
	{
	  ay_draw_object(togl, p, AY_TRUE);
	  p = p->next;
	}
    }

 return AY_OK;
} /* ay_ipatch_drawcb */


/* ay_ipatch_shadecb:
 *  shade (display in an Ayam view window) callback function of ipatch object
 */
int
ay_ipatch_shadecb(struct Togl *togl, ay_object *o)
{
 ay_ipatch_object *ipatch = NULL;
 ay_object *p = NULL;

  if(!o)
    return AY_ENULL;

  ipatch = (ay_ipatch_object *)o->refine;

  if(!ipatch)
    return AY_ENULL;

  p = ipatch->npatch;
  while(p)
    {
      ay_shade_object(togl, p, AY_FALSE);
      p = p->next;
    } /* while */

  if(ipatch->caps_and_bevels)
    {
      p = ipatch->caps_and_bevels;
      while(p)
	{
	  ay_shade_object(togl, p, AY_FALSE);
	  p = p->next;
	}
    }

 return AY_OK;
} /* ay_ipatch_shadecb */


/* ay_ipatch_drawacb:
 *  draw handles (in an Ayam view window) callback function of ipatch object
 */
int
ay_ipatch_drawacb(struct Togl *togl, ay_object *o)
{
 int width = 0, height = 0;
 ay_ipatch_object *ipatch;
 GLdouble *ver = NULL;
 /*double point_size = ay_prefs.handle_size;*/

  if(!o)
    return AY_ENULL;

  ipatch = (ay_ipatch_object *) o->refine;

  if(!ipatch)
    return AY_ENULL;

  width = ipatch->width;
  height = ipatch->height;

  ver = ipatch->controlv;

  /* draw arrow */
  ay_draw_arrow(togl, &(ver[width*height*3-6]), &(ver[width*height*3-3]));

 return AY_OK;
} /* ay_ipatch_drawacb */


/* ay_ipatch_drawhcb:
 *  draw handles (in an Ayam view window) callback function of ipatch object
 */
int
ay_ipatch_drawhcb(struct Togl *togl, ay_object *o)
{
 int i;
 double *pnts;
 ay_ipatch_object *ipatch;

  if(!o)
    return AY_ENULL;

  ipatch = (ay_ipatch_object *) o->refine;

  if(!ipatch)
    return AY_ENULL;

  pnts = ipatch->controlv;

  /* draw points */
  glBegin(GL_POINTS);
   for(i = 0; i < (ipatch->width*ipatch->height); i++)
     {
       glVertex3dv((GLdouble *)pnts);
       pnts += 3;
     }
  glEnd();

  /* draw derivatives */
  if(ipatch->derivs_u > 1)
    {
      if(ipatch->sderiv_u)
	{
	  pnts = ipatch->sderiv_u;
	  glBegin(GL_POINTS);
	   for(i = 0; i < ipatch->height; i++)
	     {
	       glVertex3dv((GLdouble *)pnts);
	       pnts += 3;
	     }
	  glEnd();
	}
      if(ipatch->ederiv_u)
	{
	  pnts = ipatch->ederiv_u;
	  glBegin(GL_POINTS);
	   for(i = 0; i < ipatch->height; i++)
	     {
	       glVertex3dv((GLdouble *)pnts);
	       pnts += 3;
	     }
	  glEnd();
	}
    } /* if */

  if(ipatch->derivs_v > 1)
    {
      if(ipatch->sderiv_v)
	{
	  pnts = ipatch->sderiv_v;
	  glBegin(GL_POINTS);
	   for(i = 0; i < ipatch->width; i++)
	     {
	       glVertex3dv((GLdouble *)pnts);
	       pnts += 3;
	     }
	  glEnd();
	}
      if(ipatch->ederiv_v)
	{
	  pnts = ipatch->ederiv_v;
	  glBegin(GL_POINTS);
	   for(i = 0; i < ipatch->width; i++)
	     {
	       glVertex3dv((GLdouble *)pnts);
	       pnts += 3;
	     }
	  glEnd();
	}
    } /* if */

 return AY_OK;
} /* ay_ipatch_drawhcb */


/* ay_ipatch_getpntcb:
 *  get point (editing and selection) callback function of ipatch object
 */
int
ay_ipatch_getpntcb(int mode, ay_object *o, double *p, ay_pointedit *pe)
{
 ay_ipatch_object *ipatch = NULL;
 ay_point *pnt = NULL, **lastpnt = NULL;
 double min_dist = ay_prefs.pick_epsilon, dist = 0.0;
 double *pecoord = NULL, **pecoords = NULL, **pecoordstmp;
 double *control = NULL, *c;
 int i = 0, j = 0, a = 0, found = AY_FALSE;

  if(!o || ((mode != 3) && (!p || !pe)))
    return AY_ENULL;

  ipatch = (ay_ipatch_object *)(o->refine);

  if(!ipatch)
    return AY_ENULL;

  if(min_dist == 0.0)
    min_dist = DBL_MAX;

  if(pe)
    pe->rational = AY_FALSE;

  switch(mode)
    {
    case 0:
      /* select all points */
      pe->num = ipatch->width * ipatch->height;

      if(ipatch->derivs_u && ipatch->sderiv_u && ipatch->ederiv_u)
	{
	  pe->num += 2*ipatch->height;
	}

      if(ipatch->derivs_v && ipatch->sderiv_v && ipatch->ederiv_v)
	{
	  pe->num += 2*ipatch->width;
	}

      if(!(pe->coords = calloc(pe->num, sizeof(double*))))
	return AY_EOMEM;

      for(i = 0; i < (ipatch->width*ipatch->height); i++)
	{
	  pe->coords[j] = &(ipatch->controlv[a]);
	  j++;
	  a += 3;
	}

      if(ipatch->derivs_u && ipatch->sderiv_u && ipatch->ederiv_u)
	{
	  a = 0;
	  for(i = 0; i < ipatch->height; i++)
	    {
	      pe->coords[j] = &(ipatch->sderiv_u[a]);
	      j++;
	      a += 3;
	    }
	  a = 0;
	  for(i = 0; i < ipatch->height; i++)
	    {
	      pe->coords[j] = &(ipatch->ederiv_u[a]);
	      j++;
	      a += 3;
	    }
	}

      if(ipatch->derivs_v && ipatch->sderiv_v && ipatch->ederiv_v)
	{
	  a = 0;
	  for(i = 0; i < ipatch->width; i++)
	    {
	      pe->coords[j] = &(ipatch->sderiv_v[a]);
	      j++;
	      a += 3;
	    }
	  a = 0;
	  for(i = 0; i < ipatch->width; i++)
	    {
	      pe->coords[j] = &(ipatch->ederiv_v[a]);
	      j++;
	      a += 3;
	    }
	}
      break;
    case 1:
      /* selection based on a single point */
      control = ipatch->controlv;

      for(i = 0; i < (ipatch->width * ipatch->height); i++)
	{
	  dist = AY_VLEN((p[0] - control[j]),
			 (p[1] - control[j+1]),
			 (p[2] - control[j+2]));

	  if(dist < min_dist)
	    {
	      pecoord = &(control[j]);
	      min_dist = dist;
	    }

	  j += 3;
	}

      if(ipatch->derivs_u && ipatch->sderiv_u && ipatch->ederiv_u)
	{
	  j = 0;
	  for(i = 0; i < ipatch->height; i++)
	    {
	      dist = AY_VLEN((p[0] - ipatch->sderiv_u[j]),
			     (p[1] - ipatch->sderiv_u[j+1]),
			     (p[2] - ipatch->sderiv_u[j+2]));

	      if(dist < min_dist)
		{
		  pecoord = &(ipatch->sderiv_u[j]);
		  min_dist = dist;
		}

	      j += 3;
	    } /* for */
	  j = 0;
	  for(i = 0; i < ipatch->height; i++)
	    {
	      dist = AY_VLEN((p[0] - ipatch->ederiv_u[j]),
			     (p[1] - ipatch->ederiv_u[j+1]),
			     (p[2] - ipatch->ederiv_u[j+2]));

	      if(dist < min_dist)
		{
		  pecoord = &(ipatch->ederiv_u[j]);
		  min_dist = dist;
		}
	      j += 3;
	    } /* for */
	} /* if */

      if(ipatch->derivs_v && ipatch->sderiv_v && ipatch->ederiv_v)
	{
	  j = 0;
	  for(i = 0; i < ipatch->width; i++)
	    {
	      dist = AY_VLEN((p[0] - ipatch->sderiv_v[j]),
			     (p[1] - ipatch->sderiv_v[j+1]),
			     (p[2] - ipatch->sderiv_v[j+2]));

	      if(dist < min_dist)
		{
		  pecoord = &(ipatch->sderiv_v[j]);
		  min_dist = dist;
		}

	      j += 3;
	    } /* for */
	  j = 0;
	  for(i = 0; i < ipatch->width; i++)
	    {
	      dist = AY_VLEN((p[0] - ipatch->ederiv_v[j]),
			     (p[1] - ipatch->ederiv_v[j+1]),
			     (p[2] - ipatch->ederiv_v[j+2]));

	      if(dist < min_dist)
		{
		  pecoord = &(ipatch->ederiv_v[j]);
		  min_dist = dist;
		}
	      j += 3;
	    } /* for */
	} /* if */


      if(!pecoord)
	return AY_OK; /* XXXX should this return a 'AY_EPICK' ? */

      if(!found)
	{
	  if(!(pe->coords = calloc(1, sizeof(double*))))
	    return AY_EOMEM;

	  pe->coords[0] = pecoord;
	  pe->num = 1;
	}
      break;
    case 2:
      /* selection based on planes */
      control = ipatch->controlv;
      j = 0;
      a = 0;
      for(i = 0; i < ipatch->width * ipatch->height; i++)
	{
	  c = &(control[j]);

	  /* test point c against the four planes in p */
	  if(((p[0]*c[0] + p[1]*c[1] + p[2]*c[2] + p[3]) < 0.0) &&
	     ((p[4]*c[0] + p[5]*c[1] + p[6]*c[2] + p[7]) < 0.0) &&
	     ((p[8]*c[0] + p[9]*c[1] + p[10]*c[2] + p[11]) < 0.0) &&
	     ((p[12]*c[0] + p[13]*c[1] + p[14]*c[2] + p[15]) < 0.0))
	    {
	      if(!(pecoordstmp = realloc(pecoords, (a+1)*sizeof(double *))))
		{
		  if(pecoords)
		    free(pecoords);
		  return AY_EOMEM;
		}
	      pecoords = pecoordstmp;

	      pecoords[a] = c;
	      a++;
	    } /* if */

	  j += 3;
	} /* for */

      if(ipatch->derivs_u && ipatch->sderiv_u && ipatch->ederiv_u)
	{
	  j = 0;
	  for(i = 0; i < ipatch->height; i++)
	    {
	      c = &(ipatch->sderiv_u[j]);

	      /* test point c against the four planes in p */
	      if(((p[0]*c[0] + p[1]*c[1] + p[2]*c[2] + p[3]) < 0.0) &&
		 ((p[4]*c[0] + p[5]*c[1] + p[6]*c[2] + p[7]) < 0.0) &&
		 ((p[8]*c[0] + p[9]*c[1] + p[10]*c[2] + p[11]) < 0.0) &&
		 ((p[12]*c[0] + p[13]*c[1] + p[14]*c[2] + p[15]) < 0.0))
		{
		  if(!(pecoordstmp = realloc(pecoords, (a+1)*sizeof(double *))))
		    {
		      if(pecoords)
			free(pecoords);
		      return AY_EOMEM;
		    }
		  pecoords = pecoordstmp;

		  pecoords[a] = c;
		  a++;
		} /* if */
	      j += 3;
	    } /* for */
	  j = 0;
	  for(i = 0; i < ipatch->height; i++)
	    {
	      c = &(ipatch->ederiv_u[j]);

	      /* test point c against the four planes in p */
	      if(((p[0]*c[0] + p[1]*c[1] + p[2]*c[2] + p[3]) < 0.0) &&
		 ((p[4]*c[0] + p[5]*c[1] + p[6]*c[2] + p[7]) < 0.0) &&
		 ((p[8]*c[0] + p[9]*c[1] + p[10]*c[2] + p[11]) < 0.0) &&
		 ((p[12]*c[0] + p[13]*c[1] + p[14]*c[2] + p[15]) < 0.0))
		{
		  if(!(pecoordstmp = realloc(pecoords, (a+1)*sizeof(double *))))
		    {
		      if(pecoords)
			free(pecoords);
		      return AY_EOMEM;
		    }
		  pecoords = pecoordstmp;

		  pecoords[a] = c;
		  a++;
		} /* if */
	      j += 3;
	    } /* for */
	} /* if */

      if(ipatch->derivs_v && ipatch->sderiv_v && ipatch->ederiv_v)
	{
	  j = 0;
	  for(i = 0; i < ipatch->width; i++)
	    {
	      c = &(ipatch->sderiv_v[j]);

	      /* test point c against the four planes in p */
	      if(((p[0]*c[0] + p[1]*c[1] + p[2]*c[2] + p[3]) < 0.0) &&
		 ((p[4]*c[0] + p[5]*c[1] + p[6]*c[2] + p[7]) < 0.0) &&
		 ((p[8]*c[0] + p[9]*c[1] + p[10]*c[2] + p[11]) < 0.0) &&
		 ((p[12]*c[0] + p[13]*c[1] + p[14]*c[2] + p[15]) < 0.0))
		{

		  if(!(pecoordstmp = realloc(pecoords, (a+1)*sizeof(double *))))
		    {
		      if(pecoords)
			free(pecoords);
		      return AY_EOMEM;
		    }
		  pecoords = pecoordstmp;

		  pecoords[a] = c;
		  a++;
		} /* if */
	      j += 3;
	    } /* for */
	  j = 0;
	  for(i = 0; i < ipatch->width; i++)
	    {
	      c = &(ipatch->ederiv_v[j]);

	      /* test point c against the four planes in p */
	      if(((p[0]*c[0] + p[1]*c[1] + p[2]*c[2] + p[3]) < 0.0) &&
		 ((p[4]*c[0] + p[5]*c[1] + p[6]*c[2] + p[7]) < 0.0) &&
		 ((p[8]*c[0] + p[9]*c[1] + p[10]*c[2] + p[11]) < 0.0) &&
		 ((p[12]*c[0] + p[13]*c[1] + p[14]*c[2] + p[15]) < 0.0))
		{
		  if(!(pecoordstmp = realloc(pecoords, (a+1)*sizeof(double *))))
		    {
		      if(pecoords)
			free(pecoords);
		      return AY_EOMEM;
		    }
		  pecoords = pecoordstmp;

		  pecoords[a] = c;
		  a++;
		} /* if */
	      j += 3;
	    } /* for */
	} /* if */

      if(!pecoords)
	return AY_OK; /* XXXX should this return a 'AY_EPICK' ? */

      pe->coords = pecoords;
      pe->num = a;

      break;
    case 3:
      /* rebuild from o->selp */
      pnt = o->selp;
      lastpnt = &o->selp;
      while(pnt)
	{
	  if(pnt->index < (unsigned int)(ipatch->width * ipatch->height))
	    {
	      pnt->point = &(ipatch->controlv[pnt->index*3]);
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
	} /* while */
      break;
    default:
      break;
    } /* switch */

 return AY_OK;
} /* ay_ipatch_getpntcb */


/* ay_ipatch_setpropcb:
 *  set property (from Tcl to C context) callback function of ipatch object
 */
int
ay_ipatch_setpropcb(Tcl_Interp *interp, int argc, char *argv[], ay_object *o)
{
 int ay_status = AY_OK, ay_status_s = AY_OK, ay_status_e = AY_OK;
 char *n1 = "IPatchAttrData";
 char fname[] = "ipatch_setpropcb";
 char rszerr[] = "Could not resize the patch.";
 char rszerrd[] = "Could not resize the derivative(s).";
 Tcl_Obj *to = NULL, *toa = NULL, *ton = NULL;
 ay_ipatch_object *ipatch = NULL;
 int new_ktype_u, new_close_u, new_order_u, new_width;
 int new_ktype_v, new_close_v, new_order_v, new_height;
 int i, update = AY_FALSE;

  if(!interp || !o)
    return AY_ENULL;

  ipatch = (ay_ipatch_object *)o->refine;

  if(!ipatch)
    return AY_ENULL;

  toa = Tcl_NewStringObj(n1,-1);
  ton = Tcl_NewStringObj(n1,-1);

  Tcl_SetStringObj(ton,"Width",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp,to, &new_width);

  Tcl_SetStringObj(ton,"Height",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp,to, &new_height);

  Tcl_SetStringObj(ton,"Close_U",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp,to, &new_close_u);

  Tcl_SetStringObj(ton,"Close_V",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp,to, &new_close_v);

  Tcl_SetStringObj(ton,"Order_U",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp,to, &new_order_u);

  Tcl_SetStringObj(ton,"Order_V",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp,to, &new_order_v);

  Tcl_SetStringObj(ton,"Knot-Type_U",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp,to, &new_ktype_u);

  Tcl_SetStringObj(ton,"Knot-Type_V",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp,to, &new_ktype_v);

  Tcl_SetStringObj(ton,"Derivatives_U",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp,to, &ipatch->derivs_u);

  Tcl_SetStringObj(ton,"SDLen_U",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetDoubleFromObj(interp,to, &(ipatch->sdlen_u));

  Tcl_SetStringObj(ton,"EDLen_U",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetDoubleFromObj(interp,to, &(ipatch->edlen_u));

  Tcl_SetStringObj(ton,"Derivatives_V",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp,to, &ipatch->derivs_v);

  Tcl_SetStringObj(ton,"SDLen_V",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetDoubleFromObj(interp,to, &(ipatch->sdlen_v));

  Tcl_SetStringObj(ton,"EDLen_V",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetDoubleFromObj(interp,to, &(ipatch->edlen_v));

  if((new_order_u != ipatch->order_u)||
     (new_order_v != ipatch->order_v)||
     (new_close_u != ipatch->close_u)||
     (new_close_v != ipatch->close_v)||
     (new_ktype_u != ipatch->ktype_u)||
     (new_ktype_v != ipatch->ktype_v))
    update = AY_TRUE;

  switch(new_ktype_u)
    {
    case 0:
      ipatch->ktype_u = AY_KTCHORDAL;
      break;
    case 1:
      ipatch->ktype_u = AY_KTCENTRI;
      break;
    case 2:
      ipatch->ktype_u = AY_KTUNIFORM;
      break;
    default:
      /* output error? */
      break;
    }

  switch(new_ktype_v)
    {
    case 0:
      ipatch->ktype_v = AY_KTCHORDAL;
      break;
    case 1:
      ipatch->ktype_v = AY_KTCENTRI;
      break;
    case 2:
      ipatch->ktype_v = AY_KTUNIFORM;
      break;
    default:
      /* output error? */
      break;
    }

  ipatch->close_u = new_close_u;
  ipatch->close_v = new_close_v;

  Tcl_SetStringObj(ton,"Tolerance",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetDoubleFromObj(interp,to, &(ipatch->glu_sampling_tolerance));

  Tcl_SetStringObj(ton,"DisplayMode",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp,to, &(ipatch->display_mode));

  Tcl_SetStringObj(ton,"BevelsChanged",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp,to, &i);
  if(i)
    {
      update = AY_TRUE;

      to = Tcl_NewIntObj(0);
      Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
    }

  Tcl_SetStringObj(ton,"CapsChanged",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp,to, &i);
  if(i)
    {
      update = AY_TRUE;

      to = Tcl_NewIntObj(0);
      Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
    }

  Tcl_IncrRefCount(toa);Tcl_DecrRefCount(toa);
  Tcl_IncrRefCount(ton);Tcl_DecrRefCount(ton);

  /* apply changed values to patch */

  /* resize patch */
  if(new_width != ipatch->width && (new_width > 1))
    {
      if(o->selp)
	{
	  ay_selp_clear(o);
	}

      ay_status = ay_npt_resizearrayw(&(ipatch->controlv), 3,
				      ipatch->width, ipatch->height,
				      new_width);

      if(ipatch->sderiv_v)
	{
	  ay_status_s = ay_npt_resizearrayw(&(ipatch->sderiv_v), 3,
					    ipatch->width, 1,
					    new_width);
	}

      if(ipatch->ederiv_v)
	{
	  ay_status_e = ay_npt_resizearrayw(&(ipatch->ederiv_v), 3,
					    ipatch->width, 1,
					    new_width);
	}

      if(ay_status)
	{
	  ay_error(AY_ERROR, fname, rszerr);
	}
      else
	{
	  if(ay_status_s || ay_status_e)
	    {
	      ay_error(AY_ERROR, fname, rszerrd);
	      if(ipatch->sderiv_v)
		free(ipatch->sderiv_v);
	      ipatch->sderiv_v = NULL;
	      if(ipatch->ederiv_v)
		free(ipatch->ederiv_v);
	      ipatch->ederiv_v = NULL;
	    }
	  ipatch->width = new_width;
	  update = AY_TRUE;
	} /* if */
    } /* if */

  if(new_height != ipatch->height && (new_height > 1))
    {
      if(o->selp)
	{
	  ay_selp_clear(o);
	}

      ay_status = ay_npt_resizearrayh(&(ipatch->controlv), 3,
				      ipatch->width, ipatch->height,
				      new_height);

      if(ipatch->sderiv_u)
	{
	  ay_status_s = ay_npt_resizearrayw(&(ipatch->sderiv_u), 3,
					    ipatch->height, 1,
					    new_height);
	}

      if(ipatch->ederiv_u)
	{
	  ay_status_e = ay_npt_resizearrayw(&(ipatch->ederiv_u), 3,
					    ipatch->height, 1,
					    new_height);
	}

      if(ay_status)
	{
	  ay_error(AY_ERROR, fname, rszerr);
	}
      else
	{
	  if(ay_status_s || ay_status_e)
	    {
	      ay_error(AY_ERROR, fname, rszerrd);
	      if(ipatch->sderiv_u)
		free(ipatch->sderiv_u);
	      ipatch->sderiv_u = NULL;
	      if(ipatch->ederiv_u)
		free(ipatch->ederiv_u);
	      ipatch->ederiv_u = NULL;
	    }
	  ipatch->height = new_height;
	  update = AY_TRUE;
	} /* if */
    } /* if */

  ipatch->order_u = new_order_u;
  ipatch->order_v = new_order_v;

  if(ipatch->order_u < 2)
    {
      ipatch->order_u = 2;
    }

  if(ipatch->order_u > ipatch->width)
    {
      ipatch->order_u = (ipatch->width < 4)?ipatch->width:4;
    }

  if(ipatch->order_v < 2)
    {
      ipatch->order_v = 2;
    }

  if(ipatch->order_v > ipatch->height)
    {
      ipatch->order_v = (ipatch->height < 4)?ipatch->height:4;
    }

  /* create end derivative arrays */
  if(ipatch->derivs_u)
    {
      ay_status_s = AY_OK;
      ay_status_e = AY_OK;
      if(!ipatch->sderiv_u)
	{
	  ay_status_s = ay_ipt_crtderiv(0, ipatch);
	}
      if(!ipatch->ederiv_u)
	{
	  ay_status_e = ay_ipt_crtderiv(1, ipatch);
	}
      if(ay_status_s || ay_status_e)
	{
	  if(ipatch->sderiv_u)
	    free(ipatch->sderiv_u);
	  ipatch->sderiv_u = NULL;
	  if(ipatch->ederiv_u)
	    free(ipatch->ederiv_u);
	  ipatch->derivs_u = AY_FALSE;
	}
    } /* if */

  if(ipatch->derivs_v)
    {
      ay_status_s = AY_OK;
      ay_status_e = AY_OK;
      if(!ipatch->sderiv_v)
	{
	  ay_status_s = ay_ipt_crtderiv(2, ipatch);
	}
      if(!ipatch->ederiv_v)
	{
	  ay_status_e = ay_ipt_crtderiv(3, ipatch);
	}
      if(ay_status_s || ay_status_e)
	{
	  if(ipatch->sderiv_v)
	    free(ipatch->sderiv_v);
	  ipatch->sderiv_v = NULL;
	  if(ipatch->ederiv_v)
	    free(ipatch->ederiv_v);
	  ipatch->ederiv_v = NULL;
	  ipatch->derivs_v = AY_FALSE;
	}
    } /* if */

  if(update)
    {
      ay_notify_object(o);

      o->modified = AY_TRUE;

      ay_notify_parent();
    }

 return AY_OK;
} /* ay_ipatch_setpropcb */


/* ay_ipatch_getpropcb:
 *  get property (from C to Tcl context) callback function of ipatch object
 */
int
ay_ipatch_getpropcb(Tcl_Interp *interp, int argc, char *argv[], ay_object *o)
{
 char *n1="IPatchAttrData";
 Tcl_Obj *to = NULL, *toa = NULL, *ton = NULL;
 ay_ipatch_object *ipatch = NULL;

  if(!interp || !o)
    return AY_ENULL;

  ipatch = (ay_ipatch_object *)(o->refine);

  if(!ipatch)
    return AY_ENULL;

  toa = Tcl_NewStringObj(n1,-1);
  ton = Tcl_NewStringObj(n1,-1);

  Tcl_SetStringObj(ton,"Width",-1);
  to = Tcl_NewIntObj(ipatch->width);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton,"Height",-1);
  to = Tcl_NewIntObj(ipatch->height);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton,"Close_U",-1);
  to = Tcl_NewIntObj(ipatch->close_u);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton,"Close_V",-1);
  to = Tcl_NewIntObj(ipatch->close_v);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton,"Order_U",-1);
  to = Tcl_NewIntObj(ipatch->order_u);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton,"Order_V",-1);
  to = Tcl_NewIntObj(ipatch->order_v);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton,"Knot-Type_U",-1);
  if(ipatch->ktype_u == AY_KTCHORDAL)
    to = Tcl_NewIntObj(0);
  else
    if(ipatch->ktype_u == AY_KTCENTRI)
      to = Tcl_NewIntObj(1);
    else
      to = Tcl_NewIntObj(2);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);
  Tcl_SetStringObj(ton,"Knot-Type_V",-1);
  if(ipatch->ktype_v == AY_KTCHORDAL)
    to = Tcl_NewIntObj(0);
  else
    if(ipatch->ktype_v == AY_KTCENTRI)
      to = Tcl_NewIntObj(1);
    else
      to = Tcl_NewIntObj(2);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton,"Derivatives_U",-1);
  to = Tcl_NewIntObj(ipatch->derivs_u);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton,"SDLen_U",-1);
  to = Tcl_NewDoubleObj(ipatch->sdlen_u);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton,"EDLen_U",-1);
  to = Tcl_NewDoubleObj(ipatch->edlen_u);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton,"Derivatives_V",-1);
  to = Tcl_NewIntObj(ipatch->derivs_v);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton,"SDLen_V",-1);
  to = Tcl_NewDoubleObj(ipatch->sdlen_v);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton,"EDLen_V",-1);
  to = Tcl_NewDoubleObj(ipatch->edlen_v);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton,"Tolerance",-1);
  to = Tcl_NewDoubleObj(ipatch->glu_sampling_tolerance);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton,"DisplayMode",-1);
  to = Tcl_NewIntObj(ipatch->display_mode);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton,"BevelsChanged",-1);
  to = Tcl_NewIntObj(0);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton,"CapsChanged",-1);
  to = Tcl_NewIntObj(0);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);

  ay_prop_getnpinfo(interp, n1, ipatch->npatch);

  Tcl_IncrRefCount(toa);Tcl_DecrRefCount(toa);
  Tcl_IncrRefCount(ton);Tcl_DecrRefCount(ton);

 return AY_OK;
} /* ay_ipatch_getpropcb */


/* ay_ipatch_readcb:
 *  read (from scene file) callback function of ipatch object
 */
int
ay_ipatch_readcb(FILE *fileptr, ay_object *o)
{
 int ay_status = AY_OK;
 ay_ipatch_object *ipatch = NULL;
 int i, a;

  if(!fileptr || !o)
   return AY_ENULL;

  if(!(ipatch = calloc(1, sizeof(ay_ipatch_object))))
    { return AY_EOMEM; }

  fscanf(fileptr,"%d\n",&ipatch->width);
  fscanf(fileptr,"%d\n",&ipatch->height);
  fscanf(fileptr,"%d\n",&ipatch->close_u);
  fscanf(fileptr,"%d\n",&ipatch->close_v);
  fscanf(fileptr,"%d\n",&ipatch->order_u);
  fscanf(fileptr,"%d\n",&ipatch->order_v);
  fscanf(fileptr,"%d\n",&ipatch->ktype_u);
  fscanf(fileptr,"%d\n",&ipatch->ktype_v);
  fscanf(fileptr,"%lg\n",&ipatch->sdlen_u);
  fscanf(fileptr,"%lg\n",&ipatch->edlen_u);
  fscanf(fileptr,"%lg\n",&ipatch->sdlen_v);
  fscanf(fileptr,"%lg\n",&ipatch->edlen_v);

  if(!(ipatch->controlv = calloc(ipatch->width*ipatch->height*3,
				 sizeof(double))))
    {
      ay_status = AY_EOMEM;
      goto cleanup;
    }

  a = 0;
  for(i = 0; i < ipatch->width*ipatch->height; i++)
    {
      fscanf(fileptr, "%lg %lg %lg\n",
	     &(ipatch->controlv[a]),
	     &(ipatch->controlv[a+1]),
	     &(ipatch->controlv[a+2]));
      a += 3;
    }

  fscanf(fileptr,"%d\n",&(ipatch->derivs_u));
  if(ipatch->derivs_u)
    {
      if(!(ipatch->sderiv_u = calloc(ipatch->height*3, sizeof(double))))
	{
	  ay_status = AY_EOMEM;
	  goto cleanup;
	}

      a = 0;
      for(i = 0; i < ipatch->height; i++)
	{
	  fscanf(fileptr, "%lg %lg %lg\n",
		 &(ipatch->sderiv_u[a]),
		 &(ipatch->sderiv_u[a+1]),
		 &(ipatch->sderiv_u[a+2]));
	  a += 3;
	}

      if(!(ipatch->ederiv_u = calloc(ipatch->height*3, sizeof(double))))
	{
	  ay_status = AY_EOMEM;
	  goto cleanup;
	}

      a = 0;
      for(i = 0; i < ipatch->height; i++)
	{
	  fscanf(fileptr, "%lg %lg %lg\n",
		 &(ipatch->ederiv_u[a]),
		 &(ipatch->ederiv_u[a+1]),
		 &(ipatch->ederiv_u[a+2]));
	  a += 3;
	}
    }

  fscanf(fileptr,"%d\n",&(ipatch->derivs_v));
  if(ipatch->derivs_v)
    {
      if(!(ipatch->sderiv_v = calloc(ipatch->width*3, sizeof(double))))
	{
	  ay_status = AY_EOMEM;
	  goto cleanup;
	}

      a = 0;
      for(i = 0; i < ipatch->width; i++)
	{
	  fscanf(fileptr, "%lg %lg %lg\n",
		 &(ipatch->sderiv_v[a]),
		 &(ipatch->sderiv_v[a+1]),
		 &(ipatch->sderiv_v[a+2]));
	  a += 3;
	}

      if(!(ipatch->ederiv_v = calloc(ipatch->width*3, sizeof(double))))
	{
	  ay_status = AY_EOMEM;
	  goto cleanup;
	}

      a = 0;
      for(i = 0; i < ipatch->width; i++)
	{
	  fscanf(fileptr, "%lg %lg %lg\n",
		 &(ipatch->ederiv_v[a]),
		 &(ipatch->ederiv_v[a+1]),
		 &(ipatch->ederiv_v[a+2]));
	  a += 3;
	}
    }

  fscanf(fileptr,"%lg\n",&(ipatch->glu_sampling_tolerance));
  fscanf(fileptr,"%d\n",&(ipatch->display_mode));

  o->refine = ipatch;

  /* prevent cleanup code from doing something harmful */
  ipatch = NULL;

cleanup:

  if(ipatch)
    {
      if(ipatch->controlv)
	free(ipatch->controlv);

      if(ipatch->sderiv_u)
	free(ipatch->sderiv_u);
      if(ipatch->ederiv_u)
	free(ipatch->ederiv_u);
      if(ipatch->sderiv_v)
	free(ipatch->sderiv_v);
      if(ipatch->ederiv_v)
	free(ipatch->ederiv_v);

      free(ipatch);
    }

 return ay_status;
} /* ay_ipatch_readcb */


/* ay_ipatch_writecb:
 *  write (to scene file) callback function of ipatch object
 */
int
ay_ipatch_writecb(FILE *fileptr, ay_object *o)
{
 ay_ipatch_object *ipatch = NULL;
 int i, a;

  if(!fileptr || !o)
    return AY_ENULL;

  ipatch = (ay_ipatch_object *)(o->refine);

  if(!ipatch)
    return AY_ENULL;

  fprintf(fileptr, "%d\n", ipatch->width);
  fprintf(fileptr, "%d\n", ipatch->height);
  fprintf(fileptr, "%d\n", ipatch->close_u);
  fprintf(fileptr, "%d\n", ipatch->close_v);
  fprintf(fileptr, "%d\n", ipatch->order_u);
  fprintf(fileptr, "%d\n", ipatch->order_v);
  fprintf(fileptr, "%d\n", ipatch->ktype_u);
  fprintf(fileptr, "%d\n", ipatch->ktype_v);
  fprintf(fileptr, "%g\n", ipatch->sdlen_u);
  fprintf(fileptr, "%g\n", ipatch->edlen_u);
  fprintf(fileptr, "%g\n", ipatch->sdlen_v);
  fprintf(fileptr, "%g\n", ipatch->edlen_v);

  a = 0;
  for(i = 0; i < ipatch->width*ipatch->height; i++)
    {
      fprintf(fileptr,"%g %g %g\n", ipatch->controlv[a],
	      ipatch->controlv[a+1],
	      ipatch->controlv[a+2]);
      a += 3;
    }

  fprintf(fileptr, "%d\n", ipatch->derivs_u);
  if(ipatch->derivs_u)
    {
      a = 0;
      for(i = 0; i < ipatch->height; i++)
	{
	  fprintf(fileptr,"%g %g %g\n", ipatch->sderiv_u[a],
		  ipatch->sderiv_u[a+1],
		  ipatch->sderiv_u[a+2]);
	  a += 3;
	}
      a = 0;
      for(i = 0; i < ipatch->height; i++)
	{
	  fprintf(fileptr,"%g %g %g\n", ipatch->ederiv_u[a],
		  ipatch->ederiv_u[a+1],
		  ipatch->ederiv_u[a+2]);
	  a += 3;
	}
    }

  fprintf(fileptr, "%d\n", ipatch->derivs_v);
  if(ipatch->derivs_v)
    {
      a = 0;
      for(i = 0; i < ipatch->width; i++)
	{
	  fprintf(fileptr,"%g %g %g\n", ipatch->sderiv_v[a],
		  ipatch->sderiv_v[a+1],
		  ipatch->sderiv_v[a+2]);
	  a += 3;
	}
      a = 0;
      for(i = 0; i < ipatch->width; i++)
	{
	  fprintf(fileptr,"%g %g %g\n", ipatch->ederiv_v[a],
		  ipatch->ederiv_v[a+1],
		  ipatch->ederiv_v[a+2]);
	  a += 3;
	}
    }

  fprintf(fileptr, "%g\n", ipatch->glu_sampling_tolerance);
  fprintf(fileptr, "%d\n", ipatch->display_mode);

 return AY_OK;
} /* ay_ipatch_writecb */


/* ay_ipatch_wribcb:
 *  RIB export callback function of ipatch object
 */
int
ay_ipatch_wribcb(char *file, ay_object *o)
{
 int ay_status = AY_OK;
 ay_ipatch_object *ipatch = NULL;
 ay_object *p;

  if(!o)
    return AY_ENULL;

  ipatch = (ay_ipatch_object*)(o->refine);

  if(!ipatch)
    return AY_ENULL;

  p = ipatch->npatch;
  while(p)
    {
      ay_wrib_object(file, p);
      p = p->next;
    }

  p = ipatch->caps_and_bevels;
  while(p)
    {
      ay_wrib_object(file, p);
      p = p->next;
    }

 return ay_status;
} /* ay_ipatch_wribcb */


/* ay_ipatch_bbccb:
 *  bounding box calculation callback function of ipatch object
 */
int
ay_ipatch_bbccb(ay_object *o, double *bbox, int *flags)
{
 ay_ipatch_object *ipatch = NULL;

  if(!o || !bbox || !flags)
    return AY_ENULL;

  ipatch = (ay_ipatch_object *)o->refine;

  if(!ipatch)
    return AY_ENULL;

 return ay_bbc_fromarr(ipatch->controlv, ipatch->width*ipatch->height,
		       3, bbox);
} /* ay_ipatch_bbccb */


/* ay_ipatch_notifycb:
 *  notification callback function of ipatch object
 */
int
ay_ipatch_notifycb(ay_object *o)
{
 int ay_status = AY_OK;
 int i, j, a, b, mode;
 ay_object *p = NULL;
 ay_ipatch_object *ip = NULL;
 ay_nurbpatch_object *np = NULL;
 ay_object *bevel = NULL, **nextcb;
 ay_bparam bparams = {0};
 ay_cparam cparams = {0};
 double tolerance, *cv = NULL;

  if(!o)
    return AY_ENULL;

  ip = (ay_ipatch_object *)o->refine;

  if(!ip)
    return AY_ENULL;

  mode = ip->display_mode;
  tolerance = ip->glu_sampling_tolerance;

  nextcb = &(ip->caps_and_bevels);

  if(ip->npatch)
    {
      (void)ay_object_deletemulti(ip->npatch, AY_FALSE);
      ip->npatch = NULL;
    }

  if(ip->caps_and_bevels)
    {
      (void)ay_object_deletemulti(ip->caps_and_bevels, AY_FALSE);
      ip->caps_and_bevels = NULL;
    }

  if(!(cv = calloc(ip->width*ip->height*4, sizeof(double))))
    return AY_EOMEM;

  a = 0;
  b = 0;
  for(i = 0; i < ip->width; i++)
    {
      for(j = 0; j < ip->height; j++)
	{
	  memcpy(&(cv[a]), &(ip->controlv[b]), 3*sizeof(double));

	  cv[a+3] = 1.0;

	  a += 4;
	  b += 3;
	}
    }

  ay_status = ay_npt_create(ip->order_u, ip->order_v, ip->width, ip->height,
			    AY_KTCUSTOM, AY_KTCUSTOM, cv, NULL, NULL, &np);

  if(ay_status)
    goto cleanup;

  ay_status = ay_npt_createnpatchobject(&p);

  if(ay_status || !p)
    goto cleanup;

  p->refine = (void*)np;

  if(ip->width > 2 && ip->order_u > 2)
    {
      /* interpolate along U */
      if(ip->close_u)
	{
	  /* if derivatives are not enabled, use automatic mode */
	  if(ip->derivs_u)
	    a = ip->derivs_u-1;
	  else
	    a = 0;
	  ay_status = ay_ipt_interpolateudc(np, ip->order_u, ip->ktype_u,
					    a, ip->sdlen_u, ip->edlen_u,
					    ip->sderiv_u, ip->ederiv_u);
	}
      else
	{
	  if(ip->derivs_u)
	    {
	      ay_status = ay_ipt_interpolateud(np, ip->order_u, ip->ktype_u,
		                    ip->derivs_u-1, ip->sdlen_u, ip->edlen_u,
						ip->sderiv_u, ip->ederiv_u);
	    }
	  else
	    {
	      ay_status = ay_ipt_interpolateu(np, ip->order_u, ip->ktype_u);
	    }
	}

      if(ay_status)
	goto cleanup;
    }
  else
    {
      np->uorder = 2;
      np->uknot_type = AY_KTNURB;
      ay_knots_createnp(np);
    } /* if */

  if(ip->height > 2 && ip->order_v > 2)
    {
      /* interpolate along V */
      if(ip->close_v)
	{
	  /* if derivatives are not enabled, use automatic mode */
	  if(ip->derivs_v)
	    a = ip->derivs_v-1;
	  else
	    a = 0;
	  ay_status = ay_ipt_interpolatevdc(np, ip->order_v, ip->ktype_v,
					    a, ip->sdlen_v, ip->edlen_v,
					    ip->sderiv_v, ip->ederiv_v);
	}
      else
	{
	  if(ip->derivs_v)
	    {
	      ay_status = ay_ipt_interpolatevd(np, ip->order_v, ip->ktype_v,
			           ip->derivs_v-1, ip->sdlen_v, ip->edlen_v,
					       ip->sderiv_v, ip->ederiv_v);
	    }
	  else
	    {
	      ay_status = ay_ipt_interpolatev(np, ip->order_v, ip->ktype_v);
	    }
	}

      if(ay_status)
	goto cleanup;
    }
  else
    {
      np->vorder = 2;
      np->vknot_type = AY_KTNURB;
      ay_knots_createnp(np);
    } /* if */

  ip->npatch = p;

  /* copy sampling tolerance/mode attributes to NURBS patch(es) */
  p = ip->npatch;
  while(p)
    {
      if(p->type == AY_IDNPATCH)
	{
	  ((ay_nurbpatch_object *)
	   (p->refine))->glu_sampling_tolerance = tolerance;
	  ((ay_nurbpatch_object *)
	   (p->refine))->display_mode = mode;
	}
      p = p->next;
    }

  /* get bevel and cap parameters */
  if(o->tags)
    {
      ay_bevelt_parsetags(o->tags, &bparams);
      ay_capt_parsetags(o->tags, &cparams);
    }

  /* create/add caps */
  if(cparams.has_caps)
    {
      ay_status = ay_capt_addcaps(&cparams, &bparams, ip->npatch, nextcb);
      if(ay_status)
	goto cleanup;

      while(*nextcb)
	nextcb = &((*nextcb)->next);
    }

  /* create/add bevels */
  if(bparams.has_bevels)
    {
      bparams.dirs[1] = !bparams.dirs[1];
      bparams.dirs[2] = !bparams.dirs[2];
      bparams.radii[2] = -bparams.radii[2];

      ay_status = ay_bevelt_addbevels(&bparams, &cparams, ip->npatch, nextcb);
      if(ay_status)
	goto cleanup;
    }

  bevel = ip->caps_and_bevels;
  while(bevel)
    {
      ((ay_nurbpatch_object *)
       (bevel->refine))->glu_sampling_tolerance = tolerance;
      ((ay_nurbpatch_object *)
       (bevel->refine))->display_mode = mode;
      bevel = bevel->next;
    }

  /* prevent cleanup code from doing something harmful */
  p = NULL;

cleanup:

  if(p)
    (void)ay_object_deletemulti(p, AY_FALSE);

 return ay_status;
} /* ay_ipatch_notifycb */


/* ay_ipatch_providecb:
 *  provide callback function of ipatch object
 */
int
ay_ipatch_providecb(ay_object *o, unsigned int type, ay_object **result)
{
 ay_ipatch_object *ip = NULL;

  if(!o)
    return AY_ENULL;

  ip = (ay_ipatch_object *) o->refine;

  if(!ip)
    return AY_ENULL;

 return ay_provide_nptoolobj(o, type, ip->npatch, ip->caps_and_bevels, result);
} /* ay_ipatch_providecb */


/* ay_ipatch_convertcb:
 *  convert callback function of ipatch object
 */
int
ay_ipatch_convertcb(ay_object *o, int in_place)
{
 ay_ipatch_object *ip = NULL;

  if(!o)
    return AY_ENULL;

  ip = (ay_ipatch_object *) o->refine;

  if(!ip)
    return AY_ENULL;

 return ay_convert_nptoolobj(o, ip->npatch, ip->caps_and_bevels, in_place);
} /* ay_ipatch_convertcb */


/* ay_ipatch_init:
 *  initialize the ipatch object module
 */
int
ay_ipatch_init(Tcl_Interp *interp)
{
 int ay_status = AY_OK;

  ay_status = ay_otype_registercore(ay_ipatch_name,
				    ay_ipatch_createcb,
				    ay_ipatch_deletecb,
				    ay_ipatch_copycb,
				    ay_ipatch_drawcb,
				    ay_ipatch_drawhcb,
				    ay_ipatch_shadecb,
				    ay_ipatch_setpropcb,
				    ay_ipatch_getpropcb,
				    ay_ipatch_getpntcb,
				    ay_ipatch_readcb,
				    ay_ipatch_writecb,
				    ay_ipatch_wribcb,
				    ay_ipatch_bbccb,
				    AY_IDIPATCH);

  ay_status += ay_draw_registerdacb(ay_ipatch_drawacb, AY_IDIPATCH);

  ay_status += ay_notify_register(ay_ipatch_notifycb, AY_IDIPATCH);

  ay_status += ay_convert_register(ay_ipatch_convertcb, AY_IDIPATCH);

  ay_status += ay_provide_register(ay_ipatch_providecb, AY_IDIPATCH);

 return ay_status;
} /* ay_ipatch_init */

