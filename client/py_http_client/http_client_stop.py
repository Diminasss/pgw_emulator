import requests

response = requests.post('http://localhost:8080/stop')
print(response.text)