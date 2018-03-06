#include "postgres.h"
#include "fmgr.h"
#include "utils/builtins.h"
#include "mustach-json-c.h"

PG_MODULE_MAGIC;

extern Datum hello(PG_FUNCTION_ARGS);

PG_FUNCTION_INFO_V1(hello);

Datum
hello(PG_FUNCTION_ARGS)
{
    char greet[] = "Hello, ";
    text *towhom;
    int greetlen;
    int towhomlen;
    text *greeting;

   towhom = PG_GETARG_TEXT_P(0);

    // calc string sizes
    greetlen = strlen(greet);
    towhomlen = VARSIZE(towhom) - VARHDRSZ;

    // allocate memory and set data structure size
    greeting = (text *)palloc( greetlen + towhomlen);
    SET_VARSIZE(greeting, greetlen + towhomlen + VARHDRSZ);
    
    // contruct greeting string.
    strncpy(VARDATA(greeting), greet, greetlen);
    strncpy(VARDATA(greeting) + greetlen, VARDATA(towhom), towhomlen);
    
    PG_RETURN_TEXT_P(greeting);
}

extern Datum render(PG_FUNCTION_ARGS);

PG_FUNCTION_INFO_V1(render);

Datum
render(PG_FUNCTION_ARGS)
{
    int rc;
    text *template_txt;
    text *json_txt;
    text *result_txt;
    const char *template;
    const char *json_str;
    char *result;
    size_t result_len;
    
    template_txt = PG_GETARG_TEXT_P(0);
    json_txt = PG_GETARG_TEXT_P(1);
    
    template = text_to_cstring(template_txt);
    json_str = text_to_cstring(json_txt);

    rc = mustach_json_cstring(template, json_str, &result, &result_len);
   if (!rc) {
        result_txt = cstring_to_text(result);
        PG_RETURN_TEXT_P(result_txt);
    } else {
        PG_RETURN_NULL();
    }
}

