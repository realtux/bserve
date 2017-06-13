import sys
import json
import socket

s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.connect(('127.0.0.1', 8383))
s.send('1\n')
data = s.recv(999999)
s.close()

data = json.loads(data);
print '<h1>' + data['msg'] + '</h1><h2>cool!</h2>'
