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

/*
** metautils.c:
**  the stuff for calculating the metacomponents
**  2001 Frank Pagels 
*/

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "meta.h"
#include "ayam.h"

#define pos(p) (w->mgrid[p.x * w->aktcubes * w->aktcubes + p.y * w->aktcubes + p.z])


/* calculate the effect for all components in list */
double
meta_calcall (double x1, double y1, double z1, meta_world * w)
{
  double effect, dist, radius, tmpeffect;
  meta_blob *tmp;
  ay_object *o;
  double x, y, z;
  Tcl_Obj *to = NULL, *ton = NULL;

  effect = 0;
  dist = 0;

  o = w->o;

  while (o->next != NULL)
    {

	if(o->type == metacomp_id)
	{

      	tmp = (meta_blob *) o->refine;
      	radius = tmp->r * tmp->r;

      	/* rotate and scale the Object */

		x =
	  	    (tmp->rm[0] * x1 + tmp->rm[4] * y1 + tmp->rm[8] * z1 +
	  	    tmp->rm[12] * 1.0);
	  	y =
	  	    (tmp->rm[1] * x1 + tmp->rm[5] * y1 + tmp->rm[9] * z1 +
	  	    tmp->rm[13] * 1.0);
	  	z =
	  	    (tmp->rm[2] * x1 + tmp->rm[6] * y1 + tmp->rm[10] * z1 +
	  	    tmp->rm[14] * 1.0);

	  	if (!((tmp->formula == META_BALL)&&(w->version==1)))
	     {
			x *= tmp->scalex;
			y *= tmp->scaley;
			z *= tmp->scalez;
		}


    	       /* the normal metaball */

	      if (tmp->formula == META_BALL)
	      {
    	         if(w->version==1)
		    	  dist = META_DIST (x, y, z, tmp->cp.x, tmp->cp.y, tmp->cp.z);
		    else
		    	  dist = META_DIST2 (x, y, z, tmp->cp.x, tmp->cp.y, tmp->cp.z);
	         
		    
		    if (dist <= radius)
	         {
		       tmpeffect = tmp->a * META_CUB (dist) / META_CUB (radius) +
		       tmp->b * META_SQ (dist) / META_SQ (radius) + tmp->c * dist / radius + 1.0;
	
		       if (tmp->negativ)
			  {
			    effect -= tmpeffect;
			  }
		       else
			  effect += tmpeffect;
	         }

	      }


		 /* a Cube */
		 
		 if (tmp->formula == META_CUBE)
		 {

		  {	 
		   tmpeffect = (pow(META_ABS(x-tmp->cp.x),tmp->ex) +  pow(META_ABS(y -tmp->cp.y),tmp->ey) \
		   +  pow(META_ABS(z - tmp->cp.z),tmp->ez))*9000;
		 
		   tmpeffect = 1/(tmpeffect < 0.00001 ? 0.00001 : tmpeffect);
	
		   if (tmp->negativ)
		   {
			  effect -= tmpeffect;
		   }
		      else
			effect += tmpeffect;
		  }
	
		}
	 

		 /* a Torus */
		 
		 if (tmp->formula == META_TORUS)
		{

		  if (tmp->rot)
		    tmpeffect =
		      META_SQ (META_SQ(x - tmp->cp.x) + META_SQ (y - tmp->cp.y) + META_SQ(z - tmp->cp.z) + tmp->Ro * tmp->Ro -
			  tmp->Ri * tmp->Ri) - 4.0 * META_SQ (tmp->Ro) * (META_SQ (z - tmp->cp.z) + META_SQ (y - tmp->cp.y));
		  else
		    tmpeffect =
		      META_SQ (META_SQ(x - tmp->cp.x) + META_SQ (y - tmp->cp.y) + META_SQ(z - tmp->cp.z) + tmp->Ro * tmp->Ro -
			  tmp->Ri * tmp->Ri) - 4.0 * META_SQ (tmp->Ro) * (META_SQ (x - tmp->cp.x) + META_SQ (y - tmp->cp.y));

		  if (tmp->negativ)
		    {
		      effect -=
			1 / (tmpeffect < 0.00001 ? 0.00001 : tmpeffect) * 0.006;
		    }
		  else
		    effect += 1 / (tmpeffect < 0.00001 ? 0.00001 : tmpeffect) * 0.006;
		}

          /* create a heart */

    	     if (tmp->formula == META_HEART)
	     {

	       tmpeffect =
	          META_CUB (2 * META_SQ (x - tmp->cp.x) + META_SQ (y - tmp->cp.y) + META_SQ (z - tmp->cp.z) - 1) -
		     (0.1 * META_SQ (x - tmp->cp.x) + META_SQ (y - tmp->cp.y)) * META_CUB (z - tmp->cp.z);

	       if (tmp->negativ)
	       {
	          effect -=
		     1 / (tmpeffect < 0.00001 ? 0.00001 : tmpeffect) * 0.002;
	       }
	       else
	          effect += 1 / (tmpeffect < 0.00001 ? 0.00001 : tmpeffect) * 0.002;
	     }


         /* a custom formula */
	    
	    if (tmp->formula == META_CUSTOM)
	    {
	       ton = Tcl_NewStringObj("x", -1);
	       to = Tcl_NewDoubleObj(x - tmp->cp.x);
	       Tcl_ObjSetVar2(ay_interp, ton, NULL, to, TCL_LEAVE_ERR_MSG |
			TCL_GLOBAL_ONLY);
	       Tcl_SetStringObj(ton, "y", -1);
  	       to = Tcl_NewDoubleObj(y - tmp->cp.y);
	       Tcl_ObjSetVar2(ay_interp, ton, NULL, to, TCL_LEAVE_ERR_MSG |
			TCL_GLOBAL_ONLY);

	       Tcl_SetStringObj(ton, "z", -1);
  	       to = Tcl_NewDoubleObj(z - tmp->cp.z);
	       Tcl_ObjSetVar2(ay_interp, ton, NULL, to, TCL_LEAVE_ERR_MSG |
			TCL_GLOBAL_ONLY);
	       Tcl_SetStringObj(ton, "f", -1);
  	       to = Tcl_NewDoubleObj(0.0);
	       Tcl_ObjSetVar2(ay_interp, ton, NULL, to, TCL_LEAVE_ERR_MSG |
			TCL_GLOBAL_ONLY);

	      if(tmp->expression)
	      Tcl_GlobalEvalObj(ay_interp, tmp->expression);

	      Tcl_SetStringObj(ton, "f", -1);
	      to = Tcl_ObjGetVar2(ay_interp, ton, NULL, TCL_LEAVE_ERR_MSG |
			      TCL_GLOBAL_ONLY);
	      Tcl_GetDoubleFromObj(ay_interp, to, &tmpeffect);

	      Tcl_IncrRefCount (ton);
	      Tcl_DecrRefCount (ton);

	      if(tmp->negativ)
	      {
	        effect -= 1 / (tmpeffect < 0.00001 ? 0.00001 : tmpeffect) *
		   0.002;
	      }
	      else
	      {
	        effect += 1 / (tmpeffect < 0.00001 ? 0.00001 : tmpeffect) *
		   0.002;
	      }
	    }

      }
	o = o->next;

    }

  return effect;

}


void
meta_getstart (meta_blob * b, meta_intxyz * p, meta_world * w)
{

  p->x = (int) (b->cp.x / w->edgelength) + w->aktcubes / 2;

  if (b->formula == META_TORUS)
    p->y = (int) ((b->cp.y + b->Ro) / w->edgelength) + w->aktcubes / 2;
  else
    p->y = (int) (b->cp.y / w->edgelength) + w->aktcubes / 2;

  p->z = (int) (b->cp.z / w->edgelength) + w->aktcubes / 2;

  if (p->x < 0)
    p->x = 0;

  if (p->x > w->aktcubes - 1)
    p->x = (w->aktcubes - 1);

  if (p->y < 0)
    p->y = 0;

  if (p->y > w->aktcubes - 1)
    p->y = (w->aktcubes - 1);

  if (p->z < 0)
    p->z = 0;

  if (p->z > w->aktcubes - 1)
    p->z = (w->aktcubes - 1);
}

int
meta_initcubestack (meta_world * w)
{

  if ((w->stack = (meta_gridcell *) calloc (1, sizeof (meta_gridcell) * 2000)))
    {
      w->stackpos = 0;
      w->maxstack = 2000;
      return META_OK;
    }
  else
    return AY_EOMEM;

}

int
meta_freecubestack (meta_world * w)
{
  if (w->stack)
    free (w->stack);

  return META_OK;
}

void
meta_pushcube (meta_gridcell * cube, meta_world * w)
{
  if (w->stackpos == w->maxstack)
    {
      w->stack = realloc (w->stack, sizeof (meta_gridcell) * (w->maxstack + 1000));
      w->maxstack += 1000;
    }

  w->stack[w->stackpos] = *cube;
  w->stackpos++;

}

meta_gridcell
meta_popcube (meta_world * w)
{

  w->stackpos--;
  return (w->stack[w->stackpos]);

}

void
meta_initstartcube (meta_world * w, meta_gridcell * cube, meta_intxyz * p)
{

#define length w->edgelength

  cube->p[0].x = p->x * length - w->unisize / 2;
  cube->p[0].y = p->y * length - w->unisize / 2;
  cube->p[0].z = p->z * length - w->unisize / 2;
  cube->val[0] = meta_calcall (cube->p[0].x, cube->p[0].y, cube->p[0].z, w);

  cube->p[1].x = cube->p[0].x + length;
  cube->p[1].y = cube->p[0].y;
  cube->p[1].z = cube->p[0].z;
  cube->val[1] = meta_calcall (cube->p[1].x, cube->p[1].y, cube->p[1].z, w);

  cube->p[2].x = cube->p[1].x;
  cube->p[2].y = cube->p[0].y;
  cube->p[2].z = cube->p[0].z + length;
  cube->val[2] = meta_calcall (cube->p[2].x, cube->p[2].y, cube->p[2].z, w);

  cube->p[3].x = cube->p[0].x;
  cube->p[3].y = cube->p[0].y;
  cube->p[3].z = cube->p[2].z;
  cube->val[3] = meta_calcall (cube->p[3].x, cube->p[3].y, cube->p[3].z, w);

  cube->p[4].x = cube->p[0].x;
  cube->p[4].y = cube->p[0].y + length;
  cube->p[4].z = cube->p[0].z;
  cube->val[4] = meta_calcall (cube->p[4].x, cube->p[4].y, cube->p[4].z, w);

  cube->p[5].x = cube->p[1].x;
  cube->p[5].y = cube->p[4].y;
  cube->p[5].z = cube->p[0].z;
  cube->val[5] = meta_calcall (cube->p[5].x, cube->p[5].y, cube->p[5].z, w);

  cube->p[6].x = cube->p[1].x;
  cube->p[6].y = cube->p[4].y;
  cube->p[6].z = cube->p[2].z;
  cube->val[6] = meta_calcall (cube->p[6].x, cube->p[6].y, cube->p[6].z, w);

  cube->p[7].x = cube->p[0].x;
  cube->p[7].y = cube->p[4].y;
  cube->p[7].z = cube->p[2].z;
  cube->val[7] = meta_calcall (cube->p[7].x, cube->p[7].y, cube->p[7].z, w);

#undef length
}


void
meta_addneighbors (meta_gridcell * cube, meta_world * w)
{
  int edgecode;
  int square;
  int pos;
  meta_gridcell tmpcube;

#define act w->aktcubes

  square = act * act;

  edgecode = w->edgecode;

  /* check back */
  if (!(edgecode & 4080) || (edgecode & 1 << 9) || (edgecode & 1 << 4)
      || (edgecode & 1 << 8))
    {
      if (cube->pos.z > 0)
	{
	  pos = cube->pos.x * square + act * cube->pos.y + (cube->pos.z - 1);

	  if (w->mgrid[pos] != w->lastmark)
	    {
	      tmpcube = *cube;
	      meta_moveback (&tmpcube, w);
	      meta_pushcube (&tmpcube, w);
	      w->mgrid[pos] = w->lastmark;
	    }
	}
    }



  /* check right */
  if ((edgecode & 1) || (edgecode & 1 << 9) || (edgecode & 1 << 5)
      || (edgecode & 1 << 10))
    {
      if (cube->pos.x < act - 1)
	{
	  pos = (cube->pos.x + 1) * square + act * cube->pos.y + cube->pos.z;

	  if (w->mgrid[pos] != w->lastmark)
	    {
	      tmpcube = *cube;
	      meta_moveright (&tmpcube, w);
	      meta_pushcube (&tmpcube, w);
	      w->mgrid[pos] = w->lastmark;

	    }
	}

    }


  /* check front */
  if ((edgecode & 1 << 2) || (edgecode & 1 << 6) || (edgecode & 1 << 10)
      || (edgecode & 1 << 11))
    {
      if (cube->pos.z < act - 1)
	{
	  pos = cube->pos.x * square + act * cube->pos.y + (cube->pos.z + 1);

	  if (w->mgrid[pos] != w->lastmark)
	    {
	      tmpcube = *cube;
	      meta_movefront (&tmpcube, w);
	      meta_pushcube (&tmpcube, w);
	      w->mgrid[pos] = w->lastmark;

	    }
	}


    }


  /* check left */
  if ((edgecode & 1 << 3) || (edgecode & 1 << 7) || (edgecode & 1 << 8)
      || (edgecode & 1 << 11))
    {
      if (cube->pos.x > 0)
	{

	  pos = (cube->pos.x - 1) * square + act * cube->pos.y + cube->pos.z;

	  if (w->mgrid[pos] != w->lastmark)
	    {
	      tmpcube = *cube;
	      meta_moveleft (&tmpcube, w);
	      meta_pushcube (&tmpcube, w);
	      w->mgrid[pos] = w->lastmark;

	    }

	}

    }


  /* check up */
  if ((edgecode & 1 << 4) || (edgecode & 1 << 5) || (edgecode & 1 << 6)
      || (edgecode & 1 << 7))
    {
      if (cube->pos.y < act - 1)
	{
	  pos = cube->pos.x * square + act * (cube->pos.y + 1) + cube->pos.z;

	  if (w->mgrid[pos] != w->lastmark)
	    {
	      tmpcube = *cube;
	      meta_moveup (&tmpcube, w);
	      meta_pushcube (&tmpcube, w);
	      w->mgrid[pos] = w->lastmark;

	    }
	}

    }

  /* check down */
  if (!(edgecode & 4080) || (edgecode & 1) || (edgecode & 1 << 2)
      || (edgecode & 1 << 3))
    {
      if (cube->pos.y > 0)
	{
	  pos = cube->pos.x * square + act * (cube->pos.y - 1) + cube->pos.z;

	  if (w->mgrid[pos] != w->lastmark)
	    {
	      tmpcube = *cube;
	      meta_movedown (&tmpcube, w);
	      meta_pushcube (&tmpcube, w);
	      w->mgrid[pos] = w->lastmark;

	    }
	}


    }


#undef act
}


int
meta_searchcube (meta_gridcell * cube, meta_intxyz * p, meta_world * w)
{
  int code;
  meta_intxyz pb;


  pb = *p;

  code = 0;

  /* search up */
  do
    {
      if (p->y < w->aktcubes - 1)
	{
	  p->y++;
	  meta_moveup (cube, w);	/* move one position up */

	  code = meta_polygonise (w, cube, w->isolevel);
	}
      else
	break;


    }
  while (code == 0);

  /* search down */
  if (code == 0)
    {
      do
	{
	  if (p->y > 0)
	    {
	      p->y--;
	      meta_movedown (cube, w);	/* move one position up */

	      code = meta_polygonise (w, cube, w->isolevel);
	    }
	  else
	    break;
	}
      while (code == 0);

    }

  /* search left */
  if (code == 0)
    {
      do
	{
	  if (p->x > 0)
	    {
	      p->x--;
	      meta_moveleft (cube, w);	/* move one position up */

	      code = meta_polygonise (w, cube, w->isolevel);
	    }
	  else
	    break;
	}
      while (code == 0);

    }

  /* search right */
  if (code == 0)
    {
      do
	{
	  if (p->x < w->aktcubes - 1)
	    {
	      p->x++;
	      meta_moveright (cube, w);	/* move one position up */

	      code = meta_polygonise (w, cube, w->isolevel);
	    }
	  else
	    break;
	}
      while (code == 0);

    }


  /* search front */
  if (code == 0)
    {
      do
	{
	  if (p->z < w->aktcubes - 1)
	    {
	      p->z++;
	      meta_movefront (cube, w);	/* move one position to the front */

	      code = meta_polygonise (w, cube, w->isolevel);
	    }
	  else
	    break;
	}
      while (code == 0);

    }

  /* search back */
  if (code == 0)
    {
      do
	{
	  if (p->z > 0)
	    {
	      p->z--;
	      meta_moveback (cube, w);	/* move one position to the front */

	      code = meta_polygonise (w, cube, w->isolevel);
	    }
	  else
	    break;
	}
      while (code == 0);

    }

  return (code);

}


int
meta_calceffect (meta_world * w)
{
  meta_blob *b;
  meta_intxyz p;
  int code, n;
  ay_object *o;
  meta_gridcell cube;

  n = 0;

  o = w->o;

  w->lastmark++;
  w->stackpos = 0;


#if META_USEVERTEXARRAY
  /* Reset Hash */
  memset(w->vhash,0,(sizeof (int) * ((w->tablesize-1) + (w->tablesize/10 -1) + (w->tablesize/100 -1))));
  w->h1 = w->tablesize/10;
  w->h2 = w->tablesize/100;
  w->actindex = 0;
  w->indexnum = 0;
#endif
  
  while (o->next != NULL)
    {
      n++;

	if(o->type == metacomp_id)
	{
     	 b = (meta_blob *) o->refine;

     	 meta_getstart (b, &p, w);	/* get startcube for ball */

    	  /* fill first cube with values */
	      meta_initstartcube (w, &cube, &p);

	      pos (p) = w->lastmark;	/* mark that cube is visited */

	      /* search for the first cube */

	      code = meta_searchcube (&cube, &p, w);


	      if (pos (p) == w->lastmark)	/* ball already calculated ? */
		{
		  o = o->next;
	 	 continue;		/* test next ball */
		}

     	 cube.pos = p;		/* mark position of the cube */

	      meta_addneighbors (&cube, w);	/* addneighbors cubes to stack */

		 while (w->stackpos > 0)
		{
		  /* get next cubepos */
		  w->stackpos--;

	 	 cube = w->stack[w->stackpos];


	  	if (w->currentnumpoly+150 >= (w->maxpoly))
	    	{

	     	 if (!
			  (w->vertex =
			   realloc (w->vertex,
				    sizeof (double) * 3 * 3 * (w->maxpoly + 10000 +
							       20))))
			return AY_EOMEM;

		      if (!
			  (w->nvertex =
			   realloc (w->nvertex,
			   	 sizeof (double) * 3 * 3 * (w->maxpoly + 10000 +
							       20))))
			return AY_EOMEM;

	     	 w->maxpoly += 10000;

		}
		  code = meta_polygonise (w, &cube, w->isolevel);

		  
		  pos (cube.pos) = w->lastmark;	/* mark that cube is visited */

		  if ((code != 0) || (code == 300))
		    meta_addneighbors (&cube, w);	/* add neighbors cubes to stack */

		}
	

	}
	

      o = o->next;

    }

  return META_OK;

}

void
meta_getnormal (meta_world * w, meta_xyz * p, meta_xyz * normal)
{
  double xn, yn, zn, old, scale;
  double f, d;

  d = (w->edgelength / 500);  /**w->scale;*/

  f = meta_calcall (p->x, p->y, p->z, w);

  

     xn = (meta_calcall(p->x+d, p->y, p->z,w) - meta_calcall(p->x-d, p->y, p->z,w))/(2*d);
     yn = (meta_calcall(p->x, p->y+d, p->z,w) - meta_calcall(p->x, p->y-d, p->z,w))/(2*d);
     zn = (meta_calcall(p->x, p->y, p->z+d,w) - meta_calcall(p->x, p->y, p->z-d,w))/(2*d);




/*
  xn = (meta_calcall (p->x + d, p->y, p->z, w) - f) / d;
  yn = (meta_calcall (p->x, p->y + d, p->z, w) - f) / d;
  zn = (meta_calcall (p->x, p->y, p->z + d, w) - f) / d;
*/
  old = sqrt (META_SQ (xn) + META_SQ (yn) + META_SQ (zn));

  if (old != 0.0)
    {
      scale = 1.0 / old;
      normal->x = xn * scale;
      normal->y = yn * scale;
      normal->z = zn * scale;
    }
  else
    {
      normal->x = xn;
      normal->y = yn;
      normal->z = zn;
    }

}








