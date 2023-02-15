set(INC
  .)

set(SRC
  DNA_vec_types.h
)

# build library
add_library(pp_makesdna ${SRC})
target_include_directories(pp_makesdna PRIVATE ${INC})