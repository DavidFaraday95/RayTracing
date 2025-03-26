#include "C:/raylib-5.5/include/raylib.h"
#include <math.h>
#include <stddef.h>

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600

typedef struct {
    float x, y, z;
} Vec3;

typedef struct {
    Vec3 origin;
    Vec3 direction;
} RayStruct;

typedef struct {
    Vec3 center;
    float radius;
    Color color;
} Sphere;

typedef struct {
    Vec3 Point1;
    Vec3 Point2;
    Vec3 Point3;
    Color color;
} Triangle;

typedef struct {
    Vec3 corners[4]; // Four corners of the rectangle
    Color color;
} Rectangle3D;

Vec3 Vec3Add(Vec3 a, Vec3 b) { return (Vec3){a.x + b.x, a.y + b.y, a.z + b.z}; }
Vec3 Vec3Sub(Vec3 a, Vec3 b) { return (Vec3){a.x - b.x, a.y - b.y, a.z - b.z}; }
Vec3 Vec3Scale(Vec3 v, float s) { return (Vec3){v.x * s, v.y * s, v.z * s}; }
float Vec3Dot(Vec3 a, Vec3 b) { return a.x * b.x + a.y * b.y + a.z * b.z; }
Vec3 Vec3Cross(Vec3 a, Vec3 b) {
    return (Vec3){a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x};
}
Vec3 Vec3Normalize(Vec3 v) {
    float mag = sqrtf(Vec3Dot(v, v));
    return Vec3Scale(v, 1.0f / mag);
}

// Rotate a point around the y-axis
Vec3 RotateY(Vec3 point, float angle) {
    float cosA = cosf(angle);
    float sinA = sinf(angle);
    Vec3 rotated;
    rotated.x = point.x * cosA + point.z * sinA;
    rotated.y = point.y;
    rotated.z = -point.x * sinA + point.z * cosA;
    return rotated;
}

float HitSphere(Sphere sphere, RayStruct ray) {
    Vec3 oc = Vec3Sub(ray.origin, sphere.center);
    float a = Vec3Dot(ray.direction, ray.direction);
    float b = 2.0f * Vec3Dot(oc, ray.direction);
    float c = Vec3Dot(oc, oc) - sphere.radius * sphere.radius;
    float discriminant = b * b - 4 * a * c;
    if (discriminant < 0) return -1.0f;
    return (-b - sqrtf(discriminant)) / (2.0f * a);
}

float HitTriangle(Triangle triangle, RayStruct ray) {
    const float EPSILON = 0.000001f;
    Vec3 edge1 = Vec3Sub(triangle.Point2, triangle.Point1);
    Vec3 edge2 = Vec3Sub(triangle.Point3, triangle.Point1);
    Vec3 h = Vec3Cross(ray.direction, edge2);
    float a = Vec3Dot(edge1, h);
    if (a > -EPSILON && a < EPSILON) return -1.0f;
    float f = 1.0f / a;
    Vec3 s = Vec3Sub(ray.origin, triangle.Point1);
    float u = f * Vec3Dot(s, h);
    if (u < 0.0f || u > 1.0f) return -1.0f;
    Vec3 q = Vec3Cross(s, edge1);
    float v = f * Vec3Dot(ray.direction, q);
    if (v < 0.0f || u + v > 1.0f) return -1.0f;
    float t = f * Vec3Dot(edge2, q);
    if (t > EPSILON) return t;
    return -1.0f;
}

void RectangleToTriangles(Rectangle3D rect, Triangle* tri1, Triangle* tri2) {
    tri1->Point1 = rect.corners[0];
    tri1->Point2 = rect.corners[1];
    tri1->Point3 = rect.corners[2];
    tri1->color = rect.color;
    
    tri2->Point1 = rect.corners[0];
    tri2->Point2 = rect.corners[2];
    tri2->Point3 = rect.corners[3];
    tri2->color = rect.color;
}

Color TraceRay(RayStruct ray, Sphere* spheres, int sphereCount, Triangle* triangles, int triangleCount) {
    float tMin = INFINITY;
    Color hitColor = {0, 0, 0, 0};
    int hitType = 0;
    
    for (int i = 0; i < sphereCount; i++) {
        float t = HitSphere(spheres[i], ray);
        if (t > 0 && t < tMin) {
            tMin = t;
            hitColor = spheres[i].color;
            hitType = 1;
        }
    }
    
    for (int i = 0; i < triangleCount; i++) {
        float t = HitTriangle(triangles[i], ray);
        if (t > 0 && t < tMin) {
            tMin = t;
            hitColor = triangles[i].color;
            hitType = 2;
        }
    }
    
    if (hitType > 0) return hitColor;
    
    float t = 0.5f * (ray.direction.y + 1.0f);
    return (Color){
        (unsigned char)(255 * (1.0f - t)),
        (unsigned char)(255 * (1.0f - t)),
        (unsigned char)(255),
        255
    };
}

int main(void) {
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Raytracing with Raylib - Rotating Rectangle");
    SetTargetFPS(60);

    Vec3 cameraPos = {0.0f, 0.0f, -3.0f};
    
    Sphere spheres[] = {
        {{0.0f, 0.0f, 0.0f}, 0.5f, RED},
        {{1.0f, 0.0f, 1.0f}, 0.3f, GREEN},
        {{-1.0f, 0.0f, 1.0f}, 0.4f, BLUE}
    };
    int sphereCount = sizeof(spheres) / sizeof(spheres[0]);

    // Base rectangle (unrotated initial state)
    Rectangle3D baseRect = {
        .corners = {
            {-1.5f, 1.5f, 1.0f},
            {1.5f, 1.5f, 1.0f},
            {1.5f, -1.5f, 1.0f},
            {-1.5f, -1.5f, 1.0f}
        },
        .color = PURPLE
    };

    Triangle allTriangles[2];
    int totalTriangleCount = 2;

    RenderTexture2D target = LoadRenderTexture(SCREEN_WIDTH, SCREEN_HEIGHT);

    while (!WindowShouldClose()) {
        // Get elapsed time for rotation
        float time = (float)GetTime();
        float angle = time; // Rotate 1 radian per second (adjust speed as needed)

        // Rotate the rectangle's corners
        Rectangle3D rotatedRect = baseRect;
        for (int i = 0; i < 4; i++) {
            rotatedRect.corners[i] = RotateY(baseRect.corners[i], angle);
        }

        // Update triangles with rotated rectangle
        RectangleToTriangles(rotatedRect, &allTriangles[0], &allTriangles[1]);

        BeginTextureMode(target);
        ClearBackground(BLACK);
        
        for (int y = 0; y < SCREEN_HEIGHT; y++) {
            for (int x = 0; x < SCREEN_WIDTH; x++) {
                float u = (float)x / SCREEN_WIDTH * 2.0f - 1.0f;
                float v = (float)y / SCREEN_HEIGHT * 2.0f - 1.0f;
                
                RayStruct ray;
                ray.origin = cameraPos;
                ray.direction = Vec3Normalize((Vec3){u, -v, 1.0f});
                
                Color color = TraceRay(ray, spheres, sphereCount, allTriangles, totalTriangleCount);
                DrawPixel(x, y, color);
            }
        }
        EndTextureMode();

        BeginDrawing();
        DrawTextureRec(target.texture,  
                       (Rectangle){0, 0, SCREEN_WIDTH, -SCREEN_HEIGHT},
                       (Vector2){0, 0}, 
                       WHITE);
        DrawFPS(10, 10);
        EndDrawing();
    }

    UnloadRenderTexture(target);
    CloseWindow();
    
    return 0;
}