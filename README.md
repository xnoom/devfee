# devfee

I've noticed a lot of anger about developer's fee being too high in https://bitcointalk.org/index.php?topic=2021765.0. And I agree with all of you. Earning 20ZEC/day is definitely a lot.

So, you're about to rant on it as well?

Not really. I don't rant, but instead I do things. For the reason stated above, I investigated
the binary of the 0.5.7 version (I couldn't see the difference in performance between 0.5.7
and 0.5.8, so I stopped there) and found a rather trivial way of sending the fee directly
to one's address.

Can I also do it?

Yes, and that's why I'm posting. As you are going to see, it is totally safe and free.

OK, give me the details!

So, first of all, you have to use Linux. Similarly to the developer I also like Linux environment
better. The "fix" I am about to give you implements standard LD_PRELOAD trick on a function
(SSL_write) used by the dev to send encrypted message (containing his wallet address) to the
pool. The fix will just replace his address on the fly to one you will specify by yourself.

Right, right... but how?!

Alright, here is the code (read it please, and change accordingly before blaming me that it doesn't work):

Code:

#define _GNU_SOURCE
#include <dlfcn.h>
#include <assert.h>
#include <string.h>

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

Save the code above as fix.c.
Make sure you have gcc installed on your machine.
Compile it to the shared library fix.so either by:

Code:
$ gcc fix.c -std=gnu99 -shared -o fix.so

or if that didn't work:

Code:
$ gcc fix.c -std=gnu99 -shared -fPIC -o fix.so

Use
Code:
$ LD_PRELOAD=/absolute/path/to/the/fix.so ./zm <standard-arguments-you-typically-give-to-it>
to make it work.
