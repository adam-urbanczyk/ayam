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

/* shader.c - functions for shader handling */

RtLightHandle ay_light_handle;


#ifdef AYUSESLCARGS
/* ay_shader_scanslcsarg:
 *  helper for ay_shader_scanslctcmd
 *  scan a shader arg
 */
int
ay_shader_scanslcsarg(SLC_VISSYMDEF *symbol, Tcl_DString *ds)
{
 int ay_status = AY_OK;
 char buffer[255];
 double deffltval;
 char *defstrval;
 int j;

  switch(symbol->svd_type)
    {
    case SLC_TYPE_POINT:
    case SLC_TYPE_COLOR:
    case SLC_TYPE_VECTOR:
    case SLC_TYPE_NORMAL:
      Tcl_DStringAppend(ds, "{ ", -1);
      deffltval = (double)((symbol->svd_default).pointval->xval);
      sprintf(buffer, "%g ", deffltval);
      Tcl_DStringAppend(ds, buffer, -1);
      deffltval = (double)((symbol->svd_default).pointval->yval);
      sprintf(buffer, "%g ", deffltval);
      Tcl_DStringAppend(ds, buffer, -1);
      deffltval = (double)((symbol->svd_default).pointval->zval);
      sprintf(buffer, "%g ", deffltval);
      Tcl_DStringAppend(ds, buffer, -1);
      Tcl_DStringAppend(ds, "} ", -1);
      break;
    case SLC_TYPE_MATRIX:
      Tcl_DStringAppend(ds, "{ ", -1);
      for(j = 0; j < 16; j++)
	{
	  deffltval = (double)((symbol->svd_default).matrixval[j]);
	  sprintf(buffer, "%g ", deffltval);
	  Tcl_DStringAppend(ds, buffer, -1);
	} /* for */
      Tcl_DStringAppend(ds, "} ", -1);
      break;
    case SLC_TYPE_SCALAR:
      deffltval = (double)(*(symbol->svd_default).scalarval);
      sprintf(buffer, "%g ", deffltval);
      Tcl_DStringAppend(ds, buffer, -1);
      break;
    case SLC_TYPE_STRING:
      defstrval = (symbol->svd_default).stringval;
      Tcl_DStringAppend(ds, defstrval, -1);
      Tcl_DStringAppend(ds, " ", -1);
      break;
    default:
      break;
    } /* switch */

 return ay_status;
} /* ay_shader_scanslcsarg */
#endif /* AYUSESLCARGS */

/* ay_shader_scanslctcmd:
 *  scan a shader compiled with slc (from BMRT) with libslcargs.a
 */
int
ay_shader_scanslctcmd(ClientData clientData, Tcl_Interp *interp,
		      int argc, char *argv[])
{
#ifdef AYUSESLCARGS
 int i = 0, j = 0, numargs = 0;
 SLC_VISSYMDEF *symbol = NULL, *element = NULL;
 SLC_TYPE type;
 char buffer[255];
 int arraylen;
 Tcl_DString ds;
 char vname[] = "env(SHADERS)";

  if(argc < 3)
    {
      ay_error(AY_EARGS, argv[0], "shaderpath varname");
      return TCL_OK;
    }

  SLC_SetPath(Tcl_GetVar(interp, vname, TCL_GLOBAL_ONLY|TCL_LEAVE_ERR_MSG));

  if((SLC_SetShader(argv[1])) == -1)
    {
      ay_error(AY_ERROR, argv[0], "SLC_SetShader failed for:");
      ay_error(AY_ERROR, argv[0], argv[1]);
      return TCL_OK;
    }

  Tcl_DStringInit(&ds);

  /* get name of shader */
  Tcl_DStringAppend(&ds, argv[1]/*SLC_GetName()*/, -1);

  /* get type of shader */
  type = SLC_GetType();
  switch(type)
    {
    case SLC_TYPE_SURFACE:
      Tcl_DStringAppend(&ds, " surface ", -1);
      break;
    case SLC_TYPE_DISPLACEMENT:
      Tcl_DStringAppend(&ds, " displacement ", -1);
      break;
    case SLC_TYPE_LIGHT:
      Tcl_DStringAppend(&ds, " light ", -1);
      break;
    case SLC_TYPE_VOLUME:
      Tcl_DStringAppend(&ds, " volume ", -1);
      break;
    case SLC_TYPE_IMAGER:
      Tcl_DStringAppend(&ds, " imager ", -1);
      break;
    case SLC_TYPE_TRANSFORMATION:
      Tcl_DStringAppend(&ds, " transformation ", -1);
      break;
    default:
      ay_error(AY_ERROR, fname, "skipping shader of unknown type");
      ay_error(AY_ERROR, fname, argv[1]);
      goto cleanup;
      break;
    }

  /* get arguments of shader */
  numargs = SLC_GetNArgs();
  Tcl_DStringAppend(&ds, "{ ", -1);
  for(i = 1; i <= numargs; i++)
    {
      symbol = NULL;
      symbol = SLC_GetArgById(i);

      if(!symbol)
	{
	  ay_error(AY_ERROR, argv[0], "Cannot get symbol from shader:");
	  ay_error(AY_ERROR, argv[0], argv[1]);
	  goto cleanup;
	}

      /* XXXX temporarily discard array arguments   */
      /* libslcargs cannot handle them as of 2.5h!  */
      if(symbol->svd_arraylen < 1)
	{
	  Tcl_DStringAppend(&ds, "{ ", -1);
	  Tcl_DStringAppend(&ds, symbol->svd_name, -1);
	  Tcl_DStringAppend(&ds, " ", -1 );

	  switch(symbol->svd_type)
	    {
	    case SLC_TYPE_POINT:
	      Tcl_DStringAppend(&ds, "point ", -1);
	      break;
	    case SLC_TYPE_COLOR:
	      Tcl_DStringAppend(&ds, "color ", -1);
	      break;
	    case SLC_TYPE_VECTOR:
	      Tcl_DStringAppend(&ds, "vector ", -1);
	      break;
	    case SLC_TYPE_NORMAL:
	      Tcl_DStringAppend(&ds, "normal ", -1);
	      break;
	    case SLC_TYPE_MATRIX:
	      Tcl_DStringAppend(&ds, "matrix ", -1);
	      break;
	    case SLC_TYPE_SCALAR:
	      Tcl_DStringAppend(&ds, "float ", -1);
	      break;
	    case SLC_TYPE_STRING:
	      Tcl_DStringAppend(&ds, "string ", -1);
	      break;
	    default:
	      break;
	    }

	  arraylen = symbol->svd_arraylen;
	  sprintf(buffer, "%d ", arraylen);
	  Tcl_DStringAppend(&ds, buffer, -1);

	  if(arraylen > 0)
	    {
	      Tcl_DStringAppend(&ds, "{ ", -1);
	      for(j = 0; j < arraylen; j++)
		{
		  element = NULL;
		  element = SLC_GetArrayArgElement(symbol, j);
		  if(!element)
		    {
		      ay_error(AY_ERROR, argv[0],
			       "Could not get array element:");
		      ay_error(AY_ERROR, argv[0], symbol->svd_name);
		      goto cleanup;
		    } /* if */
		  ay_shader_scanslcsarg(element, &ds);
		} /* for */

	      Tcl_DStringAppend(&ds, "} ", -1);
	    }
	  else
	    {
	      ay_shader_scanslcsarg(symbol, &ds);
	    } /* if */
	  Tcl_DStringAppend(&ds, "} ", -1);
	}
      else
	{
	  ay_error(AY_EWARN,argv[0],"Skipping array argument!");
	  /*	  ay_error(AY_EWARN,argv[0],symbol->svd_name);*/
	} /* if */
      /* XXXX temporarily discard array arguments */
    } /* for */

  Tcl_DStringAppend(&ds, "} ", -1);

  /* return result */
  Tcl_SetVar(interp, argv[2], Tcl_DStringValue(&ds), TCL_LEAVE_ERR_MSG);

cleanup:
  Tcl_DStringFree(&ds);
  SLC_EndShader();

#else
 ay_error(AY_ERROR, argv[0], "This Ayam has not been linked with libslcargs!");
#endif /* AYUSESLCARGS */
 return TCL_OK;
} /* ay_shader_scanslctcmd */


#ifdef AYUSESLXARGS
/* ay_shader_scanslxsarg:
 *  helper for ay_shader_scanslxtcmd
 *  scan a shader arg
 */
int
ay_shader_scanslxsarg(SLX_VISSYMDEF *symbol, Tcl_DString *ds)
{
 int ay_status = AY_OK;
 char buffer[255];
 double deffltval;
 char *defstrval;
 int i, j;

  switch(symbol->svd_type)
    {
    case SLX_TYPE_POINT:
    case SLX_TYPE_COLOR:
    case SLX_TYPE_VECTOR:
    case SLX_TYPE_NORMAL:
      Tcl_DStringAppend(ds, "{ ", -1);
      deffltval = (double)((symbol->svd_default).pointval->xval);
      sprintf(buffer, "%g ", deffltval);
      Tcl_DStringAppend(ds, buffer, -1);
      deffltval = (double)((symbol->svd_default).pointval->yval);
      sprintf(buffer, "%g ", deffltval);
      Tcl_DStringAppend(ds, buffer, -1);
      deffltval = (double)((symbol->svd_default).pointval->zval);
      sprintf(buffer, "%g ", deffltval);
      Tcl_DStringAppend(ds, buffer, -1);
      Tcl_DStringAppend(ds, "} ", -1);
      break;
    case SLX_TYPE_MATRIX:
      Tcl_DStringAppend(ds, "{ ", -1);
#ifndef AYOLDSLX
      for(i = 0; i < 4; i++)
	{
	  for(j = 0; j < 4; j++)
	    {
	      deffltval = (double)((symbol->svd_default).matrixval->val[i][j]);
	      sprintf(buffer, "%g ", deffltval);
	      Tcl_DStringAppend(ds, buffer, -1);
	    } /* for */
	} /* for */
#else
      for(j = 0; j < 16; j++)
	{
	  deffltval = (double)((symbol->svd_default).matrixval[j]);
	  sprintf(buffer, "%g ", deffltval);
	  Tcl_DStringAppend(ds, buffer, -1);
	} /* for */
#endif
      Tcl_DStringAppend(ds, "} ", -1);
      break;
    case SLX_TYPE_SCALAR:
      deffltval = (double)(*(symbol->svd_default).scalarval);
      sprintf(buffer, "%g ", deffltval);
      Tcl_DStringAppend(ds, buffer, -1);
      break;
    case SLX_TYPE_STRING:
      defstrval = *(symbol->svd_default).stringval;
      Tcl_DStringAppend(ds, defstrval, -1);
      Tcl_DStringAppend(ds, " ", -1);
      break;
    default:
      break;
    } /* switch */

 return ay_status;
} /* ay_shader_scanslxsarg */
#endif /* AYUSESLXARGS */


/* ay_shader_scanslxtcmd:
 *  scan a shader compiled with aqsl with libslxargs
 */
int
ay_shader_scanslxtcmd(ClientData clientData, Tcl_Interp *interp,
		      int argc, char *argv[])
{
#ifdef AYUSESLXARGS
 int i = 0, j = 0, numargs = 0;
 SLX_VISSYMDEF *symbol = NULL, *element = NULL;
 SLX_TYPE type;
 char buffer[255];
 int arraylen;
 Tcl_DString ds;
 char vname[] = "env(SHADERS)";
#ifdef WIN32
 char *c = NULL;
#endif

  if(argc < 3)
    {
      ay_error(AY_EARGS, argv[0], "shaderpath varname");
      return TCL_OK;
    }

#ifndef WIN32
  SLX_SetPath(Tcl_GetVar(ay_interp, vname, TCL_GLOBAL_ONLY|TCL_LEAVE_ERR_MSG));
  SLX_SetDSOPath(
       Tcl_GetVar(ay_interp, vname, TCL_GLOBAL_ONLY|TCL_LEAVE_ERR_MSG));
#else
  /* change all ; to : in shader search path */
  Tcl_DStringInit(&ds);
  Tcl_DStringAppend(&ds,
       Tcl_GetVar(ay_interp, vname, TCL_GLOBAL_ONLY|TCL_LEAVE_ERR_MSG), -1);
  c = strchr(Tcl_DStringValue(&ds), ';');
  while(c)
    {
      *c = ':';
      c = strchr(c, ';');
    }
  SLX_SetPath(Tcl_DStringValue(&ds));
  SLX_SetDSOPath(Tcl_DStringValue(&ds));
  Tcl_DStringFree(&ds);
#endif /* WIN32 */

  if(SLX_SetShader(argv[1]) != 0)
    {
      ay_error(AY_ERROR, argv[0], "SLX_SetShader failed for:");
      ay_error(AY_ERROR, argv[0], argv[1]);
      return TCL_OK;
    }

  Tcl_DStringInit(&ds);

  /* get name of shader */
  Tcl_DStringAppend(&ds, argv[1]/*SLX_GetName()*/, -1);

  /* get type of shader */
  type = SLX_GetType();
  switch(type)
    {
    case SLX_TYPE_SURFACE:
      Tcl_DStringAppend(&ds, " surface ", -1);
      break;
    case SLX_TYPE_DISPLACEMENT:
      Tcl_DStringAppend(&ds, " displacement ", -1);
      break;
    case SLX_TYPE_LIGHT:
      Tcl_DStringAppend(&ds, " light ", -1);
      break;
    case SLX_TYPE_VOLUME:
      Tcl_DStringAppend(&ds, " volume ", -1);
      break;
    case SLX_TYPE_IMAGER:
      Tcl_DStringAppend(&ds, " imager ", -1);
      break;
    case SLX_TYPE_TRANSFORMATION:
      Tcl_DStringAppend(&ds, " transformation ", -1);
      break;
    default:
      ay_error(AY_ERROR, argv[0], "skipping shader of unknown type");
      ay_error(AY_ERROR, argv[0], argv[1]);
      goto cleanup;
      break;
    }

  /* get arguments of shader */
  numargs = SLX_GetNArgs();
  Tcl_DStringAppend(&ds, "{ ", -1);
  for(i = 0; i < numargs; i++)
    {
      symbol = NULL;
      symbol = SLX_GetArgById(i);

      if(!symbol)
	{
	  ay_error(AY_ERROR, argv[0], "Cannot get symbol from shader:");
	  ay_error(AY_ERROR, argv[0], argv[1]);
	  goto cleanup;
	}

      /* XXXX temporarily discard array arguments   */
#ifdef AYAQSIS10
      if(symbol->svd_arraylen < 1)
#else
      if(symbol->svd_arraylen < 2)
#endif
	{

      Tcl_DStringAppend(&ds, "{ ", -1);
      Tcl_DStringAppend(&ds, symbol->svd_name, -1);
      Tcl_DStringAppend(&ds, " ", -1 );

      switch(symbol->svd_type)
	{
	case SLX_TYPE_POINT:
	  Tcl_DStringAppend(&ds, "point ", -1);
	  break;
	case SLX_TYPE_COLOR:
	  Tcl_DStringAppend(&ds, "color ", -1);
	  break;
	case SLX_TYPE_VECTOR:
	  Tcl_DStringAppend(&ds, "vector ", -1);
	  break;
	case SLX_TYPE_NORMAL:
	  Tcl_DStringAppend(&ds, "normal ", -1);
	  break;
	case SLX_TYPE_MATRIX:
	  Tcl_DStringAppend(&ds, "matrix ", -1);
	  break;
	case SLX_TYPE_SCALAR:
	  Tcl_DStringAppend(&ds, "float ", -1);
	  break;
	case SLX_TYPE_STRING:
	  Tcl_DStringAppend(&ds, "string ", -1);
	  break;
	default:
	  Tcl_DStringAppend(&ds, "unknown ", -1);
	  break;
	}

      arraylen = symbol->svd_arraylen-1;
#ifdef AYAQSIS10
      arraylen++;
#endif
      sprintf(buffer, "%d ", arraylen);
      Tcl_DStringAppend(&ds, buffer, -1);

      if(arraylen > 0)
	{
	  Tcl_DStringAppend(&ds, "{ ", -1);
	  for(j = 0; j < arraylen; j++)
	    {
	      element = NULL;
	      element = SLX_GetArrayArgElement(symbol, j);
	      if(!element)
		{
		  ay_error(AY_ERROR, argv[0], "Could not get array element:");
		  ay_error(AY_ERROR, argv[0], symbol->svd_name);
		  goto cleanup;
		} /* if */
	      ay_shader_scanslxsarg(element, &ds);
	    } /* for */

	  Tcl_DStringAppend(&ds, "} ", -1);
	}
      else
	{
	  ay_shader_scanslxsarg(symbol, &ds);
	} /* if */
      Tcl_DStringAppend(&ds, "} ", -1);

	}
      else
	{
	  ay_error(AY_EWARN,argv[0],"Skipping array argument!");
	  /*	  ay_error(AY_EWARN,argv[0],symbol->svd_name);*/
	} /* if */
      /* XXXX temporarily discard array arguments */
    } /* for */

  Tcl_DStringAppend(&ds, "} ", -1);

  /* return result */
  Tcl_SetVar(interp, argv[2], Tcl_DStringValue(&ds), TCL_LEAVE_ERR_MSG);

cleanup:
  Tcl_DStringFree(&ds);
  SLX_EndShader();
#else
 ay_error(AY_ERROR, argv[0], "This Ayam has not been linked with libslxargs!");
 ay_error(AY_ERROR, argv[0],
	  "Load the ayslx plugin and use shaderScan instead!");
#endif /* AYUSESLXARGS */
 return TCL_OK;
} /* ay_shader_scanslxtcmd */


/* ay_shader_free:
 *  free a shader and associated arguments
 */
void
ay_shader_free(ay_shader *shader)
{
 ay_shader_arg *arg = NULL, *argtmp = NULL;

  if(!shader)
    return;

  if(shader->arg)
    {
      /* delete arguments */
      arg = shader->arg;
      while(arg)
	{
	  argtmp = arg->next;
	  if(arg->name)
	    {
	      free(arg->name);
	    }

	  if(arg->type == AY_SASTRING && arg->val.string)
	    {
	      free(arg->val.string);
	    }
	  free(arg);
	  arg = argtmp;
	} /* while */
    } /* if */

  if(shader->name)
    free(shader->name);

  /* finally, delete the shader*/
  free(shader);

 return;
} /* ay_shader_free */


/* ay_shader_copyarg:
 *  copy a shader parameter
 */
int
ay_shader_copyarg(ay_shader_arg *source, ay_shader_arg **dest)
{
 int ay_status = AY_OK;
 ay_shader_arg *newp = NULL;
 char *newval;

  if(!(newp = malloc(sizeof(ay_shader_arg))))
    return AY_EOMEM;

  memcpy(newp, source, sizeof(ay_shader_arg));
  /* danger! links point to original hierachy */

  /* copy name */
  if(!(newval = malloc((strlen(source->name)+1)*sizeof(char))))
    { free(newp); return AY_EOMEM; }
  strcpy(newval, source->name);
  newp->name = newval;

  if(source->type == AY_SASTRING && source->val.string)
    {
      /* copy string */
      if(!(newval = malloc((strlen(source->val.string)+1)*sizeof(char))))
	{ free(newp->name); free(newp); return AY_EOMEM; }
      strcpy(newval, source->val.string);
      newp->val.string = newval;
    }

  *dest = newp;

 return ay_status;
} /* ay_shader_copyarg */


/* ay_shader_copy:
 *  copy a shader & associated parameters
 */
int
ay_shader_copy(ay_shader *source, ay_shader **dest)
{
 int ay_status = AY_OK;
 ay_shader *news = NULL;
 ay_shader_arg *p = NULL, **newp;
 char *newval = NULL;

  if(!(news = malloc(sizeof(ay_shader))))
    return AY_EOMEM;

  memcpy(news, source, sizeof(ay_shader));
  /* danger! links point to original hierachy */

  /* copy name */
  if(!(newval = malloc((strlen(source->name)+1)*sizeof(char))))
    { free(news); return AY_EOMEM; }
  strcpy(newval, source->name);
  news->name = newval;

  /* copy parameters */
  if(source->arg)
    {
      p = source->arg;
      newp = &(news->arg);
      while(p)
	{
	  ay_status = ay_shader_copyarg(p, newp);
	  if(ay_status)
	    {
	      ay_shader_free(news);
	      return ay_status;
	    }

	  (*newp)->next = NULL;
	  newp = &((*newp)->next);
	  p = p->next;
	} /* while */
    } /* if */

  *dest = news;

 return ay_status;
} /* ay_shader_copy */


/* ay_shader_wrib:
 *  write shader to RIB
 */
int
ay_shader_wrib(ay_shader *shader, int type, RtLightHandle *light_handle)
{
 int ay_status = AY_OK;
 char fname[] = "shader_wrib";
 ay_shader_arg *sarg = NULL;
 RtToken *tokens = NULL;
 RtPointer *values = NULL;
 int count = 0;

  if(!shader)
    return AY_ENULL;

  if(shader->name)
    {
      sarg = shader->arg;
      while(sarg)
	{
	  count++;
	  sarg = sarg->next;
	}

      if(count)
	{
	  if(!(tokens = calloc(count, sizeof(RtToken))))
	    return AY_EOMEM;
	  if(!(values = calloc(count, sizeof(RtToken))))
	    { free(tokens); return AY_EOMEM; }
	}

      sarg = shader->arg;

      count = 0;
      while(sarg)
	{
	  switch(sarg->type)
	    {
	    case AY_SACOLOR:
	      RiDeclare(sarg->name,"color");
	      values[count] = (void*)(&sarg->val.color);
	      break;
	    case AY_SAVECTOR:
	      RiDeclare(sarg->name,"vector");
	      values[count] = (void*)(&(sarg->val.point));
	      break;
	    case AY_SAPOINT:
	      RiDeclare(sarg->name,"point");
	      values[count] = (void*)(&(sarg->val.point));
	      break;
	    case AY_SANORMAL:
	      RiDeclare(sarg->name,"normal");
	      values[count] = (void*)(&(sarg->val.point));
	      break;
	    case AY_SASCALAR:
	      RiDeclare(sarg->name,"float");
	      values[count] = (void*)(&(sarg->val.scalar));
	      break;
	    case AY_SASTRING:
	      RiDeclare(sarg->name,"string");
	      values[count] = (void*)(&(sarg->val.string));
	      break;
	    case AY_SAMATRIX:
	      RiDeclare(sarg->name,"matrix");
	      values[count] = (void*)(&(sarg->val.matrix));
	      break;
	    } /* switch sarg->type */

	  tokens[count] = sarg->name;

	  count++;
	  sarg = sarg->next;
	} /* while sarg */

      switch(type)
	{
	case AY_STSURFACE:
	  RiSurfaceV(shader->name,count,tokens,values);
	  break;
	case AY_STDISPLACEMENT:
	  RiDisplacementV(shader->name,count,tokens,values);
	  break;
	case AY_STLIGHT:
	  *light_handle = RiLightSourceV(shader->name,count,tokens,values);
	  break;
	case AY_STAREALIGHT:
	  *light_handle = RiAreaLightSourceV(shader->name,count,tokens,values);
	  break;
	case AY_STINTERIOR:
	  RiInteriorV(shader->name,count,tokens,values);
	  break;
	case AY_STEXTERIOR:
	  RiExteriorV(shader->name,count,tokens,values);
	  break;
	case AY_STATMOSPHERE:
	  RiAtmosphereV(shader->name,count,tokens,values);
	  break;
	case AY_STIMAGER:
	  RiImagerV(shader->name,count,tokens,values);
	  break;
#ifndef AYNORIDEFORM
	case AY_STTRANSFORMATION:
	  RiDeformationV(shader->name,count,tokens,values);
	  break;
#endif
	default:
	  ay_error(AY_ERROR, fname, "Skipping shader of unknown type.");
	  break;
	} /* switch type */

      if(tokens)
	free(tokens);
      tokens = NULL;
      if(values)
	free(values);
      values = NULL;
    } /* if have shader name */

 return ay_status;
} /* ay_shader_wrib */


/* ay_shader_gettcmd:
 *  C -> Tcl
 */
int
ay_shader_gettcmd(ClientData clientData, Tcl_Interp *interp,
		  int argc, char *argv[])
{
 ay_object *o = NULL;
 ay_list_object *sel = ay_selection;
 ay_root_object *root = NULL;
 ay_light_object *light = NULL;
 ay_mat_object *material = NULL;
 ay_shader *shader = NULL;
 ay_shader_arg *arg = NULL;
 char *arr = NULL;
 char *n1 = NULL, *sname = NULL/*, *sfile = NULL*/;
 int shadertype = 0, i;
 Tcl_Obj *to = NULL;
 char *man[] = {"_0","_1","_2","_3","_4","_5","_6","_7","_8","_9","_10","_11","_12","_13","_14","_15"};
 char *rgb[3] ={"_R","_G","_B"};
 char *xyz[3] = {"_X","_Y","_Z"};
 Tcl_DString ds;

  if(!sel) /* oops? */
    o = ay_root;
  else
    o = sel->object;

  /* parse args */
  if(argc < 2)
    {
      ay_error(AY_EARGS, argv[0], "shadertype varname");
      return TCL_OK;
    }

  switch(argv[1][0])
    {
    case 's':
      shadertype = AY_STSURFACE;
      break;
    case 'd':
      shadertype = AY_STDISPLACEMENT;
      break;
    case 'i':
      if(argv[1][1] == 'n')
	shadertype = AY_STINTERIOR;
      else
	shadertype = AY_STIMAGER;
      break;
    case 'e':
      shadertype = AY_STEXTERIOR;
      break;
    case 'a':
      shadertype = AY_STATMOSPHERE;
      break;
    case 'l':
      shadertype = AY_STLIGHT;
      break;
    case 't':
      shadertype = AY_STTRANSFORMATION;
      break;
    default:
      ay_error(AY_EARGS, argv[0], "unknown shadertype");
      return TCL_OK;
    }

  /* get shader */
  switch(shadertype)
    {
    case AY_STIMAGER:
      if(o != ay_root)
	{
	  ay_error(AY_ERROR, argv[0], "only root has imager");
	  return TCL_OK;
	}
      else
	{
	  root = (ay_root_object*)o->refine;
	  shader = root->imager;
	}
      break;
    case AY_STATMOSPHERE:
      if(o != ay_root)
	{
	  ay_error(AY_ERROR, argv[0], "only root has atmosphere");
	  return TCL_OK;
	}
      else
	{
	  root = (ay_root_object*)o->refine;
	  shader = root->atmosphere;
	}
      break;
    case AY_STLIGHT:
      if(o->type != AY_IDLIGHT)
	{
	  ay_error(AY_ERROR, argv[0], "only light objects have light shaders");
	  return TCL_OK;
	}
      else
	{
	  light = (ay_light_object*)o->refine;
	  shader = light->lshader;
	}
      break;
    case AY_STSURFACE:
      if(o->type != AY_IDMATERIAL)
	{
	  ay_error(AY_ERROR, argv[0],
		   "only material objects have surface shaders");
	  return TCL_OK;
	}
      else
	{
	  material = (ay_mat_object*)o->refine;
	  shader = material->sshader;
	}
      break;
    case AY_STDISPLACEMENT:
      if(o->type != AY_IDMATERIAL)
	{
	  ay_error(AY_ERROR, argv[0],
		   "only material objects have displacement shaders");
	  return TCL_OK;
	}
      else
	{
	  material = (ay_mat_object*)o->refine;
	  shader = material->dshader;
	}
      break;
    case AY_STINTERIOR:
      if(o->type != AY_IDMATERIAL)
	{
	  ay_error(AY_ERROR, argv[0],
		   "only material objects have interior shaders");
	  return TCL_OK;
	}
      else
	{
	  material = (ay_mat_object*)o->refine;
	  shader = material->ishader;
	}
      break;
    case AY_STEXTERIOR:
      if(o->type != AY_IDMATERIAL)
	{
	  ay_error(AY_ERROR, argv[0],
		   "only material objects have exterior shaders");
	  return TCL_OK;
	}
      else
	{
	  material = (ay_mat_object*)o->refine;
	  shader = material->eshader;
	}
      break;
    default:
      return TCL_OK;
      /*      break;*/
    } /* switch */

  arr = argv[2];

  Tcl_SetVar2(interp, arr, "Name", "",
	      TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  if(!shader)
    { return TCL_OK; }

  arg = shader->arg;
  sname = shader->name;
  /*	  sfile = shader->loc;*/

  Tcl_SetVar2(interp, arr, "Name", sname, TCL_LEAVE_ERR_MSG |
	      TCL_GLOBAL_ONLY);
  /*  Tcl_SetVar2(interp,n1,"File",sfile,TCL_LEAVE_ERR_MSG |
	      TCL_GLOBAL_ONLY);*/
  Tcl_SetVar2(interp, arr, "ArgNames", "", TCL_LEAVE_ERR_MSG |
	      TCL_GLOBAL_ONLY);
  Tcl_SetVar2(interp, arr, "ArgTypes", "", TCL_LEAVE_ERR_MSG |
	      TCL_GLOBAL_ONLY);

  while(arg)
    {
      Tcl_SetVar2(interp, arr, "ArgNames", arg->name, TCL_LEAVE_ERR_MSG |
		  TCL_GLOBAL_ONLY | TCL_APPEND_VALUE | TCL_LIST_ELEMENT);

      switch(arg->type)
	{
	case AY_SACOLOR:
	  Tcl_SetVar2(interp, arr, "ArgTypes", "color", TCL_LEAVE_ERR_MSG |
		      TCL_GLOBAL_ONLY | TCL_APPEND_VALUE | TCL_LIST_ELEMENT);
	  Tcl_DStringInit(&ds);
	  Tcl_DStringAppend(&ds, arg->name, -1);
	  for(i = 0; i < 3; i++)
	    {
	      Tcl_DStringAppend(&ds, rgb[i], 2);
	      to = Tcl_NewIntObj((int)(arg->val.color[i]*255));
	      Tcl_SetVar2Ex(interp, arr, Tcl_DStringValue(&ds), to,
			    TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
	      Tcl_DStringSetLength(&ds, Tcl_DStringLength(&ds)-2);
	    }
	  Tcl_DStringFree(&ds);
	  break;

	case AY_SAPOINT:
	case AY_SAVECTOR:
	case AY_SANORMAL:
	  switch(arg->type)
	    {
	    case AY_SAPOINT:
	      Tcl_SetVar2(interp,n1,"ArgTypes", "point", TCL_LEAVE_ERR_MSG |
		       TCL_GLOBAL_ONLY | TCL_APPEND_VALUE | TCL_LIST_ELEMENT);
	      break;
	    case AY_SAVECTOR:
	      Tcl_SetVar2(interp,n1,"ArgTypes", "vector", TCL_LEAVE_ERR_MSG |
		       TCL_GLOBAL_ONLY | TCL_APPEND_VALUE | TCL_LIST_ELEMENT);
	    case AY_SANORMAL:
	      Tcl_SetVar2(interp,n1,"ArgTypes", "normal", TCL_LEAVE_ERR_MSG |
		       TCL_GLOBAL_ONLY | TCL_APPEND_VALUE | TCL_LIST_ELEMENT);
	      break;
	    default:
	      break;
	    }
	  Tcl_DStringInit(&ds);
	  Tcl_DStringAppend(&ds, arg->name, -1);
	  for(i = 0; i < 3; i++)
	    {
	      Tcl_DStringAppend(&ds, xyz[i], 2);
	      to = Tcl_NewDoubleObj(arg->val.point[i]);
	      Tcl_SetVar2Ex(interp, arr, Tcl_DStringValue(&ds), to,
			    TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
	      Tcl_DStringSetLength(&ds, Tcl_DStringLength(&ds)-2);
	    }
	  Tcl_DStringFree(&ds);
	  break;
	case AY_SASCALAR:
	  Tcl_SetVar2(interp, arr, "ArgTypes", "float", TCL_LEAVE_ERR_MSG |
		      TCL_GLOBAL_ONLY | TCL_APPEND_VALUE | TCL_LIST_ELEMENT);
	  to = Tcl_NewDoubleObj(arg->val.scalar);
	  Tcl_SetVar2Ex(interp, arr, arg->name, to,
			TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
	  break;

	case AY_SASTRING:
	  Tcl_SetVar2(interp, arr, "ArgTypes", "string", TCL_LEAVE_ERR_MSG |
		      TCL_GLOBAL_ONLY | TCL_APPEND_VALUE | TCL_LIST_ELEMENT);
	  if(arg->val.string)
	    Tcl_SetVar2(interp, arr, arg->name, arg->val.string,
			TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
	  else
	    Tcl_SetVar2(interp, arr, arg->name, "",
			TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
	  break;

	case AY_SAMATRIX:
	  Tcl_SetVar2(interp, arr, "ArgTypes", "matrix", TCL_LEAVE_ERR_MSG |
		      TCL_GLOBAL_ONLY | TCL_APPEND_VALUE | TCL_LIST_ELEMENT);

	  for(i = 0; i < 16; i++)
	    {
	      Tcl_DStringInit(&ds);
	      Tcl_DStringAppend(&ds, arg->name, -1);
	      Tcl_DStringAppend(&ds, man[i], -1);
	      to = Tcl_NewDoubleObj((double)(arg->val.matrix[i]));
	      Tcl_SetVar2Ex(interp, arr, Tcl_DStringValue(&ds), to,
			    TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
	      Tcl_DStringFree(&ds);
	    } /* for */
	  break;

	default:
	  Tcl_SetVar2(interp,n1,"ArgTypes", "eimer", TCL_LEAVE_ERR_MSG |
		      TCL_GLOBAL_ONLY | TCL_APPEND_VALUE | TCL_LIST_ELEMENT);

	  break;
	} /* switch */
      arg = arg->next;
    } /* while */

 return TCL_OK;
} /* ay_shader_gettcmd */


/* ay_shader_settcmd:
 *  Tcl -> C
 */
int
ay_shader_settcmd(ClientData clientData, Tcl_Interp *interp,
		  int argc, char *argv[])
{
 ay_object *o = NULL;
 ay_list_object *sel = ay_selection;
 ay_root_object *root = NULL;
 ay_light_object *light = NULL;
 ay_mat_object *material = NULL;
 ay_shader *newshader = NULL, **shader = NULL;
 ay_shader_arg *newarg = NULL, **argnext = NULL;
 const char *result;
 char *arr = NULL;
 int sargnc = 0, sargtc = 0, i, j, shadertype = 0, argtype = 0;
 double dtemp = 0.0;
 char **sargnv = NULL, **sargtv = NULL;
 Tcl_Obj *to = NULL;
 char *man[] = {"_0","_1","_2","_3","_4","_5","_6","_7","_8","_9","_10","_11","_12","_13","_14","_15"};
 char *rgb[3] ={"_R","_G","_B"};
 char *xyz[3] = {"_X","_Y","_Z"};
 Tcl_DString ds;

  if(!sel) /* oops? */
    o = ay_root;
  else
    o = sel->object;

  /* parse args */
  if(argc < 2)
    {
      ay_error(AY_EARGS, argv[0], "shadertype [varname]");
      return TCL_OK;
    }

  switch(argv[1][0])
    {
    case 's':
      shadertype = AY_STSURFACE;
      break;
    case 'd':
      shadertype = AY_STDISPLACEMENT;
      break;
    case 'i':
      if(argv[1][1] == 'n')
	shadertype = AY_STINTERIOR;
      else
	shadertype = AY_STIMAGER;
      break;
    case 'e':
      shadertype = AY_STEXTERIOR;
      break;
    case 'a':
      shadertype = AY_STATMOSPHERE;
      break;
    case 'l':
      shadertype = AY_STLIGHT;
      break;
    case 't':
      shadertype = AY_STTRANSFORMATION;
      break;
    default:
      ay_error(AY_EARGS, argv[0], "unknown shadertype");
      return TCL_OK;
    }

  /* get shader */
  switch(shadertype)
    {
    case AY_STIMAGER:
      if(o != ay_root)
	{
	  ay_error(AY_ERROR, argv[0], "only root has imager");
	  return TCL_OK;
	}
      else
	{
	  root = (ay_root_object*)o->refine;
	  shader = &(root->imager);
	  shadertype = AY_STVOLUME;
	}
      break;
    case AY_STATMOSPHERE:
      if(o != ay_root)
	{
	  ay_error(AY_ERROR, argv[0], "only root has atmosphere");
	  return TCL_OK;
	}
      else
	{
	  root = (ay_root_object*)o->refine;
	  shader = &(root->atmosphere);
	  shadertype = AY_STVOLUME;
	}
      break;
    case AY_STLIGHT:
      if(o->type != AY_IDLIGHT)
	{
	  ay_error(AY_ERROR, argv[0], "only light objects have light shaders");
	  return TCL_OK;
	}
      else
	{
	  light = (ay_light_object*)o->refine;
	  shader = &(light->lshader);
	}
      break;
    case AY_STSURFACE:
      if(o->type != AY_IDMATERIAL)
	{
	  ay_error(AY_ERROR, argv[0],
		   "only material objects have surface shaders");
	  return TCL_OK;
	}
      else
	{
	  material = (ay_mat_object*)o->refine;
	  shader = &(material->sshader);
	}
      break;
    case AY_STDISPLACEMENT:
      if(o->type != AY_IDMATERIAL)
	{
	  ay_error(AY_ERROR, argv[0],
		   "only material objects have displacement shaders");
	  return TCL_OK;
	}
      else
	{
	  material = (ay_mat_object*)o->refine;
	  shader = &(material->dshader);
	}
      break;
    case AY_STINTERIOR:
      if(o->type != AY_IDMATERIAL)
	{
	  ay_error(AY_ERROR, argv[0],
		   "only material objects have interior shaders");
	  return TCL_OK;
	}
      else
	{
	  material = (ay_mat_object*)o->refine;
	  shader = &(material->ishader);
	}
      break;
    case AY_STEXTERIOR:
      if(o->type != AY_IDMATERIAL)
	{
	  ay_error(AY_ERROR, argv[0],
		   "only material objects have exterior shaders");
	  return TCL_OK;
	}
      else
	{
	  material = (ay_mat_object*)o->refine;
	  shader = &(material->eshader);
	}
      break;
    default:
      return TCL_OK;
    } /* switch */

  if(!shader)
    return TCL_OK;

  /* always delete old shader */
  ay_shader_free(*shader);
  *shader = NULL;

  /* user specified no new shader -> bail out! */
  if(argc < 3)
    return TCL_OK;

  /* prepare Tcl Objects */
  arr = argv[2];

  /* get shadername */
  result = Tcl_GetVar2(interp, arr, "Name",
		       TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  if(!result || *result == '\0')
    {
      /* user specified no new shader -> bail out! */
      goto cleanup;
    }

  /* create shader-struct */
  if(!(newshader = calloc(1, sizeof(ay_shader))))
    {
      ay_error(AY_EOMEM, argv[0], NULL);
      goto cleanup;
    }
  if(!(newshader->name = calloc(strlen(result)+1, sizeof(char))))
    {
      free(newshader);
      ay_error(AY_EOMEM, argv[0], NULL);
      goto cleanup;
    }

  strcpy(newshader->name, result);

  newshader->type = shadertype;

  /* decompose argument-list */
  Tcl_SplitList(interp, Tcl_GetVar2(interp, arr, "ArgNames",
				    TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY),
		&sargnc, &sargnv);
  Tcl_SplitList(interp, Tcl_GetVar2(interp, arr, "ArgTypes",
				    TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY),
		&sargtc, &sargtv);

  argnext = &(newshader->arg);
  for(i = 0; i < sargnc; i++)
    {
      /* which argtype? */
      switch(sargtv[i][0])
	{
	case 'f':
	  /* float */
	  argtype = AY_SASCALAR;
	  break;
	case 'c':
	  /* color */
	  argtype = AY_SACOLOR;
	  break;
	case 'p':
	  /* point */
	  argtype = AY_SAPOINT;
	  break;
	case 's':
	  /* string */
	  argtype = AY_SASTRING;
	  break;
	case 'v':
	  /* vector */
	  argtype = AY_SAVECTOR;
	  break;
	case 'n':
	  /* normal */
	  argtype = AY_SANORMAL;
	  break;
	case 'm':
	  /* matrix */
	  argtype = AY_SAMATRIX;
	  break;
	default:
	  ay_shader_free(newshader);
	  ay_error(AY_ERROR, argv[0], "unknown argtype");
	  ay_error(AY_ERROR, argv[0], sargtv[i]);
	  goto cleanup;
	}

      /* create new shader-arg-struct */
      if(!(newarg = calloc(1, sizeof(ay_shader_arg))))
	{
	  ay_shader_free(newshader);
	  ay_error(AY_EOMEM, argv[0], NULL);
	  goto cleanup;
	}
      *argnext = newarg;
      argnext = &(newarg->next);

      newarg->type = argtype;

      if(!(newarg->name = calloc(strlen(sargnv[i])+1, sizeof(char))))
	{
	  ay_shader_free(newshader);
	  ay_error(AY_EOMEM, argv[0], NULL);
	  goto cleanup;
	}
      strcpy(newarg->name, sargnv[i]);

      switch(argtype)
	{
	case AY_SACOLOR:
	  Tcl_DStringInit(&ds);
	  Tcl_DStringAppend(&ds, sargnv[i], -1);
	  for(j = 0; j < 3; j++)
	    {
	      Tcl_DStringAppend(&ds, rgb[j], 2);
	      to = Tcl_GetVar2Ex(interp, arr, Tcl_DStringValue(&ds),
				 TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
	      Tcl_GetDoubleFromObj(interp, to, &dtemp);
	      if(dtemp < 0) dtemp = 0.0;
	      if(dtemp > 255) dtemp = 255.0;
	      newarg->val.color[j] = (float)(dtemp/255.0);
	      Tcl_DStringSetLength(&ds, Tcl_DStringLength(&ds)-2);
	    }
	  Tcl_DStringFree(&ds);
	  break;

	case AY_SAPOINT:
	case AY_SAVECTOR:
	case AY_SANORMAL:
	  Tcl_DStringInit(&ds);
	  Tcl_DStringAppend(&ds, sargnv[i], -1);
	  for(j = 0; j < 3; j++)
	    {
	      Tcl_DStringAppend(&ds, xyz[j], 2);
	      to = Tcl_GetVar2Ex(interp, arr, Tcl_DStringValue(&ds),
				  TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
	      Tcl_GetDoubleFromObj(interp, to, &dtemp);
	      newarg->val.point[j] = (float)dtemp;
	      Tcl_DStringSetLength(&ds, Tcl_DStringLength(&ds)-2);
	    }
	  Tcl_DStringFree(&ds);
	  break;

	case AY_SASCALAR:
	  to = Tcl_GetVar2Ex(interp, arr, sargnv[i], TCL_LEAVE_ERR_MSG |
			     TCL_GLOBAL_ONLY);
	  Tcl_GetDoubleFromObj(interp, to, &dtemp);
	  newarg->val.scalar = (float)dtemp;
	  break;

	case AY_SASTRING:
	  result = Tcl_GetVar2(interp, arr, sargnv[i], TCL_LEAVE_ERR_MSG |
			       TCL_GLOBAL_ONLY);
	  if(!(newarg->val.string = calloc(strlen(result)+1, sizeof(char))))
	  {
	    ay_shader_free(newshader);
	    ay_error(AY_EOMEM, argv[0], NULL);
	    goto cleanup;
	  }
	  strcpy(newarg->val.string, result);
	  break;

	case AY_SAMATRIX:
	  for(j = 0; j < 16; j++)
	    {
	      Tcl_DStringInit(&ds);
	      Tcl_DStringAppend(&ds, sargnv[i], -1);
	      Tcl_DStringAppend(&ds, man[j], -1);
	      to = Tcl_GetVar2Ex(interp, arr, Tcl_DStringValue(&ds),
				 TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
	      Tcl_GetDoubleFromObj(interp, to, &dtemp);
	      newarg->val.matrix[j] = (float)dtemp;
	      Tcl_DStringFree(&ds);
	    } /* for */
	  break;

	default:
	  break;
	} /* switch */
    } /* for */

  /* add shader to object */
  *shader = newshader;

  /* clean up */
cleanup:
  if(sargnv)
    Tcl_Free((char *) sargnv);
  if(sargtv)
    Tcl_Free((char *) sargtv);

  ay_notify_object(o);

 return TCL_OK;
} /* ay_shader_settcmd */


