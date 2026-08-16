#!/usr/bin/env python3
from __future__ import annotations

import argparse
import re
from pathlib import Path


REQUIRED = (
    "CMakeLists.txt", "platformio.ini", "idf-version.txt",
    "sdkconfig.defaults", "sdkconfig.xiao-esp32s3",
    "main/CMakeLists.txt", "main/ui_demo_main.c", "main/idf_component.yml",
    "components/board_support/board_support.c",
    "components/board_support/include/board_pins.h",
    "components/interaction_demo/CMakeLists.txt",
    "components/interaction_demo/interaction_demo.c",
    "components/interaction_demo/include/interaction_demo.h",
    "docs/ec10-display-demo.md", "dependencies.lock",
)


def validate(root: Path) -> list[str]:
    errors: list[str] = []
    for relative in REQUIRED:
        if not (root / relative).is_file():
            errors.append(f"missing required file: {relative}")

    if (root / "idf-version.txt").read_text(encoding="utf-8").strip() != "v6.0.1":
        errors.append("idf-version.txt must pin v6.0.1")

    platformio = (root / "platformio.ini").read_text(encoding="utf-8")
    for required in (
        "default_envs = xiao_esp32s3", "platform = espressif32@7.0.1",
        "board = seeed_xiao_esp32s3",
        "sdkconfig.defaults;sdkconfig.xiao-esp32s3",
    ):
        if required not in platformio:
            errors.append(f"platformio.ini missing: {required}")

    root_cmake = (root / "CMakeLists.txt").read_text(encoding="utf-8")
    if 'set(IDF_TARGET "esp32s3"' not in root_cmake:
        errors.append("root CMake must target esp32s3")

    main_cmake = (root / "main/CMakeLists.txt").read_text(encoding="utf-8")
    if 'SRCS "ui_demo_main.c"' not in main_cmake:
        errors.append("main component must build ui_demo_main.c")
    for dormant in ("fan_control", "esp_matter", "app_console.cpp", "main.cpp"):
        if dormant in main_cmake:
            errors.append(f"active main component must exclude: {dormant}")

    manifest = (root / "main/idf_component.yml").read_text(encoding="utf-8")
    if "esp_matter" in manifest or not re.search(
        r'idf:\s*">=6\.0\.0,<6\.1\.0"', manifest
    ):
        errors.append("active manifest must require only the ESP-IDF 6.0 line")
    for dependency in (
        'espressif/knob: "1.1.0"', 'espressif/button: "4.2.0"',
        'espressif/esp_lvgl_port: "2.9.0"', 'lvgl/lvgl: "9.5.0"',
        'tny-robotics/sh1106-esp-idf: "1.0.1"',
    ):
        if dependency not in manifest:
            errors.append(f"active manifest missing library: {dependency}")

    lock_file = (root / "dependencies.lock").read_text(encoding="utf-8")
    for dependency in ("espressif/button", "espressif/esp_lvgl_port",
                       "espressif/knob", "lvgl/lvgl",
                       "tny-robotics/sh1106-esp-idf"):
        if dependency not in lock_file:
            errors.append(f"dependency lock missing: {dependency}")

    s3_config = (root / "sdkconfig.xiao-esp32s3").read_text(encoding="utf-8")
    for required in (
        "CONFIG_ESPTOOLPY_FLASHSIZE_8MB=y", "CONFIG_SPIRAM=y",
        "CONFIG_SPIRAM_MODE_OCT=y",
    ):
        if required not in s3_config:
            errors.append(f"S3 sdkconfig missing: {required}")

    pins = (root / "components/board_support/include/board_pins.h").read_text(
        encoding="utf-8"
    )
    for mapping in (
        'BOARD_ID "seeed_xiao_esp32s3_sh1106_encoder_console"',
        "BOARD_OLED_SDA_GPIO GPIO_NUM_9", "BOARD_OLED_SCL_GPIO GPIO_NUM_8",
        "BOARD_OLED_I2C_ADDRESS 0x3CU",
        "BOARD_ENCODER_PUSH_GPIO GPIO_NUM_1",
        "BOARD_ENCODER_A_GPIO GPIO_NUM_2",
        "BOARD_ENCODER_B_GPIO GPIO_NUM_3",
        "BOARD_CONFIRM_GPIO GPIO_NUM_44", "BOARD_BACK_GPIO GPIO_NUM_7",
        "BOARD_FAN_PWM_GPIO GPIO_NUM_4",
    ):
        if mapping not in pins:
            errors.append(f"missing symbolic mapping: {mapping}")

    active_code = "\n".join(
        path.read_text(encoding="utf-8")
        for directory in (root / "main", root / "components/interaction_demo")
        for path in directory.rglob("*")
        if path.is_file() and path.suffix in {".c", ".h"}
    )
    for forbidden in ("BOARD_FAN_POWER", "esp_matter"):
        if forbidden in active_code:
            errors.append(f"active demo source unexpectedly contains: {forbidden}")

    interaction_source = (
        root / "components/interaction_demo/interaction_demo.c"
    ).read_text(encoding="utf-8")
    for required in (
        "iot_knob_create", "iot_button_new_gpio_device", "BUTTON_SINGLE_CLICK",
        "BUTTON_DOUBLE_CLICK", "lvgl_port_add_disp", "lv_anim_path_linear",
        "lv_anim_path_ease_out", "INPUT_EVENT_PAGE_PREVIOUS",
        "INPUT_EVENT_PAGE_NEXT", 'create_page_header(input_page, "INPUT 2/3")',
        'create_page_header(system_page, "SYSTEM 3/3")',
        "lv_font_unscii_8", "lv_font_unscii_16", "HEARTBEAT sequence=",
        "iot_knob_get_count_value",
        "esp_lcd_new_panel_sh1106",
        "lv_obj_set_style_bg_color(screen, lv_color_white()",
        ".mirror_x = false", ".mirror_y = true",
        "back_button_callback", "confirm_button_callback",
        "ledc_timer_config", "ledc_channel_config",
        "FAN_PWM_FREQUENCY_HZ = 25000",
        "fan_pwm_gpio_high_duty",
        "ledc_stop(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, 1U)",
        ".flags = {.output_invert = 0}",
        "fan_pwm_apply(running, target)",
        "pcnt_new_unit", "pcnt_unit_set_glitch_filter",
        "pcnt_channel_set_edge_action", "pcnt_unit_get_count",
        "BOARD_FAN_TACH_GPIO", "fan_logic_rpm",
    ):
        if required not in interaction_source:
            errors.append(f"library-backed demo missing: {required}")
    for forbidden in ("interaction_logic", "ui_render_frame",
                      "lv_obj_set_style_transform_scale",
                      "esp_lcd_panel_invert_color"):
        if forbidden in interaction_source:
            errors.append(f"custom implementation remains active: {forbidden}")

    for obsolete in (
        "components/interaction_demo/interaction_logic.c",
        "components/interaction_demo/ui_renderer.c",
        "scripts/render_ui_preview.py",
        "tests/test_interaction_demo.py",
        "artifacts/ec10-oled-preview.gif",
    ):
        if (root / obsolete).exists():
            errors.append(f"obsolete custom implementation remains: {obsolete}")

    main_source = (root / "main/ui_demo_main.c").read_text(encoding="utf-8")
    if "EC10_FAN_PWM_READY version=12 board=%s" not in main_source:
        errors.append("missing EC10 fan PWM runtime-ready marker")

    board_source = (root / "components/board_support/board_support.c").read_text(
        encoding="utf-8"
    )
    if "CHIP_ESP32S3" not in board_source or "esp_psram_get_size" not in board_source:
        errors.append("board runtime check must verify ESP32-S3 and PSRAM")
    if "gpio_config" in board_source or "gpio_set_level" in board_source:
        errors.append("board support must not initialize or drive GPIOs")

    docs = ((root / "README.md").read_text(encoding="utf-8") +
            (root / "docs/ec10-display-demo.md").read_text(encoding="utf-8"))
    for required in (
        "3V3", "不能接 5V", "RPM", "0x3C", "300ms",
        "不初始化独立的风扇电源切换",
        "espressif/knob", "espressif/button", "lv_anim_path_linear",
        "lv_anim_path_ease_out", "INPUT 2/3", "SYSTEM 3/3",
        "SH1106", "D10", "D9", "D8", "D7", "D4", "D3", "D0", "D1", "D2",
        "25kHz", "AOD4184L", "PCNT",
    ):
        if required not in docs:
            errors.append(f"demo documentation missing: {required}")
    return errors


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("root", nargs="?", default=".", type=Path)
    root = parser.parse_args().root.resolve()
    errors = validate(root)
    if errors:
        for error in errors:
            print(f"ERROR: {error}")
        return 1
    print(f"Project structure valid: {root}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
