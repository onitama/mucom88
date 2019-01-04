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
uniform float u_length;
uniform float u_length2;

///////////////////////////////////////////////////////////
// Varyings
varying vec2 v_texCoord;
varying vec4 v_color;
varying vec2 v_blurCoord[5];

void main()
{
    vec4 sum = vec4(0.0);
    sum += texture2D(u_texture, v_blurCoord[0]) * 0.204164;
    sum += texture2D(u_texture, v_blurCoord[1]) * 0.304005;
    sum += texture2D(u_texture, v_blurCoord[2]) * 0.304005;
    sum += texture2D(u_texture, v_blurCoord[3]) * 0.093913;
    sum += texture2D(u_texture, v_blurCoord[4]) * 0.093913;
    gl_FragColor = sum;
}

