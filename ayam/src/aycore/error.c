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

/* error.c - functions for writing out error messages */

char *ay_error_igntype = "Ignoring object of unsupported type.";

static char aye_gentxterr[] = "Error! ", aye_gentxtwarn[] = "Warning! ";
static char aye_errcmd[] = "puts stderr {", aye_warncmd[] = "puts stdout {";
static char aye_lmsg1[] = "Last message repeated ", aye_lmsg2[] = " times.";


/* prototypes of functions local to this module: */
void ay_error_wlog(const char *message);

/* functions: */

/* ay_error_wlog:
 *
 */
void
ay_error_wlog(const char *message)
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
			"Ayam: Unable to open logfile: \"%s\" for writing!\n",
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
ay_error(int code, const char *where, const char *what)
{
 Tcl_Interp *interp = ay_interp;
 Tcl_DString ds, dsl;
 Tcl_Obj *to = NULL, *ton = NULL;
 char countstr[TCL_INTEGER_SPACE];
 static char *last_message = NULL;
 static int count = 0;
 int len;

  ay_errno = code;

  if(code == AY_OK)
    {
      goto set_ay_error;
    }

  Tcl_DStringInit(&ds);

  if((code == AY_EWARN) || (code == AY_EOUTPUT))
    {
      Tcl_DStringAppend(&ds, aye_warncmd, -1);
    }
  else
    {
      Tcl_DStringAppend(&ds, aye_errcmd, -1);
    }

  if(code == AY_EFLUSH)
    {
      if(count >= 1)
	{
	  Tcl_DStringInit(&dsl);
	  Tcl_DStringAppend(&dsl, aye_warncmd, -1);
	  Tcl_DStringAppend(&dsl, aye_lmsg1, -1);
	  len = sprintf(countstr, "%d", count);
	  Tcl_DStringAppend(&dsl, countstr, len);
	  Tcl_DStringAppend(&dsl, aye_lmsg2, -1);
	  Tcl_DStringAppend(&dsl, "}", -1);
	  Tcl_Eval(interp, Tcl_DStringValue(&dsl));

	  if(ay_prefs.writelog)
	    {
	      ay_error_wlog(Tcl_DStringValue(&dsl));
	    }

	  Tcl_DStringFree(&dsl);
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
    case AY_EWARN:
      Tcl_DStringAppend(&ds, aye_gentxtwarn, -1);
      break;
    case AY_ERROR:
      Tcl_DStringAppend(&ds, aye_gentxterr, -1);
      break;
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
    case AY_ERANGE:
      Tcl_DStringAppend(&ds, "Parameter out of range: ", -1);
      break;
    case AY_ENOSEL:
      Tcl_DStringAppend(&ds, "No object(s) selected!", -1);
      break;
    case AY_EARGS:
      Tcl_DStringAppend(&ds, "Insufficient arguments, need: ", -1);
      break;
    case AY_EOPT:
      Tcl_DStringAppend(&ds, "Missing or malformed value for option: ", -1);
      break;
    case AY_EUOPT:
      Tcl_DStringAppend(&ds, "Encountered unknown option: ", -1);
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
    default:
      Tcl_DStringAppend(&ds, aye_gentxterr, -1);
      break;
    } /* switch */

  if(what)
    {
      Tcl_DStringAppend(&ds, what, -1);
    }

  if((code == AY_EARGS) || (code == AY_EWTYPE))
    {
      Tcl_DStringAppend(&ds, ".", -1);
    }

  Tcl_DStringAppend(&ds, "}", -1);

  if(last_message && !strcmp(last_message, Tcl_DStringValue(&ds)))
    {
      /* last messages were identical, just count and exit */
      count++;
      Tcl_DStringFree(&ds);
      goto set_ay_error;
    }
  else
    {
      /* last messages were not identical,
	 save the current message,
	 output the saved message */
      if(count >= 1)
	{
	  Tcl_DStringInit(&dsl);
	  Tcl_DStringAppend(&dsl, aye_warncmd, -1);
	  Tcl_DStringAppend(&dsl, aye_lmsg1, -1);
	  len = sprintf(countstr, "%d", count);
	  Tcl_DStringAppend(&dsl, countstr, len);
	  Tcl_DStringAppend(&dsl, aye_lmsg2, -1);
	  Tcl_DStringAppend(&dsl, "}", -1);
	  if(((code == AY_EOUTPUT) && (ay_prefs.errorlevel > 2)) ||
	     ((code == AY_EWARN) && (ay_prefs.errorlevel > 1)) ||
	     ay_prefs.errorlevel > 0)
	    {
	      Tcl_Eval(interp, Tcl_DStringValue(&dsl));
	    }
	  if(ay_prefs.writelog)
	    {
	      ay_error_wlog(Tcl_DStringValue(&dsl));
	    }
	  Tcl_DStringFree(&dsl);
	} /* if */
	count = 0;
    } /* if */

  /* output the current message */
  if(((code == AY_EOUTPUT) && (ay_prefs.errorlevel > 2)) ||
     ((code == AY_EWARN) && (ay_prefs.errorlevel > 1)) ||
     ay_prefs.errorlevel > 0)
    {
      Tcl_Eval(interp, Tcl_DStringValue(&ds));
    }
  if(ay_prefs.writelog)
    {
      ay_error_wlog(Tcl_DStringValue(&ds));
    }

  if(last_message)
    {
      free(last_message);
    }

  if(Tcl_DStringValue(&ds))
    {
      if(!(last_message = malloc((Tcl_DStringLength(&ds)+1) * sizeof(char))))
	{
	  fprintf(stderr,"Ayam: Cannot handle error message; out of memory!\n");
	  return;
	}
      strcpy(last_message, Tcl_DStringValue(&ds));
    }
  Tcl_DStringFree(&ds);

set_ay_error:
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
 *  Report an error to the user.
 *  Implements the \a ayError scripting interface command.
 *  See also the corresponding section in the \ayd{scayerror}.
 *  \returns TCL_OK in any case.
 */
int
ay_error_tcmd(ClientData clientData, Tcl_Interp *interp,
	      int argc, char *argv[])
{
 char *place = NULL, *detail = NULL;
 int tcl_status = TCL_OK, ecode = AY_OK;

  if(argc < 2)
    {
      ay_error(AY_EARGS, argv[0], "ecode [fname detail]");
      return TCL_OK;
    }

  if(argc == 4)
    {
      place = argv[2];
      detail = argv[3];
    }

  tcl_status = Tcl_GetInt(interp, argv[1], &ecode);
  AY_CHTCLERRRET(tcl_status, argv[0], interp);

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


/* ay_error_reportdrange:
 *  format and output double range error
 */
int
ay_error_reportdrange(char *fname, char *pname, double lb, double ub)
{
 char *msg = NULL;

  if(pname)
    {
      if((msg = calloc(strlen(pname)+TCL_DOUBLE_SPACE*2+64, sizeof(char))))
	sprintf(msg, "Parameter %s out of range, should be [%lg, %lg].",
		pname, lb, ub);
    }
  else
    {
      if((msg = calloc(TCL_DOUBLE_SPACE*2+64, sizeof(char))))
	sprintf(msg, "Parameter out of range, should be [%lg, %lg].",
		lb, ub);
    }

  if(msg)
    ay_error(AY_ERROR, fname, msg);
  else
    ay_error(AY_ERANGE, fname, pname);

  if(msg)
    free(msg);

 return AY_ERANGE;
} /* ay_error_reportdrange */


/* ay_error_reportirange:
 *  format and output integer range error
 */
int
ay_error_reportirange(char *fname, char *pname, int lb, int ub)
{
 char *msg = NULL;

  if(pname)
    {
      if((msg = calloc(strlen(pname)+TCL_INTEGER_SPACE*2+64, sizeof(char))))
	sprintf(msg, "Parameter %s out of range, should be [%d, %d].",
		pname, lb, ub);
    }
  else
    {
      if((msg = calloc(TCL_INTEGER_SPACE*2+64, sizeof(char))))
	sprintf(msg, "Parameter out of range, should be [%d, %d].",
		lb, ub);
    }

  if(msg)
    ay_error(AY_ERROR, fname, msg);
  else
    ay_error(AY_ERANGE, fname, pname);

  if(msg)
    free(msg);

 return AY_ERANGE;
} /* ay_error_reportirange */


/* ay_error_getglerrortcmd:
 *  get current GL error state and string
 *  and call ay_error with it for output
 *  Implements the \a getGLErr scripting interface command.
 *  See also the corresponding section in the \ayd{scgetglerr}.
 *  \returns TCL_OK in any case.
 */
int
ay_error_getglerrortcmd(ClientData clientData, Tcl_Interp *interp,
		       int argc, char *argv[])
{

  ay_error(AY_EOUTPUT, argv[0], (char *)gluErrorString(glGetError()));

 return TCL_OK;
} /* ay_error_getglerrortcmd */
