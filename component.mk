#
# component makefile.
#
# (Uses default behaviour of compiling all source files in directory, adding 'include' to include path.)

# COMPONENT_PRIV_INCLUDEDIRS += utils
COMPONENT_SRCDIRS += utils/json11
COMPONENT_ADD_INCLUDEDIRS += include utils
CPPFLAGS += -D_GLIBCXX_USE_C99