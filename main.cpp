// ##################################################
// # PROJECT SYSTEM TESTING (2022) - Train On Rails #
// ##################################################
// Choo-choo group: Daumantas Patapas, Maria __, Carolina Oliveira, Ander Eguiluz

// LIBRARIES
#include "mbed.h"
#include "TextLCD.h" 
#include "MCP23017.h" 
#include <string>

// #define MAXIMUM_BUFFER_SIZE

// INPUTS
DigitalIn D20(p5), D22(p6), D21(p7), switch1(p13), switch2(p14), button1(p15), button2(p16);
InterruptIn int0(p11), int1(p12), sw1(p13), sw2(p14), bt1(p15), bt2(p16);

// OUTPUTS
DigitalOut Track(p21); //Digital output bit used to drive track power via H-bridge
DigitalOut GLED(p17), RLED(p18), YLEDup(p19), YLEDdown(p20), buzzer(p23);
TextLCD lcd_screen(p30, p29, p28, p27, p26, p25);

// COMMUNICATION
I2C i2c(p9, p10); 

// EXPANDER
MCP23017 *mcp; 

// GLOBAL VARIABLES
int test = 0;

bool on_off = 0; // if 1 -> GLED on; if 0 -> RLED on
bool night_mode = 0; // if 1 -> YLEDup ON; if 0 -> YLEDdown ON
bool kill = 0; // it kills everything (GLED & RLED on until it's reset)
bool choo_choo = 0; // when pressed it activates the buzzer

int loopATrigger = 0;
int loopBTrigger = 0;

// Keeping track of the train 1
int train1DetectorCurrentHit = 0;
int train1DetectorPreviousHit = 0;

// Keeping track of the train 2
int train2DetectorCurrentHit = 0;
int train2DetectorPreviousHit = 0;

int stopAtStationA = 0;
int stopAtStationB = 0;

bool firstTrainActive = true;
bool secondTrainActive = true;

bool moveTrainOne = false;
bool moveTrainTwo = false;
// unsigned int DCCaddress = 0x02; // light red train
// unsigned int DCCaddress_dark_red = 0x01; // dark red train
// unsigned int DCCaddress_new_guy = 0x03; // new red train
unsigned int oneTrain = 0x02;
unsigned int secondTrain = 0x03;

// METHODS
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

// Interruptions
void on_int0_change() { 
    loopATrigger = 1;
} 
void on_int1_change() { 
    loopBTrigger = 1;
} 

void on_sw1_down() {  // when switch goes up
    on_off = 1;
    GLED = 1;
    RLED = 0;
} 
void on_sw2_down() { 
    night_mode = 1;
    YLEDdown = 0;
    YLEDup = 1;
}
void on_sw1_up() { 
    on_off = 0;
    RLED = 1;
    GLED = 0;
} 
void on_sw2_up() { 
    night_mode = 0;
    YLEDdown = 1;
    YLEDup = 0;        
}
void on_bt1_down() { // when it goes down
    kill = 1;
    GLED=1;
    RLED=1;    
} 
void on_bt2_down() { 
    buzzer = 1;    
}
void on_bt1_up() { 

} 
void on_bt2_up() { 
    buzzer = 0;
}
void kill_system(){
    printLCD("SYS OFF - PLEASE RESET");
    // Disabling interrupts
    int0.fall(NULL); 
    int1.fall(NULL);
    sw1.fall(NULL); 
    sw2.fall(NULL); 
    sw1.rise(NULL); 
    sw2.rise(NULL);     
    bt1.fall(NULL); 
    bt2.fall(NULL); 
    bt1.rise(NULL); 
    bt2.rise(NULL);
    printf("SYSTEM MANUALLY KILLED.\n");
    wait_us(100000);
    printf("please restart to run ");
    printf("the program again.\n");
}

void init() { 
    // Setup control box's switches
    if (switch1){
        RLED = switch1;
    }else{
        GLED = 1;
    }
    if (switch2){
        YLEDdown = switch2;
    }else{
        YLEDup = 1;
    }    
    on_off = switch1;
    night_mode = switch2;    
    // Clear current interrupts 
    mcp->_read(GPIOA); 
    mcp->_read(GPIOB); 
    // Register callbacks 
    int0.fall(&on_int0_change); 
    int1.fall(&on_int1_change); 
    sw1.fall(&on_sw1_down); 
    sw2.fall(&on_sw2_down); 
    sw1.rise(&on_sw1_up); 
    sw2.rise(&on_sw2_up);     
    bt1.fall(&on_bt1_down); 
    bt2.fall(&on_bt2_down); 
    bt1.rise(&on_bt1_up); 
    bt2.rise(&on_bt2_up);         
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

// Stop at top station
void stopTrainWhileStopA(unsigned int train) {
    if (train == oneTrain) {
        startOneTrain(oneTrain, 0x61, 10);
        firstTrainActive = false;
    } else {
        startOneTrain(secondTrain, 0x61, 10);
        secondTrainActive = false;
    }
}

// Stop at bottom station
void stopTrainWhileStopB(unsigned int train) {
    if (train == oneTrain) {
        startOneTrain(oneTrain, 0x61, 10);
        firstTrainActive = false;
    } else {
        startOneTrain(secondTrain, 0x61, 10);
        secondTrainActive = false;
    }
}

void readMCPA() {
    int sensor_dataA = mcp->_read(INTCAPA);
    printf("Switch statement A\n");
    switch(sensor_dataA) {
        case 0xfe:
            if (train1DetectorPreviousHit == 0xdf) {
                train1DetectorPreviousHit = sensor_dataA;
                startOneTrain(oneTrain, 0x76, 5);
                printf("GOT INT0 (0x%x)\n", sensor_dataA);
            } else if (train2DetectorPreviousHit == 0xdf) {
                train2DetectorPreviousHit = sensor_dataA;
                startOneTrain(secondTrain, 0x76, 7);
                printf("GOT INT0 (0x%x)\n", sensor_dataA);
            }
            break;
        case 0xfd:
            if (train1DetectorPreviousHit == 0xfe) {
                train1DetectorPreviousHit = sensor_dataA;
                startOneTrain(oneTrain, 0x67, 5);
                printf("GOT INT0 (0x%x)\n", sensor_dataA);
            } else if (train2DetectorPreviousHit == 0xfe) {
                train2DetectorPreviousHit = sensor_dataA;
                startOneTrain(secondTrain, 0x67, 7);
                printf("GOT INT0 (0x%x)\n", sensor_dataA);
            }
            break;
        case 0xfb:
            if (train1DetectorPreviousHit == 0x7f || train1DetectorPreviousHit == 0xfd) {
                train1DetectorPreviousHit = sensor_dataA;
                printf("Train at STOP B\n");
                stopTrainWhileStopB(oneTrain);
                printf("GOT INT0 (0x%x)\n", sensor_dataA);
                firstTrainActive = false;
            } else if (train2DetectorPreviousHit == 0x7f || train2DetectorPreviousHit == 0xfd) {
                train2DetectorPreviousHit = sensor_dataA;
                printf("Train at STOP B\n");
                stopTrainWhileStopB(secondTrain);
                printf("GOT INT0 (0x%x)\n", sensor_dataA);
                secondTrainActive = false;
            }
            break;
        case 0xf7:
            if (train1DetectorPreviousHit == 0xbf) {
                train1DetectorPreviousHit = sensor_dataA;
                startOneTrain(oneTrain, 0x6C, 5);
                printf("GOT INT0 (0x%x)\n", sensor_dataA);
                moveTrainTwo = true;
            } else if (train2DetectorPreviousHit == 0xbf) {
                train2DetectorPreviousHit = sensor_dataA;
                startOneTrain(secondTrain, 0x6C, 7);
                printf("GOT INT0 (0x%x)\n", sensor_dataA);
                moveTrainOne = true;
            }
            break;
        case 0xef:
            if (train1DetectorPreviousHit == 0xbf) { // TRAIN ON WRONG ROUTE
                train1DetectorPreviousHit = sensor_dataA;
                startOneTrain(oneTrain, 0x61, 5);
                printf("GOT INT0 (0x%x)\n", sensor_dataA);
            } else if (train2DetectorPreviousHit == 0xbf || train2DetectorPreviousHit == 0x7f) { // TRAIN ON WRONG ROUTE
                train2DetectorPreviousHit = sensor_dataA;
                startOneTrain(secondTrain, 0x61, 7);
                printf("GOT INT0 (0x%x)\n", sensor_dataA);
            } else if (train1DetectorPreviousHit == 0x7f) { // REACHED THE WRONG SENSOR. Let the train leave and move after it
                firstTrainActive = true;
                startOneTrain(oneTrain, 0x61, 50);
                
            } else if (train2DetectorPreviousHit == 0x7f) { // REACHED THE WRONG SENSOR. Let the train leave and move after it
                secondTrainActive = true;
                startOneTrain(secondTrain, 0x61, 50);
            }
            break;
        case 0xdf:
            // doesn't exist
            break;
        case 0xbf:
            // doesn't exist
            break;
        case 0x7f:
            if (train1DetectorPreviousHit == 0xfd || train1DetectorPreviousHit == 0xef || train1DetectorPreviousHit == 0xf7) {
                train1DetectorPreviousHit = sensor_dataA;
                startOneTrain(oneTrain, 0x6E, 5);
                printf("GOT INT0 (0x%x)\n", sensor_dataA);
            } else if (train2DetectorPreviousHit == 0xfd || train2DetectorPreviousHit == 0xef|| train2DetectorPreviousHit == 0xf7) {
                train2DetectorPreviousHit = sensor_dataA;
                startOneTrain(secondTrain, 0x6E, 7);
                printf("GOT INT0 (0x%x)\n", sensor_dataA);
            }
            break;
        default:
            printf("NO");
            break;
            // Probably show that sensor was hit but train position was lost?
        }
}

void readMCPB() {
    int sensor_data = mcp->_read(INTCAPB);
    printf("Switch statement b\n");
    switch(sensor_data) {
        case 0xfe:
            break;
        case 0xfd:
            if (train1DetectorPreviousHit == 0xf7 || train1DetectorPreviousHit == 0x7f) {
                train1DetectorPreviousHit = sensor_data;
                startOneTrain(oneTrain, 0x6B, 5);
                printf("GOT INT1 (0x%x)\n", sensor_data);
            } else if (train2DetectorPreviousHit == 0xf7 || train2DetectorPreviousHit == 0x7f) {
                train2DetectorPreviousHit = sensor_data;
                startOneTrain(secondTrain, 0x6B, 7);
                printf("GOT INT1 (0x%x)\n", sensor_data);
            }
            break;
        case 0xfb:
            if (train1DetectorPreviousHit == 0xef) {
                train1DetectorPreviousHit = sensor_data;
                startOneTrain(oneTrain, 0x74, 5);
                printf("GOT INT1 (0x%x)\n", sensor_data);
            } else if (train2DetectorPreviousHit == 0xef) {
                train2DetectorPreviousHit = sensor_data;
                startOneTrain(secondTrain, 0x74, 7);
                printf("GOT INT1 (0x%x)\n", sensor_data);
            }
            // } else if (train1DetectorPreviousHit == 0x7f) { // REACHED THE WRONG SENSOR. Let the train leave and move after it
            //     firstTrainActive = false;
            //     train1DetectorPreviousHit = sensor_data;
            //     startOneTrain(oneTrain, 0x61, 50);
            //     startOneTrain(oneTrain, 0x72, 5);
            // } else if (train2DetectorPreviousHit == 0x7f) { // REACHED THE WRONG SENSOR. Let the train leave and move after it
            //     secondTrainActive = false;
            //     train2DetectorPreviousHit = sensor_data;
            //     startOneTrain(secondTrain, 0x61, 50);
            //     startOneTrain(secondTrain, 0x74, 5);
            // }
            break;
        case 0xf7:
            if (train1DetectorPreviousHit == 0x7f) {
                train1DetectorPreviousHit = sensor_data;
                startOneTrain(oneTrain, 0x75, 5);
                printf("GOT INT1 (0x%x)\n", sensor_data);
                moveTrainTwo = true;
            } else if (train2DetectorPreviousHit == 0x7f) {
                train2DetectorPreviousHit = sensor_data;
                startOneTrain(secondTrain, 0x75, 7);
                printf("GOT INT1 (0x%x)\n", sensor_data);
                moveTrainOne = true;
            }
            printf("First train started again\n");
            break;
        case 0xef:
            if (train1DetectorPreviousHit == 0xf7 || train1DetectorPreviousHit == 0xdf) {
                train1DetectorPreviousHit = sensor_data;
                stopTrainWhileStopA(oneTrain);
                printf("GOT INT1 (0x%x)\n", sensor_data);
                firstTrainActive = false;
            } else if (train2DetectorPreviousHit == 0xf7 || train2DetectorPreviousHit == 0xdf) {
                train2DetectorPreviousHit = sensor_data;
                stopTrainWhileStopA(secondTrain);
                printf("GOT INT1 (0x%x)\n", sensor_data);
                secondTrainActive = false;
            }
            break;
        case 0xdf:
            if (train1DetectorPreviousHit == 0xef || train1DetectorPreviousHit == 0xfe) {
                train1DetectorPreviousHit = sensor_data;
                startOneTrain(oneTrain, 0x66, 5);
                printf("GOT INT1 (0x%x)\n", sensor_data);
            } else if (train2DetectorPreviousHit == 0xef || train2DetectorPreviousHit == 0xfe) {
                train2DetectorPreviousHit = sensor_data;
                startOneTrain(secondTrain, 0x66, 7);
                printf("GOT INT1 (0x%x)\n", sensor_data);
            }
            break;
        case 0xbf:
            if (train1DetectorPreviousHit == 0xfb || train1DetectorPreviousHit == 0xf7 || train1DetectorPreviousHit == 0x7f) {
                train1DetectorPreviousHit = sensor_data;
                startOneTrain(oneTrain, 0x78, 5);
                printf("GOT INT1 (0x%x)\n", sensor_data);
                moveTrainTwo = true;
            } else if (train2DetectorPreviousHit == 0xfb || train2DetectorPreviousHit == 0xf7 || train1DetectorPreviousHit == 0x7f) {
                train2DetectorPreviousHit = sensor_data;
                startOneTrain(secondTrain, 0x78, 7);
                printf("GOT INT1 (0x%x)\n", sensor_data);
                moveTrainOne = true;
            }
            break;
        case 0x7f:
            if (train1DetectorPreviousHit == 0xfd || train1DetectorPreviousHit == 0xfb) {
                train1DetectorPreviousHit = sensor_data;
                startOneTrain(oneTrain, 0x64, 5);
                printf("GOT INT1 (0x%x)\n", sensor_data);
            } else if (train2DetectorPreviousHit == 0xfd || train2DetectorPreviousHit == 0xfb) {
                train2DetectorPreviousHit = sensor_data;
                startOneTrain(secondTrain, 0x64, 7);
                printf("GOT INT1 (0x%x)\n", sensor_data);
            }
            break;
        default:
            printf("NO 2");
            break;
        }
}

void findMyTrain (bool whichTrain) {
    int triggered = 0;
    if (loopATrigger == 1) {
        printf("Loop a triggered \n");
        readMCPA();
        loopATrigger = 0;
        triggered = 1;
    }
    if (loopBTrigger == 1) {
        printf("Loop b triggered \n");
        readMCPB();
        loopBTrigger = 0;
        triggered = 1;
    }
    // if (whichTrain && triggered == 0) {
    //     startOneTrain(secondTrain, 0x63, 1);
    // } else if (whichTrain == false  && triggered == 0) {
    //     startOneTrain(oneTrain, 0x63, 1);
    // }
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

int main() {
    printLCD("Initializing system. \n");
    // char buf[MAXIMUM_BUFFER_SIZE] = {0};
    // Initialisation order: first mcp then everything else
    init_mcp();
    init();
    int loop = 0;


    while (train2DetectorPreviousHit == 0 && secondTrainActive) {
        //DCC_send_command_dark_red(secondTrain, 0x68, 10);
        if (kill == 1){
            // System KILLED
            kill_system();
            startTrains(oneTrain, secondTrain, 0x61, 0x61, 5, 5);
        }else{    
            // System looking for position train1
            wait_us(2000);
            startOneTrain(secondTrain, 0x78, 10);
            findInitialTrain2Position();
            printf("Looking for position train 2\n");
        }
    }

    while (train1DetectorPreviousHit == 0 || train2DetectorPreviousHit == 0) {
        if (kill == 1){
            // System KILLED
            kill_system();
            startTrains(oneTrain, secondTrain, 0x61, 0x61, 5, 5);
        }else{    
            // System looking for position train2
            wait_us(2000);
            startOneTrain(secondTrain, 0x78, 5);
            findInitialTrain2Position();
            startOneTrain(oneTrain, 0x68, 5);
            findInitialTrain1Position();
            printf("Looking for position\n");
        }
    }

    while (true) {
        if (kill == 1){
            // System KILLED
            kill_system();
            startTrains(oneTrain, secondTrain, 0x61, 0x61, 5, 5);
        }else{
            if (on_off == 0) {
                // System STOPPED
                startTrains(oneTrain, secondTrain, 0x61, 0x61, 5, 5);
                printLCD("trains OFF");
            } else {
                // System WORKING
                if (firstTrainActive == true) {
                    findMyTrain(false);
                    loop++;
                }
                if (secondTrainActive == true) {
                    findMyTrain(true);
                    loop++;
                }
                if (firstTrainActive == false) {
                    printf("First train off\n");
                    startOneTrain(oneTrain, 0x60, 5);
                }
                if (secondTrainActive == false) {
                    wait_us(2000);
                    printf("Second train off\n");
                    startOneTrain(secondTrain, 0x60, 5);
                }

                if (moveTrainOne == true) {
                    moveTrainOne = false;
                    printf("First train started moving\n");
                    firstTrainActive = true;
                    startOneTrain(oneTrain, 0x64, 7);
                }
                if (moveTrainTwo == true) {
                    moveTrainTwo = false;
                    printf("second train started moving\n");
                    secondTrainActive = true;
                    startOneTrain(secondTrain, 0x68, 10);
                }
            }
        }
    }

        //typical out of box default engine DCC address is 3 (at least for Bachmann trains)
        //Note: A DCC controller can reprogram the address whenever needed
        // unsigned int DCCaddress = 0x02; // light red train
        // unsigned int DCCaddress_dark_red = 0x01; // dark red train
        // unsigned int DCCaddress_new_guy = 0x03; // new red train
        //see http://www.nmra.org/standards/DCC/standards_rps/RP-921%202006%20Aug%2021.pdf
        //01DCSSSS for speed, D is direction (fwd=1 and rev=0), C is speed(SSSSC) LSB
        // unsigned int DCCinst_forward = 0x64; //forward half speed
        // unsigned int DCCinst_reverse = 0x48; //reverse half speed
        //100DDDDD for basic headlight functions
        // unsigned int DCC_func_lighton = 0x90; //F0 turns on headlight function
        // unsigned int DCC_func_dimlight = 0x91; //F0 + F1 dims headlight
        //
        //Basic DCC Demo Commands
        // DCC_send_command(DCCaddress,DCC_func_lighton,200); // turn light on full
        // DCC_send_command(DCCaddress,DCC_func_dimlight,200); //dim light
        // DCC_send_command(DCCaddress,DCC_func_lighton,200);  //light full again

}