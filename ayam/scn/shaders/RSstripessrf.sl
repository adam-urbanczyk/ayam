/* 
 * stripes surface shader
 *
 * Author: Randolf Schultz
 * For: Ice, Mops/Ayam example scene
 * Arguments:
 * firstcol,seccol: colors of the stripes  
 * Nu: Number of stripes 
 *
 * <shader type="surface" name="RSstripessrf">
 * <argument name="Ka" type="float" value="1">
 * <argument name="Kd" type="float" value="1">
 * <argument name="firstcol" type="color" value="1 0 0">
 * <argument name="seccol" type="color" value="1 1 0">
 * <argument name="Nu" type="float" value="10.0">
 */

surface
RSstripessrf( float Ka = 1, Kd = 1;
	color firstcol = color (1,0,0), seccol = color (1,1,0);
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
