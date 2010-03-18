int
drawtext(const char *text, Drawable drawable, XftDraw *xftdrawable, unsigned long col[ColLast], int x, int y, int mw) {
    int w, h;
    char buf[256];
    unsigned int len, olen;

    if(!text)
	    return 0;
    olen = len = strlen(text);
    w = 0;
    if(len >= sizeof buf)
	    len = sizeof buf - 1;
    memcpy(buf, text, len);
    buf[len] = 0;
    h = dc.h;
    y = dc.font.ascent;
    x += dc.font.height/2;
    /* shorten text if necessary */
    while(len && (w = textnw(buf, len)) > mw){
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
    if(w > mw)
	    return 0; /* too long */
    while(x <= 0)
	    x = dc.x++;
    XSetForeground(dpy, dc.gc, col[ColBG]);
    XFillRectangle(dpy, drawable, dc.gc, x - dc.font.height/2, 0, w + dc.font.height, h);
    XftDrawStringUtf8(xftdrawable, (col==dc.norm) ? dc.xftnorm : dc.xftsel,
	    dc.font.xftfont, x, y, (unsigned char*)buf, len);
    return w + dc.font.height;
}

Pixmap
initpixmap(const char *file, Button *b) {
    b->pm = XCreatePixmap(dpy, root, dc.h, dc.h, 1);
    if(BitmapSuccess == XReadBitmapFile(dpy, root, file, &b->pw, &b->ph, &b->pm, &b->px, &b->py)) {
	if (b->px == -1 || b->py == -1)
	    b->px = b->py = 0;
	return 0;
    }
    else
	eprint("echinus: cannot load Button pixmaps, check your ~/.echinusrc\n");
    return 0;
}

void
initbuttons() {
    XSetForeground(dpy, dc.gc, dc.norm[ColButton]);
    XSetBackground(dpy, dc.gc, dc.norm[ColBG]);
    initpixmap(getresource("button.left.pixmap", BLEFTPIXMAP), &look.bleft);
    initpixmap(getresource("button.right.pixmap", BRIGHTPIXMAP), &look.bright);
    initpixmap(getresource("button.center.pixmap", BCENTERPIXMAP), &look.bcenter);
    look.bleft.action = iconifyit;
    look.bright.action = killclient;
    look.bcenter.action = togglemax;
    look.bleft.x = look.bright.x = look.bcenter.x = -1;
}

int
drawbutton(Drawable d, Drawable btn, unsigned long col[ColLast], int x, int y) {
    XSetForeground(dpy, dc.gc, col[ColBG]);
    XFillRectangle(dpy, d, dc.gc, x, 0, dc.h, dc.h);
    XSetForeground(dpy, dc.gc, col[ColButton]);
    XSetBackground(dpy, dc.gc, col[ColBG]);
    XCopyPlane(dpy, btn, d, dc.gc, 0, 0, look.bleft.pw, look.bleft.ph, x, y+look.bleft.py, 1);
    return dc.h;
}

int 
drawelement(char which, int x, int position, Client *c) {
    int w, j;
    unsigned long *color = c == sel ? dc.sel : dc.norm;

    switch(which) {
	case 'T':
	    w = 0;
	    for(j = 0; j < ntags; j++) {
		if(c->tags[j])
		    w += drawtext(tags[j], c->title, c->xftdraw, color, dc.x, dc.y, dc.w);
	    }
	    break;
	case '|':
	    XSetForeground(dpy, dc.gc, color[ColBorder]);
	    XDrawLine(dpy, c->title, dc.gc, dc.x + dc.h/4, 0, dc.x + dc.h/4, dc.h);
	    w = dc.h/2;
	    break;
	case 'N':
	    w = drawtext(c->name, c->title, c->xftdraw, color, dc.x, dc.y, dc.w);
	    break;
	case 'I':
	    look.bleft.x = dc.x;
	    w = drawbutton(c->title, look.bleft.pm, color, dc.x, dc.h/2 - look.bleft.ph/2);
	    break;
	case 'M':
	    look.bcenter.x = dc.x;
	    w = drawbutton(c->title, look.bcenter.pm, color, dc.x, dc.h/2 - look.bcenter.ph/2);
	    break;
	case 'C':
	    look.bright.x = dc.x;
	    w = drawbutton(c->title, look.bright.pm, color, dc.x, dc.h/2 - look.bcenter.ph/2);
	    break;
	default:
	    w = 0;
	    break;
    }
    return w;
}

int
elementw(char which, Client *c) {
    int w, j;
    switch(which) {
	case 'I':
	case 'M':
	case 'C':
	    return dc.h;
	case 'N':
	    return textw(c->name);
	case 'T':
	    w = 0;
	    for(j = 0; j < ntags; j++) {
		if(c->tags[j])
		    w += textw(tags[j]);
	    }
	    return w;
	case '|':
	    return dc.h/2;
    }
    return 0;
}

void
drawclient(Client *c) {
    int i;
    unsigned int opacity;

    if(look.uf_opacity) {
	opacity = (c == sel) ? OPAQUE : look.uf_opacity * OPAQUE;
	setopacity(c, opacity);
    }
    if(!isvisible(c, NULL))
	return;
    if(!c->title)
	return;
    /* XXX: that's not nice. we map and unmap title all the time */
    if(!c->isfloating && !ISLTFLOATING(c->m) && !dectiled) {
	XUnmapWindow(dpy, c->title);
	return;
    }
    XMapRaised(dpy, c->title);
    XSetForeground(dpy, dc.gc, c == sel ? dc.sel[ColBG] : dc.norm[ColBG]);
    XSetLineAttributes(dpy, dc.gc, look.borderpx, LineSolid, CapNotLast, JoinMiter);
    XFillRectangle(dpy, c->title, dc.gc, 0, 0, c->w, c->th);
    dc.x = dc.y = 0;
    dc.w = c->w;
    if(dc.w < textw(c->name)) {
	dc.w -= dc.h;
	look.bright.x = dc.w;
	drawtext(c->name, c->title, c->xftdraw, c == sel ? dc.sel : dc.norm, dc.x, dc.y, dc.w);
	drawbutton(c->title, look.bright.pm, c == sel ? dc.sel : dc.norm, dc.w, dc.h/2 - look.bcenter.ph/2);
	goto end;
    }
    /* Left */
    for(i = 0; i < strlen(look.titlelayout); i++) {
	if(look.titlelayout[i] == ' ' || look.titlelayout[i] == '-')
		break;
	dc.x += drawelement(look.titlelayout[i], dc.x, AlignLeft, c);
    }
    if(i == strlen(look.titlelayout) || dc.x >= dc.w)
	goto end;
    /* Center */
    dc.x = dc.w/2;
    for(i++; i < strlen(look.titlelayout); i++) {
	if(look.titlelayout[i] == ' ' || look.titlelayout[i] == '-')
	    break;
	dc.x -= elementw(look.titlelayout[i], c)/2;
	dc.x += drawelement(look.titlelayout[i], 0, AlignCenter, c);
    }
    if(i == strlen(look.titlelayout) || dc.x >= dc.w)
	goto end;
    /* Right */
    dc.x = dc.w;
    for(i = strlen(look.titlelayout)-1; i >= 0; i--) {
	if(look.titlelayout[i] == ' ' || look.titlelayout[i] == '-')
	    break;
	dc.x -= elementw(look.titlelayout[i], c);
	drawelement(look.titlelayout[i], 0, AlignRight, c);
    }
end:
    if(look.drawoutline) {
	XSetForeground(dpy, dc.gc, c == sel ? dc.sel[ColBorder] : dc.norm[ColBorder]);
	XDrawLine(dpy, c->title, dc.gc, 0, dc.h-1, dc.w, dc.h-1);
    }
}

static void
initfont(const char *fontstr) {
    dc.font.xftfont = NULL;
    dc.font.xftfont = XftFontOpenXlfd(dpy, screen, fontstr);
    if(!dc.font.xftfont)
	 dc.font.xftfont = XftFontOpenName(dpy, screen, fontstr);
    if(!dc.font.xftfont)
	 eprint("error, cannot load font: '%s'\n", fontstr);
    dc.font.extents = emallocz(sizeof(XGlyphInfo));
    XftTextExtentsUtf8(dpy, dc.font.xftfont,
	(const unsigned char*)fontstr, strlen(fontstr), dc.font.extents);
    dc.font.height = dc.font.xftfont->ascent + dc.font.xftfont->descent + 1;
    dc.font.ascent = dc.font.xftfont->ascent;
    dc.font.descent = dc.font.xftfont->descent;
}

unsigned int
textnw(const char *text, unsigned int len) {
    XftTextExtentsUtf8(dpy, dc.font.xftfont,
	(const unsigned char*)text, strlen(text), dc.font.extents);
    return dc.font.extents->xOff;
}

unsigned int
textw(const char *text) {
    return textnw(text, strlen(text)) + dc.font.height;
}
