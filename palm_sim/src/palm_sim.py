"""Takeoff-hover-land for one CF. Useful to validate hardware config."""
import sys
#sys.path.insert(0, '/home/robert/crazyswarm/ros_ws/src/crazyswarm/scripts')
from pycrazyswarm import Crazyswarm
import asyncore

SWARM = None
BUFFER_SIZE = 8192
PALM_CORE_PROXY_PORT = 8888

TAKEOFF_DURATION = 2.5
HOVER_DURATION = 5.0

class PalmCoreHandler(asyncore.dispatcher_with_send):

    def handle_read(self):
        data = self.recv(BUFFER_SIZE)
        string_data = str(data)
        print("Received data: ", string_data)
        if len(string_data) == 1 and string_data == "R":
            # Request for a current state
            self.send("CURRENT STATE")
        else:
            # Otherwise, assume it is a command set of new positions
            robots = string_data.split('|')
            for robot_data, i in zip(robots, range(len(robots))):
                position = robot_data.split(',')
                cf = SWARM.allcfs.crazyflies[i]
                cf.cmdPosition(position)

class PalmCoreSimProxyServer(asyncore.dispatcher):

    def __init__(self, host, port):
        asyncore.dispatcher.__init__(self)
        SWARM = Crazyswarm()
        self.create_socket()
        self.set_reuse_addr()
        self.bind((host, port))
        self.listen(5)

    def handle_accepted(self, sock, addr):
        print('Incoming connection from %s' % repr(addr))
        handler = PalmCoreHandler(sock)


def main():
    proxy_server = PalmCoreSimProxyServer('localhost', PALM_CORE_PROXY_PORT)
    print("[palm_sim] Proxy server initiated, waiting for incoming connections.")
    asyncore.loop()


if __name__ == "__main__":
    main()