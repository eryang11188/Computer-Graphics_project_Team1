// WorldConfig.h
#pragma once
#include <glm/glm.hpp>

namespace World
{
    // 1.0f가 의미하는 월드 단위(보통 1m로 간주)
    constexpr float UNIT = 1.0f;

    // 지면 높이(y=0을 지면으로 사용)
    constexpr float GROUND_Y = 0.0f;

    // 월드 원점(부지/짱구집 기준점으로 통일)
    constexpr glm::vec3 ORIGIN = glm::vec3(0.0f, 0.0f, 0.0f);

    // 부지(대지) 크기
    constexpr float LOT_SIZE = 20.0f;

    // 도로/인도/연석 규격
    constexpr float ROAD_WIDTH = 7.0f;
    constexpr float SIDEWALK_W = 1.5f;
    constexpr float CURB_HEIGHT = 0.15f;

    // 동네 전체 바닥 크기(배경 빈약함 방지용)
    constexpr float WORLD_GROUND_SIZE = 120.0f;

    // 카메라 클리핑(원근)
    constexpr float NEAR_PLANE = 0.1f;
    constexpr float FAR_PLANE = 250.0f;

    // 임시 하늘색(스카이박스/텍스처 전 단계)
    constexpr glm::vec3 SKY_COLOR = glm::vec3(0.55f, 0.75f, 0.95f);

}

