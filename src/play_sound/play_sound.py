import os
import time
from numpy import arange
from pysine import sine #for preview, to not use just comment out and define sine() as 'pass'/empty function
import serial
import socket

from melodies_twisst import *
from melodies_midi import *

class NetworkPlotter:
    def __init__(self, host, port) -> None:
        self.socket = socket.socket() 
        self.host = host
        self.port = port
        self.WIFI_INPUT_BUFFER_SIZE_ESP32 = 255 #must be the same as in esp32 plotterfeeder code
        
    def open(self):
        self.socket.connect((self.host, self.port))#connect to plotter, TODO: this hangs on "no reply"
        
    def write(self, send_data):
        #send data in chunks
        #print("Sending data...")
        for chunk in self._chunk_it(send_data, (self.WIFI_INPUT_BUFFER_SIZE_ESP32-1)):#dont forget added null terminator
            #print(chunk.decode("utf-8")) #print data that was sent
            self.socket.send(chunk)
            reply = self.socket.recv(2) #TODO: this hangs on "no reply"
            #print(reply.decode("utf-8"))
            if (reply != b'OK'):#we expect an "OK"  when the data has been sent to plotter
                self.socket.close()
                print("Error: Did not receive 'OK' after data chunk. Aborting.")
                break
    
    def close(self):
        self.socket.close()
        
    #helper function to split string in chunks
    def _chunk_it(self, lst, n):
        for i in range(0, len(lst), n):
            yield lst[i:i+n]
        

def preview_notes_speaker(notes_list):
    for note in notes_list:
        frequency = note[0]
        duration = note[1]

        if frequency == -1:
            time.sleep(duration)
        else:
            sine(frequency, duration)

def preview_notes_plotter(notes_list, plotter, transpose_factor, transpose_offset, stop_after=None):
    
    offset_other_axis = 0
    travel_limit = 4500
    travel_middle = travel_limit -500
    controlled_coordinate = travel_middle

    MM_PER_PLOTTER_UNIT = 0.025 # mm/pu     # HP 7470A
    
    plotter.write("IN;PD;".encode()) # init, pen down (else speed wont work)
    plotter.write(f"PA{offset_other_axis:.4f},{controlled_coordinate:.4f};".encode())
    #plotter.write(f"PA{controlled_coordinate:.4f},{offset_other_axis:.4f};".encode())
    time.sleep(1)

    for index, note in enumerate(notes_list):
        frequency = note[0]
        duration = note[1]
        duration = duration
        

        if stop_after is not None and index > stop_after:
            break

        if frequency == -1:
            speed = 10
        else:            
            speed = transpose_factor*frequency + transpose_offset # cm/s, max 38.1, min 0.38 38 = 7700 Hz 5 1200,303. 10 1222
            #print(speed)
            if speed > 80:
                print(f"Warning: speed max truncated to 80, was {speed}")
                speed = 80
                
            elif speed < 1:
                print(f"Warning: speed min truncated to 1, was {speed}")
                speed = 1
         
        # calculate travel in plotter units
        travel_mm = (speed * 10) * duration
        travel_pu = travel_mm / MM_PER_PLOTTER_UNIT 

        extra_travel_times = 0
        if travel_pu > travel_limit: # check paper size
            extra_travel_times = int(travel_pu/travel_limit)
            travel_pu = travel_pu%travel_limit # TODO split in multiple moves instead of truncate
            #print("multitravel")
        
        plotter.write(f"VS{speed:.4f};".encode())
        
        for travel_nr in range(extra_travel_times,-1,-1):
            #print(travel_nr)
            if travel_nr > 1:
                this_step_travel = travel_limit
            else:
                this_step_travel = travel_pu
                
            if controlled_coordinate > travel_middle:
                controlled_coordinate -= this_step_travel
            else:
                controlled_coordinate += this_step_travel
        
            passive_coordinate = offset_other_axis         
            plotter.write(f"PA{passive_coordinate:.4f},{controlled_coordinate:.4f};".encode())
            #plotter.write(f"PA{controlled_coordinate:.4f},{passive_coordinate:.4f};".encode())
    

# midipath = os.path.expanduser('~/Downloads/darude-sandstorm.mid')
# song = get_song_midi(midipath,5)
song = get_song_twisst(33)

#preview_notes_speaker(song)

portname = "/dev/ttyUSB0"
baud = 9600
#plotter = serial.Serial(portname, baud, bytesize=8, parity='N', stopbits=serial.STOPBITS_ONE,
#                            timeout=5, xonxoff=False, rtscts=True)

plotter = NetworkPlotter("harryplotter", 1337)

transpose_factor = 5 * 8  / 440 #*0.25
transpose_offset = 0
stop_after = None

print(f"preview at offset {transpose_offset}, factor {transpose_factor}...")

plotter.open() # not needed for serial, required for network

preview_notes_plotter(song, plotter, transpose_factor, transpose_offset, stop_after)
plotter.close()






