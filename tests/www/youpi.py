import sys
import os
print("Status: 200 OK\r\n")
print("Content-Type: text/plain; charset=utf-8\r\n\r\n")
print("hello python cgi!")

print(os.environ)
for line in sys.stdin:
    print(line)