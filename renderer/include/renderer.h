#ifndef RENDERER_H
#define RENDERER_H

#include <libpq-fe.h>
#include <json-c/json.h> 
#include "flag_flipper.h"
#include "config.h"

/*
static char root_dir[] = "/var/html/c2v";
static char template_dir[] = "templates";
static char web_dir[] = "www";
static char query_dir[] = "queries";
typedef struct SiteConf
{
	char *root_dir;
	char *template_dir;	
	char *web_dir;
	char *query_dir;
} SiteConf;
*/

typedef struct RendererState
{
	Configurator *configurator;
	PGconn *conn;
	bool run;
	FlagFlipperState *flipper;
	pthread_t tid;
} RendererState;
/* TODO: 
 * Pre-allocate app memory at startup.
 * Try to remove as many malloc calls as possible.
 * * various sub system will require different memory management.
 * * * those sub systems will get their memory from the main application block.
 * template render 'memstream & results buffer'
 * json allows for a custom malloc hook.
 * mustache, (open up some type of custom malloc)
 * 
 * In theory other than the memory postgres allocates the rest could
 * be allocated at startup. 
 *
 * If I map 'this block' to a virtual address space thats 'outside of
 * main memory', then i can exploit the known offsets for various
 * 'tricks'. saving state, hot reload, etc, all become easy.
 * You can easily develop a memory map of where your application lives,
 * so debugging becomes way easier. etc...
 *
 * [future: each thread gets its own 'block' of main memory
 */

/* TODO
 * Try to move over to a single export which could be placed in a lib.so
 * and reloaded @ runtime.
 * 
 * remove globals and work to isolate state.
 * 
 * could hide the implementation of RenderState/SiteConf with just a 
 * forward declare here.
 */
typedef struct RendererApi
{
	RendererState *(*create)(void);
	void (*delete)(RendererState *state);
	void (*unload)(RendererState *state);
	void (*reload)(RendererState *state);
	bool (*update)(RendererState *state);
} RendererApi;
extern const RendererApi renderer_api;




// these will become static 'private' functions called from renderer_update();
int handle_pages(RendererState *self, PGresult *res, int spec_id,
		json_object *global_context);
//int handle_page(RendererState *renderer, const char *payload);
int write_page(RendererState *renderer, const char *name,
		const char *path, const char *data);
int write_pjax(RendererState *renderer, const char *name,
		const char *path, const char *data);

// vectorized version of above.
//int page_handler_thread(PGconn *conn, FlagFlipperState *flipper, PGresult **results);
/*
 * main gets notification of 'dirty_pages'
 *  * query to get dirty pages / grouped | sorted
 *  * chunk out to a work queue
 *  ** so queue = [[results], [results], [results]]
 */

#endif

