/***************************************************************************************
* Copyright (c) 2014-2022 Zihao Yu, Nanjing University
*
* NEMU is licensed under Mulan PSL v2.
* You can use this software according to the terms and conditions of the Mulan PSL v2.
* You may obtain a copy of Mulan PSL v2 at:
*          http://license.coscl.org.cn/MulanPSL2
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*
* See the Mulan PSL v2 for more details.
***************************************************************************************/

#include <isa.h>

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <regex.h>

bool check_number(char *arg);
word_t isa_reg_str2val(const char *s, bool *success);

enum {
  TK_NOTYPE = 256, TK_EQ = 257,
  TK_DNUM = 258, TK_HNUM = 259, 
  TK_NEQ = 260, TK_AND = 261, 
  TK_REG = 262, TK_PDF = 263, 

  /* TODO: Add more token types */

};

static struct rule {
  const char *regex;
  int token_type;
} rules[] = {

  /* TODO: Add more rules.
   * Pay attention to the precedence level of different rules.
   */

  {" +", TK_NOTYPE},     // spaces
  {"\\+", '+'},          // plus
  {"\\-", '-'},          // subtract
  {"\\*", '*'},          // multiply
  {"\\/", '/'},          // divide
  {"\\(", '('},          // left bracket
  {"\\)", ')'},          // right bracket
  {"\\$+[a-z]*[0-9]*", TK_REG}, // get reg value
  {"==", TK_EQ},         // equal
  {"!=", TK_NEQ},        // not equal
  {"&&", TK_AND},        // and
  {"0x[0-9]+", TK_HNUM}, // hexadecimal numbers
  {"[0-9]+", TK_DNUM},   // decimal numbers
  
};

#define NR_REGEX ARRLEN(rules)

static regex_t re[NR_REGEX] = {};
static word_t pty[512] = {};

static void init_pty()
{

  pty['*'] = pty['/'] = 4; 
  pty['+'] = pty['-'] = 5;
  pty[TK_EQ] = pty[TK_NEQ] = 8;
  pty[TK_AND] = 12;
}

/* Rules are used for many times.
 * Therefore we compile them only once before any usage.
 */
void init_regex() {
  init_pty();
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

static Token tokens[65536] __attribute__((used)) = {};
static int nr_token __attribute__((used))  = 0;

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
        
        switch (rules[i].token_type) {
          case TK_NOTYPE: break;
          case '+':
          case '-':
          case '*':
          case '/':
          case '(':
          case ')':
          case TK_EQ:
          case TK_NEQ:
          case TK_AND:
            tokens[nr_token++] = (Token){rules[i].token_type, ""};
            break;
          case TK_DNUM:
            tokens[nr_token++] = (Token){TK_DNUM, ""};
            strncpy(tokens[nr_token - 1].str, substr_start, substr_len);
            break;
          case TK_HNUM:
            tokens[nr_token++] = (Token){TK_HNUM, ""};
            strncpy(tokens[nr_token - 1].str, substr_start + 2, substr_len - 2);
            break;
          case TK_REG:
            tokens[nr_token++] = (Token){TK_HNUM, ""};
            strncpy(tokens[nr_token - 1].str, substr_start + 1, substr_len - 1);
            break;
          default: break;
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

static bool check_parentheses(int p, int q, bool *legal)
{
  if (tokens[p].type != '(' || tokens[q].type != ')') return false;
  bool flag = 1;
  int lef = 0;
  for (int i = p + 1; i < q; i++)
    if (tokens[i].type == '(') lef++;
    else if (tokens[i].type == ')') {
      if (!lef) flag = 0;
      lef--;
    }
  if (lef == 0) return flag;
  *legal = 0;
  return false;
}

static bool is_calc_bool(word_t type)
{
  return type == '+' || type == '-' || type == '*' || type == '/' || type == TK_AND || type == TK_EQ || type == TK_NEQ;
}

static int find_main_calc(int p, int q)
{
  int pos = 0;
  int lef = 0;
  for (int i = p; i <= q; i++)
    if (tokens[i].type == '(') lef++;
    else if (tokens[i].type == ')') lef--;
    else if (is_calc_bool(tokens[i].type)) {
      if (lef) continue;
      else if (!pos || pty[tokens[i].type] >= pty[tokens[pos].type]) pos = i;
    }
  return pos;
}

static word_t eval(int p, int q, bool *legal) {
  if (!(*legal)) return 1;
  if (p > q) {
    /* Bad expression */
    *legal = 0;
    return 1;
  }
  else if (p == q) {
    /* Single token.
     * For now this token should be a number.
     * Return the value of the number.
     */
    if (tokens[p].type == TK_REG) {
      word_t N = isa_reg_str2val(tokens[p].str, legal);
      if (!*legal) return 1;
      return N;
    }
    else {
      if (!check_number(tokens[p].str)) {
        *legal = 0;
        return 1;
      }
      word_t N = 0;
      if (tokens[p].type == TK_DNUM) sscanf(tokens[p].str, "%u", &N);
      else sscanf(tokens[p].str, "%x", &N);
      return N;
    }
    
  }
  else if (check_parentheses(p, q, legal) == true) {
    /* The expression is surrounded by a matched pair of parentheses.
     * If that is the case, just throw away the parentheses.
     */
    return eval(p + 1, q - 1, legal);
  }
  else {
    int op = find_main_calc(p, q);
    if (!op) {
      *legal = 0;
      return 1;
    }
    word_t val1 = eval(p, op - 1, legal);
    word_t val2 = eval(op + 1, q, legal);

    switch (tokens[op].type) {
      case '+': return val1 + val2;
      case '-': return val1 - val2;
      case '*': return val1 * val2;
      case '/': 
        if (val2 == 0) {
          printf("Divide 0 Error!\n");
          *legal = 0;
          return 1;
        }
        else return val1 / val2;
      case TK_AND: return val1 && val2;
      case TK_EQ: return val1 == val2;
      case TK_NEQ: return val1 != val2;
      default: *legal = 0; return 1;
    }
  }
  return 1;
}

word_t expr(char *e, bool *success) {
  if (!make_token(e)) {
    *success = false;
    return 0;
  }
  //for (int i = 0; i < nr_token; i++)
  //  printf("%s\n", tokens[i].str);
  /* TODO: Insert codes to evaluate the expression. */
  //TODO();

  //for (int i = 0; i < nr_token; i++)
  //  printf("%s\n", tokens[i].str);
  return eval(0, nr_token - 1, success);
}
