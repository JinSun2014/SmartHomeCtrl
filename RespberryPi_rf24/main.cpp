#include <cstdlib>
#include <curl/curl.h>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "RF24Network.h"
#include "RF24.h"
#include "compatibility.h"
#include "spi.h"

using namespace std;

const int PAYLOAD_SIZE  = 32;
const int MAX_TRIAL     = 5;
const string QUEUE_PATH = "/var/www/smarthome/control_queue";
const string API_URL    = "http://192.168.1.66:8000";

RF24 radio(8, 25);
RF24Network network(radio);

void init()
{
    cout<<"Initializing radio chip..."<<endl;
    radio.begin();
    radio.setRetries(15, 15);
    radio.setDataRate(RF24_250KBPS);
    radio.setPALevel(RF24_PA_HIGH);
    radio.setChannel(120);
    radio.setCRCLength(RF24_CRC_16);
}

int main(int argc, char** argv)
{
}
