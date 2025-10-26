float sdf(in vec3 pos){
    pos = mod(pos, 10.);
    return length(pos - vec3(5.)) - 1.;
}

void mainImage( out vec4 fragColor, in vec2 fragCoord )
{
    vec2 uv = (fragCoord * 2. - iResolution.xy)/max(iResolution.x, iResolution.y);

    // Move and rotate camera over time
    vec3 origin = vec3(0., 5., 0.) * iTime;
    float angle = radians(iTime*3.);
    uv *= mat2(cos(angle), -sin(angle), sin(angle), cos(angle));
    
    // Use spherical projection for ray direction
    vec3 ray_dir = vec3(sin(uv.x), cos(uv.x)*cos(uv.y), sin(uv.y));
    vec3 ray_pos = vec3(origin);
    
    float ray_length = 0.;
    
    for(float i = 0.; i < 7.; i++){
        float dist = sdf(ray_pos);
        ray_length += dist;
        ray_pos += ray_dir * dist;
        // Push rays outward with increasing distance
        ray_dir = normalize(ray_dir + vec3(uv.x, 0., uv.y) * dist * .3);
    }
    
    vec3 o = vec3(sdf(ray_pos));
    o = cos(o + vec3(6.,0,.5));
    o *= smoothstep(38., 20., ray_length);

    fragColor = vec4(o, 1.);
}