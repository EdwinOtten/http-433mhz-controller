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

byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
byte ip[] = { 192, 168, 192, 80 };
EthernetServer server(EthernetPort);


void setup() {
  Serial.begin(9600);

  Ethernet.begin(mac, ip);
  server.begin();
  Serial.print("server is at ");
  Serial.println(Ethernet.localIP());

  NewRemoteReceiver::init(0, PinRFreceive, commandReceived);
}

void loop()
{
  char clientline[BufferSize];
  int index = 0;

  EthernetClient client = server.available();
  if (client) {
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
        Serial.println(clientline);

        if (strstr(clientline, "GET / ") != 0) {

          printHttpHeaders(client, "200 OK", "text/html");
          client.println("Hello there! you didn't provide any parameters!");

        } else if (strstr(clientline, "GET /?") != 0) {

          // request url contains arguments, extract those
          char *parameters;

          // Example request:
          // GET /?first=John&Last=Doe HTTP/1.1

          parameters = clientline + 6; // look after the "GET /?" (6 chars)
          // clear the HTTP/1.1 from the end of line
          (strstr(clientline, " HTTP"))[0] = 0;

          // print the parameters
          Serial.println("Parameters: ");
          Serial.print(parameters);

          printHttpHeaders(client, "200 OK", "text/html");
          client.println("Hello there! You provided the following paramters:");
          client.println(parameters);

        } else {

          // everything else is a 404
          printHttpHeaders(client, "404 Not Found", "text/html");
          client.println("<h2>File Not Found!</h2>");

        }
        break;
      }
    }

    // give the web browser time to receive the data
    delay(1);
    client.stop();
  }
}

void printHttpHeaders(EthernetClient client, char httpCode[], char contentType[]) {
  client.println("HTTP/1.1 ");
  client.print(httpCode);
  client.println("Content-Type: ");
  client.print(contentType);
  client.println("Connection: close");  // the connection will be closed after completion of the response
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

  transmitCode(receivedCode, receivedCode.address);

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
