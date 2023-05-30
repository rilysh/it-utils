#define _XOPEN_SOURCE

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <openssl/evp.h>

#ifndef TRUE
    #define TRUE    1
#endif
#ifndef FALSE
    #define FALSE   0
#endif

static void errorx(const char *err)
{
    fprintf(stderr, "%s\n", err);
    exit(EXIT_FAILURE);
}

static unsigned char *digest_hash(const char *src, const char *type)
{
    if (src == NULL || strlen(src) == 0)
        errorx("Error: Source data cannot be empty or NULL");

    size_t strl;
    int len;
    unsigned int outlen, i;
    unsigned char buf[EVP_MAX_MD_SIZE];
    static unsigned char outbuf[EVP_MAX_MD_SIZE];

    strl = strlen(src);
    len = 0;

    EVP_MD_CTX *ctx;
    const EVP_MD *hctx;

    hctx = EVP_get_digestbyname(type);

    /* likely wrong or unimplemented (here) hashing algorithm was passed */
    if (hctx == NULL)
        errorx("Error: Invalid hashing algorithm");

    ctx = EVP_MD_CTX_new();

    if (EVP_DigestInit_ex(ctx, hctx, NULL) == 0)
        errorx("EVP_DigestInit_ex()");

    if (EVP_DigestUpdate(ctx, src, strl) == 0)
        errorx("EVP_DigestUpdate()");

    if (EVP_DigestFinal_ex(ctx, buf, &outlen) == 0)
        errorx("EVP_DigestFinal_ex()");

    EVP_MD_CTX_free(ctx);

    for (i = 0; i < outlen; i++)
        len += sprintf((char *)outbuf + len, "%02x", buf[i]);

    return outbuf;
}

static unsigned char *run_base64(
    int state, const unsigned char *src, int len
)
{
    unsigned char *buf;
    int l0, bed;

    if (state) {
        l0 = round(4 * ((len + 2) / 3));
        buf = calloc(l0 + 1, sizeof(char));

        bed = EVP_EncodeBlock(buf, src, len);

        if (bed != l0)
            errorx("Error: Block encoding failed");
    } else {
        l0 = round(3 * len / 4);
        buf = calloc(l0 + 1, sizeof(char));

        bed = EVP_DecodeBlock(buf, src, len);

        if (bed != l0)
            errorx("Error: Block decoding failed");
    }

    return buf;
}

static void usage(void)
{
    fprintf(stdout,
        "Digest Hash, part of it-utils\n"
        "Usage:\n"
        "   [-t]    -- Algorithm type (e.g. MD5, SHA1, SHA128, etc.)\n"
        "   [-r]    -- Raw text stream\n"
        "   [-e]    -- Encode raw strings to base64 strings\n"
        "   [-d]    -- Decode base64 strings to raw strings\n"
        "   [-h]    -- This message\n"
    );
}

int main(int argc, char **argv)
{
    if (argc < 2) {
        usage();
        exit(EXIT_SUCCESS);
    }

    size_t len;
    char *htype;
    unsigned char *rb64;
    int encst, decst, rst, opt;

    encst = decst = rst = FALSE;
    htype = NULL;

    while ((opt = getopt(argc, argv, "t:derh")) != -1) {
        switch (opt) {
        case 't':
            htype = optarg;
            break;
        
        case 'e':
            encst = TRUE;
            break;

        case 'd':
            decst = TRUE;
            break;

        case 'r':
            rst = TRUE;
            break;

        case 'h':
            usage();
            break;

        default:
            exit(EXIT_FAILURE);
        }
    }

    if (htype != NULL && rst == 0)
        errorx("Error: No input text was provided");

    if (rst) {
        if (htype == NULL || strlen(htype) == 0)
            errorx("Error: No digest method was selected");

        while (argv[optind] != NULL) {
            fprintf(stdout, "%s (%s)\n", digest_hash(argv[optind], htype), htype);
            optind++;
        }
    }

    if (encst) {
        while (argv[optind] != NULL) {
            len = strlen(argv[optind]);
            rb64 = run_base64(TRUE, (const unsigned char *)argv[optind], len);
            fprintf(stdout, "%s (encoded)\n", rb64);

            optind++;
        }

        goto done;
    }

    if (decst) {
        while (argv[optind] != NULL) {
            len = strlen(argv[optind]);
            rb64 = run_base64(FALSE, (const unsigned char *)argv[optind], len);
            fprintf(stdout, "%s (decoded)\n", rb64);

            optind++;
        }

        goto done;
    }

done:
    exit(EXIT_SUCCESS);
}
