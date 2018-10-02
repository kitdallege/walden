#include <kore/kore.h>
#include <kore/http.h>

int		page(struct http_request *);

int
page(struct http_request *req)
{
	kore_log(LOG_DEBUG, "page called");
	char *hello = "Hello World";
	http_response(req, 200, hello, strlen(hello));
	return (KORE_RESULT_OK);
}

