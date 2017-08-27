#pragma once
#include "icg_helper.h"
#include <glm/gtc/type_ptr.hpp>


class Sea {

    private:
        GLuint vertex_array_id_;                // vertex array object
        GLuint vertex_buffer_object_position_;  // memory buffer for positions
        GLuint vertex_buffer_object_index_;     // memory buffer for indices
        GLuint program_id_;                     // GLSL shader program ID
        GLuint texture_dudv;
        GLuint texture_normalmap;
        GLuint texture_reflection;                     // texture ID
        GLuint texture_refraction;
        GLuint texture_heightmap;
        GLuint num_indices_;                    // number of vertices to render
        GLuint MVP_id_;                         // model, view, proj matrix ID
        GLuint MV_id_;                         

    public:
        void load_img(string filename, const GLchar* sampler, int temp_tex_num, GLuint *texture_which_id_) {

            int width;
            int height;
            int nb_component;
            // set stb_image to have the same coordinates as OpenGL
            stbi_set_flip_vertically_on_load(1);
            unsigned char* image = stbi_load(filename.c_str(), &width,
                                             &height, &nb_component, 0);

            if(image == nullptr) {
                throw(string("Failed to load texture"));
            }

            glGenTextures(1, texture_which_id_);
            // pointer , reference
            glBindTexture(GL_TEXTURE_2D, *texture_which_id_);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

            if(nb_component == 3) {
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0,
                             GL_RGB, GL_UNSIGNED_BYTE, image);
            } else if(nb_component == 4) {
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0,
                             GL_RGBA, GL_UNSIGNED_BYTE, image);
            }

            GLuint tex_which_id = glGetUniformLocation(program_id_, sampler);
            glUniform1i(tex_which_id, temp_tex_num);

            // cleanup
            glBindTexture(GL_TEXTURE_2D, temp_tex_num);
            stbi_image_free(image);
        }

        void Init(GLuint texture_refl, GLuint texture_refr, GLuint texture_h) {
            // compile the shaders.
            program_id_ = icg_helper::LoadShaders("sea_vshader.glsl",
                                                  "sea_fshader.glsl");
            if(!program_id_) {
                exit(EXIT_FAILURE);
            }

            glUseProgram(program_id_);

            // vertex one vertex array
            glGenVertexArrays(1, &vertex_array_id_);
            glBindVertexArray(vertex_array_id_);

            // vertex coordinates and indices
            {
                std::vector<GLfloat> vertices;
                std::vector<GLuint> indices;
                // TODO 5: make a triangle grid with dimension 100x100.
                // always two subsequent entries in 'vertices' form a 2D vertex position.
                
                int length = 512;
                // important to cast int to float!
                for (int r = 0; r <= length; r++) {
                    for (int c = 0; c <= length; c++) {
                        vertices.push_back(float(c*2)/length - 1.0f);
                        vertices.push_back(1.0f - float(r*2)/length);
                    }
                }

                for (int r = 0; r < length; r++) {
                    for (int c = 0; c < length; c++) {
                        int no_vertex =  r*(length+1) + c;
                        indices.push_back(no_vertex);
                        indices.push_back(no_vertex+1);
                        indices.push_back(no_vertex+length+1);

                        indices.push_back(no_vertex+1);
                        indices.push_back(no_vertex+length+1);
                        indices.push_back(no_vertex+length+2);
                    }

                }

                num_indices_ = indices.size();

                // position buffer
                glGenBuffers(1, &vertex_buffer_object_position_);
                glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_object_position_);
                glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(GLfloat),
                             &vertices[0], GL_STATIC_DRAW);

                // vertex indices
                glGenBuffers(1, &vertex_buffer_object_index_);
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vertex_buffer_object_index_);
                glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint),
                             &indices[0], GL_STATIC_DRAW);

                // position shader attribute
                GLuint loc_position = glGetAttribLocation(program_id_, "position");
                glEnableVertexAttribArray(loc_position);
                glVertexAttribPointer(loc_position, 2, GL_FLOAT, DONT_NORMALIZE,
                                      ZERO_STRIDE, ZERO_BUFFER_OFFSET);

            }

            // load texture

            {
                //heightmap
                texture_reflection = texture_refl;
                GLuint tex_id = glGetUniformLocation(program_id_, "texrefl");
                glUniform1i(tex_id, 0 /*GL_TEXTURE0*/);

            }
            {
                //heightmap
                texture_refraction = texture_refr;
                GLuint tex_id = glGetUniformLocation(program_id_, "texrefr");
                glUniform1i(tex_id, 1 /*GL_TEXTURE0*/);

            }
            {
                //heightmap
                texture_heightmap = texture_h;
                GLuint tex_id = glGetUniformLocation(program_id_, "texh");
                glUniform1i(tex_id, 2 /*GL_TEXTURE0*/);

            }

            load_img("waterdudv.tga","tex_dudv",3, &texture_dudv);
            load_img("normalmap.tga","tex_normal",4, &texture_normalmap);

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
            glDeleteBuffers(1, &vertex_buffer_object_position_);
            glDeleteBuffers(1, &vertex_buffer_object_index_);
            glDeleteVertexArrays(1, &vertex_array_id_);
            glDeleteProgram(program_id_);

            glDeleteTextures(1, &texture_reflection);
            glDeleteTextures(1, &texture_refraction);
            glDeleteTextures(1, &texture_heightmap);
            glDeleteTextures(1, &texture_dudv);
            glDeleteTextures(1, &texture_normalmap);
        }

        void Draw(float time, const glm::mat4 &model = IDENTITY_MATRIX,
                  const glm::mat4 &view = IDENTITY_MATRIX,
                  const glm::mat4 &projection = IDENTITY_MATRIX) {
            glUseProgram(program_id_);
            glBindVertexArray(vertex_array_id_);

            // bind textures
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, texture_reflection);

            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, texture_refraction);
            
            glActiveTexture(GL_TEXTURE2);
            glBindTexture(GL_TEXTURE_2D, texture_heightmap);

            glActiveTexture(GL_TEXTURE3);
            glBindTexture(GL_TEXTURE_2D, texture_dudv);

            glActiveTexture(GL_TEXTURE4);
            glBindTexture(GL_TEXTURE_2D, texture_normalmap);

            // setup MVP
            glm::mat4 MVP = projection*view*model;
            glm::mat4 MV = view*model;

            glUniformMatrix4fv(MVP_id_, ONE, DONT_TRANSPOSE, glm::value_ptr(MVP));
            glUniformMatrix4fv(MV_id_, ONE, DONT_TRANSPOSE, glm::value_ptr(MV));

            // pass the current time stamp to the shader.
            glUniform1f(glGetUniformLocation(program_id_, "time"), time);

            // draw
            // TODO 5: for debugging it can be helpful to draw only the wireframe.
            // You can do that by uncommenting the next line.
            // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

            // TODO 5: depending on how you set up your vertex index buffer, you
            // might have to change GL_TRIANGLE_STRIP to GL_TRIANGLES.
            glDrawElements(GL_TRIANGLES, num_indices_, GL_UNSIGNED_INT, 0);




            glBindVertexArray(0);
            glUseProgram(0);
        }
};
