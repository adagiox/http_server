#ifndef UTIL_H
#define UTIL_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

char *from_string(char *s, int len);
int ch_before_sp(const char *buf, int index);
char **split_request_line(const char *buf);
char *concat_root_and_uri(const char *root, const char *uri);

#endif