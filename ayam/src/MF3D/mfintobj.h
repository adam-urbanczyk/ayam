#ifndef MF3D_INTOBJECTS_H
#define	MF3D_INTOBJECTS_H
/*==============================================================================
 *
 *	File:		MFINTOBJ.H
 *
 *	Function:	Structure definitions for Metafile objects not returned
 *				by MF3DReadAnObject().
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
 *		F3A_RWW	TOC works.
 *		F2R_RWW	Simple object model: most objects moved to "MFSUBOBJ.H".
 *		F26_RWW	File created.
 *==============================================================================
 */
#if defined(applec) || defined(__MWERKS__) || defined(THINK_C)
#pragma once
#endif

#include "mfiocall.h"
#include "mfobjcts.h"
#include "mfsubobj.h"
#include "mftypes.h"

/*==============================================================================
 *	TableOfContents
 *==============================================================================
 */
#define	kMF3DObjTableOfContents					MF3DObjectType('t','o','c',' ')
#define	kMF3DObjTableOfContentsText				"TableOfContents"

#define	kMF3DTocEntryType		1	/* always write type 1                    */
#define	kMF3DTocEntrySize		16

typedef struct MF3D_TOCEntryRec
  {
    MF3DReferenceID refID;
    MF3DPositionReferencePtr objLocation;
    MF3DObjType objType;	/* valid if tocEntryType >= 1   */
  }
MF3D_TOCEntryRec, *MF3D_TOCEntryPtr;

typedef struct MF3DTableOfContentsObj
  {
    MF3DObjHeader
    MF3DPositionReferencePtr nextTOC;	/* next Table of Contents               */
    MF3DReferenceID refSeed;
    MF3DInt32 typeSeed;
    MF3DUns32 tocEntryType;
    MF3DUns32 tocEntrySize;
    MF3DUns32 nEntries;
    MF3D_TOCEntryPtr tocEntries;	/* size: nEntries                               */
  }
MF3DTableOfContentsObj, *MF3DTableOfContentsObjPtr;

/*==============================================================================
 *	MF3D_TOCStuff
 *
 *	Reference ID lookup tables for table of contents
 *==============================================================================
 */
typedef union MF3D_TOCPosition
  {
    MF3DCStringPtr name;
    MF3DBinaryFilePosition location;
  }
MF3D_TOCPosition, *MF3D_TOCPositionPtr;

typedef struct MF3D_TOCReference
  {
    MF3DUns32 refID;
    MF3D_TOCPosition ref;
    MF3DObjType type;
  }
MF3D_TOCReference, *MF3D_TOCReferencePtr;

typedef struct MF3D_TOCStuff
  {
    MF3DCStringPtr tocLabelName;	/* txt: TOC label                               */
    MF3DUns32 refSeed;		/* next available ref num               */
    MF3DInt32 typeSeed;		/* next available type num              */
    MF3DUns32 numReferences;	/* number of refs in TOC                */
    MF3D_TOCReferencePtr references;	/* pointer to refs array                */
    /* the object type needs to be updated if we get a label on a container */
    MF3DBoolean needToUpdate;
  }
MF3D_TOCStuff, *MF3D_TOCStuffPtr;

/*==============================================================================
 *	Type (User-defined type)
 *==============================================================================
 */
#define	kMF3DObjType							MF3DObjectType('t','y','p','e')
#define	kMF3DObjTypeText						"Type"

typedef struct MF3D_TypeList
  {
    MF3DInt32 number;
    MF3DCStringPtr name;
  }
MF3D_TypeList, *MF3D_TypeListPtr;

typedef struct MF3D_TypeTable
  {
    MF3DUns32 nTypes;
    MF3D_TypeListPtr types;
  }
MF3D_TypeTable, *MF3D_TypeTablePtr;

/*==============================================================================
 *	Object Table
 *
 *	During the first-pass read of a text file, we store the locations of all
 *	the objects in the file. We resolve objRefID when we get to the
 *	Table of Contents.
 *==============================================================================
 */
typedef struct MF3D_ObjectTableEntry
  {
    MF3DBinaryFilePosition objLocation;
    MF3DReferenceID objRefID;
  }
MF3D_ObjectTableEntry, *MF3D_ObjectTableEntryPtr;

typedef struct MF3D_ObjectTable
  {
    MF3DUns32 numObjects;
    MF3D_ObjectTableEntryPtr objects;	/* size: numObjects + 1                 */
  }
MF3D_ObjectTable;

/*==============================================================================
 *	MF3D_ReadBuffer
 *
 *	We read one object in at a time. This prevents us from scanning past the
 *	end of an object.
 *==============================================================================
 */
typedef struct MF3D_SaveBuffer MF3D_SaveBuffer, *MF3D_SaveBufferPtr;
struct MF3D_SaveBuffer
  {
    MF3DUns32 bufSize;
    MF3D_SaveBufferPtr next;
  };

typedef struct MF3D_ReadBuffer
  {
    char *buf;			/* the actual buffer                    */
    MF3DUns32 bufPos;		/* current position in buffer   */
    MF3DUns32 bufSize;		/* bin: size of buffer                  */
    MF3D_SaveBufferPtr saveSize;	/* bin: stack of buffer sizes   */
  }
MF3D_ReadBuffer;

/*==============================================================================
 *	MF3D_WriteStack
 *
 *	When writing containers in a binary file format, we write a phony size until
 *	we reach the end of the container. Then, we backpatch the real size.
 *	So, we need to store the container size locations as we get them.
 *==============================================================================
 */
typedef struct MF3D_WriteStack MF3D_WriteStack, *MF3D_WriteStackPtr;
struct MF3D_WriteStack
  {
    MF3DBinaryFilePosition location;
    MF3D_WriteStackPtr next;
  };

/*==============================================================================
 *	MF3D_GroupState
 *
 *	NOTE: Group State is not actually useful in this release.
 *==============================================================================
 */
typedef struct MF3D_GroupState MF3D_GroupState, *MF3D_GroupStatePtr;

struct MF3D_GroupState
  {
    MF3DDisplayGroupStateFlags traversalFlags;	/* group state                  */
    MF3D_GroupStatePtr parent;	/* stack link                   */
  };

/*==============================================================================
 *	MF3D_ResolveState
 *
 *	Information used during a ResolveReference call.
 *==============================================================================
 */
typedef struct MF3D_FileRec MF3D_FileRec, *MF3D_FilePtr;

enum MF3D_ResolutionStateEnum
  {
    MF3D_NotResolvingReference,
    MF3D_ResolvingReference,
    MF3D_ResolvingGroup,
    MF3D_ResolvingFile = MF3D_ResolvingGroup
  };

typedef struct MF3D_ResolveStuff
  {
    MF3DUns32 resState;		/* read: resolving reference?   */
    MF3D_FilePtr reference;	/* read: link down to reference */
    MF3D_FilePtr parent;	/* read: link up to parent              */
    MF3DBinaryFilePosition returnLoc;	/* read: return location                */
  }
MF3D_ResolveStuff;

/*==============================================================================
 * MF3D_FileRec:
 *	Internal file record.
 *==============================================================================
 */
typedef enum MF3D_ReadWriteEnum
  {
    MF3D_MetafileRead,
    MF3D_MetafileWrite
  }
MF3D_ReadWriteEnum;

typedef struct MF3D_IOPrimitiveMethods *MF3D_IOPrimitivesTable;

struct MF3D_FileRec		/* internal file record                 */
  {
    MF3DUserFilePtr userFilePtr;	/* user-provided file pointer       */
    MF3D_ReadWriteEnum readWrite;	/* reading or writing                   */
    MF3DDataFormatEnum dataFormat;	/* binary, swapped, or text             */
    MF3DProcsRec procsRec;	/* internal record to callbacks */
    MF3D_IOPrimitivesTable primitives;	/* i/o methods table                    */
    MF3D_ReadBuffer readBuffer;	/* object-sized data buffer             */
    MF3DUns32 inContainer;	/* processing in a container        */
    MF3D_TOCStuff tocStuff;	/* table of contents info               */
    MF3DBinaryFilePosition tocLocation;		/* bin write: TOC ptr in header     */
    MF3D_ObjectTable objTable;	/* txt: index to all objects    */
    MF3DUns16 indent;		/* txt write: indentation               */
    MF3D_GroupState groupState;	/* read: current attributes             */
    MF3D_TypeTable typeTable;	/* user-defined types                   */
    MF3D_ResolveStuff resStuff;	/* read: reference resolution   */
    MF3D_WriteStackPtr writeStack;	/* bin write: container locs    */
  };

#define	kMF3D_MaxIndentation				127

#endif
