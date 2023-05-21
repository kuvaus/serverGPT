import requests
import json

url = 'http://localhost:5891/v1/completions'

# Set the request headers and payload
headers = {'Content-Type': 'application/json'}
payload = {
  'model': 'gpt4all-j-v1.3-groovy',
  'prompt': 'Who is Michael Jordan?',
  'max_tokens': 50,
  'temperature': 0.28,
  'top_p': 0.95,
  'n': 1,
  'echo': True,
  'stream': False
}

# Send the POST request and print the response
response = requests.post(url, headers=headers, data=json.dumps(payload))
print(response.json())

