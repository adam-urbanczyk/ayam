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

/* error.c - functions for writing out error messages */

/* ay_error_wlog:
 *
 */
void
ay_error_wlog(char *message)
{
 FILE *log = NULL;
 static int warned = AY_FALSE;
 char *m = NULL;

 if(!message)
   return;

  if(ay_prefs.writelog)
    {
      if(ay_prefs.logfile)
	{
	  if(!(log = fopen(ay_prefs.logfile, "ab")))
	    {
	      if(!warned)
		{
		  fprintf(stderr,
			  "Unable to open logfile: \"%s\" for writing!\n",
			  ay_prefs.logfile);
		  warned = AY_TRUE;
		} /* if */
	      return;
	    } /* if */
	} /* if */

      m = strchr(message, '"');
      if(m)
	{
	  fprintf(log, "%s\n", m);
	}
      else
	{
	  fprintf(log, "%s\n", message);
	} /* if */

      fclose(log);
    } /* if */

  return;
} /* ay_error_wlog */


/* ay_error:
 *  write error message to console according to:
 *  code (see error codes in ayam.h)
 *  where (function or command name)
 *  what (more exact specification of error, or additional info)
 */
void
ay_error(int code, char *where, char *what)
{
 Tcl_Interp *interp = ay_interp;
 Tcl_DString ds, dsl;
 Tcl_Obj *to = NULL, *ton = NULL;
 char gentxterr[] = "Error! ", gentxtwarn[] = "Warning! ";
 char warncmd[] = "puts stdout \"", errcmd[] = "puts stderr \"";
 char lmsg[] = "Last message repeated ", lmsg2[] = " times.";
 char *countstr = NULL;
 static char *last_message = NULL;
 static int count = 0;
 int len;

  ay_errno = code;

  if(code == AY_OK)
    {
      ton = Tcl_NewStringObj("ay_error", -1);
      to = Tcl_NewIntObj(code);
      Tcl_ObjSetVar2(interp, ton, NULL, to, TCL_LEAVE_ERR_MSG |
		     TCL_GLOBAL_ONLY);
      Tcl_IncrRefCount(ton);Tcl_DecrRefCount(ton);
      return;
    }

  Tcl_DStringInit(&ds);

  if((code == AY_EWARN) || (code == AY_EOUTPUT))
    {
      Tcl_DStringAppend(&ds, warncmd, -1);
    }
  else
    {
      Tcl_DStringAppend(&ds, errcmd, -1);
    }


  if(code == AY_EFLUSH)
    {
      if(count >= 1)
	{
	  Tcl_DStringInit(&dsl);
	  Tcl_DStringAppend(&dsl, warncmd, -1);
	  Tcl_DStringAppend(&dsl, lmsg, -1);
	  if(!(countstr = calloc(32, sizeof(char))))
	    return; /* this should never happen */
	  len = sprintf(countstr,"%d", count);
	  Tcl_DStringAppend(&dsl, countstr, len);
	  Tcl_DStringAppend(&dsl, lmsg2, -1);
	  Tcl_DStringAppend(&dsl, "\"", -1);
	  Tcl_Eval(interp, Tcl_DStringValue(&dsl));
	  ay_error_wlog(Tcl_DStringValue(&dsl));
	  Tcl_DStringFree(&dsl);
	  free(countstr);
	}
      count = 0;
      if(last_message)
	{
	  free(last_message);
	  last_message = NULL;
	}

      Tcl_DStringFree(&ds);
      return;
    } /* if */


  if(where)
    {
      Tcl_DStringAppend(&ds, where, -1);
      Tcl_DStringAppend(&ds, ": ", -1);
    }

  switch(code)
    {
    case AY_EOMEM:
      Tcl_DStringAppend(&ds, "Out of memory!", -1);
      break;
    case AY_EOUTPUT:
      break;
    case AY_EOPENFILE:
      Tcl_DStringAppend(&ds, "Unable to open file: ", -1);
      break;
    case AY_ECLOSEFILE:
      Tcl_DStringAppend(&ds, "Unable to close file: ", -1);
      break;
    case AY_EFORMAT:
      Tcl_DStringAppend(&ds, "Wrong file format!", -1);
      break;
    case AY_ENOSEL:
      Tcl_DStringAppend(&ds, "No object(s) selected!", -1);
      break;
    case AY_EARGS:
      Tcl_DStringAppend(&ds, "Insufficient arguments, need: ", -1);
      break;
    case AY_EWTYPE:
      Tcl_DStringAppend(&ds, "Object is of wrong type, need: ", -1);
      break;
    case AY_ETYPE:
      Tcl_DStringAppend(&ds, "Type exists already!", -1);
      break;
    case AY_ENTYPE:
      Tcl_DStringAppend(&ds, "No such object type: ", -1);
      break;
    case AY_EREF:
      Tcl_DStringAppend(&ds, "Reference counter is not zero!", -1);
      break;
    case AY_ENULL:
      Tcl_DStringAppend(&ds,
		      "Internal error: Illegal NULL pointer encountered!", -1);
      break;
    case AY_EUEOF:
      Tcl_DStringAppend(&ds, "Unexpected EOF encountered: ", -1);
      break;
    case AY_ERROR:
      Tcl_DStringAppend(&ds, gentxterr, -1);
      break;
    case AY_EWARN:
      Tcl_DStringAppend(&ds, gentxtwarn, -1);
      break;
    default:
      Tcl_DStringAppend(&ds, gentxterr, -1);
      break;
    } /* switch */

  if(what)
    Tcl_DStringAppend(&ds, what, -1);

  Tcl_DStringAppend(&ds, "\"", -1);

  if(last_message && !strcmp(last_message, Tcl_DStringValue(&ds)))
    {
      /* last messages were identical, just count and exit */
      count++;
      Tcl_DStringFree(&ds);
      Tcl_SetVar(interp, "ay_error", "1", TCL_GLOBAL_ONLY | TCL_LEAVE_ERR_MSG);
      return;
    }
  else
    {
      /* last messages were not identical */
      if(count >= 1)
	{
	  Tcl_DStringInit(&dsl);
	  Tcl_DStringAppend(&dsl, warncmd, -1);
	  Tcl_DStringAppend(&dsl, lmsg, -1);
	  if(!(countstr = calloc(32, sizeof(char))))
	    return; /* this should never happen */
	  len = sprintf(countstr,"%d", count);
	  Tcl_DStringAppend(&dsl, countstr, len);
	  Tcl_DStringAppend(&dsl, lmsg2, -1);
	  Tcl_DStringAppend(&dsl, "\"", -1);
	  Tcl_Eval(interp, Tcl_DStringValue(&dsl));
	  ay_error_wlog(Tcl_DStringValue(&dsl));
	  Tcl_DStringFree(&dsl);
	  free(countstr);
	} /* if */
	count = 0;
    } /* if */

  Tcl_Eval(interp, Tcl_DStringValue(&ds));
  ay_error_wlog(Tcl_DStringValue(&ds));

  if(last_message)
    {
      free(last_message);
      last_message = NULL;
    }
  if(!(last_message = calloc(strlen(Tcl_DStringValue(&ds))+1, sizeof(char))))
    {
      fprintf(stderr,"Ayam: Cannot handle error message; out of memory!\n");
      return;
    }
  strcpy(last_message, Tcl_DStringValue(&ds));

  Tcl_DStringFree(&ds);

  if(code != AY_EOUTPUT)
    {
      ton = Tcl_NewStringObj("ay_error", -1);
      to = Tcl_NewIntObj(code);
      Tcl_ObjSetVar2(interp, ton, NULL, to, TCL_LEAVE_ERR_MSG |
		     TCL_GLOBAL_ONLY);
      Tcl_IncrRefCount(ton);Tcl_DecrRefCount(ton);
    }

 return;
} /* ay_error */


/* ay_error_tcmd:
 *
 */
int
ay_error_tcmd(ClientData clientData, Tcl_Interp *interp,
	      int argc, char *argv[])
{
 char fname[] = "ayError";
 char *place = NULL, *detail = NULL;
 int ecode = AY_OK;

  if(argc < 2)
    {
      ay_error(AY_EARGS, fname, "ecode \\[fname detail\\]");
      return TCL_OK;
    }

  if(argc == 4)
    {
      place = argv[2];
      detail = argv[3];
    }

  Tcl_GetInt(interp, argv[1], &ecode);

  ay_error(ecode, place, detail);

 return TCL_OK;
} /* ay_error_tcmd */


/* ay_error_glucb:
 *   GLU error callback
 */
void
ay_error_glucb(GLenum err)
{
 char fname[] = "GLU";

  ay_error(AY_ERROR, fname, (char *)gluErrorString(err));

 return;
} /* ay_error_glucb */
