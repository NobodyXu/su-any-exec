/* set user and group id and exec */

#include <stdnoreturn.h>
#include <sys/types.h>

#include <err.h>
#include <grp.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static void parse_userspec(char *argv1, const char **user_p, const char **group_p)
{
    char *user = argv1;
    char *group = strchr(user, ':');
    if (group)
        *group++ = '\0';

    *user_p = user;
    *group_p = group;
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

/**
 * user != NULL, glist_p != NULL
 *
 * After the call, *glist_p will point to heap memory,
 * and this call will return the number of elements in it.
 */
int Getgrouplist(const char *user, gid_t gid, gid_t **glist_p)
{
    int ngroups = 0;
    gid_t *glist = NULL;

    while (1) {
        int ret = getgrouplist(user, gid, glist, &ngroups);

        if (ret >= 0) {
            *glist_p = glist;
            return ngroups;
        }

        glist = realloc(glist, ngroups * sizeof(gid_t));
        if (glist == NULL)
            err(1, "malloc");
    }
}

int main(int argc, char *argv[])
{
    if (argc < 3)
        errx(0, "Usage: %s user-spec command [args]\n", argv[0]);

    const char *user, *group;

    parse_userspec(argv[1], &user, &group);
    
    uid_t uid;
    struct passwd *pw = NULL;
    if (user[0] != '\0')
        uid = parse_user(user, &pw);
    else {
        uid = getuid();
        pw = getpwuid(uid);
    }

    setenv("HOME", pw != NULL ? pw->pw_dir : "/", 1);
    
    if (pw != NULL) {
        setenv("USER",    pw->pw_name, 1);
        setenv("LOGNAME", pw->pw_name, 1);
    } else {
        char buffer[20]; // 20 is enough for a unsigned 64-bit integer
        snprintf((char*) &buffer, 20, "%llu", (unsigned long long) uid);

        setenv("USER",    (const char*) &buffer, 1);
        setenv("LOGNAME", (const char*) &buffer, 1);
    }

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
            err(1, "setgroups(%llu)", (unsigned long long) gid);
    } else {
        gid_t *glist;
        int ngroups = Getgrouplist(pw->pw_name, gid, &glist);

        if (setgroups(ngroups, glist) < 0)
            err(1, "setgroups");

        free(glist);
    }

    if (setgid(gid) < 0)
        err(1, "setgid(%i)", gid);

    if (setuid(uid) < 0)
        err(1, "setuid(%i)", uid);

    char **cmdargv = &argv[2];
    execvp(cmdargv[0], cmdargv);
    err(1, "%s", cmdargv[0]);

    return 1;
}
