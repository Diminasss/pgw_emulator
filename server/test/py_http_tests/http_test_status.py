import requests
import subprocess
import time
import unittest
from typing import List

# Путь к скомпилированному приложению сервера
server_application_path: str = "../../../cmake-build-debug/server/server"
# Путь к скомпилированному клиенту
client_application_path: str = "../../../cmake-build-debug/client/client"

class TestStatus(unittest.TestCase):
    process: subprocess.Popen

    @classmethod
    def setUpClass(cls) -> None:
        cls.process = subprocess.Popen([server_application_path])
        time.sleep(3)

    @classmethod
    def tearDownClass(cls) -> None:
        cls.process.terminate()
        try:
            cls.process.wait(timeout=5)
        except subprocess.TimeoutExpired:
            cls.process.kill()

    def test_1_status_zero(self) -> None:
        response: requests.Response = requests.get('http://localhost:8080/status')
        self.assertEqual(response.status_code, 200)
        self.assertEqual(response.text, "Status: running\nActive sessions: 0\n")

    def test_2_status_two(self) -> None:
        imsis: List[str] = ["123456789", "12345678910"]

        for imsi in imsis:
            process: subprocess.Popen = subprocess.Popen([client_application_path, imsi])
            time.sleep(1)
            process.terminate()
            try:
                process.wait(timeout=5)
            except subprocess.TimeoutExpired:
                process.kill()

        response: requests.Response = requests.get('http://localhost:8080/status')
        self.assertEqual(response.status_code, 200)
        self.assertEqual(response.text, "Status: running\nActive sessions: 2\n")

if __name__ == '__main__':
    unittest.main()
