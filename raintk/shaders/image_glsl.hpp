std::string const image_vert_glsl = R"___DELIM___(

#ifdef GL_ES
    //
#else
    #define lowp
    #define mediump
    #define highp
#endif

attribute vec4 a_v4_position;
attribute vec2 a_v2_tex0;

uniform mat4 u_m4_pv; // proj*view
uniform mat4 u_m4_model;
uniform vec2 u_v2_tile;

varying lowp vec2 v_v2_tex0;

void main()
{
    v_v2_tex0 = a_v2_tex0*u_v2_tile;
    gl_Position = u_m4_pv*u_m4_model*a_v4_position;
}

)___DELIM___";


std::string const image_frag_glsl = R"___DELIM___(

#ifdef GL_ES
    precision mediump float;
#else
    #define lowp
    #define mediump
    #define highp
#endif

varying lowp vec2 v_v2_tex0;
uniform lowp sampler2D u_s_tex0;

void main()
{
    gl_FragColor = texture2D(u_s_tex0,v_v2_tex0);
}

)___DELIM___";
