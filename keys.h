/*
 *  echinus wm written by Alexander Polakov <polachok@gmail.com> at 23.03.2008 19:47:22 MSK
 */
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <locale.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <regex.h>
#include <X11/cursorfont.h>
#include <X11/keysym.h>
#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <X11/Xproto.h>
#include <X11/Xutil.h>
#include <X11/Xresource.h>

typedef struct
{
    const char *name;
    void (*action)(const char *arg);
} KeyItem;

Key **dkeys;

static KeyItem KeyItems[] = 
{
    { "togglebar", togglebar },
    { "focusnext", focusnext },
    { "focusprev", focusprev },
    { "viewprevtag", viewprevtag },
    { "quit", quit },
    { "killclient", killclient },
    { "togglefloating", togglefloating },
};

void
parsekey(char *s, Key *k) {
    char mod[5]="\0";
    unsigned long modmask = 0;
    char key[15]="\0";
    int i;
    sscanf(s, "%s + %s", mod, key);
    for(i = 0; i<3; i++){
        if(mod[i]=='A')
            modmask = modmask | Mod1Mask;
        if(mod[i]=='S')
            modmask = modmask | ShiftMask;
        if(mod[i]=='C')
            modmask = modmask | ControlMask;
    }
    k->mod = modmask;
    fprintf(stderr, "%s : mod=%s : key=%s\n",s, mod, key);
    k->keysym = XStringToKeysym(key);
    k->arg = NULL;
}

int
initkeys(){
    int i;
    char *tmp;
    dkeys = malloc(sizeof(Key*)*LENGTH(KeyItems));
    for(i = 0; i < LENGTH(KeyItems); i++){
        tmp = getresource(KeyItems[i].name, NULL);
        dkeys[i] = malloc(sizeof(Key));
        dkeys[i]->func = KeyItems[i].action;
        parsekey(tmp, dkeys[i]);
    }
    return 0;
}
