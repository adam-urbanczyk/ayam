/*
 * Ayam, a free 3D modeler for the RenderMan interface.
 *
 * Ayam is copyrighted 1998-2004 by Randolf Schultz
 * (rschultz@informatik.uni-rostock.de) and others.
 *
 * All rights reserved.
 *
 * See the file License for details.
 *
 */

/* tti.h - truetype font parsing helpers */

/* original version of this file provided by Frank (copper) Pagels */

#ifndef TTI_H_
#define TTI_H_ 1

enum {
  AY_TTI_OK,
  AY_TTI_NOMEM,
  AY_TTI_NOTFOUND,
  AY_TTI_BADFONT
};


#define ONOROFF	0x01
#define XSHORT	0x02
#define YSHORT	0x04
#define REPEAT	0x08
#define XSAME	0x10
#define YSAME	0x20

#define ARG_1_AND_2_ARE_WORDS    0x0001
#define ARGS_ARE_XY_VALUES       0x0002
#define XY_BOUND_TO_GRID         0x0004
#define WE_HAVE_A_SCALE          0x0008
#define MORE_COMPONENTS          0x0020
#define WE_HAVE_AN_X_AND_Y_SCALE 0x0040
#define WE_HAVE_A_TWO_BY_TWO     0x0080
#define WE_HAVE_INSTRUCTIONS     0x0100
#define USE_MY_METRICS           0x0200


typedef struct ay_tti_point_s
{
  double x;
  double y;
} ay_tti_point;


typedef struct ay_tti_glyf_s
{
  signed short numberOfContours;
  signed short xMin, yMin, xMax, yMax;
} ay_tti_glyf;

typedef struct ay_tti_outline_s
{
  int filled;
  int numpoints;
  ay_tti_point *points;
} ay_tti_outline;

typedef struct ay_tti_letter_s
{
  double xoffset;
  double yoffset;
  int numoutlines;
  ay_tti_outline *outlines;
} ay_tti_letter;

typedef struct longhormetric_s
{
  unsigned short advanceWidth;
  signed short lsb;
} LONGHORMETRIC;

typedef struct ttf_hhea_s
{
  unsigned char	version[4];
  signed short ascender, descender, lineGap;
  unsigned short advanceWidthMax;
  signed short minLSB, minRSB, xMaxExtent;
  signed short caretSlopeRise, caretSlopeRun;
  signed short reserved[5];
  signed short metricDataFormat;
  unsigned short numberOfHMetrics;
} TTF_HHEA;


typedef struct ttf_cmap0_s
{
  unsigned short format;
  unsigned short length;
  unsigned short language;
  unsigned char	glyphIndexArray[256];
} TTF_CMAP0;

typedef struct ttf_cmap4_s
{
  unsigned short format;
  unsigned short length;
  unsigned short version;
  unsigned short segCountX2;
  unsigned short searchRange;
  unsigned short entrySelector;
  unsigned short rangeShift;
} TTF_CMAP4;


typedef struct ttf_cmap_entry_s
{
  unsigned short platformID;
  unsigned short encodingID;
  unsigned int offset;
} TTF_CMAP_ENTRY;

typedef struct ttf_cmap_s
{
  unsigned short version;
  unsigned short numberOfEncodingTables;
  TTF_CMAP_ENTRY encodingTable[1];
} TTF_CMAP ;

typedef struct ttf_dir_entry_s
{
  unsigned char	tag[4];
  unsigned int checksum;
  unsigned int offset;
  unsigned int length;
} TTF_DIR_ENTRY;

typedef struct ttf_directory_s
{
  unsigned int sfntVersion;
  unsigned short numTables;
  unsigned short searchRange;
  unsigned short entrySelector;
  unsigned short rangeShift;
  TTF_DIR_ENTRY	list;
} TTF_DIRECTORY;


typedef struct ay_tti_font_s
{
  void *fontptr;
  TTF_CMAP *cmap;
  unsigned char	*loca;
  unsigned char	*glyf;
  LONGHORMETRIC *hmtx;
  unsigned char	*buffer;
  unsigned char	*dbuffer;
  int numtable;
  int size;
  int flags;
  int unitem;
  int locformat;
  short	ascent, descent;
  int nglyf;
  int soffset; /* start offset for outline in string */
  int yoffset;
  unsigned short numberOfHMetrics;
  double scale;
} ay_tti_font;

int ay_tti_getcurves(char *ttfname, int letter, ay_tti_letter *cur);

int ay_tti_outlinetoncurve(ay_tti_outline *outline, ay_object **result);


#endif /* TTI_H_ */
