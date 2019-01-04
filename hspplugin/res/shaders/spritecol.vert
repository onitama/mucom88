// Attributes
attribute vec3 a_position;
attribute vec4 a_color;

// Uniforms
uniform mat4 u_projectionMatrix;

// Varyings
varying vec4 v_color;


void main()
{
    gl_Position = u_projectionMatrix * vec4(a_position, 1);
    v_color = a_color;
}
