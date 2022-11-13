"""Takeoff-hover-land for one CF. Useful to validate hardware config."""
import sys
#sys.path.insert(0, '/home/robert/crazyswarm/ros_ws/src/crazyswarm/scripts')
from pycrazyswarm import Crazyswarm
import asyncore
from threading import Thread, Event
from time import sleep
import random

SWARM = None
BUFFER_SIZE = 8192
PALM_CORE_PROXY_PORT = 8888
UPDATE_HZ = 60.0
UPDATE_SLEEP_TIME = 1.0 / UPDATE_HZ
CANCEL_EVENT = Event()


TAKEOFF_DURATION = 2.5
HOVER_DURATION = 5.0

def listen(cf_positions):

    class PalmCoreHandler(asyncore.dispatcher_with_send):

        def handle_read(self):
            data = self.recv(BUFFER_SIZE)
            string_data = str(data)
            print("[palm_sim] Received data: ", string_data)

            if len(string_data) == 1 and string_data == "R":
                # Request for a current state
                self.send("CURRENT STATE")
            else:
                # Otherwise, assume it is a command set of new positions
                robots = string_data.split('|')
                for robot_data, i in zip(robots, range(len(robots))):
                    position = list(map(int, robot_data.split(',')))
                    cf_positions[i] = position

    class PalmCoreSimProxyServer(asyncore.dispatcher):

        def __init__(self, host, port):
            asyncore.dispatcher.__init__(self)
            self.create_socket()
            self.set_reuse_addr()
            self.bind((host, port))
            self.listen(5)

        def handle_accepted(self, sock, addr):
            print('Incoming connection from %s' % repr(addr))
            handler = PalmCoreHandler(sock)

    proxy_server = PalmCoreSimProxyServer('localhost', PALM_CORE_PROXY_PORT)
    print("[palm_sim] Proxy server initiated, waiting for incoming connections.")
    asyncore.loop()
    
    #while True:
    #    sleep(UPDATE_SLEEP_TIME)
    #    positions = []
    #    for i in range(2):
    #        cf_positions[i] = (abs(random.gauss(0, 1)), abs(random.gauss(0, 1)), abs(random.gauss(0, 1)))

def main():
    SWARM = Crazyswarm()
    timeHelper = SWARM.timeHelper

    CF_POSITIONS = [(1,1,1),(2,2,2)]

    listen_thread = Thread(target=listen, args=(CF_POSITIONS,))
    listen_thread.start()
    
    for cf in SWARM.allcfs.crazyflies:
        cf.takeoff(1, TAKEOFF_DURATION)

    while True:
        for position, i in zip(CF_POSITIONS, range(len(CF_POSITIONS))):
            cf = SWARM.allcfs.crazyflies[i]
            cf.cmdPosition(position)

        timeHelper.sleep(UPDATE_SLEEP_TIME)
        


    print("Press any key to stop simulation...")

    input();
    
    timeHelper.sleep(TAKEOFF_DURATION + HOVER_DURATION)
    

if __name__ == "__main__":
    main()