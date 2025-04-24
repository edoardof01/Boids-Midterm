//BoidsUpdateSOA.hpp
#pragma once
#include "BoidsSOA.hpp"
#include <omp.h>

// Calcolo separazione con SIMD e riduzioni esplicite
inline Vector2 separation(const int i,
                          const BoidSoA& current,
                          const int numBoids)
{
    const float selfX = current.posX[i];
    const float selfY = current.posY[i];
    const float velX = current.velX[i];
    const float velY = current.velY[i];

    float steerX = 0.0f;
    float steerY = 0.0f;
    int count = 0;

    #pragma omp simd reduction(+:steerX, steerY, count)
    for (int j = 0; j < numBoids; ++j) {
        if (i == j) continue;

        const float dx = selfX - current.posX[j];
        const float dy = selfY - current.posY[j];

        if (const float dist = std::sqrt(dx * dx + dy * dy); dist < SEPARATION_RADIUS && dist > 0.0001f) {
            const float factor = 1.0f / (dist * dist);
            steerX += dx / dist * factor;
            steerY += (dy / dist) * factor;
            count++;
        }
    }

    if (count > 0) {
        steerX /= static_cast<float>(count);
        steerY /= static_cast<float>(count);

        if (const float mag = std::sqrt(steerX * steerX + steerY * steerY); mag > 0.0f) {
            steerX = steerX / mag * MAX_SPEED - velX;
            steerY = steerY / mag * MAX_SPEED - velY;

            if (const float finalMag = std::sqrt(steerX * steerX + steerY * steerY); finalMag > MAX_FORCE) {
                steerX = steerX / finalMag * MAX_FORCE;
                steerY = steerY / finalMag * MAX_FORCE;
            }
        }
    }

    return {steerX, steerY};
}

// Calcolo allineamento con SIMD e riduzioni
inline Vector2 alignment(const int i,
                         const BoidSoA& current,
                         const int numBoids)
{
    const float selfX = current.posX[i];
    const float selfY = current.posY[i];
    const float velX = current.velX[i];
    const float velY = current.velY[i];

    float sumX = 0.0f;
    float sumY = 0.0f;
    int count = 0;

    #pragma omp simd reduction(+:sumX, sumY, count)
    for (int j = 0; j < numBoids; ++j) {
        if (i == j) continue;

        const float dx = selfX - current.posX[j];
        const float dy = selfY - current.posY[j];

        if (const float dist = std::sqrt(dx * dx + dy * dy); dist < VIEW_RADIUS) {
            const float weight = (VIEW_RADIUS - dist) / VIEW_RADIUS;
            sumX += current.velX[j] * weight;
            sumY += current.velY[j] * weight;
            count++;
        }
    }

    if (count > 0) {
        sumX /= static_cast<float>(count);
        sumY /= static_cast<float>(count);

        if (const float mag = std::sqrt(sumX * sumX + sumY * sumY); mag > 0.0f) {
            sumX = sumX / mag * MAX_SPEED - velX;
            sumY = sumY / mag * MAX_SPEED - velY;

            if (const float finalMag = std::sqrt(sumX * sumX + sumY * sumY); finalMag > MAX_FORCE) {
                sumX = sumX / finalMag * MAX_FORCE;
                sumY = sumY / finalMag * MAX_FORCE;
            }
        }
        return {sumX, sumY};
    }

    return {0.0f, 0.0f};
}

// Calcolo coesione con SIMD e riduzioni
inline Vector2 cohesion(const int i,
                        const BoidSoA& current,
                        const int numBoids)
{
    const float selfX = current.posX[i];
    const float selfY = current.posY[i];
    const float velX = current.velX[i];
    const float velY = current.velY[i];

    float centerX = 0.0f;
    float centerY = 0.0f;
    int count = 0;

    #pragma omp simd reduction(+:centerX, centerY, count)
    for (int j = 0; j < numBoids; ++j) {
        if (i == j) continue;

        const float dx = selfX - current.posX[j];
        const float dy = selfY - current.posY[j];

        if (const float dist = std::sqrt(dx * dx + dy * dy); dist < VIEW_RADIUS) {
            centerX += current.posX[j];
            centerY += current.posY[j];
            count++;
        }
    }

    if (count > 0) {
        centerX /= static_cast<float>(count);
        centerY /= static_cast<float>(count);

        float desiredX = centerX - selfX;
        float desiredY = centerY - selfY;

        if (const float mag = std::sqrt(desiredX * desiredX + desiredY * desiredY); mag > 0.0f) {
            desiredX = desiredX / mag * MAX_SPEED - velX;
            desiredY = desiredY / mag * MAX_SPEED - velY;

            if (const float finalMag = std::sqrt(desiredX * desiredX + desiredY * desiredY); finalMag > MAX_FORCE) {
                desiredX = desiredX / finalMag * MAX_FORCE;
                desiredY = desiredY / finalMag * MAX_FORCE;
            }
        }
        return {desiredX, desiredY};
    }

    return {0.0f, 0.0f};
}

// Funzione principale per aggiornare un boid
inline void computeNextBoidSoA(const int i,
                            const BoidSoA& oldState,
                            BoidSoA& newState,
                            const int numBoids)
{
    const Vector2 sep = separation(i, oldState, numBoids) * SEPARATION_WEIGHT;
    const Vector2 ali = alignment(i, oldState, numBoids) * ALIGNMENT_WEIGHT;
    const Vector2 coh = cohesion(i, oldState, numBoids) * COHESION_WEIGHT;

    Vector2 acceleration = sep + ali + coh;
    acceleration.limit(MAX_FORCE);

    Vector2 velocity{oldState.velX[i], oldState.velY[i]};
    velocity = velocity + acceleration;
    velocity.limit(MAX_SPEED);

    Vector2 position{oldState.posX[i], oldState.posY[i]};
    position = position + velocity;

    constexpr float margin = 5.0f;
    if (position.x < -margin) position.x = WIDTH + margin;
    else if (position.x > WIDTH + margin) position.x = -margin;

    if (position.y < -margin) position.y = HEIGHT + margin;
    else if (position.y > HEIGHT + margin) position.y = -margin;

    // Scrive nello stato futuro
    newState.posX[i] = position.x;
    newState.posY[i] = position.y;
    newState.velX[i] = velocity.x;
    newState.velY[i] = velocity.y;
}
