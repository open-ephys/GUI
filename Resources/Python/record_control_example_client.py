"""
    A zmq client to test remote control of open-ephys GUI
"""

import zmq
import os
import time


def run_client():

    # Basic start/stop commands
    start_cmd = 'ProcessorCommunication RecordControl StartRecord'
    stop_cmd = 'ProcessorCommunication RecordControl StopRecord'

    # Example settings
    rec_dir = os.path.join(os.getcwd(), 'Output_RecordControl')

    # Some commands
    commands = [start_cmd + ' RecordingDirectory=%s' % rec_dir,
                start_cmd + ' CreateNewDateDirectory=1',
                start_cmd + ' PrependText=Session001 AppendText=Condition001',
                start_cmd + ' PrependText=Session001 AppendText=Condition002',
                start_cmd + ' PrependText=Session002 AppendText=Condition001']

    # Connect network handler
    ip = '127.0.0.1'
    port = 5556
    timeout = 1.

    url = "tcp://%s:%d" % (ip, port)

    with zmq.Context() as context:
        with context.socket(zmq.REQ) as socket:
            socket.RCVTIMEO = int(timeout * 1000)  # timeout in milliseconds
            socket.connect(url)

            for start_cmd in commands:

                for cmd in [start_cmd, stop_cmd]:
                    socket.send(cmd)
                    answer = socket.recv()
                    print answer

                    if 'StartRecord' in cmd:
                        # Record data for 5 seconds
                        time.sleep(5)
                    else:
                        # Stop for 1 second
                        time.sleep(1)

            # Finally, stop data acquisition
            socket.send('ProcessorCommunication RecordControl StopAcquisition')
            answer = socket.recv()
            print answer


if __name__ == '__main__':
    run_client()

