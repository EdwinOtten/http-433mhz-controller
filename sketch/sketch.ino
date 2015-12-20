#include <Ethernet.h>
#include <SPI.h>
#include <NewRemoteReceiver.h>
#include <NewRemoteTransmitter.h>

#define EthernetPort        80
#define BufferSize          100
#define PinRFreceive        2
#define PinRFtransmit       8
#define RFtransmitPeriod    260
#define RFtransmitRepeats   3
#define MAX_ARG_LENGTH      3

byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
byte ip[] = { 192, 168, 1, 219 };
byte gateway[] = { 192, 168, 1, 1 };
byte subnet[] = { 255, 255, 255, 0 };
EthernetServer server(EthernetPort);

void setup() {
  Serial.begin(115200);

  Ethernet.begin(mac, ip, gateway, subnet);
  delay(500);
  server.begin();
  Serial.println();
  Serial.print("server is at ");
  Serial.println(Ethernet.localIP());

  NewRemoteReceiver::init(0, PinRFreceive, commandReceived);
}

void loop()
{
  EthernetClient client = server.available();
  if (client) {
    processRequest(client);
  }
}

void processRequest(EthernetClient client) 
{
    char clientline[BufferSize];
    int index = 0;
    
    // an http request ends with a blank line
    boolean current_line_is_blank = true;

    // reset the input buffer
    index = 0;

    while (client.connected()) {
      if (client.available()) {
        char c = client.read();

        // If it isn't a new line, add the character to the buffer
        if (c != '\n' && c != '\r') {
          clientline[index] = c;
          index++;
          // are we too big for the buffer? start tossing out data
          if (index >= BufferSize)
            index = BufferSize - 1;

          // continue to read more data!
          continue;
        }

        // got a \n or \r new line, which means the string is done
        clientline[index] = 0;

        // Print it out for debugging
        Serial.println();
        Serial.println(clientline);

        if (strstr(clientline, "GET / ") != 0) {

          printHttpResponse(client, "400 Bad Request", "text/html", "The Force is strong in you, but not ready you are! Forgot to provide parameters you did.");

        } else if (strstr(clientline, "GET /?args=") != 0) {

          // request url contains arguments, extract those
          char *parameters;

          // Example request:
          // GET /?args=1,99,2,1 HTTP/1.1

          parameters = clientline + 11; // look after the "GET /?args=" (11 chars)
          // clear the HTTP/1.1 from the end of line
          (strstr(clientline, " HTTP"))[0] = 0;
          
          char *type = subStr(parameters, ",", 1);
          char *address = subStr(parameters, ",", 2);
          char *channel = subStr(parameters, ",", 3);
          char *value = subStr(parameters, ",", 4);
          
          String data = "Hello there! You provided the following parameters:";
          data += "\rtype: ";
          data += type;
          data += "\raddress: ";
          data += address;
          data += "\rchannel: ";
          data += channel;
          data += "\rvalue: ";
          data += value;
          printHttpResponse(client, "200 OK", "text/html", data);
          
          transmit(String(type).toInt(), String(address).toInt(), String(channel).toInt(), String(value).toInt());

        } else {

          // everything else is a 404
          printHttpResponse(client, "404 Not Found", "text/html", "<h2>File Not Found!</h2>");

        }
        break;
      }
    }

    // give the web browser time to receive the data
    delay(1);
    client.stop(); 
}

void transmit(int type, int address, int channel, int value) 
{
  
}

char* subStr (char* input_string, char *separator, int segment_number) 
{
  char *act, *sub, *ptr;
  static char copy[MAX_ARG_LENGTH];
  int i;

  strcpy(copy, input_string);

  for (i = 1, act = copy; i <= segment_number; i++, act = NULL) 
  {
	sub = strtok_r(act, separator, &ptr);
	if (sub == NULL) break;
  }
  return sub;  
}

void printHttpResponse(EthernetClient client, char httpCode[], char contentType[], String data) 
{
  client.print("HTTP/1.1 ");
  client.println(httpCode);
  client.print("Content-Type: ");
  client.println(contentType);  
  client.println("Connection: close");
  client.print("Content-Length: ");
  client.println(data.length());
  client.println();
  client.print(data);
  client.println();

  client.println();
}



// Callback method for the NewRemoteReceiver
void commandReceived(NewRemoteCode receivedCode) {
  // Disable the receiver; otherwise it might pick up the retransmit as well.
  NewRemoteReceiver::disable();

  // Need interrupts for delay()
  interrupts();

  // Wait 5 seconds before sending.
  delay(5000);

  Serial.println();
  Serial.println("### Received NewRemoteCode ###");
  Serial.print("address: ");
  Serial.println(receivedCode.address);
//  transmitCode(receivedCode, receivedCode.address);

  NewRemoteReceiver::enable();
}


void transmitCode(NewRemoteCode command, unsigned long address) {
  NewRemoteTransmitter transmitter(address, PinRFtransmit, RFtransmitPeriod, RFtransmitRepeats);

  if (command.switchType == NewRemoteCode::dim || (command.switchType == NewRemoteCode::on && command.dimLevelPresent)) {
    // Dimmer signal received
    if (command.groupBit) {
      transmitter.sendGroupDim(command.dimLevel);
    } else {
      transmitter.sendDim(command.unit, command.dimLevel);
    }
  }
  else {
    // On/Off signal received
    bool isOn = command.switchType == NewRemoteCode::on;
    if (command.groupBit) {
      transmitter.sendGroup(isOn);
    } else {
      transmitter.sendUnit(command.unit, isOn);
    }
  }
}
