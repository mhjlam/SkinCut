#pragma once

#include <cstdint>
#include <numbers>
#include <numeric>

namespace SkinCut {
    namespace Constants {
        // Mathematical constants
        constexpr double EPSILON = std::numeric_limits<double>::epsilon();
        constexpr double ONE_THIRD = 1.0/3.0;
        constexpr double PI = std::numbers::pi;
        constexpr double TWO_PI = std::numbers::pi * 2.0;
        constexpr double HALF_PI = std::numbers::pi / 2.0;

        constexpr float FLOAT_MIN = std::numeric_limits<float>::min();
        constexpr float FLOAT_MAX = std::numeric_limits<float>::max();

        // Window constants
        constexpr uint32_t WINDOW_WIDTH = 1280;
        constexpr uint32_t WINDOW_HEIGHT = 720;

        // Camera constants
        constexpr float FOV_Y = 20.f;
        constexpr float NEAR_PLANE = 0.1f;
        constexpr float FAR_PLANE = 20.f;
        constexpr float MIN_DISTANCE = NEAR_PLANE + 1.0f;
        constexpr float MAX_DISTANCE = FAR_PLANE - 1.0f;
        constexpr float MIN_PITCH = -DirectX::XM_PIDIV2 + 0.2f;
        constexpr float MAX_PITCH = DirectX::XM_PIDIV2 - 0.2f;

        // Shader constants
        constexpr auto KERNEL_SAMPLES = 9; // Must be equal to NUM_SAMPLES in Subsurface.ps.hlsl

        // Test constants
        constexpr auto NUM_TEST_RUNS = 100;
    }
}
