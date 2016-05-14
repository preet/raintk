std::string const raintk_atlas_image_vert_glsl = R"___DELIM___(

#ifdef GL_ES
    //
#else
    #define lowp
    #define mediump
    #define highp
#endif

attribute vec4 a_v4_position;
attribute vec3 a_v3_tex0_opacity;

uniform mat4 u_m4_pv; // proj*view

varying lowp vec3 v_v3_tex0_opacity;

void main()
{
    v_v3_tex0_opacity = a_v3_tex0_opacity;
    gl_Position = u_m4_pv*a_v4_position;
}

)___DELIM___";


std::string const raintk_atlas_image_frag_glsl = R"___DELIM___(

#ifdef GL_ES
    precision mediump float;
#else
    #define lowp
    #define mediump
    #define highp
#endif

varying lowp vec3 v_v3_tex0_opacity;
uniform lowp sampler2D u_s_tex0;

void main()
{
    vec4 color = texture2D(u_s_tex0,v_v3_tex0_opacity.xy)*v_v3_tex0_opacity.z;
    gl_FragColor = color;
}

)___DELIM___";
