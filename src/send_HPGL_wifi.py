#this script is written for python3
import tkinter as tk
from tkinter import filedialog
import socket               

WIFI_INPUT_BUFFER_SIZE_ESP32 = 255 #must be the same as in esp32 plotterfeeder code

#helper function to split string in chunks
def chunks(lst, n):
    for i in range(0, len(lst), n):
        yield lst[i:i+n]


###start main code###

filename  = "positions_test.txt"
# #query filename to send via gui, comment out to use manual name above
# root = tk.Tk()
# root.withdraw()
# filename = filedialog.askopenfilename()
# root.destroy()

#load data
send_data =""
f= open(filename,"rb")
send_data =f.read()
f.close() 
#print(send_data)

#connect to plotter
sock = socket.socket() 
host = "harryplotter" #Plotter Server Hostname or IP in local network
port = 1337           #Plotter Server Port     
sock.connect((host, port))#connect to plotter, TODO: this hangs on "no reply"
 
#send data in chunks
print("Sending data...")
for chunk in chunks(send_data, (WIFI_INPUT_BUFFER_SIZE_ESP32-1)):#dont forget added null terminator
    print(chunk.decode("utf-8")) #print data that was sent
    sock.send(chunk)
    reply = sock.recv(2) #TODO: this hangs on "no reply"
    #print(reply.decode("utf-8"))
    if (reply != b'OK'):#we expect an "OK"  when the data has been sent to plotter
        sock.close()
        print("Error: Did not receive 'OK' after data chunk. Aborting.")
        break

sock.close()
print("Done.")