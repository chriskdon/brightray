#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <errno.h>
#include <sys/socket.h>
#include <resolv.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdbool.h>
#include <assert.h>

#include <uv.h>
#include <http_parser.h>
#include <sds.h>

#include "server.h"
#include "debug.h"

#define MAXBUF 2048
#define MAX_PATH_LENGTH 256

#define UV_ERR(err, msg) fprintf(stderr, "%s: %s\n", msg, uv_strerror(err))

#define UV_CHECK(err, msg) \
do { \
  if (err != 0) { \
    UV_ERR(err, msg); \
    exit(1); \
  } \
} while(0)

#define MAX_WRITE_HANDLES 1000
#define MAX_HTTP_HEADERS 20

// Globals
static uv_loop_t * uv_loop;
static uv_tcp_t server;
static http_parser_settings parser_settings;
static br_server_t * br_g_server;

typedef struct {
  uv_write_t req;
  uv_stream_t stream;
  http_parser parser;
  char *url;
  char *method;
  int header_lines;
  br_http_header_t headers[MAX_HTTP_HEADERS];
  char *body;
  size_t body_length;
  uv_buf_t resp_buf[2];
} http_request_t;

void br_server_set_port(br_server_t * br, int port) {
  br->port = port;
}

int br_default_handler(const br_request_t * req, br_response_t * res) {
  res->status_code = 404;
  br_response_set_content_string(res, "Not Found");

  return 0;
}

br_server_t * br_server_new() {
  br_server_t * server = malloc(sizeof(br_server_t));

  server->port = 8080;
  server->routes_root = NULL;
  server->routes_last = NULL;
  server->default_handler = br_default_handler;

  return server;
}

void br_server_free(br_server_t * br) {
  for(br_route_node_t * node = br->routes_last; node != NULL; node = node->prev)
  {
    free(node);
  }

  free(br);
}

void alloc_cb(uv_handle_t * handle, size_t suggested_size, uv_buf_t * buf) {
  *buf = uv_buf_init((char *) malloc(suggested_size), (unsigned int)suggested_size);
}

void on_close(uv_handle_t * handle) {
  http_request_t * http_request = (http_request_t *) handle->data;

  if (http_request != NULL) {
    free(http_request);
    http_request = NULL;
  }
}

void on_read(uv_stream_t * stream, ssize_t nread, const uv_buf_t * buf) {
  ssize_t parsed = 0;

  http_request_t * http_request = stream->data;

  if(nread >= 0) {
    parsed = (ssize_t) http_parser_execute(
      &http_request->parser, 
      &parser_settings, 
      buf->base, 
      (size_t)nread);

    if(parsed < nread) {
      DEBUG_PRINT("PARSE ERROR");
      uv_close((uv_handle_t *) &http_request->stream, on_close);
    }
  } else {
    if(nread != UV_EOF) {
      UV_ERR(nread, "Read error");
    }

    uv_close((uv_handle_t *) &http_request->stream, on_close);
  }

  if(buf->base != NULL) { free(buf->base); }
}

void on_connect(uv_stream_t * server_handle, int status) {
  int ret;

  UV_CHECK(status, "connect");

  assert((uv_tcp_t *) server_handle == &server);

  http_request_t * http_request = malloc(sizeof(http_request_t));

  uv_tcp_init(uv_loop, (uv_tcp_t *) &http_request->stream);

  http_request->stream.data = http_request;
  http_request->parser.data = http_request;
  http_request->req.data = http_request;

  ret = uv_accept(server_handle, &http_request->stream);
  if(ret == 0) {
    // Init request struct
    http_request->url = NULL;
    http_request->method = NULL;
    http_request->body = NULL;
    http_request->header_lines = 0;
    for (int i = 0; i < MAX_HTTP_HEADERS; ++i) {
        http_request->headers[i].field = NULL;
        http_request->headers[i].field_length = 0;
        http_request->headers[i].value = NULL;
        http_request->headers[i].value_length = 0;
    }

    http_parser_init(&http_request->parser, HTTP_REQUEST);
    uv_read_start(&http_request->stream, alloc_cb, on_read);
  } else {
    DEBUG_PRINT("Error accepting request");
    uv_close((uv_handle_t *) &http_request->stream, on_close);
  }
}

void on_get_write(uv_write_t * req, int status){
  http_request_t * http_request = req->data;
  
  UV_CHECK(status, "on_get_write");

  if (http_request->url != NULL) {
      free(http_request->url);
      http_request->url = NULL;
  }

  if (http_request->method != NULL) {
      free(http_request->method);
      http_request->method = NULL;
  }

  for (int i = 0; i < http_request->header_lines; ++i) {
      br_http_header_t * header = &http_request->headers[i];

      if (header->field != NULL) {
          free(header->field);
          header->field = NULL;
      }
      if (header->value != NULL) {
          free(header->value);
          header->value = NULL;
      }
  }

  if (!uv_is_closing((uv_handle_t *)req->handle)) {
      uv_close((uv_handle_t *) req->handle, on_close);
  }
}

int parser_on_message_begin(http_parser * parser) {
  DEBUG_PRINT("Message Begin");

  http_request_t * http_request = parser->data;
  http_request->header_lines = 0;

  return 0;
}

int parser_on_url(http_parser * parser, const char * at, size_t length) {
  DEBUG_PRINT("Url: %.*s", (int) length, at);

  http_request_t * http_request = parser->data;

  http_request->url = malloc(length + 1);

  strncpy(http_request->url, at, length);
  http_request->url[length] = '\0';

  return 0;
}

int parser_on_header_field(http_parser * parser, const char * at, size_t length) {
  DEBUG_PRINT("Header field: %.*s", (int)length, at);

  http_request_t * http_request = parser->data;
  br_http_header_t * header = &http_request->headers[http_request->header_lines];

  header->field = malloc(length + 1);
  header->field_length = length;

  strncpy(header->field, at, length);
  header->field[length] = '\0';

  return 0;
}

int parser_on_header_value(http_parser * parser, const char * at, size_t length) {
  DEBUG_PRINT("Header value: %.*s", (int)length, at);

  http_request_t * http_request = parser->data;
  br_http_header_t * header = &http_request->headers[http_request->header_lines];

  header->value = malloc(length + 1);
  header->value_length = length;

  strncpy(header->value, at, length);
  header->value[length] = '\0';

  http_request->header_lines += 1;

  return 0;
}

int parser_on_headers_complete(http_parser * parser) {
  DEBUG_PRINT("Headers Complete");

  http_request_t * http_request = parser->data;

  const char * method = http_method_str((enum http_method) parser->method);
  
  http_request->method = malloc(sizeof(method));
  strncpy(http_request->method, method, strlen(method));
  
  return 0;
}

int parser_on_body(http_parser * parser, const char * at, size_t length) {
  DEBUG_PRINT("Body: %.*s", (int) length, at);

  http_request_t * http_request = parser->data;

  http_request->body = malloc(length + 1);
  http_request->body_length = length;

  strncpy(http_request->body, at, length);
  http_request->body[length] = '\0';

  return 0;
}

int parser_on_message_complete(http_parser * parser) {
  DEBUG_PRINT("Message Complete");

  http_request_t * http_request = parser->data;

  // Find matching route
  DEBUG_PRINT("Find matching route");
  char * url = http_request->url;
  br_route_node_t * route = br_g_server->routes_root;
  while(route != NULL && strcmp(url, route->route) != 0) {
    route = route->next;
  }

  // Set the handler to the route handler or default if no route found
  DEBUG_PRINT("Set handler");
  br_handler_f handler = br_g_server->default_handler;
  if(route != NULL) {
    handler = route->handler;
  }

  assert(handler != NULL);

  
  br_request_t req = { .path = url };
  br_response_t * res = br_response_new();

  // Populate the response
  handler(&req, res);

  // Convert response to HTTP response buffer
  DEBUG_PRINT("Convert response to buffer");
  char * buffer = NULL; 
  size_t length;
  if(br_response_to_buffer(res, &buffer, &length) != 0) {
    return -1;
  }

  br_response_free(res);

  // Write the response
  http_request->resp_buf[0].base = buffer;
  http_request->resp_buf[0].len = length;

  uv_write(
    &http_request->req, 
    &http_request->stream, 
    http_request->resp_buf, 
    1, 
    on_get_write);

  return 0;
}

int br_server_run(br_server_t * br_server) {
  assert(br_server != NULL);

  struct sockaddr_in address;

  br_g_server = br_server;

  signal(SIGPIPE, SIG_IGN);

  parser_settings.on_message_begin = parser_on_message_begin;
  parser_settings.on_url = parser_on_url;
  parser_settings.on_header_field = parser_on_header_field;
  parser_settings.on_header_value = parser_on_header_value;
  parser_settings.on_headers_complete = parser_on_headers_complete;
  parser_settings.on_body = parser_on_body;
  parser_settings.on_message_complete = parser_on_message_complete;

  uv_loop = uv_default_loop();

  UV_CHECK(uv_tcp_init(uv_loop, &server), "tcp_init");
  UV_CHECK(uv_ip4_addr("0.0.0.0", br_server->port, &address), "ip4_addr");
  UV_CHECK(uv_tcp_bind(&server, (const struct sockaddr *) &address, 0), "tcp_bind");
  UV_CHECK(uv_listen((uv_stream_t *) &server, MAX_WRITE_HANDLES, on_connect), "uv_listen");
  
  printf("Listening on: 0.0.0.0:%d\n", br_server->port);

  return uv_run(uv_loop, UV_RUN_DEFAULT);
}