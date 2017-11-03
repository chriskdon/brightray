include_directories (${PROJECT_SOURCE_DIR}/src/)

file(GLOB brightray_SRC
  "*.h"
  "*.c"
)

add_library(brightray STATIC ${brightray_SRC})

target_link_libraries(brightray sds uv)