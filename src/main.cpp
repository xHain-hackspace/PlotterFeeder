#include <Arduino.h>
#include <HardwareSerial.h>
#include <WiFi.h>
#include "vectors.h"

//Plotter Interface
#define CLEAR_TO_SEND_IN_PIN 21
#define TX_OUT_PIN 22
#define RX_IN_PIN 23

//LEDs
#define LED1_PIN 13
#define LED2_PIN 12
#define LED3_PIN 14
#define LED4_PIN 27
#define LED5_PIN 26
#define NR_OF_LEDS 5

//Buttons
#define BUT1_PIN 25
#define BUT2_PIN 33
#define BUT3_PIN 32
#define BUT4_PIN 35
#define BUT5_PIN 34

//Delay
#define SWIPE_DELAY 50

//WiFi
#define WIFI_SSID "xHain Plotter"
#define WIFI_PASSWORD "plotterpassword"
#define WIFI_CHANNEL 5
#define WIFI_MAX_CONNECTIONS 1
#define LISTEN_PORT 1337
#define WIFI_INPUT_BUFFER_SIZE 255

IPAddress local_IP(192,168,4,1);
IPAddress gateway(192,168,4,9);
IPAddress subnet(255,255,255,0);
WiFiServer wifiServer(LISTEN_PORT);
WiFiClient client;
char wifi_input_buffer[WIFI_INPUT_BUFFER_SIZE];

HardwareSerial PlotterSerial(1);

void setup() {
  //Configure Pins
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(CLEAR_TO_SEND_IN_PIN, INPUT);

  pinMode(LED1_PIN, OUTPUT);
  pinMode(LED2_PIN, OUTPUT);
  pinMode(LED3_PIN, OUTPUT);
  pinMode(LED4_PIN, OUTPUT);
  pinMode(LED5_PIN, OUTPUT);

  pinMode(BUT1_PIN, INPUT_PULLUP);
  pinMode(BUT2_PIN, INPUT_PULLUP);
  pinMode(BUT3_PIN, INPUT_PULLUP);
  pinMode(BUT4_PIN, INPUT_PULLUP);
  pinMode(BUT5_PIN, INPUT_PULLUP);

  Serial.begin(9600);
  while (!Serial) {
    ; // wait for Serial port to connect.
  }
  Serial.println("Starting up...");

  PlotterSerial.begin(9600, SERIAL_8N1, RX_IN_PIN, TX_OUT_PIN);//Baud,Mode, RX Pin (In), TX Pin (Out)

  Serial.println("Starting Wifi...");
  Serial.print("Setting soft-AP configuration ... ");
  Serial.println(WiFi.softAPConfig(local_IP, gateway, subnet) ? "Ready" : "Failed!");

  Serial.print("Setting soft-AP ... ");
  Serial.println(WiFi.softAP(WIFI_SSID, WIFI_PASSWORD, WIFI_CHANNEL, false, WIFI_MAX_CONNECTIONS) ? "Ready" : "Failed!");

  Serial.print("Soft-AP IP address = ");
  Serial.println(WiFi.softAPIP());

  Serial.println("Starting WiFi Server");
  wifiServer.begin();
}

uint8_t button_pressed(){
  if(!digitalRead(BUT1_PIN))      return 1;
  else if(!digitalRead(BUT2_PIN)) return 2;
  else if(!digitalRead(BUT3_PIN)) return 3;
  else if(!digitalRead(BUT4_PIN)) return 4;
  else if(!digitalRead(BUT5_PIN)) return 5;
  else return 0;
}
void leds_off(){
  digitalWrite(LED1_PIN,LOW);
  digitalWrite(LED2_PIN,LOW);
  digitalWrite(LED3_PIN,LOW);
  digitalWrite(LED4_PIN,LOW);
  digitalWrite(LED5_PIN,LOW);
}

void leds_on(){
  digitalWrite(LED1_PIN,HIGH);
  digitalWrite(LED2_PIN,HIGH);
  digitalWrite(LED3_PIN,HIGH);
  digitalWrite(LED4_PIN,HIGH);
  digitalWrite(LED5_PIN,HIGH);
}
void leds_flash_on_off(){
  leds_on();
  delay(500);
  leds_off();
  delay(500);
}

void leds_swipe_until_user_input(){
  int leds[] = {LED1_PIN,LED2_PIN,LED3_PIN,LED4_PIN,LED5_PIN};
  int current_led=0;
  int current_swipe = 0;

  //do the effect
  while(!button_pressed() && !(client=wifiServer.available())){//return if pressed, or client connects, save client object
    
    //set the current led according to swipe
    if (current_swipe == 0) digitalWrite(leds[current_led],HIGH);
    else if (current_swipe == 1) digitalWrite(leds[current_led],LOW);
    current_led++;

    //update status variables
    if (current_led == NR_OF_LEDS){
      current_led = 0;
      current_swipe++;
      if (current_swipe==2) current_swipe = 0;
    } 

    delay(SWIPE_DELAY);
  }

  //Switch everything off before returning
  digitalWrite(LED1_PIN,LOW);
  digitalWrite(LED2_PIN,LOW);
  digitalWrite(LED3_PIN,LOW);
  digitalWrite(LED4_PIN,LOW);
  digitalWrite(LED5_PIN,LOW);
}

void test_buttons_and_led(){
    if(!digitalRead(BUT1_PIN)) digitalWrite(LED1_PIN,HIGH);
    else digitalWrite(LED1_PIN,LOW);
    
    if(!digitalRead(BUT2_PIN)) digitalWrite(LED2_PIN,HIGH);
    else digitalWrite(LED2_PIN,LOW);

    if(!digitalRead(BUT3_PIN)) digitalWrite(LED3_PIN,HIGH);
    else digitalWrite(LED3_PIN,LOW);

    if(!digitalRead(BUT4_PIN)) digitalWrite(LED4_PIN,HIGH);
    else digitalWrite(LED4_PIN,LOW);

    if(!digitalRead(BUT5_PIN)) digitalWrite(LED5_PIN,HIGH);
    else digitalWrite(LED5_PIN,LOW);
}

void send_buffered(const char input_chars[]){
  String input = input_chars;
  const uint max_blocksize = 70;
  uint inlen = input.length();

  // buffer for storing the string fragment
  String fragment;
  // number of chars that will be read into buffer
  //uint readlen;
  ulong full_blocks = inlen/max_blocksize;

  //handle full blocks
  for (int current_block=0; current_block < full_blocks; current_block++){
    fragment = input.substring(current_block*max_blocksize, current_block*max_blocksize+max_blocksize);
    PlotterSerial.flush();//wait for all bytes to be sent from previous block in esp's internal buffer
    while(digitalRead(CLEAR_TO_SEND_IN_PIN)) {delay(50);}//check if plotter is ready to receive new block
    PlotterSerial.write(fragment.c_str());//send block    
  }

  //handle last partial block
  if(full_blocks*max_blocksize < inlen){
    fragment = input.substring(full_blocks*max_blocksize, inlen);//copy last partial block
    PlotterSerial.flush();//wait for all bytes to be sent from previous block in esp's internal buffer
    while(digitalRead(CLEAR_TO_SEND_IN_PIN)) {delay(50);}//check if plotter is ready to receive new block
    PlotterSerial.write(fragment.c_str());//send block   
  }


  
  // for (int i=0; i<len; i+=max_bytes) {
  //   wait_until_low(CLEAR_TO_SEND_IN_PIN);
  //   readlen = len - i;
  //   if (readlen > max_bytes) {
  //     readlen = max_bytes;
  //   }
  //   fragment = input.substring(i, i+readlen);
  //   PlotterSerial.write(fragment.c_str());
  // }  
}

void print_id(){
  Serial.println("Getting ID");
  PlotterSerial.write("OI;");
  delay(100);  
  Serial.println("Response:");
  Serial.println(PlotterSerial.readStringUntil('\r'));
}

void loop() {
  int received_data_count = 0;
  
  for (int i = 0; i < 3; i++) leds_flash_on_off();

  while(1){

    //lure people close with flashing lights
    leds_swipe_until_user_input();

    //Check for client connection
    if (client) {
      Serial.println("Client connected");
      leds_on();//indicate active connection      
      while (client.connected()) {//while someone is connected
        delay(10); //wait for some data to arrive
        received_data_count = 0;        
        while (client.available()>0 && received_data_count<(WIFI_INPUT_BUFFER_SIZE-1)) {//while data is available and input buffer not full
          //put available char into input buffer
          wifi_input_buffer[received_data_count] = client.read();  
          received_data_count++;        
        }
        if (received_data_count > 0){//if something was received
          //add extra null terminator for safety
          wifi_input_buffer[received_data_count] = 0;
          //send data to plotter
          Serial.println("Received data over WiFi:");
          Serial.println(wifi_input_buffer);
          //Send data to plotter
          send_buffered(wifi_input_buffer);
          //send acknowledgement to that data was sent to plotter
          if (client.connected()) client.write("OK");
        }      
      } //end while connection
      client.stop();
      client = 0; //reset global client variable
      Serial.println("Client disconnected");
      leds_off();//indicate disconnect
    }//end if client

    //check what is pressed
    delay(50); //debounce wait for the button
    if(!digitalRead(BUT1_PIN)) {
      digitalWrite(LED1_PIN,HIGH);
      send_buffered(vader);
      send_buffered(logo_for_vader);
    }else if(!digitalRead(BUT2_PIN)) {
      digitalWrite(LED2_PIN,HIGH);
      send_buffered(xHain_flyer_cccamp);
      send_buffered(xHain_flyer_cccamp_party);
    }else if(!digitalRead(BUT3_PIN)){
      digitalWrite(LED3_PIN,HIGH);
      //send_buffered(treeflyer_color_a);
      //send_buffered(treeflyer_color_b);   
      send_buffered(dodekaeder1); 
      send_buffered(dodekaeder2); 
      send_buffered(dodekaeder3); 
    }
    /*else if(!digitalRead(BUT4_PIN)){
      digitalWrite(LED4_PIN,HIGH);
      send_buffered(rover);
      send_buffered(logo_for_rover);   
    }*/
     /*else if(!digitalRead(BUT4_PIN)) {
      digitalWrite(LED4_PIN,HIGH);
      send_buffered(b1_sticker_color_a1); 
      send_buffered(b1_sticker_color_a2); 
      send_buffered(b1_sticker_color_b); 
    }*/
    /*else if(!digitalRead(BUT5_PIN)) {
      digitalWrite(LED5_PIN,HIGH);
      send_buffered(cbase_flyer); 
    }*/
    else if(!digitalRead(BUT4_PIN)) {
      digitalWrite(LED5_PIN,HIGH);
      send_buffered(talk);
      send_buffered(dode_info_en1); 
      send_buffered(dode_info_en2); 
      send_buffered(dode_info_en3); 
      //send_buffered(dode_info_en4); 
      //send_buffered(dode_info_en5); 
      send_buffered(dode_info_en_red); 

      //send_buffered(wurst_logo_gruen); 
      //send_buffered(wurst_logo_rot); 
      //send_buffered(wurst_logo_blau); 
      //send_buffered(wurst_logo_gelb); 
    }
    else if(!digitalRead(BUT5_PIN)) {
      digitalWrite(LED5_PIN,HIGH);
      send_buffered(dode_info_de1); 
      send_buffered(dode_info_de2); 
      send_buffered(dode_info_de3); 
      //send_buffered(dode_info_de4); 
      //send_buffered(dode_info_de5);
      send_buffered(dode_info_de_red);  
      //send_buffered(wurst_gruen);
      //send_buffered(wurst_rot); 
      //send_buffered(wurst_blau); 
      //send_buffered(wurst_gelb); 

    }
   
  }

}