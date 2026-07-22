# Configuration and dependencies

The project uses layered defaults instead of committing generated root `sdkconfig` files.

| File | Scope | Main purpose |
|---|---|---|
| `sdkconfig.defaults` | all boards | console, logging, and optimization defaults |
| `sdkconfig.flash-4mb` | C3/C6 | 4 MB Flash |
| `sdkconfig.xiao-esp32s3` | S3 | 8 MB Flash and Octal PSRAM |

Generated per-environment configuration is stored under `.pio/sdkconfig-<environment>`.

## Effective configuration

```bash
pio run -e xiao_esp32s3 -t menuconfig
rg 'CONFIG_ESPTOOLPY_FLASHSIZE|CONFIG_SPIRAM' \
  .pio/build/xiao_esp32s3/config/sdkconfig.h
```

Move durable choices from generated configuration into the appropriate defaults file, then clean and rebuild.

## Pinned baseline

The root environment pins `platform = espressif32@7.0.1`, whose official integration contains ESP-IDF 6.0.1. A newer standalone ESP-IDF patch must not be substituted without validating PlatformIO build scripts, board manifests, tools, all three targets, and the documentation.

`project-baseline.json` is the machine-readable source for PlatformIO Core, Espressif 32, ESP-IDF, MkDocs, Material, and mkdocs-static-i18n versions. The offline validator checks that config, requirements, release notes, and both languages agree.

## Component dependencies

Pin Registry packages in `idf_component.yml` and record their license and supported targets. Use a bounded ESP-IDF requirement such as `>=6.0,<6.1`, not an unbounded wildcard.

Secrets belong in provisioning, NVS, the CI secret store, or untracked local configuration. Do not commit serial ports, proxy settings, `.pio/`, `.venv/`, `site/`, or generated `sdkconfig` files.
