/*==============================================================================
 *
 *	File:		MFMEMORY.C
 *
 *	Function:	Memory allocation routines
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
 *		FBL_JRK	C++ Compatibility
 *		FBL_JRK	Lowercase include file names
 *		FB8_JRK	Segmentation
 *		FB7_JRK	Deal with MetroWerks allocation "Bug"
 *		FB6_JRK Accept real MF3D_Uns32 and check for size_t overflow
 *		FAK_JRK	Added error debugging when DEBUG > 0
 *		F7K_PAM bug fixing for null pointers and sizes of zero
 *		Fabio	Changed file name to 8 characters
 *		F3L_RWW	File created.
 *==============================================================================
 */
#define MF3D_MEMORY_C
#include "mfmemory.h"
#undef MF3D_MEMORY_C

#if defined(DEBUG) && DEBUG > 0
#include <stdio.h>		/* printf for debugging */
#endif
#include <limits.h>		/* CHAR_BIT */

#include "mfassert.h"
#include "mferrors.h"

#if defined(applec) || defined(__MWERKS__) || defined(THINK_C)
#pragma segment __MF3D__
#endif


/* FB7_JRK
 * MetroWerks doesn't return pointers > 32K back to Mac OS we we call free()
 * so we have use NewPtr and DisposePtr directory to allocate memory.
 */

#if defined(__MWERKS__) && (defined(__MC68K__) || defined(__POWERPC__))
#define kUseNewPtrDirectly	1
#else
#define kUseNewPtrDirectly	0
#endif


#if kUseNewPtrDirectly
#include <Memory.h>
#endif


/* FB6_JRK */
/* Based on size_t, calculate max positive value for allocation. */
#if !kUseNewPtrDirectly
static const unsigned long gMaxAllocSize =
((size_t) - 1) > 0 ?
(size_t) - 1 :			/* size_t unsigned      */
(1UL << (sizeof (size_t) * CHAR_BIT - 1)) - 1;	/* size_t signed        */
#else
static const unsigned long gMaxAllocSize =
((Size) - 1) > 0 ?
(Size) - 1 :			/* Size unsigned        */
(1UL << (sizeof (Size) * CHAR_BIT - 1)) - 1;	/* Size signed  */
#endif /* kUseNewPtrDirectly */


void *
MF3D_Malloc (
	      MF3DUns32 size)
{
#if defined(DEBUG) && DEBUG > 0
  size_t *tempPtr;

  size += sizeof (*tempPtr);

  if (size > gMaxAllocSize)
    return NULL;

  tempPtr = malloc ((size_t) size);
  if (tempPtr)
    {
      *tempPtr++ = (size_t) size;
      gMallocSize += size;
#if DEBUG > 1
      printf ("%d: malloc(%lx)\n", ++gMallocCount, tempPtr);
#endif
    }
  else
    {
      printf ("MF3D_Malloc: Error allocating %ld bytes."
	      "%ld have already been allocated.\n",
	      (long) size, (long) gMallocSize);
    }

  return tempPtr;
#else
/* FB6_JRK
 * Check to see that conversion from MF3DUns32 to size_t doesn't
 * lose anything. Otherwise we might allocate something smaller.
 */
  if (size > gMaxAllocSize)
    return NULL;

#if kUseNewPtrDirectly
  return NewPtr ((Size) size);
#else
  return malloc ((size_t) size);
#endif /* kUseNewPtrDirectly */
#endif /* DEBUG */
}


void *
MF3D_Realloc (
	       void *ptr,
	       MF3DUns32 size)
{
#if defined(DEBUG) && DEBUG > 0
  size_t *tempPtr;

  /* The C manual does not seem to explicitly state that
   * realloc(NULL, size) is equivalent to malloc(size).
   * So, to make sure we do not fail with some compiler somewhere,
   * we will always malloc before realloc.
   */
  MFASSERT (size > 0);

  /* Handle the case where there is no existing pointer */
  size += sizeof (*tempPtr);

  if (size > gMaxAllocSize)
    return NULL;

  if (ptr == NULL)
    tempPtr = malloc ((size_t) size);
  else
    tempPtr = realloc ((size_t *) ptr - 1, (size_t) size);

  if (tempPtr)
    {
      if (ptr)
	gMallocSize -= *tempPtr;
      *tempPtr++ = (size_t) size;
      gMallocSize += size;
    }
  else
    {
      printf ("MF3D_Realloc: Error allocating %ld bytes."
	      "%ld have already been allocated.\n",
	      (long) size, (long) gMallocSize);
    }

/* FAK_JRK Now handled by above code.
 *    MFASSERT(tempPtr != NULL);
 */

#if DEBUG > 1
  if (tempPtr != ptr)
    {
      if (ptr == NULL)
	++gMallocCount;
      printf ("%d: realloc(%lx to %lx)\n", gMallocCount, ptr, tempPtr);
    }
#endif

  return tempPtr;
#else
  /* Handle the case where there is no existing pointer */
  if (ptr == NULL)
    return MF3D_Malloc (size);
  /* FB6_JRK
   * Check to see that conversion from MF3DUns32 to size_t doesn't
   * lose anything. Otherwise we might allocate something smaller.
   */
  else if (size <= gMaxAllocSize)
#if kUseNewPtrDirectly
    {
      void *tempPtr = NewPtr ((Size) size);
      if (tempPtr != NULL)
	{
	  BlockMove (ptr, tempPtr, (Size) size);
	  DisposePtr (ptr);
	}
      return tempPtr;
    }
#else
    return realloc (ptr, (size_t) size);
#endif /* kUseNewPtrDirectly */
  else
    return NULL;
#endif
}


void
MF3D_Free (
	    void *ptr)
{
#if defined(DEBUG) && DEBUG > 0
  if (ptr)
    {
      size_t *sizePtr = (size_t *) ptr - 1;
      gMallocSize -= *sizePtr;
      free (sizePtr);
#if DEBUG > 1
      printf ("%d: free(%lx)\n", --gMallocCount, ptr);
#endif
    }
#elif kUseNewPtrDirectly
  DisposePtr ((Ptr) ptr);
#else
  free (ptr);
#endif
}


/* FBL_JRK */
MF3DErr
MF3D_Reallocate (void **ptr, MF3DUns32 size)
{
  MF3DErr result = kMF3DNoErr;

  void *newPtr;

  newPtr = MF3D_Realloc (*ptr, size);
  if (newPtr)
    *ptr = newPtr;
  else
    result = kMF3DErrOutOfMemory;

  return result;
}
