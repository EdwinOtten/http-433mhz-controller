# HTTP 433MHz controller
API for Arduino to send and receive 433 commands (for powersocket-switches and such). Accessible via HTTP so you can trigger it with IFTTT's Maker channel. It can also make a HTTP request to Maker for every received command, so you can use it to trigger your recipes.

## Uses
You can use this sketch for several purposes:
- Send 433 commands to devices you own (e.g. powersocket switches or a smart home controlstation) from IFTTT
- Log 433 commands in your home by calling a IFTTT trigger (or your own logserver)
- Trigger IFTTT recipes with a 433 remote (basically a physical button for IFTTT)

## Dependencies
This Arduino sketch uses the [Arduino Ethernet Library]() to listen for HTTP requests and call a (currently hardcoded) URL for every incoming 433 command. It also uses the [433MHzForArduino](https://bitbucket.org/fuzzillogic/433mhzforarduino/wiki/Home) library to send and receive RF commands on 433MHz.

## Installation
```There are no installation instructions yet.```

## Project status
This project has just started and is not even ready for a pre-alpha release. However, feel free to 'watch' this repo to stay up-to-date with my progress.

## License
This project is licensed under the GNU General Public License V2. This means it's open source and anyone is allowed to use the code. However, it does require anyone who distributes the code or a derivative work to make the source available under the same terms. Spread your knowledge! ;-)
