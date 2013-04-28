import pty, termios, os, grp, time, select, socket, atexit




def handshake(fd, conn):
    quitCheck = ""
    resp = ""
    while(True):
        quitcheck = nbread(conn, 10, 0.001)
        if(quitCheck == "stop_init"):
            return 0

        master_write(fd, "n")
        time.sleep(0.1)
        resp += nbread(fd, 2, 0.001)
        if resp is not "":
            print "resp=",resp
        if resp=="bt":
            master_write(fd, "bt") 
            return 1 
        
def master_write(fd, msg):
    os.write(fd, msg)
    nbread(fd, len(msg), 0.001)
        
        
#non blocking read
def nbread(fd, length, timeout):
    ret = select.select([fd], [], [], timeout)
    if(len(ret[0]) > 0 ): 
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
    print "quitting..."
    os.remove(path);

def closeSocket(sock):
        sock.shutdown(socket.SHUT_RDWR);
        sock.close();

#main
def main():
    #open pseudo-terminal using pty module because it is more stable
    master_fd, slave_fd = pty.openpty();

    #link it to /dev/ttyUSB0 and change permissions so BWT software
    #can access it
    paths = ["/dev/ttyUSB0", "/dev/ttyUSB1", "/dev/ttyUSB2", "/dev/ttyUSB3", "/dev/ttyUSB4", "/dev/ttyUSB5", "/dev/ttyUSB6", "/dev/ttyUSB7", ]
    for serialPath in paths:
        try:
            os.symlink(os.ttyname(slave_fd), serialPath);
            os.fchmod(slave_fd, 0660);
            gid = grp.getgrnam("dialout").gr_gid;
            os.chown(serialPath, -1, gid);
            atexit.register(deletePath, serialPath)
            print "emulator connected to ", serialPath
            break;
        except OSError as e:
            print serialPath + " already exists, trying the next one..." 

    #open a socket
    HOST = ''
    PORT = 8081
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.bind((HOST, PORT))
    s.listen(1)
    conn, addr = s.accept()

    #register exit handler
    atexit.register(closeSocket, conn)
    print "Connected to ", addr
    print "in write loop"

    while(True):
      #listen for byte code - block here
      bytecode = conn.recv(16)
      if(bytecode==''):  #client has disconnected
        exit()
      print "bytecode=", bytecode
      if(bytecode == "_init"):
        ret = handshake(master_fd, conn)
        print "handshake complete"
        conn.send(chr(ret))
        continue

      if(bytecode == "quit"):
        exit()
        
      #TODO implement this.  For now just catch it to not crash btbt
      if(bytecode == "uninit"):
        continue

      #send bytecode
      master_write(master_fd, bytecode)
      time.sleep(0.1);

      #non blocking read
      response = nbread(master_fd, 10, 0.001)
      if response is not "":
        print "hardware response=",response


if __name__ == "__main__":
    main()
