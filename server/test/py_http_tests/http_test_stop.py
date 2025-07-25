import requests
import subprocess
import time
import unittest

# Путь к скомпилированному приложению сервера
server_application_path: str = "../../../cmake-build-debug/server/server"

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
        response: requests.Response  = requests.post('http://localhost:8080/stop')
        time.sleep(5)
        self.assertEqual(response.status_code, 200)
        self.assertEqual(response.text, "Graceful shutdown initiated")
        self.assertNotEqual(self.__class__.process.poll(), None)

if __name__ == '__main__':
    unittest.main()
