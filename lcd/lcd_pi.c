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

int client=0;

pthread_mutex_t lock;  // Mutex lock to synchronize access to shared variables
pthread_t buttonThread; // Thread for button handling
pthread_t lcdThread;    // Thread for LCD display
pthread_t client1;
pthread_t client2;
pthread_t client3;

int state=0;

int END1=0;
int END2=0;
int END3=0;


int led[4]={26,19,13,6};

void*client_1(void*arg){//다인님
    int clientfd=*((int*)arg);
    int cmd=0;
    char startmsg[2];
    char endmsg[2]="0";
    char msg[2];
    while(1){
        if(END1&&state==0){
            write(clientfd, endmsg, sizeof(endmsg));
            END1=0;
        }
        if(!state){
            ssize_t read_len=read(clientfd,startmsg,sizeof(startmsg));
            if(read_len==-1){
                perror("read() error");
            }
            if(strcmp(startmsg,"1")==0){
                state=1;
                lcdLoc(LINE1);
                typeln("Welcome!");
                sleep(1);
                ClrLcd();
                lcdLoc(LINE1);
                typeln("Set Timer: 0min");

                // Create the button thread
                if (pthread_create(&buttonThread, NULL, buttonHandler, NULL) != 0||pthread_create(&lcdThread, NULL, lcd_Display, NULL)!=0)
                {
                    fprintf(stderr, "Failed to create button/lcd thread.\n");
                    return 3;
                }
                    // Wait for the LCD thread to finish
                if (pthread_join(lcdThread, NULL) != 0)
                {
                    fprintf(stderr, "Failed to join LCD thread.\n");
                    return 4;
                }
	            if (pthread_detach(buttonThread, NULL) != 0)
                {
                    fprintf(stderr, "Failed to join button thread.\n");
                    return 4;
                }
            }   
        }
        else if(state){
            ssize_t read_len = read(clientfd, msg, sizeof(msg));
            if(read_len==-1){
                perror("read() error");
            }
            int light;
            if(strcmp(msg,"1")==0)
                light = 1;
            else
                light = 0;
            
            GPIOWrite(led[0], light);
        }
    }
    pthread_mutex_lock(&lock);
    client--;
    pthread_mutex_unlock(&lock);
    close(clientfd);
    free(arg);
    return NULL;
}
void*client_2(void*arg){//지영님
    int clientfd=*((int*)arg)
    int prev=0;
    char startmsg[2]="1";
    char endmsg[2]="0";
    char msg[2];
    while(1){
        if(END2){
            write(clientfd, endmsg, sizeof(endmsg));
            prev=0;
            END2=0;
        }
        if(state&&!prev){
            write(clientfd, startmsg, sizeof(startmsg));
            prev=1;
        }
        else if(state&&prev){
            ssize_t read_len = read(clientfd, msg, sizeof(msg));
            if(read_len==-1){
                perror("read() error");
            }
            if(strcmp(msg,"0")==0){
                GPIOWrite(led[1],0);
                GPIOWrite(led[2],0);
            }
            else if(strcmp(msg,"1")==0){
                GPIOWrite(led[1],1);
                GPIOWrite(led[2],0);
            }
            else if(strcmp(msg,"2")==0){
                GPIOWrite(led[1],0);
                GPIOWrite(led[2],1);
            }
            else if(strcmp(msg,"3")==0){
                GPIOWrite(led[1],1);
                GPIOWrite(led[2],1);
            }
        }
    }
    pthread_mutex_lock(&lock);
    client--;
    pthread_mutex_unlock(&lock);
    close(clientfd);
    free(arg);
    return NULL;

}
void*client_3(void*arg){//준서님
    int clientfd=*((int*)arg);
    int prev=0;
    char startmsg[2]="1";
    char endmsg[2]="0";
    char msg[2];
    while(1){
        if(END3){
            write(clientfd, endmsg, sizeof(endmsg));
            prev=0;
            END3=0;
        }
        if(state&&!prev){
            write(clientfd, startmsg, sizeof(startmsg));
            prev=1;
        }
        else if(state&&prev){
            ssize_t read_len = read(clientfd, msg, sizeof(msg));
            if(read_len==-1){
                perror("read() error");
            }
            int light;
            if(strcmp(msg,"1")==0)
                light = 1;
            else
                light = 0;
            
            GPIOWrite(led[3], light);
        }
    }
    pthread_mutex_lock(&lock);
    client--;
    pthread_mutex_unlock(&lock);
    close(clientfd);
    free(arg);
    return NULL;

}


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
                lcdLoc(LINE2);
                typeln("PRESS BUTTON");
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
                        typeln("THE END");
                        sleep(1);
                        ClrLcd();
                        state=0;
                        END1=1;
                        END2=1;
                        END3=1; 
                        pthread_cancel(buttonThread);
                        break;
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
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS,NULL);
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

        usleep(10000); // Sleep for 100ms (adjust the delay as per your requirements)
    }

    return NULL;
}

int main(int argc, char *argv[])
{
	if(argc!=2){
        printf("Usage : %s <port>\n",argv[0]);
    }

	int listen_fd = socket(PF_INET, SOCK_STREAM, 0);
    if(listen_fd == -1){
        error_handling("socket() error");
    }
    struct sockaddr_in serv_addr; 
    memset(&serv_addr, 0 , sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(atoi(argv[1]));

    if(bind(listen_fd, (struct sockaddr*) &serv_addr, sizeof(serv_addr))==-1){
        error_handling("bind() error");
    }
    if(listen(listen_fd,SOMAXCONN)==-1){
        error_handling("listen() error");
    }

	if (-1 == GPIOExport(BUTTON4)|| -1 == GPIOExport(BUTTON1)||-1==GPIOExport(BUTTON2)||-1==GPIOExport(BUTTON3))
		return(1);

	if (-1 == GPIODirection(BUTTON4, IN)|| -1 == GPIODirection(BUTTON1, IN)||-1 == GPIODirection(BUTTON2, IN)||-1 == GPIODirection(BUTTON3, IN))
		return(2);

    lcd_init(); 

    while(1){
        struct sockaddr_in clientaddr;
        socklen_t clientaddrlen=sizeof(clientaddr);
        pthread_mutex_lock(&lock);
        int *clientfd=(int *)malloc(sizeof(int));
        *clientfd=accept(listen_fd,(struct sockaddr *)&clientaddr,&clientaddrlen);
        client++;
        pthread_mutex_unlock(&lock);
        printf("client connected %d:\n",client);
        if(client==1)
        {
            if(pthread_create(&client1,NULL,client_1,clientfd)!=0){
                perror("pthread_create() error\n");
                free(clientfd);
                client--;
            }
        }
        else if(client==2){
            if(pthread_create(&client2,NULL,client_2,clientfd)!=0){
                perror("pthrea_create() error\n");
                free(clientfd);
                client--;
            }
        }
        else if(client==3){
            if(pthread_create(&client3,NULL,client_3,clientfd)!=0){
                perror("pthrea_create() error\n");
                free(clientfd);
                client--;
            }
            break;
        }

    }
////////////////////////////////////////////////////////////////

	

    // Main thread can perform other tasks or wait for threads to finish

    // Wait for the button thread to finish
   
    ////////////////////////////////////////////////////////////////////////////
    if (pthread_join(client1, NULL) != 0)
    {
        fprintf(stderr, "Failed to join client 1 thread.\n");
        return 4;
    } 
    if (pthread_join(client2, NULL) != 0)
    {
        fprintf(stderr, "Failed to join client 2 thread.\n");
        return 4;
    } 
    if (pthread_join(client3, NULL) != 0)
    {
        fprintf(stderr, "Failed to join client 3 thread.\n");
        return 4;
    } 

	if (-1 == GPIOUnexport(BUTTON4)||-1 == GPIOUnexport(BUTTON1)||-1==GPIOUnexport(BUTTON2)||-1==GPIOUnexport(BUTTON3))
		return(1);

	return(0);
}
