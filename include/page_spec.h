#ifndef PAGE_SPEC_H 
#define PAGE_SPEC_H

typedef struct PageSpec 
{
	unsigned int id;
	const char *filename;
	char *path;
	char *template;
	char *query;
	const char *query_params;
} PageSpec;

PageSpec *parse_page_spec(const char *payload);
void free_page_spec(PageSpec *spec);

#endif

