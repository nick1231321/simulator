//includes

#include <cstdint>
#include <cstddef>


// Primary template declaration - left undefined intentionally.
template<typename T>
struct funcPtrTemporaryStruct;

// Partial specialization for function types.
// This specialization is selected when T is of the form R(Args...),
// where R is the return type and Args... is a pack of argument types.
template<typename R, typename... Args>
struct funcPtrTemporaryStruct<R(Args...)> {
    // 'type' is defined as a pointer to a function that takes Args... and returns R.
    using type = R(*)(Args...);
};

// Helper alias template to simplify usage.
// Instead of writing typename funcPtrTemporaryStruct<SomeSignature>::type, you can simply write funcPtr<SomeSignature>
template<typename T>
using funcPtr = typename funcPtrTemporaryStruct<T>::type;


//defer statement
template <typename F>
struct privDefer {
	F f;
	privDefer(F f) : f(f) {}
	~privDefer() { f(); }
};

template <typename F>
privDefer<F> defer_func(F f) {
	return privDefer<F>(f);
}

#define DEFER_1(x, y) x##y
#define DEFER_2(x, y) DEFER_1(x, y)
#define DEFER_3(x)    DEFER_2(x, __COUNTER__)
#define defer(code)   auto DEFER_3(_defer_) = defer_func([&](){code;})


//modern types
using i8  = int8_t;
using u8  = uint8_t;
using i16 = int16_t;
using u16 = uint16_t;
using i32 = int32_t;
using u32 = uint32_t;
using i64 = int64_t;
using u64 = uint64_t;

using isize = ptrdiff_t;
using usize = size_t;