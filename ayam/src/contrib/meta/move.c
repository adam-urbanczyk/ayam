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
** move.c:
**  routines to move a cube
**
*/

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "meta.h"
#include "ayam.h"

/* move the Cube one unit up */
void
meta_moveup (meta_gridcell * cube, meta_world * w)
{
#define length w->edgelength

  cube->p[0] = cube->p[4];
  cube->p[1] = cube->p[5];
  cube->p[2] = cube->p[6];
  cube->p[3] = cube->p[7];

  cube->val[0] = cube->val[4];
  cube->val[1] = cube->val[5];
  cube->val[2] = cube->val[6];
  cube->val[3] = cube->val[7];

  cube->p[4].y += length;
  cube->p[5].y = cube->p[4].y;
  cube->p[6].y = cube->p[4].y;
  cube->p[7].y = cube->p[4].y;

  cube->val[4] = meta_calcall (cube->p[4].x, cube->p[4].y, cube->p[4].z, w);
  cube->val[5] = meta_calcall (cube->p[5].x, cube->p[5].y, cube->p[5].z, w);
  cube->val[6] = meta_calcall (cube->p[6].x, cube->p[6].y, cube->p[6].z, w);
  cube->val[7] = meta_calcall (cube->p[7].x, cube->p[7].y, cube->p[7].z, w);

  cube->pos.y++;

#undef length
}

/* move the Cube one unit down */
void
meta_movedown (meta_gridcell * cube, meta_world * w)
{
#define length w->edgelength

  cube->p[4] = cube->p[0];
  cube->p[5] = cube->p[1];
  cube->p[6] = cube->p[2];
  cube->p[7] = cube->p[3];

  cube->val[4] = cube->val[0];
  cube->val[5] = cube->val[1];
  cube->val[6] = cube->val[2];
  cube->val[7] = cube->val[3];

  cube->p[0].y -= length;
  cube->p[1].y = cube->p[0].y;
  cube->p[2].y = cube->p[0].y;
  cube->p[3].y = cube->p[0].y;

  cube->val[0] = meta_calcall (cube->p[0].x, cube->p[0].y, cube->p[0].z, w);
  cube->val[1] = meta_calcall (cube->p[1].x, cube->p[1].y, cube->p[1].z, w);
  cube->val[2] = meta_calcall (cube->p[2].x, cube->p[2].y, cube->p[2].z, w);
  cube->val[3] = meta_calcall (cube->p[3].x, cube->p[3].y, cube->p[3].z, w);

  cube->pos.y--;

#undef length
}

/* move the Cube one unit left */
void
meta_moveleft (meta_gridcell * cube, meta_world * w)
{
#define length w->edgelength

  cube->p[1] = cube->p[0];
  cube->p[2] = cube->p[3];
  cube->p[5] = cube->p[4];
  cube->p[6] = cube->p[7];

  cube->val[1] = cube->val[0];
  cube->val[2] = cube->val[3];
  cube->val[5] = cube->val[4];
  cube->val[6] = cube->val[7];

  cube->p[0].x -= length;
  cube->p[3].x = cube->p[0].x;
  cube->p[4].x = cube->p[0].x;
  cube->p[7].x = cube->p[0].x;

  cube->val[0] = meta_calcall (cube->p[0].x, cube->p[0].y, cube->p[0].z, w);
  cube->val[3] = meta_calcall (cube->p[3].x, cube->p[3].y, cube->p[3].z, w);
  cube->val[4] = meta_calcall (cube->p[4].x, cube->p[4].y, cube->p[4].z, w);
  cube->val[7] = meta_calcall (cube->p[7].x, cube->p[7].y, cube->p[7].z, w);

  cube->pos.x--;

#undef length

}

/* move the Cube one unit right */
void
meta_moveright (meta_gridcell * cube, meta_world * w)
{
#define length w->edgelength

  cube->p[0] = cube->p[1];
  cube->p[3] = cube->p[2];
  cube->p[4] = cube->p[5];
  cube->p[7] = cube->p[6];

  cube->val[0] = cube->val[1];
  cube->val[3] = cube->val[2];
  cube->val[4] = cube->val[5];
  cube->val[7] = cube->val[6];

  cube->p[1].x += length;
  cube->p[2].x = cube->p[1].x;
  cube->p[5].x = cube->p[1].x;
  cube->p[6].x = cube->p[1].x;

  cube->val[1] = meta_calcall (cube->p[1].x, cube->p[1].y, cube->p[1].z, w);
  cube->val[2] = meta_calcall (cube->p[2].x, cube->p[2].y, cube->p[2].z, w);
  cube->val[5] = meta_calcall (cube->p[5].x, cube->p[5].y, cube->p[5].z, w);
  cube->val[6] = meta_calcall (cube->p[6].x, cube->p[6].y, cube->p[6].z, w);

  cube->pos.x++;

#undef length

}


/* move the Cube one unit to front */
void
meta_movefront (meta_gridcell * cube, meta_world * w)
{
#define length w->edgelength

  cube->p[0] = cube->p[3];
  cube->p[1] = cube->p[2];
  cube->p[4] = cube->p[7];
  cube->p[5] = cube->p[6];

  cube->val[0] = cube->val[3];
  cube->val[1] = cube->val[2];
  cube->val[4] = cube->val[7];
  cube->val[5] = cube->val[6];

  cube->p[3].z += length;
  cube->p[2].z = cube->p[3].z;
  cube->p[7].z = cube->p[3].z;
  cube->p[6].z = cube->p[3].z;

  cube->val[3] = meta_calcall (cube->p[3].x, cube->p[3].y, cube->p[3].z, w);
  cube->val[2] = meta_calcall (cube->p[2].x, cube->p[2].y, cube->p[2].z, w);
  cube->val[7] = meta_calcall (cube->p[7].x, cube->p[7].y, cube->p[7].z, w);
  cube->val[6] = meta_calcall (cube->p[6].x, cube->p[6].y, cube->p[6].z, w);

  cube->pos.z++;

#undef length

}

/* move the Cube one unit back */
void
meta_moveback (meta_gridcell * cube, meta_world * w)
{
#define length w->edgelength

  cube->p[3] = cube->p[0];
  cube->p[2] = cube->p[1];
  cube->p[7] = cube->p[4];
  cube->p[6] = cube->p[5];

  cube->val[3] = cube->val[0];
  cube->val[2] = cube->val[1];
  cube->val[7] = cube->val[4];
  cube->val[6] = cube->val[5];

  cube->p[0].z -= length;
  cube->p[1].z = cube->p[0].z;
  cube->p[4].z = cube->p[0].z;
  cube->p[5].z = cube->p[0].z;

  cube->val[0] = meta_calcall (cube->p[0].x, cube->p[0].y, cube->p[0].z, w);
  cube->val[1] = meta_calcall (cube->p[1].x, cube->p[1].y, cube->p[1].z, w);
  cube->val[4] = meta_calcall (cube->p[4].x, cube->p[4].y, cube->p[4].z, w);
  cube->val[5] = meta_calcall (cube->p[5].x, cube->p[5].y, cube->p[5].z, w);

  cube->pos.z--;

#undef length

}
