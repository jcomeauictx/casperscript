int splitargs(int argc, char **argv, char **argp);
int prependopts(int argc, char **argv, char **argp, char **prepend, int new);
int appendopts(int argc, char **argv, char **argp, char **append, int new);
int marks_end_of_options(char *arg);
int find_end_of_options(int argc, char **argv);
int dump(int argc, char **argv);
