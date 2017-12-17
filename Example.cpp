// compile: clang dsp-sinus.c -lm -o dsp-sinus
//
// usage:   dsp-sinus [freq] [level]      (default freq. is 1000 Hz and default level is 0.5)
// example: dsp-sinus 5000 0.75

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <fcntl.h>
#include <unistd.h>
#include <x86_64-linux-gnu/sys/ioctl.h>
#include <x86_64-linux-gnu/sys/soundcard.h>


#define pi2 6.2831853071795865

int main(int argc, const char *argv[])
{
   int dsp = open("/dev/dsp0", O_RDWR);
   if (dsp != -1)
   {
      unsigned param;

      param = AFMT_S32_LE;
      if (ioctl(dsp, SNDCTL_DSP_SETFMT, &param) == -1 || param != AFMT_S32_LE)
      {
         close(dsp);
         printf("Error setting the input format (0x%X)\n", param);
         return -1;
      }

      param = 2;
      if (ioctl(dsp, SNDCTL_DSP_CHANNELS, &param) == -1 || param != 2)
      {
         close(dsp);
         printf("Error setting stereo output %d\n", param);
         return -2;
      }

      // force a sampling rate of 48 kHz
      param = 48000;
      if (ioctl(dsp, SNDCTL_DSP_SPEED, &param) != -1)
         printf("Sampling rate set to: %d Hz\n", param);
      else
      {
         close(dsp);
         printf("Error setting sampling rate %d\n", param);
         return -3;
      }

      // 24bit stereo sinus generation; default frequency = 1000 Hz, default level = 0.5 (half scale)
      int    i, L, R;
      double dt = 1.0/param;
      double f  = (argc > 1) ? strtod(argv[1], NULL) : 1000.0;
      double l  = (argc > 2) ? strtod(argv[2], NULL) : 0.5;

      if (f < 0.0 || 20000.0 < f)
         f = 1000.0;

      if (l < 0.0 || 1.0 < l)
         l = 0.5;

      for (i = 0; i < 30*param; i++)
      {
         L = R = lround(l*0x7FFFFF*sin(pi2*f * i*dt)) << 8;
         write(dsp, &L, 4);
         write(dsp, &R, 4);
      }

      close(dsp);
   }

   return 0;
}