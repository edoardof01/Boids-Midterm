#pragma once
#include "BoidsSOA.hpp"

// Calcolo separazione
inline Vector2 separation(const int i,
                          const BoidSoA& current,
                          const int numBoids)
{
    Vector2 steer{0, 0};
    int count = 0;

    const Vector2 selfPos{current.posX[i], current.posY[i]};

    for (int j = 0; j < numBoids; ++j) {
        if (i == j) continue;

        const Vector2 otherPos{current.posX[j], current.posY[j]};
        if (const float dist = (selfPos - otherPos).magnitude(); dist < SEPARATION_RADIUS) {
            Vector2 diff = (selfPos - otherPos).normalized();
            diff = diff / (dist * dist);
            steer = steer + diff;
            count++;
        }
    }

    if (count > 0) {
        steer = steer / static_cast<float>(count);
        steer = steer.normalized() * MAX_SPEED;

        const Vector2 selfVel{current.velX[i], current.velY[i]};
        steer = steer - selfVel;
        steer.limit(MAX_FORCE);
    }

    return steer;
}

// Calcolo allineamento
inline Vector2 alignment(const int i,
                         const BoidSoA& current,
                         const int numBoids)
{
    Vector2 sum{0, 0};
    int count = 0;

    const Vector2 selfPos{current.posX[i], current.posY[i]};

    for (int j = 0; j < numBoids; ++j) {
        if (i == j) continue;

        const Vector2 otherPos{current.posX[j], current.posY[j]};

        if (const float dist = (selfPos - otherPos).magnitude(); dist < VIEW_RADIUS) {
            const float weight = (VIEW_RADIUS - dist) / VIEW_RADIUS;
            Vector2 otherVel{current.velX[j], current.velY[j]};
            sum = sum + (otherVel * weight);
            count++;
        }
    }

    if (count > 0) {
        sum = sum / static_cast<float>(count);
        sum = sum.normalized() * MAX_SPEED;

        const Vector2 selfVel{current.velX[i], current.velY[i]};
        Vector2 steer = sum - selfVel;
        steer.limit(MAX_FORCE);
        return steer;
    }

    return Vector2{0, 0};
}

// Calcolo coesione
inline Vector2 cohesion(const int i,
                        const BoidSoA& current,
                        const int numBoids)
{
    Vector2 center{0, 0};
    int count = 0;

    const Vector2 selfPos{current.posX[i], current.posY[i]};

    for (int j = 0; j < numBoids; ++j) {
        if (i == j) continue;

        const Vector2 otherPos{current.posX[j], current.posY[j]};
        if (const float dist = (selfPos - otherPos).magnitude(); dist < VIEW_RADIUS) {
            center = center + otherPos;
            count++;
        }
    }

    if (count > 0) {
        center = center / static_cast<float>(count);
        const Vector2 desired = (center - selfPos).normalized() * MAX_SPEED;
        const Vector2 selfVel{current.velX[i], current.velY[i]};
        Vector2 steer = desired - selfVel;
        steer.limit(MAX_FORCE);
        return steer;
    }

    return Vector2{0, 0};
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
