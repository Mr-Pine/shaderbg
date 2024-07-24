// vi: ft=glsl
// Based on https://www.shadertoy.com/view/Wscyzr

void mainImage( out vec4 O, vec2 U )
{
    if (U.x == .5 && U.y < 3) {
        O = vec4(0);
        return;
    }
	O = vec4(.93, .36, .36, 1) * T(U).x; // attention, T(U) goes up to 5 and more
}
