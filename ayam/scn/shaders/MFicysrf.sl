
/* 
 * Icy surface shader
 *
 * Mark Fontana
 * Digital Lighting, SP 96
 *
 * Modified by Randolf Schultz
 * Split the icy.sl into a surface and a displacement
 * shader, because BMRT does no displacements in surface
 * shaders...
 */


#include "./rmannotes.sl"


surface
MFicysrf(float icicle_length = 0.1, icicle_freq = 6,
	 Ks = 0.9, Kd = 0.7, Ka = 0.3, roughness= 0.9)
{
  point NN, Nf;
  point V = -normalize(I);
  float magnitude, dist, dx, dy, dz;
  point PP, P_orig;
  float width, cutoff, fade, turb1, turb2, maxfreq = 16, f;
  uniform point down = (0,-1,0);


  Oi = 0.3;
  Nf = faceforward(normalize(N), I);
  Cs = color(0.85,0.9,1);
  Ci = Cs * (Ka * ambient() + (Kd * diffuse(normalize(N)))) +
       color(1,1,1) * Ks * specular(Nf, V, roughness);
}


