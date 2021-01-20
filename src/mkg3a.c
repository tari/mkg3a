#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#else
#include "getopt.h"
#endif /* HAS_UNISTD_H */

#include "config.h"
#include "g3a.h"
#include "images.h"

char *USAGE =
    "\nUsage: mkg3a [OPTION] input-file [output-file]\n\n"
    "  -i (uns|sel):file\n"
    "     Load unselected/selected icon from file\n"
    "  -n lc:name\n"
    "     Set localized name for language code\n"
    "  -V ver\n"
    "     Set version string\n"
    "  -v\n"
    "     Show version and license information then exit\n"
    "\nValid values for lc are basic, internal, en, es, de, fr, pt and zh.\n"
    "Empty lc is an alias for basic.  Unset names will be derived from\n"
    "basic, which defaults to output file name.\n"
    "\nMultiple -n or -i options will all be applied, with the last\n"
    "specified option overriding previous ones with the same key.";
char *VERSION =
    "mkg3a " mkg3a_VERSION_TAG " (" __DATE__ " " __TIME__ ")\n"
    "Copyright (c) 2011 Peter Marheine <peter@taricorp.net>\n"
    "\nThis is free software; see the source for copying conditions.  There\n"
    "is NO warranty; in no event will the authors be held liable for any\n"
    "damages arising from the use of this software.\n"
    "\nSee http://www.taricorp.net/projects/mkg3a/ for updates.";

/*
 * Sets the name in names corresponding to opt, where opt is of the form
 * code:value.
 * For example, "en:foobar" sets the en part (index 0) to foobar.
 */
int storeNameSpec(char *k, char *v, void *dest) {
    struct lc_names *names = (struct lc_names *)dest;
    const char *codes[] = {"en", "es", "de", "fr", "pt", "zh", "un1", "un2"};
    unsigned int i;

    if (!strcmp(k, "basic") || strlen(k) == 0) {
        names->basic = v;
    } else if (!strcmp(k, "internal")) {
        names->internal = v;
    } else {
        // Proper localized names
        for (i = 0; i < sizeof(codes) / sizeof(char *); i++) {
            if (!strcmp(k, codes[i])) {
                char *lc = names->localized[i];
                if (lc != NULL)
                    free(lc);

                names->localized[i] = v;
                break;
            }
        }
        if (i >= sizeof(codes) / sizeof(char *))
            return 1; // Didn't find lc
    }
    return 0;
}

int storeIconSpec(char *k, char *v, void *dest) {
    struct icons *icons = (struct icons *)dest;
    int32_t width, height;
    u16 *idat;
    void *cd;

    if (!strcmp(k, "uns")) {
        cd = icons->unselected;
    } else if (!strcmp(k, "sel")) {
        cd = icons->selected;
    } else {
        return 1;
    }

    idat = loadBitmap(v, &width, &height);
    if (idat == NULL)
        return 1;
    if (width != G3A_ICON_WIDTH || height != G3A_ICON_HEIGHT) {
        fprintf(stderr, "Dimensions of %s are invalid,", v);
        fprintf(stderr, " icons must be %ix%i pixels.\n", G3A_ICON_WIDTH,
                G3A_ICON_HEIGHT);
        return 1;
    }
    memcpy(cd, idat, sizeof(icons->unselected));
    free(idat);
    free(v);
    return 0;
}

/*
 * Splits opt into a key value pair like 'key:value'.  Makes a copy of value
 * and puts it in v, and points k at the key, which is in opt.  Returns
 * nonzero for failure.
 */
int splitKV(char *opt, char **k, char **v) {
    char *t = strchr(opt, ':');
    if (t == NULL)
        return 1;

    *t++ = 0;
    *k = opt;
    *v = strdup(t);
    if (*v == NULL)
        return 1; // FIXME not descriptive enough
    return 0;
}

int splitAndStore(char *opt, int (*handler)(char *, char *, void *),
                  void *dest) {
    char *k, *v;
    int failure;

    if (splitKV(opt, &k, &v)) {
        return 1;
    } else {
        // Handler expected to free v if necessary when successful
        failure = handler(k, v, dest);
        if (failure) {
            printf("Failed to parse option: `%s:%s`.  See previous error.\n", k,
                   v);
            free(v);
            return 2;
        }
    }
    return 0;
}

int main(int argc, char **argv) {
    int c, errors = 0;
    int args = 0;
    int status = 0;
    char *inFN, *outFN;
    struct lc_names names = {{{NULL}}};
    struct icons icons = {{0}, {0}, {0}};
    char *version = "01.00.0000";

    while ((c = getopt(argc, argv, ":n:i:V:hv")) != -1) {
        switch (c) {
        case 'h':
            errors++; // Force help
            break;
        case 'v':
            puts(VERSION);
            return 0;
        case 'n':
            status = splitAndStore(optarg, &storeNameSpec, &names);
            // 0 is straight up success (no additional processing)
            if (status == 1) {
                // Implicit basic specification
                storeNameSpec("", optarg, &names);
            } else if (status) {
                errors++;
            }
            break;
        case 'i':
            if (splitAndStore(optarg, &storeIconSpec, &icons))
                errors++;
            break;
        case 'V':
            version = optarg;
            break;
        case ':':
            printf("Option -%c requires operand", optopt);
            errors++;
            break;
        case '?':
            printf("Unrecognized option: -%c\n", optopt);
            errors++;
            break;
        default:
            assert(0); // BUG
        }
    }
    args = argc - optind;
    if (errors || (args != 1 && args != 2)) {
        puts(USAGE);
        return 1;
    }

    inFN = argv[optind];
    if (args < 2) {
        // Drop old file extension, tack on .g3a
        char *t;
        outFN = strdup(inFN);
        t = strrchr(outFN, '.');
        if (t != NULL)
            *t = 0;

        if ((outFN = realloc(outFN, strlen(outFN) + strlen(".g3a") + 1)) ==
            NULL) {
            printf("realloc failed on filename (OOM?).  Giving up.\n");
            return 2;
        }
        strcat(outFN, ".g3a");
    } else {
        outFN = argv[optind + 1];
    }

    if (names.basic == NULL)
        names.basic = strdup(outFN);
    if (g3a_mkG3A(inFN, outFN, &names, &icons, version)) {
        printf("Operation failed.  Output file is probably broken.\n");
        return 2;
    }

    return 0;
}
