# Result-based tests

Result-based tests validate behavioral stability against numeric baselines
instead of strict byte-identical output.

This approach is useful when small numeric drift is acceptable, but changes in
key metrics must still be detected.

## Workflow

Each case performs the following steps:

1. Run the configured simulation command.
2. Locate the newest `.vec` and `.sca` result files.
3. Extract selected vector and scalar metrics.
4. Aggregate matched module outputs.
5. Compare current values against baseline CSV files.
6. Evaluate metric thresholds.
7. Generate diagnostics, including vector plots.

## Comparison criteria

Scalar checks fail when:

`|baseline - current| > abs_threshold`

Vector checks fail when either condition is true:

- `rmse_delta > rmse_threshold`
- `max_abs_delta > max_abs_threshold`

## Typical commands

```bash
source $CROWNET_HOME/out/crownet_user/bin/activate
cd $CROWNET_HOME/crownet/tests/result_compare

# Run all discovered YAML configurations
./result_compare_test

# Run selected YAML configurations
./result_compare_test --config-file openair.yml --config-file mucFreiheit.yml

# Run selected cases
./result_compare_test --case openair_s4 --case simple_detour_wlan

# Accept validated behavior changes
./result_compare_test --update-baseline --case <case_name>
```

## Baseline and artifact layout

Baseline files are versioned per case under `baselines/<case_name>/`.

- Vector baseline: `vector_<metric_id>.csv` with `time,value`
- Scalar baseline: `scalar_<metric_id>.csv` with `value`

Comparison artifacts are written to `output/` and include summary
reports, detailed compare CSV files, and vector plots.
