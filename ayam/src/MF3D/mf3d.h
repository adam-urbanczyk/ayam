#ifndef MF3D_H
#define	MF3D_H
/*==============================================================================
 *
 *	File:		MF3D.H
 *
 *	Function:	QuickDraw 3D Metafile Read/Write API
 *
 *	Version:	Metafile:	Version 1.0 3DMF files
 *				Package:	Release #3 of this code
 *
 *	Author(s):	Rick Wong (RWW), Duet Development Corp.
 *				John Kelly (JRK), Duet Development Corp.
 *
 *	Copyright:	(c) 1995 by Apple Computer, Inc., all rights reserved.
 *
 *	Example:	A simple metafile viewer will look something like:
 *
 *				MF3DOpenInputStdCFile(filename, &metafile);
 *				while (MF3DReadAnObject(metafile, &object) != kMF3DErrNoMoreObjs)
 *				{	if (object->objectType == kMF3DObjReference)
 *					{	MF3DResolveReference(metafile, object, NULL);
 *					}
 *					else
 *					{	switch(object->objectType)
 *						{	...Handle each object type appropriately...
 *						}
 *					}
 *					DisposeObject(object);
 *				}
 *				MF3DClose(metafile);
 *
 *	Change History (most recent first):
 *		FBL_JRK	Lowercase include file names
 *		FB7_JRK	Pragma macros
 *		Fabio	Changed file name to 8 characters
 *		F2R_RWW	Change to simple object theory.
 *		F28_RWW	Finished definitions and added comments.
 *		F21_RWW	File created.
 *==============================================================================
 */
#if defined(applec) || defined(__MWERKS__) || defined(THINK_C)
#pragma once
#endif

#include "mfiocall.h"
#include "mfobjcts.h"
#include "mftypes.h"
#include "mfintobj.h"

#define	kMF3DVersionMajor	1
#define	kMF3DVersionMinor	0
#define	kAllowVariableUns	FALSE

/*==============================================================================
 *	API Routines
 *==============================================================================
 */

/*==============================================================================
 *	MF3DOpenInput
 *
 *	Open a metafile for parsing using caller-defined file-handling routines.
 *==============================================================================
 */
MF3DErr
MF3DOpenInput (
		MF3DUserOpenDataPtr inUserDataPtr,	/* In:  user-defined file info  */
		MF3DProcsPtr inUserIOProcsPtr,	/* In:  user-defined I/O procs  */
		MF3D_FilePtr * outMF3DFilePtr);		/* Out: MF3D file structure             */

/*==============================================================================
 *	MF3DOpenInputStdCFile
 *
 *	Open a metafile for parsing using standard C routines.
 *
 *	This routine just calls MF3DOpenInput with inUserDataPtr = inFilename
 *	and inUserIOProcsPtr set to call fopen, fread, NULL, ftell, fseek,
 *	and fclose.
 *==============================================================================
 */
MF3DErr
MF3DOpenInputStdCFile (
			const char *inFilename,		/* In:  C-string filename                       */
			MF3D_FilePtr * outMF3DFilePtr);		/* Out: MF3D file structure                     */

/*==============================================================================
 *	MF3DReadAnObject
 *
 *	Retrieve the next metafile object in an open metafile.
 *
 *	Metafile objects are defined in <MFOBJCTS.H>. They can be recognized
 *		by checking the fObjectType field and casting the object pointer
 *		appropriately.
 *
 *	Objects returned by MF3DReadAnObject should be disposed using
 *		MF3DDisposeObject when they are no longer needed. If an error occurs,
 *		NULL will be returned (MF3DDisposeObject ignores NULL parameters).
 *
 *	Returns:
 *		kMF3DNoErr				object was successfully read
 *		kMF3DNoMoreObjects		the end of the file has been reached
 *==============================================================================
 */
MF3DErr
MF3DReadAnObject (
		   MF3D_FilePtr inMF3DFilePtr,	/* In:  MF3D file structure                     */
		   MF3DVoidObjPtr * outMF3DObjPtr);	/* Out: metafile object                         */

/*==============================================================================
 *	MF3DDisposeObject
 *
 *	Dispose a metafile object.
 *	If inMF3DObjPtr is NULL, nothing happens.
 *==============================================================================
 */
MF3DErr
MF3DDisposeObject (
		    MF3DVoidObjPtr inMF3DObjPtr);	/* In:  metafile object                         */

/*==============================================================================
 *	MF3DResolveReference
 *
 *	Set up an MF3D file so that it will read the object pointed to by
 *	a reference object. The next call to MF3DReadAnObject will then read
 *	the referenced object, and the MF3D file will then be reset to read the
 *	object that would have been read if MF3DResolveReference had not been
 *	called.
 *
 *	If MF3DResolveReference resolves to a group object, the entire group will
 *	be read before the MF3D file is reset.
 *
 *	If inStoragePtr is not NULL, the reference is in an external file.
 *==============================================================================
 */
MF3DErr
MF3DResolveReference (
		       MF3D_FilePtr inMF3DFilePtr,	/* In:  MF3D file structure             */
		       MF3DReferenceObjPtr inMF3DRefObjPtr,	/* In:  reference object                */
		       MF3DStorageObjPtr inExternalFilePtr);	/* In:  external storage object */

/*==============================================================================
 *	MF3DClose
 *
 *	Close the metafile.
 *
 *	If inMF3DFilePtr is NULL, nothing happens.
 *==============================================================================
 */
MF3DErr
MF3DClose (
	    MF3D_FilePtr inMF3DFilePtr);	/* In:  MF3D file structure             */

/*==============================================================================
 *	MF3DOpenOutput
 *
 *	Open a metafile for writing using caller-defined file-handling routines.
 *==============================================================================
 */
MF3DErr
MF3DOpenOutput (
		 MF3DDataFormatEnum inMF3DDataFormat,	/* In:  binary or text                  */
		 MF3DUserOpenDataPtr inUserDataPtr,	/* In:  user-defined file info  */
		 MF3DProcsPtr inUserIOProcsPtr,		/* In:  user-defined I/O procs  */
		 MF3D_FilePtr * outMF3DFilePtr);	/* Out: MF3D file structure             */

/*==============================================================================
 *	MF3DOpenOutputStdCFile
 *
 *	Open a metafile for writing using standard C routines.
 *
 *	This routine just calls MF3DOpenOutput with inUserDataPtr = inFilename
 *	and inUserIOProcsPtr set to call fopen, fread, fwrite, ftell, fseek,
 *	and fclose.
 *==============================================================================
 */
MF3DErr MF3DOpenOutputStdCFile (
				 MF3DDataFormatEnum inMF3DDataFormat,	/* In:  binary or text                  */
				 const char *inFilename,	/* In:  C-string filename               */
				 MF3D_FilePtr * outMF3DFilePtr);	/* Out: MF3D file structure             */

/*==============================================================================
 *	MF3DWriteAnObject
 *
 *	Write a metafile object to an open metafile.
 *==============================================================================
 */
MF3DErr
MF3DWriteAnObject (
		    MF3D_FilePtr inMF3DFilePtr,		/* In:  MF3D file structure             */
		    MF3DVoidObjPtr inMF3DObjPtr);	/* In:  metafile object                 */

/*==============================================================================
 *	MF3DTellPosition
 *
 *	Retrieve the position of an open metafile.
 *==============================================================================
 */
MF3DErr
MF3DTellPosition (
		   MF3D_FilePtr inMF3DFilePtr,	/* In:  MF3D file structure     */
		   MF3DBinaryFilePosition * outMF3DPosPtr);	/* Out: file offset                     */

/*==============================================================================
 *	MF3DSeekPosition
 *
 *	Set the position of an open metafile.
 *==============================================================================
 */
MF3DErr
MF3DSeekPosition (
		   MF3D_FilePtr inMF3DFilePtr,	/* In:  MF3D file structure     */
		   MF3DBinaryFilePosition inMF3DPosition);	/* In:  file offset                     */

#endif
