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

const int MAX_TRIAL     = 5;
const string QUEUE_PATH = "/var/www/smarthome/control_queue";
const string API_URL    = "http://192.168.1.66:8000";

RF24 radio(8, 25);
RF24Network network(radio);

const uint16_t this_node = 1;
const uint16_t other_node = 0;

struct payload_t
{
    unsigned long ms;
    unsigned long counter;
};

void init()
{
    cout << "Initializing radio chip..." << endl;
    radio.begin();
    radio.setDataRate(RF24_1MBPS);
    radio.setRetries(15, 15);
    network.begin(90, this_node);
}

bool sendMsg(unsigned long &packets_sent_num){
    payload_t payload = {__millis(), packets_sent_num++};
    cout << payload.ms << ' ' << payload.counter << endl;
    RF24NetworkHeader header(other_node);
    bool ok = network.write(header, &payload, sizeof(payload));
    return ok;
}

int main(int argc, char** argv)
{
    const unsigned long interval = 2000; //ms
    unsigned long last_sent_time = __millis();
    unsigned long packets_sent_num = 0;
    RF24NetworkHeader header;
    payload_t payload;
    init();
    
    while(1){
        network.update();

        while (network.available()){
            cout << "OK." << endl;
            network.read(header, &payload, sizeof(payload));
            cout << "Received packet # " << payload.counter
                << " at " << payload.ms;
        }

        unsigned long now = __millis();
        if (now - last_sent_time >= interval){
            last_sent_time = now;
            cout << "Sending...";
            bool send = sendMsg(packets_sent_num);
            if (send)
                cout << "ok\n";
            else
                cout << "failed\n";
        }
    }
}

