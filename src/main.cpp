#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <vector>
#include <iostream>
#include <cmath>
#include <ctime>

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

void framebuffer_size_callback(GLFWwindow*, int width, int height) {
    glViewport(0, 0, width, height);
}

void scroll_callback(GLFWwindow*, double, double yoffset) {
    radius -= (float)yoffset * zoomSpeed;
    if (radius < radiusMin) radius = radiusMin;
    if (radius > radiusMax) radius = radiusMax;
}

void processInput(GLFWwindow* window, float deltaTime) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) glfwSetWindowShouldClose(window, true);

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
    srand((unsigned int)time(nullptr));
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

    constexpr float YARD_SCALE_W = 2.45f;
    constexpr float YARD_SCALE_L = 2.00f;

    float yardW = WC::YARD_W * YARD_SCALE_W;
    float yardL = WC::YARD_L * YARD_SCALE_L;

    float fenceHalfW = yardW * 0.5f + WC::FENCE_MARGIN;
    float fenceHalfL = yardL * 0.5f + WC::FENCE_MARGIN;

    float fenceLenX = yardW + WC::FENCE_MARGIN * 2.0f + WC::FENCE_THK;
    float fenceLenZ = yardL + WC::FENCE_MARGIN * 2.0f + WC::FENCE_THK;
    float fenceThk = WC::FENCE_THK;

    float yardFullW = fenceHalfW * 2.0f;
    float yardFullL = fenceHalfL * 2.0f;

    float gateOffsetX = 3.5f;
    float gateW = 6.0f;

    float pillarW = 0.6f;
    float pillarH = 2.4f;

    glm::vec3 fenceColor(0.75f, 0.70f, 0.60f);

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

    auto AddBox = [&](glm::vec3 pos, glm::vec3 euler, glm::vec3 scl, glm::vec3 col, bool bottomPivot) {
        items.push_back({ bottomPivot ? MakeModel_BottomPivot(pos, euler, scl) : MakeModel_CenterPivot(pos, euler, scl), col });
        };

    items.push_back({
        MakeModel_BottomPivot(glm::vec3(0.0f, groundY, 0.0f), glm::vec3(0.0f),
                              glm::vec3(WC::GROUND_SIZE, WC::GROUND_THK, WC::GROUND_SIZE)),
        WC::COL_GRASS
        });

    items.push_back({
        MakeModel_BottomPivot(glm::vec3(center.x, overlayY, center.z), glm::vec3(0.0f),
                              glm::vec3(yardFullW, WC::YARD_THK, yardFullL)),
        WC::COL_YARD
        });

    items.push_back({
        MakeModel_BottomPivot(glm::vec3(center.x, overlayY, center.z - fenceHalfL), glm::vec3(0.0f),
                              glm::vec3(fenceLenX, WC::FENCE_H, WC::FENCE_THK)),
        fenceColor
        });

    items.push_back({
        MakeModel_BottomPivot(glm::vec3(center.x - fenceHalfW, overlayY, center.z), glm::vec3(0.0f),
                              glm::vec3(WC::FENCE_THK, WC::FENCE_H, fenceLenZ)),
        fenceColor
        });

    items.push_back({
        MakeModel_BottomPivot(glm::vec3(center.x + fenceHalfW, overlayY, center.z), glm::vec3(0.0f),
                              glm::vec3(WC::FENCE_THK, WC::FENCE_H, fenceLenZ)),
        fenceColor
        });

    items.push_back({
        MakeModel_BottomPivot(glm::vec3(leftCenterX, overlayY, center.z + fenceHalfL), glm::vec3(0.0f),
                              glm::vec3(leftLen, WC::FENCE_H, fenceThk)),
        fenceColor
        });

    items.push_back({
        MakeModel_BottomPivot(glm::vec3(rightCenterX, overlayY, center.z + fenceHalfL), glm::vec3(0.0f),
                              glm::vec3(rightLen, WC::FENCE_H, fenceThk)),
        fenceColor
        });

    items.push_back({
        MakeModel_BottomPivot(glm::vec3(gateLeftX, overlayY, center.z + fenceHalfL), glm::vec3(0.0f),
                              glm::vec3(pillarW, pillarH, pillarW)),
        glm::vec3(0.70f, 0.60f, 0.50f)
        });

    items.push_back({
        MakeModel_BottomPivot(glm::vec3(gateRightX, overlayY, center.z + fenceHalfL), glm::vec3(0.0f),
                              glm::vec3(pillarW, pillarH, pillarW)),
        glm::vec3(0.70f, 0.60f, 0.50f)
        });

    float driveW = gateW;
    float driveL = 12.0f;

    float frontFenceCenterZ = center.z + fenceHalfL;
    float frontFenceOuterZ = frontFenceCenterZ + fenceThk * 0.5f;
    float driveCenterZ = frontFenceOuterZ + driveL * 0.5f;

    AddBox(glm::vec3(gateCenterX, overlayY, driveCenterZ), glm::vec3(0.0f),
        glm::vec3(driveW, WC::DRIVE_THK, driveL), glm::vec3(0.45f, 0.45f, 0.45f), true);

    auto AddPine = [&](glm::vec3 base, float trunkH, float trunkW, glm::vec3 leafColor) {
        base.y = overlayY;

        AddBox(base, glm::vec3(0.0f), glm::vec3(trunkW, trunkH, trunkW), glm::vec3(0.35f, 0.22f, 0.12f), true);

        float y1 = trunkH * 0.65f;
        float y2 = trunkH * 0.95f;
        float y3 = trunkH * 1.20f;

        AddBox(base + glm::vec3(0.0f, y1, 0.0f), glm::vec3(0.0f),
            glm::vec3(trunkW * 6.2f, trunkH * 0.35f, trunkW * 6.2f), leafColor, true);

        AddBox(base + glm::vec3(0.0f, y2, 0.0f), glm::vec3(0.0f),
            glm::vec3(trunkW * 4.4f, trunkH * 0.30f, trunkW * 4.4f), leafColor * 0.95f, true);

        AddBox(base + glm::vec3(0.0f, y3, 0.0f), glm::vec3(0.0f),
            glm::vec3(trunkW * 2.7f, trunkH * 0.28f, trunkW * 2.7f), leafColor * 0.90f, true);
        };

    glm::vec3 leafA(0.18f, 0.45f, 0.22f);
    glm::vec3 leafB(0.15f, 0.38f, 0.20f);
    glm::vec3 leafC(0.20f, 0.52f, 0.25f);

    float outZ1 = center.z + fenceHalfL + 7.0f;
    float outZ2 = center.z + fenceHalfL + 18.0f;

    float outXR = center.x + fenceHalfW + 12.0f;
    float outXL = center.x - fenceHalfW - 12.0f;

    AddPine(glm::vec3(outXR, overlayY, outZ1), 5.2f, 0.80f, leafA);
    AddPine(glm::vec3(outXR + 5.0f, overlayY, outZ2), 6.0f, 0.88f, leafB);
    AddPine(glm::vec3(outXR - 6.0f, overlayY, outZ2 + 4.0f), 5.6f, 0.84f, leafC);

    AddPine(glm::vec3(outXL, overlayY, outZ1 + 2.2f), 5.6f, 0.84f, leafB);
    AddPine(glm::vec3(outXL - 5.0f, overlayY, outZ2 + 1.2f), 6.2f, 0.90f, leafA);

    glm::vec3 colBase(0.55f, 0.45f, 0.35f);
    glm::vec3 colWall(0.85f, 0.80f, 0.70f);
    glm::vec3 colPorch(0.78f, 0.72f, 0.62f);
    glm::vec3 colDoor(0.28f, 0.16f, 0.10f);
    glm::vec3 colWindow(0.65f, 0.80f, 0.95f);
    glm::vec3 colFrame(0.92f, 0.92f, 0.92f);
    glm::vec3 colRoof(0.65f, 0.18f, 0.18f);
    glm::vec3 colChim(0.35f, 0.22f, 0.16f);

    float H_wallW = 18.0f;
    float H_wallH = 7.6f;
    float H_wallD = 13.2f;

    float H_baseY = overlayY + WC::YARD_THK;
    float H_baseH = 0.70f;
    float H_wallY = H_baseY + H_baseH;

    glm::vec3 H_center;
    H_center.x = gateCenterX - H_wallW * 0.22f;
    H_center.y = 0.0f;
    H_center.z = center.z - yardL * 0.08f;

    AddBox(glm::vec3(H_center.x, H_baseY, H_center.z), glm::vec3(0.0f),
        glm::vec3(H_wallW + 2.0f, H_baseH, H_wallD + 2.0f), colBase, true);

    AddBox(glm::vec3(H_center.x, H_wallY, H_center.z), glm::vec3(0.0f),
        glm::vec3(H_wallW, H_wallH, H_wallD), colWall, true);

    float H_frontZ = H_center.z + H_wallD * 0.5f;
    float H_backZ = H_center.z - H_wallD * 0.5f;

    glm::vec3 H_porch = H_center + glm::vec3(H_wallW * 0.22f, 0.0f, H_wallD * 0.5f - 2.2f);
    AddBox(glm::vec3(H_porch.x, H_wallY, H_porch.z), glm::vec3(0.0f),
        glm::vec3(7.2f, 4.6f, 4.4f), colPorch, true);

    float stepH = 0.28f;
    float stepD = 0.88f;
    float stepW = 3.4f;
    float stepStartZ = H_frontZ + 0.35f;

    AddBox(glm::vec3(H_porch.x, H_baseY, stepStartZ), glm::vec3(0.0f),
        glm::vec3(stepW, stepH, stepD), colBase, true);
    AddBox(glm::vec3(H_porch.x, H_baseY + stepH, stepStartZ + stepD), glm::vec3(0.0f),
        glm::vec3(stepW, stepH, stepD), colBase, true);
    AddBox(glm::vec3(H_porch.x, H_baseY + stepH * 2.0f, stepStartZ + stepD * 2.0f), glm::vec3(0.0f),
        glm::vec3(stepW, stepH, stepD), colBase, true);

    float doorW = 2.2f;
    float doorH = 4.4f;
    float doorT = 0.22f;
    float doorZ = H_frontZ + 0.10f;

    AddBox(glm::vec3(H_porch.x, H_wallY, doorZ), glm::vec3(0.0f),
        glm::vec3(doorW, doorH, doorT), colDoor, true);

    AddBox(glm::vec3(H_porch.x + doorW * 0.30f, H_wallY + doorH * 0.55f, doorZ + 0.08f), glm::vec3(0.0f),
        glm::vec3(0.20f, 0.20f, 0.20f), colFrame, false);

    AddBox(glm::vec3(H_porch.x, H_wallY + doorH + 0.18f, doorZ - 0.55f), glm::vec3(0.0f),
        glm::vec3(3.4f, 0.26f, 1.6f), colRoof * 0.92f, true);

    float winW = 2.8f;
    float winH = 1.8f;
    float winT = 0.16f;
    float winY = H_wallY + 3.1f;
    float winZ = H_frontZ + 0.12f;

    float frameT = 0.12f;
    float frameWpad = 0.34f;
    float frameHpad = 0.26f;

    auto AddWindow = [&](float cx) {
        AddBox(glm::vec3(cx, winY, winZ), glm::vec3(0.0f),
            glm::vec3(winW, winH, winT), colWindow, true);

        AddBox(glm::vec3(cx, winY + winH - frameHpad * 0.5f, winZ + 0.08f), glm::vec3(0.0f),
            glm::vec3(winW + frameWpad, frameHpad, frameT), colFrame, true);

        AddBox(glm::vec3(cx, winY + frameHpad * 0.5f, winZ + 0.08f), glm::vec3(0.0f),
            glm::vec3(winW + frameWpad, frameHpad, frameT), colFrame, true);

        AddBox(glm::vec3(cx - (winW * 0.5f) - frameWpad * 0.25f, winY + winH * 0.5f, winZ + 0.08f), glm::vec3(0.0f),
            glm::vec3(frameHpad, winH + frameHpad * 0.2f, frameT), colFrame, true);

        AddBox(glm::vec3(cx + (winW * 0.5f) + frameWpad * 0.25f, winY + winH * 0.5f, winZ + 0.08f), glm::vec3(0.0f),
            glm::vec3(frameHpad, winH + frameHpad * 0.2f, frameT), colFrame, true);

        AddBox(glm::vec3(cx, winY + winH * 0.5f, winZ + 0.10f), glm::vec3(0.0f),
            glm::vec3(winW + 0.10f, 0.12f, frameT * 0.9f), colFrame, true);

        AddBox(glm::vec3(cx, winY + winH * 0.5f, winZ + 0.10f), glm::vec3(0.0f),
            glm::vec3(0.12f, winH + 0.10f, frameT * 0.9f), colFrame, true);
        };

    AddWindow(H_center.x - 4.8f);
    AddWindow(H_center.x + 0.6f);

    float pathW = 2.6f;
    float pathT = WC::DRIVE_THK;
    float pathStartZ = frontFenceCenterZ - 0.30f;
    float pathEndZ = H_frontZ - 1.6f;
    float pathLen = std::max(2.0f, pathStartZ - pathEndZ);
    float pathCenterZ = (pathStartZ + pathEndZ) * 0.5f;

    AddBox(glm::vec3(gateCenterX, overlayY, pathCenterZ), glm::vec3(0.0f),
        glm::vec3(pathW, pathT, pathLen), glm::vec3(0.50f, 0.50f, 0.50f), true);

    AddBox(glm::vec3(gateCenterX, overlayY, pathEndZ - 0.8f), glm::vec3(0.0f),
        glm::vec3(3.2f, pathT, 2.2f), glm::vec3(0.52f, 0.52f, 0.52f), true);

    float roofTilt = glm::radians(33.0f);
    float roofThk = 0.28f;
    float roofRise = 2.10f;

    float roofOverX = 1.35f;
    float roofOverZ = 1.60f;

    float roofCenterY = H_wallY + H_wallH + roofRise;
    float roofOffsetX = (H_wallW * 0.30f);

    float roofLenZ = H_wallD + roofOverZ * 2.0f;
    float roofPanelW = H_wallW * 0.80f + roofOverX;

    AddBox(glm::vec3(H_center.x - roofOffsetX, roofCenterY, H_center.z),
        glm::vec3(0.0f, 0.0f, +roofTilt),
        glm::vec3(roofPanelW, roofThk, roofLenZ),
        colRoof, false);

    AddBox(glm::vec3(H_center.x + roofOffsetX, roofCenterY, H_center.z),
        glm::vec3(0.0f, 0.0f, -roofTilt),
        glm::vec3(roofPanelW, roofThk, roofLenZ),
        colRoof, false);

    AddBox(glm::vec3(H_center.x, roofCenterY + 0.03f, H_center.z),
        glm::vec3(0.0f),
        glm::vec3(H_wallW * 0.08f, 0.08f, roofLenZ * 0.98f),
        colRoof * 0.90f, false);

    AddBox(glm::vec3(H_center.x - 5.2f, H_wallY + H_wallH + 0.55f, H_center.z - 2.2f),
        glm::vec3(0.0f),
        glm::vec3(1.30f, 3.8f, 1.30f),
        colChim, true);

    glm::vec3 cloudColor(0.95f, 0.95f, 0.97f);

    float cloudY = overlayY + 30.0f;

    auto AddCloud = [&](glm::vec3 c, float s)
        {
            items.push_back({
                MakeModel_CenterPivot(c, glm::vec3(0.0f),
                    glm::vec3(10.0f * s, 2.5f * s, 6.0f * s)),
                cloudColor
                });
            items.push_back({
                MakeModel_CenterPivot(c + glm::vec3(-4.0f * s, 0.8f * s, 0.0f), glm::vec3(0.0f),
                    glm::vec3(7.0f * s, 2.0f * s, 5.0f * s)),
                cloudColor
                });

            items.push_back({
                MakeModel_CenterPivot(c + glm::vec3(4.5f * s, 0.4f * s, -1.0f * s), glm::vec3(0.0f),
                    glm::vec3(6.5f * s, 1.8f * s, 4.8f * s)),
                cloudColor
                });

            items.push_back({
                MakeModel_CenterPivot(c + glm::vec3(0.0f * s, -0.4f * s, 2.0f * s), glm::vec3(0.0f),
                    glm::vec3(8.0f * s, 1.6f * s, 5.8f * s)),
                cloudColor
                });
        };

    int cloudCount = 15;
    float halfGround = WC::GROUND_SIZE * 0.5f - 10.0f;

    for (int i = 0; i < cloudCount; ++i)
    {
        float x = H_center.x + (rand() % (int)(halfGround * 2) - halfGround);
        float z = H_center.z + (rand() % (int)(halfGround * 2) - halfGround);
        float y = cloudY + (rand() % 7 - 3);
        float s = 0.8f + (rand() % 60) / 100.0f;

        AddCloud(glm::vec3(x, y, z), s);
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
