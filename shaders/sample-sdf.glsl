@module sdf
@ctype vec2 sgp_vec2
@ctype float float

@vs vs
uniform uniforms {
    vec2 iResolution;
    float iTime;
};
layout(location=0) in vec4 coord;
layout(location=0) out vec2 texUV;
layout(location=1) out vec2 fragPos;
void main() {
    gl_Position = vec4(coord.xy, 0.0, 1.0);
    fragPos = vec2(coord.x * (iResolution.x/iResolution.y), coord.y);
    texUV = coord.zw;
}
@end

@fs fs
uniform texture2D iTexChannel0;
uniform sampler iSmpChannel0;
uniform uniforms {
    vec2 iResolution;
    float iTime;
};
layout(location=0) in vec2 texUV;
layout(location=1) in vec2 fragPos;
layout(location=0) out vec4 fragColor;

#line 0
#define PI 3.141592653589793238

float sd_circle(vec2 p, float r) {
    return length(p) - r;
}

float length_n(vec2 p, float n) { p=pow(abs(p), vec2(n)); return pow(p.x+p.y, 1.0/n); }

float sd_ellipsoid(vec2 p, vec2 r, float roundness){
    float k1 = length_n(p/r, roundness);
    float k2 = length_n(p/(r*r), roundness);
    return k1*(k1-1.0)/k2;
}

float sd_vesica(vec2 p, float r, float d) {
    p = abs(p);
    float b = sqrt(r*r-d*d);
    return ((p.y-b)*d>p.x*b) ? length(p-vec2(0.0,b))
                             : length(p-vec2(-d,0.0))-r;
}

float sd_arc(vec2 p, float ta, float tb, float ra, float rb) {
    vec2 sca = vec2(cos(ta), sin(ta));
    vec2 scb = vec2(cos(tb), sin(tb));
    p *= mat2(sca.x,sca.y,-sca.y,sca.x);
    p.x = abs(p.x);
    float k = (scb.y*p.x>scb.x*p.y) ? dot(p.xy,scb) : length(p.xy);
    return sqrt( dot(p,p) + ra*ra - 2.0*ra*k ) - rb;
}

float sd_line(vec2 p, vec2 a, vec2 b){
    vec2 pa = p-a, ba = b-a;
    float h = clamp( dot(pa,ba)/dot(ba,ba), 0.0, 1.0 );
    return length( pa - ba*h );
}

vec2 rotate(vec2 p, float theta) {
    float cost = cos(theta);
    float sint = sin(theta);
    return vec2(p.x*cost - p.y*sint, p.x*sint + p.y*cost);
}

float smin(float a, float b, float k) {
    float h = max(k - abs(a-b), 0.0)/k;
    return min(a, b) - h*h*k*(1.0/4.0);
}

vec3 background(vec2 uv) {
    vec2 q = abs(floor(uv));
    vec3 col = vec3(0.39);
    col += vec3(0.17)*mod(q.x+q.y, 2.0);
    return col;
}

vec3 blend(vec3 col, vec3 ocol, float d) {
    return mix(ocol, col, smoothstep(0.5, 0.0, d));
}

vec3 map(vec2 p) {
    float d, d2;
    vec2 q;
    p *= 100.0;
    p.y -= 30.0;
    vec2 sp = vec2(abs(p.x), p.y);

    // background
    vec3 ocol = background(p*0.3);

    // water
    d = sd_ellipsoid(p - vec2(0.0, -97.0), vec2(55.0, 20.0), 2.0);
    ocol = blend(vec3(0.61,0.85,0.94), ocol, d);

    // head
    q = sp;
    d = sd_ellipsoid(q, vec2(55.0, 45.0), 2.4);

    // moustache 1
    q -= vec2(45.0, -15.0);
    q.y += q.x*q.x*0.003*sin(iTime*1.0);
    d2 = sd_line(q, vec2(0.0,0.0), vec2(45.0,0.0)) - 0.2;
    d = min(d, d2);

    // moustache 2
    q.y += 6.0;
    q.y += q.x*q.x*0.003*sin(iTime*2.0);
    d2 = sd_line(q, vec2(0.0,0.0), vec2(45.0,0.0)) - 0.2;
    d = min(d, d2);

    // ears
    q = sp - vec2(32.0,30.0);
    q = rotate(q, 0.7+sin(iTime)*0.3);
    d2 = sd_vesica(q, 30.0, 15.0);
    d = min(d, d2);
    ocol = blend(vec3(0.12), ocol, d);

    // body
    q = sp;
    d = sd_ellipsoid(q - vec2(0.0,-60.0), vec2(27.0, 20.0), 2.0);
    d2 = sd_ellipsoid(q - vec2(0.0,-70.0), vec2(20.0, 16.0), 2.0);
    d = max(-d2, d);

    // middle legs
    q = sp - vec2(6.0, -50.0);
    q = rotate(q, -(0.5+0.5*sin(iTime))*0.1);
    q.x -= pow(max(-45.0-q.y, 0.0), 1.9)*0.1;
    d2 = sd_line(q, vec2(0.0,-50.0), vec2(0.0,0.0)) - 5.0;
    d = smin(d, d2, 1.0);

    // outer legs
    q = sp - vec2(22.0, -60.0);
    q = rotate(q, -(0.5+0.5*cos(iTime))*0.1);
    q.x -= pow(max(-29.0-q.y, 0.0), 1.9)*0.1;
    d2 = sd_line(q, vec2(0.0,-35.0), vec2(0.0,0.0)) - 5.0;
    d = smin(d, d2, 1.0);

    // tail
    q = p - vec2(-26.0, -60.0);
    q = rotate(q, PI*0.7);
    q.x -= sin(q.y*0.123+3.0)*4.0 + sin(q.y*0.1+iTime*10.0);
    d2 = sd_line(q, vec2(0.0,-40.0), vec2(0.0,0.0)) - 5.0 * smoothstep(-60.0, 0.0, q.y);
    d = smin(d, d2, 1.0);
    ocol = blend(vec3(0.12), ocol, d);

    // tail skin
    d = sd_circle(q - vec2(-2.0, -4.0), 1.3);
    d = min(d, sd_circle(q - vec2(-2.0, -9.0), 1.2));
    d = min(d, sd_circle(q - vec2(-2.0, -15.0), 1.1));
    d = min(d, sd_circle(q - vec2(-2.0, -20.0), 1.0));
    d = min(d, sd_circle(q - vec2(-1.6, -25.0), 0.9));
    d = min(d, sd_circle(q - vec2(-1.3, -29.0), 0.8));
    d = min(d, sd_circle(q - vec2(-0.9, -33.0), 0.7));
    d = min(d, sd_circle(q - vec2(-0.8, -37.0), 0.6));
    ocol = blend(vec3(0.61,0.85,0.94), ocol, d);

    // face
    q = sp;
    q.y -= -11.0;
    d = sd_ellipsoid(q - vec2(0.0,-5.0), vec2(40.0, 20.0), 2.9);
    d = smin(d, sd_circle(q - vec2(21.0,0.0), 20.0), 5.0);
    ocol = blend(vec3(0.98, 0.79, 0.69), ocol, d);

    // nose
    d = sd_circle(q - vec2(0.0,-12.0+sin(iTime)*0.5), 2.0);

    // mouth
    d2 = sd_arc(q - vec2(0.0,-18.0), PI+sin(iTime)*0.3, 0.3, 4.0, 0.4);
    d = min(d, d2);
    ocol = blend(vec3(0.67, 0.36, 0.32), ocol, d);

    // eye white ball
    q = sp;
    q.y -= -12.0;
    q.y /= 1.0 - smoothstep(0.98,1.0,sin(iTime*3.0));
    d = sd_ellipsoid(q - vec2(20.0,0.0), vec2(9.0, 12.0), 2.0);
    ocol = blend(vec3(1.0), ocol, d);

    // eye retina
    d = sd_ellipsoid(q - vec2(19.5,0.0), vec2(5.0, 8.0), 2.0);
    ocol = blend(vec3(0.67, 0.36, 0.32), ocol, d);

    // eye reflection
    q = p - vec2(-2.0, 0.0);
    q.x = abs(q.x);
    d = sd_circle(q - vec2(19.5,-8.0), 1.3);
    ocol = blend(vec3(1.0), ocol, d);

    return ocol;
}

void main() {
    vec3 col = map(fragPos);
    fragColor = vec4(col,1.0);
}

@end

@program program vs fs
