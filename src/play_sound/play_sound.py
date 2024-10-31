import os
import time
from numpy import arange
from pysine import sine #for preview, to not use just comment out and define sine() as 'pass'/empty function
import serial

from melodies_twisst import *
from melodies_midi import *

def preview_notes_speaker(notes_list):
    for note in notes_list:
        frequency = note[0]
        duration = note[1]

        if frequency == -1:
            time.sleep(duration)
        else:
            sine(frequency, duration)

def preview_notes_plotter(notes_list, portname, baud, transpose_factor, transpose_offset, stop_after=None):
    
    offset_y = 0
    position_x = 5000

    MM_PER_PLOTTER_UNIT = 0.025 # mm/pu

    plotter = serial.Serial(portname, baud, bytesize=8, parity='N', stopbits=serial.STOPBITS_ONE,
                            timeout=5, xonxoff=False, rtscts=True)
    
    plotter.write("IN;PD;".encode()) # init, pen down (else speed wont work)
    plotter.write(f"PA{position_x:.4f},{offset_y:.4f};".encode())
    time.sleep(1)

    for index, note in enumerate(notes_list):
        frequency = note[0]
        duration = note[1]

        if stop_after is not None and index > stop_after:
            break

        if frequency == -1:
            time.sleep(duration)
        else:            
            speed = transpose_factor*frequency + transpose_offset # cm/s, max 38.1, min 0.38 38 = 7700 Hz 5 1200,303. 10 1222
            #print(speed)
            if speed > 38.1:
                print(f"Warning: speed max truncated to 38.1, was {speed}")
                speed = 38.1
                
            elif speed < 0.38:
                print(f"Warning: speed min truncated to 0.38, was {speed}")
                speed = 0.38

            duration_plotter = duration - 0.05 # seconds            
            # calculate travel in plotter units
            travel_mm = (speed * 10) * duration_plotter
            travel_pu = travel_mm / MM_PER_PLOTTER_UNIT 

            if travel_pu > 5000: # check paper size
                travel_pu = 5000 # TODO split in multiple moves instead of truncate
                print("Warning: travel truncated!")
            
            if position_x > 5000:
                position_x -= travel_pu
            else:
                position_x += travel_pu
            
            position_y = offset_y 

            plotter.write(f"VS{speed:.4f};".encode())
            plotter.write(f"PA{position_x:.4f},{position_y:.4f};".encode())
            time.sleep(duration)
    
    plotter.flush()
    plotter.close()

midipath = os.path.expanduser('~/Downloads/Darude - Sandstorm.mid.mid')
#song = get_song_midi(midipath,5)
song = get_song_twisst(33)

#preview_notes_speaker(song)

portname = "/dev/ttyUSB0"
baud = 9600

transpose_factor = 1.0 * 3.81 / 440
transpose_offset = 0
stop_after = None

print(f"preview at offset {transpose_offset}, factor {transpose_factor}...")
preview_notes_plotter(song, portname, baud, transpose_factor, transpose_offset, stop_after)








