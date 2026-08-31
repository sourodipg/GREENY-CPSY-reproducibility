#pragma once
#include "params.hpp"
#include <vector>
#include <random>
#include <cmath>
#include <algorithm>

struct Agent {
    double x = 0.0;          // expressed belief
    double debt = 0.0;       // E_i: accumulated unassimilated prediction error
    double attachment = 0.0; // A_i in [0,1]  (single-axis mode)
    double anx = 0.0, avo = 0.0;  // ECR-R dimensions (two-axis mode)
    double kappa = 1.0;           // derived social transmission
    double theta0_trait = 0.0;    // trait baseline for theta drift bounds
    int    ep_left = 0;           // steps remaining in the current episode
    long long ep_steps = 0;       // total steps spent in episode
    double ep_dose = 0.0;         // distress carried during the current episode
    double lam = 0.0;             // per-agent dissipation rate
    double iq = 100.0;
    double alpha = 0.0;      // per-agent update rate
    double theta = 0.0;      // per-agent tolerance threshold
    double phi = 0.0;        // IQ-linked resolution capacity (variant only)
    int    style = 0;        // 0 secure, 1 avoidant, 2 anxious
    int    family_id = 0;
    long long n_collapse = 0;
    // observation accumulators (never feed back into dynamics)
    double sum_raw_mismatch = 0.0;   // undamped |S-x|
    double sum_debt_inflow  = 0.0;   // (1-alpha)|S-x|
    double sum_debt = 0.0;              // E_i over the whole run
    double sym_debt = 0.0; long long sym_n = 0;  // E_i/theta within the window
    double sym_abs  = 0.0;                       // same window, ABSOLUTE E
};

struct Population {
    std::vector<Agent> agents;
    std::vector<double> inside_w, outside_w;
};

inline Population build_population(const Params& P, unsigned seed) {
    std::mt19937 rng(seed);
    std::discrete_distribution<int> style_dist({P.pct_secure, P.pct_avoidant, P.pct_anxious});
    std::normal_distribution<double> sec_d(P.secure_mean, P.secure_std);
    std::normal_distribution<double> avd_d(P.avoid_mean,  P.avoid_std);
    std::normal_distribution<double> anx_d(P.anx_mean,    P.anx_std);
    std::normal_distribution<double> iq_d(P.iq_mean, P.iq_std);
    std::normal_distribution<double> iw_d(P.iw_mean, P.iw_std);
    std::normal_distribution<double> ratio_d(P.ratio_mean, P.ratio_std);
    std::normal_distribution<double> nrm(0.0, 1.0);

    int N = P.n_families * P.family_size;
    Population pop;
    pop.agents.resize(N);
    pop.inside_w.resize(P.n_families);
    pop.outside_w.resize(P.n_families);

    int idx = 0;
    for (int f = 0; f < P.n_families; ++f) {
        double iw = std::min(0.8, std::max(0.1, iw_d(rng)));
        double ratio = std::max(1.5, ratio_d(rng));
        pop.inside_w[f]  = iw;
        pop.outside_w[f] = iw / ratio;
        for (int m = 0; m < P.family_size; ++m) {
            Agent& a = pop.agents[idx];
            int s;
            if (P.two_axis) {
                // Draw from the ESTIMATED bivariate distribution, then classify
                // by the clinical percentile rule. Prevalences are an output.
                double z1=nrm(rng), z2=nrm(rng);
                double za = z1;
                double zv = P.dim_rho*z1 + std::sqrt(std::max(0.0,1.0-P.dim_rho*P.dim_rho))*z2;
                a.anx = std::min(0.999, std::max(0.001, P.anx_mu + P.anx_sd*za));
                a.avo = std::min(0.999, std::max(0.001, P.avo_mu + P.avo_sd*zv));
                // 25th/75th percentile cut-points of the marginals
                const double zHI = 0.6744897501960817;   // Phi^-1(0.75)
                bool hi_a = za >= zHI, hi_v = zv >= zHI;
                // Bartholomew quadrants; reported under the 3-style scheme with
                // fearful folded into anxious for anchor comparability
                if (hi_v && hi_a)       a.style = 2;   // fearful   -> anxious
                else if (hi_v)          a.style = 1;   // dismissing-> avoidant
                else if (hi_a)          a.style = 2;   // preoccupied-> anxious
                else                    a.style = 0;   // secure
                a.kappa = std::min(3.0, std::max(0.0,
                            1.0 + P.c_anx*a.anx - P.c_avo*a.avo));
                a.attachment = std::min(0.99, std::max(0.01,
                    (a.anx + P.w_avo_theta * a.avo) / (1.0 + P.w_avo_theta)));
            } else {
                s = style_dist(rng);
                a.style = s;
                double at = (s == 0) ? sec_d(rng) : (s == 1 ? avd_d(rng) : anx_d(rng));
                a.attachment = std::min(0.99, std::max(0.01, at));
                a.anx = a.attachment; a.avo = 0.0; a.kappa = 1.0;
            }
            a.iq = std::min(160.0, std::max(55.0, iq_d(rng)));
            a.alpha = std::min(0.98, std::max(P.alpha_floor,
                        P.alpha0 * (a.iq / 100.0) * (1.0 - P.K * a.attachment)));
            a.theta = P.theta0 * std::pow(1.0 - a.attachment, P.p_exp);
            a.phi   = std::min(0.90, std::max(0.10, (a.iq - 55.0) / (160.0 - 55.0)));
            a.theta0_trait = a.theta;
            a.lam = P.lambda0 * (a.iq / 100.0);
            a.family_id = f;
            ++idx;
        }
    }
    return pop;
}

// Fraction of agents of each style pinned at the alpha floor. Reported
// alongside every fit: where this is large, K is inert and the clamp -- not
// the mechanism -- is doing the fitting.
inline void alpha_clamp_fraction(const Params& P, unsigned seed, double out[3]) {
    Population pop = build_population(P, seed);
    int n[3] = {0,0,0}, c[3] = {0,0,0};
    for (const auto& a : pop.agents) {
        double raw = P.alpha0 * (a.iq / 100.0) * (1.0 - P.K * a.attachment);
        n[a.style]++;
        if (raw <= P.alpha_floor) c[a.style]++;
    }
    for (int s = 0; s < 3; ++s) out[s] = n[s] ? 100.0 * c[s] / n[s] : 0.0;
}
