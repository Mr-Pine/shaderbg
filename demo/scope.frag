void mainImage( out vec4 fragColor, in vec2 fragCoord )
{	
	vec2 npos = (fragCoord.xy / iResolution.y) + vec2(0,-0.5);
	float x = npos.x * 10. - 0.3;
	float y = 0.05 * (x*x+x*x*x*x)*exp(-x);
	y += 0.001 * (sin(iTime * 1013. + 3. * x));
	y += 0.0005 * (sin(-iTime * 450. + 2. * x));
	y += 0.01 * sin(iTime)*(cos(3.0*iTime) + sin(5.*iTime) + 0.5 * sin(0.1 * iTime)) * (sin(5.7*x)+sin(2.3*x)+sin(1.3*x))*x*exp(-x);
	y = y * (1. + 0.01 * (cos(7.*iTime) + 0.5 * sin(40.*iTime)) * (sin(4.3*x)+sin(3.5*x)) );
	
	float z = exp(-20. * abs(npos.y - y) - 1. * abs(npos.y));
	
	fragColor = vec4(z*z*z, z, 0., 1.);
}

// vi: ft=glsl
