import requests

response = requests.get('http://localhost:8080/status')
print(response.text)