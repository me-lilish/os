#pragma once
#include "ktypes.h"

namespace Math {
    int add(int a, int b);
    int subtract(int a, int b);
    int multiply(int a, int b);
    int divide(int a, int b);
    int modulo(int a, int b);
    int round_scaled(int scaled_value, int scale);

    /*
     * Rounds a floating point number to the nearest integer.
     * * Since we are in a kernel without a standard library (no <cmath>),
     * and possibly without hardware Floating Point Unit (FPU) support enabled 
     * at this stage, we handle the floating point math using integer bitwise operations.
     */
    int round(float x);

}
