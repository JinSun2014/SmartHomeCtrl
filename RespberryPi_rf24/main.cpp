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

const int MAX_TRIAL = 5;
const string COMMAND_QUEUE_PATH = "/home/pi/var/run/Command_Queue.txt";
const string API_URL = "http://192.168.81.237:8000";
const int ON = 1;
const int OFF = 0;

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

void postReport(){
    return;
}

bool readCommand(int& node, int& motion){
    ifstream fin(COMMAND_QUEUE_PATH.c_str());
    string line;
    vector<string> queue;

    if (fin.good()){
        while(fin){
            getline(fin, line);
            if (line != "") queue.push_back(line);
        }
    }
    fin.close();
    if (queue.size() > 0){
        node = queue[0][0] - '0';
        if (queue[0][3] == 'n')
            motion = ON;
        else if (queue[0][3] == 'f')
            motion = OFF;
        else
            return false;
        ofstream fout(COMMAND_QUEUE_PATH.c_str(), ofstream::trunc);
        if (fout.good()){
            for (vector<string>::const_iterator it = queue.begin() + 1;
                    it != queue.end(); ++it)
                fout << *it << endl;
        }
        return true;
    }
    return false;

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


int main(int argc, char** argv)
{
    int new_command = 0;
    int node = -1;
    int motion = -1;
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

        new_command = readCommand(node, motion);

        if (new_command){
            new_command = -1;
            cout << "Sending...";
            bool send = sendMsg(node, motion);
            if (send){
                cout << "ok\n";
                postReport();
            }
            else{
                cout << "failed\n";
                postReport();
            }
        }
    }
}

