NVCC ?= /usr/local/cuda/bin/nvcc
INCLUDES ?= -ICommon -ICommon/UtilNPP

ALL_CCFLAGS ?= --std=c++17 -O3
GENCODE_FLAGS ?= -gencode arch=compute_86,code=compute_86
LIBRARIES ?= -lnppisu_static -lnppif_static -lnppc_static -lculibos -lfreeimage

all: build

build: boxFilterNPP

boxFilterNPP.o: boxFilterNPP.cpp
	$(NVCC) $(INCLUDES) $(ALL_CCFLAGS) $(GENCODE_FLAGS) -o $@ -c $<

boxFilterNPP:boxFilterNPP.o
	$(NVCC) $(GENCODE_FLAGS) -o $@ $+ $(LIBRARIES)

run: build
	./boxFilterNPP

clean:
	rm -f boxFilterNPP boxFilterNPP.o
