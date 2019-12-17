#ifndef DICTIONARY_H
#define DICTIONARY_H

#include <stdio.h>
#include <gcrypt.h>

#define DEFAULT_DICT_SIZE 20
#define DEFAULT_MAX_LOAD 0.75f
#define DEFAULT_MIN_LOAD 0.25f // untested, somewhat arbitrary values

// 75/100 => 75/200 : LF = 0.375
// 25/100 => 25/50  : LD = 0.5


typedef struct dict_entry {
    size_t value_len;
    unsigned char *key;
    size_t key_len;
    int key_hash;
    void *value;
    struct dict_entry *next; // for chaining
} s_dict_entry;

typedef struct dict {
    size_t size;
    size_t entries;
    s_dict_entry **entry;
} s_dict;

unsigned int hash_value(unsigned char *h, unsigned int hash_len);
s_dict_entry *dict_new_entry(int key_hash, void *value, int value_len);
s_dict *dict_add_entry(s_dict *d, unsigned char *key, size_t key_len, unsigned int key_hash, void *value, size_t value_len);

#endif