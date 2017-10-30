#include <stdio.h>
#include <strings.h>

#include "response.h"

void br_response_set_content_string(br_response * response, const char * str) {
  response->content = str;
  response->content_length = strlen(str);
}

void br_response_add_header(br_response * response, const char * field, const char * value) {
  int field_length = strlen(field);
  int value_length = strlen(value);
  int length = response->header_fields_length + field_length + value_length + 4;

  char * buffer = malloc((length * sizeof(char)));

  if(response->header_fields != NULL) {
    response->header_fields_length = sprintf(buffer, "%s%s: %s\r\n", 
      response->header_fields, 
      field, value);
  } else {
    response->header_fields_length = sprintf(buffer, "%s: %s\r\n", 
      field, value);
  }

  response->header_fields = buffer;
}

const char * br_status_code_to_message(int code)
{
  switch(code)
  {
    case 200: return "OK";
    case 404: return "Not Found";
    case 500: return "Internal Server Error";
    default: return "Unknwon";
  }
}


char * br_response_to_string(br_response * r) {
  const char * html_template = "HTTP/1.1 %d %s\r\n"
                               "%s"
                               "\r\n"
                               "%s";

  char content_length[10];
  sprintf(content_length, "%d", (int)r->content_length);

  br_response_add_header(r, "Server",           "Brightray 0.0.1");
  br_response_add_header(r, "Content-Length",   content_length);
  br_response_add_header(r, "Conent-Type",      "text/html");

  char * buffer = malloc(1000 * sizeof(char));
  sprintf(buffer, html_template, 
    r->status_code, 
    br_status_code_to_message(r->status_code),
    r->header_fields,
    r->content);

  return buffer;
}