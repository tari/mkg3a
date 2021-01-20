/*
POSIX getopt for Windows

AT&T Public License

Code given out at the 1985 UNIFORUM conference in Dallas.
*/
#ifndef _GETOPT_H_
#define _GETOPT_H_

extern int opterr;
extern int optind;
extern int optopt;
extern char *optarg;

int getopt(int argc, char **argv, char *opts);

#endif /* _GETOPT_H_ */
