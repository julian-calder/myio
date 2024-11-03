# myio
Wrappers around standard read/write syscalls in C, optimized for variable sized intermingled read/write calls.

## Usage: 
The `myread`, `mywrite`, `myseek`, `myopen`, and `myclose` functions are meant to replace the IO syscalls found in the C standard library (see `man 2 write`). To use these wrapper functions, one must include the header file `#include "myio.h"` at the top of their program.

## Key improvements:
The `myopen` and `myclose` wrappers behave very similarly to their stdlib counterparts, but instead of returning an integer file descriptor we return a pointer to a struct containing key information about the file being accessed.

The `myread` and `mywrite` work much like `read(2)` and `write(2)`, but contain optimizations for repeated small reads and writes. The stdlib implementation performs a syscall for every read or write which is performed, regardless of size. In most cases this is a non-issue, as one rarely performs several consecutive read calls. However, in the hypothetical case in which one may be performing several very small read/write calls (say, 20 bytes each), the stdlib implementation will go through the syscall handler for each one of these which is quite inefficient.

My wrapper functions improve this by implementing a hidden buffer which reduces the number of syscalls required for the user to access data. If the user attempts to read fewer bytes than the size of the hidden buffer, a single larger read call will be completed and the number of bytes requested by the user will be provided. Subsequent read calls will then come out of this buffer and not from disk, eliminating the need for another syscall.