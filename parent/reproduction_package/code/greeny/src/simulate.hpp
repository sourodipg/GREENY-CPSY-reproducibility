#pragma once
#include "params.hpp"
#include "population.hpp"
#include <vector>
#include <random>
#include <cmath>
#include <algorithm>

struct RunResult {
    double rate[3] = {0,0,0};      // collapses per agent, by style
    double anx_or = 0, av_or = 0;  // odds ratios vs secure
    double r_count = 0;            // corr(anxiety, episode count)   [A3]
    double r_avo   = 0;
    double r_cnt_long = 0;            // corr(avoidance, episode count) [A5, two-axis only]
    double r_debt = 0;             // corr(A, mean debt)          [wrong sign]
    double r_raw = 0;              // corr(A, mean |S-x|)         [wrong sign]
    double r_AE = 0;               // corr(A, A*E) -- TAUTOLOGICAL, for audit only
    double collapses_per_shock = 0;// [A4] dimensionless absolute rate
    double clamp_frac[3] = {0,0,0};
};

inline double pearson(const std::vector<double>& x, const std::vector<double>& y) {
    const int n = (int)x.size();
    double sx=0, sy=0, sxx=0, syy=0, sxy=0;
    for (int i = 0; i < n; ++i) { sx+=x[i]; sy+=y[i]; sxx+=x[i]*x[i]; syy+=y[i]*y[i]; sxy+=x[i]*y[i]; }
    const double mx = sx/n, my = sy/n;
    const double den = std::sqrt((sxx/n - mx*mx) * (syy/n - my*my));
    return den > 0 ? (sxy/n - mx*my) / den : 0.0;
}

inline const std::vector<double>& fixed_schedule() {
    static const std::vector<double> T = {1.0,5.0,0.5,5.5,1.2,5.2,0.8,5.8};
    return T;
}

// Poisson-arrival shocks with independently drawn magnitudes. Removes both the
// periodicity and the anomalously small first jump of the deterministic cycle,
// each of which was shown to generate spurious apparent effects.
inline std::vector<double> random_schedule(const Params& P, unsigned seed) {
    std::mt19937 rng(seed);
    std::exponential_distribution<double> gap(1.0 / P.shock_mean_interval);
    std::uniform_real_distribution<double> mag(P.target_lo, P.target_hi);
    std::vector<double> T(P.max_steps);
    double v = mag(rng), next = gap(rng);
    for (int t = 0; t < P.max_steps; ++t) {
        while ((double)t >= next) { v = mag(rng); next += gap(rng); }
        T[t] = v;
    }
    return T;
}

inline RunResult simulate_once(const Params& P, unsigned seed) {
    Population pop = build_population(P, seed);
    const int N = (int)pop.agents.size();
    std::vector<double> T = P.random_forcing ? random_schedule(P, seed + 777013u)
                                             : std::vector<double>();
    const auto& FS = fixed_schedule();

    std::mt19937 rng(seed + 1000003u);
    std::uniform_int_distribution<int> pick(0, N - 1);
    std::uniform_real_distribution<double> unif(0.0, 1.0);
    std::normal_distribution<double> gauss(0.0, 1.0);
    long long cat[3] = {0,0,0};   // per-style episode tally, post burn-in
    const int burn = (int)(P.burn_in_frac * P.max_steps);

    // trailing cross-sectional symptom window, in life events
    const int sym_len   = std::max(1, (int)(P.symptom_window * P.shock_mean_interval));
    const int sym_start = std::max(0, P.max_steps - sym_len);
    for (int t = 0; t < P.max_steps; ++t) {
        std::vector<double> prev(N);
        for (int i = 0; i < N; ++i) prev[i] = pop.agents[i].x;
        std::vector<double> fam(P.n_families, 0.0);
        for (int i = 0; i < N; ++i) fam[pop.agents[i].family_id] += prev[i];
        for (int f = 0; f < P.n_families; ++f) fam[f] /= P.family_size;

        const double target = P.random_forcing ? T[t]
                            : FS[(t / P.shift_every) % FS.size()];

        for (int i = 0; i < N; ++i) {
            Agent& a = pop.agents[i];
            // Suspended episodes are handled BEFORE the ordinary assimilation
            // update. While in an episode, normal debt accumulation is paused;
            // only impaired tracking and dissipation occur. This makes the
            // implementation agree with the model definition: episode dynamics
            // replace, rather than follow, the healthy-state update.
            if (P.suspended_episodes && a.ep_left > 0) {
                const int f_ep = a.family_id;
                const double iw_ep = pop.inside_w[f_ep], ow_ep = pop.outside_w[f_ep];
                const double tw_ep = std::max(0.01, 1.0 - iw_ep - ow_ep);
                const double kap_ep = P.two_axis ? a.kappa : 1.0;
                const double S_ep = (tw_ep*target + kap_ep*(iw_ep*fam[f_ep] + ow_ep*prev[pick(rng)]))
                                  / (tw_ep + kap_ep*(iw_ep + ow_ep));
                a.ep_left = 1;
                a.ep_steps++;
                a.ep_dose += (a.debt / std::max(1e-9, a.theta)) / P.shock_mean_interval;
                a.x += P.track_frac * a.alpha * (S_ep - a.x);
                a.debt *= (1.0 - a.lam);
                a.sum_debt += a.debt;
                if (t >= sym_start) {
                    a.sym_debt += a.debt / std::max(1e-9, a.theta);
                    a.sym_abs += a.debt;
                    a.sym_n++;
                }
                if (a.debt < P.recover_frac * a.theta) {
                    double f_dose = a.ep_dose / std::max(1e-9, P.dose_ref);
                    a.theta = std::min(P.theta_hi_mult * a.theta0_trait,
                              std::max(P.theta_lo_mult * a.theta0_trait,
                              a.theta * std::exp(-P.delta_dose * (f_dose - 1.0))));
                    a.ep_left = 0;
                    a.ep_dose = 0.0;
                }
                continue;
            }
            const int f = a.family_id;
            const double iw = pop.inside_w[f], ow = pop.outside_w[f];
            const double tw = std::max(0.01, 1.0 - iw - ow);
            // Social transmission coefficient: scales the social channel only,
            // leaving the truth channel intact. kappa == 1 is algebraically the
            // original expression, so the default path is unchanged.
            const double kap = P.two_axis ? a.kappa
                             : ((a.style == 0) ? P.kappa_secure
                             :  (a.style == 1) ? P.kappa_avoidant
                                               : P.kappa_anxious);
            const double S = (tw*target + kap*(iw*fam[f] + ow*prev[pick(rng)]))
                           / (tw + kap*(iw + ow));

            const double S_eff = P.perception_noise > 0.0
                               ? S + P.perception_noise * gauss(rng) : S;
            const double d = S_eff - a.x;
            a.sum_raw_mismatch += std::fabs(d);
            // Arrhenius / collision-theory rate. Dimensions: E_coll and Ea are
            // both [B^2]; beta is [B^-2]; the exponent is dimensionless.
            double alpha_eff = a.alpha;
            if (P.arrhenius_update) {
                const double E_coll = 0.5 * d * d;                    // [B^2]
                const double b_i    = P.b0 * (1.0 + P.c_att * a.attachment); // [B]
                const double Ea     = 0.5 * b_i * b_i;                // [B^2]
                const double deficit = std::max(0.0, Ea - E_coll);    // [B^2]
                alpha_eff = std::min(0.98, std::max(0.02,
                              (a.iq / 100.0) * std::exp(-P.beta * deficit)));
            }
            a.x += alpha_eff * d;
            const double strain = std::max(0.0, std::fabs(d) - P.debt_deadband);
            const double inflow = (1.0 - alpha_eff) * strain;
            a.sum_debt_inflow += inflow;
            a.debt = P.gamma * a.debt + inflow;
            a.sum_debt += a.debt;   // sampled BEFORE any reset
            if (t >= sym_start) { a.sym_debt += a.debt / std::max(1e-9, a.theta); a.sym_abs += a.debt; a.sym_n++; }

            if (P.suspended_episodes && a.debt >= a.theta) {
                a.ep_left = 1;
                a.ep_dose = 0.0;
                a.n_collapse++;
                if (t >= burn) cat[a.style]++;
                continue;
            }

            if (!P.suspended_episodes) {
            bool fires;
            if (P.nucleation_collapse) {
                // Nucleation: stochastic, no hard threshold. Sub-critical
                // strain is exponentially unlikely to nucleate; the barrier
                // diverges as debt -> 0.
                const double C_i = std::max(P.nuc_Cmin, P.nuc_C0 - P.nuc_dC * a.attachment);
                const double E2  = a.debt * a.debt;                    // [B^2]
                const double p   = (E2 > 0.0) ? P.nuc_J0 * std::exp(-C_i / E2) : 0.0;
                fires = (unif(rng) < p);
            } else {
                fires = (a.debt >= a.theta);
            }
            if (fires) {
                if (P.proportional_resolution) {
                    a.x   += a.phi * (S_eff - a.x);
                    a.debt = (1.0 - a.phi) * a.debt;
                } else {
                    if (!P.decoupled_collapse) a.x = S_eff;  // forced recalibration
                    a.debt = 0.0;
                }
                a.n_collapse++;
            }
            }   // end of the non-suspended (discharge) branch
        }
    }

    RunResult R;
    int cnt[3] = {0,0,0};
    std::vector<double> A, cvec, dvec, rvec, aevec, Vvec, svec;
    A.reserve(N); cvec.reserve(N); dvec.reserve(N); rvec.reserve(N); aevec.reserve(N);
    for (const auto& a : pop.agents) {
        if (!P.suspended_episodes) cat[a.style] += a.n_collapse;
        cnt[a.style]++;
        const double mean_debt = a.sum_debt / P.max_steps;
        A.push_back(a.anx);
        Vvec.push_back(a.avo);
        cvec.push_back((double)a.n_collapse);
        dvec.push_back(mean_debt);
        rvec.push_back(a.sum_raw_mismatch / P.max_steps);
        aevec.push_back(a.attachment * mean_debt);
        // symptom score = attachment-driven severity + everything else
        double sev = a.sym_n ? a.sym_debt / a.sym_n : 0.0;
        svec.push_back(sev);
        dvec.back() = a.sym_n ? a.sym_abs / a.sym_n : 0.0;  // absolute-unit comparator
    }
    for (int s = 0; s < 3; ++s) R.rate[s] = cnt[s] ? (double)cat[s]/cnt[s] : 0.0;
    R.anx_or = R.rate[2] / std::max(1e-9, R.rate[0]);
    R.av_or  = R.rate[1] / std::max(1e-9, R.rate[0]);
    // A3/A5 use SYMPTOM SEVERITY in the cross-sectional window;
    // A4 uses the episode count over the full prospective run.
    // attenuate for non-attachment causes and measurement error
    {   double m=0; for(double z:svec) m+=z; m/= (double)svec.size();
        double v=0; for(double z:svec) v+=(z-m)*(z-m); v/= (double)svec.size();
        std::normal_distribution<double> en(0.0, std::sqrt(std::max(0.0,v*P.extra_var)));
        for (auto& z : svec) z += en(rng);
    }
    R.r_count = pearson(A, svec);
    R.r_cnt_long = pearson(A, cvec);
    R.r_avo   = P.two_axis ? pearson(Vvec, svec) : 0.0;
    R.r_debt  = pearson(A, dvec);          // absolute severity, same window
    R.r_raw   = pearson(Vvec, dvec);       // absolute severity vs avoidance
    R.r_AE    = pearson(A, aevec);
    const double n_shocks = (double)(P.max_steps - burn) / P.shock_mean_interval;
    R.collapses_per_shock = R.rate[0] / n_shocks;   // secure agents = healthiest
    alpha_clamp_fraction(P, seed, R.clamp_frac);
    return R;
}

// Average across n_seeds. Correlations are averaged (not pooled) -- with
// N = 4000 per seed the difference is negligible and averaging keeps the
// per-seed spread inspectable.
inline RunResult simulate(const Params& P) {
    RunResult M;
    for (int k = 0; k < P.n_seeds; ++k) {
        RunResult r = simulate_once(P, P.seed0 + 101u * k);
        for (int s = 0; s < 3; ++s) { M.rate[s] += r.rate[s]; M.clamp_frac[s] += r.clamp_frac[s]; }
        M.anx_or += r.anx_or; M.av_or += r.av_or;
        M.r_count += r.r_count; M.r_avo += r.r_avo; M.r_debt += r.r_debt;
        M.r_cnt_long += r.r_cnt_long;
        M.r_raw += r.r_raw; M.r_AE += r.r_AE;
        M.collapses_per_shock += r.collapses_per_shock;
    }
    const double n = P.n_seeds;
    for (int s = 0; s < 3; ++s) { M.rate[s] /= n; M.clamp_frac[s] /= n; }
    M.anx_or/=n; M.av_or/=n; M.r_count/=n; M.r_avo/=n; M.r_cnt_long/=n; M.r_debt/=n; M.r_raw/=n; M.r_AE/=n;
    M.collapses_per_shock/=n;
    return M;
}

inline bool meets(const RunResult& R, const Anchors& T, int which) {
    switch (which) {
        case 1: return R.anx_or >= T.anx_or_lo && R.anx_or <= T.anx_or_hi;
        case 2: return R.av_or  >= T.av_or_lo  && R.av_or  <= T.av_or_hi;
        case 3: return R.r_count>= T.r_lo      && R.r_count<= T.r_hi;
        case 4: return R.collapses_per_shock >= T.cps_lo
                    && R.collapses_per_shock <= T.cps_hi;
    }
    return false;
}
