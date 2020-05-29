
typedef struct arg {
    const char *key;
    const char *value;
} arg;

char is_arg(char *pa, const char *pkey);
arg parse_arg(char *parg, const char *pkey);
