#ifndef __BGMIX_H__
#define __BGMIX_H__

#ifdef __cplusplus
extern "C" {
#endif

// TODO this is kind of a global configuration variable: should be moved to a better config place
// TODO changing this also requires changing CurvePoint.h
#define CURVE_POINT_BYTESIZE 32

void hello();

bool generate_ciphers(const char * ciphers_file, const long dim_m, const long dim_n);
bool mix(const char * ciphers_file, const long dim_m, const long dim_n);

#include <stdint.h>

// key directory
#include "keys.h"

/******************* Memory ***********************/
char** makeCharArray(int size);
void setArrayString(char **a, char *s, int index, int src_index, int size);
void freeCharArray(char **a, int size);

void delete_ciphers(void* cipher_table);
void delete_str(void* s);
void delete_int_arr(int* x);
int get_int_elem(int* arr, int i);


void* create_pub_key(int key_id);
void* create_decryption_key(int keyID);
void delete_key(void* elgammal);


/******************* Interface ********************/
void init();
void* elg_encrypt(void** secrets, int secretLen, int arrayLen, int keyIndex);
void* get_ciphertexts(void* cipher_table, void* len, void* elmenent_size);
void* get_element(void* cipher_table, int index, void* len);
void* get_cipher(void* cipher_table, int i, int j, void* len);

// for verified input metadata to first chain
// same as encrypt except returned pointer does not have ciphertext directly
void* encrypt_with_proof(void** secrets, int secretLen, int arrayLen, int keyIndex);
// encrypt_cipher_part(encrypt_with_proof(x)) = encrypt(x)
void* encrypt_cipher_part(void* cipher_and_proof);
// returns a proof and its size (written to proof_size)
void* encrypt_proof_part(void* cipher_and_proof, int* proof_size);
// same as delete_ciphers except to be called on encrypt_with_proof instead of encrypt
// note: do not pass parts to other delete functions (will cause double free)
void delete_ciphers_with_proof(void* x);
// verify a proof output by encrypt_proof_part and their corresponding ciphertexts
int verify_encrypt(void* ciphers, int ciphers_size, void* proof, int proof_size);

int rows(void* cipher_table);
int cols(void* cipher_table);

void* parse_ciphers(void* ciphers, int len, void* elgammal);
void* decrypt_cipher(void* ciphers, int i, int j, void* len, void* elgamal);


void* shuffle_internal(void* reenc_key,
                       char* ciphers_in, int ciphers_array_len, int number_of_elements,
                       char** shuffled_ciphers, int* shuffled_ciphers_len,
                       int** permutation, int* permutation_len);
void prove(void* cached_shuffle,
           char** proof_out, int* proof_len,
           char** public_randoms, int* public_randoms_len);
int verify(void* elgammal,
           char* proof, int proof_len,
           char* ciphers_in, int len,
           char* post_shuffle_cipehrs, int post_shuffle_cipehrs_len,
           char* public_randoms, int public_randoms_len);

#ifdef __cplusplus
}
#endif

#endif
