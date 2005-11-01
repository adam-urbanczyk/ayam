/*
 * Ayam, a free 3D modeler for the RenderMan interface.
 *
 * Ayam is copyrighted 1998-2005 by Randolf Schultz
 * (rschultz@informatik.uni-rostock.de) and others.
 *
 * All rights reserved.
 *
 * See the file License for details.
 *
 */

#include "ayam.h"

/* quat.c - functions for quaternions */

/*
 * Quaternion code adapted from various articles
 * from news:c.g.a.opengl, from GLUT example code (trackball.c),
 * from the Irrlicht Engine (quaternion.h), and from the
 * news:c.g.algorithms FAQ Wiki (http://cgafaq.info/wiki/Quaternion)
 */

#define AY_QEPSILON (1.0e-05)

/* ay_quat_axistoquat:
 *  This function computes a quaternion based on an axis (defined by
 *  the given vector) <a> and an angle <phi> in radians about which to rotate.
 *  The result is put into the third argument <q>.
 */
void
ay_quat_axistoquat(double a[3], double phi, double q[4])
{
 double len = 0.0;

  len = AY_V3LEN(a);
  a[0] *= 1.0/len;
  a[1] *= 1.0/len;
  a[2] *= 1.0/len;

  q[0]=a[0];
  q[1]=a[1];
  q[2]=a[2];
  AY_V3SCAL(q, sin(phi/2.0))

  q[3] = cos(phi/2.0);

 return;
} /* ay_quat_axistoquat */


/* ay_quat_norm:
 *  Normalize the quaternion <q>, if needed.
 *  Quaternions always obey:  a^2 + b^2 + c^2 + d^2 = 1.0
 *  If they don't add up to 1.0, dividing by their magnitude will
 *  renormalize them.
 */
void
ay_quat_norm(double q[4])
{
 int i;
 double norm;

  norm = (q[0]*q[0] + q[1]*q[1] + q[2]*q[2] + q[3]*q[3]);

  if((norm > (1.0 + AY_EPSILON)) || (norm < (1.0 - AY_EPSILON)))
    {
      for(i = 0; i < 4; i++)
	{
	  // divide by magnitude (sqrt(norm))
	  q[i] /= sqrt(norm);
	}
    }

 return;
} /* ay_quat_norm */


/* ay_quat_add:
 *  Given two rotations, <q1> and <q2>, expressed as quaternion rotations,
 *  figure out the equivalent single rotation and stuff it into <dest>.
 *
 *  This routine also normalizes the result to keep error from creeping in.
 *
 *  NOTE: This routine is written so that <q1> or <q2> may be the same
 *  as <dest> (or each other).
 */
void
ay_quat_add(double q1[4], double q2[4], double dest[4])
{
 static int count = 0;
 double t1[4], t2[4], t3[4];
 double tf[4];

  t1[0] = q1[0];
  t1[1] = q1[1];
  t1[2] = q1[2];
  AY_V3SCAL(t1, q2[3]);

  t2[0] = q2[0];
  t2[1] = q2[1];
  t2[2] = q2[2];
  AY_V3SCAL(t2, q1[3]);

  AY_V3CROSS(t3, q2, q1);
  AY_V3ADD(tf, t1, t2);
  AY_V3ADD(tf, t3, tf);

  tf[3] = q1[3] * q2[3] - (AY_V3DOT(q1, q2));

  dest[0] = tf[0];
  dest[1] = tf[1];
  dest[2] = tf[2];
  dest[3] = tf[3];

  ay_quat_norm(dest);

 return;
} /* ay_quat_add */


/* ay_quat_torotmatrix:
 *  Build a rotation matrix m, from a quaternion rotation q.
 */
void
ay_quat_torotmatrix(double q[4], double m[16])
{
 
  m[0] = 1.0 - 2.0 * (q[1] * q[1] + q[2] * q[2]);
  m[1] = 2.0 * (q[0] * q[1] - q[2] * q[3]);
  m[2] = 2.0 * (q[2] * q[0] + q[1] * q[3]);
  m[3] = 0.0;

  m[4] = 2.0 * (q[0] * q[1] + q[2] * q[3]);
  m[5]= 1.0 - 2.0 * (q[2] * q[2] + q[0] * q[0]);
  m[6] = 2.0 * (q[1] * q[2] - q[0] * q[3]);
  m[7] = 0.0;

  m[8] = 2.0 * (q[2] * q[0] - q[1] * q[3]);
  m[9] = 2.0 * (q[1] * q[2] + q[0] * q[3]);
  m[10] = 1.0 - 2.0 * (q[1] * q[1] + q[0] * q[0]);
  m[11] = 0.0;

  m[12] = 0.0;
  m[13] = 0.0;
  m[14] = 0.0;
  m[15] = 1.0;

 return;
} /* ay_quat_torotmatrix */


/* ay_quat_toeuler:
 *  Convert the quaternion q to euler[psi, theta, phi].
 *  Angles are defined in radians.
 */
void
ay_quat_toeuler(double q[4], double euler[3])
{
 double sin_theta;      /* sin of pitch */
 double R00, R01, R02;  /* rotation matrix elements */
 double R11, R12;
 double R21, R22;

  R02 = 2.0*(q[0]*q[2] - q[3]*q[1]);

  sin_theta = -R02;

  if(fabs(sin_theta) > (1.0 - AY_QEPSILON))
    {
      /* very close to +- 90 degrees pitch, set heading to zero
         and compute the roll angle */
      euler[0] = 0.0;

      if(sin_theta > 0)
        euler[1] = AY_PI/2.0;
      else
        euler[1] = -AY_PI/2.0;

      R11 = 2.0*(q[3]*q[3] + q[1]*q[1]) - 1.0;
      R21 = 2.0*(q[1]*q[2] - q[3]*q[0]);

      euler[2] = atan2(-R21, R11);
    }
  else
    {
      R00 = 2.0*(q[3]*q[3] + q[0]*q[0]) - 1.0;
      R01 = 2.0*(q[0]*q[1] + q[3]*q[2]);
      R12 = 2.0*(q[1]*q[2] + q[3]*q[0]);
      R22 = 2.0*(q[3]*q[3] + q[2]*q[2]) - 1.0;

      /* heading */
      euler[0] = atan2(R01, R00);

      /* pitch */
      euler[1] = asin(sin_theta);

      /* roll */
      euler[2] = atan2(R12, R22);
    } /* if */

 return;
} /* ay_quat_toeuler */


/* ay_quat_slerp:
 *
 */
int
ay_quat_slerp(double time, double q1[4], double q2[4], double *r)
{
 double angle = 0.0, scale = 0.0, invscale = 0.0;
 double theta, invsintheta;

  if(!r)
    return AY_ENULL;

  angle =  ay_quat_dot(q1, q2);
 
  if(angle < 0.0f) 
    {
      q1[0] *= -1.0f;
      q1[1] *= -1.0f;
      q1[2] *= -1.0f;
      q1[3] *= -1.0f;
      angle *= -1.0f;
    }
 
  if((angle + 1.0f) > 0.05f) 
    {
      if((1.0f - angle) >= 0.05f)
	{
	  /* spherical interpolation */
	  theta = acos(angle);
	  invsintheta = 1.0f / sin(theta);
	  scale = sin(theta * (1.0f-time)) * invsintheta;
	  invscale = sin(theta * time) * invsintheta;
	}
      else
	{
	  /* linear interploation */
	  scale = 1.0f - time;
	  invscale = time;
	}
    }
  else 
    {
      /*q2 = quaternion(-q1.Y, q1.X, -q1.W, q1.Z);*/
      q2[0] = -q1[1];
      q2[1] = q1[0];
      q2[2] = -q1[3];
      q2[3] = q1[2];
      scale = sin(AY_PI * (0.5f - time));
      invscale = sin(AY_PI * time);
    } /* if */
 
  /**this = (q1*scale) + (q2*invscale);*/
  r[0] = (q1[0]*scale) + (q2[0]*invscale);
  r[1] = (q1[1]*scale) + (q2[1]*invscale);
  r[2] = (q1[2]*scale) + (q2[2]*invscale);
  r[3] = (q1[3]*scale) + (q2[3]*invscale);

 return AY_OK;
} /* ay_quat_slerp */


/* ay_quat_dot:
 *  calculate dot product of two quaternions <q1> and <q2> (q1.q2)
 *  returns result
 */
double
ay_quat_dot(double q1[4], double q2[4])
{
  return((q1[0] * q2[0]) + (q1[1] * q2[1]) +
    (q1[2] * q2[2]) + (q1[3] * q2[3]));
} /* ay_quat_dot */

#undef AY_QEPSILON
