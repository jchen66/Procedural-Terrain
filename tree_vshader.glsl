#version 330

in vec3 vpoint;
in vec2 vtexcoord;

out vec2 uv;

//diffuse shading
out vec4 vpoint_mv;
out vec3 light_dir, view_dir;
uniform mat4 MVP;
uniform mat4 MV;
uniform mat4 projection;
uniform vec3 light_pos = vec3(0.0f, 0.0f, 2.0f);

uniform int clip_plane_no;
uniform vec4 u_plane0 = vec4(0,1,0,0); // xz-plane

void main() {
    gl_Position = vec4(vpoint, 1.0);
    uv = vtexcoord;

    gl_Position = MVP * vec4(vpoint, 1.0);

    if (clip_plane_no == 1){
        gl_ClipDistance[0] = dot(u_plane0,vec4(vpoint,1.0));
    }

    //diffuse shading

    vpoint_mv = MV * vec4(vpoint, 1.0);

    // 1) compute the light direction light_dir.
    light_dir = light_pos - vpoint_mv.xyz;
    // 2) compute the view direction view_dir.
    view_dir = - vpoint_mv.xyz;
}
