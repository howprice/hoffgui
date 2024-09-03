#pragma once

#include <stddef.h> // gcc size_t
#include <type_traits> // underlying_type_t

#ifdef _MSC_VER
#define COMPILER_WARNING_PUSH __pragma(warning(push))
#define COMPILER_WARNING_POP __pragma(warning(pop))
#define COMPILER_WARNING_DISABLE(warningNumber) __pragma(warning(disable: warningNumber))
#elif defined(__GNUC__) || defined (__clang__)
#define DO_PRAGMA(X) _Pragma(#X)
#define COMPILER_WARNING_PUSH DO_PRAGMA(GCC diagnostic push)
#define COMPILER_WARNING_POP DO_PRAGMA(GCC diagnostic pop)
#define COMPILER_WARNING_DISABLE(warningName) DO_PRAGMA(GCC diagnostic ignored #warningName)
#else
#error Unsupported compiler
#endif

#ifdef _MSC_VER
#define COMPILER_WARNING_DISABLE_UNREFERENCED_FUNCTION COMPILER_WARNING_DISABLE(4505)
#elif defined(__GNUC__) || defined (__clang__)
#define COMPILER_WARNING_DISABLE_UNREFERENCED_FUNCTION COMPILER_WARNING_DISABLE(-Wunused-function)
#else
#error Unsupported compiler
#endif

// http://cnicholson.net/2011/01/stupid-c-tricks-a-better-sizeof_array/
template<typename T, size_t N> char (&COUNTOF_REQUIRES_ARRAY_ARGUMENT(const T(&)[N]) )[N];
#define COUNTOF_ARRAY(_x) sizeof(COUNTOF_REQUIRES_ARRAY_ARGUMENT(_x) )

// Helper macro to return the number of elements in an an enum class iff has 'Max' enumeration
// which is must be set to the last element. For example:
//
//   enum class State
//   {
//       Walking 
//		 Talking,
//       Shooting,
//
//       Max = Shooting
//   };
//
// This helps to catch missing case statements in switch statements on enumerations at compile time
// instead of run time (for example with MSVC C4062: enumerator 'identifier' in switch of enum 
// 'enumeration' is not handled) and avoids the need for default: FATAL_ERROR("Unhandled case") everywhere.
//
// n.b. Adding Count as last value is not sufficient, because would have unique value and would need
// to be handled in every switch statement.
//
// Unfortunately this introduces the opportunity for human error if a new Enum field is added to the end 
// and Max is not updated. For enums that will never require a switch statement, using the 'Count' method
// may be more robust.
//
#define ENUM_COUNT(EnumType) ((unsigned int)(EnumType::Max) + 1)

#define HP_UNUSED(X)	(void)X

#define FOURCC(a,b,c,d) ( (uint32_t) (((d)<<24) | ((c)<<16) | ((b)<<8) | (a)) )
#define FOURCC_BE(a,b,c,d) ( (uint32_t) (((a)<<24) | ((b)<<16) | ((c)<<8) | (d)) ) // big endian

//
// Use inside class definition to make the class non-instantiable i.e. all members must be static.
// Static classes simplify the codebase.
// n.b. All deleted class member functions should be public. See https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#Rc-delete
// 
#define NON_INSTANTIABLE_STATIC_CLASS(class_name) class_name() = delete;\
	class_name(const class_name&) = delete;\
	class_name(const class_name&&) = delete

// Use inside class definition to make non-copyable
// Typically, macros are generally constructed so they require a semi-colon at the end of the line, just like normal statements.
// https://stackoverflow.com/questions/28770213/macro-to-make-class-noncopyable
#define NON_COPYABLE_CLASS(class_name) class_name(const class_name&) = delete;\
	class_name& operator=(const class_name&) = delete

#ifdef RELEASE
#define RELEASE_CONST const
#else
#define RELEASE_CONST
#endif

// https://stackoverflow.com/a/600306
#define IS_POWER_OF_2(x) (((x) & ((x) - 1)) == 0)
constexpr bool IsPowerOfTwo(unsigned int v) { return v != 0 && (v & (v - 1)) == 0; }

// https://en.wikipedia.org/wiki/Data_structure_alignment
// If alignment is a power of two then:
//
// padding = (align - (offset & (align - 1))) & (align - 1)
//         = -offset & (align - 1)
// aligned = (offset + (align - 1)) & ~(align - 1)    [1]
//         = (offset + (align - 1)) & -align          [2]
//
// n.b. Because this will often be called with unsigned y, we prefer form [1] to avoid
// compiler warning C4146: unary minus operator applied to unsigned type, result still unsigned
#define ROUND_UP_POWER_OF_TWO(x,y) (((x) + ((y) - 1)) & (~((y) - 1))) // y must be a power of 2

#ifdef _MSC_VER
#define _PRISizeT   "I"
#else
#define _PRISizeT   "z"
#endif

// Convert enum to underlying type
template <typename E>
constexpr auto ToNumber(E e) noexcept
{
	return static_cast<std::underlying_type_t<E>>(e);
}

template<typename T>
inline T Min( const T& a, const T& b )
{
	return (a < b) ? a : b;
}

template<typename T>
inline T Max( const T& a, const T& b )
{
	return (a > b) ? a : b;
}

template<typename T>
inline T Clamp( const T& v, const T& low, const T& high )
{
	return Max( Min( v, high ), low );
}

template<typename T> 
inline T Lerp(T& a, T& b, float t) 
{ 
	return (T)(a + (b - a) * t); 
}

inline float Floor(float t)
{
	return (float)(int)t;
}

inline float Frac(float t)
{
	return t - Floor(t);
}

template<typename T>
inline void Swap(T& a, T& b)
{
	T temp = a;
	a = b;
	b = temp;
}

inline unsigned int RoundUpToNextPowerOf2(unsigned int v)
{
	if (v == 0) return 1;
	v--;
	v |= v >> 1;
	v |= v >> 2;
	v |= v >> 4;
	v |= v >> 8;
	v |= v >> 16;
	v++;
	return v;
}
