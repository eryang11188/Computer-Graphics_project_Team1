
// TransformUtils.h
#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// pos: 월드 위치
// eulerRad: (pitch, yaw, roll) 라디안
// scale: 크기(큐브 -0.5~0.5 기준 스케일)

// 중심(센터) 피봇 기준 모델행렬
inline glm::mat4 MakeModel_CenterPivot(
    const glm::vec3& pos,
    const glm::vec3& eulerRad,
    const glm::vec3& scale)
{
    glm::mat4 m(1.0f);
    m = glm::translate(m, pos);
    m = glm::rotate(m, eulerRad.y, glm::vec3(0, 1, 0)); // yaw
    m = glm::rotate(m, eulerRad.x, glm::vec3(1, 0, 0)); // pitch
    m = glm::rotate(m, eulerRad.z, glm::vec3(0, 0, 1)); // roll
    m = glm::scale(m, scale);
    return m;
}

// 바닥(보텀) 피봇처럼 배치: pos.y에 바닥이 닿도록 y를 (scale.y*0.5)만큼 자동 보정
inline glm::mat4 MakeModel_BottomPivot(
    const glm::vec3& pos,
    const glm::vec3& eulerRad,
    const glm::vec3& scale)
{
    glm::vec3 p = pos;
    p.y += (scale.y * 0.5f);

    glm::mat4 m(1.0f);
    m = glm::translate(m, p);
    m = glm::rotate(m, eulerRad.y, glm::vec3(0, 1, 0)); // yaw
    m = glm::rotate(m, eulerRad.x, glm::vec3(1, 0, 0)); // pitch
    m = glm::rotate(m, eulerRad.z, glm::vec3(0, 0, 1)); // roll
    m = glm::scale(m, scale);
    return m;
}
