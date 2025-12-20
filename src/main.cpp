#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <vector>
#include <iostream>
#include <cmath>
#include <ctime>
#include <cstdlib>
#include <algorithm>

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

    yaw = glm::radians(28.0f);
    pitch = glm::radians(19.0f);
    radius = std::max(radiusMin, std::min(radiusMax, 78.0f));

    constexpr float YARD_SCALE_W = 2.40f;
    constexpr float YARD_SCALE_L = 1.95f;

    float yardW0 = WC::YARD_W * YARD_SCALE_W;
    float yardL0 = WC::YARD_L * YARD_SCALE_L;

    float fenceThk = WC::FENCE_THK;

    float fenceHalfW = yardW0 * 0.5f + WC::FENCE_MARGIN;
    float fenceHalfL = yardL0 * 0.5f + WC::FENCE_MARGIN;

    float fenceInsetW = 26.0f;
    float fenceInsetL = 6.8f;

    fenceHalfW = std::max(4.0f, fenceHalfW - fenceInsetW);
    fenceHalfL = std::max(4.0f, fenceHalfL - fenceInsetL);

    float fenceLenX = fenceHalfW * 2.0f + fenceThk;
    float fenceLenZ = fenceHalfL * 2.0f + fenceThk;

    float gateOffsetX = 4.0f;
    float gateW = 7.2f;

    float fenceH = WC::FENCE_H * 1.90f;

    float pillarW = 0.7f;
    float pillarH = fenceH + 0.40f;

    glm::vec3 wallColor(0.92f, 0.85f, 0.55f);
    glm::vec3 capColor(0.86f, 0.79f, 0.50f);
    glm::vec3 hedgeColor(0.12f, 0.45f, 0.15f);

    float hedgeHh = fenceH * 0.55f;
    float hedgeY = overlayY + fenceH * 0.22f;
    float hedgeThk2 = 0.16f;

    float yardFullW = fenceHalfW * 2.0f;
    float yardFullL = fenceHalfL * 2.0f;

    float yardW = yardFullW;
    float yardL = yardFullL;

    float fenceLeftX = center.x - fenceLenX * 0.5f;
    float fenceRightX = center.x + fenceLenX * 0.5f;

    float gateCenterX = center.x + gateOffsetX;
    float gateLeftX = gateCenterX - gateW * 0.5f;
    float gateRightX = gateCenterX + gateW * 0.5f;

    float leftLen = gateLeftX - fenceLeftX;
    float rightLen = fenceRightX - gateRightX;

    float leftCenterX = fenceLeftX + leftLen * 0.5f;
    float rightCenterX = gateRightX + rightLen * 0.5f;

    float frontFenceCenterZ = center.z + fenceHalfL;
    float frontFenceOuterZ = frontFenceCenterZ + fenceThk * 0.5f;

    float roadW = gateW + 1.6f;
    float roadL = 92.0f;
    float roadCenterZ = frontFenceOuterZ + roadL * 0.5f;

    std::vector<RenderItem> items;

    auto AddBottom = [&](glm::vec3 pos, glm::vec3 euler, glm::vec3 scl, glm::vec3 col) {
        items.push_back({ MakeModel_BottomPivot(pos, euler, scl), col });
        };
    auto AddCenter = [&](glm::vec3 pos, glm::vec3 euler, glm::vec3 scl, glm::vec3 col) {
        items.push_back({ MakeModel_CenterPivot(pos, euler, scl), col });
        };
    auto AddBox = [&](glm::vec3 pos, glm::vec3 euler, glm::vec3 scl, glm::vec3 col, bool bottomPivot) {
        if (bottomPivot) items.push_back({ MakeModel_BottomPivot(pos, euler, scl), col });
        else items.push_back({ MakeModel_CenterPivot(pos, euler, scl), col });
        };

    AddBottom(glm::vec3(0.0f, groundY, 0.0f), glm::vec3(0.0f),
        glm::vec3(WC::GROUND_SIZE, WC::GROUND_THK, WC::GROUND_SIZE),
        WC::COL_GRASS);

    AddBottom(glm::vec3(center.x, overlayY, center.z), glm::vec3(0.0f),
        glm::vec3(yardFullW, WC::YARD_THK, yardFullL),
        WC::COL_YARD);

    auto AddWallX = [&](float cx, float cz, float len, bool addHedge) {
        AddBottom(glm::vec3(cx, overlayY, cz), glm::vec3(0.0f),
            glm::vec3(len, fenceH, fenceThk), wallColor);
        float capHh2 = 0.20f;
        float capThk2 = fenceThk * 0.82f;
        AddBottom(glm::vec3(cx, overlayY + fenceH, cz), glm::vec3(0.0f),
            glm::vec3(len, capHh2, capThk2), capColor);
        if (addHedge) {
            AddBottom(glm::vec3(cx, hedgeY, cz), glm::vec3(0.0f),
                glm::vec3(len * 0.72f, hedgeHh, hedgeThk2), hedgeColor);
        }
        };
    auto AddWallZ = [&](float cx, float cz, float len, bool addHedge) {
        AddBottom(glm::vec3(cx, overlayY, cz), glm::vec3(0.0f),
            glm::vec3(fenceThk, fenceH, len), wallColor);
        float capHh2 = 0.20f;
        float capThk2 = fenceThk * 0.82f;
        AddBottom(glm::vec3(cx, overlayY + fenceH, cz), glm::vec3(0.0f),
            glm::vec3(capThk2, capHh2, len), capColor);
        if (addHedge) {
            AddBottom(glm::vec3(cx, hedgeY, cz), glm::vec3(0.0f),
                glm::vec3(hedgeThk2, hedgeHh, len * 0.72f), hedgeColor);
        }
        };

    AddWallX(center.x, center.z - fenceHalfL, fenceLenX, true);
    AddWallZ(center.x - fenceHalfW, center.z, fenceLenZ, true);
    AddWallZ(center.x + fenceHalfW, center.z, fenceLenZ, true);

    AddWallX(leftCenterX, center.z + fenceHalfL, leftLen, false);
    AddWallX(rightCenterX, center.z + fenceHalfL, rightLen, false);

    float hedgeGapFromGate = 6.2f;

    float hedgeLenL = std::max(0.0f, leftLen - hedgeGapFromGate);
    if (hedgeLenL > 1.0f) {
        float hx = fenceLeftX + hedgeLenL * 0.5f;
        AddBottom(glm::vec3(hx, hedgeY, center.z + fenceHalfL), glm::vec3(0.0f),
            glm::vec3(hedgeLenL * 0.92f, hedgeHh, hedgeThk2), hedgeColor);
    }

    float hedgeLenR = std::max(0.0f, rightLen - hedgeGapFromGate);
    if (hedgeLenR > 1.0f) {
        float hx = fenceRightX - hedgeLenR * 0.5f;
        AddBottom(glm::vec3(hx, hedgeY, center.z + fenceHalfL), glm::vec3(0.0f),
            glm::vec3(hedgeLenR * 0.92f, hedgeHh, hedgeThk2), hedgeColor);
    }

    AddBottom(glm::vec3(gateLeftX, overlayY, center.z + fenceHalfL), glm::vec3(0.0f),
        glm::vec3(pillarW, pillarH, pillarW),
        glm::vec3(0.82f, 0.76f, 0.52f));

    AddBottom(glm::vec3(gateRightX, overlayY, center.z + fenceHalfL), glm::vec3(0.0f),
        glm::vec3(pillarW, pillarH, pillarW),
        glm::vec3(0.82f, 0.76f, 0.52f));

    {
        glm::vec3 postCol(0.25f, 0.25f, 0.28f);
        glm::vec3 boxCol(0.78f, 0.18f, 0.18f);
        glm::vec3 slotCol(0.10f, 0.10f, 0.10f);
        glm::vec3 flagCol(0.95f, 0.25f, 0.20f);

        
        const float MB_SCALE = 2.2f; 

        float mbX = gateRightX + 1.35f + 1.10f * (MB_SCALE - 1.0f); 
        float mbZ = frontFenceOuterZ + 1.00f + 0.80f * (MB_SCALE - 1.0f); 
        float mbY = overlayY;

        
        AddBottom(glm::vec3(mbX, mbY, mbZ), glm::vec3(0.0f),
            glm::vec3(0.90f * MB_SCALE, 0.12f * MB_SCALE, 0.90f * MB_SCALE), postCol * 0.85f);

        
        AddBottom(glm::vec3(mbX, mbY + 0.12f * MB_SCALE, mbZ), glm::vec3(0.0f),
            glm::vec3(0.18f * MB_SCALE, 1.25f * MB_SCALE, 0.18f * MB_SCALE), postCol);

        


        float boxY = mbY + 0.12f * MB_SCALE + 1.25f * MB_SCALE;
        AddBottom(glm::vec3(mbX, boxY, mbZ), glm::vec3(0.0f),
            glm::vec3(0.92f * MB_SCALE, 0.52f * MB_SCALE, 0.52f * MB_SCALE), boxCol);

       
            
        AddCenter(glm::vec3(mbX, boxY + 0.32f * MB_SCALE, mbZ + 0.29f * MB_SCALE), glm::vec3(0.0f),
            glm::vec3(0.60f * MB_SCALE, 0.10f * MB_SCALE, 0.05f * MB_SCALE), slotCol);

        
        AddCenter(glm::vec3(mbX + 0.52f * MB_SCALE, boxY + 0.34f * MB_SCALE, mbZ), glm::vec3(0.0f),
            glm::vec3(0.12f * MB_SCALE, 0.40f * MB_SCALE, 0.08f * MB_SCALE), flagCol);
    }

    AddBottom(glm::vec3(gateCenterX, overlayY, roadCenterZ), glm::vec3(0.0f),
        glm::vec3(roadW, WC::DRIVE_THK, roadL),
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

    float outZ1 = center.z + fenceHalfL + 16.0f;
    float outZ2 = center.z + fenceHalfL + 30.0f;

    float outXR = center.x + fenceHalfW + 15.0f;
    float outXL = center.x - fenceHalfW - 15.0f;

    AddPine(glm::vec3(outXR, overlayY, outZ1), 5.6f, 0.85f, leafA);
    AddPine(glm::vec3(outXR + 5.5f, overlayY, outZ2), 6.3f, 0.92f, leafB);
    AddPine(glm::vec3(outXR - 6.8f, overlayY, outZ2 + 4.2f), 6.0f, 0.88f, leafC);

    AddPine(glm::vec3(outXL, overlayY, outZ1 + 2.4f), 6.0f, 0.90f, leafB);
    AddPine(glm::vec3(outXL - 5.8f, overlayY, outZ2 + 1.8f), 6.8f, 0.98f, leafA);

    auto AddStreetLight = [&](glm::vec3 base, float poleH, float poleW) {
        glm::vec3 poleCol(0.35f, 0.35f, 0.38f);
        glm::vec3 lampCol(0.98f, 0.95f, 0.70f);
        base.y = overlayY;

        float lampScale = 1.85f;

        AddBottom(base, glm::vec3(0.0f),
            glm::vec3(poleW * 3.6f, poleW * 0.70f, poleW * 3.6f), poleCol * 0.90f);

        AddBottom(base + glm::vec3(0.0f, poleW * 0.70f, 0.0f), glm::vec3(0.0f),
            glm::vec3(poleW, poleH, poleW), poleCol);

        float topY = base.y + poleW * 0.70f + poleH;
        AddBottom(glm::vec3(base.x, topY, base.z), glm::vec3(0.0f),
            glm::vec3(poleW * 2.6f, poleW * 0.22f, poleW * 2.6f), poleCol);

        float armL = poleW * 4.2f;
        AddCenter(glm::vec3(base.x + armL * 0.5f, topY - poleW * 0.28f, base.z), glm::vec3(0.0f),
            glm::vec3(armL, poleW * 0.22f, poleW * 0.22f), poleCol);

        AddCenter(glm::vec3(base.x + armL, topY - poleW * 0.62f, base.z), glm::vec3(0.0f),
            glm::vec3(poleW * 0.95f * lampScale, poleW * 0.52f * lampScale, poleW * 1.35f * lampScale), lampCol);
        };

    float lampOffsetX = roadW * 0.5f + 4.2f;
    float lampStartZ = frontFenceOuterZ + 7.0f;
    float lampEndZ = frontFenceOuterZ + roadL - 5.0f;
    float lampStep = 12.0f;

    for (float z0 = lampStartZ; z0 <= lampEndZ; z0 += lampStep) {
        AddStreetLight(glm::vec3(gateCenterX - lampOffsetX, overlayY, z0), 10.6f, 0.36f);
        AddStreetLight(glm::vec3(gateCenterX + lampOffsetX, overlayY, z0), 10.6f, 0.36f);
    }

    AddPine(glm::vec3(center.x - fenceHalfW - 18.0f, overlayY, center.z - 10.0f), 6.4f, 0.95f, leafC);
    AddPine(glm::vec3(center.x - fenceHalfW - 24.0f, overlayY, center.z + 12.0f), 7.2f, 1.05f, leafB);
    AddPine(glm::vec3(center.x + fenceHalfW + 20.0f, overlayY, center.z - 18.0f), 6.8f, 1.00f, leafA);
    AddPine(glm::vec3(center.x + fenceHalfW + 28.0f, overlayY, center.z + 6.0f), 7.6f, 1.08f, leafC);

    for (int i = 0; i < 8; ++i) {
        float ringX = fenceHalfW + 22.0f + (float)(rand() % 25);
        float ringZ = fenceHalfL + 18.0f + (float)(rand() % 30);
        float sx = (rand() % 2 ? 1.0f : -1.0f) * ringX + center.x;
        float sz = (rand() % 2 ? 1.0f : -1.0f) * ringZ + center.z;
        float h = 5.8f + (float)(rand() % 20) / 10.0f;
        float w = 0.82f + (float)(rand() % 18) / 100.0f;
        glm::vec3 lc = (rand() % 3 == 0) ? leafA : ((rand() % 2) ? leafB : leafC);
        AddPine(glm::vec3(sx, overlayY, sz), h, w, lc);
    }

    glm::vec3 colBase(0.55f, 0.45f, 0.35f);
    glm::vec3 colWall(0.86f, 0.82f, 0.72f);
    glm::vec3 colTrim(0.93f, 0.93f, 0.93f);
    glm::vec3 colDoor(0.30f, 0.18f, 0.10f);
    glm::vec3 colWindow(0.65f, 0.80f, 0.95f);
    glm::vec3 colRoof(0.70f, 0.20f, 0.20f);
    glm::vec3 colWood(0.78f, 0.72f, 0.62f);
    glm::vec3 colChim(0.35f, 0.22f, 0.16f);
    glm::vec3 colRail(0.85f, 0.85f, 0.85f);

    auto AddRectWindowZ = [&](float cx, float by, float cz, float w, float h, float s, float zSign) {
        float glassT = 0.06f * s;
        float frameT = 0.05f * s;
        float inset = 0.14f * s;
        float barT = 0.045f * s;

        float zGlass = cz + 0.022f * s * zSign;
        float zFrame = cz + 0.030f * s * zSign;
        float zBar = cz + 0.032f * s * zSign;

        AddBox(glm::vec3(cx, by, zGlass), glm::vec3(0.0f),
            glm::vec3(w, h, glassT), colWindow, true);

        float fw = std::min(inset, w * 0.22f);
        float fh = std::min(inset, h * 0.22f);
        float innerW = std::max(0.01f, w - 2.0f * fw);
        float innerH = std::max(0.01f, h - 2.0f * fh);

        AddBox(glm::vec3(cx, by + h - fh, zFrame), glm::vec3(0.0f),
            glm::vec3(w, fh, frameT), colTrim, true);
        AddBox(glm::vec3(cx, by, zFrame), glm::vec3(0.0f),
            glm::vec3(w, fh, frameT), colTrim, true);
        AddBox(glm::vec3(cx - (w * 0.5f) + fw, by, zFrame), glm::vec3(0.0f),
            glm::vec3(fw, h, frameT), colTrim, true);
        AddBox(glm::vec3(cx + (w * 0.5f) - fw, by, zFrame), glm::vec3(0.0f),
            glm::vec3(fw, h, frameT), colTrim, true);

        float midY = by + fh + innerH * 0.5f - (barT * 0.5f);
        AddBox(glm::vec3(cx, midY, zBar), glm::vec3(0.0f),
            glm::vec3(innerW, barT, frameT), colTrim, true);

        AddBox(glm::vec3(cx, by + fh, zBar), glm::vec3(0.0f),
            glm::vec3(barT, innerH, frameT), colTrim, true);
        };

    auto AddWideWindow3Z = [&](float cx, float by, float cz, float w, float h, float s, float zSign) {
        float glassT = 0.06f * s;
        float frameT = 0.05f * s;
        float inset = 0.14f * s;
        float mullT = 0.05f * s;
        float barT = 0.045f * s;

        float zGlass = cz + 0.022f * s * zSign;
        float zFrame = cz + 0.030f * s * zSign;
        float zBar = cz + 0.032f * s * zSign;

        AddBox(glm::vec3(cx, by, zGlass), glm::vec3(0.0f),
            glm::vec3(w, h, glassT), colWindow, true);

        float fw = std::min(inset, w * 0.18f);
        float fh = std::min(inset, h * 0.22f);
        float innerW = std::max(0.01f, w - 2.0f * fw);
        float innerH = std::max(0.01f, h - 2.0f * fh);

        AddBox(glm::vec3(cx, by + h - fh, zFrame), glm::vec3(0.0f),
            glm::vec3(w, fh, frameT), colTrim, true);
        AddBox(glm::vec3(cx, by, zFrame), glm::vec3(0.0f),
            glm::vec3(w, fh, frameT), colTrim, true);
        AddBox(glm::vec3(cx - (w * 0.5f) + fw, by, zFrame), glm::vec3(0.0f),
            glm::vec3(fw, h, frameT), colTrim, true);
        AddBox(glm::vec3(cx + (w * 0.5f) - fw, by, zFrame), glm::vec3(0.0f),
            glm::vec3(fw, h, frameT), colTrim, true);

        float mullX1 = cx - innerW * (1.0f / 3.0f) * 0.5f;
        float mullX2 = cx + innerW * (1.0f / 3.0f) * 0.5f;

        AddBox(glm::vec3(mullX1, by + fh, zBar), glm::vec3(0.0f),
            glm::vec3(mullT, innerH, frameT), colTrim, true);
        AddBox(glm::vec3(mullX2, by + fh, zBar), glm::vec3(0.0f),
            glm::vec3(mullT, innerH, frameT), colTrim, true);

        float midY = by + fh + innerH * 0.5f - (barT * 0.5f);
        AddBox(glm::vec3(cx, midY, zBar), glm::vec3(0.0f),
            glm::vec3(innerW, barT, frameT), colTrim, true);
        };

    auto AddRectWindowX = [&](float xw, float by, float cz, float wZ, float h, float s, float xSign) {
        float glassT = 0.06f * s;
        float frameT = 0.05f * s;
        float inset = 0.14f * s;
        float barT = 0.045f * s;

        float xGlass = xw + 0.022f * s * xSign;
        float xFrame = xw + 0.030f * s * xSign;
        float xBar = xw + 0.032f * s * xSign;

        AddBox(glm::vec3(xGlass, by, cz), glm::vec3(0.0f),
            glm::vec3(glassT, h, wZ), colWindow, true);

        float fw = std::min(inset, wZ * 0.22f);
        float fh = std::min(inset, h * 0.22f);
        float innerW = std::max(0.01f, wZ - 2.0f * fw);
        float innerH = std::max(0.01f, h - 2.0f * fh);

        AddBox(glm::vec3(xFrame, by + h - fh, cz), glm::vec3(0.0f),
            glm::vec3(frameT, fh, wZ), colTrim, true);
        AddBox(glm::vec3(xFrame, by, cz), glm::vec3(0.0f),
            glm::vec3(frameT, fh, wZ), colTrim, true);

        AddBox(glm::vec3(xFrame, by, cz - (wZ * 0.5f) + fw), glm::vec3(0.0f),
            glm::vec3(frameT, h, fw), colTrim, true);
        AddBox(glm::vec3(xFrame, by, cz + (wZ * 0.5f) - fw), glm::vec3(0.0f),
            glm::vec3(frameT, h, fw), colTrim, true);

        float midY = by + fh + innerH * 0.5f - (barT * 0.5f);
        AddBox(glm::vec3(xBar, midY, cz), glm::vec3(0.0f),
            glm::vec3(frameT, barT, innerW), colTrim, true);

        AddBox(glm::vec3(xBar, by + fh, cz), glm::vec3(0.0f),
            glm::vec3(frameT, innerH, barT), colTrim, true);
        };

    auto AddDeckSkirtRoof = [&](glm::vec3 centerXZ, float eaveY,
        float deckW, float deckD,
        float outerW, float outerD,
        float pitchRad,
        float panelThk, float deckThk,
        glm::vec3 roofCol, glm::vec3 deckCol) -> float
        {
            float innerHalfW = deckW * 0.5f;
            float innerHalfD = deckD * 0.5f;
            float outerHalfW = outerW * 0.5f;
            float outerHalfD = outerD * 0.5f;

            float spanX = std::max(0.01f, outerHalfW - innerHalfW);
            float spanZ = std::max(0.01f, outerHalfD - innerHalfD);

            float drop = std::max(spanX, spanZ) * std::sin(pitchRad);
            float innerEdgeY = eaveY + drop;
            float deckTopY = innerEdgeY + 0.01f;

            AddBox(glm::vec3(centerXZ.x, deckTopY - deckThk, centerXZ.z), glm::vec3(0.0f),
                glm::vec3(deckW, deckThk, deckD), deckCol, true);

            float frontSpan = spanZ;
            float sideSpan = spanX;

            float frontZc = centerXZ.z + (innerHalfD + frontSpan * 0.5f);
            float backZc = centerXZ.z - (innerHalfD + frontSpan * 0.5f);
            float rightXc = centerXZ.x + (innerHalfW + sideSpan * 0.5f);
            float leftXc = centerXZ.x - (innerHalfW + sideSpan * 0.5f);

            float cy2 = eaveY + drop * 0.5f;

            AddBox(glm::vec3(centerXZ.x, cy2, frontZc), glm::vec3(+pitchRad, 0.0f, 0.0f),
                glm::vec3(outerW, panelThk, frontSpan), roofCol, false);
            AddBox(glm::vec3(centerXZ.x, cy2, backZc), glm::vec3(-pitchRad, 0.0f, 0.0f),
                glm::vec3(outerW, panelThk, frontSpan), roofCol, false);

            AddBox(glm::vec3(rightXc, cy2, centerXZ.z), glm::vec3(0.0f, 0.0f, -pitchRad),
                glm::vec3(sideSpan, panelThk, outerD), roofCol, false);
            AddBox(glm::vec3(leftXc, cy2, centerXZ.z), glm::vec3(0.0f, 0.0f, +pitchRad),
                glm::vec3(sideSpan, panelThk, outerD), roofCol, false);

            return deckTopY;
        };

    auto AddGableRoof_EaveZ = [&](const glm::vec3& centerXZ, float footprintW, float footprintD, float eaveY,
        float pitchRad, float thk, float overhang, const glm::vec3& roofCol, const glm::vec3& ridgeCol) -> float
        {
            float halfSpan = footprintD * 0.5f + overhang;
            float ridgeOverlap = std::max(0.10f, thk * 1.35f);
            float slabLen = halfSpan + ridgeOverlap;
            float slabW = footprintW + 2.0f * overhang;

            float ridgeRise = halfSpan * std::sin(pitchRad);
            float ridgeY = eaveY + ridgeRise;

            float zFrontEave = centerXZ.z + halfSpan;
            float zBackEave = centerXZ.z - halfSpan;

            auto T = [](float x, float y, float z) { return glm::translate(glm::mat4(1.0f), glm::vec3(x, y, z)); };
            auto S = [](float x, float y, float z) { return glm::scale(glm::mat4(1.0f), glm::vec3(x, y, z)); };
            auto Rx = [](float a) { return glm::rotate(glm::mat4(1.0f), a, glm::vec3(1, 0, 0)); };

            glm::mat4 front =
                T(centerXZ.x, eaveY, zFrontEave) *
                Rx(+pitchRad) *
                T(0.0f, thk * 0.5f, -slabLen * 0.5f) *
                S(slabW, thk, slabLen);

            glm::mat4 back =
                T(centerXZ.x, eaveY, zBackEave) *
                Rx(-pitchRad) *
                T(0.0f, thk * 0.5f, +slabLen * 0.5f) *
                S(slabW, thk, slabLen);

            items.push_back({ front, roofCol });
            items.push_back({ back,  roofCol });

            float capW = slabW * 1.06f;
            float capH = thk * 1.05f;
            float capD = std::max(0.45f, ridgeOverlap * 5.0f);

            glm::mat4 cap =
                T(centerXZ.x, ridgeY + capH * 0.5f + thk * 0.02f, centerXZ.z) *
                S(capW, capH, capD);

            items.push_back({ cap, ridgeCol });

            return ridgeY + capH;
        };

    auto AddGableRoof_EaveX = [&](const glm::vec3& centerXZ, float footprintW, float footprintD, float eaveY,
        float pitchRad, float thk, float overhang, const glm::vec3& roofCol, const glm::vec3& ridgeCol) -> float
        {
            float halfSpan = footprintW * 0.5f + overhang;
            float ridgeOverlap = std::max(0.08f, thk * 1.15f);
            float slabLen = halfSpan + ridgeOverlap;
            float slabD = footprintD + 2.0f * overhang;

            float ridgeRise = halfSpan * std::sin(pitchRad);
            float ridgeY = eaveY + ridgeRise;

            float xRightEave = centerXZ.x + halfSpan;
            float xLeftEave = centerXZ.x - halfSpan;

            auto T = [](float x, float y, float z) { return glm::translate(glm::mat4(1.0f), glm::vec3(x, y, z)); };
            auto S = [](float x, float y, float z) { return glm::scale(glm::mat4(1.0f), glm::vec3(x, y, z)); };
            auto Rz = [](float a) { return glm::rotate(glm::mat4(1.0f), a, glm::vec3(0, 0, 1)); };

            glm::mat4 right =
                T(xRightEave, eaveY, centerXZ.z) *
                Rz(-pitchRad) *
                T(-slabLen * 0.5f, thk * 0.5f, 0.0f) *
                S(slabLen, thk, slabD);

            glm::mat4 left =
                T(xLeftEave, eaveY, centerXZ.z) *
                Rz(+pitchRad) *
                T(+slabLen * 0.5f, thk * 0.5f, 0.0f) *
                S(slabLen, thk, slabD);

            items.push_back({ left,  roofCol });
            items.push_back({ right, roofCol });

            float capW = std::max(0.16f, ridgeOverlap * 2.4f);
            float capH = thk * 0.90f;
            float capD = slabD * 1.02f;

            glm::mat4 cap =
                T(centerXZ.x, ridgeY + capH * 0.5f + thk * 0.02f, centerXZ.z) *
                S(capW, capH, capD);

            items.push_back({ cap, ridgeCol });

            return ridgeY + capH;
        };

    float midEaveY_forGarage = 0.0f;

    {
        glm::vec3 Hc = center + glm::vec3(-yardW * 0.02f, 0.0f, 0.0f);
        float slabY = overlayY + WC::YARD_THK;

        float HOUSE_SCALE = 1.25f;
        float slabH = 0.60f * HOUSE_SCALE;

        float W1 = 19.0f * HOUSE_SCALE;
        float D1 = 13.6f * HOUSE_SCALE;
        float H1 = 4.4f * HOUSE_SCALE * 1.22f;

        float W2 = 15.2f * HOUSE_SCALE;
        float D2 = 11.0f * HOUSE_SCALE;
        float H2 = 3.6f * HOUSE_SCALE * 1.18f;

        AddBox(glm::vec3(Hc.x, slabY, Hc.z), glm::vec3(0.0f),
            glm::vec3(W1 + 2.4f * HOUSE_SCALE, slabH, D1 + 2.2f * HOUSE_SCALE), colBase, true);

        float f1Y = slabY + slabH;
        AddBox(glm::vec3(Hc.x, f1Y, Hc.z), glm::vec3(0.0f),
            glm::vec3(W1, H1, D1), colWall, true);

        float frontZ1 = Hc.z + D1 * 0.5f;
        float backZ1 = Hc.z - D1 * 0.5f;

        glm::vec3 porch = Hc + glm::vec3(W1 * 0.22f, 0.0f, D1 * 0.5f - 2.6f * HOUSE_SCALE);
        AddBox(glm::vec3(porch.x, f1Y, porch.z), glm::vec3(0.0f),
            glm::vec3(7.6f * HOUSE_SCALE, 4.2f * HOUSE_SCALE, 4.2f * HOUSE_SCALE), colWood, true);

        float stepH = 0.28f * HOUSE_SCALE;
        float stepD = 0.85f * HOUSE_SCALE;
        float stepW = 3.6f * HOUSE_SCALE;
        float stepStartZ = frontZ1 + 0.40f * HOUSE_SCALE;

        AddBox(glm::vec3(porch.x, slabY, stepStartZ), glm::vec3(0.0f),
            glm::vec3(stepW, stepH, stepD), colBase, true);
        AddBox(glm::vec3(porch.x, slabY + stepH, stepStartZ + stepD), glm::vec3(0.0f),
            glm::vec3(stepW, stepH, stepD), colBase, true);
        AddBox(glm::vec3(porch.x, slabY + stepH * 2.0f, stepStartZ + stepD * 2.0f), glm::vec3(0.0f),
            glm::vec3(stepW, stepH, stepD), colBase, true);

        float doorW = 1.9f * HOUSE_SCALE;
        float doorH = 4.2f * HOUSE_SCALE;
        float doorT = 0.22f * HOUSE_SCALE;

        float doorZ = frontZ1 + 0.14f * HOUSE_SCALE;
        AddBox(glm::vec3(porch.x, f1Y, doorZ), glm::vec3(0.0f),
            glm::vec3(doorW, doorH, doorT), colDoor, true);

        glm::vec3 knobCol(0.88f, 0.78f, 0.25f);
        float knobS = 0.12f * HOUSE_SCALE;
        AddBox(glm::vec3(porch.x + doorW * 0.33f, f1Y + doorH * 0.54f, doorZ + doorT * 0.60f),
            glm::vec3(0.0f), glm::vec3(knobS, knobS, knobS), knobCol, false);

        float winFrontZ = frontZ1 + 0.04f * HOUSE_SCALE;
        float winBackZ = backZ1 - 0.04f * HOUSE_SCALE;

        float winBaseY1 = f1Y + H1 * 0.55f - 1.05f * HOUSE_SCALE;

        AddWideWindow3Z(Hc.x - W1 * 0.22f, winBaseY1 + 0.12f * HOUSE_SCALE, winFrontZ,
            7.2f * HOUSE_SCALE, 1.9f * HOUSE_SCALE, HOUSE_SCALE, +1.0f);

        AddRectWindowZ(Hc.x + W1 * 0.40f, winBaseY1 + 0.18f * HOUSE_SCALE, winFrontZ,
            2.5f * HOUSE_SCALE, 2.1f * HOUSE_SCALE, HOUSE_SCALE, +1.0f);

        AddRectWindowZ(Hc.x - W1 * 0.30f, winBaseY1 + 0.14f * HOUSE_SCALE, winBackZ,
            2.6f * HOUSE_SCALE, 2.0f * HOUSE_SCALE, HOUSE_SCALE, -1.0f);

        AddRectWindowZ(Hc.x + W1 * 0.32f, winBaseY1 + 0.12f * HOUSE_SCALE, winBackZ,
            2.3f * HOUSE_SCALE, 2.0f * HOUSE_SCALE, HOUSE_SCALE, -1.0f);

        float sideX_R = Hc.x + W1 * 0.5f;
        float sideX_L = Hc.x - W1 * 0.5f;

        AddRectWindowX(sideX_R, winBaseY1 + 0.12f * HOUSE_SCALE, Hc.z + D1 * 0.18f,
            2.6f * HOUSE_SCALE, 2.0f * HOUSE_SCALE, HOUSE_SCALE, +1.0f);
        AddRectWindowX(sideX_R, winBaseY1 + 0.12f * HOUSE_SCALE, Hc.z - D1 * 0.22f,
            2.2f * HOUSE_SCALE, 2.0f * HOUSE_SCALE, HOUSE_SCALE, +1.0f);

        AddRectWindowX(sideX_L, winBaseY1 + 0.10f * HOUSE_SCALE, Hc.z - D1 * 0.10f,
            2.4f * HOUSE_SCALE, 2.0f * HOUSE_SCALE, HOUSE_SCALE, -1.0f);
        AddRectWindowX(sideX_L, winBaseY1 + 0.10f * HOUSE_SCALE, Hc.z + D1 * 0.26f,
            2.2f * HOUSE_SCALE, 2.0f * HOUSE_SCALE, HOUSE_SCALE, -1.0f);

        glm::vec3 roof2Center = Hc + glm::vec3(-0.4f * HOUSE_SCALE, 0.0f, -0.9f * HOUSE_SCALE);

        float midEaveY = f1Y + H1 + 0.02f * HOUSE_SCALE;
        midEaveY_forGarage = midEaveY;

        float outerW = W1 + 4.8f * HOUSE_SCALE;
        float outerD = D1 + 4.6f * HOUSE_SCALE;

        float deckW = outerW - 1.1f * HOUSE_SCALE;
        float deckD = outerD - 1.1f * HOUSE_SCALE;

        float midPitch = glm::radians(10.0f);
        float midPanelThk = 0.22f * HOUSE_SCALE;
        float midDeckThk = 0.20f * HOUSE_SCALE;

        float midRoofTopY = AddDeckSkirtRoof(roof2Center, midEaveY,
            deckW, deckD,
            outerW, outerD,
            midPitch,
            midPanelThk, midDeckThk,
            colRoof, colRoof * 0.92f);

        {
            float capThk = 0.12f * HOUSE_SCALE;
            float capY = midRoofTopY - capThk - 0.006f * HOUSE_SCALE;
            AddBox(glm::vec3(roof2Center.x, capY, roof2Center.z), glm::vec3(0.0f),
                glm::vec3(outerW * 0.86f, capThk, outerD * 0.86f), colRoof * 0.98f, true);
        }

        float supportThk = 0.18f * HOUSE_SCALE;
        float supportW = W2 + 1.25f * HOUSE_SCALE;
        float supportD = D2 + 1.15f * HOUSE_SCALE;

        AddBox(glm::vec3(roof2Center.x, midRoofTopY, roof2Center.z), glm::vec3(0.0f),
            glm::vec3(supportW, supportThk, supportD), colRoof * 0.88f, true);

        float f2Y = midRoofTopY + supportThk;
        AddBox(glm::vec3(roof2Center.x, f2Y, roof2Center.z), glm::vec3(0.0f),
            glm::vec3(W2, H2, D2), colWall, true);

        float f2FrontZ = roof2Center.z + D2 * 0.5f;
        float f2BackZ = roof2Center.z - D2 * 0.5f;

        float balW = W2 * 0.82f;
        float balD = 2.35f * HOUSE_SCALE;
        float balT = 0.18f * HOUSE_SCALE;

        float balFloorY = f2Y + 0.18f * HOUSE_SCALE;
        float balCenterX = roof2Center.x - 0.40f * HOUSE_SCALE;
        float balCenterZ = f2FrontZ + balD * 0.5f - 0.10f * HOUSE_SCALE;

        AddBox(glm::vec3(balCenterX, balFloorY, balCenterZ), glm::vec3(0.0f),
            glm::vec3(balW, balT, balD), colBase, true);

        float railH = 1.08f * HOUSE_SCALE;
        float railPostW = 0.14f * HOUSE_SCALE;
        float railT = 0.12f * HOUSE_SCALE;

        float railTopY = balFloorY + railH;
        float railMidY = balFloorY + 0.22f * HOUSE_SCALE;
        float railFrontZ = balCenterZ + balD * 0.5f - railT * 0.5f;

        AddBox(glm::vec3(balCenterX, railTopY, railFrontZ), glm::vec3(0.0f),
            glm::vec3(balW, 0.10f * HOUSE_SCALE, railT), colRail, true);
        AddBox(glm::vec3(balCenterX, railMidY, railFrontZ), glm::vec3(0.0f),
            glm::vec3(balW, 0.08f * HOUSE_SCALE, railT), colRail, true);

        int bars = 12;
        float leftX = balCenterX - balW * 0.5f;
        float spacing = balW / (float)bars;

        for (int i = 0; i <= bars; ++i) {
            float xx = leftX + spacing * i;
            AddBox(glm::vec3(xx, balFloorY + 0.06f * HOUSE_SCALE, railFrontZ), glm::vec3(0.0f),
                glm::vec3(railPostW, railH - 0.08f * HOUSE_SCALE, railPostW), colRail, true);
        }

        float slideW = 6.4f * HOUSE_SCALE;
        float slideH = 2.9f * HOUSE_SCALE;
        float slideY = f2Y + 0.52f * HOUSE_SCALE;
        float slideZ = f2FrontZ + 0.04f * HOUSE_SCALE;

        AddWideWindow3Z(balCenterX, slideY, slideZ, slideW, slideH, HOUSE_SCALE, +1.0f);

        float shutterW = 0.75f * HOUSE_SCALE;
        float shutterH = 2.6f * HOUSE_SCALE;
        float shutterT = 0.10f * HOUSE_SCALE;

        AddBox(glm::vec3(balCenterX - slideW * 0.5f - shutterW * 0.6f, slideY + 0.10f * HOUSE_SCALE, slideZ + 0.12f * HOUSE_SCALE), glm::vec3(0.0f),
            glm::vec3(shutterW, shutterH, shutterT), colTrim, true);
        AddBox(glm::vec3(balCenterX + slideW * 0.5f + shutterW * 0.6f, slideY + 0.10f * HOUSE_SCALE, slideZ + 0.12f * HOUSE_SCALE), glm::vec3(0.0f),
            glm::vec3(shutterW, shutterH, shutterT), colTrim, true);

        float win2BaseY = f2Y + H2 * 0.55f - 0.95f * HOUSE_SCALE;

        AddRectWindowZ(roof2Center.x - W2 * 0.18f, win2BaseY, f2BackZ - 0.04f * HOUSE_SCALE,
            2.3f * HOUSE_SCALE, 2.0f * HOUSE_SCALE, HOUSE_SCALE, -1.0f);

        float side2X_R = roof2Center.x + W2 * 0.5f;
        float side2X_L = roof2Center.x - W2 * 0.5f;

        AddRectWindowX(side2X_R, win2BaseY, roof2Center.z + D2 * 0.10f,
            2.2f * HOUSE_SCALE, 2.0f * HOUSE_SCALE, HOUSE_SCALE, +1.0f);
        AddRectWindowX(side2X_L, win2BaseY, roof2Center.z - D2 * 0.18f,
            2.2f * HOUSE_SCALE, 2.0f * HOUSE_SCALE, HOUSE_SCALE, -1.0f);

        float topEaveY = f2Y + H2;

        float topFootW = W2 + 1.9f * HOUSE_SCALE;
        float topFootD = D2 + 1.6f * HOUSE_SCALE;
        float topPitch = glm::radians(28.0f);
        float topThk = 0.22f * HOUSE_SCALE;
        float topOver = 0.65f * HOUSE_SCALE;

        (void)AddGableRoof_EaveZ(glm::vec3(roof2Center.x, 0.0f, roof2Center.z),
            topFootW, topFootD, topEaveY, topPitch, topThk, topOver,
            colRoof, colRoof * 0.92f);

        float ridgeY = topEaveY + (topFootD * 0.5f + topOver) * std::sin(topPitch);

        float seamW = topFootW * 1.06f;
        float seamD = 2.40f * HOUSE_SCALE;
        float seamH = topThk * 1.10f;
        AddCenter(glm::vec3(roof2Center.x, ridgeY + seamH * 0.55f + 0.03f * HOUSE_SCALE, roof2Center.z),
            glm::vec3(0.0f), glm::vec3(seamW, seamH, seamD), colRoof * 0.96f);

        float ridgeCapThk2 = topThk * 0.75f;
        AddCenter(glm::vec3(roof2Center.x, ridgeY + ridgeCapThk2 * 0.45f, roof2Center.z),
            glm::vec3(0.0f),
            glm::vec3(seamW * 0.98f, ridgeCapThk2, seamD * 1.08f),
            colRoof * 0.98f);

        auto AddGableSideFill = [&](float endX, float eaveY2, float ridgeY2, float depthD, float thkX, glm::vec3 col) {
            int steps2 = 6;
            float totalH = std::max(0.01f, ridgeY2 - eaveY2);
            float stepH2 = totalH / (float)steps2;
            for (int i = 0; i < steps2; ++i) {
                float t = (float)i / (float)steps2;
                float d = std::max(0.2f, depthD * (1.0f - t));
                float yb = eaveY2 + stepH2 * (float)i;
                AddBox(glm::vec3(endX, yb, roof2Center.z), glm::vec3(0.0f),
                    glm::vec3(thkX, stepH2 + 0.002f, d), col, true);
            }
            };

        float gableThk = 0.30f * HOUSE_SCALE;
        float gableDepth = topFootD;
        float gableXL = roof2Center.x - (W2 * 0.5f) - gableThk * 0.5f;
        float gableXR = roof2Center.x + (W2 * 0.5f) + gableThk * 0.5f;
        AddGableSideFill(gableXL, topEaveY, ridgeY, gableDepth, gableThk, colWall * 0.98f);
        AddGableSideFill(gableXR, topEaveY, ridgeY, gableDepth, gableThk, colWall * 0.98f);

        AddBox(glm::vec3(roof2Center.x - 4.2f * HOUSE_SCALE, topEaveY + 0.10f * HOUSE_SCALE, roof2Center.z - 2.0f * HOUSE_SCALE), glm::vec3(0.0f),
            glm::vec3(1.30f * HOUSE_SCALE, std::max(2.2f * HOUSE_SCALE, (ridgeY - topEaveY) + 1.0f * HOUSE_SCALE), 1.30f * HOUSE_SCALE), colChim, true);

        float dogY = slabY;

        float dogW = 3.4f * HOUSE_SCALE * 0.55f * 0.78f;
        float dogD = 2.6f * HOUSE_SCALE * 0.55f;
        float dogH = 1.9f * HOUSE_SCALE * 0.55f * 1.25f;

        float leftInsideX = center.x - fenceHalfW + fenceThk * 1.25f;
        float houseLeftX = Hc.x - W1 * 0.5f;
        float marginX = dogW * 0.90f + 0.65f;

        float dogX = (leftInsideX + houseLeftX) * 0.5f;
        dogX = std::max(leftInsideX + marginX, std::min(houseLeftX - marginX, dogX));

        float frontInsideZ = center.z + fenceHalfL - fenceThk * 1.25f;
        float backInsideZ = center.z - fenceHalfL + fenceThk * 1.25f;

        float dogZ = Hc.z + D1 * 0.18f;
        float marginZ = dogD * 0.85f + 0.8f;
        dogZ = std::max(backInsideZ + marginZ, std::min(frontInsideZ - marginZ, dogZ));

        glm::vec3 dogC(dogX, 0.0f, dogZ);

        glm::vec3 dogWall(0.92f, 0.92f, 0.94f);
        glm::vec3 dogRoof(0.35f, 0.75f, 0.95f);

        AddBox(glm::vec3(dogC.x, dogY, dogC.z), glm::vec3(0.0f),
            glm::vec3(dogW, dogH, dogD), dogWall, true);

        float dogEaveY = dogY + dogH;
        float dogRoofW = dogW + 0.55f * HOUSE_SCALE * 0.55f;
        float dogRoofD = dogD + 0.55f * HOUSE_SCALE * 0.55f;
        float dogPitch = glm::radians(26.0f);
        float dogThk = 0.13f * HOUSE_SCALE * 0.55f;
        float dogOver = 0.18f * HOUSE_SCALE * 0.55f;

        AddGableRoof_EaveZ(glm::vec3(dogC.x, 0.0f, dogC.z), dogRoofW, dogRoofD, dogEaveY,
            dogPitch, dogThk, dogOver, dogRoof, dogRoof * 0.92f);

        glm::vec3 holeCol(0.08f, 0.08f, 0.08f);
        AddBox(glm::vec3(dogC.x, dogY + dogH * 0.05f, dogC.z + dogD * 0.5f + 0.03f),
            glm::vec3(0.0f), glm::vec3(dogW * 0.55f, dogH * 0.62f, 0.10f), holeCol, true);


     
        {
           
            float rackX = Hc.x - W1 * 0.22f;

          
          
            float rackFrontOffset = 3.2f * HOUSE_SCALE;  
            float rackZ = winFrontZ + rackFrontOffset;

            
            float rackY = overlayY + WC::YARD_THK + 0.003f;

            glm::vec3 rackPos(rackX, rackY, rackZ);

          
            glm::vec3 colFrame(0.45f, 0.45f, 0.50f);
            glm::vec3 colWire(0.30f, 0.30f, 0.34f);

            
            const float RACK_SCALE = 2.4f;   

            float rW = 1.4f * RACK_SCALE; 
            float rD = 0.8f * RACK_SCALE; 
            float rH = 1.0f * RACK_SCALE; 
            float pThk = 0.03f * RACK_SCALE; 

           
            float wireThk = 0.012f * RACK_SCALE;
            float towelThk = 0.03f * RACK_SCALE;

            float legX[2] = { -rW * 0.5f, +rW * 0.5f };
            float legZ[2] = { +rD * 0.5f, -rD * 0.5f };

          
            for (int ix = 0; ix < 2; ++ix) {
                for (int iz = 0; iz < 2; ++iz) {
                    AddBottom(rackPos + glm::vec3(legX[ix], 0.0f, legZ[iz]),
                        glm::vec3(0.0f),
                        glm::vec3(pThk, rH, pThk),
                        colFrame);
                }
            }

           
            float topY = rH - pThk * 0.5f;

          
            for (int ix = 0; ix < 2; ++ix) {
                AddCenter(rackPos + glm::vec3(legX[ix], topY, 0.0f),
                    glm::vec3(0.0f),
                    glm::vec3(pThk, pThk, rD),
                    colFrame);
            }

           
            for (int iz = 0; iz < 2; ++iz) {
                AddCenter(rackPos + glm::vec3(0.0f, topY, legZ[iz]),
                    glm::vec3(0.0f),
                    glm::vec3(rW, pThk, pThk),
                    colFrame);
            }

        
            int wireCount = 9;                
            float wireGap = rW / (wireCount + 1);

            for (int i = 1; i <= wireCount; ++i) {
                float xPos = (rW * 0.5f) - (wireGap * i);
                AddCenter(rackPos + glm::vec3(xPos, topY, 0.0f),
                    glm::vec3(0.0f),
                    glm::vec3(wireThk, wireThk, rD),
                    colWire);
            }

         
            float towelX = (rW * 0.5f) - (wireGap * 4);
            float towelDrop = 0.35f * RACK_SCALE;

            AddCenter(rackPos + glm::vec3(towelX, topY - towelDrop, 0.0f),
                glm::vec3(0.0f),
                glm::vec3(towelThk, 0.7f * RACK_SCALE, 0.4f * RACK_SCALE),
                glm::vec3(0.92f, 0.92f, 0.95f));
        }
       
        

        glm::vec3 carCenter = Hc + glm::vec3(W1 * 0.60f + 4.8f * HOUSE_SCALE, 0.0f, D1 * 0.18f);
        float carBaseY = slabY + slabH;

        float carPitch = glm::radians(12.0f);
        float carRoofThk = 0.24f * HOUSE_SCALE;

        float minCarH = 2.2f * HOUSE_SCALE;
        float carEaveY = std::max(midEaveY_forGarage, carBaseY + minCarH);
        float carH = carEaveY - carBaseY;

        float carFootW = 8.4f * HOUSE_SCALE;
        float carFootD = 10.2f * HOUSE_SCALE;
        float carOver = 0.75f * HOUSE_SCALE;

        AddBox(glm::vec3(carCenter.x, carBaseY, carCenter.z), glm::vec3(0.0f),
            glm::vec3(carFootW * 0.98f, 0.44f * HOUSE_SCALE, carFootD * 0.98f), colBase, true);

        float colW = 0.48f * HOUSE_SCALE;
        float colX = carFootW * 0.5f - 0.95f * HOUSE_SCALE;
        float colZ = carFootD * 0.5f - 1.05f * HOUSE_SCALE;

        AddBox(glm::vec3(carCenter.x - colX, carBaseY, carCenter.z + colZ), glm::vec3(0.0f),
            glm::vec3(colW, carH, colW), colWood * 0.90f, true);
        AddBox(glm::vec3(carCenter.x - colX, carBaseY, carCenter.z - colZ), glm::vec3(0.0f),
            glm::vec3(colW, carH, colW), colWood * 0.90f, true);
        AddBox(glm::vec3(carCenter.x + colX, carBaseY, carCenter.z + colZ), glm::vec3(0.0f),
            glm::vec3(colW, carH, colW), colWood * 0.90f, true);
        AddBox(glm::vec3(carCenter.x + colX, carBaseY, carCenter.z - colZ), glm::vec3(0.0f),
            glm::vec3(colW, carH, colW), colWood * 0.90f, true);

        float beamH = 0.20f * HOUSE_SCALE;
        AddBox(glm::vec3(carCenter.x, carEaveY - beamH, carCenter.z), glm::vec3(0.0f),
            glm::vec3(carFootW * 1.02f, beamH, carFootD * 1.02f), colWood * 0.92f, true);

        AddGableRoof_EaveX(glm::vec3(carCenter.x, 0.0f, carCenter.z),
            carFootW, carFootD, carEaveY,
            carPitch, carRoofThk, carOver,
            colRoof, colRoof * 0.92f);

        {
            float baseY = carBaseY + 0.01f;
            float CAR_SCALE = 1.55f;

            float carLift = 0.55f * CAR_SCALE;
            glm::vec3 carC(carCenter.x, baseY + carLift, carCenter.z);

            glm::vec3 carGreen(0.22f, 0.72f, 0.30f);
            glm::vec3 carGreenDark(0.14f, 0.52f, 0.22f);
            glm::vec3 tire(0.07f, 0.07f, 0.07f);
            glm::vec3 rim(0.75f, 0.75f, 0.75f);
            glm::vec3 head(1.00f, 0.95f, 0.75f);
            glm::vec3 tail(0.88f, 0.15f, 0.12f);

            float bodyW = 2.80f * CAR_SCALE;
            float bodyL = 5.30f * CAR_SCALE;
            float bodyH = 0.85f * CAR_SCALE;

            float cabinW = 2.35f * CAR_SCALE;
            float cabinL = 2.70f * CAR_SCALE;
            float cabinH = 0.95f * CAR_SCALE;

            float wheelR = 0.58f * CAR_SCALE;
            float wheelThk = 0.28f * CAR_SCALE;

            AddBottom(glm::vec3(carC.x, carC.y, carC.z), glm::vec3(0),
                glm::vec3(bodyW, bodyH, bodyL), carGreen);

            AddBottom(glm::vec3(carC.x, carC.y + bodyH * 0.62f, carC.z + 1.40f * CAR_SCALE), glm::vec3(0),
                glm::vec3(bodyW * 0.92f, 0.34f * CAR_SCALE, 1.60f * CAR_SCALE), carGreen);

            AddBottom(glm::vec3(carC.x, carC.y + 0.06f * CAR_SCALE, carC.z + bodyL * 0.5f - 0.12f * CAR_SCALE), glm::vec3(0),
                glm::vec3(bodyW * 0.94f, 0.33f * CAR_SCALE, 0.24f * CAR_SCALE), carGreenDark);
            AddBottom(glm::vec3(carC.x, carC.y + 0.06f * CAR_SCALE, carC.z - bodyL * 0.5f + 0.12f * CAR_SCALE), glm::vec3(0),
                glm::vec3(bodyW * 0.94f, 0.33f * CAR_SCALE, 0.24f * CAR_SCALE), carGreenDark);

            float cabinY = carC.y + bodyH;
            glm::vec3 cabinC(carC.x, cabinY, carC.z - 0.35f * CAR_SCALE);
            AddBottom(glm::vec3(cabinC.x, cabinC.y, cabinC.z), glm::vec3(0),
                glm::vec3(cabinW, cabinH, cabinL), carGreen);

            float sideGlassT = 0.06f * CAR_SCALE;
            float sideWinH = cabinH * 0.65f;
            float sideWinL = cabinL * 0.95f;

            float sideEps = 0.01f * CAR_SCALE;
            float xL_glass = cabinC.x - (cabinW * 0.5f + sideEps);
            float xR_glass = cabinC.x + (cabinW * 0.5f + sideEps);

            float sideY = cabinC.y + cabinH * 0.62f;
            float sideZ = cabinC.z;

            glm::vec3 sideGlassCol(0.55f, 0.78f, 0.98f);

            AddCenter(glm::vec3(xL_glass, sideY, sideZ),
                glm::vec3(0.0f),
                glm::vec3(sideGlassT, sideWinH, sideWinL),
                sideGlassCol);

            AddCenter(glm::vec3(xR_glass, sideY, sideZ),
                glm::vec3(0.0f),
                glm::vec3(sideGlassT, sideWinH, sideWinL),
                sideGlassCol);

            {
                glm::vec3 dividerCol = carGreenDark * 0.85f;

                float divT = sideGlassT + 0.02f * CAR_SCALE;
                float divW = 0.10f * CAR_SCALE;

                float splitZ = cabinC.z + cabinL * 0.10f;

                float pillarBaseY = baseY - 0.01f * CAR_SCALE;
                float pillarTopY = (cabinC.y + cabinH * 0.88f);

                float divH = pillarTopY - pillarBaseY;
                if (divH < 0.05f) divH = 0.05f;

                float divEps2 = 0.012f * CAR_SCALE;
                float xLL = cabinC.x - (cabinW * 0.5f + divEps2);
                float xRR = cabinC.x + (cabinW * 0.5f + divEps2);

                AddBottom(glm::vec3(xLL, pillarBaseY, splitZ),
                    glm::vec3(0.0f),
                    glm::vec3(divT, divH, divW),
                    dividerCol);

                AddBottom(glm::vec3(xRR, pillarBaseY, splitZ),
                    glm::vec3(0.0f),
                    glm::vec3(divT, divH, divW),
                    dividerCol);
            }

            {
                glm::vec3 glassCol(0.55f, 0.78f, 0.98f);

                float glassT = 0.10f * CAR_SCALE;
                float winW = cabinW * 0.78f;
                float winH = cabinH * 0.90f;

                float yv = cabinC.y + cabinH * 0.52f;
                float zv = cabinC.z + cabinL * 0.5f + 0.04f * CAR_SCALE;

                glm::vec3 rot(glm::radians(-12.0f), 0.0f, 0.0f);

                AddCenter(glm::vec3(cabinC.x, yv, zv),
                    rot,
                    glm::vec3(winW, winH, glassT),
                    glassCol);
            }

            {
                glm::vec3 wiperCol(0.06f, 0.06f, 0.06f);

                float winW = cabinW * 0.78f;
                float winH = cabinH * 0.90f;

                float yv = cabinC.y + cabinH * 0.52f;
                float zv = cabinC.z + cabinL * 0.5f + 0.04f * CAR_SCALE;

                glm::vec3 wsRot(glm::radians(-12.0f), 0.0f, 0.0f);

                glm::vec3 base = glm::vec3(cabinC.x, yv - winH * 0.42f, zv + 0.15f * CAR_SCALE);

                float bladeT = 0.05f * CAR_SCALE;
                float bladeH = 0.04f * CAR_SCALE;
                float bladeL = winW * 0.42f;

                float armT = 0.04f * CAR_SCALE;
                float armH = 0.04f * CAR_SCALE;
                float armL = winW * 0.18f;

                {
                    float xOff = -winW * 0.18f;
                    glm::vec3 p = base + glm::vec3(xOff, 0.00f, 0.00f);

                    AddCenter(p + glm::vec3(-armL * 0.20f, 0.01f * CAR_SCALE, 0.00f),
                        wsRot + glm::vec3(0.0f, 0.0f, glm::radians(18.0f)),
                        glm::vec3(armL, armH, armT),
                        wiperCol);

                    AddCenter(p,
                        wsRot + glm::vec3(0.0f, 0.0f, glm::radians(18.0f)),
                        glm::vec3(bladeL, bladeH, bladeT),
                        wiperCol);
                }

                {
                    float xOff = +winW * 0.18f;
                    glm::vec3 p = base + glm::vec3(xOff, 0.00f, 0.00f);

                    AddCenter(p + glm::vec3(+armL * 0.20f, 0.01f * CAR_SCALE, 0.00f),
                        wsRot + glm::vec3(0.0f, 0.0f, glm::radians(-18.0f)),
                        glm::vec3(armL, armH, armT),
                        wiperCol);

                    AddCenter(p,
                        wsRot + glm::vec3(0.0f, 0.0f, glm::radians(-18.0f)),
                        glm::vec3(bladeL, bladeH, bladeT),
                        wiperCol);
                }
            }

            {
                glm::vec3 mirrorBody(0.10f, 0.10f, 0.10f);
                glm::vec3 mirrorGlass(0.70f, 0.85f, 0.95f);

                float yv = cabinC.y + cabinH * 0.58f;
                float zv = carC.z + bodyL * 0.5f - 1.60f * CAR_SCALE;

                float out = 0.18f * CAR_SCALE;

                float armLen = 0.22f * CAR_SCALE;
                float armThk = 0.06f * CAR_SCALE;
                float armH = 0.08f * CAR_SCALE;

                float housW = 0.30f * CAR_SCALE;
                float housH = 0.20f * CAR_SCALE;
                float housD = 0.16f * CAR_SCALE;

                float glassInset = 0.01f * CAR_SCALE;

                {
                    float xSide = cabinC.x - (cabinW * 0.5f + out);

                    AddCenter(glm::vec3(xSide + armLen * 0.45f, yv, zv),
                        glm::vec3(0.0f),
                        glm::vec3(armLen, armH, armThk),
                        mirrorBody);

                    AddCenter(glm::vec3(xSide, yv, zv),
                        glm::vec3(0.0f),
                        glm::vec3(housW, housH, housD),
                        mirrorBody);

                    AddCenter(glm::vec3(xSide - housW * 0.10f, yv, zv - housD * 0.5f - glassInset),
                        glm::vec3(0.0f),
                        glm::vec3(housW * 0.78f, housH * 0.78f, 0.03f * CAR_SCALE),
                        mirrorGlass);
                }

                {
                    float xSide = cabinC.x + (cabinW * 0.5f + out);

                    AddCenter(glm::vec3(xSide - armLen * 0.45f, yv, zv),
                        glm::vec3(0.0f),
                        glm::vec3(armLen, armH, armThk),
                        mirrorBody);

                    AddCenter(glm::vec3(xSide, yv, zv),
                        glm::vec3(0.0f),
                        glm::vec3(housW, housH, housD),
                        mirrorBody);

                    AddCenter(glm::vec3(xSide + housW * 0.10f, yv, zv - housD * 0.5f - glassInset),
                        glm::vec3(0.0f),
                        glm::vec3(housW * 0.78f, housH * 0.78f, 0.03f * CAR_SCALE),
                        mirrorGlass);
                }
            }

            {
                glm::vec3 doorLineCol = carGreenDark * 0.95f;
                glm::vec3 handleCol = glm::vec3(0.10f, 0.10f, 0.10f);

                float doorZ = carC.z + 0.10f * CAR_SCALE;
                float doorY = carC.y + bodyH * 0.55f;

                float doorH = bodyH * 0.55f;
                float doorL = bodyL * 0.40f;
                float panelT = 0.03f * CAR_SCALE;

                float eps = 0.005f * CAR_SCALE;
                float xL = carC.x - (bodyW * 0.5f + panelT * 0.5f + eps);
                float xR = carC.x + (bodyW * 0.5f + panelT * 0.5f + eps);

                AddCenter(glm::vec3(xL, doorY, doorZ),
                    glm::vec3(0.0f),
                    glm::vec3(panelT, doorH, doorL),
                    doorLineCol);

                AddCenter(glm::vec3(xR, doorY, doorZ),
                    glm::vec3(0.0f),
                    glm::vec3(panelT, doorH, doorL),
                    doorLineCol);

                float handleW = 0.18f * CAR_SCALE;
                float handleH = 0.05f * CAR_SCALE;
                float handleT = 0.05f * CAR_SCALE;

                float handleY = doorY + doorH * 0.28f;

                float handleZ_F = doorZ + doorL * 0.18f;
                float handleZ_B = doorZ - doorL * 0.18f;

                AddCenter(glm::vec3(xL - handleT * 0.5f, handleY, handleZ_F),
                    glm::vec3(0.0f),
                    glm::vec3(handleT, handleH, handleW),
                    handleCol);
                AddCenter(glm::vec3(xL - handleT * 0.5f, handleY, handleZ_B),
                    glm::vec3(0.0f),
                    glm::vec3(handleT, handleH, handleW),
                    handleCol);

                AddCenter(glm::vec3(xR + handleT * 0.5f, handleY, handleZ_F),
                    glm::vec3(0.0f),
                    glm::vec3(handleT, handleH, handleW),
                    handleCol);
                AddCenter(glm::vec3(xR + handleT * 0.5f, handleY, handleZ_B),
                    glm::vec3(0.0f),
                    glm::vec3(handleT, handleH, handleW),
                    handleCol);
            }

            auto AddWheelRing = [&](glm::vec3 wheelC, float radius2, float thickness, glm::vec3 colTire, glm::vec3 colRim) {
                const int N = 24;
                for (int i = 0; i < N; ++i) {
                    float a = (float)i / (float)N * 6.2831853f;
                    float zc = std::cos(a) * radius2;
                    float yc = std::sin(a) * radius2;
                    glm::vec3 p = wheelC + glm::vec3(0.0f, yc, zc);

                    AddCenter(p, glm::vec3(0.0f),
                        glm::vec3(thickness, radius2 * 0.14f, radius2 * 0.14f),
                        colTire);
                }
                AddCenter(wheelC, glm::vec3(0.0f),
                    glm::vec3(thickness * 0.75f, radius2 * 0.62f, radius2 * 0.62f),
                    colRim);
                };

            float wheelY = baseY + wheelR;
            float frontZ = carC.z + bodyL * 0.5f - 1.10f * CAR_SCALE;
            float backZ = carC.z - bodyL * 0.5f + 1.10f * CAR_SCALE;

            float wheelX = bodyW * 0.5f - wheelThk * 0.35f;

            glm::vec3 wFL(carC.x - wheelX, wheelY, frontZ);
            glm::vec3 wFR(carC.x + wheelX, wheelY, frontZ);
            glm::vec3 wBL(carC.x - wheelX, wheelY, backZ);
            glm::vec3 wBR(carC.x + wheelX, wheelY, backZ);

            AddWheelRing(wFL, wheelR, wheelThk, tire, rim);
            AddWheelRing(wFR, wheelR, wheelThk, tire, rim);
            AddWheelRing(wBL, wheelR, wheelThk, tire, rim);
            AddWheelRing(wBR, wheelR, wheelThk, tire, rim);

            float lightZOut = 0.18f * CAR_SCALE;

            float headY = carC.y + 0.48f * CAR_SCALE;
            float headZ = carC.z + bodyL * 0.5f + lightZOut;
            AddBottom(glm::vec3(carC.x - 0.90f * CAR_SCALE, headY, headZ),
                glm::vec3(0), glm::vec3(0.55f * CAR_SCALE, 0.32f * CAR_SCALE, 0.18f * CAR_SCALE), head);
            AddBottom(glm::vec3(carC.x + 0.90f * CAR_SCALE, headY, headZ),
                glm::vec3(0), glm::vec3(0.55f * CAR_SCALE, 0.32f * CAR_SCALE, 0.18f * CAR_SCALE), head);

            float tailY = carC.y + 0.48f * CAR_SCALE;
            float tailZ = carC.z - bodyL * 0.5f - lightZOut;
            AddBottom(glm::vec3(carC.x - 0.90f * CAR_SCALE, tailY, tailZ),
                glm::vec3(0), glm::vec3(0.55f * CAR_SCALE, 0.32f * CAR_SCALE, 0.18f * CAR_SCALE), tail);
            AddBottom(glm::vec3(carC.x + 0.90f * CAR_SCALE, tailY, tailZ),
                glm::vec3(0), glm::vec3(0.55f * CAR_SCALE, 0.32f * CAR_SCALE, 0.18f * CAR_SCALE), tail);

            AddBottom(glm::vec3(carC.x, carC.y - 0.08f * CAR_SCALE, carC.z),
                glm::vec3(0),
                glm::vec3(bodyW * 0.95f, 0.12f * CAR_SCALE, bodyL * 0.95f),
                carGreenDark);

            {
                glm::vec3 plateCol(0.92f, 0.92f, 0.92f);
                glm::vec3 borderCol(0.12f, 0.12f, 0.12f);

                float plateW = 1.10f * CAR_SCALE;
                float plateH = 0.32f * CAR_SCALE;
                float plateT = 0.05f * CAR_SCALE;

                float plateY = carC.y + 0.30f * CAR_SCALE;
                float fz = carC.z + bodyL * 0.5f + 0.12f * CAR_SCALE;
                float bz = carC.z - bodyL * 0.5f - 0.12f * CAR_SCALE;

                AddCenter(glm::vec3(carC.x, plateY, fz), glm::vec3(0.0f),
                    glm::vec3(plateW, plateH, plateT), plateCol);

                AddCenter(glm::vec3(carC.x, plateY, bz), glm::vec3(0.0f),
                    glm::vec3(plateW, plateH, plateT), plateCol);

                float b = 0.03f * CAR_SCALE;
                AddCenter(glm::vec3(carC.x, plateY + plateH * 0.5f - b * 0.5f, fz + plateT * 0.6f), glm::vec3(0.0f),
                    glm::vec3(plateW, b, b), borderCol);
                AddCenter(glm::vec3(carC.x, plateY - plateH * 0.5f + b * 0.5f, fz + plateT * 0.6f), glm::vec3(0.0f),
                    glm::vec3(plateW, b, b), borderCol);

                AddCenter(glm::vec3(carC.x, plateY + plateH * 0.5f - b * 0.5f, bz - plateT * 0.6f), glm::vec3(0.0f),
                    glm::vec3(plateW, b, b), borderCol);
                AddCenter(glm::vec3(carC.x, plateY - plateH * 0.5f + b * 0.5f, bz - plateT * 0.6f), glm::vec3(0.0f),
                    glm::vec3(plateW, b, b), borderCol);

                float digitW = 0.06f * CAR_SCALE;
                float digitH = plateH * 0.70f;
                float digitT = 0.03f * CAR_SCALE;
                float spacing2 = 0.16f * CAR_SCALE;
                float startX2 = carC.x - spacing2 * 1.5f;

                for (int i = 0; i < 4; ++i) {
                    float xx = startX2 + spacing2 * i;

                    AddCenter(glm::vec3(xx, plateY, fz + plateT * 0.55f), glm::vec3(0.0f),
                        glm::vec3(digitW, digitH, digitT), borderCol);

                    AddCenter(glm::vec3(xx, plateY, bz - plateT * 0.55f), glm::vec3(0.0f),
                        glm::vec3(digitW, digitH, digitT), borderCol);
                }
            }
        }

        {
         
            float poleH = 9.6f;
            float poleW = 0.34f;

        
            float outPad = 6.0f;

          
            float outLeftX = center.x - fenceHalfW - outPad;
            float outRightX = center.x + fenceHalfW + outPad;

         
            float outBackZ = center.z - fenceHalfL - outPad;

        
            AddStreetLight(glm::vec3(outLeftX, overlayY, Hc.z + D1 * 0.10f), poleH, poleW);
            AddStreetLight(glm::vec3(outLeftX, overlayY, Hc.z - D1 * 0.18f), poleH, poleW);

       
            AddStreetLight(glm::vec3(outRightX, overlayY, Hc.z + D1 * 0.08f), poleH, poleW);
            AddStreetLight(glm::vec3(outRightX, overlayY, Hc.z - D1 * 0.20f), poleH, poleW);

            
            AddStreetLight(glm::vec3(Hc.x - W1 * 0.18f, overlayY, outBackZ), poleH, poleW);
            AddStreetLight(glm::vec3(Hc.x + W1 * 0.18f, overlayY, outBackZ), poleH, poleW);
}

    }

    {
        glm::vec3 cloudColor(0.95f, 0.95f, 0.97f);
        float cloudY = overlayY + 30.0f;

        auto AddCloud = [&](glm::vec3 c, float s) {
            AddCenter(c, glm::vec3(0.0f),
                glm::vec3(10.0f * s, 2.5f * s, 6.0f * s),
                cloudColor);

            AddCenter(c + glm::vec3(-4.0f * s, 0.8f * s, 0.0f), glm::vec3(0.0f),
                glm::vec3(7.0f * s, 2.0f * s, 5.0f * s),
                cloudColor);

            AddCenter(c + glm::vec3(4.5f * s, 0.4f * s, -1.0f * s), glm::vec3(0.0f),
                glm::vec3(6.5f * s, 1.8f * s, 4.8f * s),
                cloudColor);

            AddCenter(c + glm::vec3(0.0f, -0.4f * s, 2.0f * s), glm::vec3(0.0f),
                glm::vec3(8.0f * s, 1.6f * s, 5.8f * s),
                cloudColor);
            };

        int cloudCount = 15;
        float halfGroundCloud = WC::GROUND_SIZE * 0.5f - 10.0f;

        for (int i = 0; i < cloudCount; ++i) {
            float rx = (float)(rand() % (int)(halfGroundCloud * 2.0f) - (int)halfGroundCloud);
            float rz = (float)(rand() % (int)(halfGroundCloud * 2.0f) - (int)halfGroundCloud);

            float xx = center.x + rx;
            float zz = center.z + rz;

            float yy = cloudY + (float)(rand() % 7 - 3);
            float ss = 0.8f + (float)(rand() % 60) / 100.0f;

            AddCloud(glm::vec3(xx, yy, zz), ss);
        }

        int grassPatchCount = 70;
        float halfGround = WC::GROUND_SIZE * 0.5f;

        for (int i = 0; i < grassPatchCount; ++i) {
            float patchMargin = 2.0f;

            float rx = (float)(rand() % (int)((halfGround - patchMargin) * 2.0f) - (int)(halfGround - patchMargin));
            float rz = (float)(rand() % (int)((halfGround - patchMargin) * 2.0f) - (int)(halfGround - patchMargin));

            float gx = center.x + rx;
            float gz = center.z + rz;

            if (std::abs(gx - center.x) < yardW * 0.55f &&
                std::abs(gz - center.z) < yardL * 0.55f) {
                continue;
            }

            float ww = 1.8f + (float)(rand() % 20) / 10.0f;
            float ll = 1.8f + (float)(rand() % 20) / 10.0f;

            float gg = 0.40f + (float)(rand() % 20) / 100.0f;
            glm::vec3 grassColor(0.18f, gg, 0.18f);

            items.push_back({
                MakeModel_BottomPivot(glm::vec3(gx, overlayY + 0.001f, gz),
                                      glm::vec3(0.0f),
                                      glm::vec3(ww, 0.02f, ll)),
                grassColor
                });
        }

        int flowerCount = 260;
        float halfG = WC::GROUND_SIZE * 0.5f - 8.0f;

        glm::vec3 stemCol(0.10f, 0.55f, 0.12f);
        glm::vec3 flowerRed(0.95f, 0.20f, 0.18f);
        glm::vec3 flowerYellow(0.98f, 0.92f, 0.22f);

        for (int i = 0; i < flowerCount; ++i) {
            float rx = (float)(rand() % (int)(halfG * 2.0f) - (int)halfG);
            float rz = (float)(rand() % (int)(halfG * 2.0f) - (int)halfG);

            float fx = center.x + rx;
            float fz = center.z + rz;

            if (std::abs(fx - center.x) < fenceHalfW + 2.0f &&
                std::abs(fz - center.z) < fenceHalfL + 2.0f) {
                continue;
            }

            if (std::abs(fx - gateCenterX) < roadW * 0.70f &&
                fz > frontFenceOuterZ - 1.0f &&
                fz < frontFenceOuterZ + roadL + 1.0f) {
                continue;
            }

            float stemH = 0.52f + (float)(rand() % 22) / 100.0f;
            float stemW = 0.11f;

            glm::vec3 petalCol = (rand() % 2 == 0) ? flowerRed : flowerYellow;

            AddBottom(glm::vec3(fx, overlayY + 0.001f, fz), glm::vec3(0.0f),
                glm::vec3(stemW, stemH, stemW), stemCol);

            AddCenter(glm::vec3(fx, overlayY + stemH + 0.10f, fz), glm::vec3(0.0f),
                glm::vec3(0.46f, 0.20f, 0.46f), petalCol);
        }
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
        float cyv = (float)std::cos((double)yaw);
        float syv = (float)std::sin((double)yaw);

        glm::vec3 cameraPos;
        cameraPos.x = center.x + radius * cp * syv;
        cameraPos.y = center.y + radius * sp;
        cameraPos.z = center.z + radius * cp * cyv;

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
