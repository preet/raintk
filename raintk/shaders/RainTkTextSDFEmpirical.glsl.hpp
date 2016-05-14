std::string const text_sdf_vert_glsl = R"___DELIM___(

// VERTEX SHADER
#ifdef GL_ES
    //
#else
    #define lowp
    #define mediump
    #define highp
#endif

// The uniform array size must be defined by
// the application using this shader
#define K_NUM_U_ARRAY_SIZE 0

// attributes
attribute vec4 a_v2_position;
attribute vec3 a_v3_tex0_index;

// uniforms
uniform mat4 u_m4_pv; // proj*view
uniform mat4 u_array_m4_model[K_NUM_U_ARRAY_SIZE];
uniform vec4 u_array_v4_color[K_NUM_U_ARRAY_SIZE];
uniform float u_array_f_res[K_NUM_U_ARRAY_SIZE];

// varyings
varying mediump vec2 v_v2_tex0_uv;
varying lowp vec4 v_v4_color;
varying mediump float v_f_glyph_sm_width;

void main()
{
    int index = int(a_v3_tex0_index.z);

    v_v2_tex0_uv = vec2(a_v3_tex0_index.xy);
    v_v4_color = u_array_v4_color[index];
    mat4 model_xf = u_array_m4_model[index];
    float glyph_tex_res = u_array_f_res[index];

    // Linearly interpolate to find a good smoothing value for
    // this glyph based on its texture resolution in screen space

    // The constants were empirically determined by trying different
    // smoothing values for different font sizes

    // Increasing the slope will sharpen larger text and decreasing
    // the slope will have the opposite effect

    // NOTE: We don't do this CPU side because u_array_f_res is
    // used directly by another version of this shader
    // (text_sdf_glsl_no_ext) and its convenient to not have to
    // change the uniforms

    v_f_glyph_sm_width = (glyph_tex_res-0.00414)*53.25079765 + 0.21;

    gl_Position = u_m4_pv * model_xf * a_v2_position;
}

)___DELIM___";



std::string const text_sdf_frag_glsl = R"___DELIM___(

// FRAGMENT SHADER
#ifdef GL_ES
    precision mediump float;

    #ifdef GL_OES_standard_derivatives
        #extension GL_OES_standard_derivatives : enable
    #endif

#else
    // with default (non ES) OpenGL shaders, precision
    // qualifiers aren't used -- we explicitly set them
    // to be defined as 'nothing' so they are ignored
    #define lowp
    #define mediump
    #define highp
#endif

// varyings
varying mediump vec2 v_v2_tex0_uv;
varying lowp vec4 v_v4_color;
varying float v_f_glyph_sm_width;

// uniforms
uniform highp sampler2D u_s_tex0;

// dist
// The distance field value for this fragment:
// (dist == 0.5): on the shape's edge
// (dist < 0.5): moving away from the edge outwards
// (dist > 0.5): moving away from the edge inwards

void main()
{
    const float glyph_center = 0.495;
    float dist = texture2D(u_s_tex0,v_v2_tex0_uv).r;
    float alpha = smoothstep(glyph_center-v_f_glyph_sm_width,
                             glyph_center+v_f_glyph_sm_width,
                             dist);

    gl_FragColor = v_v4_color*alpha; // premultiplied alpha
}

// Debug
//void main(void)
//{
//    vec4 color = v_v4_color*texture2D(u_s_tex0,v_v2_tex0).r;
//    color.a = 1.0;
//    gl_FragColor = color;
//}

)___DELIM___";
