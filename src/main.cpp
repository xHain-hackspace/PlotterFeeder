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

//Dart Game
#define DART_COUNTDOWN_DELAY 1000

//WiFi
#define WIFI_MODE               WIFI_AP // WIFI_STA or WIFI_AP
#define WIFI_SSID               "xHain Plotter"
#define WIFI_PASSWORD           "plotterpassword"
#define WIFI_INPUT_BUFFER_SIZE  255
#define WIFI_CHANNEL            5
#define WIFI_MAX_CONNECTIONS    1
#define HOSTNAME                "hp7550"
#define LISTEN_PORT             1337

IPAddress local_IP(192,168,4,1);
IPAddress gateway(192,168,4,13);
IPAddress subnet(255,255,255,0);

WiFiServer wifiServer(LISTEN_PORT);
WiFiClient client;
char wifi_input_buffer[WIFI_INPUT_BUFFER_SIZE];

HardwareSerial PlotterSerial(1);

void wifi_ap() {
  Serial.println("Starting Wifi...");
  Serial.print("Setting soft-AP configuration ... ");
  Serial.println(WiFi.softAPConfig(local_IP, gateway, subnet) ? "Ready" : "Failed!");

  Serial.print("Setting soft-AP ... ");
  Serial.println(WiFi.softAP(WIFI_SSID, WIFI_PASSWORD, WIFI_CHANNEL, false, WIFI_MAX_CONNECTIONS) ? "Ready" : "Failed!");

  Serial.print("Soft-AP IP address = ");
  Serial.println(WiFi.softAPIP());
}

void wifi_connect() {
  Serial.print("Connecting to ");
  Serial.print(WIFI_SSID);

  WiFi.setHostname(HOSTNAME);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.print("Hostname: ");
  Serial.println(HOSTNAME);
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
}

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

  if (WIFI_MODE == WIFI_AP) {
    wifi_ap();
  } else {
    wifi_connect();
  }

  Serial.print("Starting TCP server on port ");
  Serial.println(LISTEN_PORT);
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
}

void print_id(){
  Serial.println("Getting ID");
  PlotterSerial.write("OI;");
  delay(100);  
  Serial.println("Response:");
  Serial.println(PlotterSerial.readStringUntil('\r'));
}

void autofeed_paper(){
  PlotterSerial.write("PG;");
}

void dart_game(){
  uint target_middle_x =5350;
  uint target_middle_y = 3750;
  char stringBuffer[100];
  //init/select pen
  send_buffered("IN;SP2;");
  //go to target position
  sprintf(stringBuffer,"PA%d,%d;",target_middle_x,target_middle_y);
  send_buffered(stringBuffer);
  //draw target
  send_buffered("CI400;CI800;CI1600;CI2400;");
  //draw info text
  send_buffered("PA500,200;LBTry to hit the target: Press button now to launch arrow. Press+hold again to hit the target.;");
  delay(20000);//adjust this at least to the time it takes to plot the previous stuff
  //go to "text view" position
  //go to score position
  sprintf(stringBuffer,"PA%d,%d;",100,7000);
  send_buffered(stringBuffer);
  //wait for button press and release
  while(!button_pressed()){delay(50);}
  while(button_pressed()){delay(50);}
  // //draw countdown
  // send_buffered("LB3   ;");
  // delay(DART_COUNTDOWN_DELAY);
  // send_buffered("LB2   ;");
  // delay(DART_COUNTDOWN_DELAY);
  // send_buffered("LB1   ;");
  // delay(DART_COUNTDOWN_DELAY);
  // send_buffered("LBGo! ;");
  // delay(DART_COUNTDOWN_DELAY);
  //move pen and wait for any button
  uint score = 0;

  uint current_position_x=0;
  uint current_position_y=0;

  for (int i=0; i<5; i++){
    while(!button_pressed()){
      current_position_x = target_middle_x + random(-2400,2400);
      current_position_y = target_middle_y + random(-2400,2400);
      sprintf(stringBuffer,"PA%d,%d;",current_position_x,current_position_y);
      send_buffered(stringBuffer);
      delay(250);
    }
  
    //plot arrow
    send_buffered("LB<-------<<;");
    delay(1000);
    //determine score
    score += 3000- sqrt(pow(target_middle_x - current_position_x*1.0,2)+ pow(target_middle_y - current_position_y*1.0,2)*1.0);

    // wait until button is released
    while(button_pressed()){delay(50);}
  }

  //go to score position
  sprintf(stringBuffer,"PA%d,%d;",1000,1000);
  send_buffered(stringBuffer);
  //plot score text
  sprintf(stringBuffer,"LBYour score is: %d Thanks for playing. :)\n\nx-hain.de, Gruenberger Str. 16, 10243 Berlin;",score);
  send_buffered(stringBuffer);
  //eject paper
  autofeed_paper();  
  while(button_pressed());//wait for button to be released, so we dont immediately go to game again
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
      autofeed_paper();
    }else if(!digitalRead(BUT2_PIN)) {
      digitalWrite(LED2_PIN,HIGH);
      //send_buffered(xHain_flyer_cccamp);
      //send_buffered(xHain_flyer_cccamp_party);
      send_buffered(flyer_36c3);
      autofeed_paper();
    }else if(!digitalRead(BUT3_PIN)){
      digitalWrite(LED3_PIN,HIGH);
      dart_game();
      //send_buffered(dodekaeder1); 
      //send_buffered(dodekaeder2); 
      //send_buffered(dodekaeder3); 
      //autofeed_paper();
    }
    else if(!digitalRead(BUT4_PIN)) {
      digitalWrite(LED4_PIN,HIGH);
      //send_buffered(dode_info_en1); 
      //send_buffered(dode_info_en2); 
      //send_buffered(dode_info_en3); 
      //send_buffered(dode_info_en_red); 
      //autofeed_paper();
    }
    else if(!digitalRead(BUT5_PIN)) {
      digitalWrite(LED5_PIN,HIGH);
      //send_buffered(dode_info_de1); 
      //send_buffered(dode_info_de2); 
      //send_buffered(dode_info_de3); 
      //send_buffered(dode_info_de_red); 
      //autofeed_paper(); 
    }   
  }
}