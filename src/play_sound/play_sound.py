import os
import time
from pysine import sine #for preview, to not use just comment out and define sine() as 'pass'/empty function

from melodies_twisst import *
from melodies_midi import *

def preview_notes(notes_list):
    for note in notes_list:
        frequency = note[0]
        duration = note[1]

        if frequency == -1:
            time.sleep(duration)
        else:
            sine(frequency, duration)

midipath = os.path.expanduser('~/Downloads/test2.mid')
song = get_song_midi(midipath,8)
song = get_song_twisst(33)
preview_notes(song)





