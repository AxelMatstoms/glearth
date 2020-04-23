//#include <stdio.h>
//#include <cstdlib> #include <cmath>
#include <cstdint>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include <SDL2/SDL.h>
#include <glad/glad.h>
#include <stb_image.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define WIDTH 1280
#define HEIGHT 720

extern float sphere_vertices[];
extern int sphere_vertex_count;
extern float cube_vertices[];
extern int cube_vertex_count;

GLuint shader_compile(const std::string& path, GLenum shader_type)
{
    std::cout << "Compiling shader: " << path << std::endl;
    std::ifstream f;
    f.open(path);

    if (f.fail()) {
        std::cerr << "Could not open shader source file: " << path << std::endl;
    }
std::stringstream s;
    s << f.rdbuf();
    f.close();

    std::string shader_source_str = s.str();

    const GLchar* shader_source = shader_source_str.c_str();

    GLuint shader = glCreateShader(shader_type);
    glShaderSource(shader, 1, &shader_source, NULL);
    glCompileShader(shader);

    int status;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
    if (status == GL_FALSE) {
        GLint log_length;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &log_length);

        GLchar *log = new GLchar[log_length];
        glGetShaderInfoLog(shader, log_length, &log_length, log);

        std::cerr << "An error occurred when compiling " << path << ":" << std::endl;
        std::cerr << log << std::endl;

        delete[] log;

        glDeleteShader(shader);
        return 0;
    }

    return shader;
}

GLuint create_shader_program(const std::string& vertex_shader_path, const std::string& fragment_shader_path)
{
    GLuint fragment_shader = shader_compile(fragment_shader_path, GL_FRAGMENT_SHADER);
    GLuint vertex_shader = shader_compile(vertex_shader_path, GL_VERTEX_SHADER);

    if (!fragment_shader || !vertex_shader) {
        glDeleteShader(fragment_shader);
        glDeleteShader(vertex_shader);

        return 0;
    }

    std::cout << "Linking shader program (" << vertex_shader_path << ", " << fragment_shader_path << ")" << std::endl;

    GLuint shader_program = glCreateProgram();
    glAttachShader(shader_program, fragment_shader);
    glAttachShader(shader_program, vertex_shader);

    glLinkProgram(shader_program);

    int status;
    glGetProgramiv(shader_program, GL_LINK_STATUS, &status);
    if (status == GL_FALSE) {
        std::cerr << "An error occurred when linking shader program" << std::endl;
        GLint log_length;
        glGetProgramiv(shader_program, GL_INFO_LOG_LENGTH, &log_length);

        GLchar *log = new GLchar[log_length];
        glGetProgramInfoLog(shader_program, log_length, &log_length, log);

        std::cerr << log << std::endl;

        delete[] log;

        glDetachShader(shader_program, fragment_shader);
        glDetachShader(shader_program, vertex_shader);

        glDeleteShader(fragment_shader);
        glDeleteShader(vertex_shader);

        glDeleteProgram(shader_program);

        return 0;
    }

    glDetachShader(shader_program, fragment_shader);
    glDetachShader(shader_program, vertex_shader);

    glDeleteShader(fragment_shader);
    glDeleteShader(vertex_shader);

    return shader_program;
}

GLuint load_cube_map_texture(std::vector<std::string> faces)
{
    GLuint texture;
    int img_width, img_height, img_channels;
    uint8_t* data;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_CUBE_MAP, texture);
    for (int i = 0; i < 6; i++) {
        std::cout << "Loading texture: " << faces[i] << std::endl;
        data = stbi_load(faces[i].c_str(), &img_width, &img_height, &img_channels, 0);
        if (!data) {
            std::cerr << "Failed to load texture: " << faces[i] << std::endl;
            return 0;
        }
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                0, GL_RGB, img_width, img_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        stbi_image_free(data);
    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    return texture;
}

GLuint load_texture(std::string path)
{
    GLuint texture;
    int img_width, img_height, img_channels;
    uint8_t* data;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    std::cout << "Loading texture: " << path << std::endl;
    data = stbi_load(path.c_str(), &img_width, &img_height, &img_channels, 0);
    if (!data) {
        std::cerr << "Failed to load texture: " << path << std::endl;
        return 0;
    }
    glTexImage2D(GL_TEXTURE_2D,
            0, GL_RGB, img_width, img_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    stbi_image_free(data);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    return texture;
}

int main()
{
    SDL_Init(SDL_INIT_EVERYTHING);
    SDL_Window *window = SDL_CreateWindow("OpenGL", 0, 0, WIDTH, HEIGHT, SDL_WINDOW_OPENGL);

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);

    SDL_SetRelativeMouseMode(SDL_TRUE);

    SDL_GLContext glContext = SDL_GL_CreateContext(window);

    gladLoadGL();

    GLuint glow_sphere_shader = create_shader_program("shaders/sphere.vert", "shaders/glow_sphere.frag");
    if (!glow_sphere_shader) {
        return 1;
    }

    GLuint earth_shader = create_shader_program("shaders/sphere.vert", "shaders/earth.frag");
    if (!earth_shader) {
        return 1;
    }

    GLuint skybox_shader = create_shader_program("shaders/skybox.vert", "shaders/skybox.frag");
    if (!skybox_shader) {
        return 1;
    }

    glEnable(GL_DEPTH_TEST);
    //glEnable(GL_FRAMEBUFFER_SRGB);

    GLuint earth_day_texture = load_texture("textures/earth/day16k.png");

    GLuint earth_night_texture = load_texture("textures/earth/night8k.png");

    GLuint earth_roughness_texture = load_texture("textures/earth/roughness.png");

    GLuint earth_clouds_texture = load_texture("textures/earth/clouds8k.png");

    GLuint earth_normal_texture = load_texture("textures/earth/normal8k.png");

    GLuint sun_texture = load_texture("textures/2k_sun.png");

    GLuint skybox_texture = load_texture("textures/milkyway_lr.png");

    GLuint sphere_vao;
    glGenVertexArrays(1, &sphere_vao);

    GLuint sphere_vbo;
    glGenBuffers(1, &sphere_vbo);

    glBindBuffer(GL_ARRAY_BUFFER, sphere_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * sphere_vertex_count, sphere_vertices, GL_STATIC_DRAW);

    glBindVertexArray(sphere_vao);

    //glBindBuffer(GL_ARRAY_BUFFER, sphere_vbo);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *) 0);
    glEnableVertexAttribArray(0);

    // normal attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *) (3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    GLuint cube_vao;
    glGenVertexArrays(1, &cube_vao);

    GLuint cube_vbo;
    glGenBuffers(1, &cube_vbo);

    glBindBuffer(GL_ARRAY_BUFFER, cube_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * cube_vertex_count, cube_vertices, GL_STATIC_DRAW);

    glBindVertexArray(cube_vao);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *) 0);
    glEnableVertexAttribArray(0);

    //glUseProgram(shader_program);

    GLint glow_sphere_model_location = glGetUniformLocation(glow_sphere_shader, "model");
    GLint glow_sphere_normal_mtrx_location = glGetUniformLocation(glow_sphere_shader, "normalMtrx");
    GLint glow_sphere_view_location = glGetUniformLocation(glow_sphere_shader, "view");
    GLint glow_sphere_projection_location = glGetUniformLocation(glow_sphere_shader, "projection");
    GLint glow_sphere_texture_location = glGetUniformLocation(glow_sphere_shader, "cubeMap");
    GLint glow_sphere_time_location = glGetUniformLocation(glow_sphere_shader, "time");
    GLint glow_sphere_camera_pos_location = glGetUniformLocation(glow_sphere_shader, "cameraPos");

    GLint earth_model_location = glGetUniformLocation(earth_shader, "model");
    GLint earth_normal_mtrx_location = glGetUniformLocation(earth_shader, "normalMtrx");
    GLint earth_view_location = glGetUniformLocation(earth_shader, "view");
    GLint earth_projection_location = glGetUniformLocation(earth_shader, "projection");
    GLint earth_day_location = glGetUniformLocation(earth_shader, "daySide");
    GLint earth_night_location = glGetUniformLocation(earth_shader, "nightSide");
    GLint earth_roughness_location = glGetUniformLocation(earth_shader, "roughnessMap");
    GLint earth_clouds_location = glGetUniformLocation(earth_shader, "clouds");
    GLint earth_normal_location = glGetUniformLocation(earth_shader, "normalMap");
    GLint earth_normal_mapping_location = glGetUniformLocation(earth_shader, "normalMapping");
    GLint earth_oren_nayar_location = glGetUniformLocation(earth_shader, "orenNayar");
    GLint earth_camera_pos_location = glGetUniformLocation(earth_shader, "cameraPos");
    GLint earth_sun_position_location = glGetUniformLocation(earth_shader, "sun.position");
    GLint earth_sun_emission_location = glGetUniformLocation(earth_shader, "sun.emission");

    GLint skybox_view_location = glGetUniformLocation(skybox_shader, "view");
    GLint skybox_projection_location = glGetUniformLocation(skybox_shader, "projection");
    GLint skybox_cubemap_location = glGetUniformLocation(skybox_shader, "skybox");


    glUseProgram(glow_sphere_shader);
    glUniform1i(glow_sphere_texture_location, 0);

    glUseProgram(earth_shader);
    glUniform1i(earth_day_location, 0);
    glUniform1i(earth_night_location, 1);
    glUniform1i(earth_roughness_location, 2);
    glUniform1i(earth_clouds_location, 3);
    glUniform1i(earth_normal_location, 4);
    glUniform3f(earth_sun_position_location, 0.0f, 0.0, -3.0f);
    glUniform3f(earth_sun_emission_location, 6.0f, 6.0f, 6.0f);

    glUseProgram(skybox_shader);
    glUniform1i(skybox_cubemap_location, 0);

    glm::vec3 cam_pos(0.0f, 0.0f, 3.0f);
    glm::vec3 cam_front(0.0f, 0.0f, -1.0f);
    glm::vec3 cam_up(0.0f, 1.0f, 0.0f);
    glm::vec3 movement(0.0f, 0.0f, 0.0f);

    float yaw = 270.0f;
    float pitch = 0.0;

    bool normalMapping = true;
    bool orenNayar = true;

    int quit = 0;
    uint32_t last_tick = SDL_GetTicks();
    while (!quit) {
        SDL_Event ev;
        while (SDL_PollEvent(&ev)) {
            switch (ev.type) {
                case SDL_QUIT:
                    quit = 1;
                    break;
                case SDL_KEYDOWN:
                    switch (ev.key.keysym.sym) {
                        case SDLK_w:
                            movement.z = -1;
                            break;
                        case SDLK_s:
                            movement.z = 1;
                            break;
                        case SDLK_a:
                            movement.x = -1;
                            break;
                        case SDLK_d:
                            movement.x = 1;
                            break;
                        case SDLK_r:
                            movement.y = 1;
                            break;
                        case SDLK_f:
                            movement.y = -1;
                            break;
                        case SDLK_n:
                            normalMapping = !normalMapping;
                            break;
                        case SDLK_o:
                            orenNayar = !orenNayar;
                            break;
                    }
                    break;
                case SDL_KEYUP:
                    switch (ev.key.keysym.sym) {
                        case SDLK_w:
                            if (movement.z < 0)
                                movement.z = 0;
                            break;
                        case SDLK_s:
                            if (movement.z > 0)
                                movement.z = 0;
                            break;
                        case SDLK_a:
                            if (movement.x < 0)
                                movement.x = 0;
                            break;
                        case SDLK_d:
                            if (movement.x > 0)
                                movement.x = 0;
                            break;
                        case SDLK_r:
                            if (movement.y > 0)
                                movement.y = 0;
                            break;
                        case SDLK_f:
                            if (movement.y < 0)
                                movement.y = 0;
                            break;
                    }
                    break;
                case SDL_MOUSEMOTION:
                    yaw += ev.motion.xrel * 0.1f;;
                    pitch -= ev.motion.yrel * 0.1f;;
                    break;
            }
        }

        uint32_t tick = SDL_GetTicks();
        float time = tick / 1000.0;
        float delta = (tick - last_tick) / 1000.0;
        last_tick = tick;

        if (pitch < -89.0f) pitch = -89.0f;
        if (pitch >  89.0f) pitch =  89.0f;

        glm::vec3 front;
        front.x = cos(glm::radians(pitch)) * cos(glm::radians(yaw));
        front.y = sin(glm::radians(pitch));
        front.z = cos(glm::radians(pitch)) * sin(glm::radians(yaw));
        cam_front = glm::normalize(front);

        glm::vec3 forward = cam_front;
        forward.y = 0;
        forward = glm::normalize(forward);

        glm::vec3 up(0.0f, 1.0f, 0.0f);
        glm::vec3 cam_right = glm::normalize(glm::cross(cam_front, up));
        cam_up = glm::cross(cam_right, cam_front);

        cam_pos += cam_right * movement.x * delta * 1.0f;
        cam_pos += up * movement.y * delta * 1.0f;
        cam_pos -= forward * movement.z * delta * 1.0f;


        glm::mat4 view;
        view = glm::lookAt(cam_pos, cam_pos + cam_front, cam_up);

        glm::mat4 projection;
        projection = glm::perspective(glm::radians(45.0f), static_cast<float>(WIDTH) / static_cast<float>(HEIGHT), 0.01f, 10000.0f);

        glClearColor(0.0, 0.0, 0.0, 1.0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glDepthMask(GL_FALSE);

        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

        glUseProgram(skybox_shader);
        glUniformMatrix4fv(skybox_view_location, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(skybox_projection_location, 1, GL_FALSE, glm::value_ptr(projection));
        glBindVertexArray(cube_vao);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, skybox_texture);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        glDepthMask(GL_TRUE);

        glBindVertexArray(sphere_vao);
        glUseProgram(earth_shader);

        glm::mat4 model(1.0f);
        model = glm::translate(model, glm::vec3(3.0f * cos(time / 100.0f), 0.0f, -3.0f + 3.0f * sin(time / 100.0f)));
        model = glm::rotate(model, -time / 10.0f, glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::rotate(model, glm::radians(23.5f), glm::vec3(1.0f, 0.0f, 0.0f));

        glm::mat3 normalMtrx(1.0f);
        normalMtrx = glm::mat3(glm::transpose(glm::inverse(model)));

        glUniformMatrix4fv(earth_model_location, 1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix3fv(earth_normal_mtrx_location, 1, GL_FALSE, glm::value_ptr(normalMtrx));
        glUniformMatrix4fv(earth_view_location, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(earth_projection_location, 1, GL_FALSE, glm::value_ptr(projection));
        glUniform3f(earth_camera_pos_location, cam_pos.x, cam_pos.y, cam_pos.z);
        glUniform1i(earth_normal_mapping_location, normalMapping);
        glUniform1i(earth_oren_nayar_location, orenNayar);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, earth_day_texture);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, earth_night_texture);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, earth_roughness_texture);
        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, earth_clouds_texture);
        glActiveTexture(GL_TEXTURE4);
        glBindTexture(GL_TEXTURE_2D, earth_normal_texture);

        glBindVertexArray(sphere_vao);

        glDrawArrays(GL_TRIANGLES, 0, sphere_vertex_count);

        model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -3.0f));
        //model = glm::rotate(model, -time / 40.0f, glm::vec3(0.0f, 1.0f, 0.0f));

        normalMtrx = glm::mat3(glm::transpose(glm::inverse(model)));

        glUseProgram(glow_sphere_shader);

        glUniformMatrix4fv(glow_sphere_model_location, 1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix3fv(glow_sphere_normal_mtrx_location, 1, GL_FALSE, glm::value_ptr(normalMtrx));
        glUniformMatrix4fv(glow_sphere_view_location, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(glow_sphere_projection_location, 1, GL_FALSE, glm::value_ptr(projection));
        glUniform1f(glow_sphere_time_location, time);
        glUniform3f(glow_sphere_camera_pos_location, cam_pos.x, cam_pos.y, cam_pos.z);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, sun_texture);

        glDrawArrays(GL_TRIANGLES, 0, sphere_vertex_count);

        SDL_GL_SwapWindow(window);
    }

}
