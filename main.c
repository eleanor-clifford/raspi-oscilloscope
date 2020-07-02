//  GPIO Access macros
//  15-January-2012
//  Dom and Gert
//  Revised: 15-Feb-2013
// Access from ARM Running Linux

#define BCM2708_PERI_BASE        0x20000000
#define GPIO_BASE                (BCM2708_PERI_BASE + 0x200000) /* GPIO controller */

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <unistd.h>

#define PAGE_SIZE (4*1024)
#define BLOCK_SIZE (4*1024)

int  mem_fd;
void *gpio_map;

// I/O access
volatile unsigned *gpio;


// GPIO setup macros. Always use INP_GPIO(x) before using OUT_GPIO(x) or SET_GPIO_ALT(x,y)
#define INP_GPIO(g) *(gpio+((g)/10)) &= ~(7<<(((g)%10)*3))
#define OUT_GPIO(g) *(gpio+((g)/10)) |=  (1<<(((g)%10)*3))
#define SET_GPIO_ALT(g,a) *(gpio+(((g)/10))) |= (((a)<=3?(a)+4:(a)==4?3:2)<<(((g)%10)*3))

#define GPIO_SET *(gpio+7)  // sets   bits which are 1 ignores bits which are 0
#define GPIO_CLR *(gpio+10) // clears bits which are 1 ignores bits which are 0

#define GET_GPIO(g) (*(gpio+13)&(1<<g)) // 0 if LOW, (1<<g) if HIGH
#define GET_GPIO_ALL *(gpio+13)

#define GPIO_PULL *(gpio+37) // Pull up/pull down
#define GPIO_PULLCLK0 *(gpio+38) // Pull up/pull down clock


// Uses code from Raspberry Compote http://raspberrycompote.blogspot.com/2012/12/low-level-graphics-on-raspberry-pi-part_9509.html
// and glyphs from JSBallista https://github.com/JSBattista/Characters_To_Linux_Buffer_THE_HARD_WAY

#include <string.h>
#include <linux/fb.h>
#include <sys/ioctl.h>
#include <time.h>
#include <math.h>
#include "EngNotation.h"
// These are the sizes of the individual character arrays
#define CHAR_ARR__10x14 168
#define CHAR_HEIGHT 14
#define CHAR_WIDTH 10
#define CHAR_SPACING 15
const unsigned char *ascii_characters[256];	// Store the ASCII character set, but can have some elements blank
struct fb_var_screeninfo orig_vinfo;
struct fb_var_screeninfo vinfo;
struct fb_fix_screeninfo finfo;
long int screensize;
void setup_chars();
void display_ascii(char *fbp, char c, int x, int y);
void draw_background(char *fbp, double t);
void draw_rising_trigger(char *fbp, u_int8_t *data, int data_len, u_int8_t trigger_low, u_int8_t trigger_high);
void setup_io();
void setup_framebuffer();
void cleanup_framebuffer();
char *fbp = 0;
int fbfd = 0;
void request_data(int data_len, int delay_usec, u_int8_t *data);

// application entry point
int main(int argc, char* argv[])
{
    srand(time(NULL));
    setup_chars();
    setup_framebuffer();
    setup_io();
    // scope time...
    u_int8_t *data = malloc(512*sizeof(u_int8_t));
    int i,j;
    // clear framebuffer
    for (i = 0; i < vinfo.yres*vinfo.xres; i++) fbp[i] = 0;
    // scope
	while (1) {
		draw_background(fbp, 1.234e-9);
		request_data(512,4,data);
		draw_rising_trigger(fbp,data,512,32,224);
	}
	free(data);
    sleep(5);

  	cleanup_framebuffer();
  	return 0;
}
void display_ascii(char *fbp, char c, int x, int y)
{
    int yi;
    for (yi = 0; yi < CHAR_HEIGHT; yi++) {
        memcpy((char*)(fbp + x + (y+yi)*finfo.line_length),(char*)(ascii_characters[c] + CHAR_WIDTH*yi),CHAR_WIDTH*sizeof(u_int8_t));
    }
}
void draw_background(char *fbp, double t) {
    // create fixed background grid
    // we will use 512x256px for the actual trace, and a resolution of 640x360px
    // (8 bit ADC results in 256 levels so there's no point quantising to something else and losing information)
    // assume for now that timestep is constant t
    // although it defo won't be lol
    // lets split into 8x4 squares because why not. So lines at 64px and why not tick at 8px
    // leave 52px left/right and 64px up/down
    // i don't care too much about efficiency here because it will be executed infrequently
    int x,y;
    for (x = 52; x < 564; x++) {
        for (y = 64; y < 320; y++) {
            if ((x-52)%64 == 0 || y%64 == 0) fbp[x + y * finfo.line_length] = 2;
            else {
                int xrel = (x-52)%64;
                int yrel = y%64;
                if ((x-52)%8 == 0 && (yrel < 4 || yrel > 60)) fbp[x + y * finfo.line_length] = 2;
                if (y%8 == 0 && (xrel < 4 || xrel > 60)) fbp[x + y * finfo.line_length] = 2;
            }
        }
    }
    // write timestep in full up top then round for the markings
    char *t_eng_notation = to_engineering_string(t,8,0);
    char *test_string = "ABCDEFGHIJKkLMmNOPQRSsTUuVWXYZ\0";
    int i = 0;
    do {
        display_ascii(fbp, test_string[i], 10+i*CHAR_SPACING, 10);
    } while (test_string[++i] != '\0');
    char *test_string_2 = "0123456789\0";
    i = 0;
    do {
        display_ascii(fbp, test_string_2[i], 10+i*CHAR_SPACING, 30);
    } while (test_string_2[++i] != '\0');
	free(t_eng_notation);
}
void request_data(int data_len, int delay_usec, u_int8_t *data) {
	u_int16_t raw_data;
	int i;
	for (i=0; i<data_len; i++)
	{
		raw_data = GET_GPIO_ALL;
		data[i] = ((raw_data >> 4) & 248) | ((raw_data >> 2) & 7);
		if (delay_usec) usleep(delay_usec);
	}
	return data;
}
void draw_rising_trigger(char *fbp, u_int8_t *data, int data_len, u_int8_t trigger_low, u_int8_t trigger_high) {
    /* Trigger the signal based on an average of the times 
     * when the signal passes a high and low trigger
     *  |   /\         |   /\ 
     *  |  /  \        |  /  \
     *  |_/____\_______|_/____\____ high trigger
     *  |/      \      |/      \
     *_/|________\____/|________\__ low trigger
     *  |         \  / |
     *  |          \/  |            vertical lines will be centered on screen
     * 
     * this allows noise within trigger levels without it spasming on screen
     *
     * making this low efficiency now, will have to optimise later
     * prioritise low memory use, since `data` should fill
     * as much of main memory as possible
     */
    // find rising edges
    int i,j;
    bool low = false; // data was above low trigger
    bool high = false; // data has hit high trigger (resets on low)
    int low_marker = 0; // the index where signal rose above low trigger
    int trigger_marker; // index to trigger on
    int t_scaling = 1; // we will want later to skip/amalgamate some of the datapoints to show higher frequencies
    int startval,endval;
    //int last_trigger = 0;
    //double t_avg = 0; // average period so far (in units of timestep)
    //int rise_counter = 0
    u_int8_t *linebuffer = calloc(512,sizeof(u_int8_t)); // buffer of last value drawn to screen at this position
    // this means we can erase and write at the same position with minimal latency so that something is on the screen at all times
    for (i = 0; i < data_len; i++) {
        //printf("i: %d, data: %d\n",i,data[i]);
        if (data[i] > trigger_high) {
            if (!high) {

                if (!low) {
                    low_marker = i;
                    low = true;
                }
                trigger_marker = (int)(i + low_marker)/2; // close enough
                //t_avg = (t_avg*(rise_counter++) + trigger_marker)/rise_counter;
                // for now, lets just redraw on every rising edge, but
                // in future it's not worth it when several waveforms may be on screen
                if (trigger_marker < 256) startval = 0;
                else startval = trigger_marker - 256;
                if (trigger_marker + 256 > data_len) endval = data_len-1;
                else endval = trigger_marker + 256;
                //printf("st: %d tr: %d en: %d\n",startval,trigger_marker,endval);
                //sleep(1);
                for (j = startval; j < endval; j++) {
                //    printf("j: %d data: %d\n",j,data[j]);
                    // clear the pixel at this point in the linebuffer
                    fbp[308 + j - trigger_marker + (64+linebuffer[256+j-trigger_marker]) * finfo.line_length] = 0;
                    // set new pixel
                    fbp[308 + j - trigger_marker + (64+data[j]) * finfo.line_length] = 1; // 308 = 52 + 256, 52 is zero point
                    // write into linebuffer at new position
                    linebuffer[256+j-trigger_marker] = data[j];
                }
                usleep(10000); // wait to make sure it actually shows up
                high = true; // now wait until below low
            }
        }
        else if (data[i] > trigger_low) {
            if (!low) {
                low = true; // data has hit low trigger
                low_marker = i;
            }
        }
        else {
            high = false;
            low = false;
        }
    }
	free(linebuffer);
     
}
void setup_io()
{
	/* open /dev/mem */
	if ((mem_fd = open("/dev/mem", O_RDWR|O_SYNC) ) < 0) {
		printf("can't open /dev/mem \n");
		exit(-1);
	}

	/* mmap GPIO */
	gpio_map = mmap(
	   NULL,             //Any adddress in our space will do
	   BLOCK_SIZE,       //Map length
	   PROT_READ|PROT_WRITE,// Enable reading & writting to mapped memory
	   MAP_SHARED,       //Shared with other processes
	   mem_fd,           //File to map
	   GPIO_BASE         //Offset to GPIO peripheral
	);

	close(mem_fd); //No need to keep mem_fd open after mmap

	if (gpio_map == MAP_FAILED) {
		printf("mmap error %d\n", (int)gpio_map);//errno also set!
		exit(-1);
	}

   // Always use volatile pointer!
   gpio = (volatile unsigned *)gpio_map;
   
   // set inputs from ADC
   int g;
   for (g=2; g<= 4; g++) INP_GPIO(g);
   for (g=7; g<=11; g++) INP_GPIO(g);

}
void setup_framebuffer() {
  
  screensize = 0;
  // Open the file for reading and writing
  fbfd = open("/dev/fb0", O_RDWR);
  if (!fbfd) {
    printf("Error: cannot open framebuffer device.\n");
    exit(1);
  }
  printf("The framebuffer device was opened successfully.\n");

  // Get variable screen information
  if (ioctl(fbfd, FBIOGET_VSCREENINFO, &vinfo)) {
    printf("Error reading variable information.\n");
  }
  printf("Original %dx%d, %dbpp\n", vinfo.xres, vinfo.yres, 
         vinfo.bits_per_pixel );

  // Store for reset (copy vinfo to vinfo_orig)
  memcpy(&orig_vinfo, &vinfo, sizeof(struct fb_var_screeninfo));

  // Change variable info
  vinfo.bits_per_pixel = 8;
  if (ioctl(fbfd, FBIOPUT_VSCREENINFO, &vinfo)) {
    printf("Error setting variable information.\n");
  }
  
  // Get fixed screen information
  if (ioctl(fbfd, FBIOGET_FSCREENINFO, &finfo)) {
    printf("Error reading fixed information.\n");
  }

  // map fb to user mem 
  screensize = finfo.smem_len;
  fbp = (char*)mmap(0, 
                    screensize, 
                    PROT_READ | PROT_WRITE, 
                    MAP_SHARED, 
                    fbfd, 
                    0);

  if ((int)fbp == -1) {
    printf("Failed to mmap.\n");
	exit(1);
  }
}
void cleanup_framebuffer() {
	// cleanup
  	munmap(fbp, screensize);
  	if (ioctl(fbfd, FBIOPUT_VSCREENINFO, &orig_vinfo)) {
    	printf("Error re-setting variable information.\n");
  	}
  	close(fbfd);
}
void setup_chars()
{
	{
		const unsigned char A__10x14[CHAR_ARR__10x14] = {
							        				0,0,0,0,1,1,0,0,0,0,
									        		0,0,0,1,1,1,1,0,0,0,
        											0,0,1,1,1,1,1,1,0,0,
		        									0,1,1,1,0,0,1,1,1,0,
				        							1,1,1,0,0,0,0,1,1,1,
						        					1,1,0,0,0,0,0,0,1,1,
								        			1,1,0,0,0,0,0,0,1,1,
										        	1,1,0,0,0,0,0,0,1,1,
        											1,1,1,1,1,1,1,1,1,1,
		        									1,1,1,1,1,1,1,1,1,1,
				        							1,1,0,0,0,0,0,0,1,1,
						        					1,1,0,0,0,0,0,0,1,1,
								        			1,1,0,0,0,0,0,0,1,1,
										        	1,1,0,0,0,0,0,0,1,1
		};
		const unsigned char B__10x14[CHAR_ARR__10x14] = {
													1,1,1,1,1,1,1,0,0,0,
													1,1,1,1,1,1,1,1,1,0,
													1,1,0,0,0,0,0,1,1,1,
													1,1,0,0,0,0,0,0,1,1,
													1,1,0,0,0,0,0,0,1,1,
													1,1,0,0,0,0,0,1,1,1,
													1,1,1,1,1,1,1,1,1,0,
													1,1,1,1,1,1,1,1,0,0,
													1,1,0,0,0,0,0,1,1,0,
													1,1,0,0,0,0,0,0,1,1,
													1,1,0,0,0,0,0,0,1,1,
													1,1,0,0,0,0,0,0,1,1,
													1,1,0,0,0,0,0,1,1,0,
													1,1,1,1,1,1,1,0,0,0
		};
		const unsigned char C__10x14[CHAR_ARR__10x14] = {
													0,0,0,1,1,1,1,1,0,0,
													0,0,1,1,1,1,1,1,0,0,
													1,1,1,0,0,0,0,1,1,0,
													1,1,0,0,0,0,0,0,1,1,
													1,1,0,0,0,0,0,0,1,1,
													1,1,0,0,0,0,0,0,0,0,
													1,1,0,0,0,0,0,0,0,0,
													1,1,0,0,0,0,0,0,0,0,
													1,1,0,0,0,0,0,0,0,0,
													1,1,0,0,0,0,0,0,1,1,
													1,1,0,0,0,0,0,0,1,1,
													1,1,1,0,0,0,0,1,1,0,
													0,0,1,1,1,1,1,1,0,0,
													0,0,0,1,1,1,1,1,0,0
		};
		const unsigned char D__10x14[CHAR_ARR__10x14] = {
													1,1,1,1,1,1,1,0,0,0,
													1,1,1,1,1,1,1,1,1,0,
													1,1,0,0,0,0,0,1,1,1,
													1,1,0,0,0,0,0,0,1,1,
													1,1,0,0,0,0,0,0,1,1,
													1,1,0,0,0,0,0,0,1,1,
													1,1,0,0,0,0,0,0,1,1,
													1,1,0,0,0,0,0,0,1,1,
													1,1,0,0,0,0,0,0,1,1,
													1,1,0,0,0,0,0,0,1,1,
													1,1,0,0,0,0,0,0,1,1,
													1,1,0,0,0,0,0,0,1,1,
													1,1,0,0,0,0,0,1,1,1,
													1,1,1,1,1,1,1,0,0,0
		};
		const unsigned char E__10x14[CHAR_ARR__10x14] = {
													1,1,1,1,1,1,1,1,1,1,
													1,1,1,1,1,1,1,1,1,1,
													1,1,0,0,0,0,0,0,0,0,
													1,1,0,0,0,0,0,0,0,0,
													1,1,0,0,0,0,0,0,0,0,
													1,1,0,0,0,0,0,0,0,0,
													1,1,1,1,1,1,1,1,0,0,
													1,1,1,1,1,1,1,1,0,0,
													1,1,0,0,0,0,0,0,0,0,
													1,1,0,0,0,0,0,0,0,0,
													1,1,0,0,0,0,0,0,0,0,
													1,1,0,0,0,0,0,0,0,0,
													1,1,1,1,1,1,1,1,1,1,
													1,1,1,1,1,1,1,1,1,1
		};
		const unsigned char F__10x14[CHAR_ARR__10x14] = {
													1,1,1,1,1,1,1,1,1,1,
													1,1,1,1,1,1,1,1,1,1,
													1,1,0,0,0,0,0,0,0,0,
													1,1,0,0,0,0,0,0,0,0,
													1,1,0,0,0,0,0,0,0,0,
													1,1,0,0,0,0,0,0,0,0,
													1,1,1,1,1,1,1,1,0,0,
													1,1,1,1,1,1,1,1,0,0,
													1,1,0,0,0,0,0,0,0,0,
													1,1,0,0,0,0,0,0,0,0,
													1,1,0,0,0,0,0,0,0,0,
													1,1,0,0,0,0,0,0,0,0,
													1,1,0,0,0,0,0,0,0,0,
													1,1,0,0,0,0,0,0,0,0
		};
		const unsigned char G__10x14[CHAR_ARR__10x14] = {
													0,0,0,1,1,1,1,1,0,0,
													0,0,1,1,1,1,1,1,0,0,
													1,1,1,0,0,0,0,1,1,0,
													1,1,0,0,0,0,0,0,1,1,
													1,1,0,0,0,0,0,0,1,1,
													1,1,0,0,0,0,0,0,0,0,
													1,1,0,0,0,0,0,0,0,0,
													1,1,0,0,0,0,0,0,0,0,
													1,1,0,0,0,0,0,0,0,0,
													1,1,0,0,0,1,1,1,1,1,
													1,1,0,0,0,1,1,1,1,1,
													1,1,1,0,0,0,0,1,1,1,
													0,0,1,1,1,1,1,1,0,0,
													0,0,0,1,1,1,1,1,0,0
		};
		const unsigned char H__10x14[CHAR_ARR__10x14] = {
													1,1,0,0,0,0,0,0,1,1,
													1,1,0,0,0,0,0,0,1,1,
													1,1,0,0,0,0,0,0,1,1,
													1,1,0,0,0,0,0,0,1,1,
													1,1,0,0,0,0,0,0,1,1,
													1,1,0,0,0,0,0,0,1,1,
													1,1,1,1,1,1,1,1,1,1,
													1,1,1,1,1,1,1,1,1,1,
													1,1,0,0,0,0,0,0,1,1,
													1,1,0,0,0,0,0,0,1,1,
													1,1,0,0,0,0,0,0,1,1,
													1,1,0,0,0,0,0,0,1,1,
													1,1,0,0,0,0,0,0,1,1,
													1,1,0,0,0,0,0,0,1,1
		};
		const unsigned char I__10x14[CHAR_ARR__10x14] = {
													1,1,1,1,1,1,1,1,1,1,
													1,1,1,1,1,1,1,1,1,1,
													0,0,0,0,1,1,0,0,0,0,
													0,0,0,0,1,1,0,0,0,0,
													0,0,0,0,1,1,0,0,0,0,
													0,0,0,0,1,1,0,0,0,0,
													0,0,0,0,1,1,0,0,0,0,
													0,0,0,0,1,1,0,0,0,0,
													0,0,0,0,1,1,0,0,0,0,
													0,0,0,0,1,1,0,0,0,0,
													0,0,0,0,1,1,0,0,0,0,
													0,0,0,0,1,1,0,0,0,0,
													1,1,1,1,1,1,1,1,1,1,
													1,1,1,1,1,1,1,1,1,1
		};
		const unsigned char J__10x14[CHAR_ARR__10x14] = {
													1,1,1,1,1,1,1,1,1,1,
													1,1,1,1,1,1,1,1,1,1,
													0,0,0,0,1,1,0,0,0,0,
													0,0,0,0,1,1,0,0,0,0,
													0,0,0,0,1,1,0,0,0,0,
													0,0,0,0,1,1,0,0,0,0,
													0,0,0,0,1,1,0,0,0,0,
													0,0,0,0,1,1,0,0,0,0,
													0,0,0,0,1,1,0,0,0,0,
													0,0,0,0,1,1,0,0,0,0,
													1,1,0,0,1,1,0,0,0,0,
													1,1,0,0,1,1,0,0,0,0,
													0,1,1,1,1,1,0,0,0,0,
													0,0,1,1,1,0,0,0,0,0
		};
		const unsigned char K__10x14[CHAR_ARR__10x14] = {
													1,1,0,0,0,0,0,0,1,1,
													1,1,0,0,0,0,0,1,1,0,
													1,1,0,0,0,0,1,1,0,0,
													1,1,0,0,0,1,1,0,0,0,
													1,1,0,0,1,1,0,0,0,0,
													1,1,1,1,1,0,0,0,0,0,
													1,1,1,1,0,0,0,0,0,0,
													1,1,1,1,0,0,0,0,0,0,
													1,1,1,1,0,0,0,0,0,0,
													1,1,0,1,1,0,0,0,0,0,
													1,1,0,0,1,1,0,0,0,0,
													1,1,0,0,0,0,1,1,0,0,
													1,1,0,0,0,0,0,1,1,0,
													1,1,0,0,0,0,0,0,1,1
		};
		const unsigned char L__10x14[CHAR_ARR__10x14] = {
													1,1,0,0,0,0,0,0,0,0,
													1,1,0,0,0,0,0,0,0,0,
													1,1,0,0,0,0,0,0,0,0,
													1,1,0,0,0,0,0,0,0,0,
													1,1,0,0,0,0,0,0,0,0,
													1,1,0,0,0,0,0,0,0,0,
													1,1,0,0,0,0,0,0,0,0,
													1,1,0,0,0,0,0,0,0,0,
													1,1,0,0,0,0,0,0,0,0,
													1,1,0,0,0,0,0,0,0,0,
													1,1,0,0,0,0,0,0,0,0,
													1,1,0,0,0,0,0,0,0,0,
													1,1,1,1,1,1,1,1,1,1,
													1,1,1,1,1,1,1,1,1,1
		};
		const unsigned char M__10x14[CHAR_ARR__10x14] = {
													1,1,0,0,0,0,0,0,1,1,
													1,1,1,0,0,0,0,1,1,1,
													1,1,1,1,0,0,1,1,1,1,
													1,1,0,1,1,1,1,0,1,1,
													1,1,0,0,1,1,0,0,1,1,
													1,1,0,0,1,1,0,0,1,1,
													1,1,0,0,0,0,0,0,1,1,
													1,1,0,0,0,0,0,0,1,1,
													1,1,0,0,0,0,0,0,1,1,
													1,1,0,0,0,0,0,0,1,1,
													1,1,0,0,0,0,0,0,1,1,
													1,1,0,0,0,0,0,0,1,1,
													0,1,0,0,0,0,0,0,1,1,
													1,1,0,0,0,0,0,0,1,1
		};
		const unsigned char N__10x14[CHAR_ARR__10x14] = {
													1,1,0,0,0,0,0,0,1,1,
													1,1,0,0,0,0,0,0,1,1,
													1,1,0,0,0,0,0,0,1,1,
													1,1,1,0,0,0,0,0,1,1,
													1,1,1,1,0,0,0,0,1,1,
													1,1,1,1,1,0,0,0,1,1,
													1,1,0,1,1,1,0,0,1,1,
													1,1,0,0,1,1,1,0,1,1,
													1,1,0,0,0,1,1,1,1,1,
													1,1,0,0,0,0,1,1,1,1,
													1,1,0,0,0,0,0,1,1,1,
													1,1,0,0,0,0,0,0,1,1,
													1,1,0,0,0,0,0,0,1,1,
													1,1,0,0,0,0,0,0,1,1
		};
		const unsigned char O__10x14[CHAR_ARR__10x14] = {
													0,0,1,1,1,1,1,1,0,0,
													0,1,1,1,1,1,1,1,1,0,
													1,1,1,0,0,0,0,1,1,1,
													1,1,0,0,0,0,0,0,1,1,
													1,1,0,0,0,0,0,0,1,1,
													1,1,0,0,0,0,0,0,1,1,
													1,1,0,0,0,0,0,0,1,1,
													1,1,0,0,0,0,0,0,1,1,
													1,1,0,0,0,0,0,0,1,1,
													1,1,0,0,0,0,0,0,1,1,
													1,1,0,0,0,0,0,0,1,1,
													1,1,1,0,0,0,0,1,1,1,
													0,1,1,1,1,1,1,1,1,0,
													0,0,1,1,1,1,1,1,0,0
		};
		const unsigned char P__10x14[CHAR_ARR__10x14] = {
													1,1,1,1,1,1,1,1,0,0,
													1,1,1,1,1,1,1,1,0,0,
													1,1,0,0,0,0,0,1,1,0,
													1,1,0,0,0,0,0,0,1,1,
													1,1,0,0,0,0,0,0,1,1,
													1,1,0,0,0,0,0,1,1,0,
													1,1,1,1,1,1,1,1,0,0,
													1,1,1,1,1,1,1,0,0,0,
													1,1,0,0,0,0,0,0,0,0,
													1,1,0,0,0,0,0,0,0,0,
													1,1,0,0,0,0,0,0,0,0,
													1,1,0,0,0,0,0,0,0,0,
													1,1,0,0,0,0,0,0,0,0,
													1,1,0,0,0,0,0,0,0,0
		};
		const unsigned char Q__10x14[CHAR_ARR__10x14] = {
													0,0,1,1,1,1,1,1,0,0,
													0,1,1,1,1,1,1,1,1,0,
													1,1,1,0,0,0,0,1,1,1,
													1,1,0,0,0,0,0,0,1,1,
													1,1,0,0,0,0,0,0,1,1,
													1,1,0,0,0,0,0,0,1,1,
													1,1,0,0,0,0,0,0,1,1,
													1,1,0,0,0,0,0,0,1,1,
													1,1,0,0,0,0,0,0,1,1,
													1,1,0,0,1,1,1,0,1,1,
													1,1,0,0,0,1,1,1,1,1,
													1,1,1,0,0,0,1,1,1,1,
													0,1,1,1,1,1,1,1,1,1,
													0,0,1,1,1,1,1,1,1,1
		};
        const unsigned char R__10x14[CHAR_ARR__10x14] = {
													1,1,1,1,1,1,1,1,0,0,
													1,1,1,1,1,1,1,1,0,0,
													1,1,0,0,0,0,0,1,1,0,
													1,1,0,0,0,0,0,0,1,1,
													1,1,0,0,0,0,0,0,1,1,
													1,1,0,0,0,0,0,1,1,0,
													1,1,1,1,1,1,1,1,0,0,
													1,1,1,1,1,1,1,0,0,0,
													1,1,0,1,1,1,0,0,0,0,
													1,1,0,0,0,1,1,0,0,0,
													1,1,0,0,0,0,1,1,0,0,
													1,1,0,0,0,0,0,1,1,0,
													1,1,0,0,0,0,0,0,1,1,
													1,1,0,0,0,0,0,0,1,1
		};
		const unsigned char S__10x14[CHAR_ARR__10x14] = {
													0,0,0,1,1,1,1,0,0,0,
													0,0,1,1,1,1,1,1,0,0,
													0,1,1,0,0,0,1,1,1,0,
													1,1,0,0,0,0,0,0,1,1,
													1,1,0,0,0,0,0,0,0,0,
													1,1,1,0,0,0,0,0,0,0,
													0,1,1,1,1,1,1,0,0,0,
													0,0,1,1,1,1,1,1,0,0,
													0,0,0,0,0,0,0,1,1,0,
													0,0,0,0,0,0,0,0,1,1,
													1,1,0,0,0,0,0,0,1,1,
													1,1,1,0,0,0,0,1,1,0,
													0,0,1,1,1,1,1,1,0,0,
													0,0,0,1,1,1,1,0,0,0
		};
		const unsigned char T__10x14[CHAR_ARR__10x14] = {
													1,1,1,1,1,1,1,1,1,1,
													1,1,1,1,1,1,1,1,1,1,
													0,0,0,0,1,1,0,0,0,0,
													0,0,0,0,1,1,0,0,0,0,
													0,0,0,0,1,1,0,0,0,0,
													0,0,0,0,1,1,0,0,0,0,
													0,0,0,0,1,1,0,0,0,0,
													0,0,0,0,1,1,0,0,0,0,
													0,0,0,0,1,1,0,0,0,0,
													0,0,0,0,1,1,0,0,0,0,
													0,0,0,0,1,1,0,0,0,0,
													0,0,0,0,1,1,0,0,0,0,
													0,0,0,0,1,1,0,0,0,0,
													0,0,0,0,1,1,0,0,0,0
		};
		const unsigned char U__10x14[CHAR_ARR__10x14] = {
													1,1,0,0,0,0,0,0,1,1,
													1,1,0,0,0,0,0,0,1,1,
													1,1,0,0,0,0,0,0,1,1,
													1,1,0,0,0,0,0,0,1,1,
													1,1,0,0,0,0,0,0,1,1,
													1,1,0,0,0,0,0,0,1,1,
													1,1,0,0,0,0,0,0,1,1,
													1,1,0,0,0,0,0,0,1,1,
													1,1,0,0,0,0,0,0,1,1,
													1,1,0,0,0,0,0,0,1,1,
													1,1,0,0,0,0,0,0,1,1,
													0,1,1,0,0,0,0,1,1,0,
													0,0,1,1,1,1,1,1,0,0,
													0,0,0,1,1,1,1,0,0,0
		};
		const unsigned char V__10x14[CHAR_ARR__10x14] = {
													1,1,0,0,0,0,0,0,1,1,
													1,1,0,0,0,0,0,0,1,1,
													1,1,0,0,0,0,0,0,1,1,
													1,1,0,0,0,0,0,0,1,1,
													1,1,0,0,0,0,0,0,1,1,
													1,1,0,0,0,0,0,0,1,1,
													1,1,0,0,0,0,0,0,1,1,
													1,1,0,0,0,0,0,0,1,1,
													1,1,0,0,0,0,0,0,1,1,
													0,1,1,0,0,0,0,1,1,0,
													0,1,1,0,0,0,0,1,1,0,
													0,0,1,1,0,0,1,1,0,0,
													0,0,0,1,1,1,1,0,0,0,
													0,0,0,0,1,1,0,0,0,0
		};
		const unsigned char W__10x14[CHAR_ARR__10x14] = {
													1,1,0,0,0,0,0,0,1,1,
													1,1,0,0,0,0,0,0,1,1,
													1,1,0,0,0,0,0,0,1,1,
													1,1,0,0,0,0,0,0,1,1,
													1,1,0,0,0,0,0,0,1,1,
													1,1,0,0,0,0,0,0,1,1,
													1,1,0,0,0,0,0,0,1,1,
													1,1,0,0,0,0,0,0,1,1,
													1,1,0,0,1,1,0,0,1,1,
													1,1,0,1,1,1,1,0,1,1,
													1,1,0,1,1,1,1,0,1,1,
													1,1,1,1,0,0,1,1,1,1,
													1,1,1,0,0,0,1,1,1,1,
													1,1,0,0,0,0,0,0,1,1
		};
		const unsigned char X__10x14[CHAR_ARR__10x14] = {
													1,1,0,0,0,0,0,0,1,1,
													1,1,0,0,0,0,0,0,1,1,
													1,1,1,0,0,0,0,1,1,1,
													0,1,1,1,0,0,1,1,1,0,
													0,0,1,1,0,0,1,1,0,0,
													0,0,0,1,1,1,1,0,0,0,
													0,0,0,0,1,1,0,0,0,0,
													0,0,0,0,1,1,0,0,0,0,
													0,0,0,1,1,1,1,0,0,0,
													0,0,1,1,0,0,1,1,0,0,
													0,1,1,1,0,0,1,1,1,1,
													1,1,1,0,0,0,0,1,1,1,
													1,1,0,0,0,0,0,0,1,1,
													1,1,0,0,0,0,0,0,1,1
		};
		const unsigned char Y__10x14[CHAR_ARR__10x14] = {
													1,1,0,0,0,0,0,0,1,1,
													1,1,0,0,0,0,0,0,1,1,
													1,1,1,0,0,0,0,1,1,1,
													0,1,1,1,0,0,1,1,1,0,
													0,0,1,1,0,0,1,1,0,0,
													0,0,0,1,1,1,1,0,0,0,
													0,0,0,0,1,1,0,0,0,0,
													0,0,0,0,1,1,0,0,0,0,
													0,0,0,0,1,1,0,0,0,0,
													0,0,0,0,1,1,0,0,0,0,
													0,0,0,0,1,1,0,0,0,0,
													0,0,0,0,1,1,0,0,0,0,
													0,0,0,0,1,1,0,0,0,0,
													0,0,0,0,1,1,0,0,0,0
		};
		const unsigned char Z__10x14[CHAR_ARR__10x14] = {
													1,1,1,1,1,1,1,1,1,1,
													1,1,1,1,1,1,1,1,1,1,
													1,1,0,0,0,0,0,0,0,0,
													1,1,0,0,0,0,0,0,0,0,
													0,1,1,0,0,0,0,0,0,0,
													0,0,1,1,0,0,0,0,0,0,
													0,0,0,1,1,0,0,0,0,0,
													0,0,0,0,1,1,0,0,0,0,
													0,0,0,0,0,1,1,0,0,0,
													0,0,0,0,0,0,1,1,0,0,
													0,0,0,0,0,0,0,1,1,0,
													0,0,0,0,0,0,0,0,1,1,
													1,1,1,1,1,1,1,1,1,1,
													1,1,1,1,1,1,1,1,1,1
		};
        const unsigned char u__10x14[CHAR_ARR__10x14] = {
													0,0,0,0,0,0,0,0,0,0,
													0,0,0,0,0,0,0,0,0,0,
													0,0,0,0,0,0,0,0,0,0,
                                                    1,1,0,0,0,0,0,0,1,1,
													1,1,0,0,0,0,0,0,1,1,
													1,1,0,0,0,0,0,0,1,1,
													1,1,0,0,0,0,0,0,1,1,
													1,1,0,0,0,0,0,0,1,1,
													1,1,0,0,0,0,0,0,1,1,
													1,1,1,0,0,0,0,1,1,1,
													1,1,1,1,1,1,1,1,1,1,
													1,1,0,1,1,1,1,0,1,1,
													1,1,0,0,0,0,0,0,0,0,
													1,1,0,0,0,0,0,0,0,0
		};
        const unsigned char m__10x14[CHAR_ARR__10x14] = {
													0,0,0,0,0,0,0,0,0,0,
													0,0,0,0,0,0,0,0,0,0,
													0,0,0,0,0,0,0,0,0,0,
													0,0,0,0,0,0,0,0,0,0,
													0,0,0,0,0,0,0,0,0,0,
													0,0,0,0,0,0,0,0,0,0,
													1,1,1,0,0,0,0,1,1,1,
													1,1,1,1,0,0,1,1,1,1,
													1,1,0,1,1,1,1,0,1,1,
													1,1,0,0,1,1,0,0,1,1,
													1,1,0,0,0,0,0,0,1,1,
													1,1,0,0,0,0,0,0,1,1,
													1,1,0,0,0,0,0,0,1,1,
													1,1,0,0,0,0,0,0,1,1,
		};
		const unsigned char k__10x14[CHAR_ARR__10x14] = {
													0,0,0,0,0,0,0,0,0,0,
													0,0,0,0,0,0,0,0,0,0,
													0,0,0,0,0,0,0,0,0,0,
													1,1,0,0,0,0,1,1,0,0,
													1,1,0,0,0,1,1,0,0,0,
													1,1,0,0,1,1,0,0,0,0,
													1,1,0,1,1,0,0,0,0,0,
													1,1,1,1,0,0,0,0,0,0,
													1,1,1,1,0,0,0,0,0,0,
													1,1,1,1,0,0,0,0,0,0,
													1,1,0,1,1,0,0,0,0,0,
													1,1,0,0,1,1,0,0,0,0,
													1,1,0,0,0,1,1,0,0,0,
													1,1,0,0,0,0,1,1,0,0,
		};
        const unsigned char s__10x14[CHAR_ARR__10x14] = {
                                                    0,0,0,0,0,0,0,0,0,0,
                                                    0,0,0,0,0,0,0,0,0,0,
                                                    0,0,0,1,1,1,0,0,0,0,
													0,0,1,1,1,1,1,1,0,0,
													0,1,1,0,0,0,1,1,1,0,
													1,1,0,0,0,0,0,1,1,0,
													1,1,1,0,0,0,0,0,0,0,
													0,0,1,1,1,1,1,0,0,0,
													0,0,0,0,0,1,1,1,0,0,
													0,0,0,0,0,0,0,1,1,0,
													1,1,0,0,0,0,0,1,1,0,
													1,1,1,0,0,0,1,1,0,0,
													0,0,1,1,1,1,1,0,0,0,
													0,0,0,1,1,1,0,0,0,0
		};
        const unsigned char AR1__10x14[CHAR_ARR__10x14] = {
													0,0,0,0,1,1,0,0,0,0,
													0,0,0,1,1,1,0,0,0,0,
													0,0,1,1,1,1,0,0,0,0,
													0,0,0,0,1,1,0,0,0,0,
													0,0,0,0,1,1,0,0,0,0,
													0,0,0,0,1,1,0,0,0,0,
													0,0,0,0,1,1,0,0,0,0,
													0,0,0,0,1,1,0,0,0,0,
													0,0,0,0,1,1,0,0,0,0,
													0,0,0,0,1,1,0,0,0,0,
													0,0,0,0,1,1,0,0,0,0,
													0,0,0,0,1,1,0,0,0,0,
													0,0,1,1,1,1,1,1,0,0,
													0,0,1,1,1,1,1,1,0,0
        };
		const unsigned char AR2__10x14[CHAR_ARR__10x14] = {
													0,0,0,1,1,1,1,0,0,0,
													0,1,1,1,1,1,1,1,0,0,
													1,1,0,0,0,0,0,0,1,1,
													0,0,0,0,0,0,0,0,1,1,
													0,0,0,0,0,0,0,1,1,0,
													0,0,0,0,0,0,1,1,0,0,
													0,0,0,0,0,1,1,0,0,0,
													0,0,0,0,1,1,0,0,0,0,
													0,0,0,1,1,0,0,0,0,0,
													0,0,1,1,0,0,0,0,0,0,
													0,1,1,0,0,0,0,0,0,0,
													1,1,0,0,0,0,0,0,0,0,
													1,1,1,1,1,1,1,1,1,1,
													1,1,1,1,1,1,1,1,1,1
		};
        const unsigned char AR3__10x14[CHAR_ARR__10x14] = {
													1,1,1,1,1,1,1,1,1,1,
													1,1,1,1,1,1,1,1,1,1,
													0,0,0,0,0,0,0,0,1,1,
													0,0,0,0,0,0,0,1,1,0,
													0,0,0,0,0,0,1,1,0,0,
													0,0,0,0,0,1,1,0,0,0,
													0,0,0,0,1,1,1,1,0,0,
													0,0,0,0,0,0,0,0,1,1,
													0,0,0,0,0,0,0,0,1,1,
													0,0,0,0,0,0,0,0,1,1,
													0,0,0,0,0,0,0,0,1,1,
													1,1,0,0,0,0,0,1,1,0,
													1,1,1,1,1,1,1,1,0,0,
													0,1,1,1,1,1,1,0,0,0
		};
		const unsigned char AR4__10x14[CHAR_ARR__10x14] = {
													0,0,0,0,0,0,1,1,0,0,
													0,0,0,0,0,1,1,1,0,0,
													0,0,0,0,1,1,1,1,0,0,
													0,0,0,1,1,0,1,1,0,0,
													0,0,1,1,0,0,1,1,0,0,
													0,1,1,0,0,0,1,1,0,0,
													1,1,0,0,0,0,1,1,0,0,
													1,1,1,1,1,1,1,1,1,1,
													1,1,1,1,1,1,1,1,1,1,
													0,0,0,0,0,0,1,1,0,0,
													0,0,0,0,0,0,1,1,0,0,
													0,0,0,0,0,0,1,1,0,0,
													0,0,0,0,0,0,1,1,0,0,
													0,0,0,0,0,0,1,1,0,0
		};
		const unsigned char AR5__10x14[CHAR_ARR__10x14] = {
													1,1,1,1,1,1,1,1,1,1,
													1,1,1,1,1,1,1,1,1,1,
													1,1,0,0,0,0,0,0,0,0,
													1,1,0,0,0,0,0,0,0,0,
													1,1,0,0,0,0,0,0,0,0,
													1,1,1,1,1,1,1,0,0,0,
													1,1,1,1,1,1,1,1,0,0,
													0,0,0,0,0,0,0,1,1,0,
													0,0,0,0,0,0,0,0,1,1,
													0,0,0,0,0,0,0,0,1,1,
													1,1,0,0,0,0,0,0,1,1,
													1,1,0,0,0,0,0,1,1,0,
													0,1,1,1,1,1,1,1,0,0,
													0,0,1,1,1,1,1,0,0,0
		};
		const unsigned char AR6__10x14[CHAR_ARR__10x14] = {
													0,0,0,0,0,1,1,1,1,1,
													0,0,0,0,1,1,1,1,1,1,
													0,0,0,1,1,0,0,0,0,0,
													0,0,1,1,0,0,0,0,0,0,
													0,1,1,0,0,0,0,0,0,0,
													1,1,0,0,0,0,0,0,0,0,
													1,1,1,1,1,1,1,1,0,0,
													1,1,1,1,1,1,1,1,1,0,
													1,1,0,0,0,0,0,0,1,1,
													1,1,0,0,0,0,0,0,1,1,
													1,1,0,0,0,0,0,0,1,1,
													1,1,0,0,0,0,0,0,1,1,
													0,1,1,1,1,1,1,1,1,0,
													0,0,1,1,1,1,1,1,0,0
		};
		const unsigned char AR7__10x14[CHAR_ARR__10x14] = {
													1,1,1,1,1,1,1,1,1,1,
													1,1,1,1,1,1,1,1,1,1,
													0,0,0,0,0,0,0,0,1,1,
													0,0,0,0,0,0,0,0,1,1,
													0,0,0,0,0,0,0,1,1,0,
													0,0,0,0,0,0,1,1,0,0,
													0,0,0,0,0,1,1,0,0,0,
													0,0,0,0,1,1,0,0,0,0,
													0,0,0,0,1,1,0,0,0,0,
													0,0,0,0,1,1,0,0,0,0,
													0,0,0,0,1,1,0,0,0,0,
													0,0,0,0,1,1,0,0,0,0,
													0,0,0,0,1,1,0,0,0,0,
													0,0,0,0,1,1,0,0,0,0
		};
		const unsigned char AR8__10x14[CHAR_ARR__10x14] = {
													0,0,1,1,1,1,1,1,0,0,
													0,1,1,1,1,1,1,1,1,0,
													1,1,0,0,0,0,0,0,1,1,
													1,1,0,0,0,0,0,0,1,1,
													1,1,0,0,0,0,0,0,1,1,
													1,1,0,0,0,0,0,0,1,1,
													0,1,1,1,1,1,1,1,1,0,
													0,0,1,1,1,1,1,1,0,0,
													0,1,1,1,1,1,1,1,1,0,
													1,1,0,0,0,0,0,0,1,1,
													1,1,0,0,0,0,0,0,1,1,
													1,1,0,0,0,0,0,0,1,1,
													0,1,1,1,1,1,1,1,1,0,
													0,0,1,1,1,1,1,1,0,0
		};
		const unsigned char AR9__10x14[CHAR_ARR__10x14] = {
													0,0,1,1,1,1,1,1,0,0,
													0,1,1,1,1,1,1,1,1,0,
													1,1,0,0,0,0,0,0,1,1,
													1,1,0,0,0,0,0,0,1,1,
													1,1,0,0,0,0,0,0,1,1,
													0,1,1,1,1,1,1,1,1,1,
													0,0,1,1,1,1,1,1,1,1,
													0,0,0,0,0,0,0,1,1,1,
													0,0,0,0,0,0,0,1,1,1,
													0,0,0,0,0,0,1,1,1,0,
													0,0,0,0,0,1,1,1,0,0,
													0,0,0,0,1,1,1,0,0,0,
													1,1,1,1,1,1,0,0,0,0,
													1,1,1,1,1,0,0,0,0,0
		};
		const unsigned char AR0__10x14[CHAR_ARR__10x14] = {
													0,0,1,1,1,1,1,1,0,0,
													0,1,1,1,1,1,1,1,1,0,
													1,1,0,0,0,0,1,1,1,1,
													1,1,0,0,0,0,1,1,1,1,
													1,1,0,0,0,1,1,1,1,1,
													1,1,0,0,0,1,1,0,1,1,
													1,1,0,0,1,1,0,0,1,1,
													1,1,0,0,1,1,0,0,1,1,
													1,1,0,1,1,0,0,0,1,1,
													1,1,1,1,1,0,0,0,1,1,
													1,1,1,1,0,0,0,0,1,1,
													1,1,1,1,0,0,0,0,1,1,
													0,1,1,1,1,1,1,1,1,0,
													0,0,1,1,1,1,1,1,0,0
		};
        const unsigned char DOT__10x14[CHAR_ARR__10x14] = {
        											0,0,0,0,0,0,0,0,0,0,
		        									0,0,0,0,0,0,0,0,0,0,
				        							0,0,0,0,0,0,0,0,0,0,
						        					0,0,0,0,0,0,0,0,0,0,
								        			0,0,0,0,0,0,0,0,0,0,
										        	0,0,0,0,0,0,0,0,0,0,
        											0,0,0,0,0,0,0,0,0,0,
		        									0,0,0,0,0,0,0,0,0,0,
				        							0,0,0,0,0,0,0,0,0,0,
						        					0,0,0,0,0,0,0,0,0,0,
								        			0,0,0,1,1,1,1,0,0,0,
										        	0,0,0,1,1,1,1,0,0,0,
							        				0,0,0,1,1,1,1,0,0,0,
									        		0,0,0,1,1,1,1,0,0,0
        };
		const unsigned char COLON__10x14[CHAR_ARR__10x14] = {
				        							0,0,0,1,1,1,1,0,0,0,
        											0,0,0,1,1,1,1,0,0,0,
		        									0,0,0,1,1,1,1,0,0,0,
				        							0,0,0,1,1,1,1,0,0,0,
						        					0,0,0,1,1,1,1,0,0,0,
								        			0,0,0,0,0,0,0,0,0,0,
										        	0,0,0,0,0,0,0,0,0,0,
        											0,0,0,0,0,0,0,0,0,0,
		        									0,0,0,0,0,0,0,0,0,0,
				        							0,0,0,1,1,1,1,0,0,0,
						        					0,0,0,1,1,1,1,0,0,0,
								        			0,0,0,1,1,1,1,0,0,0,
										        	0,0,0,1,1,1,1,0,0,0,
											        0,0,0,1,1,1,1,0,0,0
		};
		const unsigned char SPACE__10x14[CHAR_ARR__10x14] = {
													0,0,0,0,0,0,0,0,0,0,
													0,0,0,0,0,0,0,0,0,0,
													0,0,0,0,0,0,0,0,0,0,
													0,0,0,0,0,0,0,0,0,0,
													0,0,0,0,0,0,0,0,0,0,
													0,0,0,0,0,0,0,0,0,0,
													0,0,0,0,0,0,0,0,0,0,
													0,0,0,0,0,0,0,0,0,0,
													0,0,0,0,0,0,0,0,0,0,
													0,0,0,0,0,0,0,0,0,0,
													0,0,0,0,0,0,0,0,0,0,
													0,0,0,0,0,0,0,0,0,0,
													0,0,0,0,0,0,0,0,0,0,
													0,0,0,0,0,0,0,0,0,0
		};

		ascii_characters[32] = SPACE__10x14;
		ascii_characters[46] = DOT__10x14;
		ascii_characters[48] = AR0__10x14;
		ascii_characters[49] = AR1__10x14;
		ascii_characters[50] = AR2__10x14;
		ascii_characters[51] = AR3__10x14;
		ascii_characters[52] = AR4__10x14;
		ascii_characters[53] = AR5__10x14;
		ascii_characters[54] = AR6__10x14;
		ascii_characters[55] = AR7__10x14;
		ascii_characters[56] = AR8__10x14;
		ascii_characters[57] = AR9__10x14;
		ascii_characters[58] = COLON__10x14;
		ascii_characters[65] = A__10x14;
		ascii_characters[66] = B__10x14;
		ascii_characters[67] = C__10x14;
		ascii_characters[68] = D__10x14;
		ascii_characters[69] = E__10x14;
		ascii_characters[70] = F__10x14;
		ascii_characters[71] = G__10x14;
		ascii_characters[72] = H__10x14;
		ascii_characters[73] = I__10x14;
		ascii_characters[74] = J__10x14;
		ascii_characters[75] = K__10x14;
		ascii_characters[76] = L__10x14;
		ascii_characters[77] = M__10x14;
		ascii_characters[78] = N__10x14;
		ascii_characters[79] = O__10x14;
		ascii_characters[80] = P__10x14;
		ascii_characters[81] = Q__10x14;
		ascii_characters[82] = R__10x14;
		ascii_characters[83] = S__10x14;
		ascii_characters[84] = T__10x14;
		ascii_characters[85] = U__10x14;
		ascii_characters[86] = V__10x14;
		ascii_characters[87] = W__10x14;
		ascii_characters[88] = X__10x14;
		ascii_characters[89] = Y__10x14;
		ascii_characters[90] = Z__10x14;

        // just blank out lowercase except as needed
        ascii_characters[97] = SPACE__10x14;
        ascii_characters[98] = SPACE__10x14;
        ascii_characters[99] = SPACE__10x14;
        ascii_characters[100] = SPACE__10x14;
        ascii_characters[101] = SPACE__10x14;
        ascii_characters[102] = SPACE__10x14;
        ascii_characters[103] = SPACE__10x14;
        ascii_characters[104] = SPACE__10x14;
        ascii_characters[105] = SPACE__10x14;
        ascii_characters[106] = SPACE__10x14;
        ascii_characters[107] = k__10x14;
        ascii_characters[108] = SPACE__10x14;
        ascii_characters[109] = m__10x14;
        ascii_characters[110] = SPACE__10x14;
        ascii_characters[111] = SPACE__10x14;
        ascii_characters[112] = SPACE__10x14;
        ascii_characters[113] = SPACE__10x14;
        ascii_characters[114] = SPACE__10x14;
        ascii_characters[115] = s__10x14;
        ascii_characters[116] = SPACE__10x14;
        ascii_characters[117] = u__10x14;
        ascii_characters[118] = SPACE__10x14;
        ascii_characters[119] = SPACE__10x14;
        ascii_characters[120] = SPACE__10x14;
        ascii_characters[121] = SPACE__10x14;
        ascii_characters[122] = SPACE__10x14;
    }
}
