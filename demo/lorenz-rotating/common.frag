// vi: ft=glsl
// Based on https://www.shadertoy.com/view/Ws3cz8

                                      // --- graphics settings
int            N = 400;               // number of segments
float VIEW_SCALE = .015,              // scene scaling
        Y_OFFSET = .4,                // scene offset
           STEPS = 96.,               // number of iteration per frame
           SPEED = .02,               // dt = SPEED/60
       INTENSITY = .2,
        //  FADE = .99,
            FADE = .99,
           FOCUS = 1.,

  O = 10., P = 28., B = 8./3.;        // --- System Parameters

//vec4 COL = vec4(.93, .36, .36, 1);
//const vec4 COL = pow( vec4(.93, .36, .36, 1), vec4(2.2) );
  vec4 COL = vec4(1, .2, .2, 1) *2.;  // equivalent look with gamma

#define T(U)  texelFetch( iBufferA, ivec2(U), 0 )


//https://en.wikipedia.org/wiki/Lorenz_system

vec3 Integrate(vec3 cur, float dt)    // --- Calculate the next position 
{
	return cur + dt * vec3(    O  * (cur.y - cur.x)       ,
                            cur.x * (P - cur.z) - cur.y   ,
                            cur.x *   cur.y     - B*cur.z   );
}
