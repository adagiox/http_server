#include "dictionary.h"

// seems to work without the initialization, not sure of consequences
void init_gcrypt() {
    if (!gcry_check_version(GCRYPT_VERSION)) {
        fputs("libgcrypt version mismatch\n", stderr);
        exit(EXIT_FAILURE);
    }
    gcry_control(GCRYCTL_DISABLE_SECMEM, 0);
    gcry_control(GCRYCTL_INITIALIZATION_FINISHED, 0);
}

unsigned char *hash(const void *data, int len) {
    unsigned int dlen = gcry_md_get_algo_dlen(GCRY_MD_MD5);
    unsigned char *h = malloc(sizeof(char) * dlen);
    gcry_md_hash_buffer(GCRY_MD_MD5, h, data, len);
    return h;
}

unsigned int hash_value(unsigned char *h, unsigned int hash_len) {
    unsigned int v;
    // just using the lower 4 bytes
    v = (unsigned int)h[hash_len - 4] << 24 |
        (unsigned int)h[hash_len - 3] << 16 |
        (unsigned int)h[hash_len - 2] << 8 |
        (unsigned int)h[hash_len - 1];
    return v;
}

char *hash_to_hex(const unsigned char *digest, char **hex_string, int len_bytes) {
    // len_bytes is the length of digest in bytes
    // doesn't yet handle a digest that is very long and will overflow
    int len = len_bytes * 2;
    char *hex = "0123456789abcdef";
    char mask_upper = 0xf0;
    char mask_lower = 0x0f;
    int j = 0;
    for (int i = 0; i < len; i+=2) {
        (*hex_string)[i] = hex[(digest[j] & mask_upper) >> 4];
        (*hex_string)[i+1] = hex[(digest[j] & mask_lower)];
        j++;
    }
    *hex_string[len] = '\0';
    return *hex_string;
}

void print_dict(s_dict *d) {
    printf("size: %i\n", d->size);
    for (int i = 0; i < d->size; i++) {
        s_dict_entry *de = d->entry[i];
        printf("[%i] ", i);
        while (de != NULL) {
            printf("%i => %i", de->key_hash, de->value_len);
            printf(de->next == NULL ? "\n" : ", ");
            de = de->next;
        }
    }
}

s_dict *dict_new(size_t size) {
    s_dict *d;
    d = malloc(sizeof(s_dict));
    d->size = size;
    d->entry = malloc(sizeof(s_dict_entry) * size);
    for (int i = 0; i < size; i++)
        d->entry[i] = dict_new_entry(0, NULL, 0);
    return d;
}

s_dict_entry *dict_new_entry(int key_hash, void *value, int value_len) {
    s_dict_entry *new = malloc(sizeof(s_dict_entry));
    new->value_len = value_len;
    new->key_hash = key_hash;
    new->value = value;
    new->next = NULL;
    return new;
}

s_dict *dict_rehash(s_dict *d, size_t new_size) {
    s_dict *new_d = dict_new(new_size);
    for (int i = 0; i < d->size; i++) {
        s_dict_entry *de = d->entry[i];
        while (de != NULL) {
            new_d = dict_put(
                new_d, de->key, de->key_hash,
                de->value, de->key_len, de->value_len
            );
            de = de->next;
        }
    }
    return new_d;
}

s_dict *dict_put(s_dict *d, s_dict_entry *de) {
    
}

// if you need to hash the key, key_hash = 0 
// check the load before adding new entry, resize and rehash if necessary    
// if no collision, add entry
// if collision, walk to the end of the chain, add entry
// return pointer to the dict with the new entry
s_dict *dict_add_entry(s_dict *d, unsigned char *key, size_t key_len, unsigned int key_hash, void *value, size_t value_len) {
    s_dict_entry *new;
    key_hash = 0 ? hash_value(hash(key, key_len), 16) : key_hash;
    if (((d->entries + 1) / d->size) > DEFAULT_MAX_LOAD)
        d = dict_rehash(d, 2*d->size);
    if (d->entry[key_hash % d->size]->value == NULL) {
        d->entry[key_hash % d->size]->value = value;
        d->entry[key_hash % d->size]->value_len = value_len;
        d->entry[key_hash % d->size]->key = key;
        d->entry[key_hash % d->size]->key_len = key_len;
        d->entry[key_hash % d->size]->key_hash = key_hash;
        d->entries++;
    }
    else {
        new = dict_new_entry(key_hash, value, value_len);
        s_dict_entry *de = d->entry[key_hash % d->size];
        while (de->next != NULL)
            de = de->next;
        de->next = new;
        d->entries++;
    }
    return d;
}

// s_dict_entry *dict_get(s_dict *d, unsigned char *key, int key_len) {
//     unsigned char *key_hash = hash(key, key_len);
//     size_t index = key_hash % d->entries;
//     d->entry
// }

// check the load before removing an entry, resize and rehash if necessary
// also make sure the dict doesn't get too small (doesn't go below default size)
int dict_remove(s_dict *d, unsigned char *key, int key_len) {
    return 1;
}

int main(int argc, char **argv) {
    if (argc != 2)
        exit(EXIT_FAILURE);
    // unsigned char *hashed;
    // char *data = argv[1];
    // hashed = hash((void *)data, strlen(data));
    // unsigned int hv = hash_value(hashed, 16);
    // printf("%s => %x\n", argv[1], hv);

    s_dict *d = dict_new(10);
    print_dict(d);
    d = dict_put();
    exit(EXIT_SUCCESS);
}