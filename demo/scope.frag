#ifdef GL_ES
precision mediump float;
#endif

uniform float time;
uniform vec2 resolution;

void main() {
	
	vec2 npos = (gl_FragCoord.xy / resolution.y) + vec2(0,-0.5);
	float x = npos.x * 10. - 0.3;
	float y = 0.05 * (x*x+x*x*x*x)*exp(-x);
	y += 0.001 * (sin(time * 1013. + 3. * x));
	y += 0.0005 * (sin(-time * 450. + 2. * x));
	y += 0.01 * sin(time)*(cos(3.0*time) + sin(5.*time) + 0.5 * sin(0.1 * time)) * (sin(5.7*x)+sin(2.3*x)+sin(1.3*x))*x*exp(-x);
	y = y * (1. + 0.01 * (cos(7.*time) + 0.5 * sin(40.*time)) * (sin(4.3*x)+sin(3.5*x)) );
	
	float z = exp(-20. * abs(npos.y - y) - 1. * abs(npos.y));
	
	gl_FragColor = vec4(z*z*z, z, 0., 1.);
}
