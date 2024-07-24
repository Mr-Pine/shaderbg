// vi: ft=glsl
// Based of https://www.shadertoy.com/view/Ws3cz8

// 3D variant of https://shadertoy.com/view/3scyz8
// preparing 3D variant of https://shadertoy.com/view/3dtcRr
// multi-trajectory variant of https://shadertoy.com/view/3ddyRr
// optimization of https://shadertoy.com/view/Wscyzr
// simplification + comments of "lorenz11" by mrtkp9993. https://shadertoy.com/view/3tBfWd

#define rot(a)          mat2( cos( a + vec4(0,33,11,0)) )

float Line(vec2 a, vec2 b, vec2 U) // --- Distance to a line segment https://www.shadertoy.com/view/llySRh
{   
    U -= a, b -= a;
	float h = dot( U, b ) / dot(b,b),
          c = clamp(h, 0., 1.);
    return h==c ? length( U - b * c ) : 1e5;     // dist to strict segment
  //return        length( U - b * c );           // dist to segment with round ends
}

#define proj(v) ( V=v.xyz - C, V.yz *= rot(_t), V.xz *= rot(.3*t), (C+V).xy ) // rotate camera

void mainImage( out vec4 O, vec2 u ) // --- re-simulate and draw curve segments
{  
    vec3 C = vec3(0,0,Y_OFFSET) / VIEW_SCALE;    // rotation center
    vec2 R = iResolution.xy, Min, Max, p,
           U = ( u - .5*R ) / R.y; //U.y += Y_OFFSET;
      U /= VIEW_SCALE;
    float pix = 1. / R.y / VIEW_SCALE;
    float t = iTime, _t = 1.+.5*sin(.2*(t+5.));  // for camera rotation

    vec3 last, next, V;
    O = vec4(0);

    for (int y = 0; y < min(N,int(R.y)); y++) {  // --- drawing segments;
        mat2x3 M = mat2x3( T(vec2(1,y)).xyz,
                           T(vec2(2,y)).xyz );
        Min = vec2(1e5), Max = -Min;
        for (int i=0; i<8; i++)                  // get the 2D bounding box of the 3D BB
            p = proj( vec3( M[i%2][0], M[i/2%2][1], M[i/4][2] ) ),
            Min = min(Min,p), 
            Max = max(Max,p);
        Min -= pix; Max += pix;

        if (U.x>Min.x && U.y>Min.y && U.x<Max.x && U.y<Max.y) { // ---
            // segment potentially visible: re- simulate and detail-draw it                  
            last = T(vec2(0,y+1)).xyz;           // pixel (0,y,+1) saves segment start position.
            float d = 1e6, l, z;

            for(float i = 0.; i < STEPS; i++ ) { // re-simulates & draws for 1 frame range
                next = Integrate( last, .016 * SPEED );
                vec2 B = proj(last), E = proj(next);
                l = Line( B , E, U);
                if ( l < d ) d = l, z = (C+V).z; // portion of curve closest to pixel       
                last = next;
            }
            d *=  VIEW_SCALE;
            float c  =  INTENSITY / SPEED * smoothstep( FOCUS / R.y, 0., d )
                      ;// + INTENSITY / 8.5   * exp(- 1e3 * d*d );

            O +=   24.* exp(- .15 * z ) *                       // uncomment for fog effect
                 // 4.* exp(- vec4(.1,.2,.4,0) * ( z - 3. ) ) * // uncomment for col fog effect
                    c* COL * pow(FADE,float(y));                // fade with segment age 
       }
     }
    O = pow(O, vec4(1./2.2));                    // to sRGB
 }
