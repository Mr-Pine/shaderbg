// vi: ft=glsl
// Based of https://www.shadertoy.com/view/Ws3cz8

 // === simulate trajectory segments

void mainImage( out vec4 O, vec2 u )
{
    u = floor(u);

    int y = int(u.y);                        // segment #y data in line y
    if (y>N || u.x>2. ) {
        // No valid segment there
        O = vec4(0,0,0,0);
        return;
    }
    //if (y>0) { O = T(u-vec2(0,1)); return; } // cascade-save segment states   
    if (y>0) { O = T(u-vec2(0,1)); O.w = (u - vec2(0,1)).y; return; } // cascade-save segment states   
                                             // --- compute new segment 0
    vec3  last = T(0).xyz,                   // pixel (0,0) saves the current position.
          next = last,
         start = vec3( .1, .001, 0 ),        // Initial Position
         Max = last, Min = last;

    for(float i = 0.; i < STEPS; i++ )       // simulates for 1 frame range
        next = Integrate( next, .016 * SPEED ),
        Min  = min(Min, next),
        Max  = max(Max, next);

    O =   u == vec2(0,y)                     // pixel (0,y) saves the current position.
        ? iFrame < 1     ? vec4( start, 0 )  // Setup initial conditions.
                         : vec4( next , 0 )  // Save current position.
        : u == vec2(1,y) ? vec4( Min  , 0 )  // pixel (1-2,y) saves the segment bbox 
                         : vec4( Max  , 0 );
} 
