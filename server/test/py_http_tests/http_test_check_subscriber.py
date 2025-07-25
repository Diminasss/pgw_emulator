import requests
import subprocess
import time
import unittest


class TestExample(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        # Путь к скомпилированному приложению сервера
        server_application_path: str = "../../../cmake-build-debug/server/server"
        cls.process = subprocess.Popen([server_application_path])
        time.sleep(3)

    @classmethod
    def tearDownClass(cls):
        cls.process.terminate()

    def test_1_not_active_imsi(self):
        imsi = "123456789"
        response = requests.get(f'http://localhost:8080/check_subscriber?imsi={imsi}')
        self.assertEqual(response.status_code, 200)
        self.assertEqual(response.text, "not active")

    def test_2_active_imsi(self):
        imsi = "123456789"
        # Путь к скомпилированному клиенту
        client_application_path = "../../../cmake-build-debug/client/client"
        process = subprocess.Popen([client_application_path, imsi])
        time.sleep(1)
        process.terminate()
        try:
            process.wait(timeout=5)
        except subprocess.TimeoutExpired:
            process.kill()

        response = requests.get(f'http://localhost:8080/check_subscriber?imsi={imsi}')
        self.assertEqual(response.status_code, 200)
        self.assertEqual(response.text, "active")


if __name__ == '__main__':
    unittest.main()