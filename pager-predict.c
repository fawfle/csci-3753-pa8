#include <stdio.h> 
#include <stdlib.h>
#include <time.h>
#include "simulator.h"

#define MAX_TIME 2147483600

void pageit(Pentry q[MAXPROCESSES]) { 
  /* Static vars */
  static int initialized = 0;
  static int tick = 1; // artificial time
  static int timestamps[MAXPROCESSES][MAXPROCPAGES];


  static int last_pc[MAXPROCESSES];

  static int next_pc[MAXPROCESSES][MAXPROCPAGES * PAGESIZE];
  static int next_pc_alternate[MAXPROCESSES][MAXPROCPAGES * PAGESIZE];

  static int zero_jumps[MAXPROCESSES][MAXPROCPAGES * PAGESIZE];

  /* Local vars */
  int proc;
  int page;

  int lookahead = 200;
  int lookahead_alternative = 200;
    
  /* initialize static vars on first run */
  if (!initialized) {
    for (proc = 0; proc < MAXPROCESSES; proc++) {
      for (page = 0; page < MAXPROCPAGES; page++) {
          timestamps[proc][page] = MAX_TIME; 
        for (int i = 0; i < PAGESIZE; i++) {
          next_pc[proc][page * PAGESIZE + i] = page * PAGESIZE + i + 1;
          next_pc_alternate[proc][page * PAGESIZE + i] = page * PAGESIZE + i + 1;
          zero_jumps[proc][page * PAGESIZE + i] = 0;
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

    if (process_entry.pages[page] == 1) {
      timestamps[proc][page] = tick;
    }

    used_pages[proc][page] = 1;

    if (last_pc[proc] >= 0 && last_pc[proc] != pc) {
      int last_next_pc = next_pc[proc][last_pc[proc]];

      next_pc[proc][last_pc[proc]] = pc;

      if (last_next_pc != pc) next_pc_alternate[proc][last_pc[proc]] = last_next_pc;
    }

    if (last_pc[proc] > 0 && pc < PAGESIZE) zero_jumps[proc][last_pc[proc]] = 1;

    last_pc[proc] = pc;


    int predicted_pc = pc;
    for (int i = 0; i < lookahead; i++) {
      predicted_pc = next_pc[proc][predicted_pc];
      if (predicted_pc < 0 || predicted_pc > MAXPROCPAGES * PAGESIZE) break;
      int predicted_page = predicted_pc / PAGESIZE;
      predicted_pages[proc][predicted_page] = 1;
    }

    predicted_pc = pc;
    for (int i = 0; i < lookahead_alternative; i++) {
      predicted_pc = next_pc_alternate[proc][predicted_pc];
      if (predicted_pc < 0 || predicted_pc > MAXPROCPAGES * PAGESIZE) break;
      int predicted_page = predicted_pc / PAGESIZE;
      predicted_pages[proc][predicted_page] = 1;
      if (zero_jumps[proc][predicted_pc]) {
        predicted_pages[proc][0] = 1;
        break;
      }
    }

    predicted_pages[proc][(page + 1) % MAXPROCPAGES] = 1;
  }

  int eviction_count = 100;
  for (int i = 0; i < eviction_count; i++) {
    int victim_process = -1;
    int victim_page = -1;

    for (int candidate_process = 0; candidate_process < MAXPROCESSES; candidate_process++) {
      for (int candidate_page = 0; candidate_page < MAXPROCPAGES; candidate_page++) {
        if (used_pages[candidate_process][candidate_page] == 1) continue;
        if (predicted_pages[candidate_process][candidate_page] == 1) continue;
        if (victim_page < 0 || timestamps[candidate_process][candidate_page] < timestamps[victim_process][victim_page]) {
          victim_process = candidate_process;
          victim_page = candidate_page;
        }
      }
    }

    if (victim_process == -1 || victim_page == -1 || timestamps[victim_process][victim_page] == MAX_TIME) {
      break;
    }

    timestamps[victim_process][victim_page] = MAX_TIME;

    pageout(victim_process, victim_page);
  }

  // get used pages
  for (proc = 0; proc < MAXPROCESSES; proc++) {
    if (q[proc].active != 1) continue;
    for (page = 0; page < MAXPROCPAGES; page++) {
      if (used_pages[proc][page]) {
        pagein(proc, page);
        timestamps[proc][page] = tick;
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
        timestamps[proc][page] = tick;
      }
    }
  }

  /* advance time for next pageit iteration */
  tick++;
}
