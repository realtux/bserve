import sys
import json

data = json.loads(open(sys.argv[1]).read());
print '<h1>' + data['name'] + '</h1>'
