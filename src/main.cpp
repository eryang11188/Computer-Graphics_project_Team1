// main.cpp (B: Prism roof only)
// C++ + OpenGL(3.3) + GLFW + GLAD + GLM
// Ground/Road/Sidewalk + Orbit Camera + Wheel Zoom + Borderless Window
// House blocking (walls/porch) + Prism roof (single)
// main.cpp 


#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <vector>
#include <iostream>
#include <cmath>

#include "WorldConfig.h"     // same folder as main.cpp
#include "TransformUtils.h"  // same folder as main.cpp

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

void framebuffer_size_callback(GLFWwindow*, int width, int height) {
    glViewport(0, 0, width, height);
}

void scroll_callback(GLFWwindow*, double, double yoffset) {
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

enum class MeshId { Cube, Roof };

struct RenderItem {
    MeshId mesh;
    glm::mat4 model;
    glm::vec3 color;
};

static void AddCube(std::vector<RenderItem>& items,
    const glm::vec3& pos,
    const glm::vec3& eulerRad,
    const glm::vec3& scale,
    const glm::vec3& color,
    bool bottomPivot = true)
{
    glm::mat4 m = bottomPivot
        ? MakeModel_BottomPivot(pos, eulerRad, scale)
        : MakeModel_CenterPivot(pos, eulerRad, scale);

    items.push_back({ MeshId::Cube, m, color });
}

static void AddRoof(std::vector<RenderItem>& items,
    const glm::vec3& pos,
    const glm::vec3& eulerRad,
    const glm::vec3& scale,
    const glm::vec3& color,
    bool bottomPivot = true)
{
    glm::mat4 m = bottomPivot
        ? MakeModel_BottomPivot(pos, eulerRad, scale)
        : MakeModel_CenterPivot(pos, eulerRad, scale);

    items.push_back({ MeshId::Roof, m, color });
}

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

    GLFWwindow* window = glfwCreateWindow(WC::WIN_W, WC::WIN_H, "Shinchan World (Roof B Only)", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window\n";
        glfwTerminate();
        return -1;
    }

    int mx, my;
    glfwGetMonitorPos(monitor, &mx, &my);
    int x = mx + (mode->width - WC::WIN_W) / 2;
    int y = my + (mode->height - WC::WIN_H) / 2;
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

    // ===== Cube mesh (8 verts + EBO indices) =====
    float cubeVerts[] = {
        -0.5f, -0.5f, -0.5f,
         0.5f, -0.5f, -0.5f,
         0.5f,  0.5f, -0.5f,
        -0.5f,  0.5f, -0.5f,
        -0.5f, -0.5f,  0.5f,
         0.5f, -0.5f,  0.5f,
         0.5f,  0.5f,  0.5f,
        -0.5f,  0.5f,  0.5f
    };

    unsigned int cubeIdx[] = {
        0,1,2, 2,3,0,
        4,5,6, 6,7,4,
        0,4,7, 7,3,0,
        1,5,6, 6,2,1,
        0,1,5, 5,4,0,
        3,2,6, 6,7,3
    };
    constexpr int CUBE_INDEX_COUNT = 36;

    unsigned int cubeVAO, cubeVBO, cubeEBO;
    glGenVertexArrays(1, &cubeVAO);
    glGenBuffers(1, &cubeVBO);
    glGenBuffers(1, &cubeEBO);

    glBindVertexArray(cubeVAO);

    glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVerts), cubeVerts, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cubeEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cubeIdx), cubeIdx, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);

    // ===== Roof mesh (triangular prism) =====
    float roofVerts[] = {
        // left triangle (x = -0.5)
        -0.5f, -0.5f, -0.5f,   // 0
        -0.5f, -0.5f,  0.5f,   // 1
        -0.5f,  0.5f,  0.0f,   // 2

        // right triangle (x = +0.5)
         0.5f, -0.5f, -0.5f,   // 3
         0.5f, -0.5f,  0.5f,   // 4
         0.5f,  0.5f,  0.0f    // 5
    };

    unsigned int roofIdx[] = {
        // side triangles
        0,2,1,
        3,4,5,

        // bottom quad
        0,1,4,  4,3,0,

        // back quad
        0,3,5,  5,2,0,

        // front quad
        1,2,5,  5,4,1
    };
    const int ROOF_INDEX_COUNT = (int)(sizeof(roofIdx) / sizeof(unsigned int));

    unsigned int roofVAO, roofVBO, roofEBO;
    glGenVertexArrays(1, &roofVAO);
    glGenBuffers(1, &roofVBO);
    glGenBuffers(1, &roofEBO);

    glBindVertexArray(roofVAO);

    glBindBuffer(GL_ARRAY_BUFFER, roofVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(roofVerts), roofVerts, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, roofEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(roofIdx), roofIdx, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);

    // ===== Shader (simple color) =====
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
    GLint viewLoc  = glGetUniformLocation(shaderProgram, "view");
    GLint projLoc  = glGetUniformLocation(shaderProgram, "projection");
    GLint colorLoc = glGetUniformLocation(shaderProgram, "uColor");

    // ===== World items =====
    std::vector<RenderItem> items;

    const float groundY  = WC::GROUND_Y;
    const float overlayY = WC::OVERLAY_Y;

    // Pivot for camera and house placement
    glm::vec3 shinCenter = WC::SHIN_CENTER;

    // 1) Ground (grass)
    AddCube(items,
        glm::vec3(0.0f, groundY, 0.0f),
        glm::vec3(0.0f),
        glm::vec3(WC::GROUND_SIZE, WC::GROUND_THK, WC::GROUND_SIZE),
        WC::COL_GRASS,
        true);

    // 2) Road
    AddCube(items,
        glm::vec3(0.0f, overlayY, WC::ROAD_Z),
        glm::vec3(0.0f),
        glm::vec3(WC::ROAD_W, WC::ROAD_THK, WC::ROAD_L),
        WC::COL_ROAD,
        true);

    // 2-1) Road dashed center line
    for (int i = -5; i <= 5; ++i) {
        float z = WC::ROAD_Z + i * 10.0f;
        AddCube(items,
            glm::vec3(0.0f, overlayY + 0.001f, z),
            glm::vec3(0.0f),
            glm::vec3(0.5f, 0.01f, 4.0f),
            WC::COL_LINE,
            true);
    }

    // 3) Sidewalks
    float xLeft  = -(WC::ROAD_W * 0.5f + WC::SIDEWALK_W * 0.5f);
    float xRight = +(WC::ROAD_W * 0.5f + WC::SIDEWALK_W * 0.5f);

    AddCube(items,
        glm::vec3(xLeft, overlayY, WC::ROAD_Z),
        glm::vec3(0.0f),
        glm::vec3(WC::SIDEWALK_W, WC::SIDEWALK_THK, WC::SIDEWALK_L),
        WC::COL_SIDEWALK,
        true);

    AddCube(items,
        glm::vec3(xRight, overlayY, WC::ROAD_Z),
        glm::vec3(0.0f),
        glm::vec3(WC::SIDEWALK_W, WC::SIDEWALK_THK, WC::SIDEWALK_L),
        WC::COL_SIDEWALK,
        true);

    // 4) Yard
    AddCube(items,
        glm::vec3(shinCenter.x, overlayY, shinCenter.z),
        glm::vec3(0.0f),
        glm::vec3(WC::YARD_W, WC::YARD_THK, WC::YARD_L),
        WC::COL_YARD,
        true);

    // 4-1) Driveway
    AddCube(items,
        glm::vec3(0.0f, overlayY, WC::DRIVE_Z),
        glm::vec3(0.0f),
        glm::vec3(WC::DRIVE_W, WC::DRIVE_THK, WC::DRIVE_L),
        glm::vec3(0.30f, 0.30f, 0.30f),
        true);
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

    // ===== House blocking =====
    const float houseBaseY = overlayY + WC::YARD_THK;

    const glm::vec3 COL_WALL   = glm::vec3(0.82f, 0.76f, 0.63f);
    const glm::vec3 COL_PORCH  = glm::vec3(0.78f, 0.72f, 0.60f);
    const glm::vec3 COL_ROOF   = glm::vec3(0.65f, 0.18f, 0.18f);
    const glm::vec3 COL_CHIMNY = glm::vec3(0.30f, 0.18f, 0.12f);

    const float wallW = 14.0f;
    const float wallH = 7.0f;
    const float wallD = 10.0f;

    // Main walls
    AddCube(items,
        glm::vec3(shinCenter.x, houseBaseY, shinCenter.z),
        glm::vec3(0.0f),
        glm::vec3(wallW, wallH, wallD),
        COL_WALL,
        true);

    // Porch block (assume front is +Z)
    AddCube(items,
        glm::vec3(shinCenter.x + 4.5f, houseBaseY, shinCenter.z + 4.2f),
        glm::vec3(0.0f),
        glm::vec3(5.0f, 4.0f, 3.5f),
        COL_PORCH,
        true);

    // Chimney
    AddCube(items,
        glm::vec3(shinCenter.x - 4.0f, houseBaseY + wallH + 1.2f, shinCenter.z - 2.5f),
        glm::vec3(0.0f),
        glm::vec3(1.2f, 3.5f, 1.2f),
        COL_CHIMNY,
        true);

    // Roof (prism only) - placed on top of wall
    // Scale meaning: width(x) / height(y) / depth(z)
    AddRoof(items,
        glm::vec3(shinCenter.x, houseBaseY + wallH, shinCenter.z),
        glm::vec3(0.0f),
        glm::vec3(wallW + 1.0f, 4.0f, wallD + 1.0f),
        COL_ROOF,
        true);

    // ===== Render loop =====
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

        glm::mat4 projection = glm::perspective(glm::radians(45.0f), aspect, 0.1f, 300.0f);

        float cp = (float)std::cos((double)pitch);
        float sp = (float)std::sin((double)pitch);
        float cy = (float)std::cos((double)yaw);
        float sy = (float)std::sin((double)yaw);

        glm::vec3 cameraPos;
        cameraPos.x = shinCenter.x + radius * cp * sy;
        cameraPos.y = shinCenter.y + radius * sp;
        cameraPos.z = shinCenter.z + radius * cp * cy;

        glm::mat4 view = glm::lookAt(cameraPos, shinCenter, glm::vec3(0, 1, 0));

        glUseProgram(shaderProgram);
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

        for (const auto& it : items) {
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(it.model));
            glUniform3fv(colorLoc, 1, glm::value_ptr(it.color));

            if (it.mesh == MeshId::Cube) {
                glBindVertexArray(cubeVAO);
                glDrawElements(GL_TRIANGLES, CUBE_INDEX_COUNT, GL_UNSIGNED_INT, 0);
            }
            else { // MeshId::Roof
                glBindVertexArray(roofVAO);
                glDrawElements(GL_TRIANGLES, ROOF_INDEX_COUNT, GL_UNSIGNED_INT, 0);
            }
        }
        glBindVertexArray(0);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Cleanup
    glDeleteVertexArrays(1, &cubeVAO);
    glDeleteBuffers(1, &cubeVBO);
    glDeleteBuffers(1, &cubeEBO);

    glDeleteVertexArrays(1, &roofVAO);
    glDeleteBuffers(1, &roofVBO);
    glDeleteBuffers(1, &roofEBO);

    glDeleteProgram(shaderProgram);

    glfwTerminate();
    return 0;
}