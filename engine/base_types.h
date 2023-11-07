#ifndef BASE_TYPES_H
#define BASE_TYPES_H

#define AlignUpPow2(x, p) (((x) + (p)-1) & ~((p)-1))
#define AlignDownPow2(x, p) ((x) & ~((p)-1))
#define ArrayCount(a) (sizeof(a) / sizeof(*(a)))

#define Min(a, b) (((a) < (b)) ? (a) : (b))
#define Max(a, b) (((a) > (b)) ? (a) : (b))
#define ClampTop(a, b) Min(a, b)

#define KB(x) ((x) << 10)
#define MB(x) ((x) << 20)
#define GB(x) ((x) << 30)
#define TB(x) ((x) << 40)

#include <stdint.h>

typedef unsigned int uint;
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef int32_t i32;
typedef int8_t i8;
typedef float f32;
typedef int8_t b8;

#include <string.h>
#define MemoryZero(p, z) memset((p), 0, (z))
#define FieldPtr(it, T, offset) ((T *)((u8 *)(it) + (offset)))

// typedef struct m4 {
//   f32 a[4 * 4];
// } m4;

typedef f32 *v2;
typedef f32 *v3;
typedef f32 *v4;
typedef f32 *m2;
typedef f32 *m4;
typedef f32 *rect;

// static inline v2Init

static m4 m4Ortho(m4 m, f32 ll, f32 rr, f32 bb, f32 tt, f32 nn, f32 ff) {
  const f32 *expect = glm::value_ptr(glm::ortho(ll, rr, bb, tt, nn, ff));
  for (i32 i = 0; i < 16; i++) {
    m[i] = expect[i];
  }
  return m;
}

static m4 m4Perspective(m4 m, f32 fovRadians, f32 aspect, f32 zNear, f32 zFar) {
  const f32 *expect =
      glm::value_ptr(glm::perspective(fovRadians, aspect, zNear, zFar));
  for (i32 i = 0; i < 16; i++) {
    m[i] = expect[i];
  }
  return m;
}

m4 m4Copy(m4 dest, m4 src) {
  for (i32 i = 0; i < 16; i++) {
    dest[i] = src[i];
  }
  return dest;
}

typedef struct logEvent {
  va_list ap;
  const char *fmt;
  const char *file;
  int line;
  int level;
} logEvent;

// https://github.com/rxi/log.c/blob/master/src/log.c
static logEvent newLogEvent(const char *fmt, const char *file, int line,
                            int level) {
  logEvent ev = {};
  ev.fmt = fmt;
  ev.file = file;
  ev.line = line;
  ev.level = level;
  return ev;
}

enum { LOG_TRACE, LOG_DEBUG, LOG_INFO, LOG_WARN, LOG_ERROR, LOG_FATAL };

static const char *levelStrings[] = {
    "TRACE", "DEBUG", "INFO", "WARN", "ERROR", "FATAL",
};

void LogLog(int level, const char *file, int line, const char *fmt, ...);

// https://c-faq.com/varargs/handoff.html
void vlog(logEvent *ev) {
  fprintf(stderr, "%s: %s:%d: ", levelStrings[ev->level], ev->file, ev->line);
  vfprintf(stderr, ev->fmt, ev->ap);
  fprintf(stderr, "\n");
  fflush(stderr);
}

extern "C" __declspec(dllimport) void __stdcall OutputDebugStringA(
    const char *lpOutputString);

void __cdecl vlog_ms(logEvent *ev) {

  char buf[1024];
  buf[1023] = '\0';
  char *pl = &buf[1023];
  char *p = buf;
  i32 written = 0;

  written =
      snprintf(buf, sizeof(buf) - 3, "%s: %s:%d: ", levelStrings[ev->level],
               ev->file, ev->line);
  while (*p != '\0')
    p++;

  u32 avail = u32(pl - p - 3);
  written = _vsnprintf_s(p, avail, avail, ev->fmt, ev->ap); // -3 for cr/lf,null

  while (p + 2 < pl && *p != '\0')
    p++;

  *p++ = '\r';
  *p++ = '\n';
  *p++ = '\0';

  OutputDebugStringA(buf);
}

void LogLog(int level, const char *file, int line, const char *fmt, ...) {
  logEvent ev = newLogEvent(fmt, file, line, level);
  va_start(ev.ap, fmt);
  vlog_ms(&ev);
  va_end(ev.ap);
}

#define LogInfo(...) LogLog(LOG_INFO, __FILE__, __LINE__, __VA_ARGS__)
#define LogError(...) LogLog(LOG_ERROR, __FILE__, __LINE__, __VA_ARGS__)
#define LogErrorw(...) LogLogw(LOG_ERROR, __FILE__, __LINE, __VA_ARGS__)

v2 v2Clamp(v2 v, rect r) {
  if (v[0] < r[0]) {
    v[0] = r[0];
  }

  if (v[0] > r[0] + r[2]) {
    v[0] = r[0] + r[2];
  }

  if (v[1] < r[1]) {
    v[1] = r[1];
  }

  if (v[1] > r[1] + r[3]) {
    v[1] = r[1] + r[3];
  }

  return v;
}

v4 v4Set(v4 v, f32 x, f32 y, f32 z, f32 w) {
  v[0] = x;
  v[1] = y;
  v[2] = z;
  v[3] = w;
  return v;
}

v4 v4Copy(v4 v, v4 other) {
  v[0] = other[0];
  v[1] = other[1];
  v[2] = other[2];
  v[3] = other[3];
  return v;
}

b8 rectOverlaps(rect a, rect b) {
  b8 x = (a[0] >= b[0] && a[0] <= b[0] + b[2]) || // ax within bx range;
         (a[0] + a[2] >= b[0] &&
          a[0] + a[2] <= b[0] + b[2]) || // axw within bx range;
         (a[0] <= b[0] &&
          a[0] + a[2] >=
              b[0] + b[2]); // ax and axw outside bx range on different side
  b8 y = (a[1] >= b[1] && a[1] <= b[1] + b[3]) || // ay within by range;
         (a[1] + a[3] >= b[1] &&
          a[1] + a[3] <= b[1] + b[3]) || // ayh within bx range;
         (a[1] <= b[1] &&
          a[1] + a[3] >=
              b[1] + b[3]); // ay and ayh outside by range on different side

  return x && y;
}

b8 rectContainedBy(rect inner, rect outer) {
  b8 x = (inner[0] >= outer[0] && inner[0] <= outer[0] + outer[2]) &&
         (inner[0] + inner[2] >= outer[0] &&
          inner[0] + inner[2] <= outer[0] + outer[2]);
  b8 y = (inner[1] >= outer[1] && inner[1] <= outer[1] + outer[3]) &&
         (inner[1] + inner[3] >= outer[1] &&
          inner[1] + inner[3] <= outer[1] + outer[3]);
  return x && y;
}

rect rectGetOverlap(rect result, rect a, rect b) {
  f32 mi[2] = {Max(a[0], b[0]), Max(a[1], b[1])};
  f32 ma[2] = {Min(a[0] + a[2], b[0] + b[2]), Min(a[1] + a[3], b[1] + b[3])};
  result[0] = mi[0];
  result[1] = mi[1];
  result[2] = ma[0] - mi[0];
  result[3] = ma[1] - mi[1];
  return result;
}

rect rectUVCull(rect uv, rect quad, rect cullUV) {
  if (!rectOverlaps(quad, cullUV) || rectContainedBy(quad, cullUV)) {
    return uv;
  }

  b8 xShiftConstant =
      !(quad[0] >= cullUV[0] && quad[0] <= cullUV[0] + cullUV[2]);
  b8 yShiftConstant =
      !(quad[1] >= cullUV[1] && quad[1] <= cullUV[1] + cullUV[3]);

  f32 uvXRatio = uv[2] / quad[2];
  f32 uvYRatio = uv[3] / quad[3];
  f32 overlap[4];
  rectGetOverlap(overlap, quad, cullUV);
  uv[0] = uv[0] + (quad[2] - overlap[2]) * uvXRatio * xShiftConstant;
  uv[1] = uv[1] + (quad[3] - overlap[3]) * uvYRatio * yShiftConstant;
  uv[2] = overlap[2] * uvXRatio;
  uv[3] = overlap[3] * uvYRatio;
  return uv;
}

static void v2Copy(v2 a, v2 other) {
  a[0] = other[0];
  a[1] = other[1];
}

static b8 f32Equal(f32 a, f32 b) { return 0.01f > (a - b) && (a - b) > -0.01f; }

static f32 v2DistanceSquare(v2 f) { return f[0] * f[0] + f[1] * f[1]; }
static f32 v2DistanceSquare2(v2 to, v2 from) {
  return (to[0] - from[0]) * (to[0] - from[0]) +
         (to[1] - from[1]) * (to[1] - from[1]);
}

static void v2Add(v2 a, v2 other) {
  a[0] = a[0] + other[0];
  a[1] = a[1] + other[1];
}

static void v2Subtract(v2 a, v2 other) {
  a[0] = a[0] - other[0];
  a[1] = a[1] - other[1];
}

static void v2Mult(v2 a, f32 other) {
  a[0] = a[0] * other;
  a[1] = a[1] * other;
}

static f32 v2Distance(v2 f) { return sqrt(f[0] * f[0] + f[1] * f[1]); }
static f32 v3Distance(v3 f) {
  return sqrt(f[0] * f[0] + f[1] * f[1] + f[2] * f[2]);
}

static void v2Normalize(v2 a) {
  f32 dst = v2Distance(a);
  a[0] = a[0] / dst;
  a[1] = a[1] / dst;
}

static void v3Normalize(v3 a) {
  f32 dst = v3Distance(a);
  a[0] = a[0] / dst;
  a[1] = a[1] / dst;
  a[2] = a[2] / dst;
}

// m2 - column based storage
static void m2Rotate(m2 m, f32 rad) {
  f32 c = cos(rad);
  f32 s = sin(rad);
  m[0] = c;
  m[1] = s;
  m[2] = -s;
  m[3] = c;
}

// static void m2Translate(m2 m, v2 v) {

// }

static void m2MultiplyPoint(m2 m, v2 v) {
  f32 px = v[0];
  v[0] = m[0] * px + m[2] * v[1];
  v[1] = m[1] * px + m[3] * v[1];
}

#define mathPI 3.141592653589f

static f32 mathAtan2(f32 y, f32 x) {
  return atan2(y, x);
  // if (x < 0.01f && x > -0.01f) {
  //   if (y > 0.0f) {
  //     return mathPI / 2.0f;
  //   } else {
  //     return -mathPi / 2.0f;
  //   }
  // }
}

static f32 mathRadians(f32 degrees) { return degrees / 180 * mathPI; }

inline void v3Neg(v3 out) {
  out[0] = -out[0];
  out[1] = -out[1];
  out[2] = -out[2];
}

static void v3Cross(v3 out, v3 other) {
  f32 x = out[1] * other[2] - other[1] * out[2];
  f32 y = out[2] * other[0] - other[2] * out[0];
  f32 z = out[0] * other[1] - other[0] * out[1];
  out[0] = x;
  out[1] = y;
  out[2] = z;
}

inline void v3Copy(v3 out, v3 other) {
  out[0] = other[0];
  out[1] = other[1];
  out[2] = other[2];
}

inline void v3Subtract(v3 out, v3 other) {
  out[0] -= other[0];
  out[1] -= other[1];
  out[2] -= other[2];
}

inline void m4Translate(m4 out, v3 translate) {
  out[3] += translate[0];
  out[4+3] += translate[1];
  out[8+3] += translate[2];
}

inline void m4One(m4 out) {
  for (i32 i = 0; i < 16; i++) {
    if (i/4 == i%4)
      out[i] = 1;
    else
      out[i] = 0;
  }
}

#endif