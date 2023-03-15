
#define M_PI 3.14159265358979323846

void mainImage( out vec4 fragColor, in vec2 fragCoord )
{	
	vec2 pos = (fragCoord.xy / iResolution.xy) + vec2(-0.5,-0.5);
	float shifted_rad = 10.f * length(pos) - iTime;
	float angle = atan(pos.y, pos.x);
	float value = 5. * (shifted_rad + angle);
	
	float r = (sin(value) + 0.5) / 2.0;
	float b = (sin(value + 4. * M_PI / 3.) + 0.5) / 2.0;
	float g = (r + b) / 2.f;
	fragColor = vec4(r, g, b, 1.);
}
