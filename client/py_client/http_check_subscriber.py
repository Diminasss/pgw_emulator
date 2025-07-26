from test_config import TestConfig
import requests


if __name__ == "__main__":
    imsi:str="123456789"
    response: requests.Response = requests.get(f'http://{TestConfig.ip}:{TestConfig.port}/check_subscriber?imsi={imsi}')
    if response.status_code == 200:
        print(response.text)
    else:
        print("Что-то пошло не так", response.status_code)