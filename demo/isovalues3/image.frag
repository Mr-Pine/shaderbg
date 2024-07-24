// vi: ft=glsl

void mainImage( out vec4 O,  vec2 U )
{
	O = texture( iBufferA, U / iResolution.xy);
}
