/*-______________________________________________________________________
**
** displacenoise shader... dsward@webnation.com
** VIDI Presenter 3D Repository - http://www.webnation.com
**
** offsetX, offsetY, offsetZ = offset in the noise lattice
** scaleX, scaleY, scaleZ = scale in the noise lattice
** scaleRed, scaleGreen, scaleBlue = displacement scale channels
** Km = displacement maximum
**
** To match Km, set scaleRed + scaleGreen + scaleBlue = 3
** ______________________________________________________________________
*/
displacement
DSnoisedsp (float noiseType = 3,
                    offsetX = 0.0,
                    offsetY = 0.0,
                    offsetZ = 0.0,
                    scaleX = 1.0,
                    scaleY = 1.0,
                    scaleZ = 1.0,
                    scaleRed = 1.0,
                    scaleGreen = 1.0,
                    scaleBlue = 1.0,
                    Km = 1.0; )
{
   point noiseAt;
   color noisy = 0;
   float amplitude = 0;

   P = transform("object", P);
   N = transform("object", N + point "object" (0,0,0));

   if (noiseType <= 0) noiseAt = (t, 0, 0);
   if (noiseType == 1) noiseAt = (s, 0, 0);
   if (noiseType == 2) noiseAt = (s, t, 0);
   if (noiseType >= 3) noiseAt = P;

   setxcomp(noiseAt, (xcomp(noiseAt) + offsetX) * scaleX);
   setycomp(noiseAt, (ycomp(noiseAt) + offsetY) * scaleY);
   setzcomp(noiseAt, (zcomp(noiseAt) + offsetZ) * scaleZ);

   noisy = color noise (noiseAt);

   amplitude = comp(noisy, 0) * scaleRed;
   amplitude += comp(noisy, 1) * scaleGreen;
   amplitude += comp(noisy, 2) * scaleBlue;
   if (amplitude != 0.0)
   {
      amplitude = amplitude / 3.0;
   }
   amplitude *= Km;

   P += normalize(N) * amplitude;

   P = transform("object", "current", P);
   N = calculatenormal(P);
}
