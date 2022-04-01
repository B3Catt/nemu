#include "nemu.h"

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <sys/types.h>
#include <regex.h>

enum {
  TK_NOTYPE = 256, TK_EQ,

  /* TODO: Add more token types */
	TK_DECN, TK_HEXN

};

static struct rule {
  char *regex;
  int token_type;
} rules[] = {

  /* TODO: Add more rules.
   * Pay attention to the precedence level of different rules.
   */

  {" +", TK_NOTYPE},    // spaces
  {"\\+", '+'},         // plus
  {"==", TK_EQ},         // equal

	{"-", '-'},
  {"\\*", '*'},
  {"/", '/'},
  {"\\(", '('},
  {"\\)", ')'},
  {"\\d+", TK_DECN},
  {"0x[\\da-fA-F]+", TK_HEXN}
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

bool check_parentheses(int p, int q) {
  int k = p, count = 0;
  for (; k <= q; k++) {
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
    assert(0);
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
    assert(0);
  }
  else return false;
}

int find_dominated_op(int p, int q) {
  int op = 0, op_type = TK_NOTYPE, count = 0;
  for (int i = p; i <= q; i ++) {
    switch (tokens[i].type) {
      case '(':
        count ++; break;
      case ')':
        count --; break;
      case '+':
      case '-':
        if (!count) {
          op = i;
          op_type = tokens[i].type;
        }
        break;
      case '*':
      case '/':
        if (!count) {
          if (op_type != '+' && op_type != '-') {
            op = i;
            op_type = tokens[i].type;
          }
        }
        break;
      default: break;
    }
  }
  return op;
}

uint32_t eval(int p, int q) {
  if (p > q) {
    /* Bad expression */
		return 0;
  }
  else if (p == q) {
    /* Single token.
     * For now this token should be a number.
     * Return the value of the number.
    */
    int num;
    if (tokens[p].type == TK_DECN) {
      sscanf(tokens[p].str, "%d", &num);
    }
    else if (tokens[p].type == TK_HEXN) {
      sscanf(tokens[p].str, "%x", &num);
    }
    else {
      assert(0);
    }
    return num;
  }
  else if (check_parentheses(p, q) == true) {
    /* The expression is surrounded by a matched pair of parentheses.
     * If that is the case, just throw away the parentheses.
    */
    return eval(p + 1, q - 1);
  }
  else {
    /* We should do more things here. */
    int op = find_dominated_op(p, q),
      val1 = eval(p, op - 1),
      val2 = eval(op + 1, q);
    switch (tokens[op].type) {
      case '+': return val1 + val2;
      case '-': return val1 - val2;
      case '*': return val1 * val2;
      case '/': return val1 / val2;
      default: assert(0);
    }
  }
}

uint32_t expr(char *e, bool *success) {
  if (!make_token(e)) {
    *success = false;
    return 0;
  }

  /* TODO: Insert codes to evaluate the expression. */
  return eval(0, nr_token - 1);

  return 0;
}
