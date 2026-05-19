# Fingerprint tests

Fingerprint tests verify simulation behavior by comparing event fingerprints
between the current run and a stored baseline. A failing test indicates
that model behavior or simulation configuration has changed and must be
reviewed.

## Test assets

Fingerprint assets are located under `crownet/tests/fingerprint`.

Common files used by this suite:

- `fingerprints`: standard OMNeT++ fingerprint entry point
- `fingerprints_crownet`: CrowNet fingerprint entry point for coupled runs
- `crownet_fingerprinttest`: fingerprint runner script
- `*.yml`: CrowNet test-case definitions
- `*.csv`: OMNeT++ fingerprint test-case definitions
- `hash.d/*`: stored baseline fingerprints

## Running fingerprint tests

From the repository root:

```bash
source $CROWNET_HOME/out/crownet_user/bin/activate
cd $CROWNET_HOME/crownet/tests/fingerprint

# Standard OMNeT++ fingerprint cases
cd /fingerprint_only
./fingerprints

# CrowNet coupled fingerprint cases
cd /fingerprint_and_customHash
./fingerprints_crownet
```

## Containerized CrowNet cases

For coupled simulations, tests are defined in YAML files. These cases typically
run `run_script.py` and may start several containers (for example OMNeT++,
Vadere, SUMO, and control).

Container tags are provided through the `tags` section in the test definition.
If a tag uses `CFG`, the configured value is resolved from
`$CROWNET_HOME/config/CONTAINER_VERSION.config`.

## Baseline updates

If a change is expected and accepted, update baseline files after validating
behavioral correctness. Review generated `.UPDATED` and `.FAILED` files in the
fingerprint result directories before committing any baseline changes.
