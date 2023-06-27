# Hashtable-UPMEM

在upmem中实现hashtable数据结构。

# Environment

- The UPMEM DPU toolchain

  参考upmem官网：https://www.upmem.com/

## Project structure

* `dpu` directory contains the DPU code (i.e., the code running on the memory)
* `host` directory contains the Host code (i.e., running on the CPU)

## How to build

In order to build the program and tools, just type:

```shell
make
```

## How to run

```shell
./build/host_app
```

## 参数
参数在dpu中的dict.h，host中的connect.h
