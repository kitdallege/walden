#include "postgres.h"
#include "fmgr.h"
#include "utils/builtins.h"
#include "mustach-json-c.h"

PG_MODULE_MAGIC;

extern Datum render(PG_FUNCTION_ARGS);
extern Datum slugify(PG_FUNCTION_ARGS);

PG_FUNCTION_INFO_V1(render);
PG_FUNCTION_INFO_V1(slugify);

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
/*
 * Convert to ASCII if 'allow_unicode' is False. Convert spaces to hyphens.
 * Remove characters that aren't alphanumerics, underscores, or hyphens.
 * Convert to lowercase. Also strip leading and trailing whitespace.
 */
Datum
slugify(PG_FUNCTION_ARGS)
{
    //utf8_to_ascii: 
    PG_RETURN_NULL();
} 

