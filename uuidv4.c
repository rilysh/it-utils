#define _XOPEN_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <string.h>
#include <stdarg.h>

#define MAX_BUF_SZ      2048
#define SET_LOOP_VALUE(i, n, fn)    \
    for (i = 0; i < n; i++)         \
        fn

#define GEN_ENTROPY(xor)            \
    xor.body[0] = gen_entropy();    \
    xor.body[1] = gen_entropy();    \
    xor.body[2] = gen_entropy();    \
    xor.body[3] = gen_entropy();

#define ZERO_MEM(dst, lim)   \
    memset(dst, '\0', lim);

#ifndef TRUE
    #define TRUE    1
#endif
#ifndef FALSE
    #define FALSE   0
#endif

#define RFC_TIMELOW_LEN     8
#define RFC_TIMEMID_LEN     (RFC_TIMELOW_LEN >> 1)          /* 4 */
#define RFC_TIMEHI_LEN      (RFC_TIMEMID_LEN)
#define RFC_RNGCOM_LEN      (RFC_TIMEMID_LEN)
#define RFC_RNGCOMLO_LEN    (RFC_TIMEMID_LEN)
#define RFC_NODE_LEN        RFC_TIMELOW_LEN
#define BUF_SIZE            ((RFC_TIMELOW_LEN << 2) + 5)    /* 37 */
#define CHAR_SZ             1

typedef struct {
    uint64_t body[4];
} XorShift128;

const int table[] = {
    48, 49, 50, 51, 52, 53, 54, 55, 56, 57,	/* Numbers */
    97, 98, 99, 100, 101, 102,              /* Alphabets (lower-case) */
};

static uint64_t xorshift128(XorShift128 *state)
{
    uint64_t s;
    uint64_t t;

    s = state->body[0];
    t = state->body[3];

    state->body[3] = state->body[2];
    state->body[2] = state->body[1];
    state->body[1] = s;

    t ^= t << 11;
    t ^= t >> 8;

    return state->body[0] = t ^ s ^ (s >> 19);
}

static uint64_t gen_entropy(void)
{
    FILE *fp;
    uint32_t bl;
    char buf[2] = {0};

    fp = fopen("/dev/urandom", "rb");

    if (fp == NULL) {
        perror("fopen()");
        exit(EXIT_FAILURE);
    }

    fread(&bl, 1L, sizeof(buf), fp);
    fclose(fp);

    return bl;
}

static void append_str(char *dst, const char *fmt, ...)
{
    va_list ap;
    char tfmt[MAX_BUF_SZ] = {0};

    va_start(ap, fmt);
    vsprintf(tfmt, fmt, ap);
    strcat(dst, tfmt);
    va_end(ap);
}

static int sf_atoi(const char *src)
{
    char *eptr;
    long ret;

    ret = strtol(src, &eptr, 0);

    if (ret < 0 || ret >= INT32_MAX)
        return 0;

    if (eptr == src) {
        fprintf(stderr, "Error: No numbers were found\n");
        return 0;
    }

    return (int)ret;
}

/**
 * I don't see how my one is different or bad than RFC4122.
 * They used system time which indeed not a good seed for
 * randomness. If time is predictable then similar UUID can
 * be generated.
 * 
 * UUID v4 stands for its randomness betweeen 0x30 to 0x66.
 * And that's what I did here.
*/
static char *gen_uuidv4(void)
{
    int i;
    char *buf;
    XorShift128 xor;
    char time_low[RFC_TIMELOW_LEN], time_mid[RFC_TIMEMID_LEN],
        time_hi_ver[RFC_TIMEHI_LEN], rng_com[RFC_RNGCOM_LEN],
        rng_com_lo[RFC_RNGCOMLO_LEN], node[RFC_NODE_LEN];

    buf = malloc(BUF_SIZE);

    if (buf == NULL) {
        perror("malloc()");
        exit(EXIT_FAILURE);
    }

    ZERO_MEM(buf, BUF_SIZE);
    ZERO_MEM(time_low, RFC_TIMELOW_LEN);
    ZERO_MEM(time_mid, RFC_TIMEMID_LEN);
    ZERO_MEM(time_hi_ver, RFC_TIMEHI_LEN);
    ZERO_MEM(rng_com, RFC_RNGCOM_LEN);
    ZERO_MEM(rng_com_lo, RFC_RNGCOMLO_LEN);
    ZERO_MEM(node, RFC_NODE_LEN);

    GEN_ENTROPY(xor);

    SET_LOOP_VALUE(i, RFC_TIMELOW_LEN, time_low[i] = table[xorshift128(&xor) % 16]);
    SET_LOOP_VALUE(i, RFC_TIMELOW_LEN, append_str(buf, "%c", time_low[i]));
    SET_LOOP_VALUE(i, CHAR_SZ, append_str(buf, "-"));

    GEN_ENTROPY(xor);

    SET_LOOP_VALUE(i, RFC_TIMEMID_LEN, time_mid[i] = table[xorshift128(&xor) % 16]);
    SET_LOOP_VALUE(i, RFC_TIMEMID_LEN, append_str(buf, "%c", time_mid[i]));
    SET_LOOP_VALUE(i, CHAR_SZ, append_str(buf, "-"));

    GEN_ENTROPY(xor);

    SET_LOOP_VALUE(i, RFC_TIMEHI_LEN, time_hi_ver[i] = table[xorshift128(&xor) % 16]);
    SET_LOOP_VALUE(i, RFC_TIMEHI_LEN, append_str(buf, "%c", time_hi_ver[i]));
    SET_LOOP_VALUE(i, CHAR_SZ, append_str(buf, "-"));

    GEN_ENTROPY(xor);

    SET_LOOP_VALUE(i, RFC_RNGCOM_LEN, rng_com[i] = table[xorshift128(&xor) % 16]);
    SET_LOOP_VALUE(i, RFC_RNGCOM_LEN, append_str(buf, "%c", rng_com[i]));
    SET_LOOP_VALUE(i, CHAR_SZ, append_str(buf, "-"));

    GEN_ENTROPY(xor);

    SET_LOOP_VALUE(i, RFC_RNGCOMLO_LEN, rng_com_lo[i] = table[xorshift128(&xor) % 16]);
    SET_LOOP_VALUE(i, RFC_RNGCOMLO_LEN, append_str(buf, "%c", rng_com_lo[i]));

    GEN_ENTROPY(xor);

    SET_LOOP_VALUE(i, RFC_NODE_LEN, node[i] = table[xorshift128(&xor) % 16]);
    SET_LOOP_VALUE(i, RFC_NODE_LEN, append_str(buf, "%c", node[i]));

    return buf;
}

static void usage(void)
{
    fprintf(stdout,
        "uuid v4, part of it-utils\n"
        "Usage:\n"
        "   [-g]    -- Generate v4 UUID\n"
        "   [-l]    -- Set how many it will generate (max: %d\n"
        , INT32_MAX
    );
}

int main(int argc, char **argv)
{
    if (argc < 2) {
        usage();
        goto done;
    }

    char *uuidv4;
    int gst, len, opt;

    gst = len = 0;

    while ((opt = getopt(argc, argv, "gl:")) != -1) {
        switch (opt) {
        case 'g':
            gst = TRUE;
            break;

        case 'l':
            len = sf_atoi(optarg);
            break;

        case 'h':
            usage();
            break;

        default:
            exit(EXIT_FAILURE);
        }
    }

    if (gst) {
        do {
            uuidv4 = gen_uuidv4();

            fprintf(stdout, "%s\n", uuidv4);
            free(uuidv4);
        } while (len--);

        goto done;
    }

done:
    exit(EXIT_SUCCESS);
}
