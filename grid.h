#pragma once
#include "icg_helper.h"
#include <glm/gtc/type_ptr.hpp>


class Grid {

    private:
        GLuint vertex_array_id_;                // vertex array object
        GLuint vertex_buffer_object_position_;  // memory buffer for positions
        GLuint vertex_buffer_object_index_;     // memory buffer for indices
        GLuint program_id_;                     // GLSL shader program ID
        GLuint texture_id_;                     // texture ID
        GLuint texture_grass_id_;
        GLuint texture_rock_id_;
        GLuint texture_sand_id_;
        GLuint texture_snow_id_;
        GLuint texture_cloud_id_;
        GLuint num_indices_;                    // number of vertices to render
        GLuint MVP_id_;                         // model, view, proj matrix ID
        GLuint MV_id_;                         
        float square_length = 1.0;

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
        
        void Init(GLuint texture) {
            // compile the shaders.
            program_id_ = icg_helper::LoadShaders("grid_vshader.glsl",
                                                  "grid_fshader.glsl");
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
                

                int length = 1024;
                // important to cast int to float!
                for (int r = 0; r <= length; r++) {
                    for (int c = 0; c <= length; c++) {
                        vertices.push_back((float(c*2)/length - 1.0f)*square_length);
                        vertices.push_back((1.0f - float(r*2)/length)*square_length);
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
//             {
//                 //grass
//                 int width;
//                 int height;
//                 int nb_component;
//                 string filename = "grass_texture.tga";
//                 // set stb_image to have the same coordinates as OpenGL
//                 stbi_set_flip_vertically_on_load(1);
//                 unsigned char* image = stbi_load(filename.c_str(), &width,
//                                                  &height, &nb_component, 0);

//                 if(image == nullptr) {
//                     throw(string("Failed to load texture"));
//                 }

//                 glGenTextures(1, &texture_grass_id_);
//                 glBindTexture(GL_TEXTURE_2D, texture_grass_id_);
//                 glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
//                 glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

//                 if(nb_component == 3) {
//                     glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0,
//                                  GL_RGB, GL_UNSIGNED_BYTE, image);
//                 } else if(nb_component == 4) {
//                     glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0,
//                                  GL_RGBA, GL_UNSIGNED_BYTE, image);
//                 }

//                 //grass
//                 GLuint tex_grass_id = glGetUniformLocation(program_id_, "grass");
//                 glUniform1i(tex_grass_id, 1 /*GL_TEXTURE1*/);

//                 // cleanup
//                 glBindTexture(GL_TEXTURE_2D, 1);
//                 stbi_image_free(image);
//             }

//             {
//                string filename = "rock_texture.tga";
//                const GLchar* sampler = "rock";
//                int temp_tex_num = 2;
//                GLuint *texture_which_id_ = &texture_rock_id_;
//                 //rock
//                 int width;
//                 int height;
//                 int nb_component;
////                 string filename = "rock_texture.tga";
//                 // set stb_image to have the same coordinates as OpenGL
//                 stbi_set_flip_vertically_on_load(1);
//                 unsigned char* image = stbi_load(filename.c_str(), &width,
//                                                  &height, &nb_component, 0);

//                 if(image == nullptr) {
//                     throw(string("Failed to load texture"));
//                 }

//                 glGenTextures(1, texture_which_id_);
//                 glBindTexture(GL_TEXTURE_2D, *texture_which_id_);
//                 glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
//                 glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

//                 if(nb_component == 3) {
//                     glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0,
//                                  GL_RGB, GL_UNSIGNED_BYTE, image);
//                 } else if(nb_component == 4) {
//                     glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0,
//                                  GL_RGBA, GL_UNSIGNED_BYTE, image);
//                 }

//                 GLuint tex_rock_id = glGetUniformLocation(program_id_, sampler);
//                 glUniform1i(tex_rock_id, temp_tex_num /*GL_TEXTURE1*/);

//                 // cleanup
//                 glBindTexture(GL_TEXTURE_2D, temp_tex_num);
//                 stbi_image_free(image);
//             }

            load_img("grass_texture.tga","grass",1, &texture_grass_id_);
            load_img("rock_texture.tga","rock",2, &texture_rock_id_);
            load_img("sand_texture.tga","sand",3, &texture_sand_id_);
            load_img("snow_texture.tga","snow",4, &texture_snow_id_);
            load_img("snow_texture.tga","cloud",5, &texture_cloud_id_);

            {
                //heightmap
                texture_id_ = texture;
                GLuint tex_id = glGetUniformLocation(program_id_, "heightmap");
                glUniform1i(tex_id, 0 /*GL_TEXTURE0*/);

            }

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
            glDeleteTextures(1, &texture_id_);

            glDeleteTextures(1, &texture_grass_id_);
            glDeleteTextures(1, &texture_rock_id_);
            glDeleteTextures(1, &texture_sand_id_);
            glDeleteTextures(1, &texture_snow_id_);
            glDeleteTextures(1, &texture_cloud_id_);
        }

        void Draw(float time, int clip_plane_no, const glm::mat4 &model = IDENTITY_MATRIX,
                  const glm::mat4 &view = IDENTITY_MATRIX,
                  const glm::mat4 &projection = IDENTITY_MATRIX) {
            glUseProgram(program_id_);
            glBindVertexArray(vertex_array_id_);

            // bind textures
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, texture_id_);

            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, texture_grass_id_);

            glActiveTexture(GL_TEXTURE2);
            glBindTexture(GL_TEXTURE_2D, texture_rock_id_);

            glActiveTexture(GL_TEXTURE3);
            glBindTexture(GL_TEXTURE_2D, texture_sand_id_);

            glActiveTexture(GL_TEXTURE4);
            glBindTexture(GL_TEXTURE_2D, texture_snow_id_);

            glActiveTexture(GL_TEXTURE5);
            glBindTexture(GL_TEXTURE_2D, texture_cloud_id_);
            // setup MVP
            glm::mat4 MVP = projection*view*model;
            glm::mat4 MV = view*model;

            glUniformMatrix4fv(MVP_id_, ONE, DONT_TRANSPOSE, glm::value_ptr(MVP));
            glUniformMatrix4fv(MV_id_, ONE, DONT_TRANSPOSE, glm::value_ptr(MV));

            // pass the current time stamp to the shader.
            glUniform1f(glGetUniformLocation(program_id_, "time"), time);

            glUniform1i(glGetUniformLocation(program_id_, "clip_plane_no"), clip_plane_no);

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
