// BoidsCommon.hpp
#pragma once
#include <vector>
#include <cmath>
#include <random>

constexpr float WIDTH = 1000.0f;
constexpr float HEIGHT = 800.0f;
constexpr float MAX_SPEED = 3.0f;
constexpr float MAX_FORCE = 0.1f;
constexpr float VIEW_RADIUS = 100.0f;
constexpr float SEPARATION_RADIUS = 20.0f;
constexpr float SEPARATION_WEIGHT = 2.0f;
constexpr float ALIGNMENT_WEIGHT  = 1.0f;
constexpr float COHESION_WEIGHT   = 1.5f;

struct Vector2 {
    float x, y;
    Vector2() : x{0}, y{0} {}
    Vector2(const float x_, const float y_) : x{x_}, y{y_} {}

    Vector2 operator+(const Vector2& other) const { return {x + other.x, y + other.y}; }
    Vector2 operator-(const Vector2& other) const { return {x - other.x, y - other.y}; }
    Vector2 operator*(const float scalar)         const { return {x * scalar,   y * scalar}; }
    Vector2 operator/(const float scalar)         const { return {x / scalar,   y / scalar}; }

    [[nodiscard]] float magnitude() const {
        return std::sqrt(x * x + y * y);
    }

    [[nodiscard]] Vector2 normalized() const {
        const float mag = magnitude();
        return (mag > 0.0f) ? (*this / mag) : *this;
    }

    void limit(const float max) {
        if (const float mag = magnitude(); mag > max) {
            *this = (*this / mag) * max;
        }
    }
};
struct Boid {
    Vector2 position;
    Vector2 velocity;
};


