/*
 * Ayam, a free 3D modeler for the RenderMan interface.
 *
 * Ayam is copyrighted 1998-2001 by Randolf Schultz
 * (rschultz@informatik.uni-rostock.de) and others.
 *
 * All rights reserved.
 *
 * See the file License for details.
 *
 */

#include "ayam.h"

/* tc.c - TC (TextureCoordinates) tag helpers */

/* ay_tc_wrib:
 *  write all TC tags from object o to RIB
 */
int
ay_tc_wrib(ay_object *o)
{
 ay_tag_object *tag = NULL;
 RtFloat s1, s2, s3, s4, t1, t2, t3, t4;

  if(!o)
    return AY_ENULL;

  /* process tags */
  tag = o->tags;
  while(tag)
    {
      if(tag->type == ay_tc_tagtype)
	{
	  sscanf(tag->val,"%f,%f,%f,%f,%f,%f,%f,%f",
		 &s1, &t1, &s2, &t2, &s3, &t3, &s4, &t4);
	  RiTextureCoordinates(s1, t1, s2, t2, s3, t3, s4, t4);
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
  ay_tags_register(interp, "TC", &ay_tc_tagtype);

 return;
} /* ay_tc_init */
