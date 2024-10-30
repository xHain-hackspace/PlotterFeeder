import os
import time
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

def preview_notes_plotter(notes_list, portname, baud):
    
    offset_y = 0
    position_x = 5000

    MM_PER_PLOTTER_UNIT = 0.025 # mm/pu

    plotter = serial.Serial(portname, baud, bytesize=8, parity='N', stopbits=serial.STOPBITS_ONE,
                            timeout=5, xonxoff=False, rtscts=True)
    
    plotter.write("IN;PD;".encode()) # init, pen down (else speed wont work)
    plotter.write(f"PA{position_x:.4f},{offset_y:.4f};".encode())
    time.sleep(1)

    for note in notes_list:
        frequency = note[0]
        duration = note[1]

        if frequency == -1:
            time.sleep(duration)
        else:            
            speed = 5*4*frequency/440 # cm/s, max 38.1, min 0.38 38 = 7700 Hz 5 1200,303. 10 1222
            print(speed)
            if speed > 38.1:
                speed = 38.1
                print("Warning: speed max truncated")
            elif speed < 0.38:
                speed = 0.38
                print("Warning: speed min truncated")


            duration_plotter = duration*0.9 # seconds            
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

midipath = os.path.expanduser('~/Downloads/test2.mid')
#song = get_song_midi(midipath,8)
song = get_song_twisst(9)

preview_notes_speaker(song)

portname = "/dev/ttyUSB0"
baud = 9600
preview_notes_plotter(song, portname, baud)








