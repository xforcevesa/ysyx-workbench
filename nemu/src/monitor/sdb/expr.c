/***************************************************************************************
 * Copyright (c) 2014-2024 Zihao Yu, Nanjing University
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

#include "memory/vaddr.h"

// #define Log(...) (printf(__VA_ARGS__)

enum
{
  TK_NOTYPE = 256,
  TK_EQ,

  /* TODO: Add more token types */
  TK_ADD,
  TK_SUB,
  TK_MUL,
  TK_DIV,
  TK_MOD,
  TK_GT,
  TK_LT,
  TK_GE,
  TK_LE,

  TK_NUM,
  TK_IDENT,
  TK_LPAREN,
  TK_RPAREN,
};

static struct rule
{
  const char *regex;
  int token_type;
} rules[] = {

    /* TODO: Add more rules.
     * Pay attention to the precedence level of different rules.
     */

    {" +", TK_NOTYPE},                      // spaces
    {"\\+", TK_ADD},                        // plus
    {"-", TK_SUB},                          // minus
    {"==", TK_EQ},                          // equal
    {"\\*", TK_MUL},                        // asterisk
    {"/", TK_DIV},                          // slash
    {"%", TK_MOD},                          // percent
    {">", TK_GT},                           // greater than
    {"<", TK_LT},                           // less than
    {">=", TK_GE},                          // greater than or equal to
    {"<=", TK_LE},                          // less than or equal to
    {"0?x?[0-9a-fA-F]+", TK_NUM},                 // number
    {"[a-zA-Z_$][a-zA-Z0-9_$]*", TK_IDENT}, // identifier
    {"\\(", TK_LPAREN},                     // left parenthesis
    {"\\)", TK_RPAREN},                     // right parenthesis
};

#define NR_REGEX ARRLEN(rules)

static regex_t re[NR_REGEX] = {};

/* Rules are used for many times.
 * Therefore we compile them only once before any usage.
 */
void init_regex()
{
  int i;
  char error_msg[128];
  int ret;

  for (i = 0; i < NR_REGEX; i++)
  {
    ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);
    if (ret != 0)
    {
      regerror(ret, &re[i], error_msg, 128);
      panic("regex compilation failed: %s\n%s", error_msg, rules[i].regex);
    }
  }
}

typedef struct token
{
  int type;
  char str[32];
} Token;

#define MAX_TOKENS 1024

static Token tokens[MAX_TOKENS] __attribute__((used)) = {};
static int nr_token __attribute__((used)) = 0;

static bool make_token(char *e)
{
  int position = 0;
  int i;
  regmatch_t pmatch;

  nr_token = 0;

  while (e[position] != '\0')
  {
    /* Try all rules one by one. */
    for (i = 0; i < NR_REGEX; i++)
    {
      if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0)
      {
        char *substr_start = e + position;
        int substr_len = pmatch.rm_eo;

        Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s",
            i, rules[i].regex, position, substr_len, substr_len, substr_start);

        position += substr_len;

        /* TODO: Now a new token is recognized with rules[i]. Add codes
         * to record the token in the array `tokens'. For certain types
         * of tokens, some extra actions should be performed.
         */

        if (nr_token >= MAX_TOKENS)
        {
          printf("too many tokens\n");
          return false;
        }

        tokens[nr_token].type = rules[i].token_type;
        strncpy(tokens[nr_token].str, substr_start, substr_len);
        tokens[nr_token].str[substr_len] = '\0';
        nr_token++;

        switch (rules[i].token_type)
        {
        default:
          break;
        }

        break;
      }
    }

    if (i == NR_REGEX)
    {
      printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
      return false;
    }
  }

  return true;
}

static void get_token_str(int i, char *str)
{
  if (i >= nr_token)
  {
    Log("token index out of range\n");
    return;
  }

  switch (tokens[i].type)
  {
  case TK_NUM:
    strcpy(str, "TK_NUM");
    break;
  case TK_IDENT:
    strcpy(str, "TK_IDENT");
    break;
  case TK_ADD:
    strcpy(str, "TK_ADD");
    break;
  case TK_SUB:
    strcpy(str, "TK_SUB");
    break;
  case TK_MUL:
    strcpy(str, "TK_MUL");
    break;
  case TK_DIV:
    strcpy(str, "TK_DIV");
    break;
  case TK_MOD:
    strcpy(str, "TK_MOD");
    break;
  case TK_GT:
    strcpy(str, "TK_GT");
    break;
  case TK_LT:
    strcpy(str, "TK_LT");
    break;
  case TK_GE:
    strcpy(str, "TK_GE");
    break;
  case TK_LE:
    strcpy(str, "TK_LE");
    break;
  case TK_EQ:
    strcpy(str, "TK_EQ");
    break;
  case TK_NOTYPE:
    strcpy(str, "TK_NOTYPE");
    break;
  case TK_LPAREN:
    strcpy(str, "TK_LPAREN");
    break;
  case TK_RPAREN:
    strcpy(str, "TK_RPAREN");
    break;
  default:
    break;
  }
}

static int parse_index = 0;

static int error = false;

static word_t eval(int level)
{

  if (error)
  {
    return -1;
  }

  if (parse_index > nr_token)
  {
    Log("parse index out of range");
    return 0;
  }

  enum
  {
    NUM_HEX,
    NUM_OCT,
    NUM_DEC
  } number_mode;

  int type = tokens[parse_index].type;
  char *str = tokens[parse_index].str;

  if (type == TK_NOTYPE)
  {
    parse_index++;
    if (parse_index > nr_token)
    {
      Log("parse index out of range");
      return 0;
    }
    type = tokens[parse_index].type;
    str = tokens[parse_index].str;
  }

  Log("Here: level = %d, parse_index = %d, nr_token = %d, type = %d, str = %s", level, parse_index, nr_token, type, str);

  word_t lval = 0;

  bool is_success = true;

  parse_index++;

  if (parse_index > nr_token)
  {
    Log("parse index out of range");
    return 0;
  }

  switch (type)
  {
  case TK_NUM:
    number_mode = *str == '0' ? ((*(str + 1) == 'x' || *(str + 1) == 'X') ? NUM_HEX : NUM_OCT) : NUM_DEC;
    switch (number_mode)
    {
    case NUM_HEX:
      for (char *p = str + 2; *p != '\0'; p++)
      {
        if (*p >= '0' && *p <= '9')
        {
          lval = (lval << 4) + (*p - '0');
        }
        else if (*p >= 'a' && *p <= 'f')
        {
          lval = (lval << 4) + (*p - 'a' + 10);
        }
        else if (*p >= 'A' && *p <= 'F')
        {
          lval = (lval << 4) + (*p - 'A' + 10);
        }
        else
        {
          error = true;
        }
      }
      break;
    case NUM_OCT:
      for (char *p = str + 1; *p != '\0'; p++)
      {
        if (*p >= '0' && *p <= '7')
        {
          lval = (lval << 3) + (*p - '0');
        }
        else
        {
          error = true;
        }
      }
      break;
    case NUM_DEC:
      for (char *p = str; *p != '\0'; p++)
      {
        if (*p >= '0' && *p <= '9')
        {
          lval = lval * 10 + (*p - '0');
        }
        else
        {
          Log("invalid character in number: %c", *p);
          error = true;
        }
      }
      break;
    default:
      break;
    }
    break;
  case TK_IDENT:
    //  printf("identifier: %s\n", str);
    if (*str == '$')
    {
      is_success = true;
      lval = isa_reg_str2val(str + 1, &is_success);
      if (!is_success)
      {
        error = true;
      }
    }
    else
    {
      error = true;
    }
    break;
  case TK_LPAREN:
    lval = eval(TK_ADD);
    if (tokens[parse_index].type != TK_RPAREN)
    {
      Log("missing right parenthesis");
      error = true;
    }
    parse_index++;
    break;
  case TK_MUL:
    lval = eval(TK_NUM);
    lval = vaddr_read(lval, sizeof(word_t));
    parse_index++;
    break;
  default:
    break;
  }
  if (error)
  {
    Log("parse error internal 1, lval = %d", lval);
    return -1;
  }

  type = tokens[parse_index].type;
  str = tokens[parse_index].str;

  if (type == TK_NOTYPE && (parse_index + 1) < nr_token)
  {
    parse_index++;
    type = tokens[parse_index].type;
    str = tokens[parse_index].str;
  }

  word_t rval = 0;

  // parse_index++;
  if (parse_index > nr_token)
  {
    Log("parse index out of range");
    return lval;
  }

  while (type >= level && parse_index <= nr_token)
  {
    char str2[44];
    get_token_str(parse_index, str2);
    Log("In while: level = %d, parse_index = %d, type = %s, str = %s", level, parse_index, str2, str);
    parse_index++;
    if (type == TK_RPAREN) {
      parse_index--;
      break;
    }
    switch (type)
    {
    case TK_ADD:
      rval = eval(TK_MUL);
      Log("lval = %d, add rval = %d\n", lval, rval);
      lval += rval;
      break;
    case TK_SUB:
      rval = eval(TK_MUL);
      lval -= rval;
      break;
    case TK_MUL:
      rval = eval(TK_GT);
      lval *= rval;
      break;
    case TK_DIV:
      Log("lval = %d, div rval = %d\n", lval, rval);
      rval = eval(TK_GT);
      lval /= rval;
      break;
    case TK_MOD:
      rval = eval(TK_GT);
      lval %= rval;
      break;
    case TK_GT:
      rval = eval(TK_NUM);
      lval = (lval > rval) ? 1 : 0;
      break;
    case TK_LT:
      rval = eval(TK_NUM);
      lval = (lval < rval) ? 1 : 0;
      break;
    case TK_GE:
      rval = eval(TK_NUM);
      lval = (lval >= rval) ? 1 : 0;
      break;
    case TK_LE:
      rval = eval(TK_NUM);
      lval = (lval <= rval) ? 1 : 0;
      break;
    case TK_EQ:
      rval = eval(TK_NUM);
      lval = (lval == rval) ? 1 : 0;
      break;
    default:
      break;
    }
    type = tokens[parse_index].type;
    str = tokens[parse_index].str;

    while (type == TK_NOTYPE && (parse_index + 1) <= nr_token)
    {
      parse_index++;
      type = tokens[parse_index].type;
      str = tokens[parse_index].str;
    }
  }

  return lval;
}

word_t expr(char *e, bool *success)
{
  parse_index = 0;
  error = false;
  if (!make_token(e))
  {
    *success = false;
    return 0;
  }

  char str[44];

  for (int i = 0; i < nr_token; i++)
  {
    if (tokens[i].type == TK_NOTYPE)
    {
      continue;
    }
    get_token_str(i, str);
    Log("token %d: type = %s, str = %s", i, str, tokens[i].str);
  }

  /* TODO: Insert codes to evaluate the expression. */
  // TODO();
  int res = eval(TK_ADD);
  if (!error)
  {
    Log("parsed value: %d", res);
    *success = true;
  }
  else
  {
    Log("parse error");
    *success = false;
  }

  return res;
}
