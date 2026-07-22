from __future__ import annotations

from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest


class NtcThermistorHostTests(unittest.TestCase):
    def test_beta_model_and_divider_boundaries(self) -> None:
        compiler = shutil.which("cc")
        if compiler is None:
            self.skipTest("host C compiler is unavailable")

        project = Path(__file__).resolve().parents[1]
        with tempfile.TemporaryDirectory() as directory:
            executable = Path(directory) / "ntc_thermistor_test"
            subprocess.run(
                [
                    compiler,
                    "-std=c11",
                    "-Wall",
                    "-Wextra",
                    "-Werror",
                    str(project / "components/ntc_thermistor/ntc_thermistor.c"),
                    str(project / "tests/ntc_thermistor_host_test.c"),
                    "-I",
                    str(project / "components/ntc_thermistor/include"),
                    "-lm",
                    "-o",
                    str(executable),
                ],
                check=True,
            )
            subprocess.run([str(executable)], check=True)


if __name__ == "__main__":
    unittest.main()
