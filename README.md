It is a DIY coffee scale, inspired by the genius at Decent. The company that makes open source produce for coffee machine equipment and recently scale. I had made previous DIY scale for grinder that is inspired by an example or learning blog from Adafruit. This is basically build based on that code to begin with but I had to remove a few things because the earlier grinder scale is based on the M4 chip, and not ESP32S3 so some of the code is different.
I use Arduino IDE to make the program because it is the one I am most comfortable with and there are lots of helps available online and library information and learning curve is pretty short, at least for me.
Feel free to modify add improvement to this. I left some of the original coding from the grinder scale as you may wonder why I have those in there and just commented them out. Eventually I will clean them and post a clean version. However, rightnow the aim is to get it finished before I go out of the country and I need to use it in the office.
I would have put larger battery but will need a larger case maybe. By the way, the scad file is there if you want to modify or adjust it. I use the YAPP library to create custom box, and then BOSL2 to create the larger screw hole to connect the loadcell.
All the materials I use are below, I bought them from Adafruit, to support them as I always learn so much from them. Please support them as well.
Pay attention to the library and IDE version. This is the downside of programming using Arduino IDE. If someone update their library, this code may not work unless you find the same old version. I've learned this the hardway so I've started putting notes in my code to indicate which version and IDE so I remember to go back and find the old one. Or just never update the library or board module, IDE unless you really need to.
The main 4 components are :
  - load cells
  - NAU7802 Analog to Digital converter to read the load cell
  - Adalogger (has RTC though I am not using it, has SD card storage)
  - Feather ESP32S3  Reverse TFT

None of these software has to be exact. The load cells can be upgrade to 1 kG. I also upload the calibration code, follow that if you set yours the first time.
You can put as many loadcells as you want, and each has to has its own NAU7802. You can use that special connector to daisy chain them.
The Adalogger is used because I want the ability to store settings but it can be easily expanded to store your data too. Maybe someday I will add this. I don't have a use for this data so I skip it right now.
The ESP32S3 can be any model, this one is from Adafruit, I really like the platform and it has a built in buttons, and TFT display. It keeps your hardware cute and tiny. Like me!
