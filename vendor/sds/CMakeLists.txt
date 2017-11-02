file(GLOB sds_SRC
  "*.h"
  "*.c"
)

add_library(sds STATIC ${sds_SRC})