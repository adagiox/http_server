#include "files.h"

#define SITE "./site_content"

int open_dir(const char *dirname) {
    DIR *dp;
    struct dirent *ep;

    dp = opendir(dirname);
    if (dp != NULL) {
        while (ep = readdir (dp)) {
            if (strcmp(".", ep->d_name) == 0 || strcmp("..", ep->d_name) == 0) {
                continue;
            }
            puts (ep->d_name);
        }
        (void) closedir (dp);
    }
    else {
        perror ("Couldn't open the directory");
        return -1;
    }

    return 1;
}

int main() {
    open_dir(SITE);
    printf("size: %i\n", sizeof("Hello\r\n"));
    return 0;    
}