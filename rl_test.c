/*
 * Minimal reproducer for the readline prompt/history scrolling bug in gp_strdl.c.
 * Simulates the casperscript pattern: a fake prompt is pre-printed to stdout,
 * then readline is called with a dots string of matching length.
 *
 * Build: gcc -o rl_test rl_test.c -lreadline
 * Run:   ./rl_test
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>
#include <readline/readline.h>
#include <readline/history.h>

#define MAXPROMPT 256
#define MAXREPLY 16
#define MINREPLY 5
#define QUERY "\033[6n"

static int get_cursor_column(void) {
    char reply[MAXREPLY] = "";
    int replysize = 0;
    struct termios save, raw;

    tcgetattr(STDIN_FILENO, &save);
    raw = save;
    raw.c_lflag &= ~(ECHO | ICANON);
    tcsetattr(STDIN_FILENO, TCSANOW, &raw);

    write(STDOUT_FILENO, QUERY, strlen(QUERY));

    replysize = read(STDIN_FILENO, reply, MINREPLY);
    while (replysize < MAXREPLY - 1) {
        if (reply[replysize - 1] == 'R') break;
        replysize += read(STDIN_FILENO, &reply[replysize], 1);
    }

    tcsetattr(STDIN_FILENO, TCSANOW, &save);

    /* parse <ESC>[row;colR */
    int col = 0, mul = 1, off = 2;
    int digit;
    while (replysize - off > 0) {
        digit = reply[replysize - off++];
        if (digit < '0' || digit > '9') break;
        col += (digit - '0') * mul;
        mul *= 10;
    }
    return col;
}

int main(int argc, char *argv[]) {
    rl_bind_key('\t', rl_insert);  /* disable filename completion */

    /* Prompt width can be passed as arg for harness testing;
     * defaults to querying the terminal. */
    int forced_width = (argc > 1) ? atoi(argv[1]) : -1;

    for (int round = 0; round < 20; round++) {
        const char *ps_prompt = "GS> ";
        printf("%s", ps_prompt);
        fflush(stdout);

        int promptsize;
        if (forced_width >= 0) {
            promptsize = forced_width;
        } else {
            int col = get_cursor_column();
            promptsize = col - 1;
        }

        /* Use the REAL prompt string — readline redraws "GS> " during history
         * scroll, which overwrites "GS> " already on screen: idempotent and
         * correct, vs. dots which overwrite visibly. */
        const char *realprompt = ps_prompt;
        rl_already_prompted = 1;

        char *line = readline(realprompt);
        if (!line) break;

        if (strlen(line) > 0)
            add_history(line);

        printf("got: [%s]\n", line);
        free(line);
    }
    return 0;
}
