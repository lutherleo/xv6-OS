#include "types.h"
#include "stat.h"
#include "user.h"

#define BUFSIZE 512
#define DEFAULT_LINES 10

void head(int fd, char *name, int n, int show_header) {
    char buf[BUFSIZE];
    int i, cc, lines_printed = 0;
    
    if (show_header) {
        printf(1, "==> %s <==\n", name);
    }
    
    while ((cc = read(fd, buf, sizeof(buf))) > 0) {
        for (i = 0; i < cc; i++) {
            if (lines_printed >= n) {
                return;
            }
            
            printf(1, "%c", buf[i]);
            
            if (buf[i] == '\n') {
                lines_printed++;
                if (lines_printed >= n) {
                    return;
                }
            }
        }
    }
    
    if (cc < 0) {
        printf(1, "head: read error\n");
        exit();
    }
}

int main(int argc, char *argv[]) {
    int fd, i;
    int n = DEFAULT_LINES;
    int file_start = 1;
    int num_files = 0;
    int file_idx = 0;
    
    if (argc > 1) {
        if (strcmp(argv[1], "-n") == 0) {
            if (argc < 3) {
                printf(2, "head: option requires an argument -- 'n'\n");
                exit();
            }
            n = atoi(argv[2]);
            file_start = 3;
        }
        else if (argv[1][0] == '-' && argv[1][1] >= '0' && argv[1][1] <= '9') {
            n = atoi(argv[1] + 1);
            file_start = 2;
        }
    }
    
    num_files = argc - file_start;
    
    if (num_files == 0) {
        head(0, "", n, 0);
        exit();
    }
    
    for (i = file_start; i < argc; i++) {
        if ((fd = open(argv[i], 0)) < 0) {
            printf(1, "head: cannot open %s\n", argv[i]);
            exit();
        }
        
        int show_header = (num_files > 1);
        
        if (show_header && file_idx > 0) {
            printf(1, "\n");
        }
        
        head(fd, argv[i], n, show_header);
        close(fd);
        file_idx++;
    }
    
    exit();
}