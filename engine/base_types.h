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

// typedef struct m4 {
//   f32 a[4 * 4];
// } m4;

typedef f32 *v2;
typedef f32 *v3;
typedef f32 *v4;
typedef f32 *m4;
typedef f32 *rect;

// static inline v2Init

m4 m4Ortho(m4 m, f32 left, f32 right, f32 bottom, f32 top, f32 near, f32 far) {
  const f32 *expect =
      glm::value_ptr(glm::ortho(left, right, bottom, top, near, far));
  for (i32 i = 0; i < 16; i++) {
    m[i] = expect[i];
  }
  return m;
}

typedef struct logEvent {
  va_list ap;
  const char *fmt;
  const char *file;
  int line;
  int level;
} logEvent;

static logEvent newLogEvent(const char *fmt, const char *file, int line,
                            int level) {
  logEvent ev = {};
  ev.fmt = fmt;
  ev.file = file;
  ev.line = line;
  ev.level = ev.level;
  return ev;
}

enum { LOG_TRACE, LOG_DEBUG, LOG_INFO, LOG_WARN, LOG_ERROR, LOG_FATAL };
void LogLog(int level, const char *file, int line, const char *fmt, ...);

// https://c-faq.com/varargs/handoff.html
void vlog(logEvent *ev) {
  fprintf(stderr, "%d: %s:%d: ", ev->level, ev->file, ev->line);
  vfprintf(stderr, ev->fmt, ev->ap);
  fprintf(stderr, "\n");
  fflush(stderr);
}

void LogLog(int level, const char *file, int line, const char *fmt, ...) {
  logEvent ev = newLogEvent(fmt, file, line, level);
  va_start(ev.ap, fmt);
  vlog(&ev);
  va_end(ev.ap);
}

#define LogError(...) LogLog(LOG_ERROR, __FILE__, __LINE__, __VA_ARGS__)

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

#endif