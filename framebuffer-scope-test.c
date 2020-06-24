// Uses code from Raspberry Compote http://raspberrycompote.blogspot.com/2012/12/low-level-graphics-on-raspberry-pi-part_9509.html
// and glyphs from JSBallista https://github.com/JSBattista/Characters_To_Linux_Buffer_THE_HARD_WAY

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <linux/fb.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <sys/time.h>
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
void setup_chars();
void display_ascii(char *fbp, char c, int x, int y);
void draw_background(char *fbp, double t);
void draw_rising_trigger(char *fbp, u_int8_t *data, int data_len, u_int8_t trigger_low, u_int8_t trigger_high);

// application entry point
int main(int argc, char* argv[])
{
  srand(time(NULL));
  int fbfd = 0;
  long int screensize = 0;
  char *fbp = 0;
  double start,end;
  setup_chars();

  // Open the file for reading and writing
  fbfd = open("/dev/fb0", O_RDWR);
  if (!fbfd) {
    printf("Error: cannot open framebuffer device.\n");
    return(1);
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
  }
  else {
    // drawing time...
    // create example data, 131072 1-byte samples
    u_int8_t *data = malloc(131072*sizeof(u_int8_t));
    int i,j;
    // create some sample data with noise
    for (i = 0; i < 131072; i++) data[i] = (u_int8_t) 128*(sin((double)i/100)+1 + ((double)rand()/RAND_MAX*0.2-0.1));
    // clear framebuffer
    for (i = 0; i < vinfo.yres*vinfo.xres; i++) fbp[i] = 0;
    // draw background
    draw_background(fbp, 1.234e-9);
    // noise is less than 13 units, so lets set the triggers at 114 and 142
    draw_rising_trigger(fbp,data,131072,114,142);
    sleep(5);
  }

  // cleanup
  munmap(fbp, screensize);
  if (ioctl(fbfd, FBIOPUT_VSCREENINFO, &orig_vinfo)) {
    printf("Error re-setting variable information.\n");
  }
  close(fbfd);
  
  // output refresh rate
  printf("%.1fHz\n",(double)100/(end-start));
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
    bool low = false; // data is above low trigger
    int low_marker = 0; // the index where signal rose above low trigger
    int trigger_marker; // index to trigger on
    int t_scaling = 1; // we will want later to skip/amalgamate some of the datapoints to show higher frequencies
    int startval,endval;
    //int last_trigger = 0;
    //double t_avg = 0; // average period so far (in units of timestep)
    //int rise_counter = 0
    for (i = 0; i < data_len; i++) {
        if (data[i] > trigger_high) {
            if (!low) low_marker = i;
            low = false;
            trigger_marker = (int)(i + low_marker)/2; // close enough
            //t_avg = (t_avg*(rise_counter++) + trigger_marker)/rise_counter;
            // for now, lets just redraw on every rising edge, but
            // in future it's not worth it when several waveforms may be on screen
            if (trigger_marker < 256) startval = 0;
            else startval = trigger_marker - 256;
            if (trigger_marker + 256 > data_len) endval = data_len-1;
            else endval = trigger_marker + 256;
            for (j = startval; j < endval; j++) {
                fbp[308 + j - trigger_marker + (64+data[i]) * finfo.line_length] = 1; // 308 = 52 + 256, 52 is zero point
            }
            sleep(1); // wait to make sure it actually shows up
            // cleanup
            for (j = startval; j < endval; j++) {
                fbp[308 + j - trigger_marker + (64+data[i]) * finfo.line_length] = 0; // 308 = 52 + 256, 52 is zero point
            }
        }
        else if (data[i] > trigger_low) {
            low = true;
            low_marker = i;
        }
    }
     
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
