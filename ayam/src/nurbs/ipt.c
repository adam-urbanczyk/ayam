/*
 * Ayam, a free 3D modeler for the RenderMan interface.
 *
 * Ayam is copyrighted 1998-2011 by Randolf Schultz
 * (randolf.schultz@gmail.com) and others.
 *
 * All rights reserved.
 *
 * See the file License for details.
 *
 */

#include "ayam.h"

/* ipt.c - interpolating surface tools */

/* ay_ipt_swapuv:
 *
 *
 */
int
ay_ipt_swapuv(ay_ipatch_object *ip)
{
 int ay_status = AY_OK;
 int i;

  if(!ip)
    return AY_ENULL;

  ay_status = ay_npt_swaparray(&(ip->controlv), 3, ip->width, ip->height);

  i = ip->width;
  ip->width = ip->height;
  ip->height = i;

  i = ip->order_u;
  ip->order_u = ip->order_v;
  ip->order_v = i;

  i = ip->close_u;
  ip->close_u = ip->close_v;
  ip->close_v = i;

  i = ip->ktype_u;
  ip->ktype_u = ip->ktype_v;
  ip->ktype_v = i;

 return ay_status;
} /* ay_ipt_swapuv */


/* ay_ipt_revertu:
 *
 *
 */
int
ay_ipt_revertu(ay_ipatch_object *ip)
{
 int ay_status = AY_OK;
 int i, j, ii, jj, stride = 3;
 double t[4];

  for(i = 0; i < ip->height; i++)
    {
      for(j = 0; j < ip->width/2; j++)
	{
	  ii = (j*ip->height+i)*stride;
	  jj = ((ip->width-1-j)*ip->height+i)*stride;
	  memcpy(t, &(ip->controlv[ii]), stride*sizeof(double));
	  memcpy(&(ip->controlv[ii]), &(ip->controlv[jj]),
		 stride*sizeof(double));
	  memcpy(&(ip->controlv[jj]), t, stride*sizeof(double));
	}
    }

 return ay_status;
} /* ay_ipt_revertu */


/* ay_ipt_revertv:
 *
 */
int
ay_ipt_revertv(ay_ipatch_object *ip)
{
 int ay_status = AY_OK;
 int i, j, ii, jj, stride = 3;
 double t[4];

  for(i = 0; i < ip->width; i++)
    {
      ii = i*ip->height*stride;
      jj = ii + ((ip->height-1)*stride);
      for(j = 0; j < ip->height/2; j++)
	{
	  memcpy(t, &(ip->controlv[ii]), stride*sizeof(double));
	  memcpy(&(ip->controlv[ii]), &(ip->controlv[jj]),
		 stride*sizeof(double));
	  memcpy(&(ip->controlv[jj]), t, stride*sizeof(double));
	  ii += stride;
	  jj -= stride;
	}
    }

 return ay_status;
} /* ay_ipt_revertv */
