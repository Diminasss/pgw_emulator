import requests


if __name__ == "__main__":
    imsi:str="123456789"
    response: requests.Response = requests.get(f'http://localhost:8080/check_subscriber?imsi={imsi}')
    if response.status_code == 200:
        print(response.text)
    else:
        print("Что-то пошло не так", response.status_code)