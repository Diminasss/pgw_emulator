import requests

imsi = "250123456"
# Используем переменную imsi в запросе
response = requests.get(f'http://localhost:8080/check_subscriber?imsi={imsi}')
print(response.text)