#!/usr/bin/env python3

import socket
import threading
import time

HOST = "127.0.0.1"  # Standard loopback interface address (localhost)
LISTEN_PORT = 1337         # Port to listen on (non-privileged ports are > 1023)
OUT_PORT = 6969            # Port to send commands to
BUFFER_LEN = 1024

class mySocket:
    all_sockets = []
    def __init__(self, conn, addr):
        print("Connected by", addr)
        self.conn = conn
        self.addr = addr
        mySocket.all_sockets.append(self)
    
    def send(self, message):
        self.conn.send(message.encode())

def init_socket_out_listen():
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        s.bind((HOST, OUT_PORT))
        s.listen()
        while True:
            conn, addr = s.accept()
            mySocket(conn, addr)

def init_socket_in_listen():
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        s.bind((HOST, LISTEN_PORT))
        s.listen()
        while True:
            conn, addr = s.accept()
            x = threading.Thread(target=socket_listen, args=(conn,))
            x.start()
            
def socket_listen(conn):
    while True:
        recieved = conn.recv(BUFFER_LEN)
        if len(recieved) != 0:
            print(recieved.decode(), end="")
        else:
            time.sleep(1)
            
x = threading.Thread(target=init_socket_out_listen, args=())
x.start()
y = threading.Thread(target=init_socket_in_listen, args=())
y.start()
while True:
    cmd = input("> Enter command:\n")
    if cmd == "ls":
        for sock in mySocket.all_sockets:
            # Clean up list
            try:
                sock.send("ping")
                print(f"\t{sock.addr}")
            except WindowsError as e:
                mySocket.all_sockets.remove(sock)
            except Exception as e:
                print(e)
    elif cmd.startswith("send"):
        ls = cmd.split()
        if len(ls) > 2:
            try:
                port = int(ls[1])
                for sock in mySocket.all_sockets:
                    if sock.addr[1] == port:
                        print(f"Sending '{' '.join(ls[2:])}' to connection on port {port}")
                        sock.send(" ".join(ls[2:]))
                        break
            # Broken socket
            except WindowsError as e:
                try:
                    port = int(ls[1])
                    for sock in mySocket.all_sockets:
                        if sock.addr[1] == port:
                            mySocket.all_sockets.remove(sock)
                            break
                except Exception as f:
                    print(e)
                    print(f)
            except Exception as e:
                print(e)
                pass
    time.sleep(1)