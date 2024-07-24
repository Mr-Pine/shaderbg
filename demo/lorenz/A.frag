// vi: ft=glsl
// Based of https://www.shadertoy.com/view/Wscyzr

//https://en.wikipedia.org/wiki/Lorenz_system

#define MODE xz                       // choose which 2D projection to draw

float STEPS = 96.,                    // graphics settings
 VIEW_SCALE = .015,
      SPEED = .05,
  INTENSITY = .2,
       FADE = .99,
      FOCUS = 1.,
     O = 10., P = 28., B = 8./3.;     // System Parameters

vec3 start = vec3( .1, .001, 0 );     // Initial Position

vec3 Integrate(vec3 cur, float dt)    // Calculate the next position 
{
	return cur + dt * vec3(    O  * (cur.y - cur.x)       ,
                            cur.x * (P - cur.z) - cur.y   ,
                            cur.x *   cur.y     - B*cur.z   );
}

float Line(vec2 a, vec2 b, vec2 U)    // Distance to a line segment https://www.shadertoy.com/view/llySRh
{   
    U -= a, b -= a;
	float h = dot( U, b ) / dot(b,b),
          c = clamp(h, 0., 1.);
  //  return h==c ? length( U - b * c ) : 1e5;   // dist to strict segment
  return        length( U - b * c );         // dist to segment with round ends
}

void mainImage( out vec4 O, vec2 u )
{
    u = floor(u);
    if (iFrame == 0) {
        if (u == vec2(0, 1)) {
            O = vec4(start, 0);
        } else if (u == vec2(0,2)) {
            O = vec4(0,0,0,0);
        } else {
            O = vec4(0, 0, 0, 1);
        }

        return;
    }

    float pix = 1. / iResolution.y / VIEW_SCALE;
    if (u == vec2(0)) {
        O = T(ivec2(0,1));
    } else if (u == vec2(0,1)) {
        vec3 last = T(ivec2(0,1)).rgb;             // pixel (0,0) saves the current position.
        vec3 next;

        for(float i = 0.; i < STEPS; i++ ) {
            next = Integrate( last, .016 * SPEED );
            last = next;
        }

        O.xyz = next;
    } else if (u == vec2(0, 2)) {
        vec3 last = T(ivec2(0,1)).rgb;             // pixel (0,0) saves the current position.
        vec3 next;

        vec2 Min = last.MODE;
        vec2 Max = last.MODE;

        for(float i = 0.; i < STEPS; i++ ) {
            next = Integrate( last, .016 * SPEED );

            Min = min(Min, next.MODE);
            Max = max(Max, next.MODE);

            last = next;
        }

        O.xy = Min - pix;
        O.zw = Max + pix;
    } else {
        vec2 R = iResolution.xy;
        vec2 U = ( u - .5*R ) / R.y;
        U.y += .375;

        U /= VIEW_SCALE;

        vec4 bb = T(ivec2(0, 2));
        vec2 Min = bb.xy;
        vec2 Max = bb.zw;

        O.xyz = vec3(0);

        bool inBounds = U.x > Min.x && U.y > Min.y && U.x < Max.x && U.y < Max.y;
        if (inBounds) {
            vec3 last = T(0).rgb;             // pixel (0,0) saves the current position.

            if (length(U - last.MODE) > .8 * pix) {
                float d = 1e6;

                for(float i = 0.; i < STEPS; i++ )
                {
                    vec3 next = Integrate( last, .016 * SPEED );
                    d = min(d, Line(last.MODE , next.MODE, U) );
                    last = next;
                }
                // d = Line(T(0).MODE, next.MODE, U );
                d *= VIEW_SCALE * .7;
                float c =  INTENSITY / SPEED * smoothstep( FOCUS / R.y, 0., d )
                         + INTENSITY / 8.5   * exp(- 1e3 * d*d );

                O.xyz = vec3(c);
            }
        }

        float maxValue = INTENSITY / SPEED + INTENSITY / 8.5;

        O.xyz += vec3(T(u) * FADE);
        O.xyz = min(O.xyz, vec3(maxValue));
        O.w = 1;
    }


}
