sp             := $(sp).x
dirstack_$(sp) := $(d)
d              := $(dir)


FILES:=file_list.cpp file_package_manager.cpp input_file_stream.cpp \
       input_stream.cpp input_stream_wrapper.cpp memory_stream.cpp \
       output_file_stream.cpp output_stream.cpp standard_package.cpp \
       zip_package.cpp rle_packed_file_wrapper.cpp

SRC_$(d):=$(addprefix $(d)/,$(FILES))

CLEANDIRS+=$(d)

-include $(foreach FILE,$(FILES),$(d)/$(FILE:.cpp=.d))

d  := $(dirstack_$(sp))
sp := $(basename $(sp))
