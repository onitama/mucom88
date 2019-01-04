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
uniform float u_brightness;
uniform float u_contrast;

///////////////////////////////////////////////////////////
// Varyings
varying vec2 v_texCoord;
varying vec4 v_color;


void main()
{
   vec4 col = v_color * texture2D(u_texture, v_texCoord);
   col.rgb = ((col.rgb - 0.5f) * max(u_contrast, 0)) + 0.5f;
   col.rgb += u_brightness;
   gl_FragColor = col;
}