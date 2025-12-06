#include "types.h"
#include "stat.h"
#include "user.h"

int main(int argc, char *argv[]) {
    int i;
    
    for (i = 0; i < 1000; i++) {
        printf(1, "A");
    }
    printf(1, "\n");
    
    printf(1, "Done\n");
    
    exit();
}