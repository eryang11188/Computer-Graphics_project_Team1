// main.cpp 


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

// ----- Fence -----
float fenceHalfW = WC::YARD_W * 0.5f + WC::FENCE_MARGIN;
float fenceHalfL = WC::YARD_L * 0.5f + WC::FENCE_MARGIN;

float fenceLenX = WC::YARD_W + WC::FENCE_MARGIN * 2.0f + WC::FENCE_THK;
float fenceLenZ = WC::YARD_L + WC::FENCE_MARGIN * 2.0f + WC::FENCE_THK;
float fenceThk = WC::FENCE_THK;

float gateOffsetX = 3.5f;
float gateW = 6.0f;
float sideFenceLen = (fenceLenX - gateW) * 0.5f;
float sideOffset = (gateW * 0.5f) + (sideFenceLen * 0.5f);

float pillarW = 0.6f;
float pillarH = 2.4f;

glm::vec3 fenceColor(0.75f, 0.70f, 0.60f);

//yard
float yardFullW = fenceHalfW * 2.0f;
float yardFullL = fenceHalfL * 2.0f;


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

    // ground / overlay
    const float groundY = WC::GROUND_Y;
    const float overlayY = WC::OVERLAY_Y;


    glm::vec3 shinHouseCenter = WC::SHIN_CENTER;

    // ----- front fence split calculation -----
    float fenceLeftX = shinHouseCenter.x - fenceLenX * 0.5f;
    float fenceRightX = shinHouseCenter.x + fenceLenX * 0.5f;

    float gateCenterX = shinHouseCenter.x + gateOffsetX;
    float gateLeftX = gateCenterX - gateW * 0.5f;
    float gateRightX = gateCenterX + gateW * 0.5f;

    float leftLen = gateLeftX - fenceLeftX;
    float rightLen = fenceRightX - gateRightX;

    float leftCenterX = fenceLeftX + leftLen * 0.5f;
    float rightCenterX = gateRightX + rightLen * 0.5f;

    // 1) Grass ground
    items.push_back({
    MakeModel_BottomPivot(glm::vec3(0.0f, groundY, 0.0f), glm::vec3(0.0f),
                          glm::vec3(WC::GROUND_SIZE, WC::GROUND_THK, WC::GROUND_SIZE)),
    WC::COL_GRASS
        });

    //yard
    items.push_back({
    MakeModel_BottomPivot(
        glm::vec3(
            shinHouseCenter.x,
            overlayY,
            shinHouseCenter.z
        ),
        glm::vec3(0.0f),
        glm::vec3(
            yardFullW,
            WC::YARD_THK,
            yardFullL
        )
    ),
    WC::COL_YARD
        });

    // ===============================
    // Fence with Entrance
    // ===============================

    items.push_back({
       MakeModel_BottomPivot(
           glm::vec3(
               shinHouseCenter.x,
               overlayY,
               shinHouseCenter.z - fenceHalfL
           ),
           glm::vec3(0.0f),
           glm::vec3(fenceLenX, WC::FENCE_H, WC::FENCE_THK)
       ),
       fenceColor
        });

    items.push_back({
       MakeModel_BottomPivot(
           glm::vec3(
               shinHouseCenter.x - fenceHalfW,
               overlayY,
               shinHouseCenter.z
           ),
           glm::vec3(0.0f),
           glm::vec3(WC::FENCE_THK, WC::FENCE_H, fenceLenZ)
       ),
       fenceColor
        });

    items.push_back({
       MakeModel_BottomPivot(
           glm::vec3(
               shinHouseCenter.x + fenceHalfW,
               overlayY,
               shinHouseCenter.z
           ),
           glm::vec3(0.0f),
           glm::vec3(WC::FENCE_THK, WC::FENCE_H, fenceLenZ)
       ),
       fenceColor
        });

    items.push_back({
     MakeModel_BottomPivot(
         glm::vec3(
             leftCenterX,
             overlayY,
             shinHouseCenter.z + fenceHalfL
         ),
         glm::vec3(0.0f),
         glm::vec3(leftLen, WC::FENCE_H, fenceThk)
     ),
     fenceColor
        });

    items.push_back({
      MakeModel_BottomPivot(
          glm::vec3(
              rightCenterX,
              overlayY,
              shinHouseCenter.z + fenceHalfL
          ),
          glm::vec3(0.0f),
          glm::vec3(rightLen, WC::FENCE_H, fenceThk)
      ),
      fenceColor
        });



    items.push_back({
    MakeModel_BottomPivot(
        glm::vec3(gateLeftX, overlayY, shinHouseCenter.z + fenceHalfL),
        glm::vec3(0.0f),
        glm::vec3(pillarW, pillarH, pillarW)
    ),
    glm::vec3(0.7f, 0.6f, 0.5f)
        });

    items.push_back({
    MakeModel_BottomPivot(
        glm::vec3(gateRightX, overlayY, shinHouseCenter.z + fenceHalfL),
        glm::vec3(0.0f),
        glm::vec3(pillarW, pillarH, pillarW)
    ),
    glm::vec3(0.7f, 0.6f, 0.5f)
        });



    // ===== Driveway =====
    float driveW = gateW;
    float driveL = 10.0f;

    float frontFenceCenterZ = shinHouseCenter.z + fenceHalfL;
    float frontFenceOuterZ = frontFenceCenterZ + fenceThk * 0.5f;

    float driveCenterZ = frontFenceOuterZ + driveL * 0.5f;

    items.push_back({
        MakeModel_BottomPivot(
            glm::vec3(
                gateCenterX,
                overlayY,
                driveCenterZ
            ),
            glm::vec3(0.0f),
            glm::vec3(
                driveW,
                WC::DRIVE_THK,
                driveL
            )
        ),
        glm::vec3(0.45f, 0.45f, 0.45f)
        });

    // --- 소나무 (계단식 사각뿔 형태) ---
    glm::vec3 pinePos = shinHouseCenter + glm::vec3(8.0f, 0.0f, 6.0f);
    pinePos.y = overlayY;

    // 1) 나무 기둥 (Brown)
    items.push_back({
        MakeModel_BottomPivot(pinePos, glm::vec3(0.0f), glm::vec3(0.7f, 3.5f, 0.7f)),
        glm::vec3(0.35f, 0.20f, 0.10f)
        });

    // 2) 나뭇잎 - 하단
    items.push_back({
        MakeModel_BottomPivot(pinePos + glm::vec3(0.0f, 2.5f, 0.0f), glm::vec3(0.0f), glm::vec3(4.0f, 1.2f, 4.0f)),
        WC::COL_GRASS
        });

    // 3) 나뭇잎 - 중단
    items.push_back({
        MakeModel_BottomPivot(pinePos + glm::vec3(0.0f, 3.7f, 0.0f), glm::vec3(0.0f), glm::vec3(2.8f, 1.0f, 2.8f)),
        WC::COL_GRASS
        });

    // 4) 나뭇잎 - 상단
    items.push_back({
        MakeModel_BottomPivot(pinePos + glm::vec3(0.0f, 4.7f, 0.0f), glm::vec3(0.0f), glm::vec3(1.5f, 1.0f, 1.5f)),
        WC::COL_GRASS
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