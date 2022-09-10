#ifdef GL_ES
precision mediump float;
#endif

uniform float time;
uniform vec2 resolution;

#define M_PI 3.14159265358979323846

void main() {
	vec2 pos = (gl_FragCoord.xy / resolution) + vec2(-0.5,0.05);
	float value = 10.f * length(pos) - time;
	
	float r = (sin(value) + 0.5) / 2.0;
	float g = (sin(value + 2. * M_PI / 3.) + 0.5) / 2.0;
	float b = (sin(value+ 4. * M_PI / 3.) + 0.5) / 2.0;

	gl_FragColor = vec4(r, g, b, 1.);
}
