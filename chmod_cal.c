#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define R_OWNER     400
#define R_GROUP     40
#define R_OTHER     4
#define W_OWNER     (R_OWNER >> 1)      /* 200 */
#define W_GROUP     (R_GROUP >> 1)      /* 20 */
#define W_OTHER     (R_OTHER >> 1)      /* 2 */
#define X_OWNER     (W_OWNER >> 1)      /* 100 */
#define X_GROUP     (W_GROUP >> 1)      /* 10 */
#define X_OTHER     (W_OTHER >> 1)      /* 1 */

#define CHECK_PARGS(x)  \
    (perms[i + 1] == x || perms[i + 2] == x || perms[i + 3] == x)

static int calc_chmod(char *perms)
{
    int ret;
    size_t len, i;

    len = strlen(perms);
    ret = 0;

    for (i = 0; i < len; i++) {
        switch (perms[i]) {
        case 'r':
            CHECK_PARGS('u') ? ret += R_OWNER : 0;
            CHECK_PARGS('g') ? ret += R_GROUP : 0;
            CHECK_PARGS('o') ? ret += R_OTHER : 0;
            break;

        case 'w':
            CHECK_PARGS('u') ? ret += W_OWNER : 0;
            CHECK_PARGS('g') ? ret += W_GROUP : 0;
            CHECK_PARGS('o') ? ret += W_OTHER : 0;
            break;

        case 'x':
            CHECK_PARGS('u') ? ret += X_OWNER : 0;
            CHECK_PARGS('g') ? ret += X_GROUP : 0;
            CHECK_PARGS('o') ? ret += X_OTHER : 0;
            break;

        default:
            break;
        }
    }

    return ret;
}

static void usage(void)
{
    fprintf(stdout,
        "chmodcal, part of it-utils\n"
        "Usage:\n"
        "   chmodcal  [perms] (e.g. chmodcal rug)\n"
    );
}

int main(int argc, char **argv)
{
    if (argc < 2) {
        usage();
        goto done;
    }

    fprintf(stdout, "chmod: %d\n", calc_chmod(argv[1]));

done:
    exit(EXIT_SUCCESS);
}
