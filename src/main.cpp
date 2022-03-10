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
#include <stb/stb_image.h>
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

extern "C"
{
    __declspec(dllexport) unsigned long NvOptimusEnablement = 0x00000001;
}

// Configuration
const int WIDTH = 800;
const int HEIGHT = 800;

bool debug = false;
bool phasorNoise = false;
bool bufferA = false;
int currentVar = 1;

static void printHelp();

float f = 10.0f;
float b = 10.0f;
int ipk = 10.0f;

float rx = 3.0f;
float ry = 3.0f;
float rz = 3.0f;

float mx = 1.0f;
float my = 1.0f;
float mz = 1.0f;

// Program entry point. Everything starts here.
int main(int argc, char** argv)
{
    printHelp();

    Window window { "Shading", glm::ivec2(WIDTH, HEIGHT), OpenGLVersion::GL45 };
    Trackball trackball { &window, glm::radians(50.0f) };

    const Mesh mesh = loadMesh(argc == 2 ? argv[1] : "resources/brick_path.obj")[0];

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
            bufferA = !bufferA;
            break;
        }
        case GLFW_KEY_1: {
            phasorNoise = !phasorNoise;
            break;
        }
        case GLFW_KEY_2: {
            currentVar = 2;
            break;
        }
        case GLFW_KEY_3: {
            currentVar = 3;
            break;
        }
        case GLFW_KEY_4: {
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
            case 11: {
                rx += 0.5;
                break;
            }
            case 12: {
                ry += 0.5;
                break;
            }
            case 13: {
                rz += 0.5;
                break;
            }
            case 21: {
                mx += 0.5;
                break;
            }
            case 22: {
                my += 0.5;
                break;
            }
            case 23: {
                mz += 0.5;
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
            case 11: {
                rx -= 0.5;
                break;
            }
            case 12: {
                ry -= 0.5;
                break;
            }
            case 13: {
                rz -= 0.5;
                break;
            }
            case 21: {
                mx -= 0.5;
                break;
            }
            case 22: {
                my -= 0.5;
                break;
            }
            case 23: {
                mz -= 0.5;
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
        case GLFW_KEY_7: {
            ipk -= 1;
            break;
        }
        default:
            return;
        };

        if (phasorNoise) {
            std::cout << "PHASOR NOISE!" << std::endl;
        }
        if (bufferA) {
            std::cout << "BUFFER A!" << std::endl;
        }
        std::cout << "f = " << f << std::endl;
        std::cout << "b = " << b << std::endl;
        std::cout << "ipk = " << ipk << std::endl;
        std::cout << "current var = " << currentVar << std::endl;
        std::cout << "resolution = " << rx << ", " << ry << ", " << rz << std::endl;
        std::cout << "mouse = " << mx << ", " << my << ", " << mz << std::endl;
        std::cout << "__________________" << std::endl;
        
    });

    const Shader debugShader = ShaderBuilder().addStage(GL_VERTEX_SHADER, "shaders/vertex.glsl").addStage(GL_FRAGMENT_SHADER, "shaders/debug_frag.glsl").build();
    const Shader phasorNoiseShader = ShaderBuilder().addStage(GL_VERTEX_SHADER, "shaders/vertex.glsl").addStage(GL_FRAGMENT_SHADER, "shaders/phasor_noise.glsl").build();
    const Shader bufferAShader = ShaderBuilder().addStage(GL_VERTEX_SHADER, "shaders/vertex.glsl").addStage(GL_FRAGMENT_SHADER, "shaders/buffer_a.glsl").build();

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

    // Load image from disk to CPU memory.
    int width, height, sourceNumChannels; // Number of channels in source image. pixels will always be the requested number of channels (3).
    stbi_uc* pixels = stbi_load("resources/dog.png", &width, &height, &sourceNumChannels, STBI_rgb);

    // Create a texture on the GPU with 3 channels with 8 bits each.
    GLuint texToon;
    glCreateTextures(GL_TEXTURE_2D, 1, &texToon);
    glTextureStorage2D(texToon, 1, GL_RGB8, width, height);

    // Upload pixels into the GPU texture.
    glTextureSubImage2D(texToon, 0, 0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, pixels);
    // Free the CPU memory after we copied the image to the GPU.
    stbi_image_free(pixels);

    // Set behavior for when texture coordinates are outside the [0, 1] range.
    glTextureParameteri(texToon, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTextureParameteri(texToon, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // Set interpolation for texture sampling (GL_NEAREST for no interpolation).
    glTextureParameteri(texToon, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTextureParameteri(texToon, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Enable depth testing.
    glEnable(GL_DEPTH_TEST);

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
        const glm::mat4 mvp = projection * view * model;

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
                if (phasorNoise) {
                    phasorNoiseShader.bind();

                    glActiveTexture(GL_TEXTURE0);
                    glBindTexture(GL_TEXTURE_2D, texToon);
                    glUniform1i(2, 0); // Change 2 to the uniform index that you want to use.
                    //uniform vec3      iResolution;           // viewport resolution (in pixels)
                    //uniform vec4      iMouse;                // mouse pixel coords. xy: current (if MLB down), zw: click
                    //uniform samplerXX iChannel0..3;          // input channel. XX = 2D/Cube
                    glm::vec3 placeholder3 = glm::vec3(rx, ry, rz);
                    glm::vec3 placeholder4 = glm::vec4(mx, my, mz, 1.0f);
                    glUniform3fv(10, 1, glm::value_ptr(placeholder3));
                    glUniform4fv(11, 1, glm::value_ptr(placeholder4));
                    glUniform1f(12, f);
                    glUniform1f(13, b);
                    glUniform1i(14, ipk);
                    render();
                }

                if (bufferA) {
                    bufferAShader.bind();

                    glActiveTexture(GL_TEXTURE0);
                    glBindTexture(GL_TEXTURE_2D, texToon);
                    glUniform1i(2, 0); // Change 2 to the uniform index that you want to use.
                    //uniform vec3      iResolution;           // viewport resolution (in pixels)
                    //uniform vec4      iMouse;                // mouse pixel coords. xy: current (if MLB down), zw: click
                    //uniform samplerXX iChannel0..3;          // input channel. XX = 2D/Cube
                    glm::vec3 placeholder3 = glm::vec3(rx, ry, 1.0f);
                    glm::vec3 placeholder4 = glm::vec4(mx, my, mz, 1.0f);
                    glUniform3fv(10, 1, glm::value_ptr(placeholder3));
                    glUniform4fv(11, 1, glm::value_ptr(placeholder4));
                    glUniform1f(12, f);
                    glUniform1f(13, b);
                    glUniform1i(14, ipk);
                    render();
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
    glDeleteTextures(1, &texToon);
    glDeleteBuffers(1, &vbo);
    glDeleteBuffers(1, &ibo);
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
    std::cout << "2 - Increase f by 1.0" << std::endl;
    std::cout << "3 - Decrease f by 1.0" << std::endl;
    std::cout << "4 - Increase b by 1.0" << std::endl;
    std::cout << "5 - Decrease b by 1.0" << std::endl;
    std::cout << "6 - Increase ipk by 1.0" << std::endl;
    std::cout << "7 - Decrease ipk by 1.0" << std::endl;
}
