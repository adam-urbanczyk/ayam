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


/* calculate the effect for all metaballs in list */
double
mt_calcall (double x1, double y1, double z1, world * w)
{
  double effect, dist, radius, tmpeffect, falloff;
  blob *tmp;
  ay_object *o;
  double x, y, z;

  effect = 0;
  dist = 0;

  o = w->o;

  while (o->next != NULL)
    {

	if(o->type == *w->cid)
	{

      	tmp = (blob *) o->refine;
      	radius = tmp->r * tmp->r;

      	/* rotate the Object */
	 	if (!(tmp->formula == mt_TORUS))
		{

	  	x =
	  	  tmp->rm[0] * x1 + tmp->rm[4] * y1 + tmp->rm[8] * z1 +
	  	  tmp->rm[12] * 1.0;
	  	y =
	  	  tmp->rm[1] * x1 + tmp->rm[5] * y1 + tmp->rm[9] * z1 +
	  	  tmp->rm[13] * 1.0;
	  	z =
	  	  tmp->rm[2] * x1 + tmp->rm[6] * y1 + tmp->rm[10] * z1 +
	  	  tmp->rm[14] * 1.0;

	  	dist = DIST (x, y, z, tmp->cp.x, tmp->cp.y, tmp->cp.z);
		}


     	 /* the normal metaball */
	 if (tmp->formula == mt_META)
	 {
	   if (dist <= radius)
	   {
		 tmpeffect = tmp->a * CUB (dist) / CUB (radius) +
		   tmp->b * SQ (dist) / SQ (radius) + tmp->c * dist / radius +
		    1.0;
	
		      if (tmp->negativ)
			{
			  effect -= tmpeffect;
			}
		      else
			effect += tmpeffect;
	    }

	}


/*
      if (tmp->formula == 1)
      {
	if (dist <= radius)
	{
	      falloff = 1.0f - (dist / radius);
	      tmpeffect = tmp->s * falloff * falloff;

		 if (tmp->negativ)
		{
		  effect -= tmpeffect;
		}
	      else
		effect += tmpeffect;

	    }
	}

      if (tmp->formula == 2)
	{

	  tmpeffect = radius / (dist < 0.00001 ? 0.00001 : dist);

	  if (tmp->negativ)
	    {
	      effect -= tmpeffect;
	    }
	  else
	    effect += tmpeffect;

	}
*/

		 if (tmp->formula == mt_CUBE)
		 {

		  {	 
		   tmpeffect = (tmp->scalex*pow(ABS(x-tmp->cp.x),tmp->ex) +  tmp->scaley*pow(ABS(y -tmp->cp.y),tmp->ey) \
		   +  tmp->scalez*pow(ABS(z - tmp->cp.z),tmp->ez))*9000;
		 
		   tmpeffect = 1/(tmpeffect < 0.00001 ? 0.00001 : tmpeffect);
	
		   if (tmp->negativ)
		   {
			  effect -= tmpeffect;
		   }
		      else
			effect += tmpeffect;
		  }
	
		}
	 

		 if (tmp->formula == mt_TORUS)
		{

		  x1 = (x1 - tmp->cp.x);
		  y1 = (y1 - tmp->cp.y);
		  z1 = (z1 - tmp->cp.z);

		  x =
		    (tmp->rm[0] * x1 + tmp->rm[4] * y1 + tmp->rm[8] * z1 +
		     tmp->rm[12] * 1.0) * tmp->scalex;
		  y =
		    (tmp->rm[1] * x1 + tmp->rm[5] * y1 + tmp->rm[9] * z1 +
		     tmp->rm[13] * 1.0) * tmp->scaley;
		  z =
		    (tmp->rm[2] * x1 + tmp->rm[6] * y1 + tmp->rm[10] * z1 +
		     tmp->rm[14] * 1.0) * tmp->scalez;

		  if (tmp->rot)
		    tmpeffect =
		      SQ (x * x + SQ (y) + z * z + tmp->Ro * tmp->Ro -
			  tmp->Ri * tmp->Ri) - 4.0 * SQ (tmp->Ro) * (SQ (z) + SQ (y));
		  else
		    tmpeffect =
		      SQ (x * x + SQ (y) + z * z + tmp->Ro * tmp->Ro -
			  tmp->Ri * tmp->Ri) - 4.0 * SQ (tmp->Ro) * (SQ (x) + SQ (y));


		  if (tmp->negativ)
		    {
		      effect -=
			1 / (tmpeffect < 0.00001 ? 0.00001 : tmpeffect) * 0.006;
		    }
		  else
		    effect += 1 / (tmpeffect < 0.00001 ? 0.00001 : tmpeffect) * 0.006;
		}

    	if (tmp->formula == mt_HEART)
	{
	  x = x1 - tmp->cp.x;
	  y = y1 - tmp->cp.y;
	  z = z1 - tmp->cp.z;


	  tmpeffect =
	    CUB (2 * SQ (x) + SQ (y) + SQ (z) - 1) - (0.1 * SQ (x) +
	      SQ (y)) * CUB (z);


	  if (tmp->negativ)
	  {
	      effect -=
		1 / (tmpeffect < 0.00001 ? 0.00001 : tmpeffect) * 0.002;
	  }
	  else
	       effect += 1 / (tmpeffect < 0.00001 ? 0.00001 : tmpeffect) * 0.002;
	}

	}
	      o = o->next;

    }

  return effect;

}


void
mt_getstart (blob * b, INTXYZ * p, world * w)
{

  p->x = (int) (b->cp.x / w->edgelength) + w->aktcubes / 2;

  if (b->formula == mt_TORUS)
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
mt_initcubestack (world * w)
{

  if ((w->stack = (GRIDCELL *) calloc (1, sizeof (GRIDCELL) * 2000)))
    {
      w->stackpos = 0;
      w->maxstack = 2000;
      return META_OK;
    }
  else
    return AY_EOMEM;

}

int
mt_freecubestack (world * w)
{
  if (w->stack)
    free (w->stack);

  return META_OK;
}

void
mt_pushcube (GRIDCELL * cube, world * w)
{
  if (w->stackpos == w->maxstack)
    {
      w->stack = realloc (w->stack, sizeof (GRIDCELL) * (w->maxstack + 1000));
      w->maxstack += 1000;
    }

  w->stack[w->stackpos] = *cube;
  w->stackpos++;

}

GRIDCELL
mt_popcube (world * w)
{

  w->stackpos--;
  return (w->stack[w->stackpos]);

}

void
mt_initstartcube (world * w, GRIDCELL * cube, INTXYZ * p)
{

#define length w->edgelength

  cube->p[0].x = p->x * length - w->unisize / 2;
  cube->p[0].y = p->y * length - w->unisize / 2;
  cube->p[0].z = p->z * length - w->unisize / 2;
  cube->val[0] = mt_calcall (cube->p[0].x, cube->p[0].y, cube->p[0].z, w);

  cube->p[1].x = cube->p[0].x + length;
  cube->p[1].y = cube->p[0].y;
  cube->p[1].z = cube->p[0].z;
  cube->val[1] = mt_calcall (cube->p[1].x, cube->p[1].y, cube->p[1].z, w);

  cube->p[2].x = cube->p[1].x;
  cube->p[2].y = cube->p[0].y;
  cube->p[2].z = cube->p[0].z + length;
  cube->val[2] = mt_calcall (cube->p[2].x, cube->p[2].y, cube->p[2].z, w);

  cube->p[3].x = cube->p[0].x;
  cube->p[3].y = cube->p[0].y;
  cube->p[3].z = cube->p[2].z;
  cube->val[3] = mt_calcall (cube->p[3].x, cube->p[3].y, cube->p[3].z, w);

  cube->p[4].x = cube->p[0].x;
  cube->p[4].y = cube->p[0].y + length;
  cube->p[4].z = cube->p[0].z;
  cube->val[4] = mt_calcall (cube->p[4].x, cube->p[4].y, cube->p[4].z, w);

  cube->p[5].x = cube->p[1].x;
  cube->p[5].y = cube->p[4].y;
  cube->p[5].z = cube->p[0].z;
  cube->val[5] = mt_calcall (cube->p[5].x, cube->p[5].y, cube->p[5].z, w);

  cube->p[6].x = cube->p[1].x;
  cube->p[6].y = cube->p[4].y;
  cube->p[6].z = cube->p[2].z;
  cube->val[6] = mt_calcall (cube->p[6].x, cube->p[6].y, cube->p[6].z, w);

  cube->p[7].x = cube->p[0].x;
  cube->p[7].y = cube->p[4].y;
  cube->p[7].z = cube->p[2].z;
  cube->val[7] = mt_calcall (cube->p[7].x, cube->p[7].y, cube->p[7].z, w);

#undef length
}


void
mt_addneighbors (GRIDCELL * cube, world * w)
{
  int edgecode;
  int square;
  int pos;
  GRIDCELL tmpcube;

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
	      mt_moveback (&tmpcube, w);
	      mt_pushcube (&tmpcube, w);
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
	      mt_moveright (&tmpcube, w);
	      mt_pushcube (&tmpcube, w);
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
	      mt_movefront (&tmpcube, w);
	      mt_pushcube (&tmpcube, w);
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
	      mt_moveleft (&tmpcube, w);
	      mt_pushcube (&tmpcube, w);
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
	      mt_moveup (&tmpcube, w);
	      mt_pushcube (&tmpcube, w);
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
	      mt_movedown (&tmpcube, w);
	      mt_pushcube (&tmpcube, w);
	      w->mgrid[pos] = w->lastmark;

	    }
	}


    }


#undef act
}


int
mt_searchcube (GRIDCELL * cube, INTXYZ * p, world * w)
{
  int code;
  INTXYZ pb;


  pb = *p;

  code = 0;

  /* search up */
  do
    {
      if (p->y < w->aktcubes - 1)
	{
	  p->y++;
	  mt_moveup (cube, w);	/* move one position up */

	  code = Polygonise (w, cube, w->isolevel, SHADE);
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
	      mt_movedown (cube, w);	/* move one position up */

	      code = Polygonise (w, cube, w->isolevel, SHADE);
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
	      mt_moveleft (cube, w);	/* move one position up */

	      code = Polygonise (w, cube, w->isolevel, SHADE);
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
	      mt_moveright (cube, w);	/* move one position up */

	      code = Polygonise (w, cube, w->isolevel, SHADE);
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
	      mt_movefront (cube, w);	/* move one position to the front */

	      code = Polygonise (w, cube, w->isolevel, SHADE);
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
	      mt_moveback (cube, w);	/* move one position to the front */

	      code = Polygonise (w, cube, w->isolevel, SHADE);
	    }
	  else
	    break;
	}
      while (code == 0);

    }

  return (code);

}


int
mt_calceffect (world * w, int mode)
{
  blob *b;
  INTXYZ p;
  int code, n;
  ay_object *o;
  GRIDCELL cube;

  n = 0;

  o = w->o;

  w->lastmark++;
  w->stackpos = 0;

  while (o->next != NULL)
    {
      n++;

	if(o->type == *w->cid)
	{
     	 b = (blob *) o->refine;

     	 mt_getstart (b, &p, w);	/* get startcube for ball */

    	  /* fill first cube with values */
	      mt_initstartcube (w, &cube, &p);

	      pos (p) = w->lastmark;	/* mark that cube is visited */

	      /* search for the first cube */

	      code = mt_searchcube (&cube, &p, w);


	      if (pos (p) == w->lastmark)	/* ball already calculated ? */
		{
		  o = o->next;
	 	 continue;		/* test next ball */
		}

     	 cube.pos = p;		/* mark position of the cube */

	      mt_addneighbors (&cube, w);	/* addneighbors cubes to stack */

		 while (w->stackpos > 0)
		{
		  /* get next cubepos */
		  w->stackpos--;

	 	 cube = w->stack[w->stackpos];


	  	if (w->currentnumpoly >= (w->maxpoly + 5))
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
		  code = Polygonise (w, &cube, w->isolevel, SHADE);

		  pos (cube.pos) = w->lastmark;	/* mark that cube is visited */

		  if ((code != 0) || (code == 300))
		    mt_addneighbors (&cube, w);	/* add neighbors cubes to stack */

		}
	

	}
	

      o = o->next;

    }

  return META_OK;

}

void
mt_getnormal (world * w, XYZ * p, XYZ * normal)
{
  double xn, yn, zn, old, scale;
  double f, d;

  d = w->edgelength / 100;

  f = mt_calcall (p->x, p->y, p->z, w);

  /*
     xn = ncalcall(p->x-0.01, p->y, p->z,w) - ncalcall(p->x+0.01, p->y, p->z,w);
     yn = ncalcall(p->x, p->y-0.01, p->z,w) - ncalcall(p->x, p->y+0.01, p->z,w);
     zn = ncalcall(p->x, p->y, p->z-0.01,w) - ncalcall(p->x, p->y, p->z+0.01,w);
   */

  xn = (mt_calcall (p->x + d, p->y, p->z, w) - f) / d;
  yn = (mt_calcall (p->x, p->y + d, p->z, w) - f) / d;
  zn = (mt_calcall (p->x, p->y, p->z + d, w) - f) / d;

  old = sqrt (SQ (xn) + SQ (yn) + SQ (zn));

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








