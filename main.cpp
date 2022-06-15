#include "mbed.h"
#include "TextLCD.h" 
#include "MCP23017.h" 
#include <string>
//mbed DCC Model Train Demo

#define MAXIMUM_BUFFER_SIZE
 
DigitalOut Track(p21); //Digital output bit used to drive track power via H-bridge
DigitalIn D21(p7);
DigitalIn D22(p6);
DigitalIn D20(p5);
InterruptIn int0(p11), int1(p12); 
TextLCD lcd_screen(p30, p29, p28, p27, p26, p25); 
I2C i2c(p9, p10); 
MCP23017 *mcp; 
BufferedSerial pc(USBTX, USBRX);

int loopATrigger = 0;
int loopBTrigger = 0;

// Keeping track of the train 1
int train1DetectorCurrentHit = 0;
int train1DetectorPreviousHit = 0;

// Keeping track of the train 2
int train2DetectorCurrentHit = 0;
int train2DetectorPreviousHit = 0;
// unsigned int DCCaddress = 0x02; // light red train
// unsigned int DCCaddress_dark_red = 0x01; // dark red train
// unsigned int DCCaddress_new_guy = 0x03; // new red train
unsigned int oneTrain = 0x02;
unsigned int secondTrain = 0x03;

void DCC_send_command(unsigned int address, unsigned int inst, unsigned int repeat_count)
{
    unsigned __int64 command = 0x0000000000000000; // __int64 is the 64-bit integer type
    unsigned __int64 temp_command = 0x0000000000000000;
    unsigned __int64 prefix = 0x3FFF; // 14 "1" bits needed at start
    unsigned int error = 0x00; //error byte
    //calculate error detection byte with xor
    error = address ^ inst;
    //combine packet bits in basic DCC format
    command = (prefix<<28)|(address<<19)|(inst<<10)|((error)<<1)|0x01;
    //printf("\n\r %llx \n\r",command);
    int i=0;
//repeat DCC command lots of times
    while(i < repeat_count) {
        temp_command = command;
//loops through packet bits encoding and sending out digital pulses for a DCC command
        for (int j=0; j<64; j++) {
            if((temp_command&0x8000000000000000)==0) { //test packet bit
                //send data for a "0" bit
                Track=0;
                wait_us(100);
                Track=1;
                wait_us(100);
                //printf("0011");
            } else {
                //send data for a "1"bit
                Track=0;
                wait_us(58);
                Track=1;
                wait_us(58);
                //printf("01");
            }
            // next bit in packet
            temp_command = temp_command<<1;
        }
        i++;
    }
}

void printLCD(string message) {
    lcd_screen.cls(); 
    lcd_screen.printf("%s\n", message.c_str());
}

void init_mcp() { 
    // Initialisation of MCP registers, documentation on registers is available at 
    mcp = new MCP23017(i2c, 0x40); 
    mcp->_write(IODIRA, (unsigned char )0xff); 
    mcp->_write(IODIRB, (unsigned char )0xff); 
    mcp->_write(IPOLA, (unsigned char )0x00); 
    mcp->_write(IPOLB, (unsigned char )0x00); 
    mcp->_write(DEFVALA, (unsigned char )0xff); 
    mcp->_write(DEFVALB, (unsigned char )0xff); 
    mcp->_write(INTCONA, (unsigned char )0xff); 
    mcp->_write(INTCONB, (unsigned char )0xff); 
    mcp->_write(IOCONA, (unsigned char )0x2); 
    mcp->_write(IOCONB, (unsigned char )0x2); 
    mcp->_write(GPPUA, (unsigned char )0xff); 
    mcp->_write(GPPUB, (unsigned char )0xff); 
} 

void on_int0_change() { 
    loopATrigger = 1;
} 
void on_int1_change() { 
    loopBTrigger = 1;
} 

void init() { 
    // Clear current interrupts 
    mcp->_read(GPIOA); 
    mcp->_read(GPIOB); 
    // Register callbacks 
    int0.fall(&on_int0_change); 
    int1.fall(&on_int1_change); 
    // Enable interrupts on the MCP 
    mcp->_write(GPINTENA, (unsigned char )0xff); 
    mcp->_write(GPINTENB, (unsigned char )0xff); 
    // Ready to go! 
} 

void DCC_send_command_dark_red(unsigned int address, unsigned int inst, unsigned int repeat_count) 
{     
    unsigned __int64 new_command = 0x3FFF00803FC03FD; 
    unsigned __int64 temp_command = 0x00; 

    int i=0; 
//repeat DCC command lots of times  
 
    while(i < repeat_count) { 
        temp_command = new_command; 
//loops through packet bits encoding and sending out digital pulses for a DCC command 
        for (int j=0; j<64; j++) { 
            if((temp_command&0x8000000000000000)==0) { //test packet bit 
                //send data for a "0" bit 
                Track=0; 
                wait_us(100); 
                Track=1; 
                wait_us(100); 
                //printf("0011"); 
            } else { 
                //send data for a "1"bit 
                Track=0; 
                wait_us(58); 
                Track=1; 
                wait_us(58); 
                //printf("01"); 
            } 
            // next bit in packet 
            temp_command = temp_command<<1; 
        } 
        i++; 
    } 
} 

void startTrains(
        unsigned int oneTrainAddress,
        unsigned int secondTrainAddress,
        unsigned int oneTrainSpeed,
        unsigned int secondTrainSpeed,
        int repeatCountFirst,
        int reapeatCountSecond
        ) {
    DCC_send_command(oneTrainAddress, oneTrainSpeed, repeatCountFirst);
    DCC_send_command(secondTrainAddress, secondTrainSpeed, reapeatCountSecond);
}

void startOneTrain(unsigned int oneTrainAddress, unsigned int oneTrainSpeed, int repeatCount) {
    DCC_send_command(oneTrainAddress, oneTrainSpeed, repeatCount);
}

void readMCPA() {
    int sensor_data = mcp->_read(INTCAPA);
    switch(sensor_data) {
        case 0xfe:
            if (train1DetectorPreviousHit == 0xfd) {
                train1DetectorPreviousHit = 0xfe;
                startOneTrain(oneTrain, 0x72, 5);
                // train 1 hit this
            } else if (train2DetectorPreviousHit == 0xfd) {
                train2DetectorPreviousHit = 0xfe;
                startOneTrain(secondTrain, 0x72, 5);
                // train 2 hit this
            }
            printf("GOT INT0 (0x%x)\n", sensor_data);
            // 3 slowdown
            break;
        case 0xfd:
            // Slowdown 2
            if (train1DetectorPreviousHit == 0xfe) {
                train1DetectorPreviousHit = 0xfd;
                startOneTrain(oneTrain, 0x65, 5); // ToDo slowdown before stop ??
                // train 1 hit this
            } else if (train2DetectorPreviousHit == 0xfe) {
                train2DetectorPreviousHit = 0xfd;
                startOneTrain(secondTrain, 0x65, 5); // ToDo slowdown before stop ??
                // train 2 hit this
            }
            // 4 slowdown
            printf("GOT INT0 (0x%x)\n", sensor_data);
            break;
        case 0xfb:
            // STOP for 10 loops
            if (train1DetectorPreviousHit == 0x7f || train1DetectorPreviousHit == 0xfd) {
                train1DetectorPreviousHit = 0xfb;
                startOneTrain(oneTrain, 0x63, 10); // complete stop
                startOneTrain(oneTrain, 0x72, 5); // start slowly to reach the next sensor
                // train 1 hit this
            } else if (train2DetectorPreviousHit == 0x7f || train2DetectorPreviousHit == 0xfd) {
                train2DetectorPreviousHit = 0xfb;
                startOneTrain(secondTrain, 0x63, 10); // complete stop
                startOneTrain(secondTrain, 0x72, 5); // start slowly to reach the next sensor
                // train 2 hit this
            }
            // complete stop for 5 loops
            printf("GOT INT0 (0x%x)\n", sensor_data);
            break;
        case 0xf7:
            if (train1DetectorPreviousHit == 0xbf) {
                train1DetectorPreviousHit = 0xf7;
                startOneTrain(oneTrain, 0x68, 5);
                // train 1 hit this
            } else if (train2DetectorPreviousHit == 0xbf) {
                train2DetectorPreviousHit = 0xf7;
                startOneTrain(secondTrain, 0x68, 5);
                // train 2 hit this
            }
            // speed up 1
            printf("GOT INT0 (0x%x)\n", sensor_data);
            break;
        case 0xef:
            //ToDo CHECK WHICH ONE HIT D21 OR D2
            if (train1DetectorPreviousHit == 0xbf || train1DetectorPreviousHit == 0x7f) {
                train1DetectorPreviousHit = 0xef;
                startOneTrain(oneTrain, 0x68, 5);
                // train 1 hit this
            } else if (train2DetectorPreviousHit == 0xbf || train2DetectorPreviousHit == 0x7f) {
                train2DetectorPreviousHit = 0xef;
                startOneTrain(secondTrain, 0x68, 5);
                // train 2 hit this
            }
            // speed up 1
            printf("GOT INT0 (0x%x)\n", sensor_data);
            break;
        case 0xdf:
            // doesn't exist
            break;
        case 0xbf:
            // doesn't exist
            printf("GOT INT0 (0x%x)\n", sensor_data);
            break;
        case 0x7f:
            //ToDo Check which train hit D8
            if (train1DetectorPreviousHit == 0xfd || train1DetectorPreviousHit == 0xef || train1DetectorPreviousHit == 0xf7) {
                train1DetectorPreviousHit = 0x7f;
                startOneTrain(oneTrain, 0x6e, 5);
                // train 1 hit this
            } else if (train2DetectorPreviousHit == 0xfd || train2DetectorPreviousHit == 0xef|| train2DetectorPreviousHit == 0xf7) {
                train2DetectorPreviousHit = 0x7f;
                startOneTrain(secondTrain, 0x6e, 5);
                // train 2 hit this
            }
            // speed up 4
            printf("GOT INT0 (0x%x)\n", sensor_data);
            break;
        default:
            printf("NO");
            break;
            // Probably show that sensor was hit but train position was lost?
            // Maybe need to check what was the previous previous position and then guess?
        }
    loopATrigger = 0;
}

void readMCPB() {
    int sensor_data = mcp->_read(INTCAPB);
    // lcd_screen.cls(); 
    // lcd_screen.printf("GOT INT0 (0x%x)\n", sensor_data);
    switch(sensor_data) {
        case 0xfe:
            //doesn't exist
            printf("GOT INT1 (0x%x)\n", sensor_data);
            break;
        case 0xfd:
            // 2 slowdown
            if (train1DetectorPreviousHit == 0xf7 || train1DetectorPreviousHit == 0x7f) {
                train1DetectorPreviousHit = 0xfd;
                startOneTrain(oneTrain, 0x68, 5); // SPEED up
                // train 1 hit this
            } else if (train2DetectorPreviousHit == 0xf7 || train2DetectorPreviousHit == 0x7f) {
                train2DetectorPreviousHit = 0xfd;
                startOneTrain(secondTrain, 0x68, 5); // SPEED up
                // train 2 hit this
            }
            printf("GOT INT1 (0x%x)\n", sensor_data);
            break;
        case 0xfb:
            if (train1DetectorPreviousHit == 0xef || train1DetectorPreviousHit == 0x7f) {
                train1DetectorPreviousHit = 0xfb;
                startOneTrain(oneTrain, 0x72, 5); // very slow
                // train 1 hit this
            } else if (train2DetectorPreviousHit == 0xef || train2DetectorPreviousHit == 0x7f) {
                train2DetectorPreviousHit = 0xfb;
                startOneTrain(secondTrain, 0x72, 5); // very slow
                // train 2 hit this
            }
            printf("GOT INT1 (0x%x)\n", sensor_data);
            break;
        case 0xf7:
            if (train1DetectorPreviousHit == 0xef || train1DetectorPreviousHit == 0xf7) {
                train1DetectorPreviousHit = 0xf7;
                startOneTrain(oneTrain, 0x72, 5); // very slow
                // train 1 hit this
            } else if (train2DetectorPreviousHit == 0xef || train2DetectorPreviousHit == 0xf7) {
                train2DetectorPreviousHit = 0xf7;
                startOneTrain(secondTrain, 0x72, 5); // very slow
                // train 2 hit this
            }
            printf("GOT INT1 (0x%x)\n", sensor_data);
            break;
        case 0xef:
            // speed up 1
            if (train1DetectorPreviousHit == 0xf7 || train1DetectorPreviousHit == 0xdf) {
                train1DetectorPreviousHit = 0xef;
                startOneTrain(oneTrain, 0x63, 10); // complete stop
                startOneTrain(oneTrain, 0x72, 5); // start slowly to reach the next sensor
                // train 1 hit this
            } else if (train2DetectorPreviousHit == 0xf7 || train2DetectorPreviousHit == 0xdf) {
                train2DetectorPreviousHit = 0xef;
                startOneTrain(secondTrain, 0x63, 10); // complete stop
                startOneTrain(secondTrain, 0x72, 5); // start slowly to reach the next sensor
                // train 2 hit this
            }
            printf("GOT INT1 (0x%x)\n", sensor_data);
            break;
        case 0xdf:
            if (train1DetectorPreviousHit == 0xef || train1DetectorPreviousHit == 0xfe) {
                train1DetectorPreviousHit = 0xdf;
                startOneTrain(oneTrain, 0x72, 5);
                // train 1 hit this
            } else if (train2DetectorPreviousHit == 0xef || train2DetectorPreviousHit == 0xfe) {
                train2DetectorPreviousHit = 0xdf;
                startOneTrain(secondTrain, 0x72, 5);
                // train 2 hit this
            }
            // speed up 6
            printf("GOT INT1 (0x%x)\n", sensor_data);
            break;
        case 0xbf:
            if (train1DetectorPreviousHit == 0xfb || train1DetectorPreviousHit == 0xf7 || train1DetectorPreviousHit == 0x7f) {
                train1DetectorPreviousHit = 0xbf;
                startOneTrain(oneTrain, 0x72, 5); // very slow
                // train 1 hit this
            } else if (train2DetectorPreviousHit == 0xfb || train2DetectorPreviousHit == 0xf7 || train1DetectorPreviousHit == 0x7f) {
                train2DetectorPreviousHit = 0xbf;
                startOneTrain(secondTrain, 0x72, 5); // very slow
                // train 2 hit this
            }
            // speed up 5
            printf("GOT INT1 (0x%x)\n", sensor_data);
            break;
        case 0x7f:
            // speed up 4
            if (train1DetectorPreviousHit == 0xfd || train1DetectorPreviousHit == 0xfb) {
                train1DetectorPreviousHit = 0x7f;
                startOneTrain(oneTrain, 0x72, 5); // very slow
                // train 1 hit this
            } else if (train2DetectorPreviousHit == 0xfd || train2DetectorPreviousHit == 0xfb) {
                train2DetectorPreviousHit = 0x7f;
                startOneTrain(secondTrain, 0x72, 5); // very slow
                // train 2 hit this
            }
            printf("GOT INT1 (0x%x)\n", sensor_data);
            break;
        default:
            printf("NO2");
            break;
            // code block
        }

    loopBTrigger = 0;
}

void findMyTrain () {
    if (loopATrigger == 1) {
        readMCPA();
        loopATrigger = 0;
    }
    if (loopBTrigger == 1) {
        readMCPB();
        loopBTrigger = 0;
    }
}

void findInitialTrain1Position () {
    if (loopATrigger == 1) {
        int sensor_data = mcp->_read(INTCAPA);
        train1DetectorPreviousHit = sensor_data;
        loopATrigger = 0;
    }
    if (loopBTrigger == 1) {
        int sensor_data = mcp->_read(INTCAPB);
        train1DetectorPreviousHit = sensor_data;
        loopBTrigger = 0;
    }
}

void findInitialTrain2Position () {
    if (loopATrigger == 1) {
        int sensor_data = mcp->_read(INTCAPA);
        train2DetectorPreviousHit = sensor_data;
        loopATrigger = 0;
    }
    if (loopBTrigger == 1) {
        int sensor_data = mcp->_read(INTCAPB);
        train2DetectorPreviousHit = sensor_data;
        loopBTrigger = 0;
    }
}

//DCC train demo turns on headlight, dims headlight, and moves back and forth at half speed forever
int main()
{
    //typical out of box default engine DCC address is 3 (at least for Bachmann trains)
    //Note: A DCC controller can reprogram the address whenever needed
    // unsigned int DCCaddress = 0x02; // light red train
    // unsigned int DCCaddress_dark_red = 0x01; // dark red train
    // unsigned int DCCaddress_new_guy = 0x03; // new red train
    //see http://www.nmra.org/standards/DCC/standards_rps/RP-921%202006%20Aug%2021.pdf
    //01DCSSSS for speed, D is direction (fwd=1 and rev=0), C is speed(SSSSC) LSB
    unsigned int DCCinst_forward = 0x64; //forward half speed
    unsigned int DCCinst_reverse = 0x48; //reverse half speed
    //100DDDDD for basic headlight functions
    unsigned int DCC_func_lighton = 0x90; //F0 turns on headlight function
    unsigned int DCC_func_dimlight = 0x91; //F0 + F1 dims headlight
    //
    //Basic DCC Demo Commands
    // DCC_send_command(DCCaddress,DCC_func_lighton,200); // turn light on full
    // DCC_send_command(DCCaddress,DCC_func_dimlight,200); //dim light
    // DCC_send_command(DCCaddress,DCC_func_lighton,200);  //light full again
    printLCD("LCD");
    //DCC_send_command_dark_red(DCCaddress_dark_red,DCCinst_forward,400);
    //DCC_send_command_dark_red(DCCaddress_dark_red,DCCinst_forward,400);
    char buf[MAXIMUM_BUFFER_SIZE] = {0};

    bool redTrainRun = true;
    bool analogTrainRun = true;
    bool newTrainRun = false;
    if (redTrainRun) {
        oneTrain = 0x03;
    }
    // if (newTrainRun) {
    //     secondTrain = 0x03;
    // }
    // Initialisation order
    init_mcp();
    init();
    int loop = 0;
    if (redTrainRun == true && newTrainRun == true) {
        startTrains(oneTrain, secondTrain, DCCinst_forward, DCCinst_forward, 5, 5);
    } else {
        startOneTrain(oneTrain, DCCinst_forward, 3);
    }
    if (train1DetectorCurrentHit == 0) {
        findInitialTrain1Position();
    }
    if (train1DetectorCurrentHit == 0) {
        startOneTrain(oneTrain, DCCinst_forward, 2);
        findInitialTrain1Position();
    }
    // if (train2DetectorCurrentHit == 0) {
    //     findInitialTrain2Position();
    // }
    // if (train2DetectorCurrentHit == 0) {
    //     startOneTrain(oneTrain, DCCinst_forward, 5);
    //     findInitialTrain2Position();
    // }
    if (false) {
        printLCD("Trains not found try again");
        printf("Train not found");
    } else {
        while(true) {
            //startOneTrain(oneTrain, 0x64, 1);
            if (train1DetectorPreviousHit != 0) {
                printf("GOT INT0 (0x%x)\n", train1DetectorPreviousHit);
            }
            // int sensor_data = mcp->_read(INTCAPB);
            // lcd_screen.cls(); 
            // lcd_screen.printf("GOT INT0 (0x%x)\n", sensor_data);
            // wait_us(2000);
            findMyTrain();
            loop++;
        }
    }
}
 