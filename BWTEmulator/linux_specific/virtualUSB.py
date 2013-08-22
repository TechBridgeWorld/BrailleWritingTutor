import pty, termios, os, grp, time, select, socket, atexit

def handshake(fd, conn):
    resp = ""
    while(True):
        master_write(fd, "n")
        time.sleep(0.1)
        resp += nbread(fd, 2, 0.001)
        if resp is not "":
            print "resp=",resp
        if "bt" in resp:
            master_write(fd, "bt")
            return 1
        #TODO This case is here because sometimes the
        #     the write is not spit back to the master
        #     in which case master_write() swallows up
        #     the 'b'
        if "t" in resp or "b" in resp:
            master_write(fd, "bt")
            return 1

#for some reason, when writing to a master, the output
#gets spit back to the master for reading
#TODO find out why this happens and find a more robust
#     solution
def master_write(fd, msg):
    os.write(fd, msg)
    nbread(fd, len(msg), 0.001)


#non blocking read
def nbread(fd, length, timeout):
    #select.select checks if an i/o operation can
    #be performed on fd
    ret = select.select([fd], [], [], timeout)
    if(len(ret[0]) > 0 ):
        #there is something to read
        if type(fd) is int:
            response = os.read(fd, length)
            return response
        if type(fd) is socket._socketobject:
            response = fd.recv(length)
            return response
    else:
        return ""


#exit handlers
def deletePath(path):
    print "deleting ", path
    os.remove(path);

def closeSocket(sock):
    print "closing ", sock
    sock.shutdown(socket.SHUT_RDWR);
    sock.close();

#main
def main():
    #open pseudo-terminal using pty module because it is more stable
    master_fd, slave_fd = pty.openpty();

    #link it to a /dev/ttyUSB port that is available and change
    #permissions so BWT software can access it
    paths = ["/dev/ttyUSB1", "/dev/ttyUSB2", "/dev/ttyUSB3", "/dev/ttyUSB4", "/dev/ttyUSB5", "/dev/ttyUSB6", "/dev/ttyUSB7", ]
    for serialPath in paths:
        try:
            #symlink the pseudoterminal slave to /dev/ttyUSBN
            os.symlink(os.ttyname(slave_fd), serialPath);
            #change permissions and group so that btbt software
            #can access it
            os.fchmod(slave_fd, 0666);
            #gid = grp.getgrnam("dialout").gr_gid;
            #os.chown(serialPath, -1, gid);
            #register the exit handler to remove /dev/ttyUSBN
            atexit.register(deletePath, serialPath)
            print "emulator connected to ", serialPath
            break;
        except OSError as e:
            print serialPath + " already exists, trying the next one..."

    #open a socket
    HOST = '' #localhost
    PORT = 8081
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

    #set a timeout on the socket so that if the server disconnects
    #during handshake we can still exit gracefully
    s.settimeout(0.01);
    s.bind((HOST, PORT))
    s.listen(1)

    #register exit handler to close the socket
    atexit.register(closeSocket, s);

    #connect in a while loop because of the timeout
    conn = None;
    while(conn is None):
        try:
            conn, addr = s.accept()
        except socket.timeout as e:
            continue

    #register exit handler to close this socket
    atexit.register(closeSocket, conn)
    print "Connected to ", addr
    print "in write loop"

    while(True):
        #listen for byte code - block here
        bytecode = conn.recv(16)
        if(bytecode == ''):  #client has disconnected
            exit()
        print "bytecode=", bytecode
        if(bytecode == "_init"):
            ret = handshake(master_fd, conn)
            print "handshake complete"
            conn.send(chr(ret))
            continue

        if(bytecode == "quit"):
            exit()

        #send bytecode
        master_write(master_fd, bytecode)
        time.sleep(0.1);

        #non blocking read
        response = nbread(master_fd, 10, 0.001)
        if response is not "":
            print "hardware response=",response


if __name__ == "__main__":
    main()
