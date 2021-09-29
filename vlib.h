/*
Except where stated otherwise (borrowed code), this file is in the public
domain:

This is free and unencumbered software released into the public domain.
Anyone is free to copy, modify, publish, use, compile, sell, or
distribute this software, either in source code form or as a compiled
binary, for any purpose, commercial or non-commercial, and by any means.
In jurisdictions that recognize copyright laws, the author or authors
of this software dedicate any and all copyright interest in the
software to the public domain.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND.
IN NO EVENT SHALL THE AUTHORS BE HELD LIABLE IN CONNECTION WITH THE SOFTWARE.
*/

/*
@FEATURES to add in the future
- threads
*/

#if __GNUC__
#define V_GNUC 1
#elif _MSC_VER
#define V_MSVC 1
#elif __APPLE__
#define V_APPLE 1
#endif

#if __unix__ || __APPLE__
#define V_UNIXY 1
#endif

#if _WIN32
#define V_WINDOWS 1
#endif

#if V_MSVC
#pragma comment(lib, "bcrypt.lib")
#pragma comment(lib, "legacy_stdio_definitions.lib")
#endif

#ifdef V_LIB_H_
	// this file was included twice
	#if V_GNUC
		#warning "vlib.h included twice"
	#elif V_MSVC
		#pragma message("Warning: vlib.h included twice")
	#else
		// oh well
	#endif
#else

#define V_LIB_H_ 1

#if __cplusplus
extern "C" {
#endif

#ifndef V_DECL
#define V_DECL static
#endif
#ifndef V_VARDECL
#define V_VARDECL static
#endif

#include <stdbool.h>
#include <stddef.h>
#include <inttypes.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <time.h>
#include <errno.h>
#include <signal.h>

// what the fuck is wrong with you microsoft
#ifdef min
#undef min
#endif
#ifdef max
#undef max
#endif

#if __GNUC__
#define if_unlikely(x) if (__builtin_expect(x, 0))
#else
#define if_unlikely if
#endif

#ifndef alloca
	#if V_GNUC
		#define alloca __builtin_alloca
	#elif V_WINDOWS
		void *_alloca(size_t);
		#define alloca _alloca
	#elif V_UNIXY
		#include <alloca.h>
	#endif
#endif

// ---- WINAPI FUNCTIONS ----
#if V_WINDOWS
void OutputDebugStringA(const char *);
#endif // V_WINDOWS


// ---- CORE MACROS ----

#define V_STRINGIFY3(x) #x
#define V_STRINGIFY2(x) V_STRINGIFY3(x)
#define STRINGIFY(x) V_STRINGIFY2(x)

#ifndef NO_WARN_START
#if V_GNUC
#define NO_WARN_START \
	_Pragma("GCC diagnostic push") \
	_Pragma("GCC diagnostic ignored \"-Wconversion\"") \
	_Pragma("GCC diagnostic ignored \"-Wsign-conversion\"") \
	_Pragma("GCC diagnostic ignored \"-Wunused-parameter\"") \
	_Pragma("GCC diagnostic ignored \"-Wsign-compare\"") \
	_Pragma("GCC diagnostic ignored \"-Wpedantic\"") \
	_Pragma("GCC diagnostic ignored \"-Wshadow\"")
#define NO_WARN_END \
	_Pragma("GCC diagnostic pop")
#elif V_MSVC
#define NO_WARN_START \
	_Pragma("warning(push)") \
	_Pragma("warning(disable: 4456 4457 4100 4244 4701)")
#define NO_WARN_END \
	_Pragma("warning(pop)")
#else
#define NO_WARN_START
#define NO_WARN_END
#endif
#endif

#define static_arr_len(a) ((sizeof (a)) / (sizeof *(a)))

// print - like printf but works on windows
#if V_WINDOWS
	#define print(...) do {\
		char _buf[1024] = {0};\
		snprintf(_buf, sizeof _buf - 1, __VA_ARGS__);\
		OutputDebugStringA(_buf);\
	} while (0)
#else
	#define print printf
#endif

#ifndef assert
	#ifdef NDEBUG
		#define assert(x) ((void)0)
	#else
		#define assert(x) do { if (!(x)) { print("Assertion failed at %s:%d: %s\n", __FILE__, __LINE__, #x); abort(); } } while (0)
	#endif
#endif

#ifndef debug_print
	#if DEBUG
	#define debug_print printf
	#else
	#define debug_print(...) (void)0
	#endif
#endif

// ---- TYPES/MACROS ----

// --- MISC ---
#if V_DEBUG_MEMORY
// be sure to call vlib_memory_check at the end of main() !
// (it could be an __attribute__((destructor)) or something but you might
// not want to print leaks if you exit because of a failure)
#define malloc(n) V_debug_malloc(n, __FILE__, __LINE__, 0xcd /* fill memory with non-zero bytes */)
#define calloc(n,s) V_debug_calloc(n, s, __FILE__, __LINE__)
#define realloc(m,n) V_debug_realloc(m, n, __FILE__, __LINE__)
#define free(m) V_debug_free(m, __FILE__, __LINE__)
#define V_DEBUG_MEMORY_LINE __LINE__
#define V_DEBUG_MEMORY_FILE __FILE__
#else
#define vlib_memory_check()
#define V_DEBUG_MEMORY_LINE 0
#define V_DEBUG_MEMORY_FILE 0
#endif

// --- STRINGS ---
#define str_eq(a, b) (strcmp(a, b) == 0)
// necessary to avoid windows' snprintf problem
#define str_print(s, n, ...) (s)[n - 1] = '\0', snprintf(s, n - 1, __VA_ARGS__)
// use on char arrays only!
#define strbuf_print(s, ...) str_print(s, sizeof s, __VA_ARGS__)
// like str_print, but prints to the end of the string
#define str_append(s, n, ...) str_print((s) + strlen(s), (n) - strlen(s), __VA_ARGS__)
#define strbuf_append(s, ...) str_append(s, sizeof s, __VA_ARGS__)
#define strbuf_cpy(dst, src) str_cpy(dst, sizeof dst, src)
#define strbuf_cat(dst, src) str_cat(dst, sizeof dst, src)
#define UNICODE_CODE_POINTS 0x110000 // number of Unicode code points

// --- MATH ---
#define PI 3.141592653589793f
#define SQRT2 1.4142135623730951f
#define SQRT3 1.7320508075688772f
#define Vec2(x, y)        ((vec2 ){(x), (y)})
#define Vec3(x, y, z)     ((vec3 ){(x), (y), (z)})
#define Vec4(x, y, z, w)  ((vec4 ){(x), (y), (z), (w)})
#define Vec2i(x, y)       ((vec2i){(x), (y)})
#define Vec3i(x, y, z)    ((vec3i){(x), (y), (z)})
#define Vec4i(x, y, z, w) ((vec4i){(x), (y), (z), (w)})
#define degree2rad(x) ((PI / 180) * (x))
#define rad2degree(x) ((180 / PI) * (x))
#define printM3 printlnM3
#define printM4 printlnM4
#define modf modf_ // i hate libc

typedef struct {
	float x, y;
} vec2;

typedef struct {
	float x, y, z;
} vec3;

typedef struct {
	float x, y, z, w;
} vec4;

typedef struct {
	int x, y;
} vec2i;

typedef struct {
	int x, y, z;
} vec3i;

typedef struct {
	int x, y, z, w;
} vec4i;

typedef struct {
	float v[9];
} mat3;

typedef struct {
	float v[16];
} mat4;


// --- DYNAMIC ARRAY ---
typedef union {
	long num;
	void *ptr;
	void (*fnptr)(void);
#ifdef ARR_LONG_DOUBLE
	long
#endif
	double flt;
} ArrMaxAlign;

typedef struct {
	uint32_t len;
	uint32_t cap;
#if V_MSVC
	#pragma warning(push)
	#pragma warning(disable: 4200)
#endif
	ArrMaxAlign data[];
#if V_MSVC
	#pragma warning(pop)
#endif
} ArrHeader;

// watch out! do not use this if arr is NULL.
#define arr_hdr_(arr) ((ArrHeader *)((char *)(arr) - offsetof(ArrHeader, data)))

#ifdef __cplusplus
#define arr_cast_typeof(a) (decltype(a))
#elif defined __GNUC__
#define arr_cast_typeof(a) (__typeof__(a))
#else
#define arr_cast_typeof(a)
#endif

#define arr__join2(a,b) a##b
#define arr__join(a,b) arr__join2(a,b) // macro used internally

// if the array is not NULL, free it and set it to NULL
#define arr_free(a) do { if (a) { free(arr_hdr_(a)); (a) = NULL; } } while (0)
// a nice alias
#define arr_clear(a) arr_free(a)
// add an item to the array - if allocation fails, the array will be freed and set to NULL.
// (how this works: if we can successfully grow the array, increase the length and add the item.)
#define arr_add(a, x) do { if (((a) = arr_cast_typeof(a) arr_grow1_((a), sizeof *(a), V_DEBUG_MEMORY_FILE, V_DEBUG_MEMORY_LINE))) ((a)[arr_hdr_(a)->len++] = (x)); } while (0)
// like arr_add, but instead of passing it the value, it returns a pointer to the value. returns NULL if allocation failed.
// the added item will be zero-initialized.
#define arr_addp(a) arr_cast_typeof(a) arr_add_ptr_((void **)&(a), sizeof *(a), V_DEBUG_MEMORY_FILE, V_DEBUG_MEMORY_LINE)
// set the length of `a` to `n`, increasing the capacity if necessary.
// the newly-added elements are zero-initialized.
#define arr_qsort(a, cmp) qsort((a), arr_len(a), sizeof *(a), (cmp))
#define arr_remove_last(a) do { assert(a); if (--arr_hdr_(a)->len == 0) arr_free(a); } while (0)
#define arr_remove(a, i) (void)((a) = arr_remove_((a), sizeof *(a), (i)))
#define arr_insert(a, i, x) do { u32 _index = (i); (a) = arr_cast_typeof(a) arr_grow1_((a), sizeof *(a)); \
	if (a) { memmove((a) + _index + 1, (a) + _index, (arr_len(a) - _index) * sizeof *(a));\
	(a)[_index] = x; \
	++arr_hdr_(a)->len; } } while (0)
#define arr_pop_last(a) ((a)[--arr_hdr_(a)->len])
#define arr_size_in_bytes(a) (arr_len(a) * sizeof *(a))
#define arr_lastp(a) ((a) ? &(a)[arr_len(a)-1] : NULL)
#define arr_foreachp_end(a, type, var, end) type *end = (a) + arr_len(a); \
	for (type *var = (a); var != end; ++var)
// Iterate through each element of the array, setting var to a pointer to the element.
// You can't use this like, e.g.:
// if (something)
//   arr_foreachp(a, int, i);
// You'll get an error. You will need to use braces because it expands to multiple statements.
// (we need to name the end pointer something unique, which is why there's that arr__join thing
// we can't just declare it inside the for loop, because type could be something like char *.)
#define arr_foreachp(a, type, var) arr_foreachp_end(a, type, var, arr__join(_foreach_end,arr__join(var,__LINE__)))

#define arr_reverse(a, type) do { \
	u64 _i, _len = arr_len(a); \
	for (_i = 0; 2*_i < _len; ++_i) { \
		type *_x = &(a)[_i]; \
		type *_y = &(a)[_len-1-_i]; \
		type _tmp; \
		_tmp = *_x; \
		*_x = *_y; \
		*_y = _tmp; \
	} \
	} while (0)

// Ensure that enough space is allocated for n elements.
#define arr_reserve(a, n) arr_reserve_((void **)&(a), sizeof *(a), (n))
// Similar to arr_reserve, but also sets the length of the array to n.
// Does NOT shrink the capacity of the array! This is on purpose (so that you can set the length to 0
// and re-add a bunch of stuff without needing to allocate again).
#define arr_set_len(a, n) arr_set_len_((void **)&(a), sizeof *(a), (n))

// --- OS ---

typedef struct timespec Time;

typedef enum {
	FS_NON_EXISTENT,
	FS_FILE,
	FS_DIR,
	FS_LINK,
	FS_OTHER
} FsType;

enum {
	FS_PERMISSION_READ = 0x01,
	FS_PERMISSION_WRITE = 0x02
};

typedef unsigned int FsPermission;

typedef struct {
	FsType type;
	char *name;
} DirEntry;

#if V_UNIXY
#define PATH_SEPARATOR '/'
#elif V_WINDOWS
#define PATH_SEPARATOR '\\'
#else
#error "unrecognized os"
#endif

// --- FILE TYPES ---

// 8 bits per channel image
typedef struct {
	vec2i size;
	int channels;
	uint8_t *data; // **bottom-to-top** data
} Image;

// 16 bits per channel image
typedef struct {
	vec2i size;
	int channels;
	uint16_t *data; // **bottom-to-top** data
} Image16;

// --- RANDOM - NOT CRYPTOGRAPHICALLY SECURE ---

typedef struct {
	uint64_t seed;
} Generator;

// --- GUI / GL ---
#if V_GL
// t = audio position in seconds
// this should return true if t should be incremented (i.e. return false to pause audio)
typedef bool (*AudioCallback)(void *userdata, int16_t *buffer, int sample_rate, size_t frame_count, double t);
typedef struct SDL_mutex *Mutex;
#endif

#if V_GL
/*
-*- BEGIN (modified) BORROWED CODE -*-
LICENSE:
** Copyright (c) 2013-2018 The Khronos Group Inc.
**
** Permission is hereby granted, free of charge, to any person obtaining a
** copy of this software and/or associated documentation files (the
** "Materials"), to deal in the Materials without restriction, including
** without limitation the rights to use, copy, modify, merge, publish,
** distribute, sublicense, and/or sell copies of the Materials, and to
** permit persons to whom the Materials are furnished to do so, subject to
** the following conditions:
**
** The above copyright notice and this permission notice shall be included
** in all copies or substantial portions of the Materials.
**
** THE MATERIALS ARE PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
** EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
** MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
** IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
** CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
** TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
** MATERIALS OR THE USE OR OTHER DEALINGS IN THE MATERIALS.
*/
#if _WIN32
#define APIENTRY __stdcall
#endif
#ifndef APIENTRY
#define APIENTRY
#endif
#ifndef APIENTRYP
#define APIENTRYP APIENTRY *
#endif
#ifndef GLAPI
#define GLAPI extern
#endif
#ifdef _WIN64
typedef signed long long int khronos_intptr_t;
typedef signed long long int khronos_ssize_t;
#else
typedef signed long int khronos_intptr_t;
typedef signed long int khronos_ssize_t;
#endif
#ifndef GL_VERSION_1_0
#define GL_VERSION_1_0 1
typedef void GLvoid;
typedef unsigned int GLenum;
typedef float GLfloat;
typedef int GLint;
typedef int GLsizei;
typedef unsigned int GLbitfield;
typedef double GLdouble;
typedef unsigned int GLuint;
typedef unsigned char GLboolean;
typedef uint8_t GLubyte;
#define GL_DEPTH_BUFFER_BIT               0x00000100
#define GL_STENCIL_BUFFER_BIT             0x00000400
#define GL_COLOR_BUFFER_BIT               0x00004000
#define GL_FALSE                          0
#define GL_TRUE                           1
#define GL_POINTS                         0x0000
#define GL_LINES                          0x0001
#define GL_LINE_LOOP                      0x0002
#define GL_LINE_STRIP                     0x0003
#define GL_TRIANGLES                      0x0004
#define GL_TRIANGLE_STRIP                 0x0005
#define GL_TRIANGLE_FAN                   0x0006
#define GL_QUADS                          0x0007
#define GL_NEVER                          0x0200
#define GL_LESS                           0x0201
#define GL_EQUAL                          0x0202
#define GL_LEQUAL                         0x0203
#define GL_GREATER                        0x0204
#define GL_NOTEQUAL                       0x0205
#define GL_GEQUAL                         0x0206
#define GL_ALWAYS                         0x0207
#define GL_ZERO                           0
#define GL_ONE                            1
#define GL_SRC_COLOR                      0x0300
#define GL_ONE_MINUS_SRC_COLOR            0x0301
#define GL_SRC_ALPHA                      0x0302
#define GL_ONE_MINUS_SRC_ALPHA            0x0303
#define GL_DST_ALPHA                      0x0304
#define GL_ONE_MINUS_DST_ALPHA            0x0305
#define GL_DST_COLOR                      0x0306
#define GL_ONE_MINUS_DST_COLOR            0x0307
#define GL_SRC_ALPHA_SATURATE             0x0308
#define GL_NONE                           0
#define GL_FRONT_LEFT                     0x0400
#define GL_FRONT_RIGHT                    0x0401
#define GL_BACK_LEFT                      0x0402
#define GL_BACK_RIGHT                     0x0403
#define GL_FRONT                          0x0404
#define GL_BACK                           0x0405
#define GL_LEFT                           0x0406
#define GL_RIGHT                          0x0407
#define GL_FRONT_AND_BACK                 0x0408
#define GL_NO_ERROR                       0
#define GL_INVALID_ENUM                   0x0500
#define GL_INVALID_VALUE                  0x0501
#define GL_INVALID_OPERATION              0x0502
#define GL_OUT_OF_MEMORY                  0x0505
#define GL_CW                             0x0900
#define GL_CCW                            0x0901
#define GL_POINT_SIZE                     0x0B11
#define GL_POINT_SIZE_RANGE               0x0B12
#define GL_POINT_SIZE_GRANULARITY         0x0B13
#define GL_LINE_SMOOTH                    0x0B20
#define GL_LINE_WIDTH                     0x0B21
#define GL_LINE_WIDTH_RANGE               0x0B22
#define GL_LINE_WIDTH_GRANULARITY         0x0B23
#define GL_POLYGON_MODE                   0x0B40
#define GL_POLYGON_SMOOTH                 0x0B41
#define GL_CULL_FACE                      0x0B44
#define GL_CULL_FACE_MODE                 0x0B45
#define GL_FRONT_FACE                     0x0B46
#define GL_DEPTH_RANGE                    0x0B70
#define GL_DEPTH_TEST                     0x0B71
#define GL_DEPTH_WRITEMASK                0x0B72
#define GL_DEPTH_CLEAR_VALUE              0x0B73
#define GL_DEPTH_FUNC                     0x0B74
#define GL_STENCIL_TEST                   0x0B90
#define GL_STENCIL_CLEAR_VALUE            0x0B91
#define GL_STENCIL_FUNC                   0x0B92
#define GL_STENCIL_VALUE_MASK             0x0B93
#define GL_STENCIL_FAIL                   0x0B94
#define GL_STENCIL_PASS_DEPTH_FAIL        0x0B95
#define GL_STENCIL_PASS_DEPTH_PASS        0x0B96
#define GL_STENCIL_REF                    0x0B97
#define GL_STENCIL_WRITEMASK              0x0B98
#define GL_VIEWPORT                       0x0BA2
#define GL_DITHER                         0x0BD0
#define GL_BLEND_DST                      0x0BE0
#define GL_BLEND_SRC                      0x0BE1
#define GL_BLEND                          0x0BE2
#define GL_LOGIC_OP_MODE                  0x0BF0
#define GL_DRAW_BUFFER                    0x0C01
#define GL_READ_BUFFER                    0x0C02
#define GL_SCISSOR_BOX                    0x0C10
#define GL_SCISSOR_TEST                   0x0C11
#define GL_COLOR_CLEAR_VALUE              0x0C22
#define GL_COLOR_WRITEMASK                0x0C23
#define GL_DOUBLEBUFFER                   0x0C32
#define GL_STEREO                         0x0C33
#define GL_LINE_SMOOTH_HINT               0x0C52
#define GL_POLYGON_SMOOTH_HINT            0x0C53
#define GL_UNPACK_SWAP_BYTES              0x0CF0
#define GL_UNPACK_LSB_FIRST               0x0CF1
#define GL_UNPACK_ROW_LENGTH              0x0CF2
#define GL_UNPACK_SKIP_ROWS               0x0CF3
#define GL_UNPACK_SKIP_PIXELS             0x0CF4
#define GL_UNPACK_ALIGNMENT               0x0CF5
#define GL_PACK_SWAP_BYTES                0x0D00
#define GL_PACK_LSB_FIRST                 0x0D01
#define GL_PACK_ROW_LENGTH                0x0D02
#define GL_PACK_SKIP_ROWS                 0x0D03
#define GL_PACK_SKIP_PIXELS               0x0D04
#define GL_PACK_ALIGNMENT                 0x0D05
#define GL_MAX_TEXTURE_SIZE               0x0D33
#define GL_MAX_VIEWPORT_DIMS              0x0D3A
#define GL_SUBPIXEL_BITS                  0x0D50
#define GL_TEXTURE_1D                     0x0DE0
#define GL_TEXTURE_2D                     0x0DE1
#define GL_TEXTURE_WIDTH                  0x1000
#define GL_TEXTURE_HEIGHT                 0x1001
#define GL_TEXTURE_BORDER_COLOR           0x1004
#define GL_DONT_CARE                      0x1100
#define GL_FASTEST                        0x1101
#define GL_NICEST                         0x1102
#define GL_BYTE                           0x1400
#define GL_UNSIGNED_BYTE                  0x1401
#define GL_SHORT                          0x1402
#define GL_UNSIGNED_SHORT                 0x1403
#define GL_INT                            0x1404
#define GL_UNSIGNED_INT                   0x1405
#define GL_FLOAT                          0x1406
#define GL_STACK_OVERFLOW                 0x0503
#define GL_STACK_UNDERFLOW                0x0504
#define GL_CLEAR                          0x1500
#define GL_AND                            0x1501
#define GL_AND_REVERSE                    0x1502
#define GL_COPY                           0x1503
#define GL_AND_INVERTED                   0x1504
#define GL_NOOP                           0x1505
#define GL_XOR                            0x1506
#define GL_OR                             0x1507
#define GL_NOR                            0x1508
#define GL_EQUIV                          0x1509
#define GL_INVERT                         0x150A
#define GL_OR_REVERSE                     0x150B
#define GL_COPY_INVERTED                  0x150C
#define GL_OR_INVERTED                    0x150D
#define GL_NAND                           0x150E
#define GL_SET                            0x150F
#define GL_TEXTURE                        0x1702
#define GL_COLOR                          0x1800
#define GL_DEPTH                          0x1801
#define GL_STENCIL                        0x1802
#define GL_STENCIL_INDEX                  0x1901
#define GL_DEPTH_COMPONENT                0x1902
#define GL_RED                            0x1903
#define GL_GREEN                          0x1904
#define GL_BLUE                           0x1905
#define GL_ALPHA                          0x1906
#define GL_RGB                            0x1907
#define GL_RGBA                           0x1908
#define GL_POINT                          0x1B00
#define GL_LINE                           0x1B01
#define GL_FILL                           0x1B02
#define GL_KEEP                           0x1E00
#define GL_REPLACE                        0x1E01
#define GL_INCR                           0x1E02
#define GL_DECR                           0x1E03
#define GL_VENDOR                         0x1F00
#define GL_RENDERER                       0x1F01
#define GL_VERSION                        0x1F02
#define GL_EXTENSIONS                     0x1F03
#define GL_NEAREST                        0x2600
#define GL_LINEAR                         0x2601
#define GL_NEAREST_MIPMAP_NEAREST         0x2700
#define GL_LINEAR_MIPMAP_NEAREST          0x2701
#define GL_NEAREST_MIPMAP_LINEAR          0x2702
#define GL_LINEAR_MIPMAP_LINEAR           0x2703
#define GL_TEXTURE_MAG_FILTER             0x2800
#define GL_TEXTURE_MIN_FILTER             0x2801
#define GL_TEXTURE_WRAP_S                 0x2802
#define GL_TEXTURE_WRAP_T                 0x2803
#define GL_REPEAT                         0x2901
typedef void (APIENTRYP PFNGLCULLFACEPROC) (GLenum mode);
typedef void (APIENTRYP PFNGLFRONTFACEPROC) (GLenum mode);
typedef void (APIENTRYP PFNGLHINTPROC) (GLenum target, GLenum mode);
typedef void (APIENTRYP PFNGLLINEWIDTHPROC) (GLfloat width);
typedef void (APIENTRYP PFNGLPOINTSIZEPROC) (GLfloat size);
typedef void (APIENTRYP PFNGLPOLYGONMODEPROC) (GLenum face, GLenum mode);
typedef void (APIENTRYP PFNGLSCISSORPROC) (GLint x, GLint y, GLsizei width, GLsizei height);
typedef void (APIENTRYP PFNGLTEXPARAMETERFPROC) (GLenum target, GLenum pname, GLfloat param);
typedef void (APIENTRYP PFNGLTEXPARAMETERFVPROC) (GLenum target, GLenum pname, const GLfloat *params);
typedef void (APIENTRYP PFNGLTEXPARAMETERIPROC) (GLenum target, GLenum pname, GLint param);
typedef void (APIENTRYP PFNGLTEXPARAMETERIVPROC) (GLenum target, GLenum pname, const GLint *params);
typedef void (APIENTRYP PFNGLTEXIMAGE1DPROC) (GLenum target, GLint level, GLint internalformat, GLsizei width, GLint border, GLenum format, GLenum type, const void *pixels);
typedef void (APIENTRYP PFNGLTEXIMAGE2DPROC) (GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const void *pixels);
typedef void (APIENTRYP PFNGLDRAWBUFFERPROC) (GLenum buf);
typedef void (APIENTRYP PFNGLCLEARPROC) (GLbitfield mask);
typedef void (APIENTRYP PFNGLCLEARCOLORPROC) (GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
typedef void (APIENTRYP PFNGLCLEARSTENCILPROC) (GLint s);
typedef void (APIENTRYP PFNGLCLEARDEPTHPROC) (GLdouble depth);
typedef void (APIENTRYP PFNGLSTENCILMASKPROC) (GLuint mask);
typedef void (APIENTRYP PFNGLCOLORMASKPROC) (GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha);
typedef void (APIENTRYP PFNGLDEPTHMASKPROC) (GLboolean flag);
typedef void (APIENTRYP PFNGLDISABLEPROC) (GLenum cap);
typedef void (APIENTRYP PFNGLENABLEPROC) (GLenum cap);
typedef void (APIENTRYP PFNGLFINISHPROC) (void);
typedef void (APIENTRYP PFNGLFLUSHPROC) (void);
typedef void (APIENTRYP PFNGLBLENDFUNCPROC) (GLenum sfactor, GLenum dfactor);
typedef void (APIENTRYP PFNGLLOGICOPPROC) (GLenum opcode);
typedef void (APIENTRYP PFNGLSTENCILFUNCPROC) (GLenum func, GLint ref, GLuint mask);
typedef void (APIENTRYP PFNGLSTENCILOPPROC) (GLenum fail, GLenum zfail, GLenum zpass);
typedef void (APIENTRYP PFNGLDEPTHFUNCPROC) (GLenum func);
typedef void (APIENTRYP PFNGLPIXELSTOREFPROC) (GLenum pname, GLfloat param);
typedef void (APIENTRYP PFNGLPIXELSTOREIPROC) (GLenum pname, GLint param);
typedef void (APIENTRYP PFNGLREADBUFFERPROC) (GLenum src);
typedef void (APIENTRYP PFNGLREADPIXELSPROC) (GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, void *pixels);
typedef void (APIENTRYP PFNGLGETBOOLEANVPROC) (GLenum pname, GLboolean *data);
typedef void (APIENTRYP PFNGLGETDOUBLEVPROC) (GLenum pname, GLdouble *data);
typedef GLenum (APIENTRYP PFNGLGETERRORPROC) (void);
typedef void (APIENTRYP PFNGLGETFLOATVPROC) (GLenum pname, GLfloat *data);
typedef void (APIENTRYP PFNGLGETINTEGERVPROC) (GLenum pname, GLint *data);
typedef const GLubyte *(APIENTRYP PFNGLGETSTRINGPROC) (GLenum name);
typedef void (APIENTRYP PFNGLGETTEXIMAGEPROC) (GLenum target, GLint level, GLenum format, GLenum type, void *pixels);
typedef void (APIENTRYP PFNGLGETTEXPARAMETERFVPROC) (GLenum target, GLenum pname, GLfloat *params);
typedef void (APIENTRYP PFNGLGETTEXPARAMETERIVPROC) (GLenum target, GLenum pname, GLint *params);
typedef void (APIENTRYP PFNGLGETTEXLEVELPARAMETERFVPROC) (GLenum target, GLint level, GLenum pname, GLfloat *params);
typedef void (APIENTRYP PFNGLGETTEXLEVELPARAMETERIVPROC) (GLenum target, GLint level, GLenum pname, GLint *params);
typedef GLboolean (APIENTRYP PFNGLISENABLEDPROC) (GLenum cap);
typedef void (APIENTRYP PFNGLDEPTHRANGEPROC) (GLdouble n, GLdouble f);
typedef void (APIENTRYP PFNGLVIEWPORTPROC) (GLint x, GLint y, GLsizei width, GLsizei height);
#endif /* GL_VERSION_1_0 */
#ifndef GL_VERSION_1_1
#define GL_VERSION_1_1 1
typedef float GLclampf;
typedef double GLclampd;
#define GL_COLOR_LOGIC_OP                 0x0BF2
#define GL_POLYGON_OFFSET_UNITS           0x2A00
#define GL_POLYGON_OFFSET_POINT           0x2A01
#define GL_POLYGON_OFFSET_LINE            0x2A02
#define GL_POLYGON_OFFSET_FILL            0x8037
#define GL_POLYGON_OFFSET_FACTOR          0x8038
#define GL_TEXTURE_BINDING_1D             0x8068
#define GL_TEXTURE_BINDING_2D             0x8069
#define GL_TEXTURE_INTERNAL_FORMAT        0x1003
#define GL_TEXTURE_RED_SIZE               0x805C
#define GL_TEXTURE_GREEN_SIZE             0x805D
#define GL_TEXTURE_BLUE_SIZE              0x805E
#define GL_TEXTURE_ALPHA_SIZE             0x805F
#define GL_DOUBLE                         0x140A
#define GL_PROXY_TEXTURE_1D               0x8063
#define GL_PROXY_TEXTURE_2D               0x8064
#define GL_R3_G3_B2                       0x2A10
#define GL_RGB4                           0x804F
#define GL_RGB5                           0x8050
#define GL_RGB8                           0x8051
#define GL_RGB10                          0x8052
#define GL_RGB12                          0x8053
#define GL_RGB16                          0x8054
#define GL_RGBA2                          0x8055
#define GL_RGBA4                          0x8056
#define GL_RGB5_A1                        0x8057
#define GL_RGBA8                          0x8058
#define GL_RGB10_A2                       0x8059
#define GL_RGBA12                         0x805A
#define GL_RGBA16                         0x805B
#define GL_VERTEX_ARRAY                   0x8074
typedef void (APIENTRYP PFNGLDRAWARRAYSPROC) (GLenum mode, GLint first, GLsizei count);
typedef void (APIENTRYP PFNGLDRAWELEMENTSPROC) (GLenum mode, GLsizei count, GLenum type, const void *indices);
typedef void (APIENTRYP PFNGLGETPOINTERVPROC) (GLenum pname, void **params);
typedef void (APIENTRYP PFNGLPOLYGONOFFSETPROC) (GLfloat factor, GLfloat units);
typedef void (APIENTRYP PFNGLCOPYTEXIMAGE1DPROC) (GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLint border);
typedef void (APIENTRYP PFNGLCOPYTEXIMAGE2DPROC) (GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border);
typedef void (APIENTRYP PFNGLCOPYTEXSUBIMAGE1DPROC) (GLenum target, GLint level, GLint xoffset, GLint x, GLint y, GLsizei width);
typedef void (APIENTRYP PFNGLCOPYTEXSUBIMAGE2DPROC) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height);
typedef void (APIENTRYP PFNGLTEXSUBIMAGE1DPROC) (GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLenum type, const void *pixels);
typedef void (APIENTRYP PFNGLTEXSUBIMAGE2DPROC) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const void *pixels);
typedef void (APIENTRYP PFNGLBINDTEXTUREPROC) (GLenum target, GLuint texture);
typedef void (APIENTRYP PFNGLDELETETEXTURESPROC) (GLsizei n, const GLuint *textures);
typedef void (APIENTRYP PFNGLGENTEXTURESPROC) (GLsizei n, GLuint *textures);
typedef GLboolean (APIENTRYP PFNGLISTEXTUREPROC) (GLuint texture);
#endif /* GL_VERSION_1_1 */
#ifndef GL_VERSION_1_2
#define GL_VERSION_1_2 1
#define GL_UNSIGNED_BYTE_3_3_2            0x8032
#define GL_UNSIGNED_SHORT_4_4_4_4         0x8033
#define GL_UNSIGNED_SHORT_5_5_5_1         0x8034
#define GL_UNSIGNED_INT_8_8_8_8           0x8035
#define GL_UNSIGNED_INT_10_10_10_2        0x8036
#define GL_TEXTURE_BINDING_3D             0x806A
#define GL_PACK_SKIP_IMAGES               0x806B
#define GL_PACK_IMAGE_HEIGHT              0x806C
#define GL_UNPACK_SKIP_IMAGES             0x806D
#define GL_UNPACK_IMAGE_HEIGHT            0x806E
#define GL_TEXTURE_3D                     0x806F
#define GL_PROXY_TEXTURE_3D               0x8070
#define GL_TEXTURE_DEPTH                  0x8071
#define GL_TEXTURE_WRAP_R                 0x8072
#define GL_MAX_3D_TEXTURE_SIZE            0x8073
#define GL_UNSIGNED_BYTE_2_3_3_REV        0x8362
#define GL_UNSIGNED_SHORT_5_6_5           0x8363
#define GL_UNSIGNED_SHORT_5_6_5_REV       0x8364
#define GL_UNSIGNED_SHORT_4_4_4_4_REV     0x8365
#define GL_UNSIGNED_SHORT_1_5_5_5_REV     0x8366
#define GL_UNSIGNED_INT_8_8_8_8_REV       0x8367
#define GL_UNSIGNED_INT_2_10_10_10_REV    0x8368
#define GL_BGR                            0x80E0
#define GL_BGRA                           0x80E1
#define GL_MAX_ELEMENTS_VERTICES          0x80E8
#define GL_MAX_ELEMENTS_INDICES           0x80E9
#define GL_CLAMP_TO_EDGE                  0x812F
#define GL_TEXTURE_MIN_LOD                0x813A
#define GL_TEXTURE_MAX_LOD                0x813B
#define GL_TEXTURE_BASE_LEVEL             0x813C
#define GL_TEXTURE_MAX_LEVEL              0x813D
#define GL_SMOOTH_POINT_SIZE_RANGE        0x0B12
#define GL_SMOOTH_POINT_SIZE_GRANULARITY  0x0B13
#define GL_SMOOTH_LINE_WIDTH_RANGE        0x0B22
#define GL_SMOOTH_LINE_WIDTH_GRANULARITY  0x0B23
#define GL_ALIASED_LINE_WIDTH_RANGE       0x846E
typedef void (APIENTRYP PFNGLDRAWRANGEELEMENTSPROC) (GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const void *indices);
typedef void (APIENTRYP PFNGLTEXIMAGE3DPROC) (GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const void *pixels);
typedef void (APIENTRYP PFNGLTEXSUBIMAGE3DPROC) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const void *pixels);
typedef void (APIENTRYP PFNGLCOPYTEXSUBIMAGE3DPROC) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height);
#endif /* GL_VERSION_1_2 */
#ifndef GL_VERSION_1_3
#define GL_VERSION_1_3 1
#define GL_TEXTURE0                       0x84C0
#define GL_TEXTURE1                       0x84C1
#define GL_TEXTURE2                       0x84C2
#define GL_TEXTURE3                       0x84C3
#define GL_TEXTURE4                       0x84C4
#define GL_TEXTURE5                       0x84C5
#define GL_TEXTURE6                       0x84C6
#define GL_TEXTURE7                       0x84C7
#define GL_TEXTURE8                       0x84C8
#define GL_TEXTURE9                       0x84C9
#define GL_TEXTURE10                      0x84CA
#define GL_TEXTURE11                      0x84CB
#define GL_TEXTURE12                      0x84CC
#define GL_TEXTURE13                      0x84CD
#define GL_TEXTURE14                      0x84CE
#define GL_TEXTURE15                      0x84CF
#define GL_TEXTURE16                      0x84D0
#define GL_TEXTURE17                      0x84D1
#define GL_TEXTURE18                      0x84D2
#define GL_TEXTURE19                      0x84D3
#define GL_TEXTURE20                      0x84D4
#define GL_TEXTURE21                      0x84D5
#define GL_TEXTURE22                      0x84D6
#define GL_TEXTURE23                      0x84D7
#define GL_TEXTURE24                      0x84D8
#define GL_TEXTURE25                      0x84D9
#define GL_TEXTURE26                      0x84DA
#define GL_TEXTURE27                      0x84DB
#define GL_TEXTURE28                      0x84DC
#define GL_TEXTURE29                      0x84DD
#define GL_TEXTURE30                      0x84DE
#define GL_TEXTURE31                      0x84DF
#define GL_ACTIVE_TEXTURE                 0x84E0
#define GL_MULTISAMPLE                    0x809D
#define GL_SAMPLE_ALPHA_TO_COVERAGE       0x809E
#define GL_SAMPLE_ALPHA_TO_ONE            0x809F
#define GL_SAMPLE_COVERAGE                0x80A0
#define GL_SAMPLE_BUFFERS                 0x80A8
#define GL_SAMPLES                        0x80A9
#define GL_SAMPLE_COVERAGE_VALUE          0x80AA
#define GL_SAMPLE_COVERAGE_INVERT         0x80AB
#define GL_TEXTURE_CUBE_MAP               0x8513
#define GL_TEXTURE_BINDING_CUBE_MAP       0x8514
#define GL_TEXTURE_CUBE_MAP_POSITIVE_X    0x8515
#define GL_TEXTURE_CUBE_MAP_NEGATIVE_X    0x8516
#define GL_TEXTURE_CUBE_MAP_POSITIVE_Y    0x8517
#define GL_TEXTURE_CUBE_MAP_NEGATIVE_Y    0x8518
#define GL_TEXTURE_CUBE_MAP_POSITIVE_Z    0x8519
#define GL_TEXTURE_CUBE_MAP_NEGATIVE_Z    0x851A
#define GL_PROXY_TEXTURE_CUBE_MAP         0x851B
#define GL_MAX_CUBE_MAP_TEXTURE_SIZE      0x851C
#define GL_COMPRESSED_RGB                 0x84ED
#define GL_COMPRESSED_RGBA                0x84EE
#define GL_TEXTURE_COMPRESSION_HINT       0x84EF
#define GL_TEXTURE_COMPRESSED_IMAGE_SIZE  0x86A0
#define GL_TEXTURE_COMPRESSED             0x86A1
#define GL_NUM_COMPRESSED_TEXTURE_FORMATS 0x86A2
#define GL_COMPRESSED_TEXTURE_FORMATS     0x86A3
#define GL_CLAMP_TO_BORDER                0x812D
typedef void (APIENTRYP PFNGLACTIVETEXTUREPROC) (GLenum texture);
typedef void (APIENTRYP PFNGLSAMPLECOVERAGEPROC) (GLfloat value, GLboolean invert);
typedef void (APIENTRYP PFNGLCOMPRESSEDTEXIMAGE3DPROC) (GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLsizei imageSize, const void *data);
typedef void (APIENTRYP PFNGLCOMPRESSEDTEXIMAGE2DPROC) (GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const void *data);
typedef void (APIENTRYP PFNGLCOMPRESSEDTEXIMAGE1DPROC) (GLenum target, GLint level, GLenum internalformat, GLsizei width, GLint border, GLsizei imageSize, const void *data);
typedef void (APIENTRYP PFNGLCOMPRESSEDTEXSUBIMAGE3DPROC) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const void *data);
typedef void (APIENTRYP PFNGLCOMPRESSEDTEXSUBIMAGE2DPROC) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const void *data);
typedef void (APIENTRYP PFNGLCOMPRESSEDTEXSUBIMAGE1DPROC) (GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLsizei imageSize, const void *data);
typedef void (APIENTRYP PFNGLGETCOMPRESSEDTEXIMAGEPROC) (GLenum target, GLint level, void *img);
#endif /* GL_VERSION_1_3 */
#ifndef GL_VERSION_1_4
#define GL_VERSION_1_4 1
#define GL_BLEND_DST_RGB                  0x80C8
#define GL_BLEND_SRC_RGB                  0x80C9
#define GL_BLEND_DST_ALPHA                0x80CA
#define GL_BLEND_SRC_ALPHA                0x80CB
#define GL_POINT_FADE_THRESHOLD_SIZE      0x8128
#define GL_DEPTH_COMPONENT16              0x81A5
#define GL_DEPTH_COMPONENT24              0x81A6
#define GL_DEPTH_COMPONENT32              0x81A7
#define GL_MIRRORED_REPEAT                0x8370
#define GL_MAX_TEXTURE_LOD_BIAS           0x84FD
#define GL_TEXTURE_LOD_BIAS               0x8501
#define GL_INCR_WRAP                      0x8507
#define GL_DECR_WRAP                      0x8508
#define GL_TEXTURE_DEPTH_SIZE             0x884A
#define GL_TEXTURE_COMPARE_MODE           0x884C
#define GL_TEXTURE_COMPARE_FUNC           0x884D
#define GL_BLEND_COLOR                    0x8005
#define GL_BLEND_EQUATION                 0x8009
#define GL_CONSTANT_COLOR                 0x8001
#define GL_ONE_MINUS_CONSTANT_COLOR       0x8002
#define GL_CONSTANT_ALPHA                 0x8003
#define GL_ONE_MINUS_CONSTANT_ALPHA       0x8004
#define GL_FUNC_ADD                       0x8006
#define GL_FUNC_REVERSE_SUBTRACT          0x800B
#define GL_FUNC_SUBTRACT                  0x800A
#define GL_MIN                            0x8007
#define GL_MAX                            0x8008
typedef void (APIENTRYP PFNGLBLENDFUNCSEPARATEPROC) (GLenum sfactorRGB, GLenum dfactorRGB, GLenum sfactorAlpha, GLenum dfactorAlpha);
typedef void (APIENTRYP PFNGLMULTIDRAWARRAYSPROC) (GLenum mode, const GLint *first, const GLsizei *count, GLsizei drawcount);
typedef void (APIENTRYP PFNGLMULTIDRAWELEMENTSPROC) (GLenum mode, const GLsizei *count, GLenum type, const void *const*indices, GLsizei drawcount);
typedef void (APIENTRYP PFNGLPOINTPARAMETERFPROC) (GLenum pname, GLfloat param);
typedef void (APIENTRYP PFNGLPOINTPARAMETERFVPROC) (GLenum pname, const GLfloat *params);
typedef void (APIENTRYP PFNGLPOINTPARAMETERIPROC) (GLenum pname, GLint param);
typedef void (APIENTRYP PFNGLPOINTPARAMETERIVPROC) (GLenum pname, const GLint *params);
typedef void (APIENTRYP PFNGLBLENDCOLORPROC) (GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
typedef void (APIENTRYP PFNGLBLENDEQUATIONPROC) (GLenum mode);
#endif /* GL_VERSION_1_4 */
#ifndef GL_VERSION_1_5
#define GL_VERSION_1_5 1
typedef khronos_ssize_t GLsizeiptr;
typedef khronos_intptr_t GLintptr;
#define GL_BUFFER_SIZE                    0x8764
#define GL_BUFFER_USAGE                   0x8765
#define GL_QUERY_COUNTER_BITS             0x8864
#define GL_CURRENT_QUERY                  0x8865
#define GL_QUERY_RESULT                   0x8866
#define GL_QUERY_RESULT_AVAILABLE         0x8867
#define GL_ARRAY_BUFFER                   0x8892
#define GL_ELEMENT_ARRAY_BUFFER           0x8893
#define GL_ARRAY_BUFFER_BINDING           0x8894
#define GL_ELEMENT_ARRAY_BUFFER_BINDING   0x8895
#define GL_VERTEX_ATTRIB_ARRAY_BUFFER_BINDING 0x889F
#define GL_READ_ONLY                      0x88B8
#define GL_WRITE_ONLY                     0x88B9
#define GL_READ_WRITE                     0x88BA
#define GL_BUFFER_ACCESS                  0x88BB
#define GL_BUFFER_MAPPED                  0x88BC
#define GL_BUFFER_MAP_POINTER             0x88BD
#define GL_STREAM_DRAW                    0x88E0
#define GL_STREAM_READ                    0x88E1
#define GL_STREAM_COPY                    0x88E2
#define GL_STATIC_DRAW                    0x88E4
#define GL_STATIC_READ                    0x88E5
#define GL_STATIC_COPY                    0x88E6
#define GL_DYNAMIC_DRAW                   0x88E8
#define GL_DYNAMIC_READ                   0x88E9
#define GL_DYNAMIC_COPY                   0x88EA
#define GL_SAMPLES_PASSED                 0x8914
#define GL_SRC1_ALPHA                     0x8589
typedef void (APIENTRYP PFNGLGENQUERIESPROC) (GLsizei n, GLuint *ids);
typedef void (APIENTRYP PFNGLDELETEQUERIESPROC) (GLsizei n, const GLuint *ids);
typedef GLboolean (APIENTRYP PFNGLISQUERYPROC) (GLuint id);
typedef void (APIENTRYP PFNGLBEGINQUERYPROC) (GLenum target, GLuint id);
typedef void (APIENTRYP PFNGLENDQUERYPROC) (GLenum target);
typedef void (APIENTRYP PFNGLGETQUERYIVPROC) (GLenum target, GLenum pname, GLint *params);
typedef void (APIENTRYP PFNGLGETQUERYOBJECTIVPROC) (GLuint id, GLenum pname, GLint *params);
typedef void (APIENTRYP PFNGLGETQUERYOBJECTUIVPROC) (GLuint id, GLenum pname, GLuint *params);
typedef void (APIENTRYP PFNGLBINDBUFFERPROC) (GLenum target, GLuint buffer);
typedef void (APIENTRYP PFNGLDELETEBUFFERSPROC) (GLsizei n, const GLuint *buffers);
typedef void (APIENTRYP PFNGLGENBUFFERSPROC) (GLsizei n, GLuint *buffers);
typedef GLboolean (APIENTRYP PFNGLISBUFFERPROC) (GLuint buffer);
typedef void (APIENTRYP PFNGLBUFFERDATAPROC) (GLenum target, GLsizeiptr size, const void *data, GLenum usage);
typedef void (APIENTRYP PFNGLBUFFERSUBDATAPROC) (GLenum target, GLintptr offset, GLsizeiptr size, const void *data);
typedef void (APIENTRYP PFNGLGETBUFFERSUBDATAPROC) (GLenum target, GLintptr offset, GLsizeiptr size, void *data);
typedef void *(APIENTRYP PFNGLMAPBUFFERPROC) (GLenum target, GLenum access);
typedef GLboolean (APIENTRYP PFNGLUNMAPBUFFERPROC) (GLenum target);
typedef void (APIENTRYP PFNGLGETBUFFERPARAMETERIVPROC) (GLenum target, GLenum pname, GLint *params);
typedef void (APIENTRYP PFNGLGETBUFFERPOINTERVPROC) (GLenum target, GLenum pname, void **params);
#endif /* GL_VERSION_1_5 */
#ifndef GL_VERSION_2_0
#define GL_VERSION_2_0 1
typedef char GLchar;
typedef int16_t GLshort;
typedef int8_t GLbyte;
typedef uint16_t GLushort;
#define GL_BLEND_EQUATION_RGB             0x8009
#define GL_VERTEX_ATTRIB_ARRAY_ENABLED    0x8622
#define GL_VERTEX_ATTRIB_ARRAY_SIZE       0x8623
#define GL_VERTEX_ATTRIB_ARRAY_STRIDE     0x8624
#define GL_VERTEX_ATTRIB_ARRAY_TYPE       0x8625
#define GL_CURRENT_VERTEX_ATTRIB          0x8626
#define GL_VERTEX_PROGRAM_POINT_SIZE      0x8642
#define GL_VERTEX_ATTRIB_ARRAY_POINTER    0x8645
#define GL_STENCIL_BACK_FUNC              0x8800
#define GL_STENCIL_BACK_FAIL              0x8801
#define GL_STENCIL_BACK_PASS_DEPTH_FAIL   0x8802
#define GL_STENCIL_BACK_PASS_DEPTH_PASS   0x8803
#define GL_MAX_DRAW_BUFFERS               0x8824
#define GL_DRAW_BUFFER0                   0x8825
#define GL_DRAW_BUFFER1                   0x8826
#define GL_DRAW_BUFFER2                   0x8827
#define GL_DRAW_BUFFER3                   0x8828
#define GL_DRAW_BUFFER4                   0x8829
#define GL_DRAW_BUFFER5                   0x882A
#define GL_DRAW_BUFFER6                   0x882B
#define GL_DRAW_BUFFER7                   0x882C
#define GL_DRAW_BUFFER8                   0x882D
#define GL_DRAW_BUFFER9                   0x882E
#define GL_DRAW_BUFFER10                  0x882F
#define GL_DRAW_BUFFER11                  0x8830
#define GL_DRAW_BUFFER12                  0x8831
#define GL_DRAW_BUFFER13                  0x8832
#define GL_DRAW_BUFFER14                  0x8833
#define GL_DRAW_BUFFER15                  0x8834
#define GL_BLEND_EQUATION_ALPHA           0x883D
#define GL_MAX_VERTEX_ATTRIBS             0x8869
#define GL_VERTEX_ATTRIB_ARRAY_NORMALIZED 0x886A
#define GL_MAX_TEXTURE_IMAGE_UNITS        0x8872
#define GL_FRAGMENT_SHADER                0x8B30
#define GL_VERTEX_SHADER                  0x8B31
#define GL_MAX_FRAGMENT_UNIFORM_COMPONENTS 0x8B49
#define GL_MAX_VERTEX_UNIFORM_COMPONENTS  0x8B4A
#define GL_MAX_VARYING_FLOATS             0x8B4B
#define GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS 0x8B4C
#define GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS 0x8B4D
#define GL_SHADER_TYPE                    0x8B4F
#define GL_FLOAT_VEC2                     0x8B50
#define GL_FLOAT_VEC3                     0x8B51
#define GL_FLOAT_VEC4                     0x8B52
#define GL_INT_VEC2                       0x8B53
#define GL_INT_VEC3                       0x8B54
#define GL_INT_VEC4                       0x8B55
#define GL_BOOL                           0x8B56
#define GL_BOOL_VEC2                      0x8B57
#define GL_BOOL_VEC3                      0x8B58
#define GL_BOOL_VEC4                      0x8B59
#define GL_FLOAT_MAT2                     0x8B5A
#define GL_FLOAT_MAT3                     0x8B5B
#define GL_FLOAT_MAT4                     0x8B5C
#define GL_SAMPLER_1D                     0x8B5D
#define GL_SAMPLER_2D                     0x8B5E
#define GL_SAMPLER_3D                     0x8B5F
#define GL_SAMPLER_CUBE                   0x8B60
#define GL_SAMPLER_1D_SHADOW              0x8B61
#define GL_SAMPLER_2D_SHADOW              0x8B62
#define GL_DELETE_STATUS                  0x8B80
#define GL_COMPILE_STATUS                 0x8B81
#define GL_LINK_STATUS                    0x8B82
#define GL_VALIDATE_STATUS                0x8B83
#define GL_INFO_LOG_LENGTH                0x8B84
#define GL_ATTACHED_SHADERS               0x8B85
#define GL_ACTIVE_UNIFORMS                0x8B86
#define GL_ACTIVE_UNIFORM_MAX_LENGTH      0x8B87
#define GL_SHADER_SOURCE_LENGTH           0x8B88
#define GL_ACTIVE_ATTRIBUTES              0x8B89
#define GL_ACTIVE_ATTRIBUTE_MAX_LENGTH    0x8B8A
#define GL_FRAGMENT_SHADER_DERIVATIVE_HINT 0x8B8B
#define GL_SHADING_LANGUAGE_VERSION       0x8B8C
#define GL_CURRENT_PROGRAM                0x8B8D
#define GL_POINT_SPRITE_COORD_ORIGIN      0x8CA0
#define GL_LOWER_LEFT                     0x8CA1
#define GL_UPPER_LEFT                     0x8CA2
#define GL_STENCIL_BACK_REF               0x8CA3
#define GL_STENCIL_BACK_VALUE_MASK        0x8CA4
#define GL_STENCIL_BACK_WRITEMASK         0x8CA5
typedef void (APIENTRYP PFNGLBLENDEQUATIONSEPARATEPROC) (GLenum modeRGB, GLenum modeAlpha);
typedef void (APIENTRYP PFNGLDRAWBUFFERSPROC) (GLsizei n, const GLenum *bufs);
typedef void (APIENTRYP PFNGLSTENCILOPSEPARATEPROC) (GLenum face, GLenum sfail, GLenum dpfail, GLenum dppass);
typedef void (APIENTRYP PFNGLSTENCILFUNCSEPARATEPROC) (GLenum face, GLenum func, GLint ref, GLuint mask);
typedef void (APIENTRYP PFNGLSTENCILMASKSEPARATEPROC) (GLenum face, GLuint mask);
typedef void (APIENTRYP PFNGLATTACHSHADERPROC) (GLuint program, GLuint shader);
typedef void (APIENTRYP PFNGLBINDATTRIBLOCATIONPROC) (GLuint program, GLuint index, const GLchar *name);
typedef void (APIENTRYP PFNGLCOMPILESHADERPROC) (GLuint shader);
typedef GLuint (APIENTRYP PFNGLCREATEPROGRAMPROC) (void);
typedef GLuint (APIENTRYP PFNGLCREATESHADERPROC) (GLenum type);
typedef void (APIENTRYP PFNGLDELETEPROGRAMPROC) (GLuint program);
typedef void (APIENTRYP PFNGLDELETESHADERPROC) (GLuint shader);
typedef void (APIENTRYP PFNGLDETACHSHADERPROC) (GLuint program, GLuint shader);
typedef void (APIENTRYP PFNGLDISABLEVERTEXATTRIBARRAYPROC) (GLuint index);
typedef void (APIENTRYP PFNGLENABLEVERTEXATTRIBARRAYPROC) (GLuint index);
typedef void (APIENTRYP PFNGLGETACTIVEATTRIBPROC) (GLuint program, GLuint index, GLsizei bufSize, GLsizei *length, GLint *size, GLenum *type, GLchar *name);
typedef void (APIENTRYP PFNGLGETACTIVEUNIFORMPROC) (GLuint program, GLuint index, GLsizei bufSize, GLsizei *length, GLint *size, GLenum *type, GLchar *name);
typedef void (APIENTRYP PFNGLGETATTACHEDSHADERSPROC) (GLuint program, GLsizei maxCount, GLsizei *count, GLuint *shaders);
typedef GLint (APIENTRYP PFNGLGETATTRIBLOCATIONPROC) (GLuint program, const GLchar *name);
typedef void (APIENTRYP PFNGLGETPROGRAMIVPROC) (GLuint program, GLenum pname, GLint *params);
typedef void (APIENTRYP PFNGLGETPROGRAMINFOLOGPROC) (GLuint program, GLsizei bufSize, GLsizei *length, GLchar *infoLog);
typedef void (APIENTRYP PFNGLGETSHADERIVPROC) (GLuint shader, GLenum pname, GLint *params);
typedef void (APIENTRYP PFNGLGETSHADERINFOLOGPROC) (GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *infoLog);
typedef void (APIENTRYP PFNGLGETSHADERSOURCEPROC) (GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *source);
typedef GLint (APIENTRYP PFNGLGETUNIFORMLOCATIONPROC) (GLuint program, const GLchar *name);
typedef void (APIENTRYP PFNGLGETUNIFORMFVPROC) (GLuint program, GLint location, GLfloat *params);
typedef void (APIENTRYP PFNGLGETUNIFORMIVPROC) (GLuint program, GLint location, GLint *params);
typedef void (APIENTRYP PFNGLGETVERTEXATTRIBDVPROC) (GLuint index, GLenum pname, GLdouble *params);
typedef void (APIENTRYP PFNGLGETVERTEXATTRIBFVPROC) (GLuint index, GLenum pname, GLfloat *params);
typedef void (APIENTRYP PFNGLGETVERTEXATTRIBIVPROC) (GLuint index, GLenum pname, GLint *params);
typedef void (APIENTRYP PFNGLGETVERTEXATTRIBPOINTERVPROC) (GLuint index, GLenum pname, void **pointer);
typedef GLboolean (APIENTRYP PFNGLISPROGRAMPROC) (GLuint program);
typedef GLboolean (APIENTRYP PFNGLISSHADERPROC) (GLuint shader);
typedef void (APIENTRYP PFNGLLINKPROGRAMPROC) (GLuint program);
typedef void (APIENTRYP PFNGLSHADERSOURCEPROC) (GLuint shader, GLsizei count, const GLchar *const*string, const GLint *length);
typedef void (APIENTRYP PFNGLUSEPROGRAMPROC) (GLuint program);
typedef void (APIENTRYP PFNGLUNIFORM1FPROC) (GLint location, GLfloat v0);
typedef void (APIENTRYP PFNGLUNIFORM2FPROC) (GLint location, GLfloat v0, GLfloat v1);
typedef void (APIENTRYP PFNGLUNIFORM3FPROC) (GLint location, GLfloat v0, GLfloat v1, GLfloat v2);
typedef void (APIENTRYP PFNGLUNIFORM4FPROC) (GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3);
typedef void (APIENTRYP PFNGLUNIFORM1IPROC) (GLint location, GLint v0);
typedef void (APIENTRYP PFNGLUNIFORM2IPROC) (GLint location, GLint v0, GLint v1);
typedef void (APIENTRYP PFNGLUNIFORM3IPROC) (GLint location, GLint v0, GLint v1, GLint v2);
typedef void (APIENTRYP PFNGLUNIFORM4IPROC) (GLint location, GLint v0, GLint v1, GLint v2, GLint v3);
typedef void (APIENTRYP PFNGLUNIFORM1FVPROC) (GLint location, GLsizei count, const GLfloat *value);
typedef void (APIENTRYP PFNGLUNIFORM2FVPROC) (GLint location, GLsizei count, const GLfloat *value);
typedef void (APIENTRYP PFNGLUNIFORM3FVPROC) (GLint location, GLsizei count, const GLfloat *value);
typedef void (APIENTRYP PFNGLUNIFORM4FVPROC) (GLint location, GLsizei count, const GLfloat *value);
typedef void (APIENTRYP PFNGLUNIFORM1IVPROC) (GLint location, GLsizei count, const GLint *value);
typedef void (APIENTRYP PFNGLUNIFORM2IVPROC) (GLint location, GLsizei count, const GLint *value);
typedef void (APIENTRYP PFNGLUNIFORM3IVPROC) (GLint location, GLsizei count, const GLint *value);
typedef void (APIENTRYP PFNGLUNIFORM4IVPROC) (GLint location, GLsizei count, const GLint *value);
typedef void (APIENTRYP PFNGLUNIFORMMATRIX2FVPROC) (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
typedef void (APIENTRYP PFNGLUNIFORMMATRIX3FVPROC) (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
typedef void (APIENTRYP PFNGLUNIFORMMATRIX4FVPROC) (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
typedef void (APIENTRYP PFNGLVALIDATEPROGRAMPROC) (GLuint program);
typedef void (APIENTRYP PFNGLVERTEXATTRIB1DPROC) (GLuint index, GLdouble x);
typedef void (APIENTRYP PFNGLVERTEXATTRIB1DVPROC) (GLuint index, const GLdouble *v);
typedef void (APIENTRYP PFNGLVERTEXATTRIB1FPROC) (GLuint index, GLfloat x);
typedef void (APIENTRYP PFNGLVERTEXATTRIB1FVPROC) (GLuint index, const GLfloat *v);
typedef void (APIENTRYP PFNGLVERTEXATTRIB1SPROC) (GLuint index, GLshort x);
typedef void (APIENTRYP PFNGLVERTEXATTRIB1SVPROC) (GLuint index, const GLshort *v);
typedef void (APIENTRYP PFNGLVERTEXATTRIB2DPROC) (GLuint index, GLdouble x, GLdouble y);
typedef void (APIENTRYP PFNGLVERTEXATTRIB2DVPROC) (GLuint index, const GLdouble *v);
typedef void (APIENTRYP PFNGLVERTEXATTRIB2FPROC) (GLuint index, GLfloat x, GLfloat y);
typedef void (APIENTRYP PFNGLVERTEXATTRIB2FVPROC) (GLuint index, const GLfloat *v);
typedef void (APIENTRYP PFNGLVERTEXATTRIB2SPROC) (GLuint index, GLshort x, GLshort y);
typedef void (APIENTRYP PFNGLVERTEXATTRIB2SVPROC) (GLuint index, const GLshort *v);
typedef void (APIENTRYP PFNGLVERTEXATTRIB3DPROC) (GLuint index, GLdouble x, GLdouble y, GLdouble z);
typedef void (APIENTRYP PFNGLVERTEXATTRIB3DVPROC) (GLuint index, const GLdouble *v);
typedef void (APIENTRYP PFNGLVERTEXATTRIB3FPROC) (GLuint index, GLfloat x, GLfloat y, GLfloat z);
typedef void (APIENTRYP PFNGLVERTEXATTRIB3FVPROC) (GLuint index, const GLfloat *v);
typedef void (APIENTRYP PFNGLVERTEXATTRIB3SPROC) (GLuint index, GLshort x, GLshort y, GLshort z);
typedef void (APIENTRYP PFNGLVERTEXATTRIB3SVPROC) (GLuint index, const GLshort *v);
typedef void (APIENTRYP PFNGLVERTEXATTRIB4NBVPROC) (GLuint index, const GLbyte *v);
typedef void (APIENTRYP PFNGLVERTEXATTRIB4NIVPROC) (GLuint index, const GLint *v);
typedef void (APIENTRYP PFNGLVERTEXATTRIB4NSVPROC) (GLuint index, const GLshort *v);
typedef void (APIENTRYP PFNGLVERTEXATTRIB4NUBPROC) (GLuint index, GLubyte x, GLubyte y, GLubyte z, GLubyte w);
typedef void (APIENTRYP PFNGLVERTEXATTRIB4NUBVPROC) (GLuint index, const GLubyte *v);
typedef void (APIENTRYP PFNGLVERTEXATTRIB4NUIVPROC) (GLuint index, const GLuint *v);
typedef void (APIENTRYP PFNGLVERTEXATTRIB4NUSVPROC) (GLuint index, const GLushort *v);
typedef void (APIENTRYP PFNGLVERTEXATTRIB4BVPROC) (GLuint index, const GLbyte *v);
typedef void (APIENTRYP PFNGLVERTEXATTRIB4DPROC) (GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w);
typedef void (APIENTRYP PFNGLVERTEXATTRIB4DVPROC) (GLuint index, const GLdouble *v);
typedef void (APIENTRYP PFNGLVERTEXATTRIB4FPROC) (GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
typedef void (APIENTRYP PFNGLVERTEXATTRIB4FVPROC) (GLuint index, const GLfloat *v);
typedef void (APIENTRYP PFNGLVERTEXATTRIB4IVPROC) (GLuint index, const GLint *v);
typedef void (APIENTRYP PFNGLVERTEXATTRIB4SPROC) (GLuint index, GLshort x, GLshort y, GLshort z, GLshort w);
typedef void (APIENTRYP PFNGLVERTEXATTRIB4SVPROC) (GLuint index, const GLshort *v);
typedef void (APIENTRYP PFNGLVERTEXATTRIB4UBVPROC) (GLuint index, const GLubyte *v);
typedef void (APIENTRYP PFNGLVERTEXATTRIB4UIVPROC) (GLuint index, const GLuint *v);
typedef void (APIENTRYP PFNGLVERTEXATTRIB4USVPROC) (GLuint index, const GLushort *v);
typedef void (APIENTRYP PFNGLVERTEXATTRIBPOINTERPROC) (GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void *pointer);
#endif /* GL_VERSION_2_0 */
#ifndef GL_VERSION_2_1
#define GL_VERSION_2_1 1
#define GL_PIXEL_PACK_BUFFER              0x88EB
#define GL_PIXEL_UNPACK_BUFFER            0x88EC
#define GL_PIXEL_PACK_BUFFER_BINDING      0x88ED
#define GL_PIXEL_UNPACK_BUFFER_BINDING    0x88EF
#define GL_FLOAT_MAT2x3                   0x8B65
#define GL_FLOAT_MAT2x4                   0x8B66
#define GL_FLOAT_MAT3x2                   0x8B67
#define GL_FLOAT_MAT3x4                   0x8B68
#define GL_FLOAT_MAT4x2                   0x8B69
#define GL_FLOAT_MAT4x3                   0x8B6A
#define GL_SRGB                           0x8C40
#define GL_SRGB8                          0x8C41
#define GL_SRGB_ALPHA                     0x8C42
#define GL_SRGB8_ALPHA8                   0x8C43
#define GL_COMPRESSED_SRGB                0x8C48
#define GL_COMPRESSED_SRGB_ALPHA          0x8C49
typedef void (APIENTRYP PFNGLUNIFORMMATRIX2X3FVPROC) (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
typedef void (APIENTRYP PFNGLUNIFORMMATRIX3X2FVPROC) (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
typedef void (APIENTRYP PFNGLUNIFORMMATRIX2X4FVPROC) (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
typedef void (APIENTRYP PFNGLUNIFORMMATRIX4X2FVPROC) (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
typedef void (APIENTRYP PFNGLUNIFORMMATRIX3X4FVPROC) (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
typedef void (APIENTRYP PFNGLUNIFORMMATRIX4X3FVPROC) (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
#endif /* GL_VERSION_2_1 */
#ifndef GL_VERSION_3_0
#define GL_VERSION_3_0 1
typedef uint16_t GLhalf;
#define GL_COMPARE_REF_TO_TEXTURE         0x884E
#define GL_CLIP_DISTANCE0                 0x3000
#define GL_CLIP_DISTANCE1                 0x3001
#define GL_CLIP_DISTANCE2                 0x3002
#define GL_CLIP_DISTANCE3                 0x3003
#define GL_CLIP_DISTANCE4                 0x3004
#define GL_CLIP_DISTANCE5                 0x3005
#define GL_CLIP_DISTANCE6                 0x3006
#define GL_CLIP_DISTANCE7                 0x3007
#define GL_MAX_CLIP_DISTANCES             0x0D32
#define GL_MAJOR_VERSION                  0x821B
#define GL_MINOR_VERSION                  0x821C
#define GL_NUM_EXTENSIONS                 0x821D
#define GL_CONTEXT_FLAGS                  0x821E
#define GL_COMPRESSED_RED                 0x8225
#define GL_COMPRESSED_RG                  0x8226
#define GL_CONTEXT_FLAG_FORWARD_COMPATIBLE_BIT 0x00000001
#define GL_RGBA32F                        0x8814
#define GL_RGB32F                         0x8815
#define GL_RGBA16F                        0x881A
#define GL_RGB16F                         0x881B
#define GL_VERTEX_ATTRIB_ARRAY_INTEGER    0x88FD
#define GL_MAX_ARRAY_TEXTURE_LAYERS       0x88FF
#define GL_MIN_PROGRAM_TEXEL_OFFSET       0x8904
#define GL_MAX_PROGRAM_TEXEL_OFFSET       0x8905
#define GL_CLAMP_READ_COLOR               0x891C
#define GL_FIXED_ONLY                     0x891D
#define GL_MAX_VARYING_COMPONENTS         0x8B4B
#define GL_TEXTURE_1D_ARRAY               0x8C18
#define GL_PROXY_TEXTURE_1D_ARRAY         0x8C19
#define GL_TEXTURE_2D_ARRAY               0x8C1A
#define GL_PROXY_TEXTURE_2D_ARRAY         0x8C1B
#define GL_TEXTURE_BINDING_1D_ARRAY       0x8C1C
#define GL_TEXTURE_BINDING_2D_ARRAY       0x8C1D
#define GL_R11F_G11F_B10F                 0x8C3A
#define GL_UNSIGNED_INT_10F_11F_11F_REV   0x8C3B
#define GL_RGB9_E5                        0x8C3D
#define GL_UNSIGNED_INT_5_9_9_9_REV       0x8C3E
#define GL_TEXTURE_SHARED_SIZE            0x8C3F
#define GL_TRANSFORM_FEEDBACK_VARYING_MAX_LENGTH 0x8C76
#define GL_TRANSFORM_FEEDBACK_BUFFER_MODE 0x8C7F
#define GL_MAX_TRANSFORM_FEEDBACK_SEPARATE_COMPONENTS 0x8C80
#define GL_TRANSFORM_FEEDBACK_VARYINGS    0x8C83
#define GL_TRANSFORM_FEEDBACK_BUFFER_START 0x8C84
#define GL_TRANSFORM_FEEDBACK_BUFFER_SIZE 0x8C85
#define GL_PRIMITIVES_GENERATED           0x8C87
#define GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN 0x8C88
#define GL_RASTERIZER_DISCARD             0x8C89
#define GL_MAX_TRANSFORM_FEEDBACK_INTERLEAVED_COMPONENTS 0x8C8A
#define GL_MAX_TRANSFORM_FEEDBACK_SEPARATE_ATTRIBS 0x8C8B
#define GL_INTERLEAVED_ATTRIBS            0x8C8C
#define GL_SEPARATE_ATTRIBS               0x8C8D
#define GL_TRANSFORM_FEEDBACK_BUFFER      0x8C8E
#define GL_TRANSFORM_FEEDBACK_BUFFER_BINDING 0x8C8F
#define GL_RGBA32UI                       0x8D70
#define GL_RGB32UI                        0x8D71
#define GL_RGBA16UI                       0x8D76
#define GL_RGB16UI                        0x8D77
#define GL_RGBA8UI                        0x8D7C
#define GL_RGB8UI                         0x8D7D
#define GL_RGBA32I                        0x8D82
#define GL_RGB32I                         0x8D83
#define GL_RGBA16I                        0x8D88
#define GL_RGB16I                         0x8D89
#define GL_RGBA8I                         0x8D8E
#define GL_RGB8I                          0x8D8F
#define GL_RED_INTEGER                    0x8D94
#define GL_GREEN_INTEGER                  0x8D95
#define GL_BLUE_INTEGER                   0x8D96
#define GL_RGB_INTEGER                    0x8D98
#define GL_RGBA_INTEGER                   0x8D99
#define GL_BGR_INTEGER                    0x8D9A
#define GL_BGRA_INTEGER                   0x8D9B
#define GL_SAMPLER_1D_ARRAY               0x8DC0
#define GL_SAMPLER_2D_ARRAY               0x8DC1
#define GL_SAMPLER_1D_ARRAY_SHADOW        0x8DC3
#define GL_SAMPLER_2D_ARRAY_SHADOW        0x8DC4
#define GL_SAMPLER_CUBE_SHADOW            0x8DC5
#define GL_UNSIGNED_INT_VEC2              0x8DC6
#define GL_UNSIGNED_INT_VEC3              0x8DC7
#define GL_UNSIGNED_INT_VEC4              0x8DC8
#define GL_INT_SAMPLER_1D                 0x8DC9
#define GL_INT_SAMPLER_2D                 0x8DCA
#define GL_INT_SAMPLER_3D                 0x8DCB
#define GL_INT_SAMPLER_CUBE               0x8DCC
#define GL_INT_SAMPLER_1D_ARRAY           0x8DCE
#define GL_INT_SAMPLER_2D_ARRAY           0x8DCF
#define GL_UNSIGNED_INT_SAMPLER_1D        0x8DD1
#define GL_UNSIGNED_INT_SAMPLER_2D        0x8DD2
#define GL_UNSIGNED_INT_SAMPLER_3D        0x8DD3
#define GL_UNSIGNED_INT_SAMPLER_CUBE      0x8DD4
#define GL_UNSIGNED_INT_SAMPLER_1D_ARRAY  0x8DD6
#define GL_UNSIGNED_INT_SAMPLER_2D_ARRAY  0x8DD7
#define GL_QUERY_WAIT                     0x8E13
#define GL_QUERY_NO_WAIT                  0x8E14
#define GL_QUERY_BY_REGION_WAIT           0x8E15
#define GL_QUERY_BY_REGION_NO_WAIT        0x8E16
#define GL_BUFFER_ACCESS_FLAGS            0x911F
#define GL_BUFFER_MAP_LENGTH              0x9120
#define GL_BUFFER_MAP_OFFSET              0x9121
#define GL_DEPTH_COMPONENT32F             0x8CAC
#define GL_DEPTH32F_STENCIL8              0x8CAD
#define GL_FLOAT_32_UNSIGNED_INT_24_8_REV 0x8DAD
#define GL_INVALID_FRAMEBUFFER_OPERATION  0x0506
#define GL_FRAMEBUFFER_ATTACHMENT_COLOR_ENCODING 0x8210
#define GL_FRAMEBUFFER_ATTACHMENT_COMPONENT_TYPE 0x8211
#define GL_FRAMEBUFFER_ATTACHMENT_RED_SIZE 0x8212
#define GL_FRAMEBUFFER_ATTACHMENT_GREEN_SIZE 0x8213
#define GL_FRAMEBUFFER_ATTACHMENT_BLUE_SIZE 0x8214
#define GL_FRAMEBUFFER_ATTACHMENT_ALPHA_SIZE 0x8215
#define GL_FRAMEBUFFER_ATTACHMENT_DEPTH_SIZE 0x8216
#define GL_FRAMEBUFFER_ATTACHMENT_STENCIL_SIZE 0x8217
#define GL_FRAMEBUFFER_DEFAULT            0x8218
#define GL_FRAMEBUFFER_UNDEFINED          0x8219
#define GL_DEPTH_STENCIL_ATTACHMENT       0x821A
#define GL_MAX_RENDERBUFFER_SIZE          0x84E8
#define GL_DEPTH_STENCIL                  0x84F9
#define GL_UNSIGNED_INT_24_8              0x84FA
#define GL_DEPTH24_STENCIL8               0x88F0
#define GL_TEXTURE_STENCIL_SIZE           0x88F1
#define GL_TEXTURE_RED_TYPE               0x8C10
#define GL_TEXTURE_GREEN_TYPE             0x8C11
#define GL_TEXTURE_BLUE_TYPE              0x8C12
#define GL_TEXTURE_ALPHA_TYPE             0x8C13
#define GL_TEXTURE_DEPTH_TYPE             0x8C16
#define GL_UNSIGNED_NORMALIZED            0x8C17
#define GL_FRAMEBUFFER_BINDING            0x8CA6
#define GL_DRAW_FRAMEBUFFER_BINDING       0x8CA6
#define GL_RENDERBUFFER_BINDING           0x8CA7
#define GL_READ_FRAMEBUFFER               0x8CA8
#define GL_DRAW_FRAMEBUFFER               0x8CA9
#define GL_READ_FRAMEBUFFER_BINDING       0x8CAA
#define GL_RENDERBUFFER_SAMPLES           0x8CAB
#define GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE 0x8CD0
#define GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME 0x8CD1
#define GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_LEVEL 0x8CD2
#define GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_CUBE_MAP_FACE 0x8CD3
#define GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_LAYER 0x8CD4
#define GL_FRAMEBUFFER_COMPLETE           0x8CD5
#define GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT 0x8CD6
#define GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT 0x8CD7
#define GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER 0x8CDB
#define GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER 0x8CDC
#define GL_FRAMEBUFFER_UNSUPPORTED        0x8CDD
#define GL_MAX_COLOR_ATTACHMENTS          0x8CDF
#define GL_COLOR_ATTACHMENT0              0x8CE0
#define GL_COLOR_ATTACHMENT1              0x8CE1
#define GL_COLOR_ATTACHMENT2              0x8CE2
#define GL_COLOR_ATTACHMENT3              0x8CE3
#define GL_COLOR_ATTACHMENT4              0x8CE4
#define GL_COLOR_ATTACHMENT5              0x8CE5
#define GL_COLOR_ATTACHMENT6              0x8CE6
#define GL_COLOR_ATTACHMENT7              0x8CE7
#define GL_COLOR_ATTACHMENT8              0x8CE8
#define GL_COLOR_ATTACHMENT9              0x8CE9
#define GL_COLOR_ATTACHMENT10             0x8CEA
#define GL_COLOR_ATTACHMENT11             0x8CEB
#define GL_COLOR_ATTACHMENT12             0x8CEC
#define GL_COLOR_ATTACHMENT13             0x8CED
#define GL_COLOR_ATTACHMENT14             0x8CEE
#define GL_COLOR_ATTACHMENT15             0x8CEF
#define GL_COLOR_ATTACHMENT16             0x8CF0
#define GL_COLOR_ATTACHMENT17             0x8CF1
#define GL_COLOR_ATTACHMENT18             0x8CF2
#define GL_COLOR_ATTACHMENT19             0x8CF3
#define GL_COLOR_ATTACHMENT20             0x8CF4
#define GL_COLOR_ATTACHMENT21             0x8CF5
#define GL_COLOR_ATTACHMENT22             0x8CF6
#define GL_COLOR_ATTACHMENT23             0x8CF7
#define GL_COLOR_ATTACHMENT24             0x8CF8
#define GL_COLOR_ATTACHMENT25             0x8CF9
#define GL_COLOR_ATTACHMENT26             0x8CFA
#define GL_COLOR_ATTACHMENT27             0x8CFB
#define GL_COLOR_ATTACHMENT28             0x8CFC
#define GL_COLOR_ATTACHMENT29             0x8CFD
#define GL_COLOR_ATTACHMENT30             0x8CFE
#define GL_COLOR_ATTACHMENT31             0x8CFF
#define GL_DEPTH_ATTACHMENT               0x8D00
#define GL_STENCIL_ATTACHMENT             0x8D20
#define GL_FRAMEBUFFER                    0x8D40
#define GL_RENDERBUFFER                   0x8D41
#define GL_RENDERBUFFER_WIDTH             0x8D42
#define GL_RENDERBUFFER_HEIGHT            0x8D43
#define GL_RENDERBUFFER_INTERNAL_FORMAT   0x8D44
#define GL_STENCIL_INDEX1                 0x8D46
#define GL_STENCIL_INDEX4                 0x8D47
#define GL_STENCIL_INDEX8                 0x8D48
#define GL_STENCIL_INDEX16                0x8D49
#define GL_RENDERBUFFER_RED_SIZE          0x8D50
#define GL_RENDERBUFFER_GREEN_SIZE        0x8D51
#define GL_RENDERBUFFER_BLUE_SIZE         0x8D52
#define GL_RENDERBUFFER_ALPHA_SIZE        0x8D53
#define GL_RENDERBUFFER_DEPTH_SIZE        0x8D54
#define GL_RENDERBUFFER_STENCIL_SIZE      0x8D55
#define GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE 0x8D56
#define GL_MAX_SAMPLES                    0x8D57
#define GL_FRAMEBUFFER_SRGB               0x8DB9
#define GL_HALF_FLOAT                     0x140B
#define GL_MAP_READ_BIT                   0x0001
#define GL_MAP_WRITE_BIT                  0x0002
#define GL_MAP_INVALIDATE_RANGE_BIT       0x0004
#define GL_MAP_INVALIDATE_BUFFER_BIT      0x0008
#define GL_MAP_FLUSH_EXPLICIT_BIT         0x0010
#define GL_MAP_UNSYNCHRONIZED_BIT         0x0020
#define GL_COMPRESSED_RED_RGTC1           0x8DBB
#define GL_COMPRESSED_SIGNED_RED_RGTC1    0x8DBC
#define GL_COMPRESSED_RG_RGTC2            0x8DBD
#define GL_COMPRESSED_SIGNED_RG_RGTC2     0x8DBE
#define GL_RG                             0x8227
#define GL_RG_INTEGER                     0x8228
#define GL_R8                             0x8229
#define GL_R16                            0x822A
#define GL_RG8                            0x822B
#define GL_RG16                           0x822C
#define GL_R16F                           0x822D
#define GL_R32F                           0x822E
#define GL_RG16F                          0x822F
#define GL_RG32F                          0x8230
#define GL_R8I                            0x8231
#define GL_R8UI                           0x8232
#define GL_R16I                           0x8233
#define GL_R16UI                          0x8234
#define GL_R32I                           0x8235
#define GL_R32UI                          0x8236
#define GL_RG8I                           0x8237
#define GL_RG8UI                          0x8238
#define GL_RG16I                          0x8239
#define GL_RG16UI                         0x823A
#define GL_RG32I                          0x823B
#define GL_RG32UI                         0x823C
#define GL_VERTEX_ARRAY_BINDING           0x85B5
typedef void (APIENTRYP PFNGLCOLORMASKIPROC) (GLuint index, GLboolean r, GLboolean g, GLboolean b, GLboolean a);
typedef void (APIENTRYP PFNGLGETBOOLEANI_VPROC) (GLenum target, GLuint index, GLboolean *data);
typedef void (APIENTRYP PFNGLGETINTEGERI_VPROC) (GLenum target, GLuint index, GLint *data);
typedef void (APIENTRYP PFNGLENABLEIPROC) (GLenum target, GLuint index);
typedef void (APIENTRYP PFNGLDISABLEIPROC) (GLenum target, GLuint index);
typedef GLboolean (APIENTRYP PFNGLISENABLEDIPROC) (GLenum target, GLuint index);
typedef void (APIENTRYP PFNGLBEGINTRANSFORMFEEDBACKPROC) (GLenum primitiveMode);
typedef void (APIENTRYP PFNGLENDTRANSFORMFEEDBACKPROC) (void);
typedef void (APIENTRYP PFNGLBINDBUFFERRANGEPROC) (GLenum target, GLuint index, GLuint buffer, GLintptr offset, GLsizeiptr size);
typedef void (APIENTRYP PFNGLBINDBUFFERBASEPROC) (GLenum target, GLuint index, GLuint buffer);
typedef void (APIENTRYP PFNGLTRANSFORMFEEDBACKVARYINGSPROC) (GLuint program, GLsizei count, const GLchar *const*varyings, GLenum bufferMode);
typedef void (APIENTRYP PFNGLGETTRANSFORMFEEDBACKVARYINGPROC) (GLuint program, GLuint index, GLsizei bufSize, GLsizei *length, GLsizei *size, GLenum *type, GLchar *name);
typedef void (APIENTRYP PFNGLCLAMPCOLORPROC) (GLenum target, GLenum clamp);
typedef void (APIENTRYP PFNGLBEGINCONDITIONALRENDERPROC) (GLuint id, GLenum mode);
typedef void (APIENTRYP PFNGLENDCONDITIONALRENDERPROC) (void);
typedef void (APIENTRYP PFNGLVERTEXATTRIBIPOINTERPROC) (GLuint index, GLint size, GLenum type, GLsizei stride, const void *pointer);
typedef void (APIENTRYP PFNGLGETVERTEXATTRIBIIVPROC) (GLuint index, GLenum pname, GLint *params);
typedef void (APIENTRYP PFNGLGETVERTEXATTRIBIUIVPROC) (GLuint index, GLenum pname, GLuint *params);
typedef void (APIENTRYP PFNGLVERTEXATTRIBI1IPROC) (GLuint index, GLint x);
typedef void (APIENTRYP PFNGLVERTEXATTRIBI2IPROC) (GLuint index, GLint x, GLint y);
typedef void (APIENTRYP PFNGLVERTEXATTRIBI3IPROC) (GLuint index, GLint x, GLint y, GLint z);
typedef void (APIENTRYP PFNGLVERTEXATTRIBI4IPROC) (GLuint index, GLint x, GLint y, GLint z, GLint w);
typedef void (APIENTRYP PFNGLVERTEXATTRIBI1UIPROC) (GLuint index, GLuint x);
typedef void (APIENTRYP PFNGLVERTEXATTRIBI2UIPROC) (GLuint index, GLuint x, GLuint y);
typedef void (APIENTRYP PFNGLVERTEXATTRIBI3UIPROC) (GLuint index, GLuint x, GLuint y, GLuint z);
typedef void (APIENTRYP PFNGLVERTEXATTRIBI4UIPROC) (GLuint index, GLuint x, GLuint y, GLuint z, GLuint w);
typedef void (APIENTRYP PFNGLVERTEXATTRIBI1IVPROC) (GLuint index, const GLint *v);
typedef void (APIENTRYP PFNGLVERTEXATTRIBI2IVPROC) (GLuint index, const GLint *v);
typedef void (APIENTRYP PFNGLVERTEXATTRIBI3IVPROC) (GLuint index, const GLint *v);
typedef void (APIENTRYP PFNGLVERTEXATTRIBI4IVPROC) (GLuint index, const GLint *v);
typedef void (APIENTRYP PFNGLVERTEXATTRIBI1UIVPROC) (GLuint index, const GLuint *v);
typedef void (APIENTRYP PFNGLVERTEXATTRIBI2UIVPROC) (GLuint index, const GLuint *v);
typedef void (APIENTRYP PFNGLVERTEXATTRIBI3UIVPROC) (GLuint index, const GLuint *v);
typedef void (APIENTRYP PFNGLVERTEXATTRIBI4UIVPROC) (GLuint index, const GLuint *v);
typedef void (APIENTRYP PFNGLVERTEXATTRIBI4BVPROC) (GLuint index, const GLbyte *v);
typedef void (APIENTRYP PFNGLVERTEXATTRIBI4SVPROC) (GLuint index, const GLshort *v);
typedef void (APIENTRYP PFNGLVERTEXATTRIBI4UBVPROC) (GLuint index, const GLubyte *v);
typedef void (APIENTRYP PFNGLVERTEXATTRIBI4USVPROC) (GLuint index, const GLushort *v);
typedef void (APIENTRYP PFNGLGETUNIFORMUIVPROC) (GLuint program, GLint location, GLuint *params);
typedef void (APIENTRYP PFNGLBINDFRAGDATALOCATIONPROC) (GLuint program, GLuint color, const GLchar *name);
typedef GLint (APIENTRYP PFNGLGETFRAGDATALOCATIONPROC) (GLuint program, const GLchar *name);
typedef void (APIENTRYP PFNGLUNIFORM1UIPROC) (GLint location, GLuint v0);
typedef void (APIENTRYP PFNGLUNIFORM2UIPROC) (GLint location, GLuint v0, GLuint v1);
typedef void (APIENTRYP PFNGLUNIFORM3UIPROC) (GLint location, GLuint v0, GLuint v1, GLuint v2);
typedef void (APIENTRYP PFNGLUNIFORM4UIPROC) (GLint location, GLuint v0, GLuint v1, GLuint v2, GLuint v3);
typedef void (APIENTRYP PFNGLUNIFORM1UIVPROC) (GLint location, GLsizei count, const GLuint *value);
typedef void (APIENTRYP PFNGLUNIFORM2UIVPROC) (GLint location, GLsizei count, const GLuint *value);
typedef void (APIENTRYP PFNGLUNIFORM3UIVPROC) (GLint location, GLsizei count, const GLuint *value);
typedef void (APIENTRYP PFNGLUNIFORM4UIVPROC) (GLint location, GLsizei count, const GLuint *value);
typedef void (APIENTRYP PFNGLTEXPARAMETERIIVPROC) (GLenum target, GLenum pname, const GLint *params);
typedef void (APIENTRYP PFNGLTEXPARAMETERIUIVPROC) (GLenum target, GLenum pname, const GLuint *params);
typedef void (APIENTRYP PFNGLGETTEXPARAMETERIIVPROC) (GLenum target, GLenum pname, GLint *params);
typedef void (APIENTRYP PFNGLGETTEXPARAMETERIUIVPROC) (GLenum target, GLenum pname, GLuint *params);
typedef void (APIENTRYP PFNGLCLEARBUFFERIVPROC) (GLenum buffer, GLint drawbuffer, const GLint *value);
typedef void (APIENTRYP PFNGLCLEARBUFFERUIVPROC) (GLenum buffer, GLint drawbuffer, const GLuint *value);
typedef void (APIENTRYP PFNGLCLEARBUFFERFVPROC) (GLenum buffer, GLint drawbuffer, const GLfloat *value);
typedef void (APIENTRYP PFNGLCLEARBUFFERFIPROC) (GLenum buffer, GLint drawbuffer, GLfloat depth, GLint stencil);
typedef const GLubyte *(APIENTRYP PFNGLGETSTRINGIPROC) (GLenum name, GLuint index);
typedef GLboolean (APIENTRYP PFNGLISRENDERBUFFERPROC) (GLuint renderbuffer);
typedef void (APIENTRYP PFNGLBINDRENDERBUFFERPROC) (GLenum target, GLuint renderbuffer);
typedef void (APIENTRYP PFNGLDELETERENDERBUFFERSPROC) (GLsizei n, const GLuint *renderbuffers);
typedef void (APIENTRYP PFNGLGENRENDERBUFFERSPROC) (GLsizei n, GLuint *renderbuffers);
typedef void (APIENTRYP PFNGLRENDERBUFFERSTORAGEPROC) (GLenum target, GLenum internalformat, GLsizei width, GLsizei height);
typedef void (APIENTRYP PFNGLGETRENDERBUFFERPARAMETERIVPROC) (GLenum target, GLenum pname, GLint *params);
typedef GLboolean (APIENTRYP PFNGLISFRAMEBUFFERPROC) (GLuint framebuffer);
typedef void (APIENTRYP PFNGLBINDFRAMEBUFFERPROC) (GLenum target, GLuint framebuffer);
typedef void (APIENTRYP PFNGLDELETEFRAMEBUFFERSPROC) (GLsizei n, const GLuint *framebuffers);
typedef void (APIENTRYP PFNGLGENFRAMEBUFFERSPROC) (GLsizei n, GLuint *framebuffers);
typedef GLenum (APIENTRYP PFNGLCHECKFRAMEBUFFERSTATUSPROC) (GLenum target);
typedef void (APIENTRYP PFNGLFRAMEBUFFERTEXTURE1DPROC) (GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level);
typedef void (APIENTRYP PFNGLFRAMEBUFFERTEXTURE2DPROC) (GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level);
typedef void (APIENTRYP PFNGLFRAMEBUFFERTEXTURE3DPROC) (GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level, GLint zoffset);
typedef void (APIENTRYP PFNGLFRAMEBUFFERRENDERBUFFERPROC) (GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer);
typedef void (APIENTRYP PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVPROC) (GLenum target, GLenum attachment, GLenum pname, GLint *params);
typedef void (APIENTRYP PFNGLGENERATEMIPMAPPROC) (GLenum target);
typedef void (APIENTRYP PFNGLBLITFRAMEBUFFERPROC) (GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter);
typedef void (APIENTRYP PFNGLRENDERBUFFERSTORAGEMULTISAMPLEPROC) (GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height);
typedef void (APIENTRYP PFNGLFRAMEBUFFERTEXTURELAYERPROC) (GLenum target, GLenum attachment, GLuint texture, GLint level, GLint layer);
typedef void *(APIENTRYP PFNGLMAPBUFFERRANGEPROC) (GLenum target, GLintptr offset, GLsizeiptr length, GLbitfield access);
typedef void (APIENTRYP PFNGLFLUSHMAPPEDBUFFERRANGEPROC) (GLenum target, GLintptr offset, GLsizeiptr length);
typedef void (APIENTRYP PFNGLBINDVERTEXARRAYPROC) (GLuint array);
typedef void (APIENTRYP PFNGLDELETEVERTEXARRAYSPROC) (GLsizei n, const GLuint *arrays);
typedef void (APIENTRYP PFNGLGENVERTEXARRAYSPROC) (GLsizei n, GLuint *arrays);
typedef GLboolean (APIENTRYP PFNGLISVERTEXARRAYPROC) (GLuint array);
#endif /* GL_VERSION_3_0 */
#ifndef GL_VERSION_3_1
#define GL_VERSION_3_1 1
#define GL_SAMPLER_2D_RECT                0x8B63
#define GL_SAMPLER_2D_RECT_SHADOW         0x8B64
#define GL_SAMPLER_BUFFER                 0x8DC2
#define GL_INT_SAMPLER_2D_RECT            0x8DCD
#define GL_INT_SAMPLER_BUFFER             0x8DD0
#define GL_UNSIGNED_INT_SAMPLER_2D_RECT   0x8DD5
#define GL_UNSIGNED_INT_SAMPLER_BUFFER    0x8DD8
#define GL_TEXTURE_BUFFER                 0x8C2A
#define GL_MAX_TEXTURE_BUFFER_SIZE        0x8C2B
#define GL_TEXTURE_BINDING_BUFFER         0x8C2C
#define GL_TEXTURE_BUFFER_DATA_STORE_BINDING 0x8C2D
#define GL_TEXTURE_RECTANGLE              0x84F5
#define GL_TEXTURE_BINDING_RECTANGLE      0x84F6
#define GL_PROXY_TEXTURE_RECTANGLE        0x84F7
#define GL_MAX_RECTANGLE_TEXTURE_SIZE     0x84F8
#define GL_R8_SNORM                       0x8F94
#define GL_RG8_SNORM                      0x8F95
#define GL_RGB8_SNORM                     0x8F96
#define GL_RGBA8_SNORM                    0x8F97
#define GL_R16_SNORM                      0x8F98
#define GL_RG16_SNORM                     0x8F99
#define GL_RGB16_SNORM                    0x8F9A
#define GL_RGBA16_SNORM                   0x8F9B
#define GL_SIGNED_NORMALIZED              0x8F9C
#define GL_PRIMITIVE_RESTART              0x8F9D
#define GL_PRIMITIVE_RESTART_INDEX        0x8F9E
#define GL_COPY_READ_BUFFER               0x8F36
#define GL_COPY_WRITE_BUFFER              0x8F37
#define GL_UNIFORM_BUFFER                 0x8A11
#define GL_UNIFORM_BUFFER_BINDING         0x8A28
#define GL_UNIFORM_BUFFER_START           0x8A29
#define GL_UNIFORM_BUFFER_SIZE            0x8A2A
#define GL_MAX_VERTEX_UNIFORM_BLOCKS      0x8A2B
#define GL_MAX_GEOMETRY_UNIFORM_BLOCKS    0x8A2C
#define GL_MAX_FRAGMENT_UNIFORM_BLOCKS    0x8A2D
#define GL_MAX_COMBINED_UNIFORM_BLOCKS    0x8A2E
#define GL_MAX_UNIFORM_BUFFER_BINDINGS    0x8A2F
#define GL_MAX_UNIFORM_BLOCK_SIZE         0x8A30
#define GL_MAX_COMBINED_VERTEX_UNIFORM_COMPONENTS 0x8A31
#define GL_MAX_COMBINED_GEOMETRY_UNIFORM_COMPONENTS 0x8A32
#define GL_MAX_COMBINED_FRAGMENT_UNIFORM_COMPONENTS 0x8A33
#define GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT 0x8A34
#define GL_ACTIVE_UNIFORM_BLOCK_MAX_NAME_LENGTH 0x8A35
#define GL_ACTIVE_UNIFORM_BLOCKS          0x8A36
#define GL_UNIFORM_TYPE                   0x8A37
#define GL_UNIFORM_SIZE                   0x8A38
#define GL_UNIFORM_NAME_LENGTH            0x8A39
#define GL_UNIFORM_BLOCK_INDEX            0x8A3A
#define GL_UNIFORM_OFFSET                 0x8A3B
#define GL_UNIFORM_ARRAY_STRIDE           0x8A3C
#define GL_UNIFORM_MATRIX_STRIDE          0x8A3D
#define GL_UNIFORM_IS_ROW_MAJOR           0x8A3E
#define GL_UNIFORM_BLOCK_BINDING          0x8A3F
#define GL_UNIFORM_BLOCK_DATA_SIZE        0x8A40
#define GL_UNIFORM_BLOCK_NAME_LENGTH      0x8A41
#define GL_UNIFORM_BLOCK_ACTIVE_UNIFORMS  0x8A42
#define GL_UNIFORM_BLOCK_ACTIVE_UNIFORM_INDICES 0x8A43
#define GL_UNIFORM_BLOCK_REFERENCED_BY_VERTEX_SHADER 0x8A44
#define GL_UNIFORM_BLOCK_REFERENCED_BY_GEOMETRY_SHADER 0x8A45
#define GL_UNIFORM_BLOCK_REFERENCED_BY_FRAGMENT_SHADER 0x8A46
#define GL_INVALID_INDEX                  0xFFFFFFFFu
typedef void (APIENTRYP PFNGLDRAWARRAYSINSTANCEDPROC) (GLenum mode, GLint first, GLsizei count, GLsizei instancecount);
typedef void (APIENTRYP PFNGLDRAWELEMENTSINSTANCEDPROC) (GLenum mode, GLsizei count, GLenum type, const void *indices, GLsizei instancecount);
typedef void (APIENTRYP PFNGLTEXBUFFERPROC) (GLenum target, GLenum internalformat, GLuint buffer);
typedef void (APIENTRYP PFNGLPRIMITIVERESTARTINDEXPROC) (GLuint index);
typedef void (APIENTRYP PFNGLCOPYBUFFERSUBDATAPROC) (GLenum readTarget, GLenum writeTarget, GLintptr readOffset, GLintptr writeOffset, GLsizeiptr size);
typedef void (APIENTRYP PFNGLGETUNIFORMINDICESPROC) (GLuint program, GLsizei uniformCount, const GLchar *const*uniformNames, GLuint *uniformIndices);
typedef void (APIENTRYP PFNGLGETACTIVEUNIFORMSIVPROC) (GLuint program, GLsizei uniformCount, const GLuint *uniformIndices, GLenum pname, GLint *params);
typedef void (APIENTRYP PFNGLGETACTIVEUNIFORMNAMEPROC) (GLuint program, GLuint uniformIndex, GLsizei bufSize, GLsizei *length, GLchar *uniformName);
typedef GLuint (APIENTRYP PFNGLGETUNIFORMBLOCKINDEXPROC) (GLuint program, const GLchar *uniformBlockName);
typedef void (APIENTRYP PFNGLGETACTIVEUNIFORMBLOCKIVPROC) (GLuint program, GLuint uniformBlockIndex, GLenum pname, GLint *params);
typedef void (APIENTRYP PFNGLGETACTIVEUNIFORMBLOCKNAMEPROC) (GLuint program, GLuint uniformBlockIndex, GLsizei bufSize, GLsizei *length, GLchar *uniformBlockName);
typedef void (APIENTRYP PFNGLUNIFORMBLOCKBINDINGPROC) (GLuint program, GLuint uniformBlockIndex, GLuint uniformBlockBinding);
#endif /* GL_VERSION_3_1 */
#ifndef GL_VERSION_3_2
#define GL_VERSION_3_2 1
typedef struct __GLsync *GLsync;
typedef uint64_t GLuint64;
typedef int64_t GLint64;
#define GL_CONTEXT_CORE_PROFILE_BIT       0x00000001
#define GL_CONTEXT_COMPATIBILITY_PROFILE_BIT 0x00000002
#define GL_LINES_ADJACENCY                0x000A
#define GL_LINE_STRIP_ADJACENCY           0x000B
#define GL_TRIANGLES_ADJACENCY            0x000C
#define GL_TRIANGLE_STRIP_ADJACENCY       0x000D
#define GL_PROGRAM_POINT_SIZE             0x8642
#define GL_MAX_GEOMETRY_TEXTURE_IMAGE_UNITS 0x8C29
#define GL_FRAMEBUFFER_ATTACHMENT_LAYERED 0x8DA7
#define GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS 0x8DA8
#define GL_GEOMETRY_SHADER                0x8DD9
#define GL_GEOMETRY_VERTICES_OUT          0x8916
#define GL_GEOMETRY_INPUT_TYPE            0x8917
#define GL_GEOMETRY_OUTPUT_TYPE           0x8918
#define GL_MAX_GEOMETRY_UNIFORM_COMPONENTS 0x8DDF
#define GL_MAX_GEOMETRY_OUTPUT_VERTICES   0x8DE0
#define GL_MAX_GEOMETRY_TOTAL_OUTPUT_COMPONENTS 0x8DE1
#define GL_MAX_VERTEX_OUTPUT_COMPONENTS   0x9122
#define GL_MAX_GEOMETRY_INPUT_COMPONENTS  0x9123
#define GL_MAX_GEOMETRY_OUTPUT_COMPONENTS 0x9124
#define GL_MAX_FRAGMENT_INPUT_COMPONENTS  0x9125
#define GL_CONTEXT_PROFILE_MASK           0x9126
#define GL_DEPTH_CLAMP                    0x864F
#define GL_QUADS_FOLLOW_PROVOKING_VERTEX_CONVENTION 0x8E4C
#define GL_FIRST_VERTEX_CONVENTION        0x8E4D
#define GL_LAST_VERTEX_CONVENTION         0x8E4E
#define GL_PROVOKING_VERTEX               0x8E4F
#define GL_TEXTURE_CUBE_MAP_SEAMLESS      0x884F
#define GL_MAX_SERVER_WAIT_TIMEOUT        0x9111
#define GL_OBJECT_TYPE                    0x9112
#define GL_SYNC_CONDITION                 0x9113
#define GL_SYNC_STATUS                    0x9114
#define GL_SYNC_FLAGS                     0x9115
#define GL_SYNC_FENCE                     0x9116
#define GL_SYNC_GPU_COMMANDS_COMPLETE     0x9117
#define GL_UNSIGNALED                     0x9118
#define GL_SIGNALED                       0x9119
#define GL_ALREADY_SIGNALED               0x911A
#define GL_TIMEOUT_EXPIRED                0x911B
#define GL_CONDITION_SATISFIED            0x911C
#define GL_WAIT_FAILED                    0x911D
#define GL_TIMEOUT_IGNORED                0xFFFFFFFFFFFFFFFFull
#define GL_SYNC_FLUSH_COMMANDS_BIT        0x00000001
#define GL_SAMPLE_POSITION                0x8E50
#define GL_SAMPLE_MASK                    0x8E51
#define GL_SAMPLE_MASK_VALUE              0x8E52
#define GL_MAX_SAMPLE_MASK_WORDS          0x8E59
#define GL_TEXTURE_2D_MULTISAMPLE         0x9100
#define GL_PROXY_TEXTURE_2D_MULTISAMPLE   0x9101
#define GL_TEXTURE_2D_MULTISAMPLE_ARRAY   0x9102
#define GL_PROXY_TEXTURE_2D_MULTISAMPLE_ARRAY 0x9103
#define GL_TEXTURE_BINDING_2D_MULTISAMPLE 0x9104
#define GL_TEXTURE_BINDING_2D_MULTISAMPLE_ARRAY 0x9105
#define GL_TEXTURE_SAMPLES                0x9106
#define GL_TEXTURE_FIXED_SAMPLE_LOCATIONS 0x9107
#define GL_SAMPLER_2D_MULTISAMPLE         0x9108
#define GL_INT_SAMPLER_2D_MULTISAMPLE     0x9109
#define GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE 0x910A
#define GL_SAMPLER_2D_MULTISAMPLE_ARRAY   0x910B
#define GL_INT_SAMPLER_2D_MULTISAMPLE_ARRAY 0x910C
#define GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE_ARRAY 0x910D
#define GL_MAX_COLOR_TEXTURE_SAMPLES      0x910E
#define GL_MAX_DEPTH_TEXTURE_SAMPLES      0x910F
#define GL_MAX_INTEGER_SAMPLES            0x9110
typedef void (APIENTRYP PFNGLDRAWELEMENTSBASEVERTEXPROC) (GLenum mode, GLsizei count, GLenum type, const void *indices, GLint basevertex);
typedef void (APIENTRYP PFNGLDRAWRANGEELEMENTSBASEVERTEXPROC) (GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const void *indices, GLint basevertex);
typedef void (APIENTRYP PFNGLDRAWELEMENTSINSTANCEDBASEVERTEXPROC) (GLenum mode, GLsizei count, GLenum type, const void *indices, GLsizei instancecount, GLint basevertex);
typedef void (APIENTRYP PFNGLMULTIDRAWELEMENTSBASEVERTEXPROC) (GLenum mode, const GLsizei *count, GLenum type, const void *const*indices, GLsizei drawcount, const GLint *basevertex);
typedef void (APIENTRYP PFNGLPROVOKINGVERTEXPROC) (GLenum mode);
typedef GLsync (APIENTRYP PFNGLFENCESYNCPROC) (GLenum condition, GLbitfield flags);
typedef GLboolean (APIENTRYP PFNGLISSYNCPROC) (GLsync sync);
typedef void (APIENTRYP PFNGLDELETESYNCPROC) (GLsync sync);
typedef GLenum (APIENTRYP PFNGLCLIENTWAITSYNCPROC) (GLsync sync, GLbitfield flags, GLuint64 timeout);
typedef void (APIENTRYP PFNGLWAITSYNCPROC) (GLsync sync, GLbitfield flags, GLuint64 timeout);
typedef void (APIENTRYP PFNGLGETINTEGER64VPROC) (GLenum pname, GLint64 *data);
typedef void (APIENTRYP PFNGLGETSYNCIVPROC) (GLsync sync, GLenum pname, GLsizei count, GLsizei *length, GLint *values);
typedef void (APIENTRYP PFNGLGETINTEGER64I_VPROC) (GLenum target, GLuint index, GLint64 *data);
typedef void (APIENTRYP PFNGLGETBUFFERPARAMETERI64VPROC) (GLenum target, GLenum pname, GLint64 *params);
typedef void (APIENTRYP PFNGLFRAMEBUFFERTEXTUREPROC) (GLenum target, GLenum attachment, GLuint texture, GLint level);
typedef void (APIENTRYP PFNGLTEXIMAGE2DMULTISAMPLEPROC) (GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLboolean fixedsamplelocations);
typedef void (APIENTRYP PFNGLTEXIMAGE3DMULTISAMPLEPROC) (GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLboolean fixedsamplelocations);
typedef void (APIENTRYP PFNGLGETMULTISAMPLEFVPROC) (GLenum pname, GLuint index, GLfloat *val);
typedef void (APIENTRYP PFNGLSAMPLEMASKIPROC) (GLuint maskNumber, GLbitfield mask);
#endif /* GL_VERSION_3_2 */
#ifndef GL_VERSION_3_3
#define GL_VERSION_3_3 1
#define GL_VERTEX_ATTRIB_ARRAY_DIVISOR    0x88FE
#define GL_SRC1_COLOR                     0x88F9
#define GL_ONE_MINUS_SRC1_COLOR           0x88FA
#define GL_ONE_MINUS_SRC1_ALPHA           0x88FB
#define GL_MAX_DUAL_SOURCE_DRAW_BUFFERS   0x88FC
#define GL_ANY_SAMPLES_PASSED             0x8C2F
#define GL_SAMPLER_BINDING                0x8919
#define GL_RGB10_A2UI                     0x906F
#define GL_TEXTURE_SWIZZLE_R              0x8E42
#define GL_TEXTURE_SWIZZLE_G              0x8E43
#define GL_TEXTURE_SWIZZLE_B              0x8E44
#define GL_TEXTURE_SWIZZLE_A              0x8E45
#define GL_TEXTURE_SWIZZLE_RGBA           0x8E46
#define GL_TIME_ELAPSED                   0x88BF
#define GL_TIMESTAMP                      0x8E28
#define GL_INT_2_10_10_10_REV             0x8D9F
typedef void (APIENTRYP PFNGLBINDFRAGDATALOCATIONINDEXEDPROC) (GLuint program, GLuint colorNumber, GLuint index, const GLchar *name);
typedef GLint (APIENTRYP PFNGLGETFRAGDATAINDEXPROC) (GLuint program, const GLchar *name);
typedef void (APIENTRYP PFNGLGENSAMPLERSPROC) (GLsizei count, GLuint *samplers);
typedef void (APIENTRYP PFNGLDELETESAMPLERSPROC) (GLsizei count, const GLuint *samplers);
typedef GLboolean (APIENTRYP PFNGLISSAMPLERPROC) (GLuint sampler);
typedef void (APIENTRYP PFNGLBINDSAMPLERPROC) (GLuint unit, GLuint sampler);
typedef void (APIENTRYP PFNGLSAMPLERPARAMETERIPROC) (GLuint sampler, GLenum pname, GLint param);
typedef void (APIENTRYP PFNGLSAMPLERPARAMETERIVPROC) (GLuint sampler, GLenum pname, const GLint *param);
typedef void (APIENTRYP PFNGLSAMPLERPARAMETERFPROC) (GLuint sampler, GLenum pname, GLfloat param);
typedef void (APIENTRYP PFNGLSAMPLERPARAMETERFVPROC) (GLuint sampler, GLenum pname, const GLfloat *param);
typedef void (APIENTRYP PFNGLSAMPLERPARAMETERIIVPROC) (GLuint sampler, GLenum pname, const GLint *param);
typedef void (APIENTRYP PFNGLSAMPLERPARAMETERIUIVPROC) (GLuint sampler, GLenum pname, const GLuint *param);
typedef void (APIENTRYP PFNGLGETSAMPLERPARAMETERIVPROC) (GLuint sampler, GLenum pname, GLint *params);
typedef void (APIENTRYP PFNGLGETSAMPLERPARAMETERIIVPROC) (GLuint sampler, GLenum pname, GLint *params);
typedef void (APIENTRYP PFNGLGETSAMPLERPARAMETERFVPROC) (GLuint sampler, GLenum pname, GLfloat *params);
typedef void (APIENTRYP PFNGLGETSAMPLERPARAMETERIUIVPROC) (GLuint sampler, GLenum pname, GLuint *params);
typedef void (APIENTRYP PFNGLQUERYCOUNTERPROC) (GLuint id, GLenum target);
typedef void (APIENTRYP PFNGLGETQUERYOBJECTI64VPROC) (GLuint id, GLenum pname, GLint64 *params);
typedef void (APIENTRYP PFNGLGETQUERYOBJECTUI64VPROC) (GLuint id, GLenum pname, GLuint64 *params);
typedef void (APIENTRYP PFNGLVERTEXATTRIBDIVISORPROC) (GLuint index, GLuint divisor);
typedef void (APIENTRYP PFNGLVERTEXATTRIBP1UIPROC) (GLuint index, GLenum type, GLboolean normalized, GLuint value);
typedef void (APIENTRYP PFNGLVERTEXATTRIBP1UIVPROC) (GLuint index, GLenum type, GLboolean normalized, const GLuint *value);
typedef void (APIENTRYP PFNGLVERTEXATTRIBP2UIPROC) (GLuint index, GLenum type, GLboolean normalized, GLuint value);
typedef void (APIENTRYP PFNGLVERTEXATTRIBP2UIVPROC) (GLuint index, GLenum type, GLboolean normalized, const GLuint *value);
typedef void (APIENTRYP PFNGLVERTEXATTRIBP3UIPROC) (GLuint index, GLenum type, GLboolean normalized, GLuint value);
typedef void (APIENTRYP PFNGLVERTEXATTRIBP3UIVPROC) (GLuint index, GLenum type, GLboolean normalized, const GLuint *value);
typedef void (APIENTRYP PFNGLVERTEXATTRIBP4UIPROC) (GLuint index, GLenum type, GLboolean normalized, GLuint value);
typedef void (APIENTRYP PFNGLVERTEXATTRIBP4UIVPROC) (GLuint index, GLenum type, GLboolean normalized, const GLuint *value);
#endif /* GL_VERSION_3_3 */
#ifndef GL_VERSION_4_0
#define GL_VERSION_4_0 1
#define GL_SAMPLE_SHADING                 0x8C36
#define GL_MIN_SAMPLE_SHADING_VALUE       0x8C37
#define GL_MIN_PROGRAM_TEXTURE_GATHER_OFFSET 0x8E5E
#define GL_MAX_PROGRAM_TEXTURE_GATHER_OFFSET 0x8E5F
#define GL_TEXTURE_CUBE_MAP_ARRAY         0x9009
#define GL_TEXTURE_BINDING_CUBE_MAP_ARRAY 0x900A
#define GL_PROXY_TEXTURE_CUBE_MAP_ARRAY   0x900B
#define GL_SAMPLER_CUBE_MAP_ARRAY         0x900C
#define GL_SAMPLER_CUBE_MAP_ARRAY_SHADOW  0x900D
#define GL_INT_SAMPLER_CUBE_MAP_ARRAY     0x900E
#define GL_UNSIGNED_INT_SAMPLER_CUBE_MAP_ARRAY 0x900F
#define GL_DRAW_INDIRECT_BUFFER           0x8F3F
#define GL_DRAW_INDIRECT_BUFFER_BINDING   0x8F43
#define GL_GEOMETRY_SHADER_INVOCATIONS    0x887F
#define GL_MAX_GEOMETRY_SHADER_INVOCATIONS 0x8E5A
#define GL_MIN_FRAGMENT_INTERPOLATION_OFFSET 0x8E5B
#define GL_MAX_FRAGMENT_INTERPOLATION_OFFSET 0x8E5C
#define GL_FRAGMENT_INTERPOLATION_OFFSET_BITS 0x8E5D
#define GL_MAX_VERTEX_STREAMS             0x8E71
#define GL_DOUBLE_VEC2                    0x8FFC
#define GL_DOUBLE_VEC3                    0x8FFD
#define GL_DOUBLE_VEC4                    0x8FFE
#define GL_DOUBLE_MAT2                    0x8F46
#define GL_DOUBLE_MAT3                    0x8F47
#define GL_DOUBLE_MAT4                    0x8F48
#define GL_DOUBLE_MAT2x3                  0x8F49
#define GL_DOUBLE_MAT2x4                  0x8F4A
#define GL_DOUBLE_MAT3x2                  0x8F4B
#define GL_DOUBLE_MAT3x4                  0x8F4C
#define GL_DOUBLE_MAT4x2                  0x8F4D
#define GL_DOUBLE_MAT4x3                  0x8F4E
#define GL_ACTIVE_SUBROUTINES             0x8DE5
#define GL_ACTIVE_SUBROUTINE_UNIFORMS     0x8DE6
#define GL_ACTIVE_SUBROUTINE_UNIFORM_LOCATIONS 0x8E47
#define GL_ACTIVE_SUBROUTINE_MAX_LENGTH   0x8E48
#define GL_ACTIVE_SUBROUTINE_UNIFORM_MAX_LENGTH 0x8E49
#define GL_MAX_SUBROUTINES                0x8DE7
#define GL_MAX_SUBROUTINE_UNIFORM_LOCATIONS 0x8DE8
#define GL_NUM_COMPATIBLE_SUBROUTINES     0x8E4A
#define GL_COMPATIBLE_SUBROUTINES         0x8E4B
#define GL_PATCHES                        0x000E
#define GL_PATCH_VERTICES                 0x8E72
#define GL_PATCH_DEFAULT_INNER_LEVEL      0x8E73
#define GL_PATCH_DEFAULT_OUTER_LEVEL      0x8E74
#define GL_TESS_CONTROL_OUTPUT_VERTICES   0x8E75
#define GL_TESS_GEN_MODE                  0x8E76
#define GL_TESS_GEN_SPACING               0x8E77
#define GL_TESS_GEN_VERTEX_ORDER          0x8E78
#define GL_TESS_GEN_POINT_MODE            0x8E79
#define GL_ISOLINES                       0x8E7A
#define GL_FRACTIONAL_ODD                 0x8E7B
#define GL_FRACTIONAL_EVEN                0x8E7C
#define GL_MAX_PATCH_VERTICES             0x8E7D
#define GL_MAX_TESS_GEN_LEVEL             0x8E7E
#define GL_MAX_TESS_CONTROL_UNIFORM_COMPONENTS 0x8E7F
#define GL_MAX_TESS_EVALUATION_UNIFORM_COMPONENTS 0x8E80
#define GL_MAX_TESS_CONTROL_TEXTURE_IMAGE_UNITS 0x8E81
#define GL_MAX_TESS_EVALUATION_TEXTURE_IMAGE_UNITS 0x8E82
#define GL_MAX_TESS_CONTROL_OUTPUT_COMPONENTS 0x8E83
#define GL_MAX_TESS_PATCH_COMPONENTS      0x8E84
#define GL_MAX_TESS_CONTROL_TOTAL_OUTPUT_COMPONENTS 0x8E85
#define GL_MAX_TESS_EVALUATION_OUTPUT_COMPONENTS 0x8E86
#define GL_MAX_TESS_CONTROL_UNIFORM_BLOCKS 0x8E89
#define GL_MAX_TESS_EVALUATION_UNIFORM_BLOCKS 0x8E8A
#define GL_MAX_TESS_CONTROL_INPUT_COMPONENTS 0x886C
#define GL_MAX_TESS_EVALUATION_INPUT_COMPONENTS 0x886D
#define GL_MAX_COMBINED_TESS_CONTROL_UNIFORM_COMPONENTS 0x8E1E
#define GL_MAX_COMBINED_TESS_EVALUATION_UNIFORM_COMPONENTS 0x8E1F
#define GL_UNIFORM_BLOCK_REFERENCED_BY_TESS_CONTROL_SHADER 0x84F0
#define GL_UNIFORM_BLOCK_REFERENCED_BY_TESS_EVALUATION_SHADER 0x84F1
#define GL_TESS_EVALUATION_SHADER         0x8E87
#define GL_TESS_CONTROL_SHADER            0x8E88
#define GL_TRANSFORM_FEEDBACK             0x8E22
#define GL_TRANSFORM_FEEDBACK_BUFFER_PAUSED 0x8E23
#define GL_TRANSFORM_FEEDBACK_BUFFER_ACTIVE 0x8E24
#define GL_TRANSFORM_FEEDBACK_BINDING     0x8E25
#define GL_MAX_TRANSFORM_FEEDBACK_BUFFERS 0x8E70
typedef void (APIENTRYP PFNGLMINSAMPLESHADINGPROC) (GLfloat value);
typedef void (APIENTRYP PFNGLBLENDEQUATIONIPROC) (GLuint buf, GLenum mode);
typedef void (APIENTRYP PFNGLBLENDEQUATIONSEPARATEIPROC) (GLuint buf, GLenum modeRGB, GLenum modeAlpha);
typedef void (APIENTRYP PFNGLBLENDFUNCIPROC) (GLuint buf, GLenum src, GLenum dst);
typedef void (APIENTRYP PFNGLBLENDFUNCSEPARATEIPROC) (GLuint buf, GLenum srcRGB, GLenum dstRGB, GLenum srcAlpha, GLenum dstAlpha);
typedef void (APIENTRYP PFNGLDRAWARRAYSINDIRECTPROC) (GLenum mode, const void *indirect);
typedef void (APIENTRYP PFNGLDRAWELEMENTSINDIRECTPROC) (GLenum mode, GLenum type, const void *indirect);
typedef void (APIENTRYP PFNGLUNIFORM1DPROC) (GLint location, GLdouble x);
typedef void (APIENTRYP PFNGLUNIFORM2DPROC) (GLint location, GLdouble x, GLdouble y);
typedef void (APIENTRYP PFNGLUNIFORM3DPROC) (GLint location, GLdouble x, GLdouble y, GLdouble z);
typedef void (APIENTRYP PFNGLUNIFORM4DPROC) (GLint location, GLdouble x, GLdouble y, GLdouble z, GLdouble w);
typedef void (APIENTRYP PFNGLUNIFORM1DVPROC) (GLint location, GLsizei count, const GLdouble *value);
typedef void (APIENTRYP PFNGLUNIFORM2DVPROC) (GLint location, GLsizei count, const GLdouble *value);
typedef void (APIENTRYP PFNGLUNIFORM3DVPROC) (GLint location, GLsizei count, const GLdouble *value);
typedef void (APIENTRYP PFNGLUNIFORM4DVPROC) (GLint location, GLsizei count, const GLdouble *value);
typedef void (APIENTRYP PFNGLUNIFORMMATRIX2DVPROC) (GLint location, GLsizei count, GLboolean transpose, const GLdouble *value);
typedef void (APIENTRYP PFNGLUNIFORMMATRIX3DVPROC) (GLint location, GLsizei count, GLboolean transpose, const GLdouble *value);
typedef void (APIENTRYP PFNGLUNIFORMMATRIX4DVPROC) (GLint location, GLsizei count, GLboolean transpose, const GLdouble *value);
typedef void (APIENTRYP PFNGLUNIFORMMATRIX2X3DVPROC) (GLint location, GLsizei count, GLboolean transpose, const GLdouble *value);
typedef void (APIENTRYP PFNGLUNIFORMMATRIX2X4DVPROC) (GLint location, GLsizei count, GLboolean transpose, const GLdouble *value);
typedef void (APIENTRYP PFNGLUNIFORMMATRIX3X2DVPROC) (GLint location, GLsizei count, GLboolean transpose, const GLdouble *value);
typedef void (APIENTRYP PFNGLUNIFORMMATRIX3X4DVPROC) (GLint location, GLsizei count, GLboolean transpose, const GLdouble *value);
typedef void (APIENTRYP PFNGLUNIFORMMATRIX4X2DVPROC) (GLint location, GLsizei count, GLboolean transpose, const GLdouble *value);
typedef void (APIENTRYP PFNGLUNIFORMMATRIX4X3DVPROC) (GLint location, GLsizei count, GLboolean transpose, const GLdouble *value);
typedef void (APIENTRYP PFNGLGETUNIFORMDVPROC) (GLuint program, GLint location, GLdouble *params);
typedef GLint (APIENTRYP PFNGLGETSUBROUTINEUNIFORMLOCATIONPROC) (GLuint program, GLenum shadertype, const GLchar *name);
typedef GLuint (APIENTRYP PFNGLGETSUBROUTINEINDEXPROC) (GLuint program, GLenum shadertype, const GLchar *name);
typedef void (APIENTRYP PFNGLGETACTIVESUBROUTINEUNIFORMIVPROC) (GLuint program, GLenum shadertype, GLuint index, GLenum pname, GLint *values);
typedef void (APIENTRYP PFNGLGETACTIVESUBROUTINEUNIFORMNAMEPROC) (GLuint program, GLenum shadertype, GLuint index, GLsizei bufSize, GLsizei *length, GLchar *name);
typedef void (APIENTRYP PFNGLGETACTIVESUBROUTINENAMEPROC) (GLuint program, GLenum shadertype, GLuint index, GLsizei bufSize, GLsizei *length, GLchar *name);
typedef void (APIENTRYP PFNGLUNIFORMSUBROUTINESUIVPROC) (GLenum shadertype, GLsizei count, const GLuint *indices);
typedef void (APIENTRYP PFNGLGETUNIFORMSUBROUTINEUIVPROC) (GLenum shadertype, GLint location, GLuint *params);
typedef void (APIENTRYP PFNGLGETPROGRAMSTAGEIVPROC) (GLuint program, GLenum shadertype, GLenum pname, GLint *values);
typedef void (APIENTRYP PFNGLPATCHPARAMETERIPROC) (GLenum pname, GLint value);
typedef void (APIENTRYP PFNGLPATCHPARAMETERFVPROC) (GLenum pname, const GLfloat *values);
typedef void (APIENTRYP PFNGLBINDTRANSFORMFEEDBACKPROC) (GLenum target, GLuint id);
typedef void (APIENTRYP PFNGLDELETETRANSFORMFEEDBACKSPROC) (GLsizei n, const GLuint *ids);
typedef void (APIENTRYP PFNGLGENTRANSFORMFEEDBACKSPROC) (GLsizei n, GLuint *ids);
typedef GLboolean (APIENTRYP PFNGLISTRANSFORMFEEDBACKPROC) (GLuint id);
typedef void (APIENTRYP PFNGLPAUSETRANSFORMFEEDBACKPROC) (void);
typedef void (APIENTRYP PFNGLRESUMETRANSFORMFEEDBACKPROC) (void);
typedef void (APIENTRYP PFNGLDRAWTRANSFORMFEEDBACKPROC) (GLenum mode, GLuint id);
typedef void (APIENTRYP PFNGLDRAWTRANSFORMFEEDBACKSTREAMPROC) (GLenum mode, GLuint id, GLuint stream);
typedef void (APIENTRYP PFNGLBEGINQUERYINDEXEDPROC) (GLenum target, GLuint index, GLuint id);
typedef void (APIENTRYP PFNGLENDQUERYINDEXEDPROC) (GLenum target, GLuint index);
typedef void (APIENTRYP PFNGLGETQUERYINDEXEDIVPROC) (GLenum target, GLuint index, GLenum pname, GLint *params);
#endif /* GL_VERSION_4_0 */
#ifndef GL_VERSION_4_1
#define GL_VERSION_4_1 1
#define GL_FIXED                          0x140C
#define GL_IMPLEMENTATION_COLOR_READ_TYPE 0x8B9A
#define GL_IMPLEMENTATION_COLOR_READ_FORMAT 0x8B9B
#define GL_LOW_FLOAT                      0x8DF0
#define GL_MEDIUM_FLOAT                   0x8DF1
#define GL_HIGH_FLOAT                     0x8DF2
#define GL_LOW_INT                        0x8DF3
#define GL_MEDIUM_INT                     0x8DF4
#define GL_HIGH_INT                       0x8DF5
#define GL_SHADER_COMPILER                0x8DFA
#define GL_SHADER_BINARY_FORMATS          0x8DF8
#define GL_NUM_SHADER_BINARY_FORMATS      0x8DF9
#define GL_MAX_VERTEX_UNIFORM_VECTORS     0x8DFB
#define GL_MAX_VARYING_VECTORS            0x8DFC
#define GL_MAX_FRAGMENT_UNIFORM_VECTORS   0x8DFD
#define GL_RGB565                         0x8D62
#define GL_PROGRAM_BINARY_RETRIEVABLE_HINT 0x8257
#define GL_PROGRAM_BINARY_LENGTH          0x8741
#define GL_NUM_PROGRAM_BINARY_FORMATS     0x87FE
#define GL_PROGRAM_BINARY_FORMATS         0x87FF
#define GL_VERTEX_SHADER_BIT              0x00000001
#define GL_FRAGMENT_SHADER_BIT            0x00000002
#define GL_GEOMETRY_SHADER_BIT            0x00000004
#define GL_TESS_CONTROL_SHADER_BIT        0x00000008
#define GL_TESS_EVALUATION_SHADER_BIT     0x00000010
#define GL_ALL_SHADER_BITS                0xFFFFFFFF
#define GL_PROGRAM_SEPARABLE              0x8258
#define GL_ACTIVE_PROGRAM                 0x8259
#define GL_PROGRAM_PIPELINE_BINDING       0x825A
#define GL_MAX_VIEWPORTS                  0x825B
#define GL_VIEWPORT_SUBPIXEL_BITS         0x825C
#define GL_VIEWPORT_BOUNDS_RANGE          0x825D
#define GL_LAYER_PROVOKING_VERTEX         0x825E
#define GL_VIEWPORT_INDEX_PROVOKING_VERTEX 0x825F
#define GL_UNDEFINED_VERTEX               0x8260
typedef void (APIENTRYP PFNGLRELEASESHADERCOMPILERPROC) (void);
typedef void (APIENTRYP PFNGLSHADERBINARYPROC) (GLsizei count, const GLuint *shaders, GLenum binaryformat, const void *binary, GLsizei length);
typedef void (APIENTRYP PFNGLGETSHADERPRECISIONFORMATPROC) (GLenum shadertype, GLenum precisiontype, GLint *range, GLint *precision);
typedef void (APIENTRYP PFNGLDEPTHRANGEFPROC) (GLfloat n, GLfloat f);
typedef void (APIENTRYP PFNGLCLEARDEPTHFPROC) (GLfloat d);
typedef void (APIENTRYP PFNGLGETPROGRAMBINARYPROC) (GLuint program, GLsizei bufSize, GLsizei *length, GLenum *binaryFormat, void *binary);
typedef void (APIENTRYP PFNGLPROGRAMBINARYPROC) (GLuint program, GLenum binaryFormat, const void *binary, GLsizei length);
typedef void (APIENTRYP PFNGLPROGRAMPARAMETERIPROC) (GLuint program, GLenum pname, GLint value);
typedef void (APIENTRYP PFNGLUSEPROGRAMSTAGESPROC) (GLuint pipeline, GLbitfield stages, GLuint program);
typedef void (APIENTRYP PFNGLACTIVESHADERPROGRAMPROC) (GLuint pipeline, GLuint program);
typedef GLuint (APIENTRYP PFNGLCREATESHADERPROGRAMVPROC) (GLenum type, GLsizei count, const GLchar *const*strings);
typedef void (APIENTRYP PFNGLBINDPROGRAMPIPELINEPROC) (GLuint pipeline);
typedef void (APIENTRYP PFNGLDELETEPROGRAMPIPELINESPROC) (GLsizei n, const GLuint *pipelines);
typedef void (APIENTRYP PFNGLGENPROGRAMPIPELINESPROC) (GLsizei n, GLuint *pipelines);
typedef GLboolean (APIENTRYP PFNGLISPROGRAMPIPELINEPROC) (GLuint pipeline);
typedef void (APIENTRYP PFNGLGETPROGRAMPIPELINEIVPROC) (GLuint pipeline, GLenum pname, GLint *params);
typedef void (APIENTRYP PFNGLPROGRAMUNIFORM1IPROC) (GLuint program, GLint location, GLint v0);
typedef void (APIENTRYP PFNGLPROGRAMUNIFORM1IVPROC) (GLuint program, GLint location, GLsizei count, const GLint *value);
typedef void (APIENTRYP PFNGLPROGRAMUNIFORM1FPROC) (GLuint program, GLint location, GLfloat v0);
typedef void (APIENTRYP PFNGLPROGRAMUNIFORM1FVPROC) (GLuint program, GLint location, GLsizei count, const GLfloat *value);
typedef void (APIENTRYP PFNGLPROGRAMUNIFORM1DPROC) (GLuint program, GLint location, GLdouble v0);
typedef void (APIENTRYP PFNGLPROGRAMUNIFORM1DVPROC) (GLuint program, GLint location, GLsizei count, const GLdouble *value);
typedef void (APIENTRYP PFNGLPROGRAMUNIFORM1UIPROC) (GLuint program, GLint location, GLuint v0);
typedef void (APIENTRYP PFNGLPROGRAMUNIFORM1UIVPROC) (GLuint program, GLint location, GLsizei count, const GLuint *value);
typedef void (APIENTRYP PFNGLPROGRAMUNIFORM2IPROC) (GLuint program, GLint location, GLint v0, GLint v1);
typedef void (APIENTRYP PFNGLPROGRAMUNIFORM2IVPROC) (GLuint program, GLint location, GLsizei count, const GLint *value);
typedef void (APIENTRYP PFNGLPROGRAMUNIFORM2FPROC) (GLuint program, GLint location, GLfloat v0, GLfloat v1);
typedef void (APIENTRYP PFNGLPROGRAMUNIFORM2FVPROC) (GLuint program, GLint location, GLsizei count, const GLfloat *value);
typedef void (APIENTRYP PFNGLPROGRAMUNIFORM2DPROC) (GLuint program, GLint location, GLdouble v0, GLdouble v1);
typedef void (APIENTRYP PFNGLPROGRAMUNIFORM2DVPROC) (GLuint program, GLint location, GLsizei count, const GLdouble *value);
typedef void (APIENTRYP PFNGLPROGRAMUNIFORM2UIPROC) (GLuint program, GLint location, GLuint v0, GLuint v1);
typedef void (APIENTRYP PFNGLPROGRAMUNIFORM2UIVPROC) (GLuint program, GLint location, GLsizei count, const GLuint *value);
typedef void (APIENTRYP PFNGLPROGRAMUNIFORM3IPROC) (GLuint program, GLint location, GLint v0, GLint v1, GLint v2);
typedef void (APIENTRYP PFNGLPROGRAMUNIFORM3IVPROC) (GLuint program, GLint location, GLsizei count, const GLint *value);
typedef void (APIENTRYP PFNGLPROGRAMUNIFORM3FPROC) (GLuint program, GLint location, GLfloat v0, GLfloat v1, GLfloat v2);
typedef void (APIENTRYP PFNGLPROGRAMUNIFORM3FVPROC) (GLuint program, GLint location, GLsizei count, const GLfloat *value);
typedef void (APIENTRYP PFNGLPROGRAMUNIFORM3DPROC) (GLuint program, GLint location, GLdouble v0, GLdouble v1, GLdouble v2);
typedef void (APIENTRYP PFNGLPROGRAMUNIFORM3DVPROC) (GLuint program, GLint location, GLsizei count, const GLdouble *value);
typedef void (APIENTRYP PFNGLPROGRAMUNIFORM3UIPROC) (GLuint program, GLint location, GLuint v0, GLuint v1, GLuint v2);
typedef void (APIENTRYP PFNGLPROGRAMUNIFORM3UIVPROC) (GLuint program, GLint location, GLsizei count, const GLuint *value);
typedef void (APIENTRYP PFNGLPROGRAMUNIFORM4IPROC) (GLuint program, GLint location, GLint v0, GLint v1, GLint v2, GLint v3);
typedef void (APIENTRYP PFNGLPROGRAMUNIFORM4IVPROC) (GLuint program, GLint location, GLsizei count, const GLint *value);
typedef void (APIENTRYP PFNGLPROGRAMUNIFORM4FPROC) (GLuint program, GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3);
typedef void (APIENTRYP PFNGLPROGRAMUNIFORM4FVPROC) (GLuint program, GLint location, GLsizei count, const GLfloat *value);
typedef void (APIENTRYP PFNGLPROGRAMUNIFORM4DPROC) (GLuint program, GLint location, GLdouble v0, GLdouble v1, GLdouble v2, GLdouble v3);
typedef void (APIENTRYP PFNGLPROGRAMUNIFORM4DVPROC) (GLuint program, GLint location, GLsizei count, const GLdouble *value);
typedef void (APIENTRYP PFNGLPROGRAMUNIFORM4UIPROC) (GLuint program, GLint location, GLuint v0, GLuint v1, GLuint v2, GLuint v3);
typedef void (APIENTRYP PFNGLPROGRAMUNIFORM4UIVPROC) (GLuint program, GLint location, GLsizei count, const GLuint *value);
typedef void (APIENTRYP PFNGLPROGRAMUNIFORMMATRIX2FVPROC) (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
typedef void (APIENTRYP PFNGLPROGRAMUNIFORMMATRIX3FVPROC) (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
typedef void (APIENTRYP PFNGLPROGRAMUNIFORMMATRIX4FVPROC) (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
typedef void (APIENTRYP PFNGLPROGRAMUNIFORMMATRIX2DVPROC) (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value);
typedef void (APIENTRYP PFNGLPROGRAMUNIFORMMATRIX3DVPROC) (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value);
typedef void (APIENTRYP PFNGLPROGRAMUNIFORMMATRIX4DVPROC) (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value);
typedef void (APIENTRYP PFNGLPROGRAMUNIFORMMATRIX2X3FVPROC) (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
typedef void (APIENTRYP PFNGLPROGRAMUNIFORMMATRIX3X2FVPROC) (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
typedef void (APIENTRYP PFNGLPROGRAMUNIFORMMATRIX2X4FVPROC) (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
typedef void (APIENTRYP PFNGLPROGRAMUNIFORMMATRIX4X2FVPROC) (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
typedef void (APIENTRYP PFNGLPROGRAMUNIFORMMATRIX3X4FVPROC) (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
typedef void (APIENTRYP PFNGLPROGRAMUNIFORMMATRIX4X3FVPROC) (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
typedef void (APIENTRYP PFNGLPROGRAMUNIFORMMATRIX2X3DVPROC) (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value);
typedef void (APIENTRYP PFNGLPROGRAMUNIFORMMATRIX3X2DVPROC) (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value);
typedef void (APIENTRYP PFNGLPROGRAMUNIFORMMATRIX2X4DVPROC) (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value);
typedef void (APIENTRYP PFNGLPROGRAMUNIFORMMATRIX4X2DVPROC) (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value);
typedef void (APIENTRYP PFNGLPROGRAMUNIFORMMATRIX3X4DVPROC) (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value);
typedef void (APIENTRYP PFNGLPROGRAMUNIFORMMATRIX4X3DVPROC) (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value);
typedef void (APIENTRYP PFNGLVALIDATEPROGRAMPIPELINEPROC) (GLuint pipeline);
typedef void (APIENTRYP PFNGLGETPROGRAMPIPELINEINFOLOGPROC) (GLuint pipeline, GLsizei bufSize, GLsizei *length, GLchar *infoLog);
typedef void (APIENTRYP PFNGLVERTEXATTRIBL1DPROC) (GLuint index, GLdouble x);
typedef void (APIENTRYP PFNGLVERTEXATTRIBL2DPROC) (GLuint index, GLdouble x, GLdouble y);
typedef void (APIENTRYP PFNGLVERTEXATTRIBL3DPROC) (GLuint index, GLdouble x, GLdouble y, GLdouble z);
typedef void (APIENTRYP PFNGLVERTEXATTRIBL4DPROC) (GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w);
typedef void (APIENTRYP PFNGLVERTEXATTRIBL1DVPROC) (GLuint index, const GLdouble *v);
typedef void (APIENTRYP PFNGLVERTEXATTRIBL2DVPROC) (GLuint index, const GLdouble *v);
typedef void (APIENTRYP PFNGLVERTEXATTRIBL3DVPROC) (GLuint index, const GLdouble *v);
typedef void (APIENTRYP PFNGLVERTEXATTRIBL4DVPROC) (GLuint index, const GLdouble *v);
typedef void (APIENTRYP PFNGLVERTEXATTRIBLPOINTERPROC) (GLuint index, GLint size, GLenum type, GLsizei stride, const void *pointer);
typedef void (APIENTRYP PFNGLGETVERTEXATTRIBLDVPROC) (GLuint index, GLenum pname, GLdouble *params);
typedef void (APIENTRYP PFNGLVIEWPORTARRAYVPROC) (GLuint first, GLsizei count, const GLfloat *v);
typedef void (APIENTRYP PFNGLVIEWPORTINDEXEDFPROC) (GLuint index, GLfloat x, GLfloat y, GLfloat w, GLfloat h);
typedef void (APIENTRYP PFNGLVIEWPORTINDEXEDFVPROC) (GLuint index, const GLfloat *v);
typedef void (APIENTRYP PFNGLSCISSORARRAYVPROC) (GLuint first, GLsizei count, const GLint *v);
typedef void (APIENTRYP PFNGLSCISSORINDEXEDPROC) (GLuint index, GLint left, GLint bottom, GLsizei width, GLsizei height);
typedef void (APIENTRYP PFNGLSCISSORINDEXEDVPROC) (GLuint index, const GLint *v);
typedef void (APIENTRYP PFNGLDEPTHRANGEARRAYVPROC) (GLuint first, GLsizei count, const GLdouble *v);
typedef void (APIENTRYP PFNGLDEPTHRANGEINDEXEDPROC) (GLuint index, GLdouble n, GLdouble f);
typedef void (APIENTRYP PFNGLGETFLOATI_VPROC) (GLenum target, GLuint index, GLfloat *data);
typedef void (APIENTRYP PFNGLGETDOUBLEI_VPROC) (GLenum target, GLuint index, GLdouble *data);
#endif /* GL_VERSION_4_1 */
#ifndef GL_VERSION_4_2
#define GL_VERSION_4_2 1
#define GL_COPY_READ_BUFFER_BINDING       0x8F36
#define GL_COPY_WRITE_BUFFER_BINDING      0x8F37
#define GL_TRANSFORM_FEEDBACK_ACTIVE      0x8E24
#define GL_TRANSFORM_FEEDBACK_PAUSED      0x8E23
#define GL_UNPACK_COMPRESSED_BLOCK_WIDTH  0x9127
#define GL_UNPACK_COMPRESSED_BLOCK_HEIGHT 0x9128
#define GL_UNPACK_COMPRESSED_BLOCK_DEPTH  0x9129
#define GL_UNPACK_COMPRESSED_BLOCK_SIZE   0x912A
#define GL_PACK_COMPRESSED_BLOCK_WIDTH    0x912B
#define GL_PACK_COMPRESSED_BLOCK_HEIGHT   0x912C
#define GL_PACK_COMPRESSED_BLOCK_DEPTH    0x912D
#define GL_PACK_COMPRESSED_BLOCK_SIZE     0x912E
#define GL_NUM_SAMPLE_COUNTS              0x9380
#define GL_MIN_MAP_BUFFER_ALIGNMENT       0x90BC
#define GL_ATOMIC_COUNTER_BUFFER          0x92C0
#define GL_ATOMIC_COUNTER_BUFFER_BINDING  0x92C1
#define GL_ATOMIC_COUNTER_BUFFER_START    0x92C2
#define GL_ATOMIC_COUNTER_BUFFER_SIZE     0x92C3
#define GL_ATOMIC_COUNTER_BUFFER_DATA_SIZE 0x92C4
#define GL_ATOMIC_COUNTER_BUFFER_ACTIVE_ATOMIC_COUNTERS 0x92C5
#define GL_ATOMIC_COUNTER_BUFFER_ACTIVE_ATOMIC_COUNTER_INDICES 0x92C6
#define GL_ATOMIC_COUNTER_BUFFER_REFERENCED_BY_VERTEX_SHADER 0x92C7
#define GL_ATOMIC_COUNTER_BUFFER_REFERENCED_BY_TESS_CONTROL_SHADER 0x92C8
#define GL_ATOMIC_COUNTER_BUFFER_REFERENCED_BY_TESS_EVALUATION_SHADER 0x92C9
#define GL_ATOMIC_COUNTER_BUFFER_REFERENCED_BY_GEOMETRY_SHADER 0x92CA
#define GL_ATOMIC_COUNTER_BUFFER_REFERENCED_BY_FRAGMENT_SHADER 0x92CB
#define GL_MAX_VERTEX_ATOMIC_COUNTER_BUFFERS 0x92CC
#define GL_MAX_TESS_CONTROL_ATOMIC_COUNTER_BUFFERS 0x92CD
#define GL_MAX_TESS_EVALUATION_ATOMIC_COUNTER_BUFFERS 0x92CE
#define GL_MAX_GEOMETRY_ATOMIC_COUNTER_BUFFERS 0x92CF
#define GL_MAX_FRAGMENT_ATOMIC_COUNTER_BUFFERS 0x92D0
#define GL_MAX_COMBINED_ATOMIC_COUNTER_BUFFERS 0x92D1
#define GL_MAX_VERTEX_ATOMIC_COUNTERS     0x92D2
#define GL_MAX_TESS_CONTROL_ATOMIC_COUNTERS 0x92D3
#define GL_MAX_TESS_EVALUATION_ATOMIC_COUNTERS 0x92D4
#define GL_MAX_GEOMETRY_ATOMIC_COUNTERS   0x92D5
#define GL_MAX_FRAGMENT_ATOMIC_COUNTERS   0x92D6
#define GL_MAX_COMBINED_ATOMIC_COUNTERS   0x92D7
#define GL_MAX_ATOMIC_COUNTER_BUFFER_SIZE 0x92D8
#define GL_MAX_ATOMIC_COUNTER_BUFFER_BINDINGS 0x92DC
#define GL_ACTIVE_ATOMIC_COUNTER_BUFFERS  0x92D9
#define GL_UNIFORM_ATOMIC_COUNTER_BUFFER_INDEX 0x92DA
#define GL_UNSIGNED_INT_ATOMIC_COUNTER    0x92DB
#define GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT 0x00000001
#define GL_ELEMENT_ARRAY_BARRIER_BIT      0x00000002
#define GL_UNIFORM_BARRIER_BIT            0x00000004
#define GL_TEXTURE_FETCH_BARRIER_BIT      0x00000008
#define GL_SHADER_IMAGE_ACCESS_BARRIER_BIT 0x00000020
#define GL_COMMAND_BARRIER_BIT            0x00000040
#define GL_PIXEL_BUFFER_BARRIER_BIT       0x00000080
#define GL_TEXTURE_UPDATE_BARRIER_BIT     0x00000100
#define GL_BUFFER_UPDATE_BARRIER_BIT      0x00000200
#define GL_FRAMEBUFFER_BARRIER_BIT        0x00000400
#define GL_TRANSFORM_FEEDBACK_BARRIER_BIT 0x00000800
#define GL_ATOMIC_COUNTER_BARRIER_BIT     0x00001000
#define GL_ALL_BARRIER_BITS               0xFFFFFFFF
#define GL_MAX_IMAGE_UNITS                0x8F38
#define GL_MAX_COMBINED_IMAGE_UNITS_AND_FRAGMENT_OUTPUTS 0x8F39
#define GL_IMAGE_BINDING_NAME             0x8F3A
#define GL_IMAGE_BINDING_LEVEL            0x8F3B
#define GL_IMAGE_BINDING_LAYERED          0x8F3C
#define GL_IMAGE_BINDING_LAYER            0x8F3D
#define GL_IMAGE_BINDING_ACCESS           0x8F3E
#define GL_IMAGE_1D                       0x904C
#define GL_IMAGE_2D                       0x904D
#define GL_IMAGE_3D                       0x904E
#define GL_IMAGE_2D_RECT                  0x904F
#define GL_IMAGE_CUBE                     0x9050
#define GL_IMAGE_BUFFER                   0x9051
#define GL_IMAGE_1D_ARRAY                 0x9052
#define GL_IMAGE_2D_ARRAY                 0x9053
#define GL_IMAGE_CUBE_MAP_ARRAY           0x9054
#define GL_IMAGE_2D_MULTISAMPLE           0x9055
#define GL_IMAGE_2D_MULTISAMPLE_ARRAY     0x9056
#define GL_INT_IMAGE_1D                   0x9057
#define GL_INT_IMAGE_2D                   0x9058
#define GL_INT_IMAGE_3D                   0x9059
#define GL_INT_IMAGE_2D_RECT              0x905A
#define GL_INT_IMAGE_CUBE                 0x905B
#define GL_INT_IMAGE_BUFFER               0x905C
#define GL_INT_IMAGE_1D_ARRAY             0x905D
#define GL_INT_IMAGE_2D_ARRAY             0x905E
#define GL_INT_IMAGE_CUBE_MAP_ARRAY       0x905F
#define GL_INT_IMAGE_2D_MULTISAMPLE       0x9060
#define GL_INT_IMAGE_2D_MULTISAMPLE_ARRAY 0x9061
#define GL_UNSIGNED_INT_IMAGE_1D          0x9062
#define GL_UNSIGNED_INT_IMAGE_2D          0x9063
#define GL_UNSIGNED_INT_IMAGE_3D          0x9064
#define GL_UNSIGNED_INT_IMAGE_2D_RECT     0x9065
#define GL_UNSIGNED_INT_IMAGE_CUBE        0x9066
#define GL_UNSIGNED_INT_IMAGE_BUFFER      0x9067
#define GL_UNSIGNED_INT_IMAGE_1D_ARRAY    0x9068
#define GL_UNSIGNED_INT_IMAGE_2D_ARRAY    0x9069
#define GL_UNSIGNED_INT_IMAGE_CUBE_MAP_ARRAY 0x906A
#define GL_UNSIGNED_INT_IMAGE_2D_MULTISAMPLE 0x906B
#define GL_UNSIGNED_INT_IMAGE_2D_MULTISAMPLE_ARRAY 0x906C
#define GL_MAX_IMAGE_SAMPLES              0x906D
#define GL_IMAGE_BINDING_FORMAT           0x906E
#define GL_IMAGE_FORMAT_COMPATIBILITY_TYPE 0x90C7
#define GL_IMAGE_FORMAT_COMPATIBILITY_BY_SIZE 0x90C8
#define GL_IMAGE_FORMAT_COMPATIBILITY_BY_CLASS 0x90C9
#define GL_MAX_VERTEX_IMAGE_UNIFORMS      0x90CA
#define GL_MAX_TESS_CONTROL_IMAGE_UNIFORMS 0x90CB
#define GL_MAX_TESS_EVALUATION_IMAGE_UNIFORMS 0x90CC
#define GL_MAX_GEOMETRY_IMAGE_UNIFORMS    0x90CD
#define GL_MAX_FRAGMENT_IMAGE_UNIFORMS    0x90CE
#define GL_MAX_COMBINED_IMAGE_UNIFORMS    0x90CF
#define GL_COMPRESSED_RGBA_BPTC_UNORM     0x8E8C
#define GL_COMPRESSED_SRGB_ALPHA_BPTC_UNORM 0x8E8D
#define GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT 0x8E8E
#define GL_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT 0x8E8F
#define GL_TEXTURE_IMMUTABLE_FORMAT       0x912F
typedef void (APIENTRYP PFNGLDRAWARRAYSINSTANCEDBASEINSTANCEPROC) (GLenum mode, GLint first, GLsizei count, GLsizei instancecount, GLuint baseinstance);
typedef void (APIENTRYP PFNGLDRAWELEMENTSINSTANCEDBASEINSTANCEPROC) (GLenum mode, GLsizei count, GLenum type, const void *indices, GLsizei instancecount, GLuint baseinstance);
typedef void (APIENTRYP PFNGLDRAWELEMENTSINSTANCEDBASEVERTEXBASEINSTANCEPROC) (GLenum mode, GLsizei count, GLenum type, const void *indices, GLsizei instancecount, GLint basevertex, GLuint baseinstance);
typedef void (APIENTRYP PFNGLGETINTERNALFORMATIVPROC) (GLenum target, GLenum internalformat, GLenum pname, GLsizei count, GLint *params);
typedef void (APIENTRYP PFNGLGETACTIVEATOMICCOUNTERBUFFERIVPROC) (GLuint program, GLuint bufferIndex, GLenum pname, GLint *params);
typedef void (APIENTRYP PFNGLBINDIMAGETEXTUREPROC) (GLuint unit, GLuint texture, GLint level, GLboolean layered, GLint layer, GLenum access, GLenum format);
typedef void (APIENTRYP PFNGLMEMORYBARRIERPROC) (GLbitfield barriers);
typedef void (APIENTRYP PFNGLTEXSTORAGE1DPROC) (GLenum target, GLsizei levels, GLenum internalformat, GLsizei width);
typedef void (APIENTRYP PFNGLTEXSTORAGE2DPROC) (GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height);
typedef void (APIENTRYP PFNGLTEXSTORAGE3DPROC) (GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth);
typedef void (APIENTRYP PFNGLDRAWTRANSFORMFEEDBACKINSTANCEDPROC) (GLenum mode, GLuint id, GLsizei instancecount);
typedef void (APIENTRYP PFNGLDRAWTRANSFORMFEEDBACKSTREAMINSTANCEDPROC) (GLenum mode, GLuint id, GLuint stream, GLsizei instancecount);
#endif /* GL_VERSION_4_2 */
#ifndef GL_VERSION_4_3
#define GL_VERSION_4_3 1
typedef void (APIENTRY  *GLDEBUGPROC)(GLenum source,GLenum type,GLuint id,GLenum severity,GLsizei length,const GLchar *message,const void *userParam);
#define GL_NUM_SHADING_LANGUAGE_VERSIONS  0x82E9
#define GL_VERTEX_ATTRIB_ARRAY_LONG       0x874E
#define GL_COMPRESSED_RGB8_ETC2           0x9274
#define GL_COMPRESSED_SRGB8_ETC2          0x9275
#define GL_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2 0x9276
#define GL_COMPRESSED_SRGB8_PUNCHTHROUGH_ALPHA1_ETC2 0x9277
#define GL_COMPRESSED_RGBA8_ETC2_EAC      0x9278
#define GL_COMPRESSED_SRGB8_ALPHA8_ETC2_EAC 0x9279
#define GL_COMPRESSED_R11_EAC             0x9270
#define GL_COMPRESSED_SIGNED_R11_EAC      0x9271
#define GL_COMPRESSED_RG11_EAC            0x9272
#define GL_COMPRESSED_SIGNED_RG11_EAC     0x9273
#define GL_PRIMITIVE_RESTART_FIXED_INDEX  0x8D69
#define GL_ANY_SAMPLES_PASSED_CONSERVATIVE 0x8D6A
#define GL_MAX_ELEMENT_INDEX              0x8D6B
#define GL_COMPUTE_SHADER                 0x91B9
#define GL_MAX_COMPUTE_UNIFORM_BLOCKS     0x91BB
#define GL_MAX_COMPUTE_TEXTURE_IMAGE_UNITS 0x91BC
#define GL_MAX_COMPUTE_IMAGE_UNIFORMS     0x91BD
#define GL_MAX_COMPUTE_SHARED_MEMORY_SIZE 0x8262
#define GL_MAX_COMPUTE_UNIFORM_COMPONENTS 0x8263
#define GL_MAX_COMPUTE_ATOMIC_COUNTER_BUFFERS 0x8264
#define GL_MAX_COMPUTE_ATOMIC_COUNTERS    0x8265
#define GL_MAX_COMBINED_COMPUTE_UNIFORM_COMPONENTS 0x8266
#define GL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS 0x90EB
#define GL_MAX_COMPUTE_WORK_GROUP_COUNT   0x91BE
#define GL_MAX_COMPUTE_WORK_GROUP_SIZE    0x91BF
#define GL_COMPUTE_WORK_GROUP_SIZE        0x8267
#define GL_UNIFORM_BLOCK_REFERENCED_BY_COMPUTE_SHADER 0x90EC
#define GL_ATOMIC_COUNTER_BUFFER_REFERENCED_BY_COMPUTE_SHADER 0x90ED
#define GL_DISPATCH_INDIRECT_BUFFER       0x90EE
#define GL_DISPATCH_INDIRECT_BUFFER_BINDING 0x90EF
#define GL_COMPUTE_SHADER_BIT             0x00000020
#define GL_DEBUG_OUTPUT_SYNCHRONOUS       0x8242
#define GL_DEBUG_NEXT_LOGGED_MESSAGE_LENGTH 0x8243
#define GL_DEBUG_CALLBACK_FUNCTION        0x8244
#define GL_DEBUG_CALLBACK_USER_PARAM      0x8245
#define GL_DEBUG_SOURCE_API               0x8246
#define GL_DEBUG_SOURCE_WINDOW_SYSTEM     0x8247
#define GL_DEBUG_SOURCE_SHADER_COMPILER   0x8248
#define GL_DEBUG_SOURCE_THIRD_PARTY       0x8249
#define GL_DEBUG_SOURCE_APPLICATION       0x824A
#define GL_DEBUG_SOURCE_OTHER             0x824B
#define GL_DEBUG_TYPE_ERROR               0x824C
#define GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR 0x824D
#define GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR  0x824E
#define GL_DEBUG_TYPE_PORTABILITY         0x824F
#define GL_DEBUG_TYPE_PERFORMANCE         0x8250
#define GL_DEBUG_TYPE_OTHER               0x8251
#define GL_MAX_DEBUG_MESSAGE_LENGTH       0x9143
#define GL_MAX_DEBUG_LOGGED_MESSAGES      0x9144
#define GL_DEBUG_LOGGED_MESSAGES          0x9145
#define GL_DEBUG_SEVERITY_HIGH            0x9146
#define GL_DEBUG_SEVERITY_MEDIUM          0x9147
#define GL_DEBUG_SEVERITY_LOW             0x9148
#define GL_DEBUG_TYPE_MARKER              0x8268
#define GL_DEBUG_TYPE_PUSH_GROUP          0x8269
#define GL_DEBUG_TYPE_POP_GROUP           0x826A
#define GL_DEBUG_SEVERITY_NOTIFICATION    0x826B
#define GL_MAX_DEBUG_GROUP_STACK_DEPTH    0x826C
#define GL_DEBUG_GROUP_STACK_DEPTH        0x826D
#define GL_BUFFER                         0x82E0
#define GL_SHADER                         0x82E1
#define GL_PROGRAM                        0x82E2
#define GL_QUERY                          0x82E3
#define GL_PROGRAM_PIPELINE               0x82E4
#define GL_SAMPLER                        0x82E6
#define GL_MAX_LABEL_LENGTH               0x82E8
#define GL_DEBUG_OUTPUT                   0x92E0
#define GL_CONTEXT_FLAG_DEBUG_BIT         0x00000002
#define GL_MAX_UNIFORM_LOCATIONS          0x826E
#define GL_FRAMEBUFFER_DEFAULT_WIDTH      0x9310
#define GL_FRAMEBUFFER_DEFAULT_HEIGHT     0x9311
#define GL_FRAMEBUFFER_DEFAULT_LAYERS     0x9312
#define GL_FRAMEBUFFER_DEFAULT_SAMPLES    0x9313
#define GL_FRAMEBUFFER_DEFAULT_FIXED_SAMPLE_LOCATIONS 0x9314
#define GL_MAX_FRAMEBUFFER_WIDTH          0x9315
#define GL_MAX_FRAMEBUFFER_HEIGHT         0x9316
#define GL_MAX_FRAMEBUFFER_LAYERS         0x9317
#define GL_MAX_FRAMEBUFFER_SAMPLES        0x9318
#define GL_INTERNALFORMAT_SUPPORTED       0x826F
#define GL_INTERNALFORMAT_PREFERRED       0x8270
#define GL_INTERNALFORMAT_RED_SIZE        0x8271
#define GL_INTERNALFORMAT_GREEN_SIZE      0x8272
#define GL_INTERNALFORMAT_BLUE_SIZE       0x8273
#define GL_INTERNALFORMAT_ALPHA_SIZE      0x8274
#define GL_INTERNALFORMAT_DEPTH_SIZE      0x8275
#define GL_INTERNALFORMAT_STENCIL_SIZE    0x8276
#define GL_INTERNALFORMAT_SHARED_SIZE     0x8277
#define GL_INTERNALFORMAT_RED_TYPE        0x8278
#define GL_INTERNALFORMAT_GREEN_TYPE      0x8279
#define GL_INTERNALFORMAT_BLUE_TYPE       0x827A
#define GL_INTERNALFORMAT_ALPHA_TYPE      0x827B
#define GL_INTERNALFORMAT_DEPTH_TYPE      0x827C
#define GL_INTERNALFORMAT_STENCIL_TYPE    0x827D
#define GL_MAX_WIDTH                      0x827E
#define GL_MAX_HEIGHT                     0x827F
#define GL_MAX_DEPTH                      0x8280
#define GL_MAX_LAYERS                     0x8281
#define GL_MAX_COMBINED_DIMENSIONS        0x8282
#define GL_COLOR_COMPONENTS               0x8283
#define GL_DEPTH_COMPONENTS               0x8284
#define GL_STENCIL_COMPONENTS             0x8285
#define GL_COLOR_RENDERABLE               0x8286
#define GL_DEPTH_RENDERABLE               0x8287
#define GL_STENCIL_RENDERABLE             0x8288
#define GL_FRAMEBUFFER_RENDERABLE         0x8289
#define GL_FRAMEBUFFER_RENDERABLE_LAYERED 0x828A
#define GL_FRAMEBUFFER_BLEND              0x828B
#define GL_READ_PIXELS                    0x828C
#define GL_READ_PIXELS_FORMAT             0x828D
#define GL_READ_PIXELS_TYPE               0x828E
#define GL_TEXTURE_IMAGE_FORMAT           0x828F
#define GL_TEXTURE_IMAGE_TYPE             0x8290
#define GL_GET_TEXTURE_IMAGE_FORMAT       0x8291
#define GL_GET_TEXTURE_IMAGE_TYPE         0x8292
#define GL_MIPMAP                         0x8293
#define GL_MANUAL_GENERATE_MIPMAP         0x8294
#define GL_AUTO_GENERATE_MIPMAP           0x8295
#define GL_COLOR_ENCODING                 0x8296
#define GL_SRGB_READ                      0x8297
#define GL_SRGB_WRITE                     0x8298
#define GL_FILTER                         0x829A
#define GL_VERTEX_TEXTURE                 0x829B
#define GL_TESS_CONTROL_TEXTURE           0x829C
#define GL_TESS_EVALUATION_TEXTURE        0x829D
#define GL_GEOMETRY_TEXTURE               0x829E
#define GL_FRAGMENT_TEXTURE               0x829F
#define GL_COMPUTE_TEXTURE                0x82A0
#define GL_TEXTURE_SHADOW                 0x82A1
#define GL_TEXTURE_GATHER                 0x82A2
#define GL_TEXTURE_GATHER_SHADOW          0x82A3
#define GL_SHADER_IMAGE_LOAD              0x82A4
#define GL_SHADER_IMAGE_STORE             0x82A5
#define GL_SHADER_IMAGE_ATOMIC            0x82A6
#define GL_IMAGE_TEXEL_SIZE               0x82A7
#define GL_IMAGE_COMPATIBILITY_CLASS      0x82A8
#define GL_IMAGE_PIXEL_FORMAT             0x82A9
#define GL_IMAGE_PIXEL_TYPE               0x82AA
#define GL_SIMULTANEOUS_TEXTURE_AND_DEPTH_TEST 0x82AC
#define GL_SIMULTANEOUS_TEXTURE_AND_STENCIL_TEST 0x82AD
#define GL_SIMULTANEOUS_TEXTURE_AND_DEPTH_WRITE 0x82AE
#define GL_SIMULTANEOUS_TEXTURE_AND_STENCIL_WRITE 0x82AF
#define GL_TEXTURE_COMPRESSED_BLOCK_WIDTH 0x82B1
#define GL_TEXTURE_COMPRESSED_BLOCK_HEIGHT 0x82B2
#define GL_TEXTURE_COMPRESSED_BLOCK_SIZE  0x82B3
#define GL_CLEAR_BUFFER                   0x82B4
#define GL_TEXTURE_VIEW                   0x82B5
#define GL_VIEW_COMPATIBILITY_CLASS       0x82B6
#define GL_FULL_SUPPORT                   0x82B7
#define GL_CAVEAT_SUPPORT                 0x82B8
#define GL_IMAGE_CLASS_4_X_32             0x82B9
#define GL_IMAGE_CLASS_2_X_32             0x82BA
#define GL_IMAGE_CLASS_1_X_32             0x82BB
#define GL_IMAGE_CLASS_4_X_16             0x82BC
#define GL_IMAGE_CLASS_2_X_16             0x82BD
#define GL_IMAGE_CLASS_1_X_16             0x82BE
#define GL_IMAGE_CLASS_4_X_8              0x82BF
#define GL_IMAGE_CLASS_2_X_8              0x82C0
#define GL_IMAGE_CLASS_1_X_8              0x82C1
#define GL_IMAGE_CLASS_11_11_10           0x82C2
#define GL_IMAGE_CLASS_10_10_10_2         0x82C3
#define GL_VIEW_CLASS_128_BITS            0x82C4
#define GL_VIEW_CLASS_96_BITS             0x82C5
#define GL_VIEW_CLASS_64_BITS             0x82C6
#define GL_VIEW_CLASS_48_BITS             0x82C7
#define GL_VIEW_CLASS_32_BITS             0x82C8
#define GL_VIEW_CLASS_24_BITS             0x82C9
#define GL_VIEW_CLASS_16_BITS             0x82CA
#define GL_VIEW_CLASS_8_BITS              0x82CB
#define GL_VIEW_CLASS_S3TC_DXT1_RGB       0x82CC
#define GL_VIEW_CLASS_S3TC_DXT1_RGBA      0x82CD
#define GL_VIEW_CLASS_S3TC_DXT3_RGBA      0x82CE
#define GL_VIEW_CLASS_S3TC_DXT5_RGBA      0x82CF
#define GL_VIEW_CLASS_RGTC1_RED           0x82D0
#define GL_VIEW_CLASS_RGTC2_RG            0x82D1
#define GL_VIEW_CLASS_BPTC_UNORM          0x82D2
#define GL_VIEW_CLASS_BPTC_FLOAT          0x82D3
#define GL_UNIFORM                        0x92E1
#define GL_UNIFORM_BLOCK                  0x92E2
#define GL_PROGRAM_INPUT                  0x92E3
#define GL_PROGRAM_OUTPUT                 0x92E4
#define GL_BUFFER_VARIABLE                0x92E5
#define GL_SHADER_STORAGE_BLOCK           0x92E6
#define GL_VERTEX_SUBROUTINE              0x92E8
#define GL_TESS_CONTROL_SUBROUTINE        0x92E9
#define GL_TESS_EVALUATION_SUBROUTINE     0x92EA
#define GL_GEOMETRY_SUBROUTINE            0x92EB
#define GL_FRAGMENT_SUBROUTINE            0x92EC
#define GL_COMPUTE_SUBROUTINE             0x92ED
#define GL_VERTEX_SUBROUTINE_UNIFORM      0x92EE
#define GL_TESS_CONTROL_SUBROUTINE_UNIFORM 0x92EF
#define GL_TESS_EVALUATION_SUBROUTINE_UNIFORM 0x92F0
#define GL_GEOMETRY_SUBROUTINE_UNIFORM    0x92F1
#define GL_FRAGMENT_SUBROUTINE_UNIFORM    0x92F2
#define GL_COMPUTE_SUBROUTINE_UNIFORM     0x92F3
#define GL_TRANSFORM_FEEDBACK_VARYING     0x92F4
#define GL_ACTIVE_RESOURCES               0x92F5
#define GL_MAX_NAME_LENGTH                0x92F6
#define GL_MAX_NUM_ACTIVE_VARIABLES       0x92F7
#define GL_MAX_NUM_COMPATIBLE_SUBROUTINES 0x92F8
#define GL_NAME_LENGTH                    0x92F9
#define GL_TYPE                           0x92FA
#define GL_ARRAY_SIZE                     0x92FB
#define GL_OFFSET                         0x92FC
#define GL_BLOCK_INDEX                    0x92FD
#define GL_ARRAY_STRIDE                   0x92FE
#define GL_MATRIX_STRIDE                  0x92FF
#define GL_IS_ROW_MAJOR                   0x9300
#define GL_ATOMIC_COUNTER_BUFFER_INDEX    0x9301
#define GL_BUFFER_BINDING                 0x9302
#define GL_BUFFER_DATA_SIZE               0x9303
#define GL_NUM_ACTIVE_VARIABLES           0x9304
#define GL_ACTIVE_VARIABLES               0x9305
#define GL_REFERENCED_BY_VERTEX_SHADER    0x9306
#define GL_REFERENCED_BY_TESS_CONTROL_SHADER 0x9307
#define GL_REFERENCED_BY_TESS_EVALUATION_SHADER 0x9308
#define GL_REFERENCED_BY_GEOMETRY_SHADER  0x9309
#define GL_REFERENCED_BY_FRAGMENT_SHADER  0x930A
#define GL_REFERENCED_BY_COMPUTE_SHADER   0x930B
#define GL_TOP_LEVEL_ARRAY_SIZE           0x930C
#define GL_TOP_LEVEL_ARRAY_STRIDE         0x930D
#define GL_LOCATION                       0x930E
#define GL_LOCATION_INDEX                 0x930F
#define GL_IS_PER_PATCH                   0x92E7
#define GL_SHADER_STORAGE_BUFFER          0x90D2
#define GL_SHADER_STORAGE_BUFFER_BINDING  0x90D3
#define GL_SHADER_STORAGE_BUFFER_START    0x90D4
#define GL_SHADER_STORAGE_BUFFER_SIZE     0x90D5
#define GL_MAX_VERTEX_SHADER_STORAGE_BLOCKS 0x90D6
#define GL_MAX_GEOMETRY_SHADER_STORAGE_BLOCKS 0x90D7
#define GL_MAX_TESS_CONTROL_SHADER_STORAGE_BLOCKS 0x90D8
#define GL_MAX_TESS_EVALUATION_SHADER_STORAGE_BLOCKS 0x90D9
#define GL_MAX_FRAGMENT_SHADER_STORAGE_BLOCKS 0x90DA
#define GL_MAX_COMPUTE_SHADER_STORAGE_BLOCKS 0x90DB
#define GL_MAX_COMBINED_SHADER_STORAGE_BLOCKS 0x90DC
#define GL_MAX_SHADER_STORAGE_BUFFER_BINDINGS 0x90DD
#define GL_MAX_SHADER_STORAGE_BLOCK_SIZE  0x90DE
#define GL_SHADER_STORAGE_BUFFER_OFFSET_ALIGNMENT 0x90DF
#define GL_SHADER_STORAGE_BARRIER_BIT     0x00002000
#define GL_MAX_COMBINED_SHADER_OUTPUT_RESOURCES 0x8F39
#define GL_DEPTH_STENCIL_TEXTURE_MODE     0x90EA
#define GL_TEXTURE_BUFFER_OFFSET          0x919D
#define GL_TEXTURE_BUFFER_SIZE            0x919E
#define GL_TEXTURE_BUFFER_OFFSET_ALIGNMENT 0x919F
#define GL_TEXTURE_VIEW_MIN_LEVEL         0x82DB
#define GL_TEXTURE_VIEW_NUM_LEVELS        0x82DC
#define GL_TEXTURE_VIEW_MIN_LAYER         0x82DD
#define GL_TEXTURE_VIEW_NUM_LAYERS        0x82DE
#define GL_TEXTURE_IMMUTABLE_LEVELS       0x82DF
#define GL_VERTEX_ATTRIB_BINDING          0x82D4
#define GL_VERTEX_ATTRIB_RELATIVE_OFFSET  0x82D5
#define GL_VERTEX_BINDING_DIVISOR         0x82D6
#define GL_VERTEX_BINDING_OFFSET          0x82D7
#define GL_VERTEX_BINDING_STRIDE          0x82D8
#define GL_MAX_VERTEX_ATTRIB_RELATIVE_OFFSET 0x82D9
#define GL_MAX_VERTEX_ATTRIB_BINDINGS     0x82DA
#define GL_VERTEX_BINDING_BUFFER          0x8F4F
typedef void (APIENTRYP PFNGLCLEARBUFFERDATAPROC) (GLenum target, GLenum internalformat, GLenum format, GLenum type, const void *data);
typedef void (APIENTRYP PFNGLCLEARBUFFERSUBDATAPROC) (GLenum target, GLenum internalformat, GLintptr offset, GLsizeiptr size, GLenum format, GLenum type, const void *data);
typedef void (APIENTRYP PFNGLDISPATCHCOMPUTEPROC) (GLuint num_groups_x, GLuint num_groups_y, GLuint num_groups_z);
typedef void (APIENTRYP PFNGLDISPATCHCOMPUTEINDIRECTPROC) (GLintptr indirect);
typedef void (APIENTRYP PFNGLCOPYIMAGESUBDATAPROC) (GLuint srcName, GLenum srcTarget, GLint srcLevel, GLint srcX, GLint srcY, GLint srcZ, GLuint dstName, GLenum dstTarget, GLint dstLevel, GLint dstX, GLint dstY, GLint dstZ, GLsizei srcWidth, GLsizei srcHeight, GLsizei srcDepth);
typedef void (APIENTRYP PFNGLFRAMEBUFFERPARAMETERIPROC) (GLenum target, GLenum pname, GLint param);
typedef void (APIENTRYP PFNGLGETFRAMEBUFFERPARAMETERIVPROC) (GLenum target, GLenum pname, GLint *params);
typedef void (APIENTRYP PFNGLGETINTERNALFORMATI64VPROC) (GLenum target, GLenum internalformat, GLenum pname, GLsizei count, GLint64 *params);
typedef void (APIENTRYP PFNGLINVALIDATETEXSUBIMAGEPROC) (GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth);
typedef void (APIENTRYP PFNGLINVALIDATETEXIMAGEPROC) (GLuint texture, GLint level);
typedef void (APIENTRYP PFNGLINVALIDATEBUFFERSUBDATAPROC) (GLuint buffer, GLintptr offset, GLsizeiptr length);
typedef void (APIENTRYP PFNGLINVALIDATEBUFFERDATAPROC) (GLuint buffer);
typedef void (APIENTRYP PFNGLINVALIDATEFRAMEBUFFERPROC) (GLenum target, GLsizei numAttachments, const GLenum *attachments);
typedef void (APIENTRYP PFNGLINVALIDATESUBFRAMEBUFFERPROC) (GLenum target, GLsizei numAttachments, const GLenum *attachments, GLint x, GLint y, GLsizei width, GLsizei height);
typedef void (APIENTRYP PFNGLMULTIDRAWARRAYSINDIRECTPROC) (GLenum mode, const void *indirect, GLsizei drawcount, GLsizei stride);
typedef void (APIENTRYP PFNGLMULTIDRAWELEMENTSINDIRECTPROC) (GLenum mode, GLenum type, const void *indirect, GLsizei drawcount, GLsizei stride);
typedef void (APIENTRYP PFNGLGETPROGRAMINTERFACEIVPROC) (GLuint program, GLenum programInterface, GLenum pname, GLint *params);
typedef GLuint (APIENTRYP PFNGLGETPROGRAMRESOURCEINDEXPROC) (GLuint program, GLenum programInterface, const GLchar *name);
typedef void (APIENTRYP PFNGLGETPROGRAMRESOURCENAMEPROC) (GLuint program, GLenum programInterface, GLuint index, GLsizei bufSize, GLsizei *length, GLchar *name);
typedef void (APIENTRYP PFNGLGETPROGRAMRESOURCEIVPROC) (GLuint program, GLenum programInterface, GLuint index, GLsizei propCount, const GLenum *props, GLsizei count, GLsizei *length, GLint *params);
typedef GLint (APIENTRYP PFNGLGETPROGRAMRESOURCELOCATIONPROC) (GLuint program, GLenum programInterface, const GLchar *name);
typedef GLint (APIENTRYP PFNGLGETPROGRAMRESOURCELOCATIONINDEXPROC) (GLuint program, GLenum programInterface, const GLchar *name);
typedef void (APIENTRYP PFNGLSHADERSTORAGEBLOCKBINDINGPROC) (GLuint program, GLuint storageBlockIndex, GLuint storageBlockBinding);
typedef void (APIENTRYP PFNGLTEXBUFFERRANGEPROC) (GLenum target, GLenum internalformat, GLuint buffer, GLintptr offset, GLsizeiptr size);
typedef void (APIENTRYP PFNGLTEXSTORAGE2DMULTISAMPLEPROC) (GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLboolean fixedsamplelocations);
typedef void (APIENTRYP PFNGLTEXSTORAGE3DMULTISAMPLEPROC) (GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLboolean fixedsamplelocations);
typedef void (APIENTRYP PFNGLTEXTUREVIEWPROC) (GLuint texture, GLenum target, GLuint origtexture, GLenum internalformat, GLuint minlevel, GLuint numlevels, GLuint minlayer, GLuint numlayers);
typedef void (APIENTRYP PFNGLBINDVERTEXBUFFERPROC) (GLuint bindingindex, GLuint buffer, GLintptr offset, GLsizei stride);
typedef void (APIENTRYP PFNGLVERTEXATTRIBFORMATPROC) (GLuint attribindex, GLint size, GLenum type, GLboolean normalized, GLuint relativeoffset);
typedef void (APIENTRYP PFNGLVERTEXATTRIBIFORMATPROC) (GLuint attribindex, GLint size, GLenum type, GLuint relativeoffset);
typedef void (APIENTRYP PFNGLVERTEXATTRIBLFORMATPROC) (GLuint attribindex, GLint size, GLenum type, GLuint relativeoffset);
typedef void (APIENTRYP PFNGLVERTEXATTRIBBINDINGPROC) (GLuint attribindex, GLuint bindingindex);
typedef void (APIENTRYP PFNGLVERTEXBINDINGDIVISORPROC) (GLuint bindingindex, GLuint divisor);
typedef void (APIENTRYP PFNGLDEBUGMESSAGECONTROLPROC) (GLenum source, GLenum type, GLenum severity, GLsizei count, const GLuint *ids, GLboolean enabled);
typedef void (APIENTRYP PFNGLDEBUGMESSAGEINSERTPROC) (GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *buf);
typedef void (APIENTRYP PFNGLDEBUGMESSAGECALLBACKPROC) (GLDEBUGPROC callback, const void *userParam);
typedef GLuint (APIENTRYP PFNGLGETDEBUGMESSAGELOGPROC) (GLuint count, GLsizei bufSize, GLenum *sources, GLenum *types, GLuint *ids, GLenum *severities, GLsizei *lengths, GLchar *messageLog);
typedef void (APIENTRYP PFNGLPUSHDEBUGGROUPPROC) (GLenum source, GLuint id, GLsizei length, const GLchar *message);
typedef void (APIENTRYP PFNGLPOPDEBUGGROUPPROC) (void);
typedef void (APIENTRYP PFNGLOBJECTLABELPROC) (GLenum identifier, GLuint name, GLsizei length, const GLchar *label);
typedef void (APIENTRYP PFNGLGETOBJECTLABELPROC) (GLenum identifier, GLuint name, GLsizei bufSize, GLsizei *length, GLchar *label);
typedef void (APIENTRYP PFNGLOBJECTPTRLABELPROC) (const void *ptr, GLsizei length, const GLchar *label);
typedef void (APIENTRYP PFNGLGETOBJECTPTRLABELPROC) (const void *ptr, GLsizei bufSize, GLsizei *length, GLchar *label);
#endif /* GL_VERSION_4_3 */
#ifndef GL_VERSION_4_4
#define GL_VERSION_4_4 1
#define GL_MAX_VERTEX_ATTRIB_STRIDE       0x82E5
#define GL_PRIMITIVE_RESTART_FOR_PATCHES_SUPPORTED 0x8221
#define GL_TEXTURE_BUFFER_BINDING         0x8C2A
#define GL_MAP_PERSISTENT_BIT             0x0040
#define GL_MAP_COHERENT_BIT               0x0080
#define GL_DYNAMIC_STORAGE_BIT            0x0100
#define GL_CLIENT_STORAGE_BIT             0x0200
#define GL_CLIENT_MAPPED_BUFFER_BARRIER_BIT 0x00004000
#define GL_BUFFER_IMMUTABLE_STORAGE       0x821F
#define GL_BUFFER_STORAGE_FLAGS           0x8220
#define GL_CLEAR_TEXTURE                  0x9365
#define GL_LOCATION_COMPONENT             0x934A
#define GL_TRANSFORM_FEEDBACK_BUFFER_INDEX 0x934B
#define GL_TRANSFORM_FEEDBACK_BUFFER_STRIDE 0x934C
#define GL_QUERY_BUFFER                   0x9192
#define GL_QUERY_BUFFER_BARRIER_BIT       0x00008000
#define GL_QUERY_BUFFER_BINDING           0x9193
#define GL_QUERY_RESULT_NO_WAIT           0x9194
#define GL_MIRROR_CLAMP_TO_EDGE           0x8743
typedef void (APIENTRYP PFNGLBUFFERSTORAGEPROC) (GLenum target, GLsizeiptr size, const void *data, GLbitfield flags);
typedef void (APIENTRYP PFNGLCLEARTEXIMAGEPROC) (GLuint texture, GLint level, GLenum format, GLenum type, const void *data);
typedef void (APIENTRYP PFNGLCLEARTEXSUBIMAGEPROC) (GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const void *data);
typedef void (APIENTRYP PFNGLBINDBUFFERSBASEPROC) (GLenum target, GLuint first, GLsizei count, const GLuint *buffers);
typedef void (APIENTRYP PFNGLBINDBUFFERSRANGEPROC) (GLenum target, GLuint first, GLsizei count, const GLuint *buffers, const GLintptr *offsets, const GLsizeiptr *sizes);
typedef void (APIENTRYP PFNGLBINDTEXTURESPROC) (GLuint first, GLsizei count, const GLuint *textures);
typedef void (APIENTRYP PFNGLBINDSAMPLERSPROC) (GLuint first, GLsizei count, const GLuint *samplers);
typedef void (APIENTRYP PFNGLBINDIMAGETEXTURESPROC) (GLuint first, GLsizei count, const GLuint *textures);
typedef void (APIENTRYP PFNGLBINDVERTEXBUFFERSPROC) (GLuint first, GLsizei count, const GLuint *buffers, const GLintptr *offsets, const GLsizei *strides);
#endif /* GL_VERSION_4_4 */
#ifndef GL_VERSION_4_5
#define GL_VERSION_4_5 1
#define GL_CONTEXT_LOST                   0x0507
#define GL_NEGATIVE_ONE_TO_ONE            0x935E
#define GL_ZERO_TO_ONE                    0x935F
#define GL_CLIP_ORIGIN                    0x935C
#define GL_CLIP_DEPTH_MODE                0x935D
#define GL_QUERY_WAIT_INVERTED            0x8E17
#define GL_QUERY_NO_WAIT_INVERTED         0x8E18
#define GL_QUERY_BY_REGION_WAIT_INVERTED  0x8E19
#define GL_QUERY_BY_REGION_NO_WAIT_INVERTED 0x8E1A
#define GL_MAX_CULL_DISTANCES             0x82F9
#define GL_MAX_COMBINED_CLIP_AND_CULL_DISTANCES 0x82FA
#define GL_TEXTURE_TARGET                 0x1006
#define GL_QUERY_TARGET                   0x82EA
#define GL_GUILTY_CONTEXT_RESET           0x8253
#define GL_INNOCENT_CONTEXT_RESET         0x8254
#define GL_UNKNOWN_CONTEXT_RESET          0x8255
#define GL_RESET_NOTIFICATION_STRATEGY    0x8256
#define GL_LOSE_CONTEXT_ON_RESET          0x8252
#define GL_NO_RESET_NOTIFICATION          0x8261
#define GL_CONTEXT_FLAG_ROBUST_ACCESS_BIT 0x00000004
#define GL_CONTEXT_RELEASE_BEHAVIOR       0x82FB
#define GL_CONTEXT_RELEASE_BEHAVIOR_FLUSH 0x82FC
typedef void (APIENTRYP PFNGLCLIPCONTROLPROC) (GLenum origin, GLenum depth);
typedef void (APIENTRYP PFNGLCREATETRANSFORMFEEDBACKSPROC) (GLsizei n, GLuint *ids);
typedef void (APIENTRYP PFNGLTRANSFORMFEEDBACKBUFFERBASEPROC) (GLuint xfb, GLuint index, GLuint buffer);
typedef void (APIENTRYP PFNGLTRANSFORMFEEDBACKBUFFERRANGEPROC) (GLuint xfb, GLuint index, GLuint buffer, GLintptr offset, GLsizeiptr size);
typedef void (APIENTRYP PFNGLGETTRANSFORMFEEDBACKIVPROC) (GLuint xfb, GLenum pname, GLint *param);
typedef void (APIENTRYP PFNGLGETTRANSFORMFEEDBACKI_VPROC) (GLuint xfb, GLenum pname, GLuint index, GLint *param);
typedef void (APIENTRYP PFNGLGETTRANSFORMFEEDBACKI64_VPROC) (GLuint xfb, GLenum pname, GLuint index, GLint64 *param);
typedef void (APIENTRYP PFNGLCREATEBUFFERSPROC) (GLsizei n, GLuint *buffers);
typedef void (APIENTRYP PFNGLNAMEDBUFFERSTORAGEPROC) (GLuint buffer, GLsizeiptr size, const void *data, GLbitfield flags);
typedef void (APIENTRYP PFNGLNAMEDBUFFERDATAPROC) (GLuint buffer, GLsizeiptr size, const void *data, GLenum usage);
typedef void (APIENTRYP PFNGLNAMEDBUFFERSUBDATAPROC) (GLuint buffer, GLintptr offset, GLsizeiptr size, const void *data);
typedef void (APIENTRYP PFNGLCOPYNAMEDBUFFERSUBDATAPROC) (GLuint readBuffer, GLuint writeBuffer, GLintptr readOffset, GLintptr writeOffset, GLsizeiptr size);
typedef void (APIENTRYP PFNGLCLEARNAMEDBUFFERDATAPROC) (GLuint buffer, GLenum internalformat, GLenum format, GLenum type, const void *data);
typedef void (APIENTRYP PFNGLCLEARNAMEDBUFFERSUBDATAPROC) (GLuint buffer, GLenum internalformat, GLintptr offset, GLsizeiptr size, GLenum format, GLenum type, const void *data);
typedef void *(APIENTRYP PFNGLMAPNAMEDBUFFERPROC) (GLuint buffer, GLenum access);
typedef void *(APIENTRYP PFNGLMAPNAMEDBUFFERRANGEPROC) (GLuint buffer, GLintptr offset, GLsizeiptr length, GLbitfield access);
typedef GLboolean (APIENTRYP PFNGLUNMAPNAMEDBUFFERPROC) (GLuint buffer);
typedef void (APIENTRYP PFNGLFLUSHMAPPEDNAMEDBUFFERRANGEPROC) (GLuint buffer, GLintptr offset, GLsizeiptr length);
typedef void (APIENTRYP PFNGLGETNAMEDBUFFERPARAMETERIVPROC) (GLuint buffer, GLenum pname, GLint *params);
typedef void (APIENTRYP PFNGLGETNAMEDBUFFERPARAMETERI64VPROC) (GLuint buffer, GLenum pname, GLint64 *params);
typedef void (APIENTRYP PFNGLGETNAMEDBUFFERPOINTERVPROC) (GLuint buffer, GLenum pname, void **params);
typedef void (APIENTRYP PFNGLGETNAMEDBUFFERSUBDATAPROC) (GLuint buffer, GLintptr offset, GLsizeiptr size, void *data);
typedef void (APIENTRYP PFNGLCREATEFRAMEBUFFERSPROC) (GLsizei n, GLuint *framebuffers);
typedef void (APIENTRYP PFNGLNAMEDFRAMEBUFFERRENDERBUFFERPROC) (GLuint framebuffer, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer);
typedef void (APIENTRYP PFNGLNAMEDFRAMEBUFFERPARAMETERIPROC) (GLuint framebuffer, GLenum pname, GLint param);
typedef void (APIENTRYP PFNGLNAMEDFRAMEBUFFERTEXTUREPROC) (GLuint framebuffer, GLenum attachment, GLuint texture, GLint level);
typedef void (APIENTRYP PFNGLNAMEDFRAMEBUFFERTEXTURELAYERPROC) (GLuint framebuffer, GLenum attachment, GLuint texture, GLint level, GLint layer);
typedef void (APIENTRYP PFNGLNAMEDFRAMEBUFFERDRAWBUFFERPROC) (GLuint framebuffer, GLenum buf);
typedef void (APIENTRYP PFNGLNAMEDFRAMEBUFFERDRAWBUFFERSPROC) (GLuint framebuffer, GLsizei n, const GLenum *bufs);
typedef void (APIENTRYP PFNGLNAMEDFRAMEBUFFERREADBUFFERPROC) (GLuint framebuffer, GLenum src);
typedef void (APIENTRYP PFNGLINVALIDATENAMEDFRAMEBUFFERDATAPROC) (GLuint framebuffer, GLsizei numAttachments, const GLenum *attachments);
typedef void (APIENTRYP PFNGLINVALIDATENAMEDFRAMEBUFFERSUBDATAPROC) (GLuint framebuffer, GLsizei numAttachments, const GLenum *attachments, GLint x, GLint y, GLsizei width, GLsizei height);
typedef void (APIENTRYP PFNGLCLEARNAMEDFRAMEBUFFERIVPROC) (GLuint framebuffer, GLenum buffer, GLint drawbuffer, const GLint *value);
typedef void (APIENTRYP PFNGLCLEARNAMEDFRAMEBUFFERUIVPROC) (GLuint framebuffer, GLenum buffer, GLint drawbuffer, const GLuint *value);
typedef void (APIENTRYP PFNGLCLEARNAMEDFRAMEBUFFERFVPROC) (GLuint framebuffer, GLenum buffer, GLint drawbuffer, const GLfloat *value);
typedef void (APIENTRYP PFNGLCLEARNAMEDFRAMEBUFFERFIPROC) (GLuint framebuffer, GLenum buffer, GLint drawbuffer, GLfloat depth, GLint stencil);
typedef void (APIENTRYP PFNGLBLITNAMEDFRAMEBUFFERPROC) (GLuint readFramebuffer, GLuint drawFramebuffer, GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter);
typedef GLenum (APIENTRYP PFNGLCHECKNAMEDFRAMEBUFFERSTATUSPROC) (GLuint framebuffer, GLenum target);
typedef void (APIENTRYP PFNGLGETNAMEDFRAMEBUFFERPARAMETERIVPROC) (GLuint framebuffer, GLenum pname, GLint *param);
typedef void (APIENTRYP PFNGLGETNAMEDFRAMEBUFFERATTACHMENTPARAMETERIVPROC) (GLuint framebuffer, GLenum attachment, GLenum pname, GLint *params);
typedef void (APIENTRYP PFNGLCREATERENDERBUFFERSPROC) (GLsizei n, GLuint *renderbuffers);
typedef void (APIENTRYP PFNGLNAMEDRENDERBUFFERSTORAGEPROC) (GLuint renderbuffer, GLenum internalformat, GLsizei width, GLsizei height);
typedef void (APIENTRYP PFNGLNAMEDRENDERBUFFERSTORAGEMULTISAMPLEPROC) (GLuint renderbuffer, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height);
typedef void (APIENTRYP PFNGLGETNAMEDRENDERBUFFERPARAMETERIVPROC) (GLuint renderbuffer, GLenum pname, GLint *params);
typedef void (APIENTRYP PFNGLCREATETEXTURESPROC) (GLenum target, GLsizei n, GLuint *textures);
typedef void (APIENTRYP PFNGLTEXTUREBUFFERPROC) (GLuint texture, GLenum internalformat, GLuint buffer);
typedef void (APIENTRYP PFNGLTEXTUREBUFFERRANGEPROC) (GLuint texture, GLenum internalformat, GLuint buffer, GLintptr offset, GLsizeiptr size);
typedef void (APIENTRYP PFNGLTEXTURESTORAGE1DPROC) (GLuint texture, GLsizei levels, GLenum internalformat, GLsizei width);
typedef void (APIENTRYP PFNGLTEXTURESTORAGE2DPROC) (GLuint texture, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height);
typedef void (APIENTRYP PFNGLTEXTURESTORAGE3DPROC) (GLuint texture, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth);
typedef void (APIENTRYP PFNGLTEXTURESTORAGE2DMULTISAMPLEPROC) (GLuint texture, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLboolean fixedsamplelocations);
typedef void (APIENTRYP PFNGLTEXTURESTORAGE3DMULTISAMPLEPROC) (GLuint texture, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLboolean fixedsamplelocations);
typedef void (APIENTRYP PFNGLTEXTURESUBIMAGE1DPROC) (GLuint texture, GLint level, GLint xoffset, GLsizei width, GLenum format, GLenum type, const void *pixels);
typedef void (APIENTRYP PFNGLTEXTURESUBIMAGE2DPROC) (GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const void *pixels);
typedef void (APIENTRYP PFNGLTEXTURESUBIMAGE3DPROC) (GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const void *pixels);
typedef void (APIENTRYP PFNGLCOMPRESSEDTEXTURESUBIMAGE1DPROC) (GLuint texture, GLint level, GLint xoffset, GLsizei width, GLenum format, GLsizei imageSize, const void *data);
typedef void (APIENTRYP PFNGLCOMPRESSEDTEXTURESUBIMAGE2DPROC) (GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const void *data);
typedef void (APIENTRYP PFNGLCOMPRESSEDTEXTURESUBIMAGE3DPROC) (GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const void *data);
typedef void (APIENTRYP PFNGLCOPYTEXTURESUBIMAGE1DPROC) (GLuint texture, GLint level, GLint xoffset, GLint x, GLint y, GLsizei width);
typedef void (APIENTRYP PFNGLCOPYTEXTURESUBIMAGE2DPROC) (GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height);
typedef void (APIENTRYP PFNGLCOPYTEXTURESUBIMAGE3DPROC) (GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height);
typedef void (APIENTRYP PFNGLTEXTUREPARAMETERFPROC) (GLuint texture, GLenum pname, GLfloat param);
typedef void (APIENTRYP PFNGLTEXTUREPARAMETERFVPROC) (GLuint texture, GLenum pname, const GLfloat *param);
typedef void (APIENTRYP PFNGLTEXTUREPARAMETERIPROC) (GLuint texture, GLenum pname, GLint param);
typedef void (APIENTRYP PFNGLTEXTUREPARAMETERIIVPROC) (GLuint texture, GLenum pname, const GLint *params);
typedef void (APIENTRYP PFNGLTEXTUREPARAMETERIUIVPROC) (GLuint texture, GLenum pname, const GLuint *params);
typedef void (APIENTRYP PFNGLTEXTUREPARAMETERIVPROC) (GLuint texture, GLenum pname, const GLint *param);
typedef void (APIENTRYP PFNGLGENERATETEXTUREMIPMAPPROC) (GLuint texture);
typedef void (APIENTRYP PFNGLBINDTEXTUREUNITPROC) (GLuint unit, GLuint texture);
typedef void (APIENTRYP PFNGLGETTEXTUREIMAGEPROC) (GLuint texture, GLint level, GLenum format, GLenum type, GLsizei bufSize, void *pixels);
typedef void (APIENTRYP PFNGLGETCOMPRESSEDTEXTUREIMAGEPROC) (GLuint texture, GLint level, GLsizei bufSize, void *pixels);
typedef void (APIENTRYP PFNGLGETTEXTURELEVELPARAMETERFVPROC) (GLuint texture, GLint level, GLenum pname, GLfloat *params);
typedef void (APIENTRYP PFNGLGETTEXTURELEVELPARAMETERIVPROC) (GLuint texture, GLint level, GLenum pname, GLint *params);
typedef void (APIENTRYP PFNGLGETTEXTUREPARAMETERFVPROC) (GLuint texture, GLenum pname, GLfloat *params);
typedef void (APIENTRYP PFNGLGETTEXTUREPARAMETERIIVPROC) (GLuint texture, GLenum pname, GLint *params);
typedef void (APIENTRYP PFNGLGETTEXTUREPARAMETERIUIVPROC) (GLuint texture, GLenum pname, GLuint *params);
typedef void (APIENTRYP PFNGLGETTEXTUREPARAMETERIVPROC) (GLuint texture, GLenum pname, GLint *params);
typedef void (APIENTRYP PFNGLCREATEVERTEXARRAYSPROC) (GLsizei n, GLuint *arrays);
typedef void (APIENTRYP PFNGLDISABLEVERTEXARRAYATTRIBPROC) (GLuint vaobj, GLuint index);
typedef void (APIENTRYP PFNGLENABLEVERTEXARRAYATTRIBPROC) (GLuint vaobj, GLuint index);
typedef void (APIENTRYP PFNGLVERTEXARRAYELEMENTBUFFERPROC) (GLuint vaobj, GLuint buffer);
typedef void (APIENTRYP PFNGLVERTEXARRAYVERTEXBUFFERPROC) (GLuint vaobj, GLuint bindingindex, GLuint buffer, GLintptr offset, GLsizei stride);
typedef void (APIENTRYP PFNGLVERTEXARRAYVERTEXBUFFERSPROC) (GLuint vaobj, GLuint first, GLsizei count, const GLuint *buffers, const GLintptr *offsets, const GLsizei *strides);
typedef void (APIENTRYP PFNGLVERTEXARRAYATTRIBBINDINGPROC) (GLuint vaobj, GLuint attribindex, GLuint bindingindex);
typedef void (APIENTRYP PFNGLVERTEXARRAYATTRIBFORMATPROC) (GLuint vaobj, GLuint attribindex, GLint size, GLenum type, GLboolean normalized, GLuint relativeoffset);
typedef void (APIENTRYP PFNGLVERTEXARRAYATTRIBIFORMATPROC) (GLuint vaobj, GLuint attribindex, GLint size, GLenum type, GLuint relativeoffset);
typedef void (APIENTRYP PFNGLVERTEXARRAYATTRIBLFORMATPROC) (GLuint vaobj, GLuint attribindex, GLint size, GLenum type, GLuint relativeoffset);
typedef void (APIENTRYP PFNGLVERTEXARRAYBINDINGDIVISORPROC) (GLuint vaobj, GLuint bindingindex, GLuint divisor);
typedef void (APIENTRYP PFNGLGETVERTEXARRAYIVPROC) (GLuint vaobj, GLenum pname, GLint *param);
typedef void (APIENTRYP PFNGLGETVERTEXARRAYINDEXEDIVPROC) (GLuint vaobj, GLuint index, GLenum pname, GLint *param);
typedef void (APIENTRYP PFNGLGETVERTEXARRAYINDEXED64IVPROC) (GLuint vaobj, GLuint index, GLenum pname, GLint64 *param);
typedef void (APIENTRYP PFNGLCREATESAMPLERSPROC) (GLsizei n, GLuint *samplers);
typedef void (APIENTRYP PFNGLCREATEPROGRAMPIPELINESPROC) (GLsizei n, GLuint *pipelines);
typedef void (APIENTRYP PFNGLCREATEQUERIESPROC) (GLenum target, GLsizei n, GLuint *ids);
typedef void (APIENTRYP PFNGLGETQUERYBUFFEROBJECTI64VPROC) (GLuint id, GLuint buffer, GLenum pname, GLintptr offset);
typedef void (APIENTRYP PFNGLGETQUERYBUFFEROBJECTIVPROC) (GLuint id, GLuint buffer, GLenum pname, GLintptr offset);
typedef void (APIENTRYP PFNGLGETQUERYBUFFEROBJECTUI64VPROC) (GLuint id, GLuint buffer, GLenum pname, GLintptr offset);
typedef void (APIENTRYP PFNGLGETQUERYBUFFEROBJECTUIVPROC) (GLuint id, GLuint buffer, GLenum pname, GLintptr offset);
typedef void (APIENTRYP PFNGLMEMORYBARRIERBYREGIONPROC) (GLbitfield barriers);
typedef void (APIENTRYP PFNGLGETTEXTURESUBIMAGEPROC) (GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, GLsizei bufSize, void *pixels);
typedef void (APIENTRYP PFNGLGETCOMPRESSEDTEXTURESUBIMAGEPROC) (GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLsizei bufSize, void *pixels);
typedef GLenum (APIENTRYP PFNGLGETGRAPHICSRESETSTATUSPROC) (void);
typedef void (APIENTRYP PFNGLGETNCOMPRESSEDTEXIMAGEPROC) (GLenum target, GLint lod, GLsizei bufSize, void *pixels);
typedef void (APIENTRYP PFNGLGETNTEXIMAGEPROC) (GLenum target, GLint level, GLenum format, GLenum type, GLsizei bufSize, void *pixels);
typedef void (APIENTRYP PFNGLGETNUNIFORMDVPROC) (GLuint program, GLint location, GLsizei bufSize, GLdouble *params);
typedef void (APIENTRYP PFNGLGETNUNIFORMFVPROC) (GLuint program, GLint location, GLsizei bufSize, GLfloat *params);
typedef void (APIENTRYP PFNGLGETNUNIFORMIVPROC) (GLuint program, GLint location, GLsizei bufSize, GLint *params);
typedef void (APIENTRYP PFNGLGETNUNIFORMUIVPROC) (GLuint program, GLint location, GLsizei bufSize, GLuint *params);
typedef void (APIENTRYP PFNGLREADNPIXELSPROC) (GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLsizei bufSize, void *data);
typedef void (APIENTRYP PFNGLTEXTUREBARRIERPROC) (void);
#endif /* GL_VERSION_4_5 */
#ifndef GL_VERSION_4_6
#define GL_VERSION_4_6 1
#define GL_SHADER_BINARY_FORMAT_SPIR_V    0x9551
#define GL_SPIR_V_BINARY                  0x9552
#define GL_PARAMETER_BUFFER               0x80EE
#define GL_PARAMETER_BUFFER_BINDING       0x80EF
#define GL_CONTEXT_FLAG_NO_ERROR_BIT      0x00000008
#define GL_VERTICES_SUBMITTED             0x82EE
#define GL_PRIMITIVES_SUBMITTED           0x82EF
#define GL_VERTEX_SHADER_INVOCATIONS      0x82F0
#define GL_TESS_CONTROL_SHADER_PATCHES    0x82F1
#define GL_TESS_EVALUATION_SHADER_INVOCATIONS 0x82F2
#define GL_GEOMETRY_SHADER_PRIMITIVES_EMITTED 0x82F3
#define GL_FRAGMENT_SHADER_INVOCATIONS    0x82F4
#define GL_COMPUTE_SHADER_INVOCATIONS     0x82F5
#define GL_CLIPPING_INPUT_PRIMITIVES      0x82F6
#define GL_CLIPPING_OUTPUT_PRIMITIVES     0x82F7
#define GL_POLYGON_OFFSET_CLAMP           0x8E1B
#define GL_SPIR_V_EXTENSIONS              0x9553
#define GL_NUM_SPIR_V_EXTENSIONS          0x9554
#define GL_TEXTURE_MAX_ANISOTROPY         0x84FE
#define GL_MAX_TEXTURE_MAX_ANISOTROPY     0x84FF
#define GL_TRANSFORM_FEEDBACK_OVERFLOW    0x82EC
#define GL_TRANSFORM_FEEDBACK_STREAM_OVERFLOW 0x82ED
typedef void (APIENTRYP PFNGLSPECIALIZESHADERPROC) (GLuint shader, const GLchar *pEntryPoint, GLuint numSpecializationConstants, const GLuint *pConstantIndex, const GLuint *pConstantValue);
typedef void (APIENTRYP PFNGLMULTIDRAWARRAYSINDIRECTCOUNTPROC) (GLenum mode, const void *indirect, GLintptr drawcount, GLsizei maxdrawcount, GLsizei stride);
typedef void (APIENTRYP PFNGLMULTIDRAWELEMENTSINDIRECTCOUNTPROC) (GLenum mode, GLenum type, const void *indirect, GLintptr drawcount, GLsizei maxdrawcount, GLsizei stride);
typedef void (APIENTRYP PFNGLPOLYGONOFFSETCLAMPPROC) (GLfloat factor, GLfloat units, GLfloat clamp);
#endif

// -*- END BORROWED CODE -*-

typedef struct Window Window;
enum {
	EVENT_QUIT,
	EVENT_KEYDOWN,
	EVENT_KEYUP,
	EVENT_MOUSEDOWN,
	EVENT_MOUSEUP,
	EVENT_MOUSEMOTION,
	EVENT_MOUSEWHEEL,
	EVENT_TEXT
};

enum {
	MOUSE_NONE,
	MOUSE_LEFT,
	MOUSE_MIDDLE,
	MOUSE_RIGHT
};

enum {
	KEY_MODIFIER_LSHIFT = 0x01,
	KEY_MODIFIER_RSHIFT = 0x10,
	KEY_MODIFIER_LCTRL = 0x02,
	KEY_MODIFIER_RCTRL = 0x20,
	KEY_MODIFIER_LALT = 0x04,
	KEY_MODIFIER_RALT = 0x40,
	KEY_MODIFIER_LGUI = 0x08,
	KEY_MODIFIER_RGUI = 0x80
};

enum {
    KEY_UNKNOWN = 0, KEY_A = 4, KEY_B = 5, KEY_C = 6, KEY_D = 7,
    KEY_E = 8, KEY_F = 9, KEY_G = 10, KEY_H = 11, KEY_I = 12,
    KEY_J = 13, KEY_K = 14, KEY_L = 15, KEY_M = 16, KEY_N = 17,
    KEY_O = 18, KEY_P = 19, KEY_Q = 20, KEY_R = 21, KEY_S = 22,
    KEY_T = 23, KEY_U = 24, KEY_V = 25, KEY_W = 26, KEY_X = 27,
    KEY_Y = 28, KEY_Z = 29,

    KEY_1 = 30, KEY_2 = 31, KEY_3 = 32, KEY_4 = 33, KEY_5 = 34,
    KEY_6 = 35, KEY_7 = 36, KEY_8 = 37, KEY_9 = 38, KEY_0 = 39,

    KEY_RETURN = 40, KEY_ESCAPE = 41, KEY_BACKSPACE = 42, KEY_TAB = 43,
#define KEY_ENTER KEY_RETURN
    KEY_SPACE = 44,

    KEY_MINUS = 45,
    KEY_EQUALS = 46,
    KEY_LEFTBRACKET = 47,
    KEY_RIGHTBRACKET = 48,
    KEY_BACKSLASH = 49, KEY_NONUSHASH = 50, KEY_SEMICOLON = 51,
    KEY_APOSTROPHE = 52, KEY_BACKTICK = 53, KEY_COMMA = 54, KEY_PERIOD = 55,
    KEY_SLASH = 56, KEY_CAPSLOCK = 57,

    KEY_F1 = 58, KEY_F2 = 59, KEY_F3 = 60, KEY_F4 = 61, KEY_F5 = 62,
    KEY_F6 = 63, KEY_F7 = 64, KEY_F8 = 65, KEY_F9 = 66, KEY_F10 = 67,
    KEY_F11 = 68, KEY_F12 = 69,

    KEY_PRINTSCREEN = 70, KEY_SCROLLLOCK = 71, KEY_PAUSE = 72, KEY_INSERT = 73,
    KEY_HOME = 74, KEY_PAGEUP = 75, KEY_DELETE = 76, KEY_END = 77,
    KEY_PAGEDOWN = 78, KEY_RIGHT = 79, KEY_LEFT = 80, KEY_DOWN = 81,
    KEY_UP = 82, KEY_NUMLOCKCLEAR = 83, KEY_KP_DIVIDE = 84,
    KEY_KP_MULTIPLY = 85, KEY_KP_MINUS = 86, KEY_KP_PLUS = 87, KEY_KP_ENTER = 88,
    KEY_KP_1 = 89, KEY_KP_2 = 90, KEY_KP_3 = 91, KEY_KP_4 = 92,
    KEY_KP_5 = 93, KEY_KP_6 = 94, KEY_KP_7 = 95, KEY_KP_8 = 96, KEY_KP_9 = 97,
    KEY_KP_0 = 98, KEY_KP_PERIOD = 99,

    KEY_LCTRL = 224, KEY_LSHIFT = 225, KEY_LALT = 226, KEY_LGUI = 227,
    KEY_RCTRL = 228, KEY_RSHIFT = 229, KEY_RALT = 230, KEY_RGUI = 231,

	KEY_COUNT = 256
};

typedef struct {
	uint32_t type;
	bool repeat;
	uint8_t modifiers;
	unsigned keycode;
} EventKeyboard;

typedef struct {
	uint32_t type;
	int x, y;
	int dx, dy; // relative mouse motion, or scroll
	uint8_t button;
	uint8_t clicks; // 1 = single click, 2 = double click, etc.
} EventMouse;

typedef struct {
	uint32_t type;
	char text[32];
} EventText;

typedef union {
	uint32_t type;
	EventKeyboard key;
	EventMouse mouse;
	EventText text;
} Event;

// so we dont have to type out all the procs a bunch
#define V_gl_for_each_proc(f) \
	f(ClearColor, CLEARCOLOR) \
	f(Clear, CLEAR) \
	f(BindBuffer, BINDBUFFER) \
	f(BufferData, BUFFERDATA) \
	f(GenBuffers, GENBUFFERS) \
	f(DeleteBuffers, DELETEBUFFERS) \
	f(GetError, GETERROR) \
	f(GenVertexArrays, GENVERTEXARRAYS) \
	f(DeleteVertexArrays, DELETEVERTEXARRAYS) \
	f(BindVertexArray, BINDVERTEXARRAY) \
	f(VertexAttribPointer, VERTEXATTRIBPOINTER) \
	f(VertexAttribIPointer, VERTEXATTRIBIPOINTER) \
	f(GetUniformLocation, GETUNIFORMLOCATION) \
	f(Uniform1f, UNIFORM1F) \
	f(Uniform2f, UNIFORM2F) \
	f(Uniform3f, UNIFORM3F) \
	f(Uniform4f, UNIFORM4F) \
	f(Uniform1i, UNIFORM1I) \
	f(Uniform2i, UNIFORM2I) \
	f(Uniform3i, UNIFORM3I) \
	f(Uniform4i, UNIFORM4I) \
	f(Uniform1ui, UNIFORM1UI) \
	f(UniformMatrix2fv, UNIFORMMATRIX2FV) \
	f(UniformMatrix3fv, UNIFORMMATRIX3FV) \
	f(UniformMatrix4fv, UNIFORMMATRIX4FV) \
	f(UseProgram, USEPROGRAM) \
	f(DrawElements, DRAWELEMENTS) \
	f(CreateShader, CREATESHADER) \
	f(ShaderSource, SHADERSOURCE) \
	f(GetShaderiv, GETSHADERIV) \
	f(GetShaderInfoLog, GETSHADERINFOLOG) \
	f(CreateProgram, CREATEPROGRAM) \
	f(GetProgramiv, GETPROGRAMIV) \
	f(GetProgramInfoLog, GETPROGRAMINFOLOG) \
	f(DeleteShader, DELETESHADER) \
	f(DetachShader, DETACHSHADER) \
	f(DeleteProgram, DELETEPROGRAM) \
	f(AttachShader, ATTACHSHADER) \
	f(LinkProgram, LINKPROGRAM) \
	f(GetAttribLocation, GETATTRIBLOCATION) \
	f(CompileShader, COMPILESHADER) \
	f(Enable, ENABLE) \
	f(Disable, DISABLE) \
	f(GetIntegerv, GETINTEGERV) \
	f(DebugMessageCallback, DEBUGMESSAGECALLBACK) \
	f(DebugMessageControl, DEBUGMESSAGECONTROL) \
	f(DrawArrays, DRAWARRAYS) \
	f(DrawArraysInstanced, DRAWARRAYSINSTANCED) \
	f(DrawElementsInstanced, DRAWELEMENTSINSTANCED) \
	f(EnableVertexAttribArray, ENABLEVERTEXATTRIBARRAY) \
	f(PolygonMode, POLYGONMODE) \
	f(Flush, FLUSH) \
	f(Finish, FINISH) \
	f(Viewport, VIEWPORT) \
	f(GenFramebuffers, GENFRAMEBUFFERS) \
	f(DeleteFramebuffers, DELETEFRAMEBUFFERS) \
	f(BindFramebuffer, BINDFRAMEBUFFER) \
	f(FramebufferTexture2D, FRAMEBUFFERTEXTURE2D) \
	f(CheckFramebufferStatus, CHECKFRAMEBUFFERSTATUS) \
	f(GenTextures, GENTEXTURES) \
	f(DeleteTextures, DELETETEXTURES) \
	f(BindTexture, BINDTEXTURE) \
	f(TexImage2D, TEXIMAGE2D) \
	f(TexImage2DMultisample, TEXIMAGE2DMULTISAMPLE) \
	f(ActiveTexture, ACTIVETEXTURE) \
	f(TexParameteri, TEXPARAMETERI) \
	f(GenerateMipmap, GENERATEMIPMAP) \
	f(BlendFunc, BLENDFUNC) \
	f(BlendEquation, BLENDEQUATION) \
	f(DepthFunc, DEPTHFUNC) \
	f(DepthMask, DEPTHMASK) \
	f(ObjectLabel, OBJECTLABEL) \
	f(GetObjectLabel, GETOBJECTLABEL)

typedef struct {
#define gl_declare_proc(lower, upper) PFNGL##upper##PROC lower;
V_gl_for_each_proc(gl_declare_proc)
#undef gl_declare_proc
} GLProcs;

#define WINDOW_CREATE_HIDDEN 0x01
#define WINDOW_CREATE_DONT_QUIT_ON_ERROR 0x02
// press u to toggle shader uniform editor
#define WINDOW_ENABLE_UNIFORM_EDITOR 0x04

typedef struct {
	GLuint id;
	GLuint item_size;
	GLuint count;
} GLVBO;

typedef struct {
	GLuint id;
	GLuint program_id;
	GLuint count;
} GLVAO;

typedef struct {
	GLuint id;
	GLuint count;
} GLIBO;

typedef struct {
	GLenum min_filter, mag_filter, wrap_mode_s, wrap_mode_t;
	bool mipmaps;
} TextureConf;

typedef struct {
	GLuint id;
	vec2i size;
#if DEBUG
	// only used for auto-reload
	int index;
	char *filename;
	TextureConf conf;
	struct timespec last_modified;
#endif
} Texture;

typedef struct {
	GLVBO vbo;
	GLVAO vao;
	GLIBO ibo;
} GLText;

typedef struct {
	GLuint id;
	struct V_ConfigurableUniform *configurable_uniforms;

#if DEBUG

	// only used for auto-reload
	char *vfilename, *ffilename;
	struct timespec last_modified;
#endif
} GLProgram;

typedef struct {
	GLVBO vbo;
	GLVAO vao;
	vec3 min_coord, max_coord;
} Model;

#define gl_vbo_new(type, label) gl_vbo_new_with_size(sizeof (type), label)
#define gl_vbo_set_static_data(vbo, data, count) \
	gl_vbo_set_data_with_item_size(vbo, data, count, GL_STATIC_DRAW, sizeof *data)
#define gl_vbo_set_stream_data(vbo, data, count) \
	gl_vbo_set_data_with_item_size(vbo, data, count, GL_STREAM_DRAW, sizeof *data)

#define gl_vao_add_data1f(vao, vbo, attr_name, type, member) gl_vao_add_data1f_offset(vao, vbo, attr_name, sizeof (type), &((const type *)0)->member)
#define gl_vao_add_data2f(vao, vbo, attr_name, type, member) gl_vao_add_data2f_offset(vao, vbo, attr_name, sizeof (type), &((const type *)0)->member)
#define gl_vao_add_data3f(vao, vbo, attr_name, type, member) gl_vao_add_data3f_offset(vao, vbo, attr_name, sizeof (type), &((const type *)0)->member)
#define gl_vao_add_data4f(vao, vbo, attr_name, type, member) gl_vao_add_data4f_offset(vao, vbo, attr_name, sizeof (type), &((const type *)0)->member)
#define gl_vao_add_data1i(vao, vbo, attr_name, type, member) gl_vao_add_data1i_offset(vao, vbo, attr_name, sizeof (type), &((const type *)0)->member)
#define gl_vao_add_data2i(vao, vbo, attr_name, type, member) gl_vao_add_data2i_offset(vao, vbo, attr_name, sizeof (type), &((const type *)0)->member)
#define gl_vao_add_data3i(vao, vbo, attr_name, type, member) gl_vao_add_data3i_offset(vao, vbo, attr_name, sizeof (type), &((const type *)0)->member)
#define gl_vao_add_data4i(vao, vbo, attr_name, type, member) gl_vao_add_data4i_offset(vao, vbo, attr_name, sizeof (type), &((const type *)0)->member)

#define gl_vao_render(vao, ibo) gl_vao_render_with_mode(vao, ibo, GL_TRIANGLES)
#define gl_vao_render_lines(vao, ibo) gl_vao_render_with_mode(vao, ibo, GL_LINES)

#define gl_ibo_set_static_data(ibo, indices, count) \
	gl_ibo_set_data_with_usage(ibo, indices, count, GL_STATIC_DRAW)
#define gl_ibo_set_stream_data(ibo, indices, count) \
	gl_ibo_set_data_with_usage(ibo, indices, count, GL_STREAM_DRAW)

#define gl_program_attrib_location(prog, attr) (gl.GetAttribLocation((prog)->id, (attr)))
#define gl_program_uniform_location(prog, u) (gl.GetUniformLocation((prog)->id, (u)))
#define gl_program_has_attrib(prog, attr) (gl_program_attrib_location(prog, attr) >= 0)
#define gl_program_has_uniform(prog, u) (gl_program_attrib_location(prog, u) >= 0)

V_DECL void gl_text_delete(GLText *text);

#endif // V_GL

// ---- FUNCTION/VARIABLE DECLARATIONS ----

V_DECL uint32_t rand_u32(void);

#if V_DEBUG_MEMORY

#if !__linux__
#error "V_DEBUG_MEMORY is only available on Linux for now."
#endif

// valgrind is nice, but it's annoying to have to generate a suppressions file, it's really slow (because it does
// a lot of things), etc.
V_DECL void *V_debug_malloc(size_t, const char *, int, int);
V_DECL void *V_debug_calloc(size_t, size_t, const char *, int);
V_DECL void *V_debug_realloc(void *, size_t, const char *, int);
V_DECL void  V_debug_free(void *, const char *, int);
#endif

// ---- FUNCTION DEFINITIONS ----
// --- MISC ---
V_DECL uint16_t *memchr16(uint16_t *mem, uint16_t c, size_t n) {
	uint16_t *p, *end = mem + n;
	for (p = mem; p != end; ++p) {
		if_unlikely (*p == c)
			return p;
	}
	return NULL;
}
V_DECL const uint16_t *memchr16_const(const uint16_t *mem, uint16_t c, size_t n) {
	const uint16_t *p, *end = mem + n;
	for (p = mem; p != end; ++p) {
		if_unlikely (*p == c)
			return p;
	}
	return NULL;
}
V_DECL uint32_t *memchr32(uint32_t *mem, uint32_t c, size_t n) {
	uint32_t *p, *end = mem + n;
	for (p = mem; p != end; ++p) {
		if_unlikely (*p == c)
			return p;
	}
	return NULL;
}
V_DECL const uint32_t *memchr32_const(const uint32_t *mem, uint32_t c, size_t n) {
	const uint32_t *p, *end = mem + n;
	for (p = mem; p != end; ++p) {
		if_unlikely (*p == c)
			return p;
	}
	return NULL;
}
V_DECL uint64_t *memchr64(uint64_t *mem, uint64_t c, size_t n) {
	uint64_t *p, *end = mem + n;
	for (p = mem; p != end; ++p) {
		if_unlikely (*p == c)
			return p;
	}
	return NULL;
}
V_DECL const uint64_t *memchr64_const(const uint64_t *mem, uint64_t c, size_t n) {
	const uint64_t *p, *end = mem + n;
	for (p = mem; p != end; ++p) {
		if_unlikely (*p == c)
			return p;
	}
	return NULL;
}

#if V_DEBUG_MEMORY

typedef struct {
	char file[20];
	int line;
} V_MemAllocation;

// if you have more than 1M allocations at once there's something wrong with you
#define V_MAX_ALLOCATIONS (1ul<<20) // must be a power of 2
static V_MemAllocation V_allocations[V_MAX_ALLOCATIONS];

void *mmap(void *, size_t, int, int, int, long);
int munmap(void *, size_t);
void *mremap(void *, size_t, size_t, int, ...);
#define PROT_READ  0x1
#define PROT_WRITE 0x2
#define PROT_EXEC  0x4
#define MAP_SHARED    0x01
#define MAP_PRIVATE   0x02
#define MAP_ANONYMOUS 0x20
#define MAP_FAILED ((void *)-1)
#define MREMAP_MAYMOVE 1

#define V_DEBUG_MEM_MAGIC 0xdeb56565u
typedef struct {
	char file[16];
	uint32_t alloc_idx; // index within V_allocations
	int32_t line;
	uint32_t npages;
	uint32_t magic;
} V_MemHeader;
static const char V__memheader_size_assertion[(sizeof(V_MemHeader) == 32) * 2 - 1] = {'a'};

V_DECL void *V_debug_malloc(size_t n, const char *file, int line, int fill_with) {
	void *memory, *ret;
	V_MemHeader *header;
	size_t alloc_size;
	if (n == 0) return NULL;
	if (n > 0xffffffff000) return NULL; // too much memory (>16TB; # of pages must fit in a 32-bit integer)

	// space for file & line + round up to nearest multiple of 4K
	alloc_size = (n + sizeof(V_MemHeader) + 0xfff) & ~(size_t)0xfff;

	memory = mmap(NULL, alloc_size, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
	if (memory == MAP_FAILED) {
		return NULL;
	}

	// place returned memory at very end of allocation so we get a segfault if you read/write past the end
	ret = (char *)memory + alloc_size - n;
	ret = (void *)((size_t)ret & ~(size_t)0xf); // make sure returned memory is aligned to 16 bytes
	header = (V_MemHeader *)ret - 1;
	header->line = line;
	header->magic = V_DEBUG_MEM_MAGIC;
	header->npages = (uint32_t)(alloc_size >> 12);
	{
		size_t fsz = strlen(file) + 1;
		if (fsz > sizeof header->file) fsz = sizeof header->file;
		memcpy(header->file, file, fsz);
	}
	{
		V_MemAllocation *allocation;
		uint32_t idx;
		do {
			idx = rand_u32() & (V_MAX_ALLOCATIONS - 1);
			allocation = &V_allocations[idx];
		} while (allocation->line != 0);
		memcpy(allocation->file, header->file, sizeof header->file);
		allocation->line = header->line;
		header->alloc_idx = idx;
	}
	memset(ret, fill_with, n);
	return ret;
}

V_DECL void *V_debug_calloc(size_t n, size_t size, const char *file, int line) {
	size_t bytes;

	if (n == 0) return NULL;

	if (size > (~(size_t)0) / n) {
		// multiplication would overflow
		print("[MEM] Trying to allocate %zu x %zu bytes at %s:%d\n", n, size, file, line);
		return NULL;
	}
	
	bytes = n * size;
	return V_debug_malloc(bytes, file, line, 0);
}

V_DECL void *V_debug_realloc(void *mem, size_t size, const char *file, int line) {
	if (mem == NULL) {
		return V_debug_malloc(size, file, line, 0xcd);
	} else if (size == 0) {
		V_debug_free(mem, file, line);
		return NULL;
	} else {
		// just allocate some new memory, and copy the old memory over.
		// it's easier and has the added benefit that the new memory address will never be the
		// same as the old one (which can catch some bugs)
		const V_MemHeader *old_header = (V_MemHeader *)mem - 1;
		void *new_mem = V_debug_malloc(size, file, line, 0xcd);
		void *old_mem_end = (void *)(((size_t)old_header & ~(size_t)0xfff) + (old_header->npages << 12));
		size_t bytes_to_copy = (size_t)old_mem_end - (size_t)mem;
		if (bytes_to_copy > size) bytes_to_copy = size;
		memcpy(new_mem, mem, bytes_to_copy);
		V_debug_free(mem, file, line);
		return new_mem;
	}
}

V_DECL void  V_debug_free(void *mem, const char *file, int line) {
	V_MemHeader *header;
	void *base_addr;
	if (!mem) return;

	header = (V_MemHeader *)mem - 1;
	if (header->magic != V_DEBUG_MEM_MAGIC) {
		print("[MEM] Invalid free at %s:%d\n", file, line);
		abort();
	}
	{
		V_MemAllocation *allocation = &V_allocations[header->alloc_idx];
		allocation->file[0] = 0;
		allocation->line = 0;
	}
	base_addr = (void *)((size_t)header & ~(size_t)0xfff);
	munmap(base_addr, header->npages << 12);
}

V_DECL void vlib_memory_check(void) {
	size_t i;
	int any_leaks = 0;
	for (i = 0; i < V_MAX_ALLOCATIONS; ++i) {
		V_MemAllocation *allocation = &V_allocations[i];
		if_unlikely (allocation->line) {
			any_leaks = 1;
			print("[MEM] Memory leak from %s:%d\n", allocation->file, allocation->line);
		}
	}
	if (!any_leaks) {
		print("No memory leaks found.\n");
	}
}
#endif

// --- STRINGS ---

V_DECL char *str_dup(const char *str) {
	size_t bytes = strlen(str) + 1;
	char *ret = (char *)malloc(bytes);
	if (ret)
		memcpy(ret, str, bytes);
	return ret;
}

V_DECL bool str_is_prefix(const char *str, const char *prefix) {
	return strncmp(str, prefix, strlen(prefix)) == 0;
}

V_DECL bool str_is_suffix(const char *str, const char *suffix) {
	size_t str_len = strlen(str), suffix_len = strlen(suffix);
	if (str_len < suffix_len) return false;
	return memcmp(str + str_len - suffix_len, suffix, suffix_len) == 0;
}

V_DECL void strn_cpy(char *dst, size_t dst_size, const char *src, size_t src_len) {
	// number of characters to copy (not including null terminator)
	size_t len = strlen(src);
	assert(dst_size > 0);
	if (len > src_len)
		len = src_len;
	if (len > dst_size - 1)
		len = dst_size - 1;
	memmove(dst, src, len);
	dst[len] = '\0';
}

// safer version of strcpy
V_DECL void str_cpy(char *dst, size_t dst_size, const char *src) {
	strn_cpy(dst, dst_size, src, (size_t)-1);
}

V_DECL void strn_cat(char *dst, size_t dst_size, const char *src, size_t src_size) {
	size_t src_len = strlen(src), dst_len = strlen(dst);
	if (dst_len >= dst_size) {
		// dst doesn't actually contain a null-terminated string!
		assert(0);
		return;
	}
	if (src_len > src_size)
		src_len = src_size;

	if (src_len < dst_size - dst_len) {
		memmove(dst + dst_len, src, src_len);
		dst[src_len + dst_len] = '\0';
	} else {
		memmove(dst + dst_len, src, dst_size - dst_len - 1);
		dst[dst_size - 1] = '\0';
	}
}

V_DECL void str_cat(char *dst, size_t dst_size, const char *src) {
	strn_cat(dst, dst_size, src, (size_t)-1);
}

/*
does this predicate hold for all the characters of s. predicate is int (*)(int) instead
of bool (*)(char) so that you can pass isprint, etc. to it.
*/
V_DECL bool str_satisfies(const char *s, int (*predicate)(int)) {
	const char *p;
	for (p = s; *p; ++p)
		if (!predicate(*p))
			return false;
	return true;
}

// like strcmp, but can be used as the comparison function for qsort
V_DECL int strcmp_qsort(const void *av, const void *bv) {
	const char *const *a = av, *const *b = bv;
	return strcmp(*a, *b);
}

#if V_WINDOWS
extern int _stricmp(const char *, const char *);
#else
extern int strcasecmp(const char *, const char *);
#endif

// case-insensitive version of strcmp
V_DECL int strcmp_case_insensitive(const char *a, const char *b) {
	// (yes we could implement this ourselves, but we'd need to handle UTF-8 properly)
#if V_WINDOWS
	return _stricmp(a, b);
#else
	return strcasecmp(a, b);
#endif
}

// function to be passed into qsort for case insensitive sorting
V_DECL int strcmp_case_insensitive_qsort(const void *av, const void *bv) {
	const char *const *a = av, *const *b = bv;
	return strcmp_case_insensitive(*a, *b);
}

V_DECL size_t str_count_char(const char *s, char c) {
	const char *p;
	size_t total = 0;
	for (p = s; *p; ++p)
		total += *p == c;
	return total;
}

// -- utf8 --


V_DECL bool unicode_is_start_of_code_point(uint8_t byte) {
	// see https://en.wikipedia.org/wiki/UTF-8#Encoding
	// continuation bytes are of the form 10xxxxxx
	return (byte & 0xC0) != 0x80;
}

// A lot like mbrtoc32. Doesn't depend on the locale though, for one thing.
// *c will be filled with the next UTF-8 code point in `str`. `bytes` refers to the maximum
// number of bytes that can be read from `str`.
// Returns:
// 0 - if a NULL character was encountered or if `bytes` is 0
// (size_t)-1 - on invalid UTF-8
// (size_t)-2 - on incomplete code point (str should be longer)
// other - the number of bytes read from `str`.
V_DECL size_t unicode_utf8_to_utf32(uint32_t *c, const char *str, size_t bytes) {
	// it's easier to do things with unsigned integers
	uint8_t const *p = (uint8_t const *)str;
	uint32_t first_byte;
	if (bytes == 0) {
		*c = 0;
		return 0;
	}

	first_byte = *p++;
	
	if (first_byte & 0x80) {
		if ((first_byte & 0xE0) == 0xC0) {
			// two-byte code point
			if (bytes >= 2) {
				uint32_t second_byte = *p++;
				uint32_t value = (first_byte & 0x1F) << 6
					| (second_byte & 0x3F);
				*c = value;
				return 2;
			} else {
				// incomplete code point
				*c = 0;
				return (size_t)-2;
			}
		}
		if ((first_byte & 0xF0) == 0xE0) {
			// three-byte code point
			if (bytes >= 3) {
				uint32_t second_byte = *p++;
				uint32_t third_byte = *p++;
				uint32_t value = (first_byte & 0x0F) << 12
					| (second_byte & 0x3F) << 6
					| (third_byte & 0x3F);
				if (value < 0xD800 || value > 0xDFFF) {
					*c = value;
					return 3;
				} else {
					// reserved for UTF-16 surrogate halves
					*c = 0;
					return (size_t)-1;
				}
			} else {
				// incomplete
				*c = 0;
				return (size_t)-2;
			}
		}
		if ((first_byte & 0xF8) == 0xF0) {
			// four-byte code point
			if (bytes >= 4) {
				uint32_t second_byte = *p++;
				uint32_t third_byte = *p++;
				uint32_t fourth_byte = *p++;
				uint32_t value = (first_byte & 0x07) << 18
					| (second_byte & 0x3F) << 12
					| (third_byte  & 0x3F) << 6
					| (fourth_byte & 0x3F);
				if (value <= 0x10FFFF) {
					*c = value;
					return 4;
				} else {
					// Code points this big can't be encoded by UTF-16 and so are invalid UTF-8.
					*c = 0;
					return (size_t)-1;
				}
			} else {
				// incomplete
				*c = 0;
				return (size_t)-2;
			}
		}
		// invalid UTF-8
		*c = 0;
		return (size_t)-1;
	} else {
		// ASCII character
		if (first_byte == 0) {
			*c = 0;
			return 0;
		}
		*c = first_byte;
		return 1;
	}
}

// A lot like c32rtomb
// Converts a UTF-32 codepoint to a UTF-8 string. Writes at most 4 bytes to s.
// NOTE: It is YOUR JOB to null-terminate your string if the UTF-32 isn't null-terminated!
// Returns the number of bytes written to s, or (size_t)-1 on invalid UTF-32.
V_DECL size_t unicode_utf32_to_utf8(char *s, uint32_t c32) {
	uint8_t *p = (uint8_t *)s;
	if (c32 <= 0x7F) {
		// ASCII
		*p = (uint8_t)c32;
		return 1;
	} else if (c32 <= 0x7FF) {
		// two bytes needed
		*p++ = (uint8_t)(0xC0 | (c32 >> 6));
		*p   = (uint8_t)(0x80 | (c32 & 0x3F));
		return 2;
	} else if (c32 <= 0x7FFF) {
		if (c32 < 0xD800 || c32 > 0xDFFF) {
			*p++ = (uint8_t)(0xE0 | ( c32 >> 12));
			*p++ = (uint8_t)(0x80 | ((c32 >> 6) & 0x3F));
			*p   = (uint8_t)(0x80 | ( c32       & 0x3F));
			return 3;
		} else {
			// UTF-16 surrogate halves
			*p = 0;
			return (size_t)-1;
		}
	} else if (c32 <= 0x10FFFF) {
		*p++ = (uint8_t)(0xF0 | ( c32 >> 18));
		*p++ = (uint8_t)(0x80 | ((c32 >> 12) & 0x3F));
		*p++ = (uint8_t)(0x80 | ((c32 >>  6) & 0x3F));
		*p   = (uint8_t)(0x80 | ( c32        & 0x3F));
		return 4;
	} else {
		// code point too big
		*p = 0;
		return (size_t)-1;
	}
}

// --- MATH ---

#if V_GNUC
V_DECL uint16_t byte_swap16(uint16_t x) {
	return __builtin_bswap16(x);
}

V_DECL uint32_t byte_swap32(uint32_t x) {
	return __builtin_bswap32(x);
}

V_DECL uint64_t byte_swap64(uint64_t x) {
	return __builtin_bswap64(x);
}
#else
V_DECL uint16_t byte_swap16(uint16_t x) {
	return (uint16_t)((x & 0xff) << 8
		| x >> 8);
}

V_DECL uint32_t byte_swap32(uint32_t x) {
	return (x & 0xff) << 24
		| (x & 0x0000ff00) << 8
		| (x & 0x00ff0000) >> 8
		| x >> 24;
}

V_DECL uint64_t byte_swap64(uint64_t x) {
	return (uint64_t)byte_swap32((uint32_t)x) << 32
		| (uint64_t)byte_swap32((uint32_t)(x >> 32));
}
#endif

V_DECL unsigned char bit_popcount(uint64_t x) {
#if V_GNUC
	return (unsigned char)__builtin_popcountll(x);
#else
	unsigned char count;
	for (count = 0; x; ++count)
		x &= x-1;
	return count;
#endif
}

V_DECL unsigned char bit_count_leading_zeros64(uint64_t x) {
	if (x == 0) return 64;
#if V_GNUC
	return (unsigned char)__builtin_clzll(x);
#else
	{
	uint64_t bit;
	unsigned char zeros = 0;
	for (bit = (uint64_t)1 << 63; x & bit; bit >>= 1, ++zeros);
	return zeros;
	}
#endif
}

V_DECL unsigned char bit_count_trailing_zeros64(uint64_t x) {
	if (x == 0) return 64;
#if V_GNUC
	return (unsigned char)__builtin_ctzll(x);
#else
	{
	uint64_t bit;
	unsigned char zeros = 0;
	for (bit = 1; x & bit; bit <<= 1, ++zeros);
	return zeros;
	}
#endif

}

V_DECL float lerp(float a, float b, float x) {
	return x * (b-a) + a;
}
V_DECL float min(float a, float b) {
	if (a < b)
		return a;
	return b;
}
V_DECL float max(float a, float b) {
	if (a > b)
		return a;
	return b;
}
V_DECL float clamp(float x, float a, float b) {
	if (x <= a) return a;
	if (x >= b) return b;
	return x;
}

V_DECL int absi(int x) {
	if (x >= 0) return x;
	return -x;
}
V_DECL int mini(int a, int b) {
	if (a < b)
		return a;
	return b;
}
V_DECL int maxi(int a, int b) {
	if (a > b)
		return a;
	return b;
}
V_DECL int clampi(int x, int a, int b) {
	if (x <= a) return a;
	if (x >= b) return b;
	return x;
}

V_DECL float modf(float a, float b) {
	float x = fmodf(a, b) + b;
	if (x >= b) x -= b;
	return x;
}

V_DECL int modi(int a, int b) {
	int x = a % b + b;
	if (x >= b) x -= b;
	return x;
}

V_DECL double lerpd(double a, double b, double x) {
	return x * (b-a) + a;
}
V_DECL double mind(double a, double b) {
	if (a < b)
		return a;
	return b;
}
V_DECL double maxd(double a, double b) {
	if (a > b)
		return a;
	return b;
}
V_DECL double clampd(double x, double a, double b) {
	if (x <= a) return a;
	if (x >= b) return b;
	return x;
}


V_DECL float closerf(float x, float a, float b) {
	float a_diff = x > a ? x-a : a-x;
	float b_diff = x > b ? x-b : b-x;
	if (a_diff < b_diff)
		return a;
	else
		return b;
}

V_DECL int closeri(int x, int a, int b) {
	int a_diff = x > a ? x-a : a-x;
	int b_diff = x > b ? x-b : b-x;
	if (a_diff < b_diff)
		return a;
	else
		return b;
}

// -- vectors and matrices --
V_VARDECL const vec2 ZERO2 = {0, 0};
V_VARDECL const vec3 ZERO3 = {0, 0, 0};
V_VARDECL const vec4 ZERO4 = {0, 0, 0, 0};

V_VARDECL const vec2i ZERO2I = {0, 0};
V_VARDECL const vec3i ZERO3I = {0, 0, 0};
V_VARDECL const vec4i ZERO4I = {0, 0, 0, 0};

V_VARDECL const mat3 IDENTITY3 = {{
	1, 0, 0,
	0, 1, 0,
	0, 0, 1
}};

V_VARDECL const mat4 IDENTITY4 = {{
	1, 0, 0, 0,
	0, 1, 0, 0,
	0, 0, 1, 0,
	0, 0, 0, 1
}};

V_DECL void print2(vec2 v) {
	print("(%.3f, %.3f)", v.x, v.y);
}
V_DECL void println2(vec2 v) {
	print("(%.3f, %.3f)\n", v.x, v.y);
}
V_DECL void print3(vec3 v) {
	print("(%.3f, %.3f, %.3f)", v.x, v.y, v.z);
}
V_DECL void println3(vec3 v) {
	print("(%.3f, %.3f, %.3f)\n", v.x, v.y, v.z);
}
V_DECL void print4(vec4 v) {
	print("(%.3f, %.3f, %.3f, %.3f)", v.x, v.y, v.z, v.w);
}
V_DECL void println4(vec4 v) {
	print("(%.3f, %.3f, %.3f, %.3f)\n", v.x, v.y, v.z, v.w);
}
V_DECL void print2i(vec2i v) {
	print("(%d, %d)", v.x, v.y);
}
V_DECL void println2i(vec2i v) {
	print("(%d, %d)\n", v.x, v.y);
}
V_DECL void print3i(vec3i v) {
	print("(%d, %d, %d)", v.x, v.y, v.z);
}
V_DECL void println3i(vec3i v) {
	print("(%d, %d, %d)\n", v.x, v.y, v.z);
}
V_DECL void print4i(vec4i v) {
	print("(%d, %d, %d, %d)", v.x, v.y, v.z, v.w);
}
V_DECL void println4i(vec4i v) {
	print("(%d, %d, %d, %d)\n", v.x, v.y, v.z, v.w);
}
V_DECL void printlnM3(const mat3 *m) {
	const float *v = m->v;
	printf(
		"|%.3f %.3f %.3f|\n"
		"|%.3f %.3f %.3f|\n"
		"|%.3f %.3f %.3f|\n\n",
		v[0], v[3], v[6],
		v[1], v[4], v[7],
		v[2], v[5], v[8]);
}
V_DECL void printlnM4(const mat4 *m) {
	const float *v = m->v;
	printf(
		"|%.3f %.3f %.3f %.3f|\n"
		"|%.3f %.3f %.3f %.3f|\n"
		"|%.3f %.3f %.3f %.3f|\n"
		"|%.3f %.3f %.3f %.3f|\n\n",
		v[0], v[4], v[ 8], v[12],
		v[1], v[5], v[ 9], v[13],
		v[2], v[6], v[10], v[14],
		v[3], v[7], v[11], v[15]);
}

V_DECL vec3 vec3xy_z(vec2 xy, float z) {
	vec3 v;
	v.x = xy.x; v.y = xy.y; v.z = z;
	return v;
}
V_DECL vec3 vec3xz_y(vec2 xz, float y) {
	vec3 v;
	v.x = xz.x; v.y = y; v.z = xz.y;
	return v;
}
V_DECL vec4 vec4xyz_w(vec3 xyz, float w) {
	vec4 v;
	v.x = xyz.x; v.y = xyz.y; v.z = xyz.z; v.w = w;
	return v;
}

V_DECL vec2 polar(float r, float theta) {
	return Vec2(r * cosf(theta), r * sinf(theta));
}
V_DECL vec3 spherical(float r, float alpha, float beta) {
	return Vec3(r * cosf(alpha) * sinf(beta), r * sinf(alpha) * sinf(beta), r * cosf(beta));
}
V_DECL vec2 vec3_xy(vec3 v) {
	return Vec2(v.x, v.y);
}
V_DECL vec2 vec3_xz(vec3 v) {
	return Vec2(v.x, v.z);
}
V_DECL vec2 vec2_from_2i(vec2i v) {
	return Vec2((float)v.x, (float)v.y);
}
V_DECL vec3 vec3_from_3i(vec3i v) {
	return Vec3((float)v.x, (float)v.y, (float)v.z);
}
V_DECL vec4 vec4_from_4i(vec4i v) {
	return Vec4((float)v.x, (float)v.y, (float)v.z, (float)v.w);
}

V_DECL vec2 lerp2(vec2 a, vec2 b, float x) {
	return Vec2(lerp(a.x, b.x, x), lerp(a.y, b.y, x));
}
V_DECL vec2 clamp2(vec2 v, vec2 a, vec2 b) {
	return Vec2(clamp(v.x, a.x, b.x), clamp(v.y, a.y, b.y));
}
V_DECL vec2 min2(vec2 a, vec2 b) {
	return Vec2(min(a.x, b.x), min(a.y, b.y));
}
V_DECL vec2 max2(vec2 a, vec2 b) {
	return Vec2(max(a.x, b.x), max(a.y, b.y));
}
V_DECL vec2 add2(vec2 a, vec2 b) {
	return Vec2(a.x + b.x, a.y + b.y);
}
V_DECL vec2 addc2(vec2 a, float x) {
	return Vec2(a.x + x, a.y + x);
}
V_DECL vec2 sub2(vec2 a, vec2 b) {
	return Vec2(a.x - b.x, a.y - b.y);
}
V_DECL vec2 mul2(vec2 a, vec2 b) {
	return Vec2(a.x * b.x, a.y * b.y);
}
V_DECL vec2 div2(vec2 a, vec2 b) {
	return Vec2(a.x / b.x, a.y / b.y);
}
V_DECL vec2 mod2(vec2 a, vec2 b) {
	return Vec2(modf(a.x, b.x), modf(a.y, b.y));
}
V_DECL vec2 scale2(vec2 a, float x) {
	return Vec2(a.x * x, a.y * x);
}
V_DECL float dot2(vec2 a, vec2 b) {
	return a.x * b.x + a.y * b.y;
}
V_DECL float sqlength2(vec2 v) {
	return dot2(v, v);
}
V_DECL float length2(vec2 v) {
	return sqrtf(dot2(v, v));
}
V_DECL float sqdistance2(vec2 a, vec2 b) {
	return sqlength2(sub2(a, b));
}
V_DECL float distance2(vec2 a, vec2 b) {
	return length2(sub2(a, b));
}
V_DECL vec2 normalize2(vec2 v) {
	return scale2(v, 1.0f / length2(v));
}
V_DECL vec2 abs2(vec2 v) {
	return Vec2(fabsf(v.x), fabsf(v.y));
}
V_DECL vec2 floor2(vec2 v) {
	return Vec2(floorf(v.x), floorf(v.y));
}
V_DECL vec2 ceil2(vec2 v) {
	return Vec2(ceilf(v.x), ceilf(v.y));
}
V_DECL vec2 round2(vec2 v) {
	return Vec2(roundf(v.x), roundf(v.y));
}

V_DECL vec3 lerp3(vec3 a, vec3 b, float x) {
	return Vec3(lerp(a.x, b.x, x), lerp(a.y, b.y, x), lerp(a.z, b.z, x));
}
V_DECL vec3 clamp3(vec3 v, vec3 a, vec3 b) {
	return Vec3(clamp(v.x, a.x, b.x), clamp(v.y, a.y, b.y), clamp(v.z, a.z, b.z));
}
V_DECL vec3 min3(vec3 a, vec3 b) {
	return Vec3(min(a.x, b.x), min(a.y, b.y), min(a.z, b.z));
}
V_DECL vec3 max3(vec3 a, vec3 b) {
	return Vec3(max(a.x, b.x), max(a.y, b.y), max(a.z, b.z));
}
V_DECL vec3 add3(vec3 a, vec3 b) {
	return Vec3(a.x + b.x, a.y + b.y, a.z + b.z);
}
V_DECL vec3 addc3(vec3 a, float x) {
	return Vec3(a.x + x, a.y + x, a.z + x);
}
V_DECL vec3 sub3(vec3 a, vec3 b) {
	return Vec3(a.x - b.x, a.y - b.y, a.z - b.z);
}
V_DECL vec3 mul3(vec3 a, vec3 b) {
	return Vec3(a.x * b.x, a.y * b.y, a.z * b.z);
}
V_DECL vec3 div3(vec3 a, vec3 b) {
	return Vec3(a.x / b.x, a.y / b.y, a.z / b.z);
}
V_DECL vec3 mod3(vec3 a, vec3 b) {
	return Vec3(modf(a.x, b.x), modf(a.y, b.y), modf(a.z, b.z));
}
V_DECL vec3 scale3(vec3 a, float x) {
	return Vec3(a.x * x, a.y * x, a.z * x);
}
V_DECL float dot3(vec3 a, vec3 b) {
	return a.x * b.x + a.y * b.y + a.z * b.z;
}
V_DECL float sqlength3(vec3 v) {
	return dot3(v, v);
}
V_DECL float length3(vec3 v) {
	return sqrtf(dot3(v, v));
}
V_DECL float sqdistance3(vec3 a, vec3 b) {
	return sqlength3(sub3(a, b));
}
V_DECL float distance3(vec3 a, vec3 b) {
	return length3(sub3(a, b));
}
V_DECL vec3 normalize3(vec3 v) {
	return scale3(v, 1.0f / length3(v));
}
V_DECL vec3 abs3(vec3 v) {
	return Vec3(fabsf(v.x), fabsf(v.y), fabsf(v.z));
}
V_DECL vec3 floor3(vec3 v) {
	return Vec3(floorf(v.x), floorf(v.y), floorf(v.z));
}
V_DECL vec3 ceil3(vec3 v) {
	return Vec3(ceilf(v.x), ceilf(v.y), ceilf(v.z));
}
V_DECL vec3 round3(vec3 v) {
	return Vec3(roundf(v.x), roundf(v.y), roundf(v.z));
}
V_DECL vec3 cross(vec3 u, vec3 v) {
	return Vec3(u.y*v.z - u.z*v.y, u.z*v.x - u.x*v.z, u.x*v.y - u.y*v.x);
}

V_DECL vec4 lerp4(vec4 a, vec4 b, float x) {
	return Vec4(lerp(a.x, b.x, x), lerp(a.y, b.y, x), lerp(a.z, b.z, x), lerp(a.w, b.w, x));
}
V_DECL vec4 clamp4(vec4 v, vec4 a, vec4 b) {
	return Vec4(clamp(v.x, a.x, b.x), clamp(v.y, a.y, b.y), clamp(v.z, a.z, b.z), clamp(v.w, a.w, b.w));
}
V_DECL vec4 min4(vec4 a, vec4 b) {
	return Vec4(min(a.x, b.x), min(a.y, b.y), min(a.z, b.z), min(a.w, b.w));
}
V_DECL vec4 max4(vec4 a, vec4 b) {
	return Vec4(max(a.x, b.x), max(a.y, b.y), max(a.z, b.z), max(a.w, b.w));
}
V_DECL vec4 add4(vec4 a, vec4 b) {
	return Vec4(a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w);
}
V_DECL vec4 addc4(vec4 a, float x) {
	return Vec4(a.x + x, a.y + x, a.z + x, a.w + x);
}
V_DECL vec4 sub4(vec4 a, vec4 b) {
	return Vec4(a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w);
}
V_DECL vec4 mul4(vec4 a, vec4 b) {
	return Vec4(a.x * b.x, a.y * b.y, a.z * b.z, a.w * b.w);
}
V_DECL vec4 div4(vec4 a, vec4 b) {
	return Vec4(a.x / b.x, a.y / b.y, a.z / b.z, a.w / b.w);
}
V_DECL vec4 mod4(vec4 a, vec4 b) {
	return Vec4(modf(a.x, b.x), modf(a.y, b.y), modf(a.z, b.z), modf(a.w, b.w));
}
V_DECL vec4 scale4(vec4 a, float x) {
	return Vec4(a.x * x, a.y * x, a.z * x, a.w * x);
}
V_DECL float dot4(vec4 a, vec4 b) {
	return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
}
V_DECL float sqlength4(vec4 v) {
	return dot4(v, v);
}
V_DECL float length4(vec4 v) {
	return sqrtf(dot4(v, v));
}
V_DECL float sqdistance4(vec4 a, vec4 b) {
	return sqlength4(sub4(a, b));
}
V_DECL float distance4(vec4 a, vec4 b) {
	return length4(sub4(a, b));
}
V_DECL vec4 normalize4(vec4 v) {
	return scale4(v, 1.0f / length4(v));
}
V_DECL vec4 abs4(vec4 v) {
	return Vec4(fabsf(v.x), fabsf(v.y), fabsf(v.z), fabsf(v.w));
}
V_DECL vec4 floor4(vec4 v) {
	return Vec4(floorf(v.x), floorf(v.y), floorf(v.z), floorf(v.w));
}
V_DECL vec4 ceil4(vec4 v) {
	return Vec4(ceilf(v.x), ceilf(v.y), ceilf(v.z), ceilf(v.w));
}
V_DECL vec4 round4(vec4 v) {
	return Vec4(roundf(v.x), roundf(v.y), roundf(v.z), roundf(v.w));
}

V_DECL vec3i Vec3ixy_z(vec2i xy, int z) {
	vec3i v;
	v.x = xy.x; v.y = xy.y; v.z = z;
	return v;
}
V_DECL vec3i Vec3ixz_y(vec2i xz, int y) {
	vec3i v;
	v.x = xz.x; v.y = y; v.z = xz.y;
	return v;
}
V_DECL vec4i Vec4ixyz_w(vec3i xyz, int w) {
	vec4i v;
	v.x = xyz.x; v.y = xyz.y; v.z = xyz.z; v.w = w;
	return v;
}
V_DECL vec2i vec3i_xy(vec3i v) {
	return Vec2i(v.x, v.y);
}
V_DECL vec2i vec3i_xz(vec3i v) {
	return Vec2i(v.x, v.z);
}
V_DECL vec2i vec2i_from_2f(vec2 v) {
	return Vec2i((int)v.x, (int)v.y);
}
V_DECL vec3i vec3i_from_3f(vec3 v) {
	return Vec3i((int)v.x, (int)v.y, (int)v.z);
}
V_DECL vec4i vec4i_from_4f(vec4 v) {
	return Vec4i((int)v.x, (int)v.y, (int)v.z, (int)v.w);
}

V_DECL mat3 Mat3(float a, float b, float c,
	float d, float e, float f,
	float g, float h, float i) {
	mat3 M;
	float *v = M.v;
	v[0] = a; v[1] = d; v[2] = g;
	v[3] = b; v[4] = e; v[5] = h;
	v[6] = c; v[7] = f; v[8] = i;
	return M;
}

V_DECL mat3 mat3_with_cols(vec3 cx, vec3 cy, vec3 cz) {
	mat3 M;
	float *v = M.v;
	v[0] = cx.x; v[1] = cx.y; v[2] = cx.z;
	v[3] = cy.x; v[4] = cy.y; v[5] = cy.z;
	v[6] = cz.x; v[7] = cz.y; v[8] = cz.z;
	return M;
}

V_DECL mat4 Mat4(float a, float b, float c, float d,
	float e, float f, float g, float h,
	float i, float j, float k, float l,
	float m, float n, float o, float p) {
	mat4 M;
	float *v = M.v;
	// column-major, because that's what OpenGL uses
	v[ 0] = a; v[ 1] = e; v[ 2] = i; v[ 3] = m;
	v[ 4] = b; v[ 5] = f; v[ 6] = j; v[ 7] = n;
	v[ 8] = c; v[ 9] = g; v[10] = k; v[11] = o;
	v[12] = d; v[13] = h; v[14] = l; v[15] = p;
	return M;
}

V_DECL mat4 mat4_with_cols(vec4 cx, vec4 cy, vec4 cz, vec4 cw) {
	mat4 M;
	float *v = M.v;
	v[ 0] = cx.x; v[ 1] = cx.y; v[ 2] = cx.z; v[ 3] = cx.w;
	v[ 4] = cy.x; v[ 5] = cy.y; v[ 6] = cy.z; v[ 7] = cy.w;
	v[ 8] = cz.x; v[ 9] = cz.y; v[10] = cz.z; v[11] = cz.w;
	v[12] = cw.x; v[13] = cw.y; v[14] = cw.z; v[15] = cw.w;
	return M;
}

V_DECL mat3 mat3_yaw(float yaw) {
	float c = cosf(yaw), s = sinf(yaw);
	return Mat3(
		 c, 0, s,
		 0, 1, 0,
		-s, 0, c
	);
}

V_DECL mat3 mat3_pitch(float pitch) {
	float c = cosf(pitch), s = sinf(pitch);
	return Mat3(
		1, 0,  0,
		0, c, -s,
		0, s,  c
	);
}


V_DECL mat4 mat4_translate(vec3 t) {
	return Mat4(
		1, 0, 0, t.x,
		0, 1, 0, t.y,
		0, 0, 1, t.z,
		0, 0, 0, 1
	);
}

V_DECL mat3 mat3_mul(const mat3 *a, const mat3 *b) {
	mat3 out = {0};
	float *p = out.v;
	for (int i = 0; i < 3; ++i) {
		for (int j = 0; j < 3; ++j, ++p) {
			const float *as = &a->v[j];
			const float *bs = &b->v[3*i];
			*p = as[0]*bs[0] + as[3]*bs[1] + as[6]*bs[2];
		}
	}
	return out;
}

V_DECL mat3 mat3_scale(vec3 scale) {
	return Mat3(
		scale.x, 0, 0,
		0, scale.y, 0,
		0, 0, scale.z
	);
}

V_DECL mat4 mat4_from_mat3(mat3 m) {
	const float *v = m.v;
	return Mat4(
		v[0], v[3], v[6], 0,
		v[1], v[4], v[7], 0,
		v[2], v[5], v[8], 0,
		0,    0,    0,    1
	);
}

V_DECL vec3 transform3(const mat3 *m, vec3 a) {
	float const *v = m->v;
	vec3 f_x = Vec3(v[ 0], v[ 1], v[ 2]);
	vec3 f_y = Vec3(v[ 3], v[ 4], v[ 5]);
	vec3 f_z = Vec3(v[ 6], v[ 7], v[ 8]);
	return add3(
		scale3(f_x, a.x), add3(
		scale3(f_y, a.y),
		scale3(f_z, a.z)));
	
}

V_DECL vec4 transform4(const mat4 *m, vec4 a) {
	float const *v = m->v;
	vec4 f_x = Vec4(v[ 0], v[ 1], v[ 2], v[ 3]);
	vec4 f_y = Vec4(v[ 4], v[ 5], v[ 6], v[ 7]);
	vec4 f_z = Vec4(v[ 8], v[ 9], v[10], v[11]);
	vec4 f_w = Vec4(v[12], v[13], v[14], v[15]);
	return add4(
		scale4(f_x, a.x), add4(
		scale4(f_y, a.y), add4(
		scale4(f_z, a.z),
		scale4(f_w, a.w))));
}

V_DECL vec3 transform4v3(const mat4 *m, vec3 a) {
	float const *v = m->v;
	vec3 f_x = Vec3(v[ 0], v[ 1], v[ 2]);
	vec3 f_y = Vec3(v[ 4], v[ 5], v[ 6]);
	vec3 f_z = Vec3(v[ 8], v[ 9], v[10]);
	vec3 f_c = Vec3(v[12], v[13], v[14]);
	return add3(f_c, add3(
		scale3(f_x, a.x), add3(
		scale3(f_y, a.y),
		scale3(f_z, a.z))));
}

/*
4x4 perspective matrix.
fov - field of view in radians, aspect - width:height aspect ratio, z_near/z_far - clipping planes
math stolen from gluPerspective (https://www.khronos.org/registry/OpenGL-Refpages/gl2.1/xhtml/gluPerspective.xml)
*/
V_DECL mat4 mat4_perspective(float fov, float aspect, float z_near, float z_far) {
	float f = 1.0f / tanf(fov / 2.0f);
	return Mat4(
		f/aspect, 0, 0, 0,
		0, f, 0, 0,
		0, 0, (z_far+z_near) / (z_near-z_far), (2.0f*z_far*z_near) / (z_near-z_far),
		0, 0, -1, 0
	);
}

V_DECL mat4 mat4_mul(const mat4 *a, const mat4 *b) {
	mat4 out = {0};
	float *p = out.v;
	for (int i = 0; i < 4; ++i) {
		for (int j = 0; j < 4; ++j, ++p) {
			const float *as = &a->v[j];
			const float *bs = &b->v[4*i];
			*p = as[0]*bs[0] + as[4]*bs[1] + as[8]*bs[2] + as[12]*bs[3];
		}
	}
	return out;
}


V_DECL mat4 mat4_ortho(float left, float right, float bottom, float top, float near_, float far_) {
	float tx = -(right + left)/(right - left);
	float ty = -(top + bottom)/(top - bottom);
	float tz = -(far_ + near_)/(far_ - near_);
	return Mat4(
		2.0f / (right - left), 0, 0, tx,
		0, 2.0f / (top - bottom), 0, ty,
		0, 0, -2.0f / (far_ - near_), tz,
		0, 0, 0, 1
	);
}

V_DECL mat4 mat4_inverse(const mat4 *m) {
	int i;
	mat4 inv;
	const float *v = m->v;
	float *u = inv.v;

	u[0] = v[5] * v[10] * v[15] - v[5] * v[11] * v[14] - v[9] * v[6] * v[15] + v[9] * v[7] * v[14] + v[13] * v[6] * v[11] - v[13] * v[7] * v[10];
	u[1] = -v[1] * v[10] * v[15] + v[1] * v[11] * v[14] + v[9] * v[2] * v[15] - v[9] * v[3] * v[14] - v[13] * v[2] * v[11] + v[13] * v[3] * v[10];
	u[2] = v[1] * v[6] * v[15] - v[1] * v[7] * v[14] - v[5] * v[2] * v[15] + v[5] * v[3] * v[14] + v[13] * v[2] * v[7] - v[13] * v[3] * v[6];
	u[3] = -v[1] * v[6] * v[11] + v[1] * v[7] * v[10] + v[5] * v[2] * v[11] - v[5] * v[3] * v[10] - v[9] * v[2] * v[7] + v[9] * v[3] * v[6];
	u[4] = -v[4] * v[10] * v[15] + v[4] * v[11] * v[14] + v[8] * v[6] * v[15] - v[8] * v[7] * v[14] - v[12] * v[6] * v[11] + v[12] * v[7] * v[10];
	u[5] = v[0] * v[10] * v[15] - v[0] * v[11] * v[14] - v[8] * v[2] * v[15] + v[8] * v[3] * v[14] + v[12] * v[2] * v[11] - v[12] * v[3] * v[10];
	u[6] = -v[0] * v[6] * v[15] + v[0] * v[7] * v[14] + v[4] * v[2] * v[15] - v[4] * v[3] * v[14] - v[12] * v[2] * v[7] + v[12] * v[3] * v[6];
	u[7] = v[0] * v[6] * v[11] - v[0] * v[7] * v[10] - v[4] * v[2] * v[11] + v[4] * v[3] * v[10] + v[8] * v[2] * v[7] - v[8] * v[3] * v[6];
	u[8] = v[4] * v[9] * v[15] - v[4] * v[11] * v[13] - v[8] * v[5] * v[15] + v[8] * v[7] * v[13] + v[12] * v[5] * v[11] - v[12] * v[7] * v[9];
	u[9] = -v[0] * v[9] * v[15] + v[0] * v[11] * v[13] + v[8] * v[1] * v[15] - v[8] * v[3] * v[13] - v[12] * v[1] * v[11] + v[12] * v[3] * v[9];
	u[10] = v[0] * v[5] * v[15] - v[0] * v[7] * v[13] - v[4] * v[1] * v[15] + v[4] * v[3] * v[13] + v[12] * v[1] * v[7] - v[12] * v[3] * v[5];
	u[11] = -v[0] * v[5] * v[11] + v[0] * v[7] * v[9] + v[4] * v[1] * v[11] - v[4] * v[3] * v[9] - v[8] * v[1] * v[7] + v[8] * v[3] * v[5];
	u[12] = -v[4] * v[9] * v[14] + v[4] * v[10] * v[13] + v[8] * v[5] * v[14] - v[8] * v[6] * v[13] - v[12] * v[5] * v[10] + v[12] * v[6] * v[9];
	u[13] = v[0] * v[9] * v[14] - v[0] * v[10] * v[13] - v[8] * v[1] * v[14] + v[8] * v[2] * v[13] + v[12] * v[1] * v[10] - v[12] * v[2] * v[9];
	u[14] = -v[0] * v[5] * v[14] + v[0] * v[6] * v[13] + v[4] * v[1] * v[14] - v[4] * v[2] * v[13] - v[12] * v[1] * v[6] + v[12] * v[2] * v[5];
	u[15] = v[0] * v[5] * v[10] - v[0] * v[6] * v[9] - v[4] * v[1] * v[10] + v[4] * v[2] * v[9] + v[8] * v[1] * v[6] - v[8] * v[2] * v[5];

	float det = v[0] * u[0] + v[1] * u[4] + v[2] * u[8] + v[3] * u[12];

	if (det != 0) {
		det = 1 / det;
		for (i = 0; i < 16; i++)
			u[i] *= det;
	} else {
		for (i = 0; i < 16; i++)
			u[i] = 0;
	}

	return inv;
}



V_DECL vec2i clamp2i(vec2i v, vec2i a, vec2i b) {
	return Vec2i(clampi(v.x, a.x, b.x), clampi(v.y, a.y, b.y));
}
V_DECL vec2i min2i(vec2i a, vec2i b) {
	return Vec2i(mini(a.x, b.x), mini(a.y, b.y));
}
V_DECL vec2i max2i(vec2i a, vec2i b) {
	return Vec2i(maxi(a.x, b.x), maxi(a.y, b.y));
}
V_DECL vec2i add2i(vec2i a, vec2i b) {
	return Vec2i(a.x + b.x, a.y + b.y);
}
V_DECL vec2i addc2i(vec2i a, int x) {
	return Vec2i(a.x + x, a.y + x);
}
V_DECL vec2i sub2i(vec2i a, vec2i b) {
	return Vec2i(a.x - b.x, a.y - b.y);
}
V_DECL vec2i mul2i(vec2i a, vec2i b) {
	return Vec2i(a.x * b.x, a.y * b.y);
}
V_DECL vec2i div2i(vec2i a, vec2i b) {
	return Vec2i(a.x / b.x, a.y / b.y);
}
V_DECL vec2i mod2i(vec2i a, vec2i b) {
	return Vec2i(modi(a.x, b.x), modi(a.y, b.y));
}
V_DECL vec2i scale2i(vec2i a, int x) {
	return Vec2i(a.x * x, a.y * x);
}
V_DECL int dot2i(vec2i a, vec2i b) {
	return a.x * b.x + a.y * b.y;
}
V_DECL vec2i abs2i(vec2i v) {
	return Vec2i(absi(v.x), absi(v.y));
}

V_DECL vec3i clamp3i(vec3i v, vec3i a, vec3i b) {
	return Vec3i(clampi(v.x, a.x, b.x), clampi(v.y, a.y, b.y), clampi(v.z, a.z, b.z));
}
V_DECL vec3i min3i(vec3i a, vec3i b) {
	return Vec3i(mini(a.x, b.x), mini(a.y, b.y), mini(a.z, b.z));
}
V_DECL vec3i max3i(vec3i a, vec3i b) {
	return Vec3i(maxi(a.x, b.x), maxi(a.y, b.y), maxi(a.z, b.z));
}
V_DECL vec3i add3i(vec3i a, vec3i b) {
	return Vec3i(a.x + b.x, a.y + b.y, a.z + b.z);
}
V_DECL vec3i addc3i(vec3i a, int x) {
	return Vec3i(a.x + x, a.y + x, a.z + x);
}
V_DECL vec3i sub3i(vec3i a, vec3i b) {
	return Vec3i(a.x - b.x, a.y - b.y, a.z - b.z);
}
V_DECL vec3i mul3i(vec3i a, vec3i b) {
	return Vec3i(a.x * b.x, a.y * b.y, a.z * b.z);
}
V_DECL vec3i div3i(vec3i a, vec3i b) {
	return Vec3i(a.x / b.x, a.y / b.y, a.z / b.z);
}
V_DECL vec3i mod3i(vec3i a, vec3i b) {
	return Vec3i(modi(a.x, b.x), modi(a.y, b.y), modi(a.z, b.z));
}
V_DECL vec3i scale3i(vec3i a, int x) {
	return Vec3i(a.x * x, a.y * x, a.z * x);
}
V_DECL int dot3i(vec3i a, vec3i b) {
	return a.x * b.x + a.y * b.y + a.z * b.z;
}
V_DECL vec3i abs3i(vec3i v) {
	return Vec3i(absi(v.x), absi(v.y), absi(v.z));
}

V_DECL vec4i clamp4i(vec4i v, vec4i a, vec4i b) {
	return Vec4i(clampi(v.x, a.x, b.x), clampi(v.y, a.y, b.y), clampi(v.z, a.z, b.z), clampi(v.w, a.w, b.w));
}
V_DECL vec4i min4i(vec4i a, vec4i b) {
	return Vec4i(mini(a.x, b.x), mini(a.y, b.y), mini(a.z, b.z), mini(a.w, b.w));
}
V_DECL vec4i max4i(vec4i a, vec4i b) {
	return Vec4i(maxi(a.x, b.x), maxi(a.y, b.y), maxi(a.z, b.z), maxi(a.w, b.w));
}
V_DECL vec4i add4i(vec4i a, vec4i b) {
	return Vec4i(a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w);
}
V_DECL vec4i addc4i(vec4i a, int x) {
	return Vec4i(a.x + x, a.y + x, a.z + x, a.w + x);
}
V_DECL vec4i sub4i(vec4i a, vec4i b) {
	return Vec4i(a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w);
}
V_DECL vec4i mul4i(vec4i a, vec4i b) {
	return Vec4i(a.x * b.x, a.y * b.y, a.z * b.z, a.w * b.w);
}
V_DECL vec4i div4i(vec4i a, vec4i b) {
	return Vec4i(a.x / b.x, a.y / b.y, a.z / b.z, a.w / b.w);
}
V_DECL vec4i mod4i(vec4i a, vec4i b) {
	return Vec4i(modi(a.x, b.x), modi(a.y, b.y), modi(a.z, b.z), modi(a.w, b.w));
}
V_DECL vec4i scale4i(vec4i a, int x) {
	return Vec4i(a.x * x, a.y * x, a.z * x, a.w * x);
}
V_DECL int dot4i(vec4i a, vec4i b) {
	return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
}
V_DECL vec4i abs4i(vec4i v) {
	return Vec4i(absi(v.x), absi(v.y), absi(v.z), absi(v.w));
}

// 2D interpolating across a triangle
V_DECL float barycentric_interpolation_2d(vec2 p, vec2 a, float a_v, vec2 b, float b_v, vec2 c, float c_v) {
	float w_a = ((b.y - c.y) * (p.x - c.x) + (c.x - b.x) * (p.y - c.y))
		/ ((b.y - c.y) * (a.x - c.x) + (c.x - b.x) * (a.y - c.y));
	float w_b = ((c.y - a.y) * (p.x - c.x) + (a.x - c.x) * (p.y - c.y))
		/ ((b.y - c.y) * (a.x - c.x) + (c.x - b.x) * (a.y - c.y));
	float w_c = 1 - w_a - w_b;
	assert(w_a >= -0.001f && w_a <= 1.001f);
	assert(w_b >= -0.001f && w_b <= 1.001f);
	assert(w_c >= -0.001f && w_c <= 1.001f);
	return a_v * w_a + b_v * w_b + c_v * w_c;
}

// collision detection between a 3D ellipse (whatever that's called) and a box
// angle = some rotation about the y axis
// req_dist = required minimum distance from box in x, y, and z directions
// returns true if there is a collision, and sets *pos to the closest point on the box
// to the previous value of *pos
V_DECL bool collision_detect_box(vec3 *pos, vec3 req_dist, vec3 center, vec3 radius, float angle) {
	vec3 p = *pos;
	vec3 min_coord, max_coord;
	bool collision;

	// transform p so it corresponds to a box with an angle of 0.
	p = sub3(p, center);
	{
		mat3 rot = mat3_yaw(-angle);
		p = transform3(&rot, p);
	}

	min_coord = sub3(min3(radius, scale3(radius, -1)), req_dist);
	max_coord = add3(max3(radius, scale3(radius, -1)), req_dist);
	collision = p.x > min_coord.x && p.x < max_coord.x && p.y > min_coord.y && p.y < max_coord.y && p.z > min_coord.z && p.z < max_coord.z;

	if (collision) {
		float closer_x = closerf(p.x, min_coord.x, max_coord.x);
		float closer_y = closerf(p.y, min_coord.y, max_coord.y);
		float closer_z = closerf(p.z, min_coord.z, max_coord.z);
		float xdiff = fabsf(p.x - closer_x);
		float ydiff = fabsf(p.y - closer_y);
		float zdiff = fabsf(p.z - closer_z);
		if (xdiff < ydiff && xdiff < zdiff) {
			p.x = closer_x;
		} else if (ydiff < xdiff && ydiff < zdiff) {
			p.y = closer_y;
		} else {
			p.z = closer_z;
		}
	
		// undo transformation
		{
			mat3 rot = mat3_yaw(angle);
			p = transform3(&rot, p);
		}
		p = add3(p, center);
		*pos = p;
	} // else, we don't need to do anything to *pos
	return collision;
}

V_DECL vec3 color_u8_to_vec3(uint8_t r, uint8_t g, uint8_t b) {
	return scale3(Vec3(r, g, b), 1.0f / 255);
}

V_DECL vec4 color_u8_to_vec4(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
	return scale4(Vec4(r, g, b, a), 1.0f / 255);
}

V_DECL vec3 color_u32_to_vec3(uint32_t u) {
	assert((u & 0xff000000) == 0);
	return color_u8_to_vec3(
		(uint8_t)((u >> 16) & 0xff),
		(uint8_t)((u >> 8) & 0xff),
		(uint8_t)(u & 0xff)
	);
}

V_DECL vec4 color_u32_to_vec4(uint32_t u) {
	return color_u8_to_vec4(
		(uint8_t)(u >> 24),
		(uint8_t)((u >> 16) & 0xff),
		(uint8_t)((u >> 8) & 0xff),
		(uint8_t)(u & 0xff)
	);
}

// --- DYNAMIC ARRAY ---

static inline uint32_t arr_len(const void *arr) {
	return arr ? arr_hdr_((void *)arr)->len : 0;
}

// grow array to fit one more member
static void *arr_grow1_(void *arr, size_t member_size, const char *file, int line) {
	(void)file;
	(void)line;
	if (arr) {
		ArrHeader *hdr = arr_hdr_(arr);
		if_unlikely (hdr->len >= hdr->cap) {
			uint32_t new_capacity = hdr->cap * 2;
			ArrHeader *old_hdr = hdr;
		#if V_DEBUG_MEMORY
			hdr = (ArrHeader *)V_debug_realloc(old_hdr, sizeof(ArrHeader) + new_capacity * member_size, file, line);
		#else
			hdr = (ArrHeader *)realloc(old_hdr, sizeof(ArrHeader) + new_capacity * member_size);
		#endif
			if (hdr) {
				hdr->cap = new_capacity;
			} else {
				free(old_hdr);
				return NULL;
			}
		}
		return hdr->data;
	} else {
		// create a new array
		uint32_t initial_capacity = 2; // allocate enough space for two members
		ArrHeader *ret = (ArrHeader *)calloc(1, sizeof(ArrHeader) + initial_capacity * member_size);
		if (ret) {
			ret->cap = initial_capacity;
			return ret->data;
		} else {
			return NULL;
		}
	}
}

static void *arr_add_ptr_(void **arr, size_t member_size, const char *file, int line) {
	uint8_t *ret;
	*arr = arr_grow1_(*arr, member_size, file, line);
	if (*arr) {
		ret = (uint8_t *)*arr + member_size * (arr_hdr_(*arr)->len++);
		memset(ret, 0, member_size);
	} else {
		ret = NULL;
	}
	return ret;
}

static void arr_reserve_(void **arr, size_t member_size, size_t n) {
	if (n < UINT32_MAX-1) {
		if (!*arr) {
			// create a new array with capacity n+1
			ArrHeader *hdr = calloc(1, sizeof(ArrHeader) + (n+1) * member_size);
			if (hdr) {
				hdr->cap = (uint32_t)n+1;
				*arr = hdr->data;
			}
		} else {
			// increase capacity of array
			ArrHeader *hdr = arr_hdr_(*arr);
			ArrHeader *old_hdr = hdr;
			if (old_hdr->len > n) old_hdr->len = (uint32_t)n;
			hdr = (ArrHeader *)realloc(hdr, sizeof(ArrHeader) + n * member_size);
			if (hdr) {
				hdr->cap = (uint32_t)n;
				memset((char *)hdr->data + member_size * hdr->len, 0, (hdr->cap - hdr->len) * member_size);
			} else {
				// growing failed
				free(old_hdr);
				*arr = NULL;
				return;
			}
			*arr = hdr->data;
		}
	} else {
		// too big; free arr.
		if (*arr) free(arr_hdr_(*arr));
		*arr = NULL;
		return;
	}
}

static void arr_set_len_(void **arr, size_t member_size, size_t n) {
	arr_reserve_(arr, member_size, n);
	if (*arr) {
		ArrHeader *hdr = arr_hdr_(*arr);
		if (n > hdr->len) {
			// zero new elements
			memset((char *)hdr->data + hdr->len, 0, (n - hdr->len) * member_size);
		}
		hdr->len = (uint32_t)n;
	}
}

static void *arr_remove_(void *arr, size_t member_size, size_t index) {
	ArrHeader *hdr = arr_hdr_(arr);
	assert(index < hdr->len);
	memmove((char *)arr + index * member_size, (char *)arr + (index+1) * member_size, (hdr->len - (index+1)) * member_size);
	if (--hdr->len == 0) {
		free(hdr);
		return NULL;
	} else {
		return arr;
	}
}

// --- OS ---

V_DECL bool time_eq(Time a, Time b) {
	return a.tv_sec == b.tv_sec && a.tv_nsec == b.tv_nsec;
}

V_DECL int time_cmp(Time a, Time b) {
	if (a.tv_sec  > b.tv_sec ) return +1;
	if (a.tv_sec  < b.tv_sec ) return -1;
	if (a.tv_nsec > b.tv_nsec) return +1;
	if (a.tv_nsec < b.tv_nsec) return +1;
	return 0;
}

V_DECL Time time_max(Time a, Time b) {
	return time_cmp(a, b) >= 0 ? a : b;
}

V_DECL Time time_min(Time a, Time b) {
	return time_cmp(a, b) <= 0 ? a : b;
}

V_DECL double time_sub(Time a, Time b) {
	return (double)(a.tv_sec - b.tv_sec) + 1e-9 * (double)(a.tv_nsec - b.tv_nsec);
}


V_DECL bool path_is_absolute(const char *path) {
	return path[0] == '/' || path[0] == '\\'
	#if V_WINDOWS
		// "c:\foo" for example
		|| (path[0] >= 'A' && path[0] <= 'Z' && path[1] == ':' && (path[2] == '\\' || path[2] == '/'))
	#endif
		;
}

// if dir is an absolute path, sets full to dir/relpath
V_DECL void path_full(const char *dir, const char *relpath, char *full, size_t full_size) {
	assert(full_size);
	if (path_is_absolute(relpath)) {
		if (relpath[0] == '/' || relpath[0] == '\\') {
			// make sure that on windows, if dir's drive is C: the absolute path of \a is c:\a
			full[0] = '\0';
			strn_cat(full, full_size, dir, strcspn(dir, "/\\"));
			str_cat(full, full_size, relpath);
		} else {
			str_cpy(full, full_size, relpath);
		}
		return;
	}
	str_cpy(full, full_size, dir);

	while (1) {
		size_t component_len = strcspn(relpath, "/\\");
		const char *component_end = relpath + component_len;

		size_t len = strlen(full);
		if (component_len == 1 && relpath[0] == '.') {
			// ., do nothing
		} else if (component_len == 2 && relpath[0] == '.' && relpath[1] == '.') {
			// ..
			char *lastsep = strrchr(full, PATH_SEPARATOR);
			assert(lastsep);
			if (lastsep == full)
				lastsep[1] = '\0';
			else
				lastsep[0] = '\0';
		} else {
			if (full[len - 1] != PATH_SEPARATOR) {
				char pathsep[2] = {PATH_SEPARATOR, 0};
				str_cat(full, full_size, pathsep);
			}
			strn_cat(full, full_size, relpath, component_len);
		}
		if (*component_end == 0)
			break;
		else
			relpath = component_end + 1;
	}
}

V_DECL size_t fs_homedir(char *, size_t);
V_DECL bool fs_getcwd(char *, size_t);

V_DECL bool path_absolute(char const *relpath, char *abspath, size_t abspath_size) {
	char *path = str_dup(relpath); // duplicate relpath in case relpath == abspath
	if (path) {
		bool success = true;

		switch (path[0]) {
		case '~': {
			if (fs_homedir(abspath, abspath_size) < abspath_size) {
				char const *remainder = path + 1;
				if (*remainder == PATH_SEPARATOR) ++remainder;
				path_full(abspath, remainder, abspath, abspath_size);
			} else {
				success = false;
			}
		} break;
		case '/':
		case '\\': {
			char s[2] = {0,0};
			s[0] = path[0];
			path_full(s, path + 1, abspath, abspath_size);
		} break;
		default:
			if (fs_getcwd(abspath, abspath_size) < abspath_size)
				path_full(abspath, path, abspath, abspath_size);
			else
				success = false;
			break;
		}
		free(path);
		return success;
	} else {
		return false;
	}
}

V_DECL int fs_dir_entry_cmp_by_name_qsort(const void *av, const void *bv) {
	const DirEntry *a = av, *b = bv;
	return strcmp(a->name, b->name);
}

V_DECL void fs_free_dir_entries(DirEntry *entries, size_t n_entries) {
	size_t i;
	for (i = 0; i < n_entries; ++i) {
		free(entries[i].name);
		entries[i].name = NULL;
	}
}

V_DECL bool fs_copy_file(const char *in_filename, const char *out_filename) {
	// could be made faster with OS-specific stuff
	char buffer[4096] = {0};
	FILE *in = fopen(in_filename, "rb");
	FILE *out = fopen(out_filename, "wb");
	if (in && out) {
		size_t n;
		while ((n = fread(buffer, 1, sizeof buffer, in)))
			fwrite(buffer, 1, n, out);
	}
	if (in) fclose(in);
	if (out) fclose(out);
	return in && out;
}

#if V_UNIXY
	#ifndef _GNU_SOURCE
	#define _GNU_SOURCE
	#endif
	#include <sys/stat.h>
	#include <sys/types.h>
	#include <sys/random.h>
	#include <sys/file.h>
	#include <dirent.h>
	#include <errno.h>
	#include <unistd.h>

	V_DECL Time time_now(void) {
		Time t = {0};
		clock_gettime(CLOCK_MONOTONIC, &t);
		t.tv_sec += 10000; // this lets you use tv_sec == 0 for "no time available"
		return t;
	}

	V_DECL Time fs_last_modified(const char *filename) {
		struct stat statbuf = {0};
		Time t = {0};
		stat(filename, &statbuf);
	#if __linux__
		t = statbuf.st_mtim;
	#else
		t.tv_sec = statbuf.st_mtime;
	#endif
		return t;
	}

	V_DECL size_t fs_file_size(const char *filename) {
		struct stat statbuf = {0};
		stat(filename, &statbuf);
		return (size_t)statbuf.st_size;
	}

	#define V_ST_MODE_TO_FS_TYPE(mode) (\
		S_ISREG(mode) ? FS_FILE : \
		S_ISDIR(mode) ? FS_DIR  : \
		S_ISLNK(mode) ? FS_LINK : \
		FS_OTHER)

	V_DECL FsType fs_path_type(const char *path) {
		struct stat statbuf = {0};
		if (stat(path, &statbuf) != 0) {
			return FS_NON_EXISTENT;
		}
		return V_ST_MODE_TO_FS_TYPE(statbuf.st_mode);
	}

	// NOTE: files returned in no particular order
	V_DECL size_t fs_list_dir(const char *dir_name, DirEntry *entries, size_t entries_size) {
		DIR *dir = opendir(dir_name);
		if (dir) {
			struct dirent *ent;
			size_t i = 0;
			while ((ent = readdir(dir))) {
				if (entries && i < entries_size) {
					DirEntry entry = {0};
					switch (ent->d_type) {
					case DT_UNKNOWN: {
						struct stat statbuf = {0};
						fstatat(dirfd(dir), ent->d_name, &statbuf, 0);
						entry.type = V_ST_MODE_TO_FS_TYPE(statbuf.st_mode);
					} break;
					case DT_REG: entry.type = FS_FILE;  break;
					case DT_DIR: entry.type = FS_DIR;   break;
					case DT_LNK: entry.type = FS_LINK;  break;
					default:     entry.type = FS_OTHER; break;
					}
					entry.name = str_dup(ent->d_name);
					entries[i] = entry;
				}
				++i;
			}
			closedir(dir);
			return i;
		} else {
			// NOTE: there should never be 0 entries in a directory under normal circumstances,
			// because of . and ..
			return 0;
		}
	}


	V_DECL int fs_mkdir(const char *path) {
		if (mkdir(path, 0755) == 0) {
			// directory created successfully
			return 1;
		} else if (errno == EEXIST && fs_path_type(path) == FS_DIR) {
			return 0;
		} else {
			return -1;
		}
	}

	V_DECL size_t fs_homedir(char *buffer, size_t n) {
		const char *home = getenv("HOME");
		if (home) {
			if (n && buffer)
				str_cpy(buffer, n, home);
			return strlen(home) + 1;
		}
		return 0;
	}

	V_DECL bool fs_getcwd(char *buffer, size_t size) {
		return getcwd(buffer, size) != NULL;
	}
	
	V_DECL bool fs_temp_filename(char *buffer, size_t size) {
		int fd;
		if (size < 12) return false;
		memcpy(buffer, "/tmp/XXXXXX\0", 12);
		fd = mkstemp(buffer);
		if (fd == -1) {
			return false;
		} else {
			close(fd);
			unlink(buffer);
			return true;
		}
	}

	V_DECL FsPermission fs_path_permissions(const char *path) {
		FsPermission perm = 0;
		if (access(path, R_OK) == 0) perm |= FS_PERMISSION_READ;
		if (access(path, W_OK) == 0) perm |= FS_PERMISSION_WRITE;
		return perm;
	}

	V_DECL void get_random(void *out, size_t out_size) {
		size_t bytes_left = out_size;
		ssize_t n;
		while (bytes_left > 0) {
			n = getrandom((char *)out + (out_size - bytes_left), bytes_left, 0);
			if (n == -1) n = 0;
			bytes_left -= (size_t)n;
		}

	}

	V_DECL void sleep_s(double seconds) {
		struct timespec ts = {0}, rem;
		ts.tv_sec = (int64_t)seconds;
		seconds -= (double)(long)seconds;
		ts.tv_nsec = (long)floor(seconds * 1e+9);
		while (nanosleep(&ts, &rem) != 0)
			ts = rem;

	}

	V_DECL void sleep_ms(double milliseconds) {
		sleep_s(milliseconds * 1e-3);
	}
#else
	// -- WINAPI DECLARATIONS --
	// I HATE WINDOWS.H
	typedef uint32_t DWORD, *LPDWORD;
	typedef struct _FILETIME {
		DWORD dwLowDateTime;
		DWORD dwHighDateTime;
	} FILETIME, *PFILETIME, *LPFILETIME;
	typedef const char *LPCSTR;
	typedef char *LPSTR, *PSTR;
	typedef wchar_t *LPWSTR, *PWSTR;
	typedef const wchar_t *LPCWSTR;
	typedef char CHAR;
	typedef long LONG;
	typedef unsigned char *PUCHAR;
	typedef void *HANDLE, *LPVOID, *PVOID, *HLOCAL;
	typedef PVOID BCRYPT_ALG_HANDLE;
	typedef int BOOL;
	typedef int INT;
	typedef DWORD ULONG;
	typedef long HRESULT;
	typedef LONG NTSTATUS;
	typedef struct HWND__ *HWND;
	typedef struct HINSTANCE__ *HINSTANCE;
	typedef struct _WIN32_FIND_DATAA {
		DWORD dwFileAttributes;
		FILETIME ftCreationTime;
		FILETIME ftLastAccessTime;
		FILETIME ftLastWriteTime;
		DWORD nFileSizeHigh;
		DWORD nFileSizeLow;
		DWORD dwReserved0;
		DWORD dwReserved1;
		CHAR cFileName[260];
		CHAR cAlternateFileName[14];
	} WIN32_FIND_DATAA, *PWIN32_FIND_DATAA, *LPWIN32_FIND_DATAA;
	typedef struct _SECURITY_ATTRIBUTES {
		DWORD nLength;
		LPVOID lpSecurityDescriptor;
		BOOL bInheritHandle;
	} SECURITY_ATTRIBUTES, *PSECURITY_ATTRIBUTES, *LPSECURITY_ATTRIBUTES;
    typedef __int64 LONG_PTR, *PLONG_PTR;

	#define V_WINDECL __declspec(dllimport)
	#define V_WINCALL __stdcall

	V_WINDECL void V_WINCALL GetSystemTimeAsFileTime(LPFILETIME);
	V_WINDECL void V_WINCALL Sleep(DWORD);
	V_WINDECL HANDLE V_WINCALL FindFirstFileA(LPCSTR, LPWIN32_FIND_DATAA);
	V_WINDECL BOOL V_WINCALL FindNextFileA(HANDLE, LPWIN32_FIND_DATAA);
	V_WINDECL HANDLE V_WINCALL CreateFileA(LPCSTR, DWORD, DWORD, LPSECURITY_ATTRIBUTES, DWORD, DWORD, HANDLE);
	V_WINDECL HRESULT V_WINCALL SHGetFolderPathA(HWND, int, HANDLE, DWORD, LPSTR);
	V_WINDECL BOOL V_WINCALL CloseHandle(HANDLE);
	V_WINDECL BOOL V_WINCALL FindClose(HANDLE);
	V_WINDECL DWORD V_WINCALL GetFileSize(HANDLE, LPDWORD);
	V_WINDECL BOOL V_WINCALL GetFileTime(HANDLE, LPFILETIME, LPFILETIME, LPFILETIME);
	V_WINDECL DWORD V_WINCALL GetFileAttributesA(LPCSTR);
	V_WINDECL DWORD V_WINCALL GetLastError(void);
	V_WINDECL BOOL V_WINCALL CreateDirectoryA(LPCSTR, LPSECURITY_ATTRIBUTES);
	V_WINDECL BOOL V_WINCALL GetFileTime(HANDLE, LPFILETIME, LPFILETIME, LPFILETIME);
	V_WINDECL DWORD V_WINCALL GetCurrentDirectoryA(DWORD, LPSTR);
	V_WINDECL LPWSTR *V_WINCALL CommandLineToArgvW(LPCWSTR, int *);
	V_WINDECL LPWSTR V_WINCALL GetCommandLineW(void);
	V_WINDECL HLOCAL V_WINCALL LocalFree(HLOCAL);
	NTSTATUS V_WINCALL BCryptGenRandom(BCRYPT_ALG_HANDLE, PUCHAR, ULONG, ULONG);


	#define INVALID_HANDLE_VALUE ((HANDLE)(LONG_PTR)-1)
	#define GENERIC_READ (0x80000000L)
	#define OPEN_EXISTING 3
	#define INVALID_HANDLE_VALUE ((HANDLE)(LONG_PTR)-1)
	#define FILE_ATTRIBUTE_DIRECTORY 0x00000010
	#define INVALID_HANDLE_VALUE ((HANDLE)(LONG_PTR)-1)
	#define FILE_ATTRIBUTE_NORMAL 0x00000080
	#define ERROR_ALREADY_EXISTS 183L
	#define MAX_PATH 260
	#define CSIDL_PROFILE 0x0028
	#define BCRYPT_USE_SYSTEM_PREFERRED_RNG 0x00000002
	#define STATUS_SUCCESS 0
	
	int tmpnam_s(char *, size_t);

	// -- FUNCTION IMPLEMENTATIONS --
	static Time V_filetime_to_time(FILETIME ft) {
		Time t;
		uint64_t time_ns100; // time in multiples of 100 nanoseconds
		time_ns100 = (uint64_t)ft.dwHighDateTime << 32 | (uint64_t)ft.dwLowDateTime;
		t.tv_sec = (time_t)(time_ns100 / 10000000);
		t.tv_nsec = (long)(time_ns100 % 10000000) * 100;
		return t;
	}

	V_DECL Time time_now(void) {
		FILETIME ft = {0};
		GetSystemTimeAsFileTime(&ft);
		return V_filetime_to_time(ft);
	}

	V_DECL Time fs_last_modified(const char *filename) {
		Time t = {0};
		HANDLE h = CreateFileA(filename, GENERIC_READ, 7, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if (h != INVALID_HANDLE_VALUE) {
			FILETIME wtime = {0};
			GetFileTime(h, NULL, NULL, &wtime);
			t = V_filetime_to_time(wtime);
			CloseHandle(h);
		}
		return t;
	}

	V_DECL size_t fs_file_size(const char *filename) {
		uint64_t size = 0;
		HANDLE h = CreateFileA(filename, GENERIC_READ, 7, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if (h != INVALID_HANDLE_VALUE) {
			DWORD high = 0;
			size = GetFileSize(h, &high);
			size |= (uint64_t)high << 32;
			CloseHandle(h);
		}
		return (size_t)size;
	}

	V_DECL FsType fs_path_type(const char *path) {
		DWORD attrs = GetFileAttributesA(path);
		if (attrs & FILE_ATTRIBUTE_DIRECTORY)
			return FS_DIR;
		return FS_FILE;
	}

	V_DECL size_t fs_list_dir(const char *dir_name, DirEntry *entries, size_t entries_size) {
		char find_name[1024];
		WIN32_FIND_DATAA find_data = {0};
		strbuf_print(find_name, "%s\\*", dir_name);
		HANDLE handle = FindFirstFileA(find_name, &find_data);
		if (handle == INVALID_HANDLE_VALUE)
			return 0;

		size_t i = 0;
		do {
			if (i < entries_size) {
				DirEntry *entry = &entries[i];
				FsType type = FS_FILE;
				DWORD attrs = find_data.dwFileAttributes;
				char *name = str_dup(find_data.cFileName);
				if (attrs & FILE_ATTRIBUTE_DIRECTORY)
					type = FS_DIR;
				entry->name = name;
				entry->type = type;
			}
			++i;
		} while (FindNextFileA(handle, &find_data));
		FindClose(handle);
		return i;
	}

	V_DECL int fs_mkdir(const char *path) {
		if (CreateDirectoryA(path, NULL)) {
			return 1;
		} else if (GetLastError() == ERROR_ALREADY_EXISTS) {
			return 0;
		} else {
			return -1;
		}
	}

	V_DECL size_t fs_homedir(char *buffer, size_t n) {
		char path[MAX_PATH+1] = {0};
		SHGetFolderPathA(NULL, CSIDL_PROFILE, NULL, 0, path);
		str_cpy(buffer, n, path);
		return strlen(path) + 1;
	}

	V_DECL bool fs_getcwd(char *buffer, size_t size) {
		return GetCurrentDirectoryA((ULONG)size, buffer) != 0;
	}

	V_DECL bool fs_temp_filename(char *buffer, size_t size) {
		if (!buffer || size <= 1) return false;
		memset(buffer, 0, size);
		return tmpnam_s(buffer, size - 1) == 0;
	}


	V_DECL FsPermission fs_path_permissions(const char *path) {
		// meh
		(void)path;
		return FS_PERMISSION_READ | FS_PERMISSION_WRITE;
	}

	V_DECL void get_random(void *out, size_t out_size) {
		if (out_size > 0xffffffff ||
			BCryptGenRandom(NULL, out, (ULONG)out_size, BCRYPT_USE_SYSTEM_PREFERRED_RNG)
				!= STATUS_SUCCESS) {
			exit(-2);
		}
	}

	V_DECL void sleep_ms(double milliseconds) {
		DWORD ms = milliseconds > 0xffffffff ? 0xffffffff
			: milliseconds < 0 ? 0
			: (DWORD)milliseconds;
		Sleep(ms);
	}

	V_DECL void sleep_s(double seconds) {
		sleep_ms(seconds * 1000);
	}

#endif

// --- I/O ---
static char *file_read_string(const char *filename, bool binary) {
	FILE *fp = fopen(filename, binary ? "rb" : "r");
	if (fp) {
		size_t size = fs_file_size(filename);
		char *contents = calloc(1, 2 * size + 1);
		if (contents)
			fread(contents, 1, size, fp);
		fclose(fp);
		return contents;
	} else {
		return NULL;
	}
}

static bool file_write_string(const char *filename, const char *string, bool binary) {
	FILE *fp = fopen(filename, binary ? "wb" : "w");
	if (fp) {
		fputs(string, fp);
		fclose(fp);
		return true;
	} else {
		return false;
	}
}


// --- FILE TYPES ---

// stb_image.h, borrowed from https://github.com/nothings/stb (public domain)
NO_WARN_START
#if V_PNG
#define STBI_VERSION 1
enum
{
	STBI_default = 0,
	STBI_grey = 1,
	STBI_grey_alpha = 2,
	STBI_rgb = 3,
	STBI_rgb_alpha = 4
};
typedef unsigned char stbi_uc;
typedef unsigned short stbi_us;
#define STBIDEF static
typedef struct
{
	int (*read) (void *user,char *data,int size);
	void (*skip) (void *user,int n);
	int (*eof) (void *user);
} stbi_io_callbacks;
STBIDEF stbi_uc *stbi_load_from_memory (stbi_uc const *buffer, int len , int *x, int *y, int *channels_in_file, int desired_channels);
STBIDEF stbi_uc *stbi_load_from_callbacks(stbi_io_callbacks const *clbk , void *user, int *x, int *y, int *channels_in_file, int desired_channels);
STBIDEF stbi_uc *stbi_load (char const *filename, int *x, int *y, int *channels_in_file, int desired_channels);
STBIDEF stbi_uc *stbi_load_from_file (FILE *f, int *x, int *y, int *channels_in_file, int desired_channels);
STBIDEF stbi_us *stbi_load_16_from_memory (stbi_uc const *buffer, int len, int *x, int *y, int *channels_in_file, int desired_channels);
STBIDEF stbi_us *stbi_load_16_from_callbacks(stbi_io_callbacks const *clbk, void *user, int *x, int *y, int *channels_in_file, int desired_channels);
STBIDEF stbi_us *stbi_load_16 (char const *filename, int *x, int *y, int *channels_in_file, int desired_channels);
STBIDEF stbi_us *stbi_load_from_file_16(FILE *f, int *x, int *y, int *channels_in_file, int desired_channels);
STBIDEF int stbi_is_hdr_from_callbacks(stbi_io_callbacks const *clbk, void *user);
STBIDEF int stbi_is_hdr_from_memory(stbi_uc const *buffer, int len);
STBIDEF int stbi_is_hdr (char const *filename);
STBIDEF int stbi_is_hdr_from_file(FILE *f);
STBIDEF const char *stbi_failure_reason (void);
STBIDEF void stbi_image_free (void *retval_from_stbi_load);
STBIDEF int stbi_info_from_memory(stbi_uc const *buffer, int len, int *x, int *y, int *comp);
STBIDEF int stbi_info_from_callbacks(stbi_io_callbacks const *clbk, void *user, int *x, int *y, int *comp);
STBIDEF int stbi_is_16_bit_from_memory(stbi_uc const *buffer, int len);
STBIDEF int stbi_is_16_bit_from_callbacks(stbi_io_callbacks const *clbk, void *user);
STBIDEF int stbi_info (char const *filename, int *x, int *y, int *comp);
STBIDEF int stbi_info_from_file (FILE *f, int *x, int *y, int *comp);
STBIDEF int stbi_is_16_bit (char const *filename);
STBIDEF int stbi_is_16_bit_from_file(FILE *f);
STBIDEF void stbi_set_unpremultiply_on_load(int flag_true_if_should_unpremultiply);
STBIDEF void stbi_convert_iphone_png_to_rgb(int flag_true_if_should_convert);
STBIDEF void stbi_set_flip_vertically_on_load(int flag_true_if_should_flip);
STBIDEF void stbi_set_flip_vertically_on_load_thread(int flag_true_if_should_flip);
STBIDEF char *stbi_zlib_decode_malloc_guesssize(const char *buffer, int len, int initial_size, int *outlen);
STBIDEF char *stbi_zlib_decode_malloc_guesssize_headerflag(const char *buffer, int len, int initial_size, int *outlen, int parse_header);
STBIDEF char *stbi_zlib_decode_malloc(const char *buffer, int len, int *outlen);
STBIDEF int stbi_zlib_decode_buffer(char *obuffer, int olen, const char *ibuffer, int ilen);
STBIDEF char *stbi_zlib_decode_noheader_malloc(const char *buffer, int len, int *outlen);
STBIDEF int stbi_zlib_decode_noheader_buffer(char *obuffer, int olen, const char *ibuffer, int ilen);
#define STBI_ASSERT(x) assert(x)
#ifndef _MSC_VER
   #ifdef __cplusplus
   #define stbi_inline inline
   #else
   #define stbi_inline
   #endif
#else
   #define stbi_inline __forceinline
#endif
#ifndef STBI_NO_THREAD_LOCALS
   #if defined(__cplusplus) &&  __cplusplus >= 201103L
      #define STBI_THREAD_LOCAL       thread_local
   #elif defined(__GNUC__) && __GNUC__ < 5
      #define STBI_THREAD_LOCAL       __thread
   #elif defined(_MSC_VER)
      #define STBI_THREAD_LOCAL       __declspec(thread)
   #elif defined (__STDC_VERSION__) && __STDC_VERSION__ >= 201112L && !defined(__STDC_NO_THREADS__)
      #define STBI_THREAD_LOCAL       _Thread_local
   #endif
   #ifndef STBI_THREAD_LOCAL
      #if defined(__GNUC__)
        #define STBI_THREAD_LOCAL       __thread
      #endif
   #endif
#endif
typedef uint16_t stbi__uint16;
typedef int16_t  stbi__int16;
typedef uint32_t stbi__uint32;
typedef int32_t  stbi__int32;
typedef unsigned char validate_uint32[sizeof(stbi__uint32)==4 ? 1 : -1];
#ifdef _MSC_VER
#define STBI_NOTUSED(v)  (void)(v)
#else
#define STBI_NOTUSED(v)  (void)sizeof(v)
#endif
#ifdef _MSC_VER
#define STBI_HAS_LROTL
#endif
#ifdef STBI_HAS_LROTL
   #define stbi_lrot(x,y)  _lrotl(x,y)
#else
   #define stbi_lrot(x,y)  (((x) << (y)) | ((x) >> (32 - (y))))
#endif
#if defined(STBI_MALLOC) && defined(STBI_FREE) && (defined(STBI_REALLOC) || defined(STBI_REALLOC_SIZED))
#elif !defined(STBI_MALLOC) && !defined(STBI_FREE) && !defined(STBI_REALLOC) && !defined(STBI_REALLOC_SIZED)
#else
#error "Must define all or none of STBI_MALLOC, STBI_FREE, and STBI_REALLOC (or STBI_REALLOC_SIZED)."
#endif
#ifndef STBI_MALLOC
#define STBI_MALLOC(sz)           malloc(sz)
#define STBI_REALLOC(p,newsz)     realloc(p,newsz)
#define STBI_FREE(p)              free(p)
#endif
#ifndef STBI_REALLOC_SIZED
#define STBI_REALLOC_SIZED(p,oldsz,newsz) STBI_REALLOC(p,newsz)
#endif
#if defined(__x86_64__) || defined(_M_X64)
#define STBI__X64_TARGET
#elif defined(__i386) || defined(_M_IX86)
#define STBI__X86_TARGET
#endif
#if defined(STBI_NO_SIMD) && defined(STBI_NEON)
#undef STBI_NEON
#endif
#ifdef STBI_NEON
#include <arm_neon.h>
#define STBI_SIMD_ALIGN(type, name) type name __attribute__((aligned(16)))
#endif
#ifndef STBI_SIMD_ALIGN
#define STBI_SIMD_ALIGN(type, name) type name
#endif
#ifndef STBI_MAX_DIMENSIONS
#define STBI_MAX_DIMENSIONS (1 << 24)
#endif
typedef struct
{
	stbi__uint32 img_x, img_y;
	int img_n, img_out_n;
	stbi_io_callbacks io;
	void *io_user_data;
	int read_from_callbacks;
	int buflen;
	stbi_uc buffer_start[128];
	int callback_already_read;
	stbi_uc *img_buffer, *img_buffer_end;
	stbi_uc *img_buffer_original, *img_buffer_original_end;
} stbi__context;
static void stbi__refill_buffer(stbi__context *s);
static void stbi__start_mem(stbi__context *s, stbi_uc const *buffer, int len)
{
	s->io.read = NULL;
	s->read_from_callbacks = 0;
	s->callback_already_read = 0;
	s->img_buffer = s->img_buffer_original = (stbi_uc *) buffer;
	s->img_buffer_end = s->img_buffer_original_end = (stbi_uc *) buffer+len;
}
static void stbi__start_callbacks(stbi__context *s, stbi_io_callbacks *c, void *user)
{
	s->io = *c;
	s->io_user_data = user;
	s->buflen = sizeof(s->buffer_start);
	s->read_from_callbacks = 1;
	s->callback_already_read = 0;
	s->img_buffer = s->img_buffer_original = s->buffer_start;
	stbi__refill_buffer(s);
	s->img_buffer_original_end = s->img_buffer_end;
}
#ifndef STBI_NO_STDIO
static int stbi__stdio_read(void *user, char *data, int size)
{
	return (int) fread(data,1,size,(FILE*) user);
}
static void stbi__stdio_skip(void *user, int n)
{
	int ch;
	fseek((FILE*) user, n, SEEK_CUR);
	ch = fgetc((FILE*) user);  /* have to read a byte to reset feof()'s flag */
	if (!feof((FILE *)user)) {
		ungetc(ch, (FILE *) user);  /* push byte back onto stream if valid. */
	}
}
static int stbi__stdio_eof(void *user)
{
	return feof((FILE*) user) || ferror((FILE *) user);
}
static stbi_io_callbacks stbi__stdio_callbacks =
{
	stbi__stdio_read,
	stbi__stdio_skip,
	stbi__stdio_eof,
};
static void stbi__start_file(stbi__context *s, FILE *f)
{
	stbi__start_callbacks(s, &stbi__stdio_callbacks, (void *) f);
}
#endif
static void stbi__rewind(stbi__context *s)
{
	s->img_buffer = s->img_buffer_original;
	s->img_buffer_end = s->img_buffer_original_end;
}
enum
{
	STBI_ORDER_RGB,
	STBI_ORDER_BGR
};
typedef struct
{
	int bits_per_channel;
	int num_channels;
	int channel_order;
} stbi__result_info;
#ifndef STBI_NO_PNG
static int		stbi__png_test(stbi__context *s);
static void	 *stbi__png_load(stbi__context *s, int *x, int *y, int *comp, int req_comp, stbi__result_info *ri);
static int		stbi__png_info(stbi__context *s, int *x, int *y, int *comp);
static int		stbi__png_is16(stbi__context *s);
#endif
static
#ifdef STBI_THREAD_LOCAL
STBI_THREAD_LOCAL
#endif
const char *stbi__g_failure_reason;
STBIDEF const char *stbi_failure_reason(void)
{
	return stbi__g_failure_reason;
}
#ifndef STBI_NO_FAILURE_STRINGS
static int stbi__err(const char *str)
{
	stbi__g_failure_reason = str;
	return 0;
}
#endif
static void *stbi__malloc(size_t size)
{
	 return STBI_MALLOC(size);
}
static int stbi__addsizes_valid(int a, int b)
{
	if (b < 0) return 0;
	return a <= INT_MAX - b;
}
static int stbi__mul2sizes_valid(int a, int b)
{
	if (a < 0 || b < 0) return 0;
	if (b == 0) return 1;
	return a <= INT_MAX/b;
}
#if !defined(STBI_NO_JPEG) || !defined(STBI_NO_PNG) || !defined(STBI_NO_TGA) || !defined(STBI_NO_HDR)
static int stbi__mad2sizes_valid(int a, int b, int add)
{
	return stbi__mul2sizes_valid(a, b) && stbi__addsizes_valid(a*b, add);
}
#endif
static int stbi__mad3sizes_valid(int a, int b, int c, int add)
{
	return stbi__mul2sizes_valid(a, b) && stbi__mul2sizes_valid(a*b, c) &&
		stbi__addsizes_valid(a*b*c, add);
}
#if !defined(STBI_NO_JPEG) || !defined(STBI_NO_PNG) || !defined(STBI_NO_TGA) || !defined(STBI_NO_HDR)
static void *stbi__malloc_mad2(int a, int b, int add)
{
	if (!stbi__mad2sizes_valid(a, b, add)) return NULL;
	return stbi__malloc(a*b + add);
}
#endif
static void *stbi__malloc_mad3(int a, int b, int c, int add)
{
	if (!stbi__mad3sizes_valid(a, b, c, add)) return NULL;
	return stbi__malloc(a*b*c + add);
}
#ifdef STBI_NO_FAILURE_STRINGS
	#define stbi__err(x,y)  0
#elif defined(STBI_FAILURE_USERMSG)
	#define stbi__err(x,y)  stbi__err(y)
#else
	#define stbi__err(x,y)  stbi__err(x)
#endif
#define stbi__errpf(x,y)	((float *)(size_t) (stbi__err(x,y)?NULL:NULL))
#define stbi__errpuc(x,y)  ((unsigned char *)(size_t) (stbi__err(x,y)?NULL:NULL))
STBIDEF void stbi_image_free(void *retval_from_stbi_load)
{
	STBI_FREE(retval_from_stbi_load);
}
static int stbi__vertically_flip_on_load_global = 0;
STBIDEF void stbi_set_flip_vertically_on_load(int flag_true_if_should_flip)
{
	stbi__vertically_flip_on_load_global = flag_true_if_should_flip;
}
#ifndef STBI_THREAD_LOCAL
#define stbi__vertically_flip_on_load  stbi__vertically_flip_on_load_global
#else
static STBI_THREAD_LOCAL int stbi__vertically_flip_on_load_local, stbi__vertically_flip_on_load_set;
STBIDEF void stbi_set_flip_vertically_on_load_thread(int flag_true_if_should_flip)
{
	stbi__vertically_flip_on_load_local = flag_true_if_should_flip;
	stbi__vertically_flip_on_load_set = 1;
}
#define stbi__vertically_flip_on_load  (stbi__vertically_flip_on_load_set		 \
													  ? stbi__vertically_flip_on_load_local  \
													  : stbi__vertically_flip_on_load_global)
#endif
static void *stbi__load_main(stbi__context *s, int *x, int *y, int *comp, int req_comp, stbi__result_info *ri, int bpc)
{
	memset(ri, 0, sizeof(*ri));
	ri->bits_per_channel = 8;
	ri->channel_order = STBI_ORDER_RGB;
	ri->num_channels = 0;
	if (stbi__png_test(s))  return stbi__png_load(s,x,y,comp,req_comp, ri);
	return stbi__errpuc("unknown image type", "Image not of any known type, or corrupt");
}
static stbi_uc *stbi__convert_16_to_8(stbi__uint16 *orig, int w, int h, int channels)
{
	int i;
	int img_len = w * h * channels;
	stbi_uc *reduced;
	reduced = (stbi_uc *) stbi__malloc(img_len);
	if (reduced == NULL) return stbi__errpuc("outofmem", "Out of memory");
	for (i = 0; i < img_len; ++i)
		reduced[i] = (stbi_uc)((orig[i] >> 8) & 0xFF);
	STBI_FREE(orig);
	return reduced;
}
static stbi__uint16 *stbi__convert_8_to_16(stbi_uc *orig, int w, int h, int channels)
{
	int i;
	int img_len = w * h * channels;
	stbi__uint16 *enlarged;
	enlarged = (stbi__uint16 *) stbi__malloc(img_len*2);
	if (enlarged == NULL) return (stbi__uint16 *) stbi__errpuc("outofmem", "Out of memory");
	for (i = 0; i < img_len; ++i)
		enlarged[i] = (stbi__uint16)((orig[i] << 8) + orig[i]);
	STBI_FREE(orig);
	return enlarged;
}
static void stbi__vertical_flip(void *image, int w, int h, int bytes_per_pixel)
{
	int row;
	size_t bytes_per_row = (size_t)w * bytes_per_pixel;
	stbi_uc temp[2048];
	stbi_uc *bytes = (stbi_uc *)image;
	for (row = 0; row < (h>>1); row++) {
		stbi_uc *row0 = bytes + row*bytes_per_row;
		stbi_uc *row1 = bytes + (h - row - 1)*bytes_per_row;
		size_t bytes_left = bytes_per_row;
		while (bytes_left) {
			size_t bytes_copy = (bytes_left < sizeof(temp)) ? bytes_left : sizeof(temp);
			memcpy(temp, row0, bytes_copy);
			memcpy(row0, row1, bytes_copy);
			memcpy(row1, temp, bytes_copy);
			row0 += bytes_copy;
			row1 += bytes_copy;
			bytes_left -= bytes_copy;
		}
	}
}
static unsigned char *stbi__load_and_postprocess_8bit(stbi__context *s, int *x, int *y, int *comp, int req_comp)
{
	stbi__result_info ri;
	void *result = stbi__load_main(s, x, y, comp, req_comp, &ri, 8);
	if (result == NULL)
		return NULL;
	STBI_ASSERT(ri.bits_per_channel == 8 || ri.bits_per_channel == 16);
	if (ri.bits_per_channel != 8) {
		result = stbi__convert_16_to_8((stbi__uint16 *) result, *x, *y, req_comp == 0 ? *comp : req_comp);
		ri.bits_per_channel = 8;
	}
	if (stbi__vertically_flip_on_load) {
		int channels = req_comp ? req_comp : *comp;
		stbi__vertical_flip(result, *x, *y, channels * sizeof(stbi_uc));
	}
	return (unsigned char *) result;
}
static stbi__uint16 *stbi__load_and_postprocess_16bit(stbi__context *s, int *x, int *y, int *comp, int req_comp)
{
	stbi__result_info ri;
	void *result = stbi__load_main(s, x, y, comp, req_comp, &ri, 16);
	if (result == NULL)
		return NULL;
	STBI_ASSERT(ri.bits_per_channel == 8 || ri.bits_per_channel == 16);
	if (ri.bits_per_channel != 16) {
		result = stbi__convert_8_to_16((stbi_uc *) result, *x, *y, req_comp == 0 ? *comp : req_comp);
		ri.bits_per_channel = 16;
	}
	if (stbi__vertically_flip_on_load) {
		int channels = req_comp ? req_comp : *comp;
		stbi__vertical_flip(result, *x, *y, channels * sizeof(stbi__uint16));
	}
	return (stbi__uint16 *) result;
}
#ifndef STBI_NO_STDIO
static FILE *stbi__fopen(char const *filename, char const *mode)
{
	return fopen(filename, mode);
}
STBIDEF stbi_uc *stbi_load(char const *filename, int *x, int *y, int *comp, int req_comp)
{
	FILE *f = stbi__fopen(filename, "rb");
	unsigned char *result;
	if (!f) return stbi__errpuc("can't fopen", "Unable to open file");
	result = stbi_load_from_file(f,x,y,comp,req_comp);
	fclose(f);
	return result;
}
STBIDEF stbi_uc *stbi_load_from_file(FILE *f, int *x, int *y, int *comp, int req_comp)
{
	unsigned char *result;
	stbi__context s;
	stbi__start_file(&s,f);
	result = stbi__load_and_postprocess_8bit(&s,x,y,comp,req_comp);
	if (result) {
		fseek(f, - (int) (s.img_buffer_end - s.img_buffer), SEEK_CUR);
	}
	return result;
}
STBIDEF stbi__uint16 *stbi_load_from_file_16(FILE *f, int *x, int *y, int *comp, int req_comp)
{
	stbi__uint16 *result;
	stbi__context s;
	stbi__start_file(&s,f);
	result = stbi__load_and_postprocess_16bit(&s,x,y,comp,req_comp);
	if (result) {
		fseek(f, - (int) (s.img_buffer_end - s.img_buffer), SEEK_CUR);
	}
	return result;
}
STBIDEF stbi_us *stbi_load_16(char const *filename, int *x, int *y, int *comp, int req_comp)
{
	FILE *f = stbi__fopen(filename, "rb");
	stbi__uint16 *result;
	if (!f) return (stbi_us *) stbi__errpuc("can't fopen", "Unable to open file");
	result = stbi_load_from_file_16(f,x,y,comp,req_comp);
	fclose(f);
	return result;
}
#endif
STBIDEF stbi_us *stbi_load_16_from_memory(stbi_uc const *buffer, int len, int *x, int *y, int *channels_in_file, int desired_channels)
{
	stbi__context s;
	stbi__start_mem(&s,buffer,len);
	return stbi__load_and_postprocess_16bit(&s,x,y,channels_in_file,desired_channels);
}
STBIDEF stbi_us *stbi_load_16_from_callbacks(stbi_io_callbacks const *clbk, void *user, int *x, int *y, int *channels_in_file, int desired_channels)
{
	stbi__context s;
	stbi__start_callbacks(&s, (stbi_io_callbacks *)clbk, user);
	return stbi__load_and_postprocess_16bit(&s,x,y,channels_in_file,desired_channels);
}
STBIDEF stbi_uc *stbi_load_from_memory(stbi_uc const *buffer, int len, int *x, int *y, int *comp, int req_comp)
{
	stbi__context s;
	stbi__start_mem(&s,buffer,len);
	return stbi__load_and_postprocess_8bit(&s,x,y,comp,req_comp);
}
STBIDEF stbi_uc *stbi_load_from_callbacks(stbi_io_callbacks const *clbk, void *user, int *x, int *y, int *comp, int req_comp)
{
	stbi__context s;
	stbi__start_callbacks(&s, (stbi_io_callbacks *) clbk, user);
	return stbi__load_and_postprocess_8bit(&s,x,y,comp,req_comp);
}
enum
{
	STBI__SCAN_load=0,
	STBI__SCAN_type,
	STBI__SCAN_header
};
static void stbi__refill_buffer(stbi__context *s)
{
	int n = (s->io.read)(s->io_user_data,(char*)s->buffer_start,s->buflen);
	s->callback_already_read += (int) (s->img_buffer - s->img_buffer_original);
	if (n == 0) {
		s->read_from_callbacks = 0;
		s->img_buffer = s->buffer_start;
		s->img_buffer_end = s->buffer_start+1;
		*s->img_buffer = 0;
	} else {
		s->img_buffer = s->buffer_start;
		s->img_buffer_end = s->buffer_start + n;
	}
}
stbi_inline static stbi_uc stbi__get8(stbi__context *s)
{
	if (s->img_buffer < s->img_buffer_end)
		return *s->img_buffer++;
	if (s->read_from_callbacks) {
		stbi__refill_buffer(s);
		return *s->img_buffer++;
	}
	return 0;
}
#if defined(STBI_NO_JPEG) && defined(STBI_NO_PNG) && defined(STBI_NO_BMP) && defined(STBI_NO_PSD) && defined(STBI_NO_TGA) && defined(STBI_NO_GIF) && defined(STBI_NO_PIC)
#else
static void stbi__skip(stbi__context *s, int n)
{
	if (n == 0) return;
	if (n < 0) {
		s->img_buffer = s->img_buffer_end;
		return;
	}
	if (s->io.read) {
		int blen = (int) (s->img_buffer_end - s->img_buffer);
		if (blen < n) {
			s->img_buffer = s->img_buffer_end;
			(s->io.skip)(s->io_user_data, n - blen);
			return;
		}
	}
	s->img_buffer += n;
}
#endif
#if defined(STBI_NO_PNG) && defined(STBI_NO_TGA) && defined(STBI_NO_HDR) && defined(STBI_NO_PNM)
#else
static int stbi__getn(stbi__context *s, stbi_uc *buffer, int n)
{
	if (s->io.read) {
		int blen = (int) (s->img_buffer_end - s->img_buffer);
		if (blen < n) {
			int res, count;
			memcpy(buffer, s->img_buffer, blen);
			count = (s->io.read)(s->io_user_data, (char*) buffer + blen, n - blen);
			res = (count == (n-blen));
			s->img_buffer = s->img_buffer_end;
			return res;
		}
	}
	if (s->img_buffer+n <= s->img_buffer_end) {
		memcpy(buffer, s->img_buffer, n);
		s->img_buffer += n;
		return 1;
	} else
		return 0;
}
#endif
#if defined(STBI_NO_JPEG) && defined(STBI_NO_PNG) && defined(STBI_NO_PSD) && defined(STBI_NO_PIC)
#else
static int stbi__get16be(stbi__context *s)
{
	int z = stbi__get8(s);
	return (z << 8) + stbi__get8(s);
}
#endif
#if defined(STBI_NO_PNG) && defined(STBI_NO_PSD) && defined(STBI_NO_PIC)
#else
static stbi__uint32 stbi__get32be(stbi__context *s)
{
	stbi__uint32 z = stbi__get16be(s);
	return (z << 16) + stbi__get16be(s);
}
#endif
#define STBI__BYTECAST(x)  ((stbi_uc) ((x) & 255))
#if defined(STBI_NO_JPEG) && defined(STBI_NO_PNG) && defined(STBI_NO_BMP) && defined(STBI_NO_PSD) && defined(STBI_NO_TGA) && defined(STBI_NO_GIF) && defined(STBI_NO_PIC) && defined(STBI_NO_PNM)
#else
static stbi_uc stbi__compute_y(int r, int g, int b)
{
	return (stbi_uc) (((r*77) + (g*150) +  (29*b)) >> 8);
}
#endif
#if defined(STBI_NO_PNG) && defined(STBI_NO_BMP) && defined(STBI_NO_PSD) && defined(STBI_NO_TGA) && defined(STBI_NO_GIF) && defined(STBI_NO_PIC) && defined(STBI_NO_PNM)
#else
static unsigned char *stbi__convert_format(unsigned char *data, int img_n, int req_comp, unsigned int x, unsigned int y)
{
	int i,j;
	unsigned char *good;
	if (req_comp == img_n) return data;
	STBI_ASSERT(req_comp >= 1 && req_comp <= 4);
	good = (unsigned char *) stbi__malloc_mad3(req_comp, x, y, 0);
	if (good == NULL) {
		STBI_FREE(data);
		return stbi__errpuc("outofmem", "Out of memory");
	}
	for (j=0; j < (int) y; ++j) {
		unsigned char *src  = data + j * x * img_n	;
		unsigned char *dest = good + j * x * req_comp;
		#define STBI__COMBO(a,b)  ((a)*8+(b))
		#define STBI__CASE(a,b)	case STBI__COMBO(a,b): for(i=x-1; i >= 0; --i, src += a, dest += b)
		switch (STBI__COMBO(img_n, req_comp)) {
			STBI__CASE(1,2) { dest[0]=src[0]; dest[1]=255;												 } break;
			STBI__CASE(1,3) { dest[0]=dest[1]=dest[2]=src[0];											 } break;
			STBI__CASE(1,4) { dest[0]=dest[1]=dest[2]=src[0]; dest[3]=255;							} break;
			STBI__CASE(2,1) { dest[0]=src[0];																  } break;
			STBI__CASE(2,3) { dest[0]=dest[1]=dest[2]=src[0];											 } break;
			STBI__CASE(2,4) { dest[0]=dest[1]=dest[2]=src[0]; dest[3]=src[1];						} break;
			STBI__CASE(3,4) { dest[0]=src[0];dest[1]=src[1];dest[2]=src[2];dest[3]=255;		  } break;
			STBI__CASE(3,1) { dest[0]=stbi__compute_y(src[0],src[1],src[2]);						 } break;
			STBI__CASE(3,2) { dest[0]=stbi__compute_y(src[0],src[1],src[2]); dest[1] = 255;	 } break;
			STBI__CASE(4,1) { dest[0]=stbi__compute_y(src[0],src[1],src[2]);						 } break;
			STBI__CASE(4,2) { dest[0]=stbi__compute_y(src[0],src[1],src[2]); dest[1] = src[3]; } break;
			STBI__CASE(4,3) { dest[0]=src[0];dest[1]=src[1];dest[2]=src[2];						  } break;
			default: STBI_ASSERT(0); STBI_FREE(data); STBI_FREE(good); return stbi__errpuc("unsupported", "Unsupported format conversion");
		}
		#undef STBI__CASE
	}
	STBI_FREE(data);
	return good;
}
#endif
#if defined(STBI_NO_PNG) && defined(STBI_NO_PSD)
#else
static stbi__uint16 stbi__compute_y_16(int r, int g, int b)
{
	return (stbi__uint16) (((r*77) + (g*150) +  (29*b)) >> 8);
}
#endif
#if defined(STBI_NO_PNG) && defined(STBI_NO_PSD)
#else
static stbi__uint16 *stbi__convert_format16(stbi__uint16 *data, int img_n, int req_comp, unsigned int x, unsigned int y)
{
	int i,j;
	stbi__uint16 *good;
	if (req_comp == img_n) return data;
	STBI_ASSERT(req_comp >= 1 && req_comp <= 4);
	good = (stbi__uint16 *) stbi__malloc(req_comp * x * y * 2);
	if (good == NULL) {
		STBI_FREE(data);
		return (stbi__uint16 *) stbi__errpuc("outofmem", "Out of memory");
	}
	for (j=0; j < (int) y; ++j) {
		stbi__uint16 *src  = data + j * x * img_n	;
		stbi__uint16 *dest = good + j * x * req_comp;
		#define STBI__COMBO(a,b)  ((a)*8+(b))
		#define STBI__CASE(a,b)	case STBI__COMBO(a,b): for(i=x-1; i >= 0; --i, src += a, dest += b)
		switch (STBI__COMBO(img_n, req_comp)) {
			STBI__CASE(1,2) { dest[0]=src[0]; dest[1]=0xffff;												 } break;
			STBI__CASE(1,3) { dest[0]=dest[1]=dest[2]=src[0];												 } break;
			STBI__CASE(1,4) { dest[0]=dest[1]=dest[2]=src[0]; dest[3]=0xffff;							} break;
			STBI__CASE(2,1) { dest[0]=src[0];																	  } break;
			STBI__CASE(2,3) { dest[0]=dest[1]=dest[2]=src[0];												 } break;
			STBI__CASE(2,4) { dest[0]=dest[1]=dest[2]=src[0]; dest[3]=src[1];							} break;
			STBI__CASE(3,4) { dest[0]=src[0];dest[1]=src[1];dest[2]=src[2];dest[3]=0xffff;		  } break;
			STBI__CASE(3,1) { dest[0]=stbi__compute_y_16(src[0],src[1],src[2]);						 } break;
			STBI__CASE(3,2) { dest[0]=stbi__compute_y_16(src[0],src[1],src[2]); dest[1] = 0xffff; } break;
			STBI__CASE(4,1) { dest[0]=stbi__compute_y_16(src[0],src[1],src[2]);						 } break;
			STBI__CASE(4,2) { dest[0]=stbi__compute_y_16(src[0],src[1],src[2]); dest[1] = src[3]; } break;
			STBI__CASE(4,3) { dest[0]=src[0];dest[1]=src[1];dest[2]=src[2];							  } break;
			default: STBI_ASSERT(0); STBI_FREE(data); STBI_FREE(good); return (stbi__uint16*) stbi__errpuc("unsupported", "Unsupported format conversion");
		}
		#undef STBI__CASE
	}
	STBI_FREE(data);
	return good;
}
#endif
#ifndef STBI_NO_ZLIB
#define STBI__ZFAST_BITS  9
#define STBI__ZFAST_MASK  ((1 << STBI__ZFAST_BITS) - 1)
typedef struct
{
	stbi__uint16 fast[1 << STBI__ZFAST_BITS];
	stbi__uint16 firstcode[16];
	int maxcode[17];
	stbi__uint16 firstsymbol[16];
	stbi_uc  size[288];
	stbi__uint16 value[288];
} stbi__zhuffman;
stbi_inline static int stbi__bitreverse16(int n)
{
  n = ((n & 0xAAAA) >>  1) | ((n & 0x5555) << 1);
  n = ((n & 0xCCCC) >>  2) | ((n & 0x3333) << 2);
  n = ((n & 0xF0F0) >>  4) | ((n & 0x0F0F) << 4);
  n = ((n & 0xFF00) >>  8) | ((n & 0x00FF) << 8);
  return n;
}
stbi_inline static int stbi__bit_reverse(int v, int bits)
{
	STBI_ASSERT(bits <= 16);
	return stbi__bitreverse16(v) >> (16-bits);
}
static int stbi__zbuild_huffman(stbi__zhuffman *z, const stbi_uc *sizelist, int num)
{
	int i,k=0;
	int code, next_code[16], sizes[17];
	memset(sizes, 0, sizeof(sizes));
	memset(z->fast, 0, sizeof(z->fast));
	for (i=0; i < num; ++i)
		++sizes[sizelist[i]];
	sizes[0] = 0;
	for (i=1; i < 16; ++i)
		if (sizes[i] > (1 << i))
			return stbi__err("bad sizes", "Corrupt PNG");
	code = 0;
	for (i=1; i < 16; ++i) {
		next_code[i] = code;
		z->firstcode[i] = (stbi__uint16) code;
		z->firstsymbol[i] = (stbi__uint16) k;
		code = (code + sizes[i]);
		if (sizes[i])
			if (code-1 >= (1 << i)) return stbi__err("bad codelengths","Corrupt PNG");
		z->maxcode[i] = code << (16-i);
		code <<= 1;
		k += sizes[i];
	}
	z->maxcode[16] = 0x10000;
	for (i=0; i < num; ++i) {
		int s = sizelist[i];
		if (s) {
			int c = next_code[s] - z->firstcode[s] + z->firstsymbol[s];
			stbi__uint16 fastv = (stbi__uint16) ((s << 9) | i);
			z->size [c] = (stbi_uc	  ) s;
			z->value[c] = (stbi__uint16) i;
			if (s <= STBI__ZFAST_BITS) {
				int j = stbi__bit_reverse(next_code[s],s);
				while (j < (1 << STBI__ZFAST_BITS)) {
					z->fast[j] = fastv;
					j += (1 << s);
				}
			}
			++next_code[s];
		}
	}
	return 1;
}
typedef struct
{
	stbi_uc *zbuffer, *zbuffer_end;
	int num_bits;
	stbi__uint32 code_buffer;
	char *zout;
	char *zout_start;
	char *zout_end;
	int	z_expandable;
	stbi__zhuffman z_length, z_distance;
} stbi__zbuf;
stbi_inline static int stbi__zeof(stbi__zbuf *z)
{
	return (z->zbuffer >= z->zbuffer_end);
}
stbi_inline static stbi_uc stbi__zget8(stbi__zbuf *z)
{
	return stbi__zeof(z) ? 0 : *z->zbuffer++;
}
static void stbi__fill_bits(stbi__zbuf *z)
{
	do {
		if (z->code_buffer >= (1U << z->num_bits)) {
		  z->zbuffer = z->zbuffer_end;  /* treat this as EOF so we fail. */
		  return;
		}
		z->code_buffer |= (unsigned int) stbi__zget8(z) << z->num_bits;
		z->num_bits += 8;
	} while (z->num_bits <= 24);
}
stbi_inline static unsigned int stbi__zreceive(stbi__zbuf *z, int n)
{
	unsigned int k;
	if (z->num_bits < n) stbi__fill_bits(z);
	k = z->code_buffer & ((1 << n) - 1);
	z->code_buffer >>= n;
	z->num_bits -= n;
	return k;
}
static int stbi__zhuffman_decode_slowpath(stbi__zbuf *a, stbi__zhuffman *z)
{
	int b,s,k;
	k = stbi__bit_reverse(a->code_buffer, 16);
	for (s=STBI__ZFAST_BITS+1; ; ++s)
		if (k < z->maxcode[s])
			break;
	if (s >= 16) return -1;
	b = (k >> (16-s)) - z->firstcode[s] + z->firstsymbol[s];
	if (b >= sizeof (z->size)) return -1;
	if (z->size[b] != s) return -1;
	a->code_buffer >>= s;
	a->num_bits -= s;
	return z->value[b];
}
stbi_inline static int stbi__zhuffman_decode(stbi__zbuf *a, stbi__zhuffman *z)
{
	int b,s;
	if (a->num_bits < 16) {
		if (stbi__zeof(a)) {
			return -1;	/* report error for unexpected end of data. */
		}
		stbi__fill_bits(a);
	}
	b = z->fast[a->code_buffer & STBI__ZFAST_MASK];
	if (b) {
		s = b >> 9;
		a->code_buffer >>= s;
		a->num_bits -= s;
		return b & 511;
	}
	return stbi__zhuffman_decode_slowpath(a, z);
}
static int stbi__zexpand(stbi__zbuf *z, char *zout, int n)
{
	char *q;
	unsigned int cur, limit, old_limit;
	z->zout = zout;
	if (!z->z_expandable) return stbi__err("output buffer limit","Corrupt PNG");
	cur	= (unsigned int) (z->zout - z->zout_start);
	limit = old_limit = (unsigned) (z->zout_end - z->zout_start);
	if (UINT_MAX - cur < (unsigned) n) return stbi__err("outofmem", "Out of memory");
	while (cur + n > limit) {
		if(limit > UINT_MAX / 2) return stbi__err("outofmem", "Out of memory");
		limit *= 2;
	}
	q = (char *) STBI_REALLOC_SIZED(z->zout_start, old_limit, limit);
	STBI_NOTUSED(old_limit);
	if (q == NULL) return stbi__err("outofmem", "Out of memory");
	z->zout_start = q;
	z->zout		 = q + cur;
	z->zout_end	= q + limit;
	return 1;
}
static const int stbi__zlength_base[31] = {
	3,4,5,6,7,8,9,10,11,13,
	15,17,19,23,27,31,35,43,51,59,
	67,83,99,115,131,163,195,227,258,0,0 };
static const int stbi__zlength_extra[31]=
{ 0,0,0,0,0,0,0,0,1,1,1,1,2,2,2,2,3,3,3,3,4,4,4,4,5,5,5,5,0,0,0 };
static const int stbi__zdist_base[32] = { 1,2,3,4,5,7,9,13,17,25,33,49,65,97,129,193,
257,385,513,769,1025,1537,2049,3073,4097,6145,8193,12289,16385,24577,0,0};
static const int stbi__zdist_extra[32] =
{ 0,0,0,0,1,1,2,2,3,3,4,4,5,5,6,6,7,7,8,8,9,9,10,10,11,11,12,12,13,13};
static int stbi__parse_huffman_block(stbi__zbuf *a)
{
	char *zout = a->zout;
	for(;;) {
		int z = stbi__zhuffman_decode(a, &a->z_length);
		if (z < 256) {
			if (z < 0) return stbi__err("bad huffman code","Corrupt PNG");
			if (zout >= a->zout_end) {
				if (!stbi__zexpand(a, zout, 1)) return 0;
				zout = a->zout;
			}
			*zout++ = (char) z;
		} else {
			stbi_uc *p;
			int len,dist;
			if (z == 256) {
				a->zout = zout;
				return 1;
			}
			z -= 257;
			len = stbi__zlength_base[z];
			if (stbi__zlength_extra[z]) len += stbi__zreceive(a, stbi__zlength_extra[z]);
			z = stbi__zhuffman_decode(a, &a->z_distance);
			if (z < 0) return stbi__err("bad huffman code","Corrupt PNG");
			dist = stbi__zdist_base[z];
			if (stbi__zdist_extra[z]) dist += stbi__zreceive(a, stbi__zdist_extra[z]);
			if (zout - a->zout_start < dist) return stbi__err("bad dist","Corrupt PNG");
			if (zout + len > a->zout_end) {
				if (!stbi__zexpand(a, zout, len)) return 0;
				zout = a->zout;
			}
			p = (stbi_uc *) (zout - dist);
			if (dist == 1) {
				stbi_uc v = *p;
				if (len) { do *zout++ = v; while (--len); }
			} else {
				if (len) { do *zout++ = *p++; while (--len); }
			}
		}
	}
}
static int stbi__compute_huffman_codes(stbi__zbuf *a)
{
	static const stbi_uc length_dezigzag[19] = { 16,17,18,0,8,7,9,6,10,5,11,4,12,3,13,2,14,1,15 };
	stbi__zhuffman z_codelength;
	stbi_uc lencodes[286+32+137];
	stbi_uc codelength_sizes[19];
	int i,n;
	int hlit  = stbi__zreceive(a,5) + 257;
	int hdist = stbi__zreceive(a,5) + 1;
	int hclen = stbi__zreceive(a,4) + 4;
	int ntot  = hlit + hdist;
	memset(codelength_sizes, 0, sizeof(codelength_sizes));
	for (i=0; i < hclen; ++i) {
		int s = stbi__zreceive(a,3);
		codelength_sizes[length_dezigzag[i]] = (stbi_uc) s;
	}
	if (!stbi__zbuild_huffman(&z_codelength, codelength_sizes, 19)) return 0;
	n = 0;
	while (n < ntot) {
		int c = stbi__zhuffman_decode(a, &z_codelength);
		if (c < 0 || c >= 19) return stbi__err("bad codelengths", "Corrupt PNG");
		if (c < 16)
			lencodes[n++] = (stbi_uc) c;
		else {
			stbi_uc fill = 0;
			if (c == 16) {
				c = stbi__zreceive(a,2)+3;
				if (n == 0) return stbi__err("bad codelengths", "Corrupt PNG");
				fill = lencodes[n-1];
			} else if (c == 17) {
				c = stbi__zreceive(a,3)+3;
			} else if (c == 18) {
				c = stbi__zreceive(a,7)+11;
			} else {
				return stbi__err("bad codelengths", "Corrupt PNG");
			}
			if (ntot - n < c) return stbi__err("bad codelengths", "Corrupt PNG");
			memset(lencodes+n, fill, c);
			n += c;
		}
	}
	if (n != ntot) return stbi__err("bad codelengths","Corrupt PNG");
	if (!stbi__zbuild_huffman(&a->z_length, lencodes, hlit)) return 0;
	if (!stbi__zbuild_huffman(&a->z_distance, lencodes+hlit, hdist)) return 0;
	return 1;
}
static int stbi__parse_uncompressed_block(stbi__zbuf *a)
{
	stbi_uc header[4];
	int len,nlen,k;
	if (a->num_bits & 7)
		stbi__zreceive(a, a->num_bits & 7);
	k = 0;
	while (a->num_bits > 0) {
		header[k++] = (stbi_uc) (a->code_buffer & 255);
		a->code_buffer >>= 8;
		a->num_bits -= 8;
	}
	if (a->num_bits < 0) return stbi__err("zlib corrupt","Corrupt PNG");
	while (k < 4)
		header[k++] = stbi__zget8(a);
	len  = header[1] * 256 + header[0];
	nlen = header[3] * 256 + header[2];
	if (nlen != (len ^ 0xffff)) return stbi__err("zlib corrupt","Corrupt PNG");
	if (a->zbuffer + len > a->zbuffer_end) return stbi__err("read past buffer","Corrupt PNG");
	if (a->zout + len > a->zout_end)
		if (!stbi__zexpand(a, a->zout, len)) return 0;
	memcpy(a->zout, a->zbuffer, len);
	a->zbuffer += len;
	a->zout += len;
	return 1;
}
static int stbi__parse_zlib_header(stbi__zbuf *a)
{
	int cmf	= stbi__zget8(a);
	int cm	 = cmf & 15;
	/* int cinfo = cmf >> 4; */
	int flg	= stbi__zget8(a);
	if (stbi__zeof(a)) return stbi__err("bad zlib header","Corrupt PNG");
	if ((cmf*256+flg) % 31 != 0) return stbi__err("bad zlib header","Corrupt PNG");
	if (flg & 32) return stbi__err("no preset dict","Corrupt PNG");
	if (cm != 8) return stbi__err("bad compression","Corrupt PNG");
	return 1;
}
static const stbi_uc stbi__zdefault_length[288] =
{
	8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8, 8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,
	8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8, 8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,
	8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8, 8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,
	8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8, 8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,
	8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8, 9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,
	9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9, 9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,
	9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9, 9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,
	9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9, 9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,
	7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7, 7,7,7,7,7,7,7,7,8,8,8,8,8,8,8,8
};
static const stbi_uc stbi__zdefault_distance[32] =
{
	5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5
};
static int stbi__parse_zlib(stbi__zbuf *a, int parse_header)
{
	int final, type;
	if (parse_header)
		if (!stbi__parse_zlib_header(a)) return 0;
	a->num_bits = 0;
	a->code_buffer = 0;
	do {
		final = stbi__zreceive(a,1);
		type = stbi__zreceive(a,2);
		if (type == 0) {
			if (!stbi__parse_uncompressed_block(a)) return 0;
		} else if (type == 3) {
			return 0;
		} else {
			if (type == 1) {
				if (!stbi__zbuild_huffman(&a->z_length  , stbi__zdefault_length  , 288)) return 0;
				if (!stbi__zbuild_huffman(&a->z_distance, stbi__zdefault_distance,  32)) return 0;
			} else {
				if (!stbi__compute_huffman_codes(a)) return 0;
			}
			if (!stbi__parse_huffman_block(a)) return 0;
		}
	} while (!final);
	return 1;
}
static int stbi__do_zlib(stbi__zbuf *a, char *obuf, int olen, int exp, int parse_header)
{
	a->zout_start = obuf;
	a->zout		 = obuf;
	a->zout_end	= obuf + olen;
	a->z_expandable = exp;
	return stbi__parse_zlib(a, parse_header);
}
STBIDEF char *stbi_zlib_decode_malloc_guesssize(const char *buffer, int len, int initial_size, int *outlen)
{
	stbi__zbuf a;
	char *p = (char *) stbi__malloc(initial_size);
	if (p == NULL) return NULL;
	a.zbuffer = (stbi_uc *) buffer;
	a.zbuffer_end = (stbi_uc *) buffer + len;
	if (stbi__do_zlib(&a, p, initial_size, 1, 1)) {
		if (outlen) *outlen = (int) (a.zout - a.zout_start);
		return a.zout_start;
	} else {
		STBI_FREE(a.zout_start);
		return NULL;
	}
}
STBIDEF char *stbi_zlib_decode_malloc(char const *buffer, int len, int *outlen)
{
	return stbi_zlib_decode_malloc_guesssize(buffer, len, 16384, outlen);
}
STBIDEF char *stbi_zlib_decode_malloc_guesssize_headerflag(const char *buffer, int len, int initial_size, int *outlen, int parse_header)
{
	stbi__zbuf a;
	char *p = (char *) stbi__malloc(initial_size);
	if (p == NULL) return NULL;
	a.zbuffer = (stbi_uc *) buffer;
	a.zbuffer_end = (stbi_uc *) buffer + len;
	if (stbi__do_zlib(&a, p, initial_size, 1, parse_header)) {
		if (outlen) *outlen = (int) (a.zout - a.zout_start);
		return a.zout_start;
	} else {
		STBI_FREE(a.zout_start);
		return NULL;
	}
}
STBIDEF int stbi_zlib_decode_buffer(char *obuffer, int olen, char const *ibuffer, int ilen)
{
	stbi__zbuf a;
	a.zbuffer = (stbi_uc *) ibuffer;
	a.zbuffer_end = (stbi_uc *) ibuffer + ilen;
	if (stbi__do_zlib(&a, obuffer, olen, 0, 1))
		return (int) (a.zout - a.zout_start);
	else
		return -1;
}
STBIDEF char *stbi_zlib_decode_noheader_malloc(char const *buffer, int len, int *outlen)
{
	stbi__zbuf a;
	char *p = (char *) stbi__malloc(16384);
	if (p == NULL) return NULL;
	a.zbuffer = (stbi_uc *) buffer;
	a.zbuffer_end = (stbi_uc *) buffer+len;
	if (stbi__do_zlib(&a, p, 16384, 1, 0)) {
		if (outlen) *outlen = (int) (a.zout - a.zout_start);
		return a.zout_start;
	} else {
		STBI_FREE(a.zout_start);
		return NULL;
	}
}
STBIDEF int stbi_zlib_decode_noheader_buffer(char *obuffer, int olen, const char *ibuffer, int ilen)
{
	stbi__zbuf a;
	a.zbuffer = (stbi_uc *) ibuffer;
	a.zbuffer_end = (stbi_uc *) ibuffer + ilen;
	if (stbi__do_zlib(&a, obuffer, olen, 0, 0))
		return (int) (a.zout - a.zout_start);
	else
		return -1;
}
#endif
#ifndef STBI_NO_PNG
typedef struct
{
	stbi__uint32 length;
	stbi__uint32 type;
} stbi__pngchunk;
static stbi__pngchunk stbi__get_chunk_header(stbi__context *s)
{
	stbi__pngchunk c;
	c.length = stbi__get32be(s);
	c.type	= stbi__get32be(s);
	return c;
}
static int stbi__check_png_header(stbi__context *s)
{
	static const stbi_uc png_sig[8] = { 137,80,78,71,13,10,26,10 };
	int i;
	for (i=0; i < 8; ++i)
		if (stbi__get8(s) != png_sig[i]) return stbi__err("bad png sig","Not a PNG");
	return 1;
}
typedef struct
{
	stbi__context *s;
	stbi_uc *idata, *expanded, *out;
	int depth;
} stbi__png;
enum {
	STBI__F_none=0,
	STBI__F_sub=1,
	STBI__F_up=2,
	STBI__F_avg=3,
	STBI__F_paeth=4,
	STBI__F_avg_first,
	STBI__F_paeth_first
};
static stbi_uc first_row_filter[5] =
{
	STBI__F_none,
	STBI__F_sub,
	STBI__F_none,
	STBI__F_avg_first,
	STBI__F_paeth_first
};
static int stbi__paeth(int a, int b, int c)
{
	int p = a + b - c;
	int pa = abs(p-a);
	int pb = abs(p-b);
	int pc = abs(p-c);
	if (pa <= pb && pa <= pc) return a;
	if (pb <= pc) return b;
	return c;
}
static const stbi_uc stbi__depth_scale_table[9] = { 0, 0xff, 0x55, 0, 0x11, 0,0,0, 0x01 };
static int stbi__create_png_image_raw(stbi__png *a, stbi_uc *raw, stbi__uint32 raw_len, int out_n, stbi__uint32 x, stbi__uint32 y, int depth, int color)
{
	int bytes = (depth == 16? 2 : 1);
	stbi__context *s = a->s;
	stbi__uint32 i,j,stride = x*out_n*bytes;
	stbi__uint32 img_len, img_width_bytes;
	int k;
	int img_n = s->img_n;
	int output_bytes = out_n*bytes;
	int filter_bytes = img_n*bytes;
	int width = x;
	STBI_ASSERT(out_n == s->img_n || out_n == s->img_n+1);
	a->out = (stbi_uc *) stbi__malloc_mad3(x, y, output_bytes, 0);
	if (!a->out) return stbi__err("outofmem", "Out of memory");
	if (!stbi__mad3sizes_valid(img_n, x, depth, 7)) return stbi__err("too large", "Corrupt PNG");
	img_width_bytes = (((img_n * x * depth) + 7) >> 3);
	img_len = (img_width_bytes + 1) * y;
	if (raw_len < img_len) return stbi__err("not enough pixels","Corrupt PNG");
	for (j=0; j < y; ++j) {
		stbi_uc *cur = a->out + stride*j;
		stbi_uc *prior;
		int filter = *raw++;
		if (filter > 4)
			return stbi__err("invalid filter","Corrupt PNG");
		if (depth < 8) {
			if (img_width_bytes > x) return stbi__err("invalid width","Corrupt PNG");
			cur += x*out_n - img_width_bytes;
			filter_bytes = 1;
			width = img_width_bytes;
		}
		prior = cur - stride;
		if (j == 0) filter = first_row_filter[filter];
		for (k=0; k < filter_bytes; ++k) {
			switch (filter) {
				case STBI__F_none		 : cur[k] = raw[k]; break;
				case STBI__F_sub		  : cur[k] = raw[k]; break;
				case STBI__F_up			: cur[k] = STBI__BYTECAST(raw[k] + prior[k]); break;
				case STBI__F_avg		  : cur[k] = STBI__BYTECAST(raw[k] + (prior[k]>>1)); break;
				case STBI__F_paeth		: cur[k] = STBI__BYTECAST(raw[k] + stbi__paeth(0,prior[k],0)); break;
				case STBI__F_avg_first  : cur[k] = raw[k]; break;
				case STBI__F_paeth_first: cur[k] = raw[k]; break;
			}
		}
		if (depth == 8) {
			if (img_n != out_n)
				cur[img_n] = 255;
			raw += img_n;
			cur += out_n;
			prior += out_n;
		} else if (depth == 16) {
			if (img_n != out_n) {
				cur[filter_bytes]	= 255;
				cur[filter_bytes+1] = 255;
			}
			raw += filter_bytes;
			cur += output_bytes;
			prior += output_bytes;
		} else {
			raw += 1;
			cur += 1;
			prior += 1;
		}
		if (depth < 8 || img_n == out_n) {
			int nk = (width - 1)*filter_bytes;
			#define STBI__CASE(f) \
				 case f:	  \
					 for (k=0; k < nk; ++k)
			switch (filter) {
				case STBI__F_none:			memcpy(cur, raw, nk); break;
				STBI__CASE(STBI__F_sub)			 { cur[k] = STBI__BYTECAST(raw[k] + cur[k-filter_bytes]); } break;
				STBI__CASE(STBI__F_up)			  { cur[k] = STBI__BYTECAST(raw[k] + prior[k]); } break;
				STBI__CASE(STBI__F_avg)			 { cur[k] = STBI__BYTECAST(raw[k] + ((prior[k] + cur[k-filter_bytes])>>1)); } break;
				STBI__CASE(STBI__F_paeth)		  { cur[k] = STBI__BYTECAST(raw[k] + stbi__paeth(cur[k-filter_bytes],prior[k],prior[k-filter_bytes])); } break;
				STBI__CASE(STBI__F_avg_first)	 { cur[k] = STBI__BYTECAST(raw[k] + (cur[k-filter_bytes] >> 1)); } break;
				STBI__CASE(STBI__F_paeth_first)  { cur[k] = STBI__BYTECAST(raw[k] + stbi__paeth(cur[k-filter_bytes],0,0)); } break;
			}
			#undef STBI__CASE
			raw += nk;
		} else {
			STBI_ASSERT(img_n+1 == out_n);
			#define STBI__CASE(f) \
				 case f:	  \
					 for (i=x-1; i >= 1; --i, cur[filter_bytes]=255,raw+=filter_bytes,cur+=output_bytes,prior+=output_bytes) \
						 for (k=0; k < filter_bytes; ++k)
			switch (filter) {
				STBI__CASE(STBI__F_none)			{ cur[k] = raw[k]; } break;
				STBI__CASE(STBI__F_sub)			 { cur[k] = STBI__BYTECAST(raw[k] + cur[k- output_bytes]); } break;
				STBI__CASE(STBI__F_up)			  { cur[k] = STBI__BYTECAST(raw[k] + prior[k]); } break;
				STBI__CASE(STBI__F_avg)			 { cur[k] = STBI__BYTECAST(raw[k] + ((prior[k] + cur[k- output_bytes])>>1)); } break;
				STBI__CASE(STBI__F_paeth)		  { cur[k] = STBI__BYTECAST(raw[k] + stbi__paeth(cur[k- output_bytes],prior[k],prior[k- output_bytes])); } break;
				STBI__CASE(STBI__F_avg_first)	 { cur[k] = STBI__BYTECAST(raw[k] + (cur[k- output_bytes] >> 1)); } break;
				STBI__CASE(STBI__F_paeth_first)  { cur[k] = STBI__BYTECAST(raw[k] + stbi__paeth(cur[k- output_bytes],0,0)); } break;
			}
			#undef STBI__CASE
			if (depth == 16) {
				cur = a->out + stride*j;
				for (i=0; i < x; ++i,cur+=output_bytes) {
					cur[filter_bytes+1] = 255;
				}
			}
		}
	}
	if (depth < 8) {
		for (j=0; j < y; ++j) {
			stbi_uc *cur = a->out + stride*j;
			stbi_uc *in  = a->out + stride*j + x*out_n - img_width_bytes;
			stbi_uc scale = (color == 0) ? stbi__depth_scale_table[depth] : 1;
			if (depth == 4) {
				for (k=x*img_n; k >= 2; k-=2, ++in) {
					*cur++ = scale * ((*in >> 4)		 );
					*cur++ = scale * ((*in	  ) & 0x0f);
				}
				if (k > 0) *cur++ = scale * ((*in >> 4)		 );
			} else if (depth == 2) {
				for (k=x*img_n; k >= 4; k-=4, ++in) {
					*cur++ = scale * ((*in >> 6)		 );
					*cur++ = scale * ((*in >> 4) & 0x03);
					*cur++ = scale * ((*in >> 2) & 0x03);
					*cur++ = scale * ((*in	  ) & 0x03);
				}
				if (k > 0) *cur++ = scale * ((*in >> 6)		 );
				if (k > 1) *cur++ = scale * ((*in >> 4) & 0x03);
				if (k > 2) *cur++ = scale * ((*in >> 2) & 0x03);
			} else if (depth == 1) {
				for (k=x*img_n; k >= 8; k-=8, ++in) {
					*cur++ = scale * ((*in >> 7)		 );
					*cur++ = scale * ((*in >> 6) & 0x01);
					*cur++ = scale * ((*in >> 5) & 0x01);
					*cur++ = scale * ((*in >> 4) & 0x01);
					*cur++ = scale * ((*in >> 3) & 0x01);
					*cur++ = scale * ((*in >> 2) & 0x01);
					*cur++ = scale * ((*in >> 1) & 0x01);
					*cur++ = scale * ((*in	  ) & 0x01);
				}
				if (k > 0) *cur++ = scale * ((*in >> 7)		 );
				if (k > 1) *cur++ = scale * ((*in >> 6) & 0x01);
				if (k > 2) *cur++ = scale * ((*in >> 5) & 0x01);
				if (k > 3) *cur++ = scale * ((*in >> 4) & 0x01);
				if (k > 4) *cur++ = scale * ((*in >> 3) & 0x01);
				if (k > 5) *cur++ = scale * ((*in >> 2) & 0x01);
				if (k > 6) *cur++ = scale * ((*in >> 1) & 0x01);
			}
			if (img_n != out_n) {
				int q;
				cur = a->out + stride*j;
				if (img_n == 1) {
					for (q=x-1; q >= 0; --q) {
						cur[q*2+1] = 255;
						cur[q*2+0] = cur[q];
					}
				} else {
					STBI_ASSERT(img_n == 3);
					for (q=x-1; q >= 0; --q) {
						cur[q*4+3] = 255;
						cur[q*4+2] = cur[q*3+2];
						cur[q*4+1] = cur[q*3+1];
						cur[q*4+0] = cur[q*3+0];
					}
				}
			}
		}
	} else if (depth == 16) {
		stbi_uc *cur = a->out;
		stbi__uint16 *cur16 = (stbi__uint16*)cur;
		for(i=0; i < x*y*out_n; ++i,cur16++,cur+=2) {
			*cur16 = (cur[0] << 8) | cur[1];
		}
	}
	return 1;
}
static int stbi__create_png_image(stbi__png *a, stbi_uc *image_data, stbi__uint32 image_data_len, int out_n, int depth, int color, int interlaced)
{
	int bytes = (depth == 16 ? 2 : 1);
	int out_bytes = out_n * bytes;
	stbi_uc *final;
	int p;
	if (!interlaced)
		return stbi__create_png_image_raw(a, image_data, image_data_len, out_n, a->s->img_x, a->s->img_y, depth, color);
	final = (stbi_uc *) stbi__malloc_mad3(a->s->img_x, a->s->img_y, out_bytes, 0);
	for (p=0; p < 7; ++p) {
		int xorig[] = { 0,4,0,2,0,1,0 };
		int yorig[] = { 0,0,4,0,2,0,1 };
		int xspc[]  = { 8,8,4,4,2,2,1 };
		int yspc[]  = { 8,8,8,4,4,2,2 };
		int i,j,x,y;
		x = (a->s->img_x - xorig[p] + xspc[p]-1) / xspc[p];
		y = (a->s->img_y - yorig[p] + yspc[p]-1) / yspc[p];
		if (x && y) {
			stbi__uint32 img_len = ((((a->s->img_n * x * depth) + 7) >> 3) + 1) * y;
			if (!stbi__create_png_image_raw(a, image_data, image_data_len, out_n, x, y, depth, color)) {
				STBI_FREE(final);
				return 0;
			}
			for (j=0; j < y; ++j) {
				for (i=0; i < x; ++i) {
					int out_y = j*yspc[p]+yorig[p];
					int out_x = i*xspc[p]+xorig[p];
					memcpy(final + out_y*a->s->img_x*out_bytes + out_x*out_bytes,
							 a->out + (j*x+i)*out_bytes, out_bytes);
				}
			}
			STBI_FREE(a->out);
			image_data += img_len;
			image_data_len -= img_len;
		}
	}
	a->out = final;
	return 1;
}
static int stbi__compute_transparency(stbi__png *z, stbi_uc tc[3], int out_n)
{
	stbi__context *s = z->s;
	stbi__uint32 i, pixel_count = s->img_x * s->img_y;
	stbi_uc *p = z->out;
	STBI_ASSERT(out_n == 2 || out_n == 4);
	if (out_n == 2) {
		for (i=0; i < pixel_count; ++i) {
			p[1] = (p[0] == tc[0] ? 0 : 255);
			p += 2;
		}
	} else {
		for (i=0; i < pixel_count; ++i) {
			if (p[0] == tc[0] && p[1] == tc[1] && p[2] == tc[2])
				p[3] = 0;
			p += 4;
		}
	}
	return 1;
}
static int stbi__compute_transparency16(stbi__png *z, stbi__uint16 tc[3], int out_n)
{
	stbi__context *s = z->s;
	stbi__uint32 i, pixel_count = s->img_x * s->img_y;
	stbi__uint16 *p = (stbi__uint16*) z->out;
	STBI_ASSERT(out_n == 2 || out_n == 4);
	if (out_n == 2) {
		for (i = 0; i < pixel_count; ++i) {
			p[1] = (p[0] == tc[0] ? 0 : 65535);
			p += 2;
		}
	} else {
		for (i = 0; i < pixel_count; ++i) {
			if (p[0] == tc[0] && p[1] == tc[1] && p[2] == tc[2])
				p[3] = 0;
			p += 4;
		}
	}
	return 1;
}
static int stbi__expand_png_palette(stbi__png *a, stbi_uc *palette, int len, int pal_img_n)
{
	stbi__uint32 i, pixel_count = a->s->img_x * a->s->img_y;
	stbi_uc *p, *temp_out, *orig = a->out;
	p = (stbi_uc *) stbi__malloc_mad2(pixel_count, pal_img_n, 0);
	if (p == NULL) return stbi__err("outofmem", "Out of memory");
	temp_out = p;
	if (pal_img_n == 3) {
		for (i=0; i < pixel_count; ++i) {
			int n = orig[i]*4;
			p[0] = palette[n  ];
			p[1] = palette[n+1];
			p[2] = palette[n+2];
			p += 3;
		}
	} else {
		for (i=0; i < pixel_count; ++i) {
			int n = orig[i]*4;
			p[0] = palette[n  ];
			p[1] = palette[n+1];
			p[2] = palette[n+2];
			p[3] = palette[n+3];
			p += 4;
		}
	}
	STBI_FREE(a->out);
	a->out = temp_out;
	STBI_NOTUSED(len);
	return 1;
}
static int stbi__unpremultiply_on_load = 0;
static int stbi__de_iphone_flag = 0;
STBIDEF void stbi_set_unpremultiply_on_load(int flag_true_if_should_unpremultiply)
{
	stbi__unpremultiply_on_load = flag_true_if_should_unpremultiply;
}
STBIDEF void stbi_convert_iphone_png_to_rgb(int flag_true_if_should_convert)
{
	stbi__de_iphone_flag = flag_true_if_should_convert;
}
static void stbi__de_iphone(stbi__png *z)
{
	stbi__context *s = z->s;
	stbi__uint32 i, pixel_count = s->img_x * s->img_y;
	stbi_uc *p = z->out;
	if (s->img_out_n == 3) {
		for (i=0; i < pixel_count; ++i) {
			stbi_uc t = p[0];
			p[0] = p[2];
			p[2] = t;
			p += 3;
		}
	} else {
		STBI_ASSERT(s->img_out_n == 4);
		if (stbi__unpremultiply_on_load) {
			for (i=0; i < pixel_count; ++i) {
				stbi_uc a = p[3];
				stbi_uc t = p[0];
				if (a) {
					stbi_uc half = a / 2;
					p[0] = (p[2] * 255 + half) / a;
					p[1] = (p[1] * 255 + half) / a;
					p[2] = ( t	* 255 + half) / a;
				} else {
					p[0] = p[2];
					p[2] = t;
				}
				p += 4;
			}
		} else {
			for (i=0; i < pixel_count; ++i) {
				stbi_uc t = p[0];
				p[0] = p[2];
				p[2] = t;
				p += 4;
			}
		}
	}
}
#define STBI__PNG_TYPE(a,b,c,d)  (((unsigned) (a) << 24) + ((unsigned) (b) << 16) + ((unsigned) (c) << 8) + (unsigned) (d))
static int stbi__parse_png_file(stbi__png *z, int scan, int req_comp)
{
	stbi_uc palette[1024], pal_img_n=0;
	stbi_uc has_trans=0, tc[3]={0};
	stbi__uint16 tc16[3];
	stbi__uint32 ioff=0, idata_limit=0, i, pal_len=0;
	int first=1,k,interlace=0, color=0, is_iphone=0;
	stbi__context *s = z->s;
	z->expanded = NULL;
	z->idata = NULL;
	z->out = NULL;
	if (!stbi__check_png_header(s)) return 0;
	if (scan == STBI__SCAN_type) return 1;
	for (;;) {
		stbi__pngchunk c = stbi__get_chunk_header(s);
		switch (c.type) {
			case STBI__PNG_TYPE('C','g','B','I'):
				is_iphone = 1;
				stbi__skip(s, c.length);
				break;
			case STBI__PNG_TYPE('I','H','D','R'): {
				int comp,filter;
				if (!first) return stbi__err("multiple IHDR","Corrupt PNG");
				first = 0;
				if (c.length != 13) return stbi__err("bad IHDR len","Corrupt PNG");
				s->img_x = stbi__get32be(s);
				s->img_y = stbi__get32be(s);
				if (s->img_y > STBI_MAX_DIMENSIONS) return stbi__err("too large","Very large image (corrupt?)");
				if (s->img_x > STBI_MAX_DIMENSIONS) return stbi__err("too large","Very large image (corrupt?)");
				z->depth = stbi__get8(s);  if (z->depth != 1 && z->depth != 2 && z->depth != 4 && z->depth != 8 && z->depth != 16)  return stbi__err("1/2/4/8/16-bit only","PNG not supported: 1/2/4/8/16-bit only");
				color = stbi__get8(s);  if (color > 6)			return stbi__err("bad ctype","Corrupt PNG");
				if (color == 3 && z->depth == 16)						return stbi__err("bad ctype","Corrupt PNG");
				if (color == 3) pal_img_n = 3; else if (color & 1) return stbi__err("bad ctype","Corrupt PNG");
				comp  = stbi__get8(s);  if (comp) return stbi__err("bad comp method","Corrupt PNG");
				filter= stbi__get8(s);  if (filter) return stbi__err("bad filter method","Corrupt PNG");
				interlace = stbi__get8(s); if (interlace>1) return stbi__err("bad interlace method","Corrupt PNG");
				if (!s->img_x || !s->img_y) return stbi__err("0-pixel image","Corrupt PNG");
				if (!pal_img_n) {
					s->img_n = (color & 2 ? 3 : 1) + (color & 4 ? 1 : 0);
					if ((1 << 30) / s->img_x / s->img_n < s->img_y) return stbi__err("too large", "Image too large to decode");
					if (scan == STBI__SCAN_header) return 1;
				} else {
					s->img_n = 1;
					if ((1 << 30) / s->img_x / 4 < s->img_y) return stbi__err("too large","Corrupt PNG");
				}
				break;
			}
			case STBI__PNG_TYPE('P','L','T','E'):  {
				if (first) return stbi__err("first not IHDR", "Corrupt PNG");
				if (c.length > 256*3) return stbi__err("invalid PLTE","Corrupt PNG");
				pal_len = c.length / 3;
				if (pal_len * 3 != c.length) return stbi__err("invalid PLTE","Corrupt PNG");
				for (i=0; i < pal_len; ++i) {
					palette[i*4+0] = stbi__get8(s);
					palette[i*4+1] = stbi__get8(s);
					palette[i*4+2] = stbi__get8(s);
					palette[i*4+3] = 255;
				}
				break;
			}
			case STBI__PNG_TYPE('t','R','N','S'): {
				if (first) return stbi__err("first not IHDR", "Corrupt PNG");
				if (z->idata) return stbi__err("tRNS after IDAT","Corrupt PNG");
				if (pal_img_n) {
					if (scan == STBI__SCAN_header) { s->img_n = 4; return 1; }
					if (pal_len == 0) return stbi__err("tRNS before PLTE","Corrupt PNG");
					if (c.length > pal_len) return stbi__err("bad tRNS len","Corrupt PNG");
					pal_img_n = 4;
					for (i=0; i < c.length; ++i)
						palette[i*4+3] = stbi__get8(s);
				} else {
					if (!(s->img_n & 1)) return stbi__err("tRNS with alpha","Corrupt PNG");
					if (c.length != (stbi__uint32) s->img_n*2) return stbi__err("bad tRNS len","Corrupt PNG");
					has_trans = 1;
					if (z->depth == 16) {
						for (k = 0; k < s->img_n; ++k) tc16[k] = (stbi__uint16)stbi__get16be(s);
					} else {
						for (k = 0; k < s->img_n; ++k) tc[k] = (stbi_uc)(stbi__get16be(s) & 255) * stbi__depth_scale_table[z->depth];
					}
				}
				break;
			}
			case STBI__PNG_TYPE('I','D','A','T'): {
				if (first) return stbi__err("first not IHDR", "Corrupt PNG");
				if (pal_img_n && !pal_len) return stbi__err("no PLTE","Corrupt PNG");
				if (scan == STBI__SCAN_header) { s->img_n = pal_img_n; return 1; }
				if ((int)(ioff + c.length) < (int)ioff) return 0;
				if (ioff + c.length > idata_limit) {
					stbi__uint32 idata_limit_old = idata_limit;
					stbi_uc *p;
					if (idata_limit == 0) idata_limit = c.length > 4096 ? c.length : 4096;
					while (ioff + c.length > idata_limit)
						idata_limit *= 2;
					STBI_NOTUSED(idata_limit_old);
					p = (stbi_uc *) STBI_REALLOC_SIZED(z->idata, idata_limit_old, idata_limit); if (p == NULL) return stbi__err("outofmem", "Out of memory");
					z->idata = p;
				}
				if (!stbi__getn(s, z->idata+ioff,c.length)) return stbi__err("outofdata","Corrupt PNG");
				ioff += c.length;
				break;
			}
			case STBI__PNG_TYPE('I','E','N','D'): {
				stbi__uint32 raw_len, bpl;
				if (first) return stbi__err("first not IHDR", "Corrupt PNG");
				if (scan != STBI__SCAN_load) return 1;
				if (z->idata == NULL) return stbi__err("no IDAT","Corrupt PNG");
				bpl = (s->img_x * z->depth + 7) / 8;
				raw_len = bpl * s->img_y * s->img_n /* pixels */ + s->img_y /* filter mode per row */;
				z->expanded = (stbi_uc *) stbi_zlib_decode_malloc_guesssize_headerflag((char *) z->idata, ioff, raw_len, (int *) &raw_len, !is_iphone);
				if (z->expanded == NULL) return 0;
				STBI_FREE(z->idata); z->idata = NULL;
				if ((req_comp == s->img_n+1 && req_comp != 3 && !pal_img_n) || has_trans)
					s->img_out_n = s->img_n+1;
				else
					s->img_out_n = s->img_n;
				if (!stbi__create_png_image(z, z->expanded, raw_len, s->img_out_n, z->depth, color, interlace)) return 0;
				if (has_trans) {
					if (z->depth == 16) {
						if (!stbi__compute_transparency16(z, tc16, s->img_out_n)) return 0;
					} else {
						if (!stbi__compute_transparency(z, tc, s->img_out_n)) return 0;
					}
				}
				if (is_iphone && stbi__de_iphone_flag && s->img_out_n > 2)
					stbi__de_iphone(z);
				if (pal_img_n) {
					s->img_n = pal_img_n;
					s->img_out_n = pal_img_n;
					if (req_comp >= 3) s->img_out_n = req_comp;
					if (!stbi__expand_png_palette(z, palette, pal_len, s->img_out_n))
						return 0;
				} else if (has_trans) {
					++s->img_n;
				}
				STBI_FREE(z->expanded); z->expanded = NULL;
				stbi__get32be(s);
				return 1;
			}
			default:
				if (first) return stbi__err("first not IHDR", "Corrupt PNG");
				if ((c.type & (1 << 29)) == 0) {
					#ifndef STBI_NO_FAILURE_STRINGS
					static char invalid_chunk[] = "XXXX PNG chunk not known";
					invalid_chunk[0] = STBI__BYTECAST(c.type >> 24);
					invalid_chunk[1] = STBI__BYTECAST(c.type >> 16);
					invalid_chunk[2] = STBI__BYTECAST(c.type >>  8);
					invalid_chunk[3] = STBI__BYTECAST(c.type >>  0);
					#endif
					return stbi__err(invalid_chunk, "PNG not supported: unknown PNG chunk type");
				}
				stbi__skip(s, c.length);
				break;
		}
		stbi__get32be(s);
	}
}
static void *stbi__do_png(stbi__png *p, int *x, int *y, int *n, int req_comp, stbi__result_info *ri)
{
	void *result=NULL;
	if (req_comp < 0 || req_comp > 4) return stbi__errpuc("bad req_comp", "Internal error");
	if (stbi__parse_png_file(p, STBI__SCAN_load, req_comp)) {
		if (p->depth <= 8)
			ri->bits_per_channel = 8;
		else if (p->depth == 16)
			ri->bits_per_channel = 16;
		else
			return stbi__errpuc("bad bits_per_channel", "PNG not supported: unsupported color depth");
		result = p->out;
		p->out = NULL;
		if (req_comp && req_comp != p->s->img_out_n) {
			if (ri->bits_per_channel == 8)
				result = stbi__convert_format((unsigned char *) result, p->s->img_out_n, req_comp, p->s->img_x, p->s->img_y);
			else
				result = stbi__convert_format16((stbi__uint16 *) result, p->s->img_out_n, req_comp, p->s->img_x, p->s->img_y);
			p->s->img_out_n = req_comp;
			if (result == NULL) return result;
		}
		*x = p->s->img_x;
		*y = p->s->img_y;
		if (n) *n = p->s->img_n;
	}
	STBI_FREE(p->out);		p->out		= NULL;
	STBI_FREE(p->expanded); p->expanded = NULL;
	STBI_FREE(p->idata);	 p->idata	 = NULL;
	return result;
}
static void *stbi__png_load(stbi__context *s, int *x, int *y, int *comp, int req_comp, stbi__result_info *ri)
{
	stbi__png p;
	p.s = s;
	return stbi__do_png(&p, x,y,comp,req_comp, ri);
}
static int stbi__png_test(stbi__context *s)
{
	int r;
	r = stbi__check_png_header(s);
	stbi__rewind(s);
	return r;
}
static int stbi__png_info_raw(stbi__png *p, int *x, int *y, int *comp)
{
	if (!stbi__parse_png_file(p, STBI__SCAN_header, 0)) {
		stbi__rewind( p->s );
		return 0;
	}
	if (x) *x = p->s->img_x;
	if (y) *y = p->s->img_y;
	if (comp) *comp = p->s->img_n;
	return 1;
}
static int stbi__png_info(stbi__context *s, int *x, int *y, int *comp)
{
	stbi__png p;
	p.s = s;
	return stbi__png_info_raw(&p, x, y, comp);
}
static int stbi__png_is16(stbi__context *s)
{
	stbi__png p;
	p.s = s;
	if (!stbi__png_info_raw(&p, NULL, NULL, NULL))
		return 0;
	if (p.depth != 16) {
		stbi__rewind(p.s);
		return 0;
	}
	return 1;
}
#endif
static int stbi__info_main(stbi__context *s, int *x, int *y, int *comp) {
	if (stbi__png_info(s, x, y, comp))  return 1;
	return stbi__err("unknown image type", "Image not of any known type, or corrupt");
}
static int stbi__is_16_main(stbi__context *s)
{
	if (stbi__png_is16(s))  return 1;
	return 0;
}
#ifndef STBI_NO_STDIO
STBIDEF int stbi_info(char const *filename, int *x, int *y, int *comp)
{
	 FILE *f = stbi__fopen(filename, "rb");
	 int result;
	 if (!f) return stbi__err("can't fopen", "Unable to open file");
	 result = stbi_info_from_file(f, x, y, comp);
	 fclose(f);
	 return result;
}
STBIDEF int stbi_info_from_file(FILE *f, int *x, int *y, int *comp)
{
	int r;
	stbi__context s;
	long pos = ftell(f);
	stbi__start_file(&s, f);
	r = stbi__info_main(&s,x,y,comp);
	fseek(f,pos,SEEK_SET);
	return r;
}
STBIDEF int stbi_is_16_bit(char const *filename)
{
	 FILE *f = stbi__fopen(filename, "rb");
	 int result;
	 if (!f) return stbi__err("can't fopen", "Unable to open file");
	 result = stbi_is_16_bit_from_file(f);
	 fclose(f);
	 return result;
}
STBIDEF int stbi_is_16_bit_from_file(FILE *f)
{
	int r;
	stbi__context s;
	long pos = ftell(f);
	stbi__start_file(&s, f);
	r = stbi__is_16_main(&s);
	fseek(f,pos,SEEK_SET);
	return r;
}
#endif
STBIDEF int stbi_info_from_memory(stbi_uc const *buffer, int len, int *x, int *y, int *comp)
{
	stbi__context s;
	stbi__start_mem(&s,buffer,len);
	return stbi__info_main(&s,x,y,comp);
}
STBIDEF int stbi_info_from_callbacks(stbi_io_callbacks const *c, void *user, int *x, int *y, int *comp)
{
	stbi__context s;
	stbi__start_callbacks(&s, (stbi_io_callbacks *) c, user);
	return stbi__info_main(&s,x,y,comp);
}
STBIDEF int stbi_is_16_bit_from_memory(stbi_uc const *buffer, int len)
{
	stbi__context s;
	stbi__start_mem(&s,buffer,len);
	return stbi__is_16_main(&s);
}
STBIDEF int stbi_is_16_bit_from_callbacks(stbi_io_callbacks const *c, void *user)
{
	stbi__context s;
	stbi__start_callbacks(&s, (stbi_io_callbacks *) c, user);
	return stbi__is_16_main(&s);
}
#endif

// stb_vorbis.h
#if V_OGG
typedef struct
{
   char *alloc_buffer;
   int alloc_buffer_length_in_bytes;
} stb_vorbis_alloc;
typedef struct stb_vorbis stb_vorbis;
typedef struct
{
   unsigned int sample_rate;
   int channels;
   unsigned int setup_memory_required;
   unsigned int setup_temp_memory_required;
   unsigned int temp_memory_required;
   int max_frame_size;
} stb_vorbis_info;
typedef struct
{
   char *vendor;
   int comment_list_length;
   char **comment_list;
} stb_vorbis_comment;
extern stb_vorbis_info stb_vorbis_get_info(stb_vorbis *f);
extern stb_vorbis_comment stb_vorbis_get_comment(stb_vorbis *f);
extern int stb_vorbis_get_error(stb_vorbis *f);
extern void stb_vorbis_close(stb_vorbis *f);
extern int stb_vorbis_get_sample_offset(stb_vorbis *f);
extern unsigned int stb_vorbis_get_file_offset(stb_vorbis *f);
extern int stb_vorbis_decode_filename(const char *filename, int *channels, int *sample_rate, short **output);
extern int stb_vorbis_decode_memory(const unsigned char *mem, int len, int *channels, int *sample_rate, short **output);
extern stb_vorbis * stb_vorbis_open_memory(const unsigned char *data, int len, int *error, const stb_vorbis_alloc *alloc_buffer);
extern stb_vorbis * stb_vorbis_open_filename(const char *filename, int *error, const stb_vorbis_alloc *alloc_buffer);
extern stb_vorbis * stb_vorbis_open_file(FILE *f, int close_handle_on_close, int *error, const stb_vorbis_alloc *alloc_buffer);
extern stb_vorbis * stb_vorbis_open_file_section(FILE *f, int close_handle_on_close, int *error, const stb_vorbis_alloc *alloc_buffer, unsigned int len);
extern int stb_vorbis_seek_frame(stb_vorbis *f, unsigned int sample_number);
extern int stb_vorbis_seek(stb_vorbis *f, unsigned int sample_number);
extern int stb_vorbis_seek_start(stb_vorbis *f);
extern unsigned int stb_vorbis_stream_length_in_samples(stb_vorbis *f);
extern float stb_vorbis_stream_length_in_seconds(stb_vorbis *f);
extern int stb_vorbis_get_frame_float(stb_vorbis *f, int *channels, float ***output);
extern int stb_vorbis_get_frame_short_interleaved(stb_vorbis *f, int num_c, short *buffer, int num_shorts);
extern int stb_vorbis_get_frame_short (stb_vorbis *f, int num_c, short **buffer, int num_samples);
extern int stb_vorbis_get_samples_float_interleaved(stb_vorbis *f, int channels, float *buffer, int num_floats);
extern int stb_vorbis_get_samples_float(stb_vorbis *f, int channels, float **buffer, int num_samples);
extern int stb_vorbis_get_samples_short_interleaved(stb_vorbis *f, int channels, short *buffer, int num_shorts);
extern int stb_vorbis_get_samples_short(stb_vorbis *f, int channels, short **buffer, int num_samples);
enum STBVorbisError
{
   VORBIS__no_error,
   VORBIS_need_more_data=1,
   VORBIS_invalid_api_mixing,
   VORBIS_outofmem,
   VORBIS_feature_not_supported,
   VORBIS_too_many_channels,
   VORBIS_file_open_failure,
   VORBIS_seek_without_length,
   VORBIS_unexpected_eof=10,
   VORBIS_seek_invalid,
   VORBIS_invalid_setup=20,
   VORBIS_invalid_stream,
   VORBIS_missing_capture_pattern=30,
   VORBIS_invalid_stream_structure_version,
   VORBIS_continued_packet_flag_invalid,
   VORBIS_incorrect_stream_serial_number,
   VORBIS_invalid_first_page,
   VORBIS_bad_packet_type,
   VORBIS_cant_find_last_page,
   VORBIS_seek_failed,
   VORBIS_ogg_skeleton_not_supported
};
#define STB_VORBIS_MAX_CHANNELS 16
#define STB_VORBIS_PUSHDATA_CRC_COUNT 4
#define STB_VORBIS_FAST_HUFFMAN_LENGTH 10
#define STB_VORBIS_FAST_HUFFMAN_SHORT
#ifndef STB_VORBIS_BIG_ENDIAN
#define STB_VORBIS_ENDIAN 0
#else
#define STB_VORBIS_ENDIAN 1
#endif
#ifdef __MINGW32__
   #ifdef __forceinline
   #undef __forceinline
   #endif
   #define __forceinline
#elif !defined(_MSC_VER)
   #if __GNUC__
      #define __forceinline inline
   #else
      #define __forceinline
   #endif
#endif
#if 0
#include <crtdbg.h>
#define CHECK(f) _CrtIsValidHeapPointer(f->channel_buffers[1])
#else
#define CHECK(f) ((void) 0)
#endif
#define MAX_BLOCKSIZE_LOG 13
#define MAX_BLOCKSIZE (1 << MAX_BLOCKSIZE_LOG)
#ifndef TRUE
#define TRUE 1
#define FALSE 0
#endif
typedef float codetype;
#ifdef _MSC_VER
#define STBV_NOTUSED(v) (void)(v)
#else
#define STBV_NOTUSED(v) (void)sizeof(v)
#endif
#define FAST_HUFFMAN_TABLE_SIZE (1 << STB_VORBIS_FAST_HUFFMAN_LENGTH)
#define FAST_HUFFMAN_TABLE_MASK (FAST_HUFFMAN_TABLE_SIZE - 1)
typedef struct
{
   int dimensions, entries;
   uint8_t *codeword_lengths;
   float minimum_value;
   float delta_value;
   uint8_t value_bits;
   uint8_t lookup_type;
   uint8_t sequence_p;
   uint8_t sparse;
   uint32_t lookup_values;
   codetype *multiplicands;
   uint32_t *codewords;
   #ifdef STB_VORBIS_FAST_HUFFMAN_SHORT
    int16_t fast_huffman[FAST_HUFFMAN_TABLE_SIZE];
   #else
    int32_t fast_huffman[FAST_HUFFMAN_TABLE_SIZE];
   #endif
   uint32_t *sorted_codewords;
   int *sorted_values;
   int sorted_entries;
} Codebook;
typedef struct
{
   uint8_t order;
   uint16_t rate;
   uint16_t bark_map_size;
   uint8_t amplitude_bits;
   uint8_t amplitude_offset;
   uint8_t number_of_books;
   uint8_t book_list[16];
} Floor0;
typedef struct
{
   uint8_t partitions;
   uint8_t partition_class_list[32];
   uint8_t class_dimensions[16];
   uint8_t class_subclasses[16];
   uint8_t class_masterbooks[16];
   int16_t subclass_books[16][8];
   uint16_t Xlist[31*8+2];
   uint8_t sorted_order[31*8+2];
   uint8_t neighbors[31*8+2][2];
   uint8_t floor1_multiplier;
   uint8_t rangebits;
   int values;
} Floor1;
typedef union
{
   Floor0 floor0;
   Floor1 floor1;
} Floor;
typedef struct
{
   uint32_t begin, end;
   uint32_t part_size;
   uint8_t classifications;
   uint8_t classbook;
   uint8_t **classdata;
   int16_t (*residue_books)[8];
} Residue;
typedef struct
{
   uint8_t magnitude;
   uint8_t angle;
   uint8_t mux;
} MappingChannel;
typedef struct
{
   uint16_t coupling_steps;
   MappingChannel *chan;
   uint8_t submaps;
   uint8_t submap_floor[15];
   uint8_t submap_residue[15];
} Mapping;
typedef struct
{
   uint8_t blockflag;
   uint8_t mapping;
   uint16_t windowtype;
   uint16_t transformtype;
} Mode;
typedef struct
{
   uint32_t goal_crc;
   int bytes_left;
   uint32_t crc_so_far;
   int bytes_done;
   uint32_t sample_loc;
} CRCscan;
typedef struct
{
   uint32_t page_start, page_end;
   uint32_t last_decoded_sample;
} ProbedPage;
struct stb_vorbis
{
   unsigned int sample_rate;
   int channels;
   unsigned int setup_memory_required;
   unsigned int temp_memory_required;
   unsigned int setup_temp_memory_required;
   char *vendor;
   int comment_list_length;
   char **comment_list;
#ifndef STB_VORBIS_NO_STDIO
   FILE *f;
   uint32_t f_start;
   int close_on_free;
#endif
   uint8_t *stream;
   uint8_t *stream_start;
   uint8_t *stream_end;
   uint32_t stream_len;
   uint8_t push_mode;
   uint32_t first_audio_page_offset;
   ProbedPage p_first, p_last;
   stb_vorbis_alloc alloc;
   int setup_offset;
   int temp_offset;
   int eof;
   enum STBVorbisError error;
   int blocksize[2];
   int blocksize_0, blocksize_1;
   int codebook_count;
   Codebook *codebooks;
   int floor_count;
   uint16_t floor_types[64];
   Floor *floor_config;
   int residue_count;
   uint16_t residue_types[64];
   Residue *residue_config;
   int mapping_count;
   Mapping *mapping;
   int mode_count;
   Mode mode_config[64];
   uint32_t total_samples;
   float *channel_buffers[STB_VORBIS_MAX_CHANNELS];
   float *outputs [STB_VORBIS_MAX_CHANNELS];
   float *previous_window[STB_VORBIS_MAX_CHANNELS];
   int previous_length;
   #ifndef STB_VORBIS_NO_DEFER_FLOOR
   int16_t *finalY[STB_VORBIS_MAX_CHANNELS];
   #else
   float *floor_buffers[STB_VORBIS_MAX_CHANNELS];
   #endif
   uint32_t current_loc;
   int current_loc_valid;
   float *A[2],*B[2],*C[2];
   float *window[2];
   uint16_t *bit_reverse[2];
   uint32_t serial;
   int last_page;
   int segment_count;
   uint8_t segments[255];
   uint8_t page_flag;
   uint8_t bytes_in_seg;
   uint8_t first_decode;
   int next_seg;
   int last_seg;
   int last_seg_which;
   uint32_t acc;
   int valid_bits;
   int packet_bytes;
   int end_seg_with_known_loc;
   uint32_t known_loc_for_packet;
   int discard_samples_deferred;
   uint32_t samples_output;
   int page_crc_tests;
#ifndef STB_VORBIS_NO_PUSHDATA_API
   CRCscan scan[STB_VORBIS_PUSHDATA_CRC_COUNT];
#endif
   int channel_buffer_start;
   int channel_buffer_end;
};
#if defined(STB_VORBIS_NO_PUSHDATA_API)
   #define IS_PUSH_MODE(f) FALSE
#elif defined(STB_VORBIS_NO_PULLDATA_API)
   #define IS_PUSH_MODE(f) TRUE
#else
   #define IS_PUSH_MODE(f) ((f)->push_mode)
#endif
typedef struct stb_vorbis vorb;
static int error(vorb *f, enum STBVorbisError e)
{
   f->error = e;
   if (!f->eof && e != VORBIS_need_more_data) {
      f->error=e;
   }
   return 0;
}
#define array_size_required(count,size) (count*(sizeof(void *)+(size)))
#define temp_alloc(f,size) (f->alloc.alloc_buffer ? setup_temp_malloc(f,size) : alloca(size))
#define temp_free(f,p) (void)0
#define temp_alloc_save(f) ((f)->temp_offset)
#define temp_alloc_restore(f,p) ((f)->temp_offset = (p))
#define temp_block_array(f,count,size) make_block_array(temp_alloc(f,array_size_required(count,size)), count, size)
static void *make_block_array(void *mem, int count, int size)
{
   int i;
   void ** p = (void **) mem;
   char *q = (char *) (p + count);
   for (i=0; i < count; ++i) {
      p[i] = q;
      q += size;
   }
   return p;
}
static void *setup_malloc(vorb *f, int sz)
{
   sz = (sz+7) & ~7;
   f->setup_memory_required += sz;
   if (f->alloc.alloc_buffer) {
      void *p = (char *) f->alloc.alloc_buffer + f->setup_offset;
      if (f->setup_offset + sz > f->temp_offset) return NULL;
      f->setup_offset += sz;
      return p;
   }
   return sz ? malloc(sz) : NULL;
}
static void setup_free(vorb *f, void *p)
{
   if (f->alloc.alloc_buffer) return;
   free(p);
}
static void *setup_temp_malloc(vorb *f, int sz)
{
   sz = (sz+7) & ~7;
   if (f->alloc.alloc_buffer) {
      if (f->temp_offset - sz < f->setup_offset) return NULL;
      f->temp_offset -= sz;
      return (char *) f->alloc.alloc_buffer + f->temp_offset;
   }
   return malloc(sz);
}
static void setup_temp_free(vorb *f, void *p, int sz)
{
   if (f->alloc.alloc_buffer) {
      f->temp_offset += (sz+7)&~7;
      return;
   }
   free(p);
}
#define CRC32_POLY 0x04c11db7
static uint32_t crc_table[256];
static void crc32_init(void)
{
   int i,j;
   uint32_t s;
   for(i=0; i < 256; i++) {
      for (s=(uint32_t) i << 24, j=0; j < 8; ++j)
         s = (s << 1) ^ (s >= (1U<<31) ? CRC32_POLY : 0);
      crc_table[i] = s;
   }
}
static __forceinline uint32_t crc32_update(uint32_t crc, uint8_t byte)
{
   return (crc << 8) ^ crc_table[byte ^ (crc >> 24)];
}
static unsigned int bit_reverse(unsigned int n)
{
  n = ((n & 0xAAAAAAAA) >> 1) | ((n & 0x55555555) << 1);
  n = ((n & 0xCCCCCCCC) >> 2) | ((n & 0x33333333) << 2);
  n = ((n & 0xF0F0F0F0) >> 4) | ((n & 0x0F0F0F0F) << 4);
  n = ((n & 0xFF00FF00) >> 8) | ((n & 0x00FF00FF) << 8);
  return (n >> 16) | (n << 16);
}
static float square(float x)
{
   return x*x;
}
static int ilog(int32_t n)
{
   static signed char log2_4[16] = { 0,1,2,2,3,3,3,3,4,4,4,4,4,4,4,4 };
   if (n < 0) return 0;
   if (n < (1 << 14))
        if (n < (1 << 4)) return 0 + log2_4[n ];
        else if (n < (1 << 9)) return 5 + log2_4[n >> 5];
             else return 10 + log2_4[n >> 10];
   else if (n < (1 << 24))
             if (n < (1 << 19)) return 15 + log2_4[n >> 15];
             else return 20 + log2_4[n >> 20];
        else if (n < (1 << 29)) return 25 + log2_4[n >> 25];
             else return 30 + log2_4[n >> 30];
}
#ifndef M_PI
  #define M_PI 3.14159265358979323846264f
#endif
#define NO_CODE 255
static float float32_unpack(uint32_t x)
{
   uint32_t mantissa = x & 0x1fffff;
   uint32_t sign = x & 0x80000000;
   uint32_t exp = (x & 0x7fe00000) >> 21;
   double res = sign ? -(double)mantissa : (double)mantissa;
   return (float) ldexp((float)res, (int)exp-788);
}
static void add_entry(Codebook *c, uint32_t huff_code, int symbol, int count, int len, uint32_t *values)
{
   if (!c->sparse) {
      c->codewords [symbol] = huff_code;
   } else {
      c->codewords [count] = huff_code;
      c->codeword_lengths[count] = len;
      values [count] = symbol;
   }
}
static int compute_codewords(Codebook *c, uint8_t *len, int n, uint32_t *values)
{
   int i,k,m=0;
   uint32_t available[32];
   memset(available, 0, sizeof(available));
   for (k=0; k < n; ++k) if (len[k] < NO_CODE) break;
   if (k == n) { assert(c->sorted_entries == 0); return TRUE; }
   assert(len[k] < 32);
   add_entry(c, 0, k, m++, len[k], values);
   for (i=1; i <= len[k]; ++i)
      available[i] = 1U << (32-i);
   for (i=k+1; i < n; ++i) {
      uint32_t res;
      int z = len[i], y;
      if (z == NO_CODE) continue;
      assert(z < 32);
      while (z > 0 && !available[z]) --z;
      if (z == 0) { return FALSE; }
      res = available[z];
      available[z] = 0;
      add_entry(c, bit_reverse(res), i, m++, len[i], values);
      if (z != len[i]) {
         for (y=len[i]; y > z; --y) {
            assert(available[y] == 0);
            available[y] = res + (1 << (32-y));
         }
      }
   }
   return TRUE;
}
static void compute_accelerated_huffman(Codebook *c)
{
   int i, len;
   for (i=0; i < FAST_HUFFMAN_TABLE_SIZE; ++i)
      c->fast_huffman[i] = -1;
   len = c->sparse ? c->sorted_entries : c->entries;
   #ifdef STB_VORBIS_FAST_HUFFMAN_SHORT
   if (len > 32767) len = 32767;
   #endif
   for (i=0; i < len; ++i) {
      if (c->codeword_lengths[i] <= STB_VORBIS_FAST_HUFFMAN_LENGTH) {
         uint32_t z = c->sparse ? bit_reverse(c->sorted_codewords[i]) : c->codewords[i];
         while (z < FAST_HUFFMAN_TABLE_SIZE) {
             c->fast_huffman[z] = i;
             z += 1 << c->codeword_lengths[i];
         }
      }
   }
}
#ifdef _MSC_VER
#define STBV_CDECL __cdecl
#else
#define STBV_CDECL
#endif
static int STBV_CDECL uint32_compare(const void *p, const void *q)
{
   uint32_t x = * (uint32_t *) p;
   uint32_t y = * (uint32_t *) q;
   return x < y ? -1 : x > y;
}
static int include_in_sort(Codebook *c, uint8_t len)
{
   if (c->sparse) { assert(len != NO_CODE); return TRUE; }
   if (len == NO_CODE) return FALSE;
   if (len > STB_VORBIS_FAST_HUFFMAN_LENGTH) return TRUE;
   return FALSE;
}
static void compute_sorted_huffman(Codebook *c, uint8_t *lengths, uint32_t *values)
{
   int i, len;
   if (!c->sparse) {
      int k = 0;
      for (i=0; i < c->entries; ++i)
         if (include_in_sort(c, lengths[i]))
            c->sorted_codewords[k++] = bit_reverse(c->codewords[i]);
      assert(k == c->sorted_entries);
   } else {
      for (i=0; i < c->sorted_entries; ++i)
         c->sorted_codewords[i] = bit_reverse(c->codewords[i]);
   }
   qsort(c->sorted_codewords, c->sorted_entries, sizeof(c->sorted_codewords[0]), uint32_compare);
   c->sorted_codewords[c->sorted_entries] = 0xffffffff;
   len = c->sparse ? c->sorted_entries : c->entries;
   for (i=0; i < len; ++i) {
      int huff_len = c->sparse ? lengths[values[i]] : lengths[i];
      if (include_in_sort(c,huff_len)) {
         uint32_t code = bit_reverse(c->codewords[i]);
         int x=0, n=c->sorted_entries;
         while (n > 1) {
            int m = x + (n >> 1);
            if (c->sorted_codewords[m] <= code) {
               x = m;
               n -= (n>>1);
            } else {
               n >>= 1;
            }
         }
         assert(c->sorted_codewords[x] == code);
         if (c->sparse) {
            c->sorted_values[x] = values[i];
            c->codeword_lengths[x] = huff_len;
         } else {
            c->sorted_values[x] = i;
         }
      }
   }
}
static int vorbis_validate(uint8_t *data)
{
   static uint8_t vorbis[6] = { 'v', 'o', 'r', 'b', 'i', 's' };
   return memcmp(data, vorbis, 6) == 0;
}
static int lookup1_values(int entries, int dim)
{
   int r = (int) floor(exp((float) log((float) entries) / dim));
   if ((int) floor(pow((float) r+1, dim)) <= entries)
      ++r;
   if (pow((float) r+1, dim) <= entries)
      return -1;
   if ((int) floor(pow((float) r, dim)) > entries)
      return -1;
   return r;
}
static void compute_twiddle_factors(int n, float *A, float *B, float *C)
{
   int n4 = n >> 2, n8 = n >> 3;
   int k,k2;
   for (k=k2=0; k < n4; ++k,k2+=2) {
      A[k2 ] = (float) cos(4*k*M_PI/n);
      A[k2+1] = (float) -sin(4*k*M_PI/n);
      B[k2 ] = (float) cos((k2+1)*M_PI/n/2) * 0.5f;
      B[k2+1] = (float) sin((k2+1)*M_PI/n/2) * 0.5f;
   }
   for (k=k2=0; k < n8; ++k,k2+=2) {
      C[k2 ] = (float) cos(2*(k2+1)*M_PI/n);
      C[k2+1] = (float) -sin(2*(k2+1)*M_PI/n);
   }
}
static void compute_window(int n, float *window)
{
   int n2 = n >> 1, i;
   for (i=0; i < n2; ++i)
      window[i] = (float) sin(0.5 * M_PI * square((float) sin((i - 0 + 0.5) / n2 * 0.5 * M_PI)));
}
static void compute_bitreverse(int n, uint16_t *rev)
{
   int ld = ilog(n) - 1;
   int i, n8 = n >> 3;
   for (i=0; i < n8; ++i)
      rev[i] = (bit_reverse(i) >> (32-ld+3)) << 2;
}
static int init_blocksize(vorb *f, int b, int n)
{
   int n2 = n >> 1, n4 = n >> 2, n8 = n >> 3;
   f->A[b] = (float *) setup_malloc(f, sizeof(float) * n2);
   f->B[b] = (float *) setup_malloc(f, sizeof(float) * n2);
   f->C[b] = (float *) setup_malloc(f, sizeof(float) * n4);
   if (!f->A[b] || !f->B[b] || !f->C[b]) return error(f, VORBIS_outofmem);
   compute_twiddle_factors(n, f->A[b], f->B[b], f->C[b]);
   f->window[b] = (float *) setup_malloc(f, sizeof(float) * n2);
   if (!f->window[b]) return error(f, VORBIS_outofmem);
   compute_window(n, f->window[b]);
   f->bit_reverse[b] = (uint16_t *) setup_malloc(f, sizeof(uint16_t) * n8);
   if (!f->bit_reverse[b]) return error(f, VORBIS_outofmem);
   compute_bitreverse(n, f->bit_reverse[b]);
   return TRUE;
}
static void neighbors(uint16_t *x, int n, int *plow, int *phigh)
{
   int low = -1;
   int high = 65536;
   int i;
   for (i=0; i < n; ++i) {
      if (x[i] > low && x[i] < x[n]) { *plow = i; low = x[i]; }
      if (x[i] < high && x[i] > x[n]) { *phigh = i; high = x[i]; }
   }
}
typedef struct
{
   uint16_t x,id;
} stbv__floor_ordering;
static int STBV_CDECL point_compare(const void *p, const void *q)
{
   stbv__floor_ordering *a = (stbv__floor_ordering *) p;
   stbv__floor_ordering *b = (stbv__floor_ordering *) q;
   return a->x < b->x ? -1 : a->x > b->x;
}
#if defined(STB_VORBIS_NO_STDIO)
   #define USE_MEMORY(z) TRUE
#else
   #define USE_MEMORY(z) ((z)->stream)
#endif
static uint8_t get8(vorb *z)
{
   if (USE_MEMORY(z)) {
      if (z->stream >= z->stream_end) { z->eof = TRUE; return 0; }
      return *z->stream++;
   }
   #ifndef STB_VORBIS_NO_STDIO
   {
   int c = fgetc(z->f);
   if (feof(z->f)) { z->eof = TRUE; return 0; }
   return c;
   }
   #endif
}
static uint32_t get32(vorb *f)
{
   uint32_t x;
   x = get8(f);
   x += get8(f) << 8;
   x += get8(f) << 16;
   x += (uint32_t) get8(f) << 24;
   return x;
}
static int getn(vorb *z, uint8_t *data, int n)
{
   if (USE_MEMORY(z)) {
      if (z->stream+n > z->stream_end) { z->eof = 1; return 0; }
      memcpy(data, z->stream, n);
      z->stream += n;
      return 1;
   }
   #ifndef STB_VORBIS_NO_STDIO
   if (fread(data, n, 1, z->f) == 1)
      return 1;
   else {
      z->eof = 1;
      return 0;
   }
   #endif
}
static void skip(vorb *z, int n)
{
   if (USE_MEMORY(z)) {
      z->stream += n;
      if (z->stream >= z->stream_end) z->eof = 1;
      return;
   }
   #ifndef STB_VORBIS_NO_STDIO
   {
      long x = ftell(z->f);
      fseek(z->f, x+n, SEEK_SET);
   }
   #endif
}
static int set_file_offset(stb_vorbis *f, unsigned int loc)
{
   #ifndef STB_VORBIS_NO_PUSHDATA_API
   if (f->push_mode) return 0;
   #endif
   f->eof = 0;
   if (USE_MEMORY(f)) {
      if (f->stream_start + loc >= f->stream_end || f->stream_start + loc < f->stream_start) {
         f->stream = f->stream_end;
         f->eof = 1;
         return 0;
      } else {
         f->stream = f->stream_start + loc;
         return 1;
      }
   }
   #ifndef STB_VORBIS_NO_STDIO
   if (loc + f->f_start < loc || loc >= 0x80000000) {
      loc = 0x7fffffff;
      f->eof = 1;
   } else {
      loc += f->f_start;
   }
   if (!fseek(f->f, loc, SEEK_SET))
      return 1;
   f->eof = 1;
   fseek(f->f, f->f_start, SEEK_END);
   return 0;
   #endif
}
static uint8_t ogg_page_header[4] = { 0x4f, 0x67, 0x67, 0x53 };
static int capture_pattern(vorb *f)
{
   if (0x4f != get8(f)) return FALSE;
   if (0x67 != get8(f)) return FALSE;
   if (0x67 != get8(f)) return FALSE;
   if (0x53 != get8(f)) return FALSE;
   return TRUE;
}
#define PAGEFLAG_continued_packet 1
#define PAGEFLAG_first_page 2
#define PAGEFLAG_last_page 4
static int start_page_no_capturepattern(vorb *f)
{
   uint32_t loc0,loc1,n;
   if (f->first_decode && !IS_PUSH_MODE(f)) {
      f->p_first.page_start = stb_vorbis_get_file_offset(f) - 4;
   }
   if (0 != get8(f)) return error(f, VORBIS_invalid_stream_structure_version);
   f->page_flag = get8(f);
   loc0 = get32(f);
   loc1 = get32(f);
   get32(f);
   n = get32(f);
   f->last_page = n;
   get32(f);
   f->segment_count = get8(f);
   if (!getn(f, f->segments, f->segment_count))
      return error(f, VORBIS_unexpected_eof);
   f->end_seg_with_known_loc = -2;
   if (loc0 != ~0U || loc1 != ~0U) {
      int i;
      for (i=f->segment_count-1; i >= 0; --i)
         if (f->segments[i] < 255)
            break;
      if (i >= 0) {
         f->end_seg_with_known_loc = i;
         f->known_loc_for_packet = loc0;
      }
   }
   if (f->first_decode) {
      int i,len;
      len = 0;
      for (i=0; i < f->segment_count; ++i)
         len += f->segments[i];
      len += 27 + f->segment_count;
      f->p_first.page_end = f->p_first.page_start + len;
      f->p_first.last_decoded_sample = loc0;
   }
   f->next_seg = 0;
   return TRUE;
}
static int start_page(vorb *f)
{
   if (!capture_pattern(f)) return error(f, VORBIS_missing_capture_pattern);
   return start_page_no_capturepattern(f);
}
static int start_packet(vorb *f)
{
   while (f->next_seg == -1) {
      if (!start_page(f)) return FALSE;
      if (f->page_flag & PAGEFLAG_continued_packet)
         return error(f, VORBIS_continued_packet_flag_invalid);
   }
   f->last_seg = FALSE;
   f->valid_bits = 0;
   f->packet_bytes = 0;
   f->bytes_in_seg = 0;
   return TRUE;
}
static int maybe_start_packet(vorb *f)
{
   if (f->next_seg == -1) {
      int x = get8(f);
      if (f->eof) return FALSE;
      if (0x4f != x ) return error(f, VORBIS_missing_capture_pattern);
      if (0x67 != get8(f)) return error(f, VORBIS_missing_capture_pattern);
      if (0x67 != get8(f)) return error(f, VORBIS_missing_capture_pattern);
      if (0x53 != get8(f)) return error(f, VORBIS_missing_capture_pattern);
      if (!start_page_no_capturepattern(f)) return FALSE;
      if (f->page_flag & PAGEFLAG_continued_packet) {
         f->last_seg = FALSE;
         f->bytes_in_seg = 0;
         return error(f, VORBIS_continued_packet_flag_invalid);
      }
   }
   return start_packet(f);
}
static int next_segment(vorb *f)
{
   int len;
   if (f->last_seg) return 0;
   if (f->next_seg == -1) {
      f->last_seg_which = f->segment_count-1;
      if (!start_page(f)) { f->last_seg = 1; return 0; }
      if (!(f->page_flag & PAGEFLAG_continued_packet)) return error(f, VORBIS_continued_packet_flag_invalid);
   }
   len = f->segments[f->next_seg++];
   if (len < 255) {
      f->last_seg = TRUE;
      f->last_seg_which = f->next_seg-1;
   }
   if (f->next_seg >= f->segment_count)
      f->next_seg = -1;
   assert(f->bytes_in_seg == 0);
   f->bytes_in_seg = len;
   return len;
}
#define EOP (-1)
#define INVALID_BITS (-1)
static int get8_packet_raw(vorb *f)
{
   if (!f->bytes_in_seg) {
      if (f->last_seg) return EOP;
      else if (!next_segment(f)) return EOP;
   }
   assert(f->bytes_in_seg > 0);
   --f->bytes_in_seg;
   ++f->packet_bytes;
   return get8(f);
}
static int get8_packet(vorb *f)
{
   int x = get8_packet_raw(f);
   f->valid_bits = 0;
   return x;
}
static int get32_packet(vorb *f)
{
   uint32_t x;
   x = get8_packet(f);
   x += get8_packet(f) << 8;
   x += get8_packet(f) << 16;
   x += (uint32_t) get8_packet(f) << 24;
   return x;
}
static void flush_packet(vorb *f)
{
   while (get8_packet_raw(f) != EOP);
}
static uint32_t get_bits(vorb *f, int n)
{
   uint32_t z;
   if (f->valid_bits < 0) return 0;
   if (f->valid_bits < n) {
      if (n > 24) {
         z = get_bits(f, 24);
         z += get_bits(f, n-24) << 24;
         return z;
      }
      if (f->valid_bits == 0) f->acc = 0;
      while (f->valid_bits < n) {
         int z = get8_packet_raw(f);
         if (z == EOP) {
            f->valid_bits = INVALID_BITS;
            return 0;
         }
         f->acc += z << f->valid_bits;
         f->valid_bits += 8;
      }
   }
   assert(f->valid_bits >= n);
   z = f->acc & ((1 << n)-1);
   f->acc >>= n;
   f->valid_bits -= n;
   return z;
}
static __forceinline void prep_huffman(vorb *f)
{
   if (f->valid_bits <= 24) {
      if (f->valid_bits == 0) f->acc = 0;
      do {
         int z;
         if (f->last_seg && !f->bytes_in_seg) return;
         z = get8_packet_raw(f);
         if (z == EOP) return;
         f->acc += (unsigned) z << f->valid_bits;
         f->valid_bits += 8;
      } while (f->valid_bits <= 24);
   }
}
enum
{
   VORBIS_packet_id = 1,
   VORBIS_packet_comment = 3,
   VORBIS_packet_setup = 5
};
static int codebook_decode_scalar_raw(vorb *f, Codebook *c)
{
   int i;
   prep_huffman(f);
   if (c->codewords == NULL && c->sorted_codewords == NULL)
      return -1;
   if (c->entries > 8 ? c->sorted_codewords!=NULL : !c->codewords) {
      uint32_t code = bit_reverse(f->acc);
      int x=0, n=c->sorted_entries, len;
      while (n > 1) {
         int m = x + (n >> 1);
         if (c->sorted_codewords[m] <= code) {
            x = m;
            n -= (n>>1);
         } else {
            n >>= 1;
         }
      }
      if (!c->sparse) x = c->sorted_values[x];
      len = c->codeword_lengths[x];
      if (f->valid_bits >= len) {
         f->acc >>= len;
         f->valid_bits -= len;
         return x;
      }
      f->valid_bits = 0;
      return -1;
   }
   assert(!c->sparse);
   for (i=0; i < c->entries; ++i) {
      if (c->codeword_lengths[i] == NO_CODE) continue;
      if (c->codewords[i] == (f->acc & ((1 << c->codeword_lengths[i])-1))) {
         if (f->valid_bits >= c->codeword_lengths[i]) {
            f->acc >>= c->codeword_lengths[i];
            f->valid_bits -= c->codeword_lengths[i];
            return i;
         }
         f->valid_bits = 0;
         return -1;
      }
   }
   error(f, VORBIS_invalid_stream);
   f->valid_bits = 0;
   return -1;
}
#ifndef STB_VORBIS_NO_INLINE_DECODE
#define DECODE_RAW(var, f,c) \
   if (f->valid_bits < STB_VORBIS_FAST_HUFFMAN_LENGTH) \
      prep_huffman(f); \
   var = f->acc & FAST_HUFFMAN_TABLE_MASK; \
   var = c->fast_huffman[var]; \
   if (var >= 0) { \
      int n = c->codeword_lengths[var]; \
      f->acc >>= n; \
      f->valid_bits -= n; \
      if (f->valid_bits < 0) { f->valid_bits = 0; var = -1; } \
   } else { \
      var = codebook_decode_scalar_raw(f,c); \
   }
#else
static int codebook_decode_scalar(vorb *f, Codebook *c)
{
   int i;
   if (f->valid_bits < STB_VORBIS_FAST_HUFFMAN_LENGTH)
      prep_huffman(f);
   i = f->acc & FAST_HUFFMAN_TABLE_MASK;
   i = c->fast_huffman[i];
   if (i >= 0) {
      f->acc >>= c->codeword_lengths[i];
      f->valid_bits -= c->codeword_lengths[i];
      if (f->valid_bits < 0) { f->valid_bits = 0; return -1; }
      return i;
   }
   return codebook_decode_scalar_raw(f,c);
}
#define DECODE_RAW(var,f,c) var = codebook_decode_scalar(f,c);
#endif
#define DECODE(var,f,c) \
   DECODE_RAW(var,f,c) \
   if (c->sparse) var = c->sorted_values[var];
#ifndef STB_VORBIS_DIVIDES_IN_CODEBOOK
  #define DECODE_VQ(var,f,c) DECODE_RAW(var,f,c)
#else
  #define DECODE_VQ(var,f,c) DECODE(var,f,c)
#endif
#define CODEBOOK_ELEMENT(c,off) (c->multiplicands[off])
#define CODEBOOK_ELEMENT_FAST(c,off) (c->multiplicands[off])
#define CODEBOOK_ELEMENT_BASE(c) (0)
static int codebook_decode_start(vorb *f, Codebook *c)
{
   int z = -1;
   if (c->lookup_type == 0)
      error(f, VORBIS_invalid_stream);
   else {
      DECODE_VQ(z,f,c);
      if (c->sparse) assert(z < c->sorted_entries);
      if (z < 0) {
         if (!f->bytes_in_seg)
            if (f->last_seg)
               return z;
         error(f, VORBIS_invalid_stream);
      }
   }
   return z;
}
static int codebook_decode(vorb *f, Codebook *c, float *output, int len)
{
   int i,z = codebook_decode_start(f,c);
   if (z < 0) return FALSE;
   if (len > c->dimensions) len = c->dimensions;
#ifdef STB_VORBIS_DIVIDES_IN_CODEBOOK
   if (c->lookup_type == 1) {
      float last = CODEBOOK_ELEMENT_BASE(c);
      int div = 1;
      for (i=0; i < len; ++i) {
         int off = (z / div) % c->lookup_values;
         float val = CODEBOOK_ELEMENT_FAST(c,off) + last;
         output[i] += val;
         if (c->sequence_p) last = val + c->minimum_value;
         div *= c->lookup_values;
      }
      return TRUE;
   }
#endif
   z *= c->dimensions;
   if (c->sequence_p) {
      float last = CODEBOOK_ELEMENT_BASE(c);
      for (i=0; i < len; ++i) {
         float val = CODEBOOK_ELEMENT_FAST(c,z+i) + last;
         output[i] += val;
         last = val + c->minimum_value;
      }
   } else {
      float last = CODEBOOK_ELEMENT_BASE(c);
      for (i=0; i < len; ++i) {
         output[i] += CODEBOOK_ELEMENT_FAST(c,z+i) + last;
      }
   }
   return TRUE;
}
static int codebook_decode_step(vorb *f, Codebook *c, float *output, int len, int step)
{
   int i,z = codebook_decode_start(f,c);
   float last = CODEBOOK_ELEMENT_BASE(c);
   if (z < 0) return FALSE;
   if (len > c->dimensions) len = c->dimensions;
#ifdef STB_VORBIS_DIVIDES_IN_CODEBOOK
   if (c->lookup_type == 1) {
      int div = 1;
      for (i=0; i < len; ++i) {
         int off = (z / div) % c->lookup_values;
         float val = CODEBOOK_ELEMENT_FAST(c,off) + last;
         output[i*step] += val;
         if (c->sequence_p) last = val;
         div *= c->lookup_values;
      }
      return TRUE;
   }
#endif
   z *= c->dimensions;
   for (i=0; i < len; ++i) {
      float val = CODEBOOK_ELEMENT_FAST(c,z+i) + last;
      output[i*step] += val;
      if (c->sequence_p) last = val;
   }
   return TRUE;
}
static int codebook_decode_deinterleave_repeat(vorb *f, Codebook *c, float **outputs, int ch, int *c_inter_p, int *p_inter_p, int len, int total_decode)
{
   int c_inter = *c_inter_p;
   int p_inter = *p_inter_p;
   int i,z, effective = c->dimensions;
   if (c->lookup_type == 0) return error(f, VORBIS_invalid_stream);
   while (total_decode > 0) {
      float last = CODEBOOK_ELEMENT_BASE(c);
      DECODE_VQ(z,f,c);
      #ifndef STB_VORBIS_DIVIDES_IN_CODEBOOK
      assert(!c->sparse || z < c->sorted_entries);
      #endif
      if (z < 0) {
         if (!f->bytes_in_seg)
            if (f->last_seg) return FALSE;
         return error(f, VORBIS_invalid_stream);
      }
      if (c_inter + p_inter*ch + effective > len * ch) {
         effective = len*ch - (p_inter*ch - c_inter);
      }
   #ifdef STB_VORBIS_DIVIDES_IN_CODEBOOK
      if (c->lookup_type == 1) {
         int div = 1;
         for (i=0; i < effective; ++i) {
            int off = (z / div) % c->lookup_values;
            float val = CODEBOOK_ELEMENT_FAST(c,off) + last;
            if (outputs[c_inter])
               outputs[c_inter][p_inter] += val;
            if (++c_inter == ch) { c_inter = 0; ++p_inter; }
            if (c->sequence_p) last = val;
            div *= c->lookup_values;
         }
      } else
   #endif
      {
         z *= c->dimensions;
         if (c->sequence_p) {
            for (i=0; i < effective; ++i) {
               float val = CODEBOOK_ELEMENT_FAST(c,z+i) + last;
               if (outputs[c_inter])
                  outputs[c_inter][p_inter] += val;
               if (++c_inter == ch) { c_inter = 0; ++p_inter; }
               last = val;
            }
         } else {
            for (i=0; i < effective; ++i) {
               float val = CODEBOOK_ELEMENT_FAST(c,z+i) + last;
               if (outputs[c_inter])
                  outputs[c_inter][p_inter] += val;
               if (++c_inter == ch) { c_inter = 0; ++p_inter; }
            }
         }
      }
      total_decode -= effective;
   }
   *c_inter_p = c_inter;
   *p_inter_p = p_inter;
   return TRUE;
}
static int predict_point(int x, int x0, int x1, int y0, int y1)
{
   int dy = y1 - y0;
   int adx = x1 - x0;
   int err = abs(dy) * (x - x0);
   int off = err / adx;
   return dy < 0 ? y0 - off : y0 + off;
}
static float inverse_db_table[256] =
{
  1.0649863e-07f, 1.1341951e-07f, 1.2079015e-07f, 1.2863978e-07f,
  1.3699951e-07f, 1.4590251e-07f, 1.5538408e-07f, 1.6548181e-07f,
  1.7623575e-07f, 1.8768855e-07f, 1.9988561e-07f, 2.1287530e-07f,
  2.2670913e-07f, 2.4144197e-07f, 2.5713223e-07f, 2.7384213e-07f,
  2.9163793e-07f, 3.1059021e-07f, 3.3077411e-07f, 3.5226968e-07f,
  3.7516214e-07f, 3.9954229e-07f, 4.2550680e-07f, 4.5315863e-07f,
  4.8260743e-07f, 5.1396998e-07f, 5.4737065e-07f, 5.8294187e-07f,
  6.2082472e-07f, 6.6116941e-07f, 7.0413592e-07f, 7.4989464e-07f,
  7.9862701e-07f, 8.5052630e-07f, 9.0579828e-07f, 9.6466216e-07f,
  1.0273513e-06f, 1.0941144e-06f, 1.1652161e-06f, 1.2409384e-06f,
  1.3215816e-06f, 1.4074654e-06f, 1.4989305e-06f, 1.5963394e-06f,
  1.7000785e-06f, 1.8105592e-06f, 1.9282195e-06f, 2.0535261e-06f,
  2.1869758e-06f, 2.3290978e-06f, 2.4804557e-06f, 2.6416497e-06f,
  2.8133190e-06f, 2.9961443e-06f, 3.1908506e-06f, 3.3982101e-06f,
  3.6190449e-06f, 3.8542308e-06f, 4.1047004e-06f, 4.3714470e-06f,
  4.6555282e-06f, 4.9580707e-06f, 5.2802740e-06f, 5.6234160e-06f,
  5.9888572e-06f, 6.3780469e-06f, 6.7925283e-06f, 7.2339451e-06f,
  7.7040476e-06f, 8.2047000e-06f, 8.7378876e-06f, 9.3057248e-06f,
  9.9104632e-06f, 1.0554501e-05f, 1.1240392e-05f, 1.1970856e-05f,
  1.2748789e-05f, 1.3577278e-05f, 1.4459606e-05f, 1.5399272e-05f,
  1.6400004e-05f, 1.7465768e-05f, 1.8600792e-05f, 1.9809576e-05f,
  2.1096914e-05f, 2.2467911e-05f, 2.3928002e-05f, 2.5482978e-05f,
  2.7139006e-05f, 2.8902651e-05f, 3.0780908e-05f, 3.2781225e-05f,
  3.4911534e-05f, 3.7180282e-05f, 3.9596466e-05f, 4.2169667e-05f,
  4.4910090e-05f, 4.7828601e-05f, 5.0936773e-05f, 5.4246931e-05f,
  5.7772202e-05f, 6.1526565e-05f, 6.5524908e-05f, 6.9783085e-05f,
  7.4317983e-05f, 7.9147585e-05f, 8.4291040e-05f, 8.9768747e-05f,
  9.5602426e-05f, 0.00010181521f, 0.00010843174f, 0.00011547824f,
  0.00012298267f, 0.00013097477f, 0.00013948625f, 0.00014855085f,
  0.00015820453f, 0.00016848555f, 0.00017943469f, 0.00019109536f,
  0.00020351382f, 0.00021673929f, 0.00023082423f, 0.00024582449f,
  0.00026179955f, 0.00027881276f, 0.00029693158f, 0.00031622787f,
  0.00033677814f, 0.00035866388f, 0.00038197188f, 0.00040679456f,
  0.00043323036f, 0.00046138411f, 0.00049136745f, 0.00052329927f,
  0.00055730621f, 0.00059352311f, 0.00063209358f, 0.00067317058f,
  0.00071691700f, 0.00076350630f, 0.00081312324f, 0.00086596457f,
  0.00092223983f, 0.00098217216f, 0.0010459992f, 0.0011139742f,
  0.0011863665f, 0.0012634633f, 0.0013455702f, 0.0014330129f,
  0.0015261382f, 0.0016253153f, 0.0017309374f, 0.0018434235f,
  0.0019632195f, 0.0020908006f, 0.0022266726f, 0.0023713743f,
  0.0025254795f, 0.0026895994f, 0.0028643847f, 0.0030505286f,
  0.0032487691f, 0.0034598925f, 0.0036847358f, 0.0039241906f,
  0.0041792066f, 0.0044507950f, 0.0047400328f, 0.0050480668f,
  0.0053761186f, 0.0057254891f, 0.0060975636f, 0.0064938176f,
  0.0069158225f, 0.0073652516f, 0.0078438871f, 0.0083536271f,
  0.0088964928f, 0.009474637f, 0.010090352f, 0.010746080f,
  0.011444421f, 0.012188144f, 0.012980198f, 0.013823725f,
  0.014722068f, 0.015678791f, 0.016697687f, 0.017782797f,
  0.018938423f, 0.020169149f, 0.021479854f, 0.022875735f,
  0.024362330f, 0.025945531f, 0.027631618f, 0.029427276f,
  0.031339626f, 0.033376252f, 0.035545228f, 0.037855157f,
  0.040315199f, 0.042935108f, 0.045725273f, 0.048696758f,
  0.051861348f, 0.055231591f, 0.058820850f, 0.062643361f,
  0.066714279f, 0.071049749f, 0.075666962f, 0.080584227f,
  0.085821044f, 0.091398179f, 0.097337747f, 0.10366330f,
  0.11039993f, 0.11757434f, 0.12521498f, 0.13335215f,
  0.14201813f, 0.15124727f, 0.16107617f, 0.17154380f,
  0.18269168f, 0.19456402f, 0.20720788f, 0.22067342f,
  0.23501402f, 0.25028656f, 0.26655159f, 0.28387361f,
  0.30232132f, 0.32196786f, 0.34289114f, 0.36517414f,
  0.38890521f, 0.41417847f, 0.44109412f, 0.46975890f,
  0.50028648f, 0.53279791f, 0.56742212f, 0.60429640f,
  0.64356699f, 0.68538959f, 0.72993007f, 0.77736504f,
  0.82788260f, 0.88168307f, 0.9389798f, 1.0f
};
#ifndef STB_VORBIS_NO_DEFER_FLOOR
#define LINE_OP(a,b) a *= b
#else
#define LINE_OP(a,b) a = b
#endif
#ifdef STB_VORBIS_DIVIDE_TABLE
#define DIVTAB_NUMER 32
#define DIVTAB_DENOM 64
int8_t integer_divide_table[DIVTAB_NUMER][DIVTAB_DENOM];
#endif
static __forceinline void draw_line(float *output, int x0, int y0, int x1, int y1, int n)
{
   int dy = y1 - y0;
   int adx = x1 - x0;
   int ady = abs(dy);
   int base;
   int x=x0,y=y0;
   int err = 0;
   int sy;
#ifdef STB_VORBIS_DIVIDE_TABLE
   if (adx < DIVTAB_DENOM && ady < DIVTAB_NUMER) {
      if (dy < 0) {
         base = -integer_divide_table[ady][adx];
         sy = base-1;
      } else {
         base = integer_divide_table[ady][adx];
         sy = base+1;
      }
   } else {
      base = dy / adx;
      if (dy < 0)
         sy = base - 1;
      else
         sy = base+1;
   }
#else
   base = dy / adx;
   if (dy < 0)
      sy = base - 1;
   else
      sy = base+1;
#endif
   ady -= abs(base) * adx;
   if (x1 > n) x1 = n;
   if (x < x1) {
      LINE_OP(output[x], inverse_db_table[y&255]);
      for (++x; x < x1; ++x) {
         err += ady;
         if (err >= adx) {
            err -= adx;
            y += sy;
         } else
            y += base;
         LINE_OP(output[x], inverse_db_table[y&255]);
      }
   }
}
static int residue_decode(vorb *f, Codebook *book, float *target, int offset, int n, int rtype)
{
   int k;
   if (rtype == 0) {
      int step = n / book->dimensions;
      for (k=0; k < step; ++k)
         if (!codebook_decode_step(f, book, target+offset+k, n-offset-k, step))
            return FALSE;
   } else {
      for (k=0; k < n; ) {
         if (!codebook_decode(f, book, target+offset, n-k))
            return FALSE;
         k += book->dimensions;
         offset += book->dimensions;
      }
   }
   return TRUE;
}
static void decode_residue(vorb *f, float *residue_buffers[], int ch, int n, int rn, uint8_t *do_not_decode)
{
   int i,j,pass;
   Residue *r = f->residue_config + rn;
   int rtype = f->residue_types[rn];
   int c = r->classbook;
   int classwords = f->codebooks[c].dimensions;
   unsigned int actual_size = rtype == 2 ? n*2 : n;
   unsigned int limit_r_begin = (r->begin < actual_size ? r->begin : actual_size);
   unsigned int limit_r_end = (r->end < actual_size ? r->end : actual_size);
   int n_read = limit_r_end - limit_r_begin;
   int part_read = n_read / r->part_size;
   int temp_alloc_point = temp_alloc_save(f);
   #ifndef STB_VORBIS_DIVIDES_IN_RESIDUE
   uint8_t ***part_classdata = (uint8_t ***) temp_block_array(f,f->channels, part_read * sizeof(**part_classdata));
   #else
   int **classifications = (int **) temp_block_array(f,f->channels, part_read * sizeof(**classifications));
   #endif
   CHECK(f);
   for (i=0; i < ch; ++i)
      if (!do_not_decode[i])
         memset(residue_buffers[i], 0, sizeof(float) * n);
   if (rtype == 2 && ch != 1) {
      for (j=0; j < ch; ++j)
         if (!do_not_decode[j])
            break;
      if (j == ch)
         goto done;
      for (pass=0; pass < 8; ++pass) {
         int pcount = 0, class_set = 0;
         if (ch == 2) {
            while (pcount < part_read) {
               int z = r->begin + pcount*r->part_size;
               int c_inter = (z & 1), p_inter = z>>1;
               if (pass == 0) {
                  Codebook *c = f->codebooks+r->classbook;
                  int q;
                  DECODE(q,f,c);
                  if (q == EOP) goto done;
                  #ifndef STB_VORBIS_DIVIDES_IN_RESIDUE
                  part_classdata[0][class_set] = r->classdata[q];
                  #else
                  for (i=classwords-1; i >= 0; --i) {
                     classifications[0][i+pcount] = q % r->classifications;
                     q /= r->classifications;
                  }
                  #endif
               }
               for (i=0; i < classwords && pcount < part_read; ++i, ++pcount) {
                  int z = r->begin + pcount*r->part_size;
                  #ifndef STB_VORBIS_DIVIDES_IN_RESIDUE
                  int c = part_classdata[0][class_set][i];
                  #else
                  int c = classifications[0][pcount];
                  #endif
                  int b = r->residue_books[c][pass];
                  if (b >= 0) {
                     Codebook *book = f->codebooks + b;
                     #ifdef STB_VORBIS_DIVIDES_IN_CODEBOOK
                     if (!codebook_decode_deinterleave_repeat(f, book, residue_buffers, ch, &c_inter, &p_inter, n, r->part_size))
                        goto done;
                     #else
                     if (!codebook_decode_deinterleave_repeat(f, book, residue_buffers, ch, &c_inter, &p_inter, n, r->part_size))
                        goto done;
                     #endif
                  } else {
                     z += r->part_size;
                     c_inter = z & 1;
                     p_inter = z >> 1;
                  }
               }
               #ifndef STB_VORBIS_DIVIDES_IN_RESIDUE
               ++class_set;
               #endif
            }
         } else if (ch > 2) {
            while (pcount < part_read) {
               int z = r->begin + pcount*r->part_size;
               int c_inter = z % ch, p_inter = z/ch;
               if (pass == 0) {
                  Codebook *c = f->codebooks+r->classbook;
                  int q;
                  DECODE(q,f,c);
                  if (q == EOP) goto done;
                  #ifndef STB_VORBIS_DIVIDES_IN_RESIDUE
                  part_classdata[0][class_set] = r->classdata[q];
                  #else
                  for (i=classwords-1; i >= 0; --i) {
                     classifications[0][i+pcount] = q % r->classifications;
                     q /= r->classifications;
                  }
                  #endif
               }
               for (i=0; i < classwords && pcount < part_read; ++i, ++pcount) {
                  int z = r->begin + pcount*r->part_size;
                  #ifndef STB_VORBIS_DIVIDES_IN_RESIDUE
                  int c = part_classdata[0][class_set][i];
                  #else
                  int c = classifications[0][pcount];
                  #endif
                  int b = r->residue_books[c][pass];
                  if (b >= 0) {
                     Codebook *book = f->codebooks + b;
                     if (!codebook_decode_deinterleave_repeat(f, book, residue_buffers, ch, &c_inter, &p_inter, n, r->part_size))
                        goto done;
                  } else {
                     z += r->part_size;
                     c_inter = z % ch;
                     p_inter = z / ch;
                  }
               }
               #ifndef STB_VORBIS_DIVIDES_IN_RESIDUE
               ++class_set;
               #endif
            }
         }
      }
      goto done;
   }
   CHECK(f);
   for (pass=0; pass < 8; ++pass) {
      int pcount = 0, class_set=0;
      while (pcount < part_read) {
         if (pass == 0) {
            for (j=0; j < ch; ++j) {
               if (!do_not_decode[j]) {
                  Codebook *c = f->codebooks+r->classbook;
                  int temp;
                  DECODE(temp,f,c);
                  if (temp == EOP) goto done;
                  #ifndef STB_VORBIS_DIVIDES_IN_RESIDUE
                  part_classdata[j][class_set] = r->classdata[temp];
                  #else
                  for (i=classwords-1; i >= 0; --i) {
                     classifications[j][i+pcount] = temp % r->classifications;
                     temp /= r->classifications;
                  }
                  #endif
               }
            }
         }
         for (i=0; i < classwords && pcount < part_read; ++i, ++pcount) {
            for (j=0; j < ch; ++j) {
               if (!do_not_decode[j]) {
                  #ifndef STB_VORBIS_DIVIDES_IN_RESIDUE
                  int c = part_classdata[j][class_set][i];
                  #else
                  int c = classifications[j][pcount];
                  #endif
                  int b = r->residue_books[c][pass];
                  if (b >= 0) {
                     float *target = residue_buffers[j];
                     int offset = r->begin + pcount * r->part_size;
                     int n = r->part_size;
                     Codebook *book = f->codebooks + b;
                     if (!residue_decode(f, book, target, offset, n, rtype))
                        goto done;
                  }
               }
            }
         }
         #ifndef STB_VORBIS_DIVIDES_IN_RESIDUE
         ++class_set;
         #endif
      }
   }
  done:
   CHECK(f);
   #ifndef STB_VORBIS_DIVIDES_IN_RESIDUE
   temp_free(f,part_classdata);
   #else
   temp_free(f,classifications);
   #endif
   temp_alloc_restore(f,temp_alloc_point);
}
#if 0
void inverse_mdct_slow(float *buffer, int n)
{
   int i,j;
   int n2 = n >> 1;
   float *x = (float *) malloc(sizeof(*x) * n2);
   memcpy(x, buffer, sizeof(*x) * n2);
   for (i=0; i < n; ++i) {
      float acc = 0;
      for (j=0; j < n2; ++j)
         acc += x[j] * (float) cos(M_PI / 2 / n * (2 * i + 1 + n/2.0)*(2*j+1));
      buffer[i] = acc;
   }
   free(x);
}
#elif 0
void inverse_mdct_slow(float *buffer, int n, vorb *f, int blocktype)
{
   float mcos[16384];
   int i,j;
   int n2 = n >> 1, nmask = (n << 2) -1;
   float *x = (float *) malloc(sizeof(*x) * n2);
   memcpy(x, buffer, sizeof(*x) * n2);
   for (i=0; i < 4*n; ++i)
      mcos[i] = (float) cos(M_PI / 2 * i / n);
   for (i=0; i < n; ++i) {
      float acc = 0;
      for (j=0; j < n2; ++j)
         acc += x[j] * mcos[(2 * i + 1 + n2)*(2*j+1) & nmask];
      buffer[i] = acc;
   }
   free(x);
}
#elif 0
void dct_iv_slow(float *buffer, int n)
{
   float mcos[16384];
   float x[2048];
   int i,j;
   int n2 = n >> 1, nmask = (n << 3) - 1;
   memcpy(x, buffer, sizeof(*x) * n);
   for (i=0; i < 8*n; ++i)
      mcos[i] = (float) cos(M_PI / 4 * i / n);
   for (i=0; i < n; ++i) {
      float acc = 0;
      for (j=0; j < n; ++j)
         acc += x[j] * mcos[((2 * i + 1)*(2*j+1)) & nmask];
      buffer[i] = acc;
   }
}
void inverse_mdct_slow(float *buffer, int n, vorb *f, int blocktype)
{
   int i, n4 = n >> 2, n2 = n >> 1, n3_4 = n - n4;
   float temp[4096];
   memcpy(temp, buffer, n2 * sizeof(float));
   dct_iv_slow(temp, n2);
   for (i=0; i < n4 ; ++i) buffer[i] = temp[i+n4];
   for ( ; i < n3_4; ++i) buffer[i] = -temp[n3_4 - i - 1];
   for ( ; i < n ; ++i) buffer[i] = -temp[i - n3_4];
}
#endif
#ifndef LIBVORBIS_MDCT
#define LIBVORBIS_MDCT 0
#endif
#if LIBVORBIS_MDCT
typedef struct
{
  int n;
  int log2n;
  float *trig;
  int *bitrev;
  float scale;
} mdct_lookup;
extern void mdct_init(mdct_lookup *lookup, int n);
extern void mdct_clear(mdct_lookup *l);
extern void mdct_backward(mdct_lookup *init, float *in, float *out);
mdct_lookup M1,M2;
void inverse_mdct(float *buffer, int n, vorb *f, int blocktype)
{
   mdct_lookup *M;
   if (M1.n == n) M = &M1;
   else if (M2.n == n) M = &M2;
   else if (M1.n == 0) { mdct_init(&M1, n); M = &M1; }
   else {
      if (M2.n) __asm int 3;
      mdct_init(&M2, n);
      M = &M2;
   }
   mdct_backward(M, buffer, buffer);
}
#endif
static void imdct_step3_iter0_loop(int n, float *e, int i_off, int k_off, float *A)
{
   float *ee0 = e + i_off;
   float *ee2 = ee0 + k_off;
   int i;
   assert((n & 3) == 0);
   for (i=(n>>2); i > 0; --i) {
      float k00_20, k01_21;
      k00_20 = ee0[ 0] - ee2[ 0];
      k01_21 = ee0[-1] - ee2[-1];
      ee0[ 0] += ee2[ 0];
      ee0[-1] += ee2[-1];
      ee2[ 0] = k00_20 * A[0] - k01_21 * A[1];
      ee2[-1] = k01_21 * A[0] + k00_20 * A[1];
      A += 8;
      k00_20 = ee0[-2] - ee2[-2];
      k01_21 = ee0[-3] - ee2[-3];
      ee0[-2] += ee2[-2];
      ee0[-3] += ee2[-3];
      ee2[-2] = k00_20 * A[0] - k01_21 * A[1];
      ee2[-3] = k01_21 * A[0] + k00_20 * A[1];
      A += 8;
      k00_20 = ee0[-4] - ee2[-4];
      k01_21 = ee0[-5] - ee2[-5];
      ee0[-4] += ee2[-4];
      ee0[-5] += ee2[-5];
      ee2[-4] = k00_20 * A[0] - k01_21 * A[1];
      ee2[-5] = k01_21 * A[0] + k00_20 * A[1];
      A += 8;
      k00_20 = ee0[-6] - ee2[-6];
      k01_21 = ee0[-7] - ee2[-7];
      ee0[-6] += ee2[-6];
      ee0[-7] += ee2[-7];
      ee2[-6] = k00_20 * A[0] - k01_21 * A[1];
      ee2[-7] = k01_21 * A[0] + k00_20 * A[1];
      A += 8;
      ee0 -= 8;
      ee2 -= 8;
   }
}
static void imdct_step3_inner_r_loop(int lim, float *e, int d0, int k_off, float *A, int k1)
{
   int i;
   float k00_20, k01_21;
   float *e0 = e + d0;
   float *e2 = e0 + k_off;
   for (i=lim >> 2; i > 0; --i) {
      k00_20 = e0[-0] - e2[-0];
      k01_21 = e0[-1] - e2[-1];
      e0[-0] += e2[-0];
      e0[-1] += e2[-1];
      e2[-0] = (k00_20)*A[0] - (k01_21) * A[1];
      e2[-1] = (k01_21)*A[0] + (k00_20) * A[1];
      A += k1;
      k00_20 = e0[-2] - e2[-2];
      k01_21 = e0[-3] - e2[-3];
      e0[-2] += e2[-2];
      e0[-3] += e2[-3];
      e2[-2] = (k00_20)*A[0] - (k01_21) * A[1];
      e2[-3] = (k01_21)*A[0] + (k00_20) * A[1];
      A += k1;
      k00_20 = e0[-4] - e2[-4];
      k01_21 = e0[-5] - e2[-5];
      e0[-4] += e2[-4];
      e0[-5] += e2[-5];
      e2[-4] = (k00_20)*A[0] - (k01_21) * A[1];
      e2[-5] = (k01_21)*A[0] + (k00_20) * A[1];
      A += k1;
      k00_20 = e0[-6] - e2[-6];
      k01_21 = e0[-7] - e2[-7];
      e0[-6] += e2[-6];
      e0[-7] += e2[-7];
      e2[-6] = (k00_20)*A[0] - (k01_21) * A[1];
      e2[-7] = (k01_21)*A[0] + (k00_20) * A[1];
      e0 -= 8;
      e2 -= 8;
      A += k1;
   }
}
static void imdct_step3_inner_s_loop(int n, float *e, int i_off, int k_off, float *A, int a_off, int k0)
{
   int i;
   float A0 = A[0];
   float A1 = A[0+1];
   float A2 = A[0+a_off];
   float A3 = A[0+a_off+1];
   float A4 = A[0+a_off*2+0];
   float A5 = A[0+a_off*2+1];
   float A6 = A[0+a_off*3+0];
   float A7 = A[0+a_off*3+1];
   float k00,k11;
   float *ee0 = e +i_off;
   float *ee2 = ee0+k_off;
   for (i=n; i > 0; --i) {
      k00 = ee0[ 0] - ee2[ 0];
      k11 = ee0[-1] - ee2[-1];
      ee0[ 0] = ee0[ 0] + ee2[ 0];
      ee0[-1] = ee0[-1] + ee2[-1];
      ee2[ 0] = (k00) * A0 - (k11) * A1;
      ee2[-1] = (k11) * A0 + (k00) * A1;
      k00 = ee0[-2] - ee2[-2];
      k11 = ee0[-3] - ee2[-3];
      ee0[-2] = ee0[-2] + ee2[-2];
      ee0[-3] = ee0[-3] + ee2[-3];
      ee2[-2] = (k00) * A2 - (k11) * A3;
      ee2[-3] = (k11) * A2 + (k00) * A3;
      k00 = ee0[-4] - ee2[-4];
      k11 = ee0[-5] - ee2[-5];
      ee0[-4] = ee0[-4] + ee2[-4];
      ee0[-5] = ee0[-5] + ee2[-5];
      ee2[-4] = (k00) * A4 - (k11) * A5;
      ee2[-5] = (k11) * A4 + (k00) * A5;
      k00 = ee0[-6] - ee2[-6];
      k11 = ee0[-7] - ee2[-7];
      ee0[-6] = ee0[-6] + ee2[-6];
      ee0[-7] = ee0[-7] + ee2[-7];
      ee2[-6] = (k00) * A6 - (k11) * A7;
      ee2[-7] = (k11) * A6 + (k00) * A7;
      ee0 -= k0;
      ee2 -= k0;
   }
}
static __forceinline void iter_54(float *z)
{
   float k00,k11,k22,k33;
   float y0,y1,y2,y3;
   k00 = z[ 0] - z[-4];
   y0 = z[ 0] + z[-4];
   y2 = z[-2] + z[-6];
   k22 = z[-2] - z[-6];
   z[-0] = y0 + y2;
   z[-2] = y0 - y2;
   k33 = z[-3] - z[-7];
   z[-4] = k00 + k33;
   z[-6] = k00 - k33;
   k11 = z[-1] - z[-5];
   y1 = z[-1] + z[-5];
   y3 = z[-3] + z[-7];
   z[-1] = y1 + y3;
   z[-3] = y1 - y3;
   z[-5] = k11 - k22;
   z[-7] = k11 + k22;
}
static void imdct_step3_inner_s_loop_ld654(int n, float *e, int i_off, float *A, int base_n)
{
   int a_off = base_n >> 3;
   float A2 = A[0+a_off];
   float *z = e + i_off;
   float *base = z - 16 * n;
   while (z > base) {
      float k00,k11;
      float l00,l11;
      k00 = z[-0] - z[ -8];
      k11 = z[-1] - z[ -9];
      l00 = z[-2] - z[-10];
      l11 = z[-3] - z[-11];
      z[ -0] = z[-0] + z[ -8];
      z[ -1] = z[-1] + z[ -9];
      z[ -2] = z[-2] + z[-10];
      z[ -3] = z[-3] + z[-11];
      z[ -8] = k00;
      z[ -9] = k11;
      z[-10] = (l00+l11) * A2;
      z[-11] = (l11-l00) * A2;
      k00 = z[ -4] - z[-12];
      k11 = z[ -5] - z[-13];
      l00 = z[ -6] - z[-14];
      l11 = z[ -7] - z[-15];
      z[ -4] = z[ -4] + z[-12];
      z[ -5] = z[ -5] + z[-13];
      z[ -6] = z[ -6] + z[-14];
      z[ -7] = z[ -7] + z[-15];
      z[-12] = k11;
      z[-13] = -k00;
      z[-14] = (l11-l00) * A2;
      z[-15] = (l00+l11) * -A2;
      iter_54(z);
      iter_54(z-8);
      z -= 16;
   }
}
static void inverse_mdct(float *buffer, int n, vorb *f, int blocktype)
{
   int n2 = n >> 1, n4 = n >> 2, n8 = n >> 3, l;
   int ld;
   int save_point = temp_alloc_save(f);
   float *buf2 = (float *) temp_alloc(f, n2 * sizeof(*buf2));
   float *u=NULL,*v=NULL;
   float *A = f->A[blocktype];
   {
      float *d,*e, *AA, *e_stop;
      d = &buf2[n2-2];
      AA = A;
      e = &buffer[0];
      e_stop = &buffer[n2];
      while (e != e_stop) {
         d[1] = (e[0] * AA[0] - e[2]*AA[1]);
         d[0] = (e[0] * AA[1] + e[2]*AA[0]);
         d -= 2;
         AA += 2;
         e += 4;
      }
      e = &buffer[n2-3];
      while (d >= buf2) {
         d[1] = (-e[2] * AA[0] - -e[0]*AA[1]);
         d[0] = (-e[2] * AA[1] + -e[0]*AA[0]);
         d -= 2;
         AA += 2;
         e -= 4;
      }
   }
   u = buffer;
   v = buf2;
   {
      float *AA = &A[n2-8];
      float *d0,*d1, *e0, *e1;
      e0 = &v[n4];
      e1 = &v[0];
      d0 = &u[n4];
      d1 = &u[0];
      while (AA >= A) {
         float v40_20, v41_21;
         v41_21 = e0[1] - e1[1];
         v40_20 = e0[0] - e1[0];
         d0[1] = e0[1] + e1[1];
         d0[0] = e0[0] + e1[0];
         d1[1] = v41_21*AA[4] - v40_20*AA[5];
         d1[0] = v40_20*AA[4] + v41_21*AA[5];
         v41_21 = e0[3] - e1[3];
         v40_20 = e0[2] - e1[2];
         d0[3] = e0[3] + e1[3];
         d0[2] = e0[2] + e1[2];
         d1[3] = v41_21*AA[0] - v40_20*AA[1];
         d1[2] = v40_20*AA[0] + v41_21*AA[1];
         AA -= 8;
         d0 += 4;
         d1 += 4;
         e0 += 4;
         e1 += 4;
      }
   }
   ld = ilog(n) - 1;
   imdct_step3_iter0_loop(n >> 4, u, n2-1-n4*0, -(n >> 3), A);
   imdct_step3_iter0_loop(n >> 4, u, n2-1-n4*1, -(n >> 3), A);
   imdct_step3_inner_r_loop(n >> 5, u, n2-1 - n8*0, -(n >> 4), A, 16);
   imdct_step3_inner_r_loop(n >> 5, u, n2-1 - n8*1, -(n >> 4), A, 16);
   imdct_step3_inner_r_loop(n >> 5, u, n2-1 - n8*2, -(n >> 4), A, 16);
   imdct_step3_inner_r_loop(n >> 5, u, n2-1 - n8*3, -(n >> 4), A, 16);
   l=2;
   for (; l < (ld-3)>>1; ++l) {
      int k0 = n >> (l+2), k0_2 = k0>>1;
      int lim = 1 << (l+1);
      int i;
      for (i=0; i < lim; ++i)
         imdct_step3_inner_r_loop(n >> (l+4), u, n2-1 - k0*i, -k0_2, A, 1 << (l+3));
   }
   for (; l < ld-6; ++l) {
      int k0 = n >> (l+2), k1 = 1 << (l+3), k0_2 = k0>>1;
      int rlim = n >> (l+6), r;
      int lim = 1 << (l+1);
      int i_off;
      float *A0 = A;
      i_off = n2-1;
      for (r=rlim; r > 0; --r) {
         imdct_step3_inner_s_loop(lim, u, i_off, -k0_2, A0, k1, k0);
         A0 += k1*4;
         i_off -= 8;
      }
   }
   imdct_step3_inner_s_loop_ld654(n >> 5, u, n2-1, A, n);
   {
      uint16_t *bitrev = f->bit_reverse[blocktype];
      float *d0 = &v[n4-4];
      float *d1 = &v[n2-4];
      while (d0 >= v) {
         int k4;
         k4 = bitrev[0];
         d1[3] = u[k4+0];
         d1[2] = u[k4+1];
         d0[3] = u[k4+2];
         d0[2] = u[k4+3];
         k4 = bitrev[1];
         d1[1] = u[k4+0];
         d1[0] = u[k4+1];
         d0[1] = u[k4+2];
         d0[0] = u[k4+3];
         d0 -= 4;
         d1 -= 4;
         bitrev += 2;
      }
   }
   assert(v == buf2);
   {
      float *C = f->C[blocktype];
      float *d, *e;
      d = v;
      e = v + n2 - 4;
      while (d < e) {
         float a02,a11,b0,b1,b2,b3;
         a02 = d[0] - e[2];
         a11 = d[1] + e[3];
         b0 = C[1]*a02 + C[0]*a11;
         b1 = C[1]*a11 - C[0]*a02;
         b2 = d[0] + e[ 2];
         b3 = d[1] - e[ 3];
         d[0] = b2 + b0;
         d[1] = b3 + b1;
         e[2] = b2 - b0;
         e[3] = b1 - b3;
         a02 = d[2] - e[0];
         a11 = d[3] + e[1];
         b0 = C[3]*a02 + C[2]*a11;
         b1 = C[3]*a11 - C[2]*a02;
         b2 = d[2] + e[ 0];
         b3 = d[3] - e[ 1];
         d[2] = b2 + b0;
         d[3] = b3 + b1;
         e[0] = b2 - b0;
         e[1] = b1 - b3;
         C += 4;
         d += 4;
         e -= 4;
      }
   }
   {
      float *d0,*d1,*d2,*d3;
      float *B = f->B[blocktype] + n2 - 8;
      float *e = buf2 + n2 - 8;
      d0 = &buffer[0];
      d1 = &buffer[n2-4];
      d2 = &buffer[n2];
      d3 = &buffer[n-4];
      while (e >= v) {
         float p0,p1,p2,p3;
         p3 = e[6]*B[7] - e[7]*B[6];
         p2 = -e[6]*B[6] - e[7]*B[7];
         d0[0] = p3;
         d1[3] = - p3;
         d2[0] = p2;
         d3[3] = p2;
         p1 = e[4]*B[5] - e[5]*B[4];
         p0 = -e[4]*B[4] - e[5]*B[5];
         d0[1] = p1;
         d1[2] = - p1;
         d2[1] = p0;
         d3[2] = p0;
         p3 = e[2]*B[3] - e[3]*B[2];
         p2 = -e[2]*B[2] - e[3]*B[3];
         d0[2] = p3;
         d1[1] = - p3;
         d2[2] = p2;
         d3[1] = p2;
         p1 = e[0]*B[1] - e[1]*B[0];
         p0 = -e[0]*B[0] - e[1]*B[1];
         d0[3] = p1;
         d1[0] = - p1;
         d2[3] = p0;
         d3[0] = p0;
         B -= 8;
         e -= 8;
         d0 += 4;
         d2 += 4;
         d1 -= 4;
         d3 -= 4;
      }
   }
   temp_free(f,buf2);
   temp_alloc_restore(f,save_point);
}
#if 0
void inverse_mdct_naive(float *buffer, int n)
{
   float s;
   float A[1 << 12], B[1 << 12], C[1 << 11];
   int i,k,k2,k4, n2 = n >> 1, n4 = n >> 2, n8 = n >> 3, l;
   int n3_4 = n - n4, ld;
   float u[1 << 13], X[1 << 13], v[1 << 13], w[1 << 13];
   for (k=k2=0; k < n4; ++k,k2+=2) {
      A[k2 ] = (float) cos(4*k*M_PI/n);
      A[k2+1] = (float) -sin(4*k*M_PI/n);
      B[k2 ] = (float) cos((k2+1)*M_PI/n/2);
      B[k2+1] = (float) sin((k2+1)*M_PI/n/2);
   }
   for (k=k2=0; k < n8; ++k,k2+=2) {
      C[k2 ] = (float) cos(2*(k2+1)*M_PI/n);
      C[k2+1] = (float) -sin(2*(k2+1)*M_PI/n);
   }
   for (k=0; k < n2; ++k) u[k] = buffer[k];
   for ( ; k < n ; ++k) u[k] = -buffer[n - k - 1];
   for (k=k2=k4=0; k < n4; k+=1, k2+=2, k4+=4) {
      v[n-k4-1] = (u[k4] - u[n-k4-1]) * A[k2] - (u[k4+2] - u[n-k4-3])*A[k2+1];
      v[n-k4-3] = (u[k4] - u[n-k4-1]) * A[k2+1] + (u[k4+2] - u[n-k4-3])*A[k2];
   }
   for (k=k4=0; k < n8; k+=1, k4+=4) {
      w[n2+3+k4] = v[n2+3+k4] + v[k4+3];
      w[n2+1+k4] = v[n2+1+k4] + v[k4+1];
      w[k4+3] = (v[n2+3+k4] - v[k4+3])*A[n2-4-k4] - (v[n2+1+k4]-v[k4+1])*A[n2-3-k4];
      w[k4+1] = (v[n2+1+k4] - v[k4+1])*A[n2-4-k4] + (v[n2+3+k4]-v[k4+3])*A[n2-3-k4];
   }
   ld = ilog(n) - 1;
   for (l=0; l < ld-3; ++l) {
      int k0 = n >> (l+2), k1 = 1 << (l+3);
      int rlim = n >> (l+4), r4, r;
      int s2lim = 1 << (l+2), s2;
      for (r=r4=0; r < rlim; r4+=4,++r) {
         for (s2=0; s2 < s2lim; s2+=2) {
            u[n-1-k0*s2-r4] = w[n-1-k0*s2-r4] + w[n-1-k0*(s2+1)-r4];
            u[n-3-k0*s2-r4] = w[n-3-k0*s2-r4] + w[n-3-k0*(s2+1)-r4];
            u[n-1-k0*(s2+1)-r4] = (w[n-1-k0*s2-r4] - w[n-1-k0*(s2+1)-r4]) * A[r*k1]
                                - (w[n-3-k0*s2-r4] - w[n-3-k0*(s2+1)-r4]) * A[r*k1+1];
            u[n-3-k0*(s2+1)-r4] = (w[n-3-k0*s2-r4] - w[n-3-k0*(s2+1)-r4]) * A[r*k1]
                                + (w[n-1-k0*s2-r4] - w[n-1-k0*(s2+1)-r4]) * A[r*k1+1];
         }
      }
      if (l+1 < ld-3) {
         memcpy(w, u, sizeof(u));
      }
   }
   for (i=0; i < n8; ++i) {
      int j = bit_reverse(i) >> (32-ld+3);
      assert(j < n8);
      if (i == j) {
         int i8 = i << 3;
         v[i8+1] = u[i8+1];
         v[i8+3] = u[i8+3];
         v[i8+5] = u[i8+5];
         v[i8+7] = u[i8+7];
      } else if (i < j) {
         int i8 = i << 3, j8 = j << 3;
         v[j8+1] = u[i8+1], v[i8+1] = u[j8 + 1];
         v[j8+3] = u[i8+3], v[i8+3] = u[j8 + 3];
         v[j8+5] = u[i8+5], v[i8+5] = u[j8 + 5];
         v[j8+7] = u[i8+7], v[i8+7] = u[j8 + 7];
      }
   }
   for (k=0; k < n2; ++k) {
      w[k] = v[k*2+1];
   }
   for (k=k2=k4=0; k < n8; ++k, k2 += 2, k4 += 4) {
      u[n-1-k2] = w[k4];
      u[n-2-k2] = w[k4+1];
      u[n3_4 - 1 - k2] = w[k4+2];
      u[n3_4 - 2 - k2] = w[k4+3];
   }
   for (k=k2=0; k < n8; ++k, k2 += 2) {
      v[n2 + k2 ] = ( u[n2 + k2] + u[n-2-k2] + C[k2+1]*(u[n2+k2]-u[n-2-k2]) + C[k2]*(u[n2+k2+1]+u[n-2-k2+1]))/2;
      v[n-2 - k2] = ( u[n2 + k2] + u[n-2-k2] - C[k2+1]*(u[n2+k2]-u[n-2-k2]) - C[k2]*(u[n2+k2+1]+u[n-2-k2+1]))/2;
      v[n2+1+ k2] = ( u[n2+1+k2] - u[n-1-k2] + C[k2+1]*(u[n2+1+k2]+u[n-1-k2]) - C[k2]*(u[n2+k2]-u[n-2-k2]))/2;
      v[n-1 - k2] = (-u[n2+1+k2] + u[n-1-k2] + C[k2+1]*(u[n2+1+k2]+u[n-1-k2]) - C[k2]*(u[n2+k2]-u[n-2-k2]))/2;
   }
   for (k=k2=0; k < n4; ++k,k2 += 2) {
      X[k] = v[k2+n2]*B[k2 ] + v[k2+1+n2]*B[k2+1];
      X[n2-1-k] = v[k2+n2]*B[k2+1] - v[k2+1+n2]*B[k2 ];
   }
   s = 0.5;
   for (i=0; i < n4 ; ++i) buffer[i] = s * X[i+n4];
   for ( ; i < n3_4; ++i) buffer[i] = -s * X[n3_4 - i - 1];
   for ( ; i < n ; ++i) buffer[i] = -s * X[i - n3_4];
}
#endif
static float *get_window(vorb *f, int len)
{
   len <<= 1;
   if (len == f->blocksize_0) return f->window[0];
   if (len == f->blocksize_1) return f->window[1];
   return NULL;
}
#ifndef STB_VORBIS_NO_DEFER_FLOOR
typedef int16_t YTYPE;
#else
typedef int YTYPE;
#endif
static int do_floor(vorb *f, Mapping *map, int i, int n, float *target, YTYPE *finalY, uint8_t *step2_flag)
{
   int n2 = n >> 1;
   int s = map->chan[i].mux, floor;
   floor = map->submap_floor[s];
   if (f->floor_types[floor] == 0) {
      return error(f, VORBIS_invalid_stream);
   } else {
      Floor1 *g = &f->floor_config[floor].floor1;
      int j,q;
      int lx = 0, ly = finalY[0] * g->floor1_multiplier;
      for (q=1; q < g->values; ++q) {
         j = g->sorted_order[q];
         #ifndef STB_VORBIS_NO_DEFER_FLOOR
         STBV_NOTUSED(step2_flag);
         if (finalY[j] >= 0)
         #else
         if (step2_flag[j])
         #endif
         {
            int hy = finalY[j] * g->floor1_multiplier;
            int hx = g->Xlist[j];
            if (lx != hx)
               draw_line(target, lx,ly, hx,hy, n2);
            CHECK(f);
            lx = hx, ly = hy;
         }
      }
      if (lx < n2) {
         for (j=lx; j < n2; ++j)
            LINE_OP(target[j], inverse_db_table[ly]);
         CHECK(f);
      }
   }
   return TRUE;
}
static int vorbis_decode_initial(vorb *f, int *p_left_start, int *p_left_end, int *p_right_start, int *p_right_end, int *mode)
{
   Mode *m;
   int i, n, prev, next, window_center;
   f->channel_buffer_start = f->channel_buffer_end = 0;
  retry:
   if (f->eof) return FALSE;
   if (!maybe_start_packet(f))
      return FALSE;
   if (get_bits(f,1) != 0) {
      if (IS_PUSH_MODE(f))
         return error(f,VORBIS_bad_packet_type);
      while (EOP != get8_packet(f));
      goto retry;
   }
   if (f->alloc.alloc_buffer)
      assert(f->alloc.alloc_buffer_length_in_bytes == f->temp_offset);
   i = get_bits(f, ilog(f->mode_count-1));
   if (i == EOP) return FALSE;
   if (i >= f->mode_count) return FALSE;
   *mode = i;
   m = f->mode_config + i;
   if (m->blockflag) {
      n = f->blocksize_1;
      prev = get_bits(f,1);
      next = get_bits(f,1);
   } else {
      prev = next = 0;
      n = f->blocksize_0;
   }
   window_center = n >> 1;
   if (m->blockflag && !prev) {
      *p_left_start = (n - f->blocksize_0) >> 2;
      *p_left_end = (n + f->blocksize_0) >> 2;
   } else {
      *p_left_start = 0;
      *p_left_end = window_center;
   }
   if (m->blockflag && !next) {
      *p_right_start = (n*3 - f->blocksize_0) >> 2;
      *p_right_end = (n*3 + f->blocksize_0) >> 2;
   } else {
      *p_right_start = window_center;
      *p_right_end = n;
   }
   return TRUE;
}
static int vorbis_decode_packet_rest(vorb *f, int *len, Mode *m, int left_start, int left_end, int right_start, int right_end, int *p_left)
{
   Mapping *map;
   int i,j,k,n,n2;
   int zero_channel[256];
   int really_zero_channel[256];
   STBV_NOTUSED(left_end);
   n = f->blocksize[m->blockflag];
   map = &f->mapping[m->mapping];
   n2 = n >> 1;
   CHECK(f);
   for (i=0; i < f->channels; ++i) {
      int s = map->chan[i].mux, floor;
      zero_channel[i] = FALSE;
      floor = map->submap_floor[s];
      if (f->floor_types[floor] == 0) {
         return error(f, VORBIS_invalid_stream);
      } else {
         Floor1 *g = &f->floor_config[floor].floor1;
         if (get_bits(f, 1)) {
            short *finalY;
            uint8_t step2_flag[256];
            static int range_list[4] = { 256, 128, 86, 64 };
            int range = range_list[g->floor1_multiplier-1];
            int offset = 2;
            finalY = f->finalY[i];
            finalY[0] = get_bits(f, ilog(range)-1);
            finalY[1] = get_bits(f, ilog(range)-1);
            for (j=0; j < g->partitions; ++j) {
               int pclass = g->partition_class_list[j];
               int cdim = g->class_dimensions[pclass];
               int cbits = g->class_subclasses[pclass];
               int csub = (1 << cbits)-1;
               int cval = 0;
               if (cbits) {
                  Codebook *c = f->codebooks + g->class_masterbooks[pclass];
                  DECODE(cval,f,c);
               }
               for (k=0; k < cdim; ++k) {
                  int book = g->subclass_books[pclass][cval & csub];
                  cval = cval >> cbits;
                  if (book >= 0) {
                     int temp;
                     Codebook *c = f->codebooks + book;
                     DECODE(temp,f,c);
                     finalY[offset++] = temp;
                  } else
                     finalY[offset++] = 0;
               }
            }
            if (f->valid_bits == INVALID_BITS) goto error;
            step2_flag[0] = step2_flag[1] = 1;
            for (j=2; j < g->values; ++j) {
               int low, high, pred, highroom, lowroom, room, val;
               low = g->neighbors[j][0];
               high = g->neighbors[j][1];
               pred = predict_point(g->Xlist[j], g->Xlist[low], g->Xlist[high], finalY[low], finalY[high]);
               val = finalY[j];
               highroom = range - pred;
               lowroom = pred;
               if (highroom < lowroom)
                  room = highroom * 2;
               else
                  room = lowroom * 2;
               if (val) {
                  step2_flag[low] = step2_flag[high] = 1;
                  step2_flag[j] = 1;
                  if (val >= room)
                     if (highroom > lowroom)
                        finalY[j] = val - lowroom + pred;
                     else
                        finalY[j] = pred - val + highroom - 1;
                  else
                     if (val & 1)
                        finalY[j] = pred - ((val+1)>>1);
                     else
                        finalY[j] = pred + (val>>1);
               } else {
                  step2_flag[j] = 0;
                  finalY[j] = pred;
               }
            }
#ifdef STB_VORBIS_NO_DEFER_FLOOR
            do_floor(f, map, i, n, f->floor_buffers[i], finalY, step2_flag);
#else
            for (j=0; j < g->values; ++j) {
               if (!step2_flag[j])
                  finalY[j] = -1;
            }
#endif
         } else {
           error:
            zero_channel[i] = TRUE;
         }
      }
   }
   CHECK(f);
   if (f->alloc.alloc_buffer)
      assert(f->alloc.alloc_buffer_length_in_bytes == f->temp_offset);
   memcpy(really_zero_channel, zero_channel, sizeof(really_zero_channel[0]) * f->channels);
   for (i=0; i < map->coupling_steps; ++i)
      if (!zero_channel[map->chan[i].magnitude] || !zero_channel[map->chan[i].angle]) {
         zero_channel[map->chan[i].magnitude] = zero_channel[map->chan[i].angle] = FALSE;
      }
   CHECK(f);
   for (i=0; i < map->submaps; ++i) {
      float *residue_buffers[STB_VORBIS_MAX_CHANNELS];
      int r;
      uint8_t do_not_decode[256];
      int ch = 0;
      for (j=0; j < f->channels; ++j) {
         if (map->chan[j].mux == i) {
            if (zero_channel[j]) {
               do_not_decode[ch] = TRUE;
               residue_buffers[ch] = NULL;
            } else {
               do_not_decode[ch] = FALSE;
               residue_buffers[ch] = f->channel_buffers[j];
            }
            ++ch;
         }
      }
      r = map->submap_residue[i];
      decode_residue(f, residue_buffers, ch, n2, r, do_not_decode);
   }
   if (f->alloc.alloc_buffer)
      assert(f->alloc.alloc_buffer_length_in_bytes == f->temp_offset);
   CHECK(f);
   for (i = map->coupling_steps-1; i >= 0; --i) {
      int n2 = n >> 1;
      float *m = f->channel_buffers[map->chan[i].magnitude];
      float *a = f->channel_buffers[map->chan[i].angle ];
      for (j=0; j < n2; ++j) {
         float a2,m2;
         if (m[j] > 0)
            if (a[j] > 0)
               m2 = m[j], a2 = m[j] - a[j];
            else
               a2 = m[j], m2 = m[j] + a[j];
         else
            if (a[j] > 0)
               m2 = m[j], a2 = m[j] + a[j];
            else
               a2 = m[j], m2 = m[j] - a[j];
         m[j] = m2;
         a[j] = a2;
      }
   }
   CHECK(f);
#ifndef STB_VORBIS_NO_DEFER_FLOOR
   for (i=0; i < f->channels; ++i) {
      if (really_zero_channel[i]) {
         memset(f->channel_buffers[i], 0, sizeof(*f->channel_buffers[i]) * n2);
      } else {
         do_floor(f, map, i, n, f->channel_buffers[i], f->finalY[i], NULL);
      }
   }
#else
   for (i=0; i < f->channels; ++i) {
      if (really_zero_channel[i]) {
         memset(f->channel_buffers[i], 0, sizeof(*f->channel_buffers[i]) * n2);
      } else {
         for (j=0; j < n2; ++j)
            f->channel_buffers[i][j] *= f->floor_buffers[i][j];
      }
   }
#endif
   CHECK(f);
   for (i=0; i < f->channels; ++i)
      inverse_mdct(f->channel_buffers[i], n, f, m->blockflag);
   CHECK(f);
   flush_packet(f);
   if (f->first_decode) {
      f->current_loc = 0u - n2;
      f->discard_samples_deferred = n - right_end;
      f->current_loc_valid = TRUE;
      f->first_decode = FALSE;
   } else if (f->discard_samples_deferred) {
      if (f->discard_samples_deferred >= right_start - left_start) {
         f->discard_samples_deferred -= (right_start - left_start);
         left_start = right_start;
         *p_left = left_start;
      } else {
         left_start += f->discard_samples_deferred;
         *p_left = left_start;
         f->discard_samples_deferred = 0;
      }
   } else if (f->previous_length == 0 && f->current_loc_valid) {
   }
   if (f->last_seg_which == f->end_seg_with_known_loc) {
      if (f->current_loc_valid && (f->page_flag & PAGEFLAG_last_page)) {
         uint32_t current_end = f->known_loc_for_packet;
         if (current_end < f->current_loc + (right_end-left_start)) {
            if (current_end < f->current_loc) {
               *len = 0;
            } else {
               *len = current_end - f->current_loc;
            }
            *len += left_start;
            if (*len > right_end) *len = right_end;
            f->current_loc += *len;
            return TRUE;
         }
      }
      f->current_loc = f->known_loc_for_packet - (n2-left_start);
      f->current_loc_valid = TRUE;
   }
   if (f->current_loc_valid)
      f->current_loc += (right_start - left_start);
   if (f->alloc.alloc_buffer)
      assert(f->alloc.alloc_buffer_length_in_bytes == f->temp_offset);
   *len = right_end;
   CHECK(f);
   return TRUE;
}
static int vorbis_decode_packet(vorb *f, int *len, int *p_left, int *p_right)
{
   int mode, left_end, right_end;
   if (!vorbis_decode_initial(f, p_left, &left_end, p_right, &right_end, &mode)) return 0;
   return vorbis_decode_packet_rest(f, len, f->mode_config + mode, *p_left, left_end, *p_right, right_end, p_left);
}
static int vorbis_finish_frame(stb_vorbis *f, int len, int left, int right)
{
   int prev,i,j;
   if (f->previous_length) {
      int i,j, n = f->previous_length;
      float *w = get_window(f, n);
      if (w == NULL) return 0;
      for (i=0; i < f->channels; ++i) {
         for (j=0; j < n; ++j)
            f->channel_buffers[i][left+j] =
               f->channel_buffers[i][left+j]*w[ j] +
               f->previous_window[i][ j]*w[n-1-j];
      }
   }
   prev = f->previous_length;
   f->previous_length = len - right;
   for (i=0; i < f->channels; ++i)
      for (j=0; right+j < len; ++j)
         f->previous_window[i][j] = f->channel_buffers[i][right+j];
   if (!prev)
      return 0;
   if (len < right) right = len;
   f->samples_output += right-left;
   return right - left;
}
static int vorbis_pump_first_frame(stb_vorbis *f)
{
   int len, right, left, res;
   res = vorbis_decode_packet(f, &len, &left, &right);
   if (res)
      vorbis_finish_frame(f, len, left, right);
   return res;
}
#ifndef STB_VORBIS_NO_PUSHDATA_API
static int is_whole_packet_present(stb_vorbis *f)
{
   int s = f->next_seg, first = TRUE;
   uint8_t *p = f->stream;
   if (s != -1) {
      for (; s < f->segment_count; ++s) {
         p += f->segments[s];
         if (f->segments[s] < 255)
            break;
      }
      if (s == f->segment_count)
         s = -1;
      if (p > f->stream_end) return error(f, VORBIS_need_more_data);
      first = FALSE;
   }
   for (; s == -1;) {
      uint8_t *q;
      int n;
      if (p + 26 >= f->stream_end) return error(f, VORBIS_need_more_data);
      if (memcmp(p, ogg_page_header, 4)) return error(f, VORBIS_invalid_stream);
      if (p[4] != 0) return error(f, VORBIS_invalid_stream);
      if (first) {
         if (f->previous_length)
            if ((p[5] & PAGEFLAG_continued_packet)) return error(f, VORBIS_invalid_stream);
      } else {
         if (!(p[5] & PAGEFLAG_continued_packet)) return error(f, VORBIS_invalid_stream);
      }
      n = p[26];
      q = p+27;
      p = q + n;
      if (p > f->stream_end) return error(f, VORBIS_need_more_data);
      for (s=0; s < n; ++s) {
         p += q[s];
         if (q[s] < 255)
            break;
      }
      if (s == n)
         s = -1;
      if (p > f->stream_end) return error(f, VORBIS_need_more_data);
      first = FALSE;
   }
   return TRUE;
}
#endif
static int start_decoder(vorb *f)
{
   uint8_t header[6], x,y;
   int len,i,j,k, max_submaps = 0;
   int longest_floorlist=0;
   f->first_decode = TRUE;
   if (!start_page(f)) return FALSE;
   if (!(f->page_flag & PAGEFLAG_first_page)) return error(f, VORBIS_invalid_first_page);
   if (f->page_flag & PAGEFLAG_last_page) return error(f, VORBIS_invalid_first_page);
   if (f->page_flag & PAGEFLAG_continued_packet) return error(f, VORBIS_invalid_first_page);
   if (f->segment_count != 1) return error(f, VORBIS_invalid_first_page);
   if (f->segments[0] != 30) {
      if (f->segments[0] == 64 &&
          getn(f, header, 6) &&
          header[0] == 'f' &&
          header[1] == 'i' &&
          header[2] == 's' &&
          header[3] == 'h' &&
          header[4] == 'e' &&
          header[5] == 'a' &&
          get8(f) == 'd' &&
          get8(f) == '\0') return error(f, VORBIS_ogg_skeleton_not_supported);
      else
                                                    return error(f, VORBIS_invalid_first_page);
   }
   if (get8(f) != VORBIS_packet_id) return error(f, VORBIS_invalid_first_page);
   if (!getn(f, header, 6)) return error(f, VORBIS_unexpected_eof);
   if (!vorbis_validate(header)) return error(f, VORBIS_invalid_first_page);
   if (get32(f) != 0) return error(f, VORBIS_invalid_first_page);
   f->channels = get8(f); if (!f->channels) return error(f, VORBIS_invalid_first_page);
   if (f->channels > STB_VORBIS_MAX_CHANNELS) return error(f, VORBIS_too_many_channels);
   f->sample_rate = get32(f); if (!f->sample_rate) return error(f, VORBIS_invalid_first_page);
   get32(f);
   get32(f);
   get32(f);
   x = get8(f);
   {
      int log0,log1;
      log0 = x & 15;
      log1 = x >> 4;
      f->blocksize_0 = 1 << log0;
      f->blocksize_1 = 1 << log1;
      if (log0 < 6 || log0 > 13) return error(f, VORBIS_invalid_setup);
      if (log1 < 6 || log1 > 13) return error(f, VORBIS_invalid_setup);
      if (log0 > log1) return error(f, VORBIS_invalid_setup);
   }
   x = get8(f);
   if (!(x & 1)) return error(f, VORBIS_invalid_first_page);
   if (!start_page(f)) return FALSE;
   if (!start_packet(f)) return FALSE;
   if (!next_segment(f)) return FALSE;
   if (get8_packet(f) != VORBIS_packet_comment) return error(f, VORBIS_invalid_setup);
   for (i=0; i < 6; ++i) header[i] = get8_packet(f);
   if (!vorbis_validate(header)) return error(f, VORBIS_invalid_setup);
   len = get32_packet(f);
   f->vendor = (char*)setup_malloc(f, sizeof(char) * (len+1));
   if (f->vendor == NULL) return error(f, VORBIS_outofmem);
   for(i=0; i < len; ++i) {
      f->vendor[i] = get8_packet(f);
   }
   f->vendor[len] = (char)'\0';
   f->comment_list_length = get32_packet(f);
   f->comment_list = NULL;
   if (f->comment_list_length > 0)
   {
      f->comment_list = (char**) setup_malloc(f, sizeof(char*) * (f->comment_list_length));
      if (f->comment_list == NULL) return error(f, VORBIS_outofmem);
   }
   for(i=0; i < f->comment_list_length; ++i) {
      len = get32_packet(f);
      f->comment_list[i] = (char*)setup_malloc(f, sizeof(char) * (len+1));
      if (f->comment_list[i] == NULL) return error(f, VORBIS_outofmem);
      for(j=0; j < len; ++j) {
         f->comment_list[i][j] = get8_packet(f);
      }
      f->comment_list[i][len] = (char)'\0';
   }
   x = get8_packet(f);
   if (!(x & 1)) return error(f, VORBIS_invalid_setup);
   skip(f, f->bytes_in_seg);
   f->bytes_in_seg = 0;
   do {
      len = next_segment(f);
      skip(f, len);
      f->bytes_in_seg = 0;
   } while (len);
   if (!start_packet(f)) return FALSE;
   #ifndef STB_VORBIS_NO_PUSHDATA_API
   if (IS_PUSH_MODE(f)) {
      if (!is_whole_packet_present(f)) {
         if (f->error == VORBIS_invalid_stream)
            f->error = VORBIS_invalid_setup;
         return FALSE;
      }
   }
   #endif
   crc32_init();
   if (get8_packet(f) != VORBIS_packet_setup) return error(f, VORBIS_invalid_setup);
   for (i=0; i < 6; ++i) header[i] = get8_packet(f);
   if (!vorbis_validate(header)) return error(f, VORBIS_invalid_setup);
   f->codebook_count = get_bits(f,8) + 1;
   f->codebooks = (Codebook *) setup_malloc(f, sizeof(*f->codebooks) * f->codebook_count);
   if (f->codebooks == NULL) return error(f, VORBIS_outofmem);
   memset(f->codebooks, 0, sizeof(*f->codebooks) * f->codebook_count);
   for (i=0; i < f->codebook_count; ++i) {
      uint32_t *values;
      int ordered, sorted_count;
      int total=0;
      uint8_t *lengths;
      Codebook *c = f->codebooks+i;
      CHECK(f);
      x = get_bits(f, 8); if (x != 0x42) return error(f, VORBIS_invalid_setup);
      x = get_bits(f, 8); if (x != 0x43) return error(f, VORBIS_invalid_setup);
      x = get_bits(f, 8); if (x != 0x56) return error(f, VORBIS_invalid_setup);
      x = get_bits(f, 8);
      c->dimensions = (get_bits(f, 8)<<8) + x;
      x = get_bits(f, 8);
      y = get_bits(f, 8);
      c->entries = (get_bits(f, 8)<<16) + (y<<8) + x;
      ordered = get_bits(f,1);
      c->sparse = ordered ? 0 : get_bits(f,1);
      if (c->dimensions == 0 && c->entries != 0) return error(f, VORBIS_invalid_setup);
      if (c->sparse)
         lengths = (uint8_t *) setup_temp_malloc(f, c->entries);
      else
         lengths = c->codeword_lengths = (uint8_t *) setup_malloc(f, c->entries);
      if (!lengths) return error(f, VORBIS_outofmem);
      if (ordered) {
         int current_entry = 0;
         int current_length = get_bits(f,5) + 1;
         while (current_entry < c->entries) {
            int limit = c->entries - current_entry;
            int n = get_bits(f, ilog(limit));
            if (current_length >= 32) return error(f, VORBIS_invalid_setup);
            if (current_entry + n > (int) c->entries) { return error(f, VORBIS_invalid_setup); }
            memset(lengths + current_entry, current_length, n);
            current_entry += n;
            ++current_length;
         }
      } else {
         for (j=0; j < c->entries; ++j) {
            int present = c->sparse ? get_bits(f,1) : 1;
            if (present) {
               lengths[j] = get_bits(f, 5) + 1;
               ++total;
               if (lengths[j] == 32)
                  return error(f, VORBIS_invalid_setup);
            } else {
               lengths[j] = NO_CODE;
            }
         }
      }
      if (c->sparse && total >= c->entries >> 2) {
         if (c->entries > (int) f->setup_temp_memory_required)
            f->setup_temp_memory_required = c->entries;
         c->codeword_lengths = (uint8_t *) setup_malloc(f, c->entries);
         if (c->codeword_lengths == NULL) return error(f, VORBIS_outofmem);
         memcpy(c->codeword_lengths, lengths, c->entries);
         setup_temp_free(f, lengths, c->entries);
         lengths = c->codeword_lengths;
         c->sparse = 0;
      }
      if (c->sparse) {
         sorted_count = total;
      } else {
         sorted_count = 0;
         #ifndef STB_VORBIS_NO_HUFFMAN_BINARY_SEARCH
         for (j=0; j < c->entries; ++j)
            if (lengths[j] > STB_VORBIS_FAST_HUFFMAN_LENGTH && lengths[j] != NO_CODE)
               ++sorted_count;
         #endif
      }
      c->sorted_entries = sorted_count;
      values = NULL;
      CHECK(f);
      if (!c->sparse) {
         c->codewords = (uint32_t *) setup_malloc(f, sizeof(c->codewords[0]) * c->entries);
         if (!c->codewords) return error(f, VORBIS_outofmem);
      } else {
         unsigned int size;
         if (c->sorted_entries) {
            c->codeword_lengths = (uint8_t *) setup_malloc(f, c->sorted_entries);
            if (!c->codeword_lengths) return error(f, VORBIS_outofmem);
            c->codewords = (uint32_t *) setup_temp_malloc(f, sizeof(*c->codewords) * c->sorted_entries);
            if (!c->codewords) return error(f, VORBIS_outofmem);
            values = (uint32_t *) setup_temp_malloc(f, sizeof(*values) * c->sorted_entries);
            if (!values) return error(f, VORBIS_outofmem);
         }
         size = c->entries + (sizeof(*c->codewords) + sizeof(*values)) * c->sorted_entries;
         if (size > f->setup_temp_memory_required)
            f->setup_temp_memory_required = size;
      }
      if (!compute_codewords(c, lengths, c->entries, values)) {
         if (c->sparse) setup_temp_free(f, values, 0);
         return error(f, VORBIS_invalid_setup);
      }
      if (c->sorted_entries) {
         c->sorted_codewords = (uint32_t *) setup_malloc(f, sizeof(*c->sorted_codewords) * (c->sorted_entries+1));
         if (c->sorted_codewords == NULL) return error(f, VORBIS_outofmem);
         c->sorted_values = ( int *) setup_malloc(f, sizeof(*c->sorted_values ) * (c->sorted_entries+1));
         if (c->sorted_values == NULL) return error(f, VORBIS_outofmem);
         ++c->sorted_values;
         c->sorted_values[-1] = -1;
         compute_sorted_huffman(c, lengths, values);
      }
      if (c->sparse) {
         setup_temp_free(f, values, sizeof(*values)*c->sorted_entries);
         setup_temp_free(f, c->codewords, sizeof(*c->codewords)*c->sorted_entries);
         setup_temp_free(f, lengths, c->entries);
         c->codewords = NULL;
      }
      compute_accelerated_huffman(c);
      CHECK(f);
      c->lookup_type = get_bits(f, 4);
      if (c->lookup_type > 2) return error(f, VORBIS_invalid_setup);
      if (c->lookup_type > 0) {
         uint16_t *mults;
         c->minimum_value = float32_unpack(get_bits(f, 32));
         c->delta_value = float32_unpack(get_bits(f, 32));
         c->value_bits = get_bits(f, 4)+1;
         c->sequence_p = get_bits(f,1);
         if (c->lookup_type == 1) {
            int values = lookup1_values(c->entries, c->dimensions);
            if (values < 0) return error(f, VORBIS_invalid_setup);
            c->lookup_values = (uint32_t) values;
         } else {
            c->lookup_values = c->entries * c->dimensions;
         }
         if (c->lookup_values == 0) return error(f, VORBIS_invalid_setup);
         mults = (uint16_t *) setup_temp_malloc(f, sizeof(mults[0]) * c->lookup_values);
         if (mults == NULL) return error(f, VORBIS_outofmem);
         for (j=0; j < (int) c->lookup_values; ++j) {
            int q = get_bits(f, c->value_bits);
            if (q == EOP) { setup_temp_free(f,mults,sizeof(mults[0])*c->lookup_values); return error(f, VORBIS_invalid_setup); }
            mults[j] = q;
         }
#ifndef STB_VORBIS_DIVIDES_IN_CODEBOOK
         if (c->lookup_type == 1) {
            int len, sparse = c->sparse;
            float last=0;
            if (sparse) {
               if (c->sorted_entries == 0) goto skip;
               c->multiplicands = (codetype *) setup_malloc(f, sizeof(c->multiplicands[0]) * c->sorted_entries * c->dimensions);
            } else
               c->multiplicands = (codetype *) setup_malloc(f, sizeof(c->multiplicands[0]) * c->entries * c->dimensions);
            if (c->multiplicands == NULL) { setup_temp_free(f,mults,sizeof(mults[0])*c->lookup_values); return error(f, VORBIS_outofmem); }
            len = sparse ? c->sorted_entries : c->entries;
            for (j=0; j < len; ++j) {
               unsigned int z = sparse ? c->sorted_values[j] : j;
               unsigned int div=1;
               for (k=0; k < c->dimensions; ++k) {
                  int off = (z / div) % c->lookup_values;
                  float val = mults[off]*c->delta_value + c->minimum_value + last;
                  c->multiplicands[j*c->dimensions + k] = val;
                  if (c->sequence_p)
                     last = val;
                  if (k+1 < c->dimensions) {
                     if (div > UINT_MAX / (unsigned int) c->lookup_values) {
                        setup_temp_free(f, mults,sizeof(mults[0])*c->lookup_values);
                        return error(f, VORBIS_invalid_setup);
                     }
                     div *= c->lookup_values;
                  }
               }
            }
            c->lookup_type = 2;
         }
         else
#endif
         {
            float last=0;
            CHECK(f);
            c->multiplicands = (codetype *) setup_malloc(f, sizeof(c->multiplicands[0]) * c->lookup_values);
            if (c->multiplicands == NULL) { setup_temp_free(f, mults,sizeof(mults[0])*c->lookup_values); return error(f, VORBIS_outofmem); }
            for (j=0; j < (int) c->lookup_values; ++j) {
               float val = mults[j] * c->delta_value + c->minimum_value + last;
               c->multiplicands[j] = val;
               if (c->sequence_p)
                  last = val;
            }
         }
#ifndef STB_VORBIS_DIVIDES_IN_CODEBOOK
        skip:;
#endif
         setup_temp_free(f, mults, sizeof(mults[0])*c->lookup_values);
         CHECK(f);
      }
      CHECK(f);
   }
   x = get_bits(f, 6) + 1;
   for (i=0; i < x; ++i) {
      uint32_t z = get_bits(f, 16);
      if (z != 0) return error(f, VORBIS_invalid_setup);
   }
   f->floor_count = get_bits(f, 6)+1;
   f->floor_config = (Floor *) setup_malloc(f, f->floor_count * sizeof(*f->floor_config));
   if (f->floor_config == NULL) return error(f, VORBIS_outofmem);
   for (i=0; i < f->floor_count; ++i) {
      f->floor_types[i] = get_bits(f, 16);
      if (f->floor_types[i] > 1) return error(f, VORBIS_invalid_setup);
      if (f->floor_types[i] == 0) {
         Floor0 *g = &f->floor_config[i].floor0;
         g->order = get_bits(f,8);
         g->rate = get_bits(f,16);
         g->bark_map_size = get_bits(f,16);
         g->amplitude_bits = get_bits(f,6);
         g->amplitude_offset = get_bits(f,8);
         g->number_of_books = get_bits(f,4) + 1;
         for (j=0; j < g->number_of_books; ++j)
            g->book_list[j] = get_bits(f,8);
         return error(f, VORBIS_feature_not_supported);
      } else {
         stbv__floor_ordering p[31*8+2];
         Floor1 *g = &f->floor_config[i].floor1;
         int max_class = -1;
         g->partitions = get_bits(f, 5);
         for (j=0; j < g->partitions; ++j) {
            g->partition_class_list[j] = get_bits(f, 4);
            if (g->partition_class_list[j] > max_class)
               max_class = g->partition_class_list[j];
         }
         for (j=0; j <= max_class; ++j) {
            g->class_dimensions[j] = get_bits(f, 3)+1;
            g->class_subclasses[j] = get_bits(f, 2);
            if (g->class_subclasses[j]) {
               g->class_masterbooks[j] = get_bits(f, 8);
               if (g->class_masterbooks[j] >= f->codebook_count) return error(f, VORBIS_invalid_setup);
            }
            for (k=0; k < 1 << g->class_subclasses[j]; ++k) {
               g->subclass_books[j][k] = (int16_t)get_bits(f,8)-1;
               if (g->subclass_books[j][k] >= f->codebook_count) return error(f, VORBIS_invalid_setup);
            }
         }
         g->floor1_multiplier = get_bits(f,2)+1;
         g->rangebits = get_bits(f,4);
         g->Xlist[0] = 0;
         g->Xlist[1] = 1 << g->rangebits;
         g->values = 2;
         for (j=0; j < g->partitions; ++j) {
            int c = g->partition_class_list[j];
            for (k=0; k < g->class_dimensions[c]; ++k) {
               g->Xlist[g->values] = get_bits(f, g->rangebits);
               ++g->values;
            }
         }
         for (j=0; j < g->values; ++j) {
            p[j].x = g->Xlist[j];
            p[j].id = j;
         }
         qsort(p, g->values, sizeof(p[0]), point_compare);
         for (j=0; j < g->values-1; ++j)
            if (p[j].x == p[j+1].x)
               return error(f, VORBIS_invalid_setup);
         for (j=0; j < g->values; ++j)
            g->sorted_order[j] = (uint8_t) p[j].id;
         for (j=2; j < g->values; ++j) {
            int low = 0,hi = 0;
            neighbors(g->Xlist, j, &low,&hi);
            g->neighbors[j][0] = low;
            g->neighbors[j][1] = hi;
         }
         if (g->values > longest_floorlist)
            longest_floorlist = g->values;
      }
   }
   f->residue_count = get_bits(f, 6)+1;
   f->residue_config = (Residue *) setup_malloc(f, f->residue_count * sizeof(f->residue_config[0]));
   if (f->residue_config == NULL) return error(f, VORBIS_outofmem);
   memset(f->residue_config, 0, f->residue_count * sizeof(f->residue_config[0]));
   for (i=0; i < f->residue_count; ++i) {
      uint8_t residue_cascade[64];
      Residue *r = f->residue_config+i;
      f->residue_types[i] = get_bits(f, 16);
      if (f->residue_types[i] > 2) return error(f, VORBIS_invalid_setup);
      r->begin = get_bits(f, 24);
      r->end = get_bits(f, 24);
      if (r->end < r->begin) return error(f, VORBIS_invalid_setup);
      r->part_size = get_bits(f,24)+1;
      r->classifications = get_bits(f,6)+1;
      r->classbook = get_bits(f,8);
      if (r->classbook >= f->codebook_count) return error(f, VORBIS_invalid_setup);
      for (j=0; j < r->classifications; ++j) {
         uint8_t high_bits=0;
         uint8_t low_bits=get_bits(f,3);
         if (get_bits(f,1))
            high_bits = get_bits(f,5);
         residue_cascade[j] = high_bits*8 + low_bits;
      }
      r->residue_books = (short (*)[8]) setup_malloc(f, sizeof(r->residue_books[0]) * r->classifications);
      if (r->residue_books == NULL) return error(f, VORBIS_outofmem);
      for (j=0; j < r->classifications; ++j) {
         for (k=0; k < 8; ++k) {
            if (residue_cascade[j] & (1 << k)) {
               r->residue_books[j][k] = get_bits(f, 8);
               if (r->residue_books[j][k] >= f->codebook_count) return error(f, VORBIS_invalid_setup);
            } else {
               r->residue_books[j][k] = -1;
            }
         }
      }
      r->classdata = (uint8_t **) setup_malloc(f, sizeof(*r->classdata) * f->codebooks[r->classbook].entries);
      if (!r->classdata) return error(f, VORBIS_outofmem);
      memset(r->classdata, 0, sizeof(*r->classdata) * f->codebooks[r->classbook].entries);
      for (j=0; j < f->codebooks[r->classbook].entries; ++j) {
         int classwords = f->codebooks[r->classbook].dimensions;
         int temp = j;
         r->classdata[j] = (uint8_t *) setup_malloc(f, sizeof(r->classdata[j][0]) * classwords);
         if (r->classdata[j] == NULL) return error(f, VORBIS_outofmem);
         for (k=classwords-1; k >= 0; --k) {
            r->classdata[j][k] = temp % r->classifications;
            temp /= r->classifications;
         }
      }
   }
   f->mapping_count = get_bits(f,6)+1;
   f->mapping = (Mapping *) setup_malloc(f, f->mapping_count * sizeof(*f->mapping));
   if (f->mapping == NULL) return error(f, VORBIS_outofmem);
   memset(f->mapping, 0, f->mapping_count * sizeof(*f->mapping));
   for (i=0; i < f->mapping_count; ++i) {
      Mapping *m = f->mapping + i;
      int mapping_type = get_bits(f,16);
      if (mapping_type != 0) return error(f, VORBIS_invalid_setup);
      m->chan = (MappingChannel *) setup_malloc(f, f->channels * sizeof(*m->chan));
      if (m->chan == NULL) return error(f, VORBIS_outofmem);
      if (get_bits(f,1))
         m->submaps = get_bits(f,4)+1;
      else
         m->submaps = 1;
      if (m->submaps > max_submaps)
         max_submaps = m->submaps;
      if (get_bits(f,1)) {
         m->coupling_steps = get_bits(f,8)+1;
         if (m->coupling_steps > f->channels) return error(f, VORBIS_invalid_setup);
         for (k=0; k < m->coupling_steps; ++k) {
            m->chan[k].magnitude = get_bits(f, ilog(f->channels-1));
            m->chan[k].angle = get_bits(f, ilog(f->channels-1));
            if (m->chan[k].magnitude >= f->channels) return error(f, VORBIS_invalid_setup);
            if (m->chan[k].angle >= f->channels) return error(f, VORBIS_invalid_setup);
            if (m->chan[k].magnitude == m->chan[k].angle) return error(f, VORBIS_invalid_setup);
         }
      } else
         m->coupling_steps = 0;
      if (get_bits(f,2)) return error(f, VORBIS_invalid_setup);
      if (m->submaps > 1) {
         for (j=0; j < f->channels; ++j) {
            m->chan[j].mux = get_bits(f, 4);
            if (m->chan[j].mux >= m->submaps) return error(f, VORBIS_invalid_setup);
         }
      } else
         for (j=0; j < f->channels; ++j)
            m->chan[j].mux = 0;
      for (j=0; j < m->submaps; ++j) {
         get_bits(f,8);
         m->submap_floor[j] = get_bits(f,8);
         m->submap_residue[j] = get_bits(f,8);
         if (m->submap_floor[j] >= f->floor_count) return error(f, VORBIS_invalid_setup);
         if (m->submap_residue[j] >= f->residue_count) return error(f, VORBIS_invalid_setup);
      }
   }
   f->mode_count = get_bits(f, 6)+1;
   for (i=0; i < f->mode_count; ++i) {
      Mode *m = f->mode_config+i;
      m->blockflag = get_bits(f,1);
      m->windowtype = get_bits(f,16);
      m->transformtype = get_bits(f,16);
      m->mapping = get_bits(f,8);
      if (m->windowtype != 0) return error(f, VORBIS_invalid_setup);
      if (m->transformtype != 0) return error(f, VORBIS_invalid_setup);
      if (m->mapping >= f->mapping_count) return error(f, VORBIS_invalid_setup);
   }
   flush_packet(f);
   f->previous_length = 0;
   for (i=0; i < f->channels; ++i) {
      f->channel_buffers[i] = (float *) setup_malloc(f, sizeof(float) * f->blocksize_1);
      f->previous_window[i] = (float *) setup_malloc(f, sizeof(float) * f->blocksize_1/2);
      f->finalY[i] = (int16_t *) setup_malloc(f, sizeof(int16_t) * longest_floorlist);
      if (f->channel_buffers[i] == NULL || f->previous_window[i] == NULL || f->finalY[i] == NULL) return error(f, VORBIS_outofmem);
      memset(f->channel_buffers[i], 0, sizeof(float) * f->blocksize_1);
      #ifdef STB_VORBIS_NO_DEFER_FLOOR
      f->floor_buffers[i] = (float *) setup_malloc(f, sizeof(float) * f->blocksize_1/2);
      if (f->floor_buffers[i] == NULL) return error(f, VORBIS_outofmem);
      #endif
   }
   if (!init_blocksize(f, 0, f->blocksize_0)) return FALSE;
   if (!init_blocksize(f, 1, f->blocksize_1)) return FALSE;
   f->blocksize[0] = f->blocksize_0;
   f->blocksize[1] = f->blocksize_1;
#ifdef STB_VORBIS_DIVIDE_TABLE
   if (integer_divide_table[1][1]==0)
      for (i=0; i < DIVTAB_NUMER; ++i)
         for (j=1; j < DIVTAB_DENOM; ++j)
            integer_divide_table[i][j] = i / j;
#endif
   {
      uint32_t imdct_mem = (f->blocksize_1 * sizeof(float) >> 1);
      uint32_t classify_mem;
      int i,max_part_read=0;
      for (i=0; i < f->residue_count; ++i) {
         Residue *r = f->residue_config + i;
         unsigned int actual_size = f->blocksize_1 / 2;
         unsigned int limit_r_begin = r->begin < actual_size ? r->begin : actual_size;
         unsigned int limit_r_end = r->end < actual_size ? r->end : actual_size;
         int n_read = limit_r_end - limit_r_begin;
         int part_read = n_read / r->part_size;
         if (part_read > max_part_read)
            max_part_read = part_read;
      }
      #ifndef STB_VORBIS_DIVIDES_IN_RESIDUE
      classify_mem = f->channels * (sizeof(void*) + max_part_read * sizeof(uint8_t *));
      #else
      classify_mem = f->channels * (sizeof(void*) + max_part_read * sizeof(int *));
      #endif
      f->temp_memory_required = classify_mem;
      if (imdct_mem > f->temp_memory_required)
         f->temp_memory_required = imdct_mem;
   }
   if (f->alloc.alloc_buffer) {
      assert(f->temp_offset == f->alloc.alloc_buffer_length_in_bytes);
      if (f->setup_offset + sizeof(*f) + f->temp_memory_required > (unsigned) f->temp_offset)
         return error(f, VORBIS_outofmem);
   }
   if (f->next_seg == -1) {
      f->first_audio_page_offset = stb_vorbis_get_file_offset(f);
   } else {
      f->first_audio_page_offset = 0;
   }
   return TRUE;
}
static void vorbis_deinit(stb_vorbis *p)
{
   int i,j;
   setup_free(p, p->vendor);
   for (i=0; i < p->comment_list_length; ++i) {
      setup_free(p, p->comment_list[i]);
   }
   setup_free(p, p->comment_list);
   if (p->residue_config) {
      for (i=0; i < p->residue_count; ++i) {
         Residue *r = p->residue_config+i;
         if (r->classdata) {
            for (j=0; j < p->codebooks[r->classbook].entries; ++j)
               setup_free(p, r->classdata[j]);
            setup_free(p, r->classdata);
         }
         setup_free(p, r->residue_books);
      }
   }
   if (p->codebooks) {
      CHECK(p);
      for (i=0; i < p->codebook_count; ++i) {
         Codebook *c = p->codebooks + i;
         setup_free(p, c->codeword_lengths);
         setup_free(p, c->multiplicands);
         setup_free(p, c->codewords);
         setup_free(p, c->sorted_codewords);
         setup_free(p, c->sorted_values ? c->sorted_values-1 : NULL);
      }
      setup_free(p, p->codebooks);
   }
   setup_free(p, p->floor_config);
   setup_free(p, p->residue_config);
   if (p->mapping) {
      for (i=0; i < p->mapping_count; ++i)
         setup_free(p, p->mapping[i].chan);
      setup_free(p, p->mapping);
   }
   CHECK(p);
   for (i=0; i < p->channels && i < STB_VORBIS_MAX_CHANNELS; ++i) {
      setup_free(p, p->channel_buffers[i]);
      setup_free(p, p->previous_window[i]);
      #ifdef STB_VORBIS_NO_DEFER_FLOOR
      setup_free(p, p->floor_buffers[i]);
      #endif
      setup_free(p, p->finalY[i]);
   }
   for (i=0; i < 2; ++i) {
      setup_free(p, p->A[i]);
      setup_free(p, p->B[i]);
      setup_free(p, p->C[i]);
      setup_free(p, p->window[i]);
      setup_free(p, p->bit_reverse[i]);
   }
   #ifndef STB_VORBIS_NO_STDIO
   if (p->close_on_free) fclose(p->f);
   #endif
}
void stb_vorbis_close(stb_vorbis *p)
{
   if (p == NULL) return;
   vorbis_deinit(p);
   setup_free(p,p);
}
static void vorbis_init(stb_vorbis *p, const stb_vorbis_alloc *z)
{
   memset(p, 0, sizeof(*p));
   if (z) {
      p->alloc = *z;
      p->alloc.alloc_buffer_length_in_bytes &= ~7;
      p->temp_offset = p->alloc.alloc_buffer_length_in_bytes;
   }
   p->eof = 0;
   p->error = VORBIS__no_error;
   p->stream = NULL;
   p->codebooks = NULL;
   p->page_crc_tests = -1;
   #ifndef STB_VORBIS_NO_STDIO
   p->close_on_free = FALSE;
   p->f = NULL;
   #endif
}
int stb_vorbis_get_sample_offset(stb_vorbis *f)
{
   if (f->current_loc_valid)
      return f->current_loc;
   else
      return -1;
}
stb_vorbis_info stb_vorbis_get_info(stb_vorbis *f)
{
   stb_vorbis_info d;
   d.channels = f->channels;
   d.sample_rate = f->sample_rate;
   d.setup_memory_required = f->setup_memory_required;
   d.setup_temp_memory_required = f->setup_temp_memory_required;
   d.temp_memory_required = f->temp_memory_required;
   d.max_frame_size = f->blocksize_1 >> 1;
   return d;
}
stb_vorbis_comment stb_vorbis_get_comment(stb_vorbis *f)
{
   stb_vorbis_comment d;
   d.vendor = f->vendor;
   d.comment_list_length = f->comment_list_length;
   d.comment_list = f->comment_list;
   return d;
}
int stb_vorbis_get_error(stb_vorbis *f)
{
   int e = f->error;
   f->error = VORBIS__no_error;
   return e;
}
static stb_vorbis * vorbis_alloc(stb_vorbis *f)
{
   stb_vorbis *p = (stb_vorbis *) setup_malloc(f, sizeof(*p));
   return p;
}
unsigned int stb_vorbis_get_file_offset(stb_vorbis *f)
{
   #ifndef STB_VORBIS_NO_PUSHDATA_API
   if (f->push_mode) return 0;
   #endif
   if (USE_MEMORY(f)) return (unsigned int) (f->stream - f->stream_start);
   #ifndef STB_VORBIS_NO_STDIO
   return (unsigned int) (ftell(f->f) - f->f_start);
   #endif
}
static uint32_t vorbis_find_page(stb_vorbis *f, uint32_t *end, uint32_t *last)
{
   for(;;) {
      int n;
      if (f->eof) return 0;
      n = get8(f);
      if (n == 0x4f) {
         unsigned int retry_loc = stb_vorbis_get_file_offset(f);
         int i;
         if (retry_loc - 25 > f->stream_len)
            return 0;
         for (i=1; i < 4; ++i)
            if (get8(f) != ogg_page_header[i])
               break;
         if (f->eof) return 0;
         if (i == 4) {
            uint8_t header[27];
            uint32_t i, crc, goal, len;
            for (i=0; i < 4; ++i)
               header[i] = ogg_page_header[i];
            for (; i < 27; ++i)
               header[i] = get8(f);
            if (f->eof) return 0;
            if (header[4] != 0) goto invalid;
            goal = header[22] + (header[23] << 8) + (header[24]<<16) + ((uint32_t)header[25]<<24);
            for (i=22; i < 26; ++i)
               header[i] = 0;
            crc = 0;
            for (i=0; i < 27; ++i)
               crc = crc32_update(crc, header[i]);
            len = 0;
            for (i=0; i < header[26]; ++i) {
               int s = get8(f);
               crc = crc32_update(crc, s);
               len += s;
            }
            if (len && f->eof) return 0;
            for (i=0; i < len; ++i)
               crc = crc32_update(crc, get8(f));
            if (crc == goal) {
               if (end)
                  *end = stb_vorbis_get_file_offset(f);
               if (last) {
                  if (header[5] & 0x04)
                     *last = 1;
                  else
                     *last = 0;
               }
               set_file_offset(f, retry_loc-1);
               return 1;
            }
         }
        invalid:
         set_file_offset(f, retry_loc);
      }
   }
}
#define SAMPLE_unknown 0xffffffff
static int get_seek_page_info(stb_vorbis *f, ProbedPage *z)
{
   uint8_t header[27], lacing[255];
   int i,len;
   z->page_start = stb_vorbis_get_file_offset(f);
   getn(f, header, 27);
   if (header[0] != 'O' || header[1] != 'g' || header[2] != 'g' || header[3] != 'S')
      return 0;
   getn(f, lacing, header[26]);
   len = 0;
   for (i=0; i < header[26]; ++i)
      len += lacing[i];
   z->page_end = z->page_start + 27 + header[26] + len;
   z->last_decoded_sample = header[6] + (header[7] << 8) + (header[8] << 16) + (header[9] << 24);
   set_file_offset(f, z->page_start);
   return 1;
}
static int go_to_page_before(stb_vorbis *f, unsigned int limit_offset)
{
   unsigned int previous_safe, end;
   if (limit_offset >= 65536 && limit_offset-65536 >= f->first_audio_page_offset)
      previous_safe = limit_offset - 65536;
   else
      previous_safe = f->first_audio_page_offset;
   set_file_offset(f, previous_safe);
   while (vorbis_find_page(f, &end, NULL)) {
      if (end >= limit_offset && stb_vorbis_get_file_offset(f) < limit_offset)
         return 1;
      set_file_offset(f, end);
   }
   return 0;
}
static int seek_to_sample_coarse(stb_vorbis *f, uint32_t sample_number)
{
   ProbedPage left, right, mid;
   int i, start_seg_with_known_loc, end_pos, page_start;
   uint32_t delta, stream_length, padding, last_sample_limit;
   double offset = 0.0, bytes_per_sample = 0.0;
   int probe = 0;
   stream_length = stb_vorbis_stream_length_in_samples(f);
   if (stream_length == 0) return error(f, VORBIS_seek_without_length);
   if (sample_number > stream_length) return error(f, VORBIS_seek_invalid);
   padding = ((f->blocksize_1 - f->blocksize_0) >> 2);
   if (sample_number < padding)
      last_sample_limit = 0;
   else
      last_sample_limit = sample_number - padding;
   left = f->p_first;
   while (left.last_decoded_sample == ~0U) {
      set_file_offset(f, left.page_end);
      if (!get_seek_page_info(f, &left)) goto error;
   }
   right = f->p_last;
   assert(right.last_decoded_sample != ~0U);
   if (last_sample_limit <= left.last_decoded_sample) {
      if (stb_vorbis_seek_start(f)) {
         if (f->current_loc > sample_number)
            return error(f, VORBIS_seek_failed);
         return 1;
      }
      return 0;
   }
   while (left.page_end != right.page_start) {
      assert(left.page_end < right.page_start);
      delta = right.page_start - left.page_end;
      if (delta <= 65536) {
         set_file_offset(f, left.page_end);
      } else {
         if (probe < 2) {
            if (probe == 0) {
               double data_bytes = right.page_end - left.page_start;
               bytes_per_sample = data_bytes / right.last_decoded_sample;
               offset = left.page_start + bytes_per_sample * (last_sample_limit - left.last_decoded_sample);
            } else {
               double error = ((double) last_sample_limit - mid.last_decoded_sample) * bytes_per_sample;
               if (error >= 0 && error < 8000) error = 8000;
               if (error < 0 && error > -8000) error = -8000;
               offset += error * 2;
            }
            if (offset < left.page_end)
               offset = left.page_end;
            if (offset > right.page_start - 65536)
               offset = right.page_start - 65536;
            set_file_offset(f, (unsigned int) offset);
         } else {
            set_file_offset(f, left.page_end + (delta / 2) - 32768);
         }
         if (!vorbis_find_page(f, NULL, NULL)) goto error;
      }
      for (;;) {
         if (!get_seek_page_info(f, &mid)) goto error;
         if (mid.last_decoded_sample != ~0U) break;
         set_file_offset(f, mid.page_end);
         assert(mid.page_start < right.page_start);
      }
      if (mid.page_start == right.page_start) {
         if (probe >= 2 || delta <= 65536)
            break;
      } else {
         if (last_sample_limit < mid.last_decoded_sample)
            right = mid;
         else
            left = mid;
      }
      ++probe;
   }
   page_start = left.page_start;
   set_file_offset(f, page_start);
   if (!start_page(f)) return error(f, VORBIS_seek_failed);
   end_pos = f->end_seg_with_known_loc;
   assert(end_pos >= 0);
   for (;;) {
      for (i = end_pos; i > 0; --i)
         if (f->segments[i-1] != 255)
            break;
      start_seg_with_known_loc = i;
      if (start_seg_with_known_loc > 0 || !(f->page_flag & PAGEFLAG_continued_packet))
         break;
      if (!go_to_page_before(f, page_start))
         goto error;
      page_start = stb_vorbis_get_file_offset(f);
      if (!start_page(f)) goto error;
      end_pos = f->segment_count - 1;
   }
   f->current_loc_valid = FALSE;
   f->last_seg = FALSE;
   f->valid_bits = 0;
   f->packet_bytes = 0;
   f->bytes_in_seg = 0;
   f->previous_length = 0;
   f->next_seg = start_seg_with_known_loc;
   for (i = 0; i < start_seg_with_known_loc; i++)
      skip(f, f->segments[i]);
   if (!vorbis_pump_first_frame(f))
      return 0;
   if (f->current_loc > sample_number)
      return error(f, VORBIS_seek_failed);
   return 1;
error:
   stb_vorbis_seek_start(f);
   return error(f, VORBIS_seek_failed);
}
static int peek_decode_initial(vorb *f, int *p_left_start, int *p_left_end, int *p_right_start, int *p_right_end, int *mode)
{
   int bits_read, bytes_read;
   if (!vorbis_decode_initial(f, p_left_start, p_left_end, p_right_start, p_right_end, mode))
      return 0;
   bits_read = 1 + ilog(f->mode_count-1);
   if (f->mode_config[*mode].blockflag)
      bits_read += 2;
   bytes_read = (bits_read + 7) / 8;
   f->bytes_in_seg += bytes_read;
   f->packet_bytes -= bytes_read;
   skip(f, -bytes_read);
   if (f->next_seg == -1)
      f->next_seg = f->segment_count - 1;
   else
      f->next_seg--;
   f->valid_bits = 0;
   return 1;
}
int stb_vorbis_seek_frame(stb_vorbis *f, unsigned int sample_number)
{
   uint32_t max_frame_samples;
   if (IS_PUSH_MODE(f)) return error(f, VORBIS_invalid_api_mixing);
   if (!seek_to_sample_coarse(f, sample_number))
      return 0;
   assert(f->current_loc_valid);
   assert(f->current_loc <= sample_number);
   max_frame_samples = (f->blocksize_1*3 - f->blocksize_0) >> 2;
   while (f->current_loc < sample_number) {
      int left_start, left_end, right_start, right_end, mode, frame_samples;
      if (!peek_decode_initial(f, &left_start, &left_end, &right_start, &right_end, &mode))
         return error(f, VORBIS_seek_failed);
      frame_samples = right_start - left_start;
      if (f->current_loc + frame_samples > sample_number) {
         return 1;
      } else if (f->current_loc + frame_samples + max_frame_samples > sample_number) {
         vorbis_pump_first_frame(f);
      } else {
         f->current_loc += frame_samples;
         f->previous_length = 0;
         maybe_start_packet(f);
         flush_packet(f);
      }
   }
   if (f->current_loc != sample_number) return error(f, VORBIS_seek_failed);
   return 1;
}
int stb_vorbis_seek(stb_vorbis *f, unsigned int sample_number)
{
   if (!stb_vorbis_seek_frame(f, sample_number))
      return 0;
   if (sample_number != f->current_loc) {
      int n;
      uint32_t frame_start = f->current_loc;
      stb_vorbis_get_frame_float(f, &n, NULL);
      assert(sample_number > frame_start);
      assert(f->channel_buffer_start + (int) (sample_number-frame_start) <= f->channel_buffer_end);
      f->channel_buffer_start += (sample_number - frame_start);
   }
   return 1;
}
int stb_vorbis_seek_start(stb_vorbis *f)
{
   if (IS_PUSH_MODE(f)) { return error(f, VORBIS_invalid_api_mixing); }
   set_file_offset(f, f->first_audio_page_offset);
   f->previous_length = 0;
   f->first_decode = TRUE;
   f->next_seg = -1;
   return vorbis_pump_first_frame(f);
}
unsigned int stb_vorbis_stream_length_in_samples(stb_vorbis *f)
{
   unsigned int restore_offset, previous_safe;
   unsigned int end, last_page_loc;
   if (IS_PUSH_MODE(f)) return error(f, VORBIS_invalid_api_mixing);
   if (!f->total_samples) {
      unsigned int last;
      uint32_t lo,hi;
      char header[6];
      restore_offset = stb_vorbis_get_file_offset(f);
      if (f->stream_len >= 65536 && f->stream_len-65536 >= f->first_audio_page_offset)
         previous_safe = f->stream_len - 65536;
      else
         previous_safe = f->first_audio_page_offset;
      set_file_offset(f, previous_safe);
      if (!vorbis_find_page(f, &end, &last)) {
         f->error = VORBIS_cant_find_last_page;
         f->total_samples = 0xffffffff;
         goto done;
      }
      last_page_loc = stb_vorbis_get_file_offset(f);
      while (!last) {
         set_file_offset(f, end);
         if (!vorbis_find_page(f, &end, &last)) {
            break;
         }
         last_page_loc = stb_vorbis_get_file_offset(f);
      }
      set_file_offset(f, last_page_loc);
      getn(f, (unsigned char *)header, 6);
      lo = get32(f);
      hi = get32(f);
      if (lo == 0xffffffff && hi == 0xffffffff) {
         f->error = VORBIS_cant_find_last_page;
         f->total_samples = SAMPLE_unknown;
         goto done;
      }
      if (hi)
         lo = 0xfffffffe;
      f->total_samples = lo;
      f->p_last.page_start = last_page_loc;
      f->p_last.page_end = end;
      f->p_last.last_decoded_sample = lo;
     done:
      set_file_offset(f, restore_offset);
   }
   return f->total_samples == SAMPLE_unknown ? 0 : f->total_samples;
}
float stb_vorbis_stream_length_in_seconds(stb_vorbis *f)
{
   return stb_vorbis_stream_length_in_samples(f) / (float) f->sample_rate;
}
int stb_vorbis_get_frame_float(stb_vorbis *f, int *channels, float ***output)
{
   int len, right,left,i;
   if (IS_PUSH_MODE(f)) return error(f, VORBIS_invalid_api_mixing);
   if (!vorbis_decode_packet(f, &len, &left, &right)) {
      f->channel_buffer_start = f->channel_buffer_end = 0;
      return 0;
   }
   len = vorbis_finish_frame(f, len, left, right);
   for (i=0; i < f->channels; ++i)
      f->outputs[i] = f->channel_buffers[i] + left;
   f->channel_buffer_start = left;
   f->channel_buffer_end = left+len;
   if (channels) *channels = f->channels;
   if (output) *output = f->outputs;
   return len;
}
stb_vorbis * stb_vorbis_open_file_section(FILE *file, int close_on_free, int *error, const stb_vorbis_alloc *alloc, unsigned int length)
{
   stb_vorbis *f, p;
   vorbis_init(&p, alloc);
   p.f = file;
   p.f_start = (uint32_t) ftell(file);
   p.stream_len = length;
   p.close_on_free = close_on_free;
   if (start_decoder(&p)) {
      f = vorbis_alloc(&p);
      if (f) {
         *f = p;
         vorbis_pump_first_frame(f);
         return f;
      }
   }
   if (error) *error = p.error;
   vorbis_deinit(&p);
   return NULL;
}
stb_vorbis * stb_vorbis_open_file(FILE *file, int close_on_free, int *error, const stb_vorbis_alloc *alloc)
{
   unsigned int len, start;
   start = (unsigned int) ftell(file);
   fseek(file, 0, SEEK_END);
   len = (unsigned int) (ftell(file) - start);
   fseek(file, start, SEEK_SET);
   return stb_vorbis_open_file_section(file, close_on_free, error, alloc, len);
}
stb_vorbis * stb_vorbis_open_filename(const char *filename, int *error, const stb_vorbis_alloc *alloc)
{
   FILE *f;
   f = fopen(filename, "rb");
   if (f)
      return stb_vorbis_open_file(f, TRUE, error, alloc);
   if (error) *error = VORBIS_file_open_failure;
   return NULL;
}
stb_vorbis * stb_vorbis_open_memory(const unsigned char *data, int len, int *error, const stb_vorbis_alloc *alloc)
{
   stb_vorbis *f, p;
   if (!data) {
      if (error) *error = VORBIS_unexpected_eof;
      return NULL;
   }
   vorbis_init(&p, alloc);
   p.stream = (uint8_t *) data;
   p.stream_end = (uint8_t *) data + len;
   p.stream_start = (uint8_t *) p.stream;
   p.stream_len = len;
   p.push_mode = FALSE;
   if (start_decoder(&p)) {
      f = vorbis_alloc(&p);
      if (f) {
         *f = p;
         vorbis_pump_first_frame(f);
         if (error) *error = VORBIS__no_error;
         return f;
      }
   }
   if (error) *error = p.error;
   vorbis_deinit(&p);
   return NULL;
}
#define PLAYBACK_MONO 1
#define PLAYBACK_LEFT 2
#define PLAYBACK_RIGHT 4
#define L (PLAYBACK_LEFT | PLAYBACK_MONO)
#define C (PLAYBACK_LEFT | PLAYBACK_RIGHT | PLAYBACK_MONO)
#define R (PLAYBACK_RIGHT | PLAYBACK_MONO)
static int8_t channel_position[7][6] =
{
   { 0 },
   { C },
   { L, R },
   { L, C, R },
   { L, R, L, R },
   { L, C, R, L, R },
   { L, C, R, L, R, C },
};
#ifndef STB_VORBIS_NO_FAST_SCALED_FLOAT
   typedef union {
      float f;
      int i;
   } float_conv;
   typedef char stb_vorbis_float_size_test[sizeof(float)==4 && sizeof(int) == 4];
   #define FASTDEF(x) float_conv x
   #define MAGIC(SHIFT) (1.5f * (1 << (23-SHIFT)) + 0.5f/(1 << SHIFT))
   #define ADDEND(SHIFT) (((150-SHIFT) << 23) + (1 << 22))
   #define FAST_SCALED_FLOAT_TO_INT(temp,x,s) (temp.f = (x) + MAGIC(s), temp.i - ADDEND(s))
   #define check_endianness()
#else
   #define FAST_SCALED_FLOAT_TO_INT(temp,x,s) ((int) ((x) * (1 << (s))))
   #define check_endianness()
   #define FASTDEF(x)
#endif
static void copy_samples(short *dest, float *src, int len)
{
   int i;
   check_endianness();
   for (i=0; i < len; ++i) {
      FASTDEF(temp);
      int v = FAST_SCALED_FLOAT_TO_INT(temp, src[i],15);
      if ((unsigned int) (v + 32768) > 65535)
         v = v < 0 ? -32768 : 32767;
      dest[i] = v;
   }
}
static void compute_samples(int mask, short *output, int num_c, float **data, int d_offset, int len)
{
   #define STB_BUFFER_SIZE 32
   float buffer[STB_BUFFER_SIZE];
   int i,j,o,n = STB_BUFFER_SIZE;
   check_endianness();
   for (o = 0; o < len; o += STB_BUFFER_SIZE) {
      memset(buffer, 0, sizeof(buffer));
      if (o + n > len) n = len - o;
      for (j=0; j < num_c; ++j) {
         if (channel_position[num_c][j] & mask) {
            for (i=0; i < n; ++i)
               buffer[i] += data[j][d_offset+o+i];
         }
      }
      for (i=0; i < n; ++i) {
         FASTDEF(temp);
         int v = FAST_SCALED_FLOAT_TO_INT(temp,buffer[i],15);
         if ((unsigned int) (v + 32768) > 65535)
            v = v < 0 ? -32768 : 32767;
         output[o+i] = v;
      }
   }
   #undef STB_BUFFER_SIZE
}
static void compute_stereo_samples(short *output, int num_c, float **data, int d_offset, int len)
{
   #define STB_BUFFER_SIZE 32
   float buffer[STB_BUFFER_SIZE];
   int i,j,o,n = STB_BUFFER_SIZE >> 1;
   check_endianness();
   for (o = 0; o < len; o += STB_BUFFER_SIZE >> 1) {
      int o2 = o << 1;
      memset(buffer, 0, sizeof(buffer));
      if (o + n > len) n = len - o;
      for (j=0; j < num_c; ++j) {
         int m = channel_position[num_c][j] & (PLAYBACK_LEFT | PLAYBACK_RIGHT);
         if (m == (PLAYBACK_LEFT | PLAYBACK_RIGHT)) {
            for (i=0; i < n; ++i) {
               buffer[i*2+0] += data[j][d_offset+o+i];
               buffer[i*2+1] += data[j][d_offset+o+i];
            }
         } else if (m == PLAYBACK_LEFT) {
            for (i=0; i < n; ++i) {
               buffer[i*2+0] += data[j][d_offset+o+i];
            }
         } else if (m == PLAYBACK_RIGHT) {
            for (i=0; i < n; ++i) {
               buffer[i*2+1] += data[j][d_offset+o+i];
            }
         }
      }
      for (i=0; i < (n<<1); ++i) {
         FASTDEF(temp);
         int v = FAST_SCALED_FLOAT_TO_INT(temp,buffer[i],15);
         if ((unsigned int) (v + 32768) > 65535)
            v = v < 0 ? -32768 : 32767;
         output[o2+i] = v;
      }
   }
   #undef STB_BUFFER_SIZE
}
static void convert_samples_short(int buf_c, short **buffer, int b_offset, int data_c, float **data, int d_offset, int samples)
{
   int i;
   if (buf_c != data_c && buf_c <= 2 && data_c <= 6) {
      static int channel_selector[3][2] = { {0}, {PLAYBACK_MONO}, {PLAYBACK_LEFT, PLAYBACK_RIGHT} };
      for (i=0; i < buf_c; ++i)
         compute_samples(channel_selector[buf_c][i], buffer[i]+b_offset, data_c, data, d_offset, samples);
   } else {
      int limit = buf_c < data_c ? buf_c : data_c;
      for (i=0; i < limit; ++i)
         copy_samples(buffer[i]+b_offset, data[i]+d_offset, samples);
      for ( ; i < buf_c; ++i)
         memset(buffer[i]+b_offset, 0, sizeof(short) * samples);
   }
}
int stb_vorbis_get_frame_short(stb_vorbis *f, int num_c, short **buffer, int num_samples)
{
   float **output = NULL;
   int len = stb_vorbis_get_frame_float(f, NULL, &output);
   if (len > num_samples) len = num_samples;
   if (len)
      convert_samples_short(num_c, buffer, 0, f->channels, output, 0, len);
   return len;
}
static void convert_channels_short_interleaved(int buf_c, short *buffer, int data_c, float **data, int d_offset, int len)
{
   int i;
   check_endianness();
   if (buf_c != data_c && buf_c <= 2 && data_c <= 6) {
      assert(buf_c == 2);
      for (i=0; i < buf_c; ++i)
         compute_stereo_samples(buffer, data_c, data, d_offset, len);
   } else {
      int limit = buf_c < data_c ? buf_c : data_c;
      int j;
      for (j=0; j < len; ++j) {
         for (i=0; i < limit; ++i) {
            FASTDEF(temp);
            float f = data[i][d_offset+j];
            int v = FAST_SCALED_FLOAT_TO_INT(temp, f,15);
            if ((unsigned int) (v + 32768) > 65535)
               v = v < 0 ? -32768 : 32767;
            *buffer++ = v;
         }
         for ( ; i < buf_c; ++i)
            *buffer++ = 0;
      }
   }
}
int stb_vorbis_get_frame_short_interleaved(stb_vorbis *f, int num_c, short *buffer, int num_shorts)
{
   float **output;
   int len;
   if (num_c == 1) return stb_vorbis_get_frame_short(f,num_c,&buffer, num_shorts);
   len = stb_vorbis_get_frame_float(f, NULL, &output);
   if (len) {
      if (len*num_c > num_shorts) len = num_shorts / num_c;
      convert_channels_short_interleaved(num_c, buffer, f->channels, output, 0, len);
   }
   return len;
}
int stb_vorbis_get_samples_short_interleaved(stb_vorbis *f, int channels, short *buffer, int num_shorts)
{
   float **outputs;
   int len = num_shorts / channels;
   int n=0;
   while (n < len) {
      int k = f->channel_buffer_end - f->channel_buffer_start;
      if (n+k >= len) k = len - n;
      if (k)
         convert_channels_short_interleaved(channels, buffer, f->channels, f->channel_buffers, f->channel_buffer_start, k);
      buffer += k*channels;
      n += k;
      f->channel_buffer_start += k;
      if (n == len) break;
      if (!stb_vorbis_get_frame_float(f, NULL, &outputs)) break;
   }
   return n;
}
int stb_vorbis_get_samples_short(stb_vorbis *f, int channels, short **buffer, int len)
{
   float **outputs;
   int n=0;
   while (n < len) {
      int k = f->channel_buffer_end - f->channel_buffer_start;
      if (n+k >= len) k = len - n;
      if (k)
         convert_samples_short(channels, buffer, n, f->channels, f->channel_buffers, f->channel_buffer_start, k);
      n += k;
      f->channel_buffer_start += k;
      if (n == len) break;
      if (!stb_vorbis_get_frame_float(f, NULL, &outputs)) break;
   }
   return n;
}
int stb_vorbis_decode_filename(const char *filename, int *channels, int *sample_rate, short **output)
{
   int data_len, offset, total, limit, error;
   short *data;
   stb_vorbis *v = stb_vorbis_open_filename(filename, &error, NULL);
   if (v == NULL) return -1;
   limit = v->channels * 4096;
   *channels = v->channels;
   if (sample_rate)
      *sample_rate = v->sample_rate;
   offset = data_len = 0;
   total = limit;
   data = (short *) malloc(total * sizeof(*data));
   if (data == NULL) {
      stb_vorbis_close(v);
      return -2;
   }
   for (;;) {
      int n = stb_vorbis_get_frame_short_interleaved(v, v->channels, data+offset, total-offset);
      if (n == 0) break;
      data_len += n;
      offset += n * v->channels;
      if (offset + limit > total) {
         short *data2;
         total *= 2;
         data2 = (short *) realloc(data, total * sizeof(*data));
         if (data2 == NULL) {
            free(data);
            stb_vorbis_close(v);
            return -2;
         }
         data = data2;
      }
   }
   *output = data;
   stb_vorbis_close(v);
   return data_len;
}
int stb_vorbis_decode_memory(const uint8_t *mem, int len, int *channels, int *sample_rate, short **output)
{
   int data_len, offset, total, limit, error;
   short *data;
   stb_vorbis *v = stb_vorbis_open_memory(mem, len, &error, NULL);
   if (v == NULL) return -1;
   limit = v->channels * 4096;
   *channels = v->channels;
   if (sample_rate)
      *sample_rate = v->sample_rate;
   offset = data_len = 0;
   total = limit;
   data = (short *) malloc(total * sizeof(*data));
   if (data == NULL) {
      stb_vorbis_close(v);
      return -2;
   }
   for (;;) {
      int n = stb_vorbis_get_frame_short_interleaved(v, v->channels, data+offset, total-offset);
      if (n == 0) break;
      data_len += n;
      offset += n * v->channels;
      if (offset + limit > total) {
         short *data2;
         total *= 2;
         data2 = (short *) realloc(data, total * sizeof(*data));
         if (data2 == NULL) {
            free(data);
            stb_vorbis_close(v);
            return -2;
         }
         data = data2;
      }
   }
   *output = data;
   stb_vorbis_close(v);
   return data_len;
}
int stb_vorbis_get_samples_float_interleaved(stb_vorbis *f, int channels, float *buffer, int num_floats)
{
   float **outputs;
   int len = num_floats / channels;
   int n=0;
   int z = f->channels;
   if (z > channels) z = channels;
   while (n < len) {
      int i,j;
      int k = f->channel_buffer_end - f->channel_buffer_start;
      if (n+k >= len) k = len - n;
      for (j=0; j < k; ++j) {
         for (i=0; i < z; ++i)
            *buffer++ = f->channel_buffers[i][f->channel_buffer_start+j];
         for ( ; i < channels; ++i)
            *buffer++ = 0;
      }
      n += k;
      f->channel_buffer_start += k;
      if (n == len)
         break;
      if (!stb_vorbis_get_frame_float(f, NULL, &outputs))
         break;
   }
   return n;
}
int stb_vorbis_get_samples_float(stb_vorbis *f, int channels, float **buffer, int num_samples)
{
   float **outputs;
   int n=0;
   int z = f->channels;
   if (z > channels) z = channels;
   while (n < num_samples) {
      int i;
      int k = f->channel_buffer_end - f->channel_buffer_start;
      if (n+k >= num_samples) k = num_samples - n;
      if (k) {
         for (i=0; i < z; ++i)
            memcpy(buffer[i]+n, f->channel_buffers[i]+f->channel_buffer_start, sizeof(float)*k);
         for ( ; i < channels; ++i)
            memset(buffer[i]+n, 0, sizeof(float) * k);
      }
      n += k;
      f->channel_buffer_start += k;
      if (n == num_samples)
         break;
      if (!stb_vorbis_get_frame_float(f, NULL, &outputs))
         break;
   }
   return n;
}
#endif
NO_WARN_END

typedef struct {
	int sample_rate;
	int channels;
	size_t nframes;
	int16_t *samples;
} AudioFile;

// load an ogg vorbis file. returns false on failure.
// remember to call audio_file_delete
V_DECL bool audio_file_load(AudioFile *f, const char *filename) {
	// @TODO(eventually): wav
	(void)filename;
	memset(f, 0, sizeof *f);
#if V_OGG
	{
		int len = stb_vorbis_decode_filename(filename, &f->channels, &f->sample_rate, &f->samples);
		if (len < 0) {
			memset(f, 0, sizeof *f);
		} else {
			f->nframes = (size_t)len;
		}
	}
	return f->samples != NULL;
#else
	return false;
#endif
}

// duration in seconds
V_DECL double audio_file_duration(const AudioFile *f) {
	return (double)f->nframes / (double)f->sample_rate;
}

V_DECL void audio_file_delete(AudioFile *a) {
	free(a->samples);
	memset(a, 0, sizeof *a);
}

static int V_BMP_transparent_index = -1;

// make color table index i fully transparent when loading BMPs
V_DECL void image_bmp_set_transparent_index(int i) {
	V_BMP_transparent_index = i;
}

V_DECL Image image_load(const char *filename) {
	Image img = {0};
	char *dot = strchr(filename, '.');
	FILE *fp;
	if (!dot) {
		debug_print("Image file without extension: %s\n", filename);
		return img;
	}
	fp = fopen(filename, "rb");
	if (!fp) {
		debug_print("%s doesn't exist or can't be accessed.\n", filename);
		return img;
	}
	if (strcmp(dot, ".bmp") == 0) {
		char BM[2];
		fread(BM, 2, 1, fp);
		if (memcmp(BM, "BM", 2) == 0) {
			int width = 0, height = 0;
			uint32_t data_offset = 0;
			int16_t bit_count = 0;
			uint32_t compression = 0;
			uint32_t clrused = 0;
			uint32_t header_size = 0;
			long header_start;

			fseek(fp, 8, SEEK_CUR); // unimportant
			fread(&data_offset, 4, 1, fp);
			header_start = ftell(fp);
			fread(&header_size, 4, 1, fp);
			fread(&width, 4, 1, fp);
			fread(&height, 4, 1, fp);
			fseek(fp, 2, SEEK_CUR);
			fread(&bit_count, 2, 1, fp);
			fread(&compression, 4, 1, fp);
			fseek(fp, 12, SEEK_CUR);
			fread(&clrused, 4, 1, fp);
			if (compression == 3 && (bit_count == 16 || bit_count == 32)) { // BI_BITFIELDS
				// lol funny story: (my version of) gimp produces 24-bit bmps with compression BI_BITFIELDS,
				// which is not allowed. it itself can't even read those files, so i won't bother to
				size_t area = (size_t)width * (size_t)height;
				char *pixels_in = calloc(area, (size_t)bit_count / 8);
				uint32_t *pixels_out = (uint32_t *)calloc(area, 4);

				if (pixels_in && pixels_out) {
					uint32_t rmask = 0, gmask = 0, bmask = 0, amask = 0;
					int rbits, gbits, bbits, abits;
					int rshift, gshift, bshift, ashift;
					char *in, *end = pixels_in + area * ((size_t)bit_count / 8);
					uint32_t *out = pixels_out;

					fseek(fp, 20, SEEK_CUR);
					fread(&rmask, 4, 1, fp);
					fread(&gmask, 4, 1, fp);
					fread(&bmask, 4, 1, fp);
					fread(&amask, 4, 1, fp);
					rshift = bit_count_trailing_zeros64(rmask);
					gshift = bit_count_trailing_zeros64(gmask);
					bshift = bit_count_trailing_zeros64(bmask);
					ashift = bit_count_trailing_zeros64(amask);
					rbits = bit_popcount(rmask);
					gbits = bit_popcount(gmask);
					bbits = bit_popcount(bmask);
					abits = bit_popcount(amask);
					fseek(fp, (long)data_offset, SEEK_SET);
					fread(pixels_in, (size_t)bit_count / 8, area, fp);

					for (in = pixels_in; in != end; in += bit_count / 8, ++out) {
						uint32_t color = bit_count == 32 ? *(uint32_t *)in : *(uint16_t *)in;
						unsigned red   = (color & rmask) >> rshift;
						unsigned green = (color & gmask) >> gshift;
						unsigned blue  = (color & bmask) >> bshift;
						unsigned alpha = amask == 0 ? 255 // no alpha channel in image
									   : (color & amask) >> ashift;
						red   = rbits <= 8 ? (red   << (8 - rbits)) : (red   >> (rbits - 8));
						green = gbits <= 8 ? (green << (8 - gbits)) : (green >> (gbits - 8));
						blue  = bbits <= 8 ? (blue  << (8 - bbits)) : (blue  >> (bbits - 8));
						alpha = abits <= 8 ? (alpha << (8 - abits)) : (alpha >> (abits - 8));
						*out = red | green << 8 | blue << 16 | alpha << 24;
					}
					img.size = Vec2i(width, height);
					img.channels = 4;
					img.data = (uint8_t *)pixels_out;
				} else {
					free(pixels_out);
					debug_print("Not enough memory to hold %dx%d image %s.\n", width, height, filename);
				}
				free(pixels_in);
			} else if (compression == 0 && bit_count == 8 && clrused <= 256) {
				uint32_t bmi_colors[256] = {0};
				size_t area = (size_t)height * (size_t)width;
				uint8_t *data = (uint8_t *)calloc(1, 4 * area);
				uint32_t *image = (uint32_t *)data;
				uint8_t *color_indices = data + 3 * area;
				size_t i;
				fseek(fp, header_start + header_size, SEEK_SET);
				//printf("%x\n",co
				// read the color table
				fread(bmi_colors, sizeof *bmi_colors, clrused, fp);
				// read the image data
				fseek(fp, (long)data_offset, SEEK_SET);
				fread(color_indices, 1, area, fp);
				for (i = 0; i < area; ++i) {
					uint8_t index = color_indices[i];
					if (index == V_BMP_transparent_index)
						image[i] = 0;
					else
						image[i] = (byte_swap32(bmi_colors[index]) >> 8) | 0xff000000;
				}
				img.data = (uint8_t *)image;
				img.size = Vec2i(width, height);
				img.channels = 4;
			} else if (compression == 0 && (bit_count == 24 || bit_count == 32)) {
				size_t bytes = (size_t)height * (size_t)(width * (bit_count / 8));
				uint8_t *data = (uint8_t *)calloc(1, bytes);
				if (data) {
					fseek(fp, (long)data_offset, SEEK_SET);
					fread(data, 1, bytes, fp);
					img.size = Vec2i(width, height);
					img.channels = bit_count / 8;
					img.data = data;
				} else {
					debug_print("Not enough memory to hold %dx%d image %s.\n", width, height, filename);
				}
			} else {
				debug_print("%s is an unrecognized BMP format (not supported by this program).\n", filename);
			}
		} else {
			debug_print("%s is not a BMP file.\n", filename);
		}
	}
	#if V_PNG
	else if (strcmp(dot, ".png") == 0) {
		int width = 0, height = 0, channels = 0;
		unsigned char *data = stbi_load_from_file(fp, &width, &height, &channels, 0);
		if (data) {
			// flip
			stbi__vertical_flip(data, width, height, channels);
			img.data = data;
			img.size = Vec2i(width, height);
			img.channels = channels;
		} else {
			debug_print("Couldn't load PNG image %s.\n", filename);
		}
	}
	#endif
	else {
	
		assert(0);
	}
	fclose(fp);
	return img;
}

#if V_PNG
V_DECL Image16 image16_load(const char *filename) {
	Image16 img = {0};
	uint16_t *data;
	int w = 0, h = 0, c = 0;

	data = stbi_load_16(filename, &w, &h, &c, 0);

	if (data) {
		img.data = data;
		img.size = Vec2i(w, h);
		img.channels = c;
	} else {
		debug_print("Couldn't load PNG image %s.\n", filename);
	}

	return img;
}
#endif

V_DECL vec4 image_get_pixel(const Image *image, int x, int y) {
	const uint8_t *p = &image->data[image->channels * (y * image->size.x + x)];
	vec4i icolor = {0};
	assert(x >= 0 && y >= 0 && x < image->size.x && y < image->size.y);

	switch (image->channels) {
	case 1:
		icolor.x = p[0];
		icolor.y = p[0];
		icolor.z = p[0];
		icolor.w = 255;
		break;
	case 3:
		icolor.x = p[0];
		icolor.y = p[1];
		icolor.z = p[2];
		icolor.w = 255;
		break;
	case 4:
		icolor.x = p[0];
		icolor.y = p[1];
		icolor.z = p[2];
		icolor.w = p[3];
		break;
	default: assert(0); break;
	}
	return scale4(vec4_from_4i(icolor), 1.0f / 255.0f);
}

// get pixel at (x, y) with linear blending, clamping to edges
V_DECL vec4 image_get_pixel_linear(const Image *image, float x, float y) {
	x = clamp(x, 0, (float)image->size.x - .01f);
	y = clamp(y, 0, (float)image->size.y - .01f);
	
	{
	int ix1 = (int)x;
	int iy1 = (int)y;
	float fx = x - (float)ix1, fy = y - (float)iy1;
	int ix2 = mini(ix1 + 1, image->size.x-1);
	int iy2 = mini(iy1 + 1, image->size.y-1);
	vec4 s1 = image_get_pixel(image, ix1, iy1);
	vec4 s2 = image_get_pixel(image, ix2, iy1);
	vec4 s3 = image_get_pixel(image, ix1, iy2);
	vec4 s4 = image_get_pixel(image, ix2, iy2);
	vec4 t1 = lerp4(s1, s2, fx);
	vec4 t2 = lerp4(s3, s4, fx);
	return lerp4(t1, t2, fy);
	}

}

V_DECL vec4 image16_get_pixel(const Image16 *image, int x, int y) {
	const uint16_t *p = &image->data[image->channels * (y * image->size.x + x)];
	vec4i icolor = {0};
	assert(x >= 0 && y >= 0 && x < image->size.x && y < image->size.y);

	switch (image->channels) {
	case 1:
		icolor.x = p[0];
		icolor.y = p[0];
		icolor.z = p[0];
		icolor.w = 65535;
		break;
	case 3:
		icolor.x = p[0];
		icolor.y = p[1];
		icolor.z = p[2];
		icolor.w = 65535;
		break;
	case 4:
		icolor.x = p[0];
		icolor.y = p[1];
		icolor.z = p[2];
		icolor.w = p[3];
		break;
	default: assert(0); break;
	}
	return scale4(vec4_from_4i(icolor), 1.0f / 65535.0f);
}

// get pixel at (x, y) with linear blending, clamping to edges
V_DECL vec4 image16_get_pixel_linear(const Image16 *image, float x, float y) {
	x = clamp(x, 0, (float)image->size.x - .01f);
	y = clamp(y, 0, (float)image->size.y - .01f);
	
	{
	int ix1 = (int)x;
	int iy1 = (int)y;
	float fx = x - (float)ix1, fy = y - (float)iy1;
	int ix2 = mini(ix1 + 1, image->size.x-1);
	int iy2 = mini(iy1 + 1, image->size.y-1);
	vec4 s1 = image16_get_pixel(image, ix1, iy1);
	vec4 s2 = image16_get_pixel(image, ix2, iy1);
	vec4 s3 = image16_get_pixel(image, ix1, iy2);
	vec4 s4 = image16_get_pixel(image, ix2, iy2);
	vec4 t1 = lerp4(s1, s2, fx);
	vec4 t2 = lerp4(s3, s4, fx);
	return lerp4(t1, t2, fy);
	}

}

V_DECL void image_delete(Image *image) {
	free(image->data);
	memset(image, 0, sizeof *image);
}

V_DECL void image16_delete(Image16 *image) {
	free(image->data);
	memset(image, 0, sizeof *image);
}

// --- RANDOM - NOT CRYPTOGRAPHICALLY SECURE ---

V_DECL void srand_randomly(void) {
	unsigned x;
	get_random(&x, sizeof x);
	srand(x);
}

V_DECL float randf(void) {
	float p = (1.0f / 32768.0f);
	return p * (float)(rand() & 0x7fff)
		+  p * p * (float)(rand() & 0x7fff);
}

V_DECL uint32_t rand_u32(void) {
	return (uint32_t)(rand() & 0x7fff)
		| ((uint32_t)(rand() & 0x7fff)) << 15
		| ((uint32_t)(rand() & 0x3)) << 30;
}

V_DECL uint64_t rand_u64(void) {
	return (uint64_t)(rand() & 0x7fff)
		| ((uint64_t)(rand() & 0x7fff)) << 15
		| ((uint64_t)(rand() & 0x7fff)) << 30
		| ((uint64_t)(rand() & 0x7fff)) << 45
		| ((uint64_t)(rand() & 0xf)) << 60;
}

V_DECL int rand_int(int up_to) {
	unsigned u = (unsigned)up_to;
	uint32_t reject_past, n;
	
	assert(up_to > 0);
	
	reject_past = 0xffffffff - 0xffffffff % u;
	do
		n = rand_u32();
	while (n > reject_past);
	return (int)(n % u);
}

V_DECL float rand_normal(void) {
	// https://en.wikipedia.org/wiki/Normal_distribution#Generating_values_from_normal_distribution
	float U, V;
	do
		U = randf(), V = randf();
	while (U == 0 || V == 0);
	return sqrtf(-2 * logf(U)) * cosf(2 * PI * V);
}

V_DECL vec2 rand_vec2(void) {
	return Vec2(randf(), randf());
}

V_DECL vec3 rand_vec3(void) {
	return Vec3(randf(), randf(), randf());
}

V_DECL vec4 rand_vec4(void) {
	return Vec4(randf(), randf(), randf(), randf());
}

V_DECL vec2 rand_unit_vec2(void) {
	float theta = 2 * PI * randf();
	return Vec2(cosf(theta), sinf(theta));
}

V_DECL vec3 rand_unit_vec3(void) {
	vec3 v;
	do
		v = addc3(scale3(rand_vec3(),2),-1);
	while (sqlength3(v) > 1);
	return normalize3(v);
}

V_DECL vec4 rand_unit_vec4(void) {
	vec4 v;
	do
		v = addc4(scale4(rand_vec4(),2),-1);
	while (sqlength4(v) > 1);
	return normalize4(v);
}

static uint64_t str_hash(const char *s) {
	const uint8_t *p;
	uint64_t hash;

	hash = 18392852384417039532u;
	for (p = (const uint8_t *)s; *p; ++p) {
		hash *= 6364136223846793005u;
		hash += 1442695040888963407u * (*p);
	}
	return hash;
}


static uint64_t V_gen_rand(Generator *g) {
	// musl libc uses these numbers so they're probably good
	// (see https://git.musl-libc.org/cgit/musl/tree/src/prng/rand.c)
    g->seed = 6364136223846793005 * g->seed + 1;
    return g->seed >> 32;
}

V_DECL Generator generator_new(uint64_t seed) {
	Generator g;
	g.seed = seed;
	V_gen_rand(&g); // make it a bit more random
	return g;
}

V_DECL Generator generator_new_with_random_seed(void) {
	uint64_t seed;
	get_random(&seed, 8);
	return generator_new(seed);
}

V_DECL uint32_t gen_rand_u32(Generator *g) {
	return (uint32_t)V_gen_rand(g);
}

V_DECL uint64_t gen_rand_u64(Generator *g) {
	return V_gen_rand(g) | V_gen_rand(g) << 32;
}

V_DECL float gen_randf(Generator *g) {
	return (float)V_gen_rand(g) * (1.0f / 4294967296.0f);
}

V_DECL vec2 gen_rand2(Generator *g) {
	float x = gen_randf(g), y = gen_randf(g);
	// don't pass them directly because function arguments are evaluated in an unspecified order
	return Vec2(x, y);
}

V_DECL vec3 gen_rand3(Generator *g) {
	float x = gen_randf(g), y = gen_randf(g), z = gen_randf(g);
	return Vec3(x, y, z);
}

V_DECL vec4 gen_rand4(Generator *g) {
	float x = gen_randf(g), y = gen_randf(g), z = gen_randf(g), w = gen_randf(g);
	return Vec4(x, y, z, w);
}

V_DECL int gen_randi(Generator *g, int up_to) {
	uint64_t u = (uint64_t)up_to, reject_past, n;
	assert(up_to > 0);
	reject_past = 0xffffffff - 0xffffffff % u;
	do
		n = V_gen_rand(g);
	while (n > reject_past);
	return (int)(n % u);
}

V_DECL vec2i gen_rand2i(Generator *g, vec2i up_to) {
	int x = gen_randi(g, up_to.x), y = gen_randi(g, up_to.y);
	return Vec2i(x, y);
}

V_DECL vec3i gen_rand3i(Generator *g, vec3i up_to) {
	int x = gen_randi(g, up_to.x), y = gen_randi(g, up_to.y), z = gen_randi(g, up_to.z);
	return Vec3i(x, y, z);
}

V_DECL vec4i gen_rand4i(Generator *g, vec4i up_to) {
	int x = gen_randi(g, up_to.x), y = gen_randi(g, up_to.y), z = gen_randi(g, up_to.z), w = gen_randi(g, up_to.w);
	return Vec4i(x, y, z, w);
}

V_DECL float gen_rand_normal(Generator *g) {
	// https://en.wikipedia.org/wiki/Normal_distribution#Generating_values_from_normal_distribution
	float U, V;
	do
		U = gen_randf(g), V = gen_randf(g);
	while (U == 0 || V == 0);
	return sqrtf(-2 * logf(U)) * cosf(2 * PI * V);
}

V_DECL vec2 gen_rand_vec2(Generator *g) {
	return Vec2(gen_randf(g), gen_randf(g));
}

V_DECL vec3 gen_rand_vec3(Generator *g) {
	return Vec3(gen_randf(g), gen_randf(g), gen_randf(g));
}

V_DECL vec4 gen_rand_vec4(Generator *g) {
	return Vec4(gen_randf(g), gen_randf(g), gen_randf(g), gen_randf(g));
}

V_DECL vec2 gen_rand_unit_vec2(Generator *g) {
	float theta = 2 * PI * gen_randf(g);
	return Vec2(cosf(theta), sinf(theta));
}

V_DECL vec3 gen_rand_unit_vec3(Generator *g) {
	vec3 v;
	do
		v = gen_rand_vec3(g);
	while (sqlength3(v) > 1);
	return normalize3(v);
}

V_DECL vec4 gen_rand_unit_vec4(Generator *g) {
	vec4 v;
	do
		v = gen_rand_vec4(g);
	while (sqlength4(v) > 1);
	return normalize4(v);
}

#if V_GL
#include <SDL.h>

static SDL_Window *V_sdl_window;
static char V_window_error[2048];
static SDL_GLContext V_gl_context;

V_VARDECL int gl_version_major, gl_version_minor;
V_VARDECL GLProcs gl;

V_DECL const char *window_geterr(void) {
	return V_window_error;
}

#if DEBUG
static void APIENTRY V_gl_message_callback(GLenum source, GLenum type, unsigned int id, GLenum severity,
	GLsizei length, const char *message, const void *userParam) {
	(void)source; (void)type; (void)id; (void)length; (void)userParam;
	if (severity == GL_DEBUG_SEVERITY_NOTIFICATION) return;
	debug_print("Message from OpenGL: %s.\n", message);
}
#endif

V_DECL void window_message_box_error(const char *title, const char *message) {
	SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, title, message, NULL);
}

static void V_window_die(void) {
	window_message_box_error("Error", V_window_error);
	exit(-1);
}

static unsigned V_sdl_keycode_to_key(SDL_Keycode code) {
	switch (code) {
	case SDLK_UNKNOWN: return KEY_UNKNOWN; case SDLK_a: return KEY_A; case SDLK_b: return KEY_B;
	case SDLK_c: return KEY_C; case SDLK_d: return KEY_D; case SDLK_e: return KEY_E;
	case SDLK_f: return KEY_F; case SDLK_g: return KEY_G; case SDLK_h: return KEY_H;
	case SDLK_i: return KEY_I; case SDLK_j: return KEY_J; case SDLK_k: return KEY_K;
	case SDLK_l: return KEY_L; case SDLK_m: return KEY_M; case SDLK_n: return KEY_N;
	case SDLK_o: return KEY_O; case SDLK_p: return KEY_P; case SDLK_q: return KEY_Q;
	case SDLK_r: return KEY_R; case SDLK_s: return KEY_S; case SDLK_t: return KEY_T;
	case SDLK_u: return KEY_U; case SDLK_v: return KEY_V; case SDLK_w: return KEY_W;
	case SDLK_x: return KEY_X; case SDLK_y: return KEY_Y; case SDLK_z: return KEY_Z;
	case SDLK_1: return KEY_1; case SDLK_2: return KEY_2; case SDLK_3: return KEY_3;
	case SDLK_4: return KEY_4; case SDLK_5: return KEY_5; case SDLK_6: return KEY_6;
	case SDLK_7: return KEY_7; case SDLK_8: return KEY_8; case SDLK_9: return KEY_9;
	case SDLK_0: return KEY_0; case SDLK_RETURN: return KEY_RETURN;
	case SDLK_ESCAPE: return KEY_ESCAPE; case SDLK_BACKSPACE: return KEY_BACKSPACE;
	case SDLK_TAB: return KEY_TAB; case SDLK_SPACE: return KEY_SPACE;
	case SDLK_MINUS: return KEY_MINUS; case SDLK_EQUALS: return KEY_EQUALS;
	case SDLK_LEFTBRACKET: return KEY_LEFTBRACKET; case SDLK_RIGHTBRACKET: return KEY_RIGHTBRACKET;
	case SDLK_BACKSLASH: return KEY_BACKSLASH; case SDLK_SEMICOLON: return KEY_SEMICOLON;
	case SDLK_QUOTE: return KEY_APOSTROPHE; case SDLK_BACKQUOTE: return KEY_BACKTICK;
	case SDLK_COMMA: return KEY_COMMA; case SDLK_PERIOD: return KEY_PERIOD;
	case SDLK_SLASH: return KEY_SLASH; case SDLK_CAPSLOCK: return KEY_CAPSLOCK;
	case SDLK_F1: return KEY_F1; case SDLK_F2: return KEY_F2; case SDLK_F3: return KEY_F3;
	case SDLK_F4: return KEY_F4; case SDLK_F5: return KEY_F5; case SDLK_F6: return KEY_F6;
	case SDLK_F7: return KEY_F7; case SDLK_F8: return KEY_F8; case SDLK_F9: return KEY_F9;
	case SDLK_F10: return KEY_F10; case SDLK_F11: return KEY_F11; case SDLK_F12: return KEY_F12;
	case SDLK_PRINTSCREEN: return KEY_PRINTSCREEN; case SDLK_SCROLLLOCK: return KEY_SCROLLLOCK;
	case SDLK_PAUSE: return KEY_PAUSE; case SDLK_INSERT: return KEY_INSERT;
	case SDLK_HOME: return KEY_HOME; case SDLK_PAGEUP: return KEY_PAGEUP;
	case SDLK_DELETE: return KEY_DELETE; case SDLK_END: return KEY_END;
	case SDLK_PAGEDOWN: return KEY_PAGEDOWN; case SDLK_RIGHT: return KEY_RIGHT;
	case SDLK_LEFT: return KEY_LEFT; case SDLK_DOWN: return KEY_DOWN;
	case SDLK_UP: return KEY_UP; case SDLK_NUMLOCKCLEAR: return KEY_NUMLOCKCLEAR;
	case SDLK_KP_DIVIDE: return KEY_KP_DIVIDE; case SDLK_KP_MULTIPLY: return KEY_KP_MULTIPLY;
	case SDLK_KP_MINUS: return KEY_KP_MINUS; case SDLK_KP_PLUS: return KEY_KP_PLUS;
	case SDLK_KP_ENTER: return KEY_KP_ENTER; case SDLK_KP_1: return KEY_KP_1;
	case SDLK_KP_2: return KEY_KP_2; case SDLK_KP_3: return KEY_KP_3;
	case SDLK_KP_4: return KEY_KP_4; case SDLK_KP_5: return KEY_KP_5;
	case SDLK_KP_6: return KEY_KP_6; case SDLK_KP_7: return KEY_KP_7;
	case SDLK_KP_8: return KEY_KP_8; case SDLK_KP_9: return KEY_KP_9;
	case SDLK_KP_0: return KEY_KP_0; case SDLK_KP_PERIOD: return KEY_KP_PERIOD;
	case SDLK_LCTRL: return KEY_LCTRL; case SDLK_LSHIFT: return KEY_LSHIFT; case SDLK_LALT: return KEY_LALT;
	case SDLK_LGUI: return KEY_LGUI; case SDLK_RCTRL: return KEY_RCTRL; case SDLK_RSHIFT: return KEY_RSHIFT;
	case SDLK_RALT: return KEY_RALT; case SDLK_RGUI: return KEY_RGUI;
	default:
		return (unsigned)code & ~(1u<<30);
	}
}

static unsigned V_key_to_sdl_scancode(unsigned key) {
	switch (key) {
	case KEY_UNKNOWN: return SDL_SCANCODE_UNKNOWN;
	case KEY_A: return SDL_SCANCODE_A; case KEY_B: return SDL_SCANCODE_B; case KEY_C: return SDL_SCANCODE_C;
	case KEY_D: return SDL_SCANCODE_D; case KEY_E: return SDL_SCANCODE_E; case KEY_F: return SDL_SCANCODE_F;
	case KEY_G: return SDL_SCANCODE_G; case KEY_H: return SDL_SCANCODE_H; case KEY_I: return SDL_SCANCODE_I;
	case KEY_J: return SDL_SCANCODE_J; case KEY_K: return SDL_SCANCODE_K; case KEY_L: return SDL_SCANCODE_L;
	case KEY_M: return SDL_SCANCODE_M; case KEY_N: return SDL_SCANCODE_N; case KEY_O: return SDL_SCANCODE_O;
	case KEY_P: return SDL_SCANCODE_P; case KEY_Q: return SDL_SCANCODE_Q; case KEY_R: return SDL_SCANCODE_R;
	case KEY_S: return SDL_SCANCODE_S; case KEY_T: return SDL_SCANCODE_T; case KEY_U: return SDL_SCANCODE_U;
	case KEY_V: return SDL_SCANCODE_V; case KEY_W: return SDL_SCANCODE_W; case KEY_X: return SDL_SCANCODE_X;
	case KEY_Y: return SDL_SCANCODE_Y; case KEY_Z: return SDL_SCANCODE_Z; case KEY_1: return SDL_SCANCODE_1;
	case KEY_2: return SDL_SCANCODE_2; case KEY_3: return SDL_SCANCODE_3; case KEY_4: return SDL_SCANCODE_4;
	case KEY_5: return SDL_SCANCODE_5; case KEY_6: return SDL_SCANCODE_6; case KEY_7: return SDL_SCANCODE_7;
	case KEY_8: return SDL_SCANCODE_8; case KEY_9: return SDL_SCANCODE_9; case KEY_0: return SDL_SCANCODE_0;
	case KEY_RETURN: return SDL_SCANCODE_RETURN; case KEY_ESCAPE: return SDL_SCANCODE_ESCAPE;
	case KEY_BACKSPACE: return SDL_SCANCODE_BACKSPACE; case KEY_TAB: return SDL_SCANCODE_TAB;
	case KEY_SPACE: return SDL_SCANCODE_SPACE; case KEY_MINUS: return SDL_SCANCODE_MINUS;
	case KEY_EQUALS: return SDL_SCANCODE_EQUALS; case KEY_LEFTBRACKET: return SDL_SCANCODE_LEFTBRACKET;
	case KEY_RIGHTBRACKET: return SDL_SCANCODE_RIGHTBRACKET; case KEY_BACKSLASH: return SDL_SCANCODE_BACKSLASH;
	case KEY_NONUSHASH: return SDL_SCANCODE_NONUSHASH; case KEY_SEMICOLON: return SDL_SCANCODE_SEMICOLON;
	case KEY_APOSTROPHE: return SDL_SCANCODE_APOSTROPHE; case KEY_BACKTICK: return SDL_SCANCODE_GRAVE;
	case KEY_COMMA: return SDL_SCANCODE_COMMA; case KEY_PERIOD: return SDL_SCANCODE_PERIOD;
	case KEY_SLASH: return SDL_SCANCODE_SLASH; case KEY_CAPSLOCK: return SDL_SCANCODE_CAPSLOCK;
	case KEY_F1: return SDL_SCANCODE_F1; case KEY_F2: return SDL_SCANCODE_F2;
	case KEY_F3: return SDL_SCANCODE_F3; case KEY_F4: return SDL_SCANCODE_F4;
	case KEY_F5: return SDL_SCANCODE_F5; case KEY_F6: return SDL_SCANCODE_F6;
	case KEY_F7: return SDL_SCANCODE_F7; case KEY_F8: return SDL_SCANCODE_F8;
	case KEY_F9: return SDL_SCANCODE_F9; case KEY_F10: return SDL_SCANCODE_F10;
	case KEY_F11: return SDL_SCANCODE_F11; case KEY_F12: return SDL_SCANCODE_F12;
	case KEY_PRINTSCREEN: return SDL_SCANCODE_PRINTSCREEN; case KEY_SCROLLLOCK: return SDL_SCANCODE_SCROLLLOCK;
	case KEY_PAUSE: return SDL_SCANCODE_PAUSE; case KEY_INSERT: return SDL_SCANCODE_INSERT;
	case KEY_HOME: return SDL_SCANCODE_HOME; case KEY_PAGEUP: return SDL_SCANCODE_PAGEUP;
	case KEY_DELETE: return SDL_SCANCODE_DELETE; case KEY_END: return SDL_SCANCODE_END;
	case KEY_PAGEDOWN: return SDL_SCANCODE_PAGEDOWN; case KEY_RIGHT: return SDL_SCANCODE_RIGHT;
	case KEY_LEFT: return SDL_SCANCODE_LEFT; case KEY_DOWN: return SDL_SCANCODE_DOWN;
	case KEY_UP: return SDL_SCANCODE_UP; case KEY_NUMLOCKCLEAR: return SDL_SCANCODE_NUMLOCKCLEAR;
	case KEY_KP_DIVIDE: return SDL_SCANCODE_KP_DIVIDE; case KEY_KP_MULTIPLY: return SDL_SCANCODE_KP_MULTIPLY;
	case KEY_KP_MINUS: return SDL_SCANCODE_KP_MINUS; case KEY_KP_PLUS: return SDL_SCANCODE_KP_PLUS;
	case KEY_KP_ENTER: return SDL_SCANCODE_KP_ENTER; case KEY_KP_1: return SDL_SCANCODE_KP_1;
	case KEY_KP_2: return SDL_SCANCODE_KP_2; case KEY_KP_3: return SDL_SCANCODE_KP_3;
	case KEY_KP_4: return SDL_SCANCODE_KP_4; case KEY_KP_5: return SDL_SCANCODE_KP_5;
	case KEY_KP_6: return SDL_SCANCODE_KP_6; case KEY_KP_7: return SDL_SCANCODE_KP_7;
	case KEY_KP_8: return SDL_SCANCODE_KP_8; case KEY_KP_9: return SDL_SCANCODE_KP_9;
	case KEY_KP_0: return SDL_SCANCODE_KP_0; case KEY_KP_PERIOD: return SDL_SCANCODE_KP_PERIOD;
	case KEY_LCTRL: return SDL_SCANCODE_LCTRL; case KEY_LSHIFT: return SDL_SCANCODE_LSHIFT;
	case KEY_LALT: return SDL_SCANCODE_LALT; case KEY_LGUI: return SDL_SCANCODE_LGUI;
	case KEY_RCTRL: return SDL_SCANCODE_RCTRL; case KEY_RSHIFT: return SDL_SCANCODE_RSHIFT;
	case KEY_RALT: return SDL_SCANCODE_RALT; case KEY_RGUI: return SDL_SCANCODE_RGUI;
	default:
		return key;
	}
}

#if DEBUG
static bool V_gl_uniform_editor_enabled;
static bool V_gl_uniform_editor_shown = false;
static GLText V_gl_uniform_editor_text;
static GLText V_gl_uniform_editor_text_highlighted;
#endif
static void V_gl_font_load(void);

// i was getting some problems labelling objects on windows
#if DEBUG && __linux__
#define V_GL_LABEL_OBJECTS 1
#endif

V_DECL bool window_create(const char *title, int width, int height, uint32_t flags) {
	if (V_sdl_window) {
		strbuf_cpy(V_window_error, "Window created for a second time.");
		if (!(flags & WINDOW_CREATE_DONT_QUIT_ON_ERROR))
			V_window_die();
		return false;
	}
	SDL_SetHint("SDL_HINT_NO_SIGNAL_HANDLERS", "1"); // if this program is sent a SIGTERM/SIGINT, don't turn it into a quit event
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_AUDIO) == 0) {
		uint32_t sdlflags = SDL_WINDOW_OPENGL;
		if (flags & WINDOW_CREATE_HIDDEN)
			sdlflags |= SDL_WINDOW_HIDDEN;
		else
			sdlflags |= SDL_WINDOW_SHOWN;
		V_sdl_window =
			SDL_CreateWindow(title, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
				width, height, sdlflags);
		if (V_sdl_window) {
			struct {
				int major, minor;
			} versions[] = {
				{4, 6}, {4, 5}, {4, 4}, {4, 3}, {4, 2}, {4, 1}, {4, 0},
				{3, 3}, {3, 2}, {3, 1}, {3, 0},
				{0, 0}
			};

			int i;
			for (i = 0; versions[i].major; ++i) {
				gl_version_major = versions[i].major;
				gl_version_minor = versions[i].minor;
				SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, gl_version_major);
				SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, gl_version_minor);
				#if DEBUG
				SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);
				#endif
				V_gl_context = SDL_GL_CreateContext(V_sdl_window);
				if (V_gl_context) break;
			}
			if (V_gl_context) {
			#define V_get_proc(lower, upper) gl.lower = (PFNGL##upper##PROC)SDL_GL_GetProcAddress("gl" #lower);
				NO_WARN_START
				V_gl_for_each_proc(V_get_proc)
				NO_WARN_END
				#if DEBUG
				{
					// setup debug context
					GLint glctx_flags = 0;
					gl.GetIntegerv(GL_CONTEXT_FLAGS, &glctx_flags);
					gl.Enable(GL_DEBUG_OUTPUT);
					gl.Enable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
					if (glctx_flags & GL_CONTEXT_FLAG_DEBUG_BIT) {
						gl.DebugMessageCallback(V_gl_message_callback, NULL);
						gl.DebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, NULL, GL_TRUE);
					}
				}
				#endif
				SDL_GL_SetSwapInterval(1); // vsync
				gl.BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // set default blend func to something reasonable
				V_gl_font_load();
			#if DEBUG
				V_gl_uniform_editor_enabled = (flags & WINDOW_ENABLE_UNIFORM_EDITOR) != 0;
			#endif
				return true;
			} else {
				strbuf_cpy(V_window_error, SDL_GetError());
				if (!(flags & WINDOW_CREATE_DONT_QUIT_ON_ERROR))
					V_window_die();
			}
		} else {
			strbuf_cpy(V_window_error, SDL_GetError());
			if (!(flags & WINDOW_CREATE_DONT_QUIT_ON_ERROR))
				V_window_die();
		}
	} else {
		strbuf_cpy(V_window_error, SDL_GetError());
		V_window_die();
		if (!(flags & WINDOW_CREATE_DONT_QUIT_ON_ERROR))
			V_window_die();
	}
	return false;
}

void window_audio_add_from_file(const AudioFile *f, float amplitude, bool loop, int16_t *buffer, int sample_rate, size_t frame_count, double t) {
	double dt = 1/(double)sample_rate;
	unsigned channels = (unsigned)f->channels;
	bool stereo = channels > 1;
	double sr = (double)f->sample_rate;
	if (!f->samples) return; // audio file failed to load
	for (size_t i = 0; i < frame_count; ++i, t += dt) {
		double frame_idx = t * sr;
		size_t f0 = (size_t)frame_idx;
		if (f0 >= f->nframes-1) {
			if (loop)
				f0 %= f->nframes / channels * channels;
			else
				break;
		}

		// if the audio file's sample rate is different from the audio device's sample rate,
		// we'll get a fractional input sample index. so, we'll lerp between floor(i) and
		// floor(i)+1 according to fract(i)
		// without this, there are weird audio artifacts
		int16_t *frame0 = &f->samples[channels * f0];
		int16_t *frame1 = frame0 + channels;
		float s0l = (float)frame0[0];
		float s1l = (float)frame1[0];
		float s0r = (float)frame0[stereo];
		float s1r = (float)frame1[stereo];
		float m = (float)fmod(frame_idx, 1);
		float l = lerp(s0l, s1l, m);
		float r = lerp(s0r, s1r, m);

		buffer[2*i+0] += (int16_t)(amplitude * l);
		buffer[2*i+1] += (int16_t)(amplitude * r);
	}
}

typedef struct {
	AudioCallback callback;
	int sample_rate;
	double t;
	void *userdata;
} V_AudioUserData;

static V_AudioUserData V_audio_user_data = {0};
static SDL_AudioDeviceID V_audio_device;

static SDLCALL void V_audio_callback(void *userdata, uint8_t *buffer, int len) {
	V_AudioUserData *audio = (V_AudioUserData *)userdata;
	if (len > 0) {
		memset(buffer, 0, (size_t)len);
		if (audio->callback(audio->userdata, (int16_t *)buffer, audio->sample_rate, (size_t)(len / 4), audio->t))
			audio->t += (double)len * 0.25 / audio->sample_rate;
	}
}

// NOTE: audio may be run in a separate thread, so make sure you use
// window_audio_lock and window_audio_unlock when changing userdata.
// Also: you will need to call window_audio_start after this
V_DECL void window_audio_setup(AudioCallback callback, void *userdata) {
	if (V_audio_user_data.callback) {
		strbuf_cpy(V_window_error, "Set up audio twice.");
		V_window_die();
	} else if (!callback) {
		strbuf_cpy(V_window_error, "NULL audio callback.");
		V_window_die();
	} else {
		SDL_AudioSpec want = {0}, have = {0};
		
		V_audio_user_data.callback = callback;
		V_audio_user_data.userdata = userdata;

		want.freq = 44100;
		want.format = AUDIO_S16;
		want.channels = 2;
		want.samples = 4096;
		want.callback = V_audio_callback;
		want.userdata = &V_audio_user_data;
		V_audio_device = SDL_OpenAudioDevice(NULL, 0, &want, &have, SDL_AUDIO_ALLOW_FREQUENCY_CHANGE | SDL_AUDIO_ALLOW_SAMPLES_CHANGE);
		if (V_audio_device == 0) {
			debug_print("Couldn't open audio: %s\n", SDL_GetError());
		}
		V_audio_user_data.sample_rate = have.freq;
	}

}

V_DECL void window_audio_start(void) {
	SDL_PauseAudioDevice(V_audio_device, 0);
}

V_DECL void window_audio_lock(void) {
	SDL_LockAudioDevice(V_audio_device);
}

V_DECL void window_audio_unlock(void) {
	SDL_UnlockAudioDevice(V_audio_device);
}

V_DECL Mutex mutex_new(void) {
	return SDL_CreateMutex();
}

V_DECL void mutex_lock(Mutex m) {
	SDL_LockMutex(m);
}

V_DECL void mutex_unlock(Mutex m) {
	SDL_UnlockMutex(m);
}

V_DECL bool mutex_trylock(Mutex m) {
	return SDL_TryLockMutex(m) == 0;
}

V_DECL void mutex_delete(Mutex m) {
	SDL_DestroyMutex(m);
}

V_DECL void window_set_fullscreen(bool fullscreen) {
	SDL_SetWindowFullscreen(V_sdl_window, fullscreen ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0);
}

V_DECL void window_set_shown(bool shown) {
	if (shown)
		SDL_ShowWindow(V_sdl_window);
	else
		SDL_HideWindow(V_sdl_window);
}

V_DECL void window_set_resizable(bool resizable) {
	SDL_SetWindowResizable(V_sdl_window, resizable);
}

V_DECL bool window_has_focus(void) {
	return (SDL_GetWindowFlags(V_sdl_window) & SDL_WINDOW_INPUT_FOCUS) != 0;
}

static uint8_t V_key_presses[KEY_COUNT];
static bool V_mouse_buttons_pressed[8];

// how many times was this key pressed this frame?
V_DECL int window_n_key_presses(unsigned key) {
	if (key < KEY_COUNT)
		return V_key_presses[key];
	assert(0);
	return 0;
}

V_DECL bool window_next_event(Event *event) {
	SDL_Event sdl = {0};
top:
	if (SDL_PollEvent(&sdl)) {
		switch (sdl.type) {
		case SDL_QUIT:
			event->type = EVENT_QUIT;
			break;
		case SDL_KEYDOWN:
			event->type = EVENT_KEYDOWN;
			goto key_event;
		case SDL_KEYUP:
			event->type = EVENT_KEYUP;
			goto key_event;
		key_event: {
			uint8_t modifiers = 0;
			unsigned key = V_sdl_keycode_to_key(sdl.key.keysym.sym);
			unsigned sdlmod = sdl.key.keysym.mod;
			EventKeyboard *kbd = &event->key;
			if (sdlmod & KMOD_LCTRL) modifiers |= KEY_MODIFIER_LCTRL;
			if (sdlmod & KMOD_RCTRL) modifiers |= KEY_MODIFIER_RCTRL;
			if (sdlmod & KMOD_LSHIFT) modifiers |= KEY_MODIFIER_LSHIFT;
			if (sdlmod & KMOD_RSHIFT) modifiers |= KEY_MODIFIER_RSHIFT;
			if (sdlmod & KMOD_LALT) modifiers |= KEY_MODIFIER_LALT;
			if (sdlmod & KMOD_RALT) modifiers |= KEY_MODIFIER_RALT;
			if (sdlmod & KMOD_LGUI) modifiers |= KEY_MODIFIER_LGUI;
			if (sdlmod & KMOD_RGUI) modifiers |= KEY_MODIFIER_RGUI;
		#if DEBUG
			if (key == KEY_ESCAPE && event->type == EVENT_KEYDOWN && V_gl_uniform_editor_shown) {
				V_gl_uniform_editor_shown = false;
				goto top;
			}
		#endif
			kbd->keycode = key;
			kbd->modifiers = modifiers;
			kbd->repeat = sdl.key.repeat;
			if (key < KEY_COUNT && event->type == EVENT_KEYDOWN)
				++V_key_presses[key];
		} break;
		case SDL_MOUSEBUTTONUP:
			event->type = EVENT_MOUSEUP;
			goto mouse_button_event;
		case SDL_MOUSEBUTTONDOWN:
			event->type = EVENT_MOUSEDOWN;
			goto mouse_button_event;
		mouse_button_event: {
			uint8_t button = sdl.button.button;
			switch (button) {
			case SDL_BUTTON_LEFT:
				button = MOUSE_LEFT;
				break;
			case SDL_BUTTON_MIDDLE:
				button = MOUSE_MIDDLE;
				break;
			case SDL_BUTTON_RIGHT:
				button = MOUSE_RIGHT;
				break;
			}
			if (button < static_arr_len(V_mouse_buttons_pressed) && event->type == EVENT_MOUSEDOWN)
				V_mouse_buttons_pressed[button] = true;
			event->mouse.x = sdl.button.x;
			event->mouse.y = sdl.button.y;
			event->mouse.dx = event->mouse.dy = 0;
			event->mouse.button = button;
			event->mouse.clicks = sdl.button.clicks;
		} break;
		case SDL_MOUSEMOTION:
			event->type = EVENT_MOUSEMOTION;
			event->mouse.button = MOUSE_NONE;
			event->mouse.clicks = 0;
			event->mouse.x = sdl.motion.x;
			event->mouse.y = sdl.motion.y;
			event->mouse.dx = sdl.motion.xrel;
			event->mouse.dy = sdl.motion.yrel;
			break;
		case SDL_MOUSEWHEEL:
			event->type = EVENT_MOUSEWHEEL;
			event->mouse.button = MOUSE_NONE;
			event->mouse.clicks = 0;
			event->mouse.x = event->mouse.y = 0;
			event->mouse.dx = sdl.wheel.x;
			event->mouse.dy = sdl.wheel.y;
			if (sdl.wheel.direction == SDL_MOUSEWHEEL_FLIPPED) {
				event->mouse.dx = -event->mouse.dx;
				event->mouse.dy = -event->mouse.dy;
			}
			break;
		case SDL_TEXTINPUT:
			event->type = EVENT_TEXT;
			strbuf_cpy(event->text.text, sdl.text.text);
			break;
		default:
			goto top;
		}
		return true;
	} else return false;
}

V_DECL void window_set_relative_mouse(bool on) {
	SDL_SetRelativeMouseMode(on);
	SDL_CaptureMouse(on);
}

V_DECL vec2i window_get_size(void) {
	int w = 0, h = 0;
	SDL_GetWindowSize(V_sdl_window, &w, &h);
	return Vec2i(w, h);
}

V_DECL void window_set_size(vec2i size) {
	SDL_SetWindowSize(V_sdl_window, size.x, size.y);
}

V_DECL bool window_is_mouse_button_down(unsigned button) {
	return (SDL_GetMouseState(NULL, NULL) & (1u<<button)) != 0;
}

V_DECL vec2i window_get_mouse_pos(void) {
	vec2i v;
	SDL_GetMouseState(&v.x, &v.y);
	return v;
}

V_DECL bool window_is_key_down(unsigned key) {
	return SDL_GetKeyboardState(NULL)[V_key_to_sdl_scancode(key)];
}

V_DECL bool window_is_shift_down(void) {
	return window_is_key_down(KEY_LSHIFT) || window_is_key_down(KEY_RSHIFT);
}

V_DECL bool window_is_ctrl_down(void) {
	return window_is_key_down(KEY_LCTRL) || window_is_key_down(KEY_RCTRL);
}

V_DECL bool window_is_alt_down(void) {
	return window_is_key_down(KEY_LALT) || window_is_key_down(KEY_RALT);
}

V_DECL bool window_mouse_button_pressed(int button) {
	if (button >= 0 && button < (int)static_arr_len(V_mouse_buttons_pressed)) {
		return V_mouse_buttons_pressed[button];
	} else assert(0);
	return false;
}

#if DEBUG
static void V_gl_textures_reload(void);
static void V_gl_programs_reload(void);
#endif

static Time V_last_frame;

static GLProgram V_gl_programs[1000];
static GLProgram V_null_program;

V_DECL void window_close_debug_menus(void) {
#if DEBUG
	V_gl_uniform_editor_shown = false;
#endif
}

static void V_gl_font_delete(void);
static void V_gl_quit(void);

V_DECL void window_delete(void) {
	if (V_audio_user_data.callback) {
		// close audio
		SDL_CloseAudioDevice(V_audio_device);
		memset(&V_audio_user_data, 0, sizeof V_audio_user_data);
	}

#if DEBUG
	gl_text_delete(&V_gl_uniform_editor_text);
	gl_text_delete(&V_gl_uniform_editor_text_highlighted);
#endif

	V_gl_font_delete();
	V_gl_quit();
	SDL_DestroyWindow(V_sdl_window); V_sdl_window = NULL;
	SDL_GL_DeleteContext(V_gl_context); V_gl_context = NULL;

	SDL_Quit();
}

V_DECL char *window_get_clipboard_text(void) {
	if (SDL_HasClipboardText())
		return SDL_GetClipboardText();
	else
		return NULL;
}

V_DECL bool window_set_clipboard_text(const char *text) {
	return SDL_SetClipboardText(text) == 0;
}

V_DECL void window_set_maximization(int m) {
	switch (m) {
	case -1:
		SDL_MinimizeWindow(V_sdl_window);
		break;
	case  0:
		SDL_RestoreWindow(V_sdl_window);
		break;
	case +1:
		SDL_MaximizeWindow(V_sdl_window);
		break;
	default: assert(0); break;
	}
}

#if V_GL
V_DECL mat4 mat4_camera(vec3 player_pos, float player_yaw, float player_pitch, float fov, float z_near, float z_far) {
	vec2i window_size = window_get_size();
	mat4 camera_translation = mat4_translate(scale3(player_pos, -1));
	mat4 camera_pitch = mat4_from_mat3(mat3_pitch(-player_pitch));
	mat4 camera_yaw = mat4_from_mat3(mat3_yaw(-player_yaw));
	mat4 camera_rotation = mat4_mul(&camera_pitch, &camera_yaw);
	mat4 camera = mat4_mul(&camera_rotation, &camera_translation);
	mat4 perspective = mat4_perspective(fov, (float)window_size.x / (float)window_size.y, z_near, z_far);
	return mat4_mul(&perspective, &camera);
}
#endif

// how many vbos/vaos/etc to generate at a time
#define V_BATCH_SIZE 100

static GLuint V_buffers[V_BATCH_SIZE], V_buffers_used = 0;
static GLuint V_arrays[V_BATCH_SIZE], V_arrays_used = 0;

#if DEBUG
#define V_BUFFER_TRACK_MAX 10000
#define V_ARRAY_TRACK_MAX  10000
static GLuint V_unfreed_buffers[V_BUFFER_TRACK_MAX];
static GLuint V_unfreed_arrays[V_ARRAY_TRACK_MAX];
#endif

// to save time, generate vbos/vaos in batches
static GLuint V_buffer_new() {
	GLuint id;
	if (V_buffers_used == 0)
		gl.GenBuffers(V_BATCH_SIZE, V_buffers);
	id = V_buffers[V_buffers_used++];
	if (V_buffers_used >= V_BATCH_SIZE)
		V_buffers_used = 0;
#if DEBUG
	{
		GLuint *p = memchr32(V_unfreed_buffers, 0, V_BUFFER_TRACK_MAX);
		if (p) {
			*p = id;
		} else {
			static bool printed;
			if (!printed) {
				printed = true;
				print("Too many buffers to keep track of. Buffer leaks might not be printed.\n");
			}
		}
	}
#endif
	return id;
}

static void V_buffer_delete(GLuint buffer) {
	if (!buffer) return;
#if DEBUG
	{
		GLuint *p = memchr32(V_unfreed_buffers, buffer, V_BUFFER_TRACK_MAX);
		if (p) {
			*p = 0;
		} else {
			print("Unknown buffer %u freed", buffer);
		#if V_GL_LABEL_OBJECTS
			{
				char label[64] = {0};
				gl.GetObjectLabel(GL_BUFFER, buffer, sizeof label, NULL, label);
				if (*label) {
					print(" with label %s", label);
				} else {
					print(" with no label?");
				}
			}
		#endif
			print("\n");
			
		}
	}
#endif

	// ive found that glDeleteBuffers is much faster than glGenBuffers, so batching isn't necessary,
	// and saves gpu memory
	gl.DeleteBuffers(1, &buffer);
}

static GLuint V_array_new() {
	GLuint id;
	if (V_arrays_used == 0)
		gl.GenVertexArrays(V_BATCH_SIZE, V_arrays);
	id = V_arrays[V_arrays_used++];
	if (V_arrays_used >= V_BATCH_SIZE)
		V_arrays_used = 0;
#if DEBUG
	{
		GLuint *p = memchr32(V_unfreed_arrays, 0, V_ARRAY_TRACK_MAX);
		if (p) {
			*p = id;
		} else {
			static bool printed;
			if (!printed) {
				printed = true;
				print("Too many vertex arrays to keep track of. Leaks may not be printed.\n");
			}
		}
	}
#endif
	return id;
}

static void V_array_delete(GLuint array) {
	if (!array) return;
	gl.DeleteVertexArrays(1, &array);
#if DEBUG
	{
		GLuint *p = memchr32(V_unfreed_arrays, array, V_ARRAY_TRACK_MAX);
		if (p) {
			*p = 0;
		} else {
			print("Unknown vertex array %u freed", array);
		#if V_GL_LABEL_OBJECTS
			{
				char label[64] = {0};
				gl.GetObjectLabel(GL_VERTEX_ARRAY, array, sizeof label, NULL, label);
				if (*label) {
					print(" with label %s", label);
				} else {
					print(" with no label?");
				}
			}
		#endif
			print("\n");
			
		}
	}
#endif
}

static void V_gl_quit(void) {
	// delete unused buffers and arrays
	if (V_buffers_used)
		gl.DeleteBuffers(V_BATCH_SIZE - (GLint)V_buffers_used, V_buffers);
	if (V_arrays_used)
		gl.DeleteVertexArrays(V_BATCH_SIZE - (GLint)V_arrays_used, V_arrays);
#if DEBUG
	{
	size_t i;
	// check for buffers, arrays that the client didn't delete
	for (i = 0; i < V_BUFFER_TRACK_MAX; ++i) {
		GLuint buffer = V_unfreed_buffers[i];
		if_unlikely (buffer) {
			print("Buffer %u not freed", buffer);
		#if V_GL_LABEL_OBJECTS
			{
				char label[64] = {0};
				gl.GetObjectLabel(GL_BUFFER, buffer, sizeof label, NULL, label);
				if (*label) {
					print(" (label: %s)", label);
				} else {
					print(" (unlabeled?)");
				}
			}
		#endif
			print("\n");
		}
	}

	for (i = 0; i < V_ARRAY_TRACK_MAX; ++i) {
		GLuint array = V_unfreed_arrays[i];
		if_unlikely (array) {
			print("Vertex array %u not freed", array);
		#if V_GL_LABEL_OBJECTS
			{
				char label[64] = {0};
				gl.GetObjectLabel(GL_VERTEX_ARRAY, array, sizeof label, NULL, label);
				if (*label) {
					print(" (label: %s)", label);
				} else {
					print(" (unlabeled?)");
				}
			}
		#endif
			print("\n");
		}
	}
	}
#endif
}

V_DECL int gl_version_number(void) {
	return gl_version_major * 100 + gl_version_minor;
}

V_DECL GLVBO gl_vbo_new_with_size(size_t item_size, const char *label) {
	GLVBO vbo = {0};
	(void)label;
	assert(item_size < UINT32_MAX);
	vbo.id = V_buffer_new();
	vbo.item_size = (GLuint)item_size;
#if V_GL_LABEL_OBJECTS
	if (label && gl_version_number() >= 403) {
		char full_label[256];
		strbuf_print(full_label, "VBO %s", label);
		gl.ObjectLabel(GL_BUFFER, vbo.id, -1, full_label);
	}
#endif
	return vbo;
}

V_DECL void gl_vbo_set_data_with_item_size(GLVBO *vbo, const void *data, size_t count, GLenum usage, size_t item_size) {
	assert(item_size == vbo->item_size);
	assert(count < INT32_MAX / vbo->item_size);
	vbo->count = (GLuint)count;
	gl.BindBuffer(GL_ARRAY_BUFFER, vbo->id);
	gl.BufferData(GL_ARRAY_BUFFER, (GLsizeiptr)(vbo->item_size * count), data, usage);
}

V_DECL void gl_vbo_delete(GLVBO *vbo) {
	V_buffer_delete(vbo->id);
	memset(vbo, 0, sizeof *vbo);
}

V_DECL GLVAO gl_vao_new(GLProgram *program, const char *label) {
	GLVAO vao = {0};
	(void)label;
	assert(program->id);
	vao.program_id = program->id;
	vao.id = V_array_new();
#if V_GL_LABEL_OBJECTS
	if (label && gl_version_number() >= 403) {
		char full_label[256];
		strbuf_print(full_label, "VAO %s", label);
		gl.ObjectLabel(GL_VERTEX_ARRAY, vao.id, -1, full_label);
	}
#endif
	return vao;
}

V_DECL void gl_vao_clear(GLVAO *vao) {
	vao->count = 0;
}

V_DECL void gl_vao_add_data_with_offset(GLVAO *vao, GLVBO vbo,
	const char *attr_name, size_t item_size, const void *member_offset,
	int n_elements, GLenum element_kind) {
	GLint location;

	assert(item_size == vbo.item_size);
	assert(vao->count == 0 || vao->count == vbo.count);
	
	vao->count = vbo.count;
	gl.BindVertexArray(vao->id);
	gl.BindBuffer(GL_ARRAY_BUFFER, vbo.id);

	location = gl.GetAttribLocation(vao->program_id, attr_name);
	if (location >= 0) {
		switch (element_kind) {
		case GL_FLOAT:
			gl.VertexAttribPointer((GLuint)location, n_elements, GL_FLOAT, 0, (GLsizei)item_size,
				member_offset);
			break;
		case GL_INT:
			gl.VertexAttribIPointer((GLuint)location, n_elements, GL_INT, (GLsizei)item_size,
				member_offset);
			break;
		}
		gl.EnableVertexAttribArray((GLuint)location);
	} else {
		debug_print("Couldn't find vertex attribute: %s\n", attr_name);
	}
}

V_DECL void gl_vao_add_data1f_offset(GLVAO *vao, GLVBO vbo, const char *attr_name, size_t item_size, const float *member_offset) {
	gl_vao_add_data_with_offset(vao, vbo, attr_name, item_size, member_offset, 1, GL_FLOAT);
}
V_DECL void gl_vao_add_data2f_offset(GLVAO *vao, GLVBO vbo, const char *attr_name, size_t item_size, const vec2 *member_offset) {
	gl_vao_add_data_with_offset(vao, vbo, attr_name, item_size, member_offset, 2, GL_FLOAT);
}
V_DECL void gl_vao_add_data3f_offset(GLVAO *vao, GLVBO vbo, const char *attr_name, size_t item_size, const vec3 *member_offset) {
	gl_vao_add_data_with_offset(vao, vbo, attr_name, item_size, member_offset, 3, GL_FLOAT);
}
V_DECL void gl_vao_add_data4f_offset(GLVAO *vao, GLVBO vbo, const char *attr_name, size_t item_size, const vec4 *member_offset) {
	gl_vao_add_data_with_offset(vao, vbo, attr_name, item_size, member_offset, 4, GL_FLOAT);
}

V_DECL void gl_vao_add_data1i_offset(GLVAO *vao, GLVBO vbo, const char *attr_name, size_t item_size, const int *member_offset) {
	gl_vao_add_data_with_offset(vao, vbo, attr_name, item_size, member_offset, 1, GL_INT);
}
V_DECL void gl_vao_add_data2i_offset(GLVAO *vao, GLVBO vbo, const char *attr_name, size_t item_size, const vec2i *member_offset) {
	gl_vao_add_data_with_offset(vao, vbo, attr_name, item_size, member_offset, 2, GL_INT);
}
V_DECL void gl_vao_add_data3i_offset(GLVAO *vao, GLVBO vbo, const char *attr_name, size_t item_size, const vec3i *member_offset) {
	gl_vao_add_data_with_offset(vao, vbo, attr_name, item_size, member_offset, 3, GL_INT);
}
V_DECL void gl_vao_add_data4i_offset(GLVAO *vao, GLVBO vbo, const char *attr_name, size_t item_size, const vec4i *member_offset) {
	gl_vao_add_data_with_offset(vao, vbo, attr_name, item_size, member_offset, 4, GL_INT);
}

V_DECL void gl_vao_delete(GLVAO *vao) {
	V_array_delete(vao->id);
	memset(vao, 0, sizeof *vao);
}

#if DEBUG
	static GLuint V_program_in_use;
	#define V_gl_check_program_in_use(x) assert((x) == V_program_in_use)
	#define V_gl_set_program_in_use(x) (V_program_in_use = x)
#else
	#define V_gl_check_program_in_use(x) ((void)0)
	#define V_gl_set_program_in_use(x) ((void)0)
#endif

static uint64_t V_vertex_count;

V_DECL void gl_vao_render_with_mode(GLVAO vao, const GLIBO *ibo, GLenum mode) {
	assert(vao.id);
	if (vao.id) {
		V_gl_check_program_in_use(vao.program_id);
		gl.BindVertexArray(vao.id);
		if (ibo)
			gl.BindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo->id);
		if (ibo) {
			gl.DrawElements(mode, (GLsizei)ibo->count, GL_UNSIGNED_INT, NULL);
			V_vertex_count += ibo->count;
		} else {
			gl.DrawArrays(mode, 0, (GLsizei)vao.count);
			V_vertex_count += vao.count;
		}
	}
}

V_DECL int V_glsl_version(void) {
	switch (gl_version_number()) {
	case 200: return 110;
	case 210: return 120;
	case 300: return 130;
	case 310: return 140;
	case 320: return 150;
	case 330: return 330;
	default: return 400; // don't use hypermodern GLSL; it doesnt have gl_FragColor aaaa
	}
}

#define V_TYPE_INT 0
#define V_TYPE_FLOAT 1
typedef struct V_ConfigurableUniform {
	unsigned char type;
	unsigned char count;
	char name[46];
	double min_val, max_val;
	union {
		int i[4];
		float f[4];
	} val;
} V_ConfigurableUniform;


static char *V_preprocess_code(const char *code, V_ConfigurableUniform **configurable_uniforms) {
	size_t i = 0, cap = 100;
	char *str = (char *)calloc(1, cap);
	const char *in = code;
	int line_number = 1;
	
	while (*in) {
		const char *append = NULL;
		char *to_free = NULL;
		size_t append_len = 0;
		char append_buf[256] = {0};
		char type[30] = {0}, name[40] = {0};
		double v_min = 0, v_max = 0;
		int bytes_read = 0;
		bool newline = false;

		size_t line_len = strcspn(in, "\r\n");
		if (strncmp(in, "#include \"", 10) == 0 && in[line_len - 1] == '"') {
			char filename[256];
			strbuf_print(filename, "assets/%.*s", (int)line_len - 11, in + 10);
			char *included_code = file_read_string(filename, true);
			if (!included_code) {
				debug_print("File not found: %s\n", filename);
				exit(-1);
			}
			char *processed_code = V_preprocess_code(included_code, configurable_uniforms);
			size_t processed_len = strlen(processed_code);
			free(included_code);
			processed_code = realloc(processed_code, processed_len + 50);
			snprintf(processed_code + processed_len, 40, "#line %d\n", line_number + 1);
			append = processed_code;
			append_len = strlen(processed_code);
			to_free = processed_code;
		} else if (configurable_uniforms &&
			sscanf(in, "uniform %20s %35s range(%lf,%lf) = %n", type, name, &v_min, &v_max, &bytes_read) == 4) {
			// stuff for the uniform editor
			V_ConfigurableUniform u = {0};
			bool valid_type = true, valid_value = false;

			strbuf_print(append_buf, "uniform %s %s;", type, name);
			strbuf_cpy(u.name, name);
			u.min_val = v_min;
			u.max_val = v_max;

			if (str_eq(type, "float")) {
				u.type = V_TYPE_FLOAT;
				u.count = 1;
				valid_value = sscanf(in + bytes_read, "%f;", &u.val.f[0]) == 1;
			} else if (str_eq(type, "vec2")) {
				u.type = V_TYPE_FLOAT;
				u.count = 2;
				valid_value = sscanf(in + bytes_read, "(%f, %f);", &u.val.f[0], &u.val.f[1]) == 2;
			} else if (str_eq(type, "vec3")) {
				u.type = V_TYPE_FLOAT;
				u.count = 3;
				valid_value = sscanf(in + bytes_read, "(%f, %f, %f);", &u.val.f[0], &u.val.f[1], &u.val.f[2]) == 3;
			} else if (str_eq(type, "vec4")) {
				u.type = V_TYPE_FLOAT;
				u.count = 4;
				valid_value = sscanf(in + bytes_read, "(%f, %f, %f, %f);", &u.val.f[0], &u.val.f[1], &u.val.f[2], &u.val.f[3]) == 4;
			} else if (str_eq(type, "int")) {
				u.type = V_TYPE_INT;
				u.count = 1;
				valid_value = sscanf(in + bytes_read, "%d;", &u.val.i[0]) == 1;
			} else if (str_eq(type, "ivec2")) {
				u.type = V_TYPE_INT;
				u.count = 2;
				valid_value = sscanf(in + bytes_read, "(%d, %d);", &u.val.i[0], &u.val.i[1]) == 2;
			} else if (str_eq(type, "ivec3")) {
				u.type = V_TYPE_INT;
				u.count = 3;
				valid_value = sscanf(in + bytes_read, "(%d, %d, %d);", &u.val.i[0], &u.val.i[1], &u.val.i[2]) == 3;
			} else if (str_eq(type, "ivec4")) {
				u.type = V_TYPE_INT;
				u.count = 4;
				valid_value = sscanf(in + bytes_read, "(%d, %d, %d, %d);", &u.val.i[0], &u.val.i[1], &u.val.i[2], &u.val.i[3]) == 4;
			} else {
				valid_type = false;
			}
			if (valid_type) {
				if (valid_value) {
					// yay!
					arr_add(*configurable_uniforms, u);
				} else {
					debug_print("Invalid default value for configurable uniform %s: %.*s\n", name, (int)line_len - bytes_read, in + bytes_read);
				}
			} else {
				debug_print("Invalid type for configurable uniform %s: '%s'\n", name, type);
			}
			append = append_buf;
			append_len = strlen(append_buf);
			newline = true;
		} else {
			append = in;
			append_len = line_len;
			newline = true;
		}
		
		while (i + append_len + 5 >= cap) {
			str = (char *)realloc(str, cap * 2);
			memset(str + cap, 0, cap);
			cap *= 2;
		}
		memcpy(&str[i], append, append_len);
		i += append_len;
		if (newline) str[i++] = '\n';
		in += line_len;
		if (*in == '\r') ++in;
		if (*in == '\n') ++in;
		free(to_free);
		++line_number;
	}
	return str;
}

static GLuint V_gl_shader_compile_code(const char *filename_for_errors, V_ConfigurableUniform **configurable_uniforms,
	const char *code, GLenum type) {
	GLuint id;
	char version_string[32] = {0};
	const GLchar *sources[3];
	GLint lengths[3] = {
		-1, -1, -1
	};
	char *preprocessed = V_preprocess_code(code, configurable_uniforms);

	(void)filename_for_errors;
	
	id = gl.CreateShader(type);
	strbuf_print(version_string, "#version %d\n", V_glsl_version());
	sources[0] = version_string;
	char header[256];
	strbuf_print(header,
		"#define GLSL_VERSION %d\n"
		"#define PI 3.14159265\n"
		"#if GLSL_VERSION >= 400\n"
		"#define SAMPLE_ID gl_SampleID\n"
		"#else\n"
		"#define SAMPLE_ID 0\n"
		"#endif\n"
		"#line 1\n", V_glsl_version());
	sources[1] = header;
	sources[2] = preprocessed;

	gl.ShaderSource(id, 3, sources, lengths);
	free(preprocessed);
	gl.CompileShader(id);
	{
		GLint status = 0;
		gl.GetShaderiv(id, GL_COMPILE_STATUS, &status);
		if (status == 1) {
			// all good!
		} else {
			char log[2048];
			gl.GetShaderInfoLog(id, sizeof log, NULL, log);
			debug_print("Error compiling %s:\n%s\n", filename_for_errors, log);
			gl.DeleteShader(id);
			id = 0;
		}
	}
	return id;
}

static GLuint V_gl_shader_compile(const char *filename, V_ConfigurableUniform **configurable_uniforms, GLenum type) {
	GLuint id = 0;
	char *code = file_read_string(filename, true);

	if (code) {
		id = V_gl_shader_compile_code(filename, configurable_uniforms, code, type);
		free(code);
	} else {
		debug_print("Shader file not found: %s\n", filename);
	}
	return id;
}

// links the vertex and framgent shader into the given program object
static bool V_gl_link_program(GLuint id, V_ConfigurableUniform **configurable_uniforms,
	const char *vshader_filename, const char *fshader_filename) {
	bool success = false;

	GLuint vshader = V_gl_shader_compile(vshader_filename, configurable_uniforms, GL_VERTEX_SHADER);
	GLuint fshader = V_gl_shader_compile(fshader_filename, configurable_uniforms, GL_FRAGMENT_SHADER);

	if (vshader && fshader) {
		GLint status;
		
		gl.AttachShader(id, vshader);
		gl.AttachShader(id, fshader);
		gl.LinkProgram(id);
		gl.GetProgramiv(id, GL_LINK_STATUS, &status);
		if (status == 1) {
			// all good!
			success = true;
		} else {
			char log[2048];
			gl.GetProgramInfoLog(id, sizeof log, NULL, log);
			debug_print("Error linking %s-%s:\n%s\n", vshader_filename, fshader_filename, log);
		}
		gl.DetachShader(id, vshader);
		gl.DetachShader(id, fshader);
	}
	gl.DeleteShader(vshader);
	gl.DeleteShader(fshader);
	return success;
}

V_DECL GLProgram *gl_program_new(const char *vshader_filename, const char *fshader_filename) {
	GLProgram *p = 0, *end = V_gl_programs + static_arr_len(V_gl_programs);
	GLuint id;

	for (p = V_gl_programs; p != end; ++p) {
		if (!p->id) break;
	}
	if (p == end) {
		debug_print("Too many shader programs\n");
		return &V_null_program;
	}
	id = gl.CreateProgram();
	if (V_gl_link_program(id, &p->configurable_uniforms, vshader_filename, fshader_filename)) {
		p->id = id;
	#if DEBUG
		p->vfilename = str_dup(vshader_filename);
		p->ffilename = str_dup(fshader_filename);
		p->last_modified = time_max(fs_last_modified(vshader_filename), fs_last_modified(fshader_filename));
	#endif
		return p;
	} else {
		gl.DeleteProgram(id);
		return &V_null_program;
	}
}

#if DEBUG
// called by window_frame
static void V_gl_programs_reload(void) {
	GLProgram *p = V_gl_programs, *end = p + static_arr_len(V_gl_programs);
	for (; p != end; ++p) {
		if (p->id) {
			Time last_modified;
			bool changed = false;
			// wait for any text editor program to finish writing to the shader file
			while (1) {
				last_modified = time_max(fs_last_modified(p->vfilename), fs_last_modified(p->ffilename));
				if (time_eq(p->last_modified, last_modified)) break;
				changed = true;
				sleep_ms(50);
				p->last_modified = last_modified;
			}
			if (changed) {
				V_ConfigurableUniform *new_uniforms = NULL;
				printf("Reloading %s - %s\n", p->vfilename, p->ffilename);
				if (V_gl_link_program(p->id, &new_uniforms, p->vfilename, p->ffilename)) {
					arr_free(p->configurable_uniforms);
					p->configurable_uniforms = new_uniforms;
				}
			}
		}
	}
}
#endif

V_DECL void gl_program_delete(GLProgram *program) {
	gl.DeleteProgram(program->id);
	arr_free(program->configurable_uniforms);
#if DEBUG
	free(program->vfilename);
	free(program->ffilename);
#endif
	memset(program, 0, sizeof *program);
}

V_DECL GLint V_gl_uniform_location(GLProgram *program, const char *name) {
#if DEBUG
	GLint status;
	gl.GetProgramiv(program->id, GL_LINK_STATUS, &status);
	if (status) {
	V_gl_check_program_in_use(program->id);
#else
	if (1) {
#endif
	return gl.GetUniformLocation(program->id, name);
	} else return -1;
	
}

V_DECL void gl_uniform1f(GLProgram *program, const char *u, float v) {
	gl.Uniform1f(V_gl_uniform_location(program, u), v);
}
V_DECL void gl_uniform2f(GLProgram *program, const char *u, vec2 v) {
	gl.Uniform2f(V_gl_uniform_location(program, u), v.x, v.y);
}
V_DECL void gl_uniform3f(GLProgram *program, const char *u, vec3 v) {
	gl.Uniform3f(V_gl_uniform_location(program, u), v.x, v.y, v.z);
}
V_DECL void gl_uniform4f(GLProgram *program, const char *u, vec4 v) {
	gl.Uniform4f(V_gl_uniform_location(program, u), v.x, v.y, v.z, v.w);
}


V_DECL void gl_uniform1i(GLProgram *program, const char *u, int v) {
	gl.Uniform1i(V_gl_uniform_location(program, u), v);
}
V_DECL void gl_uniform2i(GLProgram *program, const char *u, vec2i v) {
	gl.Uniform2i(V_gl_uniform_location(program, u), v.x, v.y);
}
V_DECL void gl_uniform3i(GLProgram *program, const char *u, vec3i v) {
	gl.Uniform3i(V_gl_uniform_location(program, u), v.x, v.y, v.z);
}
V_DECL void gl_uniform4i(GLProgram *program, const char *u, vec4i v) {
	gl.Uniform4i(V_gl_uniform_location(program, u), v.x, v.y, v.z, v.w);
}

V_DECL void gl_uniformM4(GLProgram *program, const char *u, const mat4 *m) {
	gl.UniformMatrix4fv(V_gl_uniform_location(program, u), 1, 0, m->v);
}
V_DECL void gl_uniformM3(GLProgram *program, const char *u, const mat3 *m) {
	gl.UniformMatrix3fv(V_gl_uniform_location(program, u), 1, 0, m->v);
}

V_DECL void gl_program_use(GLProgram *program) {
	GLuint id = program->id;
	#if DEBUG
	GLint status = 0;
	// prevent flood of error messages if program fails to link when reloaded
	gl.GetProgramiv(id, GL_LINK_STATUS, &status);
	if (!status) id = 0;
	#endif
	gl.UseProgram(id);
	V_gl_set_program_in_use(id);

	{
		V_ConfigurableUniform *p, *end;
		for (p = program->configurable_uniforms, end = p + arr_len(program->configurable_uniforms);
			p != end; ++p) {
			switch (p->type) {
			case V_TYPE_FLOAT:
				switch (p->count) {
				case 1: gl_uniform1f(program, p->name, p->val.f[0]); break;
				case 2: gl_uniform2f(program, p->name, Vec2(p->val.f[0], p->val.f[1])); break;
				case 3: gl_uniform3f(program, p->name, Vec3(p->val.f[0], p->val.f[1], p->val.f[2])); break;
				case 4: gl_uniform4f(program, p->name, Vec4(p->val.f[0], p->val.f[1], p->val.f[2], p->val.f[3])); break;
				default: assert(0); break;
				}
				break;
			case V_TYPE_INT:
				switch (p->count) {
				case 1: gl_uniform1i(program, p->name, p->val.i[0]); break;
				case 2: gl_uniform2i(program, p->name, Vec2i(p->val.i[0], p->val.i[1])); break;
				case 3: gl_uniform3i(program, p->name, Vec3i(p->val.i[0], p->val.i[1], p->val.i[2])); break;
				case 4: gl_uniform4i(program, p->name, Vec4i(p->val.i[0], p->val.i[1], p->val.i[2], p->val.i[3])); break;
				default: assert(0); break;
				}
				break;
			default: assert(0); break;
			}
		}
	}
}

V_DECL GLIBO gl_ibo_new(const char *label) {
	GLIBO ibo = {0};
	ibo.id = V_buffer_new();
	(void)label;
#if V_GL_LABEL_OBJECTS
	if (label && gl_version_number() >= 403) {
		char full_label[256];
		strbuf_print(full_label, "IBO %s", label);
		gl.ObjectLabel(GL_BUFFER, ibo.id, -1, full_label);
	}
#endif
	return ibo;
}

V_DECL void gl_ibo_set_data_with_usage(GLIBO *ibo, const uint32_t *indices, size_t count, GLenum usage) {
	assert(count < INT32_MAX);
	ibo->count = (GLuint)count;
	gl.BindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo->id);
	gl.BufferData(GL_ELEMENT_ARRAY_BUFFER, (GLsizeiptr)(count * sizeof *indices), indices, usage);
}

V_DECL void gl_ibo_delete(GLIBO *ibo) {
	V_buffer_delete(ibo->id);
	memset(ibo, 0, sizeof *ibo);
}

// NOTE: this is just the number of textures supported for *auto-reload*, not in general
#if DEBUG
static Texture V_gl_textures[1000];
#endif

V_VARDECL const TextureConf texconf_default = {GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, true};

static void V_image_to_texture(const void *data, int bpc, int width, int height, int channels, GLuint tex_id, const TextureConf *conf) {
	uint16_t gl_channels = 0;
	GLenum gl_type = 0;

	switch (bpc) {
	case 8:  gl_type = GL_UNSIGNED_BYTE; break;
	case 16: gl_type = GL_UNSIGNED_SHORT; break;
	default: assert(0); break;
	}

	switch (channels) {
	case 1:
		gl_channels = GL_RED;
		break;
	case 2:
		gl_channels = GL_RG;
		break;
	case 3:
		gl_channels = GL_RGB;
		break;
	case 4:
		gl_channels = GL_RGBA;
		break;
	default:
		assert(0);
		break;
	}
	gl.BindTexture(GL_TEXTURE_2D, tex_id);
	gl.TexImage2D(GL_TEXTURE_2D, 0, gl_channels, width, height, 0, gl_channels,
		gl_type, data);
	gl.TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, (GLint)conf->min_filter);
	gl.TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, (GLint)conf->mag_filter);
	gl.TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, (GLint)conf->wrap_mode_s);
	gl.TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, (GLint)conf->wrap_mode_t);
	if (conf->mipmaps)
		gl.GenerateMipmap(GL_TEXTURE_2D);
}

V_DECL Texture texture_load_from_image(Image image, const TextureConf *conf) {
	Texture tex = {0};
	GLuint id = 0;
	if (image.data) {
		gl.GenTextures(1, &id);
		V_image_to_texture(image.data, 8, image.size.x, image.size.y, image.channels, id, conf);
		tex.id = id;
		tex.size = image.size;

	}
	return tex;
}

V_DECL Texture texture_load_from_image16(Image16 image, const TextureConf *conf) {
	Texture tex = {0};
	GLuint id = 0;
	if (image.data) {
		gl.GenTextures(1, &id);
		V_image_to_texture(image.data, 16, image.size.x, image.size.y, image.channels, id, conf);
		tex.id = id;
		tex.size = image.size;

	}
	return tex;
}

V_DECL Texture texture_load(const char *filename, const TextureConf *conf) {
	Image image = image_load(filename);
	Texture tex = texture_load_from_image(image, conf);

	#if DEBUG
	tex.conf = *conf;
	tex.filename = str_dup(filename);
	tex.last_modified = fs_last_modified(filename);
	{
		Texture *p = 0, *end = V_gl_textures + static_arr_len(V_gl_textures);
		for (p = V_gl_textures; p != end; ++p) {
			if (!p->id) break;
		}
		if (p != end) {
			tex.index = (int)(p - V_gl_textures);
			*p = tex;
		} else {
			tex.index = -1;
		}
	}
	#endif

	image_delete(&image);
	return tex;
}

V_DECL void gl_texture_activate(Texture texture, int unit) {
	assert(unit >= 0 && unit < 32);
	assert(texture.id);
	gl.ActiveTexture(GL_TEXTURE0 + (GLenum)unit);
	gl.BindTexture(GL_TEXTURE_2D, texture.id);
}

V_DECL void gl_texture_deactivate(int unit) {
	assert(unit >= 0 && unit < 32);
	gl.ActiveTexture(GL_TEXTURE0 + (GLenum)unit);
	gl.BindTexture(GL_TEXTURE_2D, 0);
}


V_DECL void texture_delete(Texture *texture) {
	gl.DeleteTextures(1, &texture->id);
#if DEBUG
	free(texture->filename);
	if (texture->index >= 0)
		memset(&V_gl_textures[texture->index], 0, sizeof *texture);
#endif
	memset(texture, 0, sizeof *texture);
}

#if DEBUG
// gets called by window_frame
static void V_gl_textures_reload(void) {
	Texture *p = V_gl_textures, *end = p + static_arr_len(V_gl_textures);
	for (; p != end; ++p) {
		if (p->id) {
			char *filename = p->filename;
			Time last_modified = fs_last_modified(filename);
			if (!time_eq(p->last_modified, last_modified)) {
				// image writes might take a while, so let's wait for the image editor or whatever to stop writing
				do {
					sleep_ms(100);
					p->last_modified = last_modified;
					last_modified = fs_last_modified(filename);
				} while (!time_eq(p->last_modified, last_modified));

				Image image;
				debug_print("Reloading %s\n", filename);
				image = image_load(filename);
				V_image_to_texture(image.data, 8, image.size.x, image.size.y, image.channels, p->id, &p->conf);
				image_delete(&image);
			}
		}
	}
}
#endif

static Texture V_gl_font;
static GLProgram V_gl_text_shader;

#define V_FONT_CHAR_W 9
#define V_FONT_CHAR_H 15
#define V_FONT_COLS 24
#define V_FONT_ROWS 4
#define V_FONT_IMG_W (V_FONT_CHAR_W * V_FONT_COLS)
#define V_FONT_IMG_H (V_FONT_CHAR_H * V_FONT_ROWS)

static void V_gl_font_load(void) {
	static const uint64_t bitmap[(V_FONT_IMG_W * V_FONT_IMG_H + 63) / 64] =
	{0,0,0,0,0,0,0x83e,0xf8f87f3f8f8823e,0x7d048041208f8fc,0x3e3f0f8fe4120904,0x8214211048440201,0x482080110880412,0x904824120904104,0x1209048222209008,0x2402010082080111,0x806330904824120,0x9041041209044441,0x2110082402010082,0x801120805528904,0x8241209001041111,0x428413e100827c3,0xe100fe08011c0805,0x524904fc413f0e01,0x411112410412110,0x82402011c820801,0x1408049229048041,0x2401810411112410,0x7f20900824020104,0x8208011208049219,0x480412200410410,0xa124284120900824,0x201048208011108,0x412090480512110,0x410410a124444121,0x1048440201048208,0x2110880412090480,0x492090410410a154,0x82413e0f8f87f200,0xf8823e1e104fe412,0x8f8803e208f8103,0xe040888200000000,0,0x400,0,0,0,0x3000000000000,0,0,0,0,0,0,0,0,0,0,0x20000020,0x70008018031007,0,0x20,0x200088008000,0x10010000000000,0x200000000,0x200000200088,0x80000010010000,0x200,0x3e2f0f87,0xa3e080f4bc380710,0x410762f0f8bc3d27,0xf8fc42209048201,0x30904864108108c2,0x801118104930904,0xc243189042042209,0x444012090082413,0xe108820801160104,0x9209048241109002,0x4211124283f2090,0x827f08108820801,0x1801049209048241,0x100f820421112410,0x41209008240080f0,0x8208011601049209,0x482411000420420,0xa124284330904864,0x81008208011181,0x4920904c2431010,0x422420a154443d2f,0xf87a3e080f8823e,0x211047c41208f8bc,0x3d100f81c3d04088,0x8200000000000001,0x400002100000000,0x800100000000,0,0x10400002100,0x800100,0,0xf800,0x1e000000000000,0x8001000000000000,0,0,0,0x100000,0,0x200000,0x1020,0x80e0207c7f011fc,0x3c7f0e0f80000188,0x207c000408410300,0x4020100411060820,0x1031004001111040,0x2482082241f14,0x8284824820100020,0x8a08202051008002,0x2090400002302082,0x2424948444815040,0x800209200404091,0x7880021110400000,0x209e7e24090824,0x80e0400800208200,0x80e11184bc040e10,0xc007f00020a22414,0x200030150400800,0x20820100121004c2,0x4110f4fe0000020,0xa6240e0200031248,0x4008002082020013,0xf804820820804000,0x9a7e050480,0x4a0404008002082,0x400101004820820,0x804007f000008024,0x489400440004008,0x11020804101104,0x8210110080000000,0x20802424894004a0,0x2010000e1fcfe3,0xe010f87c100e0f00,0x207c001f10,0x8003100020100000,0,0,0x4000000000010,0x2000000000000000,0,0,0,0,0,0,0,0,0,0x1e1e00000000,0x738020000,0,0x100210,0x800000020804,0x200012010807c41,0x3f80000000000000,0x1002080000800000,0x408040200012020,0x4082410080000000,0x1002080000,0x8000000408040200,0x120402082220100,0x4000001002,0x401800000000804,0x802018000801002,0x1402108fe0004000,0x10020201800000,0x10180602018001,0x8040804108040,0x40000010020200,0x10180602,0x10008080808,0x10808003f8000010,0x201000000000020,0x408020000008010,0x1008101081000040,0x100200800000,0x400804020000,0x40201008201082,0x400000100200,0x8180006060400804,0x201800020400008,0x2011840000400000,0x1002004180006060,0x8008040201800010,0x8010083f8e8fe000,0x1e1e000080,0x20000007380200,0,0x800ff00000000000,0x800020000000,0,0x108000000000,0x1000040,0,0xf0000};

	static const char vshader_code[] =
		"uniform vec2 u_offset;\n"
		"uniform float u_scale;\n"
		"uniform ivec2 u_window_size;\n"
		"attribute vec2 v_pos;\n"
		"attribute vec2 v_uv;\n"
		"varying vec2 uv;\n"
		"void main() {\n"
		"	vec2 pos = (v_pos + u_offset) * vec2(" STRINGIFY(V_FONT_CHAR_W) ".0, " STRINGIFY(V_FONT_CHAR_H) ".0) * u_scale / vec2(u_window_size) * 2.0 - 1.0;\n"
		"	uv = v_uv;\n"
		"	gl_Position = vec4(pos, 0.0, 1.0);\n"
		"}\n";
	static const char fshader_code[] =
		"varying vec2 uv;\n"
		"uniform vec3 u_color;\n"
		"uniform sampler2D texture;\n"
		"uniform vec4 u_bg_color;\n"
		"void main() {\n"
		"	if (texture2D(texture, uv).x < 0.5) {\n"
		"		if (u_bg_color.w > 0.0)"
		"			gl_FragColor = u_bg_color;\n"
		"		else\n"
		"			discard;\n"
		"	} else {\n"
		"		gl_FragColor = vec4(u_color, 1.0);\n"
		"	}\n"
		"}";

	Image img = {0};
	img.data = calloc(V_FONT_IMG_W, V_FONT_IMG_H);
	if (img.data) {
		int x, y, i = 0;
		for (y = 0; y < V_FONT_IMG_H; ++y) {
			for (x = 0; x < V_FONT_IMG_W; ++x, ++i) {
				img.data[i] = (bitmap[(unsigned)i / 64] & ((uint64_t)0x8000000000000000 >> (i%64)))
					? 255 : 0;
			}
		}
		
		img.size = Vec2i(V_FONT_IMG_W, V_FONT_IMG_H);
		img.channels = 1;
		V_gl_font.size = img.size;

		V_gl_font = texture_load_from_image(img, &texconf_default);

		{
		GLuint vshader = V_gl_shader_compile_code("textv.glsl", NULL, vshader_code, GL_VERTEX_SHADER);
		GLuint fshader = V_gl_shader_compile_code("textf.glsl", NULL, fshader_code, GL_FRAGMENT_SHADER);
		GLint status = 0;
		GLuint prog = V_gl_text_shader.id = gl.CreateProgram();
		gl.AttachShader(prog, vshader);
		gl.AttachShader(prog, fshader);
		gl.LinkProgram(prog);
		gl.GetProgramiv(prog, GL_LINK_STATUS, &status);
		gl.DetachShader(prog, vshader);
		gl.DetachShader(prog, fshader);
		assert(status);
		}


		image_delete(&img);


	}
}

typedef struct {
	vec2 pos;
	vec2 uv;
} V_TextVertex;

// make sure you zero *text before calling this for the first time
V_DECL void gl_text_set(GLText *text, const char *str) {
	static const struct {
		unsigned char u, v;
	} text_positions[] = {
		[' '] = {255, 255},

		['A'] = { 0, 0}, ['B'] = { 1, 0}, ['C'] = { 2, 0}, ['D'] = { 3, 0},
		['E'] = { 4, 0}, ['F'] = { 5, 0}, ['G'] = { 6, 0}, ['H'] = { 7, 0},
		['I'] = { 8, 0}, ['J'] = { 9, 0}, ['K'] = {10, 0}, ['L'] = {11, 0},
		['M'] = {12, 0}, ['N'] = {13, 0}, ['O'] = {14, 0}, ['P'] = {15, 0},
		['Q'] = {16, 0}, ['R'] = {17, 0}, ['S'] = {18, 0}, ['T'] = {19, 0},
		['U'] = {20, 0}, ['V'] = {21, 0}, ['W'] = {22, 0}, ['X'] = {23, 0},

		['a'] = { 0, 1}, ['b'] = { 1, 1}, ['c'] = { 2, 1}, ['d'] = { 3, 1},
		['e'] = { 4, 1}, ['f'] = { 5, 1}, ['g'] = { 6, 1}, ['h'] = { 7, 1},
		['i'] = { 8, 1}, ['j'] = { 9, 1}, ['k'] = {10, 1}, ['l'] = {11, 1},
		['m'] = {12, 1}, ['n'] = {13, 1}, ['o'] = {14, 1}, ['p'] = {15, 1},
		['q'] = {16, 1}, ['r'] = {17, 1}, ['s'] = {18, 1}, ['t'] = {19, 1},
		['u'] = {20, 1}, ['v'] = {21, 1}, ['w'] = {22, 1}, ['x'] = {23, 1},

		['`'] = { 0, 2}, ['0'] = { 1, 2}, ['1'] = { 2, 2}, ['2'] = { 3, 2},
		['3'] = { 4, 2}, ['4'] = { 5, 2}, ['5'] = { 6, 2}, ['6'] = { 7, 2},
		['7'] = { 8, 2}, ['8'] = { 9, 2}, ['9'] = {10, 2}, ['-'] = {11, 2},
		['='] = {12, 2}, ['~'] = {13, 2}, ['!'] = {14, 2}, [64 /* at sign */] = {15, 2},
		['#'] = {16, 2}, ['$'] = {17, 2}, ['%'] = {18, 2}, ['^'] = {19, 2},
		['&'] = {20, 2}, ['*'] = {21, 2}, ['('] = {22, 2}, [')'] = {23, 2},

		['['] = { 0, 3}, [']'] = { 1, 3}, ['\\']= { 2, 3}, [';'] = { 3, 3},
		['\'']= { 4, 3}, [','] = { 5, 3}, ['.'] = { 6, 3}, ['/'] = { 7, 3},
		['{'] = { 8, 3}, ['}'] = { 9, 3}, ['|'] = {10, 3}, [':'] = {11, 3},
		['"'] = {12, 3}, ['<'] = {13, 3}, ['>'] = {14, 3}, ['?'] = {15, 3},
		['Y'] = {16, 3}, ['Z'] = {17, 3}, ['y'] = {18, 3}, ['z'] = {19, 3},
		['_'] = {20, 3}, ['+'] = {21, 3}
	};
	GLuint i, len = (GLuint)strlen(str);
	V_TextVertex *vertices = calloc(4 * len, sizeof *vertices), *vptr = vertices;
	uint32_t *indices = calloc(6 * len, sizeof *indices), *iptr = indices;

	GLVBO *vbo = &text->vbo;
	GLVAO *vao = &text->vao;
	GLIBO *ibo = &text->ibo;
	if (!vbo->id)
		*vbo = gl_vbo_new(V_TextVertex, "text");
	if (!vao->id)
		*vao = gl_vao_new(&V_gl_text_shader, "text");
	if (!ibo->id)
		*ibo = gl_ibo_new("IBO text");
	if (vertices && indices) {
		float du = 1.0f / V_FONT_COLS, dv = 1.0f / V_FONT_ROWS;
		float x = 0, y = 0;
		for (i = 0; i < len; ++i) {
			char c = str[i];
			if (c == '\n') {
				x = 0;
				y -= 1;
			} else {
				// NOTE: we generate vertices, even for ' ', to support background colors
				float u1 = du * (float)text_positions[(int)c].u;
				float v2 = dv * (float)text_positions[(int)c].v;
				float u2 = u1 + du, v1 = v2 + dv;
				float x1 = x, x2 = x1 + 1;
				float y1 = y, y2 = y1 + 1;
				GLuint i0;
				V_TextVertex t1, t2, t3, t4;
				t1.pos = Vec2(x1, y1); t1.uv = Vec2(u1, v1);
				t2.pos = Vec2(x2, y1); t2.uv = Vec2(u2, v1);
				t3.pos = Vec2(x2, y2); t3.uv = Vec2(u2, v2);
				t4.pos = Vec2(x1, y2); t4.uv = Vec2(u1, v2);
				i0 = (GLuint)(vptr - vertices);
				*vptr++ = t1;
				*vptr++ = t2;
				*vptr++ = t3;
				*vptr++ = t4;
				*iptr++ = i0+0;
				*iptr++ = i0+1;
				*iptr++ = i0+2;
				*iptr++ = i0+0;
				*iptr++ = i0+2;
				*iptr++ = i0+3;
				x += 1;
			}
		}
	}

	gl_vao_clear(vao);
	gl_vbo_set_static_data(vbo, vertices, (size_t)(vptr - vertices));
	gl_ibo_set_static_data(ibo, indices,  (size_t)(iptr - indices));
	gl_vao_add_data2f(vao, *vbo, "v_pos", V_TextVertex, pos);
	gl_vao_add_data2f(vao, *vbo, "v_uv", V_TextVertex, uv);

	free(vertices);
	free(indices);
}

// if bg_color.w == 0, a transparent background will be used, even if alpha blending is off (via discard)
V_DECL void gl_text_render(const GLText *text, vec2 pos, float scale, vec3 color, vec4 bg_color) {
	vec2i wsize = window_get_size();
	if (scale == 0) scale = 1;
	vec2 text_grid_size = div2(vec2_from_2i(wsize), Vec2(scale * V_FONT_CHAR_W, scale * V_FONT_CHAR_H));
	pos = mod2(pos, text_grid_size);
	gl_program_use(&V_gl_text_shader);
	gl_uniform2f(&V_gl_text_shader, "u_offset", pos);
	gl_uniform1f(&V_gl_text_shader, "u_scale", scale);
	gl_uniform2i(&V_gl_text_shader, "u_window_size", wsize);
	gl_uniform3f(&V_gl_text_shader, "u_color", color);
	gl_uniform4f(&V_gl_text_shader, "u_bg_color", bg_color);
	gl_texture_activate(V_gl_font, 0);
	gl_uniform1i(&V_gl_text_shader, "u_texture", 0);
	gl.DepthFunc(GL_ALWAYS); // disable depth testing
	gl_vao_render(text->vao, &text->ibo);
	gl.DepthFunc(GL_LESS);
}

static void V_gl_font_delete(void) {
	texture_delete(&V_gl_font);
}

V_DECL void gl_text_delete(GLText *text) {
	gl_ibo_delete(&text->ibo);
	gl_vbo_delete(&text->vbo);
	gl_vao_delete(&text->vao);
}


V_DECL uint64_t gl_vertices_drawn_this_frame(void) {
	return V_vertex_count;
}

// returns dt since last frame
V_DECL float window_frame(void) {
	vec2i window_size;
	Time now;
	float dt;


#if DEBUG
	static GLProgram *program;
	static float last_dt;
	if (V_gl_uniform_editor_enabled) {
		if (window_n_key_presses(KEY_U)) {
			V_gl_uniform_editor_shown = !V_gl_uniform_editor_shown;
			program = NULL;
		}
	}
	if (V_gl_uniform_editor_shown) {
		// uniform editor
		static int sel;
		static int sel_comp = 0;
		static int nlines = 0;
		int sel_text_col = 0;
		int dsel = window_n_key_presses(KEY_DOWN) - window_n_key_presses(KEY_UP);
		int dcomp = window_n_key_presses(KEY_RIGHT) - window_n_key_presses(KEY_LEFT);
		char text[8192];
		char text_hl[256];
		vec4 editor_bg = {0, 0, 0, 1};
		vec3 editor_fg = {1, 1, 1};
		vec3 editor_hl = {1, 0, 1};
		if (nlines)
			sel = modi(sel + dsel, nlines);
		else
			sel = 0;
		if (program && sel < (int)arr_len(program->configurable_uniforms)) {
			int ncomp = program->configurable_uniforms[sel].count;
			if (ncomp)
				sel_comp = modi(sel_comp + dcomp, ncomp);
			else
				sel_comp = 0;
		}
		*text = '\0';
		*text_hl = '\0';
		if (program) {
			V_ConfigurableUniform *u, *end;

			nlines = 0;
			for (u = program->configurable_uniforms, end = u + arr_len(program->configurable_uniforms);
				u != end; ++u) {
				int c;
				strbuf_append(text, "%s: ", u->name);
				for (c = 0; c < u->count; ++c) {
					char value[64];
					if (u->count > 1)
						strbuf_cat(text, c == 0 ? "(" : ", ");
					if (u->type == V_TYPE_INT)
						strbuf_print(value, "%d", u->val.i[c]);
					else
						strbuf_print(value, "%f", u->val.f[c]);
					if (nlines == sel && c == sel_comp) {
						// this is the component we're currently editing!
						char spcs[sizeof value + 1] = {0};
						char *start_of_line = strrchr(text, '\n');
						memset(spcs, ' ', strlen(value));
						if (start_of_line)
							++start_of_line;
						else
							start_of_line = text;
						sel_text_col = (int)strlen(start_of_line);

						strbuf_cpy(text_hl, value);
						strbuf_cat(text, spcs);
						{
							double minv = u->min_val;
							double maxv = u->max_val;
							if (u->type == V_TYPE_INT) {
								// edit integer value
								int dvalue = window_n_key_presses(KEY_E) - window_n_key_presses(KEY_Q);
								u->val.i[c] += dvalue;
								u->val.i[c] = clampi(u->val.i[c], (int)minv, (int)maxv);
							} else {
								// edit float value
								float range = (float)(maxv - minv);
								float change_speed = range * (window_is_shift_down() ? 0.1f : 0.4f);
								float dvalue = last_dt * change_speed * (float)(window_is_key_down(KEY_E) - window_is_key_down(KEY_Q));
								u->val.f[c] += dvalue;
								u->val.f[c] = clamp(u->val.f[c], (float)minv, (float)maxv);
							}
						}
					} else {
						strbuf_cat(text, value);
					}
					if (c == u->count-1 && u->count > 1)
						strbuf_cat(text, ")");
				}
				strbuf_cat(text, "\n");
				++nlines;
			}
		} else {
			// program selector
			GLProgram *p, *end;
			nlines = 0;
			for (p = V_gl_programs, end = p + static_arr_len(V_gl_programs);
				p != end; ++p) {
				if (p->configurable_uniforms) {
					char name[64], *trimmed_name;
					char *dot;
					strbuf_cpy(name, p->vfilename);
					dot = strrchr(name, '.');
					if (dot > name)
						dot[-1] = '\0'; // remove "v.glsl" or "f.glsl" suffix
					trimmed_name = name + (str_is_prefix(name, "assets/") ? 7 : 0);
					if (nlines == sel) {
						strbuf_cpy(text_hl, trimmed_name);
						if (window_n_key_presses(KEY_ENTER)) {
							program = p;
						}
					} else {
						strbuf_cat(text, trimmed_name);
					}
					strbuf_cat(text, "\n");
					++nlines;
				}
			}

		}

		gl_text_set(&V_gl_uniform_editor_text, text);
		{
			vec2 pos = Vec2(0, (float)nlines - 1);
			gl_text_render(&V_gl_uniform_editor_text, pos, 0, editor_fg, editor_bg);
		}
		if (*text_hl) {
			gl_text_set(&V_gl_uniform_editor_text_highlighted, text_hl);
			{
				vec2 pos = Vec2((float)sel_text_col, (float)(nlines - 1 - sel));
				gl_text_render(&V_gl_uniform_editor_text_highlighted, pos, 0, editor_hl, editor_bg);
			}
		}
	}
#endif

	memset(V_key_presses, 0, sizeof V_key_presses);
	memset(V_mouse_buttons_pressed, 0, sizeof V_mouse_buttons_pressed);
	
	SDL_GL_SwapWindow(V_sdl_window);
	gl.ClearColor(0, 0, 0, 1);
	gl.Clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
#if DEBUG
	V_gl_textures_reload();
	V_gl_programs_reload();
#endif
	window_size = window_get_size();
	gl.Viewport(0, 0, window_size.x, window_size.y);

	now = time_now();
	if (V_last_frame.tv_sec == 0)
		dt = 1.f/60; // just for the first frame, use dt = 1/60 s
	else
		dt = (float)time_sub(now, V_last_frame);
#if DEBUG
	last_dt = dt;
#endif
	V_last_frame = now;
	V_vertex_count = 0;

	dt = clamp(dt, 0.001f, 0.1f); // prevent really short/long frames
	return dt;
}
typedef struct {
	vec3 pos;
	vec3 normal;
	vec2 uv;
} V_ModelVertex;

V_DECL bool model_load(Model *model, GLProgram *program, const char *filename) {
	char *dot = strchr(filename, '.');
	
	memset(model, 0, sizeof *model);

	if (dot) {
		FILE *fp = fopen(filename, "r");
		if (fp) {
			char line[1024] = {0};
			vec3 *positions = 0;
			vec2 *uvs = 0;
			vec3 *normals = 0;
			V_ModelVertex *vertices = 0;
			
			while (fgets(line, sizeof line, fp)) {
				char *spc = strchr(line, ' ');
				if (!spc) continue;
				*spc = '\0';
				char const *operation = line;
				char const *operands = spc + 1;
				switch (operation[0]) {
				case 'v': if (operation[1] == '\0' || operation[2] == '\0') switch (operation[1]) {
					case '\0': {
						float x = 0, y = 0, z = 0;
						if (sscanf(operands, "%f %f %f", &x, &y, &z) != 3) {
							debug_print("Couldn't parse .obj v operands: %s\n", operands);
						}
						arr_add(positions, Vec3(x, y, z));
					} break;
					case 't': {
						float u = 0, v = 0;
						if (sscanf(operands, "%f %f", &u, &v) != 2) {
							debug_print("Couldn't parse .obj vt operands: %s\n", operands);
						}
						arr_add(uvs, Vec2(u, v));
					} break;
					case 'n': {
						float x = 0, y = 0, z = 0;
						if (sscanf(operands, "%f %f %f", &x, &y, &z) != 3) {
							debug_print("Couldn't parse .obj vn operands: %s\n", operands);
						}
						arr_add(normals, Vec3(x, y, z));
					} break;
				}
				break;
				case 'f':
					if (operation[1] == '\0') {
						int bytes_read = 0;
						unsigned pos_idx = 0, uv_idx = 0, norm_idx = 0;
						int polygon_nvertices = 0;
						while (sscanf(operands, "%u/%u/%u%n", &pos_idx, &uv_idx, &norm_idx, &bytes_read) == 3
							|| sscanf(operands, "%u//%u%n", &pos_idx, &norm_idx, &bytes_read) == 2) {
							++polygon_nvertices;
							if (polygon_nvertices > 3) {
								debug_print("Error: Please triangulate mesh %s.\n", filename);
								break;
							}
							if (pos_idx < 1 || pos_idx > arr_len(positions)) {
								debug_print("Vertex has invalid position in %s\n", filename);
							} else if (norm_idx < 1 || norm_idx > arr_len(normals)) {
								debug_print("Invalid normal index in %s\n", filename);
							} else if (uv_idx > arr_len(uvs)) {
								debug_print("Invalid UV index in %s\n", filename);
							} else {
								V_ModelVertex *v = arr_addp(vertices);
								bool has_uv = uv_idx != 0;
								--pos_idx;
								--norm_idx;
								if (has_uv)
									--uv_idx;
								v->pos = positions[pos_idx];
								v->uv = has_uv ? uvs[uv_idx] : Vec2(0, 0);
								v->normal = normals[norm_idx];
							}
							operands += bytes_read;
						}
					}
					break;
				}
				
			}
			
			{
			size_t i;
			vec3 min_coord = {1e+30f, 1e+30f, 1e+30f}, max_coord = {-1e+30f, -1e+30f, -1e+30f};
			for (i = 0; i < arr_len(positions); ++i) {
				vec3 pos = positions[i];
				min_coord = min3(min_coord, pos);
				max_coord = max3(max_coord, pos);
			}
			model->min_coord = min_coord;
			model->max_coord = max_coord;
			}
			
			{
				char label[64];
				strbuf_print(label, "%s", filename);
				GLVBO vbo = gl_vbo_new(V_ModelVertex, label);
				strbuf_print(label, "%s", filename);
				GLVAO vao = gl_vao_new(program, label);
				gl_vbo_set_static_data(&vbo, vertices, arr_len(vertices));
				gl_vao_add_data3f(&vao, vbo, "v_pos", V_ModelVertex, pos);
				gl_vao_add_data3f(&vao, vbo, "v_normal", V_ModelVertex, normal);
				if (gl_program_has_attrib(program, "v_uv"))
					gl_vao_add_data2f(&vao, vbo, "v_uv", V_ModelVertex, uv);
				
				model->vbo = vbo;
				model->vao = vao;
			}

			arr_free(vertices);
			arr_free(positions);
			arr_free(uvs);
			arr_free(normals);
			
			fclose(fp);

			return true;
		} else {
			debug_print("Couldn't find model %s\n", filename);
			return false;
		}
	} else {
		debug_print("Model %s has an unrecognized file format.\n", filename);
		return false;
	}
}

// note: you have to supply your own offset/etc. (i don't know how your shader program works)
static void model_render(Model *model) {
	gl_vao_render(model->vao, NULL);
}

static void model_delete(Model *model) {
	gl_vbo_delete(&model->vbo);
	gl_vao_delete(&model->vao);
}

#endif // V_GL

#if __cplusplus
}
#endif


#if V_WINDOWED // windowed application -- this defines WinMain for you
	int main_(int argc, char **argv);

	#if _WIN32
	INT WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
		PSTR lpCmdLine, INT nCmdShow) {
		(void)hInstance; (void)hPrevInstance; (void)lpCmdLine; (void)nCmdShow;
		int argc = 0;
		LPWSTR* wide_argv = CommandLineToArgvW(GetCommandLineW(), &argc);
		char** argv = malloc(argc * sizeof *argv);
		if (!argv) {
			return -1;
		}
		for (int i = 0; i < argc; i++) {
			LPWSTR wide_arg = wide_argv[i];
			int len = (int)wcslen(wide_arg);
			argv[i] = malloc(len + 1);
			if (!argv[i]) {
				return -1;
			}
			// @TODO(eventually) : proper unicode handling
			for (int j = 0; j <= len; j++)
				argv[i][j] = (char)wide_arg[j];
		}
		LocalFree(wide_argv);
		return main_(argc, argv);
	}
	#else
	int main(int argc, char **argv) {
		return main_(argc, argv);
	}
	#endif
	#define main main_
#endif


#endif // V_LIB_H_
