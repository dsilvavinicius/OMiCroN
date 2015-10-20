############################################
# Two-level segregate fit build description.
# Path: src/lib/tlsf
############################################

SET (CMAKE_C_FLAGS "-DUSE_MMAP=1 -DUSE_SBRK=1 -DTLSF_STATISTIC=1")

add_library(Tlsf tlsf.c)