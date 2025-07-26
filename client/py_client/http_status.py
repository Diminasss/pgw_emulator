import requests


if __name__ == '__main__':
    response: requests.Response = requests.get('http://localhost:8080/status')
    if response.status_code == 200:
        print(response.text)
    else:
        print('Что-то пошло не так', response.status_code)

