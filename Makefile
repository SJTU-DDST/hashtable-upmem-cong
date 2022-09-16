BUILDDIR ?= build

HOST_BINARY=${BUILDDIR}/host_app
HOST_SOURCES=$(wildcard host/*.c)
HOST_HEADERS=$(wildcard host/*.h)

DPU_BINARY=${BUILDDIR}/dpu_task
DPU_SOURCES=$(wildcard dpu/*.c)
DPU_HEADERS=$(wildcard dpu/*.h)

CHECK_FORMAT_FILES=${HOST_SOURCES} ${HOST_HEADERS} ${DPU_SOURCES} ${DPU_HEADERS}
CHECK_FORMAT_DEPENDENCIES=$(addsuffix -check-format,${CHECK_FORMAT_FILES})

NR_TASKLETS ?= 1

__dirs := $(shell mkdir -p ${BUILDDIR})

.PHONY: all clean run plotdata check check-format tools

all: ${HOST_BINARY} ${DPU_BINARY} tools
clean:
	rm -rf ${BUILDDIR}

###
### HOST APPLICATION
###
CFLAGS=-g -O2 -std=c99 `dpu-pkg-config --cflags dpu`-DNR_TASKLETS=${NR_TASKLETS}
LDFLAGS=`dpu-pkg-config --libs dpu`

${HOST_BINARY}: ${HOST_SOURCES} ${HOST_HEADERS} ${DPU_BINARY}
	$(CC) -o $@ ${HOST_SOURCES} $(LDFLAGS) $(CFLAGS) -DDPU_BINARY=\"$(realpath ${DPU_BINARY})\"

###
### DPU BINARY
###
DPU_FLAGS=-g -O2 -DNR_TASKLETS=${NR_TASKLETS} -DSTACK_SIZE_DEFAULT=512

${DPU_BINARY}: ${DPU_SOURCES} ${DPU_HEADERS}
	dpu-upmem-dpurte-clang ${DPU_FLAGS} ${DPU_SOURCES} -o $@
