#pragma once
#include "icg_helper.h"

class Tree {

    private:
        GLuint vertex_array_id_;        // vertex array object
        GLuint program_id_;             // GLSL shader program ID
        GLuint vertex_buffer_object_;   // memory buffer
        GLuint texture_id_;             // texture ID
        GLuint num_vertices = 0;
        GLuint MVP_id_;                         // model, view, proj matrix ID
        GLuint MV_id_;
        std::vector<GLfloat> vertices;
        std::vector<float> start_points;

    public:
        void InitTree(glm::vec4 spoint, glm::vec4 dir, float length, float width, int depth) {
            if (depth >= 1) {
                glm::vec4 p1 = spoint - glm::vec4(0.0, -width/2, 0.0, 0.0);
                glm::vec4 p2 = p1 + dir*length;
                glm::vec4 p3 = p2 + glm::vec4(0.0, width, width, 0.0);
                glm::vec4 p4 = p1 + glm::vec4(0.0, width, 0.0, 0.0);

                //vertices
                vertices.push_back(p1[0]);
                vertices.push_back(p1[1]);
                vertices.push_back(p1[2]);

                vertices.push_back(p2[0]);
                vertices.push_back(p2[1]);
                vertices.push_back(p2[2]);

                vertices.push_back(p3[0]);
                vertices.push_back(p3[1]);
                vertices.push_back(p3[2]);

                vertices.push_back(p1[0]);
                vertices.push_back(p1[1]);
                vertices.push_back(p1[2]);

                vertices.push_back(p3[0]);
                vertices.push_back(p3[1]);
                vertices.push_back(p3[2]);

                vertices.push_back(p4[0]);
                vertices.push_back(p4[1]);
                vertices.push_back(p4[2]);

                num_vertices += 6;
            }

            if (depth > 1) {
                float angle = float(rand())/RAND_MAX * 3.14 / 2.0;
                float scale = 1.5;
                //x-rotate
                glm::mat4 r_matX = glm::mat4(1.0f);
                r_matX[1][1] = cos(angle);
                r_matX[1][2] = sin(angle);
                r_matX[2][1] = -sin(angle);
                r_matX[2][2] = cos(angle);
                //y-rotate
                glm::mat4 r_matY = glm::mat4(1.0f);
                angle *= 2.0;
                r_matY[0][0] = cos(angle);
                r_matY[2][0] = sin(angle);
                r_matY[0][2] = -sin(angle);
                r_matY[2][2] = cos(angle);
                //z-rotate
                glm::mat4 r_matZ = glm::mat4(1.0f);
                r_matZ[0][0] = cos(angle);
                r_matZ[0][1] = sin(angle);
                r_matZ[1][0] = -sin(angle);
                r_matZ[1][1] = cos(angle);

                InitTree(spoint+dir*length, glm::normalize(dir*r_matX), length/scale, width/scale, depth-1);
                InitTree(spoint+dir*length, glm::normalize(dir*r_matX*r_matY), length/scale, width/scale, depth-1);
                InitTree(spoint+dir*length, glm::normalize(dir*r_matX*r_matY*r_matY), length/scale, width/scale, depth-1);

            }
        }
        void Init(std::vector<float> spoints, glm::vec4 dir, float length, float width, int depth) {

     
            // compile the shaders
            program_id_ = icg_helper::LoadShaders("tree_vshader.glsl",
                                                  "tree_fshader.glsl");
            if(!program_id_) {
                exit(EXIT_FAILURE);
            }

            glUseProgram(program_id_);

            // vertex one vertex Array
            glGenVertexArrays(1, &vertex_array_id_);
            glBindVertexArray(vertex_array_id_);

            // vertex coordinates
            {   
                for (int i=0; i<spoints.size(); i+=3) {
                    InitTree(glm::vec4(spoints[i]*2-1.0,spoints[i+2],-(spoints[i+1]*2-1.0),1.0), dir, length, width, depth);
                }

                // GLfloat vertex_point[] = { /*V1*/ -0.5f, -0.0f, 0.0f,
                //                                  /*V2*/ +0.5f, -0.0f, 0.0f,
                //                                  /*V3*/ -0.5f, +0.5f, 0.0f,
                //                                  /*V4*/ +1.0f, +1.0f, 0.0f,
                //                                         +0.5f, +1.0f, 0.0f,
                //                                         +1.0f, +0.5f, 0.0f};
                // num_vertices = 3;
                // buffer
                glGenBuffers(1, &vertex_buffer_object_);
                glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_object_);
                glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(GLfloat)/*sizeof(vertex_point)*/,
                             &vertices[0]/*vertex_point*/, GL_STATIC_DRAW);

                // attribute
                GLuint vertex_point_id = glGetAttribLocation(program_id_, "vpoint");
                glEnableVertexAttribArray(vertex_point_id);
                glVertexAttribPointer(vertex_point_id, 3, GL_FLOAT, DONT_NORMALIZE,
                                      ZERO_STRIDE, ZERO_BUFFER_OFFSET);
            }

            // texture coordinates
            {
                const GLfloat vertex_texture_coordinates[] = { /*V1*/ 0.0f, 0.0f,
                                                               /*V2*/ 1.0f, 0.0f,
                                                               /*V3*/ 0.0f, 1.0f,
                                                               /*V4*/ 1.0f, 1.0f};

                // buffer
                glGenBuffers(1, &vertex_buffer_object_);
                glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_object_);
                glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_texture_coordinates),
                             vertex_texture_coordinates, GL_STATIC_DRAW);

                // attribute
                GLuint vertex_texture_coord_id = glGetAttribLocation(program_id_,
                                                                     "vtexcoord");
                glEnableVertexAttribArray(vertex_texture_coord_id);
                glVertexAttribPointer(vertex_texture_coord_id, 2, GL_FLOAT,
                                      DONT_NORMALIZE, ZERO_STRIDE,
                                      ZERO_BUFFER_OFFSET);
            }

            // load/Assign texture
            
            // this->texture_id_ = texture;
            // GLuint tex_id = glGetUniformLocation(program_id_, "heightmap");
            // glUniform1i(tex_id, 0 /*GL_TEXTURE0*/);
          
            // other uniforms
            MVP_id_ = glGetUniformLocation(program_id_, "MVP");
            MV_id_ = glGetUniformLocation(program_id_, "MV");


            // to avoid the current object being polluted
            glBindVertexArray(0);
            glUseProgram(0);
        }

        void Cleanup() {
            glBindVertexArray(0);
            glUseProgram(0);
            glDeleteBuffers(1, &vertex_buffer_object_);
            glDeleteProgram(program_id_);
            glDeleteVertexArrays(1, &vertex_array_id_);
            glDeleteTextures(1, &texture_id_);
        }


        void Draw(int clip_plane_no, const glm::mat4 &model = IDENTITY_MATRIX,
                  const glm::mat4 &view = IDENTITY_MATRIX,
                  const glm::mat4 &projection = IDENTITY_MATRIX) {
            glUseProgram(program_id_);
            glBindVertexArray(vertex_array_id_);

            // // pass choice uniform
            // //GLuint whichPass_id = glGetUniformLocation(program_id_, "whichPass");
            // //glUniform1i(whichPass_id, whichPass);
            // int p[512]; 
            // for (int i=0; i < 256 ; i++){
            //     p[256+i] = permutation[i];
            //     p[i] = permutation[i];
            // }
            // GLuint p_id = glGetUniformLocation(program_id_, "p");
            // glUniform1iv(p_id, 512, p);
            // GLuint permutation_id = glGetUniformLocation(program_id_, "permutation");
            // glUniform1iv(permutation_id, 256, permutation);
            
            // bind texture
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, texture_id_);

            // setup MVP
            glm::mat4 MVP = projection*view*model;
            glm::mat4 MV = view*model;

            glUniformMatrix4fv(MVP_id_, ONE, DONT_TRANSPOSE, glm::value_ptr(MVP));
            glUniformMatrix4fv(MV_id_, ONE, DONT_TRANSPOSE, glm::value_ptr(MV));

            glUniform1i(glGetUniformLocation(program_id_, "clip_plane_no"), clip_plane_no);

            // draw
            glDrawArrays(GL_TRIANGLES/*_STRIP*/, 0, num_vertices);

            glBindVertexArray(0);
            glUseProgram(0);
        }
};
