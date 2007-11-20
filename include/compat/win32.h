#ifdef WIN32


#define sleep(x) Sleep((x) * 1000);
#define strdup _strdup
#define access _access
#define bcopy(src, dst, count) memcpy((void *)dst, (const void *)src, (size_t) count)
#define strncpy(a, b, c) strncpy_s((a), _countof(a), (b), (c))
#define rindex(s,c) (strrchr((s),(c)))
#define index(s,c) (strchr((s),(c)))
#define strtoull _strtoi64
#endif