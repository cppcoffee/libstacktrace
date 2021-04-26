# Introduce

The libstacktrace is a c library that provides a simple API to output the crash call stack to **STDERR** when the process crash.


## Build

```shell
$ make
```


## Usage

Include **stacktrace.h** to project, and call **init_stacktrace()** API to initialize library.

```c
#include "stacktrace.h"

int main(int argc, char *argv[]) {
{
    // initialize library.
    init_stacktrace();

    // do some thing.
}
```


## Example

Examples of use are provided in **example.c** of the repository, which makes null pointer access, and the process crashes with the following output:

```shell
[root@localhost libstacktrace]# ./example
/home/sharp/libstacktrace/example - STACK TRACE:
/lib64/libc.so.6(+0x36450) [0x7fca8db52450]
./example() [0x4036f0] example.c bar()
./example() [0x403703] example.c foo()
./example() [0x40371d] ?? main()
/lib64/libc.so.6(__libc_start_main+0xf5) [0x7fca8db3e555]
./example() [0x402c9a] ?? _start()
Segmentation fault
```

If **CCFLAGS** is added **-g** flag, more detailed information will be output:

```shell
[root@localhost libstacktrace]# ./example
/home/sharp/libstacktrace/example - STACK TRACE:
/lib64/libc.so.6(+0x36450) [0x7fa21a2e0450]
./example() [0x4036f0] /home/sharp/libstacktrace/example.c bar():8
./example() [0x403703] /home/sharp/libstacktrace/example.c foo():15
./example() [0x40371d] /home/sharp/libstacktrace/example.c main():24
/lib64/libc.so.6(__libc_start_main+0xf5) [0x7fa21a2cc555]
./example() [0x402c9a] ?? _start()
Segmentation fault
```


## Dependence

The libstacktrace dependence on **libbfd** (Binary File Descriptor library). 

bfd build elf sections data, find the file name, function name, and source line where the stack pointer is located based on the elf sections data.

bfd is provided by the **binutils** package, which can be installed in CentOS distributions with the following command:

```shell
yum install binutils-devel -y
```


## Reference

[https://man7.org/linux/man-pages/man1/addr2line.1.html](https://man7.org/linux/man-pages/man1/addr2line.1.html)

[https://sourceware.org/binutils/docs/bfd/](https://sourceware.org/binutils/docs/bfd/)

