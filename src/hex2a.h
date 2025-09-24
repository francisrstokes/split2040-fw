#pragma once

#include "keyboard.h"

#define ___  KC_NONE

#define LAYOUT_HEX2A(k0, k1, k2, k3, k4, k5, k6, k7, k8, k9, k10, k11, k12, k13, k14, k15, k16, k17, k18, k19, k20, k21, k22, k23, k24, k25, k26, k27, k28, k29, k30, k31, k32, k33, k34, k35, k39, k40, k41, k42, k43, k44) { \
    {k0,  k1,  k2,  k3,  k4,  k5,  k6,  k7,  k8,  k9,  k10, k11}, \
    {k12, k13, k14, k15, k16, k17, k18, k19, k20, k21, k22, k23}, \
    {k24, k25, k26, k27, k28, k29, k30, k31, k32, k33, k34, k35}, \
    {___, ___, ___, k39, k40, k41, k42, k43, k44, ___, ___, ___} \
}

