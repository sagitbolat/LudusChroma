#pragma once
#include <stdint.h>
#include "sky_structs.h"
#include "skymath.h"
#include "skyengine.h"
#include "skyintrinsics.h"



// SECTION: 2D Collision:


// NOTE: Given two OBBs in the form of transforms, test if they are colliding
// NOTE: Uses the Separating Axis Theorem (SAT).
bool TestCollision(Transform obb1, Transform obb2) {
    // NOTE: A and B are the two colliding rectangles
    Vector3 p_a = obb1.position; // Position of A
    Vector3 a_x = RightVector(obb1); // Local x axis of A
    Vector3 a_y = UpVector(obb1); // Local y axis of A
    float w_a = obb1.scale.x / 2; // Half-width of A
    float h_a = obb1.scale.y / 2; // Half-height of A

    Vector3 p_b = obb2.position; // Position of B
    Vector3 b_x = RightVector(obb2); // Local x axis of B
    Vector3 b_y = UpVector(obb2); // Local  y axis of A
    float w_b = obb2.scale.x / 2; // Half-width of B
    float h_b = obb2.scale.y / 2; // Half-height of B
    
    Vector3 T = p_b - p_a;

    // Check against axis a_x:
    float Ta_x = fAbs(VectorDot(T, a_x));
    float Ta_x_test = w_a + fAbs(VectorDot((w_b * b_x), a_x)) + fAbs(VectorDot((h_b * b_y), a_x)); 
    if (Ta_x > Ta_x_test) {
        return false;
    }

    // Check against axis a_y:
    float Ta_y = fAbs(VectorDot(T, a_y));
    float Ta_y_test = h_a + fAbs(VectorDot((w_b * b_x), a_y)) + fAbs(VectorDot((h_b * b_y), a_y)); 
    if (Ta_y > Ta_y_test) {
        return false;
    }

    // Check against axis b_x:
    float Tb_x = fAbs(VectorDot(T, b_x));
    float Tb_x_test = w_b + fAbs(VectorDot((w_a * a_x), b_x)) + fAbs(VectorDot((h_a * a_y), b_x)); 
    if (Tb_x > Tb_x_test) {
        return false;
    }

    // Check against axis a_y:
    float Tb_y = fAbs(VectorDot(T, b_y));
    float Tb_y_test = h_b + fAbs(VectorDot((w_a * a_x), b_y)) + fAbs(VectorDot((h_a * a_y), b_y)); 
    if (Tb_y > Tb_y_test) {
        return false;
    }
    return true;
}