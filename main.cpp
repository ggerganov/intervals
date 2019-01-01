/*! \file main.cpp
 *  \brief Enter description here.
 *  \author Georgi Gerganov
 */

#include "intervals.h"

int main() {
    IntervalArray input;
    input.push_back({0, 5,    "blue"});
    input.push_back({5, 9,    "green"});
    input.push_back({9, 10,   "blue"});
    input.push_back({10, 15,  "green"});
    input.push_back({15, 55,  "blue"});
    input.push_back({55, 65,  "green"});
    input.push_back({65, 100, "blue"});

    int N = 3;
    auto res = downsample(input, N);
    //auto res = downsample2(input);

    for (int i = 0; i <= N; ++ i) {
        auto & sol = res[i];
        printf("Using %d intervals, F = %d\n", i, sol.F);
        for (auto & interval : sol) {
            printf("  - {%d, %d, '%s'}\n", interval.x0, interval.x1, interval.color.c_str());
        }
    }

    return 0;
}
