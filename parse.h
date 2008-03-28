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
    int l = strlen(s);
    unsigned long modmask = 0;
    char *pos, *opos;
    char *tmp;
    int i;
    pos = strchr(s, '+');
    if(!pos)
        return;
    opos = pos;
    tmp = emallocz((pos-s)*sizeof(char));
    strncpy(tmp, s, pos-s);
    for(i = 0; i <= strlen(tmp); i++){
        if(tmp[i]=='A')
            modmask = modmask | Mod1Mask;
        if(tmp[i]=='S')
            modmask = modmask | ShiftMask;
        if(tmp[i]=='C')
            modmask = modmask | ControlMask;
    }
    k->mod = modmask;
    free(tmp);
    pos = strchr(s, '=');
    /* TODO: handle = and + in keysyms */
    if(pos){
        tmp = emallocz((pos-opos)*sizeof(char));
        for(opos++;!isalnum(opos[0]);opos++);
        strncpy(tmp, opos, pos-opos-1);
        k->keysym = XStringToKeysym(tmp);
        free(tmp);
        tmp = emallocz((s+l-pos+1)*sizeof(char));
        for(pos++;!isalnum(pos[0]);pos++);
        strncpy(tmp, pos, s+l-pos);
        k->arg = tmp;
    }
    else {
        tmp = emallocz((s+l-opos)*sizeof(char));
        for(opos++;!isalnum(opos[0]);opos++);
        strncpy(tmp, opos, s+l-opos);
        k->keysym = XStringToKeysym(tmp);
        free(tmp);
    }
}

int
initkeys(){
    int i,j;
    char *tmp;
    char t[64];
    keys = malloc(sizeof(Key*)*LENGTH(KeyItems));
    /* global functions */
    for(i = 0; i < LENGTH(KeyItems); i++){
        tmp = getresource(KeyItems[i].name, NULL);
        if(!tmp)
            continue;
        keys[nkeys] = malloc(sizeof(Key));
        keys[nkeys]->func = KeyItems[i].action;
        keys[nkeys]->arg = NULL;
        parsekey(tmp, keys[nkeys]);
        nkeys++;
    }
    /* per tag functions */
    for(j = 0; j < LENGTH(KeyItemsByTag); j++){
        for(i = 0; i < ntags; i++){
            sprintf(t, "%s%d", KeyItemsByTag[j].name, i);
            tmp = getresource(t, NULL);
            if(!tmp)
                continue;
            keys = realloc(keys, sizeof(Key*)*(nkeys+1));
            keys[nkeys] = malloc(sizeof(Key));
            keys[nkeys]->func = KeyItemsByTag[j].action;
            keys[nkeys]->arg = tags[i];
            parsekey(tmp, keys[nkeys]);
            nkeys++;
        }
    }
    /* layout setting */
    for(i = 0; i<LENGTH(layouts); i++){
            sprintf(t, "setlayout%s", layouts[i].symbol);
            tmp = getresource(t, NULL);
            if(!tmp)
                continue;
            keys = realloc(keys, sizeof(Key*)*(nkeys+1));
            keys[nkeys] = malloc(sizeof(Key));
            keys[nkeys]->func = setlayout;
            keys[nkeys]->arg = layouts[i].symbol;
            parsekey(tmp, keys[nkeys]);
            nkeys++;
    }
    /* spawn */
     for(i = 0; i<64; i++){
            sprintf(t, "spawn%d", i);
            tmp = getresource(t, NULL);
            if(!tmp)
                continue;
            keys = realloc(keys, sizeof(Key*)*(nkeys+1));
            keys[nkeys] = malloc(sizeof(Key));
            keys[nkeys]->func = spawn;
            keys[nkeys]->arg = NULL;
            parsekey(tmp, keys[nkeys]);
            nkeys++;
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
    rules = emallocz(64*sizeof(Rule*));
    for(i = 0; i < 64; i++){
            sprintf(t, "rule%d", i);
            tmp = getresource(t, NULL);
            if(!tmp)
                continue;
            rules[nrules] = emallocz(sizeof(Rule));
            parserule(tmp, rules[nrules]);
            nrules++;
    }
    rules = realloc(rules, nrules*sizeof(Rule*));
}
