#if !defined(LOGL_PLATFORM_H)

#include <stdint.h>
#include <stddef.h>

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;
typedef i32 b32;

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef size_t memory_index;

typedef float f32;
typedef double r64;

#define internal static
#define local_persist static
#define global_variable static

#ifdef LOGL_SLOW
#define Assert(cond) if(!(cond)) {*(int *)0 = 0;}
#else
#define Assert(cond)
#endif

#define InvalidCodePath Assert(!"InvalidCodePath");

#define Kilobytes(Value) ((Value)*1024LL)
#define Megabytes(Value) (Kilobytes(Value)*1024LL)
#define Gigabytes(Value) (Megabytes(Value)*1024LL)
#define Terabytes(Value) (Gigabytes(Value)*1024LL)

#define ArrayCount(Array) (sizeof(Array) / sizeof((Array)[0]))

#define Swap(a, b, t) {t temp = a; a = b; b = temp;}

inline u32
SafeTruncateUInt64(u64 Value)
{
    Assert(Value <= 0xFFFFFFFF);
    u32 Result = (u32)Value;
    return(Result);
}

void ThrowError(const char *message)
{
    MessageBoxA(nullptr, message, "LOGL ERROR", MB_OK | MB_ICONERROR);
    __debugbreak();
}

#define LOGL_PLATFORM_H
#endif
