#!/usr/bin/python
import socket
import sys
import os
import time
import signal

server_address = '/opt/entropia/usblockd-compat-layer.sock'

try:
    os.unlink(server_address)
except OSError:
    if os.path.exists(server_address):
        raise

sock = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
sock.bind(server_address)
sock.listen(0)

pid = int(open("/var/run/usblockd.pid").read())

while True:
    connection, client_address = sock.accept()
    try:
        while True:
            data = connection.recv(1)
            if data:
                if data == 'O':
                    os.kill(pid, signal.SIGUSR2)
                elif data == 'P':
                    os.kill(pid, signal.SIGHUP)
                elif data == 'C':
                    os.kill(pid, signal.SIGUSR1)

            break
    finally:
        # Clean up the connection
        connection.close()