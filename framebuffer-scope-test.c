#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <linux/fb.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <math.h>

// application entry point
int main(int argc, char* argv[])
{
  int fbfd = 0;
  struct fb_var_screeninfo orig_vinfo;
  struct fb_var_screeninfo vinfo;
  struct fb_fix_screeninfo finfo;
  long int screensize = 0;
  char *fbp = 0;
  double start,end;

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
    // create example data, 512 1-byte samples
    u_int8_t *data = malloc(512*sizeof(u_int8_t));
    int i,j;
    // create some sample data
    for (i = 0; i < 512; i++) data[i] = (u_int8_t) 128*(sin((double)i/100)+1);
    // clear framebuffer
    for (i = 0; i < vinfo.yres*vinfo.xres; i++) fbp[i] = 0;

    // create fixed background grid
    // we will use 512x256px for the actual trace, and a resolution of 640x360px
    // (8 bit ADC results in 256 levels so there's no point quantising to something else and losing information)
    // lets split into 8x4 squares because why not. So lines at 64px and why not tick at 8px
    // leave 52px left/right and 64px up/down
    // i don't care about efficiency here because it will only be executed once.
    int x,y;
    for (x = 52; x < 564; x++) {
        for (y = 64; y < 320; y++) {
            if ((x-52)%64 == 0 || y%64) fbp[x + y * finfo.line_length] = 2;
            else {
                int xrel = (x-52)%64;
                int yrel = y%64;
                if ((x-52)%8 == 0 && (yrel < 4 || yrel > 60)) fbp[x + y * finfo.line_length] = 2;
                if (y%8 == 0 && (xrel < 4 || xrel > 60)) fbp[x + y * finfo.line_length] = 2;
            }
        }
    }

    // start timer
    struct timeval tv;
    gettimeofday(&tv, 0);
    start = (double)tv.tv_sec + ((double)tv.tv_usec / 1E6);
    for (j = 0; j < 100; j++) {
        // set sample data
        for (i = 52; i < 564; i++) {
            fbp[64 + i + data[i] * finfo.line_length] = 1;
        }
        // reset sample data
        for (i = 52; i < 564; i++) {
            fbp[64 + i + data[i] * finfo.line_length] = 3;
        }
    }
    gettimeofday(&tv, 0);
	end = (double)tv.tv_sec + ((double)tv.tv_usec / 1E6);
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
