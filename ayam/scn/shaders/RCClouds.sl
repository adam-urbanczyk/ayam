/* Listing 13.1  An example shader */

/* clouds(): a surface shader for a cloudy surface */
surface 
RCClouds( 
      float    Kd=.8, 
               Ka=.2 )
{
    float sum ;
    float i, freq;
    color white = color(1.0, 1.0, 1.0);
    
    sum = 0; 
    freq = 4.0;
    for (i = 0; i < 6; i = i + 1) {
        sum = sum + 1/freq * abs(.5 - noise( freq * P)) ;
        freq = 2 * freq;
    }
    Ci = mix(Cs, white, sum*4.0);
    Oi = 1.0;            /* Always make the surface opaque */
}

