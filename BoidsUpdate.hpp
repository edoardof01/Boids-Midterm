//BoidsUpdate.hpp
#pragma once
#include "BoidsCommon.hpp"


inline Vector2 separation(const Boid& b, const std::vector<Boid>& current) {
    Vector2 steer{0, 0};
    int count = 0;

    for (const auto& other : current) {
        if (const float dist = (b.position - other.position).magnitude();
            &b != &other && dist < SEPARATION_RADIUS) {
            Vector2 diff = (b.position - other.position).normalized();
            diff = diff / (dist * dist);
            steer = steer + diff;
            count++;
        }
    }
    if (count > 0) {
        steer = steer / static_cast<float>(count);
        steer = steer.normalized() * MAX_SPEED;
        steer = steer - b.velocity;
        steer.limit(MAX_FORCE);
    }
    return steer;
}


inline Vector2 alignment(const Boid& b, const std::vector<Boid>& current) {
    Vector2 sum{0, 0};
    int count = 0;

    for (const auto& other : current) {
        if (const float dist = (b.position - other.position).magnitude(); &b != &other && dist < VIEW_RADIUS) {
            const float weight = (VIEW_RADIUS - dist) / VIEW_RADIUS;
            sum = sum + (other.velocity * weight);
            count++;
        }
    }
    if (count > 0) {
        sum = sum / static_cast<float>(count);
        sum = sum.normalized() * MAX_SPEED;
        Vector2 steer = sum - b.velocity;
        steer.limit(MAX_FORCE);
        return steer;
    }
    return Vector2{0, 0};
}

inline Vector2 cohesion(const Boid& b, const std::vector<Boid>& current) {
    Vector2 center{0, 0};
    int count = 0;
    for (const auto& other : current) {
        if (const float dist = (b.position - other.position).magnitude(); &b != &other && dist < VIEW_RADIUS) {
            center = center + other.position;
            count++;
        }
    }
    if (count > 0) {
        center = center / static_cast<float>(count);
        const Vector2 desired = (center - b.position).normalized() * MAX_SPEED;
        Vector2 steer = desired - b.velocity;
        steer.limit(MAX_FORCE);
        return steer;
    }
    return Vector2{0, 0};
}


inline void computeNextBoid(const int i,
                            const std::vector<Boid>& oldState,
                            std::vector<Boid>& newState)
{
    const Boid& b = oldState[i];
    auto&[position, velocity] = newState[i];

    const Vector2 sep = separation(b, oldState) * SEPARATION_WEIGHT;
    const Vector2 ali = alignment(b, oldState)   * ALIGNMENT_WEIGHT;
    const Vector2 coh = cohesion(b, oldState)    * COHESION_WEIGHT;

    Vector2 acceleration = sep + ali + coh;
    acceleration.limit(MAX_FORCE);
    velocity = b.velocity + acceleration;
    velocity.limit(MAX_SPEED);
    position = b.position + velocity;

    constexpr float margin = 5.0f;
    if (position.x < -margin) position.x = WIDTH + margin;
    else if (position.x > WIDTH + margin) position.x = -margin;
    if (position.y < -margin) position.y = HEIGHT + margin;
    else if (position.y > HEIGHT + margin) position.y = -margin;
}


