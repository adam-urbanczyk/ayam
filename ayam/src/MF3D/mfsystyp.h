#ifndef MF3DSYSTYPES_H
#define	MF3DSYSTYPES_H
/*==============================================================================
 *
 *	File:		MFSYSTYP.H
 *
 *	Function:	Basic system-dependent typedefs. These should be defined as
 *				appropriate to your platform.
 *
 *	Version:	Metafile:	Version 1.0 3DMF files
 *				Package:	Release #3 of this code
 *
 *	Author(s):	Rick Wong (RWW), Duet Development Corp.
 *				John Kelly (JRK), Duet Development Corp.
 *
 *	Copyright:	(c) 1995 by Apple Computer, Inc., all rights reserved.
 *
 *	Change History (most recent first):
 *		FBL_JRK	Lowercase include file names
 *		FB7_JRK	Pragma macros
 *		Fabio	Changed file name to 8 characters
 *		F26_RWW	File created.
 *==============================================================================
 */
#if defined(applec) || defined(__MWERKS__) || defined(THINK_C)
#pragma once
#endif

#include <string.h>		/* strlen */

typedef unsigned char MF3DUns8;	/* 1 byte unsigned integer                      */
typedef signed char MF3DInt8;	/* 1 byte signed integer                        */
typedef unsigned short MF3DUns16;	/* 2 byte unsigned integer                      */
typedef signed short MF3DInt16;	/* 2 byte signed integer                        */
typedef unsigned int MF3DUns32;	/* 4 byte unsigned integer                      */
typedef signed int MF3DInt32;	/* 4 byte signed integer                        */
typedef struct MF3DUns64
  {
    MF3DUns32 hi;
    MF3DUns32 lo;
  }
MF3DUns64;			/* 8 byte unsigned integer                      */
typedef struct MFINT64
  {
    MF3DInt32 hi;
    MF3DUns32 lo;
  }
MFINT64;			/* 8 byte signed integer                        */
typedef float MF3DFloat32;	/* 4 byte floating point number         */
typedef double MF3DFloat64;	/* 8 byte floating point number         */

typedef char *MF3DCStringPtr;	/* pointer to a c string                        */
typedef const char *MF3DConstCStringPtr;
										/* pointer to a constant c string       */
#define	CStringLen(x)	strlen(x)

/* EOF character is a character which will not appear in a text file            */
#define	kMF3DEOFChar				(-1)

#endif
