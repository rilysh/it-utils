#define _XOPEN_SOURCE

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <limits.h>

#ifndef TRUE
    #define TRUE    1
#endif
#ifndef FALSE
    #define FALSE   0
#endif

#define MAX_WORD_SIZE       26
#define MAX_NUM_SIZE        10
#define MAX_SYM_SIZE        (MAX_WORD_SIZE + 1)
#define MAX_MIXLU_SIZE      ((MAX_NUM_SIZE << 3) + 9)

const char *upcw[MAX_WORD_SIZE] = {
    "A", "B", "C", "D", "E", "F", "G", "H", "I",
    "J", "K", "L", "M", "N", "O", "P", "Q", "R",
    "S", "T", "U", "V", "W", "X", "Y", "Z"
};

const char *locw[MAX_WORD_SIZE] = {
    "a", "b", "c", "d", "e", "f", "g", "h", "i",
    "j", "k", "l", "m", "n", "o", "p", "q", "r",
    "s", "t", "u", "v", "w", "x", "y", "z"
};

const int int_lst[MAX_NUM_SIZE] = {
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9
};

const char *sym_lst[MAX_SYM_SIZE] = {
    "~", "!", "@", "$", "%%", "^", "&", "*", "(", ")", "-", "_", "+",
    "=", "|", "\\", "[", "]", "{", "}", ";", ":", "?", "/", ".", ",",
    "`"
};

const char *loupcw[MAX_MIXLU_SIZE] = {
    "A", "a", "B", "b", "C", "c", "D", "d", "E", "e",
    "F", "f", "G", "g", "H", "h", "I", "i", "J", "j",
    "K", "k", "L", "l", "M", "m", "N", "n", "O", "o",
    "P", "p", "Q", "q", "R", "r", "S", "s", "T", "t",
    "U", "u", "V", "v", "W", "w", "X", "x", "Y", "y",
    "Z", "z", "~", "!", "@", "$", "%%", "^", "&", "*",
    "(", ")", "-", "_", "+", "=", "|", "\\", "[", "]",
    "{", "}", ";", ":", "?", "/", ".", ",", "`", "0",
    "1", "2", "3", "4", "5", "6", "7", "8", "9"
};

static uint64_t rdtsc(void)
{
    uint32_t lo, hi;

    lo = hi = 0;

    __asm__ volatile(
        "rdtsc" : "=a" (lo), "=r" (hi)
    );

    return ((uint64_t)hi << 32) | lo;
}

static long sf_atoi(const char *src)
{
    char *eptr;
    long ret;

    ret = strtol(src, &eptr, 0);

    /* we likely encountered with an error */
    if (eptr == src || (src[0] != '0' && ret == 0) ||
        ret < 0)
        return 0L;

    if (ret > INT_MAX)
        return INT_MAX;

    return ret;
}

static uint32_t XorShift32(uint32_t seed)
{
    uint32_t x;

    x = seed;
    x ^= x << 13;
	x ^= x >> 17;
	x ^= x << 5;

    return x;
}

static void usage(void)
{
    fprintf(stdout,
        "Token Generator, part of it-utils\n"
        "Usage:\n"
        "   [-l]    -- Generate lower-case charecters\n"
        "   [-u]    -- Generate upper-case charecters\n"
        "   [-i]    -- Generate numbers\n"
        "   [-s]    -- Generate other symbols (e.g. !, @, # etc.)\n"
        "   [-c]    -- Count how many symbols it will generate (default is 32)\n"
        "   [-h]    -- This message\n"
    );
}

int main(int argc, char **argv)
{
    if (argc < 2) {
        usage();
        exit(EXIT_SUCCESS);
    }

    uint32_t rng;
    int lcs, ucs, rcs, ilist, slist, opt, i, count;

    lcs = ucs = rcs = ilist = slist = FALSE;
    count = 32;

    while ((opt = getopt(argc, argv, "luisrhc:")) != -1) {
        switch (opt) {
        case 'l':
            lcs = TRUE;
            break;

        case 'u':
            ucs = TRUE;
            break;

        case 'i':
            ilist = TRUE;
            break;

        case 's':
            slist = TRUE;
            break;

        case 'c':
            if (!lcs && !ucs && !ilist &&
                !slist && !rcs) {
                fprintf(stderr, "Error: No operation opt was passed\n");
                goto end;
            }
            count = sf_atoi(optarg);
            break;

        case 'r':
            rcs = TRUE;
            break;

        case 'h':
            usage();
            break;

        default:
            exit(EXIT_FAILURE);
        }
    }

    if (lcs || ucs) {
        for (i = 0; i < count; i++) {
            rng = XorShift32((uint32_t)rdtsc()) % MAX_WORD_SIZE;

            if (lcs)
                fprintf(stdout, "%s", locw[rng]);

            if (ucs)
                fprintf(stdout, "%s", upcw[rng]);
        }

        goto end;
    }

    if (ilist) {
        for (i = 0; i < count; i++) {
            rng = XorShift32((uint32_t)rdtsc()) % MAX_NUM_SIZE;

            fprintf(stdout, "%d", int_lst[rng]);
        }

        goto end;
    }

    if (slist) {
        for (i = 0; i < count; i++) {
            rng = XorShift32((uint32_t)rdtsc()) % MAX_SYM_SIZE;

            fprintf(stdout, "%s", sym_lst[rng]);
        }

        goto end;
    }

    if (rcs) {
        for (i = 0; i < count; i++) {
            rng = XorShift32((uint32_t)rdtsc()) % MAX_MIXLU_SIZE;

            fprintf(stdout, "%s", loupcw[rng]);
        }

        goto end;
    }

end:
    fprintf(stdout, "\n");
    exit(EXIT_SUCCESS);
}
