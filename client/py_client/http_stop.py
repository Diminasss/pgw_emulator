from test_config import TestConfig
import requests


if __name__ == "__main__":
    response: requests.Response  = requests.post(f'http://{TestConfig.ip}:{TestConfig.port}/stop')
    if response.status_code == 200:
        print(response.text)
    else:
        print("Что-то пошло не так", response.status_code)
