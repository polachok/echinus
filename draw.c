
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
    if(len >= sizeof buf)
            len = sizeof buf - 1;
    memcpy(buf, text, len);
    buf[len] = 0;
    h = dc.h;
    y = dc.h-dc.font.height/2;
    x = dc.x+h/2;
    /* shorten text if necessary */
    while(len && (w = textnw(buf, len)) > dc.w){
            buf[--len] = 0;
    }
    if(len < olen) {
            if(len > 1)
                    buf[len - 1] = '.';
            if(len > 2)
                    buf[len - 2] = '.';
            if(len > 3)
                    buf[len - 3] = '.';
    }
    if(w > dc.w)
            return; /* too long */
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
    while(x <= 0)
            x = dc.x++;
    XftDrawStringUtf8(dc.xftdrawable, (col==dc.norm) ? dc.xftnorm : dc.xftsel,
            dc.font.xftfont, x, drawoutline ? y : y+1, (unsigned char*)buf, len);
    if(drawoutline){
                XSetForeground(dpy, dc.gc, col[ColBorder]);
                XDrawLine(dpy, dc.drawable, dc.gc, 0, dc.h-1, dc.w, dc.h-1);
    }
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
    int x, y;
    y = drawoutline ? dc.h * 2 : dc.h*2+2;
    x = c->w-3*dc.h;
    XSetForeground(dpy, dc.gc, (c == sel) ? dc.sel[ColButton] : dc.norm[ColButton]);
    XSetBackground(dpy, dc.gc, (c == sel) ? dc.sel[ColBG] : dc.norm[ColBG]);

    XCopyPlane(dpy, bleft.pm, dc.drawable, dc.gc, px*2, py*2, dc.h, y, x, 0, 1);
    x+=dc.h;
    XCopyPlane(dpy, bcenter.pm, dc.drawable, dc.gc, px*2, py*2, dc.h, y, x, 0, 1);
    x+=dc.h;
    XCopyPlane(dpy, bright.pm, dc.drawable, dc.gc, px*2, py*2, dc.h, y, x, 0, 1);
}

void
drawclient(Client *c) {
    unsigned int i;
    unsigned int opacity;
    if(NOTITLES)
        return;
    if(!isvisible(c))
        return;
    if(c->isfloating)
        resize(c, c->x, c->y, c->w, c->h, True);
    XSetForeground(dpy, dc.gc, dc.norm[ColBG]);
    XSetLineAttributes(dpy, dc.gc, borderpx, LineSolid, CapNotLast, JoinMiter);
    XFillRectangle(dpy, c->title, dc.gc, 0, 0, c->w, dc.h);
    dc.x = dc.y = 0;
    dc.w = c->w;
    dc.h = dc.h;
    if(tbpos){
        for(i=0; i < ntags; i++) {
            if(c->tags[i]){
                drawtext(tags[i], c == sel ? dc.sel : dc.norm, TitleLeft);
                XSetForeground(dpy, dc.gc, c== sel ? dc.sel[ColBorder] : dc.norm[ColBorder]);
                if(c->border)
                    XDrawLine(dpy, dc.drawable, dc.gc, dc.x+dc.h/2, 0, dc.x+dc.h/2, dc.h);
                dc.x+=dc.h/2+1;
            }
        }
    }
    drawtext(c->name, c == sel ? dc.sel : dc.norm, tpos);
    if(c->w>=6*dc.h && dc.x <= c->w-6*dc.h && tpos != TitleRight)
        drawbuttons(c);
    XCopyArea(dpy, dc.drawable, c->title, dc.gc,
			0, 0, c->w, dc.h+2*borderpx, 0, 0);
    if (c==sel)
      opacity = OPAQUE;
    else {
      if(uf_opacity)
	      opacity = (unsigned int) (uf_opacity * OPAQUE);
      else
	      opacity = OPAQUE;
    }
    setopacity(c, opacity);
    if(c->title)
        XMapWindow(dpy, c->title);
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
    return dc.font.extents->width;
}

unsigned int
textw(const char *text) {
    return textnw(text, strlen(text));
}
