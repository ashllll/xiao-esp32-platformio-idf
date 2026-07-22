# Hardware validation matrix

Build, upload, runtime, and peripheral measurement are separate evidence levels.

| Environment | Compile/package | Current runtime evidence | Status |
|---|---|---|---|
| `xiao_esp32c3` | Passed | No revision-bound device log recorded | Build verified only |
| `xiao_esp32s3` | Passed | Historical read-only evidence was not bound to the current commit | Re-test required |
| `xiao_esp32c6` | Passed | No revision-bound device log recorded | Build verified only |

## Runtime contract

Acceptance requires all of the following:

1. selected chip family matches the running silicon;
2. C3/C6 report 4 MB Flash, or S3 reports 8 MB Flash and initialized Octal PSRAM;
3. `XIAO_RUNTIME_READY version=1` appears;
4. at least ten `heartbeat count=` values increase consecutively;
5. the log records environment, port, source commit, and test date.

```bash
python3 scripts/verify_hardware.py \
  --environment xiao_esp32s3 --port auto --heartbeats 10 --json-output
```

Without `--flash`, the verifier may reset and read the selected device but does not replace persistent firmware. `--flash` is an explicit write operation.

An LED heartbeat is supplementary evidence only. It does not replace the serial contract, and sensor readings require their own wiring and reference validation.
