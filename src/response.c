#include <stdio.h>
#include <strings.h>
#include <assert.h>

#include <sds.h>

#include "response.h"
#include "debug.h"
#include "errors.h"

br_response_t * br_response_new() {
  br_response_t * response = malloc(sizeof(br_response_t));

  response->status_code = 0;
  response->headers_length = 0;
  response->content = NULL;
  response->content_length = 0;
  
  return response;
}

void br_response_free(br_response_t * response) {
  assert(response != NULL);

  for(int i = 0; i < response->headers_length; i++) {
    free(response->headers[i]);
  }

  free(response->content);
  free(response);
}

void br_response_set_content_string(br_response_t * response, const char * str) {
  response->content_length = strlen(str);
  response->content = malloc(response->content_length * sizeof(char));
  strcpy(response->content, str);
}

void br_response_add_header(br_response_t * response, const char * field, const char * value) {
  assert(response != NULL);
  assert(field != NULL);
  assert(value != NULL);

  br_http_header_t * header = malloc(sizeof(br_http_header_t));

  header->field_length = strlen(field);
  header->value_length = strlen(value);
  header->field = malloc(header->field_length * sizeof(char) + 1);
  header->value = malloc(header->value_length * sizeof(char) + 1);
  strcpy(header->field, field);
  strcpy(header->value, value);
  
  response->headers[response->headers_length] = header;
  response->headers_length += 1;
}

const char * br_status_code_to_message(int code)
{
  switch(code)
  {
    case 200: return "OK";
    case 404: return "Not Found";
    case 500: return "Internal Server Error";
    default: return "Unknown";
  }
}

int br_response_to_buffer(br_response_t * r, char ** buffer, size_t * length) {
  assert(r != NULL);
  
  const char * html_template = "HTTP/1.1 %d %s\r\n"
                               "%s"
                               "\r\n"
                               "%s";

  char content_length[10];
  sprintf(content_length, "%d", (int)r->content_length);

  br_response_add_header(r, "Server",           "Brightray 0.0.1");
  br_response_add_header(r, "Content-Length",   content_length);
  br_response_add_header(r, "Content-Type",      "text/html");

  // Create header block  
  sds headers = sdsempty();
  for(int i = 0; i < r->headers_length; i++) {
    br_http_header_t * header = r->headers[i];
    headers = sdscatprintf(headers, "%s: %s\r\n", header->field, header->value);
  }

  *buffer = malloc(1000 * sizeof(char)); // TODO
  sprintf(*buffer, html_template, 
    r->status_code, 
    br_status_code_to_message(r->status_code),
    headers,
    r->content);

  sdsfree(headers);

  *length = strlen(*buffer);

  return BR_SUCCESS;
}