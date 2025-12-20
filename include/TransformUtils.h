#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

inline glm::mat4 MakeModel_CenterPivot(const glm::vec3& pos, const glm::vec3& eulerRad, const glm::vec3& scale) {
    glm::mat4 M(1.0f);
    M = glm::translate(M, pos);
    M = glm::rotate(M, eulerRad.z, glm::vec3(0.0f, 0.0f, 1.0f));
    M = glm::rotate(M, eulerRad.y, glm::vec3(0.0f, 1.0f, 0.0f));
    M = glm::rotate(M, eulerRad.x, glm::vec3(1.0f, 0.0f, 0.0f));
    M = glm::scale(M, scale);
    return M;
}

inline glm::mat4 MakeModel_BottomPivot(const glm::vec3& pos, const glm::vec3& eulerRad, const glm::vec3& scale) {
    glm::mat4 M(1.0f);
    M = glm::translate(M, pos);
    M = glm::translate(M, glm::vec3(0.0f, scale.y * 0.5f, 0.0f));
    M = glm::rotate(M, eulerRad.z, glm::vec3(0.0f, 0.0f, 1.0f));
    M = glm::rotate(M, eulerRad.y, glm::vec3(0.0f, 1.0f, 0.0f));
    M = glm::rotate(M, eulerRad.x, glm::vec3(1.0f, 0.0f, 0.0f));
    M = glm::scale(M, scale);
    return M;
}
