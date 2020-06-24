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
    // start timer
    double start,end;
    gettimeofday(&tv, 0);
	start = (double)tv.tv_sec + ((double)tv.tv_usec / 1E6);
    for (j = 0; j < 10000; j++) {
        // clear framebuffer
        for (i = 0; i < vinfo.yres*vinfo.xres; i++) fbp[pix_offset] = 0
        // then set sample data
        for (i = 0; i < 512; i++) {
            fbp[x + data[x] * finfo.line_length] = 1;
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
  printf("%.1fHz\n",(double)10000/(end-start));
  return 0;
  
}
