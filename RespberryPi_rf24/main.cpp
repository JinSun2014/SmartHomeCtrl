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
const string COMMAND_QUEUE_PATH = "/var/www/smarthome/control_queue";
const string API_URL    = "http://192.168.81.237:8000";
const int OPEN = 1;
const int CLOSE = 0;

const uint16_t this_node = 0;
const uint16_t other_node = 1;

RF24 radio(8, 25);
RF24Network network(radio);

struct payload_t
{
    int msg;
    unsigned long time;
};

void init()
{
    cout << "Initializing radio chip..." << endl;
    radio.begin();
    radio.setDataRate(RF24_1MBPS);
    radio.setRetries(15, 15);
    network.begin(90, this_node);
}

bool readCommand(const bool& a){
    return a;
}

bool sendMsg(const uint16_t& node, int msg){
    /**
      * Send message to Arduino
    */
    payload_t payload = {msg, __millis()};
    cout << node << ' ' << msg << endl;
    RF24NetworkHeader header(node);
    bool ok = network.write(header, &payload, sizeof(payload));
    return ok;
}

void postReport(){
    return;
}

int main(int argc, char** argv)
{
    const unsigned long interval = 2000; //ms
    unsigned long last_sent_time = __millis();
    unsigned long packets_sent_num = 0;
    bool a = false;

    int new_command = 0;
    RF24NetworkHeader header;
    payload_t payload;
    init();
    
    while(1){
        network.update();
        //receive message
        while (network.available()){
            cout << "OK." << endl;
            network.read(header, &payload, sizeof(payload));
            cout << "Received msg " << payload.msg 
                << " at " << payload.time / 1000;
        }

        unsigned long now = __millis();
        if (now - last_sent_time >= interval){
            a = !a;
            new_command = readCommand(a);
        }

        if (new_command != -1){
            last_sent_time = now;
            cout << "Sending...";
            bool send = sendMsg(other_node, new_command);
            if (send){
                cout << "ok\n";
                postReport();
                new_command = -1;
            }
            else{
                cout << "failed\n";
                postReport();
                new_command = -1;
            }
        }
    }
}

