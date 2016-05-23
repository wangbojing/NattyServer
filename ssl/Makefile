
objects : openssl_aes_cfb.o openssl_aes_core.o openssl_cbc_enc.o openssl_cfb128.o openssl_md5.o openssl_des_enc.o openssl_ncbc_enc.o openssl_set_key.o openssl_sha1.o

openssl : $(objects)
	cc -o openssl $(objects)

openssl_aes_cfb.o : openssl_aes_cfb.c

openssl_aes_core.o : openssl_aes_core.c

openssl_cfb128.o : openssl_cfb128.c

openssl_cbc_enc.o : openssl_cbc_enc.c

openssl_des_enc.o : openssl_des_enc.c

openssl_md5.o : openssl_md5.c

openssl_ncbc_enc.o : openssl_ncbc_enc.c

openssl_set_key.o : openssl_set_key.c

openssl_sha1.o : openssl_sha1.c

clean : 
	rm -rf *.o