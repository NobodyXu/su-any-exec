/* set user and group id and exec */

#include <stdnoreturn.h>
#include <sys/types.h>

#include <err.h>
#include <errno.h>
#include <grp.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static noreturn void usage(const char *argv0, int exitcode)
{
    printf("Usage: %s user-spec command [args]\n", argv0);
    exit(exitcode);
}

/**
 * user != NULL, pw_p != NULL
 */
static uid_t parse_user(const char *user, struct passwd **pw_p)
{
    char *end;
    uid_t nuid = strtol(user, &end, 10);

    if (*end == '\0') {
        *pw_p = getpwuid(nuid);
        return nuid;
    } else {
        *pw_p = getpwnam(user);
        if (*pw_p == NULL)
            err(1, "getpwnam(%s)", user);
        return (*pw_p)->pw_uid;
    }
}

/**
 * group != NULL
 */
static gid_t parse_group(const char *group)
{
    char *end;
    gid_t ngid = strtol(group, &end, 10);

    if (*end == '\0')
        return ngid;
    else {
        struct group *gr = getgrnam(group);
        if (gr == NULL)
            err(1, "getgrnam(%s)", group);
        return gr->gr_gid;
    }
}

int main(int argc, char *argv[])
{
    if (argc < 3)
        usage(argv[0], 0);

    char *user, *group, **cmdargv;

    user = argv[1];
    group = strchr(user, ':');
    if (group)
        *group++ = '\0';

    cmdargv = &argv[2];

    uid_t uid;
    struct passwd *pw = NULL;
    if (user[0] != '\0')
        uid = parse_user(user, &pw);
    else
        uid = getuid();

    setenv("HOME", pw != NULL ? pw->pw_dir : "/", 1);

    gid_t gid;
    if (group && group[0] != '\0') {
        /* group was specified, ignore grouplist for setgroups later */
        pw = NULL;
        gid = parse_group(group);
    } else if (pw != NULL)
        gid = pw->pw_gid;
    else
        gid = getgid();

    if (pw == NULL) {
        if (setgroups(1, &gid) < 0)
            err(1, "setgroups(%i)", gid);
    } else {
        int ngroups = 0;
        gid_t *glist = NULL;

        while (1) {
            int r = getgrouplist(pw->pw_name, gid, glist, &ngroups);

            if (r >= 0) {
                if (setgroups(ngroups, glist) < 0)
                    err(1, "setgroups");
                break;
            }

            glist = realloc(glist, ngroups * sizeof(gid_t));
            if (glist == NULL)
                err(1, "malloc");
        }

        free(glist);
    }

    if (setgid(gid) < 0)
        err(1, "setgid(%i)", gid);

    if (setuid(uid) < 0)
        err(1, "setuid(%i)", uid);

    execvp(cmdargv[0], cmdargv);
    err(1, "%s", cmdargv[0]);

    return 1;
}
