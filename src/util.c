#include "util.h"

char *from_string(char *s, int len) {
    char *new = malloc(sizeof(char) * len + 1);
    for (int i = 0; i < len; i++) {
        new[i] = s[i];
    }
    new[len] = '\0';
    return new;
}

int ch_before_sp(const char *buf, int index) {
    int i = 0;
    while (buf[index] != ' ') {
        i++;
        index++;
    }
    return i;
}

char **split_request_line(const char *buf) {
    int i = 0;
    int s = 0;
    char ch = buf[i];
    int method_size;
    int uri_size;
    int version_size = 8;
    method_size = ch_before_sp(buf, 0);
    i = method_size + 1;
    uri_size = ch_before_sp(buf, i);
    char **split = malloc(sizeof(char *) * 3);
    char *method = malloc(sizeof(char) * method_size + 1);
    char *uri = malloc(sizeof(char) * uri_size + 1);
    char *version = malloc(sizeof(char) * version_size + 1);
    split[0] = method;
    split[1] = uri;
    split[2] = version;
    i = 0;
    for (int a = 0; a < method_size; a++){
        method[a] = buf[i];
        i++;
    }
    i++;
    method[method_size] = '\0';
    for (int a = 0; a < uri_size; a++){
        uri[a] = buf[i];
        i++;
    }
    i++;
    uri[uri_size] = '\0';
    for (int a = 0; a < version_size; a++){
        version[a] = buf[i];
        i++;
    }
    version[version_size] = '\0';
    return split;
}

char *concat_root_and_uri(const char *root, const char *uri) {
    // ex. "./site_content" "/index.html"
    int root_len = strlen(root);
    int uri_len = strlen(uri);
    int concat_len = root_len + uri_len;
    char *concat = malloc(sizeof(char) * concat_len + 1);
    int i = 0;
    while (i < root_len) {
        concat[i] = root[i];
        i++;
    }
    while (i < concat_len) {
        concat[i] = uri[i - root_len];
        i++;
    }
    concat[concat_len] = '\0';
    return concat;
}