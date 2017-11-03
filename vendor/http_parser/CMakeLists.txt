file(GLOB http_parser_SRC
"*.h"
"*.c"
)

add_library(http_parser STATIC ${http_parser_SRC})