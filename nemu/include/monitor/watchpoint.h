#ifndef __WATCHPOINT_H__
#define __WATCHPOINT_H__

#include "common.h"

typedef struct watchpoint {
  int NO;
  struct watchpoint *next;

  /* TODO: Add more members if necessary */
  char expr[64];
  uint32_t new_val, old_val;
  uint32_t address;
  char type;
  uint8_t old_op;
	bool is_hitted;
} WP;

int set_watchpoint(char*);

bool delete_watchpoint(int);

void list_watchpoint();

WP* scan_watchpoint();

void set_breakpoint(uint32_t);

WP* get_breakpoint(uint32_t);

void create_breakpoint();

#endif
