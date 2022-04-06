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
  create_breakpoint();
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

static int cmd_p(char *args);

static int cmd_w(char *args);

static int cmd_b(char *args);

static int cmd_d(char *args);

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
  { "x", "Display 4*N bytes from the addr in hexadecimal", cmd_x},
	{ "p", "Calculate the value of the expression", cmd_p},
  { "w", "Set the watchpoint", cmd_w},
  { "b", "Set the breakpoint", cmd_b},
  { "d", "Delete watchpoint/breakpoint", cmd_d}
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
  long long n = 1;
	if (str_num) {
    sscanf(str_num, "%lld", &n);
	}
	cpu_exec(n);
  return 0;
}

static int cmd_info(char *args){
  // 分割字符
  char *sub_cmd = strtok(NULL, " ");
  // 判断子命令是否是r
  if (strcmp(sub_cmd, "r") == 0) {
      // 依次打印所有寄存器
      // 这里给个例子：打印出 eax 寄存器的值
      for (int i = 0; i < 8; i ++){
        printf("%s:\t%08x\t\n", regsl[i], reg_l(i));
      }
      for (int i = 0; i < 8; i ++){
        printf("%s:\t%04x\t\n", regsw[i], reg_w(i));
      }
      for (int i = 0; i < 4; i ++){
        printf("%s:\t%02x\t%s:\t%02x\t\n", regsb[i], reg_b(i), regsb[i + 4], reg_b((i + 4)));
			}
  }
  else if (strcmp(sub_cmd, "w") == 0) {
	  list_watchpoint();
	}
  else {
    printf("Unknown command '%s'\n", sub_cmd);
  }
  return 0;
}

static int cmd_x(char *args){
  //分割字符串，得到起始位置和要读取的次数
  char *str_num = strtok(NULL, " "),
    *str_expr = strtok(NULL, " ");
  int n = 0;
  vaddr_t addr = 0;

  sscanf(str_num, "%d", &n);
  sscanf(str_expr, "%x", &addr);
 	printf("Address \tDword block\tByte sequence\n");
  //循环使用 vaddr_read 函数来读取内存
  for (int i = 0; i < n; i ++) {
    uint32_t read = vaddr_read(addr + 4 * i, 4);
    uint8_t bytes[4];
    for (int j = 0; j < 4; j ++) {
      bytes[j] = vaddr_read(addr + 4 * i + j, 1);
	}

  //每次循环将读取到的数据用 printf 打印出来
    printf("0x%08x\t0x%08x\t%02x %02x %02x %02x\n", addr + 4 * i, read, bytes[0], bytes[1], bytes[2], bytes[3]);
  }
  return 0;
}

static int cmd_p(char* args) {
  bool flag = true;
  uint32_t val = expr(args, &flag);
  if (flag) {
    printf("%d\n", val);
  }
  else {
    printf("Wrong expression \"%s\"\n", args);
  }
  return 0;
}

static int cmd_w(char* args) {
  return set_watchpoint(args);
}

static int cmd_b(char* args) {
  uint32_t addr;
  sscanf(args, "%x", &addr);
  set_breakpoint(addr);
  return 0;
}

static int cmd_d(char* args) {
  int n;
  sscanf(args, "%d", &n);
  bool flag = delete_watchpoint(n);
  if (flag) {
    printf("Watchpoint %d deleted\n", n);
  }
  else {
    printf("Watchpoint %d not exist", n);
  }
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
