#!/usr/bin/env python3

import sys
import os
import sys
from time import sleep

def eprint(*args, **kwargs):
    print(*args, file=sys.stderr, **kwargs)

# Correct CGI header
print("Content-Type: text/html")
print()

i = 0
while True:
    sleep(1)
    eprint("cgi count: ", i)
    i += 1
