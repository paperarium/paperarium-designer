# Created by: Evan Kirkiles
# Created on: Fri Dec 16 2022
# for Paperarium

set(INC
  .
  .intern

  ../paperkernel)

set(SRC
  intern/context.c

  PKE_context.h)

# create draw library
add_library(pp_draw SRC)

# add includes
target_include_directories(pp_draw INC)