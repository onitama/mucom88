#if defined(OPENGL_ES) || defined(GL_ES)
#ifdef GL_FRAGMENT_PRECISION_HIGH
precision highp float;
#else
precision mediump float;
#endif
#endif

///////////////////////////////////////////////////////////
// Uniforms
uniform sampler2D u_texture;

uniform float u_curvex;
uniform float u_curvey;
#define SCANLINE 448.0		// ƒXƒLƒƒƒ“ƒ‰ƒCƒ“

///////////////////////////////////////////////////////////
// Varyings
varying vec2 v_texCoord;
varying vec4 v_color;


void main()
{
	vec2 tex = vec2(v_texCoord.x, v_texCoord.y);
	float dx = abs(0.5-tex.x);
	float dy = abs(0.5-tex.y);
	tex.x -= 0.5;
	tex.x *= 1.0 + (dy * dy * u_curvex);
	tex.x += 0.5;

	tex.y -= 0.5;
	tex.y *= 1.0 + (dx * dx * u_curvey);
	tex.y += 0.5;

	vec4 col;
	if(tex.y > 1.0 || tex.x < 0.0 || tex.x > 1.0 || tex.y < 0.0) {
		col.rgb = vec3( 0,0,0 );
	} else {
		// Get texel, and add in scanline if need be
		col = texture2D(u_texture, vec2(tex.x, tex.y));
		col.rgb += sin(tex.y * SCANLINE) * 0.1;
	}
	gl_FragColor = col * v_color;
}