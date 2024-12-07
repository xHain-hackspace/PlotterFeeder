#!/usr/bin/env python
from play_sound import *
from melodies_twisst import *
import argparse
import sys



def main():

    ap = argparse.ArgumentParser(prog="PlotterMusic", description="Play music on the PlotterFeeder")
    ap.add_argument("--id", type=int, help="play song with specific id")
    args = ap.parse_args()

    if args.id:
        song = get_song_twisst(args.id)
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
    plotter.write(f"SP1;PU;PA500,500;LBYou have been rickrolled by Harry Plotter at xHain. Have a great day! :) \n\rx-hain.de, Gruenberger Str 16, 10243 Berlin\x03;".encode())
    plotter.close()


if __name__ == "__main__":
    main()
