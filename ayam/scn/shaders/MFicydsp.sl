
/* 
 * Icy displacement shader
 *
 * Mark Fontana
 * Digital Lighting, SP 96
 *
 * Modified by Randolf Schultz
 * Split the icy.sl into a surface and a displacement
 * shader, because BMRT does no displacements in surface
 * shaders...
 * Changed the displacement to occur only if the angle
 * between N and the down vector is smaller than 90
 * to ensure the upper parts of a primitive don't get displaced
 *
 */

#include "./rmannotes.sl"


displacement
MFicydsp(float icicle_length = 0.1, icicle_freq = 6)
{
  point NN, Nf;
  point V = -normalize(I);
  float magnitude, dist, dx, dy, dz;
  point PP, P_orig;
  float width, cutoff, fade, turb1, turb2, maxfreq = 16, f;
  uniform point down = (0,-1,0);


  if((normalize(N).down)<90)
  {
   PP = transform("shader", P) * icicle_freq;

   width = filterwidth_point(PP);
   cutoff = clamp(0.5 / width, 0, maxfreq);

   turb1 = 0;
   for (f = 1; f < 0.5 * cutoff; f *= 2) 
     turb1 += abs(snoise(PP * f)) / f;

   fade = clamp(2 * (cutoff - f) / cutoff, 0, 1);
   turb1 += fade * abs(snoise(PP * f)) / f;

   turb2 = 0;
   for (f = 1; f < 0.5 * cutoff; f *= 2) 
     turb2 += abs(snoise(N.PP * f)) / f;
   fade = clamp(2 * (cutoff - f) / cutoff, 0, 1);
   turb2 += fade * abs(snoise(PP * f)) / f;

   magnitude = pow(turb1, 4);
   NN = normalize(N);

   P_orig = P;
   if (ycomp(NN) > -0.3)
      P += (1-ycomp(NN)) * icicle_length * magnitude * normalize(Ng);     
   else
      P += icicle_length * magnitude * normalize(Ng);
     
   P += down*turb1*turb2*3;

   N = calculatenormal(P);
  }
}


