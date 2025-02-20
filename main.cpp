#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stb_image.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <shaders/shader_s.h>

#include <iostream>

// Forward declarations
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

// Settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// Camera globals
glm::vec3 cameraPos = glm::vec3(0.0f, 2.1f, 3.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

float deltaTime = 0.0f; // current frame - last frame
float lastFrame = 0.0f; // time of last frame

float lastX = 400.0f;
float lastY = 300.0f;
float yaw = -90.0f;
float pitch = 0.0f;
bool  firstMouse = true;

float fov = 45.0f;

int main()
{
    // -------------------------------------------------------
    // 1. Initialize GLFW and configure
    // -------------------------------------------------------
    if (!glfwInit())
    {
        std::cout << "Failed to initialize GLFW\n";
        return -1;
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // -------------------------------------------------------
    // 2. Create window and make context current
    // -------------------------------------------------------
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", nullptr, nullptr);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window\n";
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    // -------------------------------------------------------
    // 3. Load all OpenGL function pointers via GLAD
    // -------------------------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD\n";
        return -1;
    }

    // Set callbacks
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // -------------------------------------------------------
    // 4. Build and compile our shader program
    // -------------------------------------------------------
    Shader ourShader("res/shaders/infinite_grid.vs", "res/shaders/infinite_grid.fs");

    // -------------------------------------------------------
    // 5. Minimal VAO setup
    // -------------------------------------------------------
    unsigned int VAO;
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    // Enable depth test
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // final color = src * alpha + dst * (1 - alpha)

    // -------------------------------------------------------
    // 6. Render loop
    // -------------------------------------------------------
    while (!glfwWindowShouldClose(window))
    {
        // Calculate time between frames
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // Poll events
        glfwPollEvents();
        if (glfwGetWindowAttrib(window, GLFW_ICONIFIED) != 0)
        {
            // If minimized, optionally sleep to reduce CPU usage
            continue;
        }

        // Clear screen
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // 3D scene rendering
        processInput(window);

        // Use our shader
        ourShader.use();

        // Construct typical MVP matrix
        glm::mat4 model = glm::mat4(1.0f);
        glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
        glm::mat4 projection = glm::perspective(
            glm::radians(fov),
            (float)SCR_WIDTH / (float)SCR_HEIGHT,
            0.1f,
            100.0f
        );
        glm::mat4 gVP = projection * view * model;

        glm::vec3 gCameraWorldPos = cameraPos;
        // Pass gVP to shader
        int gVPLoc = glGetUniformLocation(ourShader.ID, "gVP");
        int gCameraWorldPosLoc = glGetUniformLocation(ourShader.ID, "gCameraWorldPos");
        glUniformMatrix4fv(gVPLoc, 1, GL_FALSE, glm::value_ptr(gVP));
        glUniform3fv(gCameraWorldPosLoc, 1, glm::value_ptr(gCameraWorldPos));

        // Draw the plane (generated in the shader)
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        // Swap buffers
        glfwSwapBuffers(window);
    }

    // -------------------------------------------------------
    // 7. Cleanup
    // -------------------------------------------------------
    glDeleteVertexArrays(1, &VAO);

    // Cleanup GLFW
    glfwTerminate();
    return 0;
}

// -------------------------------------------------------
// Process all input
// -------------------------------------------------------
void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    float cameraSpeed = 2.5f * deltaTime;

    // 1) Create a "horizontal" forward vector by ignoring the y-component
    glm::vec3 horizontalFront = glm::normalize(glm::vec3(cameraFront.x, 0.0f, cameraFront.z));
    // 2) Create a "horizontal" right vector also constrained to x-z plane
    glm::vec3 horizontalRight = glm::normalize(glm::cross(horizontalFront, cameraUp));

    // Move forward/back in the x-z plane only
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        cameraPos += cameraSpeed * horizontalFront;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        cameraPos -= cameraSpeed * horizontalFront;
    // Move left/right in the x-z plane only
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        cameraPos -= cameraSpeed * horizontalRight;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        cameraPos += cameraSpeed * horizontalRight;
}

// -------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

// -------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    if (firstMouse)
    {
        lastX = (float)xpos;
        lastY = (float)ypos;
        firstMouse = false;
    }

    float xoffset = (float)xpos - lastX;
    float yoffset = lastY - (float)ypos; // reversed since y-coordinates go bottom->top
    lastX = (float)xpos;
    lastY = (float)ypos;

    float sensitivity = 0.05f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw += xoffset;
    pitch += yoffset;

    if (pitch > 89.0f)  pitch = 89.0f;
    if (pitch < -89.0f) pitch = -89.0f;

    glm::vec3 front;
    front.x = cos(glm::radians(pitch)) * cos(glm::radians(yaw));
    front.y = sin(glm::radians(pitch));
    front.z = cos(glm::radians(pitch)) * sin(glm::radians(yaw));
    cameraFront = glm::normalize(front);
}

// -------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    // Optionally adjust fov
    if (fov >= 1.0f)
        fov -= (float)yoffset;
    if (fov <= 1.0f)
        fov = 1.0f;
}

