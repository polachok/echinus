/*
 *    echinus wm written by Alexander Polakov <polachok@gmail.com> at 24.03.2008 00:19:38 MSK
 */

Rule **drules;

  //  { "Firefox.*",          "web",      False, True }, 
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
    drules = malloc(sizeof(Rule*));
    for(i=0; i < 64; i++){
            sprintf(t, "rule%d", i);
            tmp = getresource(t, NULL);
            fprintf(stderr, "rule%d=%s\n", i, tmp);
            if(!tmp)
                continue;
            drules[i] = malloc(sizeof(Rule));
            parserule(tmp, drules[i]);
            nrules++;
    }
}
