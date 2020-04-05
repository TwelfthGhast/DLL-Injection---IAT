#!/usr/bin/env python3

import socket
import threading 

HOST = "127.0.0.1"  # Standard loopback interface address (localhost)
PORT = 1337         # Port to listen on (non-privileged ports are > 1023)
BUFFER_LEN = 1024

def socket_thread(conn, addr):
    print("Connected by", addr)
    while True:
        data = conn.recv(BUFFER_LEN)
        if not data:
            break
        print(data)

with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
    s.bind((HOST, PORT))
    s.listen()
    while True:
        conn, addr = s.accept()
        x = threading.Thread(target=socket_thread, args=(conn, addr))
        x.start()
            
