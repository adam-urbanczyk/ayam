/*==============================================================================
 *
 *	File:		MFGROUPS.C
 *
 *	Function:	Handle group state and attribute states.
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
 *		FB8_JRK	Segmentation
 *		Fabio	Changed file name to 8 characters
 *		F2R_RWW Use the simple object model, eliminating the need for most
 *				of the meat of all these routines.
 *		F2N_RWW	File created.
 *==============================================================================
 */

#include "mfgroups.h"

#include "mferrors.h"
#include "mfsubobj.h"
#include "mfassert.h"
#include "mfintobj.h"
#include "mfmacros.h"
#include "mfmemory.h"

#if defined(applec) || defined(__MWERKS__) || defined(THINK_C)
#pragma segment __MF3D__
#endif

/*==============================================================================
 *	MF3D_InitGroup
 *
 *	Setup internal file record group stuff
 *==============================================================================
 */
MF3DErr
MF3D_InitGroup (
		 MF3D_FilePtr ioMetafile)
{
  MF3DErr result;

  MFASSERT (ioMetafile->readWrite == MF3D_MetafileRead);

  result = kMF3DNoErr;

  ioMetafile->groupState.traversalFlags = MF3DDisplayGroupDefaultFlags;
  ioMetafile->groupState.parent = NULL;

  return result;
}

/*==============================================================================
 *	MF3D_PushGroup
 *
 *	Make a copy of the current group and push it onto the group stack
 *==============================================================================
 */
MF3DErr
MF3D_PushGroup (
		 MF3D_FilePtr ioMetafile)
{
  MF3D_GroupStatePtr oldGroup;
  MF3DErr result;

  MFASSERT (ioMetafile->readWrite == MF3D_MetafileRead);

  result = kMF3DNoErr;

  MF3D_Allocate (oldGroup);

  if (result == kMF3DNoErr)
    {
      oldGroup->traversalFlags =
	ioMetafile->groupState.traversalFlags;
      oldGroup->parent = ioMetafile->groupState.parent;
      ioMetafile->groupState.parent = oldGroup;
    }

  return result;
}

/*==============================================================================
 *	MF3D_PopGroup
 *
 *	Pop the group stack
 *==============================================================================
 */
MF3DErr
MF3D_PopGroup (
		MF3D_FilePtr ioMetafile)
{
  MF3D_GroupStatePtr parentGroup;
  MF3DErr result;

  MFASSERT (ioMetafile->readWrite == MF3D_MetafileRead);

  result = kMF3DNoErr;

  parentGroup = ioMetafile->groupState.parent;
  if (ioMetafile->groupState.parent == NULL)
    result = kMF3DErrTooManyEndGroups;

  if (result == kMF3DNoErr)
    {
      ioMetafile->groupState.parent = parentGroup->parent;
      MF3D_Free (parentGroup);
    }

  return result;
}

/*==============================================================================
 *	MF3D_DisposeGroup
 *
 *	Clean up top-level group pointers
 *==============================================================================
 */
MF3DErr
MF3D_DisposeGroup (
		    MF3D_FilePtr ioMetafile)
{
  MF3DErr result;

  MFASSERT (ioMetafile->readWrite == MF3D_MetafileRead);

  result = kMF3DNoErr;

  if (ioMetafile->groupState.parent != NULL)
    {
      result = kMF3DErrNotEnoughEndGroups;
      MF3D_PopGroup (ioMetafile);	/* errors ignored */
      MF3D_DisposeGroup (ioMetafile);	/* because we already have one */
    }

  return result;
}
