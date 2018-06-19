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
/*
typedef struct PageSpecGroup
{
	int len;
	char template[256];
	char query[256];
	// path, filename, query_params 
	// the [spec] could be SOA style, need a packed_str_array type
	// as it'd allow us to store everything in contigious blocks
	// which we burn-through in the inner loop.
	PageSpec specs[1024];
} PageSpecGroup;
*/

PageSpec *parse_page_spec(const char *payload);
//void set_page_spec_from_result(PQresult *res, int idx, PageSpecGroup *);
void free_page_spec(PageSpec *spec);

#endif

