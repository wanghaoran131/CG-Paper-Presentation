// Disable compiler warnings in third-party code (which we cannot change).
#include <framework/disable_all_warnings.h>
#include <framework/opengl_includes.h>
DISABLE_WARNINGS_PUSH()
// Include glad before glfw3
#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/mat4x4.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
//#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb/stb_image.h>
#include <stb/stb_image_write.h>
DISABLE_WARNINGS_POP()
#include <algorithm>
#include <cassert>
#include <cstdlib> // EXIT_FAILURE
#include <framework/mesh.h>
#include <framework/shader.h>
#include <framework/trackball.h>
#include <framework/window.h>
#include <iostream>
#include <numeric>
#include <optional>
#include <span>
#include <vector>

#define GLM_ENABLE_EXPERIMENTAL

extern "C"
{
    __declspec(dllexport) unsigned long NvOptimusEnablement = 0x00000001;
}

// Configuration
const int WIDTH = 800;
const int HEIGHT = 800;

bool debug = false;
bool phasorNoise = false;
bool phaseField = true;
bool first = false;
bool second = false;
bool third = false;
bool fourth = false;
int currentVar = 1;

static void printHelp();

float f = 50.0f;
float b = 30.0f;
int ipk = 16;

// Program entry point. Everything starts here.
int main(int argc, char** argv)
{
    printHelp();

    Window window { "Shading", glm::ivec2(WIDTH, HEIGHT), OpenGLVersion::GL45 };
    Trackball trackball { &window, glm::radians(50.0f) };
    Trackball trackball2{ &window, glm::radians(50.0f) };

    const Mesh mesh = loadMesh(argc == 2 ? argv[1] : "resources/square_centered.obj")[0];

    window.registerKeyCallback([&](int key, int /* scancode */, int action, int /* mods */) {
        if (action != GLFW_RELEASE)
            return;

        const bool shiftPressed = window.isKeyPressed(GLFW_KEY_LEFT_SHIFT) || window.isKeyPressed(GLFW_KEY_RIGHT_SHIFT);

        switch (key) {
        case GLFW_KEY_0: {
            debug = !debug;
            break;
        }
        case GLFW_KEY_9: {
            phaseField = !phaseField;
            break;
        }
        case GLFW_KEY_1: {
            phasorNoise = !phasorNoise;
            break;
        }
        case GLFW_KEY_5: {
            first = !first;
            break;
        }
        case GLFW_KEY_6: {
            second = !second;
            break;
        }
        case GLFW_KEY_7: {
            third = !third;
            break;
        }
        case GLFW_KEY_8: {
            fourth = !fourth;
            break;
        }
        case GLFW_KEY_F: {
            currentVar = 2;
            trackball2.disableTranslation();
            break;
        }
        case GLFW_KEY_B: {
            currentVar = 3;
            break;
        }
        case GLFW_KEY_I: {
            currentVar = 4;
            break;
        }
        case GLFW_KEY_RIGHT: {
            switch (currentVar) {
            case 2: {
                f += 1.0;
                break;
            }
            case 3: {
                b += 1.0;
                break;
            }
            case 4: {
                ipk += 1.0;
                break;
            }
            default:
                return;
            };
            break;
        }
        case GLFW_KEY_LEFT: {
            switch (currentVar) {
            case 2: {
                f -= 1.0;
                break;
            }
            case 3: {
                b -= 1.0;
                break;
            }
            case 4: {
                ipk -= 1.0;
                break;
            }
            default:
                return;
            };
            break;
        }
        case GLFW_KEY_R: {
            currentVar = 10;
            break;
        }
        case GLFW_KEY_M: {
            currentVar = 20;
            break;
        }
        case GLFW_KEY_X: {
            switch (currentVar) {
            case 10: {
                currentVar = 11;
                break;
            }
            case 20: {
                currentVar = 21;
                break;
            }
            default:
                return;
            };
            break;
        }
        case GLFW_KEY_Y: {
            switch (currentVar) {
            case 10: {
                currentVar = 12;
                break;
            }
            case 20: {
                currentVar = 22;
                break;
            }
            default:
                return;
            };
            break;
        }
        case GLFW_KEY_Z: {
            switch (currentVar) {
            case 10: {
                currentVar = 13;
                break;
            }
            case 20: {
                currentVar = 23;
                break;
            }
            default:
                return;
            };
            break;
        }
        default:
            return;
        };

        if (phaseField) {
            std::cout << "PHASE FIELD!" << std::endl;
        }
        else {
            if (phasorNoise) {
                std::cout << "PHASOR NOISE!" << std::endl;
                if (first) {
                    std::cout << "function 1 ON" << std::endl;
                }
                if (second) {
                    std::cout << "function 2 ON" << std::endl;
                }
                if (third) {
                    std::cout << "function 3 ON" << std::endl;
                }
                if (fourth) {
                    std::cout << "function 4 ON" << std::endl;
                }
            }
        }
        std::cout << "f = " << f << std::endl;
        std::cout << "b = " << b << std::endl;
        std::cout << "ipk = " << ipk << std::endl;
        std::cout << "current var = " << currentVar << std::endl;
        std::cout << "__________________" << std::endl;
        
    });

    const Shader debugShader = ShaderBuilder().addStage(GL_VERTEX_SHADER, "shaders/vertex.glsl").addStage(GL_FRAGMENT_SHADER, "shaders/debug_frag.glsl").build();
    const Shader phasorNoiseShader = ShaderBuilder().addStage(GL_VERTEX_SHADER, "shaders/vertex.glsl").addStage(GL_FRAGMENT_SHADER, "shaders/phasor_noise.glsl").build();
    const Shader bufferAShader = ShaderBuilder().addStage(GL_VERTEX_SHADER, "shaders/vertex.glsl").addStage(GL_FRAGMENT_SHADER, "shaders/phase_field.glsl").build();

    // Create Vertex Buffer Object and Index Buffer Objects.
    GLuint vbo;
    glCreateBuffers(1, &vbo);
    glNamedBufferStorage(vbo, static_cast<GLsizeiptr>(mesh.vertices.size() * sizeof(Vertex)), mesh.vertices.data(), 0);

    GLuint ibo;
    glCreateBuffers(1, &ibo);
    glNamedBufferStorage(ibo, static_cast<GLsizeiptr>(mesh.triangles.size() * sizeof(decltype(Mesh::triangles)::value_type)), mesh.triangles.data(), 0);

    // Bind vertex data to shader inputs using their index (location).
    // These bindings are stored in the Vertex Array Object.
    GLuint vao;
    glCreateVertexArrays(1, &vao);

    // The indices (pointing to vertices) should be read from the index buffer.
    glVertexArrayElementBuffer(vao, ibo);

    // The position and normal vectors should be retrieved from the specified Vertex Buffer Object.
    // The stride is the distance in bytes between vertices. We use the offset to point to the normals
    // instead of the positions.
    glVertexArrayVertexBuffer(vao, 0, vbo, offsetof(Vertex, position), sizeof(Vertex));
    glVertexArrayVertexBuffer(vao, 1, vbo, offsetof(Vertex, normal), sizeof(Vertex));
    glEnableVertexArrayAttrib(vao, 0);
    glEnableVertexArrayAttrib(vao, 1);    

    GLuint framebufferTexture;     // actual texture
    glCreateTextures(GL_TEXTURE_2D, 1, &framebufferTexture);            // create a 2d texture for the framebufferTexture
    glTextureStorage2D(framebufferTexture, 1, GL_RGB8, WIDTH, HEIGHT);  // It has 3 chanels and width heigt

    glTextureParameteri(framebufferTexture, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);    // Set the clamping mode
    glTextureParameteri(framebufferTexture, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    GLuint fbo;     // frame buffer for the extra texture
    glCreateFramebuffers(1, &fbo);
    glNamedFramebufferTexture(fbo, GL_COLOR_ATTACHMENT0, framebufferTexture, 0);    // Atach this frame buffer to the texture
  
    // Enable depth testing.
    glEnable(GL_DEPTH_TEST);

    const glm::mat4 oldView = trackball.viewMatrix();

    // Main loop.
    while (!window.shouldClose()) {
        window.updateInput();

        // Clear the framebuffer to black and depth to maximum value (ranges from [-1.0 to +1.0]).
        glViewport(0, 0, window.getWindowSize().x, window.getWindowSize().y);
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Set model/view/projection matrix.
        const glm::vec3 cameraPos = glm::vec3(5.0f, 0.0f, 0.0f);
        const glm::mat4 model { 1.0f };
        const glm::mat4 view = trackball.viewMatrix();
        const glm::mat4 projection = trackball.projectionMatrix();
        glm::mat4 mvp = projection * view * model;

        bool renderedSomething = false;
        auto render = [&]() {
            renderedSomething = true;

            // Set the model/view/projection matrix that is used to transform the vertices in the vertex shader.
            glUniformMatrix4fv(0, 1, GL_FALSE, glm::value_ptr(mvp));

            // Bind vertex data.
            glBindVertexArray(vao);

            // Execute draw command.
            glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(mesh.triangles.size()) * 3, GL_UNSIGNED_INT, nullptr);
        };

        if (!debug) {
            // Draw mesh into depth buffer but disable color writes.
            glDepthMask(GL_TRUE);
            glDepthFunc(GL_LEQUAL);
            glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
            debugShader.bind();
            render();

            // Draw the mesh again for each light / shading model.
            glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE); // Enable color writes.
            glDepthMask(GL_FALSE); // Disable depth writes.
            glDepthFunc(GL_EQUAL); // Only draw a pixel if it's depth matches the value stored in the depth buffer.
            glEnable(GL_BLEND); // Enable blending.
            glBlendFunc(GL_SRC_ALPHA, GL_ONE); // Additive blending.

            renderedSomething = false;
            if (!renderedSomething) {


                // ------------------------------------------------- this is useful

                if (phaseField) {

                    bufferAShader.bind();
                    glUniform1f(13, b);
                    glUniform1i(14, ipk);
                    render();
                }

                
                else {
                    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

                    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
                    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
                    glViewport(0, 0, WIDTH, HEIGHT);

                    bufferAShader.bind();

                    glUniform1f(13, b);
                    glUniform1i(14, ipk);
                    glm::mat4 mvp2 = glm::mat4(-2.15, 0, 0, 0,
                        0, 2.15, 0, 0,
                        0, 0, 1, 1,
                        -1.75, -3.86, 4, 4);
                    //const glm::mat4 lightMVP = glm::mat4(-2.14451, 0, 0, 1.02936,
                    //    0, 2.14451, 0, -1.0937,
                    //    0, 0, 1.0002, 1.4803,
                    //    0, 0, 1, 1); // this was kinda close to letting the existing cube fill the screen but not quite good

                    //const glm::mat4 view2 = trackball2.viewMatrix();
                    //glm::mat4 mvp2 = projection * view2 * model;
                    glUniformMatrix4fv(0, 1, GL_FALSE, glm::value_ptr(mvp2));

                    // Bind vertex data. would be nicer to directly draw a quad in view space if i know how to open gl
                    glBindVertexArray(vao);

                    // Execute draw command to render the cube to the texture.
                    glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(mesh.triangles.size()) * 3, GL_UNSIGNED_INT, nullptr);

                    glBindFramebuffer(GL_FRAMEBUFFER, 0);


                    if (phasorNoise) {
                        phasorNoiseShader.bind();

                        // texture from framebuffer
                        glActiveTexture(GL_TEXTURE0);
                        glBindTexture(GL_TEXTURE_2D, framebufferTexture);
                        glUniform1i(2, 0);
                        glUniform1f(12, f);
                        glUniform1f(13, b);
                        glUniform1i(14, ipk);
                        glUniform1i(31, first);
                        glUniform1i(32, second);
                        glUniform1i(33, third);
                        glUniform1i(34, fourth);
                        render();
                    }

                }

            }

            // Restore default depth test settings and disable blending.
            glDepthFunc(GL_LEQUAL);
            glDepthMask(GL_TRUE);
            glDisable(GL_BLEND);
        }
        if (!renderedSomething) {
            debugShader.bind();
            //glUniform3fv(1, 1, glm::value_ptr(cameraPos)); // viewPos.
            render();
        }

        // Present result to the screen.
        window.swapBuffers();
    }

    // Be a nice citizen and clean up after yourself.
    glDeleteTextures(1, &framebufferTexture);
    glDeleteBuffers(1, &vbo);
    glDeleteBuffers(1, &ibo);
    glDeleteFramebuffers(1, &fbo);
    glDeleteVertexArrays(1, &vao);

    return 0;
}

static std::optional<glm::vec3> getWorldPositionOfPixel(const Trackball& trackball, const glm::vec2& pixel)
{
    float depth;
    glReadPixels(static_cast<int>(pixel.x), static_cast<int>(pixel.y), 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &depth);

    if (depth == 1.0f) {
        // This is a work around for a bug in GCC:
        // https://gcc.gnu.org/bugzilla/show_bug.cgi?id=80635
        //
        // This bug will emit a warning about a maybe uninitialized value when writing:
        // return {};
        constexpr std::optional<glm::vec3> tmp;
        return tmp;
    }

    // Coordinates convert from pixel space to OpenGL screen space (range from -1 to +1)
    const glm::vec3 win { pixel, depth };

    // View matrix
    const glm::mat4 view = trackball.viewMatrix();
    const glm::mat4 projection = trackball.projectionMatrix();

    const glm::vec4 viewport { 0, 0, WIDTH, HEIGHT };
    return glm::unProject(win, view, projection, viewport);
}

static void printHelp()
{
    Trackball::printHelp();
    std::cout << std::endl;
    std::cout << "Program Usage:" << std::endl;
    std::cout << "0 - activate Debug" << std::endl;
    std::cout << "______________________" << std::endl;
    std::cout << "1 - Phasor noise" << std::endl;
    std::cout << "      5 - (De)activate function 1" << std::endl;
    std::cout << "      6 - (De)activate function 2" << std::endl;
    std::cout << "      7 - (De)activate function 3" << std::endl;
    std::cout << "      8 - (De)activate function 4" << std::endl;
    std::cout << "9 - Phase field" << std::endl;
    std::cout << "______________________" << std::endl;
    std::cout << "RIGHT - Increase value" << std::endl;
    std::cout << "LEFT - Decrease value" << std::endl;
    std::cout << "F - Select f" << std::endl;
    std::cout << "B - Select b" << std::endl;
    std::cout << "I - Select ipk" << std::endl;
}
