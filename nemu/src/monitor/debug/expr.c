#include "nemu.h"

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <sys/types.h>
#include <regex.h>

enum {
  TK_NOTYPE = 256,
  TK_OR,     //逻辑或
  TK_AND,    //逻辑与
  TK_NEQ,    //不等于
  TK_EQ,     //等于
  TK_BE, TK_NB, TK_B, TK_NBE,   //比较运算符
  TK_PLUS, TK_SUB,  //加减
  TK_MUL, TK_DIV,   //乘除
  TK_NOT,         //逻辑非
  TK_NEG,         //负号
  TK_PTR,         //指针

  TK_DECN, TK_HEXN,
  R_EIP,
};

static struct rule {
  char *regex;
  int token_type;
} rules[] = {
  {" +", TK_NOTYPE},            // spaces
  {"0x[0-9a-fA-F]+", TK_HEXN},  //十六进制数
  {"[0-9]+", TK_DECN},          //十进制数
  {"\\$eax", R_EAX},            //寄存器
  {"\\$ecx", R_ECX},
  {"\\$edx", R_EDX},
  {"\\$ebx", R_EBX},
  {"\\$esp", R_ESP},
  {"\\$ebp", R_EBP},
  {"\\$esi", R_ESI},
  {"\\$edi", R_EDI},
  {"\\$eip", R_EIP},
  {"\\(", '('},                 //四则运算
  {"\\)", ')'},
  {"\\+", TK_PLUS},
  {"-", TK_SUB},
  {"\\*", TK_MUL},
  {"/", TK_DIV},
  {"==", TK_EQ},                //逻辑运算
  {"!=", TK_NEQ},
  {"&&", TK_AND},
  {"\\|\\|", TK_OR},
  {"<=", TK_BE},
  {">=", TK_NB},
  {"<", TK_B},
  {">", TK_NBE},
  {"!", TK_NOT}
};

#define NR_REGEX (sizeof(rules) / sizeof(rules[0]) )

static regex_t re[NR_REGEX];

/* Rules are used for many times.
 * Therefore we compile them only once before any usage.
 */
void init_regex() {
  int i;
  char error_msg[128];
  int ret;

  for (i = 0; i < NR_REGEX; i ++) {
    ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);
    if (ret != 0) {
      regerror(ret, &re[i], error_msg, 128);
      panic("regex compilation failed: %s\n%s", error_msg, rules[i].regex);
    }
  }
}

typedef struct token {
  int type;
  char str[32];
} Token;

Token tokens[32];
int nr_token;

static bool make_token(char *e) {
  int position = 0;
  int i;
  regmatch_t pmatch;

  nr_token = 0;

  while (e[position] != '\0') {
    /* Try all rules one by one. */
    for (i = 0; i < NR_REGEX; i ++) {
      if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) {
        char *substr_start = e + position;
        int substr_len = pmatch.rm_eo;

        Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s",
            i, rules[i].regex, position, substr_len, substr_len, substr_start);
        position += substr_len;

        /* TODO: Now a new token is recognized with rules[i]. Add codes
         * to record the token in the array `tokens'. For certain types
         * of tokens, some extra actions should be performed.
         */
				int j;
        switch (rules[i].token_type) {
          case TK_NOTYPE: break;
          case TK_HEXN:
          case TK_DECN:
            for (j = 0; j < 32 && j < substr_len; j ++) {
              tokens[nr_token].str[j] = *(substr_start + j);
            }
            tokens[nr_token].str[j] = '\0';
						tokens[nr_token].type = rules[i].token_type;
            nr_token++;
            break;
         
				 	default:
            tokens[nr_token].type = rules[i].token_type;
            nr_token++;
            break;
				}

        break;
      }
    }

    if (i == NR_REGEX) {
      printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
      return false;
    }
  }

  return true;
}

bool check_parentheses(int p, int q, bool *success) {
  int k = p, count = 0;
  for (; k <= q; k ++) {
    if (tokens[k].type == '(') {
      count++;
    }
    else if (tokens[k].type == ')') {
      count--;
    }

    if (count <= 0) {
      break;
    }
  }
  if (k == q && count == 0) {
    return true;
  }
  else if (count < 0) {
    *success = false;
    return false;
  }

  for (k++; k <= q; k++) {
    if (tokens[k].type == '(') {
      count++;
    }
    else if (tokens[k].type == ')') {
      count--;
    }

    if (count <= 0) {
      break;
    }
  }
  if (count < 0) {
    *success = false;
    return false;
  }
  else return false;
}

int find_dominated_op(int p, int q, bool *success) {
  int op = 0, op_type = TK_NOTYPE, count = 0;
  for (int i = p; i <= q; i ++) {
    switch (tokens[i].type) {
      case '(':
        count ++; break;
      case ')':
        count --; break;
      case TK_OR:
        if (!count) {
          op = i;
          op_type = tokens[i].type;
        }
        break;
      case TK_AND:
        if (!count) {
          if (op_type != TK_OR) {
            op = i;
            op_type = tokens[i].type;
          }
        }
        break;
      case TK_NEQ:
      case TK_EQ:
        if (!count) {
          if (op_type != TK_OR && op_type != TK_AND) {
            op = i;
            op_type = tokens[i].type;
          }
        }
        break;
      case TK_BE:
      case TK_NB:
      case TK_B:
      case TK_NBE:
        if (!count) {
          if (op_type > TK_NEQ || op_type < TK_OR) {
            op = i;
            op_type = tokens[i].type;
          }
        }
        break;
      case TK_PLUS:
      case TK_SUB:
        if (!count) {
          if (op_type > TK_NBE || op_type < TK_OR) {
            op = i;
            op_type = tokens[i].type;
          }
        }
        break;
      case TK_MUL:
      case TK_DIV:
        if (!count) {
          if (op_type > TK_SUB || op_type < TK_OR) {
            op = i;
            op_type = tokens[i].type;
          }
        }
        break;
      case TK_NOT:
      case TK_NEG:
      case TK_PTR:
        if (!count) {
          if (op_type == TK_NOTYPE) {
            op = i;
            op_type = tokens[i].type;
          }
        }
        break;
      default: break;
    }
  }
  if (op_type == TK_NOTYPE) {
    *success = false;
  }
  return op;
}

uint32_t eval(int p, int q, bool *success) {
  if (*success) {
    if (p > q) {
      /* Bad expression */
      *success = false;
      return 0;
    }
    else if (p == q) {
      /* Single token.
        * For now this token should be a number.
        * Return the value of the number.
      */
      uint32_t num = 0;
      if (tokens[p].type == TK_DECN) {
        sscanf(tokens[p].str, "%d", &num);
      }
      else if (tokens[p].type == TK_HEXN) {
        sscanf(tokens[p].str, "%x", &num);
      }
      else if (tokens[p].type >= R_EAX && tokens[p].type <= R_EDI) {
        num = reg_l(tokens[p].type);
      }
      else if (tokens[p].type == R_EIP) {
        num = cpu.eip;
      }
      else {
        *success = false;
      }
      return num;
    }
    else if (check_parentheses(p, q, success) == true) {
      /* The expression is surrounded by a matched pair of parentheses.
        * If that is the case, just throw away the parentheses.
      */
      return eval(p + 1, q - 1, success);
    }
    else {
      int op = find_dominated_op(p, q, success);
      int op_type = tokens[op].type;
			printf("%d\n", op_type);
      //单目运算符
      if (op_type >= TK_NOT || op_type <= TK_PTR) {
        int val = eval(p + 1, q, success);
        switch (op_type) {
          case TK_NEG: return -val;
          case TK_PTR: return vaddr_read(val, 4);
          case TK_NOT: return !val;
          default: break;
        }
      }
      //双目运算符
      else {
        int val1 = eval(p, op - 1, success),
          val2 = eval(op + 1, q, success);
        switch (op_type) {
          case TK_PLUS: return val1 + val2;
          case TK_SUB: return val1 - val2;
          case TK_MUL: return val1 * val2;
          case TK_DIV: return val1 / val2;
          case TK_EQ: return val1 == val2;
          case TK_NEQ: return val1 != val2;
          case TK_AND: return val1 && val2;
          case TK_OR: return val1 || val2;
          case TK_BE: return val1 <= val2;
          case TK_NB: return val1 >= val2;
          case TK_B: return val1 < val2;
          case TK_NBE: return val1 > val2;
          default: success = false; break;
        }
      }
      return 0;
    }
  }
  else {
    return 0;
  }
}

uint32_t expr(char *e, bool *success) {
  if (!make_token(e)) {
    *success = false;
    return 0;
  }
  //判断是否为单目运算符
  for (int i = 0; i < nr_token; i ++) {
    if (tokens[i].type == '*' && (i == 0 || tokens[i - 1].type == '(' || (tokens[i - 1].type >= TK_OR && tokens[i - 1].type <= TK_PTR))) {
        tokens[i].type = TK_PTR;
    }
    else if (tokens[i].type == '-' && (i == 0 || tokens[i - 1].type == '(' || (tokens[i - 1].type >= TK_OR && tokens[i - 1].type <= TK_PTR))) {
        tokens[i].type = TK_NEG;
    }
  }
  return eval(0, nr_token - 1, success);

  return 0;
}
