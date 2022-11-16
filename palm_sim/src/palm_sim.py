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
UPDATE_HZ = 10.0
UPDATE_SLEEP_TIME = 1.0 / UPDATE_HZ
CANCEL_EVENT = Event()


TAKEOFF_DURATION = 2.5
HOVER_DURATION = 5.0

def listen(cf_positions, cf_current_positions):

    class PalmCoreHandler(asyncore.dispatcher_with_send):

        def handle_read(self):
            data = self.recv(BUFFER_SIZE)
            string_data = data.decode('utf-8')

            if string_data != "R":
                robots = string_data.split('|')
                for robot_data, i in zip(robots, range(len(robots))):
                    position = list(map(float, robot_data.split(',')))
                    cf_positions[i] = position
            # Reply with current state
            response = str.join("|", cf_current_positions) + "\n"
            self.send(response.encode('utf-8'))
                

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
#
    proxy_server = PalmCoreSimProxyServer('localhost', PALM_CORE_PROXY_PORT)
    print("[palm_sim] Proxy server initiated, waiting for incoming connections.")
    asyncore.loop()


def main():
    SWARM = Crazyswarm()
    timeHelper = SWARM.timeHelper
    
    print("[palm_sim] Swarm initialized, taking off.")

    for cf in SWARM.allcfs.crazyflies:
        cf.takeoff(1, TAKEOFF_DURATION)

    timeHelper.sleep(TAKEOFF_DURATION + 0.1)
    CF_GOAL_POSITIONS = []
    CF_CURRENT_POSITIONS = []

    print("[palm_sim] Takeoff completed, getting initial positions.");

    for cf in SWARM.allcfs.crazyflies:
        pos = cf.position()
        CF_GOAL_POSITIONS.append(pos)
        CF_CURRENT_POSITIONS.append(f"{pos[0]},{pos[1]},{pos[2]}")

    print("[palm_sim] Initializing listener thread (to palm_core).")
    listen_thread = Thread(target=listen, args=(CF_GOAL_POSITIONS,CF_CURRENT_POSITIONS))
    listen_thread.start()

    print(f"[palm_sim] Entering update loop at {UPDATE_HZ} hz.")
    while True:
        for position, i in zip(CF_GOAL_POSITIONS, range(len(CF_GOAL_POSITIONS))):
            cf = SWARM.allcfs.crazyflies[i]
            cf.cmdPosition(position)
            current_position = cf.position()
            CF_CURRENT_POSITIONS[i] = f"{current_position[0]},{current_position[1]},{current_position[2]}"

        timeHelper.sleep(UPDATE_SLEEP_TIME)
    

if __name__ == "__main__":
    main()