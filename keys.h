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
#include <X11/XF86keysym.h>
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

static KeyItem KeyItemsByTag[] = 
{
    { "view", view },
    { "toggleview", toggleview },
    { "focusview", focusview },
    { "tag", tag },
    { "toggletag", toggletag },
};

void
parsekey(char *s, Key *k) {
    char mod[15]="\0";
    unsigned long modmask = 0;
    char key[32]="\0";
    char *arg = emallocz(sizeof(char)*64);
    int i;
    sscanf(s, "%s + %s = %s", mod, key, arg);
    for(i = 0; i<3; i++){
        if(mod[i]=='A')
            modmask = modmask | Mod1Mask;
        if(mod[i]=='S')
            modmask = modmask | ShiftMask;
        if(mod[i]=='C')
            modmask = modmask | ControlMask;
    }
    k->mod = modmask;
    if(strlen(arg))
        k->arg = arg;
    else
        free(arg);
    fprintf(stderr, "%s : mod=%s : key=[%s] arg = %s ksym = %s\n",s, mod, key, arg, XKeysymToString(XStringToKeysym(key)));
    k->keysym = XStringToKeysym(key);
}

int
initkeys(){
    int i,j,k;
    char *tmp;
    char t[64];
    dkeys = malloc(sizeof(Key*)*LENGTH(KeyItems));
    /* global functions */
    for(i = 0; i < LENGTH(KeyItems); i++){
        tmp = getresource(KeyItems[i].name, NULL);
        dkeys[i] = malloc(sizeof(Key));
        dkeys[i]->func = KeyItems[i].action;
        dkeys[i]->arg = NULL;
        parsekey(tmp, dkeys[i]);
        ndkeys = i;
    }
    k = i;
    /* per tag functions */
    for(j = 0; j < LENGTH(KeyItemsByTag); j++){
        for(i = 0; i < ndtags; i++){
            sprintf(t, "%s%d", KeyItemsByTag[j].name, i);
            fprintf(stderr, "KeyItemsByTag[%d]=[%s%d]\n", j, KeyItemsByTag[j].name, i);
            tmp = getresource(t, NULL);
            if(!tmp)
                continue;
            dkeys = realloc(dkeys, sizeof(Key*)*(k+1));
            dkeys[k] = malloc(sizeof(Key));
            dkeys[k]->func = KeyItemsByTag[j].action;
            dkeys[k]->arg = tags[i];
            fprintf(stderr, "arg=%s\n", dkeys[k]->arg);
            parsekey(tmp, dkeys[k]);
            k++;
            ndkeys = k;
        }
    }
    /* layout setting */
    for(i = 0; i<LENGTH(layouts); i++){
            sprintf(t, "setlayout%s", layouts[i].symbol);
            fprintf(stderr, "setlayout%s\n", layouts[i].symbol);
            tmp = getresource(t, NULL);
            if(!tmp)
                continue;
            dkeys = realloc(dkeys, sizeof(Key*)*(k+1));
            dkeys[k] = malloc(sizeof(Key));
            dkeys[k]->func = setlayout;
            dkeys[k]->arg = layouts[i].symbol;
            fprintf(stderr, "arg=%s\n", dkeys[k]->arg);
            parsekey(tmp, dkeys[k]);
            k++;
            ndkeys = k;
    }
    /* spawn */
     for(i = 0; i<64; i++){
            sprintf(t, "spawn%d", i);
            fprintf(stderr, "spawn%d\n", i);
            tmp = getresource(t, NULL);
            if(!tmp)
                continue;
            dkeys = realloc(dkeys, sizeof(Key*)*(k+1));
            dkeys[k] = malloc(sizeof(Key));
            dkeys[k]->func = spawn;
            dkeys[k]->arg = NULL;
            parsekey(tmp, dkeys[k]);
            fprintf(stderr, "arg=%s\n", dkeys[k]->arg);
            k++;
            ndkeys = k;
    }
 

    return 0;
}
