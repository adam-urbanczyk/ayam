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

/* ayslb.c - plugin to scan shaders compiled with shaded (Air)
   using code kindly provided by Scott Iverson (SiTex Graphics) */

/*
 
    Routines for obtaining the parameters for
    shaders compiled to .slb files for AIR
 
    (c) 2000 SiTex Graphics.  All rights reserved.
 
    Last Modified:  July 14, 2000
 
*/

/* prototypes of functions local to this module: */
int ayslb_scanshader(char *fname, Tcl_DString *ds);

int ayslb_scanslbtcmd(ClientData clientData, Tcl_Interp *interp,
		      int argc, char *argv[]);

int Ayslb_Init(Tcl_Interp *interp);

#define MAXSHADERLENGTH 160000
#define MAXNSHADERARGS 4000
#define MAXNSHADERTOKENS (MAXNSHADERARGS*2+3)

typedef char ShaderString [MAXSHADERLENGTH];

enum ShaderType {
  shSurface,
  shDisplace,
  shVolume,
  shImager,
  shLight,
  shTransform,
  shProcedure
};

enum ParameterType {
  pString,
  pFloat,
  pColor,
  pPoint,
  pVector,
  pNormal,
  pMatrix
};

typedef struct {
  char ptype;
  char ptspc;
  short paddr, pnarr;
  short pflags;
  char pname[24];
} Parameter;

typedef struct {
  long nextShader;
  short shsize;
  char type;
  char opt;
  char name[48];
  unsigned short nparm,ncode,ndata,localbase;
  short codix,dataix;
  char vary[32],output[32],cachevar[32];
  short minvary,maxvary,minoutput,maxoutput;
  short shaderflags;
  short ix[8];
  long rsrved[8];
  long code, data;
  Parameter parm[MAXNSHADERARGS];
  char buf[128000];
} ShaderHeader;

typedef struct {
  short magic,size;
} MagicHeader;

/* functions: */
/* ayslb_scanshader: Returns a fully qualified shader
 *  declaration of the shader in file "fname" in the Tcl_DString
 *  pointed to by "ds"
 */
int
ayslb_scanshader(char *fname, Tcl_DString *ds)
{
 MagicHeader h1;
 ShaderHeader h;
 unsigned short i,j,k,na;
 unsigned int jj;
 char *t, buffer[50];
 float *fp;
 FILE *f;

  f = fopen(fname,"rb");
  if(f)
    {
      t = (char *)&h1;
      if(fread(t, 1, 4, f) == 4)
	{
	  t = (char *)&h;
	  jj = h1.size;
	  if(jj > sizeof(h))
	    {
	      jj = sizeof(h);
	    }

	  if(fread(t, 1, sizeof(h), f) > 0)
	    {
	      switch(h.type)
		{
		case shSurface:
		  Tcl_DStringAppend(ds, " surface ", -1);
		  break;
		case shDisplace:
		  Tcl_DStringAppend(ds, " displacement ", -1);
		  break;
		case shVolume:
		  Tcl_DStringAppend(ds, " volume ", -1);
		  break;
		case shImager:
		  Tcl_DStringAppend(ds, " imager ", -1);
		  break;
		case shLight:
		  Tcl_DStringAppend(ds, " light ", -1);
		  break;
		case shTransform:
		  Tcl_DStringAppend(ds, " transformation ", -1);
		  break;
		case shProcedure:
		  ay_error(AY_EWARN, fname,
			   "skipping unsupported procedure shader");
		  return AY_ERROR; /* early exit! */
		  /*Tcl_DStringAppend(ds, " procedure ", -1);*/
		  break;
		default:
		  ay_error(AY_EWARN, fname,
			   "skipping shader of unkown type");
		  return AY_ERROR; /* early exit! */
		  break;
		}

	      /*
		t=fname;
		while (*fname!='\0') {
		if (*fname=='\\') {t=fname; t++;}
		fname++;
		}
		fname=strchr(t,'.');
		if (fname!=NULL) {
		ch=*fname; *fname='\0';
		strcat(s,t); *fname=ch;
		} else
		strcat(s,t);
	      */

	      Tcl_DStringAppend(ds, "{ ", -1);

	      for(i = 0; i < h.nparm; i++)
		{

		  /*  j=i/8;
		      if ((h.vary[j] && (1>>((i-j*8)))))
		      strcat(s,"varying "); */
		  /*
		    j=h.parm[i].pflags;
		    if ((j==2) || (j==3)) strcat(s,"output ");
		    if ((j==1) || (j==3)) strcat(s,"varying ");
		  */

		  na = h.parm[i].pnarr;
		  /* XXXX temporarily skip all array parameters */
		  if(na <= 1)
		    {
		      Tcl_DStringAppend(ds, "{ ", -1);

		      Tcl_DStringAppend(ds, h.parm[i].pname, -1);

		      switch(h.parm[i].ptype)
			{
			case pString:
			  Tcl_DStringAppend(ds, " string ", -1);
			  break;
			case pFloat:
			  Tcl_DStringAppend(ds, " float ", -1);
			  break;
			case pColor:
			  Tcl_DStringAppend(ds, " color ", -1);
			  break;
			case pPoint:
			  Tcl_DStringAppend(ds, " point ", -1);
			  break;
			case pVector:
			  Tcl_DStringAppend(ds, " vector ", -1);
			  break;
			case pNormal:
			  Tcl_DStringAppend(ds, " normal ", -1);
			  break;
			case pMatrix:
			  Tcl_DStringAppend(ds, " matrix ", -1);
			  break;
			default:
			  Tcl_DStringAppend(ds, " unknown ", -1);
			  break;
			}

		      sprintf(buffer, "%d ", na);
		      Tcl_DStringAppend(ds, buffer, -1);

		      /* get default values */
		      t = (char *)(&h);
		      t += (sizeof(h) - (MAXNSHADERARGS-h.nparm) *
			    sizeof(Parameter) - 128000 + h.ncode * 4);
		      /*t += (h.dataix+h.parm[i].paddr*4-200);*/
		      t += (h.parm[i].paddr * 4 - 200);
		      fp = (float *)t;

		      if(na > 1)
			{
			  Tcl_DStringAppend(ds, "{ ", -1);
			}

		      for(j = 0; j < na; j++)
			{
			  switch (h.parm[i].ptype)
			    {
			    case pString:
			      Tcl_DStringAppend(ds, t, -1);
			      Tcl_DStringAppend(ds, " ", -1);
			      t += 160;
			      break;
			    case pFloat:
			      sprintf(buffer, "%f ", *fp); fp++;
			      Tcl_DStringAppend(ds, buffer, -1);
			      Tcl_DStringAppend(ds, " ", -1);
			      break;
			    case pColor:
			    case pPoint:
			    case pVector:
			    case pNormal:
			      Tcl_DStringAppend(ds, "{ ", -1);
			      sprintf(buffer, "%f ", *fp); fp++;
			      Tcl_DStringAppend(ds, buffer, -1);
			      sprintf(buffer, "%f ", *fp); fp++;
			      Tcl_DStringAppend(ds, buffer, -1);
			      sprintf(buffer, "%f ", *fp); fp++;
			      Tcl_DStringAppend(ds, buffer, -1);
			      Tcl_DStringAppend(ds, "} ", -1);
			      break;
			    case pMatrix:
			      Tcl_DStringAppend(ds, "{ ", -1);
			      for(k = 1; k < 16; k++)
				{
				  sprintf(buffer, "%f ", *fp); fp++;
				  Tcl_DStringAppend(ds, buffer, -1);
				} /* for */
			      Tcl_DStringAppend(ds, "} ", -1);
			      break;
			    default:
			      break;
			    } /* switch */

			} /* for */
		      if(na > 1)
			{
			  Tcl_DStringAppend(ds, "} ", -1);
			}
		      Tcl_DStringAppend(ds, "} ", -1);
		    } 
		  else
		    {
		      ay_error(AY_EWARN, h.parm[i].pname,
			       "skipping array parameter");
		      /* reset ay_errno so that scanAllShaders does not
			 stop parsing because of this */
		      ay_error(AY_OK, NULL, NULL);
		    } /* if */
		} /* for */
	      Tcl_DStringAppend(ds, "}", -1);
	    } /* if */
	} /* if */

      fclose(f);

      return AY_OK; /* early exit! */
    } /* if */

 return AY_ERROR;
} /* ayslb_scanshader */


/* ayslb_scanslbtcmd:
 *  scan a shader compiled with shaded (Air)
 */
int
ayslb_scanslbtcmd(ClientData clientData, Tcl_Interp *interp,
		  int argc, char *argv[])
{
 int ay_status = AY_OK; 
 char fname[] = "shaderScanSLB";
 Tcl_DString ds;
 char buf[255];

  if(argc < 3)
    {
      ay_error(AY_EARGS, fname, "shaderpath varname");
      return TCL_OK;
    }

  if(strlen(argv[1]) < 240)
    {
      sprintf(buf, "shader_findShader %s", argv[1]);

      if(Tcl_Eval(interp, buf) != TCL_OK)
	{
	  ay_error(AY_ERROR, fname, "findShader failed");
	  return TCL_OK;
	}
    }
  else
    {
      ay_error(AY_ERROR, fname, "shader name too long");
      return TCL_OK;
    }

  if(strlen(interp->result) < 2)
    {
      ay_error(AY_ERROR, fname, "could not find shader:");
      ay_error(AY_ERROR, fname, argv[1]);
      return TCL_OK;
    }

  Tcl_DStringInit(&ds);

  Tcl_DStringAppend(&ds, argv[1], -1);

  ay_status = ayslb_scanshader(interp->result, &ds);

  if(ay_status == AY_OK)
    {
      Tcl_SetVar(interp, argv[2], Tcl_DStringValue(&ds), TCL_LEAVE_ERR_MSG);
    }

  Tcl_DStringFree(&ds);

 return TCL_OK;
} /* ayslb_scanslbtcmd */


/* note: this function _must_ be capitalized exactly this way
 * regardless of filename (see: man n load)!
 */
int
Ayslb_Init(Tcl_Interp *interp)
{
 char fname[] = "ayslb_init";

  Tcl_SetVar(interp, "ay(sext)", ".slb", TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_CreateCommand(interp, "shaderScan", ayslb_scanslbtcmd,
		    (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  ay_error(AY_EOUTPUT, fname,
	   "Plug-In \\\"ayslb\\\" successfully loaded.");
  ay_error(AY_EOUTPUT, fname,
	   "Ayam will now scan for .slb-shaders only!");

 return TCL_OK;
} /* Ayslb_Init */
