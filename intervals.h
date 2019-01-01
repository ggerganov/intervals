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

struct IntervalArray : public std::vector<Interval> {
    int F = -1;
};

/*
 * Downsample input intervals to just N foreground intervals (or less)
 *
 * Sample input:
 *
 *   IntervalArray input;
 *   input.push_back({0, 5,    "blue"});
 *   input.push_back({5, 9,    "green"});
 *   input.push_back({9, 10,   "blue"});
 *   input.push_back({10, 15,  "green"});
 *   input.push_back({15, 55,  "blue"});
 *   input.push_back({55, 65,  "green"});
 *   input.push_back({65, 100, "blue"});
 *
 * Usage:
 *
 *   auto res = downsample(input, 3);
 *
 * Sample output:
 *
 *   res[1] = {
 *     {0, 55, 'blue'}
 *     {55, 65, 'green'}
 *     {65, 100, 'blue'}
 *   }, F = 18
 *
 *   res[2] = {
 *     {0, 5, 'blue'}
 *     {5, 15, 'green'}
 *     {15, 55, 'blue'}
 *     {55, 65, 'green'}
 *     {65, 100, 'blue'}
 *   }, F = 1
 *
 *   res[3] = {
 *     {0, 5, 'blue'}
 *     {5, 9, 'green'}
 *     {9, 10, 'blue'}
 *     {10, 15, 'green'}
 *     {15, 55, 'blue'}
 *     {55, 65, 'green'}
 *     {65, 100, 'blue'}
 *   }, F = 0
 *
 * The algorithm uses the Dynamic Programming method.
 * Should work fast enough for x_max < 2000 and N < 200.
 *
 * The result is a vector of vectors R. R.size() == N.
 * R[k] is a vector with k foreground (green) intervals - the downsampling result using just k intervals.
 *
 */
std::vector<IntervalArray> downsample(const IntervalArray & input, int N) {
    int x_max = input.back().x1;

    // Pre-calculate G array
    std::vector<int> ls;
    std::vector<int> rs;
    std::vector<int> G(x_max + 1, 0);
    for (const auto & ii : input) {
        if (ii.color == "green") {
            ls.push_back(ii.x0);
            rs.push_back(ii.x1);
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

    struct Cell {
        int fval = -1;
        int xprev = -1;
        int w = -1;
    };

    // Function F + initial conditions
    std::vector<std::vector<Cell>> F(x_max + 1);
    for (int x = 0; x <= x_max; ++x) {
        F[x].resize(N + 1);
        for (auto f : F[x]) f.fval = inf;
        F[x][0].fval = 2*G_max;
    }

    for (int n = 1; n <= N; ++n) {
        for (int x = 0; x <= n; ++x) {
            F[x][n].fval = inf;
        }
    }

    // DP core
    for (int n = 1; n <= N; ++n) {
        for (int x = n + 1; x <= x_max; ++x) {
            int best_fval = inf;
            int best_xprev = -1;
            int best_w = -1;

            //for (int l = n; l < x; ++l) {
            for (auto l : ls) {
                if (l < n) continue;
                if (l >= x) break;
                //for (int r = l + 1; r <= x; ++r) {
                for (auto r : rs) {
                    if (r < l + 1) continue;
                    if (r > x) break;
                    int cur_fval = F[l][n - 1].fval + (r - l) - 3*(G[r] - G[l]);
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
    std::vector<IntervalArray> res(N + 1);
    {
        for (int i = 1; i <= N; ++i) {
            IntervalArray resCur;

            int n = i;
            std::vector<int> grid(x_max + 1, 0); // initally, everything is background (i.e. blue)

            int best_x = 0;
            int best_fval = F[0][n].fval;
            for (int x = 0; x <= x_max; ++x) {
                if (F[x][n].fval < best_fval) {
                    best_fval = F[x][n].fval;
                    best_x = x;
                }
            }

            resCur.F = best_fval;
            while (true) {
                int xi = F[best_x][n].xprev;
                int wi = F[best_x][n].w;
                for (int x = xi; x < xi + wi; ++x) {
                    grid[x] = 1; // i.e. green
                }
                best_x = F[best_x][n].xprev;
                --n;
                if (n == 0) break;
            }

            int x0 = 0;
            int col = grid[0];
            for (int x1 = 1; x1 <= x_max; ++x1) {
                if (grid[x1] != grid[x1 - 1] || x1 == x_max) {
                    resCur.push_back({x0, x1, (col == 0) ? "blue" : "green"});
                    x0 = x1;
                    col = grid[x1];
                }
            }

            res[i] = std::move(resCur);
        }
    }

    return res;
}

/*
 * A simple alternative algorithm for downsampling as suggested in https://stackoverflow.com/a/53980663/4039976
 *
 * Easy to implement, fast, preserves color balance.
 *
 */
std::vector<IntervalArray> downsample2(IntervalArray input) {
    int N = input.size();
    std::vector<IntervalArray> res(N);

    for (int n = N - 1; n >= 1; n -= 2) {
        int idx = -1;
        int lmin = 1e6;
        for (int i = 1; i < (int) input.size() - 2; ++i) {
            int lcur = input[i+1].x1 - input[i].x0;
            if (lcur < lmin) {
                lmin = lcur;
                idx = i;
            }
        }

        input[idx - 1].x1 += input[idx + 1].x1 - input[idx + 1].x0;
        input[idx + 2].x0 = input[idx - 1].x1;
        input.erase(input.begin() + idx);
        input.erase(input.begin() + idx);

        res[n] = input;
        res[n-1] = input;
    }

    return res;
}
