#pragma once
#include <glm/glm.hpp>

namespace WC {

	inline constexpr int WIN_W = 1280;
	inline constexpr int WIN_H = 720;

	inline constexpr float CAM_PITCH_DEG = 19.0f;
	inline constexpr float CAM_RADIUS = 78.0f;
	inline constexpr float CAM_SPEED = 1.6f;

	inline constexpr float ZOOM_SPEED = 2.2f;
	inline constexpr float R_MIN = 25.0f;
	inline constexpr float R_MAX = 160.0f;

	inline constexpr float GROUND_Y = 0.0f;
	inline constexpr float GROUND_SIZE = 240.0f;
	inline constexpr float GROUND_THK = 0.12f;

	inline constexpr float OVERLAY_Y = GROUND_Y + 0.001f;

	inline const glm::vec3 SHIN_CENTER = glm::vec3(0.0f, 0.0f, 0.0f);

	inline constexpr float YARD_W = 42.0f;
	inline constexpr float YARD_L = 32.0f;
	inline constexpr float YARD_THK = 0.14f;

	inline constexpr float DRIVE_THK = 0.06f;

	inline constexpr float FENCE_THK = 0.62f;
	inline constexpr float FENCE_H = 2.15f;
	inline constexpr float FENCE_MARGIN = 1.10f;

	inline const glm::vec3 COL_GRASS = glm::vec3(0.18f, 0.50f, 0.20f);
	inline const glm::vec3 COL_YARD = glm::vec3(0.82f, 0.79f, 0.68f);

} // namespace WC
