# ICP Project 2020/2021 - variant 1
Authors: Radek Manak (xmanak20) & Branislav Brezani (xbreza01)

This project focuses on utilizing the MQTT protocol using client library Eclipse Paho.

### Usage:
> BUILD BINARIES: make

> RUN EXPLORER: make run

> RUN SIMULATOR: make sim

> BUILD DOCUMENTATION: make doxygen

### Explorer:
This program is a MQTT client with GUI that provides structured overview of topics and allows publishing.
Unimplemented features:
- Messages filtering
- Recognize images from binary data
- Explorer state saving
- Dashboard

### Traffic simulator:
This program simulates operation of many various concurrent sensors and collects their output, which is published to specified MQTT server based on FIFO rule.
Currently these types of sensors are supported:
- Sensors outputting integer/float values in specified range, value is randomly increased or decreased every period
- A door switch, which has 2 states(opened & closed), state changes after random period from specified period range
- A valve, which has 2 states(opened & closed), state changes after receiving commands "open" or "close"
- A thernostat outputting integer values, value can be set using command "set <value>", current value then gradually changes every period until it equals the new one
- A camera which publishes new image from specified list every period

 Simulator configuration can be customized in file sim/traffic.cfg
