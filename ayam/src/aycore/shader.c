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
 *  scan a shader compiled with slc with libslcargs.a
 */
int
ay_shader_scanslctcmd(ClientData clientData, Tcl_Interp *interp,
		      int argc, char *argv[])
{
 char fname[] = "shaderScanSLC";
#ifdef AYUSESLCARGS
 int i = 0, j = 0, numargs = 0;
 SLC_VISSYMDEF *symbol = NULL, *element = NULL;
 SLC_TYPE type;
 char buffer[255];
 int arraylen;
 Tcl_DString ds;

  if(argc < 3)
    {
      ay_error(AY_EARGS, fname, "shaderpath varname");
      return TCL_OK;
    }

  if((SLC_SetShader(argv[1])) == -1)
    {
      ay_error(AY_ERROR, fname, "SLC_SetShader failed for:");
      ay_error(AY_ERROR, fname, argv[1]);
      return TCL_OK;
    }

  Tcl_DStringInit(&ds);

  /* get name of shader */
  Tcl_DStringAppend(&ds, argv[1]/*SLC_GetName()*/, -1);

  /* get type of shader */
  type = SLC_GetType();
  switch (type)
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
	  ay_error(AY_ERROR, fname, "Cannot get symbol from shader:");
	  ay_error(AY_ERROR, fname, argv[1]);
	  SLC_EndShader();
	  Tcl_DStringFree(&ds);
	  return TCL_OK;
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
		  ay_error(AY_ERROR, fname, "Could not get array element:");
		  ay_error(AY_ERROR, fname, symbol->svd_name);
		  Tcl_DStringFree(&ds);
		  return TCL_OK;
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
	  ay_error(AY_EWARN,fname,"Skipping array argument!");
	  /*	  ay_error(AY_EWARN,fname,symbol->svd_name);*/
	} /* if */
      /* XXXX temporarily discard array arguments */
    } /* for */
  Tcl_DStringAppend(&ds, "} ", -1);


  SLC_EndShader();

  Tcl_SetVar(interp, argv[2], Tcl_DStringValue(&ds), TCL_LEAVE_ERR_MSG);

  Tcl_DStringFree(&ds);
#else
 ay_error(AY_ERROR, fname, "This Ayam has not been linked with libslcargs!");
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
 char fname[] = "shaderScanSLX";
#ifdef AYUSESLXARGS
 int i = 0, j = 0, numargs = 0;
 SLX_VISSYMDEF *symbol = NULL, *element = NULL;
 SLX_TYPE type;
 char buffer[255];
 int arraylen;
 Tcl_DString ds;
 char vname[] = "ayprefs(Shaders)";
 char *c = NULL;

  if(argc < 3)
    {
      ay_error(AY_EARGS, fname, "shaderpath varname");
      return TCL_OK;
    }

#ifndef WIN32
  SLX_SetPath(Tcl_GetVar(ay_interp, vname, TCL_GLOBAL_ONLY|TCL_LEAVE_ERR_MSG));
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
  Tcl_DStringFree(&ds);
#endif /* WIN32 */

  if((SLX_SetShader(argv[1])) == -1)
    {
      ay_error(AY_ERROR, fname, "SLX_SetShader failed for:");
      ay_error(AY_ERROR, fname, argv[1]);
      return TCL_OK;
    }

  Tcl_DStringInit(&ds);

  /* get name of shader */
  Tcl_DStringAppend(&ds, argv[1]/*SLX_GetName()*/, -1);

  /* get type of shader */
  type = SLX_GetType();
  switch (type)
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
	  ay_error(AY_ERROR, fname, "Cannot get symbol from shader:");
	  ay_error(AY_ERROR, fname, argv[1]);
	  /*
	  SLX_EndShader();
	  Tcl_DStringFree(&ds);
	  return TCL_OK;
	  */
	}

      /* XXXX temporarily discard array arguments   */
      if(symbol->svd_arraylen < 1)
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

      arraylen = symbol->svd_arraylen;
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
		  ay_error(AY_ERROR, fname, "Could not get array element:");
		  ay_error(AY_ERROR, fname, symbol->svd_name);
		  Tcl_DStringFree(&ds);
		  return TCL_OK;
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
	  ay_error(AY_EWARN,fname,"Skipping array argument!");
	  /*	  ay_error(AY_EWARN,fname,symbol->svd_name);*/
	} /* if */
      /* XXXX temporarily discard array arguments */
    } /* for */
  Tcl_DStringAppend(&ds, "} ", -1);


  SLX_EndShader();

  Tcl_SetVar(interp, argv[2], Tcl_DStringValue(&ds), TCL_LEAVE_ERR_MSG);

  Tcl_DStringFree(&ds);
#else
 ay_error(AY_ERROR, fname, "This Ayam has not been linked with libslxargs!");
 ay_error(AY_ERROR, fname,
	  "Load the ayslx plugin and use shaderScan instead!");
#endif /* AYUSESLXARGS */
 return TCL_OK;
} /* ay_shader_scanslxtcmd */


/* ay_shader_free:
 *  free a shader and associated arguments
 */
int
ay_shader_free(ay_shader *shader)
{
 int ay_status = AY_OK;
 ay_shader_arg *arg = NULL, *argtmp = NULL;

  if(!shader)
    return AY_OK;

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

	  if(arg->type == AY_SASTRING)
	    {
	      free(arg->val.string);
	    }

	  free(arg);
	  arg = argtmp;
	}

    }

  if(shader->name)
    free(shader->name);

  /* finally, delete the shader*/
  free(shader);

 return ay_status;
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

  if(!(newp = calloc(1, sizeof(ay_shader_arg))))
    return AY_EOMEM; 

  memcpy(newp, source, sizeof(ay_shader_arg)); 
  /* danger! links point to original hierachy */

  /* copy name */
  if(!(newval = calloc(1, strlen(source->name)+1)))
    { free(newp); return AY_EOMEM; }
  strcpy(newval, source->name);
  newp->name = newval;

  if(source->type == AY_SASTRING)
    {
      /* copy string */
      newval = NULL;
      if(!(newval = calloc(1, strlen(source->val.string)+1)))
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

  if(!(news = calloc(1,sizeof(ay_shader))))
    return AY_EOMEM; 

  memcpy(news, source, sizeof(ay_shader)); 
  /* danger! links point to original hierachy */

  /* copy name */
  if(!(newval = calloc(1, strlen(source->name)+1)))
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
	    return(AY_EOMEM);
	  if(!(values = calloc(count, sizeof(RtToken))))
	    { free(tokens); return(AY_EOMEM); }
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
	    }

	  tokens[count] = sarg->name;
	  
	  count++;
	  sarg = sarg->next;
	}

      switch(type)
	{
	case AY_STSURFACE:
	  RiSurfaceV(shader->name,count,tokens,values);
	  break;
	case AY_STDISPLACEMENT:
	  RiDisplacementV(shader->name,count,tokens,values);
	  break;
	case AY_STLIGHT:
	  *light_handle = RiLightSourceV(shader->name,count,
					   tokens,values);
	  break;
	case AY_STAREALIGHT:
	  *light_handle = RiAreaLightSourceV(shader->name,count,
					       tokens,values);
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
/* XXXX RiDeformationV() is missing from BMRT2.6 libribout.a? */
#ifndef AYUSEBMRTRIBOUT
	case AY_STTRANSFORMATION:
	  RiDeformationV(shader->name,count,tokens,values);
	  break;
#endif
	}

      if(tokens)
	free(tokens);
      tokens = NULL;
      if(values)
	free(values);
      values = NULL;

    } /* if */

 return ay_status;
} /* ay_shader_wrib */


/* ay_shader_gettcmd:
 *  C -> Tcl
 */
int
ay_shader_gettcmd(ClientData clientData, Tcl_Interp * interp,
		  int argc, char *argv[])
{
 ay_object *o = NULL;
 ay_list_object *sel = ay_selection;
 ay_root_object *root = NULL;
 ay_light_object *light = NULL;
 ay_mat_object *material = NULL;
 ay_shader *shader = NULL;
 ay_shader_arg *arg = NULL;
 char *n1 = NULL, *sname = NULL/*, *sfile = NULL*/;
 int shadertype = 0, itemp = 0, i;
 Tcl_Obj *to = NULL, *toa = NULL, *ton = NULL;
 char fname[] = "shaderGet";
 char *man[] = {"_0","_1","_2","_3","_4","_5","_6","_7","_8","_9","_10","_11","_12","_13","_14","_15"};

  if(!sel) /* oops? */
    o = ay_root;
  else
    o = sel->object;

  /* parse args */
  if(argc < 2)
    {
      ay_error(AY_EARGS, fname, "shadertype varname");
      return TCL_OK;
    }

  if(strstr(argv[1],"surface"))
    shadertype = AY_STSURFACE;
  else
  if(strstr(argv[1],"displacement"))
    shadertype = AY_STDISPLACEMENT;
  else
  if(strstr(argv[1],"interior"))
    shadertype = AY_STINTERIOR;
  else
  if(strstr(argv[1],"exterior"))
    shadertype = AY_STEXTERIOR;
  else
  if(strstr(argv[1],"atmosphere"))
    shadertype = AY_STATMOSPHERE;
  else
  if(strstr(argv[1],"light"))
    shadertype = AY_STLIGHT;
  else
  if(strstr(argv[1],"imager"))
    shadertype = AY_STIMAGER;
  else
  if(strstr(argv[1],"transformation"))
    shadertype = AY_STTRANSFORMATION;

  /* get shader */
  switch(shadertype)
    {
    case AY_STIMAGER:
      if(o != ay_root)
	{
	  ay_error(AY_ERROR, fname, "only root has imager");
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
	  ay_error(AY_ERROR, fname, "only root has atmosphere");
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
	  ay_error(AY_ERROR, fname, "only light objects have light shaders");
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
	  ay_error(AY_ERROR, fname,
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
	  ay_error(AY_ERROR, fname,
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
	  ay_error(AY_ERROR, fname,
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
	  ay_error(AY_ERROR, fname,
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


  n1 = argv[2];

  Tcl_SetVar2(interp,n1,"Name","",TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  if(!shader)
    { return TCL_OK; }

  toa = Tcl_NewStringObj(n1,-1);
  ton = Tcl_NewStringObj(n1,-1);


  arg = shader->arg;
  sname = shader->name;
  /*	  sfile = shader->loc;*/

  Tcl_SetVar2(interp,n1,"Name",sname,TCL_LEAVE_ERR_MSG |
	      TCL_GLOBAL_ONLY);
  /*  Tcl_SetVar2(interp,n1,"File",sfile,TCL_LEAVE_ERR_MSG |
	      TCL_GLOBAL_ONLY);*/
  Tcl_SetVar2(interp,n1,"ArgNames","",TCL_LEAVE_ERR_MSG |
	      TCL_GLOBAL_ONLY);
  Tcl_SetVar2(interp,n1,"ArgTypes","",TCL_LEAVE_ERR_MSG |
	      TCL_GLOBAL_ONLY);

  while(arg)
    {
      Tcl_SetVar2(interp,n1,"ArgNames",arg->name, TCL_LEAVE_ERR_MSG |
		  TCL_GLOBAL_ONLY | TCL_APPEND_VALUE | TCL_LIST_ELEMENT);     

      switch(arg->type)
	{
	case AY_SACOLOR:
	  Tcl_SetVar2(interp,n1,"ArgTypes", "color", TCL_LEAVE_ERR_MSG |
		      TCL_GLOBAL_ONLY | TCL_APPEND_VALUE | TCL_LIST_ELEMENT);

	  Tcl_SetStringObj(ton,arg->name,-1);
	  Tcl_AppendToObj(ton, "_R", -1);

	  itemp = (int)(arg->val.color[0]*255);

	  to = Tcl_NewIntObj(itemp);
	  Tcl_ObjSetVar2(interp,toa,ton,to, TCL_LEAVE_ERR_MSG |
			 TCL_GLOBAL_ONLY);

	  Tcl_SetStringObj(ton,arg->name,-1);
	  Tcl_AppendToObj(ton, "_G", -1);
	  itemp = (int)(arg->val.color[1]*255);
	  to = Tcl_NewIntObj(itemp);
	  Tcl_ObjSetVar2(interp,toa,ton,to, TCL_LEAVE_ERR_MSG |
			 TCL_GLOBAL_ONLY);

	  Tcl_SetStringObj(ton,arg->name,-1);
	  Tcl_AppendToObj(ton, "_B", -1);
	  itemp = (int)(arg->val.color[2]*255);
	  to = Tcl_NewIntObj(itemp);
	  Tcl_ObjSetVar2(interp,toa,ton,to, TCL_LEAVE_ERR_MSG |
			 TCL_GLOBAL_ONLY);

	  break;

	case AY_SAPOINT:
	  Tcl_SetVar2(interp,n1,"ArgTypes", "point", TCL_LEAVE_ERR_MSG |
		      TCL_GLOBAL_ONLY | TCL_APPEND_VALUE | TCL_LIST_ELEMENT);

	  Tcl_SetStringObj(ton,arg->name,-1);
	  Tcl_AppendToObj(ton, "_X", -1);
	  to = Tcl_NewDoubleObj(arg->val.point[0]);
	  Tcl_ObjSetVar2(interp,toa,ton,to, TCL_LEAVE_ERR_MSG |
			 TCL_GLOBAL_ONLY);
	  Tcl_SetStringObj(ton,arg->name,-1);
	  Tcl_AppendToObj(ton, "_Y", -1);
	  to = Tcl_NewDoubleObj(arg->val.point[1]);
	  Tcl_ObjSetVar2(interp,toa,ton,to, TCL_LEAVE_ERR_MSG |
			 TCL_GLOBAL_ONLY);
	  Tcl_SetStringObj(ton,arg->name,-1);
	  Tcl_AppendToObj(ton, "_Z", -1);
	  to = Tcl_NewDoubleObj(arg->val.point[2]);
	  Tcl_ObjSetVar2(interp,toa,ton,to, TCL_LEAVE_ERR_MSG |
			 TCL_GLOBAL_ONLY);
	  break;

	case AY_SAVECTOR:
	  Tcl_SetVar2(interp,n1,"ArgTypes", "vector", TCL_LEAVE_ERR_MSG |
		      TCL_GLOBAL_ONLY | TCL_APPEND_VALUE | TCL_LIST_ELEMENT);

	  Tcl_SetStringObj(ton,arg->name,-1);
	  Tcl_AppendToObj(ton, "_X", -1);
	  to = Tcl_NewDoubleObj(arg->val.point[0]);
	  Tcl_ObjSetVar2(interp,toa,ton,to, TCL_LEAVE_ERR_MSG |
			 TCL_GLOBAL_ONLY);
	  Tcl_SetStringObj(ton,arg->name,-1);
	  Tcl_AppendToObj(ton, "_Y", -1);
	  to = Tcl_NewDoubleObj(arg->val.point[1]);
	  Tcl_ObjSetVar2(interp,toa,ton,to, TCL_LEAVE_ERR_MSG |
			 TCL_GLOBAL_ONLY);
	  Tcl_SetStringObj(ton,arg->name,-1);
	  Tcl_AppendToObj(ton, "_Z", -1);
	  to = Tcl_NewDoubleObj(arg->val.point[2]);
	  Tcl_ObjSetVar2(interp,toa,ton,to, TCL_LEAVE_ERR_MSG |
			 TCL_GLOBAL_ONLY);
	  break;

	case AY_SANORMAL:
	  Tcl_SetVar2(interp,n1,"ArgTypes", "normal", TCL_LEAVE_ERR_MSG |
		      TCL_GLOBAL_ONLY | TCL_APPEND_VALUE | TCL_LIST_ELEMENT);

	  Tcl_SetStringObj(ton,arg->name,-1);
	  Tcl_AppendToObj(ton, "_X", -1);
	  to = Tcl_NewDoubleObj(arg->val.point[0]);
	  Tcl_ObjSetVar2(interp,toa,ton,to, TCL_LEAVE_ERR_MSG |
			 TCL_GLOBAL_ONLY);
	  Tcl_SetStringObj(ton,arg->name,-1);
	  Tcl_AppendToObj(ton, "_Y", -1);
	  to = Tcl_NewDoubleObj(arg->val.point[1]);
	  Tcl_ObjSetVar2(interp,toa,ton,to, TCL_LEAVE_ERR_MSG |
			 TCL_GLOBAL_ONLY);
	  Tcl_SetStringObj(ton,arg->name,-1);
	  Tcl_AppendToObj(ton, "_Z", -1);
	  to = Tcl_NewDoubleObj(arg->val.point[2]);
	  Tcl_ObjSetVar2(interp,toa,ton,to, TCL_LEAVE_ERR_MSG |
			 TCL_GLOBAL_ONLY);
	  break;

	case AY_SASCALAR:
	  Tcl_SetVar2(interp,n1,"ArgTypes", "float", TCL_LEAVE_ERR_MSG |
		      TCL_GLOBAL_ONLY | TCL_APPEND_VALUE | TCL_LIST_ELEMENT);

	  Tcl_SetStringObj(ton,arg->name,-1);
	  to = Tcl_NewDoubleObj(arg->val.scalar);
	  Tcl_ObjSetVar2(interp,toa,ton,to, TCL_LEAVE_ERR_MSG |
			 TCL_GLOBAL_ONLY);
	  break;

	case AY_SASTRING:
	  Tcl_SetVar2(interp,n1,"ArgTypes", "string", TCL_LEAVE_ERR_MSG |
		      TCL_GLOBAL_ONLY | TCL_APPEND_VALUE | TCL_LIST_ELEMENT);
	  Tcl_SetVar2(interp,n1, arg->name, arg->val.string,
		      TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
	  break;
	case AY_SAMATRIX:
	  Tcl_SetVar2(interp,n1,"ArgTypes", "matrix", TCL_LEAVE_ERR_MSG |
		      TCL_GLOBAL_ONLY | TCL_APPEND_VALUE | TCL_LIST_ELEMENT);
	  for(i = 0; i < 16; i++)
	    {
	      Tcl_SetStringObj(ton,arg->name,-1);
	      Tcl_AppendToObj(ton, man[i], -1);
	      to = Tcl_NewDoubleObj((double)(arg->val.matrix[i]));
	      Tcl_ObjSetVar2(interp,toa,ton,to, TCL_LEAVE_ERR_MSG |
			     TCL_GLOBAL_ONLY);
	    } /* for */
	  break;
	default:
	  Tcl_SetVar2(interp,n1,"ArgTypes", "eimer", TCL_LEAVE_ERR_MSG |
		      TCL_GLOBAL_ONLY | TCL_APPEND_VALUE | TCL_LIST_ELEMENT);

	  break;
	} /* switch */
      arg = arg->next;
    } /* while */

  Tcl_IncrRefCount(toa);Tcl_DecrRefCount(toa);
  Tcl_IncrRefCount(ton);Tcl_DecrRefCount(ton);

 return TCL_OK;
} /* ay_shader_gettcmd */


/* ay_shader_settcmd:
 *  Tcl -> C
 */
int
ay_shader_settcmd(ClientData clientData, Tcl_Interp * interp,
		  int argc, char *argv[])
{
 ay_object *o = NULL;
 ay_list_object *sel = ay_selection;
 ay_root_object *root = NULL;
 ay_light_object *light = NULL;
 ay_mat_object *material = NULL;
 ay_shader *newshader = NULL, **shader = NULL;
 ay_shader_arg *newarg = NULL, **argnext = NULL;
 char *result, *n1=NULL;
 int sargnc = 0, sargtc = 0, i, j, shadertype = 0, argtype = 0;
 double dtemp = 0.0;
 char **sargnv, **sargtv;
 Tcl_Obj *to = NULL, *toa = NULL, *ton = NULL;
 char fname[] = "shaderSet";
 char *man[] = {"_0","_1","_2","_3","_4","_5","_6","_7","_8","_9","_10","_11","_12","_13","_14","_15"};

  if(!sel) /* oops? */
    o = ay_root;
  else
    o = sel->object;

  /* parse args */
  if(argc < 2)
    {
      ay_error(AY_EARGS, fname, "shadertype \\[varname\\]");
      return TCL_OK;
    }

  if(strstr(argv[1],"surface"))
    shadertype = AY_STSURFACE;
  else
  if(strstr(argv[1],"displacement"))
    shadertype = AY_STDISPLACEMENT;
  else
  if(strstr(argv[1],"interior"))
    shadertype = AY_STINTERIOR;
  else
  if(strstr(argv[1],"exterior"))
    shadertype = AY_STEXTERIOR;
  else
  if(strstr(argv[1],"atmosphere"))
    shadertype = AY_STATMOSPHERE;
  else
  if(strstr(argv[1],"light"))
    shadertype = AY_STLIGHT;
  else
  if(strstr(argv[1],"imager"))
    shadertype = AY_STIMAGER;
  else
  if(strstr(argv[1],"transformation"))
    shadertype = AY_STTRANSFORMATION;


  /* get shader */
  switch(shadertype)
    {
    case AY_STIMAGER:
      if(o != ay_root)
	{
	  ay_error(AY_ERROR, fname, "only root has imager");
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
	  ay_error(AY_ERROR, fname, "only root has atmosphere");
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
	  ay_error(AY_ERROR, fname, "only light objects have light shaders");
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
	  ay_error(AY_ERROR, fname,
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
	  ay_error(AY_ERROR, fname,
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
	  ay_error(AY_ERROR, fname,
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
	  ay_error(AY_ERROR, fname,
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
  n1 = argv[2];
  toa = Tcl_NewStringObj(n1,-1);
  ton = Tcl_NewStringObj(n1,-1);

  /* get shadername */
  result = Tcl_GetVar2(interp, n1, "Name",
		       TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  /* user specified no new shader -> bail out! */
  if(!strcmp("",result))
    {
      return TCL_OK;
    }

  /* create shader-struct */
  if(!(newshader = calloc(1, sizeof(ay_shader))))
    {
      Tcl_IncrRefCount(toa);Tcl_DecrRefCount(toa);
      Tcl_IncrRefCount(ton);Tcl_DecrRefCount(ton);
      ay_error(AY_EOMEM, fname, NULL);
      return TCL_OK;
    }
  if(!(newshader->name = calloc(strlen(result)+1, sizeof(char))))
    {
      free(newshader);
      Tcl_IncrRefCount(toa);Tcl_DecrRefCount(toa);
      Tcl_IncrRefCount(ton);Tcl_DecrRefCount(ton);
      ay_error(AY_EOMEM, fname, NULL);
      return TCL_OK;
    }

  strcpy(newshader->name, result);

  newshader->type = shadertype;

  /* decompose argument-list */
  Tcl_SplitList(interp,Tcl_GetVar2(interp, n1, "ArgNames",
				   TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY),
		&sargnc, &sargnv);
  Tcl_SplitList(interp,Tcl_GetVar2(interp, n1, "ArgTypes",
				   TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY),
		&sargtc, &sargtv);


  argnext = &(newshader->arg);
  for(i=0; i<sargnc; i++)
    {
      /* which argtype? */
      if(!strcmp(sargtv[i],"float"))
	argtype = AY_SASCALAR;
      else
      if(!strcmp(sargtv[i],"color"))
	argtype = AY_SACOLOR;
      else
      if(!strcmp(sargtv[i],"point"))
	argtype = AY_SAPOINT;
      else
      if(!strcmp(sargtv[i],"string"))
	argtype = AY_SASTRING;
      else
      if(!strcmp(sargtv[i],"vector"))
	argtype = AY_SAVECTOR;
      else
      if(!strcmp(sargtv[i],"normal"))
	argtype = AY_SANORMAL;
      else
      if(!strcmp(sargtv[i],"matrix"))
	argtype = AY_SAMATRIX;

      /* create new shader-arg-struct */
      if(!(newarg = calloc(1, sizeof(ay_shader_arg))))
	{
	  ay_shader_free(newshader);
	  Tcl_Free((char *) sargnv);
	  Tcl_Free((char *) sargtv);
	  Tcl_IncrRefCount(toa);Tcl_DecrRefCount(toa);
	  Tcl_IncrRefCount(ton);Tcl_DecrRefCount(ton);

	  ay_error(AY_EOMEM, fname, NULL);
	  return TCL_OK;
	}
      *argnext = newarg;
      argnext = &(newarg->next);

      newarg->type = argtype;

      if(!(newarg->name = calloc(strlen(sargnv[i])+1, sizeof(char))))
	{
	  ay_shader_free(newshader);
	  Tcl_Free((char *) sargnv);
	  Tcl_Free((char *) sargtv);
	  Tcl_IncrRefCount(toa);Tcl_DecrRefCount(toa);
	  Tcl_IncrRefCount(ton);Tcl_DecrRefCount(ton);

	  ay_error(AY_EOMEM, fname, NULL);
	  return TCL_OK;
	}
      strcpy(newarg->name, sargnv[i]);

      switch(argtype)
	{
	case AY_SACOLOR:
	  Tcl_SetStringObj(ton,sargnv[i],-1);
	  Tcl_AppendStringsToObj(ton,"_R",NULL);
	  to  = Tcl_ObjGetVar2(interp, toa, ton, TCL_LEAVE_ERR_MSG |
			       TCL_GLOBAL_ONLY);
	  Tcl_GetDoubleFromObj(interp, to, &dtemp);
	  if(dtemp < 0) dtemp = 0.0;
	  if(dtemp > 255) dtemp = 255.0;
	  newarg->val.color[0]=(float)(dtemp/255.0);

	  Tcl_SetStringObj(ton,sargnv[i],-1);
	  Tcl_AppendStringsToObj(ton,"_G",NULL);
	  to  = Tcl_ObjGetVar2(interp, toa, ton, TCL_LEAVE_ERR_MSG |
			       TCL_GLOBAL_ONLY);
	  Tcl_GetDoubleFromObj(interp, to, &dtemp);
	  if(dtemp < 0) dtemp = 0.0;
	  if(dtemp > 255) dtemp = 255.0;
	  newarg->val.color[1]=(float)(dtemp/255.0);

	  Tcl_SetStringObj(ton,sargnv[i],-1);
	  Tcl_AppendStringsToObj(ton,"_B",NULL);
	  to  = Tcl_ObjGetVar2(interp, toa, ton, TCL_LEAVE_ERR_MSG |
			       TCL_GLOBAL_ONLY);
	  Tcl_GetDoubleFromObj(interp, to, &dtemp);
	  if(dtemp < 0) dtemp = 0.0;
	  if(dtemp > 255) dtemp = 255.0;
	  newarg->val.color[2]=(float)(dtemp/255.0);

	  break;
	case AY_SAPOINT:
	case AY_SAVECTOR:
	case AY_SANORMAL:
	  Tcl_SetStringObj(ton,sargnv[i],-1);
	  Tcl_AppendStringsToObj(ton,"_X",NULL);
	  to  = Tcl_ObjGetVar2(interp, toa, ton, TCL_LEAVE_ERR_MSG |
			       TCL_GLOBAL_ONLY);
	  Tcl_GetDoubleFromObj(interp, to, &dtemp);
	  newarg->val.point[0] = (float)dtemp;

	  Tcl_SetStringObj(ton,sargnv[i],-1);
	  Tcl_AppendStringsToObj(ton,"_Y",NULL);
	  to  = Tcl_ObjGetVar2(interp, toa, ton, TCL_LEAVE_ERR_MSG |
			       TCL_GLOBAL_ONLY);
	  Tcl_GetDoubleFromObj(interp, to, &dtemp);
	  newarg->val.point[1] = (float)dtemp;

	  Tcl_SetStringObj(ton,sargnv[i],-1);
	  Tcl_AppendStringsToObj(ton,"_Z",NULL);
	  to  = Tcl_ObjGetVar2(interp, toa, ton, TCL_LEAVE_ERR_MSG |
			       TCL_GLOBAL_ONLY);
	  Tcl_GetDoubleFromObj(interp, to, &dtemp);
	  newarg->val.point[2] = (float)dtemp;
	  break;
	case AY_SASCALAR:
	  Tcl_SetStringObj(ton,sargnv[i],-1);
	  to  = Tcl_ObjGetVar2(interp, toa, ton, TCL_LEAVE_ERR_MSG |
			       TCL_GLOBAL_ONLY);
	  Tcl_GetDoubleFromObj(interp, to, &dtemp);
	  newarg->val.scalar = (float)dtemp;
	  break;
	case AY_SASTRING:
	  result = Tcl_GetVar2(interp, n1, sargnv[i], TCL_LEAVE_ERR_MSG |
			       TCL_GLOBAL_ONLY);
	  if(!(newarg->val.string = calloc(strlen(result)+1, sizeof(char))))
	  {
	    ay_shader_free(newshader);
	    Tcl_Free((char *) sargnv);
	    Tcl_Free((char *) sargtv);
	    ay_error(AY_EOMEM, fname, NULL);
	    return TCL_OK;
	  }
	  strcpy(newarg->val.string, result);
	  break;
	case AY_SAMATRIX:
	  for(j = 0; j < 16; j++)
	    {
	      Tcl_SetStringObj(ton,sargnv[i],-1);
	      Tcl_AppendStringsToObj(ton,man[j],NULL);
	      to  = Tcl_ObjGetVar2(interp, toa, ton, TCL_LEAVE_ERR_MSG |
				   TCL_GLOBAL_ONLY);
	      Tcl_GetDoubleFromObj(interp, to, &dtemp);
	      newarg->val.matrix[j] = (float)dtemp;
	    } /* for */
	  break;

	} /* switch */

    } /* for */

  /* add shader to object */
  *shader = newshader;

  /* clean up */
  Tcl_Free((char *) sargnv);
  Tcl_Free((char *) sargtv);

  Tcl_IncrRefCount(toa);Tcl_DecrRefCount(toa);
  Tcl_IncrRefCount(ton);Tcl_DecrRefCount(ton);

  ay_notify_force(o);

 return TCL_OK;
} /* ay_shader_settcmd */


