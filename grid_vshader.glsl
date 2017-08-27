#version 330

in vec2 position;

out vec2 uv;
//out vec3 vcolor;

uniform mat4 MVP;
uniform mat4 MV;
uniform float time;

uniform sampler2D heightmap;
uniform sampler2D cloud;

uniform int grid_length = 16;

//diffuse shading
out vec4 vpoint_mv;
out vec3 light_dir, view_dir;
out float height;

uniform mat4 projection;
uniform vec3 light_pos = vec3(0.0f, 0.0f, 2.0f);

uniform int clip_plane_no;
uniform vec4 u_plane0 = vec4(0,1,0,0); // xz-plane
uniform vec4 u_plane1 = vec4(0,-1,0,0); // xz-plane
uniform float cloud_mid_height = 0.5;


void main() {
    vec4 u_plane_1 = vec4(0, 1,0,-cloud_mid_height); 
    vec4 u_plane_2 = vec4(0,-1,0,cloud_mid_height);


    uv = (position + vec2(1.0, 1.0)) * 0.5;

    height = texture(heightmap, uv).r;

    if (clip_plane_no == -1 ) {
        height += cloud_mid_height;
        //height = cloud_mid_height + texture(cloud, uv).r * 0.1;
    } else if (clip_plane_no == -2) {
        height *= -0.5;
        height += cloud_mid_height;
        //height = cloud_mid_height - 0.5 * texture(cloud,uv).r * 0.1;
    }

//    vcolor = vec3(height*0.1,height*0.5,height*0.1);
//    if (height == 0) {
//        vcolor = vec3(0.0,0.5,1.0);
//    }

    vec3 pos_3d = vec3(position.x, height, -position.y);

    gl_Position = MVP * vec4(pos_3d, 1.0);

    if (clip_plane_no == 1){
        gl_ClipDistance[0] = dot(u_plane0,vec4(pos_3d,1.0));
    } else if (clip_plane_no == 2){
        gl_ClipDistance[0] = dot(u_plane1,vec4(pos_3d,1.0));
    } else if (clip_plane_no == -1){
        gl_ClipDistance[0] = dot(u_plane_1,vec4(pos_3d,1.0));
    } else if (clip_plane_no == -2){
        gl_ClipDistance[0] = dot(u_plane_2,vec4(pos_3d,1.0));
    } 

    //diffuse shading

    vpoint_mv = MV * vec4(pos_3d, 1.0);

    // 1) compute the light direction light_dir.
    light_dir = light_pos - vpoint_mv.xyz;
    // 2) compute the view direction view_dir.
    view_dir = - vpoint_mv.xyz;
    
}
