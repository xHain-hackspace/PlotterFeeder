# partly based on https://courses.ideate.cmu.edu/16-375/f2023/Python/theater/MIDI-player.py
import mido
import logging

log = logging.getLogger()
#log.setLevel(logging.DEBUG)

def get_song_midi(midipath, selected_channel):

    notes_list = []

    midi_tempo = 500000

    midifile = mido.MidiFile(midipath, clip=True)
    ticks_per_beat = midifile.ticks_per_beat
    playtrack = mido.merge_tracks(midifile.tracks)

    # iterate through all the events in the sequence, creating a channel notes list with timepoints of each note 
    notes = []
    timepoints = [] # time counter at the point of the note event
    current_time = 0
    for event in playtrack:
        log.debug("Playing event: %s", event)

        # don't perform the end of track, it can have an unreasonable delay
        if event.type == 'end_of_track':
            log.debug("Found end event: %s", event)
            break # from for loop

        else:
            # if the next event has a preceding delay, update the time counter before adding the event info to the lists
            if event.time > 0:
                event_s = mido.tick2second(event.time, ticks_per_beat, midi_tempo)
                current_time += event_s

            # ignore metadata messages track_name, instrument_name, key_signature, smpte_offset, etc.
            if event.type == 'set_tempo':
                log.debug("MIDI tempo change: %d usec/beat (%f BPM).", event.tempo, mido.tempo2bpm(event.tempo))
                midi_tempo = event.tempo

            elif event.type in ['note_on', 'note_off'] and event.channel == selected_channel:
                log.debug("Note On: channel %d, note %d, velocity %d", event.channel, event.note, event.velocity)
                #self.bridge.perform_event(event)
                #sine(440, 0.1)
                notes.append(event)
                timepoints.append(current_time)


    on_index = None
    off_index = None
    for curr_index, note in enumerate(notes): # iterate over all notes in order
        
        if on_index is None: # if we don't have a note start already
            if note.type == "note_on" and note.velocity != 0: # check if this is a proper note on command
                on_index = curr_index # and remember note start
        
        elif off_index is None: # if we don't have an off already
            #check if this is a note_off or note_on with no volume/velocity
            if (note.type == "note_on" and note.velocity == 0) or note.type == "note_off":
                off_index = curr_index
        
        if on_index is not None and off_index is not None: # we found a pair of off/on notes
                if off_index < on_index: # off before on = silence
                    pause = timepoints[on_index] - timepoints[off_index]
                    log.debug(f"pause {pause}")
                    #time.sleep(pause)
                    notes_list.append((-1,pause))
                    off_index = None # keep on, search next off
                elif on_index < off_index: # on before off = sound
                    duration = timepoints[off_index] - timepoints[on_index]
                    # calculate duration and play it 
                    note_value = notes[on_index].note
                    frequency = 440 * 2 ** ((note_value-69)/12) # in Hz
                    log.debug(f"play {frequency},{duration}")
                    #sine(frequency, duration)
                    notes_list.append((frequency,duration))
                    on_index = None # keep off, search next on
                else:
                    log.error("on_index is same as off_index")
    return notes_list