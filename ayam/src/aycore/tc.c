/*
 * Ayam, a free 3D modeler for the RenderMan interface.
 *
 * Ayam is copyrighted 1998-2001 by Randolf Schultz
 * (randolf.schultz@gmail.com) and others.
 *
 * All rights reserved.
 *
 * See the file License for details.
 *
 */

#include "ayam.h"

/* tc.c - TC (TextureCoordinates) tag helpers */

/* ay_tc_wrib:
 *  write the first TC tag from object o to RIB
 */
int
ay_tc_wrib(ay_object *o)
{
 char fname[] = "ay_tc_wrib";
 int count;
 ay_tag *tag = NULL;
 RtFloat s1, s2, s3, s4, t1, t2, t3, t4;

  if(!o)
    return AY_ENULL;

  /* process tags */
  tag = o->tags;
  while(tag)
    {
      if(tag->type == ay_tc_tagtype)
	{
	  count = sscanf(tag->val,"%f,%f,%f,%f,%f,%f,%f,%f",
			 &s1, &t1, &s2, &t2, &s3, &t3, &s4, &t4);
	  if(count != 8)
	    {
	      ay_error(AY_ERROR, fname, "Malformed TC tag encountered.");
	      ay_error_reportobject(AY_ERROR, fname, o);
	      return AY_ERROR;
	    }
	  RiTextureCoordinates(s1, t1, s2, t2, s3, t3, s4, t4);
	  return AY_OK;
	} /* if(tagtype== */

      tag = tag->next;
    } /* while */

 return AY_OK;
} /* ay_tc_wrib */


/* ay_tc_init:
 *  initialize tc module by registering the TC tag type
 */
void
ay_tc_init(Tcl_Interp *interp)
{

  /* register TC tag type */
  ay_tags_register(ay_tc_tagname, &ay_tc_tagtype);

 return;
} /* ay_tc_init */
