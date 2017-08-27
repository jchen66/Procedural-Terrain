#version 330

in vec3 vpoint;
in vec2 vtexcoord;

out vec2 uv;

//diffuse shading
out vec4 vpoint_mv;
out vec3 light_dir, view_dir;

uniform mat4 MVP;
uniform mat4 MV;
uniform float time;
uniform mat4 projection;
uniform vec3 light_pos = vec3(0.0f, 0.0f, 2.0f);

uniform int clip_plane_no;
uniform vec4 u_plane0 = vec4(0,1,0,0); // xz-plane

uniform float acc = 1.0;
uniform float delta_time = 0.1;
uniform float v0 = 0.3;

void main() {
    gl_Position = vec4(vpoint, 1.0);

    //height
    float height = vpoint[1];//cos(time_diff);

    float current_v = sqrt(2*acc*(1.0-height)+v0*v0);
    float max_v = sqrt(2.0*acc*1.0+v0*v0);
    float period = 1.0*2.0/(max_v+v0);
    float fly_time = mod(time,period);
    float touch_bottom_time = 2.0*height/(current_v+max_v);


    /*for(float i=0; i<time; i+=0.1) {
        height -= current_v*0.1;
        current_v += acc*0.1;
        if (height <= 0.0) {
            height = 1.0;
            current_v = 0.5;
        }
    }*/


    if (fly_time <= touch_bottom_time) {
        height -= current_v*fly_time+acc*fly_time*fly_time/2.0;
    } else {
        height = 1.0 - v0*(fly_time-touch_bottom_time) - acc*(fly_time-touch_bottom_time)*(fly_time-touch_bottom_time)/2.0;
    }
    
    // x and z coordinates
    float x = vpoint[0] + cos(time)*(1.0-height)/5;
    float z = vpoint[2] + sin(time)*(1.0-height)/5;


    uv = vtexcoord;

    gl_Position = MVP * vec4(x, height, z, 1.0);

    //if (clip_plane_no == 1){
    //    gl_ClipDistance[0] = dot(u_plane0,vec4(vpoint,1.0));
    //}



    //diffuse shading

    vpoint_mv = MV * vec4(x, height, z, 1.0);

    // 1) compute the light direction light_dir.
    light_dir = light_pos - vpoint_mv.xyz;
    // 2) compute the view direction view_dir.
    view_dir = - vpoint_mv.xyz;
}
