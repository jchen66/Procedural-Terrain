// glew must be before glfw
#include <GL/glew.h>
#include <GLFW/glfw3.h>

// contains helper functions such as shader compiler
#include "icg_helper.h"
#include <glm/gtc/matrix_transform.hpp>

#include "framebuffer.h"
#include "heightmap/heightmap.h"
#include "cloudmap/cloudmap.h"
#include "noisefbm.h"

#include "cube/cube.h"
#include "tree/tree.h"
#include "grid/grid.h"
#include "sea/sea.h"
#include "snow/snow.h"


#include "trackball.h"

#define M_PI 3.1415926535897932384626433832795
#define NUM_TREE 50
#define NUM_SNOW 10000

// camera movement velocity (ws_speed,ad_speed,qe_speed)
vec3 cam_speed = vec3(0.0f); 
float time_cursor = 0.0f;


// 5, 0.2, 0.1, 10
float InitFreq = 5.0, Lacunarity = 0.2, Gain = 0.1;
int Octaves = 10;
vec4 Dir(0.0,1.0,0.0,0.0);
float Length = 0.03;
float Width = 0.001;
int Depth = 8;

// view matrix
vec3 Eye(1.0f, 1.0f, 1.0f); //(2,2,4)
vec3 Center(0.0f, 0.0f, 0.0f);
vec3 Up(0.0f, 1.0f, 0.0f);

Cube cube;
Grid grid, upcloud, downcloud;
Tree tree;
Sea sea;
Snow snow;

std::vector<float> snow_coord;

Noisefbm noisefbm;

int window_width = 800;
int window_height = 600;

FrameBuffer framebuffer, fb_cloud;
FrameBuffer fb_reflection, fb_refraction;

HeightMap heightmap;
CloudMap cloudmap;
GLuint framebuffer_texture_id, fb_tid_cloud, fb_tid_reflection, fb_tid_refraction;

using namespace glm;

mat4 projection_matrix;
mat4 view_matrix;
mat4 trackball_matrix;
mat4 old_trackball_matrix;
mat4 cube_scale;
mat4 quad_model_matrix;

Trackball trackball;

//FPS globals
GLfloat* pixels = new GLfloat[1025 * 1025];
//Bezier globals
bool BezierMode = false;
GLfloat BezierStep = 0.001f;
const int POINT_NUM = 13;
float xa = 1.4f, xb = 0.9f, xh = 0.1f;
vec3 BezierPoints[POINT_NUM] = {vec3(0.0f,xh,xa), vec3(xb,xh,xa),vec3(xa,xh,xb),vec3(xa,xh,0.0f),
                               vec3(xa,xh,-xb),vec3(xb,xh,-xa),vec3(0.0f,xh,-xa),
                               vec3(-xb,xh,-xa),vec3(-xa,xh,-xb),vec3(-xa,xh,0.0f),
                               vec3(-xa,xh,xb),vec3(-xb,xh,xa),vec3(0.0f,xh,xa)};


mat4 OrthographicProjection(float left, float right, float bottom,
                            float top, float near, float far) {
    assert(right > left);
    assert(far > near);
    assert(top > bottom);
    mat4 projection = mat4(1.0f);
    projection[0][0] = 2.0f / (right - left);
    projection[1][1] = 2.0f / (top - bottom);
    projection[2][2] = -2.0f / (far - near);
    projection[3][3] = 1.0f;
    projection[3][0] = -(right + left) / (right - left);
    projection[3][1] = -(top + bottom) / (top - bottom);
    projection[3][2] = -(far + near) / (far - near);
    return projection;
}

mat4 PerspectiveProjection(float fovy, float aspect, float near, float far) {
    // TODO 1: Create a perspective projection matrix given the field of view,
    // aspect ratio, and near and far plane distances.
    mat4 projection = IDENTITY_MATRIX;

    //determine the variables for the matrices
    float bottom= -near*(tanf((0.5f*fovy*M_PI)/180.0f));
    float top= -bottom;

    float left=aspect*bottom;
    float right=-left;

    //don't forget that this is column
    //change the last row of the matrix
    projection[3][3]=(float)0;
    projection[2][3]=(float)-1;

    //enter the rest of the vars
    projection[0][0]=(float)(2*near)/(float)(right-left);
    projection[2][0]=(float)(right+left)/(float)(right-left);

    projection[1][1]=(float)(2*near)/(float)(top-bottom);
    projection[2][1]=(float)(top+bottom)/(top-bottom);

    projection[2][2]=(float)-(far+near)/(float)(far-near);
    projection[3][2]=(float)-(2*far*near)/(float)(far-near);

    return projection;
}

mat4 LookAt(vec3 eye, vec3 center, vec3 up) {
    vec3 z_cam = normalize(eye - center);
    vec3 x_cam = normalize(cross(up, z_cam));
    vec3 y_cam = cross(z_cam, x_cam);

    mat3 R(x_cam, y_cam, z_cam);
    R = transpose(R);

    mat4 look_at(vec4(R[0], 0.0f),
                 vec4(R[1], 0.0f),
                 vec4(R[2], 0.0f),
                 vec4(-R * (eye), 1.0f));
    return look_at;
}


std::vector<float> above_coords(int max_points, int above_terrain){
    int point_left = max_points;
    std::vector<float> results;
    while (point_left > 0) {
        float x = float(rand())/RAND_MAX;
        
        float y = float(rand())/RAND_MAX;
        
        float z = 0.0;
        for (int i=2; i<20; i+=10) {
            z += (noisefbm.fBm(glm::vec2(x,y), i, 0.3, 1.5/pow(float(i),1.3), 10));
        }
        z = (0.5 - z)*1.0;

        // if above sea
        if (above_terrain == 0 && z > 0 && z < 0.1) {
            results.push_back(x);
            results.push_back(y);
            results.push_back(z);
            point_left --;
        } else if (above_terrain == 1 && z > 0) {
            // y = -y*2.0+1.0;
            // y += float(rand())/RAND_MAX * (1.0-y);
            // results.push_back(x*2.0-1.0);
            // results.push_back(z);
            // results.push_back(y);
            results.push_back( float(rand())/RAND_MAX*2.0-1.0 );
            results.push_back( float(rand())/RAND_MAX );
            results.push_back( float(rand())/RAND_MAX*2.0-1.0 );

            point_left --;
        }

    }
    return results;
}



void Init(GLFWwindow* window) {
    // sets background color
    glClearColor(0.937, 0.937, 0.937 /*gray*/, 1.0 /*solid*/);
    // enable depth test.
    glEnable(GL_DEPTH_TEST);

    // TODO 3: once you use the trackball, you should use a view matrix that
    // looks straight down the -z axis. Otherwise the trackball's rotation gets
    // applied in a rotated coordinate frame.
    // uncomment lower line to achieve this.
    view_matrix = LookAt(Eye, Center, Up);
    // view_matrix = translate(mat4(1.0f), vec3(0.0f, 0.0f, -4.0f));

    trackball_matrix = IDENTITY_MATRIX;

    // scaling matrix to scale the cube down to a reasonable size.
    float scale_of_cube = 5.0;
    cube_scale = mat4(scale_of_cube, 0.0f,  0.0f,  0.0f,
                      0.0f,  scale_of_cube, 0.0f,  0.0f,
                      0.0f,  0.0f,  scale_of_cube, 0.0f,
                      0.0f,  0.0f,  0.0f,  1.0f);
    quad_model_matrix = translate(mat4(1.0f), vec3(0.0f, -0.25f, 0.0f));

    // on retina/hidpi displays, pixels != screen coordinates
    // this unsures that the framebuffer has the same size as the window
    // (see http://www.glfw.org/docs/latest/window.html#window_fbsize)
    //glfwGetFramebufferSize(window, &window_width, &window_height);

    framebuffer_texture_id = framebuffer.Init(1024, 1024, true); //FPS-part
    //framebuffer_texture_id = framebuffer.Init(window_width, window_height);
    fb_tid_cloud = fb_cloud.Init(window_width, window_height);
    fb_tid_reflection = fb_reflection.Init(window_width, window_height);
    fb_tid_refraction = fb_refraction.Init(window_width, window_height);
    
    heightmap.Init(1024, 1024, framebuffer_texture_id); //FPS-part
    //heightmap.Init(window_width, window_height, framebuffer_texture_id);
    cloudmap.Init(window_width, window_height, fb_tid_cloud);
    
    cube.Init();
    grid.Init(framebuffer_texture_id);
    upcloud.Init(fb_tid_cloud);
    downcloud.Init(fb_tid_cloud);
    sea.Init(fb_tid_reflection, fb_tid_refraction, framebuffer_texture_id);


    tree.Init(above_coords(NUM_TREE, 0), Dir, Length, Width, Depth);
    snow_coord = above_coords(NUM_SNOW, 1);
    snow.Init(snow_coord);

}

// render height map
void RenderHeightMap() {
    // render a full screen quad
    framebuffer.Bind();
    {
         glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
         heightmap.Draw();
    }
    framebuffer.Unbind();
    // reset the screen size
}


// gets called for every frame.
void Display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    const float time = glfwGetTime();

     mat4 cube_transf = rotate(mat4(1.0f), 2.0f * time, vec3(0.0f, 1.0f, 0.0f));
     cube_transf = translate(cube_transf, vec3(0.75f, 0.0f, 0.0f));
     cube_transf = rotate(cube_transf, 2.0f * time, vec3(0.0f, 1.0f, 0.0f));

     mat4 cube_model_matrix = /*cube_transf*/ rotate(mat4(1.0f), 3.1415f/2.0f, vec3(1.0f, 0.0f, 0.0f)) * cube_scale;

     cube.Draw(time, 0, projection_matrix * view_matrix * trackball_matrix * cube_model_matrix); // mvp here is pvm

    // draw a quad on the ground.
    // heightmap.Draw();

    grid.Draw(time, 0, trackball_matrix * quad_model_matrix, view_matrix, projection_matrix);
    upcloud.Draw(time, -1, trackball_matrix * quad_model_matrix, view_matrix, projection_matrix);
    downcloud.Draw(time, -2, trackball_matrix * quad_model_matrix, view_matrix, projection_matrix);

    tree.Draw(0, trackball_matrix * quad_model_matrix, view_matrix, projection_matrix);
    
    // glEnable( GL_POINT_SMOOTH ); // make the point circular
    snow.Draw(0, time, trackball_matrix * quad_model_matrix, view_matrix, projection_matrix);
    // glDisable( GL_POINT_SMOOTH);

    mat4 reflect_scale = mat4(1.0f, 0.0f,  0.0f,  0.0f,
                        0.0f,  -1.0f, 0.0f,  0.0f,
                        0.0f,  0.0f,  1.0f, 0.0f,
                        0.0f,  0.0f,  0.0f,  1.0f);
    mat4 swap_yz = mat4(1.0f, 0.0f,  0.0f,  0.0f,
                        0.0f,  0.0f, 1.0f,  0.0f,
                        0.0f,  -1.0f,  0.0f, 0.0f,
                        0.0f,  0.0f,  0.0f,  1.0f);
    // grid.Draw(time, 1,  trackball_matrix * quad_model_matrix *  reflect_scale, view_matrix, projection_matrix);
    {
//        double plane[4] = {0.0, 1.0, 0.0, 0.0}; //water at y=0
//        glClipPlane(GL_CLIP_PLANE0, plane);
       glEnable(GL_CLIP_PLANE0); // this is necessary
//        grid.Draw(time, trackball_matrix * quad_model_matrix, view_matrix, projection_matrix );
//        glDisable(GL_CLIP_PLANE0);


        //reflection
        fb_reflection.Bind();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        cube.Draw(time, 1, projection_matrix * view_matrix * trackball_matrix * cube_model_matrix * reflect_scale); // mvp here is pvm
        tree.Draw(1, trackball_matrix * quad_model_matrix * reflect_scale, view_matrix, projection_matrix);
        grid.Draw(time, 1,  trackball_matrix * quad_model_matrix * reflect_scale, view_matrix, projection_matrix);
        //snow.Draw(1, time, trackball_matrix * quad_model_matrix * reflect_scale, view_matrix, projection_matrix);
        //upcloud.Draw(time, 1,  trackball_matrix * quad_model_matrix * reflect_scale, view_matrix, projection_matrix);
        //downcloud.Draw(time, 1,  trackball_matrix * quad_model_matrix * reflect_scale, view_matrix, projection_matrix);
        fb_reflection.Unbind();

        // refraction
        fb_refraction.Bind();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        grid.Draw(time, 2, trackball_matrix * quad_model_matrix , view_matrix, projection_matrix);
        fb_refraction.Unbind();


    }


    mat4 translation_sea = mat4(1.0f, 0.0f, 0.0f, 0.0f,
                                0.0f, 1.0f, 0.0f, 0.0f,
                                0.0f, 0.0f, 1.0f, 0.0f,
                                0.0f, 0.0f, 0.0f, 1.0f);
    sea.Draw(time, trackball_matrix * quad_model_matrix * translation_sea, view_matrix, projection_matrix);

    check_error_gl();
    
}

// transforms glfw screen coordinates into normalized OpenGL coordinates.
vec2 TransformScreenCoords(GLFWwindow* window, int x, int y) {
    // the framebuffer and the window doesn't necessarily have the same size
    // i.e. hidpi screens. so we need to get the correct one
    int width;
    int height;
    glfwGetWindowSize(window, &width, &height);
    return vec2(2.0f * (float)x / width - 1.0f,
                1.0f - 2.0f * (float)y / height);
}

void MouseButton(GLFWwindow* window, int button, int action, int mod) {
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        double x_i, y_i;
        glfwGetCursorPos(window, &x_i, &y_i);
        vec2 p = TransformScreenCoords(window, x_i, y_i);
        trackball.BeingDrag(p.x, p.y);
        old_trackball_matrix = trackball_matrix;
        // Store the current state of the model matrix.
    }
}

void MousePos(GLFWwindow* window, double x, double y) {
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
        vec2 p = TransformScreenCoords(window, x, y);
        // TODO 3: Calculate 'trackball_matrix' given the return value of
        // trackball.Drag(...) and the value stored in 'old_trackball_matrix'.
        // See also the mouse_button(...) function.
        // trackball_matrix = ...
        trackball_matrix = trackball.Drag(p.x, p.y) * old_trackball_matrix;
    }

    // zoom
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS) {
        // TODO 4: Implement zooming. When the right mouse button is pressed,
        // moving the mouse cursor up and down (along the screen's y axis)
        // should zoom out and it. For that you have to update the current
        // 'view_matrix' with a translation along the z axis.
        // view_matrix = ...
        vec2 p = TransformScreenCoords(window, x, y);
        view_matrix = translate(mat4(1.0f), vec3(0.0f, 0.0f, 0.1f * p.y)) * view_matrix;

    }
}

// Gets called when the windows/framebuffer is resized.
void SetupProjection(GLFWwindow* window, int width, int height) {
    window_width = width;
    window_height = height;

    cout << "Window has been resized to "
         << window_width << "x" << window_height << "." << endl;

    glViewport(0, 0, window_width, window_height);

    // TODO 1: Use a perspective projection instead;
    //GLfloat top = 1.0f;
    //GLfloat right = (GLfloat)window_width / window_height * top;
    //projection_matrix = OrthographicProjection(-right, right, -top, top, -10.0, 10.0f);
    projection_matrix = PerspectiveProjection(45.0f,(GLfloat)window_width / window_height,0.1f, 100.0f);
}

void ErrorCallback(int error, const char* description) {
    fputs(description, stderr);
}


vec3 Rodrigues(vec3 p, vec3 vv, vec3 q, float t) {
    vec3 vector = normalize(vv);
    float u = vector.x;
    float v = vector.y;
    float w = vector.z;
    float a = p.x;
    float b = p.y;
    float c = p.z;
    float x = q.x;
    float y = q.y;
    float z = q.z;

    return vec3((a*(v*v+w*w)-u*(b*v+c*w-u*x-v*y-w*z))*(1-cos(t))+x*cos(t)+(-c*v+b*w-w*y+v*z)*sin(t),
                (b*(u*u+w*w)-v*(a*u+c*w-u*x-v*y-w*z))*(1-cos(t))+y*cos(t)+( c*u-a*w+w*x-u*z)*sin(t),
                (c*(u*u+v*v)-w*(a*u+b*v-u*x-v*y-w*z))*(1-cos(t))+z*cos(t)+(-b*u+a*v-v*x+u*y)*sin(t));
}

//speeds: (ws, ad, qe)
void cam_move(vec3 speeds) {
    //ws-speed, w+s-
    vec3 move_distance = speeds[0] * (Eye-Center) / float(distance(Eye,Center));
    Eye -= move_distance;
    Center -= move_distance;
    //ad-speed, a+d-
    Center = Rodrigues(Eye, Up, Center, speeds[1]);
    //qe-speed, q+e-
    vec3 oldCenter = Center;
    Center = Rodrigues(Eye, cross(Center-Eye, Up), Center, speeds[2]);
    Up  = Rodrigues(Eye, cross(oldCenter-Eye, Up), Up + Eye, speeds[2]) - Eye;

    //printf("Eye:(%f, %f, %f) Center:(%f, %f, %f) Up:(%f, %f, %f) Speed:(%f, %f, %f)\n",
    //    Eye.x,Eye.y,Eye.z,Center.x,Center.y,Center.z,Up.x,Up.y,Up.z,speeds.x,speeds.y,speeds.z);
    view_matrix = LookAt(Eye, Center, Up);
}

void fps() {
    if (Eye.x <= 1.0f && Eye.x >= -1.0f &&
        Eye.z <= 1.0f && Eye.z >= -1.0f) {

        float x = (Eye.x+1.0f)/2.0f;
        float y = (-Eye.z+1.0f)/2.0f;
        float z = 0.0;
        //fbm
        for (int i=2; i<20; i+=10) {
            z += (noisefbm.fBm(glm::vec2(x,y), i, 0.3, 1.5/pow(float(i),1.3), 10));
        }
        z = (0.5 - z)*1.0;

        // GLint x = static_cast<int>((Eye.x+1.0f)*512);
        // GLint y = static_cast<int>((Eye.z+1.0f)*512);
        // GLfloat h = pixels[y*1025 + x];
        // //h = (h < 0.0f) ? 0.0f : h;
        // Eye.y = h;

        Center.y += z-Eye.y;
        Eye.y = z;
        //view_matrix = LookAt(Eye, Center, Up);

        //printf("h:  %f\n", Eye.y);
    }
}

vec3 CurvePoint(vec3 points[], GLfloat t, int count) {
    if(count == 1){
        return points[0];
    }
    else{
        GLfloat x, y, z;
        vec3 * newpoints = new vec3[count];
        for(int i=0; i<count; i++){
            x = (1-t) * points[i].x + t * points[i+1].x;
            y = (1-t) * points[i].y + t * points[i+1].y;
            z = (1-t) * points[i].z + t * points[i+1].z;
            newpoints[i] = vec3(x, y, z);
        }
        return CurvePoint(newpoints, t, count - 1);
    }
}

void BezierCurve(vec3 points[], GLFWwindow* window) {
    GLfloat t = 0.0f;
    GLfloat time = glfwGetTime();
    while (t <= 1.0f) {
        if(!BezierMode){
            break;
        }
        Display();
        glfwSwapBuffers(window);
        glfwPollEvents();
        if (glfwGetTime() - time >= 0.00001f) {
            Eye = CurvePoint(points, t, POINT_NUM);
            //printf("midEye: (%f, %f, %f)\n", Eye.x,Eye.y,Eye.z);
            view_matrix = LookAt(Eye, Center, Up);
            t += BezierStep;
        }
        time = glfwGetTime();
    }
}

void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GL_TRUE);
    }
    if (BezierMode) {
        switch(key) {
            case GLFW_KEY_B:{
                if(action == GLFW_PRESS){
                    BezierMode = false;
                    printf("Normal mode\n");
                }
            }
            break;
            case GLFW_KEY_W:
                BezierStep += 0.001f;
            break;
            case GLFW_KEY_S:
                BezierStep -= 0.001f;
            break;
        }
    } else {
        switch(key) {
            case GLFW_KEY_B:{
                if(action == GLFW_PRESS){
                    BezierMode = true;
                    printf("Bezier mode\n");
                }
            }
            break;
            case GLFW_KEY_W:{
                //accelerate until button released
                fps();
                cam_speed[0] += 0.006f;
                cam_move(cam_speed);
            }
            break;
            case GLFW_KEY_S:{
                //accelerate until button released
                fps();
                cam_speed[0] -= 0.006f;
                cam_move(cam_speed);
            }
            break;
            case GLFW_KEY_A:{
                //accelerate until button released
                cam_speed[1] += 0.01f;
                cam_move(cam_speed);
            }
            break;
            case GLFW_KEY_D:{
                //accelerate until button released
                cam_speed[1] -= 0.01f;
                cam_move(cam_speed);
            }
            break;
            case GLFW_KEY_Q:{
                //accelerate until button released
                cam_speed[2] += 0.01f;
                cam_move(cam_speed);
            }
            break;
            case GLFW_KEY_E:{
                //accelerate until button released
                cam_speed[2] -= 0.01f;
                cam_move(cam_speed);
            }


            // case GLFW_KEY_RIGHT:
            //     fps();
            //     Eye.x += 0.05f;
            //     Center.x += 0.05f;
            //     printf("Eye:(%f, %f, %f) Center:(%f, %f, %f) Up:(%f, %f, %f)\n",Eye.x,Eye.y,Eye.z,Center.x,Center.y,Center.z,Up.x,Up.y,Up.z);
            //     view_matrix = LookAt(Eye, Center, Up);
            // break;
            // case GLFW_KEY_LEFT: 
            //     fps();
            //     Eye.x -= 0.05f;
            //     Center.x -= 0.05f;
            //     printf("Eye:(%f, %f, %f) Center:(%f, %f, %f) Up:(%f, %f, %f)\n",Eye.x,Eye.y,Eye.z,Center.x,Center.y,Center.z,Up.x,Up.y,Up.z);
            //     view_matrix = LookAt(Eye, Center, Up);
            // break;
            // case GLFW_KEY_UP:
            //     fps();
            //     Eye.z -= 0.05f;
            //     Center.z -= 0.05f;
            //     printf("Eye:(%f, %f, %f) Center:(%f, %f, %f) Up:(%f, %f, %f)\n",Eye.x,Eye.y,Eye.z,Center.x,Center.y,Center.z,Up.x,Up.y,Up.z);
            //     view_matrix = LookAt(Eye, Center, Up);
            // break;
            // case GLFW_KEY_DOWN: 
            //     fps();
            //     Eye.z += 0.05f;
            //     Center.z += 0.05f;
            //     printf("Eye:(%f, %f, %f) Center:(%f, %f, %f) Up:(%f, %f, %f)\n",Eye.x,Eye.y,Eye.z,Center.x,Center.y,Center.z,Up.x,Up.y,Up.z);
            //     view_matrix = LookAt(Eye, Center, Up);
            // break;
        }
    }
}


int main(int argc, char *argv[]) {
    // GLFW Initialization
    if(!glfwInit()) {
        fprintf(stderr, "Failed to initialize GLFW\n");
        return EXIT_FAILURE;
    }

    glfwSetErrorCallback(ErrorCallback);

    // hint GLFW that we would like an OpenGL 3 context (at least)
    // http://www.glfw.org/faq.html#how-do-i-create-an-opengl-30-context
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // attempt to open the window: fails if required version unavailable
    // note some Intel GPUs do not support OpenGL 3.2
    // note update the driver of your graphic card
    GLFWwindow* window = glfwCreateWindow(window_width, window_height,
                                          "Trackball", NULL, NULL);
    if(!window) {
        glfwTerminate();
        return EXIT_FAILURE;
    }

    // makes the OpenGL context of window current on the calling thread
    glfwMakeContextCurrent(window);

    // set the callback for escape key
    glfwSetKeyCallback(window, KeyCallback);

    // time_cursor = glfwGetTime();
    // while (cam_speed[0] != 0 || ) {
    //     if ((glfwGetTime() - time_cursor) >= 0.9f) {
    //         //move camera
    //         cam_move(cam_speed);
    //         //update time and speed
    //         time_cursor = glfwGetTime();
    //         cam_speed[0] -= 0.005f;
    //     }
    // }
    // cam_speed[0] = 0.0f;

    // set the framebuffer resize callback
    glfwSetFramebufferSizeCallback(window, SetupProjection);

    // set the mouse press and position callback
    glfwSetMouseButtonCallback(window, MouseButton);
    glfwSetCursorPosCallback(window, MousePos);

    // GLEW Initialization (must have a context)
    // https://www.opengl.org/wiki/OpenGL_Loading_Library
    glewExperimental = GL_TRUE; // fixes glew error (see above link)
    if(glewInit() != GLEW_NO_ERROR) {
        fprintf( stderr, "Failed to initialize GLEW\n");
        return EXIT_FAILURE;
    }
    check_error_gl();

    cout << "OpenGL" << glGetString(GL_VERSION) << endl;

    // initialize our OpenGL program
    Init(window);

    // update the window size with the framebuffer size (on hidpi screens the
    // framebuffer is bigger)
    glfwGetFramebufferSize(window, &window_width, &window_height);
    SetupProjection(window, window_width, window_height);

    RenderHeightMap();

    // read texture rendered in the framebuffer --- FPS part
    glBindTexture(GL_TEXTURE_2D, framebuffer_texture_id);
    glGetTexImage(GL_TEXTURE_2D, 0, GL_RED/*GL_RGBA*/, GL_FLOAT, pixels);


    {
        //cloud rendering
        fb_cloud.Bind();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        cloudmap.Draw();
        fb_cloud.Unbind();
    }


    // render loop
    while(!glfwWindowShouldClose(window)){
        if(BezierMode){
            Up = vec3(0.0f,1.0f,0.0f);
            Center = vec3(0.0f);
            BezierCurve(BezierPoints,window);
        }
        if (cam_speed != vec3(0.0f)) {
            for (int i=0; i<3; i++) {
                if (cam_speed[i] > 0.0f) {
                    cam_speed[i] -= min(cam_speed, vec3(0.005))[i];
                } else if (cam_speed[i] < 0.0f) {
                    cam_speed[i] -= max(cam_speed, vec3(-0.005))[i];
                }
            }
            fps();
            cam_move(cam_speed);
            //printf("Speed:(%f, %f, %f)\n",cam_speed.x,cam_speed.y,cam_speed.z);
        } 
        Display();
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    grid.Cleanup();
    upcloud.Cleanup();
    downcloud.Cleanup();
    cube.Cleanup();
    tree.Cleanup();
    sea.Cleanup();
    snow.Cleanup();


    heightmap.Cleanup();
    cloudmap.Cleanup();

    framebuffer.Cleanup();
    fb_cloud.Cleanup();
    fb_reflection.Cleanup();
    fb_refraction.Cleanup();



    // close OpenGL window and terminate GLFW
    glfwDestroyWindow(window);
    glfwTerminate();
    return EXIT_SUCCESS;
}
