#! /usr/bin/env python3
import socket, struct, argparse

HOST = "127.0.0.1"
PORT = 58351

def main(nomefile):
    print(f"Mi collego a {HOST} sulla porta {PORT}")
    with open(nomefile, "r") as f:
        for line in f:
            with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
                s.connect((HOST, PORT))
                connection_type = 0
                s.sendall(struct.pack("<i",connection_type)) # Comunica che è una connessione di tipo A
                encoded_line = str.encode(line)
                length_bytes = len(encoded_line)
                s.sendall(struct.pack("<i",length_bytes))
                s.sendall(encoded_line)
                s.shutdown(socket.SHUT_RDWR)



parser = argparse.ArgumentParser()
parser.add_argument("nomefile", help="nome del file da leggere")
args = parser.parse_args()
main(args.nomefile)

