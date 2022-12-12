# Smart_mikrowelle
![alt text](https://m.media-amazon.com/images/I/41DaUCtyl8L._AC_SL1002_.jpg)

Only the pcb that controls it was replaced, all safety and hw features remained intact. ( for example: thermal switch,lamp,cooling ventillator, plate rotating motor)

Parts:
-A 128*64 oled display
-4channel relay
-Two buttons
-Esp32 dual core MCU

Features [x]:
-NTP time and date displaying from wifi
-Working without Wifi or NTP Time connection

-Two buttons
  --With the firs u can add 30s to the time (after 8min it will be adding 1 minutes until it reaches 30min after that it goes back to 0)
  --The second button has multi pourpuse if the micro is not operating it is a Popcorn "button" (automatic 3min timer) if it is working this button serves as a stopp button
  --IF both is pressed it restarts
 
 -IF the door is open while functioning all the operation stopps and resets, if it opens up while time adding it waits until it closed with a message on display 
 -IF the door is opened the lamp turns on
  
 -Sleep after X hours without operation the first button wakes it up
 -OTA w/ password



Further development opportunities [ ]:
-Better multitasking
-Better code optimalization

Pics: 

![20221212_142618](https://user-images.githubusercontent.com/37541810/207056705-b00b36d4-cf03-4e0e-8007-e8fc40055d9f.jpg)
![20221212_142643](https://user-images.githubusercontent.com/37541810/207056708-7504eca1-efe4-401b-b4da-4e904fdccc01.jpg)
