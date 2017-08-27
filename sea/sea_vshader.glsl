#version 330

in vec2 position;

out vec2 uv;

uniform mat4 MVP;
uniform mat4 MV;
uniform float time;
uniform sampler2D texrefl;

uniform int grid_length = 16;

//diffuse shading
out vec4 vpoint_mv;
out vec3 light_dir, view_dir;
out vec4 pos;

uniform mat4 projection;
uniform vec3 light_pos = vec3(0.0f, 0.0f, 2.0f);

void main() {
    uv = (position + vec2(1.0, 1.0)) * 0.5;

    float height = 0.0;//0.01 * ( cos(5*time*(uv[0]+uv[1])) + cos(5*time*(uv[0]-uv[1])) + cos(5*time*(uv[0]*uv[0]+uv[1]*uv[1])) );//texture(heightmap, uv).r;

    vec3 pos_3d = vec3(position.x, height, -position.y);

    gl_Position = MVP * vec4(pos_3d, 1.0);

    pos = MVP * vec4(pos_3d, 1.0);
    //diffuse shading

    vpoint_mv = MV * vec4(pos_3d, 1.0);

    // 1) compute the light direction light_dir.
    light_dir = light_pos - vpoint_mv.xyz;
    // 2) compute the view direction view_dir.
    view_dir = - vpoint_mv.xyz;
    
}
