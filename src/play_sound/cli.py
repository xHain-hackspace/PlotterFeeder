#!/usr/bin/env python
from play_sound import *
from melodies_twisst import *
from melodies_midi import *
import argparse
import sys
from os import path



def check_midi_param(values):
    if not path.isfile(values[0]):
        print("Please enter path to file")
        sys.exit(1)
    try:
        values[1] = int(values[1])
    except ValueError:
        print("Please enter integer")
        sys.exit(1)
    return values

def main():

    ap = argparse.ArgumentParser(prog="PlotterMusic", description="Play music on the PlotterFeeder")
    ap.add_argument("--id", type=int, help="play song with specific id")
    ap.add_argument("--midi", nargs=2, help="play midi file")
    args = ap.parse_args()


    if args.id:
        song = get_song_twisst(args.id)
    elif args.midi:
        check_midi_param(args.midi)
        song = get_song_midi(args.midi[0], args.midi[1])
    else:
        sys.exit(0)

    #portname = "/dev/ttyUSB0"
    #baud = 9600
    #plotter = serial.Serial(portname, baud, bytesize=8, parity='N', stopbits=serial.STOPBITS_ONE,
    #                        timeout=5, xonxoff=False, rtscts=True)

    transpose_factor = 5 * 8  / 440 #*0.25
    transpose_offset = 0
    start_at = None #100
    stop_after = 127 #140
    print(f"preview at offset {transpose_offset}, factor {transpose_factor}...")


    plotter = NetworkPlotter("harryplotter", 1337)


    plotter.open() # not needed for serial, required for network
    preview_notes_plotter(song, plotter, transpose_factor, transpose_offset, start_at, stop_after)
    plotter.close()


if __name__ == "__main__":
    main()
