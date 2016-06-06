// This version of the shader doesn't require dFdx,dFdy or fwidth
// which may not be available on GL ES 2 devices

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
attribute vec2 a_v2_tex0;
attribute vec2 a_v2_highlight_index;

// uniforms
uniform mat4 u_m4_pv; // proj*view
uniform mat4 u_array_m4_model[K_NUM_U_ARRAY_SIZE];
uniform vec4 u_array_v4_color[K_NUM_U_ARRAY_SIZE];
uniform vec4 u_array_v4_highlight_color[K_NUM_U_ARRAY_SIZE];
uniform float u_array_f_res[K_NUM_U_ARRAY_SIZE];

// varyings
varying mediump vec2 v_v2_tex0_uv;
varying lowp vec4 v_v4_color;
varying highp float v_f_res;

void main()
{
    int highlight = int(a_v2_highlight_index.x);
    int index = int(a_v2_highlight_index.y);

    v_v2_tex0_uv = a_v2_tex0;

    v_v4_color =
            (1-highlight)*u_array_v4_color[index] +
            (highlight)*u_array_v4_highlight_color[index];

    v_f_res = u_array_f_res[index];
    mat4 model_xf = u_array_m4_model[index];

    gl_Position = u_m4_pv * model_xf * a_v2_position;
}

)___DELIM___";



std::string const text_sdf_frag_glsl = R"___DELIM___(

// FRAGMENT SHADER
#ifdef GL_ES
    precision mediump float;
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
varying float v_f_res;

// uniforms
uniform highp sampler2D u_s_tex0;

// dist
// The distance field value for this fragment:
// (dist == 0.5): on the shape's edge
// (dist < 0.5): moving away from the edge outwards
// (dist > 0.5): moving away from the edge inwards

void main()
{
    // To find the min and max around 0.5, emulate the glsl
    // function fwidth (unavailable in glsl for es 2). We
    // use v_f_res, which is the tex_coords/pixels ratio of
    // the glyph and sample the texture for the next pixels
    // along x and y to find the sum of absolute derivatives
    // for this pixel.

    vec2 d_tex0_uv_dx = vec2(v_f_res,0.0f);
    vec2 d_tex0_uv_dy = vec2(0.0f,v_f_res);

    float dist_next_x = texture2D(u_s_tex0,v_v2_tex0_uv+d_tex0_uv_dx).r;
    float dist_next_y = texture2D(u_s_tex0,v_v2_tex0_uv+d_tex0_uv_dy).r;

    const float glyph_center = 0.495;
    float dist = texture2D(u_s_tex0,v_v2_tex0_uv).r;

    // equivalent to fwidth, but this might be faster since
    // we don't have to calculate dfdx and dfdy again
    float d_dist_dxy = abs(dist_next_x-dist) + abs(dist_next_y-dist);


    // Get the initial alpha for this fragment by using
    // a smoothstep interpolation around 0.5

    // The min and max is set based on how much the distance
    // field texture changes between adjacent pixels; we
    // want less smoothing when the change is small
    // float width = fwidth(dist) * 0.35;   // 0.35 is empirical, can be tuned
    float width = d_dist_dxy * 0.35;


    float dist_min = glyph_center-width;
    float dist_max = glyph_center+width;

    float alpha = smoothstep(dist_min,dist_max,dist);

    // The current alpha value is fine for medium and large
    // text, but smaller text needs supersampling
    // ref: https://www.reddit.com/r/gamedev/comments/2879jd/...
    //      just_found_out_about_signed_distance_field_text/

    // Get 4 adjacent points on the texture to sample
    float dscale = 0.354; // half of 1/sqrt2, can be tuned
    float weight = 0.5; // sample weight, can be tuned
    vec2 sample_extents = dscale * (d_tex0_uv_dx + d_tex0_uv_dy);
    vec4 box = vec4(v_v2_tex0_uv-sample_extents,
                    v_v2_tex0_uv+sample_extents);

    // Sum the samples
    // TODO this seems pretty expensive, is there a faster way?
    float sum_samples =
            smoothstep(dist_min,dist_max,texture2D(u_s_tex0,box.xy).r)+
            smoothstep(dist_min,dist_max,texture2D(u_s_tex0,box.zw).r)+
            smoothstep(dist_min,dist_max,texture2D(u_s_tex0,box.xw).r)+
            smoothstep(dist_min,dist_max,texture2D(u_s_tex0,box.zy).r);

    // Weighted average
    alpha = (alpha + weight*sum_samples) / (1.0 + 4.0*weight);

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
