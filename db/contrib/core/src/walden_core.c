#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include "postgres.h"
#include "fmgr.h"
#include "utils/builtins.h"


PG_MODULE_MAGIC;

PG_FUNCTION_INFO_V1(slugify);

extern int is_skip_char(const char *ch);

inline int is_skip_char(const char *ch)
{
	return (*ch == ' ' || *ch == '_' || *ch == '-' || iscntrl(*ch));
}


/*
 * Convert to ASCII if 'allow_unicode' is False. Convert spaces to hyphens.
 * Remove characters that aren't alphanumerics, underscores, or hyphens.
 * Convert to lowercase. Also strip leading and trailing whitespace.
 */
Datum
slugify(PG_FUNCTION_ARGS)
{
	int first = 1;
	int needs_hyphen = 0;
	char *ch;
	char *src;
	char *buffer;
	char *dest;
	text *src_txt;
	text *dest_txt;

	src_txt = PG_GETARG_TEXT_P(0);
	src = text_to_cstring(src_txt); 

	dest = palloc(strlen(src) + 1);
	buffer = dest;
	ch = src;
	while (*ch) {
		if (isalnum(*ch)) {
			if (needs_hyphen) {
				*buffer++ = '-';
				needs_hyphen = 0;
			}
			*buffer++ = tolower(*ch++);
			first = 0;
		} else if (is_skip_char(ch))  {
			while (*ch && is_skip_char(ch)){ch++;}
			needs_hyphen = first ? 0:1;
		} else {
			ch++; 
		}
	}
	*buffer = 0x0;
	dest_txt = cstring_to_text(dest);
	pfree(dest);
	PG_RETURN_TEXT_P(dest_txt);
} 

