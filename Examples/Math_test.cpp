#include "../Engine/skymath.h"
#include <stdio.h>
#include <cmath>


void TestX(Vector3 v, float angle);

inline bool CompFloats(float a, float b, float eps = 1e-5f) {
    return std::fabs(a-b) < eps;
}

bool CompVector3(Vector3 v, Vector3 w, float eps = 1e-5f) {
    bool x = CompFloats(v.x, w.x, eps);
    bool y = CompFloats(v.y, w.y, eps);
    bool z = CompFloats(v.z, w.z, eps);

    if (x && y && z) return true;
    return false;
}

int main() {

    Vector3 in_vecs[7] = {
        {1.0f, 1.0f, 1.0f},
        {1.0f, 1.0f, 0.0f},
        {1.0f, 0.0f, 1.0f},
        {0.0f, 1.0f, 1.0f},
        {1.0f, 0.0f, 0.0f},
        {0.0f, 1.0f, 0.0f},
        {0.0f, 0.0f, 1.0f},
    };

    float angle = 90.0f;

    for (int i = 0; i < 7; ++i) {
        TestX(in_vecs[i], angle);
    }

    return 0;
}


void TestX(Vector3 v, float angle) {
    Vector3 result = RotateX(v, angle);

    printf("InVec: {%f, %f, %f} // OutVec: {%f, %f, %f}\n", v.x, v.y, v.z, result.x, result.y, result.z);

}
