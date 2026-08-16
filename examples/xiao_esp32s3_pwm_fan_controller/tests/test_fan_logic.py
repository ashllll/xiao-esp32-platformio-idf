from __future__ import annotations

import ctypes
import subprocess
import tempfile
import unittest
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]


class LogicState(ctypes.Structure):
    _fields_ = [
        ("no_tach_ms", ctypes.c_uint32),
        ("cutoff_remaining_ms", ctypes.c_uint32),
        ("fault_latched", ctypes.c_bool),
    ]


class LogicOutput(ctypes.Structure):
    _fields_ = [
        ("drive_power", ctypes.c_bool),
        ("applied_percent", ctypes.c_uint8),
        ("newly_faulted", ctypes.c_bool),
    ]


class FanLogicTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        cls.temp = tempfile.TemporaryDirectory()
        library = Path(cls.temp.name) / "libfan_logic.dylib"
        source = ROOT / "components/fan_logic/fan_logic.c"
        include = ROOT / "components/fan_logic/include"
        subprocess.run(
            ["cc", "-std=c11", "-shared", "-fPIC", f"-I{include}", str(source), "-o", str(library)],
            check=True,
        )
        cls.lib = ctypes.CDLL(str(library))
        cls.lib.fan_logic_ledc_duty.argtypes = [ctypes.c_uint8, ctypes.c_uint32]
        cls.lib.fan_logic_ledc_duty.restype = ctypes.c_uint32
        cls.lib.fan_logic_rpm.argtypes = [ctypes.c_uint32, ctypes.c_uint32, ctypes.c_uint32]
        cls.lib.fan_logic_rpm.restype = ctypes.c_uint32
        cls.lib.fan_logic_restore_requested.argtypes = [
            ctypes.c_bool, ctypes.c_uint8, ctypes.c_bool, ctypes.c_uint8,
            ctypes.POINTER(ctypes.c_bool), ctypes.POINTER(ctypes.c_uint8),
        ]
        cls.lib.fan_logic_init.argtypes = [ctypes.POINTER(LogicState)]
        cls.lib.fan_logic_clear_fault.argtypes = [ctypes.POINTER(LogicState)]
        cls.lib.fan_logic_step.argtypes = [
            ctypes.POINTER(LogicState), ctypes.c_bool, ctypes.c_uint8,
            ctypes.c_uint32, ctypes.c_uint32, ctypes.c_uint32,
            ctypes.c_uint32, ctypes.POINTER(LogicOutput),
        ]

    @classmethod
    def tearDownClass(cls) -> None:
        cls.temp.cleanup()

    def step(self, state: LogicState, pulses: int = 0, elapsed: int = 100) -> LogicOutput:
        output = LogicOutput()
        self.lib.fan_logic_step(
            ctypes.byref(state), True, 50, pulses, elapsed, 5000, 500,
            ctypes.byref(output),
        )
        return output

    def test_duty_mapping(self) -> None:
        self.assertEqual(self.lib.fan_logic_ledc_duty(0, 1023), 0)
        self.assertEqual(self.lib.fan_logic_ledc_duty(50, 1023), 512)
        self.assertEqual(self.lib.fan_logic_ledc_duty(100, 1023), 1023)
        self.assertEqual(self.lib.fan_logic_ledc_duty(255, 1023), 1023)

    def test_rpm_math(self) -> None:
        self.assertEqual(self.lib.fan_logic_rpm(100, 2, 1000), 3000)
        self.assertEqual(self.lib.fan_logic_rpm(1, 2, 1000), 30)
        self.assertEqual(self.lib.fan_logic_rpm(100, 0, 1000), 0)

    def test_nvs_restore_and_first_boot_defaults(self) -> None:
        power = ctypes.c_bool(True)
        percent = ctypes.c_uint8(99)
        self.lib.fan_logic_restore_requested(
            False, 0, False, 0, ctypes.byref(power), ctypes.byref(percent)
        )
        self.assertFalse(power.value)
        self.assertEqual(percent.value, 50)

        self.lib.fan_logic_restore_requested(
            True, 1, True, 73, ctypes.byref(power), ctypes.byref(percent)
        )
        self.assertTrue(power.value)
        self.assertEqual(percent.value, 73)

        self.lib.fan_logic_restore_requested(
            True, 1, True, 101, ctypes.byref(power), ctypes.byref(percent)
        )
        self.assertFalse(power.value)
        self.assertEqual(percent.value, 50)

    def test_tach_resets_stall_timer(self) -> None:
        state = LogicState()
        self.lib.fan_logic_init(ctypes.byref(state))
        for _ in range(49):
            output = self.step(state)
            self.assertFalse(output.newly_faulted)
        self.step(state, pulses=1)
        self.assertEqual(state.no_tach_ms, 0)
        self.assertFalse(state.fault_latched)

    def test_stall_zeroes_pwm_then_cuts_power(self) -> None:
        state = LogicState()
        self.lib.fan_logic_init(ctypes.byref(state))
        output = LogicOutput()
        for _ in range(50):
            output = self.step(state)
        self.assertTrue(output.newly_faulted)
        self.assertTrue(output.drive_power)
        self.assertEqual(output.applied_percent, 0)
        for _ in range(4):
            output = self.step(state)
            self.assertTrue(output.drive_power)
        output = self.step(state)
        self.assertFalse(output.drive_power)
        self.assertTrue(state.fault_latched)

        self.lib.fan_logic_clear_fault(ctypes.byref(state))
        output = self.step(state, pulses=1)
        self.assertTrue(output.drive_power)
        self.assertEqual(output.applied_percent, 50)


if __name__ == "__main__":
    unittest.main()
