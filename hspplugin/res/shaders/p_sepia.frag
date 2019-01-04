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

///////////////////////////////////////////////////////////
// Varyings
varying vec2 v_texCoord;
varying vec4 v_color;


void main()
{
    // Sample the diffuse texture for base color
    float gray = dot(texture2D(u_texture, v_texCoord).rgb, vec3(0.299, 0.587, 0.114));
    		
    // Play with these rgb weightings to get different tones, but rgb weightings should add up to 1.0 so you won't lighten or darken the image.
	gl_FragColor = vec4(gray * vec3(1.2, 1.0, 0.8), 1.0);
}

