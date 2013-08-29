#version 120

// textures
uniform sampler2D TDiffuse, THeightMap, TNormalMap, TConeMap, TRConeMap;
// light position
uniform vec3 LightPos;
// rendering mode
uniform int Mode;
// render performance?
uniform int DrawPerformance;
// root finding method (0 = none, 1 = binary search)
uniform int RootMethod;

// variables from vertex shader
varying vec2 Coord;
varying vec3 EPos;

const vec3 Eye = vec3(0.0, 0.0, -1.0);
// performance (iteration count) scale
const int low_i = 0, mid_i = 12, high_i = 40;
// surface steepness
const float steepness = 0.15;
// relief steps
const int relief_steps = 30;

float sstep(int lo, int hi, int val)
{
    float t;
    t = clamp((val - lo) / float(hi - lo), 0.0, 1.0);
    return t * t * (3.0 - 2.0 * t);
}

vec4 compute_frag_color(vec4 diffuse, vec3 norm, vec3 pos)
{
    norm = normalize(gl_NormalMatrix * norm);
    vec3 to_light = LightPos - pos;
    vec3 dir_light = normalize(to_light);
    float dist_light = length(to_light);
    vec3 refl_light = reflect(dir_light, norm);

    //float att =  1.0 / (1.0 + 0.3 * dist_light * dist_light);
    float att = 1.0;
    float dif = max(dot(norm, dir_light), 0.0);
    float spec = max(pow(-dot(refl_light, Eye), 30.0), 0.0);

    return att * (diffuse * dif + vec4(0.5,0.5,0.5,1) * spec);
}

void main()
{
    vec3 coord = vec3(Coord, 1.0);
    vec3 coord_last = coord;
    vec3 norm = vec3(0, 0, 1);
    vec3 pos = EPos;
    int iters = 0;
    // eye vector in tangent space
    vec3 eyets = normalize(transpose(gl_NormalMatrix) * Eye);

    if (Mode == 2) {
        // Parallax Mapping
        iters += 1;
        // sample heightmap
        float h = 1 - texture2D(THeightMap, coord.st).r;
        coord.st += eyets.xy * h * steepness;
        coord_last = coord;
    }
    else if (Mode == 3) {
        // Relief Mapping
        vec3 rstep = vec3(eyets.xy, eyets.z / steepness);
        rstep *= -1.0 / rstep.z / float(relief_steps);
        while (texture2D(THeightMap, coord.st).r < coord.z) {
            coord_last = coord;
            coord += rstep;
            ++iters;
        }
    }
    else if (Mode == 4) {
        // Cone Stepping
        vec3 rstep = vec3(eyets.xy, eyets.z / steepness);
        rstep *= -1.0 / rstep.z;
        float ray_ratio = length(rstep.xy);
        coord_last = vec3(coord.xy, 1.01);

        while (texture2D(THeightMap, coord.st).x < coord.z - 1.0/512.0 && iters < 35) {
            coord_last = coord;
            ++iters;
            float cone = texture2D(TConeMap, coord.st).x;
            float dep = texture2D(THeightMap, coord.st).x;
            float depth_step = -(dep - coord.z) * cone / (ray_ratio + cone);
            coord += depth_step * rstep;
        }
    }


    if (RootMethod == 1) {
        // binary search
        while (length(coord - coord_last) > (1.0/512.0)) {
            vec3 mid = (coord + coord_last) / 2.0;
            ++iters;
            if (texture2D(THeightMap, mid.st).r > mid.z) coord = mid;
            else coord_last = mid;
        }
    }

    if (Mode > 0) {
        // compute normal approx. from heightmap
        float ht  = texture2D(THeightMap, coord.st).x;
        float hts = texture2D(THeightMap, coord.st + vec2(2.0 / 512, 0)).x;
        float htt = texture2D(THeightMap, coord.st + vec2(0, 2.0 / 512)).x;
        norm = normalize(vec3(ht - hts, ht - htt, steepness));
    }


    if (DrawPerformance == 1) {
        gl_FragColor = vec4(sstep(low_i, mid_i, iters), 1-sstep(mid_i, high_i, iters), 0, 1);
        //gl_FragColor = vec4(texture2D(TConeMap, Coord.st).sss, 1);
    }
    else {
        //vec4 clr = vec4(texture2D(TDiffuse, coord.st).rgb, 1);
        vec4 clr = vec4(0.1, texture2D(THeightMap, coord.st).g, 0.3, 1);
        //vec4 clr = vec4(abs(sin(coord.s * 12.5)), abs(sin(coord.s * coord.t * 30)), coord.t, 1.0);
        if (max(coord.s, coord.t) > 1.0 || min(coord.s, coord.t) < 0.0) discard;
        gl_FragColor = compute_frag_color(clr, norm, pos);
        //gl_FragColor = vec4(coord, 1);
    }
}
