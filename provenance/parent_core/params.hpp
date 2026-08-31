#pragma once
#include <string>

// ============================================================================
// All tunable knobs live here. DEFAULTS ARE THE PAPER'S CALIBRATED VALUES:
// running any experiment with no overrides reproduces the published numbers.
// Every field is settable from the command line (see cli.hpp).
// ============================================================================
struct Params {
    // ---- Fitted to clinical anchors -----------------------------------------
    // alpha0 : base belief-update rate. IDENTIFIED (feasible band 0.025-0.047).
    // K      : attachment suppression of update rate. NOT IDENTIFIED by any
    //          available anchor -- moves every target by <12% of its tolerance
    //          across a 5.5x change. Reported as a range, not a point estimate.
    double alpha0 = 0.02;
    double K      = 0.30;

    // ---- Vulnerability threshold: theta_i = theta0 * (1 - A_i)^p ------------
    // Jointly identified along a diagonal ridge: theta0 in [1.25, 4.50],
    // p in [1.35, 1.90], anti-correlated. Asserted in the source manuscript,
    // never fitted there; both asserted values fall inside the fitted region.
    double theta0 = 28.0;
    double p_exp  = 1.50;

    // ---- Debt leak ---------------------------------------------------------
    // gamma: retention of unassimilated prediction error per step.
    // Loosely grounded in RLWM forgetting rates (Collins & Frank).
    double gamma  = 0.93;

    // ---- Population structure ----------------------------------------------
    int    n_families  = 1000;
    int    family_size = 4;
    double iw_mean = 0.40, iw_std = 0.06;   // within-family coupling weight
    double ratio_mean = 3.0, ratio_std = 0.2; // inside:outside weight ratio

    // Attachment style prevalence (Mickelson et al., 1997) and per-style
    // insecurity priors. Prevalences sum to 95%; the residual ~5%
    // (disorganised/fearful) is not represented -- see paper limitations.
    double pct_secure = 59.0, pct_avoidant = 25.0, pct_anxious = 11.0;
    double secure_mean = 0.25, secure_std = 0.10;
    double avoid_mean  = 0.55, avoid_std  = 0.12;
    double anx_mean    = 0.80, anx_std    = 0.10;
    double iq_mean = 100.0, iq_std = 15.0;   // WAIS-IV standardisation

    // ---- Environmental forcing ---------------------------------------------
    // random_forcing=true  : Poisson arrivals, magnitude ~ U(lo,hi)  [DEFAULT]
    // random_forcing=false : the deterministic 8-value/500-step cycle used by
    //                        the source manuscript. Retained ONLY to reproduce
    //                        the schedule-dependence result: the calibration
    //                        is conditional on this choice (see exp forcing).
    bool   random_forcing = true;
    double shock_mean_interval = 500.0;
    double target_lo = 0.5, target_hi = 5.8;

    int    max_steps   = 12000;
    int    shift_every = 500;   // period of the deterministic schedule

    // ---- Optional mechanism variants (all OFF by default) ------------------
    // proportional_resolution: crisis discharge is only phi-complete, where
    //   phi is an IQ-linked capacity trait. Off reproduces the original
    //   all-or-nothing rule.
    bool   proportional_resolution = false;
    // decoupled_collapse: discharge debt WITHOUT snapping belief onto the
    //   signal. Isolates the recalibration artifact (see exp distress).
    bool   decoupled_collapse = false;

    // ---- Run control -------------------------------------------------------
    int    n_seeds = 3;
    unsigned seed0 = 101;
    // Episodes in the opening transient are counted from a cold start, where
    // every agent begins far from the world state. Measured over the full run
    // the rate is about 10 per cent higher than over a doubled run. The rate is
    // therefore counted only after a burn-in fraction of the run.
    double burn_in_frac = 0.25;
    // ---- Everything else that causes distress -------------------------------
    // The model contains ONE pathway to distress: attachment-driven failure to
    // absorb prediction error. Real depression also arises from genetics,
    // illness, circumstance and loss, none of which are represented. Both
    // real-world measures also carry error (ECR alpha ~.77-.91; depression
    // scales ~.85). The model therefore correlates TRUE attachment with a PURE
    // attachment-driven outcome and must over-estimate any observable r.
    //
    // extra_var is the ratio of non-attachment to attachment variance in the
    // measured symptom score. It attenuates by 1/sqrt(1 + extra_var), the
    // standard formula, and is the only quantity here fitted to the gap
    // between the model's correlation and the meta-analytic one.
    double extra_var = 1.10;
    // ---- Episode as a graded, dissipating state -----------------------------
    // Not a freeze. During an episode E stays high but DRAINS at a rate tied
    // to cognitive resources, and belief tracking is impaired but not abolished
    // (freezing x entirely made episodes absorbing: agents woke decalibrated
    // and re-entered within one step).
    //
    //   during:  E <- (1-lambda_i) E,   x updates at track_frac * alpha
    //   lambda_i = lambda0 * (IQ_i/100)      low IQ -> slow drain -> long episode
    //   ends when E < recover_frac * theta   DURATION IS EMERGENT, not set
    //
    // Episodes are then unequal in a measurable way. The distinguishing
    // quantity is DOSE, the distress carried while above threshold:
    //   dose = sum of E over the episode
    // which depends on how far over you went AND how slowly it drained.
    //
    //   theta <- theta * exp(-delta_dose * (dose/dose_ref - 1))
    // A light episode raises theta (steeling); a heavy one lowers it
    // (scarring). Same event, opposite consequence, decided by severity
    // rather than a coin flip.
    double lambda0      = 0.004;   // base dissipation per step during episode
    double track_frac   = 0.25;   // belief tracking retained while unwell
    double recover_frac = 0.80;   // episode ends below this fraction of theta
    // Dose reference, DIMENSIONLESS. Dose is accumulated as severity (L/theta,
    // itself dimensionless) per unit of shock-interval time, so D is invariant
    // both to the arbitrary belief scale and to the step size. The earlier
    // formulation summed raw L over steps, giving units of belief x steps and a
    // dose_ref that silently carried them; scaling the belief unit then changed
    // the episode rate, which it must not.
    double dose_ref     = 0.16;
    double delta_dose   = 0.15;   // sensitivity of theta to dose
    // ---- Episodes as SUSPENDED ANIMATION, not discharge ---------------------
    // A depressive episode is a sustained state, not an instantaneous reset.
    // Crossing theta enters an episode: belief updating and debt dynamics are
    // SUSPENDED (E stays above threshold) for a duration, after which the
    // agent emerges with theta shifted probabilistically -- sensitised
    // (scarring/kindling) or strengthened (steeling).
    //
    // This is what makes a cross-sectional severity measure valid: agents who
    // episode more often now spend MORE time at high E, instead of being
    // zeroed by the very event that marks pathology.
    //
    // Duration is dimensionless, in life events: mean MDE ~6 months, at
    // ~1.5 life events/yr gives W_ep ~ 0.75.
    bool   suspended_episodes = true;
    double episode_W    = 0.75;   // duration, in life events
    double p_steel      = 0.50;   // P(theta rises) on emergence
    double delta_theta  = 0.10;   // multiplicative step in theta
    double release_frac = 0.60;   // E released to this fraction of theta
    double theta_lo_mult = 0.5, theta_hi_mult = 2.0;  // bounds on drift
    // ---- Dimensionless observation windows ---------------------------------
    // The model has no intrinsic time unit; its only clock with an external
    // referent is the shock process, one shock = one major life event
    // (~1.5/yr). All windows are therefore expressed as W = T_obs/tau_shock,
    // in units of life events.
    //
    //   A4 rate      Solomon et al., 10-yr prospective   -> W ~ 15
    //   A3/A5 corr   CES-D/BDI/PHQ-9, cross-sectional,
    //                1-2 week symptom window             -> W ~ 0.06
    //
    // Because a 2-week window contains almost no episodes, the construct-valid
    // analogue of a cross-sectional symptom SCALE is not an episode count but
    // the current unresolved-distress level E averaged over that window.
    // W, in life events. Both this and the episode-rate anchor derive from one
    // assumed life-event rate (1-2/yr, midpoint 1.5/yr), applied to different
    // windows: CES-D asks about the past two weeks, 14 days x 1.5/365 = 0.058;
    // the Solomon anchor covers ten years, 10 x 1.5 = 15.
    // W, in life events. Both this and the episode-rate anchor derive from one
    // assumed life-event rate (1-2/yr, midpoint 1.5/yr), applied to different
    // windows: CES-D asks about the past two weeks, 14 days x 1.5/365 = 0.058;
    // the Solomon anchor covers ten years, 10 x 1.5 = 15.
    double symptom_window = 0.06;
    // Lower bound on the per-agent update rate. At the fitted alpha0 the
    // original floor of 0.02 was binding for 70-93% of agents, making K and
    // the IQ term inert and the fit clamp-driven rather than mechanistic.
    double alpha_floor = 0.002;  // inactive at the fitted point; verified

    // ---- TWO-DIMENSIONAL ATTACHMENT (ECR-R structure) -----------------------
    // Adult attachment is measured on two orthogonal dimensions, anxiety and
    // avoidance, not one insecurity scale. Collapsing them onto a single axis
    // -- with avoidant placed BETWEEN secure and anxious -- is what makes every
    // single-channel mechanism move rate and ratio together, because both are
    // then functions of the same scalar.
    //
    // The four quadrants are the standard Bartholomew typology, and their
    // prevalences sum to 100%, unlike the 95% of the three-style scheme (the
    // missing ~5% is the fearful quadrant, high on BOTH dimensions).
    //
    //   theta_i = theta0 * (1 - anx_i)^p          vulnerability from ANXIETY
    //   kappa_i = clamp(1 + c_anx*anx_i - c_avo*avo_i, 0, 3)
    //                                             hyperactivation from anxiety,
    //                                             deactivation from avoidance
    //
    // This SUBSUMES the per-style kappa: instead of three free values it is two
    // coefficients on two measured dimensions.
    bool   two_axis = true;
    // ---- ESTIMATED, not asserted ------------------------------------------
    // Normative ECR-S moments from Wei et al. (2007), N = 851, as tabulated by
    // Hegarty et al. (2024). Scores are 6 items on a 7-point Likert scale,
    // range 6-42, normalised here to [0,1] by (x-6)/36.
    //
    //   anxiety    M = 22.45, SD = 7.14  ->  mu = 0.457, sigma = 0.198
    //   avoidance  M = 14.97, SD = 6.40  ->  mu = 0.249, sigma = 0.178
    //
    // Agents are drawn from a bivariate normal with these moments and a small
    // positive correlation, then CLASSIFIED by the percentile rule used
    // clinically (Low <= 25th, High >= 75th). Quadrant prevalences are
    // therefore an OUTPUT of the fitted distribution, not an input -- and can
    // be compared against observed prevalences as a check.
    //
    // This replaces eight asserted quadrant means, four asserted prevalences
    // and a free within-quadrant spread with five constants taken from
    // published normative data.
    double anx_mu = 0.4569, anx_sd = 0.1983;
    double avo_mu = 0.2492, avo_sd = 0.1778;
    double dim_rho = 0.28;   // anxiety-avoidance correlation, Wei et al. (2007),
                             // SAME sample as the means and SDs above (r=.28 for
                             // the 12-item ECR-S, .30 for the 36-item ECR).
    double c_anx = 0.50;   // hyperactivation gain on the social channel
    double c_avo = 0.90;   // deactivation gain on the social channel
    // Vulnerability index feeding theta uses a DIFFERENT projection of the
    // (anxiety, avoidance) plane than kappa does. Two distinct linear
    // functionals of the same 2-D position is what makes the model genuinely
    // two-dimensional; a single shared projection would collapse back to one
    // axis under a different name.
    //   v_i     = clamp(anx_i + w_avo_theta * avo_i, 0, 0.99)
    //   theta_i = theta0 * (1 - v_i)^p
    double w_avo_theta = 0.59;

    // ---- Perceptual deadband on debt accrual --------------------------------
    // E <- gamma*E + (1-alpha)*max(0, |d| - debt_deadband)
    // Mismatch below the deadband is not experienced as strain (perceptual
    // just-noticeable-difference; minor daily discrepancy does not accumulate
    // as distress). Placed UPSTREAM of accumulation, where the transmission
    // coefficient succeeded and where every firing-rule mechanism failed to
    // driving-force compensation. Units [B]. Zero reproduces the original rule.
    double debt_deadband = 0.0;

    // ---- Idiosyncratic perception noise ------------------------------------
    // One parameter, [B], added to each agent's perceived signal each step.
    // Purpose: create belief DISPERSION without building a full interaction
    // potential. The unmodified model collapses to total consensus (sd of
    // beliefs ~1e-4 against a target range of 5.3), so any mechanism whose
    // value depends on belief diversity -- Lennard-Jones spacing, bounded
    // confidence, contagion -- has nothing to act on. This tests cheaply
    // whether nonzero dispersion moves the anchors in a useful direction
    // before paying the parameter cost of an interaction potential.
    double perception_noise = 0.0;

    // ---- Nucleation-theory collapse rule -----------------------------------
    // Classical nucleation theory: dG* = 16 pi gamma^3 / (3 dg^2), so the
    // barrier DIVERGES as the driving force dg vanishes and the rate is
    // J = J0 exp(-dG*/kT). Here the driving force is the accumulated debt E_i
    // and the barrier enters ADDITIVELY in the attachment term:
    //
    //   C_i   = max(C_min, nuc_C0 - nuc_dC * A_i)        barrier  [B^2]
    //   p_i   = nuc_J0 * exp(-C_i / E_i^2)               per-step probability
    //
    // Dimensions: E_i is [B] so E_i^2 is [B^2]; C_i is [B^2]; the exponent is
    // dimensionless.
    //
    // WHY ADDITIVE: the ratio of nucleation rates between two styles is
    //   J_a/J_s = exp( nuc_dC * (A_a - A_s) / E^2 )
    // which depends on nuc_dC ALONE, while the overall scale is set by nuc_C0.
    // Rate and ratio are therefore controlled by separate parameters. Under
    // the multiplicative threshold rule theta_i = theta0*(1-A_i)^p they are
    // not: theta0 moves both together via first-passage, which is the
    // rate-ratio bind this rule is meant to escape.
    //
    // Replaces the hard threshold when enabled; nucleation_collapse=false
    // leaves the original deterministic rule untouched.
    bool   nucleation_collapse = false;
    double nuc_C0 = 2.0;    // barrier at A = 0                       [B^2]
    double nuc_dC = 1.5;    // additive attachment reduction of barrier[B^2]
    double nuc_J0 = 1.0;    // attempt frequency (per step, bounded)   [1]
    double nuc_Cmin = 0.02; // floor keeping the barrier positive      [B^2]


    // ---- Arrhenius / collision-theory update rate --------------------------
    // DIMENSIONS.  [B] = belief-units (units of x, T, S).  Energies are [B^2],
    // as for a harmonic displacement.  The barrier Ea_i is DELIBERATELY a
    // separate quantity from the collapse threshold theta_i: theta is [B] (it
    // is compared to debt, which accumulates |S-x|), so theta cannot serve as
    // an activation energy without a dimensional error -- exactly the defect
    // in the originally proposed form, where theta/(kT + E_coll) evaluates to
    // [B]/[B^2] = [B^-1] in an exponent that must be dimensionless.
    //
    //   E_coll(t) = 0.5*(S_i - x_i)^2                     [B^2]
    //   Ea_i      = 0.5*b_i^2,  b_i = b0*(1 + c*A_i)      [B^2], b in [B]
    //   alpha_i(t)= clamp[(IQ/100)*exp(-beta*max(0, Ea_i - E_coll)), .02, .98]
    //   beta      = 1/(k_B T_social)                      [B^-2]
    //
    // beta*Ea is dimensionless.  Limits: E_coll >= Ea gives full response;
    // E_coll = 0 recovers pure Arrhenius; beta -> 0 gives a linear-like model;
    // beta -> inf gives a hard threshold at E_coll = Ea.
    //
    // Substantively this makes the update rate RISE with discrepancy: small
    // gaps fall below activation and are ignored, large gaps saturate. That is
    // structurally opposite to bounded-confidence opinion dynamics, in which
    // distant opinions are the ones ignored.
    bool   arrhenius_update = false;   // false -> original linear alpha
    double b0    = 1.0;    // barrier displacement scale        [B]
    double c_att = 0.0;    // attachment dependence of barrier  [1]
    double beta  = 1.0;    // inverse social temperature        [B^-2]

    // ---- Social transmission coefficient (chemical-kinetic analogue) -------
    // kappa_i scales the SOCIAL portion of the perceived signal only; the
    // truth channel is untouched. kappa = 1 reproduces the original signal
    // equation algebraically; kappa -> 0 is total steric hindrance, in which
    // social collisions bounce off and the agent tracks the environment alone.
    //
    // Placed here, NOT in the update rate, so theta_i keeps its single role as
    // the collapse threshold. Reusing theta as an activation barrier would make
    // it govern both update speed and decompensation -- the same double-duty
    // that inverted an earlier result in this codebase.
    //
    // Because avoidant attachment sits BETWEEN secure and anxious on the single
    // insecurity axis A, a kappa that is low for avoidant and high for the
    // other two is non-monotone in A and therefore constitutes a genuine second
    // axis (deactivating strategies), which A alone cannot express.
    double kappa_secure   = 1.0;
    double kappa_avoidant = 1.0;
    double kappa_anxious  = 1.0;
};

// Clinical anchor targets used throughout.
struct Anchors {
    // A1: anxious/secure decompensation odds (Zheng 2020; Sirikantraporn 2021)
    double anx_or_lo = 3.86, anx_or_hi = 6.00;
    // A2: avoidant/secure odds (Zheng 2020; Bifulco 2002)
    double av_or_lo  = 1.60, av_or_hi  = 2.20;
    // A3/A5: DIMENSIONAL correlations with depression from Zhang et al.
    // (2022), k = 124 studies: attachment anxiety r = .40, attachment
    // avoidance r = .28. These are commensurable with the ECR-derived
    // distribution above -- both are dimensional. The categorical odds
    // ratios (A1, A2) come from forced-choice instruments and are NOT
    // commensurable with either; they are retained only as a secondary
    // consistency check, not as fitting targets.
    double r_lo = 0.36, r_hi = 0.44;      // A3: anxiety x depression, .40
    double ravo_lo = 0.24, ravo_hi = 0.32; // A5: avoidance x depression, .28
    // A4: absolute event rate, dimensionless as episodes per major life
    //     event. Solomon et al. 2000: 0.21 episodes/yr; 25-yr cohort: 0.12/yr;
    //     major life events ~1-2/yr. Generous band used.
    double cps_lo = 0.105, cps_hi = 0.210;  // derived from Solomon: 0.21/yr at 1-2 events/yr
};
