#include "monitor/monitor.h"
#include "monitor/expr.h"
#include "monitor/watchpoint.h"
#include "nemu.h"

#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>

void cpu_exec(uint64_t);

/* We use the `readline' library to provide more flexibility to read from stdin. */
char* rl_gets() {
  static char *line_read = NULL;

  if (line_read) {
    free(line_read);
    line_read = NULL;
  }

  line_read = readline("(nemu) ");

  if (line_read && *line_read) {
    add_history(line_read);
  }

  return line_read;
}

static int cmd_c(char *args) {
  cpu_exec(-1);
  return 0;
}

static int cmd_q(char *args) {
  return -1;
}

static int cmd_help(char *args);

static int cmd_si(char *args);

static int cmd_info(char *args);

static int cmd_x(char *args);

static struct {
  char *name;
  char *description;
  int (*handler) (char *);
} cmd_table [] = {
  { "help", "Display informations about all supported commands", cmd_help },
  { "c", "Continue the execution of the program", cmd_c },
  { "q", "Exit NEMU", cmd_q },

  /* TODO: Add more commands */
  { "si", "Execute N(default = 1) steps of the program", cmd_si },
  { "info", "Display the status of GPRs(r) / Display informations about the watchpoints(w)", cmd_info},
  { "x", "Display 4*N bytes addrs in hexadecimal from EXPR", cmd_x}
};

#define NR_CMD (sizeof(cmd_table) / sizeof(cmd_table[0]))

static int cmd_help(char *args) {
  /* extract the first argument */
  char *arg = strtok(NULL, " ");
  int i;

  if (arg == NULL) {
    /* no argument given */
    for (i = 0; i < NR_CMD; i ++) {
      printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
    }
  }
  else {
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(arg, cmd_table[i].name) == 0) {
        printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
        return 0;
      }
    }
    printf("Unknown command '%s'\n", arg);
  }
  return 0;
}

static int cmd_si(char *args){
    // TODO: 利用 strtok 读取出 N
    char *str_num = strtok(NULL, " ");
    // TODO: 然后根据 N 来执行对应的 cpu_exec(N) 操作
    long long n = 0;
    int i = 0, neg_flag = 1;;
    if (str_num == NULL) {
      cpu_exec(1);
    }
    else {
      if (str_num[0] == '-') {
        i++;
        neg_flag = -1;
      }
      for (; i < strlen(str_num); i++) {
        n *= 10;
        char ch = str_num[i];
        if (ch >= '0' && ch <= '9') {
          n += ch - '0';
        }
        else {
          printf("Unknown command '%s'\n", str_num);
          return 0;
        }
      }
      cpu_exec(n * neg_flag);
    }
    return 0;
}

static int cmd_info(char *args){
    // 分割字符
    char *sub_cmd = strtok(NULL, " ");
    // 判断子命令是否是r
    if (strcmp(sub_cmd, "r") == 0) {
        // 依次打印所有寄存器
        // 这里给个例子：打印出 eax 寄存器的值
        for (int i = 0; i < 8; i++){
          printf("%s:\t%8x\t\n", regsl[i], reg_l(i));
          printf("%s:\t%8x\t\n", regsw[i], reg_w(i));
          printf("%s:\t%8x\t\n", regsb[i], reg_b(i));
        }
    }
    else if (strcmp(sub_cmd, "w") == 0)
    {
        // 这里我们会在 PA1.3 中实现
    }
    else {
      printf("Unknown command '%s'\n", sub_cmd);
    }
    return 0;
}

static int cmd_x(char *args){
	  return 0;	
}

void ui_mainloop(int is_batch_mode) {
  if (is_batch_mode) {
    cmd_c(NULL);
    return;
  }

  while (1) {
    char *str = rl_gets();
    char *str_end = str + strlen(str);

    /* extract the first token as the command */
    char *cmd = strtok(str, " ");
    if (cmd == NULL) { continue; }

    /* treat the remaining string as the arguments,
     * which may need further parsing
     */
    char *args = cmd + strlen(cmd) + 1;
    if (args >= str_end) {
      args = NULL;
    }

#ifdef HAS_IOE
    extern void sdl_clear_event_queue(void);
    sdl_clear_event_queue();
#endif

    int i;
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(cmd, cmd_table[i].name) == 0) {
        if (cmd_table[i].handler(args) < 0) { return; }
        break;
      }
    }

    if (i == NR_CMD) { printf("Unknown command '%s'\n", cmd); }
  }
}
