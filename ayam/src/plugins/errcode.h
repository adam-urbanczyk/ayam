
/* Return/Error Codes */
#define AY_OK          0 /* everything all right */  
#define AY_EWARN       1 /* warning */
#define AY_ERROR       2 /* unspecified error */
#define AY_EFLUSH      3 /* flush error messages */
#define AY_EOUTPUT     4 /* used for unspecific output */
#define AY_EOMEM       5 /* out of memory */
#define AY_EOPENFILE  10 /* error opening file */
#define AY_ECLOSEFILE 11 /* error closing file */
#define AY_EFORMAT    12 /* wrong file format  */
#define AY_EUEOF      13 /* unexpected EOF read */
#define AY_EEOF       14 /* EOF read */
#define AY_EDONOTLINK 15 /* Do not link read object! */
#define AY_ENOSEL     20 /* nothing selected */
#define AY_EARGS      21 /* wrong args */
#define AY_EWTYPE     22 /* object is of wrong type */
#define AY_ETYPE      30 /* type exists */
#define AY_ENTYPE     31 /* type does not exist */
#define AY_EREF       40 /* reference counter not zero */
#define AY_ENULL      50 /* illegal zero pointer encountered */
