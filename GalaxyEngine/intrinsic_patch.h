
#ifndef INTRINSIC_PATCH_H
#define INTRINSIC_PATCH_H

#ifdef __clang__
// Provide dummy definitions if they are not already defined.
// These definitions will let the headers compile even if they don't
// do anything useful (which is acceptable if your code never calls them).

// __bf16: bfloat16 type. A common workaround is to define it as unsigned short.
#ifndef __bf16
typedef unsigned short __bf16;
#endif

// __tile1024i: This type is used for certain AVX-512 tile operations.
#ifndef __tile1024i
typedef int __tile1024i;
#endif

#endif // __clang__

#endif // INTRINSIC_PATCH_H