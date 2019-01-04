#if defined(OPENGL_ES) || defined(GL_ES)
precision highp float;
#endif

// Varyings
varying vec4 v_color;


void main()
{
    gl_FragColor = v_color;
}