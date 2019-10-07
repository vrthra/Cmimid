#include "mjs_extra.h"
void cs_log_set_level(enum cs_log_level level) __attribute__((weak));
void cs_log_set_level(enum cs_log_level level) {
  cs_log_level = level;



}
char *cs_read_file(const char *path, size_t *size) __attribute__((weak));
char *cs_read_file(const char *path, size_t *size) {
  FILE *fp;
  char *data = 
              __null
                  ;
  if ((fp = fopen(path, "rb")) == 
                                 __null
                                     ) {
  } else if (fseek(fp, 0, 
                         2
                                 ) != 0) {
    fclose(fp);
  } else {
    *size = ftell(fp);
    data = (char *) malloc(*size + 1);
    if (data != 
               __null
                   ) {
      fseek(fp, 0, 
                  0
                          );
      if (fread(data, 1, *size, fp) != *size) {
        free(data);
        return 
              __null
                  ;
      }
      data[*size] = '\0';
    }
    fclose(fp);
  }
  return data;
}
size_t cs_varint_llen(uint64_t num) {
  size_t llen = 0;

  do {
    llen++;
  } while (num >>= 7);

  return llen;
}

size_t cs_varint_encode(uint64_t num, uint8_t *buf, size_t buf_size) {
  size_t llen = 0;

  do {
    uint8_t byte = num & 0x7f;
    num >>= 7;
    if (num != 0) byte |= 0x80;
    if (llen < buf_size) *buf++ = byte;
    llen++;
  } while (num != 0);

  return llen;
}

bool cs_varint_decode(const uint8_t *buf, size_t buf_size, uint64_t *num,
                      size_t *llen) {
  size_t i = 0, shift = 0;
  uint64_t n = 0;

  do {
    if (i == buf_size || i == (8 * sizeof(*num) / 7 + 1)) return false;




    n |= ((uint64_t)(buf[i] & 0x7f)) << shift;




    i++;
    shift += 7;
  } while (shift < sizeof(uint64_t) * 8 && (buf[i - 1] & 0x80));

  *num = n;
  *llen = i;

  return true;
}

uint64_t cs_varint_decode_unsafe(const uint8_t *buf, int *llen) {
  uint64_t v;
  size_t l;
  cs_varint_decode(buf, ~0, &v, &l);
  *llen = l;
  return v;
}
void mbuf_init(struct mbuf *mbuf, size_t initial_size) __attribute__((weak));
void mbuf_init(struct mbuf *mbuf, size_t initial_size) {
  mbuf->len = mbuf->size = 0;
  mbuf->buf = 
             __null
                 ;
  mbuf_resize(mbuf, initial_size);
}

void mbuf_free(struct mbuf *mbuf) __attribute__((weak));
void mbuf_free(struct mbuf *mbuf) {
  if (mbuf->buf != 
                  __null
                      ) {
    free(mbuf->buf);
    mbuf_init(mbuf, 0);
  }
}

void mbuf_resize(struct mbuf *a, size_t new_size) __attribute__((weak));
void mbuf_resize(struct mbuf *a, size_t new_size) {
  if (new_size > a->size || (new_size < a->size && new_size >= a->len)) {
    char *buf = (char *) realloc(a->buf, new_size);





    if (buf == 
              __null 
                   && new_size != 0) return;
    a->buf = buf;
    a->size = new_size;
  }
}

void mbuf_trim(struct mbuf *mbuf) __attribute__((weak));
void mbuf_trim(struct mbuf *mbuf) {
  mbuf_resize(mbuf, mbuf->len);
}

size_t mbuf_insert(struct mbuf *a, size_t off, const void *buf, size_t) __attribute__((weak));
size_t mbuf_insert(struct mbuf *a, size_t off, const void *buf, size_t len) {
  char *p = 
           __null
               ;

  
 (static_cast <bool> (
 a != 
 __null) ? void (0) : __assert_fail (
 "a != NULL"
 , "mjs.c", 231, __extension__ __PRETTY_FUNCTION__))
                  ;
  
 (static_cast <bool> (
 a->len <= a->size
 ) ? void (0) : __assert_fail (
 "a->len <= a->size"
 , "mjs.c", 232, __extension__ __PRETTY_FUNCTION__))
                          ;
  
 (static_cast <bool> (
 off <= a->len
 ) ? void (0) : __assert_fail (
 "off <= a->len"
 , "mjs.c", 233, __extension__ __PRETTY_FUNCTION__))
                      ;


  if (~(size_t) 0 - (size_t) a->buf < len) return 0;

  if (a->len + len <= a->size) {
    memmove(a->buf + off + len, a->buf + off, a->len - off);
    if (buf != 
              __null
                  ) {
      memcpy(a->buf + off, buf, len);
    }
    a->len += len;
  } else {
    size_t min_size = (a->len + len);
    size_t new_size = (size_t)(min_size * 1.5);
    if (new_size - min_size > 
                             8192
                                                   ) {
      new_size = min_size + 
                           8192
                                                 ;
    }
    p = (char *) realloc(a->buf, new_size);
    if (p == 
            __null 
                 && new_size != min_size) {
      new_size = min_size;
      p = (char *) realloc(a->buf, new_size);
    }
    if (p != 
            __null
                ) {
      a->buf = p;
      if (off != a->len) {
        memmove(a->buf + off + len, a->buf + off, a->len - off);
      }
      if (buf != 
                __null
                    ) memcpy(a->buf + off, buf, len);
      a->len += len;
      a->size = new_size;
    } else {
      len = 0;
    }
  }

  return len;
}

size_t mbuf_append(struct mbuf *a, const void *buf, size_t len) __attribute__((weak));
size_t mbuf_append(struct mbuf *a, const void *buf, size_t len) {
  return mbuf_insert(a, a->len, buf, len);
}

size_t mbuf_append_and_free(struct mbuf *a, void *buf, size_t len) __attribute__((weak));
size_t mbuf_append_and_free(struct mbuf *a, void *data, size_t len) {
  size_t ret;


  if (a->len == 0) {
    if (a->buf != 
                 __null
                     ) free(a->buf);
    a->buf = (char *) data;
    a->len = a->size = len;
    return len;
  }
  ret = mbuf_insert(a, a->len, data, len);
  free(data);
  return ret;
}

void mbuf_remove(struct mbuf *mb, size_t n) __attribute__((weak));
void mbuf_remove(struct mbuf *mb, size_t n) {
  if (n > 0 && n <= mb->len) {
    memmove(mb->buf, mb->buf + n, mb->len - n);
    mb->len -= n;
  }
}

void mbuf_clear(struct mbuf *mb) __attribute__((weak));
void mbuf_clear(struct mbuf *mb) {
  mb->len = 0;
}

void mbuf_move(struct mbuf *from, struct mbuf *to) __attribute__((weak));
void mbuf_move(struct mbuf *from, struct mbuf *to) {
  memcpy(to, from, sizeof(*to));
  memset(from, 0, sizeof(*from));
}


int mg_ncasecmp(const char *s1, const char *s2, size_t len) __attribute__((weak));

struct mg_str mg_mk_str(const char *s) __attribute__((weak));
struct mg_str mg_mk_str(const char *s) {
  struct mg_str ret = {s, 0};
  if (s != 
          __null
              ) ret.len = strlen(s);
  return ret;
}

struct mg_str mg_mk_str_n(const char *s, size_t len) __attribute__((weak));
struct mg_str mg_mk_str_n(const char *s, size_t len) {
  struct mg_str ret = {s, len};
  return ret;
}

int mg_vcmp(const struct mg_str *str1, const char *str2) __attribute__((weak));
int mg_vcmp(const struct mg_str *str1, const char *str2) {
  size_t n2 = strlen(str2), n1 = str1->len;
  int r = strncmp(str1->p, str2, (n1 < n2) ? n1 : n2);
  if (r == 0) {
    return n1 - n2;
  }
  return r;
}

int mg_vcasecmp(const struct mg_str *str1, const char *str2) __attribute__((weak));
int mg_vcasecmp(const struct mg_str *str1, const char *str2) {
  size_t n2 = strlen(str2), n1 = str1->len;
  int r = mg_ncasecmp(str1->p, str2, (n1 < n2) ? n1 : n2);
  if (r == 0) {
    return n1 - n2;
  }
  return r;
}

static struct mg_str mg_strdup_common(const struct mg_str s,
                                      int nul_terminate) {
  struct mg_str r = {
                    __null
                        , 0};
  if (s.len > 0 && s.p != 
                         __null
                             ) {
    char *sc = (char *) malloc(s.len + (nul_terminate ? 1 : 0));
    if (sc != 
             __null
                 ) {
      memcpy(sc, s.p, s.len);
      if (nul_terminate) sc[s.len] = '\0';
      r.p = sc;
      r.len = s.len;
    }
  }
  return r;
}

struct mg_str mg_strdup(const struct mg_str s) __attribute__((weak));
struct mg_str mg_strdup(const struct mg_str s) {
  return mg_strdup_common(s, 0 );
}

struct mg_str mg_strdup_nul(const struct mg_str s) __attribute__((weak));
struct mg_str mg_strdup_nul(const struct mg_str s) {
  return mg_strdup_common(s, 1 );
}

const char *mg_strchr(const struct mg_str s, int c) __attribute__((weak));
const char *mg_strchr(const struct mg_str s, int c) {
  size_t i;
  for (i = 0; i < s.len; i++) {
    if (s.p[i] == c) return &s.p[i];
  }
  return 
        __null
            ;
}

int mg_strcmp(const struct mg_str str1, const struct mg_str str2) __attribute__((weak));
int mg_strcmp(const struct mg_str str1, const struct mg_str str2) {
  size_t i = 0;
  while (i < str1.len && i < str2.len) {
    if (str1.p[i] < str2.p[i]) return -1;
    if (str1.p[i] > str2.p[i]) return 1;
    i++;
  }
  if (i < str1.len) return 1;
  if (i < str2.len) return -1;
  return 0;
}

int mg_strncmp(const struct mg_str, const struct mg_str, size_t n) __attribute__((weak));
int mg_strncmp(const struct mg_str str1, const struct mg_str str2, size_t n) {
  struct mg_str s1 = str1;
  struct mg_str s2 = str2;

  if (s1.len > n) {
    s1.len = n;
  }
  if (s2.len > n) {
    s2.len = n;
  }
  return mg_strcmp(s1, s2);
}

void mg_strfree(struct mg_str *s) __attribute__((weak));
void mg_strfree(struct mg_str *s) {
  char *sp = (char *) s->p;
  s->p = 
        __null
            ;
  s->len = 0;
  if (sp != 
           __null
               ) free(sp);
}

const char *mg_strstr(const struct mg_str haystack,
                      const struct mg_str needle) __attribute__((weak));
const char *mg_strstr(const struct mg_str haystack,
                      const struct mg_str needle) {
  size_t i;
  if (needle.len > haystack.len) return 
                                       __null
                                           ;
  for (i = 0; i <= haystack.len - needle.len; i++) {
    if (memcmp(haystack.p + i, needle.p, needle.len) == 0) {
      return haystack.p + i;
    }
  }
  return 
        __null
            ;
}

struct mg_str mg_strstrip(struct mg_str s) __attribute__((weak));
struct mg_str mg_strstrip(struct mg_str s) {
  while (s.len > 0 && isspace((int) *s.p)) {
    s.p++;
    s.len--;
  }
  while (s.len > 0 && isspace((int) *(s.p + s.len - 1))) {
    s.len--;
  }
  return s;
}

int mg_str_starts_with(struct mg_str s, struct mg_str prefix) __attribute__((weak));
int mg_str_starts_with(struct mg_str s, struct mg_str prefix) {
  const struct mg_str sp = { s.p, prefix.len };
  if (s.len < prefix.len) return 0;
  return (mg_strcmp(sp, prefix) == 0);
}
size_t c_strnlen(const char *s, size_t maxlen) __attribute__((weak));
size_t c_strnlen(const char *s, size_t maxlen) {
  size_t l = 0;
  for (; l < maxlen && s[l] != '\0'; l++) {
  }
  return l;
}
static int c_itoa(char *buf, size_t buf_size, int64_t num, int base, int flags,
                  int field_width) {
  char tmp[40];
  int i = 0, k = 0, neg = 0;

  if (num < 0) {
    neg++;
    num = -num;
  }


  do {
    int rem = num % base;
    if (rem < 10) {
      tmp[k++] = '0' + rem;
    } else {
      tmp[k++] = 'a' + (rem - 10);
    }
    num /= base;
  } while (num > 0);


  if (flags && 1) {
    while (k < field_width && k < (int) sizeof(tmp) - 1) {
      tmp[k++] = '0';
    }
  }


  if (neg) {
    tmp[k++] = '-';
  }


  while (--k >= 0) {
    do { if (i < (int) buf_size) buf[i] = tmp[k]; i++; } while (0);
  }

  return i;
}

int c_vsnprintf(char *buf, size_t buf_size, const char *fmt, va_list ap) __attribute__((weak));
int c_vsnprintf(char *buf, size_t buf_size, const char *fmt, va_list ap) {
  int ch, i = 0, len_mod, flags, precision, field_width;

  while ((ch = *fmt++) != '\0') {
    if (ch != '%') {
      do { if (i < (int) buf_size) buf[i] = ch; i++; } while (0);
    } else {
      flags = field_width = precision = len_mod = 0;


      if (*fmt == '0') {
        flags |= 1;
      }


      while (*fmt >= '0' && *fmt <= '9') {
        field_width *= 10;
        field_width += *fmt++ - '0';
      }

      if (*fmt == '*') {
        field_width = 
                     __builtin_va_arg(
                     ap
                     ,
                     int
                     )
                                    ;
        fmt++;
      }


      if (*fmt == '.') {
        fmt++;
        if (*fmt == '*') {
          precision = 
                     __builtin_va_arg(
                     ap
                     ,
                     int
                     )
                                    ;
          fmt++;
        } else {
          while (*fmt >= '0' && *fmt <= '9') {
            precision *= 10;
            precision += *fmt++ - '0';
          }
        }
      }


      switch (*fmt) {
        case 'h':
        case 'l':
        case 'L':
        case 'I':
        case 'q':
        case 'j':
        case 'z':
        case 't':
          len_mod = *fmt++;
          if (*fmt == 'h') {
            len_mod = 'H';
            fmt++;
          }
          if (*fmt == 'l') {
            len_mod = 'q';
            fmt++;
          }
          break;
      }

      ch = *fmt++;
      if (ch == 's') {
        const char *s = 
                       __builtin_va_arg(
                       ap
                       ,
                       const char *
                       )
                                               ;
        int j;
        int pad = field_width - (precision >= 0 ? c_strnlen(s, precision) : 0);
        for (j = 0; j < pad; j++) {
          do { if (i < (int) buf_size) buf[i] = ' '; i++; } while (0);
        }


        if (s != 
                __null
                    ) {

          for (j = 0; (precision <= 0 || j < precision) && s[j] != '\0'; j++) {
            do { if (i < (int) buf_size) buf[i] = s[j]; i++; } while (0);
          }
        }
      } else if (ch == 'c') {
        ch = 
            __builtin_va_arg(
            ap
            ,
            int
            )
                           ;
        do { if (i < (int) buf_size) buf[i] = ch; i++; } while (0);
      } else if (ch == 'd' && len_mod == 0) {
        i += c_itoa(buf + i, buf_size - i, 
                                          __builtin_va_arg(
                                          ap
                                          ,
                                          int
                                          )
                                                         , 10, flags,
                    field_width);
      } else if (ch == 'd' && len_mod == 'l') {
        i += c_itoa(buf + i, buf_size - i, 
                                          __builtin_va_arg(
                                          ap
                                          ,
                                          long
                                          )
                                                          , 10, flags,
                    field_width);

      } else if (ch == 'd' && len_mod == 'z') {
        i += c_itoa(buf + i, buf_size - i, 
                                          __builtin_va_arg(
                                          ap
                                          ,
                                          ssize_t
                                          )
                                                             , 10, flags,
                    field_width);

      } else if (ch == 'd' && len_mod == 'q') {
        i += c_itoa(buf + i, buf_size - i, 
                                          __builtin_va_arg(
                                          ap
                                          ,
                                          int64_t
                                          )
                                                             , 10, flags,
                    field_width);
      } else if ((ch == 'x' || ch == 'u') && len_mod == 0) {
        i += c_itoa(buf + i, buf_size - i, 
                                          __builtin_va_arg(
                                          ap
                                          ,
                                          unsigned
                                          )
                                                              ,
                    ch == 'x' ? 16 : 10, flags, field_width);
      } else if ((ch == 'x' || ch == 'u') && len_mod == 'l') {
        i += c_itoa(buf + i, buf_size - i, 
                                          __builtin_va_arg(
                                          ap
                                          ,
                                          unsigned long
                                          )
                                                                   ,
                    ch == 'x' ? 16 : 10, flags, field_width);
      } else if ((ch == 'x' || ch == 'u') && len_mod == 'z') {
        i += c_itoa(buf + i, buf_size - i, 
                                          __builtin_va_arg(
                                          ap
                                          ,
                                          size_t
                                          )
                                                            ,
                    ch == 'x' ? 16 : 10, flags, field_width);
      } else if (ch == 'p') {
        unsigned long num = (unsigned long) (uintptr_t) 
                                                       __builtin_va_arg(
                                                       ap
                                                       ,
                                                       void *
                                                       )
                                                                         ;
        do { if (i < (int) buf_size) buf[i] = '0'; i++; } while (0);
        do { if (i < (int) buf_size) buf[i] = 'x'; i++; } while (0);
        i += c_itoa(buf + i, buf_size - i, num, 16, flags, 0);
      } else {





        abort();

      }
    }
  }


  if (buf_size > 0) {
    buf[i < (int) buf_size ? i : (int) buf_size - 1] = '\0';
  }

  return i;
}


int c_snprintf(char *buf, size_t buf_size, const char *fmt, ...) __attribute__((weak));
int c_snprintf(char *buf, size_t buf_size, const char *fmt, ...) {
  int result;
  va_list ap;
  
 __builtin_va_start(
 ap
 ,
 fmt
 )
                  ;
  result = c_vsnprintf(buf, buf_size, fmt, ap);
  
 __builtin_va_end(
 ap
 )
           ;
  return result;
}
const char *c_strnstr(const char *s, const char *find, size_t slen) __attribute__((weak));
const char *c_strnstr(const char *s, const char *find, size_t slen) {
  size_t find_length = strlen(find);
  size_t i;

  for (i = 0; i < slen; i++) {
    if (i + find_length > slen) {
      return 
            __null
                ;
    }

    if (strncmp(&s[i], find, find_length) == 0) {
      return &s[i];
    }
  }

  return 
        __null
            ;
}
void cs_to_hex(char *to, const unsigned char *p, size_t len) __attribute__((weak));
void cs_to_hex(char *to, const unsigned char *p, size_t len) {
  static const char *hex = "0123456789abcdef";

  for (; len--; p++) {
    *to++ = hex[p[0] >> 4];
    *to++ = hex[p[0] & 0x0f];
  }
  *to = '\0';
}

static int fourbit(int ch) {
  if (ch >= '0' && ch <= '9') {
    return ch - '0';
  } else if (ch >= 'a' && ch <= 'f') {
    return ch - 'a' + 10;
  } else if (ch >= 'A' && ch <= 'F') {
    return ch - 'A' + 10;
  }
  return 0;
}

void cs_from_hex(char *to, const char *p, size_t len) __attribute__((weak));
void cs_from_hex(char *to, const char *p, size_t len) {
  size_t i;

  for (i = 0; i < len; i += 2) {
    *to++ = (fourbit(p[i]) << 4) + fourbit(p[i + 1]);
  }
  *to = '\0';
}
static int str_util_lowercase(const char *s) {
  return tolower(*(const unsigned char *) s);
}

int mg_ncasecmp(const char *s1, const char *s2, size_t len) __attribute__((weak));
int mg_ncasecmp(const char *s1, const char *s2, size_t len) {
  int diff = 0;

  if (len > 0) do {
      diff = str_util_lowercase(s1++) - str_util_lowercase(s2++);
    } while (diff == 0 && s1[-1] != '\0' && --len > 0);

  return diff;
}

int mg_casecmp(const char *s1, const char *s2) __attribute__((weak));
int mg_casecmp(const char *s1, const char *s2) {
  return mg_ncasecmp(s1, s2, (size_t) ~0);
}

int mg_asprintf(char **buf, size_t size, const char *fmt, ...) __attribute__((weak));
int mg_asprintf(char **buf, size_t size, const char *fmt, ...) {
  int ret;
  va_list ap;
  
 __builtin_va_start(
 ap
 ,
 fmt
 )
                  ;
  ret = mg_avprintf(buf, size, fmt, ap);
  
 __builtin_va_end(
 ap
 )
           ;
  return ret;
}

int mg_avprintf(char **buf, size_t size, const char *fmt, va_list ap) __attribute__((weak));
int mg_avprintf(char **buf, size_t size, const char *fmt, va_list ap) {
  va_list ap_copy;
  int len;

  
 __builtin_va_copy(
 ap_copy
 ,
 ap
 )
                     ;
  len = vsnprintf(*buf, size, fmt, ap_copy);
  
 __builtin_va_end(
 ap_copy
 )
                ;

  if (len < 0) {



    *buf = 
          __null
              ;
    while (len < 0) {
      free(*buf);
      if (size == 0) {
        size = 5;
      }
      size *= 2;
      if ((*buf = (char *) malloc(size)) == 
                                              __null
                                                  ) {
        len = -1;
        break;
      }
      
     __builtin_va_copy(
     ap_copy
     ,
     ap
     )
                         ;
      len = vsnprintf(*buf, size - 1, fmt, ap_copy);
      
     __builtin_va_end(
     ap_copy
     )
                    ;
    }





    (*buf)[len] = 0;

  } else if (len >= (int) size) {

    if ((*buf = (char *) malloc(len + 1)) == 
                                               __null
                                                   ) {
      len = -1;
    } else {
      
     __builtin_va_copy(
     ap_copy
     ,
     ap
     )
                         ;
      len = vsnprintf(*buf, len + 1, fmt, ap_copy);
      
     __builtin_va_end(
     ap_copy
     )
                    ;
    }
  }

  return len;
}

const char *mg_next_comma_list_entry(const char *, struct mg_str *,
                                     struct mg_str *) __attribute__((weak));
const char *mg_next_comma_list_entry(const char *list, struct mg_str *val,
                                     struct mg_str *eq_val) {
  struct mg_str ret = mg_next_comma_list_entry_n(mg_mk_str(list), val, eq_val);
  return ret.p;
}

struct mg_str mg_next_comma_list_entry_n(struct mg_str list, struct mg_str *val,
                                         struct mg_str *eq_val) __attribute__((weak));
struct mg_str mg_next_comma_list_entry_n(struct mg_str list, struct mg_str *val,
                                         struct mg_str *eq_val) {
  if (list.len == 0) {

    list = mg_mk_str(
                    __null
                        );
  } else {
    const char *chr = 
                     __null
                         ;
    *val = list;

    if ((chr = mg_strchr(*val, ',')) != 
                                       __null
                                           ) {

      val->len = chr - val->p;
      chr++;
      list.len -= (chr - list.p);
      list.p = chr;
    } else {

      list = mg_mk_str_n(list.p + list.len, 0);
    }

    if (eq_val != 
                 __null
                     ) {


      eq_val->len = 0;
      eq_val->p = (const char *) memchr(val->p, '=', val->len);
      if (eq_val->p != 
                      __null
                          ) {
        eq_val->p++;
        eq_val->len = val->p + val->len - eq_val->p;
        val->len = (eq_val->p - val->p) - 1;
      }
    }
  }

  return list;
}

size_t mg_match_prefix_n(const struct mg_str, const struct mg_str) __attribute__((weak));
size_t mg_match_prefix_n(const struct mg_str pattern, const struct mg_str str) {
  const char *or_str;
  size_t res = 0, len = 0, i = 0, j = 0;

  if ((or_str = (const char *) memchr(pattern.p, '|', pattern.len)) != 
                                                                      __null 
                                                                           ||
      (or_str = (const char *) memchr(pattern.p, ',', pattern.len)) != 
                                                                      __null
                                                                          ) {
    struct mg_str pstr = {pattern.p, (size_t)(or_str - pattern.p)};
    res = mg_match_prefix_n(pstr, str);
    if (res > 0) return res;
    pstr.p = or_str + 1;
    pstr.len = (pattern.p + pattern.len) - (or_str + 1);
    return mg_match_prefix_n(pstr, str);
  }

  for (; i < pattern.len && j < str.len; i++, j++) {
    if (pattern.p[i] == '?') {
      continue;
    } else if (pattern.p[i] == '*') {
      i++;
      if (i < pattern.len && pattern.p[i] == '*') {
        i++;
        len = str.len - j;
      } else {
        len = 0;
        while (j + len < str.len && str.p[j + len] != '/') len++;
      }
      if (i == pattern.len || (pattern.p[i] == '$' && i == pattern.len - 1))
        return j + len;
      do {
        const struct mg_str pstr = {pattern.p + i, pattern.len - i};
        const struct mg_str sstr = {str.p + j + len, str.len - j - len};
        res = mg_match_prefix_n(pstr, sstr);
      } while (res == 0 && len != 0 && len-- > 0);
      return res == 0 ? 0 : j + res + len;
    } else if (str_util_lowercase(&pattern.p[i]) !=
               str_util_lowercase(&str.p[j])) {
      break;
    }
  }
  if (i < pattern.len && pattern.p[i] == '$') {
    return j == str.len ? str.len : 0;
  }
  return i == pattern.len ? j : 0;
}

size_t mg_match_prefix(const char *, int, const char *) __attribute__((weak));
size_t mg_match_prefix(const char *pattern, int pattern_len, const char *str) {
  const struct mg_str pstr = {pattern, (size_t) pattern_len};
  struct mg_str s = {str, 0};
  if (str != 
            __null
                ) s.len = strlen(str);
  return mg_match_prefix_n(pstr, s);
}
struct frozen {
  const char *end;
  const char *cur;

  const char *cur_name;
  size_t cur_name_len;


  char path[256];
  size_t path_len;
  void *callback_data;
  json_walk_callback_t callback;
};

struct fstate {
  const char *ptr;
  size_t path_len;
};
static int json_append_to_path(struct frozen *f, const char *str, int size) {
  int n = f->path_len;
  int left = sizeof(f->path) - n - 1;
  if (size > left) size = left;
  memcpy(f->path + n, str, size);
  f->path[n + size] = '\0';
  f->path_len += size;
  return n;
}

static void json_truncate_path(struct frozen *f, size_t len) {
  f->path_len = len;
  f->path[len] = '\0';
}

static int json_parse_object(struct frozen *f);
static int json_parse_value(struct frozen *f);
static int json_left(const struct frozen *f) {
  return f->end - f->cur;
}

static int json_isspace(int ch) {
  return ch == ' ' || ch == '\t' || ch == '\r' || ch == '\n';
}

static void json_skip_whitespaces(struct frozen *f) {
  while (f->cur < f->end && json_isspace(*f->cur)) f->cur++;
}

static int json_cur(struct frozen *f) {
  json_skip_whitespaces(f);
  return f->cur >= f->end ? (-1) : *(unsigned char *) f->cur;
}

static int json_test_and_skip(struct frozen *f, int expected) {
  int ch = json_cur(f);
  if (ch == expected) {
    f->cur++;
    return 0;
  }
  return ch == (-1) ? -2 : -1;
}

static int json_isalpha(int ch) {
  return (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z');
}

static int json_isdigit(int ch) {
  return ch >= '0' && ch <= '9';
}

static int json_isxdigit(int ch) {
  return json_isdigit(ch) || (ch >= 'a' && ch <= 'f') ||
         (ch >= 'A' && ch <= 'F');
}

static int json_get_escape_len(const char *s, int len) {
  switch (*s) {
    case 'u':
      return len < 6 ? -2
                     : json_isxdigit(s[1]) && json_isxdigit(s[2]) &&
                               json_isxdigit(s[3]) && json_isxdigit(s[4])
                           ? 5
                           : -1;
    case '"':
    case '\\':
    case '/':
    case 'b':
    case 'f':
    case 'n':
    case 'r':
    case 't':
      return len < 2 ? -2 : 1;
    default:
      return -1;
  }
}


static int json_parse_identifier(struct frozen *f) {
  do { if (!(json_isalpha(json_cur(f)))) return (-1); } while (0);
  {
    struct fstate fstate = {(f->cur), (f)->path_len}; json_append_to_path((f), (""), (0));;
    while (f->cur < f->end &&
           (*f->cur == '_' || json_isalpha(*f->cur) || json_isdigit(*f->cur))) {
      f->cur++;
    }
    json_truncate_path(f, fstate.path_len);
    do { if ((f)->callback && ((f)->path_len == 0 || (f)->path[(f)->path_len - 1] != '.')) { struct json_token t = {(fstate.ptr), (int) (f->cur - fstate.ptr), (JSON_TYPE_STRING)}; (f)->callback((f)->callback_data, (f)->cur_name, (f)->cur_name_len, (f)->path, &t); (f)->cur_name = 
   __null
   ; (f)->cur_name_len = 0; } } while (0);
  }
  return 0;
}

static int json_get_utf8_char_len(unsigned char ch) {
  if ((ch & 0x80) == 0) return 1;
  switch (ch & 0xf0) {
    case 0xf0:
      return 4;
    case 0xe0:
      return 3;
    default:
      return 2;
  }
}


static int json_parse_string(struct frozen *f) {
  int n, ch = 0, len = 0;
  do { int _n = json_test_and_skip(f, '"'); if (_n < 0) return _n; } while (0);
  {
    struct fstate fstate = {(f->cur), (f)->path_len}; json_append_to_path((f), (""), (0));;
    for (; f->cur < f->end; f->cur += len) {
      ch = *(unsigned char *) f->cur;
      len = json_get_utf8_char_len((unsigned char) ch);
      do { if (!(ch >= 32 && len > 0)) return (-1); } while (0);
      do { if (!(len <= json_left(f))) return (-2); } while (0);
      if (ch == '\\') {
        do { if (!((n = json_get_escape_len(f->cur + 1, json_left(f))) > 0)) return (n); } while (0);
        len += n;
      } else if (ch == '"') {
        json_truncate_path(f, fstate.path_len);
        do { if ((f)->callback && ((f)->path_len == 0 || (f)->path[(f)->path_len - 1] != '.')) { struct json_token t = {(fstate.ptr), (int) (f->cur - fstate.ptr), (JSON_TYPE_STRING)}; (f)->callback((f)->callback_data, (f)->cur_name, (f)->cur_name_len, (f)->path, &t); (f)->cur_name = 
       __null
       ; (f)->cur_name_len = 0; } } while (0);
        f->cur++;
        break;
      };
    }
  }
  return ch == '"' ? 0 : -2;
}


static int json_parse_number(struct frozen *f) {
  int ch = json_cur(f);
  struct fstate fstate = {(f->cur), (f)->path_len}; json_append_to_path((f), (""), (0));;
  if (ch == '-') f->cur++;
  do { if (!(f->cur < f->end)) return (-2); } while (0);
  if (f->cur + 1 < f->end && f->cur[0] == '0' && f->cur[1] == 'x') {
    f->cur += 2;
    do { if (!(f->cur < f->end)) return (-2); } while (0);
    do { if (!(json_isxdigit(f->cur[0]))) return (-1); } while (0);
    while (f->cur < f->end && json_isxdigit(f->cur[0])) f->cur++;
  } else {
    do { if (!(json_isdigit(f->cur[0]))) return (-1); } while (0);
    while (f->cur < f->end && json_isdigit(f->cur[0])) f->cur++;
    if (f->cur < f->end && f->cur[0] == '.') {
      f->cur++;
      do { if (!(f->cur < f->end)) return (-2); } while (0);
      do { if (!(json_isdigit(f->cur[0]))) return (-1); } while (0);
      while (f->cur < f->end && json_isdigit(f->cur[0])) f->cur++;
    }
    if (f->cur < f->end && (f->cur[0] == 'e' || f->cur[0] == 'E')) {
      f->cur++;
      do { if (!(f->cur < f->end)) return (-2); } while (0);
      if ((f->cur[0] == '+' || f->cur[0] == '-')) f->cur++;
      do { if (!(f->cur < f->end)) return (-2); } while (0);
      do { if (!(json_isdigit(f->cur[0]))) return (-1); } while (0);
      while (f->cur < f->end && json_isdigit(f->cur[0])) f->cur++;
    }
  }
  json_truncate_path(f, fstate.path_len);
  do { if ((f)->callback && ((f)->path_len == 0 || (f)->path[(f)->path_len - 1] != '.')) { struct json_token t = {(fstate.ptr), (int) (f->cur - fstate.ptr), (JSON_TYPE_NUMBER)}; (f)->callback((f)->callback_data, (f)->cur_name, (f)->cur_name_len, (f)->path, &t); (f)->cur_name = 
 __null
 ; (f)->cur_name_len = 0; } } while (0);
  return 0;
}



static int json_parse_array(struct frozen *f) {
  int i = 0, current_path_len;
  char buf[20];
  do { if ((f)->callback && ((f)->path_len == 0 || (f)->path[(f)->path_len - 1] != '.')) { struct json_token t = {(
 __null
 ), (int) (0), (JSON_TYPE_ARRAY_START)}; (f)->callback((f)->callback_data, (f)->cur_name, (f)->cur_name_len, (f)->path, &t); (f)->cur_name = 
 __null
 ; (f)->cur_name_len = 0; } } while (0);
  do { int _n = json_test_and_skip(f, '['); if (_n < 0) return _n; } while (0);
  {
    {
      struct fstate fstate = {(f->cur - 1), (f)->path_len}; json_append_to_path((f), (""), (0));;
      while (json_cur(f) != ']') {
        snprintf(buf, sizeof(buf), "[%d]", i);
        i++;
        current_path_len = json_append_to_path(f, buf, strlen(buf));
        f->cur_name =
            f->path + strlen(f->path) - strlen(buf) + 1 ;
        f->cur_name_len = strlen(buf) - 2 ;
        do { int _n = json_parse_value(f); if (_n < 0) return _n; } while (0);
        json_truncate_path(f, current_path_len);
        if (json_cur(f) == ',') f->cur++;
      }
      do { int _n = json_test_and_skip(f, ']'); if (_n < 0) return _n; } while (0);
      json_truncate_path(f, fstate.path_len);
      do { if ((f)->callback && ((f)->path_len == 0 || (f)->path[(f)->path_len - 1] != '.')) { struct json_token t = {(fstate.ptr), (int) (f->cur - fstate.ptr), (JSON_TYPE_ARRAY_END)}; (f)->callback((f)->callback_data, (f)->cur_name, (f)->cur_name_len, (f)->path, &t); (f)->cur_name = 
     __null
     ; (f)->cur_name_len = 0; } } while (0);
    }
  }
  return 0;
}


static int json_expect(struct frozen *f, const char *s, int len,
                       enum json_token_type tok_type) {
  int i, n = json_left(f);
  struct fstate fstate = {(f->cur), (f)->path_len}; json_append_to_path((f), (""), (0));;
  for (i = 0; i < len; i++) {
    if (i >= n) return -2;
    if (f->cur[i] != s[i]) return -1;
  }
  f->cur += len;
  json_truncate_path(f, fstate.path_len);

  do { if ((f)->callback && ((f)->path_len == 0 || (f)->path[(f)->path_len - 1] != '.')) { struct json_token t = {(fstate.ptr), (int) (f->cur - fstate.ptr), (tok_type)}; (f)->callback((f)->callback_data, (f)->cur_name, (f)->cur_name_len, (f)->path, &t); (f)->cur_name = 
 __null
 ; (f)->cur_name_len = 0; } } while (0);

  return 0;
}


static int json_parse_value(struct frozen *f) {
  int ch = json_cur(f);

  switch (ch) {
    case '"':
      do { int _n = json_parse_string(f); if (_n < 0) return _n; } while (0);
      break;
    case '{':
      do { int _n = json_parse_object(f); if (_n < 0) return _n; } while (0);
      break;

    case '[':
      do { int _n = json_parse_array(f); if (_n < 0) return _n; } while (0);
      break;

    case 'n':
      do { int _n = json_expect(f, "null", 4, JSON_TYPE_NULL); if (_n < 0) return _n; } while (0);
      break;
    case 't':
      do { int _n = json_expect(f, "true", 4, JSON_TYPE_TRUE); if (_n < 0) return _n; } while (0);
      break;
    case 'f':
      do { int _n = json_expect(f, "false", 5, JSON_TYPE_FALSE); if (_n < 0) return _n; } while (0);
      break;
    case '-':
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
      do { int _n = json_parse_number(f); if (_n < 0) return _n; } while (0);
      break;
    default:
      return ch == (-1) ? -2 : -1;
  }

  return 0;
}


static int json_parse_key(struct frozen *f) {
  int ch = json_cur(f);
  if (json_isalpha(ch)) {
    do { int _n = json_parse_identifier(f); if (_n < 0) return _n; } while (0);
  } else if (ch == '"') {
    do { int _n = json_parse_string(f); if (_n < 0) return _n; } while (0);
  } else {
    return ch == (-1) ? -2 : -1;
  }
  return 0;
}


static int json_parse_pair(struct frozen *f) {
  int current_path_len;
  const char *tok;
  json_skip_whitespaces(f);
  tok = f->cur;
  do { int _n = json_parse_key(f); if (_n < 0) return _n; } while (0);
  {
    f->cur_name = *tok == '"' ? tok + 1 : tok;
    f->cur_name_len = *tok == '"' ? f->cur - tok - 2 : f->cur - tok;
    current_path_len = json_append_to_path(f, f->cur_name, f->cur_name_len);
  }
  do { int _n = json_test_and_skip(f, ':'); if (_n < 0) return _n; } while (0);
  do { int _n = json_parse_value(f); if (_n < 0) return _n; } while (0);
  json_truncate_path(f, current_path_len);
  return 0;
}


static int json_parse_object(struct frozen *f) {
  do { if ((f)->callback && ((f)->path_len == 0 || (f)->path[(f)->path_len - 1] != '.')) { struct json_token t = {(
 __null
 ), (int) (0), (JSON_TYPE_OBJECT_START)}; (f)->callback((f)->callback_data, (f)->cur_name, (f)->cur_name_len, (f)->path, &t); (f)->cur_name = 
 __null
 ; (f)->cur_name_len = 0; } } while (0);
  do { int _n = json_test_and_skip(f, '{'); if (_n < 0) return _n; } while (0);
  {
    struct fstate fstate = {(f->cur - 1), (f)->path_len}; json_append_to_path((f), ("."), (1));;
    while (json_cur(f) != '}') {
      do { int _n = json_parse_pair(f); if (_n < 0) return _n; } while (0);
      if (json_cur(f) == ',') f->cur++;
    }
    do { int _n = json_test_and_skip(f, '}'); if (_n < 0) return _n; } while (0);
    json_truncate_path(f, fstate.path_len);
    do { if ((f)->callback && ((f)->path_len == 0 || (f)->path[(f)->path_len - 1] != '.')) { struct json_token t = {(fstate.ptr), (int) (f->cur - fstate.ptr), (JSON_TYPE_OBJECT_END)}; (f)->callback((f)->callback_data, (f)->cur_name, (f)->cur_name_len, (f)->path, &t); (f)->cur_name = 
   __null
   ; (f)->cur_name_len = 0; } } while (0);
  }
  return 0;
}

static int json_doit(struct frozen *f) {
  if (f->cur == 0 || f->end < f->cur) return -1;
  if (f->end == f->cur) return -2;
  return json_parse_value(f);
}

int json_escape(struct json_out *out, const char *p, size_t len) __attribute__((weak));
int json_escape(struct json_out *out, const char *p, size_t len) {
  size_t i, cl, n = 0;
  const char *hex_digits = "0123456789abcdef";
  const char *specials = "btnvfr";

  for (i = 0; i < len; i++) {
    unsigned char ch = ((unsigned char *) p)[i];
    if (ch == '"' || ch == '\\') {
      n += out->printer(out, "\\", 1);
      n += out->printer(out, p + i, 1);
    } else if (ch >= '\b' && ch <= '\r') {
      n += out->printer(out, "\\", 1);
      n += out->printer(out, &specials[ch - '\b'], 1);
    } else if (isprint(ch)) {
      n += out->printer(out, p + i, 1);
    } else if ((cl = json_get_utf8_char_len(ch)) == 1) {
      n += out->printer(out, "\\u00", 4);
      n += out->printer(out, &hex_digits[(ch >> 4) % 0xf], 1);
      n += out->printer(out, &hex_digits[ch % 0xf], 1);
    } else {
      n += out->printer(out, p + i, cl);
      i += cl - 1;
    }
  }

  return n;
}

int json_printer_buf(struct json_out *out, const char *buf, size_t len) __attribute__((weak));
int json_printer_buf(struct json_out *out, const char *buf, size_t len) {
  size_t avail = out->u.buf.size - out->u.buf.len;
  size_t n = len < avail ? len : avail;
  memcpy(out->u.buf.buf + out->u.buf.len, buf, n);
  out->u.buf.len += n;
  if (out->u.buf.size > 0) {
    size_t idx = out->u.buf.len;
    if (idx >= out->u.buf.size) idx = out->u.buf.size - 1;
    out->u.buf.buf[idx] = '\0';
  }
  return len;
}

int json_printer_file(struct json_out *out, const char *buf, size_t len) __attribute__((weak));
int json_printer_file(struct json_out *out, const char *buf, size_t len) {
  return fwrite(buf, 1, len, out->u.fp);
}


static int b64idx(int c) {
  if (c < 26) {
    return c + 'A';
  } else if (c < 52) {
    return c - 26 + 'a';
  } else if (c < 62) {
    return c - 52 + '0';
  } else {
    return c == 62 ? '+' : '/';
  }
}

static int b64rev(int c) {
  if (c >= 'A' && c <= 'Z') {
    return c - 'A';
  } else if (c >= 'a' && c <= 'z') {
    return c + 26 - 'a';
  } else if (c >= '0' && c <= '9') {
    return c + 52 - '0';
  } else if (c == '+') {
    return 62;
  } else if (c == '/') {
    return 63;
  } else {
    return 64;
  }
}

static int b64enc(struct json_out *out, const unsigned char *p, int n) {
  char buf[4];
  int i, len = 0;
  for (i = 0; i < n; i += 3) {
    int a = p[i], b = i + 1 < n ? p[i + 1] : 0, c = i + 2 < n ? p[i + 2] : 0;
    buf[0] = b64idx(a >> 2);
    buf[1] = b64idx((a & 3) << 4 | (b >> 4));
    buf[2] = b64idx((b & 15) << 2 | (c >> 6));
    buf[3] = b64idx(c & 63);
    if (i + 1 >= n) buf[2] = '=';
    if (i + 2 >= n) buf[3] = '=';
    len += out->printer(out, buf, sizeof(buf));
  }
  return len;
}

static int b64dec(const char *src, int n, char *dst) {
  const char *end = src + n;
  int len = 0;
  while (src + 3 < end) {
    int a = b64rev(src[0]), b = b64rev(src[1]), c = b64rev(src[2]),
        d = b64rev(src[3]);
    dst[len++] = (a << 2) | (b >> 4);
    if (src[2] != '=') {
      dst[len++] = (b << 4) | (c >> 2);
      if (src[3] != '=') {
        dst[len++] = (c << 6) | d;
      }
    }
    src += 4;
  }
  return len;
}


static unsigned char hexdec(const char *s) {

  int a = tolower(*(const unsigned char *) s);
  int b = tolower(*(const unsigned char *) (s + 1));
  return ((a >= '0' && a <= '9' ? a - '0' : a - 'W') << 4) | (b >= '0' && b <= '9' ? b - '0' : b - 'W');
}

int json_vprintf(struct json_out *out, const char *fmt, va_list xap) __attribute__((weak));
int json_vprintf(struct json_out *out, const char *fmt, va_list xap) {
  int len = 0;
  const char *quote = "\"", *null = "null";
  va_list ap;
  
 __builtin_va_copy(
 ap
 ,
 xap
 )
                 ;

  while (*fmt != '\0') {
    if (strchr(":, \r\n\t[]{}\"", *fmt) != 
                                          __null
                                              ) {
      len += out->printer(out, fmt, 1);
      fmt++;
    } else if (fmt[0] == '%') {
      char buf[21];
      size_t skip = 2;

      if (fmt[1] == 'l' && fmt[2] == 'l' && (fmt[3] == 'd' || fmt[3] == 'u')) {
        int64_t val = 
                     __builtin_va_arg(
                     ap
                     ,
                     int64_t
                     )
                                        ;
        const char *fmt2 = fmt[3] == 'u' ? "%" 
                                              "l" "u" 
                                                         : "%" 
                                                               "l" "d"
                                                                        ;
        snprintf(buf, sizeof(buf), fmt2, val);
        len += out->printer(out, buf, strlen(buf));
        skip += 2;
      } else if (fmt[1] == 'z' && fmt[2] == 'u') {
        size_t val = 
                    __builtin_va_arg(
                    ap
                    ,
                    size_t
                    )
                                      ;
        snprintf(buf, sizeof(buf), "%lu", (unsigned long) val);
        len += out->printer(out, buf, strlen(buf));
        skip += 1;
      } else if (fmt[1] == 'M') {
        json_printf_callback_t f = 
                                  __builtin_va_arg(
                                  ap
                                  ,
                                  json_printf_callback_t
                                  )
                                                                    ;
        len += f(out, &ap);
      } else if (fmt[1] == 'B') {
        int val = 
                 __builtin_va_arg(
                 ap
                 ,
                 int
                 )
                                ;
        const char *str = val ? "true" : "false";
        len += out->printer(out, str, strlen(str));
      } else if (fmt[1] == 'H') {

        const char *hex = "0123456789abcdef";
        int i, n = 
                  __builtin_va_arg(
                  ap
                  ,
                  int
                  )
                                 ;
        const unsigned char *p = 
                                __builtin_va_arg(
                                ap
                                ,
                                const unsigned char *
                                )
                                                                 ;
        len += out->printer(out, quote, 1);
        for (i = 0; i < n; i++) {
          len += out->printer(out, &hex[(p[i] >> 4) & 0xf], 1);
          len += out->printer(out, &hex[p[i] & 0xf], 1);
        }
        len += out->printer(out, quote, 1);

      } else if (fmt[1] == 'V') {

        const unsigned char *p = 
                                __builtin_va_arg(
                                ap
                                ,
                                const unsigned char *
                                )
                                                                 ;
        int n = 
               __builtin_va_arg(
               ap
               ,
               int
               )
                              ;
        len += out->printer(out, quote, 1);
        len += b64enc(out, p, n);
        len += out->printer(out, quote, 1);

      } else if (fmt[1] == 'Q' ||
                 (fmt[1] == '.' && fmt[2] == '*' && fmt[3] == 'Q')) {
        size_t l = 0;
        const char *p;

        if (fmt[1] == '.') {
          l = (size_t) 
                      __builtin_va_arg(
                      ap
                      ,
                      int
                      )
                                     ;
          skip += 2;
        }
        p = 
           __builtin_va_arg(
           ap
           ,
           char *
           )
                             ;

        if (p == 
                __null
                    ) {
          len += out->printer(out, null, 4);
        } else {
          if (fmt[1] == 'Q') {
            l = strlen(p);
          }
          len += out->printer(out, quote, 1);
          len += json_escape(out, p, l);
          len += out->printer(out, quote, 1);
        }
      } else {
        const char *end_of_format_specifier = "sdfFeEgGlhuIcx.*-0123456789";
        int n = strspn(fmt + 1, end_of_format_specifier);
        char *pbuf = buf;
        int need_len, size = sizeof(buf);
        char fmt2[20];
        va_list ap_copy;
        strncpy(fmt2, fmt,
                n + 1 > (int) sizeof(fmt2) ? sizeof(fmt2) : (size_t) n + 1);
        fmt2[n + 1] = '\0';

        
       __builtin_va_copy(
       ap_copy
       ,
       ap
       )
                           ;
        need_len = vsnprintf(pbuf, size, fmt2, ap_copy);
        
       __builtin_va_end(
       ap_copy
       )
                      ;

        if (need_len < 0) {




          pbuf = 
                __null
                    ;
          while (need_len < 0) {
            free(pbuf);
            size *= 2;
            if ((pbuf = (char *) malloc(size)) == 
                                                 __null
                                                     ) break;
            
           __builtin_va_copy(
           ap_copy
           ,
           ap
           )
                               ;
            need_len = vsnprintf(pbuf, size, fmt2, ap_copy);
            
           __builtin_va_end(
           ap_copy
           )
                          ;
          }
        } else if (need_len >= (int) sizeof(buf)) {




          if ((pbuf = (char *) malloc(need_len + 1)) != 
                                                       __null
                                                           ) {
            
           __builtin_va_copy(
           ap_copy
           ,
           ap
           )
                               ;
            vsnprintf(pbuf, need_len + 1, fmt2, ap_copy);
            
           __builtin_va_end(
           ap_copy
           )
                          ;
          }
        }
        if (pbuf == 
                   __null
                       ) {
          buf[0] = '\0';
          pbuf = buf;
        }







        if ((n + 1 == strlen("%" 
                                "l" "d"
                                      ) && strcmp(fmt2, "%" 
                                                            "l" "d"
                                                                  ) == 0) ||
            (n + 1 == strlen("%" 
                                "l" "u"
                                      ) && strcmp(fmt2, "%" 
                                                            "l" "u"
                                                                  ) == 0)) {
          (void) 
                __builtin_va_arg(
                ap
                ,
                int64_t
                )
                                   ;
        } else if (strcmp(fmt2, "%.*s") == 0) {
          (void) 
                __builtin_va_arg(
                ap
                ,
                int
                )
                               ;
          (void) 
                __builtin_va_arg(
                ap
                ,
                char *
                )
                                  ;
        } else {
          switch (fmt2[n]) {
            case 'u':
            case 'd':
              (void) 
                    __builtin_va_arg(
                    ap
                    ,
                    int
                    )
                                   ;
              break;
            case 'g':
            case 'f':
              (void) 
                    __builtin_va_arg(
                    ap
                    ,
                    double
                    )
                                      ;
              break;
            case 'p':
              (void) 
                    __builtin_va_arg(
                    ap
                    ,
                    void *
                    )
                                      ;
              break;
            default:

              (void) 
                    __builtin_va_arg(
                    ap
                    ,
                    int
                    )
                                   ;
          }
        }

        len += out->printer(out, pbuf, strlen(pbuf));
        skip = n + 1;


        if (pbuf != buf) {
          free(pbuf);
          pbuf = 
                __null
                    ;
        }
      }
      fmt += skip;
    } else if (*fmt == '_' || json_isalpha(*fmt)) {
      len += out->printer(out, quote, 1);
      while (*fmt == '_' || json_isalpha(*fmt) || json_isdigit(*fmt)) {
        len += out->printer(out, fmt, 1);
        fmt++;
      }
      len += out->printer(out, quote, 1);
    } else {
      len += out->printer(out, fmt, 1);
      fmt++;
    }
  }
  
 __builtin_va_end(
 ap
 )
           ;

  return len;
}

int json_printf(struct json_out *out, const char *fmt, ...) __attribute__((weak));
int json_printf(struct json_out *out, const char *fmt, ...) {
  int n;
  va_list ap;
  
 __builtin_va_start(
 ap
 ,
 fmt
 )
                  ;
  n = json_vprintf(out, fmt, ap);
  
 __builtin_va_end(
 ap
 )
           ;
  return n;
}

int json_printf_array(struct json_out *out, va_list *ap) __attribute__((weak));
int json_printf_array(struct json_out *out, va_list *ap) {
  int len = 0;
  char *arr = 
             __builtin_va_arg(
             *ap
             ,
             char *
             )
                                ;
  size_t i, arr_size = 
                      __builtin_va_arg(
                      *ap
                      ,
                      size_t
                      )
                                         ;
  size_t elem_size = 
                    __builtin_va_arg(
                    *ap
                    ,
                    size_t
                    )
                                       ;
  const char *fmt = 
                   __builtin_va_arg(
                   *ap
                   ,
                   char *
                   )
                                      ;
  len += json_printf(out, "[", 1);
  for (i = 0; arr != 
                    __null 
                         && i < arr_size / elem_size; i++) {
    union {
      int64_t i;
      double d;
    } val;
    memcpy(&val, arr + i * elem_size,
           elem_size > sizeof(val) ? sizeof(val) : elem_size);
    if (i > 0) len += json_printf(out, ", ");
    if (strpbrk(fmt, "efg") != 
                              __null
                                  ) {
      len += json_printf(out, fmt, val.d);
    } else {
      len += json_printf(out, fmt, val.i);
    }
  }
  len += json_printf(out, "]", 1);
  return len;
}
int json_walk(const char *json_string, int json_string_length,
              json_walk_callback_t callback, void *callback_data) __attribute__((weak));
int json_walk(const char *json_string, int json_string_length,
              json_walk_callback_t callback, void *callback_data) {
  struct frozen frozen;

  memset(&frozen, 0, sizeof(frozen));
  frozen.end = json_string + json_string_length;
  frozen.cur = json_string;
  frozen.callback_data = callback_data;
  frozen.callback = callback;

  do { int _n = json_doit(&frozen); if (_n < 0) return _n; } while (0);

  return frozen.cur - json_string;
}

struct scan_array_info {
  int found;
  char path[256];
  struct json_token *token;
};

static void json_scanf_array_elem_cb(void *callback_data, const char *name,
                                     size_t name_len, const char *path,
                                     const struct json_token *token) {
  struct scan_array_info *info = (struct scan_array_info *) callback_data;

  (void) name;
  (void) name_len;

  if (strcmp(path, info->path) == 0) {
    *info->token = *token;
    info->found = 1;
  }
}

int json_scanf_array_elem(const char *s, int len, const char *path, int idx,
                          struct json_token *token) __attribute__((weak));
int json_scanf_array_elem(const char *s, int len, const char *path, int idx,
                          struct json_token *token) {
  struct scan_array_info info;
  info.token = token;
  info.found = 0;
  memset(token, 0, sizeof(*token));
  snprintf(info.path, sizeof(info.path), "%s[%d]", path, idx);
  json_walk(s, len, json_scanf_array_elem_cb, &info);
  return info.found ? token->len : -1;
}

struct json_scanf_info {
  int num_conversions;
  char *path;
  const char *fmt;
  void *target;
  void *user_data;
  int type;
};

int json_unescape(const char *src, int slen, char *dst, int dlen) __attribute__((weak));
int json_unescape(const char *src, int slen, char *dst, int dlen) {
  char *send = (char *) src + slen, *dend = dst + dlen, *orig_dst = dst, *p;
  const char *esc1 = "\"\\/bfnrt", *esc2 = "\"\\/\b\f\n\r\t";

  while (src < send) {
    if (*src == '\\') {
      if (++src >= send) return -2;
      if (*src == 'u') {
        if (send - src < 5) return -2;

        if (src[1] == '0' && src[2] == '0') {

          if (dst < dend) *dst = hexdec(src + 3);
          src += 4;
        } else {

          return -1;
        }
      } else if ((p = (char *) strchr(esc1, *src)) != 
                                                     __null
                                                         ) {
        if (dst < dend) *dst = esc2[p - esc1];
      } else {
        return -1;
      }
    } else {
      if (dst < dend) *dst = *src;
    }
    dst++;
    src++;
  }

  return dst - orig_dst;
}

static void json_scanf_cb(void *callback_data, const char *name,
                          size_t name_len, const char *path,
                          const struct json_token *token) {
  struct json_scanf_info *info = (struct json_scanf_info *) callback_data;
  char buf[32];

  (void) name;
  (void) name_len;

  if (token->ptr == 
                   __null
                       ) {




    return;
  }

  if (strcmp(path, info->path) != 0) {

    return;
  }

  switch (info->type) {
    case 'B':
      info->num_conversions++;
      switch (sizeof(bool)) {
        case sizeof(char):
          *(char *) info->target = (token->type == JSON_TYPE_TRUE ? 1 : 0);
          break;
        case sizeof(int):
          *(int *) info->target = (token->type == JSON_TYPE_TRUE ? 1 : 0);
          break;
        default:

          abort();
      }
      break;
    case 'M': {
      union {
        void *p;
        json_scanner_t f;
      } u = {info->target};
      info->num_conversions++;
      u.f(token->ptr, token->len, info->user_data);
      break;
    }
    case 'Q': {
      char **dst = (char **) info->target;
      if (token->type == JSON_TYPE_NULL) {
        *dst = 
              __null
                  ;
      } else {
        int unescaped_len = json_unescape(token->ptr, token->len, 
                                                                 __null
                                                                     , 0);
        if (unescaped_len >= 0 &&
            (*dst = (char *) malloc(unescaped_len + 1)) != 
                                                          __null
                                                              ) {
          info->num_conversions++;
          if (json_unescape(token->ptr, token->len, *dst, unescaped_len) ==
              unescaped_len) {
            (*dst)[unescaped_len] = '\0';
          } else {
            free(*dst);
            *dst = 
                  __null
                      ;
          }
        }
      }
      break;
    }
    case 'H': {

      char **dst = (char **) info->user_data;
      int i, len = token->len / 2;
      *(int *) info->target = len;
      if ((*dst = (char *) malloc(len + 1)) != 
                                              __null
                                                  ) {
        for (i = 0; i < len; i++) {
          (*dst)[i] = hexdec(token->ptr + 2 * i);
        }
        (*dst)[len] = '\0';
        info->num_conversions++;
      }

      break;
    }
    case 'V': {

      char **dst = (char **) info->target;
      int len = token->len * 4 / 3 + 2;
      if ((*dst = (char *) malloc(len + 1)) != 
                                              __null
                                                  ) {
        int n = b64dec(token->ptr, token->len, *dst);
        (*dst)[n] = '\0';
        *(int *) info->user_data = n;
        info->num_conversions++;
      }

      break;
    }
    case 'T':
      info->num_conversions++;
      *(struct json_token *) info->target = *token;
      break;
    default:
      if (token->len >= (int) sizeof(buf)) break;

      memcpy(buf, token->ptr, token->len);
      buf[token->len] = '\0';

      if (info->fmt[1] == 'd' || (info->fmt[1] == 'l' && info->fmt[2] == 'd') ||
          info->fmt[1] == 'i') {
        char *endptr = 
                      __null
                          ;
        long r = strtol(buf, &endptr, 0 );
        if (*endptr == '\0') {
          if (info->fmt[1] == 'l') {
            *((long *) info->target) = r;
          } else {
            *((int *) info->target) = (int) r;
          }
          info->num_conversions++;
        }
      } else if (info->fmt[1] == 'u' ||
                 (info->fmt[1] == 'l' && info->fmt[2] == 'u')) {
        char *endptr = 
                      __null
                          ;
        unsigned long r = strtoul(buf, &endptr, 0 );
        if (*endptr == '\0') {
          if (info->fmt[1] == 'l') {
            *((unsigned long *) info->target) = r;
          } else {
            *((unsigned int *) info->target) = (unsigned int) r;
          }
          info->num_conversions++;
        }
      } else {

        info->num_conversions += sscanf(buf, info->fmt, info->target);

      }
      break;
  }
}

int json_vscanf(const char *s, int len, const char *fmt, va_list ap) __attribute__((weak));
int json_vscanf(const char *s, int len, const char *fmt, va_list ap) {
  char path[256] = "", fmtbuf[20];
  int i = 0;
  char *p = 
           __null
               ;
  struct json_scanf_info info = {0, path, fmtbuf, 
                                                 __null
                                                     , 
                                                       __null
                                                           , 0};

  while (fmt[i] != '\0') {
    if (fmt[i] == '{') {
      strcat(path, ".");
      i++;
    } else if (fmt[i] == '}') {
      if ((p = strrchr(path, '.')) != 
                                     __null
                                         ) *p = '\0';
      i++;
    } else if (fmt[i] == '%') {
      info.target = 
                   __builtin_va_arg(
                   ap
                   ,
                   void *
                   )
                                     ;
      info.type = fmt[i + 1];
      switch (fmt[i + 1]) {
        case 'M':
        case 'V':
        case 'H':
          info.user_data = 
                          __builtin_va_arg(
                          ap
                          ,
                          void *
                          )
                                            ;

        case 'B':
        case 'Q':
        case 'T':
          i += 2;
          break;
        default: {
          const char *delims = ", \t\r\n]}";
          int conv_len = strcspn(fmt + i + 1, delims) + 1;
          memcpy(fmtbuf, fmt + i, conv_len);
          fmtbuf[conv_len] = '\0';
          i += conv_len;
          i += strspn(fmt + i, delims);
          break;
        }
      }
      json_walk(s, len, json_scanf_cb, &info);
    } else if (json_isalpha(fmt[i]) || json_get_utf8_char_len(fmt[i]) > 1) {
      char *pe;
      const char *delims = ": \r\n\t";
      int key_len = strcspn(&fmt[i], delims);
      if ((p = strrchr(path, '.')) != 
                                     __null
                                         ) p[1] = '\0';
      pe = path + strlen(path);
      memcpy(pe, fmt + i, key_len);
      pe[key_len] = '\0';
      i += key_len + strspn(fmt + i + key_len, delims);
    } else {
      i++;
    }
  }
  return info.num_conversions;
}

int json_scanf(const char *str, int len, const char *fmt, ...) __attribute__((weak));
int json_scanf(const char *str, int len, const char *fmt, ...) {
  int result;
  va_list ap;
  
 __builtin_va_start(
 ap
 ,
 fmt
 )
                  ;
  result = json_vscanf(str, len, fmt, ap);
  
 __builtin_va_end(
 ap
 )
           ;
  return result;
}

int json_vfprintf(const char *file_name, const char *fmt, va_list ap) __attribute__((weak));
int json_vfprintf(const char *file_name, const char *fmt, va_list ap) {
  int res = -1;
  FILE *fp = fopen(file_name, "wb");
  if (fp != 
           __null
               ) {
    struct json_out out = { json_printer_file, { { (char *) fp, 0, 0 } } };
    res = json_vprintf(&out, fmt, ap);
    fputc('\n', fp);
    fclose(fp);
  }
  return res;
}

int json_fprintf(const char *file_name, const char *fmt, ...) __attribute__((weak));
int json_fprintf(const char *file_name, const char *fmt, ...) {
  int result;
  va_list ap;
  
 __builtin_va_start(
 ap
 ,
 fmt
 )
                  ;
  result = json_vfprintf(file_name, fmt, ap);
  
 __builtin_va_end(
 ap
 )
           ;
  return result;
}

char *json_fread(const char *path) __attribute__((weak));
char *json_fread(const char *path) {
  FILE *fp;
  char *data = 
              __null
                  ;
  if ((fp = fopen(path, "rb")) == 
                                 __null
                                     ) {
  } else if (fseek(fp, 0, 
                         2
                                 ) != 0) {
    fclose(fp);
  } else {
    long size = ftell(fp);
    if (size > 0 && (data = (char *) malloc(size + 1)) != 
                                                         __null
                                                             ) {
      fseek(fp, 0, 
                  0
                          );
      if (fread(data, 1, size, fp) != (size_t) size) {
        free(data);
        data = 
              __null
                  ;
      } else {
        data[size] = '\0';
      }
    }
    fclose(fp);
  }
  return data;
}

struct json_setf_data {
  const char *json_path;
  const char *base;
  int matched;
  int pos;
  int end;
  int prev;
};

static int get_matched_prefix_len(const char *s1, const char *s2) {
  int i = 0;
  while (s1[i] && s2[i] && s1[i] == s2[i]) i++;
  return i;
}

static void json_vsetf_cb(void *userdata, const char *name, size_t name_len,
                          const char *path, const struct json_token *t) {
  struct json_setf_data *data = (struct json_setf_data *) userdata;
  int off, len = get_matched_prefix_len(path, data->json_path);
  if (t->ptr == 
               __null
                   ) return;
  off = t->ptr - data->base;
  if (len > data->matched) data->matched = len;





  if (len < data->matched && data->pos == 0 &&
      (t->type == JSON_TYPE_OBJECT_END || t->type == JSON_TYPE_ARRAY_END)) {
    data->pos = data->end = data->prev;
  }


  if (strcmp(path, data->json_path) == 0 && t->type != JSON_TYPE_OBJECT_START &&
      t->type != JSON_TYPE_ARRAY_START) {
    data->pos = off;
    data->end = off + t->len;
  }
  if (data->pos == 0) {
    data->prev = off + t->len;
  } else if ((t->ptr[0] == '[' || t->ptr[0] == '{') && off + 1 < data->pos &&
             off + 1 > data->prev) {
    data->prev = off + 1;
  }
  (void) name;
  (void) name_len;
}

int json_vsetf(const char *s, int len, struct json_out *out,
               const char *json_path, const char *json_fmt, va_list ap) __attribute__((weak));
int json_vsetf(const char *s, int len, struct json_out *out,
               const char *json_path, const char *json_fmt, va_list ap) {
  struct json_setf_data data;
  memset(&data, 0, sizeof(data));
  data.json_path = json_path;
  data.base = s;
  data.end = len;
  json_walk(s, len, json_vsetf_cb, &data);
  if (json_fmt == 
                 __null
                     ) {

    json_printf(out, "%.*s", data.prev, s);

    if (s[data.prev - 1] == '{' || s[data.prev - 1] == '[') {
      int i = data.end;
      while (i < len && json_isspace(s[i])) i++;
      if (s[i] == ',') data.end = i + 1;
    }
    json_printf(out, "%.*s", len - data.end, s + data.end);
  } else {

    int n, off = data.matched, depth = 0;


    json_printf(out, "%.*s", data.pos, s);


    while ((n = strcspn(&json_path[off], ".[")) > 0) {
      if (s[data.prev - 1] != '{' && s[data.prev - 1] != '[' && depth == 0) {
        json_printf(out, ",");
      }
      if (off > 0 && json_path[off - 1] != '.') break;
      json_printf(out, "%.*Q:", n, json_path + off);
      off += n;
      if (json_path[off] != '\0') {
        json_printf(out, "%c", json_path[off] == '.' ? '{' : '[');
        depth++;
        off++;
      }
    }

    json_vprintf(out, json_fmt, ap);


    for (; off > data.matched; off--) {
      int ch = json_path[off];
      const char *p = ch == '.' ? "}" : ch == '[' ? "]" : "";
      json_printf(out, "%s", p);
    }


    json_printf(out, "%.*s", len - data.end, s + data.end);
  }
  return data.end > data.pos ? 1 : 0;
}

int json_setf(const char *s, int len, struct json_out *out,
              const char *json_path, const char *json_fmt, ...) __attribute__((weak));
int json_setf(const char *s, int len, struct json_out *out,
              const char *json_path, const char *json_fmt, ...) {
  int result;
  va_list ap;
  
 __builtin_va_start(
 ap
 ,
 json_fmt
 )
                       ;
  result = json_vsetf(s, len, out, json_path, json_fmt, ap);
  
 __builtin_va_end(
 ap
 )
           ;
  return result;
}

struct prettify_data {
  struct json_out *out;
  int level;
  int last_token;
};

static void indent(struct json_out *out, int level) {
  while (level-- > 0) out->printer(out, "  ", 2);
}

static void print_key(struct prettify_data *pd, const char *path,
                      const char *name, int name_len) {
  if (pd->last_token != JSON_TYPE_INVALID &&
      pd->last_token != JSON_TYPE_ARRAY_START &&
      pd->last_token != JSON_TYPE_OBJECT_START) {
    pd->out->printer(pd->out, ",", 1);
  }
  if (path[0] != '\0') pd->out->printer(pd->out, "\n", 1);
  indent(pd->out, pd->level);
  if (path[0] != '\0' && path[strlen(path) - 1] != ']') {
    pd->out->printer(pd->out, "\"", 1);
    pd->out->printer(pd->out, name, (int) name_len);
    pd->out->printer(pd->out, "\"", 1);
    pd->out->printer(pd->out, ": ", 2);
  }
}

static void prettify_cb(void *userdata, const char *name, size_t name_len,
                        const char *path, const struct json_token *t) {
  struct prettify_data *pd = (struct prettify_data *) userdata;
  switch (t->type) {
    case JSON_TYPE_OBJECT_START:
    case JSON_TYPE_ARRAY_START:
      print_key(pd, path, name, name_len);
      pd->out->printer(pd->out, t->type == JSON_TYPE_ARRAY_START ? "[" : "{",
                       1);
      pd->level++;
      break;
    case JSON_TYPE_OBJECT_END:
    case JSON_TYPE_ARRAY_END:
      pd->level--;
      if (pd->last_token != JSON_TYPE_INVALID &&
          pd->last_token != JSON_TYPE_ARRAY_START &&
          pd->last_token != JSON_TYPE_OBJECT_START) {
        pd->out->printer(pd->out, "\n", 1);
        indent(pd->out, pd->level);
      }
      pd->out->printer(pd->out, t->type == JSON_TYPE_ARRAY_END ? "]" : "}", 1);
      break;
    case JSON_TYPE_NUMBER:
    case JSON_TYPE_NULL:
    case JSON_TYPE_TRUE:
    case JSON_TYPE_FALSE:
    case JSON_TYPE_STRING:
      print_key(pd, path, name, name_len);
      if (t->type == JSON_TYPE_STRING) pd->out->printer(pd->out, "\"", 1);
      pd->out->printer(pd->out, t->ptr, t->len);
      if (t->type == JSON_TYPE_STRING) pd->out->printer(pd->out, "\"", 1);
      break;
    default:
      break;
  }
  pd->last_token = t->type;
}

int json_prettify(const char *s, int len, struct json_out *out) __attribute__((weak));
int json_prettify(const char *s, int len, struct json_out *out) {
  struct prettify_data pd = {out, 0, JSON_TYPE_INVALID};
  return json_walk(s, len, prettify_cb, &pd);
}

int json_prettify_file(const char *file_name) __attribute__((weak));
int json_prettify_file(const char *file_name) {
  int res = -1;
  char *s = json_fread(file_name);
  FILE *fp;
  if (s != 
          __null 
               && (fp = fopen(file_name, "wb")) != 
                                                   __null
                                                       ) {
    struct json_out out = { json_printer_file, { { (char *) fp, 0, 0 } } };
    res = json_prettify(s, strlen(s), &out);
    if (res < 0) {

      fclose(fp);
      fp = fopen(file_name, "wb");
      fseek(fp, 0, 
                  0
                          );
      fwrite(s, 1, strlen(s), fp);
    } else {
      fputc('\n', fp);
    }
    fclose(fp);
  }
  free(s);
  return res;
}

struct next_data {
  void *handle;
  const char *path;
  int path_len;
  int found;
  struct json_token *key;
  struct json_token *val;
  int *idx;
};

static void next_set_key(struct next_data *d, const char *name, int name_len,
                         int is_array) {
  if (is_array) {

    if (d->key != 
                 __null
                     ) {
      d->key->len = 0;
      d->key->ptr = 
                   __null
                       ;
    }
    if (d->idx != 
                 __null
                     ) *d->idx = atoi(name);
  } else {

    if (d->key != 
                 __null
                     ) {
      d->key->ptr = name;
      d->key->len = name_len;
    }
    if (d->idx != 
                 __null
                     ) *d->idx = -1;
  }
}

static void json_next_cb(void *userdata, const char *name, size_t name_len,
                         const char *path, const struct json_token *t) {
  struct next_data *d = (struct next_data *) userdata;
  const char *p = path + d->path_len;
  if (d->found) return;
  if (d->path_len >= (int) strlen(path)) return;
  if (strncmp(d->path, path, d->path_len) != 0) return;
  if (strchr(p + 1, '.') != 
                           __null
                               ) return;
  if (strchr(p + 1, '[') != 
                           __null
                               ) return;

  if (t->type == JSON_TYPE_OBJECT_START || t->type == JSON_TYPE_ARRAY_START) {
    next_set_key(d, name, name_len, p[0] == '[');
  } else if (d->handle == 
                         __null 
                              || d->handle < (void *) t->ptr) {
    if (t->type != JSON_TYPE_OBJECT_END && t->type != JSON_TYPE_ARRAY_END) {
      next_set_key(d, name, name_len, p[0] == '[');
    }
    if (d->val != 
                 __null
                     ) *d->val = *t;
    d->handle = (void *) t->ptr;
    d->found = 1;
  }
}

static void *json_next(const char *s, int len, void *handle, const char *path,
                       struct json_token *key, struct json_token *val, int *i) {
  struct json_token tmpval, *v = val == 
                                       __null 
                                            ? &tmpval : val;
  struct json_token tmpkey, *k = key == 
                                       __null 
                                            ? &tmpkey : key;
  int tmpidx, *pidx = i == 
                          __null 
                               ? &tmpidx : i;
  struct next_data data = {handle, path, (int) strlen(path), 0, k, v, pidx};
  json_walk(s, len, json_next_cb, &data);
  return data.found ? data.handle : 
                                   __null
                                       ;
}

void *json_next_key(const char *s, int len, void *handle, const char *path,
                    struct json_token *key, struct json_token *val) __attribute__((weak));
void *json_next_key(const char *s, int len, void *handle, const char *path,
                    struct json_token *key, struct json_token *val) {
  return json_next(s, len, handle, path, key, val, 
                                                  __null
                                                      );
}

void *json_next_elem(const char *s, int len, void *handle, const char *path,
                     int *idx, struct json_token *val) __attribute__((weak));
void *json_next_elem(const char *s, int len, void *handle, const char *path,
                     int *idx, struct json_token *val) {
  return json_next(s, len, handle, path, 
                                        __null
                                            , val, idx);
}

static int json_sprinter(struct json_out *out, const char *str, size_t len) {
  size_t old_len = out->u.buf.buf == 
                                    __null 
                                         ? 0 : strlen(out->u.buf.buf);
  size_t new_len = len + old_len;
  char *p = (char *) realloc(out->u.buf.buf, new_len + 1);
  if (p != 
          __null
              ) {
    memcpy(p + old_len, str, len);
    p[new_len] = '\0';
    out->u.buf.buf = p;
  }
  return len;
}

char *json_vasprintf(const char *fmt, va_list ap) __attribute__((weak));
char *json_vasprintf(const char *fmt, va_list ap) {
  struct json_out out;
  memset(&out, 0, sizeof(out));
  out.printer = json_sprinter;
  json_vprintf(&out, fmt, ap);
  return out.u.buf.buf;
}

char *json_asprintf(const char *fmt, ...) __attribute__((weak));
char *json_asprintf(const char *fmt, ...) {
  char *result = 
                __null
                    ;
  va_list ap;
  
 __builtin_va_start(
 ap
 ,
 fmt
 )
                  ;
  result = json_vasprintf(fmt, ap);
  
 __builtin_va_end(
 ap
 )
           ;
  return result;
}
void ffi_set_word(struct ffi_arg *arg, ffi_word_t v) {
  arg->ctype = FFI_CTYPE_WORD;
  arg->v.i = v;
}

void ffi_set_bool(struct ffi_arg *arg, bool v) {
  arg->ctype = FFI_CTYPE_BOOL;
  arg->v.i = v;
}

void ffi_set_ptr(struct ffi_arg *arg, void *v) {
  ffi_set_word(arg, (ffi_word_t) v);
}

void ffi_set_double(struct ffi_arg *arg, double v) {
  arg->ctype = FFI_CTYPE_DOUBLE;
  arg->v.d = v;
}

void ffi_set_float(struct ffi_arg *arg, float v) {
  arg->ctype = FFI_CTYPE_FLOAT;
  arg->v.f = v;
}
typedef ffi_word_t (*w4w_t)(ffi_word_t, ffi_word_t, ffi_word_t, ffi_word_t);
typedef ffi_word_t (*w5w_t)(ffi_word_t, ffi_word_t, ffi_word_t, ffi_word_t,
                            ffi_word_t);
typedef ffi_word_t (*w6w_t)(ffi_word_t, ffi_word_t, ffi_word_t, ffi_word_t,
                            ffi_word_t, ffi_word_t);

typedef ffi_word_t (*wdw_t)(double, ffi_word_t);
typedef ffi_word_t (*wwd_t)(ffi_word_t, double);
typedef ffi_word_t (*wdd_t)(double, double);

typedef ffi_word_t (*wwwd_t)(ffi_word_t, ffi_word_t, double);
typedef ffi_word_t (*wwdw_t)(ffi_word_t, double, ffi_word_t);
typedef ffi_word_t (*wwdd_t)(ffi_word_t, double, double);
typedef ffi_word_t (*wdww_t)(double, ffi_word_t, ffi_word_t);
typedef ffi_word_t (*wdwd_t)(double, ffi_word_t, double);
typedef ffi_word_t (*wddw_t)(double, double, ffi_word_t);
typedef ffi_word_t (*wddd_t)(double, double, double);

typedef ffi_word_t (*wfw_t)(float, ffi_word_t);
typedef ffi_word_t (*wwf_t)(ffi_word_t, float);
typedef ffi_word_t (*wff_t)(float, float);

typedef ffi_word_t (*wwwf_t)(ffi_word_t, ffi_word_t, float);
typedef ffi_word_t (*wwfw_t)(ffi_word_t, float, ffi_word_t);
typedef ffi_word_t (*wwff_t)(ffi_word_t, float, float);
typedef ffi_word_t (*wfww_t)(float, ffi_word_t, ffi_word_t);
typedef ffi_word_t (*wfwf_t)(float, ffi_word_t, float);
typedef ffi_word_t (*wffw_t)(float, float, ffi_word_t);
typedef ffi_word_t (*wfff_t)(float, float, float);

typedef bool (*b4w_t)(ffi_word_t, ffi_word_t, ffi_word_t, ffi_word_t);
typedef bool (*b5w_t)(ffi_word_t, ffi_word_t, ffi_word_t, ffi_word_t,
                      ffi_word_t);
typedef bool (*b6w_t)(ffi_word_t, ffi_word_t, ffi_word_t, ffi_word_t,
                      ffi_word_t, ffi_word_t);
typedef bool (*bdw_t)(double, ffi_word_t);
typedef bool (*bwd_t)(ffi_word_t, double);
typedef bool (*bdd_t)(double, double);

typedef bool (*bwwd_t)(ffi_word_t, ffi_word_t, double);
typedef bool (*bwdw_t)(ffi_word_t, double, ffi_word_t);
typedef bool (*bwdd_t)(ffi_word_t, double, double);
typedef bool (*bdww_t)(double, ffi_word_t, ffi_word_t);
typedef bool (*bdwd_t)(double, ffi_word_t, double);
typedef bool (*bddw_t)(double, double, ffi_word_t);
typedef bool (*bddd_t)(double, double, double);

typedef bool (*bfw_t)(float, ffi_word_t);
typedef bool (*bwf_t)(ffi_word_t, float);
typedef bool (*bff_t)(float, float);

typedef bool (*bwwf_t)(ffi_word_t, ffi_word_t, float);
typedef bool (*bwfw_t)(ffi_word_t, float, ffi_word_t);
typedef bool (*bwff_t)(ffi_word_t, float, float);
typedef bool (*bfww_t)(float, ffi_word_t, ffi_word_t);
typedef bool (*bfwf_t)(float, ffi_word_t, float);
typedef bool (*bffw_t)(float, float, ffi_word_t);
typedef bool (*bfff_t)(float, float, float);

typedef double (*d4w_t)(ffi_word_t, ffi_word_t, ffi_word_t, ffi_word_t);
typedef double (*d5w_t)(ffi_word_t, ffi_word_t, ffi_word_t, ffi_word_t,
                        ffi_word_t);
typedef double (*d6w_t)(ffi_word_t, ffi_word_t, ffi_word_t, ffi_word_t,
                        ffi_word_t, ffi_word_t);
typedef double (*ddw_t)(double, ffi_word_t);
typedef double (*dwd_t)(ffi_word_t, double);
typedef double (*ddd_t)(double, double);

typedef double (*dwwd_t)(ffi_word_t, ffi_word_t, double);
typedef double (*dwdw_t)(ffi_word_t, double, ffi_word_t);
typedef double (*dwdd_t)(ffi_word_t, double, double);
typedef double (*ddww_t)(double, ffi_word_t, ffi_word_t);
typedef double (*ddwd_t)(double, ffi_word_t, double);
typedef double (*dddw_t)(double, double, ffi_word_t);
typedef double (*dddd_t)(double, double, double);

typedef float (*f4w_t)(ffi_word_t, ffi_word_t, ffi_word_t, ffi_word_t);
typedef float (*f5w_t)(ffi_word_t, ffi_word_t, ffi_word_t, ffi_word_t,
                       ffi_word_t);
typedef float (*f6w_t)(ffi_word_t, ffi_word_t, ffi_word_t, ffi_word_t,
                       ffi_word_t, ffi_word_t);
typedef float (*ffw_t)(float, ffi_word_t);
typedef float (*fwf_t)(ffi_word_t, float);
typedef float (*fff_t)(float, float);

typedef float (*fwwf_t)(ffi_word_t, ffi_word_t, float);
typedef float (*fwfw_t)(ffi_word_t, float, ffi_word_t);
typedef float (*fwff_t)(ffi_word_t, float, float);
typedef float (*ffww_t)(float, ffi_word_t, ffi_word_t);
typedef float (*ffwf_t)(float, ffi_word_t, float);
typedef float (*fffw_t)(float, float, ffi_word_t);
typedef float (*ffff_t)(float, float, float);

int ffi_call(ffi_fn_t *func, int nargs, struct ffi_arg *res,
             struct ffi_arg *args) {
  int i, doubles = 0, floats = 0;

  if (nargs > 6) return -1;
  for (i = 0; i < nargs; i++) {
    doubles += (((args[i]).ctype == FFI_CTYPE_DOUBLE));
    floats += (((args[i]).ctype == FFI_CTYPE_FLOAT));
  }


  if (doubles > 0 && floats > 0) {
    return -1;
  }

  switch (res->ctype) {
    case FFI_CTYPE_WORD: {
      ffi_word_t r;
      if (doubles == 0) {
        if (floats == 0) {




          if (nargs <= 4) {
            w4w_t f = (w4w_t) func;
            r = f(((ffi_word_t)(args[0]).v.i), ((ffi_word_t)(args[1]).v.i), ((ffi_word_t)(args[2]).v.i), ((ffi_word_t)(args[3]).v.i));
          } else if (nargs == 5) {
            w5w_t f = (w5w_t) func;
            r = f(((ffi_word_t)(args[0]).v.i), ((ffi_word_t)(args[1]).v.i), ((ffi_word_t)(args[2]).v.i), ((ffi_word_t)(args[3]).v.i), ((ffi_word_t)(args[4]).v.i));
          } else if (nargs == 6) {
            w6w_t f = (w6w_t) func;
            r = f(((ffi_word_t)(args[0]).v.i), ((ffi_word_t)(args[1]).v.i), ((ffi_word_t)(args[2]).v.i), ((ffi_word_t)(args[3]).v.i), ((ffi_word_t)(args[4]).v.i),
                  ((ffi_word_t)(args[5]).v.i));
          } else {
            abort();
          }
        } else {

          switch (nargs) {
            case 0:
            case 1:
            case 2:
              if (((args[0]).ctype == FFI_CTYPE_FLOAT) && ((args[1]).ctype == FFI_CTYPE_FLOAT)) {
                wff_t f = (wff_t) func;
                r = f(((args[0]).v.f), ((args[1]).v.f));
              } else if (((args[0]).ctype == FFI_CTYPE_FLOAT)) {
                wfw_t f = (wfw_t) func;
                r = f(((args[0]).v.f), ((ffi_word_t)(args[1]).v.i));
              } else {
                wwf_t f = (wwf_t) func;
                r = f(((ffi_word_t)(args[0]).v.i), ((args[1]).v.f));
              }
              break;

            case 3:
              if (((args[0]).ctype == FFI_CTYPE_WORD) && ((args[1]).ctype == FFI_CTYPE_WORD) && ((args[2]).ctype == FFI_CTYPE_FLOAT)) {
                wwwf_t f = (wwwf_t) func;
                r = f(((ffi_word_t)(args[0]).v.i), ((ffi_word_t)(args[1]).v.i), ((args[2]).v.f));
              } else if (((args[0]).ctype == FFI_CTYPE_WORD) && ((args[1]).ctype == FFI_CTYPE_FLOAT) && ((args[2]).ctype == FFI_CTYPE_WORD)) {
                wwfw_t f = (wwfw_t) func;
                r = f(((ffi_word_t)(args[0]).v.i), ((args[1]).v.f), ((ffi_word_t)(args[2]).v.i));
              } else if (((args[0]).ctype == FFI_CTYPE_WORD) && ((args[1]).ctype == FFI_CTYPE_FLOAT) && ((args[2]).ctype == FFI_CTYPE_FLOAT)) {
                wwff_t f = (wwff_t) func;
                r = f(((ffi_word_t)(args[0]).v.i), ((args[1]).v.f), ((args[2]).v.f));
              } else if (((args[0]).ctype == FFI_CTYPE_FLOAT) && ((args[1]).ctype == FFI_CTYPE_WORD) && ((args[2]).ctype == FFI_CTYPE_WORD)) {
                wfww_t f = (wfww_t) func;
                r = f(((args[0]).v.f), ((ffi_word_t)(args[1]).v.i), ((ffi_word_t)(args[2]).v.i));
              } else if (((args[0]).ctype == FFI_CTYPE_FLOAT) && ((args[1]).ctype == FFI_CTYPE_WORD) && ((args[2]).ctype == FFI_CTYPE_FLOAT)) {
                wfwf_t f = (wfwf_t) func;
                r = f(((args[0]).v.f), ((ffi_word_t)(args[1]).v.i), ((args[2]).v.f));
              } else if (((args[0]).ctype == FFI_CTYPE_FLOAT) && ((args[1]).ctype == FFI_CTYPE_FLOAT) && ((args[2]).ctype == FFI_CTYPE_WORD)) {
                wffw_t f = (wffw_t) func;
                r = f(((args[0]).v.f), ((args[1]).v.f), ((ffi_word_t)(args[2]).v.i));
              } else if (((args[0]).ctype == FFI_CTYPE_FLOAT) && ((args[1]).ctype == FFI_CTYPE_FLOAT) && ((args[2]).ctype == FFI_CTYPE_FLOAT)) {
                wfff_t f = (wfff_t) func;
                r = f(((args[0]).v.f), ((args[1]).v.f), ((args[2]).v.f));
              } else {

                abort();
              }
              break;
            default:
              return -1;
          }
        }
      } else {

        switch (nargs) {
          case 0:
          case 1:
          case 2:
            if (((args[0]).ctype == FFI_CTYPE_DOUBLE) && ((args[1]).ctype == FFI_CTYPE_DOUBLE)) {
              wdd_t f = (wdd_t) func;
              r = f(((args[0]).v.d), ((args[1]).v.d));
            } else if (((args[0]).ctype == FFI_CTYPE_DOUBLE)) {
              wdw_t f = (wdw_t) func;
              r = f(((args[0]).v.d), ((ffi_word_t)(args[1]).v.i));
            } else {
              wwd_t f = (wwd_t) func;
              r = f(((ffi_word_t)(args[0]).v.i), ((args[1]).v.d));
            }
            break;

          case 3:
            if (((args[0]).ctype == FFI_CTYPE_WORD) && ((args[1]).ctype == FFI_CTYPE_WORD) && ((args[2]).ctype == FFI_CTYPE_DOUBLE)) {
              wwwd_t f = (wwwd_t) func;
              r = f(((ffi_word_t)(args[0]).v.i), ((ffi_word_t)(args[1]).v.i), ((args[2]).v.d));
            } else if (((args[0]).ctype == FFI_CTYPE_WORD) && ((args[1]).ctype == FFI_CTYPE_DOUBLE) && ((args[2]).ctype == FFI_CTYPE_WORD)) {
              wwdw_t f = (wwdw_t) func;
              r = f(((ffi_word_t)(args[0]).v.i), ((args[1]).v.d), ((ffi_word_t)(args[2]).v.i));
            } else if (((args[0]).ctype == FFI_CTYPE_WORD) && ((args[1]).ctype == FFI_CTYPE_DOUBLE) && ((args[2]).ctype == FFI_CTYPE_DOUBLE)) {
              wwdd_t f = (wwdd_t) func;
              r = f(((ffi_word_t)(args[0]).v.i), ((args[1]).v.d), ((args[2]).v.d));
            } else if (((args[0]).ctype == FFI_CTYPE_DOUBLE) && ((args[1]).ctype == FFI_CTYPE_WORD) && ((args[2]).ctype == FFI_CTYPE_WORD)) {
              wdww_t f = (wdww_t) func;
              r = f(((args[0]).v.d), ((ffi_word_t)(args[1]).v.i), ((ffi_word_t)(args[2]).v.i));
            } else if (((args[0]).ctype == FFI_CTYPE_DOUBLE) && ((args[1]).ctype == FFI_CTYPE_WORD) && ((args[2]).ctype == FFI_CTYPE_DOUBLE)) {
              wdwd_t f = (wdwd_t) func;
              r = f(((args[0]).v.d), ((ffi_word_t)(args[1]).v.i), ((args[2]).v.d));
            } else if (((args[0]).ctype == FFI_CTYPE_DOUBLE) && ((args[1]).ctype == FFI_CTYPE_DOUBLE) && ((args[2]).ctype == FFI_CTYPE_WORD)) {
              wddw_t f = (wddw_t) func;
              r = f(((args[0]).v.d), ((args[1]).v.d), ((ffi_word_t)(args[2]).v.i));
            } else if (((args[0]).ctype == FFI_CTYPE_DOUBLE) && ((args[1]).ctype == FFI_CTYPE_DOUBLE) && ((args[2]).ctype == FFI_CTYPE_DOUBLE)) {
              wddd_t f = (wddd_t) func;
              r = f(((args[0]).v.d), ((args[1]).v.d), ((args[2]).v.d));
            } else {

              abort();
            }
            break;
          default:
            return -1;
        }
      }
      res->v.i = (uint64_t) r;
    } break;
    case FFI_CTYPE_BOOL: {
      ffi_word_t r;
      if (doubles == 0) {
        if (floats == 0) {




          if (nargs <= 4) {
            b4w_t f = (b4w_t) func;
            r = f(((ffi_word_t)(args[0]).v.i), ((ffi_word_t)(args[1]).v.i), ((ffi_word_t)(args[2]).v.i), ((ffi_word_t)(args[3]).v.i));
          } else if (nargs == 5) {
            b5w_t f = (b5w_t) func;
            r = f(((ffi_word_t)(args[0]).v.i), ((ffi_word_t)(args[1]).v.i), ((ffi_word_t)(args[2]).v.i), ((ffi_word_t)(args[3]).v.i), ((ffi_word_t)(args[4]).v.i));
          } else if (nargs == 6) {
            b6w_t f = (b6w_t) func;
            r = f(((ffi_word_t)(args[0]).v.i), ((ffi_word_t)(args[1]).v.i), ((ffi_word_t)(args[2]).v.i), ((ffi_word_t)(args[3]).v.i), ((ffi_word_t)(args[4]).v.i),
                  ((ffi_word_t)(args[5]).v.i));
          } else {
            abort();
          }
        } else {

          switch (nargs) {
            case 0:
            case 1:
            case 2:
              if (((args[0]).ctype == FFI_CTYPE_FLOAT) && ((args[1]).ctype == FFI_CTYPE_FLOAT)) {
                bff_t f = (bff_t) func;
                r = f(((args[0]).v.f), ((args[1]).v.f));
              } else if (((args[0]).ctype == FFI_CTYPE_FLOAT)) {
                bfw_t f = (bfw_t) func;
                r = f(((args[0]).v.f), ((ffi_word_t)(args[1]).v.i));
              } else {
                bwf_t f = (bwf_t) func;
                r = f(((ffi_word_t)(args[0]).v.i), ((args[1]).v.f));
              }
              break;

            case 3:
              if (((args[0]).ctype == FFI_CTYPE_WORD) && ((args[1]).ctype == FFI_CTYPE_WORD) && ((args[2]).ctype == FFI_CTYPE_FLOAT)) {
                bwwf_t f = (bwwf_t) func;
                r = f(((ffi_word_t)(args[0]).v.i), ((ffi_word_t)(args[1]).v.i), ((args[2]).v.f));
              } else if (((args[0]).ctype == FFI_CTYPE_WORD) && ((args[1]).ctype == FFI_CTYPE_FLOAT) && ((args[2]).ctype == FFI_CTYPE_WORD)) {
                bwfw_t f = (bwfw_t) func;
                r = f(((ffi_word_t)(args[0]).v.i), ((args[1]).v.f), ((ffi_word_t)(args[2]).v.i));
              } else if (((args[0]).ctype == FFI_CTYPE_WORD) && ((args[1]).ctype == FFI_CTYPE_FLOAT) && ((args[2]).ctype == FFI_CTYPE_FLOAT)) {
                bwff_t f = (bwff_t) func;
                r = f(((ffi_word_t)(args[0]).v.i), ((args[1]).v.f), ((args[2]).v.f));
              } else if (((args[0]).ctype == FFI_CTYPE_FLOAT) && ((args[1]).ctype == FFI_CTYPE_WORD) && ((args[2]).ctype == FFI_CTYPE_WORD)) {
                bfww_t f = (bfww_t) func;
                r = f(((args[0]).v.f), ((ffi_word_t)(args[1]).v.i), ((ffi_word_t)(args[2]).v.i));
              } else if (((args[0]).ctype == FFI_CTYPE_FLOAT) && ((args[1]).ctype == FFI_CTYPE_WORD) && ((args[2]).ctype == FFI_CTYPE_FLOAT)) {
                bfwf_t f = (bfwf_t) func;
                r = f(((args[0]).v.f), ((ffi_word_t)(args[1]).v.i), ((args[2]).v.f));
              } else if (((args[0]).ctype == FFI_CTYPE_FLOAT) && ((args[1]).ctype == FFI_CTYPE_FLOAT) && ((args[2]).ctype == FFI_CTYPE_WORD)) {
                bffw_t f = (bffw_t) func;
                r = f(((args[0]).v.f), ((args[1]).v.f), ((ffi_word_t)(args[2]).v.i));
              } else if (((args[0]).ctype == FFI_CTYPE_FLOAT) && ((args[1]).ctype == FFI_CTYPE_FLOAT) && ((args[2]).ctype == FFI_CTYPE_FLOAT)) {
                bfff_t f = (bfff_t) func;
                r = f(((args[0]).v.f), ((args[1]).v.f), ((args[2]).v.f));
              } else {

                abort();
              }
              break;
            default:
              return -1;
          }
        }
      } else {

        switch (nargs) {
          case 0:
          case 1:
          case 2:
            if (((args[0]).ctype == FFI_CTYPE_DOUBLE) && ((args[1]).ctype == FFI_CTYPE_DOUBLE)) {
              bdd_t f = (bdd_t) func;
              r = f(((args[0]).v.d), ((args[1]).v.d));
            } else if (((args[0]).ctype == FFI_CTYPE_DOUBLE)) {
              bdw_t f = (bdw_t) func;
              r = f(((args[0]).v.d), ((ffi_word_t)(args[1]).v.i));
            } else {
              bwd_t f = (bwd_t) func;
              r = f(((ffi_word_t)(args[0]).v.i), ((args[1]).v.d));
            }
            break;

          case 3:
            if (((args[0]).ctype == FFI_CTYPE_WORD) && ((args[1]).ctype == FFI_CTYPE_WORD) && ((args[2]).ctype == FFI_CTYPE_DOUBLE)) {
              bwwd_t f = (bwwd_t) func;
              r = f(((ffi_word_t)(args[0]).v.i), ((ffi_word_t)(args[1]).v.i), ((args[2]).v.d));
            } else if (((args[0]).ctype == FFI_CTYPE_WORD) && ((args[1]).ctype == FFI_CTYPE_DOUBLE) && ((args[2]).ctype == FFI_CTYPE_WORD)) {
              bwdw_t f = (bwdw_t) func;
              r = f(((ffi_word_t)(args[0]).v.i), ((args[1]).v.d), ((ffi_word_t)(args[2]).v.i));
            } else if (((args[0]).ctype == FFI_CTYPE_WORD) && ((args[1]).ctype == FFI_CTYPE_DOUBLE) && ((args[2]).ctype == FFI_CTYPE_DOUBLE)) {
              bwdd_t f = (bwdd_t) func;
              r = f(((ffi_word_t)(args[0]).v.i), ((args[1]).v.d), ((args[2]).v.d));
            } else if (((args[0]).ctype == FFI_CTYPE_DOUBLE) && ((args[1]).ctype == FFI_CTYPE_WORD) && ((args[2]).ctype == FFI_CTYPE_WORD)) {
              bdww_t f = (bdww_t) func;
              r = f(((args[0]).v.d), ((ffi_word_t)(args[1]).v.i), ((ffi_word_t)(args[2]).v.i));
            } else if (((args[0]).ctype == FFI_CTYPE_DOUBLE) && ((args[1]).ctype == FFI_CTYPE_WORD) && ((args[2]).ctype == FFI_CTYPE_DOUBLE)) {
              bdwd_t f = (bdwd_t) func;
              r = f(((args[0]).v.d), ((ffi_word_t)(args[1]).v.i), ((args[2]).v.d));
            } else if (((args[0]).ctype == FFI_CTYPE_DOUBLE) && ((args[1]).ctype == FFI_CTYPE_DOUBLE) && ((args[2]).ctype == FFI_CTYPE_WORD)) {
              bddw_t f = (bddw_t) func;
              r = f(((args[0]).v.d), ((args[1]).v.d), ((ffi_word_t)(args[2]).v.i));
            } else if (((args[0]).ctype == FFI_CTYPE_DOUBLE) && ((args[1]).ctype == FFI_CTYPE_DOUBLE) && ((args[2]).ctype == FFI_CTYPE_DOUBLE)) {
              bddd_t f = (bddd_t) func;
              r = f(((args[0]).v.d), ((args[1]).v.d), ((args[2]).v.d));
            } else {

              abort();
            }
            break;
          default:
            return -1;
        }
      }
      res->v.i = (uint64_t) r;
    } break;
    case FFI_CTYPE_DOUBLE: {
      double r;
      if (doubles == 0) {


        if (nargs <= 4) {
          d4w_t f = (d4w_t) func;
          r = f(((ffi_word_t)(args[0]).v.i), ((ffi_word_t)(args[1]).v.i), ((ffi_word_t)(args[2]).v.i), ((ffi_word_t)(args[3]).v.i));
        } else if (nargs == 5) {
          d5w_t f = (d5w_t) func;
          r = f(((ffi_word_t)(args[0]).v.i), ((ffi_word_t)(args[1]).v.i), ((ffi_word_t)(args[2]).v.i), ((ffi_word_t)(args[3]).v.i), ((ffi_word_t)(args[4]).v.i));
        } else if (nargs == 6) {
          d6w_t f = (d6w_t) func;
          r = f(((ffi_word_t)(args[0]).v.i), ((ffi_word_t)(args[1]).v.i), ((ffi_word_t)(args[2]).v.i), ((ffi_word_t)(args[3]).v.i), ((ffi_word_t)(args[4]).v.i),
                ((ffi_word_t)(args[5]).v.i));
        } else {
          abort();
        }
      } else {
        switch (nargs) {
          case 0:
          case 1:
          case 2:
            if (((args[0]).ctype == FFI_CTYPE_DOUBLE) && ((args[1]).ctype == FFI_CTYPE_DOUBLE)) {
              ddd_t f = (ddd_t) func;
              r = f(((args[0]).v.d), ((args[1]).v.d));
            } else if (((args[0]).ctype == FFI_CTYPE_DOUBLE)) {
              ddw_t f = (ddw_t) func;
              r = f(((args[0]).v.d), ((ffi_word_t)(args[1]).v.i));
            } else {
              dwd_t f = (dwd_t) func;
              r = f(((ffi_word_t)(args[0]).v.i), ((args[1]).v.d));
            }
            break;

          case 3:
            if (((args[0]).ctype == FFI_CTYPE_WORD) && ((args[1]).ctype == FFI_CTYPE_WORD) && ((args[2]).ctype == FFI_CTYPE_DOUBLE)) {
              dwwd_t f = (dwwd_t) func;
              r = f(((ffi_word_t)(args[0]).v.i), ((ffi_word_t)(args[1]).v.i), ((args[2]).v.d));
            } else if (((args[0]).ctype == FFI_CTYPE_WORD) && ((args[1]).ctype == FFI_CTYPE_DOUBLE) && ((args[2]).ctype == FFI_CTYPE_WORD)) {
              dwdw_t f = (dwdw_t) func;
              r = f(((ffi_word_t)(args[0]).v.i), ((args[1]).v.d), ((ffi_word_t)(args[2]).v.i));
            } else if (((args[0]).ctype == FFI_CTYPE_WORD) && ((args[1]).ctype == FFI_CTYPE_DOUBLE) && ((args[2]).ctype == FFI_CTYPE_DOUBLE)) {
              dwdd_t f = (dwdd_t) func;
              r = f(((ffi_word_t)(args[0]).v.i), ((args[1]).v.d), ((args[2]).v.d));
            } else if (((args[0]).ctype == FFI_CTYPE_DOUBLE) && ((args[1]).ctype == FFI_CTYPE_WORD) && ((args[2]).ctype == FFI_CTYPE_WORD)) {
              ddww_t f = (ddww_t) func;
              r = f(((args[0]).v.d), ((ffi_word_t)(args[1]).v.i), ((ffi_word_t)(args[2]).v.i));
            } else if (((args[0]).ctype == FFI_CTYPE_DOUBLE) && ((args[1]).ctype == FFI_CTYPE_WORD) && ((args[2]).ctype == FFI_CTYPE_DOUBLE)) {
              ddwd_t f = (ddwd_t) func;
              r = f(((args[0]).v.d), ((ffi_word_t)(args[1]).v.i), ((args[2]).v.d));
            } else if (((args[0]).ctype == FFI_CTYPE_DOUBLE) && ((args[1]).ctype == FFI_CTYPE_DOUBLE) && ((args[2]).ctype == FFI_CTYPE_WORD)) {
              dddw_t f = (dddw_t) func;
              r = f(((args[0]).v.d), ((args[1]).v.d), ((ffi_word_t)(args[2]).v.i));
            } else if (((args[0]).ctype == FFI_CTYPE_DOUBLE) && ((args[1]).ctype == FFI_CTYPE_DOUBLE) && ((args[2]).ctype == FFI_CTYPE_DOUBLE)) {
              dddd_t f = (dddd_t) func;
              r = f(((args[0]).v.d), ((args[1]).v.d), ((args[2]).v.d));
            } else {

              abort();
            }
            break;
          default:
            return -1;
        }
      }
      res->v.d = r;
    } break;
    case FFI_CTYPE_FLOAT: {
      double r;
      if (floats == 0) {


        if (nargs <= 4) {
          f4w_t f = (f4w_t) func;
          r = f(((ffi_word_t)(args[0]).v.i), ((ffi_word_t)(args[1]).v.i), ((ffi_word_t)(args[2]).v.i), ((ffi_word_t)(args[3]).v.i));
        } else if (nargs == 5) {
          f5w_t f = (f5w_t) func;
          r = f(((ffi_word_t)(args[0]).v.i), ((ffi_word_t)(args[1]).v.i), ((ffi_word_t)(args[2]).v.i), ((ffi_word_t)(args[3]).v.i), ((ffi_word_t)(args[4]).v.i));
        } else if (nargs == 6) {
          f6w_t f = (f6w_t) func;
          r = f(((ffi_word_t)(args[0]).v.i), ((ffi_word_t)(args[1]).v.i), ((ffi_word_t)(args[2]).v.i), ((ffi_word_t)(args[3]).v.i), ((ffi_word_t)(args[4]).v.i),
                ((ffi_word_t)(args[5]).v.i));
        } else {
          abort();
        }
      } else {

        switch (nargs) {
          case 0:
          case 1:
          case 2:
            if (((args[0]).ctype == FFI_CTYPE_FLOAT) && ((args[1]).ctype == FFI_CTYPE_FLOAT)) {
              fff_t f = (fff_t) func;
              r = f(((args[0]).v.f), ((args[1]).v.f));
            } else if (((args[0]).ctype == FFI_CTYPE_FLOAT)) {
              ffw_t f = (ffw_t) func;
              r = f(((args[0]).v.f), ((ffi_word_t)(args[1]).v.i));
            } else {
              fwf_t f = (fwf_t) func;
              r = f(((ffi_word_t)(args[0]).v.i), ((args[1]).v.f));
            }
            break;

          case 3:
            if (((args[0]).ctype == FFI_CTYPE_WORD) && ((args[1]).ctype == FFI_CTYPE_WORD) && ((args[2]).ctype == FFI_CTYPE_FLOAT)) {
              fwwf_t f = (fwwf_t) func;
              r = f(((ffi_word_t)(args[0]).v.i), ((ffi_word_t)(args[1]).v.i), ((args[2]).v.f));
            } else if (((args[0]).ctype == FFI_CTYPE_WORD) && ((args[1]).ctype == FFI_CTYPE_FLOAT) && ((args[2]).ctype == FFI_CTYPE_WORD)) {
              fwfw_t f = (fwfw_t) func;
              r = f(((ffi_word_t)(args[0]).v.i), ((args[1]).v.f), ((ffi_word_t)(args[2]).v.i));
            } else if (((args[0]).ctype == FFI_CTYPE_WORD) && ((args[1]).ctype == FFI_CTYPE_FLOAT) && ((args[2]).ctype == FFI_CTYPE_FLOAT)) {
              fwff_t f = (fwff_t) func;
              r = f(((ffi_word_t)(args[0]).v.i), ((args[1]).v.f), ((args[2]).v.f));
            } else if (((args[0]).ctype == FFI_CTYPE_FLOAT) && ((args[1]).ctype == FFI_CTYPE_WORD) && ((args[2]).ctype == FFI_CTYPE_WORD)) {
              ffww_t f = (ffww_t) func;
              r = f(((args[0]).v.f), ((ffi_word_t)(args[1]).v.i), ((ffi_word_t)(args[2]).v.i));
            } else if (((args[0]).ctype == FFI_CTYPE_FLOAT) && ((args[1]).ctype == FFI_CTYPE_WORD) && ((args[2]).ctype == FFI_CTYPE_FLOAT)) {
              ffwf_t f = (ffwf_t) func;
              r = f(((args[0]).v.f), ((ffi_word_t)(args[1]).v.i), ((args[2]).v.f));
            } else if (((args[0]).ctype == FFI_CTYPE_FLOAT) && ((args[1]).ctype == FFI_CTYPE_FLOAT) && ((args[2]).ctype == FFI_CTYPE_WORD)) {
              fffw_t f = (fffw_t) func;
              r = f(((args[0]).v.f), ((args[1]).v.f), ((ffi_word_t)(args[2]).v.i));
            } else if (((args[0]).ctype == FFI_CTYPE_FLOAT) && ((args[1]).ctype == FFI_CTYPE_FLOAT) && ((args[2]).ctype == FFI_CTYPE_FLOAT)) {
              ffff_t f = (ffff_t) func;
              r = f(((args[0]).v.f), ((args[1]).v.f), ((args[2]).v.f));
            } else {

              abort();
            }
            break;
          default:
            return -1;
        }
      }
      res->v.f = r;
    } break;
  }

  return 0;
}
static int v_sprintf_s(char *buf, size_t size, const char *fmt, ...) {
  size_t n;
  va_list ap;
  
 __builtin_va_start(
 ap
 ,
 fmt
 )
                  ;
  n = c_vsnprintf(buf, size, fmt, ap);
  if (n > size) {
    return size;
  }
  return n;
}

mjs_val_t mjs_mk_array(struct mjs *mjs) {
  mjs_val_t ret = mjs_mk_object(mjs);

  ret &= ~((uint64_t)(1) << 63 | (uint64_t) 0x7ff0 << 48 | (uint64_t)(15) << 48);
  ret |= ((uint64_t)(1) << 63 | (uint64_t) 0x7ff0 << 48 | (uint64_t)(12) << 48);
  return ret;
}

int mjs_is_array(mjs_val_t v) {
  return (v & ((uint64_t)(1) << 63 | (uint64_t) 0x7ff0 << 48 | (uint64_t)(15) << 48)) == ((uint64_t)(1) << 63 | (uint64_t) 0x7ff0 << 48 | (uint64_t)(12) << 48);
}

mjs_val_t mjs_array_get(struct mjs *mjs, mjs_val_t arr, unsigned long index) {
  return mjs_array_get2(mjs, arr, index, 
                                        __null
                                            );
}

mjs_val_t mjs_array_get2(struct mjs *mjs, mjs_val_t arr, unsigned long index,
                         int *has) {
  mjs_val_t res = ((uint64_t)(1) << 63 | (uint64_t) 0x7ff0 << 48 | (uint64_t)(3) << 48);

  if (has != 
            __null
                ) {
    *has = 0;
  }

  if (mjs_is_object(arr)) {
    struct mjs_property *p;
    char buf[20];
    int n = v_sprintf_s(buf, sizeof(buf), "%lu", index);
    p = mjs_get_own_property(mjs, arr, buf, n);
    if (p != 
            __null
                ) {
      if (has != 
                __null
                    ) {
        *has = 1;
      }
      res = p->value;
    }
  }

  return res;
}

unsigned long mjs_array_length(struct mjs *mjs, mjs_val_t v) {
  struct mjs_property *p;
  unsigned long len = 0;

  if (!mjs_is_object(v)) {
    len = 0;
    goto clean;
  }

  for (p = get_object_struct(v)->properties; p != 
                                                 __null
                                                     ; p = p->next) {
    int ok = 0;
    unsigned long n = 0;
    str_to_ulong(mjs, p->name, &ok, &n);
    if (ok && n >= len && n < 0xffffffff) {
      len = n + 1;
    }
  }

clean:
  return len;
}

mjs_err_t mjs_array_set(struct mjs *mjs, mjs_val_t arr, unsigned long index,
                        mjs_val_t v) {
  mjs_err_t ret = MJS_OK;

  if (mjs_is_object(arr)) {
    char buf[20];
    int n = v_sprintf_s(buf, sizeof(buf), "%lu", index);
    ret = mjs_set(mjs, arr, buf, n, v);
  } else {
    ret = MJS_TYPE_ERROR;
  }

  return ret;
}

void mjs_array_del(struct mjs *mjs, mjs_val_t arr, unsigned long index) {
  char buf[20];
  int n = v_sprintf_s(buf, sizeof(buf), "%lu", index);
  mjs_del(mjs, arr, buf, n);
}

mjs_err_t mjs_array_push(struct mjs *mjs, mjs_val_t arr, mjs_val_t v) {
  return mjs_array_set(mjs, arr, mjs_array_length(mjs, arr), v);
}

static void mjs_array_push_internal(struct mjs *mjs) {
  mjs_err_t rcode = MJS_OK;
  mjs_val_t ret = ((uint64_t)(1) << 63 | (uint64_t) 0x7ff0 << 48 | (uint64_t)(3) << 48);
  int nargs = mjs_nargs(mjs);
  int i;


  if (!mjs_check_arg(mjs, -1 , "this", MJS_TYPE_OBJECT_ARRAY, 
                                                                     __null
                                                                         )) {
    goto clean;
  }


  for (i = 0; i < nargs; i++) {
    rcode = mjs_array_push(mjs, mjs->vals.this_obj, mjs_arg(mjs, i));
    if (rcode != MJS_OK) {
      mjs_prepend_errorf(mjs, rcode, "");
      goto clean;
    }
  }


  ret = mjs_mk_number(mjs, mjs_array_length(mjs, mjs->vals.this_obj));

clean:
  mjs_return(mjs, ret);
  return;
}

static void move_item(struct mjs *mjs, mjs_val_t arr, unsigned long from,
                      unsigned long to) {
  mjs_val_t cur = mjs_array_get(mjs, arr, from);
  mjs_array_set(mjs, arr, to, cur);
  mjs_array_del(mjs, arr, from);
}

static void mjs_array_splice(struct mjs *mjs) {
  int nargs = mjs_nargs(mjs);
  mjs_err_t rcode = MJS_OK;
  mjs_val_t ret = mjs_mk_array(mjs);
  mjs_val_t start_v = ((uint64_t)(1) << 63 | (uint64_t) 0x7ff0 << 48 | (uint64_t)(3) << 48);
  mjs_val_t deleteCount_v = ((uint64_t)(1) << 63 | (uint64_t) 0x7ff0 << 48 | (uint64_t)(3) << 48);
  int start = 0;
  int arr_len;
  int delete_cnt = 0;
  int new_items_cnt = 0;
  int delta = 0;
  int i;


  if (!mjs_check_arg(mjs, -1 , "this", MJS_TYPE_OBJECT_ARRAY, 
                                                                     __null
                                                                         )) {
    goto clean;
  }


  arr_len = mjs_array_length(mjs, mjs->vals.this_obj);


  if (!mjs_check_arg(mjs, 0, "start", MJS_TYPE_NUMBER, &start_v)) {
    goto clean;
  }
  start = mjs_normalize_idx(mjs_get_int(mjs, start_v), arr_len);


  if (nargs >= 2) {

    if (!mjs_check_arg(mjs, 1, "deleteCount", MJS_TYPE_NUMBER,
                       &deleteCount_v)) {
      goto clean;
    }
    delete_cnt = mjs_get_int(mjs, deleteCount_v);
    new_items_cnt = nargs - 2;
  } else {

    delete_cnt = arr_len - start;
  }
  if (delete_cnt > arr_len - start) {
    delete_cnt = arr_len - start;
  } else if (delete_cnt < 0) {
    delete_cnt = 0;
  }


  delta = new_items_cnt - delete_cnt;





  for (i = 0; i < delete_cnt; i++) {
    mjs_val_t cur = mjs_array_get(mjs, mjs->vals.this_obj, start + i);
    rcode = mjs_array_push(mjs, ret, cur);
    if (rcode != MJS_OK) {
      mjs_prepend_errorf(mjs, rcode, "");
      goto clean;
    }
  }


  if (delta < 0) {
    for (i = start; i < arr_len; i++) {
      if (i >= start - delta) {
        move_item(mjs, mjs->vals.this_obj, i, i + delta);
      } else {
        mjs_array_del(mjs, mjs->vals.this_obj, i);
      }
    }
  } else if (delta > 0) {
    for (i = arr_len - 1; i >= start; i--) {
      move_item(mjs, mjs->vals.this_obj, i, i + delta);
    }
  }


  for (i = 0; i < nargs - 2; i++) {
    mjs_array_set(mjs, mjs->vals.this_obj, start + i,
                  mjs_arg(mjs, 2 + i));
  }

clean:
  mjs_return(mjs, ret);
}
static void add_lineno_map_item(struct pstate *pstate) {
  if (pstate->last_emitted_line_no < pstate->line_no) {
    int offset = pstate->cur_idx - pstate->start_bcode_idx;
    size_t offset_llen = cs_varint_llen(offset);
    size_t lineno_llen = cs_varint_llen(pstate->line_no);
    mbuf_resize(&pstate->offset_lineno_map,
                pstate->offset_lineno_map.size + offset_llen + lineno_llen);


    cs_varint_encode(offset, (uint8_t *) pstate->offset_lineno_map.buf +
                                 pstate->offset_lineno_map.len,
                     offset_llen);
    pstate->offset_lineno_map.len += offset_llen;


    cs_varint_encode(pstate->line_no,
                     (uint8_t *) pstate->offset_lineno_map.buf +
                         pstate->offset_lineno_map.len,
                     lineno_llen);
    pstate->offset_lineno_map.len += lineno_llen;

    pstate->last_emitted_line_no = pstate->line_no;
  }
}

static void emit_byte(struct pstate *pstate, uint8_t byte) {
  add_lineno_map_item(pstate);
  mbuf_insert(&pstate->mjs->bcode_gen, pstate->cur_idx, &byte, sizeof(byte));
  pstate->cur_idx += sizeof(byte);
}

static void emit_int(struct pstate *pstate, int64_t n) {
  struct mbuf *b = &pstate->mjs->bcode_gen;
  size_t llen = cs_varint_llen(n);
  add_lineno_map_item(pstate);
  mbuf_insert(b, pstate->cur_idx, 
                                 __null
                                     , llen);
  cs_varint_encode(n, (uint8_t *) b->buf + pstate->cur_idx, llen);
  pstate->cur_idx += llen;
}

static void emit_str(struct pstate *pstate, const char *ptr, size_t len) {
  struct mbuf *b = &pstate->mjs->bcode_gen;
  size_t llen = cs_varint_llen(len);
  add_lineno_map_item(pstate);
  mbuf_insert(b, pstate->cur_idx, 
                                 __null
                                     , llen + len);
  cs_varint_encode(len, (uint8_t *) b->buf + pstate->cur_idx, llen);
  memcpy(b->buf + pstate->cur_idx + llen, ptr, len);
  pstate->cur_idx += llen + len;
}

static int mjs_bcode_insert_offset(struct pstate *p, struct mjs *mjs,
                                        size_t offset, size_t v) {
  int llen = (int) cs_varint_llen(v);
  int diff = llen - 1;
  
 (static_cast <bool> (
 offset < mjs->bcode_gen.len
 ) ? void (0) : __assert_fail (
 "offset < mjs->bcode_gen.len"
 , "mjs.c", 3361, __extension__ __PRETTY_FUNCTION__))
                                    ;
  if (diff > 0) {
    mbuf_resize(&mjs->bcode_gen, mjs->bcode_gen.size + diff);
  }




  memmove(mjs->bcode_gen.buf + offset + llen,
          mjs->bcode_gen.buf + offset + 1,
          mjs->bcode_gen.len - offset - 1);
  mjs->bcode_gen.len += diff;
  cs_varint_encode(v, (uint8_t *) mjs->bcode_gen.buf + offset, llen);





  if (p->cur_idx >= (int) offset) {
    p->cur_idx += diff;
  }
  return diff;
}

static void mjs_bcode_part_add(struct mjs *mjs,
                                    const struct mjs_bcode_part *bp) {
  mbuf_append(&mjs->bcode_parts, bp, sizeof(*bp));
}

static struct mjs_bcode_part *mjs_bcode_part_get(struct mjs *mjs,
                                                      int num) {
  
 (static_cast <bool> (
 num < mjs_bcode_parts_cnt(mjs)
 ) ? void (0) : __assert_fail (
 "num < mjs_bcode_parts_cnt(mjs)"
 , "mjs.c", 3392, __extension__ __PRETTY_FUNCTION__))
                                       ;
  return (struct mjs_bcode_part *) (mjs->bcode_parts.buf +
                                    num * sizeof(struct mjs_bcode_part));
}

static struct mjs_bcode_part *mjs_bcode_part_get_by_offset(struct mjs *mjs,
                                                                size_t offset) {
  int i;
  int parts_cnt = mjs_bcode_parts_cnt(mjs);
  struct mjs_bcode_part *bp = 
                             __null
                                 ;

  if (offset >= mjs->bcode_len) {
    return 
          __null
              ;
  }

  for (i = 0; i < parts_cnt; i++) {
    bp = mjs_bcode_part_get(mjs, i);
    if (offset < bp->start_idx + bp->data.len) {
      break;
    }
  }


  
 (static_cast <bool> (
 i < parts_cnt
 ) ? void (0) : __assert_fail (
 "i < parts_cnt"
 , "mjs.c", 3415, __extension__ __PRETTY_FUNCTION__))
                      ;

  return bp;
}

static int mjs_bcode_parts_cnt(struct mjs *mjs) {
  return mjs->bcode_parts.len / sizeof(struct mjs_bcode_part);
}

static void mjs_bcode_commit(struct mjs *mjs) {
  struct mjs_bcode_part bp;
  memset(&bp, 0, sizeof(bp));


  mbuf_trim(&mjs->bcode_gen);


  bp.data.p = mjs->bcode_gen.buf;
  bp.data.len = mjs->bcode_gen.len;
  mbuf_init(&mjs->bcode_gen, 0);

  bp.start_idx = mjs->bcode_len;
  bp.exec_res = MJS_ERRS_CNT;

  mjs_bcode_part_add(mjs, &bp);

  mjs->bcode_len += bp.data.len;
}
static void mjs_print(struct mjs *mjs) {
  size_t i, num_args = mjs_nargs(mjs);
  for (i = 0; i < num_args; i++) {
    mjs_fprintf(mjs_arg(mjs, i), mjs, 
                                     stdout
                                           );
    putchar(' ');
  }
  putchar('\n');
  mjs_return(mjs, ((uint64_t)(1) << 63 | (uint64_t) 0x7ff0 << 48 | (uint64_t)(3) << 48));
}





static struct mjs_bcode_part *mjs_get_loaded_file_bcode(struct mjs *mjs,
                                                        const char *filename) {
  int parts_cnt = mjs_bcode_parts_cnt(mjs);
  int i;

  if (filename == 
                 __null
                     ) {
    return 0;
  }

  for (i = 0; i < parts_cnt; i++) {
    struct mjs_bcode_part *bp = mjs_bcode_part_get(mjs, i);
    const char *cur_fn = mjs_get_bcode_filename(mjs, bp);
    if (strcmp(filename, cur_fn) == 0) {
      return bp;
    }
  }
  return 
        __null
            ;
}

static void mjs_load(struct mjs *mjs) {
  mjs_val_t res = ((uint64_t)(1) << 63 | (uint64_t) 0x7ff0 << 48 | (uint64_t)(3) << 48);
  mjs_val_t arg0 = mjs_arg(mjs, 0);
  mjs_val_t arg1 = mjs_arg(mjs, 1);
  int custom_global = 0;

  if (mjs_is_string(arg0)) {
    const char *path = mjs_get_cstring(mjs, &arg0);
    struct mjs_bcode_part *bp = 
                               __null
                                   ;
    mjs_err_t ret;

    if (mjs_is_object(arg1)) {
      custom_global = 1;
      push_mjs_val(&mjs->scopes, arg1);
    }
    bp = mjs_get_loaded_file_bcode(mjs, path);
    if (bp == 
             __null
                 ) {

      ret = mjs_exec_file(mjs, path, &res);
    } else {
      if (bp->exec_res != MJS_OK || custom_global) {
        ret = mjs_execute(mjs, bp->start_idx, &res);
      } else {
        ret = MJS_OK;
      }
    }
    if (ret != MJS_OK) {




      arg0 = mjs_arg(mjs, 0);
      path = mjs_get_cstring(mjs, &arg0);
      mjs_prepend_errorf(mjs, ret, "failed to exec file \"%s\"", path);
      goto clean;
    }

  clean:
    if (custom_global) {
      mjs_pop_val(&mjs->scopes);
    }
  }
  mjs_return(mjs, res);
}

static void mjs_get_mjs(struct mjs *mjs) {
  mjs_return(mjs, mjs_mk_foreign(mjs, mjs));
}

static void mjs_chr(struct mjs *mjs) {
  mjs_val_t arg0 = mjs_arg(mjs, 0), res = ((uint64_t)(1) << 63 | (uint64_t) 0x7ff0 << 48 | (uint64_t)(15) << 48);
  int n = mjs_get_int(mjs, arg0);
  if (mjs_is_number(arg0) && n >= 0 && n <= 255) {
    uint8_t s = n;
    res = mjs_mk_string(mjs, (const char *) &s, sizeof(s), 1);
  }
  mjs_return(mjs, res);
}

static void mjs_do_gc(struct mjs *mjs) {
  mjs_val_t arg0 = mjs_arg(mjs, 0);
  mjs_gc(mjs, mjs_is_boolean(arg0) ? mjs_get_bool(mjs, arg0) : 0);
  mjs_return(mjs, arg0);
}

static void mjs_s2o(struct mjs *mjs) {
  mjs_return(mjs,
             mjs_struct_to_obj(mjs, mjs_get_ptr(mjs, mjs_arg(mjs, 0)),
                               (const struct mjs_c_struct_member *) mjs_get_ptr(
                                   mjs, mjs_arg(mjs, 1))));
}

void mjs_init_builtin(struct mjs *mjs, mjs_val_t obj) {
  mjs_val_t v;

  mjs_set(mjs, obj, "global", ~0, obj);

  mjs_set(mjs, obj, "load", ~0,
          mjs_mk_foreign_func(mjs, (mjs_func_ptr_t) mjs_load));
  mjs_set(mjs, obj, "print", ~0,
          mjs_mk_foreign_func(mjs, (mjs_func_ptr_t) mjs_print));
  mjs_set(mjs, obj, "ffi", ~0,
          mjs_mk_foreign_func(mjs, (mjs_func_ptr_t) mjs_ffi_call));
  mjs_set(mjs, obj, "ffi_cb_free", ~0,
          mjs_mk_foreign_func(mjs, (mjs_func_ptr_t) mjs_ffi_cb_free));
  mjs_set(mjs, obj, "mkstr", ~0,
          mjs_mk_foreign_func(mjs, (mjs_func_ptr_t) mjs_mkstr));
  mjs_set(mjs, obj, "getMJS", ~0,
          mjs_mk_foreign_func(mjs, (mjs_func_ptr_t) mjs_get_mjs));
  mjs_set(mjs, obj, "die", ~0,
          mjs_mk_foreign_func(mjs, (mjs_func_ptr_t) mjs_die));
  mjs_set(mjs, obj, "gc", ~0,
          mjs_mk_foreign_func(mjs, (mjs_func_ptr_t) mjs_do_gc));
  mjs_set(mjs, obj, "chr", ~0,
          mjs_mk_foreign_func(mjs, (mjs_func_ptr_t) mjs_chr));
  mjs_set(mjs, obj, "s2o", ~0,
          mjs_mk_foreign_func(mjs, (mjs_func_ptr_t) mjs_s2o));




  v = mjs_mk_object(mjs);
  mjs_set(mjs, v, "stringify", ~0,
          mjs_mk_foreign_func(mjs, (mjs_func_ptr_t) mjs_op_json_stringify));
  mjs_set(mjs, v, "parse", ~0,
          mjs_mk_foreign_func(mjs, (mjs_func_ptr_t) mjs_op_json_parse));
  mjs_set(mjs, obj, "JSON", ~0, v);




  v = mjs_mk_object(mjs);
  mjs_set(mjs, v, "create", ~0,
          mjs_mk_foreign_func(mjs, (mjs_func_ptr_t) mjs_op_create_object));
  mjs_set(mjs, obj, "Object", ~0, v);




  mjs_set(mjs, obj, "NaN", ~0, ((uint64_t)(1) << 63 | (uint64_t) 0x7ff0 << 48 | (uint64_t)(5) << 48));
  mjs_set(mjs, obj, "isNaN", ~0,
          mjs_mk_foreign_func(mjs, (mjs_func_ptr_t) mjs_op_isnan));
}
static mjs_err_t mjs_to_string(struct mjs *mjs, mjs_val_t *v, char **p,
                                    size_t *sizep, int *need_free) {
  mjs_err_t ret = MJS_OK;

  *p = 
      __null
          ;
  *sizep = 0;
  *need_free = 0;

  if (mjs_is_string(*v)) {
    *p = (char *) mjs_get_string(mjs, v, sizep);
  } else if (mjs_is_number(*v)) {
    char buf[50] = "";
    struct json_out out = { json_printer_buf, { { buf, sizeof(buf), 0 } } };
    mjs_jprintf(*v, mjs, &out);
    *sizep = strlen(buf);
    *p = (char*)malloc(*sizep + 1);
    if (*p == 
             __null
                 ) {
      ret = MJS_OUT_OF_MEMORY;
      goto clean;
    }
    memmove(*p, buf, *sizep + 1);
    *need_free = 1;
  } else if (mjs_is_boolean(*v)) {
    if (mjs_get_bool(mjs, *v)) {
      *p = (char*)"true";
      *sizep = 4;
    } else {
      *p = (char*) "false";
      *sizep = 5;
    }
  } else if (mjs_is_undefined(*v)) {
    *p = (char*) "undefined";
    *sizep = 9;
  } else if (mjs_is_null(*v)) {
    *p = (char*) "null";
    *sizep = 4;
  } else if (mjs_is_object(*v)) {
    ret = MJS_TYPE_ERROR;
    mjs_set_errorf(mjs, ret,
                   "conversion from object to string is not supported");
  } else if (mjs_is_foreign(*v)) {
    *p = (char*) "TODO_foreign";
    *sizep = 12;
  } else {
    ret = MJS_TYPE_ERROR;
    mjs_set_errorf(mjs, ret, "unknown type to convert to string");
  }

clean:
  return ret;
}

static mjs_val_t mjs_to_boolean_v(struct mjs *mjs, mjs_val_t v) {
  size_t len;
  int is_truthy;

  is_truthy =
      ((mjs_is_boolean(v) && mjs_get_bool(mjs, v)) ||
       (mjs_is_number(v) && mjs_get_double(mjs, v) != 0.0) ||
       (mjs_is_string(v) && mjs_get_string(mjs, &v, &len) && len > 0) ||
       (mjs_is_function(v)) || (mjs_is_foreign(v)) || (mjs_is_object(v))) &&
      v != ((uint64_t)(1) << 63 | (uint64_t) 0x7ff0 << 48 | (uint64_t)(5) << 48);

  return mjs_mk_boolean(mjs, is_truthy);
}

static int mjs_is_truthy(struct mjs *mjs, mjs_val_t v) {
  return mjs_get_bool(mjs, mjs_to_boolean_v(mjs, v));
}
void mjs_destroy(struct mjs *mjs) {
  {
    int parts_cnt = mjs_bcode_parts_cnt(mjs);
    int i;
    for (i = 0; i < parts_cnt; i++) {
      struct mjs_bcode_part *bp = mjs_bcode_part_get(mjs, i);
      if (!bp->in_rom) {
        free((void *) bp->data.p);
      }
    }
  }

  mbuf_free(&mjs->bcode_gen);
  mbuf_free(&mjs->bcode_parts);
  mbuf_free(&mjs->stack);
  mbuf_free(&mjs->call_stack);
  mbuf_free(&mjs->arg_stack);
  mbuf_free(&mjs->owned_strings);
  mbuf_free(&mjs->foreign_strings);
  mbuf_free(&mjs->owned_values);
  mbuf_free(&mjs->scopes);
  mbuf_free(&mjs->loop_addresses);
  mbuf_free(&mjs->json_visited_stack);
  free(mjs->error_msg);
  free(mjs->stack_trace);
  mjs_ffi_args_free_list(mjs);
  gc_arena_destroy(mjs, &mjs->object_arena);
  gc_arena_destroy(mjs, &mjs->property_arena);
  gc_arena_destroy(mjs, &mjs->ffi_sig_arena);
  free(mjs);
}

struct mjs *mjs_create(void) {
  mjs_val_t global_object;
  struct mjs *mjs = (struct mjs*)calloc(1, sizeof(*mjs));
  mbuf_init(&mjs->stack, 0);
  mbuf_init(&mjs->call_stack, 0);
  mbuf_init(&mjs->arg_stack, 0);
  mbuf_init(&mjs->owned_strings, 0);
  mbuf_init(&mjs->foreign_strings, 0);
  mbuf_init(&mjs->bcode_gen, 0);
  mbuf_init(&mjs->bcode_parts, 0);
  mbuf_init(&mjs->owned_values, 0);
  mbuf_init(&mjs->scopes, 0);
  mbuf_init(&mjs->loop_addresses, 0);
  mbuf_init(&mjs->json_visited_stack, 0);

  mjs->bcode_len = 0;





  {
    char z = 0;
    mbuf_append(&mjs->owned_strings, &z, 1);
  }

  gc_arena_init(&mjs->object_arena, sizeof(struct mjs_object),
                20, 10);
  gc_arena_init(&mjs->property_arena, sizeof(struct mjs_property),
                20, 10);
  gc_arena_init(&mjs->ffi_sig_arena, sizeof(struct mjs_ffi_sig),
                20, 10);
  mjs->ffi_sig_arena.destructor = mjs_ffi_sig_destructor;

  global_object = mjs_mk_object(mjs);
  mjs_init_builtin(mjs, global_object);

  push_mjs_val(&mjs->scopes, global_object);
  mjs->vals.this_obj = ((uint64_t)(1) << 63 | (uint64_t) 0x7ff0 << 48 | (uint64_t)(3) << 48);
  mjs->vals.dataview_proto = ((uint64_t)(1) << 63 | (uint64_t) 0x7ff0 << 48 | (uint64_t)(3) << 48);

  return mjs;
}

mjs_err_t mjs_set_errorf(struct mjs *mjs, mjs_err_t err, const char *fmt, ...) {
  va_list ap;
  
 __builtin_va_start(
 ap
 ,
 fmt
 )
                  ;
  free(mjs->error_msg);
  mjs->error_msg = 
                  __null
                      ;
  mjs->error = err;
  if (fmt != 
            __null
                ) {
    mg_avprintf(&mjs->error_msg, 0, fmt, ap);
  }
  
 __builtin_va_end(
 ap
 )
           ;
  return err;
}

mjs_err_t mjs_prepend_errorf(struct mjs *mjs, mjs_err_t err, const char *fmt,
                             ...) {
  char *old_error_msg = mjs->error_msg;
  char *new_error_msg = 
                       __null
                           ;
  va_list ap;
  
 __builtin_va_start(
 ap
 ,
 fmt
 )
                  ;


  
 (static_cast <bool> (
 err != MJS_OK
 ) ? void (0) : __assert_fail (
 "err != MJS_OK"
 , "mjs.c", 3849, __extension__ __PRETTY_FUNCTION__))
                      ;

  mjs->error_msg = 
                  __null
                      ;

  if (mjs->error == MJS_OK) {
    mjs->error = err;
  }
  mg_avprintf(&new_error_msg, 0, fmt, ap);
  
 __builtin_va_end(
 ap
 )
           ;

  if (old_error_msg != 
                      __null
                          ) {
    mg_asprintf(&mjs->error_msg, 0, "%s: %s", new_error_msg, old_error_msg);
    free(new_error_msg);
    free(old_error_msg);
  } else {
    mjs->error_msg = new_error_msg;
  }
  return err;
}

void mjs_print_error(struct mjs *mjs, FILE *fp, const char *msg,
                     int print_stack_trace) {
  if (print_stack_trace && mjs->stack_trace != 
                                              __null
                                                  ) {
    fprintf(fp, "%s", mjs->stack_trace);
  }

  if (msg == 
            __null
                ) {
    msg = "MJS error";
  }

  fprintf(fp, "%s: %s\n", msg, mjs_strerror(mjs, mjs->error));
}

static void mjs_die(struct mjs *mjs) {
  mjs_val_t msg_v = ((uint64_t)(1) << 63 | (uint64_t) 0x7ff0 << 48 | (uint64_t)(3) << 48);
  const char *msg = 
                   __null
                       ;
  size_t msg_len = 0;


  if (!mjs_check_arg(mjs, 0, "msg", MJS_TYPE_STRING, &msg_v)) {
    goto clean;
  }

  msg = mjs_get_string(mjs, &msg_v, &msg_len);


  mjs_prepend_errorf(mjs, MJS_TYPE_ERROR, "%.*s", (int) msg_len, msg);

clean:
  mjs_return(mjs, ((uint64_t)(1) << 63 | (uint64_t) 0x7ff0 << 48 | (uint64_t)(3) << 48));
}

const char *mjs_strerror(struct mjs *mjs, enum mjs_err err) {
  const char *err_names[] = {
      "NO_ERROR", "SYNTAX_ERROR", "REFERENCE_ERROR",
      "TYPE_ERROR", "OUT_OF_MEMORY", "INTERNAL_ERROR",
      "NOT_IMPLEMENTED", "FILE_OPEN_ERROR", "BAD_ARGUMENTS"};
  return mjs->error_msg == 
                          __null 
                               || mjs->error_msg[0] == '\0' ? err_names[err]
                                                             : mjs->error_msg;
}

static size_t mjs_get_func_addr(mjs_val_t v) {
  return v & ~((uint64_t)(1) << 63 | (uint64_t) 0x7ff0 << 48 | (uint64_t)(15) << 48);
}

static enum mjs_type mjs_get_type(mjs_val_t v) {
  int tag;
  if (mjs_is_number(v)) {
    return MJS_TYPE_NUMBER;
  }
  tag = (v & ((uint64_t)(1) << 63 | (uint64_t) 0x7ff0 << 48 | (uint64_t)(15) << 48)) >> 48;
  switch (tag) {
    case ((uint64_t)(1) << 63 | (uint64_t) 0x7ff0 << 48 | (uint64_t)(2) << 48) >> 48:
      return MJS_TYPE_FOREIGN;
    case ((uint64_t)(1) << 63 | (uint64_t) 0x7ff0 << 48 | (uint64_t)(3) << 48) >> 48:
      return MJS_TYPE_UNDEFINED;
    case ((uint64_t)(1) << 63 | (uint64_t) 0x7ff0 << 48 | (uint64_t)(1) << 48) >> 48:
      return MJS_TYPE_OBJECT_GENERIC;
    case ((uint64_t)(1) << 63 | (uint64_t) 0x7ff0 << 48 | (uint64_t)(12) << 48) >> 48:
      return MJS_TYPE_OBJECT_ARRAY;
    case ((uint64_t)(1) << 63 | (uint64_t) 0x7ff0 << 48 | (uint64_t)(13) << 48) >> 48:
      return MJS_TYPE_OBJECT_FUNCTION;
    case ((uint64_t)(1) << 63 | (uint64_t) 0x7ff0 << 48 | (uint64_t)(6) << 48) >> 48:
    case ((uint64_t)(1) << 63 | (uint64_t) 0x7ff0 << 48 | (uint64_t)(8) << 48) >> 48:
    case ((uint64_t)(1) << 63 | (uint64_t) 0x7ff0 << 48 | (uint64_t)(9) << 48) >> 48:
    case ((uint64_t)(1) << 63 | (uint64_t) 0x7ff0 << 48 | (uint64_t)(11) << 48) >> 48:
    case ((uint64_t)(1) << 63 | (uint64_t) 0x7ff0 << 48 | (uint64_t)(7) << 48) >> 48:
      return MJS_TYPE_STRING;
    case ((uint64_t)(1) << 63 | (uint64_t) 0x7ff0 << 48 | (uint64_t)(4) << 48) >> 48:
      return MJS_TYPE_BOOLEAN;
    case ((uint64_t)(1) << 63 | (uint64_t) 0x7ff0 << 48 | (uint64_t)(15) << 48) >> 48:
      return MJS_TYPE_NULL;
    default:
      abort();
      return MJS_TYPE_UNDEFINED;
  }
}

mjs_val_t mjs_get_global(struct mjs *mjs) {
  return *vptr(&mjs->scopes, 0);
}

static void mjs_append_stack_trace_line(struct mjs *mjs, size_t offset) {
  if (offset != ((size_t) 0x7fffffff)) {
    const char *filename = mjs_get_bcode_filename_by_offset(mjs, offset);
    int line_no = mjs_get_lineno_by_offset(mjs, offset);
    char *new_line = 
                    __null
                        ;
    const char *fmt = "  at %s:%d\n";
    if (filename == 
                   __null
                       ) {
      fprintf(
             stderr
                   ,
              "ERROR during stack trace generation: wrong bcode offset %d\n",
              (int) offset);
      filename = "<unknown-filename>";
    }
    mg_asprintf(&new_line, 0, fmt, filename, line_no);

    if (mjs->stack_trace != 
                           __null
                               ) {
      char *old = mjs->stack_trace;
      mg_asprintf(&mjs->stack_trace, 0, "%s%s", mjs->stack_trace, new_line);
      free(old);
      free(new_line);
    } else {
      mjs->stack_trace = new_line;
    }
  }
}

static void mjs_gen_stack_trace(struct mjs *mjs, size_t offset) {
  mjs_append_stack_trace_line(mjs, offset);
  while (mjs->call_stack.len >=
         sizeof(mjs_val_t) * CALL_STACK_FRAME_ITEMS_CNT) {
    int i;


    offset = mjs_get_int(
        mjs, *vptr(&mjs->call_stack, -1 - CALL_STACK_FRAME_ITEM_RETURN_ADDR));


    for (i = 0; i < CALL_STACK_FRAME_ITEMS_CNT; i++) {
      mjs_pop_val(&mjs->call_stack);
    }

    mjs_append_stack_trace_line(mjs, offset);
  }
}

void mjs_own(struct mjs *mjs, mjs_val_t *v) {
  mbuf_append(&mjs->owned_values, &v, sizeof(v));
}

int mjs_disown(struct mjs *mjs, mjs_val_t *v) {
  mjs_val_t **vp = (mjs_val_t **) (mjs->owned_values.buf +
                                   mjs->owned_values.len - sizeof(v));

  for (; (char *) vp >= mjs->owned_values.buf; vp--) {
    if (*vp == v) {
      *vp = *(mjs_val_t **) (mjs->owned_values.buf + mjs->owned_values.len -
                             sizeof(v));
      mjs->owned_values.len -= sizeof(v);
      return 1;
    }
  }

  return 0;
}





static int mjs_getretvalpos(struct mjs *mjs) {
  int pos;
  mjs_val_t *ppos = vptr(&mjs->call_stack, -1);

  
 (static_cast <bool> (
 ppos != 
 __null 
 && mjs_is_number(*ppos)
 ) ? void (0) : __assert_fail (
 "ppos != NULL && mjs_is_number(*ppos)"
 , "mjs.c", 4023, __extension__ __PRETTY_FUNCTION__))
                                             ;
  pos = mjs_get_int(mjs, *ppos) - 1;
  
 (static_cast <bool> (
 pos < (int) mjs_stack_size(&mjs->stack)
 ) ? void (0) : __assert_fail (
 "pos < (int) mjs_stack_size(&mjs->stack)"
 , "mjs.c", 4025, __extension__ __PRETTY_FUNCTION__))
                                                ;
  return pos;
}

int mjs_nargs(struct mjs *mjs) {
  int top = mjs_stack_size(&mjs->stack);
  int pos = mjs_getretvalpos(mjs) + 1;

  return pos > 0 && pos < top ? top - pos : 0;
}

mjs_val_t mjs_arg(struct mjs *mjs, int arg_index) {
  mjs_val_t res = ((uint64_t)(1) << 63 | (uint64_t) 0x7ff0 << 48 | (uint64_t)(3) << 48);
  int top = mjs_stack_size(&mjs->stack);
  int pos = mjs_getretvalpos(mjs) + 1;

  if (pos > 0 && pos + arg_index < top) {
    res = *vptr(&mjs->stack, pos + arg_index);
  }
  return res;
}

void mjs_return(struct mjs *mjs, mjs_val_t v) {
  int pos = mjs_getretvalpos(mjs);

  mjs->stack.len = sizeof(mjs_val_t) * pos;
  mjs_push(mjs, v);
}

static mjs_val_t vtop(struct mbuf *m) {
  size_t size = mjs_stack_size(m);
  return size > 0 ? *vptr(m, size - 1) : ((uint64_t)(1) << 63 | (uint64_t) 0x7ff0 << 48 | (uint64_t)(3) << 48);
}

static size_t mjs_stack_size(const struct mbuf *m) {
  return m->len / sizeof(mjs_val_t);
}

static mjs_val_t *vptr(struct mbuf *m, int idx) {
  int size = mjs_stack_size(m);
  if (idx < 0) idx = size + idx;
  return idx >= 0 && idx < size ? &((mjs_val_t *) m->buf)[idx] : 
                                                                __null
                                                                    ;
}

static mjs_val_t mjs_pop(struct mjs *mjs) {
  if (mjs->stack.len == 0) {
    mjs_set_errorf(mjs, MJS_INTERNAL_ERROR, "stack underflow");
    return ((uint64_t)(1) << 63 | (uint64_t) 0x7ff0 << 48 | (uint64_t)(3) << 48);
  } else {
    return mjs_pop_val(&mjs->stack);
  }
}

static void push_mjs_val(struct mbuf *m, mjs_val_t v) {
  mbuf_append(m, &v, sizeof(v));
}

static mjs_val_t mjs_pop_val(struct mbuf *m) {
  mjs_val_t v = ((uint64_t)(1) << 63 | (uint64_t) 0x7ff0 << 48 | (uint64_t)(3) << 48);
  
 (static_cast <bool> (
 m->len >= sizeof(v)
 ) ? void (0) : __assert_fail (
 "m->len >= sizeof(v)"
 , "mjs.c", 4084, __extension__ __PRETTY_FUNCTION__))
                            ;
  if (m->len >= sizeof(v)) {
    memcpy(&v, m->buf + m->len - sizeof(v), sizeof(v));
    m->len -= sizeof(v);
  }
  return v;
}

static void mjs_push(struct mjs *mjs, mjs_val_t v) {
  push_mjs_val(&mjs->stack, v);
}

void mjs_set_generate_jsc(struct mjs *mjs, int generate_jsc) {
  mjs->generate_jsc = generate_jsc;
}
void *mjs_mem_to_ptr(unsigned val) {
  return (void *) (uintptr_t) val;
}

void *mjs_mem_get_ptr(void *base, int offset) {
  return (char *) base + offset;
}

void mjs_mem_set_ptr(void *ptr, void *val) {
  *(void **) ptr = val;
}

double mjs_mem_get_dbl(void *ptr) {
  double v;
  memcpy(&v, ptr, sizeof(v));
  return v;
}

void mjs_mem_set_dbl(void *ptr, double val) {
  memcpy(ptr, &val, sizeof(val));
}





double mjs_mem_get_uint(void *ptr, int size, int bigendian) {
  uint8_t *p = (uint8_t *) ptr;
  int i, inc = bigendian ? 1 : -1;
  unsigned int res = 0;
  p += bigendian ? 0 : size - 1;
  for (i = 0; i < size; i++, p += inc) {
    res <<= 8;
    res |= *p;
  }
  return res;
}





double mjs_mem_get_int(void *ptr, int size, int bigendian) {
  uint8_t *p = (uint8_t *) ptr;
  int i, inc = bigendian ? 1 : -1;
  int res = 0;
  p += bigendian ? 0 : size - 1;

  for (i = 0; i < size; i++, p += inc) {
    res <<= 8;
    res |= *p;
  }


  {
    int extra = sizeof(res) - size;
    for (i = 0; i < extra; i++) res <<= 8;
    for (i = 0; i < extra; i++) res >>= 8;
  }

  return res;
}

void mjs_mem_set_uint(void *ptr, unsigned int val, int size, int bigendian) {
  uint8_t *p = (uint8_t *) ptr + (bigendian ? size - 1 : 0);
  int i, inc = bigendian ? -1 : 1;
  for (i = 0; i < size; i++, p += inc) {
    *p = val & 0xff;
    val >>= 8;
  }
}

void mjs_mem_set_int(void *ptr, int val, int size, int bigendian) {
  mjs_mem_set_uint(ptr, val, size, bigendian);
}
static void call_stack_push_frame(struct mjs *mjs, size_t offset,
                                  mjs_val_t retval_stack_idx) {

  mjs_val_t this_obj = mjs_pop_val(&mjs->arg_stack);




  push_mjs_val(&mjs->call_stack, mjs->vals.this_obj);
  mjs->vals.this_obj = this_obj;

  push_mjs_val(&mjs->call_stack, mjs_mk_number(mjs, (double) offset));
  push_mjs_val(&mjs->call_stack,
               mjs_mk_number(mjs, (double) mjs_stack_size(&mjs->scopes)));
  push_mjs_val(
      &mjs->call_stack,
      mjs_mk_number(mjs, (double) mjs_stack_size(&mjs->loop_addresses)));
  push_mjs_val(&mjs->call_stack, retval_stack_idx);
}




static size_t call_stack_restore_frame(struct mjs *mjs) {
  size_t retval_stack_idx, return_address, scope_index, loop_addr_index;
  
 (static_cast <bool> (
 mjs_stack_size(&mjs->call_stack) >= CALL_STACK_FRAME_ITEMS_CNT
 ) ? void (0) : __assert_fail (
 "mjs_stack_size(&mjs->call_stack) >= CALL_STACK_FRAME_ITEMS_CNT"
 , "mjs.c", 4245, __extension__ __PRETTY_FUNCTION__))
                                                                       ;




  retval_stack_idx = mjs_get_int(mjs, mjs_pop_val(&mjs->call_stack));
  loop_addr_index = mjs_get_int(mjs, mjs_pop_val(&mjs->call_stack));
  scope_index = mjs_get_int(mjs, mjs_pop_val(&mjs->call_stack));
  return_address = mjs_get_int(mjs, mjs_pop_val(&mjs->call_stack));
  mjs->vals.this_obj = mjs_pop_val(&mjs->call_stack);


  while (mjs_stack_size(&mjs->scopes) > scope_index) {
    mjs_pop_val(&mjs->scopes);
  }


  while (mjs_stack_size(&mjs->loop_addresses) > loop_addr_index) {
    mjs_pop_val(&mjs->loop_addresses);
  }


  mjs->stack.len = retval_stack_idx * sizeof(mjs_val_t);


  return return_address;
}

static mjs_val_t mjs_find_scope(struct mjs *mjs, mjs_val_t key) {
  size_t num_scopes = mjs_stack_size(&mjs->scopes);
  while (num_scopes > 0) {
    mjs_val_t scope = *vptr(&mjs->scopes, num_scopes - 1);
    num_scopes--;
    if (mjs_get_own_property_v(mjs, scope, key) != 
                                                  __null
                                                      ) return scope;
  }
  mjs_set_errorf(mjs, MJS_REFERENCE_ERROR, "[%s] is not defined",
                 mjs_get_cstring(mjs, &key));
  return ((uint64_t)(1) << 63 | (uint64_t) 0x7ff0 << 48 | (uint64_t)(3) << 48);
}

mjs_val_t mjs_get_this(struct mjs *mjs) {
  return mjs->vals.this_obj;
}

static double do_arith_op(double da, double db, int op, bool *resnan) {
  *resnan = false;

  if (isnan(da) || isnan(db)) {
    *resnan = true;
    return 0;
  }

  switch (op) {
    case TOK_MINUS: return da - db;
    case TOK_PLUS: return da + db;
    case TOK_MUL: return da * db;
    case TOK_DIV:
      if (db != 0) {
        return da / db;
      } else {

        *resnan = true;
        return 0;
      }
    case TOK_REM:




      db = (int) db;
      if (db != 0) {
        bool neg = false;
        if (da < 0) {
          neg = true;
          da = -da;
        }
        if (db < 0) {
          db = -db;
        }
        da = (double) ((int64_t) da % (int64_t) db);
        if (neg) {
          da = -da;
        }
        return da;
      } else {
        *resnan = true;
        return 0;
      }
    case TOK_AND: return (double) ((int64_t) da & (int64_t) db);
    case TOK_OR: return (double) ((int64_t) da | (int64_t) db);
    case TOK_XOR: return (double) ((int64_t) da ^ (int64_t) db);
    case TOK_LSHIFT: return (double) ((int64_t) da << (int64_t) db);
    case TOK_RSHIFT: return (double) ((int64_t) da >> (int64_t) db);
    case TOK_URSHIFT: return (double) ((uint32_t) da >> (uint32_t) db);
  }

  *resnan = true;
  return 0;
}

static void set_no_autoconversion_error(struct mjs *mjs) {
  mjs_prepend_errorf(mjs, MJS_TYPE_ERROR,
                     "implicit type conversion is prohibited");
}

static mjs_val_t do_op(struct mjs *mjs, mjs_val_t a, mjs_val_t b, int op) {
  mjs_val_t ret = ((uint64_t)(1) << 63 | (uint64_t) 0x7ff0 << 48 | (uint64_t)(3) << 48);
  bool resnan = false;
  if ((mjs_is_foreign(a) || mjs_is_number(a)) &&
      (mjs_is_foreign(b) || mjs_is_number(b))) {
    int is_result_ptr = 0;
    double da, db, result;

    if (mjs_is_foreign(a) && mjs_is_foreign(b)) {

      if (op != TOK_MINUS) {
        mjs_prepend_errorf(mjs, MJS_TYPE_ERROR, "invalid operands");
      }
    } else if (mjs_is_foreign(a) || mjs_is_foreign(b)) {




      if (op != TOK_MINUS && op != TOK_PLUS) {
        mjs_prepend_errorf(mjs, MJS_TYPE_ERROR, "invalid operands");
      }
      is_result_ptr = 1;
    }
    da = mjs_is_number(a) ? mjs_get_double(mjs, a)
                          : (double) (uintptr_t) mjs_get_ptr(mjs, a);
    db = mjs_is_number(b) ? mjs_get_double(mjs, b)
                          : (double) (uintptr_t) mjs_get_ptr(mjs, b);
    result = do_arith_op(da, db, op, &resnan);
    if (resnan) {
      ret = ((uint64_t)(1) << 63 | (uint64_t) 0x7ff0 << 48 | (uint64_t)(5) << 48);
    } else {




      ret = is_result_ptr ? mjs_mk_foreign(mjs, (void *) (uintptr_t) result)
                          : mjs_mk_number(mjs, result);
    }
  } else if (mjs_is_string(a) && mjs_is_string(b) && (op == TOK_PLUS)) {
    ret = s_concat(mjs, a, b);
  } else {
    set_no_autoconversion_error(mjs);
  }
  return ret;
}

static void op_assign(struct mjs *mjs, int op) {
  mjs_val_t val = mjs_pop(mjs);
  mjs_val_t obj = mjs_pop(mjs);
  mjs_val_t key = mjs_pop(mjs);
  if (mjs_is_object(obj) && mjs_is_string(key)) {
    mjs_val_t v = mjs_get_v(mjs, obj, key);
    mjs_set_v(mjs, obj, key, do_op(mjs, v, val, op));
    mjs_push(mjs, v);
  } else {
    mjs_set_errorf(mjs, MJS_TYPE_ERROR, "invalid operand");
  }
}

static int check_equal(struct mjs *mjs, mjs_val_t a, mjs_val_t b) {
  int ret = 0;
  if (a == ((uint64_t)(1) << 63 | (uint64_t) 0x7ff0 << 48 | (uint64_t)(5) << 48) && b == ((uint64_t)(1) << 63 | (uint64_t) 0x7ff0 << 48 | (uint64_t)(5) << 48)) {
    ret = 0;
  } else if (a == b) {
    ret = 1;
  } else if (mjs_is_number(a) && mjs_is_number(b)) {




    ret = 0;
  } else if (mjs_is_string(a) && mjs_is_string(b)) {
    ret = s_cmp(mjs, a, b) == 0;
  } else if (mjs_is_foreign(a) && b == ((uint64_t)(1) << 63 | (uint64_t) 0x7ff0 << 48 | (uint64_t)(15) << 48)) {
    ret = mjs_get_ptr(mjs, a) == 
                                __null
                                    ;
  } else if (a == ((uint64_t)(1) << 63 | (uint64_t) 0x7ff0 << 48 | (uint64_t)(15) << 48) && mjs_is_foreign(b)) {
    ret = mjs_get_ptr(mjs, b) == 
                                __null
                                    ;
  } else {
    ret = 0;
  }
  return ret;
}

static void exec_expr(struct mjs *mjs, int op) {
  switch (op) {
    case TOK_DOT:
      break;
    case TOK_MINUS:
    case TOK_PLUS:
    case TOK_MUL:
    case TOK_DIV:
    case TOK_REM:
    case TOK_XOR:
    case TOK_AND:
    case TOK_OR:
    case TOK_LSHIFT:
    case TOK_RSHIFT:
    case TOK_URSHIFT: {
      mjs_val_t b = mjs_pop(mjs);
      mjs_val_t a = mjs_pop(mjs);
      mjs_push(mjs, do_op(mjs, a, b, op));
      break;
    }
    case TOK_UNARY_MINUS: {
      double a = mjs_get_double(mjs, mjs_pop(mjs));
      mjs_push(mjs, mjs_mk_number(mjs, -a));
      break;
    }
    case TOK_NOT: {
      mjs_val_t val = mjs_pop(mjs);
      mjs_push(mjs, mjs_mk_boolean(mjs, !mjs_is_truthy(mjs, val)));
      break;
    }
    case TOK_TILDA: {
      double a = mjs_get_double(mjs, mjs_pop(mjs));
      mjs_push(mjs, mjs_mk_number(mjs, (double) (~(int64_t) a)));
      break;
    }
    case TOK_UNARY_PLUS:
      break;
    case TOK_EQ:
      mjs_set_errorf(mjs, MJS_NOT_IMPLEMENTED_ERROR, "Use ===, not ==");
      break;
    case TOK_NE:
      mjs_set_errorf(mjs, MJS_NOT_IMPLEMENTED_ERROR, "Use !==, not !=");
      break;
    case TOK_EQ_EQ: {
      mjs_val_t a = mjs_pop(mjs);
      mjs_val_t b = mjs_pop(mjs);
      mjs_push(mjs, mjs_mk_boolean(mjs, check_equal(mjs, a, b)));
      break;
    }
    case TOK_NE_NE: {
      mjs_val_t a = mjs_pop(mjs);
      mjs_val_t b = mjs_pop(mjs);
      mjs_push(mjs, mjs_mk_boolean(mjs, !check_equal(mjs, a, b)));
      break;
    }
    case TOK_LT: {
      double b = mjs_get_double(mjs, mjs_pop(mjs));
      double a = mjs_get_double(mjs, mjs_pop(mjs));
      mjs_push(mjs, mjs_mk_boolean(mjs, a < b));
      break;
    }
    case TOK_GT: {
      double b = mjs_get_double(mjs, mjs_pop(mjs));
      double a = mjs_get_double(mjs, mjs_pop(mjs));
      mjs_push(mjs, mjs_mk_boolean(mjs, a > b));
      break;
    }
    case TOK_LE: {
      double b = mjs_get_double(mjs, mjs_pop(mjs));
      double a = mjs_get_double(mjs, mjs_pop(mjs));
      mjs_push(mjs, mjs_mk_boolean(mjs, a <= b));
      break;
    }
    case TOK_GE: {
      double b = mjs_get_double(mjs, mjs_pop(mjs));
      double a = mjs_get_double(mjs, mjs_pop(mjs));
      mjs_push(mjs, mjs_mk_boolean(mjs, a >= b));
      break;
    }
    case TOK_ASSIGN: {
      mjs_val_t val = mjs_pop(mjs);
      mjs_val_t obj = mjs_pop(mjs);
      mjs_val_t key = mjs_pop(mjs);
      if (mjs_is_object(obj)) {
        mjs_set_v(mjs, obj, key, val);
      } else if (mjs_is_foreign(obj)) {






        int ikey = mjs_get_int(mjs, key);
        int ival = mjs_get_int(mjs, val);

        if (!mjs_is_number(key)) {
          mjs_prepend_errorf(mjs, MJS_TYPE_ERROR, "index must be a number");
          val = ((uint64_t)(1) << 63 | (uint64_t) 0x7ff0 << 48 | (uint64_t)(3) << 48);
        } else if (!mjs_is_number(val) || ival < 0 || ival > 0xff) {
          mjs_prepend_errorf(mjs, MJS_TYPE_ERROR,
                             "only number 0 .. 255 can be assigned");
          val = ((uint64_t)(1) << 63 | (uint64_t) 0x7ff0 << 48 | (uint64_t)(3) << 48);
        } else {
          uint8_t *ptr = (uint8_t *) mjs_get_ptr(mjs, obj);
          *(ptr + ikey) = (uint8_t) ival;
        }
      } else {
        mjs_prepend_errorf(mjs, MJS_TYPE_ERROR, "unsupported object type");
      }
      mjs_push(mjs, val);
      break;
    }
    case TOK_POSTFIX_PLUS: {
      mjs_val_t obj = mjs_pop(mjs);
      mjs_val_t key = mjs_pop(mjs);
      if (mjs_is_object(obj) && mjs_is_string(key)) {
        mjs_val_t v = mjs_get_v(mjs, obj, key);
        mjs_val_t v1 = do_op(mjs, v, mjs_mk_number(mjs, 1), TOK_PLUS);
        mjs_set_v(mjs, obj, key, v1);
        mjs_push(mjs, v);
      } else {
        mjs_set_errorf(mjs, MJS_TYPE_ERROR, "invalid operand for ++");
      }
      break;
    }
    case TOK_POSTFIX_MINUS: {
      mjs_val_t obj = mjs_pop(mjs);
      mjs_val_t key = mjs_pop(mjs);
      if (mjs_is_object(obj) && mjs_is_string(key)) {
        mjs_val_t v = mjs_get_v(mjs, obj, key);
        mjs_val_t v1 = do_op(mjs, v, mjs_mk_number(mjs, 1), TOK_MINUS);
        mjs_set_v(mjs, obj, key, v1);
        mjs_push(mjs, v);
      } else {
        mjs_set_errorf(mjs, MJS_TYPE_ERROR, "invalid operand for --");
      }
      break;
    }
    case TOK_MINUS_MINUS: {
      mjs_val_t obj = mjs_pop(mjs);
      mjs_val_t key = mjs_pop(mjs);
      if (mjs_is_object(obj) && mjs_is_string(key)) {
        mjs_val_t v = mjs_get_v(mjs, obj, key);
        v = do_op(mjs, v, mjs_mk_number(mjs, 1), TOK_MINUS);
        mjs_set_v(mjs, obj, key, v);
        mjs_push(mjs, v);
      } else {
        mjs_set_errorf(mjs, MJS_TYPE_ERROR, "invalid operand for --");
      }
      break;
    }
    case TOK_PLUS_PLUS: {
      mjs_val_t obj = mjs_pop(mjs);
      mjs_val_t key = mjs_pop(mjs);
      if (mjs_is_object(obj) && mjs_is_string(key)) {
        mjs_val_t v = mjs_get_v(mjs, obj, key);
        v = do_op(mjs, v, mjs_mk_number(mjs, 1), TOK_PLUS);
        mjs_set_v(mjs, obj, key, v);
        mjs_push(mjs, v);
      } else {
        mjs_set_errorf(mjs, MJS_TYPE_ERROR, "invalid operand for ++");
      }
      break;
    }







    case TOK_MINUS_ASSIGN: op_assign(mjs, TOK_MINUS); break;
    case TOK_PLUS_ASSIGN: op_assign(mjs, TOK_PLUS); break;
    case TOK_MUL_ASSIGN: op_assign(mjs, TOK_MUL); break;
    case TOK_DIV_ASSIGN: op_assign(mjs, TOK_DIV); break;
    case TOK_REM_ASSIGN: op_assign(mjs, TOK_REM); break;
    case TOK_AND_ASSIGN: op_assign(mjs, TOK_AND); break;
    case TOK_OR_ASSIGN: op_assign(mjs, TOK_OR); break;
    case TOK_XOR_ASSIGN: op_assign(mjs, TOK_XOR); break;
    case TOK_LSHIFT_ASSIGN: op_assign(mjs, TOK_LSHIFT); break;
    case TOK_RSHIFT_ASSIGN: op_assign(mjs, TOK_RSHIFT); break;
    case TOK_URSHIFT_ASSIGN: op_assign(mjs, TOK_URSHIFT); break;
    case TOK_COMMA: break;

    case TOK_KEYWORD_TYPEOF:
      mjs_push(mjs, mjs_mk_string(mjs, mjs_typeof(mjs_pop(mjs)), ~0, 1));
      break;
    default:
      do { if (cs_log_print_prefix(LL_ERROR, "mjs.c", 4621)) { cs_log_printf ("Unknown expr: %d", op); } } while (0);
      break;
  }
}

static int getprop_builtin_string(struct mjs *mjs, mjs_val_t val,
                                  const char *name, size_t name_len,
                                  mjs_val_t *res) {
  int isnum = 0;
  int idx = cstr_to_ulong(name, name_len, &isnum);

  if (strcmp(name, "length") == 0) {
    size_t val_len;
    mjs_get_string(mjs, &val, &val_len);
    *res = mjs_mk_number(mjs, (double) val_len);
    return 1;
  } else if (strcmp(name, "at") == 0 || strcmp(name, "charCodeAt") == 0) {
    *res = mjs_mk_foreign_func(mjs, (mjs_func_ptr_t) mjs_string_char_code_at);
    return 1;
  } else if (strcmp(name, "indexOf") == 0) {
    *res = mjs_mk_foreign_func(mjs, (mjs_func_ptr_t) mjs_string_index_of);
    return 1;
  } else if (strcmp(name, "slice") == 0) {
    *res = mjs_mk_foreign_func(mjs, (mjs_func_ptr_t) mjs_string_slice);
    return 1;
  } else if (isnum) {




    size_t val_len;
    const char *str = mjs_get_string(mjs, &val, &val_len);
    if (idx >= 0 && idx < (int) val_len) {
      *res = mjs_mk_string(mjs, str + idx, 1, 1);
    } else {
      *res = ((uint64_t)(1) << 63 | (uint64_t) 0x7ff0 << 48 | (uint64_t)(3) << 48);
    }
    return 1;
  }
  return 0;
}

static int getprop_builtin_array(struct mjs *mjs, mjs_val_t val,
                                 const char *name, size_t name_len,
                                 mjs_val_t *res) {
  if (strcmp(name, "splice") == 0) {
    *res = mjs_mk_foreign_func(mjs, (mjs_func_ptr_t) mjs_array_splice);
    return 1;
  } else if (strcmp(name, "push") == 0) {
    *res = mjs_mk_foreign_func(mjs, (mjs_func_ptr_t) mjs_array_push_internal);
    return 1;
  } else if (strcmp(name, "length") == 0) {
    *res = mjs_mk_number(mjs, mjs_array_length(mjs, val));
    return 1;
  }

  (void) name_len;
  return 0;
}

static int getprop_builtin_foreign(struct mjs *mjs, mjs_val_t val,
                                   const char *name, size_t name_len,
                                   mjs_val_t *res) {
  int isnum = 0;
  int idx = cstr_to_ulong(name, name_len, &isnum);

  if (!isnum) {
    mjs_prepend_errorf(mjs, MJS_TYPE_ERROR, "index must be a number");
  } else {
    uint8_t *ptr = (uint8_t *) mjs_get_ptr(mjs, val);
    *res = mjs_mk_number(mjs, *(ptr + idx));
  }
  return 1;
}

static void mjs_apply_(struct mjs *mjs) {
  mjs_val_t res = ((uint64_t)(1) << 63 | (uint64_t) 0x7ff0 << 48 | (uint64_t)(3) << 48), *args = 
                                        __null
                                            ;
  mjs_val_t func = mjs->vals.this_obj, v = mjs_arg(mjs, 1);
  int i, nargs = 0;
  if (mjs_is_array(v)) {
    nargs = mjs_array_length(mjs, v);
    args = (mjs_val_t*)calloc(nargs, sizeof(args[0]));
    for (i = 0; i < nargs; i++) args[i] = mjs_array_get(mjs, v, i);
  }
  mjs_apply(mjs, &res, func, mjs_arg(mjs, 0), nargs, args);
  free(args);
  mjs_return(mjs, res);
}

static int getprop_builtin(struct mjs *mjs, mjs_val_t val, mjs_val_t name,
                           mjs_val_t *res) {
  size_t n;
  char *s = 
           __null
               ;
  int need_free = 0;
  int handled = 0;

  mjs_err_t err = mjs_to_string(mjs, &name, &s, &n, &need_free);

  if (err == MJS_OK) {
    if (mjs_is_string(val)) {
      handled = getprop_builtin_string(mjs, val, s, n, res);
    } else if (s != 
                   __null 
                        && n == 5 && strncmp(s, "apply", n) == 0) {
      *res = mjs_mk_foreign_func(mjs, (mjs_func_ptr_t) mjs_apply_);
      handled = 1;
    } else if (mjs_is_array(val)) {
      handled = getprop_builtin_array(mjs, val, s, n, res);
    } else if (mjs_is_foreign(val)) {
      handled = getprop_builtin_foreign(mjs, val, s, n, res);
    }
  }

  if (need_free) {
    free(s);
    s = 
       __null
           ;
  }

  return handled;
}

static mjs_err_t mjs_execute(struct mjs *mjs, size_t off, mjs_val_t *res) {
  size_t i;
  uint8_t prev_opcode = OP_MAX;
  uint8_t opcode = OP_MAX;




  int stack_len = mjs->stack.len;
  int call_stack_len = mjs->call_stack.len;
  int arg_stack_len = mjs->arg_stack.len;
  int scopes_len = mjs->scopes.len;
  int loop_addresses_len = mjs->loop_addresses.len;
  size_t start_off = off;
  const uint8_t *code;

  struct mjs_bcode_part bp = *mjs_bcode_part_get_by_offset(mjs, off);

  mjs_set_errorf(mjs, MJS_OK, 
                             __null
                                 );
  free(mjs->stack_trace);
  mjs->stack_trace = 
                    __null
                        ;

  off -= bp.start_idx;

  for (i = off; i < bp.data.len; i++) {
    mjs->cur_bcode_offset = i;

    if (mjs->need_gc) {
      if (maybe_gc(mjs)) {
        mjs->need_gc = 0;
      }
    }




    code = (const uint8_t *) bp.data.p;
    mjs_disasm_single(code, i);
    prev_opcode = opcode;
    opcode = code[i];
    switch (opcode) {
      case OP_BCODE_HEADER: {
        mjs_header_item_t bcode_offset;
        memcpy(&bcode_offset,
               code + i + 1 +
                   sizeof(mjs_header_item_t) * MJS_HDR_ITEM_BCODE_OFFSET,
               sizeof(bcode_offset));
        i += bcode_offset;
      } break;
      case OP_PUSH_NULL:
        mjs_push(mjs, mjs_mk_null());
        break;
      case OP_PUSH_UNDEF:
        mjs_push(mjs, mjs_mk_undefined());
        break;
      case OP_PUSH_FALSE:
        mjs_push(mjs, mjs_mk_boolean(mjs, 0));
        break;
      case OP_PUSH_TRUE:
        mjs_push(mjs, mjs_mk_boolean(mjs, 1));
        break;
      case OP_PUSH_OBJ:
        mjs_push(mjs, mjs_mk_object(mjs));
        break;
      case OP_PUSH_ARRAY:
        mjs_push(mjs, mjs_mk_array(mjs));
        break;
      case OP_PUSH_FUNC: {
        int llen, n = cs_varint_decode_unsafe(&code[i + 1], &llen);
        mjs_push(mjs, mjs_mk_function(mjs, bp.start_idx + i - n));
        i += llen;
        break;
      }
      case OP_PUSH_THIS:
        mjs_push(mjs, mjs->vals.this_obj);
        break;
      case OP_JMP: {
        int llen, n = cs_varint_decode_unsafe(&code[i + 1], &llen);
        i += n + llen;
        break;
      }
      case OP_JMP_FALSE: {
        int llen, n = cs_varint_decode_unsafe(&code[i + 1], &llen);
        i += llen;
        if (!mjs_is_truthy(mjs, mjs_pop(mjs))) {
          mjs_push(mjs, ((uint64_t)(1) << 63 | (uint64_t) 0x7ff0 << 48 | (uint64_t)(3) << 48));
          i += n;
        }
        break;
      }





      case OP_JMP_NEUTRAL_TRUE: {
        int llen, n = cs_varint_decode_unsafe(&code[i + 1], &llen);
        i += llen;
        if (mjs_is_truthy(mjs, vtop(&mjs->stack))) {
          i += n;
        }
        break;
      }
      case OP_JMP_NEUTRAL_FALSE: {
        int llen, n = cs_varint_decode_unsafe(&code[i + 1], &llen);
        i += llen;
        if (!mjs_is_truthy(mjs, vtop(&mjs->stack))) {
          i += n;
        }
        break;
      }
      case OP_FIND_SCOPE: {
        mjs_val_t key = vtop(&mjs->stack);
        mjs_push(mjs, mjs_find_scope(mjs, key));
        break;
      }
      case OP_CREATE: {
        mjs_val_t obj = mjs_pop(mjs);
        mjs_val_t key = mjs_pop(mjs);
        if (mjs_get_own_property_v(mjs, obj, key) == 
                                                    __null
                                                        ) {
          mjs_set_v(mjs, obj, key, ((uint64_t)(1) << 63 | (uint64_t) 0x7ff0 << 48 | (uint64_t)(3) << 48));
        }
        break;
      }
      case OP_APPEND: {
        mjs_val_t val = mjs_pop(mjs);
        mjs_val_t arr = mjs_pop(mjs);
        mjs_err_t err = mjs_array_push(mjs, arr, val);
        if (err != MJS_OK) {
          mjs_set_errorf(mjs, MJS_TYPE_ERROR, "append to non-array");
        }
        break;
      }
      case OP_GET: {
        mjs_val_t obj = mjs_pop(mjs);
        mjs_val_t key = mjs_pop(mjs);
        mjs_val_t val = ((uint64_t)(1) << 63 | (uint64_t) 0x7ff0 << 48 | (uint64_t)(3) << 48);

        if (!getprop_builtin(mjs, obj, key, &val)) {
          if (mjs_is_object(obj)) {
            val = mjs_get_v_proto(mjs, obj, key);
          } else {
            mjs_prepend_errorf(mjs, MJS_TYPE_ERROR, "type error");
          }
        }

        mjs_push(mjs, val);
        if (prev_opcode != OP_FIND_SCOPE) {




          mjs->vals.last_getprop_obj = obj;
        } else {




          mjs->vals.last_getprop_obj = ((uint64_t)(1) << 63 | (uint64_t) 0x7ff0 << 48 | (uint64_t)(3) << 48);
        }
        break;
      }
      case OP_DEL_SCOPE:
        if (mjs->scopes.len <= 1) {
          mjs_set_errorf(mjs, MJS_INTERNAL_ERROR, "scopes underflow");
        } else {
          mjs_pop_val(&mjs->scopes);
        }
        break;
      case OP_NEW_SCOPE:
        push_mjs_val(&mjs->scopes, mjs_mk_object(mjs));
        break;
      case OP_PUSH_SCOPE:
        
       (static_cast <bool> (
       mjs_stack_size(&mjs->scopes) > 0
       ) ? void (0) : __assert_fail (
       "mjs_stack_size(&mjs->scopes) > 0"
       , "mjs.c", 4913, __extension__ __PRETTY_FUNCTION__))
                                               ;
        mjs_push(mjs, vtop(&mjs->scopes));
        break;
      case OP_PUSH_STR: {
        int llen, n = cs_varint_decode_unsafe(&code[i + 1], &llen);
        mjs_push(mjs, mjs_mk_string(mjs, (char *) code + i + 1 + llen, n, 1));
        i += llen + n;
        break;
      }
      case OP_PUSH_INT: {
        int llen;
        int64_t n = cs_varint_decode_unsafe(&code[i + 1], &llen);
        mjs_push(mjs, mjs_mk_number(mjs, (double) n));
        i += llen;
        break;
      }
      case OP_PUSH_DBL: {
        int llen, n = cs_varint_decode_unsafe(&code[i + 1], &llen);
        mjs_push(mjs, mjs_mk_number(
                          mjs, strtod((char *) code + i + 1 + llen, 
                                                                   __null
                                                                       )));
        i += llen + n;
        break;
      }
      case OP_FOR_IN_NEXT: {







        mjs_val_t *iterator = vptr(&mjs->stack, -1);
        mjs_val_t obj = *vptr(&mjs->stack, -2);
        if (mjs_is_object(obj)) {
          mjs_val_t var_name = *vptr(&mjs->stack, -3);
          mjs_val_t key = mjs_next(mjs, obj, iterator);
          if (key != ((uint64_t)(1) << 63 | (uint64_t) 0x7ff0 << 48 | (uint64_t)(3) << 48)) {
            mjs_val_t scope = mjs_find_scope(mjs, var_name);
            mjs_set_v(mjs, scope, var_name, key);
          }
        } else {
          mjs_set_errorf(mjs, MJS_TYPE_ERROR,
                         "can't iterate over non-object value");
        }
        break;
      }
      case OP_RETURN: {




        size_t off_ret = call_stack_restore_frame(mjs);
        if (off_ret != ((size_t) 0x7fffffff)) {
          bp = *mjs_bcode_part_get_by_offset(mjs, off_ret);
          code = (const uint8_t *) bp.data.p;
          i = off_ret - bp.start_idx;
          do { if (cs_log_print_prefix(LL_VERBOSE_DEBUG, "mjs.c", 4969)) { cs_log_printf ("RETURNING TO %d", (int) off_ret + 1); } } while (0);
        } else {
          goto clean;
        }

        break;
      }
      case OP_ARGS: {





        if (prev_opcode != OP_GET) {
          mjs->vals.last_getprop_obj = ((uint64_t)(1) << 63 | (uint64_t) 0x7ff0 << 48 | (uint64_t)(3) << 48);
        }





        push_mjs_val(&mjs->arg_stack, mjs->vals.last_getprop_obj);




        push_mjs_val(&mjs->arg_stack,
                     mjs_mk_number(mjs, (double) mjs_stack_size(&mjs->stack)));
        break;
      }
      case OP_CALL: {


        int func_pos;
        mjs_val_t *func;
        mjs_val_t retval_stack_idx = vtop(&mjs->arg_stack);
        func_pos = mjs_get_int(mjs, retval_stack_idx) - 1;
        func = vptr(&mjs->stack, func_pos);


        mjs_pop_val(&mjs->arg_stack);

        if (mjs_is_function(*func)) {
          size_t off_call;
          call_stack_push_frame(mjs, bp.start_idx + i, retval_stack_idx);





          off_call = mjs_get_func_addr(*func) - 1;
          bp = *mjs_bcode_part_get_by_offset(mjs, off_call);
          code = (const uint8_t *) bp.data.p;
          i = off_call - bp.start_idx;

          *func = ((uint64_t)(1) << 63 | (uint64_t) 0x7ff0 << 48 | (uint64_t)(3) << 48);

        } else if (mjs_is_string(*func) || mjs_is_ffi_sig(*func)) {


          call_stack_push_frame(mjs, bp.start_idx + i, retval_stack_idx);


          mjs_ffi_call2(mjs);

          call_stack_restore_frame(mjs);
        } else if (mjs_is_foreign(*func)) {


          call_stack_push_frame(mjs, bp.start_idx + i, retval_stack_idx);


          ((void (*) (struct mjs *)) mjs_get_ptr(mjs, *func))(mjs);

          call_stack_restore_frame(mjs);
        } else {
          mjs_set_errorf(mjs, MJS_TYPE_ERROR, "calling non-callable");
        }
        break;
      }
      case OP_SET_ARG: {
        int llen1, llen2, n,
            arg_no = cs_varint_decode_unsafe(&code[i + 1], &llen1);
        mjs_val_t obj, key, v;
        n = cs_varint_decode_unsafe(&code[i + llen1 + 1], &llen2);
        key = mjs_mk_string(mjs, (char *) code + i + 1 + llen1 + llen2, n, 1);
        obj = vtop(&mjs->scopes);
        v = mjs_arg(mjs, arg_no);
        mjs_set_v(mjs, obj, key, v);
        i += llen1 + llen2 + n;
        break;
      }
      case OP_SETRETVAL: {
        if (mjs_stack_size(&mjs->call_stack) < CALL_STACK_FRAME_ITEMS_CNT) {
          mjs_set_errorf(mjs, MJS_INTERNAL_ERROR, "cannot return");
        } else {
          size_t retval_pos = mjs_get_int(
              mjs, *vptr(&mjs->call_stack,
                         -1 - CALL_STACK_FRAME_ITEM_RETVAL_STACK_IDX));
          *vptr(&mjs->stack, retval_pos - 1) = mjs_pop(mjs);
        }


        break;
      }
      case OP_EXPR: {
        int op = code[i + 1];
        exec_expr(mjs, op);
        i++;
        break;
      }
      case OP_DROP: {
        mjs_pop(mjs);
        break;
      }
      case OP_DUP: {
        mjs_push(mjs, vtop(&mjs->stack));
        break;
      }
      case OP_SWAP: {
        mjs_val_t a = mjs_pop(mjs);
        mjs_val_t b = mjs_pop(mjs);
        mjs_push(mjs, a);
        mjs_push(mjs, b);
        break;
      }
      case OP_LOOP: {
        int l1, l2, off = cs_varint_decode_unsafe(&code[i + 1], &l1);

        push_mjs_val(&mjs->loop_addresses,
                     mjs_mk_number(mjs, (double) mjs_stack_size(&mjs->scopes)));


        push_mjs_val(
            &mjs->loop_addresses,
            mjs_mk_number(mjs, (double) (i + 1 + l1 + off)));
        off = cs_varint_decode_unsafe(&code[i + 1 + l1], &l2);


        push_mjs_val(
            &mjs->loop_addresses,
            mjs_mk_number(mjs, (double) (i + 1 + l1 + l2 + off)));
        i += l1 + l2;
        break;
      }
      case OP_CONTINUE: {
        if (mjs_stack_size(&mjs->loop_addresses) >= 3) {
          size_t scopes_len = mjs_get_int(mjs, *vptr(&mjs->loop_addresses, -3));
          
         (static_cast <bool> (
         mjs_stack_size(&mjs->scopes) >= scopes_len
         ) ? void (0) : __assert_fail (
         "mjs_stack_size(&mjs->scopes) >= scopes_len"
         , "mjs.c", 5117, __extension__ __PRETTY_FUNCTION__))
                                                           ;
          mjs->scopes.len = scopes_len * sizeof(mjs_val_t);


          i = mjs_get_int(mjs, vtop(&mjs->loop_addresses)) - 1;
        } else {
          mjs_set_errorf(mjs, MJS_SYNTAX_ERROR, "misplaced 'continue'");
        }
      } break;
      case OP_BREAK: {
        if (mjs_stack_size(&mjs->loop_addresses) >= 3) {
          size_t scopes_len;

          mjs_pop_val(&mjs->loop_addresses);


          i = mjs_get_int(mjs, mjs_pop_val(&mjs->loop_addresses)) - 1;


          scopes_len = mjs_get_int(mjs, mjs_pop_val(&mjs->loop_addresses));
          
         (static_cast <bool> (
         mjs_stack_size(&mjs->scopes) >= scopes_len
         ) ? void (0) : __assert_fail (
         "mjs_stack_size(&mjs->scopes) >= scopes_len"
         , "mjs.c", 5137, __extension__ __PRETTY_FUNCTION__))
                                                           ;
          mjs->scopes.len = scopes_len * sizeof(mjs_val_t);

          do { if (cs_log_print_prefix(LL_VERBOSE_DEBUG, "mjs.c", 5140)) { cs_log_printf ("BREAKING TO %d", (int) i + 1); } } while (0);
        } else {
          mjs_set_errorf(mjs, MJS_SYNTAX_ERROR, "misplaced 'break'");
        }
      } break;
      case OP_NOP:
        break;
      case OP_EXIT:
        i = bp.data.len;
        break;
      default:

        mjs_dump(mjs, 1);

        mjs_set_errorf(mjs, MJS_INTERNAL_ERROR, "Unknown opcode: %d, off %d+%d",
                       (int) opcode, (int) bp.start_idx, (int) i);
        i = bp.data.len;
        break;
    }
    if (mjs->error != MJS_OK) {
      mjs_gen_stack_trace(mjs, bp.start_idx + i - 1 );


      mjs->stack.len = stack_len;
      mjs->call_stack.len = call_stack_len;
      mjs->arg_stack.len = arg_stack_len;
      mjs->scopes.len = scopes_len;
      mjs->loop_addresses.len = loop_addresses_len;


      mjs_push(mjs, ((uint64_t)(1) << 63 | (uint64_t) 0x7ff0 << 48 | (uint64_t)(3) << 48));
      break;
    }
  }

clean:

  mjs_bcode_part_get_by_offset(mjs, start_off)->exec_res = mjs->error;

  *res = mjs_pop(mjs);
  return mjs->error;
}

static mjs_err_t mjs_exec_internal(struct mjs *mjs, const char *path,
                                        const char *src, int generate_jsc,
                                        mjs_val_t *res) {
  size_t off = mjs->bcode_len;
  mjs_val_t r = ((uint64_t)(1) << 63 | (uint64_t) 0x7ff0 << 48 | (uint64_t)(3) << 48);
  mjs->error = mjs_parse(path, src, mjs);
  if (cs_log_level >= LL_VERBOSE_DEBUG) mjs_dump(mjs, 1);
  if (generate_jsc == -1) generate_jsc = mjs->generate_jsc;
  if (mjs->error == MJS_OK) {
    return mjs->error;
    (void) generate_jsc;


    mjs_execute(mjs, off, &r);
  }
  if (res != 
            __null
                ) *res = r;
  return mjs->error;
}

mjs_err_t mjs_exec(struct mjs *mjs, const char *src, mjs_val_t *res) {
  return mjs_exec_internal(mjs, "<stdin>", src, 0 , res);
}

mjs_err_t mjs_exec_file(struct mjs *mjs, const char *path, mjs_val_t *res) {
  mjs_err_t error = MJS_FILE_READ_ERROR;
  mjs_val_t r = ((uint64_t)(1) << 63 | (uint64_t) 0x7ff0 << 48 | (uint64_t)(3) << 48);
  size_t size;
  char *source_code = cs_read_file(path, &size);

  if (source_code == 
                    __null
                        ) {
    error = MJS_FILE_READ_ERROR;
    mjs_prepend_errorf(mjs, error, "failed to read file \"%s\"", path);
    goto clean;
  }

  r = ((uint64_t)(1) << 63 | (uint64_t) 0x7ff0 << 48 | (uint64_t)(3) << 48);
  error = mjs_exec_internal(mjs, path, source_code, -1, &r);
  free(source_code);

clean:
  if (res != 
            __null
                ) *res = r;
  return error;
}

mjs_err_t mjs_call(struct mjs *mjs, mjs_val_t *res, mjs_val_t func,
                   mjs_val_t this_val, int nargs, ...) {
  va_list ap;
  int i;
  mjs_err_t ret;
  mjs_val_t *args = (mjs_val_t*)calloc(nargs, sizeof(mjs_val_t));
  
 __builtin_va_start(
 ap
 ,
 nargs
 )
                    ;
  for (i = 0; i < nargs; i++) {
    args[i] = 
             __builtin_va_arg(
             ap
             ,
             mjs_val_t
             )
                                  ;
  }
  
 __builtin_va_end(
 ap
 )
           ;

  ret = mjs_apply(mjs, res, func, this_val, nargs, args);

  free(args);
  return ret;
}

mjs_err_t mjs_apply(struct mjs *mjs, mjs_val_t *res, mjs_val_t func,
                    mjs_val_t this_val, int nargs, mjs_val_t *args) {
  mjs_val_t r, prev_this_val, retval_stack_idx, *resp;
  int i;

  if (!mjs_is_function(func) && !mjs_is_foreign(func) &&
      !mjs_is_ffi_sig(func)) {
    return mjs_set_errorf(mjs, MJS_TYPE_ERROR, "calling non-callable");
  }

  do { if (cs_log_print_prefix(LL_VERBOSE_DEBUG, "mjs.c", 5319)) { cs_log_printf ("applying func %d", (int) mjs_get_func_addr(func)); } } while (0);

  prev_this_val = mjs->vals.this_obj;


  mjs_push(mjs, func);
  resp = vptr(&mjs->stack, -1);


  retval_stack_idx = mjs_mk_number(mjs, (double) mjs_stack_size(&mjs->stack));


  for (i = 0; i < nargs; i++) {
    mjs_push(mjs, args[i]);
  }


  push_mjs_val(&mjs->arg_stack, this_val);


  call_stack_push_frame(mjs, ((size_t) 0x7fffffff), retval_stack_idx);

  if (mjs_is_foreign(func)) {
    ((void (*) (struct mjs *)) mjs_get_ptr(mjs, func))(mjs);
    if (res != 
              __null
                  ) *res = *resp;
  } else if (mjs_is_ffi_sig(func)) {
    mjs_ffi_call2(mjs);
    if (res != 
              __null
                  ) *res = *resp;
  } else {
    size_t addr = mjs_get_func_addr(func);
    mjs_execute(mjs, addr, &r);
    if (res != 
              __null
                  ) *res = r;
  }





  if (mjs->error != MJS_OK) {
    call_stack_restore_frame(mjs);


    mjs_pop(mjs);
  }
  mjs->vals.this_obj = prev_this_val;

  return mjs->error;
}
static ffi_fn_t *get_cb_impl_by_signature(const mjs_ffi_sig_t *sig);





struct cbdata {

  mjs_val_t func;

  mjs_val_t userdata;


  int8_t func_idx;

  int8_t userdata_idx;
};

void mjs_set_ffi_resolver(struct mjs *mjs, mjs_ffi_resolver_t *dlsym) {
  mjs->dlsym = dlsym;
}

static mjs_ffi_ctype_t parse_cval_type(struct mjs *mjs, const char *s,
                                       const char *e) {
  struct mg_str ms = { 
                    __null
                    , 0 };

  while (s < e && isspace((int) *s)) s++;
  while (e > s && isspace((int) e[-1])) e--;
  ms.p = s;
  ms.len = e - s;
  if (mg_vcmp(&ms, "void") == 0) {
    return MJS_FFI_CTYPE_NONE;
  } else if (mg_vcmp(&ms, "userdata") == 0) {
    return MJS_FFI_CTYPE_USERDATA;
  } else if (mg_vcmp(&ms, "int") == 0) {
    return MJS_FFI_CTYPE_INT;
  } else if (mg_vcmp(&ms, "bool") == 0) {
    return MJS_FFI_CTYPE_BOOL;
  } else if (mg_vcmp(&ms, "double") == 0) {
    return MJS_FFI_CTYPE_DOUBLE;
  } else if (mg_vcmp(&ms, "float") == 0) {
    return MJS_FFI_CTYPE_FLOAT;
  } else if (mg_vcmp(&ms, "char*") == 0 || mg_vcmp(&ms, "char *") == 0) {
    return MJS_FFI_CTYPE_CHAR_PTR;
  } else if (mg_vcmp(&ms, "void*") == 0 || mg_vcmp(&ms, "void *") == 0) {
    return MJS_FFI_CTYPE_VOID_PTR;
  } else if (mg_vcmp(&ms, "struct mg_str") == 0) {
    return MJS_FFI_CTYPE_STRUCT_MG_STR;
  } else if (mg_vcmp(&ms, "struct mg_str *") == 0 ||
             mg_vcmp(&ms, "struct mg_str*") == 0) {
    return MJS_FFI_CTYPE_STRUCT_MG_STR_PTR;
  } else {
    mjs_prepend_errorf(mjs, MJS_TYPE_ERROR, "failed to parse val type \"%.*s\"",
                       (int) ms.len, ms.p);
    return MJS_FFI_CTYPE_INVALID;
  }
}

static const char *find_paren(const char *s, const char *e) {
  for (; s < e; s++) {
    if (*s == '(') return s;
  }
  return 
        __null
            ;
}

static const char *find_closing_paren(const char *s, const char *e) {
  int nesting = 1;
  while (s < e) {
    if (*s == '(') {
      nesting++;
    } else if (*s == ')') {
      if (--nesting == 0) break;
    }
    s++;
  }
  return (s < e ? s : 
                     __null
                         );
}

static mjs_err_t mjs_parse_ffi_signature(struct mjs *mjs, const char *s,
                                              int sig_len, mjs_ffi_sig_t *sig,
                                              enum ffi_sig_type sig_type) {
  mjs_err_t ret = MJS_OK;
  int vtidx = 0;
  const char *cur, *e, *tmp_e, *tmp;
  struct mg_str rt = { 
                    __null
                    , 0 }, fn = { 
                                      __null
                                      , 0 }, args = { 
                                                          __null
                                                          , 0 };
  mjs_ffi_ctype_t val_type = MJS_FFI_CTYPE_INVALID;
  if (sig_len == ~0) {
    sig_len = strlen(s);
  }
  e = s + sig_len;

  mjs_ffi_sig_init(sig);


  for (cur = s; cur < e && isspace((int) *cur); cur++)
    ;


  tmp_e = find_paren(cur, e);
  if (tmp_e == 
              __null 
                   || tmp_e - s < 2) {
    ret = MJS_TYPE_ERROR;
    mjs_prepend_errorf(mjs, ret, "1");
    goto clean;
  }
  tmp = find_closing_paren(tmp_e + 1, e);
  if (tmp == 
            __null
                ) {
    ret = MJS_TYPE_ERROR;
    mjs_prepend_errorf(mjs, ret, "2");
    goto clean;
  }


  args.p = find_paren(tmp + 1, e);
  if (args.p == 
               __null
                   ) {

    fn.p = tmp_e - 1;
    while (fn.p > cur && isspace((int) *fn.p)) fn.p--;
    while (fn.p > cur && (isalnum((int) *fn.p) || *fn.p == '_')) {
      fn.p--;
      fn.len++;
    }
    fn.p++;
    rt.p = cur;
    rt.len = fn.p - rt.p;

    args.p = tmp_e + 1;
    args.len = tmp - args.p;
  } else {


    fn.p = tmp + 1;
    fn.len = args.p - tmp;
    rt.p = cur;
    rt.len = tmp_e - rt.p;
    args.p++;
    tmp = find_closing_paren(args.p, e);
    if (tmp == 
              __null
                  ) {
      ret = MJS_TYPE_ERROR;
      mjs_prepend_errorf(mjs, ret, "3");
      goto clean;
    }
    args.len = tmp - args.p;




    sig->is_callback = 1;
  }

  val_type = parse_cval_type(mjs, rt.p, rt.p + rt.len);
  if (val_type == MJS_FFI_CTYPE_INVALID) {
    ret = mjs->error;
    goto clean;
  }
  mjs_ffi_sig_set_val_type(sig, vtidx++, val_type);


  if (!sig->is_callback) {
    char buf[100];
    if (mjs->dlsym == 
                     __null
                         ) {
      ret = MJS_TYPE_ERROR;
      mjs_prepend_errorf(mjs, ret,
                         "resolver is not set, call mjs_set_ffi_resolver");
      goto clean;
    }

    snprintf(buf, sizeof(buf), "%.*s", (int) fn.len, fn.p);
    sig->fn = (ffi_fn_t *) mjs->dlsym(
                                     ((void *) 0)
                                                 , buf);
    if (sig->fn == 
                  __null
                      ) {
      ret = MJS_TYPE_ERROR;
      mjs_prepend_errorf(mjs, ret, "dlsym('%s') failed", buf);
      goto clean;
    }
  } else {
    tmp_e = strchr(tmp_e, ')');
    if (tmp_e == 
                __null
                    ) {
      ret = MJS_TYPE_ERROR;
      goto clean;
    }
  }


  cur = tmp_e = args.p;


  while (tmp_e - args.p < (ptrdiff_t) args.len) {
    int level = 0;
    int is_fp = 0;
    tmp_e = cur;


    while (*tmp_e && (level > 0 || (*tmp_e != ',' && *tmp_e != ')'))) {
      switch (*tmp_e) {
        case '(':
          level++;




          is_fp = 1;
          break;
        case ')':
          level--;
          break;
      }
      tmp_e++;
    }

    if (tmp_e == cur) break;


    if (is_fp) {

      if (sig->cb_sig != 
                        __null
                            ) {





        ret = MJS_TYPE_ERROR;
        mjs_prepend_errorf(mjs, ret, "only one callback is allowed");
        goto clean;
      }

      sig->cb_sig = (mjs_ffi_sig*)calloc(sizeof(*sig->cb_sig), 1);
      ret = mjs_parse_ffi_signature(mjs, cur, tmp_e - cur, sig->cb_sig,
                                    FFI_SIG_CALLBACK);
      if (ret != MJS_OK) {
        mjs_ffi_sig_free(sig->cb_sig);
        free(sig->cb_sig);
        sig->cb_sig = 
                     __null
                         ;
        goto clean;
      }
      val_type = MJS_FFI_CTYPE_CALLBACK;
    } else {

      val_type = parse_cval_type(mjs, cur, tmp_e);
      if (val_type == MJS_FFI_CTYPE_INVALID) {

        ret = MJS_TYPE_ERROR;
        goto clean;
      }
    }

    if (!mjs_ffi_sig_set_val_type(sig, vtidx++, val_type)) {
      ret = MJS_TYPE_ERROR;
      mjs_prepend_errorf(mjs, ret, "too many callback args");
      goto clean;
    }

    if (*tmp_e == ',') {

      cur = tmp_e + 1;
      while (*cur == ' ') cur++;
    } else {

      break;
    }
  }



  mjs_ffi_sig_validate(mjs, sig, sig_type);
  if (!sig->is_valid) {
    ret = MJS_TYPE_ERROR;
    goto clean;
  }


  if (sig->is_callback) {
    sig->fn = get_cb_impl_by_signature(sig);
    if (sig->fn == 
                  __null
                      ) {
      ret = MJS_TYPE_ERROR;
      mjs_prepend_errorf(mjs, ret,
                         "the callback signature is valid, but there's "
                         "no existing callback implementation for it");
      goto clean;
    }
  }

clean:
  if (ret != MJS_OK) {
    mjs_prepend_errorf(mjs, ret, "bad ffi signature: \"%.*s\"", sig_len, s);
    sig->is_valid = 0;
  }
  return ret;
}




union ffi_cb_data_val {
  void *p;
  uintptr_t w;
  double d;
  float f;
};

struct ffi_cb_data {
  union ffi_cb_data_val args[6];
};

static union ffi_cb_data_val ffi_cb_impl_generic(void *param,
                                                 struct ffi_cb_data *data) {
  struct mjs_ffi_cb_args *cbargs = (struct mjs_ffi_cb_args *) param;
  mjs_val_t *args, res = ((uint64_t)(1) << 63 | (uint64_t) 0x7ff0 << 48 | (uint64_t)(3) << 48);
  union ffi_cb_data_val ret;
  int i;
  struct mjs *mjs = cbargs->mjs;
  mjs_ffi_ctype_t return_ctype = MJS_FFI_CTYPE_NONE;
  mjs_err_t err;

  memset(&ret, 0, sizeof(ret));
  mjs_own(mjs, &res);


  
 (static_cast <bool> (
 cbargs->sig.args_cnt > 0
 ) ? void (0) : __assert_fail (
 "cbargs->sig.args_cnt > 0"
 , "mjs.c", 5709, __extension__ __PRETTY_FUNCTION__))
                                 ;


  args = (mjs_val_t*)calloc(1, sizeof(mjs_val_t) * cbargs->sig.args_cnt);
  for (i = 0; i < cbargs->sig.args_cnt; i++) {
    mjs_ffi_ctype_t val_type =
        cbargs->sig.val_types[i + 1 ];
    switch (val_type) {
      case MJS_FFI_CTYPE_USERDATA:
        args[i] = cbargs->userdata;
        break;
      case MJS_FFI_CTYPE_INT:
        args[i] = mjs_mk_number(mjs, (double) data->args[i].w);
        break;
      case MJS_FFI_CTYPE_BOOL:
        args[i] = mjs_mk_boolean(mjs, !!data->args[i].w);
        break;
      case MJS_FFI_CTYPE_CHAR_PTR: {
        const char *s = (char *) data->args[i].w;
        if (s == 
                __null
                    ) s = "";
        args[i] = mjs_mk_string(mjs, s, ~0, 1);
        break;
      }
      case MJS_FFI_CTYPE_VOID_PTR:
        args[i] = mjs_mk_foreign(mjs, (void *) data->args[i].w);
        break;
      case MJS_FFI_CTYPE_DOUBLE:
        args[i] = mjs_mk_number(mjs, data->args[i].d);
        break;
      case MJS_FFI_CTYPE_FLOAT:
        args[i] = mjs_mk_number(mjs, data->args[i].f);
        break;
      case MJS_FFI_CTYPE_STRUCT_MG_STR_PTR: {
        struct mg_str *s = (struct mg_str *) (void *) data->args[i].w;
        args[i] = mjs_mk_string(mjs, s->p, s->len, 1);
        break;
      }
      default:

        do { if (cs_log_print_prefix(LL_ERROR, "mjs.c", 5748)) { cs_log_printf ("unexpected val type for arg #%d: %d\n", i, val_type); } } while (0);
        abort();
    }
  }






  return_ctype = cbargs->sig.val_types[0];


  do { if (cs_log_print_prefix(LL_VERBOSE_DEBUG,
                                                        "mjs.c"
  ,
                                                        5762
  )) { cs_log_printf ("calling JS callback void-void %d from C", mjs_get_int(mjs, cbargs->func)); } } while (0)
                                                         ;
  err = mjs_apply(mjs, &res, cbargs->func, ((uint64_t)(1) << 63 | (uint64_t) 0x7ff0 << 48 | (uint64_t)(3) << 48), cbargs->sig.args_cnt,
                  args);




  cbargs = 
          __null
              ;
  if (err != MJS_OK) {



    mjs_print_error(mjs, 
                        stderr
                              , "MJS callback error",
                    1 );

    goto clean;
  }


  switch (return_ctype) {
    case MJS_FFI_CTYPE_NONE:

      break;
    case MJS_FFI_CTYPE_INT:
      ret.w = mjs_get_int(mjs, res);
      break;
    case MJS_FFI_CTYPE_BOOL:
      ret.w = mjs_get_bool(mjs, res);
      break;
    case MJS_FFI_CTYPE_VOID_PTR:
      ret.p = mjs_get_ptr(mjs, res);
      break;
    case MJS_FFI_CTYPE_DOUBLE:
      ret.d = mjs_get_double(mjs, res);
      break;
    case MJS_FFI_CTYPE_FLOAT:
      ret.f = (float) mjs_get_double(mjs, res);
      break;
    default:

      do { if (cs_log_print_prefix(LL_ERROR, "mjs.c", 5802)) { cs_log_printf ("unexpected return val type %d\n", return_ctype); } } while (0);
      abort();
  }

clean:
  free(args);
  mjs_disown(mjs, &res);
  return ret;
}

static void ffi_init_cb_data_wwww(struct ffi_cb_data *data, uintptr_t w0,
                                  uintptr_t w1, uintptr_t w2, uintptr_t w3,
                                  uintptr_t w4, uintptr_t w5) {
  memset(data, 0, sizeof(*data));
  data->args[0].w = w0;
  data->args[1].w = w1;
  data->args[2].w = w2;
  data->args[3].w = w3;
  data->args[4].w = w4;
  data->args[5].w = w5;
}

static uintptr_t ffi_cb_impl_wpwwwww(uintptr_t w0, uintptr_t w1, uintptr_t w2,
                                     uintptr_t w3, uintptr_t w4, uintptr_t w5) {
  struct ffi_cb_data data;
  ffi_init_cb_data_wwww(&data, w0, w1, w2, w3, w4, w5);
  return ffi_cb_impl_generic((void *) w0, &data).w;
}

static uintptr_t ffi_cb_impl_wwpwwww(uintptr_t w0, uintptr_t w1, uintptr_t w2,
                                     uintptr_t w3, uintptr_t w4, uintptr_t w5) {
  struct ffi_cb_data data;
  ffi_init_cb_data_wwww(&data, w0, w1, w2, w3, w4, w5);
  return ffi_cb_impl_generic((void *) w1, &data).w;
}

static uintptr_t ffi_cb_impl_wwwpwww(uintptr_t w0, uintptr_t w1, uintptr_t w2,
                                     uintptr_t w3, uintptr_t w4, uintptr_t w5) {
  struct ffi_cb_data data;
  ffi_init_cb_data_wwww(&data, w0, w1, w2, w3, w4, w5);
  return ffi_cb_impl_generic((void *) w2, &data).w;
}

static uintptr_t ffi_cb_impl_wwwwpww(uintptr_t w0, uintptr_t w1, uintptr_t w2,
                                     uintptr_t w3, uintptr_t w4, uintptr_t w5) {
  struct ffi_cb_data data;
  ffi_init_cb_data_wwww(&data, w0, w1, w2, w3, w4, w5);
  return ffi_cb_impl_generic((void *) w3, &data).w;
}

static uintptr_t ffi_cb_impl_wwwwwpw(uintptr_t w0, uintptr_t w1, uintptr_t w2,
                                     uintptr_t w3, uintptr_t w4, uintptr_t w5) {
  struct ffi_cb_data data;
  ffi_init_cb_data_wwww(&data, w0, w1, w2, w3, w4, w5);
  return ffi_cb_impl_generic((void *) w4, &data).w;
}

static uintptr_t ffi_cb_impl_wwwwwwp(uintptr_t w0, uintptr_t w1, uintptr_t w2,
                                     uintptr_t w3, uintptr_t w4, uintptr_t w5) {
  struct ffi_cb_data data;
  ffi_init_cb_data_wwww(&data, w0, w1, w2, w3, w4, w5);
  return ffi_cb_impl_generic((void *) w5, &data).w;
}

static uintptr_t ffi_cb_impl_wpd(uintptr_t w0, double d1) {
  struct ffi_cb_data data;

  memset(&data, 0, sizeof(data));
  data.args[0].w = w0;
  data.args[1].d = d1;

  return ffi_cb_impl_generic((void *) w0, &data).w;
}

static uintptr_t ffi_cb_impl_wdp(double d0, uintptr_t w1) {
  struct ffi_cb_data data;

  memset(&data, 0, sizeof(data));
  data.args[0].d = d0;
  data.args[1].w = w1;

  return ffi_cb_impl_generic((void *) w1, &data).w;
}


static struct mjs_ffi_cb_args **ffi_get_matching(struct mjs_ffi_cb_args **plist,
                                                 mjs_val_t func,
                                                 mjs_val_t userdata) {
  for (; *plist != 
                  __null
                      ; plist = &((*plist)->next)) {
    if ((*plist)->func == func && (*plist)->userdata == userdata) {
      break;
    }
  }
  return plist;
}

static ffi_fn_t *get_cb_impl_by_signature(const mjs_ffi_sig_t *sig) {
  if (sig->is_valid) {
    int i;
    int double_cnt = 0;
    int float_cnt = 0;
    int userdata_idx = 0 ;

    for (i = 1 ; i < (6 + 1 );
         i++) {
      mjs_ffi_ctype_t type = sig->val_types[i];
      switch (type) {
        case MJS_FFI_CTYPE_DOUBLE:
          double_cnt++;
          break;
        case MJS_FFI_CTYPE_FLOAT:
          float_cnt++;
          break;
        case MJS_FFI_CTYPE_USERDATA:
          
         (static_cast <bool> (
         userdata_idx == 0
         ) ? void (0) : __assert_fail (
         "userdata_idx == 0"
         , "mjs.c", 5916, __extension__ __PRETTY_FUNCTION__))
                                  ;
          userdata_idx = i;
          break;
        default:
          break;
      }
    }

    if (float_cnt > 0) {

      return 
            __null
                ;
    }

    
   (static_cast <bool> (
   userdata_idx > 0
   ) ? void (0) : __assert_fail (
   "userdata_idx > 0"
   , "mjs.c", 5929, __extension__ __PRETTY_FUNCTION__))
                           ;

    if (sig->args_cnt <= 6) {
      if (mjs_ffi_is_regular_word_or_void(sig->val_types[0])) {

        switch (double_cnt) {
          case 0:

            switch (userdata_idx) {
              case 1:
                return (ffi_fn_t *) ffi_cb_impl_wpwwwww;
              case 2:
                return (ffi_fn_t *) ffi_cb_impl_wwpwwww;
              case 3:
                return (ffi_fn_t *) ffi_cb_impl_wwwpwww;
              case 4:
                return (ffi_fn_t *) ffi_cb_impl_wwwwpww;
              case 5:
                return (ffi_fn_t *) ffi_cb_impl_wwwwwpw;
              case 6:
                return (ffi_fn_t *) ffi_cb_impl_wwwwwwp;
              default:

                abort();
            }
            break;
          case 1:

            switch (userdata_idx) {
              case 1:
                return (ffi_fn_t *) ffi_cb_impl_wpd;
              case 2:
                return (ffi_fn_t *) ffi_cb_impl_wdp;
            }
            break;
        }
      }
    } else {


    }
  }

  return 
        __null
            ;
}

static mjs_val_t mjs_ffi_sig_to_value(struct mjs_ffi_sig *psig) {
  if (psig == 
             __null
                 ) {
    return ((uint64_t)(1) << 63 | (uint64_t) 0x7ff0 << 48 | (uint64_t)(15) << 48);
  } else {
    return mjs_legit_pointer_to_value(psig) | ((uint64_t)(1) << 63 | (uint64_t) 0x7ff0 << 48 | (uint64_t)(14) << 48);
  }
}

static int mjs_is_ffi_sig(mjs_val_t v) {
  return (v & ((uint64_t)(1) << 63 | (uint64_t) 0x7ff0 << 48 | (uint64_t)(15) << 48)) == ((uint64_t)(1) << 63 | (uint64_t) 0x7ff0 << 48 | (uint64_t)(14) << 48);
}

static struct mjs_ffi_sig *mjs_get_ffi_sig_struct(mjs_val_t v) {
  struct mjs_ffi_sig *ret = 
                           __null
                               ;
  
 (static_cast <bool> (
 mjs_is_ffi_sig(v)
 ) ? void (0) : __assert_fail (
 "mjs_is_ffi_sig(v)"
 , "mjs.c", 5989, __extension__ __PRETTY_FUNCTION__))
                          ;
  ret = (struct mjs_ffi_sig *) get_ptr(v);
  return ret;
}

static mjs_val_t mjs_mk_ffi_sig(struct mjs *mjs) {
  struct mjs_ffi_sig *psig = new_ffi_sig(mjs);
  mjs_ffi_sig_init(psig);
  return mjs_ffi_sig_to_value(psig);
}

static void mjs_ffi_sig_destructor(struct mjs *mjs, void *psig) {
  mjs_ffi_sig_free((mjs_ffi_sig_t *) psig);
  (void) mjs;
}

static mjs_err_t mjs_ffi_call(struct mjs *mjs) {
  mjs_err_t e = MJS_OK;
  const char *sig_str = 
                       __null
                           ;
  mjs_val_t sig_str_v = mjs_arg(mjs, 0);
  mjs_val_t ret_v = ((uint64_t)(1) << 63 | (uint64_t) 0x7ff0 << 48 | (uint64_t)(3) << 48);
  struct mjs_ffi_sig *psig = mjs_get_ffi_sig_struct(mjs_mk_ffi_sig(mjs));
  size_t sig_str_len;

  sig_str = mjs_get_string(mjs, &sig_str_v, &sig_str_len);
  e = mjs_parse_ffi_signature(mjs, sig_str, sig_str_len, psig, FFI_SIG_FUNC);
  if (e != MJS_OK) goto clean;
  ret_v = mjs_ffi_sig_to_value(psig);

clean:
  mjs_return(mjs, ret_v);
  return e;
}

static mjs_err_t mjs_ffi_call2(struct mjs *mjs) {
  mjs_err_t ret = MJS_OK;
  mjs_ffi_sig_t *psig = 
                       __null
                           ;
  mjs_ffi_ctype_t rtype;
  mjs_val_t sig_v = *vptr(&mjs->stack, mjs_getretvalpos(mjs));

  int i, nargs;
  struct ffi_arg res;
  struct ffi_arg args[6];
  struct cbdata cbdata;


  mjs_val_t resv = mjs_mk_undefined();





  mjs_val_t argvs[6];
  struct mg_str argvmgstr[6];

  if (mjs_is_ffi_sig(sig_v)) {
    psig = mjs_get_ffi_sig_struct(sig_v);
  } else {
    ret = MJS_TYPE_ERROR;
    mjs_prepend_errorf(mjs, ret, "non-ffi-callable value");
    goto clean;
  }

  memset(&cbdata, 0, sizeof(cbdata));
  cbdata.func_idx = -1;
  cbdata.userdata_idx = -1;

  rtype = psig->val_types[0];

  switch (rtype) {
    case MJS_FFI_CTYPE_DOUBLE:
      res.ctype = FFI_CTYPE_DOUBLE;
      break;
    case MJS_FFI_CTYPE_FLOAT:
      res.ctype = FFI_CTYPE_FLOAT;
      break;
    case MJS_FFI_CTYPE_BOOL:
      res.ctype = FFI_CTYPE_BOOL;
      break;
    case MJS_FFI_CTYPE_USERDATA:
    case MJS_FFI_CTYPE_INT:
    case MJS_FFI_CTYPE_CHAR_PTR:
    case MJS_FFI_CTYPE_VOID_PTR:
    case MJS_FFI_CTYPE_NONE:
      res.ctype = FFI_CTYPE_WORD;
      break;

    case MJS_FFI_CTYPE_INVALID:
      ret = MJS_TYPE_ERROR;
      mjs_prepend_errorf(mjs, ret, "wrong ffi return type");
      goto clean;
  }
  res.v.i = 0;

  nargs =
      mjs_stack_size(&mjs->stack) - mjs_get_int(mjs, vtop(&mjs->call_stack));

  if (nargs != psig->args_cnt) {
    ret = MJS_TYPE_ERROR;
    mjs_prepend_errorf(mjs, ret, "got %d actuals, but function takes %d args",
                       nargs, psig->args_cnt);
    goto clean;
  }

  for (i = 0; i < nargs; i++) {
    mjs_val_t arg = mjs_arg(mjs, i);

    switch (psig->val_types[1 + i]) {
      case MJS_FFI_CTYPE_NONE:





        ret = MJS_TYPE_ERROR;
        if (i == 0) {

          mjs_prepend_errorf(mjs, ret, "ffi-ed function takes no arguments");
        } else {




          mjs_prepend_errorf(mjs, ret, "bad ffi arg #%d type: \"void\"", i);
        }

        goto clean;
      case MJS_FFI_CTYPE_USERDATA:

        if (cbdata.userdata_idx != -1) {
          ret = MJS_TYPE_ERROR;
          mjs_prepend_errorf(mjs, ret, "two or more userdata args: #%d and %d",
                             cbdata.userdata_idx, i);

          goto clean;
        }
        cbdata.userdata = arg;
        cbdata.userdata_idx = i;
        break;
      case MJS_FFI_CTYPE_INT: {
        int intval = 0;
        if (mjs_is_number(arg)) {
          intval = mjs_get_int(mjs, arg);
        } else if (mjs_is_boolean(arg)) {
          intval = mjs_get_bool(mjs, arg);
        } else {
          ret = MJS_TYPE_ERROR;
          mjs_prepend_errorf(
              mjs, ret, "actual arg #%d is not an int (the type idx is: %s)", i,
              mjs_typeof(arg));
        }
        ffi_set_word(&args[i], intval);
      } break;
      case MJS_FFI_CTYPE_STRUCT_MG_STR_PTR: {
        if (!mjs_is_string(arg)) {
          ret = MJS_TYPE_ERROR;
          mjs_prepend_errorf(
              mjs, ret, "actual arg #%d is not a string (the type idx is: %s)",
              i, mjs_typeof(arg));
          goto clean;
        }
        argvs[i] = arg;
        argvmgstr[i].p = mjs_get_string(mjs, &argvs[i], &argvmgstr[i].len);




        ffi_set_ptr(&args[i], (void *) &argvmgstr[i]);
        break;
      }
      case MJS_FFI_CTYPE_BOOL: {
        int intval = 0;
        if (mjs_is_number(arg)) {
          intval = !!mjs_get_int(mjs, arg);
        } else if (mjs_is_boolean(arg)) {
          intval = mjs_get_bool(mjs, arg);
        } else {
          ret = MJS_TYPE_ERROR;
          mjs_prepend_errorf(
              mjs, ret, "actual arg #%d is not a bool (the type idx is: %s)", i,
              mjs_typeof(arg));
        }
        ffi_set_word(&args[i], intval);
      } break;
      case MJS_FFI_CTYPE_DOUBLE:
        ffi_set_double(&args[i], mjs_get_double(mjs, arg));
        break;
      case MJS_FFI_CTYPE_FLOAT:
        ffi_set_float(&args[i], (float) mjs_get_double(mjs, arg));
        break;
      case MJS_FFI_CTYPE_CHAR_PTR: {
        size_t s;
        if (mjs_is_string(arg)) {




          argvs[i] = arg;
          ffi_set_ptr(&args[i], (void *) mjs_get_string(mjs, &argvs[i], &s));
        } else if (mjs_is_null(arg)) {
          ffi_set_ptr(&args[i], 
                               __null
                                   );
        } else {
          ret = MJS_TYPE_ERROR;
          mjs_prepend_errorf(
              mjs, ret, "actual arg #%d is not a string (the type idx is: %s)",
              i, mjs_typeof(arg));
          goto clean;
        }
      } break;
      case MJS_FFI_CTYPE_VOID_PTR:
        if (mjs_is_string(arg)) {
          size_t n;




          argvs[i] = arg;
          ffi_set_ptr(&args[i], (void *) mjs_get_string(mjs, &argvs[i], &n));
        } else if (mjs_is_foreign(arg)) {
          ffi_set_ptr(&args[i], (void *) mjs_get_ptr(mjs, arg));
        } else if (mjs_is_null(arg)) {
          ffi_set_ptr(&args[i], 
                               __null
                                   );
        } else {
          ret = MJS_TYPE_ERROR;
          mjs_prepend_errorf(mjs, ret, "actual arg #%d is not a ptr", i);
          goto clean;
        }
        break;
      case MJS_FFI_CTYPE_CALLBACK:
        if (mjs_is_function(arg) || mjs_is_foreign(arg) ||
            mjs_is_ffi_sig(arg)) {




          cbdata.func = arg;
          cbdata.func_idx = i;
        } else {
          ret = MJS_TYPE_ERROR;
          mjs_prepend_errorf(mjs, ret,
                             "actual arg #%d is not a function, but %s", i,
                             mjs_stringify_type((enum mjs_type) arg));
          goto clean;
        }
        break;
      case MJS_FFI_CTYPE_INVALID:

        ret = MJS_TYPE_ERROR;
        mjs_prepend_errorf(mjs, ret, "wrong arg type");
        goto clean;
      default:
        abort();
        break;
    }
  }

  if (cbdata.userdata_idx >= 0 && cbdata.func_idx >= 0) {
    struct mjs_ffi_cb_args *cbargs = 
                                    __null
                                        ;
    struct mjs_ffi_cb_args **pitem = 
                                    __null
                                        ;







    pitem = ffi_get_matching(&mjs->ffi_cb_args, cbdata.func, cbdata.userdata);
    if (*pitem == 
                 __null
                     ) {

      cbargs = (mjs_ffi_cb_args*)calloc(1, sizeof(*cbargs));
      cbargs->mjs = mjs;
      cbargs->func = cbdata.func;
      cbargs->userdata = cbdata.userdata;
      mjs_ffi_sig_copy(&cbargs->sig, psig->cb_sig);


      *pitem = cbargs;
    } else {

      cbargs = *pitem;
    }

    {
      union {
        ffi_fn_t *fn;
        void *p;
      } u;
      u.fn = psig->cb_sig->fn;
      ffi_set_ptr(&args[cbdata.func_idx], u.p);
      ffi_set_ptr(&args[cbdata.userdata_idx], cbargs);
    }
  } else if (!(cbdata.userdata_idx == -1 && cbdata.func_idx == -1)) {






    abort();
  }

  ffi_call(psig->fn, nargs, &res, args);

  switch (rtype) {
    case MJS_FFI_CTYPE_CHAR_PTR: {
      const char *s = (const char *) (uintptr_t) res.v.i;
      if (s != 
              __null
                  ) {
        resv = mjs_mk_string(mjs, s, ~0, 1);
      } else {
        resv = ((uint64_t)(1) << 63 | (uint64_t) 0x7ff0 << 48 | (uint64_t)(15) << 48);
      }
      break;
    }
    case MJS_FFI_CTYPE_VOID_PTR:
      resv = mjs_mk_foreign(mjs, (void *) (uintptr_t) res.v.i);
      break;
    case MJS_FFI_CTYPE_INT:
      resv = mjs_mk_number(mjs, (int) res.v.i);
      break;
    case MJS_FFI_CTYPE_BOOL:
      resv = mjs_mk_boolean(mjs, !!res.v.i);
      break;
    case MJS_FFI_CTYPE_DOUBLE:
      resv = mjs_mk_number(mjs, res.v.d);
      break;
    case MJS_FFI_CTYPE_FLOAT:
      resv = mjs_mk_number(mjs, res.v.f);
      break;
    default:
      resv = mjs_mk_undefined();
      break;
  }

clean:




  if (ret != MJS_OK) {
    mjs_prepend_errorf(mjs, ret, "failed to call FFIed function");

  }
  mjs_return(mjs, resv);

  return ret;
}





static void mjs_ffi_cb_free(struct mjs *mjs) {
  mjs_val_t ret = mjs_mk_number(mjs, 0);
  mjs_val_t func = mjs_arg(mjs, 0);
  mjs_val_t userdata = mjs_arg(mjs, 1);

  if (mjs_is_function(func)) {
    struct mjs_ffi_cb_args **pitem =
        ffi_get_matching(&mjs->ffi_cb_args, func, userdata);
    if (*pitem != 
                 __null
                     ) {

      struct mjs_ffi_cb_args *cbargs = *pitem;
      *pitem = cbargs->next;
      mjs_ffi_sig_free(&cbargs->sig);
      free(cbargs);
      ret = mjs_mk_number(mjs, 1);
    }
  } else {
    mjs_prepend_errorf(mjs, MJS_TYPE_ERROR, "missing argument 'func'");
  }

  mjs_return(mjs, ret);
}

void mjs_ffi_args_free_list(struct mjs *mjs) {
  ffi_cb_args_t *next = mjs->ffi_cb_args;

  while (next != 
                __null
                    ) {
    ffi_cb_args_t *cur = next;
    next = next->next;
    free(cur);
  }
}

static void mjs_ffi_sig_init(mjs_ffi_sig_t *sig) {
  memset(sig, 0, sizeof(*sig));
}

static void mjs_ffi_sig_copy(mjs_ffi_sig_t *to,
                                  const mjs_ffi_sig_t *from) {
  memcpy(to, from, sizeof(*to));
  if (from->cb_sig != 
                     __null
                         ) {
    to->cb_sig = (mjs_ffi_sig*)calloc(sizeof(*to->cb_sig), 1);
    mjs_ffi_sig_copy(to->cb_sig, from->cb_sig);
  }
}

static void mjs_ffi_sig_free(mjs_ffi_sig_t *sig) {
  if (sig->cb_sig != 
                    __null
                        ) {
    free(sig->cb_sig);
    sig->cb_sig = 
                 __null
                     ;
  }
}

static int mjs_ffi_sig_set_val_type(mjs_ffi_sig_t *sig, int idx,
                                         mjs_ffi_ctype_t type) {
  if (idx < (6 + 1 )) {
    sig->val_types[idx] = type;
    return 1;
  } else {

    return 0;
  }
}

static int mjs_ffi_sig_validate(struct mjs *mjs, mjs_ffi_sig_t *sig,
                                     enum ffi_sig_type sig_type) {
  int ret = 0;
  int i;
  int callback_idx = 0;
  int userdata_idx = 0;

  sig->is_valid = 0;

  switch (sig_type) {
    case FFI_SIG_FUNC:

      if (sig->val_types[0] != MJS_FFI_CTYPE_NONE &&
          sig->val_types[0] != MJS_FFI_CTYPE_INT &&
          sig->val_types[0] != MJS_FFI_CTYPE_BOOL &&
          sig->val_types[0] != MJS_FFI_CTYPE_DOUBLE &&
          sig->val_types[0] != MJS_FFI_CTYPE_FLOAT &&
          sig->val_types[0] != MJS_FFI_CTYPE_VOID_PTR &&
          sig->val_types[0] != MJS_FFI_CTYPE_CHAR_PTR) {
        mjs_prepend_errorf(mjs, MJS_TYPE_ERROR, "invalid return value type");
        goto clean;
      }
      break;
    case FFI_SIG_CALLBACK:

      if (sig->val_types[0] != MJS_FFI_CTYPE_NONE &&
          sig->val_types[0] != MJS_FFI_CTYPE_INT &&
          sig->val_types[0] != MJS_FFI_CTYPE_BOOL &&
          sig->val_types[0] != MJS_FFI_CTYPE_DOUBLE &&
          sig->val_types[0] != MJS_FFI_CTYPE_FLOAT &&
          sig->val_types[0] != MJS_FFI_CTYPE_VOID_PTR) {
        mjs_prepend_errorf(mjs, MJS_TYPE_ERROR, "invalid return value type");
        goto clean;
      }
  }


  for (i = 1; i < (6 + 1 ); i++) {
    mjs_ffi_ctype_t type = sig->val_types[i];
    switch (type) {
      case MJS_FFI_CTYPE_USERDATA:
        if (userdata_idx != 0) {

          mjs_prepend_errorf(mjs, MJS_TYPE_ERROR,
                             "more than one userdata arg: #%d and #%d",
                             (userdata_idx - 1), (i - 1));
          goto clean;
        }
        userdata_idx = i;
        break;
      case MJS_FFI_CTYPE_CALLBACK:
        switch (sig_type) {
          case FFI_SIG_FUNC:
            break;
          case FFI_SIG_CALLBACK:
            mjs_prepend_errorf(mjs, MJS_TYPE_ERROR,
                               "callback can't take another callback");
            goto clean;
        }
        callback_idx = i;
        break;
      case MJS_FFI_CTYPE_INT:
      case MJS_FFI_CTYPE_BOOL:
      case MJS_FFI_CTYPE_VOID_PTR:
      case MJS_FFI_CTYPE_CHAR_PTR:
      case MJS_FFI_CTYPE_STRUCT_MG_STR_PTR:
      case MJS_FFI_CTYPE_DOUBLE:
      case MJS_FFI_CTYPE_FLOAT:

        break;
      case MJS_FFI_CTYPE_NONE:

        goto args_over;
      default:
        mjs_prepend_errorf(mjs, MJS_INTERNAL_ERROR, "invalid ffi_ctype: %d",
                           type);
        goto clean;
    }

    sig->args_cnt++;
  }
args_over:

  switch (sig_type) {
    case FFI_SIG_FUNC:
      if (!((callback_idx > 0 && userdata_idx > 0) ||
            (callback_idx == 0 && userdata_idx == 0))) {
        mjs_prepend_errorf(mjs, MJS_TYPE_ERROR,
                           "callback and userdata should be either both "
                           "present or both absent");
        goto clean;
      }
      break;
    case FFI_SIG_CALLBACK:
      if (userdata_idx == 0) {

        mjs_prepend_errorf(mjs, MJS_TYPE_ERROR, "no userdata arg");
        goto clean;
      }
      break;
  }

  ret = 1;

clean:
  if (ret) {
    sig->is_valid = 1;
  }
  return ret;
}

static int mjs_ffi_is_regular_word(mjs_ffi_ctype_t type) {
  switch (type) {
    case MJS_FFI_CTYPE_INT:
    case MJS_FFI_CTYPE_BOOL:
      return 1;
    default:
      return 0;
  }
}

static int mjs_ffi_is_regular_word_or_void(mjs_ffi_ctype_t type) {
  return (type == MJS_FFI_CTYPE_NONE || mjs_ffi_is_regular_word(type));
}
static struct gc_block *gc_new_block(struct gc_arena *a, size_t size);
static void gc_free_block(struct gc_block *b);
static void gc_mark_mbuf_pt(struct mjs *mjs, const struct mbuf *mbuf);

static struct mjs_object *new_object(struct mjs *mjs) {
  return (struct mjs_object *) gc_alloc_cell(mjs, &mjs->object_arena);
}

static struct mjs_property *new_property(struct mjs *mjs) {
  return (struct mjs_property *) gc_alloc_cell(mjs, &mjs->property_arena);
}

static struct mjs_ffi_sig *new_ffi_sig(struct mjs *mjs) {
  return (struct mjs_ffi_sig *) gc_alloc_cell(mjs, &mjs->ffi_sig_arena);
}


static void gc_arena_init(struct gc_arena *a, size_t cell_size,
                               size_t initial_size, size_t size_increment) {
  
 (static_cast <bool> (
 cell_size >= sizeof(uintptr_t)
 ) ? void (0) : __assert_fail (
 "cell_size >= sizeof(uintptr_t)"
 , "mjs.c", 6607, __extension__ __PRETTY_FUNCTION__))
                                       ;

  memset(a, 0, sizeof(*a));
  a->cell_size = cell_size;
  a->size_increment = size_increment;
  a->blocks = gc_new_block(a, initial_size);
}

static void gc_arena_destroy(struct mjs *mjs, struct gc_arena *a) {
  struct gc_block *b;

  if (a->blocks != 
                  __null
                      ) {
    gc_sweep(mjs, a, 0);
    for (b = a->blocks; b != 
                            __null
                                ;) {
      struct gc_block *tmp;
      tmp = b;
      b = b->next;
      gc_free_block(tmp);
    }
  }
}

static void gc_free_block(struct gc_block *b) {
  free(b->base);
  free(b);
}

static struct gc_block *gc_new_block(struct gc_arena *a, size_t size) {
  struct gc_cell *cur;
  struct gc_block *b;

  b = (struct gc_block *) calloc(1, sizeof(*b));
  if (b == 
          __null
              ) abort();

  b->size = size;
  b->base = (struct gc_cell *) calloc(a->cell_size, b->size);
  if (b->base == 
                __null
                    ) abort();

  for (cur = ((struct gc_cell *) (((char *) (b->base)) +((0) * (a)->cell_size)));
       cur < ((struct gc_cell *) (((char *) (b->base)) +((b->size) * (a)->cell_size)));
       cur = ((struct gc_cell *) (((char *) (cur)) +((1) * (a)->cell_size)))) {
    cur->head.link = a->free;
    a->free = cur;
  }

  return b;
}





static int gc_arena_is_gc_needed(struct gc_arena *a) {
  struct gc_cell *r = a->free;
  int i;

  for (i = 0; i <= 2; i++, r = r->head.link) {
    if (r == 
            __null
                ) {
      return 1;
    }
  }

  return 0;
}

static int gc_strings_is_gc_needed(struct mjs *mjs) {
  struct mbuf *m = &mjs->owned_strings;
  return (double) m->len / (double) m->size > 0.9;
}

static void *gc_alloc_cell(struct mjs *mjs, struct gc_arena *a) {
  struct gc_cell *r;

  if (a->free == 
                __null
                    ) {
    struct gc_block *b = gc_new_block(a, a->size_increment);
    b->next = a->blocks;
    a->blocks = b;
  }
  r = a->free;

  (((struct gc_cell *) (r))->head.word &= ~1);

  a->free = r->head.link;







  if (gc_arena_is_gc_needed(a)) {
    mjs->need_gc = 1;
  }





  memset(r, 0, a->cell_size);
  return (void *) r;
}







void gc_sweep(struct mjs *mjs, struct gc_arena *a, size_t start) {
  struct gc_block *b;
  struct gc_cell *cur;
  struct gc_block **prevp = &a->blocks;
  {
    struct gc_cell *next;
    for (cur = a->free; cur != 
                              __null
                                  ; cur = next) {
      next = cur->head.link;
      (((struct gc_cell *) (cur))->head.word |= 2);
    }
  }




  a->free = 
           __null
               ;

  for (b = a->blocks; b != 
                          __null
                              ;) {
    size_t freed_in_block = 0;





    struct gc_cell *prev_free = a->free;

    for (cur = ((struct gc_cell *) (((char *) (b->base)) +((start) * (a)->cell_size)));
         cur < ((struct gc_cell *) (((char *) (b->base)) +((b->size) * (a)->cell_size)));
         cur = ((struct gc_cell *) (((char *) (cur)) +((1) * (a)->cell_size)))) {
      if ((((struct gc_cell *) (cur))->head.word & 1)) {

        (((struct gc_cell *) (cur))->head.word &= ~1);



      } else {






        if ((((struct gc_cell *) (cur))->head.word & 2)) {

          (((struct gc_cell *) (cur))->head.word &= ~2);
        } else {




          if (a->destructor != 
                              __null
                                  ) {
            a->destructor(mjs, cur);
          }
          memset(cur, 0, a->cell_size);
        }


        cur->head.link = a->free;
        a->free = cur;
        freed_in_block++;



      }
    }






    if (b->next != 
                  __null 
                       && freed_in_block == b->size) {
      *prevp = b->next;
      gc_free_block(b);
      b = *prevp;
      a->free = prev_free;
    } else {
      prevp = &b->next;
      b = b->next;
    }
  }
}


static void gc_mark_ffi_sig(struct mjs *mjs, mjs_val_t *v) {
  struct mjs_ffi_sig *psig;

  
 (static_cast <bool> (
 mjs_is_ffi_sig(*v)
 ) ? void (0) : __assert_fail (
 "mjs_is_ffi_sig(*v)"
 , "mjs.c", 6810, __extension__ __PRETTY_FUNCTION__))
                           ;

  psig = mjs_get_ffi_sig_struct(*v);





  if (!gc_check_val(mjs, *v)) {
    abort();
  }

  if ((((struct gc_cell *) (psig))->head.word & 1)) return;

  (((struct gc_cell *) (psig))->head.word |= 1);
}


static void gc_mark_object(struct mjs *mjs, mjs_val_t *v) {
  struct mjs_object *obj_base;
  struct mjs_property *prop;
  struct mjs_property *next;

  
 (static_cast <bool> (
 mjs_is_object(*v)
 ) ? void (0) : __assert_fail (
 "mjs_is_object(*v)"
 , "mjs.c", 6833, __extension__ __PRETTY_FUNCTION__))
                          ;

  obj_base = get_object_struct(*v);





  if (!gc_check_val(mjs, *v)) {
    abort();
  }

  if ((((struct gc_cell *) (obj_base))->head.word & 1)) return;


  for ((prop = obj_base->properties), (((struct gc_cell *) (obj_base))->head.word |= 1); prop != 
                                                             __null
                                                                 ;
       prop = next) {
    if (!gc_check_ptr(&mjs->property_arena, prop)) {
      abort();
    }

    gc_mark(mjs, &prop->name);
    gc_mark(mjs, &prop->value);

    next = prop->next;
    (((struct gc_cell *) (prop))->head.word |= 1);
  }







}


static void gc_mark_string(struct mjs *mjs, mjs_val_t *v) {
  mjs_val_t h, tmp = 0;
  char *s;
  
 (static_cast <bool> (
 (*v & ((uint64_t)(1) << 63 | (uint64_t) 0x7ff0 << 48 | (uint64_t)(15) << 48)) == ((uint64_t)(1) << 63 | (uint64_t) 0x7ff0 << 48 | (uint64_t)(8) << 48)
 ) ? void (0) : __assert_fail (
 "(*v & MJS_TAG_MASK) == MJS_TAG_STRING_O"
 , "mjs.c", 6899, __extension__ __PRETTY_FUNCTION__))
                                                ;

  s = mjs->owned_strings.buf + gc_string_mjs_val_to_offset(*v);
  
 (static_cast <bool> (
 s < mjs->owned_strings.buf + mjs->owned_strings.len
 ) ? void (0) : __assert_fail (
 "s < mjs->owned_strings.buf + mjs->owned_strings.len"
 , "mjs.c", 6902, __extension__ __PRETTY_FUNCTION__))
                                                            ;
  if (s[-1] == '\0') {
    memcpy(&tmp, s, sizeof(tmp) - 2);
    tmp |= ((uint64_t)(1) << 63 | (uint64_t) 0x7ff0 << 48 | (uint64_t)(10) << 48);
  } else {
    memcpy(&tmp, s, sizeof(tmp) - 2);
    tmp |= ((uint64_t)(1) << 63 | (uint64_t) 0x7ff0 << 48 | (uint64_t)(2) << 48);
  }

  h = (mjs_val_t)(uintptr_t) v;
  s[-1] = 1;
  memcpy(s, &h, sizeof(h) - 2);
  memcpy(v, &tmp, sizeof(tmp));
}

static void gc_mark(struct mjs *mjs, mjs_val_t *v) {
  if (mjs_is_object(*v)) {
    gc_mark_object(mjs, v);
  }
  if (mjs_is_ffi_sig(*v)) {
    gc_mark_ffi_sig(mjs, v);
  }
  if ((*v & ((uint64_t)(1) << 63 | (uint64_t) 0x7ff0 << 48 | (uint64_t)(15) << 48)) == ((uint64_t)(1) << 63 | (uint64_t) 0x7ff0 << 48 | (uint64_t)(8) << 48)) {
    gc_mark_string(mjs, v);
  }
}

static uint64_t gc_string_mjs_val_to_offset(mjs_val_t v) {
  return (((uint64_t)(uintptr_t) get_ptr(v)) & ~((uint64_t)(1) << 63 | (uint64_t) 0x7ff0 << 48 | (uint64_t)(15) << 48));
}

static mjs_val_t gc_string_val_from_offset(uint64_t s) {
  return s | ((uint64_t)(1) << 63 | (uint64_t) 0x7ff0 << 48 | (uint64_t)(8) << 48);
}

void gc_compact_strings(struct mjs *mjs) {
  char *p = mjs->owned_strings.buf + 1;
  uint64_t h, next, head = 1;
  int len, llen;

  while (p < mjs->owned_strings.buf + mjs->owned_strings.len) {
    if (p[-1] == '\1') {

      h = 0;
      memcpy(&h, p, sizeof(h) - 2);






      for (; (h & ((uint64_t)(1) << 63 | (uint64_t) 0x7ff0 << 48 | (uint64_t)(15) << 48)) != ((uint64_t)(1) << 63 | (uint64_t) 0x7ff0 << 48 | (uint64_t)(10) << 48); h = next) {
        h &= ~((uint64_t)(1) << 63 | (uint64_t) 0x7ff0 << 48 | (uint64_t)(15) << 48);
        memcpy(&next, (char *) (uintptr_t) h, sizeof(h));

        *(mjs_val_t *) (uintptr_t) h = gc_string_val_from_offset(head);
      }
      h &= ~((uint64_t)(1) << 63 | (uint64_t) 0x7ff0 << 48 | (uint64_t)(15) << 48);





      len = cs_varint_decode_unsafe((unsigned char *) &h, &llen);
      len += llen + 1;





      memcpy(p, &h, sizeof(h) - 2);




      memmove(mjs->owned_strings.buf + head, p, len);
      mjs->owned_strings.buf[head - 1] = 0x0;
      p += len;
      head += len;
    } else {
      len = cs_varint_decode_unsafe((unsigned char *) p, &llen);
      len += llen + 1;

      p += len;
    }
  }

  mjs->owned_strings.len = head;
}

static int maybe_gc(struct mjs *mjs) {
  if (!mjs->inhibit_gc) {
    mjs_gc(mjs, 0);
    return 1;
  }
  return 0;
}




static void gc_mark_val_array(struct mjs *mjs, mjs_val_t *vals, size_t len) {
  mjs_val_t *vp;
  for (vp = vals; vp < vals + len; vp++) {
    gc_mark(mjs, vp);
  }
}




static void gc_mark_mbuf_pt(struct mjs *mjs, const struct mbuf *mbuf) {
  mjs_val_t **vp;
  for (vp = (mjs_val_t **) mbuf->buf; (char *) vp < mbuf->buf + mbuf->len;
       vp++) {
    gc_mark(mjs, *vp);
  }
}




static void gc_mark_mbuf_val(struct mjs *mjs, const struct mbuf *mbuf) {
  gc_mark_val_array(mjs, (mjs_val_t *) mbuf->buf,
                    mbuf->len / sizeof(mjs_val_t));
}

static void gc_mark_ffi_cbargs_list(struct mjs *mjs, ffi_cb_args_t *cbargs) {
  for (; cbargs != 
                  __null
                      ; cbargs = cbargs->next) {
    gc_mark(mjs, &cbargs->func);
    gc_mark(mjs, &cbargs->userdata);
  }
}


void mjs_gc(struct mjs *mjs, int full) {
  gc_mark_val_array(mjs, (mjs_val_t *) &mjs->vals,
                    sizeof(mjs->vals) / sizeof(mjs_val_t));

  gc_mark_mbuf_pt(mjs, &mjs->owned_values);
  gc_mark_mbuf_val(mjs, &mjs->scopes);
  gc_mark_mbuf_val(mjs, &mjs->stack);
  gc_mark_mbuf_val(mjs, &mjs->call_stack);

  gc_mark_ffi_cbargs_list(mjs, mjs->ffi_cb_args);

  gc_compact_strings(mjs);

  gc_sweep(mjs, &mjs->object_arena, 0);
  gc_sweep(mjs, &mjs->property_arena, 0);
  gc_sweep(mjs, &mjs->ffi_sig_arena, 0);

  if (full) {





    size_t trimmed_size = mjs->owned_strings.len + 100;
    if (trimmed_size < mjs->owned_strings.size) {
      mbuf_resize(&mjs->owned_strings, trimmed_size);
    }
  }
}

static int gc_check_val(struct mjs *mjs, mjs_val_t v) {
  if (mjs_is_object(v)) {
    return gc_check_ptr(&mjs->object_arena, get_object_struct(v));
  }
  if (mjs_is_ffi_sig(v)) {
    return gc_check_ptr(&mjs->ffi_sig_arena, mjs_get_ffi_sig_struct(v));
  }
  return 1;
}

static int gc_check_ptr(const struct gc_arena *a, const void *ptr) {
  const struct gc_cell *p = (const struct gc_cell *) ptr;
  struct gc_block *b;
  for (b = a->blocks; b != 
                          __null
                              ; b = b->next) {
    if (p >= b->base && p < ((struct gc_cell *) (((char *) (b->base)) +((b->size) * (a)->cell_size)))) {
      return 1;
    }
  }
  return 0;
}
static int should_skip_for_json(enum mjs_type type) {
  int ret;
  switch (type) {

    case MJS_TYPE_NULL:
    case MJS_TYPE_BOOLEAN:
    case MJS_TYPE_NUMBER:
    case MJS_TYPE_STRING:
    case MJS_TYPE_OBJECT_GENERIC:
    case MJS_TYPE_OBJECT_ARRAY:
      ret = 0;
      break;
    default:
      ret = 1;
      break;
  }
  return ret;
}

static const char *hex_digits = "0123456789abcdef";
static char *append_hex(char *buf, char *limit, uint8_t c) {
  if (buf < limit) *buf++ = 'u';
  if (buf < limit) *buf++ = '0';
  if (buf < limit) *buf++ = '0';
  if (buf < limit) *buf++ = hex_digits[(int) ((c >> 4) % 0xf)];
  if (buf < limit) *buf++ = hex_digits[(int) (c & 0xf)];
  return buf;
}







static int snquote(char *buf, size_t size, const char *s, size_t len) {
  char *limit = buf + size;
  const char *end;
  const char *specials = "btnvfr";
  size_t i = 0;

  i++;
  if (buf < limit) *buf++ = '"';

  for (end = s + len; s < end; s++) {
    if (*s == '"' || *s == '\\') {
      i++;
      if (buf < limit) *buf++ = '\\';
    } else if (*s >= '\b' && *s <= '\r') {
      i += 2;
      if (buf < limit) *buf++ = '\\';
      if (buf < limit) *buf++ = specials[*s - '\b'];
      continue;
    } else if ((unsigned char) *s < '\b' || (*s > '\r' && *s < ' ')) {
      i += 6 ;
      if (buf < limit) *buf++ = '\\';
      buf = append_hex(buf, limit, (uint8_t) *s);
      continue;
    }
    i++;
    if (buf < limit) *buf++ = *s;
  }

  i++;
  if (buf < limit) *buf++ = '"';

  if (buf < limit) {
    *buf = '\0';
  } else if (size != 0) {




    *(buf - 1) = '\0';
  }
  return i;
}

static mjs_err_t to_json_or_debug(struct mjs *mjs, mjs_val_t v, char *buf,
                                       size_t size, size_t *res_len,
                                       uint8_t is_debug) {
  mjs_val_t el;
  char *vp;
  mjs_err_t rcode = MJS_OK;
  size_t len = 0;




  if (size > 0) *buf = '\0';

  if (!is_debug && should_skip_for_json(mjs_get_type(v))) {
    goto clean;
  }

  for (vp = mjs->json_visited_stack.buf;
       vp < mjs->json_visited_stack.buf + mjs->json_visited_stack.len;
       vp += sizeof(mjs_val_t)) {
    if (*(mjs_val_t *) vp == v) {
      strncpy(buf, "[Circular]", size);
      len = 10;
      goto clean;
    }
  }

  switch (mjs_get_type(v)) {
    case MJS_TYPE_NULL:
    case MJS_TYPE_BOOLEAN:
    case MJS_TYPE_NUMBER:
    case MJS_TYPE_UNDEFINED:
    case MJS_TYPE_FOREIGN:

      {

        char *p = 
                 __null
                     ;
        int need_free = 0;
        rcode = mjs_to_string(mjs, &v, &p, &len, &need_free);
        c_snprintf(buf, size, "%.*s", (int) len, p);
        if (need_free) {
          free(p);
        }
      }
      goto clean;

    case MJS_TYPE_STRING: {




      size_t n;
      const char *str = mjs_get_string(mjs, &v, &n);
      len = snquote(buf, size, str, n);
      goto clean;
    }

    case MJS_TYPE_OBJECT_FUNCTION:
    case MJS_TYPE_OBJECT_GENERIC: {
      char *b = buf;
      struct mjs_property *prop = 
                                 __null
                                     ;
      struct mjs_object *o = 
                            __null
                                ;

      mbuf_append(&mjs->json_visited_stack, (char *) &v, sizeof(v));
      b += c_snprintf(b, (((size_t)(b - buf) < (size)) ? ((size) - (b - buf)) : 0), "{");
      o = get_object_struct(v);
      for (prop = o->properties; prop != 
                                        __null
                                            ; prop = prop->next) {
        size_t n;
        const char *s;
        if (!is_debug && should_skip_for_json(mjs_get_type(prop->value))) {
          continue;
        }
        if (b - buf != 1) {
          b += c_snprintf(b, (((size_t)(b - buf) < (size)) ? ((size) - (b - buf)) : 0), ",");
        }
        s = mjs_get_string(mjs, &prop->name, &n);
        b += c_snprintf(b, (((size_t)(b - buf) < (size)) ? ((size) - (b - buf)) : 0), "\"%.*s\":", (int) n, s);
        {
          size_t tmp = 0;
          rcode = to_json_or_debug(mjs, prop->value, b, (((size_t)(b - buf) < (size)) ? ((size) - (b - buf)) : 0),
                                   &tmp, is_debug);
          if (rcode != MJS_OK) {
            goto clean_iter;
          }
          b += tmp;
        }
      }

      b += c_snprintf(b, (((size_t)(b - buf) < (size)) ? ((size) - (b - buf)) : 0), "}");
      mjs->json_visited_stack.len -= sizeof(v);

    clean_iter:
      len = b - buf;
      goto clean;
    }
    case MJS_TYPE_OBJECT_ARRAY: {
      int has;
      char *b = buf;
      size_t i, alen = mjs_array_length(mjs, v);
      mbuf_append(&mjs->json_visited_stack, (char *) &v, sizeof(v));
      b += c_snprintf(b, (((size_t)(b - buf) < (size)) ? ((size) - (b - buf)) : 0), "[");
      for (i = 0; i < alen; i++) {
        el = mjs_array_get2(mjs, v, i, &has);
        if (has) {
          size_t tmp = 0;
          if (!is_debug && should_skip_for_json(mjs_get_type(el))) {
            b += c_snprintf(b, (((size_t)(b - buf) < (size)) ? ((size) - (b - buf)) : 0), "null");
          } else {
            rcode = to_json_or_debug(mjs, el, b, (((size_t)(b - buf) < (size)) ? ((size) - (b - buf)) : 0), &tmp,
                                     is_debug);
            if (rcode != MJS_OK) {
              goto clean;
            }
          }
          b += tmp;
        } else {
          b += c_snprintf(b, (((size_t)(b - buf) < (size)) ? ((size) - (b - buf)) : 0), "null");
        }
        if (i != alen - 1) {
          b += c_snprintf(b, (((size_t)(b - buf) < (size)) ? ((size) - (b - buf)) : 0), ",");
        }
      }
      b += c_snprintf(b, (((size_t)(b - buf) < (size)) ? ((size) - (b - buf)) : 0), "]");
      mjs->json_visited_stack.len -= sizeof(v);
      len = b - buf;
      goto clean;
    }

    case MJS_TYPES_CNT:
      abort();
  }

  abort();

  len = 0;
  goto clean;

clean:
  if (rcode != MJS_OK) {
    len = 0;
  }
  if (res_len != 
                __null
                    ) {
    *res_len = len;
  }
  return rcode;
}

static mjs_err_t mjs_json_stringify(struct mjs *mjs, mjs_val_t v,
                                         char *buf, size_t size, char **res) {
  mjs_err_t rcode = MJS_OK;
  char *p = buf;
  size_t len;

  to_json_or_debug(mjs, v, buf, size, &len, 0);

  if (len >= size) {

    p = (char *) malloc(len + 1);
    rcode = mjs_json_stringify(mjs, v, p, len + 1, res);
    
   (static_cast <bool> (
   *res == p
   ) ? void (0) : __assert_fail (
   "*res == p"
   , "mjs.c", 7362, __extension__ __PRETTY_FUNCTION__))
                    ;
    goto clean;
  } else {
    *res = p;
    goto clean;
  }

clean:




  if (rcode != MJS_OK && p != buf) {
    free(p);
  }
  return rcode;
}





struct json_parse_frame {
  mjs_val_t val;
  struct json_parse_frame *up;
};




struct json_parse_ctx {
  struct mjs *mjs;
  mjs_val_t result;
  struct json_parse_frame *frame;
  enum mjs_err rcode;
};


static struct json_parse_frame *alloc_json_frame(struct json_parse_ctx *ctx,
                                                 mjs_val_t v) {
  struct json_parse_frame *frame =
      (struct json_parse_frame *) calloc(sizeof(struct json_parse_frame), 1);
  frame->val = v;
  mjs_own(ctx->mjs, &frame->val);
  return frame;
}


static struct json_parse_frame *free_json_frame(
    struct json_parse_ctx *ctx, struct json_parse_frame *frame) {
  struct json_parse_frame *up = frame->up;
  mjs_disown(ctx->mjs, &frame->val);
  free(frame);
  return up;
}


static void frozen_cb(void *data, const char *name, size_t name_len,
                      const char *path, const struct json_token *token) {
  struct json_parse_ctx *ctx = (struct json_parse_ctx *) data;
  mjs_val_t v = ((uint64_t)(1) << 63 | (uint64_t) 0x7ff0 << 48 | (uint64_t)(3) << 48);

  (void) path;

  mjs_own(ctx->mjs, &v);

  switch (token->type) {
    case JSON_TYPE_STRING: {
      char *dst;
      if (token->len > 0 && (dst = (char*)malloc(token->len)) != 
                                                                __null
                                                                    ) {
        int len = json_unescape(token->ptr, token->len, dst, token->len);
        if (len < 0) {
          mjs_prepend_errorf(ctx->mjs, MJS_TYPE_ERROR, "invalid JSON string");
          break;
        }
        v = mjs_mk_string(ctx->mjs, dst, len, 1 );
        free(dst);
      } else {




        v = mjs_mk_string(ctx->mjs, "", 0, 1 );
      }
      break;
    }
    case JSON_TYPE_NUMBER:
      v = mjs_mk_number(ctx->mjs, strtod(token->ptr, 
                                                    __null
                                                        ));
      break;
    case JSON_TYPE_TRUE:
      v = mjs_mk_boolean(ctx->mjs, 1);
      break;
    case JSON_TYPE_FALSE:
      v = mjs_mk_boolean(ctx->mjs, 0);
      break;
    case JSON_TYPE_NULL:
      v = ((uint64_t)(1) << 63 | (uint64_t) 0x7ff0 << 48 | (uint64_t)(15) << 48);
      break;
    case JSON_TYPE_OBJECT_START:
      v = mjs_mk_object(ctx->mjs);
      break;
    case JSON_TYPE_ARRAY_START:
      v = mjs_mk_array(ctx->mjs);
      break;

    case JSON_TYPE_OBJECT_END:
    case JSON_TYPE_ARRAY_END: {

      ctx->frame = free_json_frame(ctx, ctx->frame);
    } break;

    default:
      do { if (cs_log_print_prefix(LL_ERROR, "mjs.c", 7474)) { cs_log_printf ("Wrong token type %d\n", token->type); } } while (0);
      break;
  }

  if (!mjs_is_undefined(v)) {
    if (name != 
               __null 
                    && name_len != 0) {

      if (mjs_is_object(ctx->frame->val)) {
        mjs_set(ctx->mjs, ctx->frame->val, name, name_len, v);
      } else if (mjs_is_array(ctx->frame->val)) {




        int idx = (int) strtod(name, 
                                    __null
                                        );
        mjs_array_set(ctx->mjs, ctx->frame->val, idx, v);
      } else {
        do { if (cs_log_print_prefix(LL_ERROR, "mjs.c", 7491)) { cs_log_printf ("Current value is neither object nor array\n"); } } while (0);
      }
    } else {

      
     (static_cast <bool> (
     ctx->frame == 
     __null) ? void (0) : __assert_fail (
     "ctx->frame == NULL"
     , "mjs.c", 7495, __extension__ __PRETTY_FUNCTION__))
                               ;





      ctx->result = v;
    }

    if (token->type == JSON_TYPE_OBJECT_START ||
        token->type == JSON_TYPE_ARRAY_START) {


      struct json_parse_frame *new_frame = alloc_json_frame(ctx, v);
      new_frame->up = ctx->frame;
      ctx->frame = new_frame;
    }
  }

  mjs_disown(ctx->mjs, &v);
}

static mjs_err_t
mjs_json_parse(struct mjs *mjs, const char *str, size_t len, mjs_val_t *res) {
  struct json_parse_ctx *ctx =
      (struct json_parse_ctx *) calloc(sizeof(struct json_parse_ctx), 1);
  int json_res;
  enum mjs_err rcode = MJS_OK;

  ctx->mjs = mjs;
  ctx->result = ((uint64_t)(1) << 63 | (uint64_t) 0x7ff0 << 48 | (uint64_t)(3) << 48);
  ctx->frame = 
              __null
                  ;
  ctx->rcode = MJS_OK;

  mjs_own(mjs, &ctx->result);

  {





    char *stmp = (char*)malloc(len);
    memcpy(stmp, str, len);
    json_res = json_walk(stmp, len, frozen_cb, ctx);
    free(stmp);
    stmp = 
          __null
              ;


    str = 
         __null
             ;
  }

  if (ctx->rcode != MJS_OK) {
    rcode = ctx->rcode;
    mjs_prepend_errorf(mjs, rcode, "invalid JSON string");
  } else if (json_res < 0) {

    rcode = MJS_TYPE_ERROR;
    mjs_prepend_errorf(mjs, rcode, "invalid JSON string");
  } else {

    *res = ctx->result;


    
   (static_cast <bool> (
   ctx->frame == 
   __null) ? void (0) : __assert_fail (
   "ctx->frame == NULL"
   , "mjs.c", 7559, __extension__ __PRETTY_FUNCTION__))
                             ;
  }

  if (rcode != MJS_OK) {

    while (ctx->frame != 
                        __null
                            ) {
      ctx->frame = free_json_frame(ctx, ctx->frame);
    }
  }

  mjs_disown(mjs, &ctx->result);
  free(ctx);

  return rcode;
}

static void mjs_op_json_stringify(struct mjs *mjs) {
  mjs_val_t ret = ((uint64_t)(1) << 63 | (uint64_t) 0x7ff0 << 48 | (uint64_t)(3) << 48);
  mjs_val_t val = mjs_arg(mjs, 0);

  if (mjs_nargs(mjs) < 1) {
    mjs_prepend_errorf(mjs, MJS_TYPE_ERROR, "missing a value to stringify");
  } else {
    char *p = 
             __null
                 ;
    if (mjs_json_stringify(mjs, val, 
                                    __null
                                        , 0, &p) == MJS_OK) {
      ret = mjs_mk_string(mjs, p, ~0, 1 );
      free(p);
    }
  }

  mjs_return(mjs, ret);
}

static void mjs_op_json_parse(struct mjs *mjs) {
  mjs_val_t ret = ((uint64_t)(1) << 63 | (uint64_t) 0x7ff0 << 48 | (uint64_t)(3) << 48);
  mjs_val_t arg0 = mjs_arg(mjs, 0);

  if (mjs_is_string(arg0)) {
    size_t len;
    const char *str = mjs_get_string(mjs, &arg0, &len);
    mjs_json_parse(mjs, str, len, &ret);
  } else {
    mjs_prepend_errorf(mjs, MJS_TYPE_ERROR, "string argument required");
  }

  mjs_return(mjs, ret);
}
static mjs_val_t mjs_object_to_value(struct mjs_object *o) {
  if (o == 
          __null
              ) {
    return ((uint64_t)(1) << 63 | (uint64_t) 0x7ff0 << 48 | (uint64_t)(15) << 48);
  } else {
    return mjs_legit_pointer_to_value(o) | ((uint64_t)(1) << 63 | (uint64_t) 0x7ff0 << 48 | (uint64_t)(1) << 48);
  }
}

static struct mjs_object *get_object_struct(mjs_val_t v) {
  struct mjs_object *ret = 
                          __null
                              ;
  if (mjs_is_null(v)) {
    ret = 
         __null
             ;
  } else {
    
   (static_cast <bool> (
   mjs_is_object(v)
   ) ? void (0) : __assert_fail (
   "mjs_is_object(v)"
   , "mjs.c", 7700, __extension__ __PRETTY_FUNCTION__))
                           ;
    ret = (struct mjs_object *) get_ptr(v);
  }
  return ret;
}

mjs_val_t mjs_mk_object(struct mjs *mjs) {
  struct mjs_object *o = new_object(mjs);
  if (o == 
          __null
              ) {
    return ((uint64_t)(1) << 63 | (uint64_t) 0x7ff0 << 48 | (uint64_t)(15) << 48);
  }
  (void) mjs;
  o->properties = 
                 __null
                     ;
  return mjs_object_to_value(o);
}

int mjs_is_object(mjs_val_t v) {
  return (v & ((uint64_t)(1) << 63 | (uint64_t) 0x7ff0 << 48 | (uint64_t)(15) << 48)) == ((uint64_t)(1) << 63 | (uint64_t) 0x7ff0 << 48 | (uint64_t)(1) << 48) ||
         (v & ((uint64_t)(1) << 63 | (uint64_t) 0x7ff0 << 48 | (uint64_t)(15) << 48)) == ((uint64_t)(1) << 63 | (uint64_t) 0x7ff0 << 48 | (uint64_t)(12) << 48);
}

static struct mjs_property *mjs_get_own_property(struct mjs *mjs,
                                                      mjs_val_t obj,
                                                      const char *name,
                                                      size_t len) {
  struct mjs_property *p;
  struct mjs_object *o;

  if (!mjs_is_object(obj)) {
    return 
          __null
              ;
  }

  o = get_object_struct(obj);

  if (len <= 5) {
    mjs_val_t ss = mjs_mk_string(mjs, name, len, 1);
    for (p = o->properties; p != 
                                __null
                                    ; p = p->next) {
      if (p->name == ss) return p;
    }
  } else {
    for (p = o->properties; p != 
                                __null
                                    ; p = p->next) {
      if (mjs_strcmp(mjs, &p->name, name, len) == 0) return p;
    }
    return p;
  }

  return 
        __null
            ;
}

static struct mjs_property *mjs_get_own_property_v(struct mjs *mjs,
                                                        mjs_val_t obj,
                                                        mjs_val_t key) {
  size_t n;
  char *s = 
           __null
               ;
  int need_free = 0;
  struct mjs_property *p = 
                          __null
                              ;
  mjs_err_t err = mjs_to_string(mjs, &key, &s, &n, &need_free);
  if (err == MJS_OK) {
    p = mjs_get_own_property(mjs, obj, s, n);
  }
  if (need_free) free(s);
  return p;
}

static struct mjs_property *mjs_mk_property(struct mjs *mjs,
                                                 mjs_val_t name,
                                                 mjs_val_t value) {
  struct mjs_property *p = new_property(mjs);
  p->next = 
           __null
               ;
  p->name = name;
  p->value = value;
  return p;
}

mjs_val_t mjs_get(struct mjs *mjs, mjs_val_t obj, const char *name,
                  size_t name_len) {
  struct mjs_property *p;

  if (name_len == (size_t) ~0) {
    name_len = strlen(name);
  }

  p = mjs_get_own_property(mjs, obj, name, name_len);
  if (p == 
          __null
              ) {
    return ((uint64_t)(1) << 63 | (uint64_t) 0x7ff0 << 48 | (uint64_t)(3) << 48);
  } else {
    return p->value;
  }
}

mjs_val_t mjs_get_v(struct mjs *mjs, mjs_val_t obj, mjs_val_t name) {
  size_t n;
  char *s = 
           __null
               ;
  int need_free = 0;
  mjs_val_t ret = ((uint64_t)(1) << 63 | (uint64_t) 0x7ff0 << 48 | (uint64_t)(3) << 48);

  mjs_err_t err = mjs_to_string(mjs, &name, &s, &n, &need_free);

  if (err == MJS_OK) {

    ret = mjs_get(mjs, obj, s, n);
  }

  if (need_free) {
    free(s);
    s = 
       __null
           ;
  }
  return ret;
}

mjs_val_t mjs_get_v_proto(struct mjs *mjs, mjs_val_t obj, mjs_val_t key) {
  struct mjs_property *p;
  mjs_val_t pn = mjs_mk_string(mjs, "__p", ~0, 1);
  if ((p = mjs_get_own_property_v(mjs, obj, key)) != 
                                                    __null
                                                        ) return p->value;
  if ((p = mjs_get_own_property_v(mjs, obj, pn)) == 
                                                   __null
                                                       ) return ((uint64_t)(1) << 63 | (uint64_t) 0x7ff0 << 48 | (uint64_t)(3) << 48);
  return mjs_get_v_proto(mjs, p->value, key);
}

mjs_err_t mjs_set(struct mjs *mjs, mjs_val_t obj, const char *name,
                  size_t name_len, mjs_val_t val) {
  return mjs_set_internal(mjs, obj, ((uint64_t)(1) << 63 | (uint64_t) 0x7ff0 << 48 | (uint64_t)(3) << 48), (char *) name, name_len,
                          val);
}

mjs_err_t mjs_set_v(struct mjs *mjs, mjs_val_t obj, mjs_val_t name,
                    mjs_val_t val) {
  return mjs_set_internal(mjs, obj, name, 
                                         __null
                                             , 0, val);
}

static mjs_err_t mjs_set_internal(struct mjs *mjs, mjs_val_t obj,
                                       mjs_val_t name_v, char *name,
                                       size_t name_len, mjs_val_t val) {
  mjs_err_t rcode = MJS_OK;

  struct mjs_property *p;

  int need_free = 0;

  if (name == 
             __null
                 ) {

    rcode = mjs_to_string(mjs, &name_v, &name, &name_len, &need_free);
    if (rcode != MJS_OK) {
      goto clean;
    }
  } else {




    name_v = ((uint64_t)(1) << 63 | (uint64_t) 0x7ff0 << 48 | (uint64_t)(3) << 48);
  }

  p = mjs_get_own_property(mjs, obj, name, name_len);

  if (p == 
          __null
              ) {
    struct mjs_object *o;
    if (!mjs_is_object(obj)) {
      return MJS_REFERENCE_ERROR;
    }





    if (!mjs_is_string(name_v)) {
      name_v = mjs_mk_string(mjs, name, name_len, 1);
    }

    p = mjs_mk_property(mjs, name_v, val);

    o = get_object_struct(obj);
    p->next = o->properties;
    o->properties = p;
  }

  p->value = val;

clean:
  if (need_free) {
    free(name);
    name = 
          __null
              ;
  }
  return rcode;
}

static void mjs_destroy_property(struct mjs_property **p) {
  *p = 
      __null
          ;
}




int mjs_del(struct mjs *mjs, mjs_val_t obj, const char *name, size_t len) {
  struct mjs_property *prop, *prev;

  if (!mjs_is_object(obj)) {
    return -1;
  }
  if (len == (size_t) ~0) {
    len = strlen(name);
  }
  for (prev = 
             __null
                 , prop = get_object_struct(obj)->properties; prop != 
                                                                      __null
                                                                          ;
       prev = prop, prop = prop->next) {
    size_t n;
    const char *s = mjs_get_string(mjs, &prop->name, &n);
    if (n == len && strncmp(s, name, len) == 0) {
      if (prev) {
        prev->next = prop->next;
      } else {
        get_object_struct(obj)->properties = prop->next;
      }
      mjs_destroy_property(&prop);
      return 0;
    }
  }
  return -1;
}

mjs_val_t mjs_next(struct mjs *mjs, mjs_val_t obj, mjs_val_t *iterator) {
  struct mjs_property *p = 
                          __null
                              ;
  mjs_val_t key = ((uint64_t)(1) << 63 | (uint64_t) 0x7ff0 << 48 | (uint64_t)(3) << 48);

  if (*iterator == ((uint64_t)(1) << 63 | (uint64_t) 0x7ff0 << 48 | (uint64_t)(3) << 48)) {
    struct mjs_object *o = get_object_struct(obj);
    p = o->properties;
  } else {
    p = ((struct mjs_property *) get_ptr(*iterator))->next;
  }

  if (p == 
          __null
              ) {
    *iterator = ((uint64_t)(1) << 63 | (uint64_t) 0x7ff0 << 48 | (uint64_t)(3) << 48);
  } else {
    key = p->name;
    *iterator = mjs_mk_foreign(mjs, p);
  }

  return key;
}

static void mjs_op_create_object(struct mjs *mjs) {
  mjs_val_t ret = ((uint64_t)(1) << 63 | (uint64_t) 0x7ff0 << 48 | (uint64_t)(3) << 48);
  mjs_val_t proto_v = mjs_arg(mjs, 0);

  if (!mjs_check_arg(mjs, 0, "proto", MJS_TYPE_OBJECT_GENERIC, &proto_v)) {
    goto clean;
  }

  ret = mjs_mk_object(mjs);
  mjs_set(mjs, ret, "__p", ~0, proto_v);

clean:
  mjs_return(mjs, ret);
}

mjs_val_t mjs_struct_to_obj(struct mjs *mjs, const void *base,
                            const struct mjs_c_struct_member *defs) {
  mjs_val_t obj;
  const struct mjs_c_struct_member *def = defs;
  if (base == 
             __null 
                  || def == 
                            __null
                                ) return ((uint64_t)(1) << 63 | (uint64_t) 0x7ff0 << 48 | (uint64_t)(3) << 48);
  obj = mjs_mk_object(mjs);

  mjs_own(mjs, &obj);





  while (def->name != 
                     __null
                         ) def++;
  for (def--; def >= defs; def--) {
    mjs_val_t v = ((uint64_t)(1) << 63 | (uint64_t) 0x7ff0 << 48 | (uint64_t)(3) << 48);
    const char *ptr = (const char *) base + def->offset;
    switch (def->type) {
      case MJS_STRUCT_FIELD_TYPE_STRUCT: {
        const void *sub_base = (const void *) ptr;
        const struct mjs_c_struct_member *sub_def =
            (const struct mjs_c_struct_member *) def->arg;
        v = mjs_struct_to_obj(mjs, sub_base, sub_def);
        break;
      }
      case MJS_STRUCT_FIELD_TYPE_STRUCT_PTR: {
        const void **sub_base = (const void **) ptr;
        const struct mjs_c_struct_member *sub_def =
            (const struct mjs_c_struct_member *) def->arg;
        if (*sub_base != 
                        __null
                            ) {
          v = mjs_struct_to_obj(mjs, *sub_base, sub_def);
        } else {
          v = ((uint64_t)(1) << 63 | (uint64_t) 0x7ff0 << 48 | (uint64_t)(15) << 48);
        }
        break;
      }
      case MJS_STRUCT_FIELD_TYPE_INT: {
        double value = (double) (*(int *) ptr);
        v = mjs_mk_number(mjs, value);
        break;
      }
      case MJS_STRUCT_FIELD_TYPE_BOOL: {
        v = mjs_mk_boolean(mjs, *(bool *) ptr);
        break;
      }
      case MJS_STRUCT_FIELD_TYPE_DOUBLE: {
        v = mjs_mk_number(mjs, *(double *) ptr);
        break;
      }
      case MJS_STRUCT_FIELD_TYPE_FLOAT: {
        float value = *(float *) ptr;
        v = mjs_mk_number(mjs, value);
        break;
      }
      case MJS_STRUCT_FIELD_TYPE_CHAR_PTR: {
        const char *value = *(const char **) ptr;
        v = mjs_mk_string(mjs, value, ~0, 1);
        break;
      }
      case MJS_STRUCT_FIELD_TYPE_VOID_PTR: {
        v = mjs_mk_foreign(mjs, *(void **) ptr);
        break;
      }
      case MJS_STRUCT_FIELD_TYPE_MG_STR_PTR: {
        const struct mg_str *s = *(const struct mg_str **) ptr;
        if (s != 
                __null
                    ) {
          v = mjs_mk_string(mjs, s->p, s->len, 1);
        } else {
          v = ((uint64_t)(1) << 63 | (uint64_t) 0x7ff0 << 48 | (uint64_t)(15) << 48);
        }
        break;
      }
      case MJS_STRUCT_FIELD_TYPE_MG_STR: {
        const struct mg_str *s = (const struct mg_str *) ptr;
        v = mjs_mk_string(mjs, s->p, s->len, 1);
        break;
      }
      case MJS_STRUCT_FIELD_TYPE_DATA: {
        const char *dptr = (const char *) ptr;
        const intptr_t dlen = (intptr_t) def->arg;
        v = mjs_mk_string(mjs, dptr, dlen, 1);
        break;
      }
      case MJS_STRUCT_FIELD_TYPE_INT8: {
        double value = (double) (*(int8_t *) ptr);
        v = mjs_mk_number(mjs, value);
        break;
      }
      case MJS_STRUCT_FIELD_TYPE_INT16: {
        double value = (double) (*(int16_t *) ptr);
        v = mjs_mk_number(mjs, value);
        break;
      }
      case MJS_STRUCT_FIELD_TYPE_UINT8: {
        double value = (double) (*(uint8_t *) ptr);
        v = mjs_mk_number(mjs, value);
        break;
      }
      case MJS_STRUCT_FIELD_TYPE_UINT16: {
        double value = (double) (*(uint16_t *) ptr);
        v = mjs_mk_number(mjs, value);
        break;
      }
      case MJS_STRUCT_FIELD_TYPE_CUSTOM: {
        mjs_val_t (*fptr)(struct mjs *, const void *) =
            (mjs_val_t (*) (struct mjs *, const void *)) def->arg;
        v = fptr(mjs, ptr);
      }
      default: { break; }
    }
    mjs_set(mjs, obj, def->name, ~0, v);
  }
  mjs_disown(mjs, &obj);
  return obj;
}
static mjs_err_t parse_statement(struct pstate *p);
static mjs_err_t parse_expr(struct pstate *p);

static int ptest(struct pstate *p) {
  struct pstate saved = *p;
  int tok = pnext(p);
  *p = saved;
  return tok;
}

static int s_unary_ops[] = {TOK_NOT, TOK_TILDA, TOK_PLUS_PLUS, TOK_MINUS_MINUS,
                            TOK_KEYWORD_TYPEOF, TOK_MINUS, TOK_PLUS, TOK_EOF};
static int s_comparison_ops[] = {TOK_LT, TOK_LE, TOK_GT, TOK_GE, TOK_EOF};
static int s_postfix_ops[] = {TOK_PLUS_PLUS, TOK_MINUS_MINUS, TOK_EOF};
static int s_equality_ops[] = {TOK_EQ, TOK_NE, TOK_EQ_EQ, TOK_NE_NE, TOK_EOF};
static int s_assign_ops[] = {
    TOK_ASSIGN, TOK_PLUS_ASSIGN, TOK_MINUS_ASSIGN, TOK_MUL_ASSIGN,
    TOK_DIV_ASSIGN, TOK_REM_ASSIGN, TOK_LSHIFT_ASSIGN, TOK_RSHIFT_ASSIGN,
    TOK_URSHIFT_ASSIGN, TOK_AND_ASSIGN, TOK_XOR_ASSIGN, TOK_OR_ASSIGN,
    TOK_EOF};

static int findtok(int *toks, int tok) {
  int i = 0;
  while (tok != toks[i] && toks[i] != TOK_EOF) i++;
  return toks[i];
}

static void emit_op(struct pstate *pstate, int tok) {
  
 (static_cast <bool> (
 tok >= 0 && tok <= 255
 ) ? void (0) : __assert_fail (
 "tok >= 0 && tok <= 255"
 , "mjs.c", 8139, __extension__ __PRETTY_FUNCTION__))
                               ;
  emit_byte(pstate, OP_EXPR);
  emit_byte(pstate, (uint8_t) tok);
}
static void emit_init_offset(struct pstate *p) {
  size_t i;
  for (i = 0; i < 1; i++) {
    emit_byte(p, 0);
  }
}






static mjs_err_t parse_statement_list(struct pstate *p, int et) {
  mjs_err_t res = MJS_OK;
  int drop = 0;
  do { do { if (cs_log_print_prefix(LL_VERBOSE_DEBUG, "mjs.c", 8218)) { cs_log_printf ("  PNEXT %d", 8218); } } while (0); pnext(p); } while (0);
  while (res == MJS_OK && p->tok.tok != TOK_EOF && p->tok.tok != et) {
    if (drop) emit_byte(p, OP_DROP);
    res = parse_statement(p);
    drop = 1;
    while (p->tok.tok == TOK_SEMICOLON) do { do { if (cs_log_print_prefix(LL_VERBOSE_DEBUG, "mjs.c", 8223)) { cs_log_printf ("  PNEXT %d", 8223); } } while (0); pnext(p); } while (0);
  }





  if (!drop) {
    emit_byte(p, OP_PUSH_UNDEF);
  }
  return res;
}

static mjs_err_t parse_block(struct pstate *p, int mkscope) {
  mjs_err_t res = MJS_OK;
  p->depth++;
  if (p->depth > (8192 / 16)) {
    mjs_set_errorf(p->mjs, MJS_SYNTAX_ERROR, "parser stack overflow");
    res = MJS_SYNTAX_ERROR;
    return res;
  }
  do { if (cs_log_print_prefix(LL_VERBOSE_DEBUG, "mjs.c", 8244)) { cs_log_printf ("[%.*s]", 10, p->tok.ptr); } } while (0);
  if (mkscope) emit_byte(p, OP_NEW_SCOPE);
  res = parse_statement_list(p, TOK_CLOSE_CURLY);
  if ((p)->tok.tok != (TOK_CLOSE_CURLY)) do { mjs_set_errorf(p->mjs, MJS_SYNTAX_ERROR, "parse error at line %d: [%.*s]", p->line_no, 10, p->tok.ptr); return MJS_SYNTAX_ERROR; } while (0); else do { do { if (cs_log_print_prefix(LL_VERBOSE_DEBUG, "mjs.c", 8247)) { cs_log_printf ("  PNEXT %d", 8247); } } while (0); pnext(p); } while (0);;
  if (mkscope) emit_byte(p, OP_DEL_SCOPE);
  return res;
}

static mjs_err_t parse_function(struct pstate *p) {
  size_t prologue, off;
  int arg_no = 0;
  int name_provided = 0;
  mjs_err_t res = MJS_OK;

  if ((p)->tok.tok != (TOK_KEYWORD_FUNCTION)) do { mjs_set_errorf(p->mjs, MJS_SYNTAX_ERROR, "parse error at line %d: [%.*s]", p->line_no, 10, p->tok.ptr); return MJS_SYNTAX_ERROR; } while (0); else do { do { if (cs_log_print_prefix(LL_VERBOSE_DEBUG, "mjs.c", 8258)) { cs_log_printf ("  PNEXT %d", 8258); } } while (0); pnext(p); } while (0);;

  if (p->tok.tok == TOK_IDENT) {

    struct tok tmp = p->tok;
    name_provided = 1;
    emit_byte(p, OP_PUSH_STR);
    emit_str(p, tmp.ptr, tmp.len);
    emit_byte(p, OP_PUSH_SCOPE);
    emit_byte(p, OP_CREATE);
    emit_byte(p, OP_PUSH_STR);
    emit_str(p, tmp.ptr, tmp.len);
    emit_byte(p, OP_FIND_SCOPE);
    do { do { if (cs_log_print_prefix(LL_VERBOSE_DEBUG, "mjs.c", 8271)) { cs_log_printf ("  PNEXT %d", 8271); } } while (0); pnext(p); } while (0);
  }

  emit_byte(p, OP_JMP);
  off = p->cur_idx;
  emit_init_offset(p);

  prologue = p->cur_idx;

  if ((p)->tok.tok != (TOK_OPEN_PAREN)) do { mjs_set_errorf(p->mjs, MJS_SYNTAX_ERROR, "parse error at line %d: [%.*s]", p->line_no, 10, p->tok.ptr); return MJS_SYNTAX_ERROR; } while (0); else do { do { if (cs_log_print_prefix(LL_VERBOSE_DEBUG, "mjs.c", 8280)) { cs_log_printf ("  PNEXT %d", 8280); } } while (0); pnext(p); } while (0);;
  emit_byte(p, OP_NEW_SCOPE);

  while (p->tok.tok != TOK_CLOSE_PAREN) {
    if (p->tok.tok != TOK_IDENT) do { mjs_set_errorf(p->mjs, MJS_SYNTAX_ERROR, "parse error at line %d: [%.*s]", p->line_no, 10, p->tok.ptr); return MJS_SYNTAX_ERROR; } while (0);
    emit_byte(p, OP_SET_ARG);
    emit_int(p, arg_no);
    arg_no++;
    emit_str(p, p->tok.ptr, p->tok.len);
    if (ptest(p) == TOK_COMMA) do { do { if (cs_log_print_prefix(LL_VERBOSE_DEBUG, "mjs.c", 8289)) { cs_log_printf ("  PNEXT %d", 8289); } } while (0); pnext(p); } while (0);
    do { do { if (cs_log_print_prefix(LL_VERBOSE_DEBUG, "mjs.c", 8290)) { cs_log_printf ("  PNEXT %d", 8290); } } while (0); pnext(p); } while (0);
  }
  if ((p)->tok.tok != (TOK_CLOSE_PAREN)) do { mjs_set_errorf(p->mjs, MJS_SYNTAX_ERROR, "parse error at line %d: [%.*s]", p->line_no, 10, p->tok.ptr); return MJS_SYNTAX_ERROR; } while (0); else do { do { if (cs_log_print_prefix(LL_VERBOSE_DEBUG, "mjs.c", 8292)) { cs_log_printf ("  PNEXT %d", 8292); } } while (0); pnext(p); } while (0);;
  if ((res = parse_block(p, 0)) != MJS_OK) return res;
  emit_byte(p, OP_RETURN);
  prologue += mjs_bcode_insert_offset(p, p->mjs, off,
                                      p->cur_idx - off - 1);
  emit_byte(p, OP_PUSH_FUNC);
  emit_int(p, p->cur_idx - 1 - prologue);
  if (name_provided) {
    emit_op(p, TOK_ASSIGN);
  }

  return res;
}

static mjs_err_t parse_object_literal(struct pstate *p) {
  mjs_err_t res = MJS_OK;
  if ((p)->tok.tok != (TOK_OPEN_CURLY)) do { mjs_set_errorf(p->mjs, MJS_SYNTAX_ERROR, "parse error at line %d: [%.*s]", p->line_no, 10, p->tok.ptr); return MJS_SYNTAX_ERROR; } while (0); else do { do { if (cs_log_print_prefix(LL_VERBOSE_DEBUG, "mjs.c", 8308)) { cs_log_printf ("  PNEXT %d", 8308); } } while (0); pnext(p); } while (0);;
  emit_byte(p, OP_PUSH_OBJ);
  while (p->tok.tok != TOK_CLOSE_CURLY) {
    if (p->tok.tok != TOK_IDENT && p->tok.tok != TOK_STR) do { mjs_set_errorf(p->mjs, MJS_SYNTAX_ERROR, "parse error at line %d: [%.*s]", p->line_no, 10, p->tok.ptr); return MJS_SYNTAX_ERROR; } while (0);
    emit_byte(p, OP_DUP);
    emit_byte(p, OP_PUSH_STR);
    emit_str(p, p->tok.ptr, p->tok.len);
    emit_byte(p, OP_SWAP);
    do { do { if (cs_log_print_prefix(LL_VERBOSE_DEBUG, "mjs.c", 8316)) { cs_log_printf ("  PNEXT %d", 8316); } } while (0); pnext(p); } while (0);
    if ((p)->tok.tok != (TOK_COLON)) do { mjs_set_errorf(p->mjs, MJS_SYNTAX_ERROR, "parse error at line %d: [%.*s]", p->line_no, 10, p->tok.ptr); return MJS_SYNTAX_ERROR; } while (0); else do { do { if (cs_log_print_prefix(LL_VERBOSE_DEBUG, "mjs.c", 8317)) { cs_log_printf ("  PNEXT %d", 8317); } } while (0); pnext(p); } while (0);;
    if ((res = parse_expr(p)) != MJS_OK) return res;
    emit_op(p, TOK_ASSIGN);
    emit_byte(p, OP_DROP);
    if (p->tok.tok == TOK_COMMA) {
      do { do { if (cs_log_print_prefix(LL_VERBOSE_DEBUG, "mjs.c", 8322)) { cs_log_printf ("  PNEXT %d", 8322); } } while (0); pnext(p); } while (0);
    } else if (p->tok.tok != TOK_CLOSE_CURLY) {
      do { mjs_set_errorf(p->mjs, MJS_SYNTAX_ERROR, "parse error at line %d: [%.*s]", p->line_no, 10, p->tok.ptr); return MJS_SYNTAX_ERROR; } while (0);
    }
  }
  return res;
}

static mjs_err_t parse_array_literal(struct pstate *p) {
  mjs_err_t res = MJS_OK;
  if ((p)->tok.tok != (TOK_OPEN_BRACKET)) do { mjs_set_errorf(p->mjs, MJS_SYNTAX_ERROR, "parse error at line %d: [%.*s]", p->line_no, 10, p->tok.ptr); return MJS_SYNTAX_ERROR; } while (0); else do { do { if (cs_log_print_prefix(LL_VERBOSE_DEBUG, "mjs.c", 8332)) { cs_log_printf ("  PNEXT %d", 8332); } } while (0); pnext(p); } while (0);;
  emit_byte(p, OP_PUSH_ARRAY);
  while (p->tok.tok != TOK_CLOSE_BRACKET) {
    emit_byte(p, OP_DUP);
    if ((res = parse_expr(p)) != MJS_OK) return res;
    emit_byte(p, OP_APPEND);
    if (p->tok.tok == TOK_COMMA) do { do { if (cs_log_print_prefix(LL_VERBOSE_DEBUG, "mjs.c", 8338)) { cs_log_printf ("  PNEXT %d", 8338); } } while (0); pnext(p); } while (0);
  }
  return res;
}

static enum mjs_err parse_literal(struct pstate *p, const struct tok *t) {
  struct mbuf *bcode_gen = &p->mjs->bcode_gen;
  enum mjs_err res = MJS_OK;
  int tok = t->tok;
  do { if (cs_log_print_prefix(LL_VERBOSE_DEBUG, "mjs.c", 8347)) { cs_log_printf ("[%.*s] %p", p->tok.len, p->tok.ptr, (void *) &t); } } while (0);
  switch (t->tok) {
    case TOK_KEYWORD_FALSE:
      emit_byte(p, OP_PUSH_FALSE);
      break;
    case TOK_KEYWORD_TRUE:
      emit_byte(p, OP_PUSH_TRUE);
      break;
    case TOK_KEYWORD_UNDEFINED:
      emit_byte(p, OP_PUSH_UNDEF);
      break;
    case TOK_KEYWORD_NULL:
      emit_byte(p, OP_PUSH_NULL);
      break;
    case TOK_IDENT: {
      int prev_tok = p->prev_tok;
      int next_tok = ptest(p);
      emit_byte(p, OP_PUSH_STR);
      emit_str(p, t->ptr, t->len);
      emit_byte(p, (uint8_t)(prev_tok == TOK_DOT ? OP_SWAP : OP_FIND_SCOPE));
      if (!findtok(s_assign_ops, next_tok) &&
          !findtok(s_postfix_ops, next_tok) &&

          !findtok(s_postfix_ops, prev_tok)) {
        emit_byte(p, OP_GET);
      }
      break;
    }
    case TOK_NUM: {
      double iv, d = strtod(t->ptr, 
                                   __null
                                       );
      unsigned long uv = strtoul(t->ptr + 2, 
                                            __null
                                                , 16);
      if (t->ptr[0] == '0' && t->ptr[1] == 'x') d = uv;
      if (modf(d, &iv) == 0) {
        emit_byte(p, OP_PUSH_INT);
        emit_int(p, (int64_t) d);
      } else {
        emit_byte(p, OP_PUSH_DBL);
        emit_str(p, t->ptr, t->len);
      }
      break;
    }
    case TOK_STR: {
      size_t oldlen;
      emit_byte(p, OP_PUSH_STR);
      oldlen = bcode_gen->len;
      embed_string(bcode_gen, p->cur_idx, t->ptr, t->len, 2);
      p->cur_idx += bcode_gen->len - oldlen;
    } break;
    case TOK_OPEN_BRACKET:
      res = parse_array_literal(p);
      break;
    case TOK_OPEN_CURLY:
      res = parse_object_literal(p);
      break;
    case TOK_OPEN_PAREN:
      do { do { if (cs_log_print_prefix(LL_VERBOSE_DEBUG, "mjs.c", 8402)) { cs_log_printf ("  PNEXT %d", 8402); } } while (0); pnext(p); } while (0);
      res = parse_expr(p);
      if (p->tok.tok != TOK_CLOSE_PAREN) do { mjs_set_errorf(p->mjs, MJS_SYNTAX_ERROR, "parse error at line %d: [%.*s]", p->line_no, 10, p->tok.ptr); return MJS_SYNTAX_ERROR; } while (0);
      break;
    case TOK_KEYWORD_FUNCTION:
      res = parse_function(p);
      break;
    case TOK_KEYWORD_THIS:
      emit_byte(p, OP_PUSH_THIS);
      break;
    default:
      do { mjs_set_errorf(p->mjs, MJS_SYNTAX_ERROR, "parse error at line %d: [%.*s]", p->line_no, 10, p->tok.ptr); return MJS_SYNTAX_ERROR; } while (0);
  }
  if (tok != TOK_KEYWORD_FUNCTION) do { do { if (cs_log_print_prefix(LL_VERBOSE_DEBUG, "mjs.c", 8415)) { cs_log_printf ("  PNEXT %d", 8415); } } while (0); pnext(p); } while (0);
  return res;
}

static mjs_err_t parse_call_dot_mem(struct pstate *p, int prev_op) {
  int ops[] = {TOK_DOT, TOK_OPEN_PAREN, TOK_OPEN_BRACKET, TOK_EOF};
  mjs_err_t res = MJS_OK;
  if ((res = parse_literal(p, &p->tok)) != MJS_OK) return res;
  while (findtok(ops, p->tok.tok) != TOK_EOF) {
    if (p->tok.tok == TOK_OPEN_BRACKET) {
      int prev_tok = p->prev_tok;
      if ((p)->tok.tok != (TOK_OPEN_BRACKET)) do { mjs_set_errorf(p->mjs, MJS_SYNTAX_ERROR, "parse error at line %d: [%.*s]", p->line_no, 10, p->tok.ptr); return MJS_SYNTAX_ERROR; } while (0); else do { do { if (cs_log_print_prefix(LL_VERBOSE_DEBUG, "mjs.c", 8426)) { cs_log_printf ("  PNEXT %d", 8426); } } while (0); pnext(p); } while (0);;
      if ((res = parse_expr(p)) != MJS_OK) return res;
      emit_byte(p, OP_SWAP);
      if ((p)->tok.tok != (TOK_CLOSE_BRACKET)) do { mjs_set_errorf(p->mjs, MJS_SYNTAX_ERROR, "parse error at line %d: [%.*s]", p->line_no, 10, p->tok.ptr); return MJS_SYNTAX_ERROR; } while (0); else do { do { if (cs_log_print_prefix(LL_VERBOSE_DEBUG, "mjs.c", 8429)) { cs_log_printf ("  PNEXT %d", 8429); } } while (0); pnext(p); } while (0);;
      if (!findtok(s_assign_ops, p->tok.tok) &&
          !findtok(s_postfix_ops, p->tok.tok) &&

          !findtok(s_postfix_ops, prev_tok)) {
        emit_byte(p, OP_GET);
      }
    } else if (p->tok.tok == TOK_OPEN_PAREN) {
      if ((p)->tok.tok != (TOK_OPEN_PAREN)) do { mjs_set_errorf(p->mjs, MJS_SYNTAX_ERROR, "parse error at line %d: [%.*s]", p->line_no, 10, p->tok.ptr); return MJS_SYNTAX_ERROR; } while (0); else do { do { if (cs_log_print_prefix(LL_VERBOSE_DEBUG, "mjs.c", 8437)) { cs_log_printf ("  PNEXT %d", 8437); } } while (0); pnext(p); } while (0);;
      emit_byte(p, OP_ARGS);
      while (p->tok.tok != TOK_CLOSE_PAREN) {
        if ((res = parse_expr(p)) != MJS_OK) return res;
        if (p->tok.tok == TOK_COMMA) do { do { if (cs_log_print_prefix(LL_VERBOSE_DEBUG, "mjs.c", 8441)) { cs_log_printf ("  PNEXT %d", 8441); } } while (0); pnext(p); } while (0);
      }
      emit_byte(p, OP_CALL);
      if ((p)->tok.tok != (TOK_CLOSE_PAREN)) do { mjs_set_errorf(p->mjs, MJS_SYNTAX_ERROR, "parse error at line %d: [%.*s]", p->line_no, 10, p->tok.ptr); return MJS_SYNTAX_ERROR; } while (0); else do { do { if (cs_log_print_prefix(LL_VERBOSE_DEBUG, "mjs.c", 8444)) { cs_log_printf ("  PNEXT %d", 8444); } } while (0); pnext(p); } while (0);;
    } else if (p->tok.tok == TOK_DOT) {
      if ((p)->tok.tok != (TOK_DOT)) do { mjs_set_errorf(p->mjs, MJS_SYNTAX_ERROR, "parse error at line %d: [%.*s]", p->line_no, 10, p->tok.ptr); return MJS_SYNTAX_ERROR; } while (0); else do { do { if (cs_log_print_prefix(LL_VERBOSE_DEBUG, "mjs.c", 8446)) { cs_log_printf ("  PNEXT %d", 8446); } } while (0); pnext(p); } while (0);;
      if ((res = parse_call_dot_mem(p, TOK_DOT)) != MJS_OK) return res;
    }
  }
  (void) prev_op;
  return res;
}

static mjs_err_t parse_postfix(struct pstate *p, int prev_op) {
  mjs_err_t res = MJS_OK;
  if ((res = parse_call_dot_mem(p, prev_op)) != MJS_OK) return res;
  if (p->tok.tok == TOK_PLUS_PLUS || p->tok.tok == TOK_MINUS_MINUS) {
    int op = p->tok.tok == TOK_PLUS_PLUS ? TOK_POSTFIX_PLUS : TOK_POSTFIX_MINUS;
    emit_op(p, op);
    do { do { if (cs_log_print_prefix(LL_VERBOSE_DEBUG, "mjs.c", 8460)) { cs_log_printf ("  PNEXT %d", 8460); } } while (0); pnext(p); } while (0);
  }
  return res;
}

static mjs_err_t parse_unary(struct pstate *p, int prev_op) {
  mjs_err_t res = MJS_OK;
  int op = TOK_EOF;
  if (findtok(s_unary_ops, p->tok.tok) != TOK_EOF) {
    op = p->tok.tok;
    do { do { if (cs_log_print_prefix(LL_VERBOSE_DEBUG, "mjs.c", 8470)) { cs_log_printf ("  PNEXT %d", 8470); } } while (0); pnext(p); } while (0);
  }
  if (findtok(s_unary_ops, p->tok.tok) != TOK_EOF) {
    res = parse_unary(p, prev_op);
  } else {
    res = parse_postfix(p, prev_op);
  }
  if (res != MJS_OK) return res;
  if (op != TOK_EOF) {
    if (op == TOK_MINUS) op = TOK_UNARY_MINUS;
    if (op == TOK_PLUS) op = TOK_UNARY_PLUS;
    emit_op(p, op);
  }
  return res;
}

static mjs_err_t parse_mul_div_rem(struct pstate *p, int prev_op) {
  int ops[] = {TOK_MUL, TOK_DIV, TOK_REM, TOK_EOF};
  do { mjs_err_t res = MJS_OK; p->depth++; if (p->depth > (8192 / 16)) { mjs_set_errorf(p->mjs, MJS_SYNTAX_ERROR, "parser stack overflow"); res = MJS_SYNTAX_ERROR; goto binop_clean; } if ((res = parse_unary(p, TOK_EOF)) != MJS_OK) goto binop_clean; if (prev_op != TOK_EOF) emit_op(p, prev_op); if (findtok(ops, p->tok.tok) != TOK_EOF) { int op = p->tok.tok; size_t off_if = 0; if (ops[0] == TOK_LOGICAL_AND || ops[0] == TOK_LOGICAL_OR) { emit_byte(p, (uint8_t)(ops[0] == TOK_LOGICAL_AND ? OP_JMP_NEUTRAL_FALSE : OP_JMP_NEUTRAL_TRUE)); off_if = p->cur_idx; emit_init_offset(p); emit_byte(p, (uint8_t) OP_DROP); op = TOK_EOF; } do { do { if (cs_log_print_prefix(LL_VERBOSE_DEBUG, "mjs.c", 8488)) { cs_log_printf ("  PNEXT %d", 8488); } } while (0); pnext(p); } while (0); if ((res = parse_mul_div_rem(p, op)) != MJS_OK) goto binop_clean; if (off_if != 0) { mjs_bcode_insert_offset(p, p->mjs, off_if, p->cur_idx - off_if - 1); } } binop_clean: p->depth--; return res; } while (0);
}

static mjs_err_t parse_plus_minus(struct pstate *p, int prev_op) {
  int ops[] = {TOK_PLUS, TOK_MINUS, TOK_EOF};
  do { mjs_err_t res = MJS_OK; p->depth++; if (p->depth > (8192 / 16)) { mjs_set_errorf(p->mjs, MJS_SYNTAX_ERROR, "parser stack overflow"); res = MJS_SYNTAX_ERROR; goto binop_clean; } if ((res = parse_mul_div_rem(p, TOK_EOF)) != MJS_OK) goto binop_clean; if (prev_op != TOK_EOF) emit_op(p, prev_op); if (findtok(ops, p->tok.tok) != TOK_EOF) { int op = p->tok.tok; size_t off_if = 0; if (ops[0] == TOK_LOGICAL_AND || ops[0] == TOK_LOGICAL_OR) { emit_byte(p, (uint8_t)(ops[0] == TOK_LOGICAL_AND ? OP_JMP_NEUTRAL_FALSE : OP_JMP_NEUTRAL_TRUE)); off_if = p->cur_idx; emit_init_offset(p); emit_byte(p, (uint8_t) OP_DROP); op = TOK_EOF; } do { do { if (cs_log_print_prefix(LL_VERBOSE_DEBUG, "mjs.c", 8493)) { cs_log_printf ("  PNEXT %d", 8493); } } while (0); pnext(p); } while (0); if ((res = parse_plus_minus(p, op)) != MJS_OK) goto binop_clean; if (off_if != 0) { mjs_bcode_insert_offset(p, p->mjs, off_if, p->cur_idx - off_if - 1); } } binop_clean: p->depth--; return res; } while (0);
}

static mjs_err_t parse_shifts(struct pstate *p, int prev_op) {
  int ops[] = {TOK_LSHIFT, TOK_RSHIFT, TOK_URSHIFT, TOK_EOF};
  do { mjs_err_t res = MJS_OK; p->depth++; if (p->depth > (8192 / 16)) { mjs_set_errorf(p->mjs, MJS_SYNTAX_ERROR, "parser stack overflow"); res = MJS_SYNTAX_ERROR; goto binop_clean; } if ((res = parse_plus_minus(p, TOK_EOF)) != MJS_OK) goto binop_clean; if (prev_op != TOK_EOF) emit_op(p, prev_op); if (findtok(ops, p->tok.tok) != TOK_EOF) { int op = p->tok.tok; size_t off_if = 0; if (ops[0] == TOK_LOGICAL_AND || ops[0] == TOK_LOGICAL_OR) { emit_byte(p, (uint8_t)(ops[0] == TOK_LOGICAL_AND ? OP_JMP_NEUTRAL_FALSE : OP_JMP_NEUTRAL_TRUE)); off_if = p->cur_idx; emit_init_offset(p); emit_byte(p, (uint8_t) OP_DROP); op = TOK_EOF; } do { do { if (cs_log_print_prefix(LL_VERBOSE_DEBUG, "mjs.c", 8498)) { cs_log_printf ("  PNEXT %d", 8498); } } while (0); pnext(p); } while (0); if ((res = parse_shifts(p, op)) != MJS_OK) goto binop_clean; if (off_if != 0) { mjs_bcode_insert_offset(p, p->mjs, off_if, p->cur_idx - off_if - 1); } } binop_clean: p->depth--; return res; } while (0);
}

static mjs_err_t parse_comparison(struct pstate *p, int prev_op) {
  do { mjs_err_t res = MJS_OK; p->depth++; if (p->depth > (8192 / 16)) { mjs_set_errorf(p->mjs, MJS_SYNTAX_ERROR, "parser stack overflow"); res = MJS_SYNTAX_ERROR; goto binop_clean; } if ((res = parse_shifts(p, TOK_EOF)) != MJS_OK) goto binop_clean; if (prev_op != TOK_EOF) emit_op(p, prev_op); if (findtok(s_comparison_ops, p->tok.tok) != TOK_EOF) { int op = p->tok.tok; size_t off_if = 0; if (s_comparison_ops[0] == TOK_LOGICAL_AND || s_comparison_ops[0] == TOK_LOGICAL_OR) { emit_byte(p, (uint8_t)(s_comparison_ops[0] == TOK_LOGICAL_AND ? OP_JMP_NEUTRAL_FALSE : OP_JMP_NEUTRAL_TRUE)); off_if = p->cur_idx; emit_init_offset(p); emit_byte(p, (uint8_t) OP_DROP); op = TOK_EOF; } do { do { if (cs_log_print_prefix(LL_VERBOSE_DEBUG, "mjs.c", 8502)) { cs_log_printf ("  PNEXT %d", 8502); } } while (0); pnext(p); } while (0); if ((res = parse_comparison(p, op)) != MJS_OK) goto binop_clean; if (off_if != 0) { mjs_bcode_insert_offset(p, p->mjs, off_if, p->cur_idx - off_if - 1); } } binop_clean: p->depth--; return res; } while (0);
}

static mjs_err_t parse_equality(struct pstate *p, int prev_op) {
  do { mjs_err_t res = MJS_OK; p->depth++; if (p->depth > (8192 / 16)) { mjs_set_errorf(p->mjs, MJS_SYNTAX_ERROR, "parser stack overflow"); res = MJS_SYNTAX_ERROR; goto binop_clean; } if ((res = parse_comparison(p, TOK_EOF)) != MJS_OK) goto binop_clean; if (prev_op != TOK_EOF) emit_op(p, prev_op); if (findtok(s_equality_ops, p->tok.tok) != TOK_EOF) { int op = p->tok.tok; size_t off_if = 0; if (s_equality_ops[0] == TOK_LOGICAL_AND || s_equality_ops[0] == TOK_LOGICAL_OR) { emit_byte(p, (uint8_t)(s_equality_ops[0] == TOK_LOGICAL_AND ? OP_JMP_NEUTRAL_FALSE : OP_JMP_NEUTRAL_TRUE)); off_if = p->cur_idx; emit_init_offset(p); emit_byte(p, (uint8_t) OP_DROP); op = TOK_EOF; } do { do { if (cs_log_print_prefix(LL_VERBOSE_DEBUG, "mjs.c", 8506)) { cs_log_printf ("  PNEXT %d", 8506); } } while (0); pnext(p); } while (0); if ((res = parse_equality(p, op)) != MJS_OK) goto binop_clean; if (off_if != 0) { mjs_bcode_insert_offset(p, p->mjs, off_if, p->cur_idx - off_if - 1); } } binop_clean: p->depth--; return res; } while (0);
}

static mjs_err_t parse_bitwise_and(struct pstate *p, int prev_op) {
  int ops[] = {TOK_AND, TOK_EOF};
  do { mjs_err_t res = MJS_OK; p->depth++; if (p->depth > (8192 / 16)) { mjs_set_errorf(p->mjs, MJS_SYNTAX_ERROR, "parser stack overflow"); res = MJS_SYNTAX_ERROR; goto binop_clean; } if ((res = parse_equality(p, TOK_EOF)) != MJS_OK) goto binop_clean; if (prev_op != TOK_EOF) emit_op(p, prev_op); if (findtok(ops, p->tok.tok) != TOK_EOF) { int op = p->tok.tok; size_t off_if = 0; if (ops[0] == TOK_LOGICAL_AND || ops[0] == TOK_LOGICAL_OR) { emit_byte(p, (uint8_t)(ops[0] == TOK_LOGICAL_AND ? OP_JMP_NEUTRAL_FALSE : OP_JMP_NEUTRAL_TRUE)); off_if = p->cur_idx; emit_init_offset(p); emit_byte(p, (uint8_t) OP_DROP); op = TOK_EOF; } do { do { if (cs_log_print_prefix(LL_VERBOSE_DEBUG, "mjs.c", 8511)) { cs_log_printf ("  PNEXT %d", 8511); } } while (0); pnext(p); } while (0); if ((res = parse_bitwise_and(p, op)) != MJS_OK) goto binop_clean; if (off_if != 0) { mjs_bcode_insert_offset(p, p->mjs, off_if, p->cur_idx - off_if - 1); } } binop_clean: p->depth--; return res; } while (0);
}

static mjs_err_t parse_bitwise_xor(struct pstate *p, int prev_op) {
  int ops[] = {TOK_XOR, TOK_EOF};
  do { mjs_err_t res = MJS_OK; p->depth++; if (p->depth > (8192 / 16)) { mjs_set_errorf(p->mjs, MJS_SYNTAX_ERROR, "parser stack overflow"); res = MJS_SYNTAX_ERROR; goto binop_clean; } if ((res = parse_bitwise_and(p, TOK_EOF)) != MJS_OK) goto binop_clean; if (prev_op != TOK_EOF) emit_op(p, prev_op); if (findtok(ops, p->tok.tok) != TOK_EOF) { int op = p->tok.tok; size_t off_if = 0; if (ops[0] == TOK_LOGICAL_AND || ops[0] == TOK_LOGICAL_OR) { emit_byte(p, (uint8_t)(ops[0] == TOK_LOGICAL_AND ? OP_JMP_NEUTRAL_FALSE : OP_JMP_NEUTRAL_TRUE)); off_if = p->cur_idx; emit_init_offset(p); emit_byte(p, (uint8_t) OP_DROP); op = TOK_EOF; } do { do { if (cs_log_print_prefix(LL_VERBOSE_DEBUG, "mjs.c", 8516)) { cs_log_printf ("  PNEXT %d", 8516); } } while (0); pnext(p); } while (0); if ((res = parse_bitwise_xor(p, op)) != MJS_OK) goto binop_clean; if (off_if != 0) { mjs_bcode_insert_offset(p, p->mjs, off_if, p->cur_idx - off_if - 1); } } binop_clean: p->depth--; return res; } while (0);
}

static mjs_err_t parse_bitwise_or(struct pstate *p, int prev_op) {
  int ops[] = {TOK_OR, TOK_EOF};
  do { mjs_err_t res = MJS_OK; p->depth++; if (p->depth > (8192 / 16)) { mjs_set_errorf(p->mjs, MJS_SYNTAX_ERROR, "parser stack overflow"); res = MJS_SYNTAX_ERROR; goto binop_clean; } if ((res = parse_bitwise_xor(p, TOK_EOF)) != MJS_OK) goto binop_clean; if (prev_op != TOK_EOF) emit_op(p, prev_op); if (findtok(ops, p->tok.tok) != TOK_EOF) { int op = p->tok.tok; size_t off_if = 0; if (ops[0] == TOK_LOGICAL_AND || ops[0] == TOK_LOGICAL_OR) { emit_byte(p, (uint8_t)(ops[0] == TOK_LOGICAL_AND ? OP_JMP_NEUTRAL_FALSE : OP_JMP_NEUTRAL_TRUE)); off_if = p->cur_idx; emit_init_offset(p); emit_byte(p, (uint8_t) OP_DROP); op = TOK_EOF; } do { do { if (cs_log_print_prefix(LL_VERBOSE_DEBUG, "mjs.c", 8521)) { cs_log_printf ("  PNEXT %d", 8521); } } while (0); pnext(p); } while (0); if ((res = parse_bitwise_or(p, op)) != MJS_OK) goto binop_clean; if (off_if != 0) { mjs_bcode_insert_offset(p, p->mjs, off_if, p->cur_idx - off_if - 1); } } binop_clean: p->depth--; return res; } while (0);
}

static mjs_err_t parse_logical_and(struct pstate *p, int prev_op) {
  int ops[] = {TOK_LOGICAL_AND, TOK_EOF};
  do { mjs_err_t res = MJS_OK; p->depth++; if (p->depth > (8192 / 16)) { mjs_set_errorf(p->mjs, MJS_SYNTAX_ERROR, "parser stack overflow"); res = MJS_SYNTAX_ERROR; goto binop_clean; } if ((res = parse_bitwise_or(p, TOK_EOF)) != MJS_OK) goto binop_clean; if (prev_op != TOK_EOF) emit_op(p, prev_op); if (findtok(ops, p->tok.tok) != TOK_EOF) { int op = p->tok.tok; size_t off_if = 0; if (ops[0] == TOK_LOGICAL_AND || ops[0] == TOK_LOGICAL_OR) { emit_byte(p, (uint8_t)(ops[0] == TOK_LOGICAL_AND ? OP_JMP_NEUTRAL_FALSE : OP_JMP_NEUTRAL_TRUE)); off_if = p->cur_idx; emit_init_offset(p); emit_byte(p, (uint8_t) OP_DROP); op = TOK_EOF; } do { do { if (cs_log_print_prefix(LL_VERBOSE_DEBUG, "mjs.c", 8526)) { cs_log_printf ("  PNEXT %d", 8526); } } while (0); pnext(p); } while (0); if ((res = parse_logical_and(p, op)) != MJS_OK) goto binop_clean; if (off_if != 0) { mjs_bcode_insert_offset(p, p->mjs, off_if, p->cur_idx - off_if - 1); } } binop_clean: p->depth--; return res; } while (0);
}

static mjs_err_t parse_logical_or(struct pstate *p, int prev_op) {
  int ops[] = {TOK_LOGICAL_OR, TOK_EOF};
  do { mjs_err_t res = MJS_OK; p->depth++; if (p->depth > (8192 / 16)) { mjs_set_errorf(p->mjs, MJS_SYNTAX_ERROR, "parser stack overflow"); res = MJS_SYNTAX_ERROR; goto binop_clean; } if ((res = parse_logical_and(p, TOK_EOF)) != MJS_OK) goto binop_clean; if (prev_op != TOK_EOF) emit_op(p, prev_op); if (findtok(ops, p->tok.tok) != TOK_EOF) { int op = p->tok.tok; size_t off_if = 0; if (ops[0] == TOK_LOGICAL_AND || ops[0] == TOK_LOGICAL_OR) { emit_byte(p, (uint8_t)(ops[0] == TOK_LOGICAL_AND ? OP_JMP_NEUTRAL_FALSE : OP_JMP_NEUTRAL_TRUE)); off_if = p->cur_idx; emit_init_offset(p); emit_byte(p, (uint8_t) OP_DROP); op = TOK_EOF; } do { do { if (cs_log_print_prefix(LL_VERBOSE_DEBUG, "mjs.c", 8531)) { cs_log_printf ("  PNEXT %d", 8531); } } while (0); pnext(p); } while (0); if ((res = parse_logical_or(p, op)) != MJS_OK) goto binop_clean; if (off_if != 0) { mjs_bcode_insert_offset(p, p->mjs, off_if, p->cur_idx - off_if - 1); } } binop_clean: p->depth--; return res; } while (0);
}

static mjs_err_t parse_ternary(struct pstate *p, int prev_op) {
  mjs_err_t res = MJS_OK;
  if ((res = parse_logical_or(p, TOK_EOF)) != MJS_OK) return res;
  if (prev_op != TOK_EOF) emit_op(p, prev_op);

  if (p->tok.tok == TOK_QUESTION) {
    size_t off_if, off_endif, off_else;
    if ((p)->tok.tok != (TOK_QUESTION)) do { mjs_set_errorf(p->mjs, MJS_SYNTAX_ERROR, "parse error at line %d: [%.*s]", p->line_no, 10, p->tok.ptr); return MJS_SYNTAX_ERROR; } while (0); else do { do { if (cs_log_print_prefix(LL_VERBOSE_DEBUG, "mjs.c", 8541)) { cs_log_printf ("  PNEXT %d", 8541); } } while (0); pnext(p); } while (0);;

    emit_byte(p, OP_JMP_FALSE);
    off_if = p->cur_idx;
    emit_init_offset(p);

    if ((res = parse_ternary(p, TOK_EOF)) != MJS_OK) return res;

    emit_byte(p, OP_JMP);
    off_else = p->cur_idx;
    emit_init_offset(p);
    off_endif = p->cur_idx;

    emit_byte(p, OP_DROP);

    if ((p)->tok.tok != (TOK_COLON)) do { mjs_set_errorf(p->mjs, MJS_SYNTAX_ERROR, "parse error at line %d: [%.*s]", p->line_no, 10, p->tok.ptr); return MJS_SYNTAX_ERROR; } while (0); else do { do { if (cs_log_print_prefix(LL_VERBOSE_DEBUG, "mjs.c", 8556)) { cs_log_printf ("  PNEXT %d", 8556); } } while (0); pnext(p); } while (0);;
    if ((res = parse_ternary(p, TOK_EOF)) != MJS_OK) return res;





    off_endif += mjs_bcode_insert_offset(
        p, p->mjs, off_else, p->cur_idx - off_else - 1);

    mjs_bcode_insert_offset(p, p->mjs, off_if,
                            off_endif - off_if - 1);
  }

  return res;
}

static mjs_err_t parse_assignment(struct pstate *p, int prev_op) {
  do { mjs_err_t res = MJS_OK; (void) prev_op; if ((res = parse_ternary(p, TOK_EOF)) != MJS_OK) return res; if (findtok(s_assign_ops, p->tok.tok) != TOK_EOF) { int op = p->tok.tok; do { do { if (cs_log_print_prefix(LL_VERBOSE_DEBUG, "mjs.c", 8574)) { cs_log_printf ("  PNEXT %d", 8574); } } while (0); pnext(p); } while (0); if ((res = parse_assignment(p, TOK_EOF)) != MJS_OK) return res; emit_op(p, op); } return res; } while (0);
}

static mjs_err_t parse_expr(struct pstate *p) {
  return parse_assignment(p, TOK_EOF);
}

static mjs_err_t parse_let(struct pstate *p) {
  mjs_err_t res = MJS_OK;
  do { if (cs_log_print_prefix(LL_VERBOSE_DEBUG, "mjs.c", 8583)) { cs_log_printf ("[%.*s]", 10, p->tok.ptr); } } while (0);
  if ((p)->tok.tok != (TOK_KEYWORD_LET)) do { mjs_set_errorf(p->mjs, MJS_SYNTAX_ERROR, "parse error at line %d: [%.*s]", p->line_no, 10, p->tok.ptr); return MJS_SYNTAX_ERROR; } while (0); else do { do { if (cs_log_print_prefix(LL_VERBOSE_DEBUG, "mjs.c", 8584)) { cs_log_printf ("  PNEXT %d", 8584); } } while (0); pnext(p); } while (0);;
  for (;;) {
    struct tok tmp = p->tok;
    if ((p)->tok.tok != (TOK_IDENT)) do { mjs_set_errorf(p->mjs, MJS_SYNTAX_ERROR, "parse error at line %d: [%.*s]", p->line_no, 10, p->tok.ptr); return MJS_SYNTAX_ERROR; } while (0); else do { do { if (cs_log_print_prefix(LL_VERBOSE_DEBUG, "mjs.c", 8587)) { cs_log_printf ("  PNEXT %d", 8587); } } while (0); pnext(p); } while (0);;

    emit_byte(p, OP_PUSH_STR);
    emit_str(p, tmp.ptr, tmp.len);
    emit_byte(p, OP_PUSH_SCOPE);
    emit_byte(p, OP_CREATE);

    if (p->tok.tok == TOK_ASSIGN) {
      do { do { if (cs_log_print_prefix(LL_VERBOSE_DEBUG, "mjs.c", 8595)) { cs_log_printf ("  PNEXT %d", 8595); } } while (0); pnext(p); } while (0);
      emit_byte(p, OP_PUSH_STR);
      emit_str(p, tmp.ptr, tmp.len);
      emit_byte(p, OP_FIND_SCOPE);
      if ((res = parse_expr(p)) != MJS_OK) return res;
      emit_op(p, TOK_ASSIGN);
    } else {
      emit_byte(p, OP_PUSH_UNDEF);
    }
    if (p->tok.tok == TOK_COMMA) {
      emit_byte(p, OP_DROP);
      do { do { if (cs_log_print_prefix(LL_VERBOSE_DEBUG, "mjs.c", 8606)) { cs_log_printf ("  PNEXT %d", 8606); } } while (0); pnext(p); } while (0);
    }
    if (p->tok.tok == TOK_SEMICOLON || p->tok.tok == TOK_EOF) break;
  }
  return res;
}

static mjs_err_t parse_block_or_stmt(struct pstate *p, int cs) {
  if (ptest(p) == TOK_OPEN_CURLY) {
    return parse_block(p, cs);
  } else {
    return parse_statement(p);
  }
}

static mjs_err_t parse_for_in(struct pstate *p) {
  mjs_err_t res = MJS_OK;
  size_t off_b, off_check_end;


  emit_byte(p, OP_NEW_SCOPE);


  if (p->tok.tok == TOK_KEYWORD_LET) {
    if ((p)->tok.tok != (TOK_KEYWORD_LET)) do { mjs_set_errorf(p->mjs, MJS_SYNTAX_ERROR, "parse error at line %d: [%.*s]", p->line_no, 10, p->tok.ptr); return MJS_SYNTAX_ERROR; } while (0); else do { do { if (cs_log_print_prefix(LL_VERBOSE_DEBUG, "mjs.c", 8630)) { cs_log_printf ("  PNEXT %d", 8630); } } while (0); pnext(p); } while (0);;
    emit_byte(p, OP_PUSH_STR);
    emit_str(p, p->tok.ptr, p->tok.len);
    emit_byte(p, OP_PUSH_SCOPE);
    emit_byte(p, OP_CREATE);
  }
  emit_byte(p, OP_PUSH_STR);
  emit_str(p, p->tok.ptr, p->tok.len);


  if ((p)->tok.tok != (TOK_IDENT)) do { mjs_set_errorf(p->mjs, MJS_SYNTAX_ERROR, "parse error at line %d: [%.*s]", p->line_no, 10, p->tok.ptr); return MJS_SYNTAX_ERROR; } while (0); else do { do { if (cs_log_print_prefix(LL_VERBOSE_DEBUG, "mjs.c", 8640)) { cs_log_printf ("  PNEXT %d", 8640); } } while (0); pnext(p); } while (0);;
  if ((p)->tok.tok != (TOK_KEYWORD_IN)) do { mjs_set_errorf(p->mjs, MJS_SYNTAX_ERROR, "parse error at line %d: [%.*s]", p->line_no, 10, p->tok.ptr); return MJS_SYNTAX_ERROR; } while (0); else do { do { if (cs_log_print_prefix(LL_VERBOSE_DEBUG, "mjs.c", 8641)) { cs_log_printf ("  PNEXT %d", 8641); } } while (0); pnext(p); } while (0);;
  parse_expr(p);
  if ((p)->tok.tok != (TOK_CLOSE_PAREN)) do { mjs_set_errorf(p->mjs, MJS_SYNTAX_ERROR, "parse error at line %d: [%.*s]", p->line_no, 10, p->tok.ptr); return MJS_SYNTAX_ERROR; } while (0); else do { do { if (cs_log_print_prefix(LL_VERBOSE_DEBUG, "mjs.c", 8643)) { cs_log_printf ("  PNEXT %d", 8643); } } while (0); pnext(p); } while (0);;

  emit_byte(p, OP_PUSH_UNDEF);


  emit_byte(p, OP_LOOP);
  off_b = p->cur_idx;
  emit_init_offset(p);
  emit_byte(p, 0);

  emit_byte(p, OP_FOR_IN_NEXT);
  emit_byte(p, OP_DUP);
  emit_byte(p, OP_JMP_FALSE);
  off_check_end = p->cur_idx;
  emit_init_offset(p);


  if (p->tok.tok == TOK_OPEN_CURLY) {
    if ((res = parse_statement_list(p, TOK_CLOSE_CURLY)) != MJS_OK) return res;
    do { do { if (cs_log_print_prefix(LL_VERBOSE_DEBUG, "mjs.c", 8662)) { cs_log_printf ("  PNEXT %d", 8662); } } while (0); pnext(p); } while (0);
  } else {
    if ((res = parse_statement(p)) != MJS_OK) return res;
  }
  emit_byte(p, OP_DROP);
  emit_byte(p, OP_CONTINUE);


  mjs_bcode_insert_offset(p, p->mjs, off_check_end,
                          p->cur_idx - off_check_end - 1);



  emit_byte(p, OP_BREAK);


  mjs_bcode_insert_offset(p, p->mjs, off_b,
                          p->cur_idx - off_b - 1);

  emit_byte(p, OP_DROP);
  emit_byte(p, OP_DROP);
  emit_byte(p, OP_DROP);
  emit_byte(p, OP_DEL_SCOPE);

  return res;
}

static int check_for_in(struct pstate *p) {
  struct pstate saved = *p;
  int forin = 0;
  if (p->tok.tok == TOK_KEYWORD_LET) do { do { if (cs_log_print_prefix(LL_VERBOSE_DEBUG, "mjs.c", 8692)) { cs_log_printf ("  PNEXT %d", 8692); } } while (0); pnext(p); } while (0);
  if (p->tok.tok == TOK_IDENT) {
    do { do { if (cs_log_print_prefix(LL_VERBOSE_DEBUG, "mjs.c", 8694)) { cs_log_printf ("  PNEXT %d", 8694); } } while (0); pnext(p); } while (0);
    if (p->tok.tok == TOK_KEYWORD_IN) forin = 1;
  }
  *p = saved;
  return forin;
}

static mjs_err_t parse_for(struct pstate *p) {
  mjs_err_t res = MJS_OK;
  size_t off_b, off_c, off_init_end;
  size_t off_incr_begin, off_cond_begin, off_cond_end;
  int buf_cur_idx;

  do { if (cs_log_print_prefix(LL_VERBOSE_DEBUG, "mjs.c", 8707)) { cs_log_printf ("[%.*s]", 10, p->tok.ptr); } } while (0);
  if ((p)->tok.tok != (TOK_KEYWORD_FOR)) do { mjs_set_errorf(p->mjs, MJS_SYNTAX_ERROR, "parse error at line %d: [%.*s]", p->line_no, 10, p->tok.ptr); return MJS_SYNTAX_ERROR; } while (0); else do { do { if (cs_log_print_prefix(LL_VERBOSE_DEBUG, "mjs.c", 8708)) { cs_log_printf ("  PNEXT %d", 8708); } } while (0); pnext(p); } while (0);;
  if ((p)->tok.tok != (TOK_OPEN_PAREN)) do { mjs_set_errorf(p->mjs, MJS_SYNTAX_ERROR, "parse error at line %d: [%.*s]", p->line_no, 10, p->tok.ptr); return MJS_SYNTAX_ERROR; } while (0); else do { do { if (cs_log_print_prefix(LL_VERBOSE_DEBUG, "mjs.c", 8709)) { cs_log_printf ("  PNEXT %d", 8709); } } while (0); pnext(p); } while (0);;


  if (check_for_in(p)) return parse_for_in(p);
  emit_byte(p, OP_NEW_SCOPE);


  emit_byte(p, OP_LOOP);
  off_b = p->cur_idx;
  emit_init_offset(p);
  off_c = p->cur_idx;
  emit_init_offset(p);


  if (p->tok.tok == TOK_KEYWORD_LET) {
    if ((res = parse_let(p)) != MJS_OK) return res;
  } else {
    if ((res = parse_expr(p)) != MJS_OK) return res;
  }
  if ((p)->tok.tok != (TOK_SEMICOLON)) do { mjs_set_errorf(p->mjs, MJS_SYNTAX_ERROR, "parse error at line %d: [%.*s]", p->line_no, 10, p->tok.ptr); return MJS_SYNTAX_ERROR; } while (0); else do { do { if (cs_log_print_prefix(LL_VERBOSE_DEBUG, "mjs.c", 8747)) { cs_log_printf ("  PNEXT %d", 8747); } } while (0); pnext(p); } while (0);;
  emit_byte(p, OP_DROP);

  emit_byte(p, OP_JMP);
  off_init_end = p->cur_idx;
  emit_init_offset(p);

  off_incr_begin = p->cur_idx;
  off_cond_begin = p->cur_idx;


  if ((res = parse_expr(p)) != MJS_OK) return res;
  if ((p)->tok.tok != (TOK_SEMICOLON)) do { mjs_set_errorf(p->mjs, MJS_SYNTAX_ERROR, "parse error at line %d: [%.*s]", p->line_no, 10, p->tok.ptr); return MJS_SYNTAX_ERROR; } while (0); else do { do { if (cs_log_print_prefix(LL_VERBOSE_DEBUG, "mjs.c", 8759)) { cs_log_printf ("  PNEXT %d", 8759); } } while (0); pnext(p); } while (0);;



  buf_cur_idx = p->cur_idx;
  p->cur_idx = off_incr_begin;

  if ((res = parse_expr(p)) != MJS_OK) return res;
  if ((p)->tok.tok != (TOK_CLOSE_PAREN)) do { mjs_set_errorf(p->mjs, MJS_SYNTAX_ERROR, "parse error at line %d: [%.*s]", p->line_no, 10, p->tok.ptr); return MJS_SYNTAX_ERROR; } while (0); else do { do { if (cs_log_print_prefix(LL_VERBOSE_DEBUG, "mjs.c", 8767)) { cs_log_printf ("  PNEXT %d", 8767); } } while (0); pnext(p); } while (0);;
  emit_byte(p, OP_DROP);





  {
    int incr_size = p->cur_idx - off_incr_begin;
    off_cond_begin += incr_size;
    p->cur_idx = buf_cur_idx + incr_size;
  }



  emit_byte(p, OP_JMP_FALSE);
  off_cond_end = p->cur_idx;
  emit_init_offset(p);


  if (p->tok.tok == TOK_OPEN_CURLY) {
    if ((res = parse_statement_list(p, TOK_CLOSE_CURLY)) != MJS_OK) return res;
    do { do { if (cs_log_print_prefix(LL_VERBOSE_DEBUG, "mjs.c", 8789)) { cs_log_printf ("  PNEXT %d", 8789); } } while (0); pnext(p); } while (0);
  } else {
    if ((res = parse_statement(p)) != MJS_OK) return res;
  }
  emit_byte(p, OP_DROP);
  emit_byte(p, OP_CONTINUE);




  mjs_bcode_insert_offset(p, p->mjs, off_cond_end,
                          p->cur_idx - off_cond_end - 1);


  off_incr_begin += mjs_bcode_insert_offset(
      p, p->mjs, off_init_end,
      off_cond_begin - off_init_end - 1);


  mjs_bcode_insert_offset(p, p->mjs, off_c,
                          off_incr_begin - off_c - 1);

  emit_byte(p, OP_BREAK);


  mjs_bcode_insert_offset(p, p->mjs, off_b,
                          p->cur_idx - off_b - 1);

  emit_byte(p, OP_DEL_SCOPE);

  return res;
}

static mjs_err_t parse_while(struct pstate *p) {
  size_t off_cond_end, off_b;
  mjs_err_t res = MJS_OK;

  if ((p)->tok.tok != (TOK_KEYWORD_WHILE)) do { mjs_set_errorf(p->mjs, MJS_SYNTAX_ERROR, "parse error at line %d: [%.*s]", p->line_no, 10, p->tok.ptr); return MJS_SYNTAX_ERROR; } while (0); else do { do { if (cs_log_print_prefix(LL_VERBOSE_DEBUG, "mjs.c", 8826)) { cs_log_printf ("  PNEXT %d", 8826); } } while (0); pnext(p); } while (0);;
  if ((p)->tok.tok != (TOK_OPEN_PAREN)) do { mjs_set_errorf(p->mjs, MJS_SYNTAX_ERROR, "parse error at line %d: [%.*s]", p->line_no, 10, p->tok.ptr); return MJS_SYNTAX_ERROR; } while (0); else do { do { if (cs_log_print_prefix(LL_VERBOSE_DEBUG, "mjs.c", 8827)) { cs_log_printf ("  PNEXT %d", 8827); } } while (0); pnext(p); } while (0);;


  emit_byte(p, OP_NEW_SCOPE);
  emit_byte(p, OP_LOOP);
  off_b = p->cur_idx;
  emit_init_offset(p);
  emit_byte(p, 0);


  if ((res = parse_expr(p)) != MJS_OK) return res;
  if ((p)->tok.tok != (TOK_CLOSE_PAREN)) do { mjs_set_errorf(p->mjs, MJS_SYNTAX_ERROR, "parse error at line %d: [%.*s]", p->line_no, 10, p->tok.ptr); return MJS_SYNTAX_ERROR; } while (0); else do { do { if (cs_log_print_prefix(LL_VERBOSE_DEBUG, "mjs.c", 8855)) { cs_log_printf ("  PNEXT %d", 8855); } } while (0); pnext(p); } while (0);;


  emit_byte(p, OP_JMP_FALSE);
  off_cond_end = p->cur_idx;
  emit_init_offset(p);


  if (p->tok.tok == TOK_OPEN_CURLY) {
    if ((res = parse_statement_list(p, TOK_CLOSE_CURLY)) != MJS_OK) return res;
    do { do { if (cs_log_print_prefix(LL_VERBOSE_DEBUG, "mjs.c", 8865)) { cs_log_printf ("  PNEXT %d", 8865); } } while (0); pnext(p); } while (0);
  } else {
    if ((res = parse_statement(p)) != MJS_OK) return res;
  }
  emit_byte(p, OP_DROP);
  emit_byte(p, OP_CONTINUE);


  mjs_bcode_insert_offset(p, p->mjs, off_cond_end,
                          p->cur_idx - off_cond_end - 1);



  emit_byte(p, OP_BREAK);


  mjs_bcode_insert_offset(p, p->mjs, off_b,
                          p->cur_idx - off_b - 1);

  emit_byte(p, OP_DEL_SCOPE);
  return res;
}

static mjs_err_t parse_if(struct pstate *p) {
  size_t off_if, off_endif;
  mjs_err_t res = MJS_OK;
  do { if (cs_log_print_prefix(LL_VERBOSE_DEBUG, "mjs.c", 8891)) { cs_log_printf ("[%.*s]", 10, p->tok.ptr); } } while (0);
  if ((p)->tok.tok != (TOK_KEYWORD_IF)) do { mjs_set_errorf(p->mjs, MJS_SYNTAX_ERROR, "parse error at line %d: [%.*s]", p->line_no, 10, p->tok.ptr); return MJS_SYNTAX_ERROR; } while (0); else do { do { if (cs_log_print_prefix(LL_VERBOSE_DEBUG, "mjs.c", 8892)) { cs_log_printf ("  PNEXT %d", 8892); } } while (0); pnext(p); } while (0);;
  if ((p)->tok.tok != (TOK_OPEN_PAREN)) do { mjs_set_errorf(p->mjs, MJS_SYNTAX_ERROR, "parse error at line %d: [%.*s]", p->line_no, 10, p->tok.ptr); return MJS_SYNTAX_ERROR; } while (0); else do { do { if (cs_log_print_prefix(LL_VERBOSE_DEBUG, "mjs.c", 8893)) { cs_log_printf ("  PNEXT %d", 8893); } } while (0); pnext(p); } while (0);;
  if ((res = parse_expr(p)) != MJS_OK) return res;

  emit_byte(p, OP_JMP_FALSE);
  off_if = p->cur_idx;
  emit_init_offset(p);

  if ((p)->tok.tok != (TOK_CLOSE_PAREN)) do { mjs_set_errorf(p->mjs, MJS_SYNTAX_ERROR, "parse error at line %d: [%.*s]", p->line_no, 10, p->tok.ptr); return MJS_SYNTAX_ERROR; } while (0); else do { do { if (cs_log_print_prefix(LL_VERBOSE_DEBUG, "mjs.c", 8900)) { cs_log_printf ("  PNEXT %d", 8900); } } while (0); pnext(p); } while (0);;
  if ((res = parse_block_or_stmt(p, 1)) != MJS_OK) return res;

  if (p->tok.tok == TOK_KEYWORD_ELSE) {





    size_t off_else, off_endelse;
    do { do { if (cs_log_print_prefix(LL_VERBOSE_DEBUG, "mjs.c", 8910)) { cs_log_printf ("  PNEXT %d", 8910); } } while (0); pnext(p); } while (0);
    emit_byte(p, OP_JMP);
    off_else = p->cur_idx;
    emit_init_offset(p);
    off_endif = p->cur_idx;

    emit_byte(p, OP_DROP);
    if ((res = parse_block_or_stmt(p, 1)) != MJS_OK) return res;
    off_endelse = p->cur_idx;





    off_endif += mjs_bcode_insert_offset(
        p, p->mjs, off_else, off_endelse - off_else - 1);
  } else {


    off_endif = p->cur_idx;
  }

  mjs_bcode_insert_offset(p, p->mjs, off_if,
                          off_endif - off_if - 1);

  return res;
}

static void pstate_revert(struct pstate *p, struct pstate *old,
                          int old_bcode_gen_len) {
  p->pos = old->pos;
  p->line_no = old->line_no;
  p->last_emitted_line_no = old->last_emitted_line_no;
  p->offset_lineno_map.len = old->offset_lineno_map.len;
  p->prev_tok = old->prev_tok;
  p->tok = old->tok;
  p->mjs->bcode_gen.len = old_bcode_gen_len;
  p->cur_idx = old->cur_idx;
  p->depth = old->depth;
}

static mjs_err_t parse_return(struct pstate *p) {
  int old_bcode_gen_len;
  struct pstate p_saved;
  if ((p)->tok.tok != (TOK_KEYWORD_RETURN)) do { mjs_set_errorf(p->mjs, MJS_SYNTAX_ERROR, "parse error at line %d: [%.*s]", p->line_no, 10, p->tok.ptr); return MJS_SYNTAX_ERROR; } while (0); else do { do { if (cs_log_print_prefix(LL_VERBOSE_DEBUG, "mjs.c", 8954)) { cs_log_printf ("  PNEXT %d", 8954); } } while (0); pnext(p); } while (0);;
  p_saved = *p;
  old_bcode_gen_len = p->mjs->bcode_gen.len;
  if (parse_expr(p) != MJS_OK) {




    pstate_revert(p, &p_saved, old_bcode_gen_len);
    emit_byte(p, OP_PUSH_UNDEF);
  }
  emit_byte(p, OP_SETRETVAL);
  emit_byte(p, OP_RETURN);
  return MJS_OK;
}

static mjs_err_t parse_statement(struct pstate *p) {
  do { if (cs_log_print_prefix(LL_VERBOSE_DEBUG, "mjs.c", 8971)) { cs_log_printf ("[%.*s]", 10, p->tok.ptr); } } while (0);
  switch (p->tok.tok) {
    case TOK_SEMICOLON:
      emit_byte(p, OP_PUSH_UNDEF);
      do { do { if (cs_log_print_prefix(LL_VERBOSE_DEBUG, "mjs.c", 8975)) { cs_log_printf ("  PNEXT %d", 8975); } } while (0); pnext(p); } while (0);
      return MJS_OK;
    case TOK_KEYWORD_LET:
      return parse_let(p);
    case TOK_OPEN_CURLY:
      return parse_block(p, 1);
    case TOK_KEYWORD_RETURN:
      return parse_return(p);
    case TOK_KEYWORD_FOR:
      return parse_for(p);
    case TOK_KEYWORD_WHILE:
      return parse_while(p);
    case TOK_KEYWORD_BREAK:
      emit_byte(p, OP_PUSH_UNDEF);
      emit_byte(p, OP_BREAK);
      do { do { if (cs_log_print_prefix(LL_VERBOSE_DEBUG, "mjs.c", 8990)) { cs_log_printf ("  PNEXT %d", 8990); } } while (0); pnext(p); } while (0);
      return MJS_OK;
    case TOK_KEYWORD_CONTINUE:
      emit_byte(p, OP_CONTINUE);
      do { do { if (cs_log_print_prefix(LL_VERBOSE_DEBUG, "mjs.c", 8994)) { cs_log_printf ("  PNEXT %d", 8994); } } while (0); pnext(p); } while (0);
      return MJS_OK;
    case TOK_KEYWORD_IF:
      return parse_if(p);
    case TOK_KEYWORD_CASE:
    case TOK_KEYWORD_CATCH:
    case TOK_KEYWORD_DELETE:
    case TOK_KEYWORD_DO:
    case TOK_KEYWORD_INSTANCEOF:
    case TOK_KEYWORD_NEW:
    case TOK_KEYWORD_SWITCH:
    case TOK_KEYWORD_THROW:
    case TOK_KEYWORD_TRY:
    case TOK_KEYWORD_VAR:
    case TOK_KEYWORD_VOID:
    case TOK_KEYWORD_WITH:
      mjs_set_errorf(p->mjs, MJS_SYNTAX_ERROR, "[%.*s] is not implemented",
                     p->tok.len, p->tok.ptr);
      return MJS_SYNTAX_ERROR;
    default: {
      mjs_err_t res = MJS_OK;
      for (;;) {
        if ((res = parse_expr(p)) != MJS_OK) return res;
        if (p->tok.tok != TOK_COMMA) break;
        emit_byte(p, OP_DROP);
        do { do { if (cs_log_print_prefix(LL_VERBOSE_DEBUG, "mjs.c", 9019)) { cs_log_printf ("  PNEXT %d", 9019); } } while (0); pnext(p); } while (0);
      }
      return res;
    }
  }
}

static mjs_err_t
mjs_parse(const char *path, const char *buf, struct mjs *mjs) {
  mjs_err_t res = MJS_OK;
  struct pstate p;
  size_t start_idx, llen;
  int map_len;
  mjs_header_item_t bcode_offset, map_offset, total_size;

  pinit(path, buf, &p);
  p.mjs = mjs;
  p.cur_idx = p.mjs->bcode_gen.len;
  emit_byte(&p, OP_BCODE_HEADER);







  start_idx = p.mjs->bcode_gen.len;
  mbuf_append(&p.mjs->bcode_gen, 
                                __null
                                    ,
              sizeof(mjs_header_item_t) * MJS_HDR_ITEMS_CNT);


  mbuf_append(&p.mjs->bcode_gen, path, strlen(path) + 1 );

  bcode_offset = p.mjs->bcode_gen.len - start_idx;
  memcpy(p.mjs->bcode_gen.buf + start_idx +
             sizeof(mjs_header_item_t) * MJS_HDR_ITEM_BCODE_OFFSET,
         &bcode_offset, sizeof(mjs_header_item_t));

  p.start_bcode_idx = p.mjs->bcode_gen.len;
  p.cur_idx = p.mjs->bcode_gen.len;

  res = parse_statement_list(&p, TOK_EOF);
  emit_byte(&p, OP_EXIT);


  map_offset = p.mjs->bcode_gen.len - start_idx;
  memcpy(p.mjs->bcode_gen.buf + start_idx +
             sizeof(mjs_header_item_t) * MJS_HDR_ITEM_MAP_OFFSET,
         &map_offset, sizeof(mjs_header_item_t));


  map_len = p.offset_lineno_map.len;
  llen = cs_varint_llen(map_len);
  mbuf_resize(&p.mjs->bcode_gen, p.mjs->bcode_gen.size + llen);
  cs_varint_encode(
      map_len, (uint8_t *) p.mjs->bcode_gen.buf + p.mjs->bcode_gen.len, llen);
  p.mjs->bcode_gen.len += llen;


  mbuf_append(&p.mjs->bcode_gen, p.offset_lineno_map.buf,
              p.offset_lineno_map.len);

  total_size = p.mjs->bcode_gen.len - start_idx;
  memcpy(p.mjs->bcode_gen.buf + start_idx +
             sizeof(mjs_header_item_t) * MJS_HDR_ITEM_TOTAL_SIZE,
         &total_size, sizeof(mjs_header_item_t));

  mbuf_free(&p.offset_lineno_map);





  if (res == MJS_OK) {
    mjs_bcode_commit(mjs);
  } else {
    mbuf_free(&mjs->bcode_gen);
  }

  return res;
}
mjs_val_t mjs_mk_null(void) {
  return ((uint64_t)(1) << 63 | (uint64_t) 0x7ff0 << 48 | (uint64_t)(15) << 48);
}

int mjs_is_null(mjs_val_t v) {
  return v == ((uint64_t)(1) << 63 | (uint64_t) 0x7ff0 << 48 | (uint64_t)(15) << 48);
}

mjs_val_t mjs_mk_undefined(void) {
  return ((uint64_t)(1) << 63 | (uint64_t) 0x7ff0 << 48 | (uint64_t)(3) << 48);
}

int mjs_is_undefined(mjs_val_t v) {
  return v == ((uint64_t)(1) << 63 | (uint64_t) 0x7ff0 << 48 | (uint64_t)(3) << 48);
}

mjs_val_t mjs_mk_number(struct mjs *mjs, double v) {
  mjs_val_t res;
  (void) mjs;

  if (isnan(v)) {
    res = ((uint64_t)(1) << 63 | (uint64_t) 0x7ff0 << 48 | (uint64_t)(5) << 48);
  } else {
    union {
      double d;
      mjs_val_t r;
    } u;
    u.d = v;
    res = u.r;
  }
  return res;
}

static double get_double(mjs_val_t v) {
  union {
    double d;
    mjs_val_t v;
  } u;
  u.v = v;

  return u.d;
}

double mjs_get_double(struct mjs *mjs, mjs_val_t v) {
  (void) mjs;
  return get_double(v);
}

int mjs_get_int(struct mjs *mjs, mjs_val_t v) {
  (void) mjs;




  return (int) (unsigned int) get_double(v);
}

int32_t mjs_get_int32(struct mjs *mjs, mjs_val_t v) {
  (void) mjs;
  return (int32_t) get_double(v);
}

int mjs_is_number(mjs_val_t v) {
  return v == ((uint64_t)(1) << 63 | (uint64_t) 0x7ff0 << 48 | (uint64_t)(5) << 48) || !isnan(get_double(v));
}

mjs_val_t mjs_mk_boolean(struct mjs *mjs, int v) {
  (void) mjs;
  return (v ? 1 : 0) | ((uint64_t)(1) << 63 | (uint64_t) 0x7ff0 << 48 | (uint64_t)(4) << 48);
}

int mjs_get_bool(struct mjs *mjs, mjs_val_t v) {
  (void) mjs;
  if (mjs_is_boolean(v)) {
    return v & 1;
  } else {
    return 0;
  }
}

int mjs_is_boolean(mjs_val_t v) {
  return (v & ((uint64_t)(1) << 63 | (uint64_t) 0x7ff0 << 48 | (uint64_t)(15) << 48)) == ((uint64_t)(1) << 63 | (uint64_t) 0x7ff0 << 48 | (uint64_t)(4) << 48);
}




static mjs_val_t mjs_pointer_to_value(struct mjs *mjs, void *p) {
  uint64_t n = ((uint64_t)(uintptr_t) p);

  if (!(((n) &((uint64_t)(1) << 63 | (uint64_t) 0x7ff0 << 48 | (uint64_t)(15) << 48)) == 0 || ((n) &((uint64_t)(1) << 63 | (uint64_t) 0x7ff0 << 48 | (uint64_t)(15) << 48)) == (~0 & ((uint64_t)(1) << 63 | (uint64_t) 0x7ff0 << 48 | (uint64_t)(15) << 48)))) {
    mjs_prepend_errorf(mjs, MJS_TYPE_ERROR, "invalid pointer value: %p", p);
  }
  return n & ~((uint64_t)(1) << 63 | (uint64_t) 0x7ff0 << 48 | (uint64_t)(15) << 48);
}

static mjs_val_t mjs_legit_pointer_to_value(void *p) {
  uint64_t n = ((uint64_t)(uintptr_t) p);

  
 (static_cast <bool> (
 (((n) &((uint64_t)(1) << 63 | (uint64_t) 0x7ff0 << 48 | (uint64_t)(15) << 48)) == 0 || ((n) &((uint64_t)(1) << 63 | (uint64_t) 0x7ff0 << 48 | (uint64_t)(15) << 48)) == (~0 & ((uint64_t)(1) << 63 | (uint64_t) 0x7ff0 << 48 | (uint64_t)(15) << 48)))
 ) ? void (0) : __assert_fail (
 "MJS_IS_POINTER_LEGIT(n)"
 , "mjs.c", 9211, __extension__ __PRETTY_FUNCTION__))
                                ;
  return n & ~((uint64_t)(1) << 63 | (uint64_t) 0x7ff0 << 48 | (uint64_t)(15) << 48);
}

static void *get_ptr(mjs_val_t v) {
  return (void *) (uintptr_t)(v & 0xFFFFFFFFFFFFUL);
}

void *mjs_get_ptr(struct mjs *mjs, mjs_val_t v) {
  (void) mjs;
  if (!mjs_is_foreign(v)) {
    return 
          __null
              ;
  }
  return get_ptr(v);
}

mjs_val_t mjs_mk_foreign(struct mjs *mjs, void *p) {
  (void) mjs;
  return mjs_pointer_to_value(mjs, p) | ((uint64_t)(1) << 63 | (uint64_t) 0x7ff0 << 48 | (uint64_t)(2) << 48);
}

mjs_val_t mjs_mk_foreign_func(struct mjs *mjs, mjs_func_ptr_t fn) {
  union {
    mjs_func_ptr_t fn;
    void *p;
  } u;
  u.fn = fn;
  (void) mjs;
  return mjs_pointer_to_value(mjs, u.p) | ((uint64_t)(1) << 63 | (uint64_t) 0x7ff0 << 48 | (uint64_t)(2) << 48);
}

int mjs_is_foreign(mjs_val_t v) {
  return (v & ((uint64_t)(1) << 63 | (uint64_t) 0x7ff0 << 48 | (uint64_t)(15) << 48)) == ((uint64_t)(1) << 63 | (uint64_t) 0x7ff0 << 48 | (uint64_t)(2) << 48);
}

mjs_val_t mjs_mk_function(struct mjs *mjs, size_t off) {
  (void) mjs;
  return (mjs_val_t) off | ((uint64_t)(1) << 63 | (uint64_t) 0x7ff0 << 48 | (uint64_t)(13) << 48);
}

int mjs_is_function(mjs_val_t v) {
  return (v & ((uint64_t)(1) << 63 | (uint64_t) 0x7ff0 << 48 | (uint64_t)(15) << 48)) == ((uint64_t)(1) << 63 | (uint64_t) 0x7ff0 << 48 | (uint64_t)(13) << 48);
}

static void mjs_op_isnan(struct mjs *mjs) {
  mjs_val_t ret = ((uint64_t)(1) << 63 | (uint64_t) 0x7ff0 << 48 | (uint64_t)(3) << 48);
  mjs_val_t val = mjs_arg(mjs, 0);

  ret = mjs_mk_boolean(mjs, val == ((uint64_t)(1) << 63 | (uint64_t) 0x7ff0 << 48 | (uint64_t)(5) << 48));

  mjs_return(mjs, ret);
}
typedef unsigned short Rune;
static int chartorune(Rune *rune, const char *str) {
  *rune = *(unsigned char *) str;
  return 1;
}
static int runetochar(char *str, Rune *rune) {
  str[0] = (char) *rune;
  return 1;
}





static size_t unescape(const char *s, size_t len, char *to);

static void embed_string(struct mbuf *m, size_t offset, const char *p,
                              size_t len, uint8_t flags);




int mjs_is_string(mjs_val_t v) {
  uint64_t t = v & ((uint64_t)(1) << 63 | (uint64_t) 0x7ff0 << 48 | (uint64_t)(15) << 48);
  return t == ((uint64_t)(1) << 63 | (uint64_t) 0x7ff0 << 48 | (uint64_t)(6) << 48) || t == ((uint64_t)(1) << 63 | (uint64_t) 0x7ff0 << 48 | (uint64_t)(9) << 48) ||
         t == ((uint64_t)(1) << 63 | (uint64_t) 0x7ff0 << 48 | (uint64_t)(8) << 48) || t == ((uint64_t)(1) << 63 | (uint64_t) 0x7ff0 << 48 | (uint64_t)(7) << 48) ||
         t == ((uint64_t)(1) << 63 | (uint64_t) 0x7ff0 << 48 | (uint64_t)(11) << 48);
}

mjs_val_t mjs_mk_string(struct mjs *mjs, const char *p, size_t len, int copy) {
  struct mbuf *m;
  mjs_val_t offset, tag = ((uint64_t)(1) << 63 | (uint64_t) 0x7ff0 << 48 | (uint64_t)(9) << 48);
  if (len == 0) {






    copy = 1;
  }
  m = copy ? &mjs->owned_strings : &mjs->foreign_strings;
  offset = m->len;

  if (len == ~((size_t) 0)) len = strlen(p);

  if (copy) {

    if (len <= 4) {
      char *s = ((char *) &(offset)) + 1;
      offset = 0;
      if (p != 0) {
        memcpy(s, p, len);
      }
      s[-1] = len;
      tag = ((uint64_t)(1) << 63 | (uint64_t) 0x7ff0 << 48 | (uint64_t)(6) << 48);
    } else if (len == 5) {
      char *s = ((char *) &(offset));
      offset = 0;
      if (p != 0) {
        memcpy(s, p, len);
      }
      tag = ((uint64_t)(1) << 63 | (uint64_t) 0x7ff0 << 48 | (uint64_t)(7) << 48);




    } else {
      if (gc_strings_is_gc_needed(mjs)) {
        mjs->need_gc = 1;
      }






      if ((m->len + len) > m->size) {
        char *prev_buf = m->buf;
        mbuf_resize(m, m->len + len + 100);






        if (p >= prev_buf && p < (prev_buf + m->len)) {
          p += (m->buf - prev_buf);
        }
      }

      embed_string(m, m->len, p, len, 1);
      tag = ((uint64_t)(1) << 63 | (uint64_t) 0x7ff0 << 48 | (uint64_t)(8) << 48);
    }
  } else {

    if (sizeof(void *) <= 4 && len <= (1 << 15)) {

      offset = (uint64_t) len << 32 | (uint64_t)(uintptr_t) p;
    } else {

      size_t pos = m->len;
      size_t llen = cs_varint_llen(len);


      mbuf_insert(m, pos, 
                         __null
                             , llen + sizeof(p));

      cs_varint_encode(len, (uint8_t *) (m->buf + pos), llen);
      memcpy(m->buf + pos + llen, &p, sizeof(p));
    }
    tag = ((uint64_t)(1) << 63 | (uint64_t) 0x7ff0 << 48 | (uint64_t)(9) << 48);
  }


  return (offset & ~((uint64_t)(1) << 63 | (uint64_t) 0x7ff0 << 48 | (uint64_t)(15) << 48)) | tag;
}


const char *mjs_get_string(struct mjs *mjs, mjs_val_t *v, size_t *sizep) {
  uint64_t tag = v[0] & ((uint64_t)(1) << 63 | (uint64_t) 0x7ff0 << 48 | (uint64_t)(15) << 48);
  const char *p = 
                 __null
                     ;
  size_t size = 0, llen;

  if (!mjs_is_string(*v)) {
    goto clean;
  }

  if (tag == ((uint64_t)(1) << 63 | (uint64_t) 0x7ff0 << 48 | (uint64_t)(6) << 48)) {
    p = ((char *) &(*v)) + 1;
    size = p[-1];
  } else if (tag == ((uint64_t)(1) << 63 | (uint64_t) 0x7ff0 << 48 | (uint64_t)(7) << 48)) {
    p = ((char *) &(*v));
    size = 5;




  } else if (tag == ((uint64_t)(1) << 63 | (uint64_t) 0x7ff0 << 48 | (uint64_t)(8) << 48)) {
    size_t offset = (size_t) gc_string_mjs_val_to_offset(*v);
    char *s = mjs->owned_strings.buf + offset;
    uint64_t v = 0;
    if (offset < mjs->owned_strings.len &&
        cs_varint_decode((uint8_t *) s, mjs->owned_strings.len - offset, &v,
                         &llen)) {
      size = v;
      p = s + llen;
    } else {
      goto clean;
    }
  } else if (tag == ((uint64_t)(1) << 63 | (uint64_t) 0x7ff0 << 48 | (uint64_t)(9) << 48)) {
    uint16_t len = (*v >> 32) & 0xFFFF;
    if (sizeof(void *) <= 4 && len != 0) {
      size = (size_t) len;
      p = (const char *) (uintptr_t) *v;
    } else {
      size_t offset = (size_t) gc_string_mjs_val_to_offset(*v);
      char *s = mjs->foreign_strings.buf + offset;
      uint64_t v = 0;
      if (offset < mjs->foreign_strings.len &&
          cs_varint_decode((uint8_t *) s, mjs->foreign_strings.len - offset, &v,
                           &llen)) {
        size = v;
        memcpy((char **) &p, s + llen, sizeof(p));
      } else {
        goto clean;
      }
    }
  } else {
    
   (static_cast <bool> (
   0
   ) ? void (0) : __assert_fail (
   "0"
   , "mjs.c", 9464, __extension__ __PRETTY_FUNCTION__))
            ;
  }

clean:
  if (sizep != 
              __null
                  ) {
    *sizep = size;
  }
  return p;
}

const char *mjs_get_cstring(struct mjs *mjs, mjs_val_t *value) {
  size_t size;
  const char *s = mjs_get_string(mjs, value, &size);
  if (s == 
          __null
              ) return 
                       __null
                           ;
  if (s[size] != 0 || strlen(s) != size) {
    return 
          __null
              ;
  }
  return s;
}

int mjs_strcmp(struct mjs *mjs, mjs_val_t *a, const char *b, size_t len) {
  size_t n;
  const char *s;
  if (len == (size_t) ~0) len = strlen(b);
  s = mjs_get_string(mjs, a, &n);
  if (n != len) {
    return n - len;
  }
  return strncmp(s, b, len);
}

static unsigned long cstr_to_ulong(const char *s, size_t len, int *ok) {
  char *e;
  unsigned long res = strtoul(s, &e, 10);
  *ok = (e == s + len) && len != 0;
  return res;
}

static mjs_err_t
str_to_ulong(struct mjs *mjs, mjs_val_t v, int *ok, unsigned long *res) {
  enum mjs_err ret = MJS_OK;
  size_t len = 0;
  const char *p = mjs_get_string(mjs, &v, &len);
  *res = cstr_to_ulong(p, len, ok);

  return ret;
}

static int s_cmp(struct mjs *mjs, mjs_val_t a, mjs_val_t b) {
  size_t a_len, b_len;
  const char *a_ptr, *b_ptr;

  a_ptr = mjs_get_string(mjs, &a, &a_len);
  b_ptr = mjs_get_string(mjs, &b, &b_len);

  if (a_len == b_len) {
    return memcmp(a_ptr, b_ptr, a_len);
  }
  if (a_len > b_len) {
    return 1;
  } else if (a_len < b_len) {
    return -1;
  } else {
    return 0;
  }
}

static mjs_val_t s_concat(struct mjs *mjs, mjs_val_t a, mjs_val_t b) {
  size_t a_len, b_len, res_len;
  const char *a_ptr, *b_ptr, *res_ptr;
  mjs_val_t res;


  a_ptr = mjs_get_string(mjs, &a, &a_len);
  b_ptr = mjs_get_string(mjs, &b, &b_len);


  res = mjs_mk_string(mjs, 
                          __null
                              , a_len + b_len, 1);


  a_ptr = mjs_get_string(mjs, &a, &a_len);
  b_ptr = mjs_get_string(mjs, &b, &b_len);


  res_ptr = mjs_get_string(mjs, &res, &res_len);
  memcpy((char *) res_ptr, a_ptr, a_len);
  memcpy((char *) res_ptr + a_len, b_ptr, b_len);

  return res;
}

static void mjs_string_slice(struct mjs *mjs) {
  int nargs = mjs_nargs(mjs);
  mjs_val_t ret = mjs_mk_number(mjs, 0);
  mjs_val_t beginSlice_v = ((uint64_t)(1) << 63 | (uint64_t) 0x7ff0 << 48 | (uint64_t)(3) << 48);
  mjs_val_t endSlice_v = ((uint64_t)(1) << 63 | (uint64_t) 0x7ff0 << 48 | (uint64_t)(3) << 48);
  int beginSlice = 0;
  int endSlice = 0;
  size_t size;
  const char *s = 
                 __null
                     ;


  if (!mjs_check_arg(mjs, -1 , "this", MJS_TYPE_STRING, 
                                                               __null
                                                                   )) {
    goto clean;
  }
  s = mjs_get_string(mjs, &mjs->vals.this_obj, &size);


  if (!mjs_check_arg(mjs, 0, "beginSlice", MJS_TYPE_NUMBER, &beginSlice_v)) {
    goto clean;
  }
  beginSlice = mjs_normalize_idx(mjs_get_int(mjs, beginSlice_v), size);

  if (nargs >= 2) {


    if (!mjs_check_arg(mjs, 1, "endSlice", MJS_TYPE_NUMBER, &endSlice_v)) {
      goto clean;
    }
    endSlice = mjs_normalize_idx(mjs_get_int(mjs, endSlice_v), size);
  } else {

    endSlice = size;
  }

  if (endSlice < beginSlice) {
    endSlice = beginSlice;
  }

  ret = mjs_mk_string(mjs, s + beginSlice, endSlice - beginSlice, 1);

clean:
  mjs_return(mjs, ret);
}

static void mjs_string_index_of(struct mjs *mjs) {
  mjs_val_t ret = mjs_mk_number(mjs, -1);
  mjs_val_t substr_v = ((uint64_t)(1) << 63 | (uint64_t) 0x7ff0 << 48 | (uint64_t)(3) << 48);
  mjs_val_t idx_v = ((uint64_t)(1) << 63 | (uint64_t) 0x7ff0 << 48 | (uint64_t)(3) << 48);
  int idx = 0;
  const char *str = 
                   __null
                       , *substr = 
                                   __null
                                       ;
  size_t str_len = 0, substr_len = 0;

  if (!mjs_check_arg(mjs, -1 , "this", MJS_TYPE_STRING, 
                                                                 __null
                                                                     )) {
    goto clean;
  }
  str = mjs_get_string(mjs, &mjs->vals.this_obj, &str_len);

  if (!mjs_check_arg(mjs, 0, "searchValue", MJS_TYPE_STRING, &substr_v)) {
    goto clean;
  }
  substr = mjs_get_string(mjs, &substr_v, &substr_len);
  if (mjs_nargs(mjs) > 1) {
    if (!mjs_check_arg(mjs, 1, "fromIndex", MJS_TYPE_NUMBER, &idx_v)) {
      goto clean;
    }
    idx = mjs_get_int(mjs, idx_v);
    if (idx < 0) idx = 0;
    if ((size_t) idx > str_len) idx = str_len;
  }
  {
    const char *substr_p;
    struct mg_str mgstr, mgsubstr;
    mgstr.p = str + idx;
    mgstr.len = str_len - idx;
    mgsubstr.p = substr;
    mgsubstr.len = substr_len;
    substr_p = mg_strstr(mgstr, mgsubstr);
    if (substr_p != 
                   __null
                       ) {
      ret = mjs_mk_number(mjs, (int) (substr_p - str));
    }
  }

clean:
  mjs_return(mjs, ret);
}

static void mjs_string_char_code_at(struct mjs *mjs) {
  mjs_val_t ret = ((uint64_t)(1) << 63 | (uint64_t) 0x7ff0 << 48 | (uint64_t)(3) << 48);
  mjs_val_t idx_v = ((uint64_t)(1) << 63 | (uint64_t) 0x7ff0 << 48 | (uint64_t)(3) << 48);
  int idx = 0;
  size_t size;
  const char *s = 
                 __null
                     ;


  if (!mjs_check_arg(mjs, -1 , "this", MJS_TYPE_STRING, 
                                                               __null
                                                                   )) {
    goto clean;
  }
  s = mjs_get_string(mjs, &mjs->vals.this_obj, &size);


  if (!mjs_check_arg(mjs, 0, "index", MJS_TYPE_NUMBER, &idx_v)) {
    goto clean;
  }
  idx = mjs_normalize_idx(mjs_get_int(mjs, idx_v), size);
  if (idx >= 0 && idx < (int) size) {
    ret = mjs_mk_number(mjs, ((unsigned char *) s)[idx]);
  }

clean:
  mjs_return(mjs, ret);
}

static void mjs_mkstr(struct mjs *mjs) {
  int nargs = mjs_nargs(mjs);
  mjs_val_t ret = ((uint64_t)(1) << 63 | (uint64_t) 0x7ff0 << 48 | (uint64_t)(3) << 48);

  char *ptr = 
             __null
                 ;
  int offset = 0;
  int len = 0;
  int copy = 0;

  mjs_val_t ptr_v = ((uint64_t)(1) << 63 | (uint64_t) 0x7ff0 << 48 | (uint64_t)(3) << 48);
  mjs_val_t offset_v = ((uint64_t)(1) << 63 | (uint64_t) 0x7ff0 << 48 | (uint64_t)(3) << 48);
  mjs_val_t len_v = ((uint64_t)(1) << 63 | (uint64_t) 0x7ff0 << 48 | (uint64_t)(3) << 48);
  mjs_val_t copy_v = ((uint64_t)(1) << 63 | (uint64_t) 0x7ff0 << 48 | (uint64_t)(3) << 48);

  if (nargs == 2) {
    ptr_v = mjs_arg(mjs, 0);
    len_v = mjs_arg(mjs, 1);
  } else if (nargs == 3) {
    ptr_v = mjs_arg(mjs, 0);
    offset_v = mjs_arg(mjs, 1);
    len_v = mjs_arg(mjs, 2);
  } else if (nargs == 4) {
    ptr_v = mjs_arg(mjs, 0);
    offset_v = mjs_arg(mjs, 1);
    len_v = mjs_arg(mjs, 2);
    copy_v = mjs_arg(mjs, 3);
  } else {
    mjs_prepend_errorf(mjs, MJS_TYPE_ERROR,
                       "mkstr takes 2, 3 or 4 arguments: (ptr, len), (ptr, "
                       "offset, len) or (ptr, offset, len, copy)");
    goto clean;
  }

  if (!mjs_is_foreign(ptr_v)) {
    mjs_prepend_errorf(mjs, MJS_TYPE_ERROR, "ptr should be a foreign pointer");
    goto clean;
  }

  if (offset_v != ((uint64_t)(1) << 63 | (uint64_t) 0x7ff0 << 48 | (uint64_t)(3) << 48) && !mjs_is_number(offset_v)) {
    mjs_prepend_errorf(mjs, MJS_TYPE_ERROR, "offset should be a number");
    goto clean;
  }

  if (!mjs_is_number(len_v)) {
    mjs_prepend_errorf(mjs, MJS_TYPE_ERROR, "len should be a number");
    goto clean;
  }

  copy = mjs_is_truthy(mjs, copy_v);



  ptr = (char *) mjs_get_ptr(mjs, ptr_v);
  if (offset_v != ((uint64_t)(1) << 63 | (uint64_t) 0x7ff0 << 48 | (uint64_t)(3) << 48)) {
    offset = mjs_get_int(mjs, offset_v);
  }
  len = mjs_get_int(mjs, len_v);

  ret = mjs_mk_string(mjs, ptr + offset, len, copy);

clean:
  mjs_return(mjs, ret);
}

enum unescape_error {
  SLRE_INVALID_HEX_DIGIT,
  SLRE_INVALID_ESC_CHAR,
  SLRE_UNTERM_ESC_SEQ,
};

static int hex(int c) {
  if (c >= '0' && c <= '9') return c - '0';
  if (c >= 'a' && c <= 'f') return c - 'a' + 10;
  if (c >= 'A' && c <= 'F') return c - 'A' + 10;
  return -SLRE_INVALID_HEX_DIGIT;
}

static int nextesc(const char **p) {
  const unsigned char *s = (unsigned char *) (*p)++;
  switch (*s) {
    case 0:
      return -SLRE_UNTERM_ESC_SEQ;
    case 'c':
      ++*p;
      return *s & 31;
    case 'b':
      return '\b';
    case 't':
      return '\t';
    case 'n':
      return '\n';
    case 'v':
      return '\v';
    case 'f':
      return '\f';
    case 'r':
      return '\r';
    case '\\':
      return '\\';
    case 'u':
      if (isxdigit(s[1]) && isxdigit(s[2]) && isxdigit(s[3]) &&
          isxdigit(s[4])) {
        (*p) += 4;
        return hex(s[1]) << 12 | hex(s[2]) << 8 | hex(s[3]) << 4 | hex(s[4]);
      }
      return -SLRE_INVALID_HEX_DIGIT;
    case 'x':
      if (isxdigit(s[1]) && isxdigit(s[2])) {
        (*p) += 2;
        return (hex(s[1]) << 4) | hex(s[2]);
      }
      return -SLRE_INVALID_HEX_DIGIT;
    default:
      return -SLRE_INVALID_ESC_CHAR;
  }
}

static size_t unescape(const char *s, size_t len, char *to) {
  const char *end = s + len;
  size_t n = 0;
  char tmp[4];
  Rune r;

  while (s < end) {
    s += chartorune(&r, s);
    if (r == '\\' && s < end) {
      switch (*s) {
        case '"':
          s++, r = '"';
          break;
        case '\'':
          s++, r = '\'';
          break;
        case '\n':
          s++, r = '\n';
          break;
        default: {
          const char *tmp_s = s;
          int i = nextesc(&s);
          switch (i) {
            case -SLRE_INVALID_ESC_CHAR:
              r = '\\';
              s = tmp_s;
              n += runetochar(to == 
                                   __null 
                                        ? tmp : to + n, &r);
              s += chartorune(&r, s);
              break;
            case -SLRE_INVALID_HEX_DIGIT:
            default:
              r = i;
          }
        }
      }
    }
    n += runetochar(to == 
                         __null 
                              ? tmp : to + n, &r);
  }

  return n;
}

static void embed_string(struct mbuf *m, size_t offset, const char *p,
                              size_t len, uint8_t flags) {
  char *old_base = m->buf;
  uint8_t p_backed_by_mbuf = p >= old_base && p < old_base + m->len;
  size_t n = (flags & 2) ? unescape(p, len, 
                                                         __null
                                                             ) : len;


  size_t k = cs_varint_llen(n);


  size_t tot_len = k + n + !!(flags & 1);


  mbuf_insert(m, offset, 
                        __null
                            , tot_len);


  if (p_backed_by_mbuf) {
    p += m->buf - old_base;
  }


  cs_varint_encode(n, (unsigned char *) m->buf + offset, k);


  if (p != 0) {
    if (flags & 2) {
      unescape(p, len, m->buf + offset + k);
    } else {
      memcpy(m->buf + offset + k, p, len);
    }
  }


  if (flags & 1) {
    m->buf[offset + tot_len - 1] = '\0';
  }
}





static void pinit(const char *file_name, const char *buf,
                       struct pstate *p) {
  memset(p, 0, sizeof(*p));
  p->line_no = 1;
  p->last_emitted_line_no = 1;
  p->file_name = file_name;
  p->buf = p->pos = buf;
  mbuf_init(&p->offset_lineno_map, 0);
}



static int mjs_is_space(int c) {
  return c == ' ' || c == '\r' || c == '\n' || c == '\t' || c == '\f' ||
         c == '\v';
}

static int mjs_is_digit(int c) {
  return c >= '0' && c <= '9';
}

static int mjs_is_alpha(int c) {
  return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

static int mjs_is_ident(int c) {
  return c == '_' || c == '$' || mjs_is_alpha(c);
}


static int longtok(struct pstate *p, const char *first_chars,
                   const char *second_chars) {
  if (strchr(first_chars, p->pos[0]) == 
                                       __null
                                           ) return TOK_EOF;
  if (p->pos[1] != '\0' && strchr(second_chars, p->pos[1]) != 
                                                             __null
                                                                 ) {
    p->tok.len++;
    p->pos++;
    return p->pos[-1] << 8 | p->pos[0];
  }
  return p->pos[0];
}


static int longtok3(struct pstate *p, char a, char b, char c) {
  if (p->pos[0] == a && p->pos[1] == b && p->pos[2] == c) {
    p->tok.len += 2;
    p->pos += 2;
    return p->pos[-2] << 16 | p->pos[-1] << 8 | p->pos[0];
  }
  return TOK_EOF;
}


static int longtok4(struct pstate *p, char a, char b, char c, char d) {
  if (p->pos[0] == a && p->pos[1] == b && p->pos[2] == c && p->pos[3] == d) {
    p->tok.len += 3;
    p->pos += 3;
    return p->pos[-3] << 24 | p->pos[-2] << 16 | p->pos[-1] << 8 | p->pos[0];
  }
  return TOK_EOF;
}

static int getnum(struct pstate *p) {
  if (p->pos[0] == '0' && p->pos[1] == 'x') {

    strtoul(p->pos + 2, (char **) &p->pos, 16);
  } else {
    strtod(p->pos, (char **) &p->pos);
  }
  p->tok.len = p->pos - p->tok.ptr;
  p->pos--;
  return TOK_NUM;
}

static int is_reserved_word_token(const char *s, int len) {
  const char *reserved[] = {
      "break", "case", "catch", "continue", "debugger", "default",
      "delete", "do", "else", "false", "finally", "for",
      "function", "if", "in", "instanceof", "new", "null",
      "return", "switch", "this", "throw", "true", "try",
      "typeof", "var", "void", "while", "with", "let",
      "undefined", 
                  __null
                      };
  int i;
  if (!mjs_is_alpha(s[0])) return 0;
  for (i = 0; reserved[i] != 
                            __null
                                ; i++) {
    if (len == (int) strlen(reserved[i]) && strncmp(s, reserved[i], len) == 0)
      return i + 1;
  }
  return 0;
}

static int getident(struct pstate *p) {
  while (mjs_is_ident(p->pos[0]) || mjs_is_digit(p->pos[0])) p->pos++;
  p->tok.len = p->pos - p->tok.ptr;
  p->pos--;
  return TOK_IDENT;
}

static int getstr(struct pstate *p) {
  int quote = *p->pos++;
  p->tok.ptr++;
  while (p->pos[0] != '\0' && p->pos[0] != quote) {
    if (p->pos[0] == '\\' && p->pos[1] != '\0' &&
        (p->pos[1] == quote || strchr("bfnrtv\\", p->pos[1]) != 
                                                               __null
                                                                   )) {
      p->pos += 2;
    } else {
      p->pos++;
    }
  }
  p->tok.len = p->pos - p->tok.ptr;
  return TOK_STR;
}

static void skip_spaces_and_comments(struct pstate *p) {
  const char *pos;
  do {
    pos = p->pos;
    while (mjs_is_space(p->pos[0])) {
      if (p->pos[0] == '\n') p->line_no++;
      p->pos++;
    }
    if (p->pos[0] == '/' && p->pos[1] == '/') {
      while (p->pos[0] != '\0' && p->pos[0] != '\n') p->pos++;
    }
    if (p->pos[0] == '/' && p->pos[1] == '*') {
      p->pos += 2;
      while (p->pos[0] != '\0') {
        if (p->pos[0] == '\n') p->line_no++;
        if (p->pos[0] == '*' && p->pos[1] == '/') {
          p->pos += 2;
          break;
        }
        p->pos++;
      }
    }
  } while (pos < p->pos);
}

static int ptranslate(int tok) {





  switch (tok) {
    case ':': return TOK_COLON;
    case ';': return TOK_SEMICOLON;
    case ',': return TOK_COMMA;
    case '=': return TOK_ASSIGN;
    case '{': return TOK_OPEN_CURLY;
    case '}': return TOK_CLOSE_CURLY;
    case '(': return TOK_OPEN_PAREN;
    case ')': return TOK_CLOSE_PAREN;
    case '[': return TOK_OPEN_BRACKET;
    case ']': return TOK_CLOSE_BRACKET;
    case '*': return TOK_MUL;
    case '+': return TOK_PLUS;
    case '-': return TOK_MINUS;
    case '/': return TOK_DIV;
    case '%': return TOK_REM;
    case '&': return TOK_AND;
    case '|': return TOK_OR;
    case '^': return TOK_XOR;
    case '.': return TOK_DOT;
    case '?': return TOK_QUESTION;
    case '!': return TOK_NOT;
    case '~': return TOK_TILDA;
    case '<': return TOK_LT;
    case '>': return TOK_GT;
    case (('<') << 8 | ('<')): return TOK_LSHIFT;
    case (('>') << 8 | ('>')): return TOK_RSHIFT;
    case (('-') << 8 | ('-')): return TOK_MINUS_MINUS;
    case (('+') << 8 | ('+')): return TOK_PLUS_PLUS;
    case (('+') << 8 | ('=')): return TOK_PLUS_ASSIGN;
    case (('-') << 8 | ('=')): return TOK_MINUS_ASSIGN;
    case (('*') << 8 | ('=')): return TOK_MUL_ASSIGN;
    case (('/') << 8 | ('=')): return TOK_DIV_ASSIGN;
    case (('&') << 8 | ('=')): return TOK_AND_ASSIGN;
    case (('|') << 8 | ('=')): return TOK_OR_ASSIGN;
    case (('%') << 8 | ('=')): return TOK_REM_ASSIGN;
    case (('^') << 8 | ('=')): return TOK_XOR_ASSIGN;
    case (('=') << 8 | ('=')): return TOK_EQ;
    case (('!') << 8 | ('=')): return TOK_NE;
    case (('<') << 8 | ('=')): return TOK_LE;
    case (('>') << 8 | ('=')): return TOK_GE;
    case (('&') << 8 | ('&')): return TOK_LOGICAL_AND;
    case (('|') << 8 | ('|')): return TOK_LOGICAL_OR;
    case (('=') << 16 | ('=') << 8 | ('=')): return TOK_EQ_EQ;
    case (('!') << 16 | ('=') << 8 | ('=')): return TOK_NE_NE;
    case (('<') << 16 | ('<') << 8 | ('=')): return TOK_LSHIFT_ASSIGN;
    case (('>') << 16 | ('>') << 8 | ('=')): return TOK_RSHIFT_ASSIGN;
    case (('>') << 16 | ('>') << 8 | ('>')): return TOK_URSHIFT;
    case (('>') << 24 | ('>') << 16 | ('>') << 8 | ('=')): return TOK_URSHIFT_ASSIGN;
  }

  return tok;
}

static int pnext(struct pstate *p) {
  int tmp, tok = TOK_INVALID;

  skip_spaces_and_comments(p);
  p->tok.ptr = p->pos;
  p->tok.len = 1;

  if (p->pos[0] == '\0') {
    tok = TOK_EOF;
  } else if (mjs_is_digit(p->pos[0])) {
    tok = getnum(p);
  } else if (p->pos[0] == '\'' || p->pos[0] == '"') {
    tok = getstr(p);
  } else if (mjs_is_ident(p->pos[0])) {
    tok = getident(p);






    tok += is_reserved_word_token(p->tok.ptr, p->tok.len);
  } else if (strchr(",.:;{}[]()?", p->pos[0]) != 
                                                __null
                                                    ) {
    tok = p->pos[0];
  } else if ((tmp = longtok3(p, '<', '<', '=')) != TOK_EOF ||
             (tmp = longtok3(p, '>', '>', '=')) != TOK_EOF ||
             (tmp = longtok4(p, '>', '>', '>', '=')) != TOK_EOF ||
             (tmp = longtok3(p, '>', '>', '>')) != TOK_EOF ||
             (tmp = longtok3(p, '=', '=', '=')) != TOK_EOF ||
             (tmp = longtok3(p, '!', '=', '=')) != TOK_EOF ||
             (tmp = longtok(p, "&", "&=")) != TOK_EOF ||
             (tmp = longtok(p, "|", "|=")) != TOK_EOF ||
             (tmp = longtok(p, "<", "<=")) != TOK_EOF ||
             (tmp = longtok(p, ">", ">=")) != TOK_EOF ||
             (tmp = longtok(p, "-", "-=")) != TOK_EOF ||
             (tmp = longtok(p, "+", "+=")) != TOK_EOF) {
    tok = tmp;
  } else if ((tmp = longtok(p, "^~+-%/*<>=!|&", "=")) != TOK_EOF) {
    tok = tmp;
  }
  if (p->pos[0] != '\0') p->pos++;
  do { if (cs_log_print_prefix(LL_VERBOSE_DEBUG, "mjs.c", 10115)) { cs_log_printf ("  --> %d [%.*s]", tok, p->tok.len, p->tok.ptr); } } while (0);
  p->prev_tok = p->tok.tok;
  p->tok.tok = ptranslate(tok);
  return p->tok.tok;
}
const char *mjs_typeof(mjs_val_t v) {
  return mjs_stringify_type(mjs_get_type(v));
}

static const char *mjs_stringify_type(enum mjs_type t) {
  switch (t) {
    case MJS_TYPE_NUMBER:
      return "number";
    case MJS_TYPE_BOOLEAN:
      return "boolean";
    case MJS_TYPE_STRING:
      return "string";
    case MJS_TYPE_OBJECT_ARRAY:
      return "array";
    case MJS_TYPE_OBJECT_GENERIC:
      return "object";
    case MJS_TYPE_FOREIGN:
      return "foreign_ptr";
    case MJS_TYPE_OBJECT_FUNCTION:
      return "function";
    case MJS_TYPE_NULL:
      return "null";
    case MJS_TYPE_UNDEFINED:
      return "undefined";
    default:
      return "???";
  }
}

void mjs_jprintf(mjs_val_t v, struct mjs *mjs, struct json_out *out) {
  if (mjs_is_number(v)) {
    double iv, d = mjs_get_double(mjs, v);
    if (modf(d, &iv) == 0) {
      json_printf(out, "%" 
                          "l" "d"
                                   , (int64_t) d);
    } else {
      json_printf(out, "%f", mjs_get_double(mjs, v));
    }
  } else if (mjs_is_boolean(v)) {
    json_printf(out, "%s", mjs_get_bool(mjs, v) ? "true" : "false");
  } else if (mjs_is_string(v)) {
    size_t i, size;
    const char *s = mjs_get_string(mjs, &v, &size);
    for (i = 0; i < size; i++) {
      int ch = ((unsigned char *) s)[i];
      if (isprint(ch)) {
        json_printf(out, "%c", ch);
      } else {
        json_printf(out, "%s%02x", "\\x", ch);
      }
    }
  } else if (mjs_is_array(v)) {
    json_printf(out, "%s", "<array>");
  } else if (mjs_is_object(v)) {
    json_printf(out, "%s", "<object>");
  } else if (mjs_is_foreign(v)) {
    json_printf(out, "%s%lx%s", "<foreign_ptr@",
                (unsigned long) (uintptr_t) mjs_get_ptr(mjs, v), ">");
  } else if (mjs_is_function(v)) {
    json_printf(out, "%s%d%s", "<function@", (int) mjs_get_func_addr(v), ">");
  } else if (mjs_is_null(v)) {
    json_printf(out, "%s", "null");
  } else if (mjs_is_undefined(v)) {
    json_printf(out, "%s", "undefined");
  } else {
    json_printf(out, "%s%" 
                          "l" "d" 
                                    "%s", "<???", (int64_t) v, ">");
  }
}

void mjs_sprintf(mjs_val_t v, struct mjs *mjs, char *buf, size_t n) {
  struct json_out out = { json_printer_buf, { { buf, n, 0 } } };
  mjs_jprintf(v, mjs, &out);
}

void mjs_fprintf(mjs_val_t v, struct mjs *mjs, FILE *fp) {
  struct json_out out = { json_printer_file, { { (char *) fp, 0, 0 } } };
  mjs_jprintf(v, mjs, &out);
}



static const char *opcodetostr(uint8_t opcode) {
  static const char *names[] = {
      "NOP", "DROP", "DUP", "SWAP", "JMP", "JMP_TRUE", "JMP_NEUTRAL_TRUE",
      "JMP_FALSE", "JMP_NEUTRAL_FALSE", "FIND_SCOPE", "PUSH_SCOPE", "PUSH_STR",
      "PUSH_TRUE", "PUSH_FALSE", "PUSH_INT", "PUSH_DBL", "PUSH_NULL",
      "PUSH_UNDEF", "PUSH_OBJ", "PUSH_ARRAY", "PUSH_FUNC", "PUSH_THIS", "GET",
      "CREATE", "EXPR", "APPEND", "SET_ARG", "NEW_SCOPE", "DEL_SCOPE", "CALL",
      "RETURN", "LOOP", "BREAK", "CONTINUE", "SETRETVAL", "EXIT", "BCODE_HDR",
      "ARGS", "FOR_IN_NEXT",
  };
  const char *name = "???";
  
 (static_cast <bool> (
 (sizeof(names) / sizeof(names[0])) == OP_MAX
 ) ? void (0) : __assert_fail (
 "ARRAY_SIZE(names) == OP_MAX"
 , "mjs.c", 10231, __extension__ __PRETTY_FUNCTION__))
                                    ;
  if (opcode < (sizeof(names) / sizeof(names[0]))) name = names[opcode];
  return name;
}

static size_t mjs_disasm_single(const uint8_t *code, size_t i) {
  char buf[40];
  size_t start_i = i;
  size_t llen;
  uint64_t n;

  snprintf(buf, sizeof(buf), "\t%-3u %-8s", (unsigned) i, opcodetostr(code[i]));

  switch (code[i]) {
    case OP_PUSH_FUNC: {
      cs_varint_decode(&code[i + 1], ~0, &n, &llen);
      do { if (cs_log_print_prefix(LL_VERBOSE_DEBUG, "mjs.c", 10247)) { cs_log_printf ("%s %04u", buf, (unsigned) (i - n)); } } while (0);
      i += llen;
      break;
    }
    case OP_PUSH_INT: {
      cs_varint_decode(&code[i + 1], ~0, &n, &llen);
      do { if (cs_log_print_prefix(LL_VERBOSE_DEBUG, "mjs.c", 10253)) { cs_log_printf ("%s\t%lu", buf, (unsigned long) n); } } while (0);
      i += llen;
      break;
    }
    case OP_SET_ARG: {
      size_t llen2;
      uint64_t arg_no;
      cs_varint_decode(&code[i + 1], ~0, &arg_no, &llen);
      cs_varint_decode(&code[i + llen + 1], ~0, &n, &llen2);
      do { if (cs_log_print_prefix(LL_VERBOSE_DEBUG,
                                                                            "mjs.c"
      ,
                                                                            10263
      )) { cs_log_printf ("%s\t[%.*s] %u", buf, (int) n, code + i + 1 + llen + llen2, (unsigned) arg_no); } } while (0)
                                                                             ;
      i += llen + llen2 + n;
      break;
    }
    case OP_PUSH_STR:
    case OP_PUSH_DBL: {
      cs_varint_decode(&code[i + 1], ~0, &n, &llen);
      do { if (cs_log_print_prefix(LL_VERBOSE_DEBUG, "mjs.c", 10270)) { cs_log_printf ("%s\t[%.*s]", buf, (int) n, code + i + 1 + llen); } } while (0);
      i += llen + n;
      break;
    }
    case OP_JMP:
    case OP_JMP_TRUE:
    case OP_JMP_NEUTRAL_TRUE:
    case OP_JMP_FALSE:
    case OP_JMP_NEUTRAL_FALSE: {
      cs_varint_decode(&code[i + 1], ~0, &n, &llen);
      do { if (cs_log_print_prefix(LL_VERBOSE_DEBUG,


                                                                                "mjs.c"
      ,


                                                                                10283
      )) { cs_log_printf ("%s\t%u", buf, (unsigned) (i + n + llen + 1 )); } } while (0)


                                                                                 ;
      i += llen;
      break;
    }
    case OP_LOOP: {
      size_t l1, l2;
      uint64_t n1, n2;
      cs_varint_decode(&code[i + 1], ~0, &n1, &l1);
      cs_varint_decode(&code[i + l1 + 1], ~0, &n2, &l2);
      do { if (cs_log_print_prefix(LL_VERBOSE_DEBUG,


                                                                         "mjs.c"
      ,


                                                                         10295
      )) { cs_log_printf ("%s\tB:%lu C:%lu (%d)", buf, (unsigned long) (i + 1 + l1 + n1), (unsigned long) (i + 1 + l1 + l2 + n2), (int) i); } } while (0)


                                                                          ;
      i += l1 + l2;
      break;
    }
    case OP_EXPR: {
      int op = code[i + 1];
      const char *name = "???";

      switch (op) {
        case TOK_DOT: name = "."; break;
        case TOK_MINUS: name = "-"; break;
        case TOK_PLUS: name = "+"; break;
        case TOK_MUL: name = "*"; break;
        case TOK_DIV: name = "/"; break;
        case TOK_REM: name = "%"; break;
        case TOK_XOR: name = "^"; break;
        case TOK_AND: name = "&"; break;
        case TOK_OR: name = "|"; break;
        case TOK_LSHIFT: name = "<<"; break;
        case TOK_RSHIFT: name = ">>"; break;
        case TOK_URSHIFT: name = ">>>"; break;
        case TOK_UNARY_MINUS: name = "- (unary)"; break;
        case TOK_UNARY_PLUS: name = "+ (unary)"; break;
        case TOK_NOT: name = "!"; break;
        case TOK_TILDA: name = "~"; break;
        case TOK_EQ: name = "=="; break;
        case TOK_NE: name = "!="; break;
        case TOK_EQ_EQ: name = "==="; break;
        case TOK_NE_NE: name = "!=="; break;
        case TOK_LT: name = "<"; break;
        case TOK_GT: name = ">"; break;
        case TOK_LE: name = "<="; break;
        case TOK_GE: name = ">="; break;
        case TOK_ASSIGN: name = "="; break;
        case TOK_POSTFIX_PLUS: name = "++ (postfix)"; break;
        case TOK_POSTFIX_MINUS: name = "-- (postfix)"; break;
        case TOK_MINUS_MINUS: name = "--"; break;
        case TOK_PLUS_PLUS: name = "++"; break;
        case TOK_LOGICAL_AND: name = "&&"; break;
        case TOK_LOGICAL_OR: name = "||"; break;
        case TOK_KEYWORD_TYPEOF: name = "typeof"; break;
        case TOK_PLUS_ASSIGN: name = "+="; break;
        case TOK_MINUS_ASSIGN: name = "-="; break;
        case TOK_MUL_ASSIGN: name = "*="; break;
        case TOK_DIV_ASSIGN: name = "/="; break;
        case TOK_REM_ASSIGN: name = "%="; break;
        case TOK_XOR_ASSIGN: name = "^="; break;
        case TOK_AND_ASSIGN: name = "&="; break;
        case TOK_OR_ASSIGN: name = "|="; break;
        case TOK_LSHIFT_ASSIGN: name = "<<="; break;
        case TOK_RSHIFT_ASSIGN: name = ">>="; break;
        case TOK_URSHIFT_ASSIGN: name = ">>>="; break;
      }

      do { if (cs_log_print_prefix(LL_VERBOSE_DEBUG, "mjs.c", 10349)) { cs_log_printf ("%s\t%s", buf, name); } } while (0);
      i++;
      break;
    }
    case OP_BCODE_HEADER: {
      size_t start = 0;
      mjs_header_item_t map_offset = 0, total_size = 0;
      start = i;
      memcpy(&total_size, &code[i + 1], sizeof(total_size));
      memcpy(&map_offset,
             &code[i + 1 + MJS_HDR_ITEM_MAP_OFFSET * sizeof(total_size)],
             sizeof(map_offset));
      i += sizeof(mjs_header_item_t) * MJS_HDR_ITEMS_CNT;
      do { if (cs_log_print_prefix(LL_VERBOSE_DEBUG,

                                                                "mjs.c"
      ,

                                                                10364
      )) { cs_log_printf ("%s\t[%s] end:%lu map_offset: %lu", buf, &code[i + 1], (unsigned long) start + total_size, (unsigned long) start + map_offset); } } while (0)

                                                                 ;
      i += strlen((char *) (code + i + 1)) + 1;
      break;
    }
    default:
      do { if (cs_log_print_prefix(LL_VERBOSE_DEBUG, "mjs.c", 10369)) { cs_log_printf ("%s", buf); } } while (0);
      break;
  }
  return i - start_i;
}

void mjs_disasm(const uint8_t *code, size_t len) {
  size_t i, start = 0;
  mjs_header_item_t map_offset = 0, total_size = 0;

  for (i = 0; i < len; i++) {
    size_t delta = mjs_disasm_single(code, i);
    if (code[i] == OP_BCODE_HEADER) {
      start = i;
      memcpy(&total_size, &code[i + 1], sizeof(total_size));
      memcpy(&map_offset,
             &code[i + 1 + MJS_HDR_ITEM_MAP_OFFSET * sizeof(total_size)],
             sizeof(map_offset));
    }

    i += delta;

    if (map_offset > 0 && i == start + map_offset) {
      i = start + total_size - 1;
      continue;
    }
  }
}

static void mjs_dump_obj_stack(const char *name, const struct mbuf *m,
                               struct mjs *mjs) {
  char buf[50];
  size_t i, n;
  n = mjs_stack_size(m);
  do { if (cs_log_print_prefix(LL_VERBOSE_DEBUG, "mjs.c", 10403)) { cs_log_printf ("%12s (%d elems): ", name, (int) n); } } while (0);
  for (i = 0; i < n; i++) {
    mjs_sprintf(((mjs_val_t *) m->buf)[i], mjs, buf, sizeof(buf));
    do { if (cs_log_print_prefix(LL_VERBOSE_DEBUG, "mjs.c", 10406)) { cs_log_printf ("%34s", buf); } } while (0);
  }
}

void mjs_dump(struct mjs *mjs, int do_disasm) {
  do { if (cs_log_print_prefix(LL_VERBOSE_DEBUG, "mjs.c", 10411)) { cs_log_printf ("------- MJS VM DUMP BEGIN"); } } while (0);
  mjs_dump_obj_stack("DATA_STACK", &mjs->stack, mjs);
  mjs_dump_obj_stack("CALL_STACK", &mjs->call_stack, mjs);
  mjs_dump_obj_stack("SCOPES", &mjs->scopes, mjs);
  mjs_dump_obj_stack("LOOP_OFFSETS", &mjs->loop_addresses, mjs);
  mjs_dump_obj_stack("ARG_STACK", &mjs->arg_stack, mjs);
  if (do_disasm) {
    int parts_cnt = mjs_bcode_parts_cnt(mjs);
    int i;
    do { if (cs_log_print_prefix(LL_VERBOSE_DEBUG, "mjs.c", 10420)) { cs_log_printf ("%23s", "CODE:"); } } while (0);
    for (i = 0; i < parts_cnt; i++) {
      struct mjs_bcode_part *bp = mjs_bcode_part_get(mjs, i);
      mjs_disasm((uint8_t *) bp->data.p, bp->data.len);
    }
  }
  do { if (cs_log_print_prefix(LL_VERBOSE_DEBUG, "mjs.c", 10426)) { cs_log_printf ("------- MJS VM DUMP END"); } } while (0);
}

static int mjs_check_arg(struct mjs *mjs, int arg_num,
                              const char *arg_name, enum mjs_type expected_type,
                              mjs_val_t *parg) {
  mjs_val_t arg = ((uint64_t)(1) << 63 | (uint64_t) 0x7ff0 << 48 | (uint64_t)(3) << 48);
  enum mjs_type actual_type;

  if (arg_num >= 0) {
    int nargs = mjs_nargs(mjs);
    if (nargs < arg_num + 1) {
      mjs_prepend_errorf(mjs, MJS_TYPE_ERROR, "missing argument %s", arg_name);
      return 0;
    }

    arg = mjs_arg(mjs, arg_num);
  } else {

    arg = mjs->vals.this_obj;
  }

  actual_type = mjs_get_type(arg);
  if (actual_type != expected_type) {
    mjs_prepend_errorf(mjs, MJS_TYPE_ERROR, "%s should be a %s, %s given",
                       arg_name, mjs_stringify_type(expected_type),
                       mjs_stringify_type(actual_type));
    return 0;
  }

  if (parg != 
             __null
                 ) {
    *parg = arg;
  }

  return 1;
}

static int mjs_normalize_idx(int idx, int size) {
  if (idx < 0) {
    idx = size + idx;
    if (idx < 0) {
      idx = 0;
    }
  }
  if (idx > size) {
    idx = size;
  }
  return idx;
}

static const char *mjs_get_bcode_filename(struct mjs *mjs,
                                               struct mjs_bcode_part *bp) {
  (void) mjs;
  return bp->data.p + 1 +
         sizeof(mjs_header_item_t) * MJS_HDR_ITEMS_CNT;
}

const char *mjs_get_bcode_filename_by_offset(struct mjs *mjs, int offset) {
  const char *ret = 
                   __null
                       ;
  struct mjs_bcode_part *bp = mjs_bcode_part_get_by_offset(mjs, offset);
  if (bp != 
           __null
               ) {
    ret = mjs_get_bcode_filename(mjs, bp);
  }
  return ret;
}

int mjs_get_lineno_by_offset(struct mjs *mjs, int offset) {
  size_t llen;
  uint64_t map_len;
  int prev_line_no, ret = 1;
  struct mjs_bcode_part *bp = mjs_bcode_part_get_by_offset(mjs, offset);
  uint8_t *p, *pe;
  if (bp != 
           __null
               ) {
    mjs_header_item_t map_offset, bcode_offset;
    memcpy(&map_offset, bp->data.p + 1 +
                            sizeof(mjs_header_item_t) * MJS_HDR_ITEM_MAP_OFFSET,
           sizeof(map_offset));

    memcpy(&bcode_offset,
           bp->data.p + 1 +
               sizeof(mjs_header_item_t) * MJS_HDR_ITEM_BCODE_OFFSET,
           sizeof(bcode_offset));

    offset -= (1 + bcode_offset) + bp->start_idx;


    p = (uint8_t *) bp->data.p + 1 + map_offset;

    cs_varint_decode(p, ~0, &map_len, &llen);
    p += llen;
    pe = p + map_len;

    prev_line_no = 1;
    while (p < pe) {
      uint64_t cur_offset, line_no;
      cs_varint_decode(p, ~0, &cur_offset, &llen);
      p += llen;
      cs_varint_decode(p, ~0, &line_no, &llen);
      p += llen;

      if (cur_offset >= (uint64_t) offset) {
        ret = prev_line_no;
        break;
      }
      prev_line_no = line_no;
    }
  }
  return ret;
}

int mjs_get_offset_by_call_frame_num(struct mjs *mjs, int cf_num) {
  int ret = -1;
  if (cf_num == 0) {

    ret = mjs->cur_bcode_offset;
  } else if (cf_num > 0 &&
             mjs->call_stack.len >=
                 sizeof(mjs_val_t) * CALL_STACK_FRAME_ITEMS_CNT * cf_num) {

    int pos = CALL_STACK_FRAME_ITEM_RETURN_ADDR +
              CALL_STACK_FRAME_ITEMS_CNT * (cf_num - 1);
    mjs_val_t val = *vptr(&mjs->call_stack, -1 - pos);
    ret = mjs_get_int(mjs, val);
  }
  return ret;
}




int main(int argc, char *argv[]) {
  char my_string[10240];
  if (argc == 1) {
    char *v = fgets(my_string, 10240, 
                                     stdin
                                          );
    if (!v) {
      exit(1);
    }
    int read = strlen(my_string);
    if (my_string[read - 1] == '\n') {
      my_string[read - 1] = '\0';
    }
  } else {
    strcpy(my_string, argv[1]);
    int read = strlen(my_string);
    if (my_string[read - 1] == '\n') {
      my_string[read - 1] = '\0';
    }
  }
  printf("val: <%s>\n", my_string);

  struct mjs *mjs = mjs_create();
  mjs_val_t res =
      ((uint64_t)(1) << 63 | (uint64_t)0x7ff0 << 48 | (uint64_t)(3) << 48);
  mjs_err_t err = MJS_OK;
  int i;

  err = mjs_exec(mjs, my_string, &res);

  if (err != MJS_OK) {

    return 1;
  }
}
