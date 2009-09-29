/*
 *  echinus wm written by Alexander Polakov <polachok@gmail.com>
 *  this file contains code to parse rules and keybindings
 */
typedef struct
{
    const char *name;
    void (*action)(const char *arg);
} KeyItem;

static KeyItem KeyItems[] = 
{
    { "togglestruts", togglestruts },
    { "focusnext", focusnext },
    { "focusprev", focusprev },
    { "viewprevtag", viewprevtag },
    { "viewlefttag", viewlefttag },
    { "viewrighttag", viewrighttag },
    { "quit", quit },
    { "restart", quit },
    { "killclient", killclient },
    { "togglefloating", togglefloating },
    { "decmwfact", setmwfact },
    { "incmwfact", setmwfact },
    { "incnmaster", incnmaster },
    { "decnmaster", incnmaster },
    { "iconify", iconifyit },
    { "zoom", zoom },
    { "moveright", moveresizekb },
    { "moveleft", moveresizekb },
    { "moveup", moveresizekb },
    { "movedown", moveresizekb },
    { "resizedecx", moveresizekb },
    { "resizeincx", moveresizekb },
    { "resizedecy", moveresizekb },
    { "resizeincy", moveresizekb },
    { "togglemonitor", togglemonitor },
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
    if(!s || !(pos-s) || !pos)
	return;
    opos = pos;
    for(i = 0, tmp = s; tmp < pos; i++, tmp++){
	if(*tmp=='A')
	    modmask = modmask | Mod1Mask;
	if(*tmp=='S')
	    modmask = modmask | ShiftMask;
	if(*tmp=='C')
	    modmask = modmask | ControlMask;
	if(*tmp=='W') 
	    modmask = modmask | Mod4Mask; 
    }
    k->mod = modmask;
    pos = strchr(s, '=');
    if(pos){
	tmp = emallocz((pos-opos)*sizeof(char));
	for(opos++;!isalnum(opos[0]);opos++);
	strncpy(tmp, opos, pos-opos-1);
	k->keysym = XStringToKeysym(tmp);
	free(tmp);
	tmp = emallocz((s+l-pos+1)*sizeof(char));
	for(pos++;!isgraph(pos[0]);pos++);
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

void
initmodkey(){
    char tmp;
    strncpy(&tmp, getresource("modkey", "A"), 1);
    if(tmp=='S')
	modkey = ShiftMask;
    if(tmp=='C')
	modkey = ControlMask;
    if(tmp=='W') 
	modkey = Mod4Mask; 
    else
	modkey = Mod1Mask;
}

int
initkeys(){
    int i,j;
    char *tmp;
    char t[64];

    initmodkey();
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
	    snprintf(t, 63, "%s%d", KeyItemsByTag[j].name, i);
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
	    snprintf(t, 63, "setlayout%s", layouts[i].symbol);
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
	    snprintf(t, 63, "spawn%d", i);
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
	    snprintf(t, 63, "rule%d", i);
	    tmp = getresource(t, NULL);
	    if(!tmp)
		continue;
	    rules[nrules] = emallocz(sizeof(Rule));
	    parserule(tmp, rules[nrules]);
	    nrules++;
    }
    rules = realloc(rules, nrules*sizeof(Rule*));
}
