#ifndef PAGE_SPEC_H 
#define PAGE_SPEC_H

typedef struct page_spec 
{
	unsigned int id;
	const char *filename;
	char *path;
	char *template;
	char *query;
	const char *query_params;
} page_spec;

page_spec *parse_page_spec(const char *payload);
void free_page_spec(page_spec *spec);

#endif

