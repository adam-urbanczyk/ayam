/*==============================================================================
 *
 *	File:		MFTEXTWR.C
 *
 *	Function:	Routines for writing Text files
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
 *		F2K_RWW	File created.
 *==============================================================================
 */
#include "mftextwr.h"

#include <stdarg.h>		/* variable argument lists */
#include <stdio.h>		/* vsprintf */

#include "mferrors.h"
#include "mfassert.h"
#include "mfintobj.h"
#include "mftextst.h"

#if defined(applec) || defined(__MWERKS__) || defined(THINK_C)
#pragma segment __MF3D__
#endif

/*==============================================================================
 *	MF3D_WriteNewLine
 *
 *	Add a new line to a text file (it is supposed to be readable, after all)
 *==============================================================================
 */
MF3DErr
MF3D_WriteNewLine(
	MF3D_FilePtr	inMetafile)
{
	char	 			buffer[kMF3D_MaxIndentation + 1];
	unsigned int		indentation;

	if (!MF3DIsTextFormat(inMetafile->dataFormat))
		return kMF3DNoErr;

	indentation = inMetafile->indent + 1;
	while (indentation-- > 0)
		buffer[indentation] = kMF3D_TabChar;
	buffer[0] = kMF3D_NewLineChar;

	return MF3D_WriteProc(inMetafile, inMetafile->indent + 1, buffer);
}

/*==============================================================================
 *	MF3D_OutputText
 *
 *	fprintf
 *==============================================================================
 */
MF3DErr
MF3D_OutputText(MF3D_FilePtr inMetafile, const char *inFormatStr, ...)
{
	va_list		paramList;
	int			charsWritten;
	char		buffer[kMaxOutputTextBuffer];
	MF3DErr		result;

	result = kMF3DNoErr;

	va_start(paramList, inFormatStr);
	
	charsWritten = vsprintf(buffer, inFormatStr, paramList);

	va_end(paramList);

	MFASSERT(charsWritten < kMaxOutputTextBuffer);

	if (charsWritten < 0)
		result = kMF3DErrCantWrite;

	if (result == kMF3DNoErr)
		result = MF3D_WriteProc(inMetafile, charsWritten, buffer);

	return result;
}

/*==============================================================================
 *	MF3D_WriteTextString
 *
 *	Write a text string
 *==============================================================================
 */
MF3DErr
MF3D_WriteTextString(
	MF3D_FilePtr		inMetafilePtr,
	MF3DCStringPtr		inStringPtr)
{
	MF3DErr				result;
	char				c, *strPtr;

	result = kMF3DNoErr;

	strPtr = inStringPtr;

	/* Output characters until we reach EOS */
	while (result == kMF3DNoErr && (c = *strPtr++) != '\0')
	{	switch (c)
		{	/* These characters are escaped */
			case '\a':	c = 'a'; goto OutputEscape;
			case '\b':	c = 'b'; goto OutputEscape;
			case '\f':	c = 'f'; goto OutputEscape;
			case '\n':	c = 'n'; goto OutputEscape;
			case '\r':	c = 'r'; goto OutputEscape;
			case '\t':	c = 't'; goto OutputEscape;
			case '\v':	c = 'v'; goto OutputEscape;
			case '\\':
			case '\'':
			  /*			case '\"':*/ /* XXXX */
OutputEscape:
				result = MF3D_OutputText(inMetafilePtr, kMF3D_StringEscapeStr);
				/* Fall through to normal output */
			default:
				if (result == kMF3DNoErr)
					result = MF3D_OutputText(inMetafilePtr, "%c", c);
		}
	}

	return result;
}

