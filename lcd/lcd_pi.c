/*
 * LED 1: GPIO 26(37), LED 2: GPIO 19(35), LED 3: GPIO 13(33)// 6

◆ GPIO12 //Button 1: GPIO16(36), Button 2: GPIO20(38), Button 3: GPIO21(40)
*/

#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <wiringPiI2C.h>
#include <wiringPi.h>
#include <lcd.h>
#include <time.h>
#include <pthread.h> // Include the pthread library for multi-threading

#include "gpio.h"
#include "lcd.h"

#define IN 0
#define OUT 1

#define BUTTON1 12
#define BUTTON2 16
#define BUTTON3 20
#define BUTTON4 21

int timerMinutes = 0;   // Variable to store the timer minutes set by the user
int timerSeconds = 0;   // Variable to store the timer seconds remaining
int isTimerSet = 0;     // Flag to indicate if the timer is set
pthread_mutex_t lock;  // Mutex lock to synchronize access to shared variables
pthread_t buttonThread; // Thread for button handling
pthread_t lcdThread;    // Thread for LCD display


void *lcd_Display(void *arg)
{
    int isTimeOver = 0; // Flag to indicate if the timer is done
    int done=0;

    while (1)
    {
        pthread_mutex_lock(&lock);
        if (isTimerSet == 1&& !isTimeOver)
        {
            // Timer is set, calculate and display remaining time
            int minutes = timerSeconds / 60;
            int seconds = timerSeconds % 60;

            // Check if the timer has reached zero
            if (timerSeconds <= 0 & !done)
            {
                isTimeOver = 1;    // Set the time over flag
                timerSeconds = 0;  // Reset the timer seconds
				timerMinutes=0;

                ClrLcd();
				lcdLoc(LINE1);
                typeln("Time is over"); // Replace 'lcd_write_text()' with the appropriate function from your LCD library
                while(isTimeOver){
                    int button1State = 1;
        	        int button2State = 1;
        	        int button3State = 1;
        	        int button4State = 1;
                    // Read button states
                    button1State = GPIORead(BUTTON1);
                    button2State = GPIORead(BUTTON2);
                    button3State = GPIORead(BUTTON3);
                    button4State = GPIORead(BUTTON4);

                    // Check if any button is pressed
                    if (button1State == LOW || button2State == LOW || button3State == LOW || button4State == LOW)
                    {
                        isTimerSet=0;
                        isTimeOver = 0; // Reset the time over flag
                        done=1;
                        ClrLcd();
				        lcdLoc(LINE1);
                        typeln("Set Timer: 0min"); // Write the text on the LCD
                    }   
                    }
            }
            else
            {
                done=0;
                lcdLoc(LINE1);
                char buffer[17]; // Adjust the buffer size as per your LCD module's requirements
                snprintf(buffer, sizeof(buffer), "%02d:%02d", minutes, seconds);
                typeln(buffer); // Replace 'lcd_write_text()' with the appropriate function from your LCD library

                // Update the timer seconds
                timerSeconds--;
            }
        }
		pthread_mutex_unlock(&lock);

        sleep(1); 
    }

    return NULL;
}

void *buttonHandler(void *arg)
{
    while (1)
    {
        pthread_mutex_lock(&lock);
		int button1State = 1;
        int button2State = 1;
        int button3State = 1;
        int button4State = 1;	

        button1State = GPIORead(BUTTON1);
        button2State = GPIORead(BUTTON2);
        button3State = GPIORead(BUTTON3);
        button4State = GPIORead(BUTTON4);
        usleep(10000); 
        pthread_mutex_unlock(&lock);
	

        // Check button states and perform corresponding actions
        if (button1State == LOW)
        {
            // Button 1 is pressed
            if (isTimerSet == 0)
            {
                pthread_mutex_lock(&lock);
                // Increment the timer minutes
                timerMinutes+=1;
                usleep(10000);
                pthread_mutex_unlock(&lock);

                ClrLcd();
    
				lcdLoc(LINE1);
                char buffer[17]; // Adjust the buffer size as per your LCD module's requirements
                snprintf(buffer, sizeof(buffer), "Set Timer: %dmin", timerMinutes);
                typeln(buffer); // Write the text on the LCD
            }
        }
        else if (button2State == LOW)
        {
            // Button 2 is pressed
            if (isTimerSet == 0 && timerMinutes > 0)
            {
                pthread_mutex_lock(&lock);
                // Decrement the timer minutes
                timerMinutes-=1;
                usleep(10000);
                pthread_mutex_unlock(&lock);
                
                if(timerMinutes<10)
                {
                    ClrLcd();
                    }

				lcdLoc(LINE1);
                char buffer[17]; // Adjust the buffer size as per your LCD module's requirements
                snprintf(buffer, sizeof(buffer), "Set Timer: %dmin", timerMinutes);
                typeln(buffer); // Write the text on the LCD
            }
        }
        else if (button3State == LOW)
        {
            // Button 3 is pressed
            if (isTimerSet == 0)
            {
                // Set the timer and start countdown
                pthread_mutex_lock(&lock);
                isTimerSet = 1;
                timerSeconds = timerMinutes * 60;
                pthread_mutex_unlock(&lock);

                // Clear the LCD display after setting the timer
                ClrLcd(); // Clear the LCD display using the appropriate function from your LCD library

                
            }
        }
        else if (button4State == LOW)
        {

			if(isTimerSet==0){
                timerMinutes = 0;
				ClrLcd();
				lcdLoc(LINE1);
                typeln("Set Timer: 0min"); // Write the text on the LCD
			}
            // Button 4 is pressed
            if (isTimerSet == 1)
            {
                // Cancel the timer
                pthread_mutex_lock(&lock);
                isTimerSet = 0;
                timerMinutes = 0;
                timerSeconds = 0;
                pthread_mutex_unlock(&lock);

                // Clear the LCD display after canceling the timer
                ClrLcd(); // Clear the LCD display using the appropriate function from your LCD library

				lcdLoc(LINE1);
                typeln("Set Timer: 0min");
            }
        }

        // Add some delay or debouncing mechanism here if needed
        usleep(10000); // Sleep for 100ms (adjust the delay as per your requirements)
    }

    return NULL;
}

int main(int argc, char *argv[])
{
	int led[3]={26,19,13};
	
	if (-1 == GPIOExport(BUTTON4)|| -1 == GPIOExport(BUTTON1)||-1==GPIOExport(BUTTON2)||-1==GPIOExport(BUTTON3))
		return(1);

	if (-1 == GPIODirection(BUTTON4, IN)|| -1 == GPIODirection(BUTTON1, IN)||-1 == GPIODirection(BUTTON2, IN)||-1 == GPIODirection(BUTTON3, IN))
		return(2);

	// Initialize the LCD module
    lcd_init(); // Replace with the initialization function provided by your LCD library

	lcdLoc(LINE1);
    typeln("Set Timer: 0min");
    // Create the button thread
    if (pthread_create(&buttonThread, NULL, buttonHandler, NULL) != 0||pthread_create(&lcdThread, NULL, lcd_Display, NULL)!=0)
    {
        fprintf(stderr, "Failed to create button thread.\n");
        return 3;
    }
	if (pthread_join(buttonThread, NULL) != 0)
    {
        fprintf(stderr, "Failed to join button thread.\n");
        return 4;
    }

    // Main thread can perform other tasks or wait for threads to finish

    // Wait for the button thread to finish
   

    // Wait for the LCD thread to finish
    if (pthread_join(lcdThread, NULL) != 0)
    {
        fprintf(stderr, "Failed to join LCD thread.\n");
        return 4;
    }

	if (-1 == GPIOUnexport(BUTTON4)||-1 == GPIOUnexport(BUTTON1)||-1==GPIOUnexport(BUTTON2)||-1==GPIOUnexport(BUTTON3))
		return(1);

	return(0);
}
