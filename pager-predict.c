#include <stdio.h> 
#include <stdlib.h>
#include "simulator.h"

void pageit(Pentry q[MAXPROCESSES]) { 
  /* Static vars */
  static int initialized = 0;
  static int tick = 1; // artificial time
  static int timestamps[MAXPROCESSES][MAXPROCPAGES];


  static int last_pc[MAXPROCESSES];

  static int next_pc[MAXPROCESSES][MAXPROCPAGES * PAGESIZE];

  /* Local vars */
  int proc;
  int page;

  int lookahead = 256;
    
  /* initialize static vars on first run */
  if (!initialized) {
    for (proc = 0; proc < MAXPROCESSES; proc++) {
      for (page = 0; page < MAXPROCPAGES; page++) {
        timestamps[proc][page] = 0; 
        for (int i = 0; i < PAGESIZE; i++) {
          next_pc[proc][page * PAGESIZE + i] = -1;
        }
      }
      last_pc[proc] = -1;
    }

    initialized = 1;
  }

  int used_pages[MAXPROCESSES][MAXPROCESSES];
  int predicted_pages[MAXPROCESSES][MAXPROCESSES];

  for (proc = 0; proc < MAXPROCESSES; proc++) {
    for (page = 0; page < MAXPROCPAGES; page++) {
      used_pages[proc][page] = 0;
      predicted_pages[proc][page] = 0;
    }
  }


  for (proc = 0; proc < MAXPROCESSES; proc++) {
    Pentry process_entry = q[proc];
    if (process_entry.active != 1) continue;

    int pc = process_entry.pc;
    int page = pc / PAGESIZE;

    used_pages[proc][page] = 1;

    if (last_pc[proc] >= 0 && last_pc[proc] != pc) {
     next_pc[proc][last_pc[proc]] = pc;
    }

    last_pc[proc] = pc;

    int predicted_pc = pc;
    for (int i = 0; i < lookahead; i++) {
      predicted_pc = next_pc[proc][predicted_pc];
      if (predicted_pc < 0 || predicted_pc > MAXPROCPAGES * PAGESIZE) break;
      int predicted_page = predicted_pc / PAGESIZE;
      predicted_pages[proc][predicted_page] = 1;
    }

    if (page + 1 < PAGESIZE) {
      predicted_pages[proc][page + 1] = 1;
    }
  }

  for (int candidate_process = 0; candidate_process < MAXPROCESSES; candidate_process++) {
    for (int candidate_page = 0; candidate_page < MAXPROCPAGES; candidate_page++) {
      if (used_pages[candidate_process][candidate_page] == 1) continue;
      if (predicted_pages[candidate_process][candidate_page] == 1) continue;
      pageout(candidate_process, candidate_page);
    }
  }

  // get used pages
  for (proc = 0; proc < MAXPROCESSES; proc++) {
    if (q[proc].active != 1) continue;
    for (page = 0; page < MAXPROCPAGES; page++) {
      if (used_pages[proc][page]) {
        pagein(proc, page);
        break;
      }
    }
  }
  
  // get next pages
  for (proc = 0; proc < MAXPROCESSES; proc++) {
    if (q[proc].active != 1) continue;
    for (page = 0; page < MAXPROCPAGES; page++) {
      if (predicted_pages[proc][page]) {
        pagein(proc, page);
      }
    }
  }

  /* advance time for next pageit iteration */
  tick++;
}
