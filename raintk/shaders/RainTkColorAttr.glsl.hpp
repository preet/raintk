std::string const color_attr_vert_glsl = R"___DELIM___(

#ifdef GL_ES
    //
#else
    #define lowp
    #define mediump
    #define highp
#endif

attribute vec4 a_v4_position;
attribute vec4 a_v4_color;

uniform mat4 u_m4_pv; // proj*view

varying lowp vec4 v_v4_color;

void main()
{
    gl_Position = u_m4_pv * a_v4_position;
    v_v4_color = a_v4_color;
}

)___DELIM___";


std::string const color_attr_frag_glsl = R"___DELIM___(

#ifdef GL_ES
    precision mediump float;
#else
    #define lowp
    #define mediump
    #define highp
#endif

varying lowp vec4 v_v4_color;

void main(void)
{
    gl_FragColor = v_v4_color;
}

)___DELIM___";
 
