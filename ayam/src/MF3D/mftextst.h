#ifndef	MF3D_TEXTSTRINGS_H
#define	MF3D_TEXTSTRINGS_H
/*==============================================================================
 *
 *	File:		MFTEXTST.H
 *
 *	Function:	Miscellaneous text snippets required for I/O
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
 *		FB7_JRK	Pragma macros
 *		Fabio	Changed file name to 8 characters
 *		F3A_RWW	TOC stuff works.
 *		F2K_RWW	File created.
 *==============================================================================
 */
#if defined(applec) || defined(__MWERKS__) || defined(THINK_C)
#pragma once
#endif

/* Maximum length of format passed to MF3D_ScanTextFile */
#define	kMF3D_MaxFormatLength	90

/* FBL_JRK
 * Some of the strings below could be made simpler, but the MPW C++ preprocessor
 * doesn't concatenate strings in macro-replacements for some reason.
 */

/* List of white space characters (std C) */
#define	kMF3D_WhitespaceList	" \f\n\r\t\v"
#define	kMF3D_IngoreWSNFormat	"%*[ \f\n\r\t\v]%n"
#define	kMF3D_EndOfLineList		"\n\r"
#define kMF3D_IgnoreEOLNFormat	"%*[^\n\r]%n"
#define	kMF3D_HexCharList		"0123456789ABCDEFabcdef"
#define	kMF3D_HexCharPattern	"[0123456789ABCDEFabcdef]"
#define	kMF3D_HexCharFormat		"%[0123456789ABCDEFabcdef]"
#define	kMF3D_TextCharList		"0123456789"					\
								"ABCDEFGHIJKLMNOPQRSTUVWXYZ"	\
								"abcdefghijklmnopqrstuvwxyz"
#define	kMF3D_TextCharFormat	"%[0123456789"					\
								"ABCDEFGHIJKLMNOPQRSTUVWXYZ"	\
								"abcdefghijklmnopqrstuvwxyz]"
#define	kMF3D_BeginCharStr		"("
#define kMF3D_BeginCharFormat	"%1[(]"
#define	kMF3D_EndCharStr		")"
#define	kMF3D_EndCharFormat		"%1[)]"
#define	kMF3D_CommentLineStr	"#"
#define kMF3D_CommentLineNFormat	"%1[#]%n"
#define	kMF3D_StringBeginStr	"\""
#define kMF3D_StringBeginNFormat	"%1[\"]%n"
#define	kMF3D_StringEndStr		kMF3D_StringBeginStr
#define	kMF3D_OrCharStr			"|"
#define	kMF3D_OrCharFormat		"%1[|]"
#define	kMF3D_ReferencePtrStr	">"
#define	kMF3D_LabelCharStr		":"
#define	kMF3D_NoTOCPtrStr		"nextTOC"
#define	kMF3D_DefaultTOCPtrStr	"toc"
#define	kMF3D_DefaultLabelStr	"MF3DLabel_"
#define	kMF3D_StringEscapeStr	"\\"

#define kMF3D_IgnoreTextFormat	"%1c%*[^( \f\n\r\t\v]"
#define kMF3D_NameFormat		"%[^() \f\n\r\t\v]"

#define	kMF3D_HexCharListLen	22	/* strlen(kMF3D_HexCharList)      */
#define	kMF3D_MaxDefaultLabelLen	64	/* arbitrarily large                      */

/* Hex string prefix */
#define	kMF3D_RawDataHexPrefix	"0x"
#define	kMF3D_RawDataHexPrefixLen	2

/* Used during preprocessing before we can use MF3D_ScanTextBuffer */
#define	kMF3D_BeginChar			'('
#define	kMF3D_EndChar			')'
#define	kMF3D_FilePtrChar		'>'
#define	kMF3D_LabelChar			':'
#define	kMF3D_CommentLineChar	'#'
#define	kMF3D_StringBeginChar	'"'
#define	kMF3D_StringEndChar		kMF3D_StringBeginChar
#define	kMF3D_StringEscapeChar	'\\'
#define	kMF3D_NewLineChar		'\n'
#define	kMF3D_TabChar			'\t'

#endif
