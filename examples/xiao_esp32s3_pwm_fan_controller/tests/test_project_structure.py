from __future__ import annotations

import importlib.util
import unittest
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
SPEC = importlib.util.spec_from_file_location(
    "validate_project", ROOT / "scripts/validate_project.py"
)
MODULE = importlib.util.module_from_spec(SPEC)
assert SPEC.loader is not None
SPEC.loader.exec_module(MODULE)


class ProjectStructureTests(unittest.TestCase):
    def test_project_is_structurally_valid(self) -> None:
        self.assertEqual(MODULE.validate(ROOT), [])

    def test_active_profile_is_s3_ui_demo_only(self) -> None:
        root_cmake = (ROOT / "CMakeLists.txt").read_text(encoding="utf-8")
        main_cmake = (ROOT / "main/CMakeLists.txt").read_text(encoding="utf-8")
        self.assertIn('set(IDF_TARGET "esp32s3"', root_cmake)
        self.assertIn('SRCS "ui_demo_main.c"', main_cmake)
        self.assertNotIn("fan_control", main_cmake)
        self.assertNotIn("esp_matter", main_cmake)

    def test_encoder_console_pin_map(self) -> None:
        pins = (ROOT / "components/board_support/include/board_pins.h").read_text(
            encoding="utf-8"
        )
        self.assertIn("BOARD_ENCODER_PUSH_GPIO GPIO_NUM_1", pins)
        self.assertIn("BOARD_ENCODER_A_GPIO GPIO_NUM_2", pins)
        self.assertIn("BOARD_ENCODER_B_GPIO GPIO_NUM_3", pins)
        self.assertIn("BOARD_CONFIRM_GPIO GPIO_NUM_44", pins)
        self.assertIn("BOARD_BACK_GPIO GPIO_NUM_7", pins)
        self.assertIn("BOARD_OLED_SDA_GPIO GPIO_NUM_9", pins)
        self.assertIn("BOARD_OLED_SCL_GPIO GPIO_NUM_8", pins)
        self.assertIn("BOARD_FAN_PWM_GPIO GPIO_NUM_4", pins)
        self.assertIn("BOARD_FAN_TACH_GPIO GPIO_NUM_5", pins)

    def test_board_support_is_validation_only(self) -> None:
        source = (ROOT / "components/board_support/board_support.c").read_text(
            encoding="utf-8"
        )
        self.assertNotIn("gpio_config", source)
        self.assertNotIn("gpio_set_level", source)
        self.assertIn("CHIP_ESP32S3", source)

    def test_interaction_uses_official_components(self) -> None:
        source = (
            ROOT / "components/interaction_demo/interaction_demo.c"
        ).read_text(encoding="utf-8")
        self.assertIn("iot_knob_create", source)
        self.assertIn("iot_button_new_gpio_device", source)
        self.assertIn("ledc_timer_config", source)
        self.assertIn("ledc_channel_config", source)
        self.assertIn("fan_pwm_apply(running, target)", source)
        self.assertIn("fan_pwm_gpio_high_duty", source)
        self.assertIn("ledc_stop(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, 1U)", source)
        self.assertIn("gpio_set_level(BOARD_FAN_PWM_GPIO, 1)", source)
        self.assertIn(".duty = FAN_PWM_MAX_DUTY", source)
        self.assertIn(".flags = {.output_invert = 0}", source)
        self.assertIn("FAN_PWM_FREQUENCY_HZ = 25000", source)
        self.assertIn("FAN_TACH_PULSES_PER_REVOLUTION = 2", source)
        self.assertIn("FAN_TACH_SAMPLE_MS = 1000", source)
        self.assertIn("pcnt_new_unit", source)
        self.assertIn("pcnt_unit_set_glitch_filter", source)
        self.assertIn("pcnt_channel_set_edge_action", source)
        self.assertIn("pcnt_unit_get_count", source)
        self.assertIn("fan_logic_rpm", source)
        self.assertIn('lv_label_set_text_fmt(s_rpm_label, "RPM %" PRIu32, rpm)', source)
        start = source.index("esp_err_t interaction_demo_start(void)")
        startup_source = source[start:]
        self.assertLess(
            startup_source.index("init_fan_pwm()"),
            startup_source.index("init_display()"),
        )
        self.assertLess(
            startup_source.index("init_fan_tach()"),
            startup_source.index("heartbeat_task"),
        )
        self.assertIn("esp_lcd_new_panel_sh1106", source)
        self.assertIn("lvgl_port_add_disp", source)
        self.assertIn("lv_anim_path_linear", source)
        self.assertIn("lv_anim_path_ease_out", source)
        self.assertIn("INPUT_EVENT_PAGE_PREVIOUS", source)
        self.assertIn("INPUT_EVENT_PAGE_NEXT", source)
        self.assertIn('create_page_header(input_page, "INPUT 2/3")', source)
        self.assertIn('create_page_header(system_page, "SYSTEM 3/3")', source)
        self.assertIn('"OLED SH1106 3V3"', source)
        self.assertIn("lv_font_unscii_8", source)
        self.assertIn("lv_font_unscii_16", source)
        self.assertNotIn("lv_obj_set_style_transform_scale", source)
        self.assertNotIn("lv_spinner_create", source)
        self.assertIn("iot_knob_get_count_value", source)
        self.assertNotIn("gpio_diagnostic_task", source)
        self.assertNotIn("esp_lcd_panel_invert_color", source)
        self.assertIn("lv_obj_set_style_bg_color(screen, lv_color_white()", source)
        self.assertIn("back_button_callback", source)
        self.assertIn("confirm_button_callback", source)
        self.assertNotIn("reserved_button_callback", source)

    def test_custom_input_and_renderer_are_removed(self) -> None:
        for relative in (
            "components/interaction_demo/interaction_logic.c",
            "components/interaction_demo/ui_renderer.c",
            "scripts/render_ui_preview.py",
            "tests/test_interaction_demo.py",
        ):
            self.assertFalse((ROOT / relative).exists(), relative)


if __name__ == "__main__":
    unittest.main()
