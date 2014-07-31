/*
 Copyright (C) 2012 James Coliz, Jr. <maniacbug@ymail.com>

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 version 2 as published by the Free Software Foundation.
 */

/**
 * Simplest possible example of using RF24Network,
 *
 * RECEIVER NODE
 * Listens for messages from the transmitter and prints them out.
 */

#include <RF24Network.h>
#include <RF24.h>
#include <SPI.h>

// nRF24L01(+) radio attached using Getting Started board 
RF24 radio(9,10);

// Network uses that radio
RF24Network network(radio);

// Status Code
const bool OPEN = 1;
const bool CLOSE = 0;

// Pin of Relay
const int RELAY_PIN = 4;

// Address of our node
const uint16_t this_node = 1;

// Address of the other node
const uint16_t other_node = 0;

// How many have we sent already
unsigned long packets_sent;

//Last Send Time
unsigned long last_sent;

//Switch
int relay;

// Structure of our payload
struct payload_t
{
  int msg;
  unsigned long time;
};

void setup(void)
{
  Serial.begin(57600);
  Serial.println("SmartHome Node1");
 
  SPI.begin();
  radio.begin();
  network.begin(/*channel*/ 90, /*node address*/ this_node);
  
  //pinMode(RELAY_PIN, OUTPUT);
}

bool sendMsg(){
  // Send Msg to Raspberry Pi
  payload_t payload = {millis(), packets_sent++};
  RF24NetworkHeader header(/*to node*/ other_node);
  bool ok = network.write(header,&payload,sizeof(payload));
  return ok;
}

void loop(void)
{
  // Pump the network regularly
  network.update();

  // Is there anything ready for us?
  while ( network.available() )
  {
    // If so, grab it and print it out
    RF24NetworkHeader header;
    payload_t payload;
    network.read(header,&payload,sizeof(payload));
    Serial.print("Received message ");
    Serial.print(payload.msg);
    Serial.print(" at ");
    Serial.println(payload.time / 1000);
    relay = payload.msg;
    if (relay == OPEN)  digitalWrite(RELAY_PIN, HIGH);
    else if (relay == CLOSE) digitalWrite(RELAY_PIN, LOW);
    else{
      sendMsg();
    }
  }
  
  unsigned long now = millis();
  
  if (now - last_sent >= 8000 && false){
    bool ok = sendMsg();
    if (ok)
      Serial.println("ok.");
    else
      Serial.println("failed.");
    last_sent = now;
  }
}
// vim:ai:cin:sts=2 sw=2 ft=cpp
