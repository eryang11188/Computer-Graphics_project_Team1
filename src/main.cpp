#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <vector>
#include <iostream>
#include <cmath>

#include "WorldConfig.h"
#include "TransformUtils.h"

float yaw = 0.0f;
float pitch = glm::radians(WC::CAM_PITCH_DEG);
float radius = WC::CAM_RADIUS;
float angularSpeed = WC::CAM_SPEED;

float zoomSpeed = WC::ZOOM_SPEED;
float radiusMin = WC::R_MIN;
float radiusMax = WC::R_MAX;

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

    GLFWmonitor* monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* mode = glfwGetVideoMode(monitor);

    glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

    const int WIN_W = WC::WIN_W;
    const int WIN_H = WC::WIN_H;

    GLFWwindow* window = glfwCreateWindow(WIN_W, WIN_H, "World", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window\n";
        glfwTerminate();
        return -1;
    }

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

    const float groundY = WC::GROUND_Y;
    const float overlayY = WC::OVERLAY_Y;
    glm::vec3 center = WC::SHIN_CENTER;

    constexpr float YARD_SCALE_W = 2.40f;
    constexpr float YARD_SCALE_L = 1.95f;

    float yardW = WC::YARD_W * YARD_SCALE_W;
    float yardL = WC::YARD_L * YARD_SCALE_L;

    float fenceHalfW = yardW * 0.5f + WC::FENCE_MARGIN;
    float fenceHalfL = yardL * 0.5f + WC::FENCE_MARGIN;

    float fenceLenX = yardW + WC::FENCE_MARGIN * 2.0f + WC::FENCE_THK;
    float fenceLenZ = yardL + WC::FENCE_MARGIN * 2.0f + WC::FENCE_THK;
    float fenceThk = WC::FENCE_THK;

    float gateOffsetX = 4.0f;
    float gateW = 7.2f;

    float pillarW = 0.7f;
    float pillarH = 2.8f;

    glm::vec3 fenceColor(0.75f, 0.70f, 0.60f);

    float yardFullW = fenceHalfW * 2.0f;
    float yardFullL = fenceHalfL * 2.0f;

    float fenceLeftX = center.x - fenceLenX * 0.5f;
    float fenceRightX = center.x + fenceLenX * 0.5f;

    float gateCenterX = center.x + gateOffsetX;
    float gateLeftX = gateCenterX - gateW * 0.5f;
    float gateRightX = gateCenterX + gateW * 0.5f;

    float leftLen = gateLeftX - fenceLeftX;
    float rightLen = fenceRightX - gateRightX;

    float leftCenterX = fenceLeftX + leftLen * 0.5f;
    float rightCenterX = gateRightX + rightLen * 0.5f;

    std::vector<RenderItem> items;

    auto AddBottom = [&](glm::vec3 pos, glm::vec3 euler, glm::vec3 scl, glm::vec3 col) {
        items.push_back({ MakeModel_BottomPivot(pos, euler, scl), col });
    };
    auto AddCenter = [&](glm::vec3 pos, glm::vec3 euler, glm::vec3 scl, glm::vec3 col) {
        items.push_back({ MakeModel_CenterPivot(pos, euler, scl), col });
    };

    AddBottom(glm::vec3(0.0f, groundY, 0.0f), glm::vec3(0.0f),
              glm::vec3(WC::GROUND_SIZE, WC::GROUND_THK, WC::GROUND_SIZE),
              WC::COL_GRASS);

    AddBottom(glm::vec3(center.x, overlayY, center.z), glm::vec3(0.0f),
              glm::vec3(yardFullW, WC::YARD_THK, yardFullL),
              WC::COL_YARD);

    AddBottom(glm::vec3(center.x, overlayY, center.z - fenceHalfL), glm::vec3(0.0f),
              glm::vec3(fenceLenX, WC::FENCE_H, WC::FENCE_THK),
              fenceColor);

    AddBottom(glm::vec3(center.x - fenceHalfW, overlayY, center.z), glm::vec3(0.0f),
              glm::vec3(WC::FENCE_THK, WC::FENCE_H, fenceLenZ),
              fenceColor);

    AddBottom(glm::vec3(center.x + fenceHalfW, overlayY, center.z), glm::vec3(0.0f),
              glm::vec3(WC::FENCE_THK, WC::FENCE_H, fenceLenZ),
              fenceColor);

    AddBottom(glm::vec3(leftCenterX, overlayY, center.z + fenceHalfL), glm::vec3(0.0f),
              glm::vec3(leftLen, WC::FENCE_H, fenceThk),
              fenceColor);

    AddBottom(glm::vec3(rightCenterX, overlayY, center.z + fenceHalfL), glm::vec3(0.0f),
              glm::vec3(rightLen, WC::FENCE_H, fenceThk),
              fenceColor);

    AddBottom(glm::vec3(gateLeftX, overlayY, center.z + fenceHalfL), glm::vec3(0.0f),
              glm::vec3(pillarW, pillarH, pillarW),
              glm::vec3(0.70f, 0.60f, 0.50f));

    AddBottom(glm::vec3(gateRightX, overlayY, center.z + fenceHalfL), glm::vec3(0.0f),
              glm::vec3(pillarW, pillarH, pillarW),
              glm::vec3(0.70f, 0.60f, 0.50f));

    float driveW = gateW;
    float driveL = 15.0f;

    float frontFenceCenterZ = center.z + fenceHalfL;
    float frontFenceOuterZ = frontFenceCenterZ + fenceThk * 0.5f;
    float driveCenterZ = frontFenceOuterZ + driveL * 0.5f;

    AddBottom(glm::vec3(gateCenterX, overlayY, driveCenterZ), glm::vec3(0.0f),
              glm::vec3(driveW, WC::DRIVE_THK, driveL),
              glm::vec3(0.45f, 0.45f, 0.45f));

    auto AddPine = [&](glm::vec3 base, float trunkH, float trunkW, glm::vec3 leafColor) {
        base.y = overlayY;

        AddBottom(base, glm::vec3(0.0f),
                  glm::vec3(trunkW, trunkH, trunkW),
                  glm::vec3(0.35f, 0.22f, 0.12f));

        float y1 = trunkH * 0.65f;
        float y2 = trunkH * 0.95f;
        float y3 = trunkH * 1.20f;

        AddBottom(base + glm::vec3(0.0f, y1, 0.0f), glm::vec3(0.0f),
                  glm::vec3(trunkW * 6.2f, trunkH * 0.35f, trunkW * 6.2f),
                  leafColor);

        AddBottom(base + glm::vec3(0.0f, y2, 0.0f), glm::vec3(0.0f),
                  glm::vec3(trunkW * 4.4f, trunkH * 0.30f, trunkW * 4.4f),
                  leafColor * 0.95f);

        AddBottom(base + glm::vec3(0.0f, y3, 0.0f), glm::vec3(0.0f),
                  glm::vec3(trunkW * 2.8f, trunkH * 0.28f, trunkW * 2.8f),
                  leafColor * 0.90f);
    };

    glm::vec3 leafA(0.18f, 0.45f, 0.22f);
    glm::vec3 leafB(0.15f, 0.38f, 0.20f);
    glm::vec3 leafC(0.20f, 0.52f, 0.25f);

    float outZ1 = center.z + fenceHalfL + 9.0f;
    float outZ2 = center.z + fenceHalfL + 22.0f;

    float outXR = center.x + fenceHalfW + 13.0f;
    float outXL = center.x - fenceHalfW - 13.0f;

    AddPine(glm::vec3(outXR, overlayY, outZ1), 5.6f, 0.85f, leafA);
    AddPine(glm::vec3(outXR + 5.5f, overlayY, outZ2), 6.3f, 0.92f, leafB);
    AddPine(glm::vec3(outXR - 6.8f, overlayY, outZ2 + 4.2f), 6.0f, 0.88f, leafC);

    AddPine(glm::vec3(outXL, overlayY, outZ1 + 2.4f), 6.0f, 0.90f, leafB);
    AddPine(glm::vec3(outXL - 5.8f, overlayY, outZ2 + 1.8f), 6.8f, 0.98f, leafA);

    glm::vec3 colBase(0.55f, 0.45f, 0.35f);
    glm::vec3 colWall(0.86f, 0.82f, 0.72f);
    glm::vec3 colTrim(0.93f, 0.93f, 0.93f);
    glm::vec3 colDoor(0.30f, 0.18f, 0.10f);
    glm::vec3 colWindow(0.65f, 0.80f, 0.95f);
    glm::vec3 colRoof(0.70f, 0.20f, 0.20f);
    glm::vec3 colWood(0.78f, 0.72f, 0.62f);
    glm::vec3 colChim(0.35f, 0.22f, 0.16f);
    glm::vec3 colRail(0.85f, 0.85f, 0.85f);

    auto AddBox = [&](glm::vec3 pos, glm::vec3 euler, glm::vec3 scl, glm::vec3 col, bool bottomPivot) {
        if (bottomPivot) items.push_back({ MakeModel_BottomPivot(pos, euler, scl), col });
        else items.push_back({ MakeModel_CenterPivot(pos, euler, scl), col });
    };

    auto AddHipRoof = [&](glm::vec3 roofCenter, float roofW, float roofD, float pitchRad, float thk, glm::vec3 col) {
        float offX = roofW * 0.22f;
        float offZ = roofD * 0.22f;

        AddBox(roofCenter + glm::vec3(-offX, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, +pitchRad),
               glm::vec3(roofW * 0.62f, thk, roofD), col, false);

        AddBox(roofCenter + glm::vec3(+offX, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, -pitchRad),
               glm::vec3(roofW * 0.62f, thk, roofD), col, false);

        AddBox(roofCenter + glm::vec3(0.0f, 0.0f, +offZ), glm::vec3(-pitchRad, 0.0f, 0.0f),
               glm::vec3(roofW, thk, roofD * 0.62f), col, false);

        AddBox(roofCenter + glm::vec3(0.0f, 0.0f, -offZ), glm::vec3(+pitchRad, 0.0f, 0.0f),
               glm::vec3(roofW, thk, roofD * 0.62f), col, false);

        AddBox(roofCenter + glm::vec3(0.0f, 0.12f, 0.0f), glm::vec3(0.0f),
               glm::vec3(roofW * 0.10f, 0.14f, roofD * 0.10f), col * 0.92f, false);
    };

    {
        glm::vec3 Hc = center + glm::vec3(-yardW * 0.08f, 0.0f, -yardL * 0.10f);

        float slabY = overlayY + WC::YARD_THK;
        float slabH = 0.60f;

        float W1 = 19.0f;
        float H1 = 4.4f;
        float D1 = 13.6f;

        float W2 = 15.2f;
        float H2 = 3.6f;
        float D2 = 11.0f;

        AddBox(glm::vec3(Hc.x, slabY, Hc.z), glm::vec3(0.0f),
               glm::vec3(W1 + 2.4f, slabH, D1 + 2.2f), colBase, true);

        float f1Y = slabY + slabH;
        AddBox(glm::vec3(Hc.x, f1Y, Hc.z), glm::vec3(0.0f),
               glm::vec3(W1, H1, D1), colWall, true);

        glm::vec3 H2c = Hc + glm::vec3(-0.6f, 0.0f, -1.2f);
        float f2Y = f1Y + H1;
        AddBox(glm::vec3(H2c.x, f2Y, H2c.z), glm::vec3(0.0f),
               glm::vec3(W2, H2, D2), colWall, true);

        float frontZ1 = Hc.z + D1 * 0.5f;
        float backZ1  = Hc.z - D1 * 0.5f;

        glm::vec3 porch = Hc + glm::vec3(W1 * 0.20f, 0.0f, D1 * 0.5f - 2.4f);
        AddBox(glm::vec3(porch.x, f1Y, porch.z), glm::vec3(0.0f),
               glm::vec3(7.2f, 3.8f, 4.0f), colWood, true);

        float stepH = 0.28f;
        float stepD = 0.85f;
        float stepW = 3.4f;
        float stepStartZ = frontZ1 + 0.40f;

        AddBox(glm::vec3(porch.x, slabY, stepStartZ), glm::vec3(0.0f),
               glm::vec3(stepW, stepH, stepD), colBase, true);
        AddBox(glm::vec3(porch.x, slabY + stepH, stepStartZ + stepD), glm::vec3(0.0f),
               glm::vec3(stepW, stepH, stepD), colBase, true);
        AddBox(glm::vec3(porch.x, slabY + stepH * 2.0f, stepStartZ + stepD * 2.0f), glm::vec3(0.0f),
               glm::vec3(stepW, stepH, stepD), colBase, true);

        float doorW = 1.9f;
        float doorH = 3.9f;
        float doorT = 0.22f;
        AddBox(glm::vec3(porch.x, f1Y, frontZ1 + 0.12f), glm::vec3(0.0f),
               glm::vec3(doorW, doorH, doorT), colDoor, true);

        AddBox(glm::vec3(porch.x + doorW * 0.30f, f1Y + doorH * 0.55f, frontZ1 + 0.20f), glm::vec3(0.0f),
               glm::vec3(0.16f, 0.16f, 0.16f), colTrim, false);

        float winW = 2.6f;
        float winH = 1.7f;
        float winT = 0.16f;
        float winY = f1Y + 2.7f;
        float winZ = frontZ1 + 0.12f;

        float frameT = 0.12f;
        float frameWpad = 0.30f;
        float frameHpad = 0.22f;

        auto AddWindow = [&](float cx, float cy, float cz) {
            AddBox(glm::vec3(cx, cy, cz), glm::vec3(0.0f),
                   glm::vec3(winW, winH, winT), colWindow, true);

            AddBox(glm::vec3(cx, cy + winH - frameHpad * 0.5f, cz + 0.08f), glm::vec3(0.0f),
                   glm::vec3(winW + frameWpad, frameHpad, frameT), colTrim, true);

            AddBox(glm::vec3(cx, cy + frameHpad * 0.5f, cz + 0.08f), glm::vec3(0.0f),
                   glm::vec3(winW + frameWpad, frameHpad, frameT), colTrim, true);

            AddBox(glm::vec3(cx - (winW * 0.5f) - frameWpad * 0.25f, cy + winH * 0.5f, cz + 0.08f), glm::vec3(0.0f),
                   glm::vec3(frameHpad, winH + frameHpad * 0.2f, frameT), colTrim, true);

            AddBox(glm::vec3(cx + (winW * 0.5f) + frameWpad * 0.25f, cy + winH * 0.5f, cz + 0.08f), glm::vec3(0.0f),
                   glm::vec3(frameHpad, winH + frameHpad * 0.2f, frameT), colTrim, true);

            AddBox(glm::vec3(cx, cy + winH * 0.5f, cz + 0.10f), glm::vec3(0.0f),
                   glm::vec3(winW + 0.10f, 0.12f, frameT * 0.9f), colTrim, true);

            AddBox(glm::vec3(cx, cy + winH * 0.5f, cz + 0.10f), glm::vec3(0.0f),
                   glm::vec3(0.12f, winH + 0.10f, frameT * 0.9f), colTrim, true);
        };

        AddWindow(Hc.x - 5.2f, winY, winZ);
        AddWindow(Hc.x + 0.0f, winY, winZ);

        float f2FrontZ = H2c.z + D2 * 0.5f;
        float f2WinY = f2Y + 2.0f;
        AddBox(glm::vec3(H2c.x + 3.2f, f2WinY, f2FrontZ + 0.12f), glm::vec3(0.0f),
               glm::vec3(1.4f, 1.9f, 0.16f), colWindow, true);

        float canopyY = f2Y - 0.05f;
        float canopyW = W2 + 5.0f;
        float canopyD = D2 + 4.2f;
        constexpr float canopyPitch = glm::radians(14.0f);

        glm::vec3 canopyCenter(H2c.x + 0.6f, canopyY + 0.9f, H2c.z + 1.4f);
        AddHipRoof(canopyCenter, canopyW, canopyD, canopyPitch, 0.26f, colRoof);

        float balY = f2Y + 0.60f;
        float balH = 1.05f;
        float balZ = H2c.z + D2 * 0.5f - 0.55f;
        float balW = W2 * 0.72f;

        AddBox(glm::vec3(H2c.x - 0.4f, balY, balZ), glm::vec3(0.0f),
               glm::vec3(balW, 0.18f, 0.18f), colRail, true);

        AddBox(glm::vec3(H2c.x - 0.4f - balW * 0.5f, balY, balZ - 0.8f), glm::vec3(0.0f),
               glm::vec3(0.18f, balH, 1.8f), colRail, true);

        AddBox(glm::vec3(H2c.x - 0.4f + balW * 0.5f, balY, balZ - 0.8f), glm::vec3(0.0f),
               glm::vec3(0.18f, balH, 1.8f), colRail, true);

        AddBox(glm::vec3(H2c.x - 0.4f, balY + balH - 0.12f, balZ - 0.8f), glm::vec3(0.0f),
               glm::vec3(balW + 0.36f, 0.18f, 1.8f), colRail, true);

        float roofBaseY = f2Y + H2;
        glm::vec3 roofCenter(H2c.x, roofBaseY + 1.2f, H2c.z);
        constexpr float roofPitch = glm::radians(24.0f);
        AddHipRoof(roofCenter, W2 + 6.2f, D2 + 6.2f, roofPitch, 0.30f, colRoof);

        AddBox(glm::vec3(roofCenter.x - 4.8f, roofBaseY + 0.6f, roofCenter.z - 2.2f), glm::vec3(0.0f),
               glm::vec3(1.35f, 3.8f, 1.35f), colChim, true);

        glm::vec3 carCenter = Hc + glm::vec3(W1 * 0.60f + 4.8f, 0.0f, D1 * 0.18f);
        float carBaseY = slabY + slabH;
        float carH = 2.9f;

        AddBox(glm::vec3(carCenter.x, carBaseY, carCenter.z), glm::vec3(0.0f),
               glm::vec3(7.4f, 0.40f, 6.8f), colBase, true);

        AddBox(glm::vec3(carCenter.x - 3.2f, carBaseY, carCenter.z + 2.8f), glm::vec3(0.0f),
               glm::vec3(0.45f, carH, 0.45f), colWood * 0.90f, true);

        AddBox(glm::vec3(carCenter.x - 3.2f, carBaseY, carCenter.z - 2.8f), glm::vec3(0.0f),
               glm::vec3(0.45f, carH, 0.45f), colWood * 0.90f, true);

        AddBox(glm::vec3(carCenter.x + 3.2f, carBaseY, carCenter.z + 2.8f), glm::vec3(0.0f),
               glm::vec3(0.45f, carH, 0.45f), colWood * 0.90f, true);

        AddBox(glm::vec3(carCenter.x + 3.2f, carBaseY, carCenter.z - 2.8f), glm::vec3(0.0f),
               glm::vec3(0.45f, carH, 0.45f), colWood * 0.90f, true);

        glm::vec3 carRoofCenter(carCenter.x, carBaseY + carH + 0.35f, carCenter.z);
        constexpr float carPitch = glm::radians(12.0f);
        AddHipRoof(carRoofCenter, 8.2f, 7.6f, carPitch, 0.26f, colRoof);
    }

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

        glm::mat4 projection = glm::perspective(glm::radians(45.0f), aspect, 0.1f, 260.0f);

        float cp = (float)std::cos((double)pitch);
        float sp = (float)std::sin((double)pitch);
        float cy = (float)std::cos((double)yaw);
        float sy = (float)std::sin((double)yaw);

        glm::vec3 cameraPos;
        cameraPos.x = center.x + radius * cp * sy;
        cameraPos.y = center.y + radius * sp;
        cameraPos.z = center.z + radius * cp * cy;

        glm::mat4 view = glm::lookAt(cameraPos, center, glm::vec3(0, 1, 0));

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
