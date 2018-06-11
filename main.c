//CS120B Spring Quarter 2018 Custom Project
//Austin Tung

#include <avr/io.h>
#include <avr/eeprom.h>
#include <scheduler.h>
#include <timer.h>
#include <io.c>
#include <util/delay.h>

#define F_CPU 1000000UL

#define T1 7         // location of top obstacle
#define B1 11        // location of bottom obstacle
#define T_HOLE 14         // location of top hole
#define TRK_LENGTH 16   // max number of columns on display


const unsigned char TRACK1[] = {' ',' ',' ',' ',' ',' ',' ',' ',2,' ',' ',' ',' ',' ',' ',4};  // Top track
const unsigned char TRACK2[] = {' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',2,' ',' ',' '};  // Bot track

// Shared variables -----------------------------------------------------------------------------------------
uint8_t score_cnt = 0x00;
uint8_t highscore = 0x00;
unsigned char menustring[] = "   Space Race   ";
unsigned char menusize = 16;
unsigned char pos = 0;   // scroll pos for speed1
unsigned char pos2 = 0;	// scroll pos for speed2
unsigned char pos3 = 0; // scroll pos for speed3
unsigned char pause;	//flag for pause 
unsigned char lose;		//flag for lose
unsigned char top_obs, btm_obs, top_hole; //flags for obstacles
unsigned char ship[]={0x1E,0x08,0x0C,0x1F,0x1F,0x0C,0x08,0x1E};
unsigned char rock[]={0x04,0x0E, 0x1F,0x1F,0x1F,0x1F,0x0E,0x04};
unsigned char gate[]={0x0E,0x11,0x11,0x11,0x11,0x11,0x11,0x0E};
unsigned char blk_hole[]={0x1F,0x10,0x17,0x15,0x15,0x15,0x11,0x1F};
	
	
//Shift Register Code------------------------------------------------------------------------------------------	
void transmit_data(unsigned char data) {
	int i;
	for (i = 0; i < 8 ; ++i) {
		// Sets SRCLR to 1 allowing data to be set
		// Also clears SRCLK in preparation of sending data
		PORTB = 0x08;
		// set SER = next bit of data to be sent.
		PORTB |= ((data >> i) & 0x01);
		// set SRCLK = 1. Rising edge shifts next bit of data into the shift register
		PORTB |= 0x02;
	}
	// set RCLK = 1. Rising edge copies data from “Shift” register to “Storage” register
	PORTB |= 0x04;
	// clears all lines in preparation of a new transmission
	PORTB = 0x00;
}	
//Joystick ADC Code-----------------------------------------------------------------------------------
void ADC_init() {
	ADCSRA |= (1 << ADEN) | (1 << ADSC) | (1 << ADATE);
	// ADEN: setting this bit enables analog-to-digital conversion.
	// ADSC: setting this bit starts the first conversion.
	// ADATE: setting this bit enables auto-triggering. Since we are
	//        in Free Running Mode, a new conversion will trigger whenever
	//        the previous conversion completes.
}

//State Machines--------------------------------------------------------------------------------------
enum S_States {S_START, S_INIT, SCROLL, LOSE};

int Tick_Scroll(int state) //Scrolls through LCD to determine position
{
	// Transitions
	switch(state) 
	{
		case S_START:
		state = S_INIT;     
		break;
		case S_INIT:
		pos = 0;               
		state = SCROLL;     
		break;
		case SCROLL:
		if (pos == T1)
		{
			score_cnt++;
			transmit_data((char)(score_cnt));
		}
		if (pos == B1)
		{
			score_cnt++;
			transmit_data((char)(score_cnt));
		}
		if (lose)
		{
			 state = LOSE;
		}
		else if (pos >= TRK_LENGTH) 
		{
		pos = 0;
		}
		break;
		case LOSE:
		if(!lose)
		{
		 state = S_INIT;
		}
		break;
		default:
		state = S_START; 
		break;   
	}

	// Actions
	switch(state) 
	{
		case SCROLL:
		if (!pause)
		pos++;
		break;
		default: break;
	}

	return state;
}

enum S_States2 {S_START2, S_INIT2, SCROLL2, LOSE2};

int Tick_Scroll2(int state) //Scrolls through LCD to determine position
{
	// Transitions
	switch(state) 
	{
		case S_START2:
		state = S_INIT2;       
		break;
		case S_INIT2:
		pos2 = 0;              
		state = SCROLL2;       
		break;
		case SCROLL2:
		if (lose)
		{
			 state = LOSE2;
		}
		else if (pos2 >= TRK_LENGTH)
		{
			pos2 = 0;
		}
		break;
		case LOSE2:
		if(!lose)
		{ 
		state = S_INIT2;
		}
		break;
		default:
		state = S_START2;
		break;   
	}

	// Actions
	switch(state) 
	{
		case SCROLL2:
		if (!pause)
		pos2++;
		break;
		default: break;
	}

	return state;
}

enum S_States3 {S_START3, S_INIT3, SCROLL3, LOSE3};

int Tick_Scroll3(int state) //Scrolls through LCD to determine position
{
	// Transitions
	switch(state) 
	{
		case S_START3:
		state = S_INIT3;          
		break;
		case S_INIT3:
		pos3 = 0;                
		state = SCROLL3;         
		break;
		case SCROLL3:

		if (lose)
		{
			 state = LOSE3;
		}
		else if (pos3 >= TRK_LENGTH)
		{
			pos3 = 0;
		}
		break;
		case LOSE3:
		if(!lose)
		{
			 state = S_INIT3;
		}
		break;
		default:
		state = S_START3; break; 
	}

	// Actions
	switch(state) 
	{
		case SCROLL3:
		if (!pause)
		pos3++;
		break;
		default: break;
	}

	return state;
}
enum T_States {T_START, T_INIT, TOP, T_LOSE};

int Tick_Top(int state)
{
static unsigned char i, j;  // variables to track active position
	if(score_cnt >= 6 && score_cnt < 12)
	{
		pos = pos2;
	}
	else if(score_cnt >= 12)
	{
		pos = pos3;
	}
// Transitions
switch(state) 
{
	case T_START:
	state = T_INIT;      
	break;
	case T_INIT:
	i = 1;                  
	j = pos;           
	state = TOP;     
	break;
	case TOP:
	if(lose)
	{
		state = T_LOSE;
		j = 0;
		top_obs = 0;
		i = 1;  
	}
	else if (i > TRK_LENGTH) 
	{
		state = T_INIT;
	}
	break;
	case T_LOSE:
	if(!lose) state = T_INIT;
	break;
	default:
	state = T_START; break;   
}

// Actions
switch(state) 
{
	case TOP:
	LCD_Cursor(i);  
	LCD_WriteData(TRACK1[j % TRK_LENGTH]);
	i++; j++;
	if (pos == T1) //obstacle detection
	{
		top_obs = 1;
	}
	else top_obs = 0;
	if (pos == T_HOLE) //gate detection
	{
		top_hole = 1;
	}
	else top_hole = 0;
	break;
	default: break;
}

return state;
}

enum B_States {B_START, B_INIT, BOT, B_LOSE};

int Tick_Bot(int state)
{
	static unsigned char i, j; //variables to track position
	if(score_cnt >= 6 && score_cnt < 12)
	{
		pos = pos2;
	}
	else if(score_cnt >= 12)
	{
		pos = pos3;
	}

	// Transitions
	switch(state) 
	{
		case B_START:
		state = B_INIT;      
		break;
		case B_INIT:
		i = 17;               
		j = pos;            
		state = BOT;      
		case BOT:
		if(lose) 
		{
			state = B_LOSE;
			j = 0;
			btm_obs = 0;
			i = 17;     
		}

		else if (i > TRK_LENGTH*2) 
		{
			state = B_INIT;
		}
		break;
		case B_LOSE:
		if(!lose) state = B_INIT;
		break;
		default:
		state = B_START; break;   
	}

	// Actions
	switch(state) 
	{
		case BOT:
		LCD_Cursor(i); 
		LCD_WriteData(TRACK2[j % TRK_LENGTH]);
		i++; j++;
		if (pos == B1)
		{
		 btm_obs = 1;   
		}
		else btm_obs = 0; 
		break;
		default: break;
	}

	return state;
}

enum P_States {P_START, P_INIT,P_MENU, PLAY,P_LOSE, P_PAUSE};	

int Tick_Play(int state)
{	
	static unsigned char i, j;
	unsigned short x = ADC;
	j = ~PINA; //read user inputs
	switch(state)
	{
		case P_START:
		state = P_INIT;
		break;
		case P_INIT:
		i = 2;
		pause = 1;
		lose = 0;
		state = P_MENU;
		break;
		case P_MENU:
		if((j & 0x04) && pause)
		{
			state = PLAY;
		}
		case PLAY:
		// right button is pressed
		if((x > 551) && !pause) 
		{
			i = 2;          // move player to top row
		}
		// left button is pressed
		else if ((x < 551) && !pause) 
		{
			i = 18;         // move player to bottom row
		}
		//soft reset
		else if (j & 0x04) 
		{
			state = P_START;         // go to start
			if (lose) lose = 0;         // restart game
			pos = 0;
			score_cnt = 0;
			transmit_data((char)(score_cnt));
		}
		// pause/play button is pressed
		else if (j & 0x02) 
		{
			pause = (pause) ? 0 : 1;  // toggle pause
			state = P_PAUSE;        
			if (lose) lose = 0;        
		}
		// character collision with obstacle
		if ((i == 2 && top_obs) || (i == 18 && btm_obs)) 
		{
			pause = lose = 1;       
			state = P_LOSE;     
		}
		if(i == 2 && top_hole)
		{
			score_cnt = 0;
			transmit_data((char)(score_cnt));
		}
		break;
		case P_LOSE:
		if(score_cnt > highscore)
		{
			highscore =score_cnt;
			eeprom_update_byte((uint8_t*)46, highscore);
		}
		delay_ms(3000);
		LCD_ClearScreen();
		LCD_DisplayString(1, "   You Died!    Your Score:");
		LCD_Cursor(29);
		LCD_WriteData(score_cnt + '0');
		
		highscore = eeprom_read_byte((uint8_t*)46);
		delay_ms(3000);
		LCD_ClearScreen();
		LCD_DisplayString(1, "   Try Again?   High Score:");
		LCD_Cursor(29);
		LCD_WriteData(highscore + '0');
		delay_ms(3000);
		LCD_ClearScreen();
		LCD_DisplayString(1, " Press Reset To   Start Over  ");
		if (j & 0x04)
		{
			score_cnt = 0;
			transmit_data((char)(score_cnt));
			state = P_START;
			
		}
		case P_PAUSE:
		//unpause
		if (j ^ 0x02) 
		{
			state = PLAY;
		}
		break;
		
		default:
		state = P_START;
		pos = 0;
		break;
	}
	switch(state)
	{
		case P_START: break;
		case P_INIT: break;
		case P_MENU:
		LCD_ClearScreen();
		LCD_Cursor(1);
		for(i = 0; i < menusize; ++i)
		{
			LCD_WriteData(menustring[i]);
		}
		i = 2;
		break;
		case PLAY:
		if(!lose)
		{
			LCD_Cursor(i);
			LCD_WriteData(1);
		}
		case P_LOSE:
		break;
		case P_PAUSE: break;
		default: break;
	}
	return state;
}

int main()
{
DDRA = 0x00; PORTA = 0xFF; //Inputs
DDRB = 0xFF; PORTB = 0x00; //Outputs
DDRC = 0xFF; PORTC = 0x00; //Outputs
DDRD = 0xFF; PORTD = 0x00; //Outputs



// Period for the tasks
unsigned long int SMTick1_calc = 400;
unsigned long int SMTick2_calc = 20;
unsigned long int SMTick3_calc = 20;
unsigned long int SMTick4_calc = 20;

//faster speeds
unsigned long int SMTick5_calc = 350;
unsigned long int SMTick6_calc = 300;

//Calculating GCD
unsigned long int tmpGCD = 1;
tmpGCD = findGCD(SMTick1_calc, SMTick2_calc);
tmpGCD = findGCD(tmpGCD, SMTick3_calc);
tmpGCD = findGCD(tmpGCD, SMTick4_calc);
tmpGCD = findGCD(tmpGCD, SMTick5_calc);
tmpGCD = findGCD(tmpGCD, SMTick6_calc);


//Greatest common divisor for all tasks or smallest time unit for tasks.
unsigned long int GCD = tmpGCD;

//Recalculate GCD periods for scheduler
unsigned long int SMTick1_period = SMTick1_calc/GCD;
unsigned long int SMTick2_period = SMTick2_calc/GCD;
unsigned long int SMTick3_period = SMTick3_calc/GCD;
unsigned long int SMTick4_period = SMTick4_calc/GCD;
unsigned long int SMTick5_period = SMTick5_calc/GCD;
unsigned long int SMTick6_period = SMTick6_calc/GCD;

//Declare an array of tasks
static task task1, task2, task3, task4, task5, task6;
task *tasks[] = { &task1, &task2, &task3, &task4, &task5, &task6 };
const unsigned short numTasks = sizeof(tasks)/sizeof(task*);

// Task 1
task1.state = S_START;//Task initial state.
task1.period = SMTick1_period;//Task Period.
task1.elapsedTime = SMTick1_period;//Task current elapsed time.
task1.TickFct = &Tick_Scroll;//Function pointer for the tick.

// Task 2
task2.state = T_START;//Task initial state.
task2.period = SMTick2_period;//Task Period.
task2.elapsedTime = SMTick2_period;//Task current elapsed time.
task2.TickFct = &Tick_Top;//Function pointer for the tick.

// Task 3
task3.state = B_START;//Task initial state.
task3.period = SMTick3_period;//Task Period.
task3.elapsedTime = SMTick3_period; // Task current elapsed time.
task3.TickFct = &Tick_Bot; // Function pointer for the tick.

// Task 4
task4.state = P_START;//Task initial state.
task4.period = SMTick4_period;//Task Period.
task4.elapsedTime = SMTick4_period; // Task current elapsed time.
task4.TickFct = &Tick_Play; // Function pointer for the tick.

//Task 5
task5.state = S_START2;//Task initial state.
task5.period = SMTick5_period;//Task Period.
task5.elapsedTime = SMTick5_period; // Task current elapsed time.
task5.TickFct = &Tick_Scroll2; // Function pointer for the tick.

//Task 6
task6.state = S_START3;//Task initial state.
task6.period = SMTick6_period;//Task Period.
task6.elapsedTime = SMTick6_period; // Task current elasped time.
task6.TickFct = &Tick_Scroll3; // Function pointer for the tick.

LCD_Custom(1,ship);
LCD_Custom(2,rock);
LCD_Custom(3,gate);
LCD_Custom(4,blk_hole);
ADC_init();
LCD_init();
LCD_ClearScreen();

// Set the timer and turn it on
TimerSet(GCD);
TimerOn();

unsigned short i; // Scheduler for-loop iterator
while(1) {
	// Scheduler code
	for ( i = 0; i < numTasks; i++ ) {
		// Task is ready to tick
		if ( tasks[i]->elapsedTime == tasks[i]->period ) {
			// Setting next state for task
			tasks[i]->state = tasks[i]->TickFct(tasks[i]->state);
			// Reset the elapsed time for next tick.
			tasks[i]->elapsedTime = 0;
		}
		tasks[i]->elapsedTime += 1;
	}
	while(!TimerFlag);
	TimerFlag = 0;
}

// Error: Program should not exit!
return 0;

}