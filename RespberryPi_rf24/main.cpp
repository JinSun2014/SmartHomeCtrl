#include <cstdlib>
#include <curl/curl.h>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "RF24.h"
#include "compatibility.h"

using namespace std;

/*
   ADDRESS_TABLE = {
   "LT01": {
   "receive": 0xACACACACE1,
   "send": 0xACACACACD2,
   },
   "LT02": {
   "receive": 0xACACACAC31,
   "send": 0xACACACAC42,
   },
   "AC01": {
   "receive": 0xACACACACF0,
   "send": 0xACACACACA3,
   },
   "AC02": {
   "receive": 0xACACACAC10,
   "send": 0xACACACAC20,
   },
   }
   */
const uint64_t bed_light_receive    = 0xACACACACE1LL;
const uint64_t living_light_receive = 0xACACACAC31LL;
const uint64_t ac1_receive          = 0xACACACACF0LL;
const uint64_t ac2_receive          = 0xACACACAC10LL;

const uint64_t send_addresses[] = {
    0xACACACACD2LL,
    0xACACACAC42LL,
    0xACACACACA3LL,
    0xACACACAC20LL,
};

const string BED_LIGHT_ID    = "LT01";
const string LIVING_LIGHT_ID = "LT02";
const string AC1_ID          = "AC01";
const string AC2_ID          = "AC02";

const int PAYLOAD_SIZE  = 32;
const int MAX_TRIAL     = 5;
const string QUEUE_PATH = "/var/www/smarthome/control_queue";
const string API_URL    = "http://192.168.1.66:8000";

RF24 radio(8, 25);

void init()
{
    cout<<"Initializing radio chip..."<<endl;
    radio.begin();
    radio.setRetries(15, 15);
    radio.setDataRate(RF24_250KBPS);
    radio.setPALevel(RF24_PA_HIGH);
    radio.setChannel(120);
    radio.setCRCLength(RF24_CRC_16);

    cout<<"Mapping device channels..."<<endl;
    radio.stopListening();
    //TODO better way
    for (int pipe = 1; pipe <= 4; pipe++)
    {
        radio.openReadingPipe(pipe, send_addresses[pipe - 1]);
    }
    radio.startListening();

    // Dump the configuration of the RF unit for debugging
    //radio.printDetails();
}

int send_message(const string device_id, const string send_message, string& return_message)
{
    int result;
    char response[PAYLOAD_SIZE + 1];
    long start_at;
    bool timeout;
    string response_device_id;

    radio.stopListening();
    //TODO use hash table
    if (device_id == "LT01")
        radio.openWritingPipe(bed_light_receive);
    else if (device_id == "LT02")
        radio.openWritingPipe(living_light_receive);
    else if (device_id == "AC01")
        radio.openWritingPipe(ac1_receive);
    else if (device_id == "AC02")
        radio.openWritingPipe(ac2_receive);
    else
        return 1;

    for (int times = 1; times <= MAX_TRIAL; times++)
    {
        result = radio.write(send_message.c_str(), PAYLOAD_SIZE);
        if (result == 1)
        {
            radio.startListening();

            start_at = __millis();
            timeout = false;
            while (!radio.available() && !timeout)
                if (__millis() - start_at > 500)
                    timeout = true;

            if (!timeout)
            {
                radio.read(response, PAYLOAD_SIZE);
                response[PAYLOAD_SIZE] = '\0';
                response_device_id = string(response, 1, 4);
                if (response[0] == 'R' && device_id == response_device_id)
                {
                    ostringstream oss;
                    oss<<"{\"result\": true, \"message\": \""<<response<<
                        "\", \"trial\": "<<times<<", \"rtt\": "<<__millis() - start_at<<
                        "}";
                    return_message = oss.str();
                    return 0;
                }
            }
        }
    }

    ostringstream oss;
    oss<<"{\"result\": false, \"message\": \"Sending Failed\", \"trial\": "<<
        MAX_TRIAL<<", \"rtt\": 0}";
    return_message = oss.str();
    return 1;
}


void read_queue(string& command)
{
    ifstream fin(QUEUE_PATH.c_str());
    string line;
    vector<string> queue;

    if (fin.good())
    {
        while (fin)
        {
            getline(fin, line);
            if (line != "")
                queue.push_back(line);
        }
    }

    fin.close();

    if (queue.size() > 0)
    {
        command = queue[0];
        ofstream fout(QUEUE_PATH.c_str(), ofstream::trunc);
        if (fout.good())
        {
            for (vector<string>::const_iterator iter = ++queue.begin();
                    iter != queue.end(); iter++)
                fout<<*iter<<endl;
        }

        fout.close();
    }
    else
        command = "";
}

size_t dummy_write(char *ptr, size_t size, size_t nmemb, void *userdata)
{
       return size * nmemb;
}

void post_data(const string url, const string data)
{
    CURL *curl;
    CURLcode res;

    curl_global_init(CURL_GLOBAL_ALL);
    curl = curl_easy_init();

    if (curl)
    {
        // Hide curl output
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &dummy_write);
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data.c_str());

        res = curl_easy_perform(curl);
        if(res != CURLE_OK)
        {
            cout<<"Errror in posting request, "<<curl_easy_strerror(res)<<endl;
        }
        curl_easy_cleanup(curl);
    }
    curl_global_cleanup();
}

int main(int argc, char** argv)
{
    char response[PAYLOAD_SIZE + 1];
    string command;
    string return_message;
    string response_device_id;

    init();
    cout<<"Start reading control queue"<<endl;

    while (true)
    {
        read_queue(command);
        if (command == "")
        {
            if (radio.available())
            {
                radio.read(response, PAYLOAD_SIZE);
                response[PAYLOAD_SIZE] = '\0';
                if (response[0] == 'T')
                {
                    response_device_id = string(response, 1, 4);
                    cout<<response<<endl;

                    ostringstream url;
                    url<<API_URL<<"/event/2/"<<response_device_id;
                    post_data(url.str(), string(response));
                }
            }
        }
        else
        {
            size_t pos = 0;
            vector<string> data;

            while ((pos = command.find(";")) != string::npos)
            {
                data.push_back(command.substr(0, pos));
                command.erase(0, pos + 1);
            }
            data.push_back(command);

            if (data.size() == 3)
            {
                ostringstream oss;
                oss<<"S"<<data[0]<<data[2];
                send_message(data[0], oss.str(), return_message);
                cout<<return_message<<endl;

                ostringstream url;
                url<<API_URL<<"/operation_log/"<<data[0]<<"/"<<data[1];
                post_data(url.str(), return_message);
            }
            else
                cout<<"Invalid Format"<<endl;
        }
    }
}
