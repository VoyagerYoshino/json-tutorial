#include "leptjson.h"
#include <string.h>
#include <errno.h>
#include <math.h>
#include <assert.h>  /* assert() */
#include <stdlib.h>  /* NULL, strtod() */

#define EXPECT(c, ch)       do { assert(*c->json == (ch)); c->json++; } while(0)
#define ISDIGIT(ch)         ((ch) >= '0' && (ch) <= '9')
#define ISDIGIT1TO9(ch)     ((ch) >= '1' && (ch) <= '9')

typedef struct {
    const char* json;
}lept_context;

static void lept_parse_whitespace(lept_context* c) {
    const char *p = c->json;
    while (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r')
        p++;
    c->json = p;
}

static int lept_parse_literal(lept_context* c, lept_value* v,const char* s,lept_type t) {
    size_t i;
    EXPECT(c, s[0]);
    for(i=1;i<strlen(s);i++){
        if ( *(c->json+i-1) !=s [i]){
            return LEPT_PARSE_INVALID_VALUE;
        }
    }
    c->json += strlen(s)-1;
    v->type = t;
    return LEPT_PARSE_OK;
}

static int lept_parse_number(lept_context* c, lept_value* v) {
/*  Solution could work but... 

    const char* ptr=c->json;
    char* end;
    double value;
    errno=0;
    if (*c->json == '-'){
        ptr=(c->json+1);
    }
    value = strtod(c->json, &end);
    if (ptr == end || !ISDIGIT(*ptr) || *(end-1)=='.')
        return LEPT_PARSE_INVALID_VALUE;
    if (*ptr == '0' && *(ptr+1) != '.'){
         end=ptr+1;
    }
    if (*(ptr+1) != '.' && !ISDIGIT1TO9(*(ptr+1)) && *(ptr+1) != 'E' && *(ptr+1) != 'e'){
        end=ptr+1;
    }
    if (errno == ERANGE && (value == HUGE_VAL || value== -HUGE_VAL)){
        return LEPT_PARSE_NUMBER_TOO_BIG;
    }
    v->n = value;
    c->json = end;
    v->type = LEPT_NUMBER;
    return LEPT_PARSE_OK;
*/

/*  More resonable solution */
    const char* ptr = c->json;
    /* [ "-" ] */
    if (*ptr == '-'){
        ptr++;
    }
    /* int = "0" / digit1-9 *digit */
    if (*ptr == '0'){
        ptr++;
    }else{
        if (!ISDIGIT1TO9(*ptr)){
            return LEPT_PARSE_INVALID_VALUE;
        }
        for(ptr++;ISDIGIT(*ptr);ptr++);
    }
    /* frac = "." 1*digit */
    if (*ptr == '.'){
        ptr++;
        if (!ISDIGIT(*ptr)){
            return LEPT_PARSE_INVALID_VALUE;
        }
        for(ptr++;ISDIGIT(*ptr);ptr++);
    }
    /* exp = ("e" / "E") ["-" / "+"] 1*digit */
    if (*ptr == 'e' || *ptr =='E'){
        ptr++;
        if (*ptr == '-' || *ptr =='+'){
            ptr++;
        }
        if (!ISDIGIT(*ptr)){
            return LEPT_PARSE_INVALID_VALUE;
        }
        for(ptr++;ISDIGIT(*ptr);ptr++);
    }
    errno = 0;
    v->n = strtod(c->json, NULL);
    if (errno == ERANGE && (v->n == HUGE_VAL || v->n == -HUGE_VAL))
        return LEPT_PARSE_NUMBER_TOO_BIG;
    v->type = LEPT_NUMBER;
    c->json = ptr;
    return LEPT_PARSE_OK;

}

static int lept_parse_value(lept_context* c, lept_value* v) {
    switch (*c->json) {
        case 't':  return lept_parse_literal(c, v,"true",LEPT_TRUE);
        case 'f':  return lept_parse_literal(c, v,"false",LEPT_FALSE);
        case 'n':  return lept_parse_literal(c, v,"null",LEPT_NULL);
        default:   return lept_parse_number(c, v);
        case '\0': return LEPT_PARSE_EXPECT_VALUE;
    }
}

int lept_parse(lept_value* v, const char* json) {
    lept_context c;
    int ret;
    assert(v != NULL);
    c.json = json;
    v->type = LEPT_NULL;
    lept_parse_whitespace(&c);
    if ((ret = lept_parse_value(&c, v)) == LEPT_PARSE_OK) {
        lept_parse_whitespace(&c);
        if (*c.json != '\0') {
            v->type = LEPT_NULL;
            ret = LEPT_PARSE_ROOT_NOT_SINGULAR;
        }
    }
    return ret;
}

lept_type lept_get_type(const lept_value* v) {
    assert(v != NULL);
    return v->type;
}

double lept_get_number(const lept_value* v) {
    assert(v != NULL && v->type == LEPT_NUMBER);
    return v->n;
}
