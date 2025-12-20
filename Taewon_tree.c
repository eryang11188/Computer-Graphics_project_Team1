// --- 소나무 (계단식 사각뿔 형태) ---
glm::vec3 pinePos = WC::SHIN_CENTER + glm::vec3(8.0f, overlayY, 6.0f);

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