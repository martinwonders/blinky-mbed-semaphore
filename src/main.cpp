#include <mbed.h>
#include <C12832.h>

Semaphore available;    /*set up a semaphore to coordinate reading and displaying */

enum { ON, OFF };


volatile float voltage1; /* shared variable between threads */
volatile float voltage2; /* shared variable between threads */

void read(void)
{
	AnalogIn pot1 (A0);
        AnalogIn pot2 (A1);
	while(true){
                float v1 = pot1 * 3.3;          /* convert to volts see schematic */
                float v2 = pot2 * 3.3; 
                /* if data has changed by some threshold */
                /* Add code here for the other sensors */
		if(abs(v1 - voltage1) > 0.3 || abs(v2 - voltage2) > 0.3) {  
	                /*2.1.1 the semaphore is not released unless the 
                        voltage changes by +-0.3 volts so this stops the
                        LCD from running continuously */
			voltage1 = v1;          /* write data to shared variables */
                        voltage2 = v2;          
			available.release();   /* signal data is available */
		}
		wait(0.2);  /* measure at 5Hz sample rate */
	}
}

void display(void)
{
	// Using Arduino pin notation
	C12832 lcd(D11, D13, D12, D7, D10);
        DigitalOut red(D5,1);
	DigitalOut blue(D8);
	blue = ON;              /* turn on LED to show thread active */
	
	while(true){
		available.wait(); /* wait for data to be available */
		red = ON;         /* indicate writing to display */
		lcd.locate(0,0);
		lcd.printf("potentiometer 1: %4.2fV", voltage1);
                lcd.locate(0,8);
		lcd.printf("potentiometer 2: %4.2fV", voltage2);
		red = OFF;        /* indicate finished writing */
	}
}

/* if we changed the sense of the semaphore i.e If the LCD was treated as a critical resource and we release() once it has updated and wait() for this in the read function i.e the sensor threads had to wait for it to be available. The pots would be sampled based on the time it takes
to update the LCD i.e. we'd miss a lot of readings */

int main(void)
{
	Thread scan;
	Thread update;

	update.start(display);

	wait(3); /* wait to show display does not update until data is ready */

	scan.start(read);

	update.join();
}

