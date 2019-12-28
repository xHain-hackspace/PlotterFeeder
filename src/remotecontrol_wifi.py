#this script is written for python3
from inputs import devices
import time
from inputs import get_gamepad
import socket               

WIFI_INPUT_BUFFER_SIZE_ESP32 = 255 #must be the same as in esp32 plotterfeeder code

UPDATE_DELAY =0.050 #seconds


#helper function to split string in chunks
def chunks(lst, n):
    for i in range(0, len(lst), n):
        yield lst[i:i+n]

def send_plotter(send_data):
    send_data=send_data.encode('utf-8')#convert string to bytes
    for chunk in chunks(send_data, (WIFI_INPUT_BUFFER_SIZE_ESP32-1)):#dont forget added null terminator
        print(chunk.decode("utf-8")) #print data that was sent
        sock.send(chunk)
        reply = sock.recv(2) #TODO: this hangs on "no reply"
        #print(reply.decode("utf-8"))
        if (reply != b'OK'):#we expect an "OK"  when the data has been sent to plotter
            sock.close()
            print("Error: Did not receive 'OK' after data chunk. Aborting.")
            break


current_x_pos = 0
current_y_pos = 0
last_position_update_time = 0
def send_absolute_position():
    send_plotter("PA "+str(current_x_pos)+","+str(current_y_pos)+";")


###start main code###

#connect to plotter
sock = socket.socket() 
host = "192.168.4.1" #Plotter IP in local network
port = 1337           #Plotter Server Port     
sock.connect((host, port))#connect to plotter, TODO: this hangs on "no reply"

#get pen
send_plotter("SP2;")

while 1:
    events = get_gamepad()
    for event in events:#handle gamepad events, we only get an event for changed values
        if (event.code == "ABS_X"):
            #print(event.ev_type, event.code, event.state)
            current_x_pos =event.state * 40
        if (event.code == "ABS_Y"):
            #print(event.ev_type, event.code, event.state)
            current_y_pos = (255-event.state) *30
        if (event.code == "BTN_THUMB2"):
            #print(event.ev_type, event.code, event.state)
            button_status =event.state
            if (event.state == 1):
                print("Button down")
                send_plotter("PD;")
            if (event.state == 0):
                print("Button up") 
                send_plotter("PU;")       

    if((time.time()-last_position_update_time)>=UPDATE_DELAY):
        send_absolute_position()
        last_position_update_time = time.time()

sock.close()
print("Done.")