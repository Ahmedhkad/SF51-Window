/* special specs of NodeMCU pins */ 
//#define       16  //D0     //-pin is High on boot
//#define       5   //D1     //good
//#define       4   //D2     //good
//#define       0   //D3     //-boot failure if pulled low
//#define       2   //D4     //-boot failure if pulled low
//#define       14  //D5     //good  
//#define       12  //D6     //good  
//#define       13  //D7     //good
//#define       15  //D8     //-boot failure if pulled high !! must be low as in LDR with 4.7K R to GND
//#define       3     //RX     //-pin is High on boot , Output reversed

/* I/O pins selection of NodeMCU */ 

#define IRtransmit              4   //D2

#define SilderEnable            13  //D7
#define SliderDir1              12  //D6
#define SliderDir2              5   //D1 