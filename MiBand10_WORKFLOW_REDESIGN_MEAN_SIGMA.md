# MiBand10 Hypothesis Workflow Redesign (Current Scope: `mu`, `sigma`, `H0/H1`)

## 1) Scope Baseline

This redesign assumes the **current practical capability** is:

- Core inferential target: population mean (`mu`)
- Inputs: sample mean, sample size, sample standard deviation, optional known population standard deviation (`sigma`)
- Hypothesis forms: `H0` and `H1` through tail selection (two-tail, left-tail, right-tail)
- Tests:
  - One-sample **t-test** (default path)
  - One-sample **z-test** only when known `sigma` is available

No new statistical families are required in this redesign. The goal is to make the existing flow explicit, consistent, and MiBand10-focused.

## 2) Redesigned User Flow (Screen Runway)

## 2.1 Primary Runway

1. Metric Entry
- User chooses MiBand10 metric context:
  - Heart Rate
  - Steps
  - Sleep
  - Battery

2. Data Input Mode
- User selects:
  - `Data Array` (raw values)
  - `Summary Stats` (`mean`, `n`, `s`, optional `sigma`, `mu0`)

3. Statistics Input / Derivation
- If array mode: derive `mean`, `n`, `s` automatically.
- If summary mode: user enters `mean`, `n`, `s`, optional `sigma`, and hypothesized `mu0`.

4. Test Routing
- If `sigma` exists and `sigma > 0`: show both `Z-Test` and `T-Test`.
- Else: show only `T-Test`.

5. Tail / Hypothesis Definition
- User selects:
  - Two-tail (`H0: mu = mu0`, `H1: mu != mu0`)
  - Left-tail (`H0: mu >= mu0`, `H1: mu < mu0`)
  - Right-tail (`H0: mu <= mu0`, `H1: mu > mu0`)

6. Compute and Interpret
- Run test
- Show test statistic, p-values, confidence interval, decision at alpha
- Display plain-language interpretation for selected metric

## 2.2 Decision Logic (Single Sample)

- Rule A: if known population `sigma` is provided and valid -> prefer `z-test`.
- Rule B: if `sigma` missing/invalid -> use `t-test`.
- Rule C: alpha defaults to `0.05` unless explicitly user-configurable.
- Rule D: conclusion uses selected tail p-value, not always two-tail p-value.

## 3) File Usage Redesign (What Each File Owns)

## 3.1 Existing Files to Keep and Re-scope

- `src/pages/hypothesis/index.ux`
  - Role: Hypothesis home and first router.
  - Redesign: add metric-context selector or route to a metric selector before one-sample flow.

- `src/pages/hypothesis/one_sample_mode/one_sample_mode.ux`
  - Role: input mode selection.
  - Redesign: unchanged behavior; optionally show current metric context badge.

- `src/pages/hypothesis/one_sample_input/one_sample_input.ux`
  - Role: capture/derive `mean`, `n`, `s`, optional `sigma`, and `mu0`.
  - Redesign:
    - enforce validation gates before route:
      - `n > 1`
      - `s >= 0`
      - if provided, `sigma > 0`
    - show quick preview of implied hypotheses using `mu0`.

- `src/pages/hypothesis/one_sample/one_sample.ux`
  - Role: test family selection.
  - Redesign:
    - keep `z-test` visible only when `sigma` valid.
    - keep `t-test` always available.

- `src/pages/hypothesis/tail_type_select/tail_type_select.ux`
  - Role: define `H0/H1` via tail choice.
  - Redesign:
    - normalize formulas to user-facing mean-vs-hypothesis format:
      - two-tail: `mu = mu0` vs `mu != mu0`
      - left-tail: `mu >= mu0` vs `mu < mu0`
      - right-tail: `mu <= mu0` vs `mu > mu0`
    - pass explicit `tailType` forward.

- `src/pages/hypothesis/t_test/t_test.ux`
  - Role: one-sample t inference and result presentation.
  - Redesign:
    - use selected-tail p-value for decision.
    - show all p-values for transparency.
    - label decision with selected alpha and tail.

- `src/pages/hypothesis/z_test/z_test.ux`
  - Role: one-sample z inference and result presentation.
  - Redesign:
    - same decision behavior as `t_test` (selected-tail aware).
    - keep sigma display explicit as known population parameter.

- `src/common/hypothesis_engine.js`
  - Role: statistical computation source of truth.
  - Redesign:
    - keep one-sample t and z API as canonical backend for this scope.
    - keep return payload consistent (`stat`, `p-values`, `CI`, `error`).

## 3.2 Suggested New Documentation File (No Code Dependency)

- `docs/hypothesis/mean_sigma_workflow.md` (or root-level equivalent)
  - Role: product + QA reference for expected one-sample flow, formulas, and pass/fail logic.

## 4) Screen-to-File Routing Map

- Hypothesis Home -> `src/pages/hypothesis/index.ux`
- One-Sample Mode -> `src/pages/hypothesis/one_sample_mode/one_sample_mode.ux`
- Input -> `src/pages/hypothesis/one_sample_input/one_sample_input.ux`
- Test Select -> `src/pages/hypothesis/one_sample/one_sample.ux`
- Tail Select -> `src/pages/hypothesis/tail_type_select/tail_type_select.ux`
- Execute T -> `src/pages/hypothesis/t_test/t_test.ux`
- Execute Z -> `src/pages/hypothesis/z_test/z_test.ux`
- Compute Core -> `src/common/hypothesis_engine.js`

## 5) MiBand10 Metric Adaptation Under Mean-Based Constraint

Because current scope is mean testing, each MiBand10 metric is mapped to a mean endpoint:

- Heart Rate
  - endpoint: mean signed error (`HR_miband - HR_ref`) or mean absolute error
  - `mu0` often set to `0` (for signed error) or threshold value (for MAE)

- Steps
  - endpoint: mean percentage error per interval/session
  - `mu0` set to `0` or tolerance threshold

- Sleep
  - endpoint: mean total sleep time error (minutes) per night
  - `mu0 = 0` or acceptable bias threshold

- Battery
  - endpoint: mean drain rate (`%/hour`) or mean runtime
  - `mu0` set from requirement spec

This keeps the same testing engine while supporting all four business metrics.

## 6) Input Validation and Guardrails

Mandatory guards before compute:

- `n` integer and `n > 1`
- `s >= 0`
- `sigma` optional; if entered, `sigma > 0`
- `mean`, `mu0` finite numbers
- In array mode, at least 2 valid points

Behavioral guards:

- If `sigma` empty -> hide z-test path.
- If any validation fails -> block route and show inline error panel.
- Always show selected `tailType` in final results.

## 7) Result Contract (Unified Across T and Z Screens)

Every result screen should present:

- Inputs summary: `mean`, `n`, `s`, optional `sigma`, `mu0`
- Procedure summary: test type, tail type, alpha, `H0`, `H1`
- Numeric outputs:
  - test statistic (`t` or `z`)
  - two-tail/left-tail/right-tail p-values
  - confidence interval
- Decision:
  - Reject `H0` or Fail to reject `H0`
  - based on selected-tail p-value vs alpha
- MiBand10 interpretation sentence:
  - "Metric passes/does not pass threshold under current condition"

## 8) Recommended Minimal Refactor Plan (Design-Level)

Phase 1 (alignment):

- Standardize hypothesis strings in tail-select and result pages.
- Standardize decision logic to selected tail for both z and t screens.

Phase 2 (clarity):

- Add metric-context label to all one-sample pages.
- Add clear "known population sigma" helper text near sigma input.

Phase 3 (documentation):

- Publish one-page QA checklist tied to this flow.
- Add expected routing and payload table for regression checks.

## 9) Why This Redesign Fits Current Reality

- Uses existing pages and statistical engine.
- Avoids introducing unsupported tests or UI complexity.
- Creates a single, coherent path from `mu`/`sigma` input to `H0/H1` decision.
- Keeps flow consistent with the established interaction pattern used in distribution pages.
