#include "../Engine/collisions.cpp"
#include <stdio.h>
#include <cmath>

Transform TInit(Vector3 position, float z_rotation) {
    Vector3 rotation = Vector3{0.0f, 0.0f, z_rotation};
    Vector3 scale = Vector3{1.0f, 1.0f, 1.0f};

    Transform t = {0.0f};
    t.position = position;
    t.rotation = rotation;
    t.scale = scale;

    return t;
}



int main() {

    Transform transform_inputs[12] = {
        TInit({0.0f, 0.0f, 0.0f}, 0.0f), TInit({0.5f, 0.5f, 0.0f}, 0.0f), // Unrotated colliding
        TInit({0.0f, 0.0f, 0.0f}, 0.0f), TInit({1.5f, 1.5f, 0.0f}, 0.0f), // Unrotated non-colliding
        TInit({0.0f, 0.0f, 0.0f}, 0.0f), TInit({0.0f, 0.6f, 0.0f}, 45.0f), // One rotated colliding
        TInit({0.0f, 0.0f, 0.0f}, 0.0f), TInit({0.0f, 2.0f, 0.0f}, 45.0f), // One rotated non-colliding
        TInit({0.0f, 0.0f, 0.0f}, 45.0f), TInit({1.0f, 0.0f, 0.0f}, 45.0f), // Both rotated colliding
        TInit({0.0f, 0.0f, 0.0f}, 45.0f), TInit({2.0f, 0.0f, 0.0f}, 45.0f), // Both rotated non-colliding
    };

    bool results[6] = {
        true, false, true, false, true, false
    };


    for (int i = 0; i < 6; ++i) {
        int ti = i * 2;
        bool out = TestCollision(transform_inputs[ti], transform_inputs[ti + 1]);
        if (out != results[i]) {
            printf("Test %d failed.\n", i);
        }
    }

    printf("Testing complete\n"); 

    return 0;

}