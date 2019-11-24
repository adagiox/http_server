#include "files.h"

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