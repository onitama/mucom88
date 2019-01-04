///////////////////////////////////////////////////////////
// Attributes
attribute vec3 a_position;
attribute vec2 a_texCoord;
attribute vec4 a_color;

///////////////////////////////////////////////////////////
// Uniforms
uniform mat4 u_projectionMatrix;
uniform float u_length;
uniform float u_length2;

///////////////////////////////////////////////////////////
// Varyings
varying vec2 v_texCoord;
varying vec4 v_color;
varying vec2 v_blurCoord[5];

void main()
{
    gl_Position = u_projectionMatrix * vec4(a_position, 1);
    v_texCoord = a_texCoord;
    v_color = a_color;

    v_blurCoord[0] = a_texCoord.xy;
    v_blurCoord[1] = a_texCoord.xy + vec2( u_length,u_length2 ) * 1.407333;
    v_blurCoord[2] = a_texCoord.xy - vec2( u_length,u_length2 ) * 1.407333;
    v_blurCoord[3] = a_texCoord.xy + vec2( u_length,u_length2 ) * 3.294215;
    v_blurCoord[4] = a_texCoord.xy - vec2( u_length,u_length2 ) * 3.294215;
}
