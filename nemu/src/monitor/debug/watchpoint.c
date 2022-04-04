#include "monitor/watchpoint.h"
#include "monitor/expr.h"

#define NR_WP 32

static WP wp_pool[NR_WP];
static WP *head, *free_;

void init_wp_pool() {
  int i;
  for (i = 0; i < NR_WP; i ++) {
    wp_pool[i].NO = i;
    wp_pool[i].next = &wp_pool[i + 1];
  }
  wp_pool[NR_WP - 1].next = NULL;

  head = NULL;
  free_ = wp_pool;
}

/* TODO: Implement the functionality of watchpoint */
WP* new_wp() {
  if (!free_) {
    assert(0);
  }

  WP *p;
  int NO = 0;
  if (!head) {
    head = free_;
    free_ = free_->next;
    p = head;
  }
  else {
    p = head;
    while (p->next) {
      p = p->next;
      NO++;
    }
    p->next = free_;
    free_ = free_->next;
    p = p->next;
    NO++;
  }
  p->next = NULL;
  p->NO = NO;
  return p;
}

void free_wp(WP *wp) {
  if (!wp) {
    assert(0);
  }
  WP *p, *q = NULL;
  p = head;
  while (p != wp) {
    q = p;
    p = p->next;
  }
  if (q) {
    q->next = p->next;
    q = q->next;
  }
  else {
    head = head->next;
    q = head;
  }
  p->next = free_;
  free_ = p;
  while (q) {
    q->NO --;
    q = q->next;
  }
} 

int set_watchpoint(char *e) {
  bool flag = true;
  uint32_t val = expr(e, &flag);
  if (flag) {
    WP *p = new_wp();
    p->expr = e;
    p->old_val = val;
    printf("Set watchpoint #%d\n", p->NO);
    printf("expr      = %s\n", p->expr);
    printf("old value = %08x\n", p->old_val);
  }
  else {
    printf("Wrong expression \"%s\"\n", e);
  }
  return 0;
}

bool delete_watchpoint(int NO) {
  WP *p = head;
  if (!p) {
    return false;
  }

  while (p) {
    if (p->NO == NO) {
      break;
    }
    p = p->next;
  }
  free_wp(p);
  return true;
}

void list_watchpoint() {
  WP *p = head;
  printf("NO Expr\t\tOld Value\n");
  while (p) {
    printf("%d %s\t\t%08x", p->NO, p->expr, p->old_val);
    p = p->next;
  }
}

WP* scan_watchpoint() {
  WP *p = head, *q = head;
	bool flag = true;
  while (q) {
    q->new_val = expr(q->expr, &flag);
    q = q->next;
  }
  while (p) {
    if (p->old_val != p->new_val) {
      return p;
    }
    p = p->next;
  }
  return NULL;
}

