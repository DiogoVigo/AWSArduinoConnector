# AWSArduinoConnector
Connect to AWS IoT through a ESP8266 using Arduino IDE

# How to
- Make sure you have your certs and Root CA on the data folder.
- Using openssl convert cert files.
- Upload files to SPIFFS by clicking "Tools -> ESP8266 Sketch Data Upload"
- Change your WiFi SSD and password 
- Run it!

ESP8266 due to his low ram can have some hard time when running. Just try a few times until it works. Make sure your things have the righ permissions on their certificates.
Use AWS IoT console to test if your messages are being sent.

# References 
https://gist.github.com/eLement87/133cddc5bd0472daf5cb35a20bfd811e
