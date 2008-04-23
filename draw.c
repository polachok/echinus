
void
drawtext(const char *text, unsigned long col[ColLast], unsigned int position) {
    int x, y, w, h;
    char buf[256];
    unsigned int len, olen;
    XRectangle r = { dc.x, 0, dc.w, dc.h };

    XSetForeground(dpy, dc.gc, col[ColBG]);
    XFillRectangles(dpy, dc.drawable, dc.gc, &r, 1);
    if(!text)
            return;
    olen = len = strlen(text);
    w = 0;
    fprintf(stderr, "%d>>>w = %d<<<",__LINE__, w);
    fprintf(stderr, "%d>>>w = %d len=%d<<<",__LINE__, w, len);
    if(len >= sizeof buf)
            len = sizeof buf - 1;
    memcpy(buf, text, len);
    buf[len] = 0;
    fprintf(stderr, "%d>>>w = %d len=%d<<<",__LINE__, w, len);
    h = dc.h;
    y = dc.h-dc.font.height/2+1;
    x = dc.x+h/2;
    fprintf(stderr, "%d>>>w = %d dc.w = %d<<<",__LINE__, w, dc.w);
    /* shorten text if necessary */
    while(len && (w = textnw(buf, len)) > dc.w){
        fprintf(stderr, "%d w = %d dc.w = %d\n",__LINE__, w, dc.w);
            buf[--len] = 0;
    }
    fprintf(stderr, "%d>>>w = %d<<<",__LINE__, w);
    if(len < olen) {
            if(len > 1)
                    buf[len - 1] = '.';
            if(len > 2)
                    buf[len - 2] = '.';
            if(len > 3)
                    buf[len - 3] = '.';
    }
    fprintf(stderr, "%d>>>w = %d<<<",__LINE__, w);
    if(w > dc.w)
            return; /* too long */
    fprintf(stderr, "%d>>>w = %d<<<",__LINE__, w);
    switch(position) {
    case TitleCenter:
            x = dc.x + dc.w/2 - w/2;
            break;
    case TitleLeft:
            x = dc.x + h/2;
            break;
    case TitleRight:
            x = dc.w - w - h;
            break;
    }
    fprintf(stderr, "%d>>>w = %d<<<",__LINE__, w);
    while(x <= 0)
            x = dc.x++;
    fprintf(stderr, "%d>>>w = %d<<<",__LINE__, w);
    XftDrawStringUtf8(dc.xftdrawable, (col==dc.norm) ? dc.xftnorm : dc.xftsel,
            dc.font.xftfont, x, y, (unsigned char*)buf, len);
    fprintf(stderr, "x = %d %d w = %d\n", x,__LINE__, w);
    dc.x = x + w;
}

Pixmap
initpixmap(const char *file) {
    Pixmap pmap;
    pmap = XCreatePixmap(dpy, root, 20, 20, 1);
    unsigned int pw, ph;
    if(BitmapSuccess == XReadBitmapFile(dpy, root, file, &pw, &ph, &pmap, &px, &py))
        return pmap;
    else
        eprint("echinus: cannot load button pixmaps, check your ~/.echinusrc\n");
    return 0;
}

void
initbuttons() {
    XSetForeground(dpy, dc.gc, dc.norm[ColButton]);
    XSetBackground(dpy, dc.gc, dc.norm[ColBG]);
    bleft.pm = initpixmap(getresource("button.left.pixmap", BLEFTPIXMAP));
    bright.pm = initpixmap(getresource("button.right.pixmap", BRIGHTPIXMAP));
    bcenter.pm = initpixmap(getresource("button.center.pixmap", BCENTERPIXMAP));
    bleft.action = iconifyit;
    bright.action = killclient;
    bcenter.action = togglemax;
}

void
drawbuttons(Client *c) {
    XSetForeground(dpy, dc.gc, (c == sel) ? dc.sel[ColButton] : dc.norm[ColButton]);
    XSetBackground(dpy, dc.gc, (c == sel) ? dc.sel[ColBG] : dc.norm[ColBG]);
    XCopyPlane(dpy, bright.pm, dc.drawable, dc.gc, px*2, py*2, c->th, c->th*2, c->tw-c->th, 0, 1);
    XCopyPlane(dpy, bleft.pm, dc.drawable, dc.gc, px*2, py*2, c->th, c->th*2, c->tw-3*c->th, 0, 1);
    XCopyPlane(dpy, bcenter.pm, dc.drawable, dc.gc, px*2, py*2, c->th, c->th*2, c->tw-2*c->th, 0, 1);
}

void
drawclient(Client *c) {
    unsigned int i;
    unsigned int opacity;
    if(NOTITLES)
        return;
    if(!isvisible(c))
        return;
    resizetitle(c);
    XSetForeground(dpy, dc.gc, dc.norm[ColBG]);
    XFillRectangle(dpy, c->title, dc.gc, 0, 0, c->tw, c->th);
    dc.x = dc.y = 0;
    dc.w = c->w;
    dc.h = c->th;
    if(tbpos){
        for(i=0; i <= ntags; i++) {
            if(c->tags[i]){
                drawtext(tags[i], c == sel ? dc.sel : dc.norm, TitleLeft);
                XSetForeground(dpy, dc.gc, c== sel ? dc.sel[ColBorder] : dc.norm[ColBorder]);
                XDrawLine(dpy, dc.drawable, dc.gc, dc.x+dc.h/2, 0, dc.x+dc.h/2, dc.h);
                dc.x+=dc.h/2+1;
            }
        }
    }
    drawtext(c->name, c == sel ? dc.sel : dc.norm, tpos);
    if(c->tw>=6*c->th && dc.x <= c->tw-6*c->th && tpos != TitleRight)
        drawbuttons(c);
    XCopyArea(dpy, dc.drawable, c->title, dc.gc,
			0, 0, c->tw, c->th+2*borderpx, 0, 0);
    if (c==sel)
      opacity = OPAQUE;
    else
      opacity = (unsigned int) (uf_opacity * OPAQUE);
    setopacity(c, opacity);
    XMapWindow(dpy, c->title);
}

void
drawfloating() {
    Client *c;
    for(c = clients; c; c=c->next){
        if(c->isfloating){
            c->hastitle = c->hadtitle;
            drawclient(c);
            if(layout->arrange != floating && layout->arrange != ifloating){
                XRaiseWindow(dpy, c->win);
                XRaiseWindow(dpy, c->title);
            }
        }
    }
}

static void
initfont(const char *fontstr) {
    dc.font.xftfont = XftFontOpenXlfd(dpy,screen,fontstr);
    if(!dc.font.xftfont)
         dc.font.xftfont = XftFontOpenName(dpy,screen,fontstr);
    if(!dc.font.xftfont)
         eprint("error, cannot load font: '%s'\n", fontstr);
    dc.font.extents = emallocz(sizeof(XGlyphInfo));
    XftTextExtentsUtf8(dpy,dc.font.xftfont,(unsigned char*)fontstr, strlen(fontstr), dc.font.extents);
    dc.font.height = dc.font.extents->y+dc.font.extents->yOff;
    dc.font.width = (dc.font.extents->width)/strlen(fontstr);
}

unsigned int
textnw(const char *text, unsigned int len) {
    XftTextExtentsUtf8(dpy,dc.font.xftfont,(unsigned char*)text, strlen(text), dc.font.extents);
    fprintf(stderr, "width = %d len = %d", dc.font.extents->width, len);
    return dc.font.extents->width;
}

unsigned int
textw(const char *text) {
    return textnw(text, strlen(text));
}
