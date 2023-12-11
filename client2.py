#! /usr/bin/env python3
import concurrent.futures, socket, struct, argparse, threading, time

HOST = "127.0.0.1"
PORT = 58351

def main(nomifile):
    with concurrent.futures.ThreadPoolExecutor(max_workers=5) as executor:
        for nomefile in nomifile:
            executor.submit(body(nomefile))


def body(nomefile):
    with open(nomefile, "r") as f:
        with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
            s.connect((HOST,PORT))
            connection_type = 1
            s.sendall(struct.pack("<i",connection_type)) # Comunica connessione di tipo B
            for line in f:
                encoded_line = str.encode(line)
                length_bytes = len(encoded_line)
                s.sendall(struct.pack("<i",length_bytes))
                s.sendall(encoded_line)
            s.sendall(struct.pack("<i",0)) # Comunica che ha terminato
            s.shutdown(socket.SHUT_RDWR)



parser = argparse.ArgumentParser()
parser.add_argument("nomifile", nargs="+", help="nomi dei file da leggere")
args = parser.parse_args()
main(args.nomifile)