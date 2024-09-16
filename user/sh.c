#include "user.h"
#include "fcntl.h"
#include "other.h"

#define CMD_MAXLEN 128

typedef struct {
    int type;
} cmd;

int get_cmd(char *buf, size_t n);
cmd* parse_cmd(char* buf);
void run_cmd(cmd* cmd);
int fork_safe(void);

int main(void) {

    // ensure 3 fds are open
    int fd;
    while ((fd = open("console", O_RDWR) < 3)) {
        if (fd == -1) {
            printf("Error: could not open console\n");
            exit(EXIT_FAILURE);
        }
    }

    // read and run commands
    char buf[CMD_MAXLEN];
    while (get_cmd(buf, sizeof(buf)) >= 0) {
        // built-in commands
        if (strncmp(buf, "cd ", 3) == 0) {
            buf[strlen(buf) - 1] = 0; // remove newline
            if (chdir(buf + 3) < 0) {
                printf("Error: could not cd to %s\n", buf + 3);
            }
            continue;
        }
        if (fork_safe() == 0) {
            run_cmd(parse_cmd(buf));
        }
        wait();
    }
}

int fork_safe(void) {
    int pid = fork();
    if (pid < 0) {
        printf("Error: could not fork\n");
        exit(EXIT_FAILURE);
    }
    return pid;
}

int get_cmd(char *buf, size_t n) {
    printf("> ");
    memset(buf, 0, n);
    gets(buf, n);
    if (buf[0] == 0) return -1; // EOF
    return 0;
}
