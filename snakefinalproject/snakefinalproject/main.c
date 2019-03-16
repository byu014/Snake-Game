/*
 * finalproject.c
 *
 * Created: 2/28/2019 12:38:21 PM
 * Author : Sky
 */ 

#include <avr/io.h>
#include <avr/eeprom.h>
#include <io.c>
#include <avr/interrupt.h>
#include <stdlib.h>
#include <time.h>// for rand

volatile unsigned char TimerFlag = 0;
unsigned long _avr_timer_M = 1;
unsigned long _avr_timer_cntcurr = 0;

const unsigned char heart1[8] = {0b00000,
	0b00000,
	0b01010,
	0b10101,
	0b10001,
	0b01110,
	0b00100,
0b00000};

const unsigned char heart2[8] = {0b00000,
	0b00000,
	0b01010,
	0b11111,
	0b11111,
	0b01110,
	0b00100,
0b00000};

const unsigned char upCustom[8] = {
	0b00000,
	0b00100,
	0b01110,
	0b11111,
	0b01110,
	0b01110,
	0b00000,
	0b00000
};

const unsigned char downCustom[8] = {
	0b00000,
	0b01110,
	0b01110,
	0b11111,
	0b01110,
	0b00100,
	0b00000,
	0b00000
};

const unsigned char leftCustom[8] = {
	0b00000,
	0b00100,
	0b01111,
	0b11111,
	0b01111,
	0b00100,
	0b00000,
	0b00000
};

const unsigned char rightCustom[8] = {
	0b00000,
	0b00100,
	0b11110,
	0b11111,
	0b11110,
	0b00100,
	0b00000,
	0b00000,
};

typedef struct task{
	int state;
	unsigned long period;
	unsigned long elapsedTime;
	int(*TickFct)(int);
} task;

struct snake{
	unsigned char x;
	unsigned char y;
	struct snake* next;
};

struct food{
	unsigned char x;
	unsigned char y;
	};

task tasks[4];
const unsigned char tasksNum = 4;
unsigned char score = 0;
unsigned char reset = 0;
unsigned char direction = 0;
unsigned char curDirection = 0;
unsigned char gameStart = 0;
struct snake* mySnake = NULL;
struct food* snakeFood = NULL;
unsigned char eaten = 0;
unsigned char gameOver = 0;
unsigned tempHeadX = 0;
unsigned tempHeadY = 0;
unsigned char coordinates[8][8];
struct snake* head = NULL;
unsigned char speed = 0;
unsigned char EEMEM EEMEMhighScore = 0;

unsigned char SetBit(unsigned char x, unsigned char k, unsigned char b) {
	return (b ? x | (0x01 << k) : x & ~(0x01 << k));
}
unsigned char GetBit(unsigned char x, unsigned char k) {
	return ((x & (0x01 << k)) != 0);
}

void adc_init()
{
	ADCSRA |= (1 << ADEN) | (1 << ADSC) | (1 << ADATE);
}

void set_adc_pin(unsigned char pinNum)
{
	ADMUX = (pinNum <= 0x07) ? pinNum : ADMUX;
	
	static unsigned char i = 0;
	for(i = 0; i < 15; ++i)
	{
		asm("nop");
	}
}

void TimerOn()
{
	TCCR1B = 0x0B;
	OCR1A = 125;
	TIMSK1 = 0x02;
	TCNT1 = 0;
	_avr_timer_cntcurr = _avr_timer_M;
	SREG |= 0x80;
}

void TimerOff()
{
	TimerFlag = 1;
}

void TimerSet(unsigned long M)
{
	_avr_timer_M = M;
	_avr_timer_cntcurr = _avr_timer_M;
}
unsigned char msgShown = 0;
void TimerISR()
{
	
	unsigned char i;
	//static char msgShown = 0;
	for(i = 0; i < tasksNum; ++i)
	{
		if(tasks[i].elapsedTime >= tasks[i].period)
		{
			tasks[i].state = tasks[i].TickFct(tasks[i].state);
			tasks[i].elapsedTime = 0;
		}
		tasks[i].elapsedTime += 1;
	}
			unsigned short potentiometer = 0;
			static unsigned short changed = 0;
			set_adc_pin(0x06);
			potentiometer = ADC;
			if((potentiometer >= 200 && potentiometer < 300) && tasks[2].period != 300)
			{
				tasks[2].period = 300;
				changed = 1;
			}
			else if(potentiometer < 100 && tasks[2].period != 100)
			{
				tasks[2].period = 100 ;
				changed = 1;
			}
			else if((potentiometer >= 100 && potentiometer < 200) && tasks[2].period != 200)
			{
				tasks[2].period = 200;
				changed = 1;
			}
			else if((potentiometer >= 300 && potentiometer < 400) && tasks[2].period != 400)
			{
				tasks[2].period = 400;
				changed = 1;
			}
			else if((potentiometer >= 400 && potentiometer < 500) && tasks[2].period != 500)
			{
				tasks[2].period = 500;
				changed = 1;
			}
			else if((potentiometer >= 500 && potentiometer < 600) && tasks[2].period != 600)
			{
				tasks[2].period = 600;
				changed = 1;
			}
			else if((potentiometer >= 600 && potentiometer < 700) && tasks[2].period != 700)
			{
				tasks[2].period = 700;
				changed = 1;
			}
			else if((potentiometer >= 700 && potentiometer < 800) && tasks[2].period != 800)
			{
				tasks[2].period = 800;
				changed = 1;
			}
			else if(potentiometer >= 800  && tasks[2].period != 900)
			{
				tasks[2].period = 900;
				changed = 1;
			}
			speed = tasks[2].period/100;
			
			if(changed)
			{
				msgShown = 0;
				tasks[2].elapsedTime = 0;
				changed = 0;
			}
	
}

ISR(TIMER1_COMPA_vect) {
	// CPU automatically calls when TCNT1 == OCR1 (every 1 ms per TimerOn settings)
	_avr_timer_cntcurr--; // Count down to 0 rather than up to TOP
	if (_avr_timer_cntcurr == 0) { // results in a more efficient compare
		TimerISR(); // Call the ISR that the user uses
		_avr_timer_cntcurr = _avr_timer_M;
	}
}

enum startGameSM{startGame_start, startGame_wait, startGame_ready};
int TickFct_startGame(int state)
{
	static unsigned char msgDisplayed = 0;
	static int next = 0;
	static unsigned char highScore[2];
	switch(state)
	{
		case startGame_start:
			gameStart = 0;
			reset = 0;
			if((~PINA & 0x08) == 0x08)
			{
				gameStart = 1;
				LCD_DisplayString(1, "Score: 0");
				LCD_DisplayStringNoClear(17,"Direction: ");
				msgDisplayed = 0;
				score = 0;
				next = startGame_ready;
				state = startGame_wait;
			}
			else if(!msgDisplayed)
			{
				LCD_ClearScreen();
				LCD_DisplayStringNoClear(17, "Press start S: ");
				LCD_DisplayStringNoClear(1, "High Score: ");
				//eeprom_write_byte(&EEMEMhighScore,8); reset eemem value for testing purposes
				sprintf(highScore,"%d", eeprom_read_byte(&EEMEMhighScore));
				LCD_DisplayStringNoClear(13, highScore);
				msgDisplayed = 1;
			}
			if(!msgShown)
			{
				LCD_Cursor(32);
				LCD_WriteData('0' + speed);
				msgShown = 1;
			}
			
			break;
		
		case startGame_wait:
			if((~PINA & 0x08) != 0x00)
			{
				break;
			}
			state = next;
			break;
		
		case startGame_ready:
			if((~PINA & 0x08) == 0x08)
			{
				LCD_ClearScreen();
				LCD_DisplayString(1, "Resetting");
				reset = 1;
				gameStart = 0;
				msgShown = 0;
				next = startGame_start;
				state = startGame_wait;
			}
			break;
		
		default:
		break;
	}
	return state;
}

enum directionSM{direction_start, direction_getDir};
int TickFct_direction(int state)
{
	static unsigned short lr = 0;
	static unsigned short ud = 0;
	static unsigned char imageNum = 1;
	switch(state)
	{
		case direction_start:
			if(!gameStart)
			break;
			direction = 0;
			curDirection = 0;
			lr = 0;
			ud = 0;
			state = direction_getDir;
			break;
		
		case direction_getDir:
			if(reset)
			{
				state = direction_start;
				break;
			}
						if(gameOver)
						{
							break;
						}
			set_adc_pin(0x04);
			lr = ADC;
			set_adc_pin(0x05);
			ud = ADC;
			LCD_Cursor(28);
			//unsigned char string[32];
					//sprintf(string, "x: %d, y: %d", lr, ud);
					//LCD_DisplayString( 1, string);
			if(lr < (512 - 300) && abs(ud - 512) < 256) // logic for going left
			{
				direction = 'l';
				imageNum = 0x01;
				LCD_WriteData(0x01);// custom image
				
			}
			else if(lr > (512 + 300) && abs(ud - 512) < 256)// logic for going right
			{
				direction = 'r';
				imageNum = 0x02;
				LCD_WriteData(0x02);// custom image
			}
			else if(ud < (512 - 300) && abs(lr - 512) < 256)// logic for going up
			{
				direction = 'u';
				imageNum = 0x03;
				LCD_WriteData(0x03);// custom image
			}
			else if(ud > (512 + 300) && abs(lr - 512) < 256)// logic for going down
			{
				direction = 'd';
				imageNum = 0x04;
				LCD_WriteData(0x04);// custom image
			}
			if(curDirection == 0)
			{
				curDirection = direction;
			}
			LCD_Cursor(29);
			//LCD_ClearScreen();
			//LCD_Cursor(1);
			//LCD_WriteData(direction);
			break;
	}
	return state;
}

struct snake* setSnakeDirection(struct snake* snakeParameter)
{
	struct snake* cur = snakeParameter;
	while(cur->next != NULL)
	{
		if(cur->next != NULL)
		{
			cur->x = cur->next->x;
			cur->y = cur->next->y;
		}
		cur = cur->next;
	}
	return cur; //basically head of snake
}
struct snake* findHead(struct snake* snakeParameter)
{
	struct snake* cur = snakeParameter;
	while(cur->next != NULL)
	{
		cur = cur->next;
	}
	return cur; //basically head of snake
}
unsigned char crashed = 0;
void moveSnake(struct snake* snakeParameter, struct snake* head)
{
		static unsigned char wait = 0;
		unsigned char i = 0;
		unsigned char j = 0;
		unsigned char numElements = 0;
		for(i = 0; i < 8; ++i)
		{
			for(j = 0; j < 8; ++j)
			{
				coordinates[i][j] = 0;
			}
		}
	struct snake* cur = snakeParameter;
	unsigned char tempColVal = 0x00;
	unsigned char tempColSel = 0xFF;
	while(cur != NULL) // fills 2d array with coordinates that are lit
	{
		coordinates[cur->y][cur->x] += 1;
		cur = cur->next;
		numElements += 1;
	}

	if(tempHeadX == snakeFood->x && tempHeadY == snakeFood->y) // replaces food position as new head
	{
		struct snake* newHead = malloc(sizeof(struct snake));
		newHead->x = snakeFood->x;
		newHead->y = snakeFood->y;
		newHead->next = NULL;
		head->next = newHead;
		head = head->next;
		eaten = 1;
		unsigned char scoreString[2];
		score += 1;
		sprintf(scoreString,"%d", score);
		LCD_DisplayStringNoClear(8, scoreString);
		if(score > eeprom_read_byte(&EEMEMhighScore)) // check score against highscore and replaces highscore if score > highscore
		{
			eeprom_write_byte(&EEMEMhighScore, score);
		}
		if(score == 63) // victory
		{
			score = 0;
			LCD_DisplayString(1,"VICTORY");
			LCD_DisplayStringNoClear(17,"Press to restart");
			gameOver = 1;
		}
	}
	else if(coordinates[head->y][head->x] >1) // checks collision, if coordinates at head has two points, means head is sharing a space with part of its body
	{
		crashed = 1;
		return;
	}
	generateFood( tempColVal, tempColSel, coordinates);
}

void checkCrash(struct snake* snakeParameter, struct snake* head)
{
	struct snake* cur = snakeParameter;
	for(unsigned row = 0; row < 8; row++)
	{
		for(unsigned col = 0; col < 8; col++)
		{
			if(coordinates[row][col] > 1)
			{
				//crashed = 1;
			}
		}
	}
	if(coordinates[head->y][head->x] > 1)
	{
		crashed = 1;
	}
}
void generateFood(unsigned char tempColVal, unsigned char tempColSel, unsigned char coordinates[8][8])// generates random food coordinate and displays the snake
{
	unsigned char i = 0;
	static unsigned char j = 0;
	if(!eaten) 
	{
		coordinates[snakeFood->y][snakeFood->x] = 1;
		
		for(i = 0; i < 8; ++i)
		{
			if(coordinates[j][i] == 1)
			{
				tempColVal = SetBit(tempColVal, j, 1);
				tempColSel = SetBit(tempColSel, i, 0);
				
			}
		}
		PORTC = tempColVal;
		PORTB = tempColSel;
		j+= 1;
		if(j >= 8)// led matrix multiplexing logic one row at a time every ms
		{
			j = 0;
		}
		return;
	}
	eaten = 0;
	unsigned char tempX = rand() % 8;
	unsigned char tempY = rand() % 8;
	while(coordinates[tempY][tempX] >= 1) // makes sure that food doesnt spawn on top of snake
	{
		tempX = rand() % 8;
		tempY = rand() % 8;
	}
	snakeFood->x = tempX;
	snakeFood->y = tempY;
	tempColVal = SetBit(tempColVal,tempY, 1);
	tempColSel = SetBit(tempColSel, tempX, 0);
	PORTC = tempColVal;
	PORTB = tempColSel;
}



enum gameSM{game_start, game_running, game_gameOver};
int TickFct_game(int state)
{
	static unsigned char msgDisplayed2 = 0;
	unsigned char column_val = 0x00; // sets the pattern displayed on columns
	unsigned char column_sel = 0xFF; // grounds column to display pattern, 
	//static unsigned char flash = 0x00;
	

	switch(state)
	{
		case game_start:
			if(!gameStart)
			{
				break;
			}
			crashed = 0;
			mySnake = malloc(sizeof(struct snake));
			mySnake->x = 5;
			mySnake->y = 3;
			mySnake->next = NULL;
			head = mySnake;
			snakeFood = malloc(sizeof(struct food));
			snakeFood->x = 2;
			snakeFood->y = 3;
			eaten = 0;
			column_val = SetBit(column_val, mySnake->y,1); //y
			column_sel = SetBit(column_sel, mySnake->x, 0); //x
			PORTC = SetBit(column_val, snakeFood->y,1); //y
			PORTB = SetBit(column_sel, snakeFood->x, 0); //x
			state = game_running;
			break;
		
		case game_running:	
			if(gameOver)
			{
				state = game_gameOver;
			}			
			if(reset)
			{
				struct snake* tempCur = mySnake;
				struct snake* tempNext = tempCur->next;
				while(tempCur != NULL)
				{
					tempNext = tempCur->next;
					free(tempCur);
					tempCur = tempNext;
				}
				mySnake = NULL;
				state = game_start;
				PORTC = 0xFF; // sets blank screen row select 0 - 7 up to down
				PORTB = 0xFF; // inverse column select 0 = column selected 7 - 0 left to right
				break;
			}
			if(!direction)
			break;
			
			//head = setSnakeDirection(mySnake);
			head = findHead(mySnake);
			tempHeadX = head->x;
			tempHeadY = head->y;
			if(curDirection == 'l') //  cases makes sure you can't do a 180 turn 
			{
				if(direction == 'u')
				{
					curDirection = 'u';
					//head->y -= 1;
					tempHeadY -=1;
				}
				else if(direction == 'd')
				{
					curDirection = 'd';
					//head->y += 1;
					tempHeadY += 1;
				}
				else
				{
					//head->x +=1 ;
					tempHeadX += 1;
				}
			}
			else if(curDirection == 'r') //  cases makes sure you can't do a 180 turn
			{
				if(direction == 'u')
				{
					curDirection = 'u';
					//head->y -= 1;
					tempHeadY -=1;
				}
				else if(direction == 'd')
				{
					curDirection = 'd';
					//head->y += 1;
					tempHeadY += 1;
				}
				else
				{
					//head->x -=1 ;
					tempHeadX -=1;
				}
			}
			else if(curDirection == 'u') //  cases makes sure you can't do a 180 turn
			{
				if(direction == 'l')
				{
					curDirection = 'l';
					//head->x += 1;
					tempHeadX += 1;
				}
				else if(direction == 'r')
				{
					curDirection = 'r';
					//head->x -= 1;
					tempHeadX -=1;
				}
				else
				{
					//head->y -=1 ;
					tempHeadY -=1;
				}
			}
			else if(curDirection == 'd') //  cases makes sure you can't do a 180 turn
			{
				if(direction == 'l')
				{
					curDirection = 'l';
					//head->x += 1;
					tempHeadX +=1;
				}
				else if(direction == 'r')
				{
					curDirection = 'r';
					//head->x -= 1;
					tempHeadX -=1;
				}
				else
				{
					//head->y +=1 ;
					tempHeadY +=1;
				}
			}
			
			//checkCrash(mySnake, head);
			//if(head->x < 0 || head->x > 7 || head->y > 7 || head->y < 0 || crashed == 1) // head ran into side game over
			if(tempHeadX <0 || tempHeadX > 7 || tempHeadY > 7 || tempHeadY < 0 || crashed ==1)
			{
				gameOver = 1;
				state = game_gameOver;
				break;
			}
			if(tempHeadX != snakeFood->x || tempHeadY != snakeFood->y)
			{
				setSnakeDirection(mySnake);
			head->x = tempHeadX;
			head->y = tempHeadY;
			}

			//moveSnake(mySnake, head,coordinates);
			
			break;
			
		case game_gameOver:
		{
			if(!msgDisplayed2)
			{
				
				LCD_DisplayString(1,"Game Over");
				LCD_DisplayStringNoClear(17,"Press to restart");
				msgDisplayed2 = 1;
			}
			
			
			if(reset)
			{
				gameOver = 0;
				msgDisplayed2 = 0;
				struct snake* tempCur = mySnake;
				struct snake* tempNext = tempCur->next;
				while(tempCur != NULL)
				{
					tempNext = tempCur->next;
					free(tempCur);
					tempCur = tempNext;
				}
				mySnake = NULL;
				state = game_start;
				PORTC = 0xFF; // sets blank screen row select 0 - 7 up to down
				PORTB = 0xFF; // inverse column select 0 = column selected 7 - 0 left to right
				break;
			}
			break;
		}
		
	}
	return state;
}

enum showSnakeSM{showSnake_start, showSnake_run};
int TickFct_showSnake(int state)
{
	switch(state)
	{
		case showSnake_start:
			if(!gameStart)
			break;
			
			state = showSnake_run;
			break;
		
		case showSnake_run:
			if(reset)
			{
				state = showSnake_start;
				break;
			}
			if(gameOver)
			{
				PORTC = 0xFF;
				PORTB = 0x00;
				break;
			}
			
			moveSnake(mySnake, head);
			break;
	}
	return state;
}


int main(void)
{
    /* Replace with your application code */
	unsigned short potentiometer = 0x00;
	DDRB = 0xFF; PORTB = 0x00; // x axis of led column_sel
	DDRC = 0xFF; PORTC = 0x00; // y axis of led column_val
	DDRD = 0xFF; PORTD = 0x00; // lcd display
	DDRA = 0x07; PORTA = 0x08; //PA4 is lr, PA5 is ud
	unsigned char i = 0;
	tasks[i].state = startGame_start;
	tasks[i].period = 300;
	tasks[i].TickFct = &TickFct_startGame;
	++i;
	tasks[i].state = direction_start;
	tasks[i].period = 100;
	tasks[i].TickFct = &TickFct_direction;
	++i;
	tasks[i].state = game_start;
	tasks[i].period = 100;
	tasks[i].TickFct = &TickFct_game;
	++i;
	tasks[i].state = showSnake_start;
	tasks[i].period = 1;
	tasks[i].TickFct = &TickFct_showSnake;

	LCD_init();
	adc_init();
	LCD_CreateCustomChar(1, leftCustom);
	LCD_CreateCustomChar(2, rightCustom);
	LCD_CreateCustomChar(3, upCustom);
	LCD_CreateCustomChar(4, downCustom);
	srand(time(NULL));
	TimerSet(1);
	TimerOn();
    while (1) 
    {	
		//tasks[2].period = potentiometer ;
    }
}

