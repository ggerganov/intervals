/*! \file intervals.h
 *  \brief Enter description here.
 *  \author Georgi Gerganov
 */

#include <vector>
#include <string>

#pragma once

struct Interval {
    int x0;
    int x1;
    std::string color;
};

using IntervalArray = std::vector<Interval>;

IntervalArray downsample(const IntervalArray & input, int N) {
    int x_max = input.back().x1;

    std::vector<int> G(x_max + 1, 0);
    for (const auto & ii : input) {
        if (ii.color == "green") {
            for (int x = ii.x0; x < ii.x1; ++x) {
                G[x + 1] = G[x] + 1;
            }
        } else {
            for (int x = ii.x0; x < ii.x1; ++x) {
                G[x + 1] = G[x];
            }
        }
    }

    int G_max = G[x_max];
    int inf = 100*G_max;
    printf("G_max = %d\n", G_max);
    printf("G[8]  = %d\n", G[8]);
    printf("G[9]  = %d\n", G[9]);
    printf("G[10] = %d\n", G[10]);

    struct Cell {
        int fval = -1;
        int xprev = -1;
        int w = -1;
    };

    std::vector<std::vector<Cell>> F(x_max + 1);
    for (int x = 0; x <= x_max; ++x) {
        F[x].resize(N + 1);
        F[x][0].fval = G_max;
    }

    for (int n = 1; n <= N; ++n) {
        for (int x = 0; x <= n; ++x) {
            F[x][n].fval = inf;
        }
    }

    for (int n = 1; n <= N; ++n) {
        for (int x = n + 1; x <= x_max; ++x) {
            int best_fval = inf;
            int best_xprev = -1;
            int best_w = -1;
            for (int l = n; l < x; ++l) {
                for (int r = l + 1; r <= x; ++r) {
                    int cur_fval = F[l][n - 1].fval + (r - l) - 2*(G[r] - G[l]);
                    if (cur_fval < best_fval) {
                        best_fval = cur_fval;
                        best_xprev = l;
                        best_w = r - l;
                    }
                }
            }
            F[x][n].fval = best_fval;
            F[x][n].xprev = best_xprev;
            F[x][n].w = best_w;
        }
    }

    // generate output
    IntervalArray res;
    {
        std::vector<int> grid(x_max, 0); // initally, everything is background (i.e. blue)

        int best_x = 0;
        int best_fval = F[0][N].fval;
        for (int x = 0; x <= x_max; ++x) {
            if (F[x][N].fval < best_fval) {
                best_fval = F[x][N].fval;
                best_x = x;
            }
        }

        while (true) {
            printf("F = %d x = %d, xprev = %d, xprev + w = %d, w = %d\n",
                   F[best_x][N].fval, best_x, F[best_x][N].xprev, F[best_x][N].xprev + F[best_x][N].w, F[best_x][N].w);
            int xi = F[best_x][N].xprev;
            int wi = F[best_x][N].w;
            for (int x = xi; x < xi + wi; ++x) {
                grid[x] = 1; // i.e. green
            }
            best_x = F[best_x][N].xprev;
            --N;
            if (N == 0) break;
        }

        int x0 = 0;
        int col = grid[0];
        for (int x1 = 1; x1 < x_max; ++x1) {
            if (grid[x1] != grid[x1 - 1]) {
                res.push_back({x0, x1, (col == 0) ? "blue" : "green"});
                x0 = x1;
                col = grid[x1];
            }
        }
    }

    return res;
}
