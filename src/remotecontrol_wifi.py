#this script is written for python3
#from inputs import devices
import time
#from inputs import get_gamepad
import pygame
import socket               

WIFI_INPUT_BUFFER_SIZE_ESP32 = 255 #must be the same as in esp32 plotterfeeder code

SEND_DELAY =0.050 #seconds, for sending position to plotter
JOYSTICK_DELAY = 0.005 #seconds, for handling jostick changes
MAX_SPEED = 7500

JOYSTICK_DEADZONE = 0.05 #+/- this value around the events = get_gamepad()middle position is ignored
JOYSTICK_MIDDLE_VALUE = 0

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
pygame.init()
pygame.joystick.init()
myjoystick = pygame.joystick.Joystick(0)
myjoystick.init()

#connect to plotter
sock = socket.socket() 
host = "192.168.4.1" #Plotter IP in local network
port = 1337           #Plotter Server Port     
sock.connect((host, port))#connect to plotter, TODO: this hangs on "no reply"

last_joystick_poll_time = time.time()
last_button_status = 0

while 1:
    #get joystick values
    pygame.event.pump()
    joystick_x = myjoystick.get_axis(0)
    joystick_y = myjoystick.get_axis(1)
    button_status = myjoystick.get_button(2)

    #update local absolute position based on current joystick position
    if((time.time()-last_joystick_poll_time)>=JOYSTICK_DELAY):
        if ((joystick_y < (JOYSTICK_MIDDLE_VALUE-JOYSTICK_DEADZONE)) or (joystick_y > (JOYSTICK_MIDDLE_VALUE+JOYSTICK_DEADZONE))):
            current_y_pos += joystick_y*-1*MAX_SPEED*JOYSTICK_DELAY
            if current_y_pos <0:
                current_y_pos=0
            if current_y_pos > 7500:
                current_y_pos = 7500
        if ((joystick_x < (JOYSTICK_MIDDLE_VALUE-JOYSTICK_DEADZONE)) or (joystick_x > (JOYSTICK_MIDDLE_VALUE+JOYSTICK_DEADZONE))):
                current_x_pos += joystick_x*MAX_SPEED*JOYSTICK_DELAY
                if current_x_pos <0:
                    current_x_pos=0
                if current_x_pos > 10700:
                    current_x_pos = 10700
        last_joystick_poll_time = time.time()

    #send updated position to plotter and also check for button press for pen           
    if((time.time()-last_position_update_time)>=SEND_DELAY):
        send_absolute_position()
        #check for button press
        if (button_status != last_button_status):
            last_button_status = button_status
            if(button_status == 1): send_plotter("PD;")                            
            else: send_plotter("PU;")
        last_position_update_time = time.time()
        
        

sock.close()
print("Done.")