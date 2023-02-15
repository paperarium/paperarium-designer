# Created by: Evan Kirkiles
# Created on: Fri Dec 16 2022
# for Paperarium

set(INC
  .
  ./intern)

set(SRC
  intern/context.c

  PKE_context.h)

# create kernel library
add_library(pp_paperkernel ${SRC})

# add includes
target_include_directories(pp_paperkernel PRIVATE ${INC})