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
 *  write the first TC tag from object o to RIB
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
	  return AY_OK;
	} /* if(tagtype== */

      tag = tag->next;
    } /* while */

 return AY_OK;
} /* ay_tc_wrib */


/* ay_tc_wribindex:
 *  write TC tag number <index> from object o to RIB
 *  could be improved further (accelerated) using static variables
 */
int
ay_tc_wribindex(ay_object *o, int index)
{
 ay_tag_object *tag = NULL;
 RtFloat s1, s2, s3, s4, t1, t2, t3, t4;
 int i = 0;

  if(!o)
    return AY_ENULL;

  /* process tags */
  tag = o->tags;
  while(tag)
    {
      if(tag->type == ay_tc_tagtype)
	{
	  if(i == index)
	    {
	      sscanf(tag->val,"%f,%f,%f,%f,%f,%f,%f,%f",
		     &s1, &t1, &s2, &t2, &s3, &t3, &s4, &t4);
	      RiTextureCoordinates(s1, t1, s2, t2, s3, t3, s4, t4);
	      return AY_OK;
	    }

	  i++;
	} /* if(tagtype== */

      tag = tag->next;
    } /* while */

 return AY_OK;
} /* ay_tc_wribindex */


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
