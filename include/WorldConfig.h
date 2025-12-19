// WorldConfig.h
#pragma once
#include <glm/glm.hpp>

namespace WC {

    // 창
    constexpr int WIN_W = 1600;
    constexpr int WIN_H = 900;

    // 카메라
    constexpr float CAM_PITCH_DEG = 20.0f;
    constexpr float CAM_RADIUS = 55.0f;
    constexpr float CAM_SPEED = 1.5f;

    constexpr float ZOOM_SPEED = 3.0f;
    constexpr float R_MIN = 10.0f;
    constexpr float R_MAX = 180.0f;

    // 월드 높이
    constexpr float GROUND_Y = 0.0f;
    constexpr float GROUND_THK = 0.08f;

    // 잔디 윗면보다 살짝 위에 도로/마당/진입로를 올린다
    constexpr float OVERLAY_Y = GROUND_Y + GROUND_THK + 0.002f;

    // 월드 크기
    constexpr float GROUND_SIZE = 140.0f;

    // 도로/인도
    constexpr float ROAD_W = 14.0f;
    constexpr float ROAD_L = 120.0f;
    constexpr float ROAD_Z = 10.0f;   // (기존 ROAD_Y 같은 역할: z 위치)
    constexpr float ROAD_THK = 0.04f;

    constexpr float SIDEWALK_W = 6.0f;
    constexpr float SIDEWALK_L = ROAD_L;
    constexpr float SIDEWALK_THK = 0.14f;

    // 짱구집 중심(카메라 피봇)
    inline const glm::vec3 SHIN_CENTER = glm::vec3(0.0f, GROUND_Y, -22.0f);

    // 마당/진입로
    constexpr float YARD_W = 26.0f;
    constexpr float YARD_L = 22.0f;
    constexpr float YARD_THK = 0.06f;

    constexpr float DRIVE_W = 6.0f;
    constexpr float DRIVE_L = 16.0f;
    constexpr float DRIVE_Z = -6.0f;
    constexpr float DRIVE_THK = 0.03f;

    // 기본색
    inline const glm::vec3 COL_GRASS = glm::vec3(0.20f, 0.48f, 0.20f);
    inline const glm::vec3 COL_ROAD = glm::vec3(0.13f, 0.13f, 0.13f);
    inline const glm::vec3 COL_LINE = glm::vec3(0.85f, 0.78f, 0.20f);
    inline const glm::vec3 COL_SIDEWALK = glm::vec3(0.62f, 0.62f, 0.62f);
    inline const glm::vec3 COL_YARD = glm::vec3(0.35f, 0.30f, 0.22f);
}
