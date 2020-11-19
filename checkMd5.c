

#include <openssl/conf.h>
#include <openssl/evp.h>
#include <openssl/err.h>
#include <stdio.h>


int generate_checksum(unsigned char *pin, 
        int in_len, unsigned char *pout, 
        unsigned int *pout_len, char *digest_name)
{
    EVP_MD_CTX mdctx;
    const EVP_MD *md;

    OpenSSL_add_all_digests();
    if(!(md = EVP_get_digestbyname(digest_name)))
    {
        printf("get md error \n"); //debug log
        return -1;
    }
    EVP_MD_CTX_init(&mdctx);
    EVP_DigestInit_ex(&mdctx, md, NULL);
    EVP_DigestUpdate(&mdctx, pin, in_len); 
    EVP_DigestFinal_ex(&mdctx, pout, pout_len);

    EVP_MD_CTX_cleanup(&mdctx);

    return 0;
}

int main(void)
{
    unsigned char out[16] = {0};
    int                 i;
    int                 outlen = 0;

    generate_checksum("hello", 5, out, &outlen, "md5");
    for (i = 0; i < 16; i++)
    {
        printf("%x", out[i]);
    }
    printf("\n");
    printf("outlen:%d\n", outlen);

    return 0;
}
