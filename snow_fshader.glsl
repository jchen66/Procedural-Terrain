#version 330

in vec2 uv;

out vec3 color;

in vec4 vpoint_mv;
in vec3 light_dir, view_dir;
in vec3 vcolor;

uniform vec3 La = vec3(1.0f), Ld = vec3(1.0f), Ls = vec3(1.0f);
uniform vec3 ka = vec3(0.18f,0.1f,0.1f), kd = vec3(0.9f,0.5f,0.5f), ks = vec3(0.8f,0.8f,0.8f);
uniform float alpha = 60.0f;


void main() {
    color = vec3(1.0,1.0,1.0);

    // compute triangle normal using dFdx and dFdy
    vec3 x = dFdx(vpoint_mv.xyz) ;
    vec3 y = dFdy(vpoint_mv.xyz) ;
    vec3 n = normalize(cross(x, y));
    vec3 l = normalize(light_dir);
    
    // compute diffuse term.
    
    float lambert = dot(n, l);
    if (lambert > 0.0) {
        color += Ld  * kd * lambert;
    }
}

