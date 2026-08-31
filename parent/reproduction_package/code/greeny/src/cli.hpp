#pragma once
#include "params.hpp"
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <string>

// Every field of Params is overridable as --name=value. Defaults reproduce
// the paper, so `./sim <experiment>` with no flags gives published numbers.
inline bool cli_apply(Params& P, const std::string& arg) {
    auto eq = arg.find('=');
    if (arg.rfind("--", 0) != 0 || eq == std::string::npos) return false;
    std::string k = arg.substr(2, eq - 2), v = arg.substr(eq + 1);
    auto D = [&](double& f){ f = atof(v.c_str()); return true; };
    auto I = [&](int& f){ f = atoi(v.c_str()); return true; };
    auto B = [&](bool& f){ f = (v=="1"||v=="true"||v=="on"||v=="yes"); return true; };
    if (k=="alpha0") return D(P.alpha0);
    if (k=="K") return D(P.K);
    if (k=="theta0") return D(P.theta0);
    if (k=="p") return D(P.p_exp);
    if (k=="gamma") return D(P.gamma);
    if (k=="families") return I(P.n_families);
    if (k=="family-size") return I(P.family_size);
    if (k=="steps") return I(P.max_steps);
    if (k=="seeds") return I(P.n_seeds);
    if (k=="seed0") { P.seed0 = (unsigned)atoi(v.c_str()); return true; }
    if (k=="random-forcing") return B(P.random_forcing);
    if (k=="shock-interval") return D(P.shock_mean_interval);
    if (k=="target-lo") return D(P.target_lo);
    if (k=="target-hi") return D(P.target_hi);
    if (k=="pct-anxious") return D(P.pct_anxious);
    if (k=="pct-secure") return D(P.pct_secure);
    if (k=="pct-avoidant") return D(P.pct_avoidant);
    if (k=="alpha-floor") return D(P.alpha_floor);
    if (k=="two-axis") return B(P.two_axis);
    if (k=="c-anx") return D(P.c_anx);
    if (k=="c-avo") return D(P.c_avo);
    if (k=="w-avo-theta") return D(P.w_avo_theta);
    if (k=="dim-rho") return D(P.dim_rho);
    if (k=="symptom-window") return D(P.symptom_window);
    if (k=="suspended") return B(P.suspended_episodes);
    if (k=="episode-w") return D(P.episode_W);
    if (k=="p-steel") return D(P.p_steel);
    if (k=="delta-theta") return D(P.delta_theta);
    if (k=="release-frac") return D(P.release_frac);
    if (k=="lambda0") return D(P.lambda0);
    if (k=="track-frac") return D(P.track_frac);
    if (k=="dose-ref") return D(P.dose_ref);
    if (k=="recover-frac") return D(P.recover_frac);
    if (k=="extra-var") return D(P.extra_var);
    if (k=="burn-in") return D(P.burn_in_frac);
    if (k=="delta-dose") return D(P.delta_dose);
    if (k=="deadband") return D(P.debt_deadband);
    if (k=="noise") return D(P.perception_noise);
    if (k=="nucleation") return B(P.nucleation_collapse);
    if (k=="C0") return D(P.nuc_C0);
    if (k=="dC") return D(P.nuc_dC);
    if (k=="J0") return D(P.nuc_J0);
    if (k=="arrhenius") return B(P.arrhenius_update);
    if (k=="b0") return D(P.b0);
    if (k=="c-att") return D(P.c_att);
    if (k=="beta") return D(P.beta);
    if (k=="kappa-secure") return D(P.kappa_secure);
    if (k=="kappa-avoidant") return D(P.kappa_avoidant);
    if (k=="kappa-anxious") return D(P.kappa_anxious);
    if (k=="proportional-resolution") return B(P.proportional_resolution);
    if (k=="decoupled-collapse") return B(P.decoupled_collapse);
    fprintf(stderr, "unknown option: --%s\n", k.c_str());
    exit(2);
}

inline void cli_knobs_help() {
    printf(
"Model knobs (all optional; defaults reproduce the paper)\n"
"  --alpha0=F                 base update rate            [0.04]  FITTED\n"
"  --K=F                      attachment suppression      [0.30]  NOT IDENTIFIED\n"
"  --theta0=F                 tolerance scale             [2.50]  fitted (ridge)\n"
"  --p=F                      tolerance exponent          [1.50]  fitted (ridge)\n"
"  --gamma=F                  debt retention              [0.93]\n"
"  --families=N               number of families          [1000]\n"
"  --family-size=N            agents per family           [4]\n"
"  --steps=N                  timesteps per run           [12000]\n"
"  --seeds=N                  Monte Carlo seeds           [3]\n"
"  --seed0=N                  first seed                  [101]\n"
"  --random-forcing=0|1       Poisson vs fixed cycle      [1]\n"
"  --shock-interval=F         mean steps between shocks   [500]\n"
"  --target-lo=F --target-hi=F  shock magnitude range     [0.5 5.8]\n"
"  --pct-secure/-avoidant/-anxious=F  composition   [59 25 11]\n"
"  --two-axis=0|1             ECR-R anxiety x avoidance   [0]\n"
"  --c-anx=F                  hyperactivation gain        [0.50]\n"
"  --c-avo=F                  deactivation gain           [1.00]\n"
"  --w-avo-theta=F            avoidance weight in theta   [0.00]\n"
"  --dim-rho=F                anxiety-avoidance correlation[0.10]\n"
"  --deadband=F               perceptual deadband on debt  [0]\n"
"  --noise=F                  idiosyncratic perception noise [0]\n"
"  --nucleation=0|1           nucleation collapse rule    [0]\n"
"  --C0=F                     nucleation barrier at A=0   [2.00]\n"
"  --dC=F                     attachment reduction of it  [1.50]\n"
"  --J0=F                     attempt frequency           [1.00]\n"
"  --arrhenius=0|1            collision-theory update rate [0]\n"
"  --b0=F                     barrier displacement scale  [1.00]\n"
"  --c-att=F                  attachment dep. of barrier   [0.00]\n"
"  --beta=F                   inverse social temperature   [1.00]\n"
"  --kappa-secure=F           social transmission, secure  [1.00]\n"
"  --kappa-avoidant=F         social transmission, avoidant[1.00]  <- steric hindrance\n"
"  --kappa-anxious=F          social transmission, anxious [1.00]\n"
"  --proportional-resolution=0|1  phi-gated discharge     [0]\n"
"  --decoupled-collapse=0|1   discharge without recalibration [0]\n");
}
