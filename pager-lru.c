#include <stdio.h> 
#include <stdlib.h>
#include "simulator.h"

void pageit(Pentry q[MAXPROCESSES]) { 

    /* Static vars */
    static int initialized = 0;
    static int tick = 1; // artificial time
    static int timestamps[MAXPROCESSES][MAXPROCPAGES];

    /* Local vars */
    int proc;
    int page;

    /* initialize timestamp array on first run */
    if (!initialized) {
        for (proc=0; proc < MAXPROCESSES; proc++) {
            for (page=0; page < MAXPROCPAGES; page++) {
                timestamps[proc][page] = 0; 
            }
        }
        initialized = 1;
    }
    
    for (proc = 0; proc < MAXPROCESSES; proc++) {
      Pentry process_entry = q[proc];
      if (process_entry.active != 1) continue;
      // printf("tick: %d, proc: %d\n", tick, proc);

      int pc = process_entry.pc;
      int page = pc / PAGESIZE;

      if (process_entry.pages[page]) {
        timestamps[proc][page] = tick;
        continue;
      }

	    if (pagein(proc, page)) {
	       timestamps[proc][page] = tick;
	       continue;
	     }

      int victim = -1;

      for (int candidate = 0; candidate < MAXPROCPAGES; candidate++) {
        if (candidate == page) continue;
        if (q[proc].pages[candidate] != 1) continue;

        if (victim < 0 || timestamps[proc][candidate] < timestamps[proc][victim]) {
          victim = candidate;
        }
      }

      if (pageout(proc,victim) == 1) {
        if (pagein(proc, page)) {
          timestamps[proc][page] = tick;
          continue;
        }
      }
    }

    /* advance time for next pageit iteration */
    tick++;
} 
