**About:**
A little tool I developed to test an area around a static receiving node to map out deadspots and compare viability of receiver locations.
This was made with the Xiao ESP32-S3 from SEEED Studio along with their LoRa module. The GPS module is from M5Stack.
It involves one static receiver node and a mobile node:
-The receiver node is placed in the location to be tested.
-The mobile node moves around and pings the GPS coordinates of itself to the receiver (this is all encrypted). 
-The receiver then logs the gps and the RSSI and SNR and prints it via serial along with the logged gps coordinates.
-The receiver responds with a ping and when the mobile node receives this, it will display the RSSI and SNR of the received response.

**Data logging and visualisation:**
The first python script will read the serial message printed by the receiver, parse the data and log it in a csv file. 
The second python script will take the csv file and produce a heatmap with it.

**NOTES:**
The code as of now (sept 2025) is the first stage of development - a proof of concept. I plan on tidying it up and making some changes to how it works.
The for the heatmap to be of much use - a fair amount of data points are needed. If using this, be aware of local transmission limits and duty cycles.
*CHANGES FOR THE FUTURE*
I am planning on also logging each transmission from the mobile node on a microSD card to provide more data points and accurate depictions of dead zones.
I want to explore the different interpolation methods for filling in data for the heatmap - there are likely some more suitable than cubic - this should help improve accuracy of the visualisation.
Potentially thinking about reversing the logic... ie the static node is the one pinging during the duration of the test and the mobile node will receive and log the RSSI, SNR, and GPS coordinates upon reception 
to a csv file on a microSD card - this can then be given to the visualisation script once the test is complete.
