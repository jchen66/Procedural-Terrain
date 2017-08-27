#pragma once
#include "icg_helper.h"
#include <glm/gtc/matrix_transform.hpp>

using namespace glm;

class Trackball {
public:
    Trackball() : radius_(1.0f) {}

    // this function is called when the user presses the left mouse button down.
    // x, and y are in [-1, 1]. (-1, -1) is the bottom left corner while (1, 1)
    // is the top right corner.
    void BeingDrag(float x, float y) {
      anchor_pos_ = vec3(x, y, 0.0f);
      ProjectOntoSurface(anchor_pos_);
    }

    // this function is called while the user moves the curser around while the
    // left mouse button is still pressed.
    // x, and y are in [-1, 1]. (-1, -1) is the bottom left corner while (1, 1)
    // is the top right corner.
    // returns the rotation of the trackball in matrix form.
    mat4 Drag(float x, float y) {
      vec3 current_pos = vec3(x, y, 0.0f);
      ProjectOntoSurface(current_pos);
      // TODO 3: Calculate the rotation given the projections of the anocher
      // point and the current position. The rotation axis is given by the cross
      // product of the two projected points, and the angle between them can be
      // used as the magnitude of the rotation.
      // you might want to scale the rotation magnitude by a scalar factor.
      // p.s. No need for using complicated quaternions as suggested in the wiki
      // article.
      vec3 v = normalize(anchor_pos_); //m0
      vec3 w = normalize(current_pos); //m1
      vec3 n = cross(v, w);
      float theta =  -acos(dot(v, w));

      mat3 N = mat3(0.0f);
      N[0][1] = - n.z; N[0][2] = n.y;
      N[1][0] = n.z;   N[1][2] = - n.x;
      N[2][0] = - n.y; N[2][1] = n.x;
      mat3 nnT = { n * n.x, n * n.y, n* n.z};
      mat3 rotation3x3 = cos(theta) * mat3(1.0f) + (1 - cos(theta)) * nnT + sin(theta) * N;
      mat4 rotation = mat4(rotation3x3);

      return rotation;
    }

private:
    // projects the point p (whose z coordiante is still empty/zero) onto the
    // trackball surface. If the position at the mouse cursor is outside the
    // trackball, use a hyberbolic sheet as explained in:
    // https://www.opengl.org/wiki/Object_Mouse_Trackball.
    // The trackball radius is given by 'radius_'.
    void ProjectOntoSurface(vec3& p) const {
      // TODO 2: Implement this function. Read above link for details.
      float x = p.x; float y = p.y;
      p.z = (x * x + y * y <= radius_ * radius_ / 2.0f) ?
                            sqrt(radius_ * radius_ - x * x - y * y) :
                            (radius_ * radius_ / 2.0f) / sqrt(x * x + y * y);
    }

    float radius_;
    vec3 anchor_pos_;
    mat4 rotation_;
};
