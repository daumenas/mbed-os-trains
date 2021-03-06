// ##################################################
// # PROJECT SYSTEM TESTING (2022) - Train On Rails #
// ##################################################
// Choo-choo group: Daumantas Patapas, Maria Adamidou, Carolina Oliveira, Ander Eguiluz

// LIBRARIES
#include "mbed.h"
#include "TextLCD.h" 
#include "MCP23017.h" 
#include <string>

#define MAXIMUM_BUFFER_SIZE

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

bool on_off = 0; // if 1 -> GLED on; if 0 -> RLED on
bool on_off_mss = 0;
bool night_mode = 0; // if 1 -> YLEDup ON; if 0 -> YLEDdown ON
bool kill = 0; // it kills everything (GLED & RLED on until it's reset)
bool kill_mss = 0; 
bool choo_choo = 0; // when pressed it activates the buzzer

// Signals address
unsigned int lights_address = 0x42;
unsigned int switch_address = 0x06;

/* Speeds
// ------
0 - stop        8 - speed3
1 - e-stop      9 - speed4
2 - speed1      10- speed5
3 - speed2      11- speed6
4 - speed1      12- speed7
5 - speed2      13- speed8
6 - speed1      14- speed9
7 - speed2      15- speed10 (MAX)
*/
unsigned int f_speed[16] = {0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6A, 0x6B, 0x6C, 0x6D, 0x6E, 0x6F}; // Forward NO lights
unsigned int f_l_speed[16] = {0x70,0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7A, 0x7B, 0x7C, 0x7D, 0x7E, 0x7F}; // Forward lights
unsigned int b_speed[16] = {0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F}; // Bacward NO lights
unsigned int b_l_speed[16] = {0x50,0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5A, 0x5B, 0x5C, 0x5D, 0x5E, 0x5F}; // Backward NO lights

unsigned int speed[16] = {0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6A, 0x6B, 0x6C, 0x6D, 0x6E, 0x6F}; // Forward NO lights

int loopATrigger = 0;
int loopBTrigger = 0;

// Keeping track of the train 1
int train1DetectorCurrentHit = 0;
int train1DetectorPreviousHit = 0xfb;
int train1DetectorNextHit = 0xbf;

// Keeping track of the train 2
int train2DetectorCurrentHit = 0;
int train2DetectorPreviousHit = 0xf7;
int train2DetectorNextHit = 0xef;

int lastCommandSpeedTrain1 = 4;
int lastCommandRepeatTrain1 = 1;

int lastCommandSpeedTrain2 = 4;
int lastCommandRepeatTrain2 = 1;

int stopAtStationA = 0;
int stopAtStationB = 0;

int indexMissedSensors = 0;

bool firstTrainActive = true;
bool secondTrainActive = true;

bool moveTrainOne = false;
bool moveTrainTwo = false;

bool isRedLight = false;
bool isFirstTrain = false;

bool switchTrack = false;
int currentLight = 0;
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

// MCP INITIALIZATION
/* Write a register on an I2C device */
void mcpWriteReg(uint8_t address, uint8_t reg, uint8_t data)
{
    char cmd[2];
    
    cmd[0] = reg;
    cmd[1] = data;
    i2c.write(address, cmd, 2);   // Write 2 bytes to device on specified address
}


/* Read a register on a I2C device */
uint8_t mcpReadReg(uint8_t address, uint8_t reg)
{
    char cmd[1];
    
    cmd[0] = reg;
    i2c.write(address, cmd, 1);     // Write address 
    i2c.read(address, cmd, 1);      // Read value (one byte)
    
    return cmd[0];                  // Return the read value
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

/* Init of the MCP23017 for the LED railway signals. These are also active low */
void initMcp1()
{
    // Set pin direction for port A and B
    mcpWriteReg(lights_address, IODIRA, 0x00);     // All outputs
    mcpWriteReg(lights_address, IODIRB, 0x00);     // All outputs
    mcpWriteReg(lights_address, OLATA, 0xff);      // All signals off
    mcpWriteReg(lights_address, OLATB, 0xff);      // All signals off
}

/* Turns off every signal in the system */
void turnOffSignals(){
    mcpWriteReg(lights_address,  OLATA, 0b11111111 );
    mcpWriteReg(lights_address,  OLATB, 0b11111111 );
}

/* Turns on a given signal in the system
int signal => number of the signal to control (0-7)
int mode => 
0 = G & R off;
1 = G on & R off;
2 = G off & R on;
3 = G & R on;
*/
void turnOnSignal(int signal, int mode){
    uint16_t current_OLATA = mcpReadReg(lights_address, OLATA);
    uint16_t current_OLATB = mcpReadReg(lights_address, OLATB);
    uint16_t new_OLATA;
    uint16_t new_OLATB;
    switch(mode){
        case 0: // Both OFF -> Set 1 in both G & R
        // Special operation for setting specific bits for specific signals
            if (signal > 0 && signal <= 3){ // OLATA
                new_OLATA = current_OLATA |= 1UL << (signal*2);
                new_OLATA = new_OLATA |= 1UL << (signal*2 + 1);
                new_OLATB = current_OLATB;
            } else if (signal > 3 && signal <= 7){ // OLATB
                new_OLATB = current_OLATB |= 1UL << ((signal-4)*2);
                new_OLATB = new_OLATB |= 1UL << ((signal-4)*2 + 1);
                new_OLATA = current_OLATA;
            }
            printf("SETTING G ON &R OFF lights ON for signal %d\n", signal);            
            mcpWriteReg(lights_address, OLATA, new_OLATA);
            mcpWriteReg(lights_address, OLATB, new_OLATB);
            break;
        case 1: // G ON & R OFF -> 0 for G, 1 for R
        // Clearing bit of signal (0) for G
            if (signal > 0 && signal <= 3){ // OLATA
                new_OLATA = current_OLATA &= ~(1UL << (signal*2));
                new_OLATA = new_OLATA |= 1UL << (signal*2 + 1);
                new_OLATB = current_OLATB;
            } else if (signal > 3 && signal <= 7){ // OLATB
                new_OLATB = current_OLATB &= ~(1UL << ((signal-4)*2));
                new_OLATB = new_OLATB |= 1UL << ((signal-4)*2 + 1);
                new_OLATA = current_OLATA;
            }
            printf("SETTING G ON &R OFF lights ON for signal %d\n", signal);            
            mcpWriteReg(lights_address, OLATA, new_OLATA);
            mcpWriteReg(lights_address, OLATB, new_OLATB);
            break;
        case 2: // G OFF & R ON -> 1 for G, 0 for R
        // Clearing bit of signal (0) for R
            if (signal > 0 && signal <= 3){ // OLATA
                new_OLATA = current_OLATA |= 1UL << (signal*2);
                new_OLATA = new_OLATA &= ~(1UL << (signal*2 + 1));
                new_OLATB = current_OLATB; 
            } else if (signal > 3 && signal <= 7){ // OLATB
                new_OLATB = current_OLATB |= 1UL << ((signal-4)*2); 
                new_OLATB = new_OLATB &= ~(1UL << ((signal-4)*2 + 1));
                new_OLATA = current_OLATA;
            }
            printf("SETTING G OFF &R lights ON for signal %d\n", signal);
            mcpWriteReg(lights_address, OLATA, new_OLATA);
            mcpWriteReg(lights_address, OLATB, new_OLATB);
            break;
        case 3: // Both ON -> 0 for G & R
        // Clearing bit of signal (0) for G & R
            if (signal > 0 && signal <= 3){ // OLATA
                new_OLATA = current_OLATA &= ~(1UL << (signal*2)); 
                new_OLATA = new_OLATA &= ~(1UL << (signal*2 + 1));
                new_OLATB = current_OLATB;
            } else if (signal > 3 && signal <= 7){ // OLATB
                new_OLATB = current_OLATB &= ~(1UL << ((signal-4)*2)); 
                new_OLATB = new_OLATB &= ~(1UL << ((signal-4)*2 + 1));
                new_OLATA = current_OLATA;
            }
            printf("SETTING G&R lights ON for signal %d\n", signal);
            mcpWriteReg(lights_address, OLATA, new_OLATA);
            mcpWriteReg(lights_address, OLATB, new_OLATB);
            break;
        default:
            break;
    }
}

/* Reads a given signal in the system
int signal => number of the signal to read
RETURNS int mode => 
0 = G & R off;
1 = G on & R off;
2 = G off & R on;
3 = G & R on;
*/
int readSignal(int signal){
    uint16_t current_OLATA = mcpReadReg(lights_address, OLATA);
    uint16_t current_OLATB = mcpReadReg(lights_address, OLATB);
    bool g_bit;
    bool r_bit;
    switch(signal){
        case 0:
            g_bit = (current_OLATA >> (signal*2)) & 1U;
            r_bit = (current_OLATA >> (signal*2 + 1)) & 1U;
            break;
        case 1:
            g_bit = (current_OLATA >> (signal*2)) & 1U;
            r_bit = (current_OLATA >> (signal*2 + 1)) & 1U;
            break;
        case 2:
            g_bit = (current_OLATA >> (signal*2)) & 1U;
            r_bit = (current_OLATA >> (signal*2 + 1)) & 1U;
            break;
        case 3:
            g_bit = (current_OLATA >> (signal*2)) & 1U;
            r_bit = (current_OLATA >> (signal*2 + 1)) & 1U;
            break;
        case 4:
            g_bit = (current_OLATB >> ((signal-4)*2)) & 1U;
            r_bit = (current_OLATB >> ((signal-4)*2 + 1)) & 1U;
            break;
        case 5:
            g_bit = (current_OLATB >> ((signal-4)*2)) & 1U;
            r_bit = (current_OLATB >> ((signal-4)*2 + 1)) & 1U;
            break;
        case 6:
            g_bit = (current_OLATB >> ((signal-4)*2)) & 1U;
            r_bit = (current_OLATB >> ((signal-4)*2 + 1)) & 1U;
            break;
        case 7:
            g_bit = (current_OLATB >> ((signal-4)*2)) & 1U;
            r_bit = (current_OLATB >> ((signal-4)*2 + 1)) & 1U;
            break;
        default:
            break;
    }
// 0 is ON and 1 is OFF
    if (g_bit){
        if (r_bit){
            return 0;
        } else {
            return 2;
        }
    } else {
        if (r_bit){
            return 1;
        } else {
            return 3;
        }
    }
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
    on_off_mss = 1;
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
    kill_mss = 1;
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
    if (kill_mss){
        turnOnSignal(0, 3);
        turnOnSignal(1, 3);
        turnOnSignal(2, 3);
        turnOnSignal(3, 3);
        turnOnSignal(4, 3);
        turnOnSignal(5, 3);
        turnOnSignal(5, 3);
        turnOnSignal(6, 3);
        turnOnSignal(7, 3);
        printLCD("EMERGENCY STOP - PLEASE RESET");
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

        kill_mss = false;
    }
}


/* Control a particular switch
int sw -> switch to control (1-4)
bool out_in -> 0 outwards, 1 inwards
ATTENTION: SW2 needs at least 2s between sending him instructions to switch again
*/
void send_command_switch(int sw, bool out_in){
    switch(sw){
        case 1:
            if (out_in){ // inwards (1 command)
                DCC_send_command(switch_address,0x81,1);
            } else { // outwards (1 + idle)
                DCC_send_command(switch_address,0x81,1);
                DCC_send_command(switch_address,0x80,1);
            }
            break;
        case 2: // WARNING -> THIS SWITCH NEEDS AT LEAST 2s to be switched again!
            if (out_in){ // inwards (1 command)
                DCC_send_command(switch_address,0x82,1);
            } else { // outwards (1 + idle)
                DCC_send_command(switch_address,0x82,1);
                DCC_send_command(switch_address,0x80,1);
            }
            break;
        case 3: // command reversed
            if (out_in){
                DCC_send_command(switch_address,0x84,1);
                DCC_send_command(switch_address,0x80,1);
            } else {
                DCC_send_command(switch_address,0x84,1);
            }        
            break;
        case 4:
            if (out_in){ // inwards (1 command)
                DCC_send_command(switch_address,0x88,1);
            } else { // outwards (1 + idle)
                DCC_send_command(switch_address,0x88,1);
                DCC_send_command(switch_address,0x80,1);
            }
        default:
            break;
    }
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
        if (!night_mode){
            DCC_send_command(oneTrainAddress, f_l_speed[oneTrainSpeed], repeatCountFirst);
            DCC_send_command(secondTrainAddress, f_l_speed[secondTrainSpeed], reapeatCountSecond);
        } else {
            DCC_send_command(oneTrainAddress, f_speed[oneTrainSpeed], repeatCountFirst);
            DCC_send_command(secondTrainAddress, f_speed[secondTrainSpeed], reapeatCountSecond);
        }
}

void startOneTrain(unsigned int oneTrainAddress, unsigned int oneTrainSpeed, int repeatCount) {
    if (oneTrainAddress != 0x02) {
        if (!night_mode){
            DCC_send_command(oneTrainAddress, f_speed[oneTrainSpeed], repeatCount);        
        } else {
            DCC_send_command(oneTrainAddress, f_l_speed[oneTrainSpeed], repeatCount);
        }
    } else {
        DCC_send_command(oneTrainAddress, f_speed[oneTrainSpeed], repeatCount);
    }

}

// Stop at top station
void stopTrainWhileStopA(unsigned int train) {
    if (train == oneTrain) {
        startOneTrain(oneTrain, 1, 10);
        firstTrainActive = false;
        turnOnSignal(5, 2);
    } else {
        startOneTrain(secondTrain, 1, 10);
        secondTrainActive = false;
        turnOnSignal(5, 2);
    }
}

// Stop at bottom station
void stopTrainWhileStopB(unsigned int train) {
    if (train == oneTrain) {
        startOneTrain(oneTrain, 1, 10);
        firstTrainActive = false;
        turnOnSignal(5, 1);
    } else {
        startOneTrain(secondTrain, 1, 10);
        secondTrainActive = false;
        turnOnSignal(5, 1);
    }
}

void set_switches(){
    // send_command_switch(1,1);
    // wait_us(10000);
    send_command_switch(2,1);
    wait_us(2000);
    startTrains(oneTrain, secondTrain, 0, 0, 3, 3);
    send_command_switch(3,1);
    wait_us(2000);
    // send_command_switch(4,1);
    // wait_us(10000);
}


void readMCPA() {
    wait_us(2000);
    int sensor_dataA = mcp->_read(INTCAPA);
    // printf("INT0 (0x%x)\n", sensor_dataA);
    switch(sensor_dataA) {
        case 0xfe:
            if (train1DetectorPreviousHit == 0xdf || train1DetectorPreviousHit == 0xef) {
                if (train1DetectorNextHit != 0xfe) {
                    indexMissedSensors++;
                }
                train1DetectorPreviousHit = sensor_dataA;
                train1DetectorNextHit = 0xfd;
                lastCommandSpeedTrain1 = 5;
                lastCommandRepeatTrain1 = 3;
                startOneTrain(oneTrain, 5, 3);
                // printf("INT0 (0x%x)\n", sensor_dataA);
            } else if (train2DetectorPreviousHit == 0xdf || train2DetectorPreviousHit == 0xef) {
                if (train2DetectorNextHit != 0xfe) {
                    indexMissedSensors++;
                }
                train2DetectorPreviousHit = sensor_dataA;
                train2DetectorNextHit = 0xfd;
                lastCommandSpeedTrain2 = 5;
                lastCommandRepeatTrain2 = 3;
                startOneTrain(secondTrain, 5, 3);
                // printf("INT0 (0x%x)\n", sensor_dataA);
            }
            break;
        case 0xfd:
            if (train1DetectorPreviousHit == 0xfe || train1DetectorNextHit == 0xfd) {
                if (train1DetectorNextHit != 0xfd) {
                    indexMissedSensors++;
                }
                train1DetectorPreviousHit = sensor_dataA;
                train1DetectorNextHit = 0x7f;
                lastCommandSpeedTrain1 = 4;
                lastCommandRepeatTrain1 = 3;
                startOneTrain(oneTrain, 4, 3);
                // printf("INT0 (0x%x)\n", sensor_dataA);
            } else if (train2DetectorPreviousHit == 0xfe || train1DetectorNextHit == 0xfd) {
                if (train2DetectorNextHit != 0xfd) {
                    indexMissedSensors++;
                }
                train2DetectorPreviousHit = sensor_dataA;
                train2DetectorNextHit = 0x7f;
                lastCommandSpeedTrain2 = 4;
                lastCommandRepeatTrain2 = 3;
                startOneTrain(secondTrain, 4, 3);
                // printf("INT0 (0x%x)\n", sensor_dataA);
            }
            printf("FD GOT HIT (0x%x)\n", sensor_dataA);
            break;
        case 0xfb:
            if (train1DetectorPreviousHit == 0x7f || train1DetectorPreviousHit == 0xfd) {
                train1DetectorPreviousHit = sensor_dataA;
                train1DetectorNextHit = 0xbf;
                stopTrainWhileStopB(oneTrain);
                firstTrainActive = false;
                // printf("INT0 (0x%x)\n", sensor_dataA);
            } else if (train2DetectorPreviousHit == 0x7f || train2DetectorPreviousHit == 0xfd) {
                train2DetectorPreviousHit = sensor_dataA;
                train2DetectorNextHit = 0xbf;
                stopTrainWhileStopB(secondTrain);
                secondTrainActive = false;
                // //printf("INT0 (0x%x)\n", sensor_dataA);
            }
            break;
        case 0xf7:
            switchTrack = true;
            isRedLight = true;
            //turnOnSignal(2, 2);
            //turnOnSignal(4, 2);
            if (train1DetectorPreviousHit == 0xbf) {
                if (train1DetectorNextHit != 0xf7) {
                    indexMissedSensors++;
                }
                train1DetectorPreviousHit = sensor_dataA;
                train1DetectorNextHit = 0xfd;
                lastCommandSpeedTrain1 = 12;
                lastCommandRepeatTrain1 = 3;
                startOneTrain(oneTrain, 12, 3);
                moveTrainTwo = true;
                //printf("INT0 (0x%x)\n", sensor_dataA);
            } else if (train2DetectorPreviousHit == 0xbf) {
                if (train2DetectorNextHit != 0xf7) {
                    indexMissedSensors++;
                }
                train2DetectorPreviousHit = sensor_dataA;
                train2DetectorNextHit = 0xfd;
                lastCommandSpeedTrain2 = 12;
                lastCommandRepeatTrain2 = 3;
                startOneTrain(secondTrain, 12, 3);
                moveTrainOne = true;
                //printf("INT0 (0x%x)\n", sensor_dataA);
            }
            break;
        case 0xef:
            indexMissedSensors = 4;
            // if (train1DetectorPreviousHit == 0xbf) { 
            //     train1DetectorPreviousHit = sensor_dataA;
            //     lastCommandSpeedTrain1 = 0x61;
            //     lastCommandRepeatTrain1 = 5;
            //     startOneTrain(oneTrain, 0x61, 5);
            //     //printf("INT0 (0x%x)\n", sensor_dataA);
            // } else if (train2DetectorPreviousHit == 0xbf || train2DetectorPreviousHit == 0x7f) { // TRAIN ON WRONG ROUTE
            //     train2DetectorPreviousHit = sensor_dataA;
            //     lastCommandSpeedTrain2 = 0x61;
            //     lastCommandRepeatTrain2 = 5;
            //     startOneTrain(secondTrain, 0x61, 5);
            //     printf("INT0 (0x%x)\n", sensor_dataA);
            // } else if (train1DetectorPreviousHit == 0x7f) { // REACHED THE WRONG SENSOR. Let the train leave and move after it
            //     firstTrainActive = false;
            //     startOneTrain(oneTrain, 0x61, 50);
            //     printLCD("Train off track");
                
            // } else if (train2DetectorPreviousHit == 0x7f) { // REACHED THE WRONG SENSOR. Let the train leave and move after it
            //     secondTrainActive = false;
            //     startOneTrain(secondTrain, 0x61, 50);
            //     printLCD("Train off track");
            // }
            break;
        case 0xdf:
            // doesn't exist
            break;
        case 0xbf:
            isRedLight = true;
            // turnOnSignal(2, 2);
            // turnOnSignal(4, 2);
            if (train1DetectorPreviousHit == 0x7f) {
                isFirstTrain = true;
                if (train1DetectorNextHit != 0xbf) {
                    indexMissedSensors++;
                }
                train1DetectorPreviousHit = sensor_dataA;
                train1DetectorNextHit = 0xf7;
                lastCommandSpeedTrain1 = 10;
                lastCommandRepeatTrain1 = 3;
                startOneTrain(oneTrain, 10, 3);
                //printf("INT0 (0x%x)\n", sensor_dataA);
            } else if (train2DetectorPreviousHit == 0x7f) {
                isFirstTrain = false;
                if (train2DetectorNextHit != 0xbf) {
                    indexMissedSensors++;
                }
                train2DetectorPreviousHit = sensor_dataA;
                train2DetectorNextHit = 0xf7;
                lastCommandSpeedTrain2 = 7;
                lastCommandRepeatTrain2 = 3;
                // startOneTrain(secondTrain, 7, 3);
                startOneTrain(secondTrain, 5, 3);
                //printf("INT0 (0x%x)\n", sensor_dataA);
            }
            break;
        case 0x7f:
            isRedLight = true;
            // turnOnSignal(2, 2);
            // turnOnSignal(4, 2);
            if (train1DetectorPreviousHit == 0xfd) {
                isFirstTrain = true;
                if (train1DetectorNextHit != 0x7f) {
                    indexMissedSensors++;
                }
                train1DetectorPreviousHit = sensor_dataA;
                train1DetectorNextHit = 0xbf;
                lastCommandSpeedTrain1 = 4;
                lastCommandRepeatTrain1 = 3;
                startOneTrain(oneTrain, 4, 3);
                //printf("INT0 (0x%x)\n", sensor_dataA);
            } else if (train2DetectorPreviousHit == 0xfd) {
                isFirstTrain = false;
                if (train2DetectorNextHit != 0x7f) {
                    indexMissedSensors++;
                }
                train2DetectorPreviousHit = sensor_dataA;
                train2DetectorNextHit = 0xbf;
                lastCommandSpeedTrain2 = 4;
                lastCommandRepeatTrain2 = 3;
                startOneTrain(secondTrain, 4, 3);
                //printf("INT0 (0x%x)\n", sensor_dataA);
            }
            break;
        default:
            break;
        }
}

void readMCPB() {
    wait_us(2500);
    int sensor_data = mcp->_read(INTCAPB);
    switch(sensor_data) {
        case 0xfd:
            if (train1DetectorPreviousHit == 0xf7) {
                isFirstTrain = true;
                if (train1DetectorNextHit != 0xfd) {
                    indexMissedSensors++;
                }
                train1DetectorPreviousHit = sensor_data;
                train1DetectorNextHit = 0x7f;
                lastCommandSpeedTrain1 = 10;
                lastCommandRepeatTrain1 = 3;
                startOneTrain(oneTrain, 10, 3);
                //printf("INT1 (0x%x)\n", sensor_data);
            } else if (train2DetectorPreviousHit == 0xf7) {
                isFirstTrain = false;
                if (train2DetectorNextHit != 0xfd) {
                    indexMissedSensors++;
                }
                train2DetectorPreviousHit = sensor_data;
                train2DetectorNextHit = 0x7f;
                lastCommandSpeedTrain2 = 10;
                lastCommandRepeatTrain2 = 3;
                startOneTrain(secondTrain, 10, 3);
                //printf("INT1 (0x%x)\n", sensor_data);
            }
            break;
        case 0xfb:
            //indexMissedSensors = 4;

            // if (train1DetectorPreviousHit == 0xef) {
            //     train1DetectorPreviousHit = sensor_data;
            //     lastCommandSpeedTrain1 = 0x74;
            //     lastCommandRepeatTrain1 = 5;
            //     startOneTrain(oneTrain, 0x74, 5);
            //     printf("INT1 (0x%x)\n", sensor_data);
            // } else if (train2DetectorPreviousHit == 0xef) {
            //     train2DetectorPreviousHit = sensor_data;
            //     lastCommandSpeedTrain2 = 0x74;
            //     lastCommandRepeatTrain2 = 5;
            //     startOneTrain(secondTrain, 0x74, 7);
            //     printf("INT1 (0x%x)\n", sensor_data);
            // } else if (train1DetectorPreviousHit == 0x7f) { // REACHED THE WRONG SENSOR. Let the train leave and move after it
            //     firstTrainActive = false;
            //     startOneTrain(oneTrain, 0x61, 50);
            //     printLCD("Train off track");
            // } else if (train2DetectorPreviousHit == 0x7f) { // REACHED THE WRONG SENSOR. Let the train leave and move after it
            //     secondTrainActive = false;
            //     startOneTrain(secondTrain, 0x61, 50);
            //     printLCD("Train off track");
            // }
            break;
        case 0xf7:
            isRedLight = false;
            // turnOnSignal(2, 1);
            // turnOnSignal(4, 1);
            if (train1DetectorPreviousHit == 0x7f || train1DetectorPreviousHit == 0xbf) {
                if (train1DetectorNextHit != 0xf7) {
                    indexMissedSensors++;
                }
                train1DetectorPreviousHit = sensor_data;
                train1DetectorNextHit = 0xef;
                lastCommandSpeedTrain1 = 5;
                lastCommandRepeatTrain1 = 3;
                startOneTrain(oneTrain, 5, 3);
                //printf("INT1 (0x%x)\n", sensor_data);
            } else if (train2DetectorPreviousHit == 0x7f || train2DetectorPreviousHit == 0xbf) {
                if (train2DetectorNextHit != 0xf7) {
                    indexMissedSensors++;
                }
                train2DetectorPreviousHit = sensor_data;
                train2DetectorNextHit = 0xef;
                lastCommandSpeedTrain2 = 5;
                lastCommandRepeatTrain2 = 3;
                startOneTrain(secondTrain, 5, 3);
                //printf("INT1 (0x%x)\n", sensor_data);
            }
            break;
        case 0xef:
            isRedLight = false;
            // turnOnSignal(2, 1);
            // turnOnSignal(4, 1);
            if (train1DetectorPreviousHit == 0xf7) {
                if (train1DetectorNextHit != 0xed) {
                    indexMissedSensors++;
                }
                train1DetectorPreviousHit = sensor_data;
                train1DetectorNextHit = 0xdf;
                stopTrainWhileStopA(oneTrain);
                firstTrainActive = false;
                //printf("INT1 (0x%x)\n", sensor_data);
            } else if (train2DetectorPreviousHit == 0xf7) {
                if (train2DetectorNextHit != 0xef) {
                    indexMissedSensors++;
                }
                train2DetectorPreviousHit = sensor_data;
                train2DetectorNextHit = 0xdf;
                stopTrainWhileStopA(secondTrain);
                secondTrainActive = false;
                //printf("INT1 (0x%x)\n", sensor_data);
            }
            break;
        case 0xdf:
            isRedLight = false;
            if (train1DetectorPreviousHit != 0xef && train2DetectorPreviousHit != 0xef) {
                indexMissedSensors = 4;
                // emergency stop
            }
            if (train1DetectorPreviousHit == 0xef) {
                if (train1DetectorNextHit != 0xdf) {
                    indexMissedSensors++;
                }
                train1DetectorPreviousHit = sensor_data;
                train1DetectorNextHit = 0xfe;
                lastCommandSpeedTrain1 = 6;
                lastCommandRepeatTrain1 = 3;
                startOneTrain(oneTrain, 6, 3);
                //printf("INT1 (0x%x)\n", sensor_data);
            } else if (train2DetectorPreviousHit == 0xef) {
                if (train2DetectorNextHit != 0xdf) {
                    indexMissedSensors++;
                }
                train2DetectorPreviousHit = sensor_data;
                train2DetectorNextHit = 0xfe;
                lastCommandSpeedTrain2 = 6;
                lastCommandRepeatTrain2 = 3;
                startOneTrain(secondTrain, 6, 3);
                //printf("INT1 (0x%x)\n", sensor_data);
            }
            break;
        case 0xbf:
            if (train1DetectorPreviousHit != 0xfb && train2DetectorPreviousHit != 0xfb) {
                indexMissedSensors = 4;
                // emergency stop
            }
            if (train1DetectorPreviousHit == 0xfb || train1DetectorPreviousHit == 0x7f) {
                if (train1DetectorNextHit != 0xbf) {
                    indexMissedSensors++;
                }
                train1DetectorPreviousHit = sensor_data;
                train1DetectorNextHit = 0xf7;
                lastCommandSpeedTrain1 = 4;
                lastCommandRepeatTrain1 = 3;
                startOneTrain(oneTrain, 4, 3);
                //printf("INT1 (0x%x)\n", sensor_data);
            } else if (train2DetectorPreviousHit == 0xfb || train2DetectorPreviousHit == 0x7f) {
                if (train2DetectorNextHit != 0xbf) {
                    indexMissedSensors++;
                }
                train2DetectorPreviousHit = sensor_data;
                train2DetectorNextHit = 0xf7;
                lastCommandSpeedTrain2 = 4;
                lastCommandRepeatTrain2 = 3;
                startOneTrain(secondTrain, 4, 3);
                //printf("INT1 (0x%x)\n", sensor_data);
            }
            break;
        case 0x7f:
            if (train1DetectorPreviousHit == 0xfd || train1DetectorNextHit == 0xfd) {
                if (train1DetectorNextHit != 0x7f) {
                    indexMissedSensors++;
                }
                train1DetectorPreviousHit = sensor_data;
                train1DetectorNextHit = 0xfb;
                lastCommandSpeedTrain1 = 3;
                lastCommandRepeatTrain1 = 3;
                startOneTrain(oneTrain, 3, 3);
                //printf("INT1 (0x%x)\n", sensor_data);
            } else if (train2DetectorPreviousHit == 0xfd || train1DetectorNextHit == 0xfd) {
                if (train2DetectorNextHit != 0x7f) {
                    indexMissedSensors++;
                }
                train2DetectorPreviousHit = sensor_data;
                train2DetectorNextHit = 0xbf;
                lastCommandSpeedTrain2 = 3;
                lastCommandRepeatTrain2 = 3;
                startOneTrain(secondTrain, 3, 3);
                //printf("INT1 (0x%x)\n", sensor_data);
            }
            break;
        default:
            break;
        }
}

bool findMyTrain () {
    if (loopATrigger == 1) {
        readMCPA();
        loopATrigger = 0;
        return true;

    } else if (loopBTrigger == 1) {
        readMCPB();
        loopBTrigger = 0;
        return true;
    }

    return false;
}

void init() { 
    // Setup control box's switches
    if (switch1){
        on_off = 0;
        RLED = 1;
    }else{
        on_off = 1;
        on_off_mss = 1;
        GLED = 1;
    }
    if (switch2){
        night_mode = 0; 
        YLEDdown = switch2;
    }else{
        night_mode = 1; 
        YLEDup = 1;
    }    
    
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
    // Setting the switches
    set_switches();
    // Ready to go! 
} 

int main() {
    printLCD("Initializing system. \n");
    char buf[MAXIMUM_BUFFER_SIZE] = {0};
    // Initialisation order: first mcp then everything else
    init_mcp();
    initMcp1();
    init();

    // wait_us(100000);
    // send_command_switch(1,1);
    // wait_us(100000);
    // send_command_switch(1,2);
    // wait_us(100000);

    turnOnSignal(0, 1);
    turnOnSignal(1, 1);
    turnOnSignal(2, 1);
    turnOnSignal(3, 1);
    turnOnSignal(4, 1);
    turnOnSignal(5, 1);
    turnOnSignal(5, 1);
    turnOnSignal(6, 1);
    turnOnSignal(7, 1);

    while (true) {
        if (kill == 0 && on_off == 1) {
            if (on_off_mss){
                printLCD("trains ON");
                on_off_mss = 0;
            }
            // if (switchTrack) {
            //     //send_command_switch(3, false);
            //     switchTrack = false;
            // }
            // System WORKING
            if (indexMissedSensors >= 2) {
                kill = 1;
                GLED=1;
                RLED=1; 
                kill_mss = true;
            }
            if (isRedLight) {
                turnOnSignal(2, 2);
                turnOnSignal(4, 2);
            }
            if (isRedLight) {
                if (isFirstTrain) {
                    firstTrainActive = false;
                } else {
                    secondTrainActive = false;
                }
                isRedLight = false;
            }

            if (secondTrainActive == true) {
                bool hasTriggered = findMyTrain();
                if (hasTriggered == false) {
                    startOneTrain(secondTrain, lastCommandSpeedTrain2, lastCommandRepeatTrain2);
                }
            }
            if (firstTrainActive == true) {
                bool hasTriggered = findMyTrain();
                if (hasTriggered == false) {
                    startOneTrain(oneTrain, lastCommandSpeedTrain1, lastCommandRepeatTrain1);
                }
            }

            if (firstTrainActive == false) {
                startOneTrain(oneTrain, 0, 5);
            }

            if (secondTrainActive == false) {
                startOneTrain(secondTrain, 0, 5);
            }

            if (moveTrainOne == true) {
                moveTrainOne = false;
                firstTrainActive = true;
            }

            if (moveTrainTwo == true) {
                moveTrainTwo = false;
                secondTrainActive = true;
            }
            if (firstTrainActive == false && secondTrainActive == false) {
                set_switches();
                loopATrigger = 0;
                loopBTrigger = 0;
                indexMissedSensors = 0;
                firstTrainActive = true;
                secondTrainActive = true;
                turnOnSignal(2, 1);
                turnOnSignal(4, 1);
            }
        } else if (kill == 1) {
            // System KILLED
            kill_system();
            startTrains(oneTrain, secondTrain, 1, 1, 5, 5);

        } else if (on_off == 0) {
            // System STOPPED
            startTrains(oneTrain, secondTrain, 1, 1, 5, 5);
            printLCD("trains OFF");
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