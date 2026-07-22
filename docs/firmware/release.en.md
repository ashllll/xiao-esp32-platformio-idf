# Firmware releases

A release is an immutable, traceable delivery, not just `firmware.bin` copied from a local build.

## Required contents

- application image;
- bootloader;
- partition table;
- portable flash arguments;
- manifest with project version, source commit, board environment, toolchain baseline, and sizes;
- SHA-256 checksums;
- upgrade, rollback, and known-issue notes.

Create each board package with:

```bash
python3 scripts/package_firmware.py --environment xiao_esp32c3
```

Build and package C3, S3, and C6 from a clean source revision. Verify checksums after copying or downloading.

## Evidence labels

Release notes must say which environments were only compiled and which exact devices passed the runtime contract. Do not turn historical or unbound serial logs into a current hardware claim.

## Versioning

Keep `PROJECT_VER`, `project-baseline.json`, `CHANGELOG.md`, documentation, and the release tag synchronized. The current source version is 0.3.0; absence of a matching immutable tag/release must be stated until one is deliberately published.

## Rollback

Preserve the previous known-good complete package, including its partition table and flash arguments. Back up user configuration when the migration path supports it, and document whether an update erases or changes NVS.

CI artifacts are suitable for validation but are not a permanent release channel.
