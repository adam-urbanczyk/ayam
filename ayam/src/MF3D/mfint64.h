#ifndef MF3DINT64_H
#define	MF3DINT64_H
/*==============================================================================
 *
 *	File:		MFINT64.H
 *
 *	Function:	If the compiler/platform does not handle 64-bit numbers,
 *				we will need some helper math routines.
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
 *		F2F_RWW	File created.
 *==============================================================================
 */
#if defined(applec) || defined(__MWERKS__) || defined(THINK_C)
#pragma once
#endif

#include "mfsystyp.h"
#include "mfassert.h"

#define	Int32ToInt64(lnum,int64)	{	if ((lnum) >= 0)					\
											(int64).hi = 0;					\
										else								\
											(int64).hi = -1;				\
										(int64).lo = (MF3DUns32)(lnum);		\
									}
#define	Int32ToUns64(lnum,uns64)	{	if ((lnum) >= 0)					\
											(uns64).hi = 0;					\
										else								\
											(uns64).hi = ULONG_MAX;			\
										(uns64).lo = (MF3DUns32)(lnum);		\
									}

#define CompareInt64(i1, i2)												\
						(((i1).hi - (i2).hi) != 0 ?							\
							(((MF3DInt32)(i1).hi) - ((MF3DInt32)(i2).hi)) :	\
							(((MF3DInt32)(i1).lo) - ((MF3DInt32)(i2).lo))	\
						)

#define	AssignInt64(idst, isrc)		(idst).hi = (isrc).hi,					\
									(idst).lo = (isrc).lo;

#define	SetInt64ToZero(idst)		(idst).hi = 0, (idst).lo = 0;

/* This macro should only be used if appropriate range checks are made          */
#define	Uns64ToUns32(int64,lnum)	MFASSERT((int64).hi == 0), (lnum) = (int64).lo

/* Calculate i1 - i2 (assumes that i1 >= i2), returning result in an Uns32      */
/* Also assumes that result will fit into an Uns32.                                                     */
#define	SubtractUns64(i1, i2)												\
					(	MFASSERT((i1).hi == (i2).hi ||					\
						((i1).hi == ((i2).hi + 1) && (i2).lo > (i1).lo)),	\
						( ((i1).hi - (i2).hi) != 0 ?						\
						((i1).lo + (MF3DUns32)(-(MF3DInt32)((i2).lo))) :	\
						((i1).lo - (i2).lo) )								\
					)

#endif
