#include "raylib.h"
#include "raymath.h"
#include <vector>
#include <cmath>
#include <omp.h>

// update list provided, check that for changes.

struct Particle {
    Vector2 position;
    Vector2 velocity;
    float radius;
    bool selected;
};

struct Quadtree {
    Rectangle bounds;
    std::vector<Particle*> particles;
    Quadtree* NW;
    Quadtree* NE;
    Quadtree* SW;
    Quadtree* SE;
    int capacity;
    bool divided;

    Quadtree(Rectangle bounds, int capacity) : bounds(bounds), capacity(capacity), divided(false), NW(nullptr), NE(nullptr), SW(nullptr), SE(nullptr) {}

    ~Quadtree() {
        delete NW;
        delete NE;
        delete SW;
        delete SE;
    }

    void subdivide() {
        float x = bounds.x;
        float y = bounds.y;
        float w = bounds.width / 2;
        float h = bounds.height / 2;
        NW = new Quadtree({ x, y, w, h }, capacity);
        NE = new Quadtree({ x + w, y, w, h }, capacity);
        SW = new Quadtree({ x, y + h, w, h }, capacity);
        SE = new Quadtree({ x + w, y + h, w, h }, capacity);
        divided = true;
    }

    void insert(Particle* p) {
        if (!CheckCollisionPointRec(p->position, bounds)) return;

        if (particles.size() < capacity) {
            particles.push_back(p);
        }
        else {
            if (!divided) subdivide();
            NW->insert(p);
            NE->insert(p);
            SW->insert(p);
            SE->insert(p);
        }
    }

    void query(Rectangle range, std::vector<Particle*>& found) {
        if (!CheckCollisionRecs(bounds, range)) return;

        for (auto* p : particles) {
            if (CheckCollisionPointRec(p->position, range)) {
                found.push_back(p);
            }
        }

        if (divided) {
            NW->query(range, found);
            NE->query(range, found);
            SW->query(range, found);
            SE->query(range, found);
        }
    }
};

std::vector<Particle> particles;
Quadtree* quadtree = nullptr;

void buildQuadtree() {
    if (quadtree) {
        delete quadtree;
    }
    quadtree = new Quadtree({ 0, 0, static_cast<float>(GetScreenWidth()), static_cast<float>(GetScreenHeight()) }, 4);

#pragma omp parallel
    {
#pragma omp for schedule(dynamic)
        for (int i = 0; i < particles.size(); i++) {
#pragma omp critical
            quadtree->insert(&particles[i]);
        }
    }
}

void applyForces() {
    const float G = 500.0f;

#pragma omp parallel for schedule(dynamic)
    for (int i = 0; i < particles.size(); i++) {
        Vector2 force = { 0, 0 };

        Rectangle range = {
            static_cast<float>(particles[i].position.x - 50),
            static_cast<float>(particles[i].position.y - 50),
            static_cast<float>(100), static_cast<float>(100)
        };
        std::vector<Particle*> neighbors;
        quadtree->query(range, neighbors);

        for (auto* neighbor : neighbors) {
            if (neighbor == &particles[i]) continue;
            Vector2 dir = Vector2Subtract(neighbor->position, particles[i].position);
            float distSq = dir.x * dir.x + dir.y * dir.y + 1.0f;
            float strength = G / distSq;
            dir = Vector2Normalize(dir);
            force = Vector2Add(force, Vector2Scale(dir, strength));
        }

        particles[i].velocity = Vector2Add(particles[i].velocity, Vector2Scale(force, GetFrameTime()));
    }
}

void updateParticles() {
#pragma omp parallel for schedule(dynamic)
    for (int i = 0; i < particles.size(); i++) {
        particles[i].position = Vector2Add(particles[i].position, Vector2Scale(particles[i].velocity, GetFrameTime()));

        if (particles[i].position.x < 0 || particles[i].position.x > GetScreenWidth()) {
            particles[i].velocity.x *= -1;
        }
        if (particles[i].position.y < 0 || particles[i].position.y > GetScreenHeight()) {
            particles[i].velocity.y *= -1;
        }
    }
}

void drawParticles() {
    BeginDrawing();
    ClearBackground(BLACK);

#pragma omp parallel for
    for (int i = 0; i < particles.size(); i++) {
        Color color = particles[i].selected ? RED : RAYWHITE;
        DrawCircleV(particles[i].position, particles[i].radius, color);
    }

    EndDrawing();
}

void handleInput() {
    if (IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
        Vector2 mousePos = GetMousePosition();

        Rectangle range = { mousePos.x - 5, mousePos.y - 5, 10, 10 };
        std::vector<Particle*> found;
        quadtree->query(range, found);

        for (auto* p : found) {
            p->selected = true;
        }
    }
    else {
#pragma omp parallel for
        for (int i = 0; i < particles.size(); i++) {
            particles[i].selected = false;
        }
    }
}

void InitParticles(int count) {
    particles.resize(count);

#pragma omp parallel for
    for (int i = 0; i < count; i++) {
        // Get random values for x and y and cast them to float
        float x = static_cast<float>(GetRandomValue(0, GetScreenWidth()));
        float y = static_cast<float>(GetRandomValue(0, GetScreenHeight()));

        // Initialize particle properties
        particles[i].position = { x, y };
        particles[i].velocity = { 0, 0 };
        particles[i].radius = 2.0f;
        particles[i].selected = false;
    }
}


int main() {
    InitWindow(1280, 720, "Galaxy Engine");
    SetTargetFPS(60);

    InitParticles(10000);

    while (!WindowShouldClose()) {
        handleInput();
        buildQuadtree();
        applyForces();
        updateParticles();
        drawParticles();
    }

    delete quadtree;
    CloseWindow();
    return 0;
}
