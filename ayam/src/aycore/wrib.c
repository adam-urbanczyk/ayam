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

/* wrib.c - RIB export */


/* prototypes of functions local to this module: */

int ay_wrib_sm(char *file, char *image, int width, int height);


/* functions: */

/* cot() used by currently unused FrameCamera() */
/*
double
cot (double in)
{
if (sin (in) == 0)
 return (0);
else
 return (cos (in) / sin (in));
}
*/

/* ay_wrib_aimz:
 *
 */
void
ay_wrib_aimz(RtPoint direction)
{
 RtFloat xzlen = (RtFloat)0.0, yzlen = (RtFloat)0.0, xrot = (RtFloat)0.0,
   yrot = (RtFloat)0.0;
 RtFloat tmp;
 
  if((direction[0] == 0.0) && (direction[1] == 0.0) && (direction[2] == 0.0))
    return;

  xzlen = (RtFloat)(sqrt(direction[0]*direction[0] +
			 direction[2]*direction[2]));
  if(xzlen == 0.0)
    {
      yrot = (direction[1] < 0.0) ? (RtFloat)180.0 : (RtFloat)0.0;
    }
  else
    {
      tmp = direction[2]/xzlen;

      if((tmp>1.0)||(tmp<-1.0))
	{
	  fprintf(stderr,"gotcha!\n");
	  fprintf(stderr,"%g %g\n",direction[0],direction[2]);
	}
      yrot = (RtFloat)AY_R2D(acos((fabs(tmp)<=1.0?tmp:(tmp<-1.0?-1.0:1.0))));
    }
 
  yzlen = (RtFloat)(sqrt(direction[1]*direction[1] + xzlen*xzlen));
  if(yzlen != 0.0)
    {
      xrot = (RtFloat)AY_R2D(acos(xzlen/yzlen));
    }
  else
    {
      xrot = (RtFloat)0;
    }

  if(direction[1] > 0.0)
    {
      if(fabs(xrot) > AY_EPSILON)
	RiRotate(xrot, (RtFloat)1.0,(RtFloat)0.0,(RtFloat)0.0);
    }
  else
    {
      if(fabs(xrot) > AY_EPSILON)
	RiRotate(-xrot, (RtFloat)1.0,(RtFloat)0.0,(RtFloat)0.0);
    }

  if(direction[0] < 0.0)
    {
      if(fabs(yrot) > AY_EPSILON)
	RiRotate(yrot, (RtFloat)0.0,(RtFloat)1.0,(RtFloat)0.0);
    }
  else
    {
      if(fabs(yrot) > AY_EPSILON)
	RiRotate(-yrot, (RtFloat)0.0,(RtFloat)1.0,(RtFloat)0.0);
    }

 return;
} /* ay_wrib_aimz */


/* ay_wrib_placecamera:
 *
 */
void
ay_wrib_placecamera(RtPoint position, RtPoint direction, double roll)
{
  /* XXXX RiIdentity(); */
  if(fabs(roll) > AY_EPSILON)
    RiRotate((RtFloat)-roll, (RtFloat)0.0, (RtFloat)0.0, (RtFloat)1.0);

  ay_wrib_aimz(direction);

  if((fabs(position[0]) > AY_EPSILON) || (fabs(position[1]) > AY_EPSILON) ||
     (fabs(position[2]) > AY_EPSILON))
    RiTranslate((fabs(position[0]) > AY_EPSILON) ? -position[0] :
		(RtFloat) 0.0,
		(fabs(position[1]) > AY_EPSILON) ? -position[1] :
		(RtFloat) 0.0,
		(fabs(position[2]) > AY_EPSILON) ? -position[2] :
		(RtFloat) 0.0);

 return;
} /* ay_wrib_placecamera */


/* unused ruin */
/* XXXX Debug me! */
/*void
FrameCamera(double zoom, double framewidth, double frameheight)
{
  RtFloat fov;
  RtFloat focallength;

  focallength = (0.5 * (framewidth<frameheight)?framewidth:frameheight)*
    cot(45.0);
  
  fprintf(stderr,"focallength:%g\n",focallength);

  fov = 2.0 * atan((((framewidth<frameheight)?framewidth:frameheight)
		    * zoom * 0.5) / focallength) * 180.0 / AY_PI;

  fprintf(stderr,"fov:%g\n",fov);
  RiProjection("perspective", RI_FOV, (RtPointer) &fov, RI_NULL);

  RiFrameAspectRatio((RtFloat)(framewidth/frameheight));

 return;
}*/


/* ay_wrib_rioptions:
 *
 */
int
ay_wrib_rioptions(void)
{
 int ay_status = AY_OK;
 ay_riopt_object *riopt = NULL;
 ay_root_object *root = NULL;
 RtInt fw = 0, fh = 0;
 RtFloat rtftemp = (RtFloat)0.0;
 RtInt rtitemp = 0;

  root = (ay_root_object*)(ay_root->refine);
  riopt = root->riopt;

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
    RiDeclare((RtToken)"minsamples", "integer");
    RiOption((RtToken)"render", (RtToken)"minsamples",
	     (RtPointer)(&rtitemp), RI_NULL);

    rtitemp = riopt->MaxSamples;
    RiDeclare((RtToken)"maxsamples", "integer");
    RiOption((RtToken)"render", (RtToken)"maxsamples",
	     (RtPointer)(&rtitemp), RI_NULL);
    
    rtitemp = riopt->MaxRayLevel;
    RiDeclare((RtToken)"max_raylevel", "integer");
    RiOption((RtToken)"render", (RtToken)"max_raylevel",
	     (RtPointer)(&rtitemp), RI_NULL);

    rtftemp = (RtFloat)riopt->ShadowBias;
    RiDeclare((RtToken)"minshadowbias", "float");
    RiOption((RtToken)"render", (RtToken)"minshadowbias",
	     (RtPointer)(&rtftemp), RI_NULL);

    rtitemp = riopt->PRManSpec;
    RiDeclare((RtToken)"prmanspecular", "integer");
    RiOption((RtToken)"render", (RtToken)"prmanspecular",
	     (RtPointer)(&rtitemp), RI_NULL);

    rtitemp = riopt->RadSteps;
    RiDeclare((RtToken)"steps", "integer");
    RiOption((RtToken)"radiosity", (RtToken)"steps",
	     (RtPointer)(&riopt->RadSteps), RI_NULL);

    rtitemp = riopt->PatchSamples;
    RiDeclare((RtToken)"minpatchsamples", "integer");
    RiOption((RtToken)"radiosity", (RtToken)"minpatchsamples",
	     (RtPointer)(&riopt->PatchSamples), RI_NULL);

    if(riopt->textures)
      if((riopt->textures)[0] != '\0')
	{
	  RiDeclare((RtToken)"texture", "string");
	  RiOption((RtToken)"searchpath", (RtToken)"texture",
		   (RtPointer)(&riopt->textures), RI_NULL);
	}

    if(riopt->includes)
      if((riopt->includes)[0] != '\0')
	{
	  RiDeclare((RtToken)"include", "string");
	  RiOption((RtToken)"searchpath", (RtToken)"include",
		   (RtPointer)(&riopt->includes), RI_NULL);
	}

    if(riopt->shaders)
      if((riopt->shaders)[0] != '\0')
	{
	  RiDeclare((RtToken)"shader", "string");
	  RiOption((RtToken)"searchpath", (RtToken)"shader",
		   (RtPointer)(&riopt->shaders), RI_NULL);
	}

    rtitemp = riopt->texturemem;
    RiDeclare((RtToken)"texturememory", "integer");
    RiOption((RtToken)"limits", (RtToken)"texturememory",
	     &rtitemp, RI_NULL);

    rtitemp = riopt->geommem;
    RiDeclare((RtToken)"geommemory", "integer");
    RiOption((RtToken)"limits", (RtToken)"geommemory",
	     &rtitemp, RI_NULL);
  }

  
 return ay_status;
} /* ay_wrib_rioptions */


/* ay_wrib_trafos:
 *
 */
int
ay_wrib_trafos(ay_object *o)
{
 int ay_status = AY_OK;
 double euler[3];

  if((fabs(o->movx) > AY_EPSILON) ||
     (fabs(o->movy) > AY_EPSILON) ||
     (fabs(o->movz) > AY_EPSILON))
    RiTranslate((RtFloat)o->movx, (RtFloat)o->movy, (RtFloat)o->movz);

  if(o->quat[0] != 0.0 || o->quat[1] != 0.0 || o->quat[2] != 0.0 ||
     o->quat[3] != 1.0)
     {
       ay_quat_toeuler(o->quat, euler);
       if(fabs(euler[2]) > AY_EPSILON)
	 RiRotate((RtFloat)AY_R2D(-euler[2]),
		  (RtFloat)1.0, (RtFloat)0.0, (RtFloat)0.0);
       if(fabs(euler[1]) > AY_EPSILON)
	 RiRotate((RtFloat)AY_R2D(-euler[1]),
		  (RtFloat)0.0, (RtFloat)1.0, (RtFloat)0.0);
       if(fabs(euler[0]) > AY_EPSILON)
	 RiRotate((RtFloat)AY_R2D(-euler[0]),
		  (RtFloat)0.0, (RtFloat)0.0, (RtFloat)1.0);
     }

  if(o->scalx != 1.0 || o->scaly != 1.0 || o->scalz != 1.0)
    {
      RiScale((RtFloat)o->scalx, (RtFloat)o->scaly, (RtFloat)o->scalz);
    }

 return ay_status;
} /* ay_wrib_trafos */


/* ay_wrib_refobject:
 *  wrib referenced object
 */
int
ay_wrib_refobject(char *file, ay_object *o)
{
 int ay_status = AY_OK;
 char *iafilename = NULL;
 int found = AY_FALSE;
 ay_tag_object *tag = NULL;

  tag = o->tags;
  while(tag && !found)
    {
      if(tag->type == ay_instt_oitagtype)
	{
	  RiTransformBegin();
	  ay_status = ay_wrib_trafos(o);

	  if(!file)
	    {
	      RiReadArchive(tag->val, (RtVoid*)RI_NULL, RI_NULL);
	    }
	  else
	    {
	      if(!(iafilename = calloc(1, strlen(tag->val)+strlen(file)+2)))
		return AY_EOMEM;
	      strcpy(iafilename,file);
	      iafilename[strlen(file)] = '-';
	      strcpy(&(iafilename[strlen(file)+1]), tag->val);
	      RiReadArchive(iafilename, (RtVoid*)RI_NULL, RI_NULL);
	      free(iafilename);
	    }
	  RiTransformEnd();
	  found = AY_TRUE;
	}
      tag = tag->next;
    }

  if(!found)
    return AY_ERROR; /* This should never happen! */

 return ay_status;
} /* ay_wrib_refobject */


/* ay_wrib_object:
 *
 */
int
ay_wrib_object(char *file, ay_object *o)
{
 int ay_status = AY_OK;
 ay_object *down = NULL;
 void **arr = NULL;
 ay_wribcb *cb = NULL;
 ay_level_object *l = NULL;
 char *parname = "name";

  if(!o)
    return AY_ENULL;

  if(ay_prefs.excludehidden && o->hide)
    return AY_OK;

  arr = ay_wribcbt.arr;
  cb = (ay_wribcb *)(arr[o->type]);

  if((!ay_prefs.resolveinstances) && (o->type != AY_IDMATERIAL) &&
     (o->refcount) && (cb))
    {
      
      ay_status = ay_wrib_refobject(file, o);

      return AY_OK; /* XXXX early exit */
    }

  if(cb)
    {
      /* write name */
      if(ay_prefs.writeident)
	{
	  if(o->name)
	    {
	      if(!ay_prefs.ristandard)
		{
		  RiDeclare(parname, "string");
		  RiAttribute("identifier", parname,
			      (RtPointer)&o->name, RI_NULL);
		  
		}
	      else
		{
		  RiArchiveRecord(RI_COMMENT, o->name);
		}
	    }
	}
      else
	{
	  if(o->name)
	    {
	      RiArchiveRecord(RI_COMMENT, o->name);
	    }
	}

      RiAttributeBegin();
      RiTransformBegin();

      /* write transformations */
      ay_wrib_trafos(o);

      /* write material information */
      if(o->type != AY_IDINSTANCE)
	{
	  if(o->mat)
	    {
	      ay_status = ay_matt_wrib(file, o->mat);
	    }

	  /* write tags */
	  if(o->tags)
	    {
	      ay_status = ay_riattr_wrib(o);
	      ay_status = ay_tc_wrib(o);
	    } /* if */

	}  /* if */


    } /* if */

  /* write child objects, but do not descend into light sources as
     the arealight geometry has been written long ago in the lights
     section of the RIB */
  if(o->down && o->down->next && (o->type != AY_IDLIGHT))
    {
      down = o->down;
      l = NULL;
      if(o->type == AY_IDLEVEL)
	{
	  l = (ay_level_object*)o->refine;
	  switch(l->type)
	    {
	    case AY_LTUNION:
	      RiSolidBegin(RI_UNION);
	      break;
	    case AY_LTDIFF:
	      RiSolidBegin(RI_DIFFERENCE);
	      break;
	    case AY_LTINT:
	      RiSolidBegin(RI_INTERSECTION);
	      break;
	    case AY_LTPRIM:
	      if(!ay_current_primlevel)
		{
		  RiSolidBegin(RI_PRIMITIVE);
		}
	      ay_current_primlevel++;
	      break;
	    default:
	      break;
	    } /* switch */
	} /* if */

      while(down->next)
	{
	  if(l && ((l->type == AY_LTUNION) || (l->type == AY_LTDIFF) ||
		   (l->type == AY_LTINT)))
	    {
	      if(!ay_current_primlevel)
		{
		  RiSolidBegin(RI_PRIMITIVE);
		}
	      ay_current_primlevel++;
	    }

	  ay_status = ay_wrib_object(file, down);
	  if(ay_status)
	    return ay_status;

	  if(l && ((l->type == AY_LTUNION) || (l->type == AY_LTDIFF) ||
		   (l->type == AY_LTINT)))
	    {
	      ay_current_primlevel--;
	      if(!ay_current_primlevel)
		{
		  RiSolidEnd();
		}
	    }

	  down = down->next;
	} /* while */

      if(l)
	{
	  if(l->type > 1)
	    {
	      if(l->type == AY_LTPRIM)
		{
		  ay_current_primlevel--;
		  if(!ay_current_primlevel)
		    {
		      RiSolidEnd();
		    }
		}
	      else
		{
		  RiSolidEnd();
		} /* if */
	    } /* if */
	} /* if */
    } /* if */

  if(cb)
    {
      ay_status = cb(file, o);

      RiTransformEnd();
      RiAttributeEnd();
    } /* if */

 return ay_status;
} /* ay_wrib_object */


/* ay_wrib_displaytags:
 *  write display tags that are attached to the root object
 */
void
ay_wrib_displaytags(void)
{
 char fname[] = "wrib_displaytags";
 ay_object *root = NULL;
 ay_tag_object *tag = NULL;
 int i = 0, j = 0;
 char *val = NULL, *name = NULL, *type = NULL, *mode = NULL;
 size_t len;
 RtToken dtype = RI_FILE, dmode = RI_RGBA;
 RtToken *tokens;
 RtPointer *values;
 RtInt argc;

  root = ay_root;
  if(!root)
    return;

  tag = root->tags;
  if(!tag)
    return;

  while(tag)
    {
      if(tag->type == ay_ridisp_tagtype)
	{
	  name = NULL;
	  val = tag->val;
	  len = strlen(val)+1;
	  if(len > 1)
	    {
	      name = NULL;
	      type = NULL;
	      mode = NULL;
	      if(!(name = calloc(len, sizeof(char))))
		return;
	      if(!(type = calloc(len, sizeof(char))))
		{ free(name); return; }
	      if(!(mode = calloc(len, sizeof(char))))
		{ free(name); free(type); return; }

	      i = 0;
	      j = 0;
	      if(val[0] != '+')
		{
		  name[0] = '+';
		  i++;
		}
	      while(val[j] && val[j] != ',')
		{
		  name[i] = val[j];
		  i++; j++;
		}
	      name[i] = '\0';
	      i = 0;
	      j++;
	      while(val[j] && val[j] != ',')
		{
		  type[i] = val[j];
		  i++; j++;
		}
	      type[i] = '\0';
	      i = 0;
	      j++;
	      while(val[j] && val[j] != ',')
		{
		  mode[i] = val[j];
		  i++; j++;
		}
	      mode[i] = '\0';

	      if(!name[0] || !type[0] || !mode[0])
		{

		  ay_error(AY_ERROR, fname,
			   "malformed RiDisplay tag encountered");
		  free(name); free(type); free(mode);
		  return;
		}
	  
	      /* get proper type */
	      if( ! ay_comp_strcase(type, "framebuffer"))
		{
		  dtype = RI_FRAMEBUFFER;
		}
	      if( ! ay_comp_strcase(type, "file"))
		{
		  dtype = RI_FILE;
		}


	      /* get proper mode */
	      if( ! ay_comp_strcase(mode, "a"))
		{
		  dmode = RI_A;
		}
	      if( ! ay_comp_strcase(mode, "z"))
		{
		  dmode = RI_Z;
		}
	      if( ! ay_comp_strcase(mode, "az"))
		{
		  dmode = RI_AZ;
		}
	      if( ! ay_comp_strcase(mode, "rgb"))
		{
		  dmode = RI_RGB;
		}
	      if( ! ay_comp_strcase(mode, "rgba"))
		{
		  dmode = RI_RGBA;
		}
	      if( ! ay_comp_strcase(mode, "rgbz"))
		{
		  dmode = RI_RGBZ;
		}
	      if( ! ay_comp_strcase(mode, "rgbaz"))
		{
		  dmode = RI_RGBAZ;
		}
	      argc = 0;
	      if(val[j] == ',')
		{
		  ay_tags_parseplist(&(val[j]), AY_TRUE,
				     &argc, &tokens, &values);
		}
	      if(argc != 0)
		{
		  RiDisplayV(name, dtype, dmode, argc, tokens, values);
		}
	      else
		{
		  RiDisplay(name, dtype, dmode, RI_NULL);
		}

	      free(name); free(type); free(mode);
	      /* free plist */

	    }
	  else
	    {
	      ay_error(AY_ERROR, fname,
		       "malformed RiDisplay tag encountered");
	    }
	} /* if */

      tag = tag->next;
    } /* while */

 return;
} /* ay_wrib_displaytags */


/* ay_wrib_hidertags:
 *  write hider tags that are attached to the root object
 */
void
ay_wrib_hidertags(void)
{
 char fname[] = "wrib_hidertags";
 ay_object *root = NULL;
 ay_tag_object *tag = NULL;
 int i = 0;
 char *val = NULL, *type = NULL;
 size_t len;
 RtToken htype = RI_HIDDEN;
 RtToken *tokens;
 RtPointer *values;
 RtInt argc;

  root = ay_root;
  if(!root)
    return;

  tag = root->tags;
  if(!tag)
    return;

  while(tag)
    {
      if(tag->type == ay_rihider_tagtype)
	{
	  val = tag->val;
	  len = strlen(val)+1;
	  if(len > 1)
	    {
	      if(!(type = calloc(len, sizeof(char))))
		{ return; }

	      i = 0;

	      while(val[i] && val[i] != ',')
		{
		  type[i] = val[i];
		  i++;
		}
	      type[i] = '\0';

	      if(!type[0])
		{
		  ay_error(AY_ERROR, fname,
			   "malformed RiHider tag encountered");
		  free(type);
		  return;
		}
	  
	      /* get proper type */
	      if( ! ay_comp_strcase(type, "hidden"))
		{
		  htype = RI_HIDDEN;
		}
	      else
	      if( ! ay_comp_strcase(type, "paint"))
		{
		  htype = RI_PAINT;
		}
	      else
		{
		  htype = type;
		}

	      argc = 0;
	      if(val[i] == ',')
		{
		  ay_tags_parseplist(&(val[i]), AY_TRUE,
				     &argc, &tokens, &values);
		}

	      if(argc != 0)
		{
		  RiHiderV(htype, argc, tokens, values);
		}
	      else
		{
		  RiHider(htype, RI_NULL);
		}

	      free(type);
	    }
	  else
	    {
	      ay_error(AY_ERROR, fname, "malformed RiHider tag encountered");
	    }
	} /* if */

      tag = tag->next;
    } /* while */

 return;
} /* ay_wrib_hidertags */


/* ay_wrib_rootsh:
 *  write shaders that are attached to root object
 *  if imager is 1, write imager shader
 *  if imager is 0, write atmosphere shader
 */
void
ay_wrib_rootsh(int imager)
{
 ay_root_object *root = NULL;

  root = (ay_root_object *)ay_root->refine;

  if(!imager)
    { /* write the atmosphere shader */
      ay_shader_wrib(root->atmosphere, AY_STATMOSPHERE, NULL);
    }
  else
    { /* write the imager shader */
      ay_shader_wrib(root->imager, AY_STIMAGER, NULL);
    } /* if */

 return;
} /* ay_wrib_rootsh */


/* ay_wrib_defmat:
 *  write default material
 */
void
ay_wrib_defmat(char *file)
{
 int ay_status = AY_OK;
 ay_mat_object *m = NULL;
 char fname[] = "wrib_defmat";

  switch(ay_prefs.defaultmat)
    {
    case 0:
      break;
    case 1:
      RiSurface("matte", RI_NULL);
      break;
    case 2:
      ay_status = ay_matt_getmaterial("default", &m);
      if(ay_status == AY_ERROR)
	{
	  ay_error(AY_EWARN, fname,
		   "No material named default. Using matte.");
	  RiSurface("matte", RI_NULL);
	}
      else
	{
	  ay_status = ay_matt_wrib(file, m);
	}
      break;
    default:
      break;
    } /* switch */

 return;
} /* ay_wrib_defmat */


/* ay_wrib_checklights:
 *
 */
int
ay_wrib_checklights(ay_object *o)
{
 int  i = 0;
 ay_light_object *light = NULL;

  if(!o)
    return 0;

  while(o->next)
    {
      if(o->down)
	{
	  i = ay_wrib_checklights(o->down);
	  if(i == 1)
	    return 1;
	}

      if(o->type == AY_IDLIGHT)
	{
	  light = (ay_light_object *)o->refine;

	  if(light->type == AY_LITCUSTOM)
	    {
	      if(light->lshader)
		{
		  if(light->on)
		    return 1;
		}
	    }
	  else
	    {
	      if(light->on)
		return 1;
	    }
	}
      o = o->next;
    }

 return i;
} /* ay_wrib_checklights */


/* ay_wrib_lights:
 *
 */
int
ay_wrib_lights(char *file, ay_object *o)
{
 int ay_status = AY_OK;
 ay_light_object *light = NULL;
 char *onstr = "on", *offstr = "off";
 RtColor color = {0.0f,0.0f,0.0f};
 RtPoint from, to;
 RtFloat intensity, coneangle, conedeltaangle, beamdistribution;
 RtFloat rim[4][4];
 double m[16];
 int i, j;
 int countsm = 1;
 /* shadowmap names */
 char *shadowptr = NULL, *pxptr = NULL, *nxptr = NULL;
 char *pyptr = NULL, *nyptr = NULL, *pzptr = NULL, *nzptr = NULL;
 int filenlen = 0;
 RtLightHandle light_handle;

  if(!o || !file)
    return ay_status;

  filenlen = strlen(file);

  if(!(shadowptr = calloc(filenlen+64, sizeof(char))))
    return AY_EOMEM;
  if(!(pxptr = calloc(filenlen+64, sizeof(char))))
    return AY_EOMEM;
  if(!(nxptr = calloc(filenlen+64, sizeof(char))))
    return AY_EOMEM;
  if(!(pyptr = calloc(filenlen+64, sizeof(char))))
    return AY_EOMEM;
  if(!(nyptr = calloc(filenlen+64, sizeof(char))))
    return AY_EOMEM;
  if(!(pzptr = calloc(filenlen+64, sizeof(char))))
    return AY_EOMEM;
  if(!(nzptr = calloc(filenlen+64, sizeof(char))))
    return AY_EOMEM;

  while(o)
    {
      if(o->down)
	{
	  glPushMatrix();
	   glTranslated((GLdouble)o->movx, (GLdouble)o->movy,
			(GLdouble)o->movz);
	   ay_quat_torotmatrix(o->quat, m);
	   glMultMatrixd((GLdouble *)m);
	   glScaled (o->scalx, o->scaly, o->scalz);
	   ay_wrib_lights(file, o->down);
	  glPopMatrix();
	}

      if(o->type == AY_IDLIGHT)
	{
	  light = (ay_light_object *)o->refine;

	  RiTransformBegin();

	  /* write transformation */
	  glPushMatrix();
	   glTranslated((GLdouble)o->movx, (GLdouble)o->movy,
			(GLdouble)o->movz);
	   ay_quat_torotmatrix(o->quat, m);
	   glMultMatrixd((GLdouble *)m);
	   glScaled (o->scalx, o->scaly, o->scalz);
	   glGetDoublev(GL_MODELVIEW_MATRIX, m);
	  glPopMatrix();

	  for(i=0;i<4;i++)
	    for(j=0;j<4;j++)
	      rim[i][j] = (RtFloat)m[i*4+j];
	  
	  RiConcatTransform(rim);

	  from[0] = (RtFloat)light->tfrom[0];
	  from[1] = (RtFloat)light->tfrom[1];
	  from[2] = (RtFloat)light->tfrom[2];

	  to[0] = (RtFloat)light->tto[0];
	  to[1] = (RtFloat)light->tto[1];
	  to[2] = (RtFloat)light->tto[2];

	  intensity = (RtFloat)light->intensity;
	  color[0] = (RtFloat)(light->colr/255.0);
	  color[1] = (RtFloat)(light->colg/255.0);
	  color[2] = (RtFloat)(light->colb/255.0);

	  if(!ay_prefs.ristandard)
	    {
	      RiDeclare((RtToken)"shadows", "string");
	      if(light->shadows)
		RiAttribute("light","shadows",(RtPointer)&onstr, RI_NULL);
	      else
		RiAttribute("light","shadows",(RtPointer)&offstr, RI_NULL);
	    }

	  switch(light->type)
	    {
	    case AY_LITCUSTOM:
	      if(light->lshader)
		{
		  
		  if(o->down && o->down->next)
		    { /* this is an AreaLight */
		      RiAttributeBegin();
		    } /* if */

		  if(light->samples > 1)
		    RiAttribute("light","nsamples",
				(RtPointer)(&light->samples), RI_NULL);

		  if(o->down && o->down->next)
		    { /* this is an AreaLight */
		      ay_status = ay_shader_wrib(light->lshader,
						 AY_STAREALIGHT,
						 &light_handle);

		      ay_status = ay_wrib_object(file, o->down);
		      RiAttributeEnd();
		    }
		  else
		    {
		      ay_status = ay_shader_wrib(light->lshader,
						 AY_STLIGHT,
						 &light_handle);
		    } /* if */
		} /* if */
	      break;
	    case AY_LITPOINT:
	      RiDeclare("intensity","float");
	      RiDeclare("lightcolor","color");
	      RiDeclare("from","point");

	      /* use shadowmap? */
	      if((ay_prefs.use_sm > 0) && light->use_sm)
		{ /* yes */
		  RiDeclare("sfpx", "uniform string");
		  RiDeclare("sfnx", "uniform string");
	          RiDeclare("sfpy", "uniform string");
		  RiDeclare("sfny", "uniform string");
		  RiDeclare("sfpz", "uniform string");
		  RiDeclare("sfnz", "uniform string");

		  sprintf(pxptr, "%s.point%d_x+.shd", file, countsm);
		  sprintf(nxptr, "%s.point%d_x-.shd", file, countsm);
		  sprintf(pyptr, "%s.point%d_y+.shd", file, countsm);
		  sprintf(nyptr, "%s.point%d_y-.shd", file, countsm);
		  sprintf(pzptr, "%s.point%d_z+.shd", file, countsm);
		  sprintf(nzptr, "%s.point%d_z-.shd", file, countsm);
		  light_handle = RiLightSource("shadowpoint",
					"intensity", (RtPointer)&intensity,
					"lightcolor", (RtPointer)&color,
					"from", (RtPointer)&from,
					"sfpx", (RtPointer)&pxptr,
			                "sfnx", (RtPointer)&nxptr,
			                "sfpy", (RtPointer)&pyptr, 
			                "sfny", (RtPointer)&nyptr,
			                "sfpz", (RtPointer)&pzptr,
			                "sfnz", (RtPointer)&nzptr,
					RI_NULL);
		  countsm++;
		}
	      else
		{ /* no, write normal lightsource */ 
		  light_handle = RiLightSource("pointlight",
					"intensity", (RtPointer)&intensity,
			                "lightcolor", (RtPointer)&color,
			                "from", (RtPointer)&from,
					RI_NULL);
		} /* if */
	      break;
	    case AY_LITSPOT:

	      RiDeclare("intensity","float");
	      RiDeclare("lightcolor","color");
	      RiDeclare("from","point");
	      RiDeclare("to","point");
	      RiDeclare("coneangle","float");
	      RiDeclare("conedeltaangle","float");
	      RiDeclare("beamdistribution","float");
	      coneangle = (RtFloat)light->cone_angle;
	      conedeltaangle = (RtFloat)light->cone_delta_angle;
	      beamdistribution = (RtFloat)light->beam_distribution;

	      /* use shadowmap? */
	      if((ay_prefs.use_sm > 0) && light->use_sm)
		{ /* yes */
		 RiDeclare("shadowname", "uniform string");
		  sprintf(shadowptr, "%s.spot%d.shd", file, countsm);
		  light_handle = RiLightSource("shadowspot",
			"intensity",(RtPointer)&intensity,
			"lightcolor",(RtPointer)&color,
			"from",(RtPointer)&from, 
			"to",(RtPointer)&to, 
			"coneangle",(RtPointer)&coneangle,
			"conedeltaangle",(RtPointer)&conedeltaangle,
			"beamdistribution",(RtPointer)&beamdistribution,
			"shadowname", (RtPointer)&shadowptr,
			RI_NULL);
		  countsm++; 
		}
	      else
		{ /* no, write normal lightsource */	  
		  light_handle = RiLightSource("spotlight",
			"intensity",(RtPointer)&intensity,
			"lightcolor",(RtPointer)&color,
			"from",(RtPointer)&from, 
			"to",(RtPointer)&to, 
			"coneangle",(RtPointer)&coneangle,
			"conedeltaangle",(RtPointer)&conedeltaangle,
			"beamdistribution",(RtPointer)&beamdistribution,
			RI_NULL);
		} /* if */
	      break;
	    case AY_LITDISTANT:
	      RiDeclare("intensity","float");
	      RiDeclare("lightcolor","color");
	      RiDeclare("from","point");
	      RiDeclare("to","point");
	      /* use shadowmap? */
	      if((ay_prefs.use_sm > 0) && light->use_sm)
		{ /* yes */
		  RiDeclare("shadowname", "uniform string");
		  sprintf(shadowptr, "%s.dist%d.shd", file, countsm);
		  light_handle = RiLightSource("shadowdistant",
					"intensity", (RtPointer)&intensity,
					"lightcolor", (RtPointer)&color,
					"from", (RtPointer)&from, 
					"to", (RtPointer)&to,
					"shadowname", (RtPointer)&shadowptr,
					RI_NULL);
		  countsm++;
		}
	      else
		{ /* no, write normal lightsource */
		  light_handle = RiLightSource("distantlight",
					"intensity", (RtPointer)&intensity,
					"lightcolor", (RtPointer)&color,
			                "from", (RtPointer)&from, 
			                "to", (RtPointer)&to,
					RI_NULL);
		} /* if */
	      break;
	    default:
	      break;
	    } /* switch */

	  switch(light->type)
	    {
	    case AY_LITCUSTOM:
	      if(light->lshader)
		{
		  if(light->on)
		    RiIlluminate(light_handle, RI_TRUE);
		  else
		    RiIlluminate(light_handle, RI_FALSE);
		}
	      break;
	    default:
	      if(light->on)
		RiIlluminate(light_handle, RI_TRUE);
	      else
		RiIlluminate(light_handle, RI_FALSE);
	      break;
	    } /* switch */

	  RiTransformEnd();
	} /* if */
      o = o->next;
    } /* while */

  /* clean up */
  if(shadowptr)
    {
      free(shadowptr);
    }
  if(pxptr)
    {
      free(pxptr);
    }
  if(nxptr)
    {
      free(nxptr);
    }
  if(pyptr)
    {
      free(pyptr);
    }
  if(nyptr)
    {
      free(nyptr);
    }
  if(pzptr)
    {
      free(pzptr);
    }
  if(nzptr)
    {
      free(nzptr);
    }

  return ay_status;
} /* ay_wrib_lights */


/* ay_wrib_scene:
 *  propagate changes to this function also to ay_wrib_pprevdraw()!
 */
int
ay_wrib_scene(char *file, char *image, double *from, double *to,
	      double roll, double zoom, double nearp, double farp,
	      int width, int height, int type)
{
 int ay_status = AY_OK;
 ay_object *o = ay_root;
 RtPoint f, t, d;
 RtFloat aspect = (RtFloat)1.0, swleft, swright, swtop, swbot;
 RtFloat fov = (RtFloat)90.0, rinearp, rifarp;
 char *objfile = NULL, *pos = NULL;
 int filenlen = 0;

  ay_current_primlevel = 0;
 
  /* create obj-file name (for use with ShadowMaps) */
  if(ay_prefs.use_sm >= 1)
    {
      ay_prefs.wrib_sm = AY_TRUE;

      filenlen = strlen(file);

      if(!(objfile = calloc(filenlen+64, sizeof(char))))
	return AY_EOMEM;

      pos = strstr(file, ".rib");

      if(pos)
	{
	  sprintf(objfile, "%s", file);
	  pos = NULL;
	  pos = strstr(objfile, ".rib");
	  if(pos)
	    sprintf(pos, "%s", ".obj.rib");
	}
      else
	{
	  sprintf(objfile, "%s.obj.rib", file);
	}
    } /* if */

 if(!ay_prefs.resolveinstances)
  {
    /* reset oid counter */
    ay_instt_createoid(NULL);
    /* create OI tags for all original (referenced) objects */
    ay_status = ay_instt_createorigids(o);
    /* create OI tags for all instance (referencing) objects */
    /*ay_status = ay_instt_createinstanceids(o);*/
    /* write archive files for all original (referenced) objects */
    if(ay_prefs.use_sm >= 1)
      {
	ay_status = ay_instt_wribiarchives(objfile, o);
      }
    else
      {
	ay_status = ay_instt_wribiarchives(file, o);
      }
  }

  ay_wrib_framenum = 1;

  /* assemble args */
  aspect = (RtFloat)(width/((double)height));

  f[0] = (RtFloat) from[0];
  f[1] = (RtFloat) from[1];
  f[2] = (RtFloat) from[2];

  t[0] = (RtFloat) to[0];
  t[1] = (RtFloat) to[1];
  t[2] = (RtFloat) to[2];

  d[0] = (RtFloat)(to[0] - from[0]);
  d[1] = (RtFloat)(to[1] - from[1]);
  d[2] = (RtFloat)(to[2] - from[2]);

  if(!file) /* dump .rib to stdout? */
    RiBegin(RI_NULL);
  else
    RiBegin(file);

  /* write shadow maps */
  if(ay_prefs.use_sm == 1)
    {
      ay_prefs.wrib_sm = AY_TRUE;

      /* write root RiOption tags (possibly containing shadow bias) */
      ay_status = ay_riopt_wrib(ay_root);

      ay_sm_wriballsm(file, objfile, ay_root->next, NULL, width, height);

      ay_prefs.wrib_sm = AY_FALSE;
    }

  RiFrameBegin((RtInt)ay_wrib_framenum++);

   if(!image) /* render to image file or to frame buffer? */
     RiDisplay(RI_NULL, RI_FRAMEBUFFER, RI_RGBA, RI_NULL);
   else
     RiDisplay(image, RI_FILE, RI_RGBA, RI_NULL);

   /* write additional RiDisplay statements from tags */
   ay_wrib_displaytags();
   /* write RiHider statements from tags */
   ay_wrib_hidertags();
   /* write imager shader */
   ay_wrib_rootsh(AY_TRUE);

   /* Camera! */
   RiFormat(width, height, (RtFloat)-1.0);
   if(type != AY_VTPERSP)
     RiProjection("orthographic", RI_NULL);
   else
     RiProjection("perspective", "fov", (RtPointer)&fov, RI_NULL);

   RiFrameAspectRatio((RtFloat)aspect);

   swleft = (RtFloat)-aspect;
   swright =  (RtFloat)aspect;
   swbot = (RtFloat)-1.0;
   swtop = (RtFloat)1.0;

   /* clipping planes */
   if(nearp != 0.0 || farp != 0.0)
     {
       if(nearp != 0.0)
	 {
	   rinearp = (RtFloat)nearp;
	   if(type == AY_VTPERSP)
	     zoom /= nearp;
	 }
       else
	 rinearp = RI_EPSILON;

       if(farp != 0.0)
	 rifarp = (RtFloat)farp;
       else
	 rifarp = RI_INFINITY;

       RiClipping(rinearp, rifarp);
     }

   RiScreenWindow((RtFloat)(swleft*zoom), (RtFloat)(swright*zoom),
		  (RtFloat)(swbot*zoom), (RtFloat)(swtop*zoom));

   RiIdentity();

   /* check for lights */
   if(ay_prefs.checklights)
     {
       if(!ay_wrib_checklights(ay_root))
	 { /* no lights in scene -> add a distant headlight */
	   RiArchiveRecord(RI_COMMENT, "Default Distant Headlight");
	   RiLightSource("distantlight", RI_NULL);
	 }
     }

   /* convert rh to lh */
   RiArchiveRecord(RI_COMMENT, "rh->lh");
   RiScale((RtFloat)-1.0, (RtFloat)1.0, (RtFloat)1.0);

   RiArchiveRecord(RI_COMMENT, "Camera!");
   ay_wrib_placecamera(f, d, roll);

   /* write RiOptions */
   ay_status = ay_wrib_rioptions();

   /* write root RiOption tags */
   ay_status = ay_riopt_wrib(ay_root);

   RiWorldBegin();

    /* Lights! */
    RiArchiveRecord(RI_COMMENT, "Lights!");
    ay_status = ay_wrib_lights(file, ay_root->next);

    RiIdentity();

    /* Action! */
    RiArchiveRecord(RI_COMMENT, "Action!");

    /* write atmosphere shader */
    ay_wrib_rootsh(AY_FALSE);

    /* write default material */
    ay_wrib_defmat(file);

    /* write objects */
    if(!ay_prefs.use_sm)
      {
	o = ay_root->next;
	while(o->next)
	  {
	    ay_status = ay_wrib_object(file, o);
	    o = o->next;
	  }
      }
    else
      {
	RiReadArchive(objfile, (RtVoid*)RI_NULL, RI_NULL);
      }

   RiWorldEnd();

  RiFrameEnd();

  /* Cut! */
  RiEnd();

  /* if shadowmaps are in use, write second RIB containing objects */
  if(ay_prefs.use_sm >= 1)
    {
      RiBegin(objfile);
       o = ay_root->next;
       while(o->next)
	 {
	   ay_status = ay_wrib_object(objfile, o);
	   o = o->next;
	 }
      RiEnd();
    } /* if */

  /* clean up */
  if(objfile)
    {
      free(objfile);
    }

 return ay_status;
} /* ay_wrib_scene */


/* ay_wrib_sm:
 *
 */
int
ay_wrib_sm(char *file, char *image, int width, int height)
{
 int ay_status = AY_OK;
 ay_object *o = ay_root;
 char *objfile = NULL, *pos = NULL;
 int filenlen = 0;

  filenlen = strlen(file);

  if(!(objfile = calloc(filenlen+64, sizeof(char))))
    return AY_EOMEM;

  pos = strstr(file, ".rib");

  if(pos)
    {
      sprintf(objfile, "%s", file);
      pos = NULL;
      pos = strstr(objfile, ".rib");
      if(pos)
	sprintf(pos, "%s", ".obj.rib");
    }
  else
    {
      sprintf(objfile, "%s.obj.rib", file);
    }

 if(!ay_prefs.resolveinstances)
  {
    /* reset oid counter */
    ay_instt_createoid(NULL);
    /* create OI tags for all original (referenced) objects */
    ay_status = ay_instt_createorigids(o);
    /* create OI tags for all instance (referencing) objects */
    /*ay_status = ay_instt_createinstanceids(o);*/
    /* write archive files for all original (referenced) objects */
    ay_status = ay_instt_wribiarchives(objfile, o);
  }

  ay_wrib_framenum = 1;

  if(!file) /* dump .rib to stdout? */
    RiBegin(RI_NULL);
  else
    RiBegin(file);

  /* inform other code that we write shadow maps now */
  ay_prefs.wrib_sm = AY_TRUE;

  /* write RiHider tags */
  ay_wrib_hidertags();

  /* write RiOption tags (possibly containing shadow bias settings) */
  ay_status = ay_riopt_wrib(ay_root);

  /* actually write the shadow maps */
  ay_sm_wriballsm(file, objfile, ay_root->next, NULL, width, height);

  /* inform other code that we do not write shadow maps anymore */
  ay_prefs.wrib_sm = AY_FALSE;

  /* Cut! */
  RiEnd();

  /* if shadowmaps are in use, write second RIB containing objects */
  RiBegin(objfile);
      
  o = ay_root->next;
  while(o->next)
    {
      ay_status = ay_wrib_object(objfile, o);
      o = o->next;
    }

  RiEnd();

  free(objfile);

 return ay_status;
} /* ay_wrib_sm */



/* ay_wrib_cb:
 *
 */
int
ay_wrib_cb(struct Togl *togl, int argc, char *argv[])
{
 int ay_status = AY_OK;
 ay_view_object *view = (ay_view_object *)Togl_GetClientData(togl);
 ay_root_object *root = NULL;
 ay_riopt_object *riopt = NULL;
 int width = Togl_Width (togl);
 int height = Togl_Height (togl);
 int i, temp = 0;
 char *file = NULL, *image = NULL;
 char fname[] = "write_rib";

#ifdef AYENABLEPPREV
  /* is a permanent preview window open? */
  if(ay_prefs.pprev_open)
    {
      ay_error(AY_ERROR, fname, "Please close the permanent preview first!");
      return AY_ERROR;
    }
#endif

  /* parse args */
  i = 2;
  while(i+1 <= argc)
    {
      if(!strcmp(argv[i],"-file"))
	file = argv[i+1];
      else
	if(!strcmp(argv[i],"-image"))
	  image = argv[i+1];
      else
	if(!strcmp(argv[i],"-temp"))
	  temp = 1;

      i += 2;
    }

  root = (ay_root_object*)(ay_root->refine);
  riopt = root->riopt;

  if(!temp)
    {
      /* override view width/height ? */
      if(riopt->width > 0)
	width = riopt->width;

      if(riopt->height > 0)
	height = riopt->height;
    }

  /* default */
  if(!file)
    {
      file = "rendrib";
    }

  /* adjust roll, if up vector points down */
  if(view->up[1] < 0.0)
    {
      view->roll += 180.0;
    }

  ay_status = ay_wrib_scene(file, image, view->from, view->to, view->roll,
				view->zoom, view->nearp, view->farp,
				width, height, view->type);

  if(view->up[1] < 0.0)
    {
      view->roll -= 180.0;
    }

  if(ay_status)
    {
      ay_error(ay_status, fname, NULL);
    }

 return TCL_OK;
} /* ay_wrib_cb */


/* ay_wrib_tcmd
 *
 */
int
ay_wrib_tcmd(ClientData clientData, Tcl_Interp * interp,
	     int argc, char *argv[])
{
 int ay_status = AY_OK;
 ay_list_object *sel = ay_selection;
 ay_camera_object *cam = NULL;
 ay_root_object *root = NULL;
 ay_riopt_object *riopt = NULL;
 int width = 400;
 int height = 300;
 int i, smonly = 0;
 char *file = NULL, *image = NULL;
 char fname[] = "wrib";

 /*
  if(argc <= 1)
    {
      ay_error(AY_EARGS, fname,
	 "\\[-file <filename>\\] \\[-image <imagename>\\] \\[-smonly\\]");
      return TCL_OK;
    }
 */

  /* parse args */
  i = 1;
  while(i+1 <= argc)
    {
      if(!strcmp(argv[i],"-file"))
	file = argv[i+1];
      else
	if(!strcmp(argv[i],"-image"))
	  image = argv[i+1];
	else
	  if(!strcmp(argv[i],"-smonly"))
	    smonly = 1;

      i += 2;
    }

  if(!smonly && (!sel || (sel->object->type != AY_IDCAMERA)))
   {
     ay_error(AY_ERROR, fname, "Please select a camera object!");
     return TCL_OK;
   }

  if(!smonly)
    cam = (ay_camera_object*)(sel->object->refine);

#ifdef AYENABLEPPREV
  /* is a permanent preview window open? */
  if(ay_prefs.pprev_open)
    {
      ay_error(AY_ERROR, fname, "Please close the permanent preview first!");
      return AY_ERROR;
    }
#endif

  root = (ay_root_object*)(ay_root->refine);
  riopt = root->riopt;

  /* override view width/height ? */
  if(riopt->width > 0)
    width = riopt->width;

  if(riopt->height > 0)
    height = riopt->height;

  /* default */
  if(!file)
    {
      file = "unnamed.rib";
    }

  if(!image)
    {
      image = "unnamed.tif";
    }

  /* adjust roll, if up vector points down */
  if(!smonly && (cam->up[1] < 0.0))
    {
      cam->roll += 180.0;
    }

  if(!smonly)
    {
      ay_status = ay_wrib_scene(file, image, cam->from, cam->to, cam->roll,
				cam->zoom, cam->nearp, cam->farp,
				width, height, AY_VTPERSP);
    }
  else
    {
      ay_status = ay_wrib_sm(file, image, width, height);
    }

  if(!smonly && (cam->up[1] < 0.0))
    {
      cam->roll -= 180.0;
    }

  if(ay_status)
    {
      ay_error(ay_status, fname, NULL);
    }

 return TCL_OK;
} /* ay_wrib_tcmd */


#ifdef AYENABLEPPREV
/* ay_wrib_pprevdraw:
 *
 */
int
ay_wrib_pprevdraw(ay_view_object *view)
{
 int ay_status = AY_OK;
 ay_object *o = ay_root;
 int old_resinstances = ay_prefs.resolveinstances;
 RtPoint f, t, d;
 RtFloat aspect = (RtFloat)1.0, swleft, swright, swtop, swbot;
 RtFloat fov = (RtFloat)90.0, rinearp, rifarp;
 struct Togl *togl = NULL;
 int width, height, i;
 double zoom, roll, *from, *to;
 char *pprender = "rgl";

  ay_current_primlevel = 0;

  if(ay_prefs.pprender && (strlen(ay_prefs.pprender) > 0))
    {
      pprender = ay_prefs.pprender;
    }

  from = view->from;
  to = view->to;
  togl = view->togl;
  width = Togl_Width(togl);
  height = Togl_Height(togl);
  zoom = view->zoom;
  roll = view->roll;
  /* adjust roll, if up vector points down */
  if(view->up[1] < 0.0)
    {
      roll += 180.0;
    }

  aspect = (RtFloat)(width/((double)height));

  /* assemble args */
  f[0] = (RtFloat) from[0];
  f[1] = (RtFloat) from[1];
  f[2] = (RtFloat) from[2];

  t[0] = (RtFloat) to[0];
  t[1] = (RtFloat) to[1];
  t[2] = (RtFloat) to[2];

  d[0] = (RtFloat)(to[0] - from[0]);
  d[1] = (RtFloat)(to[1] - from[1]);
  d[2] = (RtFloat)(to[2] - from[2]);

  /* when rendering to a pipe, do not create extra files
     for instances but rather resolve them */
  ay_prefs.resolveinstances = AY_TRUE;

  /* XXXX does this constant framenumber hurt? */
  RiFrameBegin((RtInt)1);

  RiDisplay("PPrev", RI_FRAMEBUFFER, RI_RGBA, RI_NULL);

  /* write RiHider statements from tags */
  ay_wrib_hidertags();

  /* write imager shader */
  ay_wrib_rootsh(AY_TRUE);

  /* Camera! */
  RiFormat(width, height, (RtFloat)-1.0);
  if(view->type != AY_VTPERSP)
    RiProjection("orthographic", RI_NULL);
  else
    RiProjection("perspective", "fov", (RtPointer)&fov, RI_NULL);

  RiFrameAspectRatio((RtFloat)aspect);

  swleft = (RtFloat)-aspect;
  swright =  (RtFloat)aspect;
  swbot = (RtFloat)-1.0;
  swtop = (RtFloat)1.0;

  /* clipping planes */
  if(view->nearp != 0.0 || view->farp != 0.0)
    {
      if(view->nearp != 0.0)
	{
	  rinearp = (RtFloat)view->nearp;
	  if(view->type == AY_VTPERSP)
	    zoom /= view->nearp;
	}
      else
	rinearp = RI_EPSILON;

      if(view->farp != 0.0)
	rifarp = (RtFloat)view->farp;
      else
	rifarp = RI_INFINITY;

      RiClipping(rinearp, rifarp);
    }

  RiScreenWindow((RtFloat)(swleft*zoom), (RtFloat)(swright*zoom),
		 (RtFloat)(swbot*zoom), (RtFloat)(swtop*zoom));

  RiIdentity();

  /* check for lights */
  if(ay_prefs.checklights)
    {
      if(!ay_wrib_checklights(ay_root))
	{ /* no lights in scene -> add a distant headlight */
	  RiArchiveRecord(RI_COMMENT, "Default Distant Headlight");
	  RiLightSource("distantlight", RI_NULL);
	}
    }

  /* convert rh to lh */
  RiArchiveRecord(RI_COMMENT, "rh->lh");
  RiScale((RtFloat)-1.0, (RtFloat)1.0, (RtFloat)1.0);
  RiArchiveRecord(RI_COMMENT, "Camera!");
  ay_wrib_placecamera(f, d, roll);

  /* write RiOptions */
  ay_status = ay_wrib_rioptions();

  /* write root RiOption tags */
  ay_status = ay_riopt_wrib(ay_root);

  RiWorldBegin();

  /* Lights! */
  RiArchiveRecord(RI_COMMENT, "Lights!");
  ay_status = ay_wrib_lights(pprender, ay_root->next);

  RiIdentity();

  /* Action! */
  RiArchiveRecord(RI_COMMENT, "Action!");

  /* write atmosphere shader */
  ay_wrib_rootsh(AY_FALSE);

  /* write default material */
  ay_wrib_defmat(pprender);

   o = ay_root->next;
   while(o->next)
     {
       ay_status = ay_wrib_object(pprender, o);
       o = o->next;
     }

  RiWorldEnd();
  RiFrameEnd();

  /* XXXX */
  for(i = 0;i < 500; i++)
    {
      RiArchiveRecord(RI_COMMENT, "Redraw please!\n");
    }

  ay_prefs.resolveinstances = old_resinstances;

 return ay_status;
} /* ay_wrib_pprevdraw */


/* ay_wrib_pprevopen:
 *
 */
int
ay_wrib_pprevopen(ay_view_object *view)
{
 int ay_status = AY_OK;
 char *pprender = "rgl";

  if(ay_prefs.pprender && (strlen(ay_prefs.pprender) > 0))
    {
      pprender = ay_prefs.pprender;
    }

  /* first, close eventually already open permanent preview window */
  ay_wrib_pprevclose();

  view->ppreview = AY_TRUE;

  /* now, open the new permanent preview window */
  RiBegin(pprender);

  /* and draw it */
  ay_status = ay_wrib_pprevdraw(view);

  ay_prefs.pprev_open = AY_TRUE;
  
 return ay_status;
} /* ay_wrib_pprevopen */


/* ay_wrib_pprevclose:
 *
 */
void
ay_wrib_pprevclose()
{
 ay_object *o = ay_root->down;
 ay_view_object *v = NULL;
 char fname[] = "pprevclose";

 while(o)
   {
     if(o->type == AY_IDVIEW)
       { 
	 v = (ay_view_object *)o->refine;

	 if(v->ppreview)
	   {
	     ay_error(AY_EOUTPUT, fname,
"Please close the permanent preview window now manually,");
	     ay_error(AY_EOUTPUT, fname,"using the Esc-key!");

	     RiEnd();

	     v->ppreview = AY_FALSE;
	     ay_prefs.pprev_open = AY_FALSE;

	   } /* if */

       } /* if */
     o = o->next;
   } /* while */

 return;
} /* ay_wrib_pprevclose */
#endif /* AYENABLEPPREV */


/* ay_wrib_init:
 *  initialize wrib module by registering the RiDisplay and RiHider tag type
 */
void
ay_wrib_init(Tcl_Interp *interp)
{

  /* register RiDisplay tag type */
  ay_tags_register(interp, "RiDisplay", &ay_ridisp_tagtype);

  /* register RiHider tag type */
  ay_tags_register(interp, "RiHider", &ay_rihider_tagtype);

 return;
} /* ay_wrib_init */
