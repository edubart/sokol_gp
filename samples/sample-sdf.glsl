@ctype vec2 sgp_vec2
@ctype float float

@vs sdf_vs
in vec4 coord;
out vec2 fragUV;
void main() {
    gl_Position = vec4(coord.xy, 0.0, 1.0);
    fragUV = coord.zw;
}
@end

@fs sdf_fs
uniform sampler2D iChannel0;
uniform sdf_uniform {
    vec2 iResolution;
    float iTime;
};
in vec2 fragUV;
out vec4 fragColor;

#define MAX_DIST 100.0
#define MAX_STEPS 64
#define SURF_DIST 0.0001
#define M_PI 3.1415926535897932384626433832795
#define AA 1

#define MATERIAL_BODY 1
#define MATERIAL_BELLY 2
#define MATERIAL_SKIN_YELLOW 3
#define MATERIAL_EYE 4
#define MATERIAL_FLOOR 5

struct Hit {
    float d;
    int material;
};

// Distance to sphere at origin of radius `r`
float sd_sphere(vec3 p, float r) {
    return length(p) - r;
}

// Distance to capsule of radius `r` formed by line segment from `a` to `b`
float sd_capsule(vec3 p, vec3 a, vec3 b, float r) {
    vec3 ap = p - a;
    vec3 ab = b - a;
    float t = clamp(dot(ap, ab) / dot(ab, ab), 0.0, 1.0);
    return length(ap - ab*t) - r;
}

// Distance to plane at origin of normal `n`
float sd_plane(vec3 p, vec3 n) {
    return dot(p, n);
}

// Distance to torus at origin of radius `r`
float sd_torus(vec3 p, vec2 r) {
    float x = length(p.xz) - r.x;
    return length(vec2(x, p.y)) - r.y;
}

// Distance to ellipsoid at origin of radius `r`
float sd_ellipsoid(vec3 p, vec3 r) {
    float k0 = length(p/r);
    float k1 = length(p/(r*r));
    return k0*(k0-1.0)/k1;
}

// Distance to round cone
float sd_round_cone(vec3 p, float r1, float r2, float h) {
    vec2 q = vec2(length(p.xz), p.y);

    float b = (r1-r2)/h;
    float a = sqrt(1.0-b*b);
    float k = dot(q,vec2(-b,a));

    if( k < 0.0 ) return length(q) - r1;
    if( k > a*h ) return length(q-vec2(0.0,h)) - r2;

    return dot(q, vec2(a,b) ) - r1;
}

// Distance to box at origin of radius `r`
float sd_box(vec3 p , vec3 r) {
    vec3 q = abs(p) - r;
    return length(max(q,0.0)) + min(max(q.x,max(q.y,q.z)),0.0);
}

// Create infinite copies of an object every `s` distance
vec3 op_rep(vec3 p, float s) {
    return mod(p+s*0.5,s)-s*0.5;
}

// Unite two SDFs smoothing
float op_smooth_union(float d1, float d2, float k ) {
    float h = clamp(0.5 + 0.5*(d2-d1)/k, 0.0, 1.0);
    return mix( d2, d1, h ) - k*h*(1.0-h);
}

// Rotate vector `p` along the x axis at angle `t` (in radians)
vec3 rotate_x(vec3 p, float t) {
    float cost = cos(t);
    float sint = sin(t);
    mat3 a = mat3(
        1.0,  0.0,  0.0,
        0.0, cost,-sint,
        0.0, sint, cost);
    return a * p;
}

// Rotate vector `p` along the y axis by angle `t` (in radians)
vec3 rotate_y(vec3 p, float t) {
    float cost = cos(t);
    float sint = sin(t);
    mat3 a = mat3(
         cost, 0.0, sint,
          0.0, 1.0,  0.0,
        -sint, 0.0, cost);
    return a * p;
}

// Rotate vector `p` along the z axis by angle `t` (in radians)
vec3 rotate_z(vec3 p, float t) {
    float cost = cos(t);
    float sint = sin(t);
    mat3 a = mat3(
         cost,-sint, 0.0,
         sint, cost, 0.0,
          0.0,  0.0, 1.0);
    return a * p;
}

#define check_hit(m) if(dist < mindist) { material = m; mindist = dist; }

void get_penguin_sdf(vec3 p, inout float mindist, inout int material) {
    float dist;

    // body
    dist = op_smooth_union(
        sd_ellipsoid(p - vec3(0.0, 1.0, 0.0), vec3(2.0, 2.2, 1.8)),
        sd_sphere(p - vec3(0.0, 3.0, 0.0), 1.6), 0.5);

    // arms
    const vec3 larm_pos = vec3(-2.0, 2.0, 0);
    const vec3 rarm_pos = larm_pos*vec3(-1.0, 1.0, 1.0);
    const vec3 arm_radius = vec3(0.9, 0.3, 0.8);
    dist = op_smooth_union(dist,
        sd_ellipsoid(rotate_z(p - larm_pos, 0.7 + sin(iTime*10.0)*0.2), arm_radius), 0.1);
    dist = op_smooth_union(dist,
        sd_ellipsoid(rotate_z(p - rarm_pos,-0.7 + sin(iTime*10.0)*0.2), arm_radius), 0.1);

    // rings around eyes
    const vec3 leye_pos = vec3(-0.5, 3.2, 1.0);
    const vec3 reye_pos = leye_pos*vec3(-1.0, 1.0, 1.0);
    const vec3 leye_size = vec3(0.8, 0.9, 0.7);
    const vec3 reye_size = vec3(0.8, 0.7, 0.7);
    dist = op_smooth_union(dist,
        sd_torus(rotate_x(p - leye_pos, M_PI/2.0), vec2(leye_size.x, 0.2)), 0.1);
    dist = op_smooth_union(dist,
        sd_torus(rotate_x(p - reye_pos, M_PI/2.0), vec2(reye_size.x, 0.2)), 0.1);

    check_hit(MATERIAL_BODY);

    // belly
    dist = sd_ellipsoid(p - vec3(0.0, 1.0, 0.4), vec3(2.0, 2.2, 1.8)*0.9);
    check_hit(MATERIAL_BELLY);


    // foot
    const vec3 lfoot_pos = vec3(-1.2, -0.5, 1.9);
    const vec3 rfoot_pos = vec3(-lfoot_pos.x, lfoot_pos.y, lfoot_pos.z);
    const vec3 foot_radius = vec3(1.0, 0.6, 1.0);
    dist = min(
        sd_ellipsoid(rotate_x(p - lfoot_pos, sin(iTime*10.0)*0.3), foot_radius),
        sd_ellipsoid(rotate_x(p - rfoot_pos, sin(iTime*10.0+M_PI)*0.3), foot_radius));

    // beak
    dist = min(dist, sd_round_cone(rotate_x(p - vec3(0.0, 2.5, 2.0), 3.3*M_PI/2.0), 0.1, 0.4, 0.6));
    check_hit(MATERIAL_SKIN_YELLOW);

    // eyes
    dist = min(
        sd_ellipsoid(p - leye_pos, leye_size),
        sd_ellipsoid(p - reye_pos, reye_size));
    check_hit(MATERIAL_EYE);

    // retina
    vec3 lretina_pos = vec3(-0.4 + sin(iTime)*0.1, 3.3, 1.63);
    vec3 rretina_pos = vec3(0.4 - sin(iTime)*0.1, 3.3, 1.63);
    dist = min(
        sd_ellipsoid(p - lretina_pos, vec3(0.2, 0.3, 0.1)),
        sd_ellipsoid(p - rretina_pos, vec3(0.2, 0.25, 0.1)));
    check_hit(MATERIAL_BODY);
}

// Return the closest surface distance to point p
Hit get_sdf(vec3 p) {
    float mindist = MAX_DIST;
    int material = 0;

    // penguin
    get_penguin_sdf(rotate_z(p, sin(iTime*10.0)*0.1), mindist, material);

    // floor
    //float dist = sd_plane(p - vec3(0.0, -1.0, 0.0), normalize(vec3(0.0, 1.0, 0.0)));
    //check_hit(MATERIAL_FLOOR);

    return Hit(mindist, material);
}

// Get normal at point `p` using the tetrahedron technique for computing the gradient
vec3 get_normal(vec3 p) {
    const float eps = 0.0001;
    vec2 e = vec2(1.0,-1.0);
    return normalize(e.xyy*get_sdf(p + e.xyy*eps).d +
                     e.yyx*get_sdf(p + e.yyx*eps).d +
                     e.yxy*get_sdf(p + e.yxy*eps).d +
                     e.xxx*get_sdf(p + e.xxx*eps).d);
}

// March a ray from `rayfrom` along the `raydir` direction and return the closet surface distance
Hit ray_march(vec3 rayfrom, vec3 raydir) {
    // begin at ray origin
    float t = 0.0;
    Hit hit;
    // ray march loop
    for(int i=0; i<MAX_STEPS; ++i) {
        // compute next march point
        vec3 p = rayfrom+t*raydir;
        // get the distance to the closest surface
        hit = get_sdf(p);
        // hit a surface
        if(abs(hit.d) < (SURF_DIST*t))
            break;
        // increase the distance to the closest surface
        t += hit.d;
    }
    if(t > MAX_DIST)
        hit.material = 0;
    // return the distance to `rayfrom`
    hit.d = t;
    return hit;
}

// Hard shadows
float hard_shadow(vec3 rayfrom, vec3 raydir, float tmin, float tmax) {
    float t = tmin;
    for(int i=0; i<MAX_STEPS; i++) {
        vec3 p = rayfrom + raydir*t;
        float h = get_sdf(p).d;
        if(h < SURF_DIST)
            return 0.0;
        t += h;
        if(t > tmax)
            break;
    }
    return 1.0;
}

// Get occlusion along `normal` from point of view `rayfrom`
float get_occlusion(vec3 rayfrom, vec3 normal) {
    const int AO_ITERATIONS = 5;
    const float AO_START = 0.01;
    const float AO_DELTA = 0.11;
    const float AO_DECAY = 0.95;
    const float AO_INTENSITY = 2.0;

    float occ = 0.0;
    float decay = 1.0;
    for(int i=0; i<AO_ITERATIONS; ++i) {
        float h = AO_START + float(i) * AO_DELTA;
        float d = get_sdf(rayfrom + h*normal).d;
        occ += (h-d) * decay;
        decay *= AO_DECAY;
    }
    return clamp(1.0 - occ * AO_INTENSITY, 0.0, 1.0);
}

// Return diffuse albedo color for material
vec3 get_material_diffuse(vec3 p, int material) {
    switch(material) {
        case MATERIAL_BODY:
            return vec3(0.0, 0.0, 0.0);
        case MATERIAL_BELLY:
            return vec3(0.6, 0.6, 0.6);
        case MATERIAL_FLOOR: {
            float checker = clamp(sin(2.0*p.x)*sin(2.0*p.y)*sin(2.0*p.z+iTime*8.0)+0.5, 0.0, 1.0);
            return vec3(0.05, 0.09, 0.02) + vec3(0.06, 0.06, 0.02)* checker;
        }
        case MATERIAL_SKIN_YELLOW:
            return vec3(1.0, .3, .01);
        case MATERIAL_EYE:
            return vec3(1.0, 1.0, 1.0);
        default:
            return vec3(1.0, 1.0, 1.0);
    }
}

// Return specular color for material
vec3 get_material_specular(vec3 p, int material) {
    switch(material) {
        case MATERIAL_BODY:
            return vec3(0.3);
        case MATERIAL_SKIN_YELLOW:
            return vec3(1.0, .6, .1);
        case MATERIAL_EYE:
            return vec3(0.6);
        default:
            return vec3(0.0);
    }
}

// Compute the scene light at a point
vec3 get_light(vec3 raydir, vec3 p, int material) {
    vec3 diffuse = vec3(0);
    vec3 specular = vec3(0);
    vec3 normal = get_normal(p);
    float occlusion = get_occlusion(p, normal);

    // sun light
    const float SUN_INTENSITY = 1.0;
    const float SUN_SHINESS = 8.0;
    const vec3 SUN_DIRECTION = normalize(vec3(0.6, 0.35, 0.5));
    const vec3 SUN_COLOR = vec3(1.0,0.77,0.6);

    float sun_diffuse = clamp(dot(normal, SUN_DIRECTION), 0.0, 1.0);
    float sun_shadow = hard_shadow(p, SUN_DIRECTION, 0.01, 20.0);
    float sun_specular = pow(clamp(dot(reflect(SUN_DIRECTION, normal), raydir), 0.0, 1.0), SUN_SHINESS);

    diffuse += SUN_COLOR * (sun_diffuse * sun_shadow * SUN_INTENSITY);
    specular += SUN_COLOR * sun_specular;

    // sky light
    const float SKY_INTENSITY = 1.0;
    const float SKY_SHINESS = 8.0;
    const float SKY_BIAS = 0.5;
    const vec3 SKY_COLOR = vec3(0.50,0.70,1.00);
    const vec3 SKY_DIRECTION = vec3(0.0, 1.0, 0.0);

    float sky_diffuse = SKY_BIAS + (1.0 - SKY_BIAS)*clamp(dot(normal, SKY_DIRECTION), 0.0, 1.0);
    float sky_specular = pow(clamp(dot(reflect(SKY_DIRECTION, normal), raydir), 0.0, 1.0), SKY_SHINESS);
    diffuse += SKY_COLOR * (SKY_INTENSITY * sky_diffuse * occlusion);
    specular += SKY_COLOR * (sky_specular * occlusion);

    // fake indirect light
    const float INDIRECT_INTENSITY = 0.2;
    const float INDIRECT_SHINESS = 8.0;
    const vec3 INDIRECT_COLOR = SUN_COLOR;

    vec3 ind_dir = normalize(SUN_DIRECTION * vec3(-1.0,0.0,1.0));
    float ind_diffuse = clamp(dot(normal, ind_dir), 0.0, 1.0);
    float ind_specular = pow(clamp(dot(reflect(ind_dir, normal), raydir), 0.0, 1.0), INDIRECT_SHINESS);
    diffuse += INDIRECT_COLOR * (ind_diffuse * INDIRECT_INTENSITY);
    specular += INDIRECT_COLOR * (ind_specular * INDIRECT_INTENSITY);

    // fresnel light
    const float FRESNEL_INTENSITY = 2.0;
    const vec3 FRESNEL_COLOR = SUN_COLOR;
    float fresnel_diffuse = clamp(1.0+dot(raydir, normal), 0.0, 1.0);
    diffuse += FRESNEL_COLOR * (FRESNEL_INTENSITY * fresnel_diffuse * (0.5 + 0.5*sun_diffuse));

    // apply material
    vec3 col = diffuse * get_material_diffuse(p, material) +
               specular * get_material_specular(p, material);

    // gamma correction
    col = pow(col, vec3(0.4545));

    return col;
}

vec3 get_sky_background(vec3 raydir) {
    const vec3 SKY_COLOR1 = vec3(0.4,0.75,1.0);
    const vec3 SKY_COLOR2 = vec3(0.7,0.8,0.9);
    vec3 col = mix(SKY_COLOR2, SKY_COLOR1, exp(10.0*raydir.y));
    //vec2 uv = raydir.xz / raydir.y;
    //float cl = sin(uv.x*0.3)+sin(uv.y*0.3);// + 0.5*sin(uv.x*2.0)+sin(uv.y*2.0);
    //col = mix(col, vec3(0.7,0.8,0.9), smoothstep(-0.1,0.1,-0.5+cl));
    return col;
}

// Return camera transform matrix looking from `lookfrom` towards `lookat`, with tilt rotation `tilt`,
// vertical field of view `vfov` (in degrees), at coords `uv` (in the range [-1,1])
vec3 get_ray(vec3 lookfrom, vec3 lookat, float tilt, float vfov, vec2 uv) {
    // camera up vector
    vec3 vup = vec3(sin(tilt), cos(tilt), 0.0);
    // camera look direction
    vec3 lookdir = normalize(lookat - lookfrom);
    // unit vector in camera x axis
    vec3 u = cross(lookdir, vup);
    // unit vector in camera y axis
    vec3 v = cross(u, lookdir);
    // vector in camera z axis normalized by the fov
    vec3 w = lookdir * (1.0 / tan(vfov*M_PI/360.0));
    // camera transformation matrix
    mat3 t = mat3(u, v, w);
    // camera direction
    return normalize(t * vec3(uv, 1.0));
}

vec3 render(vec2 uv) {
    float theta = sin(iTime) * 0.5;
    vec3 lookat = vec3(0.0, 1.0, 0.0);
    vec3 lookfrom = vec3(30.0*sin(theta), 5.0, 30.0*cos(theta));
    vec3 raydir = get_ray(lookfrom, lookat, 0.0, 20.0, uv);
    Hit hit = ray_march(lookfrom, raydir);
    vec3 p = lookfrom + raydir * hit.d;
    if(hit.material > 0)
        return get_light(raydir, p, hit.material);
    else
        return get_sky_background(raydir);
}

vec3 render_aa(vec2 uv) {
#if AA > 1
    float w = 1.0/iResolution.y;
    vec3 col = vec3(0.0);
    for(int n=0; n<AA*AA; ++n) {
        vec2 o = 2.0*(vec2(float(int(n / AA)),float(int(n % AA))) / float(AA) - 0.5);
        col += render(uv + o*w);
    }
    col /= float(AA*AA);
    return col;
#else
    return render(uv);
#endif
}

void main() {
    // uv coords in range from [-1,1] for y and [-aspect_ratio,aspect_ratio] for x
    vec2 fragCoord = fragUV * iResolution;
    fragCoord.y = iResolution.y - fragCoord.y;
    vec2 uv = 2.0 * ((fragCoord-0.5*iResolution.xy) / iResolution.y);
    // render the entire scene
    vec3 col = render_aa(uv);
    // set the finished color
    fragColor = vec4(col,1);
}

@end

@program sdf sdf_vs sdf_fs
