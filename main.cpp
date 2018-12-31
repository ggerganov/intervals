/*! \file main.cpp
 *  \brief Enter description here.
 *  \author Georgi Gerganov
 */

#include "downsample.h"

int main() {
    IntervalArray input;
    input.push_back({0, 5,    "blue"});
    input.push_back({5, 9,    "green"});
    input.push_back({9, 10,   "blue"});
    input.push_back({10, 15,  "green"});
    input.push_back({15, 55,  "blue"});
    input.push_back({55, 65,  "green"});
    input.push_back({65, 100, "blue"});

    downsample(input, 2);

    return 0;
}
