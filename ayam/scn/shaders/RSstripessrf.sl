/* 
 * stripes surface shader
 *
 * Author: Randolf Schultz
 * For: Ice, Mops/Ayam example scene
 * Arguments:
 * firstcol,seccol: colors of the stripes  
 * Nu: Number of stripes 
 *
 */

surface
RSstripessrf( float Ka=1, Kd=1;
	color firstcol = color (1,0,0), seccol=color (1,1,0);
	float Nu = 10.0)
{
  color mycolor;
  float select;

  select = cos(v*PI*Nu);

  if(select > 0.0)
    mycolor = firstcol;
  else
    mycolor = seccol;

  Ci = Cs *mycolor* ( Ka*ambient() + Kd*diffuse(faceforward(normalize(N),I)) );
}
