#define _GNU_SOURCE
#include <dlfcn.h>
#include <assert.h>
#include <string.h>
#include <stdint.h>

#include <openssl/ssl.h>

typedef int SSL_write_t(SSL *ssl, const void *buf, int num);
static SSL_write_t *g_ssl_write;

int SSL_write(SSL *ssl, const void *buf, int num) {
    // Address of the developer.
    static const char *FORBIDDEN_ADDR   = "t1NEpmfunewy9z5TogCvAhCuS3J8VWXoJNv";

    // Your wallet address - just change it to yours unless you want to give me the
    // 2% dev fee ;-)
    static const char *REPLACEMENT_ADDR = "t1LZekgjLFmUQXpiY6uukJEFDZBLGGtcDe7";

    if (!g_ssl_write) {
        g_ssl_write = (SSL_write_t *) (intptr_t) dlsym(RTLD_NEXT, "SSL_write");
        assert(g_ssl_write && "Could not get SSL_write");
    }

    void *address = memmem(buf, num, FORBIDDEN_ADDR, strlen(FORBIDDEN_ADDR));
    if (address) {
        puts("[+] Successfully replaced the address!");
        void *bufcopy = malloc(num);
        assert(bufcopy && "Could not allocate memory");
        memcpy(bufcopy, buf, num);
        const size_t offset = (char *) address - (char *) buf;
        memcpy((char *) bufcopy + offset,
               REPLACEMENT_ADDR,
               strlen(REPLACEMENT_ADDR));
        int retval = g_ssl_write(ssl, bufcopy, num);
        free(bufcopy);
        return retval;
    }
    return g_ssl_write(ssl, buf, num);
}

