#include <stdio.h>
#include <string.h>
#include <openssl/bio.h>
#include <openssl/err.h>
#include <openssl/bn.h>
#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/rand.h>

#include <fcntl.h>
#include <unistd.h>

BIO *publickey_bio = NULL;
BIO *privatekey_bio = NULL;
BIO *stdout_bio = NULL;

void closefiles(void);
char *RSA_pubkey_read(const char *filename, char *buffer);

int openfiles(char *IP) {
    
    char pub[64];
    char pri[64];

    stdout_bio = BIO_new(BIO_s_file());

    if (stdout_bio) {
        BIO_set_fp(stdout_bio, stdout, BIO_NOCLOSE | BIO_FP_TEXT);
    }

    publickey_bio = BIO_new(BIO_s_file());
    privatekey_bio = BIO_new(BIO_s_file());

    if (publickey_bio == NULL || privatekey_bio == NULL) {
        closefiles();
        return -1;
    }

    sprintf(pub, "public/%s.pem", IP);
    if (BIO_write_filename(publickey_bio, pub) <= 0) {
        closefiles();
        return -1;
    }
    
    sprintf(pri, "private/%s.pem", IP);
    if (BIO_write_filename(privatekey_bio, pri) <= 0) {
        closefiles();
        return -1;
    }
    return 0;
}
        
int writefiles(RSA *rsa) {

    if (! PEM_write_bio_RSA_PUBKEY(stdout_bio, rsa)) {
        return -1;
    }

    if (! PEM_write_bio_RSA_PUBKEY(publickey_bio, rsa)) {
        return -1;
    }

    if (! PEM_write_bio_RSAPrivateKey(stdout_bio, rsa, NULL, NULL, 0, NULL, NULL)) {
        return -1;
    }

    if (! PEM_write_bio_RSAPrivateKey(privatekey_bio, rsa, NULL, NULL, 0, NULL, NULL)) {
        return -1;
    }

    return 0;
}

void closefiles(void) {
    if (publickey_bio) {
        BIO_free_all(publickey_bio);
    }

    if (privatekey_bio) {
        BIO_free_all(privatekey_bio);
    }

    if (stdout_bio) {
        BIO_free_all(stdout_bio);
    }

}

RSA *gen_key(int key_len) {
    RAND_status();
    BIGNUM *bn = BN_new();
    RSA *rsa = RSA_new();
    BN_set_word(bn, RSA_F4);
 
    RSA_generate_key_ex(rsa, key_len, bn, NULL);
     
    return rsa;
}

int make_key(char *IP) {
    RSA *rsa = NULL;

    int key_len = 2048;

    rsa = gen_key(key_len);

    if (rsa) {
        if (openfiles(IP) == 0) {
            writefiles(rsa);
            closefiles();
        }

        RSA_free(rsa);
    }
    return 0;
}

char *RSA_pubkey_read(const char *filename, char *buffer) {
    int fd, temp;

    fd = open(filename, O_RDONLY);
    if (fd == -1) {
        fprintf(stderr, "file : %s, error : %s\n", filename, strerror(errno));
        return NULL;
    }

    temp = read(fd, buffer, 1024);
    if (temp == -1) {
        fprintf(stderr, "file : %s, error : %s\n", filename, strerror(errno));
        return NULL;
    }
    
    close(fd);
    return buffer;
}

char *RSA_pubkey_write(const char *filename, char *buffer, int len) {
    int fd, temp;

    fd = open(filename, O_WRONLY | O_CREAT, 00777);
    if (fd == -1) {
        fprintf(stderr, "file : %s, error : %s\n", filename, strerror(errno));
        return NULL;
    }

    temp = write(fd, buffer, len);
    if (temp == -1) {
        fprintf(stderr, "file : %s, error : %s\n", filename, strerror(errno));
        return NULL;
    }

    close(fd);
    return buffer;
}


