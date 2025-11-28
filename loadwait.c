/*
 * SPDX-License-Identifier: MIT-0
 *
 * Copyright 2022 Jeffrey H. Johnson <johnsonjh.dev@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#define _GNU_SOURCE 1

#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

static inline unsigned long
count_procs(void)
{
  unsigned long count = 0;
  cpu_set_t cs;

  CPU_ZERO(&cs);
  sched_getaffinity(0, sizeof ( cs ), &cs);
  for (unsigned int i = 0; i < 32; i++)
    {
      if (CPU_ISSET(i, &cs))
          count++;
      else
          break;
    }

  return count;
}

int
main(void)
{
  double load[3], avg, target;
  unsigned long usable_procs, count = 0;
  long configured_procs, available_procs = 0;

  configured_procs = sysconf(_SC_NPROCESSORS_CONF);
  if (configured_procs < 1)
      configured_procs = 1;

  available_procs = sysconf(_SC_NPROCESSORS_ONLN);
  if (available_procs < 1)
      available_procs = 1;

  usable_procs = count_procs();
  if (usable_procs < 1)
      usable_procs = 1;

  if (available_procs > 32)
      usable_procs = (unsigned long)available_procs;

  target
    = ( (double)( (double)usable_procs - 0.9999 ) / (double)usable_procs * 50 );

  (void)fprintf(stderr, "* CPU configuration: ");
  (void)fprintf(stderr, " %ld configured, ",  configured_procs);
  (void)fprintf(stderr, " %ld operational, ", available_procs);
  (void)fprintf(stderr, " %lu serviceable\n", usable_procs);

#define POLL_TIME 5L
again:;

  if (getloadavg(load, 3) != -1)
    {
      avg
       = ( ( ( ( 0.0001 + load[0] + load[1] + ( load[2] + load[0] / 2 ) ) / 3 )
                   / (double)usable_procs ) * 100 );

      (void)fprintf(stderr,
        "\r* Load factor:  %.1f  (target %.1f),  load average: "
        "%.2f, %.2f, %.2f             \b\b\b\b\b\b\b\b\b\b\b\b",
        avg, target, load[0], load[1], load[2]);

      if (target < avg)
        {
          sleep(POLL_TIME);
          count = count + POLL_TIME;
          goto again;
        }
    }

  (void)fprintf(stderr,
    "\r\n* Finished!  Reached target load factor after waiting %lu seconds\r\n",
    count);
  return 0;
}
