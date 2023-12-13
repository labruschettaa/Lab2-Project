#! /usr/bin/env python3

import sys, socket, argparse, os, struct, signal
import concurrent.futures, subprocess, logging

HOST = "127.0.0.1"
PORT = 58351

CONNECTION_TYPE_A = 0
CONNECTION_TYPE_B = 1
MIN_SIZE_RECV = 4

logging.basicConfig(filename=os.path.basename(sys.argv[0])[:-3]+'.log',level=logging.DEBUG,
                                              datefmt='%d/%m/%y %H:%M:%S', format='%(asctime)s - %(levelname)s - %(message)s')


def main(t,capo_let,capo_s,p):
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        try:
            s.bind((HOST, PORT))
            s.listen()
            with concurrent.futures.ThreadPoolExecutor(max_workers=int(args.t)) as executor:
                while True:
                    conn, addr = s.accept()
                    executor.submit(body(conn,capo_let,capo_s))
        except KeyboardInterrupt:
            pass
        s.shutdown(socket.SHUT_RDWR)
        os.kill(os.getpgid(p.pid),signal.SIGTERM)
        os.close(capo_let)
        os.close(capo_s)
        os.wait()
        os.unlink('capolet')
        os.unlink('caposc')
        

def body(conn,capo_let,capo_s):
    with conn:
        data = recv_all(conn,4)
        read = struct.unpack("<i",data)[0]
        if read == CONNECTION_TYPE_A:
            connection_type_A(conn,capo_let)
        elif read == CONNECTION_TYPE_B:
            connection_type_B(conn,capo_s)
        else:
            raise RuntimeError("Communication error")


def connection_type_A(conn,pipe):
    length = recv_all(conn, MIN_SIZE_RECV)
    os.write(pipe,length)
    length = struct.unpack("<i",length)[0]
    data = conn.recv(length)
    os.write(pipe,data)
    logging.debug(f"Connessione di tipo A - Byte scritti sulla pipe: {MIN_SIZE_RECV + length}.")


def connection_type_B(conn,pipe):
    total_bytes = 0
    while True:
        length_b = recv_all(conn, MIN_SIZE_RECV)
        length = struct.unpack("<i",length_b)[0]
        if length == 0:
            break
        os.write(pipe,length_b)
        data = conn.recv(length) 
        total_bytes = MIN_SIZE_RECV + total_bytes + length
        os.write(pipe,data)
    logging.debug(f"Connessione di tipo B - Byte scritti sulla pipe: {total_bytes}.")


def recv_all(conn,n):
    chunks = b''
    bytes_recd = 0
    while bytes_recd < n:
        chunk = conn.recv(min(n - bytes_recd, 1024))
        if len(chunk)==0:
            raise RuntimeError("Socket connection broken")
        chunks += chunk
        bytes_recd = bytes_recd + len(chunk)
        assert bytes_recd == len(chunks)
    return chunks


parser = argparse.ArgumentParser()
parser.add_argument("t", help="t numero di thread del server")
parser.add_argument("-r",help="-r flag num thread lettori", default=3)
parser.add_argument("-w", help="-w flag num thread scrittori", default=3)
parser.add_argument("-v", help="chiama archivio.c mediante valgrind", action="store_true")
args = parser.parse_args()
assert int(args.t)>0, "Il numero di thread server deve essere positivo"
assert int(args.r)>0, "Il numero di thread lettori deve essere positivo"
assert int(args.w)>0, "Il numero di thread scrittori deve essere positivo"
# Se il flag -v è stato specificato esegue archivio.c con valgrind
try:
    os.mkfifo('caposc')
    os.mkfifo('capolet')
except: 
    pass

with open ('log_errors.txt', 'w') as f:
    if args.v:
        p = subprocess.Popen(["valgrind","--leak-check=full", 
                            "--show-leak-kinds=all", 
                            "--log-file=valgrind-%p.log",
                            "./archivio", str(args.r), str(args.w)], stderr=f, start_new_session=True)
    # Il flag -v non è stato specificato, esegue archivio.c normalmente
    else:
        p = subprocess.Popen(["./archivio", str(args.r), str(args.w)], stderr=f, start_new_session=True)

try:
    capo_s = os.open('caposc', os.O_WRONLY)
    capo_let = os.open('capolet', os.O_WRONLY)
except Exception as e:
    print(e)

main(args.t,capo_let,capo_s,p)



