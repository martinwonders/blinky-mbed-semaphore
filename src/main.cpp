#include <mbed.h>

#include <C12832.h>

Semaphore available;

enum { ON, OFF };


volatile float volage1; /* shared variable between threads */

void read1(void)
{
	AnalogIn pot1 (A0);
	while(true){
		float v = pot1*3.3;       /* convert to volts see schematic */
		if(abs(v-volage1)>0.1){      /* if data has changed by some theshold */
			volage1=v;               /* write data to shared vaiable */
			available.release();  /* signal data is available */
		}
		wait(0.2);  /* measure at 5Hz sample rate */
	}
}

void display(void)
{
	DigitalOut red(D5,1);
	DigitalOut blue(D8);
	blue = ON; /* turn on LED to show thread active */
	// Using Arduino pin notation
	C12832 lcd(D11, D13, D12, D7, D10);
	while(true){
		available.wait(); /* wait for data to be available */
		red = ON;  /* indicate writing to display */
		lcd.locate(0,0);
		lcd.printf("potentiometer 1: %4.2fV", volage1);
		red = OFF;/* finished writing */
	}
}

int main(void)
{
	Thread scan1;
	Thread update;

	update.start(display);

	wait(3); /* wait to show display does not update until data is ready */

	scan1.start(read1);

	update.join();
}

