#pragma once
#include "icg_helper.h"

class Snow {

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
        float acc = 1.0;
        float previous_time = 0.0;
        
        

    public:
        void Update() {
            for (int i=0; i<vertices.size(); i+=3) {
                vertices[i]  += 0;
                vertices[i+1]+= 0;
                vertices[i+2]-= sqrt(2*acc*(1.0-vertices[i+2]));
                if (vertices[i+2] <= 0){
                    vertices[i+2] = 1.0;
                }
            }
        }

        void Init(std::vector<float> spoints) {

     
            // compile the shaders
            program_id_ = icg_helper::LoadShaders("snow_vshader.glsl",
                                                  "snow_fshader.glsl");
            if(!program_id_) {
                exit(EXIT_FAILURE);
            }

            glUseProgram(program_id_);

            // vertex one vertex Array
            glGenVertexArrays(1, &vertex_array_id_);
            glBindVertexArray(vertex_array_id_);

            // vertex coordinates
            {   
                float size = 0.005;
                for (int i=0; i<spoints.size(); i+=3) {
                    vertices.push_back(spoints[i]);
                    vertices.push_back(spoints[i+1]);
                    vertices.push_back(spoints[i+2]);

                    num_vertices ++;
                    // vertices.push_back(spoints[i]+size);
                    // vertices.push_back(spoints[i+1]);
                    // vertices.push_back(spoints[i+2]);

                    // vertices.push_back(spoints[i]);
                    // vertices.push_back(spoints[i+1]+size);
                    // vertices.push_back(spoints[i+2]);


                    // vertices.push_back(spoints[i]);
                    // vertices.push_back(spoints[i+1]);
                    // vertices.push_back(spoints[i+2]);

                    // vertices.push_back(spoints[i]+size);
                    // vertices.push_back(spoints[i+1]);
                    // vertices.push_back(spoints[i+2]);

                    // vertices.push_back(spoints[i]);
                    // vertices.push_back(spoints[i+1]);
                    // vertices.push_back(spoints[i+2]+size);
                    
                    // num_vertices+=6;
                }

                glGenBuffers(1, &vertex_buffer_object_);
                glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_object_);
                glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(GLfloat)/*sizeof(vertex_point)*/,
                             &vertices[0]/*vertex_point*/, GL_DYNAMIC_DRAW);

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


        void Draw(int clip_plane_no, float time,
                const glm::mat4 &model = IDENTITY_MATRIX,
                  const glm::mat4 &view = IDENTITY_MATRIX,
                  const glm::mat4 &projection = IDENTITY_MATRIX) {


            glUseProgram(program_id_);
            glBindVertexArray(vertex_array_id_);

            //make points circular
         
            //glPointSize(3); 

            // bind texture
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, texture_id_);

            // setup MVP
            glm::mat4 MVP = projection*view*model;
            glm::mat4 MV = view*model;

            glUniformMatrix4fv(MVP_id_, ONE, DONT_TRANSPOSE, glm::value_ptr(MVP));
            glUniformMatrix4fv(MV_id_, ONE, DONT_TRANSPOSE, glm::value_ptr(MV));

            glUniform1i(glGetUniformLocation(program_id_, "clip_plane_no"), clip_plane_no);
            glUniform1f(glGetUniformLocation(program_id_, "time"), time);

            // draw
            glDrawArrays(GL_POINTS/*GL_TRIANGLES*/, 0, num_vertices);

            glBindVertexArray(0);
            glUseProgram(0);
        }
};
