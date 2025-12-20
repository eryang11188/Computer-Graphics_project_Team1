// main.cpp
// C++ + OpenGL(3.3) + GLFW + GLAD + GLM
// Ground/Road/Sidewalk + Orbit Camera + Mouse Wheel Zoom + Borderless Window

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <vector>
#include <iostream>
#include <cmath> // std::sin, std::cos

#include "WorldConfig.h"
#include "TransformUtils.h"

// Orbit camera
float yaw = 0.0f;
float pitch = glm::radians(WC::CAM_PITCH_DEG);
float radius = WC::CAM_RADIUS;
float angularSpeed = WC::CAM_SPEED;

// Wheel zoom
float zoomSpeed = WC::ZOOM_SPEED;
float radiusMin = WC::R_MIN;
float radiusMax = WC::R_MAX;

// ----- Fence 담장 기준 계산 -----

//담장이 차지하는 반쪽 좌우,앞뒤 방향 거리
float fenceHalfW = WC::YARD_W * 0.5f + WC::FENCE_MARGIN;
float fenceHalfL = WC::YARD_L * 0.5f + WC::FENCE_MARGIN;

//앞/뒤 담장 전체 길이
float fenceLenX = WC::YARD_W + WC::FENCE_MARGIN * 2.0f + WC::FENCE_THK;
//좌/우 담장 전체 길이
float fenceLenZ = WC::YARD_L + WC::FENCE_MARGIN * 2.0f + WC::FENCE_THK;
//담장 두께
float fenceThk = WC::FENCE_THK;

//담장 색
glm::vec3 fenceColor(0.75f, 0.70f, 0.60f);

static void glfw_error_callback(int code, const char* desc) {
    std::cerr << "[GLFW ERROR] " << code << " : " << (desc ? desc : "") << "\n";
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

void scroll_callback(GLFWwindow* window, double, double yoffset) {
    radius -= (float)yoffset * zoomSpeed;
    if (radius < radiusMin) radius = radiusMin;
    if (radius > radiusMax) radius = radiusMax;
}

void processInput(GLFWwindow* window, float deltaTime) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) yaw -= angularSpeed * deltaTime;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) yaw += angularSpeed * deltaTime;

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) pitch += angularSpeed * deltaTime;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) pitch -= angularSpeed * deltaTime;

    constexpr float limit = glm::radians(89.0f);
    if (pitch > limit) pitch = limit;
    if (pitch < -limit) pitch = -limit;
}

GLuint compileShader(GLenum type, const char* src) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &src, nullptr);
    glCompileShader(shader);

    int success = 0;
    char infoLog[1024];
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(shader, 1024, nullptr, infoLog);
        std::cerr << "Shader compile error:\n" << infoLog << "\n";
    }
    return shader;
}

GLuint linkProgram(GLuint vs, GLuint fs) {
    GLuint prog = glCreateProgram();
    glAttachShader(prog, vs);
    glAttachShader(prog, fs);
    glLinkProgram(prog);

    int ok = 0;
    char infoLog[1024];
    glGetProgramiv(prog, GL_LINK_STATUS, &ok);
    if (!ok) {
        glGetProgramInfoLog(prog, 1024, nullptr, infoLog);
        std::cerr << "Program link error:\n" << infoLog << "\n";
    }
    return prog;
}

struct RenderItem {
    glm::mat4 model;
    glm::vec3 color;
};

int main() {
    glfwSetErrorCallback(glfw_error_callback);

    if (!glfwInit()) {
        std::cerr << "Failed to init GLFW\n";
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // Borderless window
    GLFWmonitor* monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* mode = glfwGetVideoMode(monitor);

    glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

    const int WIN_W = WC::WIN_W;
    const int WIN_H = WC::WIN_H;

    GLFWwindow* window = glfwCreateWindow(WIN_W, WIN_H, "World (Borderless Window)", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window\n";
        glfwTerminate();
        return -1;
    }

    // Center on monitor
    int mx, my;
    glfwGetMonitorPos(monitor, &mx, &my);
    int x = mx + (mode->width - WIN_W) / 2;
    int y = my + (mode->height - WIN_H) / 2;
    glfwSetWindowPos(window, x, y);

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetScrollCallback(window, scroll_callback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to init GLAD\n";
        glfwTerminate();
        return -1;
    }

    int fbW, fbH;
    glfwGetFramebufferSize(window, &fbW, &fbH);
    glViewport(0, 0, fbW, fbH);

    glEnable(GL_DEPTH_TEST);

    // Cube vertices (8) + indices (36)
    float vertices[] = {
        -0.5f, -0.5f, -0.5f,
         0.5f, -0.5f, -0.5f,
         0.5f,  0.5f, -0.5f,
        -0.5f,  0.5f, -0.5f,
        -0.5f, -0.5f,  0.5f,
         0.5f, -0.5f,  0.5f,
         0.5f,  0.5f,  0.5f,
        -0.5f,  0.5f,  0.5f
    };

    unsigned int indices[] = {
        0,1,2, 2,3,0,
        4,5,6, 6,7,4,
        0,4,7, 7,3,0,
        1,5,6, 6,2,1,
        0,1,5, 5,4,0,
        3,2,6, 6,7,3
    };

    unsigned int VAO, VBO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);

    const char* vertexShaderSrc = R"(
        #version 330 core
        layout (location = 0) in vec3 aPos;
        uniform mat4 model;
        uniform mat4 view;
        uniform mat4 projection;
        void main() { gl_Position = projection * view * model * vec4(aPos, 1.0); }
    )";

    const char* fragmentShaderSrc = R"(
        #version 330 core
        out vec4 FragColor;
        uniform vec3 uColor;
        void main() { FragColor = vec4(uColor, 1.0); }
    )";

    GLuint vs = compileShader(GL_VERTEX_SHADER, vertexShaderSrc);
    GLuint fs = compileShader(GL_FRAGMENT_SHADER, fragmentShaderSrc);
    GLuint shaderProgram = linkProgram(vs, fs);
    glDeleteShader(vs);
    glDeleteShader(fs);

    glUseProgram(shaderProgram);
    GLint modelLoc = glGetUniformLocation(shaderProgram, "model");
    GLint viewLoc = glGetUniformLocation(shaderProgram, "view");
    GLint projLoc = glGetUniformLocation(shaderProgram, "projection");
    GLint colorLoc = glGetUniformLocation(shaderProgram, "uColor");

    // World items
    std::vector<RenderItem> items;

    const float groundY = WC::GROUND_Y;
    const float overlayY = WC::OVERLAY_Y;

    // 1) Grass ground
    items.push_back({
        MakeModel_BottomPivot(glm::vec3(0.0f, groundY, 0.0f), glm::vec3(0.0f),
                              glm::vec3(WC::GROUND_SIZE, WC::GROUND_THK, WC::GROUND_SIZE)),
        WC::COL_GRASS
        });

    // 2) Road
    items.push_back({
        MakeModel_BottomPivot(glm::vec3(0.0f, overlayY, WC::ROAD_Z), glm::vec3(0.0f),
                              glm::vec3(WC::ROAD_W, WC::ROAD_THK, WC::ROAD_L)),
        WC::COL_ROAD
        });

    // 2-1) Center dashed line
    for (int i = -5; i <= 5; i++) {
        float z = WC::ROAD_Z + i * 10.0f;
        items.push_back({
            MakeModel_BottomPivot(glm::vec3(0.0f, overlayY + 0.001f, z), glm::vec3(0.0f),
                                  glm::vec3(0.5f, 0.01f, 4.0f)),
            WC::COL_LINE
            });
    }

    // 3) Sidewalks
    float xLeft = -(WC::ROAD_W * 0.5f + WC::SIDEWALK_W * 0.5f);
    float xRight = +(WC::ROAD_W * 0.5f + WC::SIDEWALK_W * 0.5f);

    items.push_back({
        MakeModel_BottomPivot(glm::vec3(xLeft, overlayY, WC::ROAD_Z), glm::vec3(0.0f),
                              glm::vec3(WC::SIDEWALK_W, WC::SIDEWALK_THK, WC::SIDEWALK_L)),
        WC::COL_SIDEWALK
        });

    items.push_back({
        MakeModel_BottomPivot(glm::vec3(xRight, overlayY, WC::ROAD_Z), glm::vec3(0.0f),
                              glm::vec3(WC::SIDEWALK_W, WC::SIDEWALK_THK, WC::SIDEWALK_L)),
        WC::COL_SIDEWALK
        });

    // 4) Shinchan house pivot + yard
    glm::vec3 shinHouseCenter = WC::SHIN_CENTER;

    items.push_back({
        MakeModel_BottomPivot(glm::vec3(shinHouseCenter.x, overlayY, shinHouseCenter.z), glm::vec3(0.0f),
                              glm::vec3(WC::YARD_W, WC::YARD_THK, WC::YARD_L)),
        WC::COL_YARD
        });


    // ===============================
    //          Fence(담장)
    // ===============================
    // Front
    items.push_back({
        MakeModel_BottomPivot(
            glm::vec3(0.0f, WC::GROUND_Y, shinHouseCenter.z + fenceHalfL),
            glm::vec3(0.0f),
            glm::vec3(fenceLenX, WC::FENCE_H, WC::FENCE_THK)
        ),
        fenceColor
        });

    // Back
    items.push_back({
        MakeModel_BottomPivot(
            glm::vec3(0.0f, WC::GROUND_Y, shinHouseCenter.z - fenceHalfL),
            glm::vec3(0.0f),
            glm::vec3(fenceLenX, WC::FENCE_H, WC::FENCE_THK)
        ),
        fenceColor
        });

    // Left
    items.push_back({
        MakeModel_BottomPivot(
            glm::vec3(-fenceHalfW, WC::GROUND_Y, shinHouseCenter.z),
            glm::vec3(0.0f),
            glm::vec3(WC::FENCE_THK, WC::FENCE_H, fenceLenZ)
        ),
        fenceColor
        });

    // Right
    items.push_back({
        MakeModel_BottomPivot(
            glm::vec3(+fenceHalfW, WC::GROUND_Y, shinHouseCenter.z),
            glm::vec3(0.0f),
            glm::vec3(WC::FENCE_THK, WC::FENCE_H, fenceLenZ)
        ),
        fenceColor
        });


    // 4-1) Driveway
    items.push_back({
        MakeModel_BottomPivot(glm::vec3(0.0f, overlayY, WC::DRIVE_Z), glm::vec3(0.0f),
                              glm::vec3(WC::DRIVE_W, WC::DRIVE_THK, WC::DRIVE_L)),
        glm::vec3(0.30f, 0.30f, 0.30f)
        });

    float lastFrame = 0.0f;

    while (!glfwWindowShouldClose(window)) {
        float currentFrame = (float)glfwGetTime();
        float deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window, deltaTime);

        glClearColor(0.55f, 0.75f, 0.95f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        int w, h;
        glfwGetFramebufferSize(window, &w, &h);
        float aspect = (h == 0) ? 1.0f : (float)w / (float)h;

        glm::mat4 projection = glm::perspective(glm::radians(45.0f), aspect, 0.1f, 200.0f);

        // Use std::sin/std::cos to avoid missing <cmath> issues
        float cp = (float)std::cos((double)pitch);
        float sp = (float)std::sin((double)pitch);
        float cy = (float)std::cos((double)yaw);
        float sy = (float)std::sin((double)yaw);

        glm::vec3 cameraPos;
        cameraPos.x = shinHouseCenter.x + radius * cp * sy;
        cameraPos.y = shinHouseCenter.y + radius * sp;
        cameraPos.z = shinHouseCenter.z + radius * cp * cy;

        glm::mat4 view = glm::lookAt(cameraPos, shinHouseCenter, glm::vec3(0, 1, 0));

        glUseProgram(shaderProgram);
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

        glBindVertexArray(VAO);
        for (const auto& it : items) {
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(it.model));
            glUniform3fv(colorLoc, 1, glm::value_ptr(it.color));
            glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
        }
        glBindVertexArray(0);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteProgram(shaderProgram);
    glfwTerminate();
    return 0;
}
