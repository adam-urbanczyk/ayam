/*==============================================================================
 *
 *	File:		MFOBJTYP.C
 *
 *	Function:	Object name/type lookup
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
 *		FB4_JRK	Parse object types we don't understand
 *		Fabio	Changed file name to 8 characters
 *		F2H_RWW	File created.
 *==============================================================================
 */
#define	MF3D_Extern
#include "mfobjtyp.h"
#undef	MF3D_Extern

#include <stdlib.h>

#include "mferrors.h"
#include "mfobjcts.h"
#include "mftypes.h"
#include "mfassert.h"
#include "mfintobj.h"
#include "mftextrd.h"
#include "mftextut.h"

#if defined(applec) || defined(__MWERKS__) || defined(THINK_C)
#pragma segment __MF3D__
#endif


/* FB5_JRK
 * We use the following string to help us parse unknown objects in a text file.
 * It is set by MF3D_FindObjectFromName and used in MF3D_ConvertUserDefinedObjType
 * to return an object name to stuff into the object. It is a HACK (!) that
 * relies on no other failed calls to MF3D_FindObjectFromName before calling
 * MF3D_ConvertUserDefinedObjType. It works, but thought you might want to know.
 */
static char gLastUnknownObjectName[kMF3D_MaxObjNameLength] = "";


/*==============================================================================
 *	MF3D_FindObjectFromName
 *
 *	Get the object stuff, given the object name
 *==============================================================================
 */
MF3DErr
MF3D_FindObjectFromName (
			  const char *inObjName,
			  MF3D_ObjStuffPtr * outObjStuffPtr,
			  MF3DObjType * outObjectType)
{
  MF3DErr result;

  result = kMF3DNoErr;

  if (inObjName[0] == '-')
    {
      MF3DInt32 objType;

      objType = atoi (inObjName);

      if (objType >= 0)
	result = kMF3DErrIllegalObjName;
      else
	{
	  *outObjStuffPtr = &gMF3D_UnknownObjStuffTable;
	  MFASSERT (sizeof (*outObjectType) == sizeof (objType));
	  *outObjectType = objType;
	}
    }
  else
    {
      MF3D_ObjStuffPtr objStuffPtr;

      objStuffPtr = gMF3D_ObjStuffTable;
      do
	{
	  if (MF3D_CompareObjNames (inObjName, objStuffPtr->name) == 0)
	    {
	      *outObjStuffPtr = objStuffPtr;
	      break;
	    }
	  ++objStuffPtr;
	}
      while (objStuffPtr->type != kMF3DObjInvalid);

      /* FB5_JRK */
      /* Remember object name on error and pass back special object stuff */
      if (objStuffPtr->type == kMF3DObjInvalid)
	{
	  *outObjStuffPtr = &gMF3D_UnknownObjStuffTable;
	  *outObjectType = kMF3DObjUnknownType;
	  strncpy (gLastUnknownObjectName, inObjName, kMF3D_MaxObjNameLength);
	  gLastUnknownObjectName[kMF3D_MaxObjNameLength - 1] = '\0';
	  result = kMF3DErrIllegalObjName;
	}
      else
	*outObjectType = objStuffPtr->type;

    }

  return result;
}

/*==============================================================================
 *	MF3D_FindObjectFromType
 *
 *	Get the object stuff, given the object type
 *==============================================================================
 */
MF3DErr
MF3D_FindObjectFromType (
			  MF3DObjType inObjType,
			  MF3D_ObjStuffPtr * outObjStuffPtr)
{
  MF3DErr result;

  result = kMF3DNoErr;

  /* FB4_JRK */
  /* Return unknown object stuff if we don't know the type of object */
  *outObjStuffPtr = &gMF3D_UnknownObjStuffTable;

  /* Look through standard table if not a user-defined type */
  if (inObjType >= 0)
    {
      MF3D_ObjStuffPtr objStuffPtr;

      objStuffPtr = gMF3D_ObjStuffTable;
      do
	{
	  if (inObjType == objStuffPtr->type)
	    {
	      *outObjStuffPtr = objStuffPtr;
	      break;		/* ### NORMAL LOOP EXIT ### */
	    }
	  ++objStuffPtr;
	}
      while (objStuffPtr->type != kMF3DObjInvalid);
      if (objStuffPtr->type == kMF3DObjInvalid)
	result = kMF3DErrIllegalObjName;
    }

  return result;
}

/*==============================================================================
 *	ConvertUserDefinedObjType
 *
 *	Negative numbers are user-defined object types.
 *	Verify that this one exists.
 *==============================================================================
 */
MF3DErr
MF3D_ConvertUserDefinedObjType (
				 MF3D_FilePtr inMetafilePtr,
				 MF3DObjType inObjType,
				 MF3DCStringPtr * outObjName)
{
  MF3DUns32 numTypes;
  MF3D_TypeListPtr typeListPtr;
  MF3DErr result;

  /* Assume we will not find what we are looking for */
  result = kMF3DErrIllegalObjName;

  /* FB5_JRK */
  /* Return last unknown name if we get unknown type */
  if (inObjType == kMF3DObjUnknownType)
    *outObjName = MF3D_DuplicateCString (gLastUnknownObjectName);
  else
    {
      typeListPtr = inMetafilePtr->typeTable.types;
      for (numTypes = inMetafilePtr->typeTable.nTypes;
	   result != kMF3DNoErr && numTypes > 0;
	   --numTypes, ++typeListPtr)
	{
	  MFASSERT (sizeof (inObjType) == sizeof (typeListPtr->number));
	  if (inObjType == (MF3DObjType) typeListPtr->number)
	    {
	      *outObjName = MF3D_DuplicateCString (typeListPtr->name);
	      result = kMF3DNoErr;	/* ### REAL LOOP EXIT ### */
	    }
	}
    }

  return result;
}
