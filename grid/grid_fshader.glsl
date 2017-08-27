#version 330

uniform sampler2D tex;
uniform float time;
uniform sampler2D grass, rock, sand, snow, cloud;

in vec4 vpoint_mv;
in vec3 light_dir, view_dir;
in vec2 uv;

//in vec3 vcolor;
in float height;

out vec3 color;

uniform vec3 La = vec3(1.0f), Ld = vec3(1.0f), Ls = vec3(1.0f);
uniform vec3 ka = vec3(0.18f,0.1f,0.1f), kd = vec3(0.9f,0.5f,0.5f), ks = vec3(0.8f,0.8f,0.8f);
uniform float alpha = 60.0f;
uniform int clip_plane_no;

void main() {
    //color = vcolor;
    vec3 basecolor = vec3(height*0.1,height*0.5,height*0.1); 

    if (clip_plane_no == -1 || clip_plane_no == -2) {
        basecolor = vec3(1.0,1.0,1.0);
    } 

    color = basecolor;

    // compute triangle normal using dFdx and dFdy
    vec3 x = dFdx(vpoint_mv.xyz) ;
    vec3 y = dFdy(vpoint_mv.xyz) ;
    vec3 n = normalize(cross(x, y));
    vec3 l = normalize(light_dir);
    
    // compute diffuse term.
    
    

    
    float lambert = dot(n, l);
    if (lambert > 0.0) {
        color = Ld  * basecolor * lambert;
        
    }

    if (clip_plane_no >= 0){
        float pg = max(2.0-height*5*2,0.0) * 0.5,
              pr = max(height*5*2-1.0,0.0) * 0.5,
              psnow = max(height*4*2-1.0,0.0),
              psand = max(1.0 - pg - pr - psnow, 0);

        vec3 from_tex = pg * texture(grass, uv*30).rgb + pr* texture(rock, uv).rgb + psnow * texture(snow,uv*30).rgb + psand*texture(sand, uv*60).rgb;
        if(height < 0) {
            from_tex = texture(sand,uv*60).rgb;
        }
        color = mix(color, from_tex, vec3(0.85));
    } else if (clip_plane_no == -1 || clip_plane_no == -2) {
        color = mix(color, texture(cloud, uv).rgb, vec3(0.85));
    }
//    float offset = cos(time);// sqrt(length(x)*length(x)+length(y)*length(y));

//    color = mix(color, vec3(height+offset/height), vec3(0.15));
//    color = vec3(offset);
}

