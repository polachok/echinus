/*
 *  echinus wm written by Alexander Polakov <polachok@gmail.com> at 23.03.2008 19:47:22 MSK
 *  this file contains code to parse rules and keybindings
 */
typedef struct
{
    const char *name;
    void (*action)(const char *arg);
} KeyItem;

static KeyItem KeyItems[] = 
{
    { "togglebar", togglebar },
    { "focusnext", focusnext },
    { "focusprev", focusprev },
    { "viewprevtag", viewprevtag },
    { "quit", quit },
    { "restart", restart },
    { "killclient", killclient },
    { "togglefloating", togglefloating },
    { "decmwfact", setmwfact },
    { "incmwfact", setmwfact },
    { "incnmaster", incnmaster },
    { "decnmaster", incnmaster },
    { "iconify", iconifyit },
    { "zoom", zoom },
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
    k->keysym = XStringToKeysym(key);
}

int
initkeys(){
    int i,j,k;
    char *tmp;
    char t[64];
    keys = malloc(sizeof(Key*)*LENGTH(KeyItems));
    /* global functions */
    for(i = 0; i < LENGTH(KeyItems); i++){
        tmp = getresource(KeyItems[i].name, NULL);
        keys[i] = malloc(sizeof(Key));
        keys[i]->func = KeyItems[i].action;
        keys[i]->arg = NULL;
        parsekey(tmp, keys[i]);
        nkeys = i;
    }
    k = i;
    /* per tag functions */
    for(j = 0; j < LENGTH(KeyItemsByTag); j++){
        for(i = 0; i < ntags; i++){
            sprintf(t, "%s%d", KeyItemsByTag[j].name, i);
            tmp = getresource(t, NULL);
            if(!tmp)
                continue;
            keys = realloc(keys, sizeof(Key*)*(k+1));
            keys[k] = malloc(sizeof(Key));
            keys[k]->func = KeyItemsByTag[j].action;
            keys[k]->arg = tags[i];
            parsekey(tmp, keys[k]);
            k++;
            nkeys = k;
        }
    }
    /* layout setting */
    for(i = 0; i<LENGTH(layouts); i++){
            sprintf(t, "setlayout%s", layouts[i].symbol);
            tmp = getresource(t, NULL);
            if(!tmp)
                continue;
            keys = realloc(keys, sizeof(Key*)*(k+1));
            keys[k] = malloc(sizeof(Key));
            keys[k]->func = setlayout;
            keys[k]->arg = layouts[i].symbol;
            parsekey(tmp, keys[k]);
            k++;
            nkeys = k;
    }
    /* spawn */
     for(i = 0; i<64; i++){
            sprintf(t, "spawn%d", i);
            tmp = getresource(t, NULL);
            if(!tmp)
                continue;
            keys = realloc(keys, sizeof(Key*)*(k+1));
            keys[k] = malloc(sizeof(Key));
            keys[k]->func = spawn;
            keys[k]->arg = NULL;
            parsekey(tmp, keys[k]);
            k++;
            nkeys = k;
    }
 

    return 0;
}

void 
parserule(char *s, Rule *r){
    char *prop = emallocz(sizeof(char)*128);
    char *tags = emallocz(sizeof(char)*64);
    sscanf(s, "%s %s %d %d", prop, tags, &r->isfloating, &r->hastitle);
    r->prop = prop;
    r->tags = tags;
}

void
initrules(){
    int i;
    char t[64];
    char *tmp;
    rules = malloc(sizeof(Rule*));
    for(i=0; i < 64; i++){
            sprintf(t, "rule%d", i);
            tmp = getresource(t, NULL);
            if(!tmp)
                continue;
            rules[i] = malloc(sizeof(Rule));
            parserule(tmp, rules[i]);
            nrules++;
    }
}
