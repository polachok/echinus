#include <regex.h>
#include <ctype.h>
#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <X11/Xproto.h>
#include <X11/Xutil.h>
#include <X11/Xft/Xft.h>
#include "echinus.h"
#include "config.h"

int
drawtext(const char *text, Drawable drawable, XftDraw * xftdrawable,
    unsigned long col[ColLast], int x, int y, int mw)
{
	int w, h;
	char buf[256];
	unsigned int len, olen;

	if (!text)
		return 0;
	olen = len = strlen(text);
	w = 0;
	if (len >= sizeof buf)
		len = sizeof buf - 1;
	memcpy(buf, text, len);
	buf[len] = 0;
	h = dc.h;
	y = dc.h / 2 + dc.font.ascent / 2 - 1 - style.drawoutline;
	x += dc.font.height / 2;
	/* shorten text if necessary */
	while (len && (w = textnw(buf, len)) > mw) {
		buf[--len] = 0;
	}
	if (len < olen) {
		if (len > 1)
			buf[len - 1] = '.';
		if (len > 2)
			buf[len - 2] = '.';
		if (len > 3)
			buf[len - 3] = '.';
	}
	if (w > mw)
		return 0;	/* too long */
	while (x <= 0)
		x = dc.x++;
	XSetForeground(dpy, dc.gc, col[ColBG]);
	XFillRectangle(dpy, drawable, dc.gc, x - dc.font.height / 2, 0,
	    w + dc.font.height, h);
	XftDrawStringUtf8(xftdrawable,
	    (col == style.color.norm) ? style.color.font[Normal] : style.color.font[Selected],
	    style.font, x, y, (unsigned char *) buf, len);
	return w + dc.font.height;
}

Pixmap
initpixmap(const char *file, Button * b)
{
	b->pm = XCreatePixmap(dpy, root, dc.h, dc.h, 1);
	if (BitmapSuccess == XReadBitmapFile(dpy, root, file, &b->pw, &b->ph,
		&b->pm, &b->px, &b->py)) {
		if (b->px == -1 || b->py == -1)
			b->px = b->py = 0;
		return 0;
	} else
		eprint("echinus: cannot load Button pixmaps, check your ~/.echinusrc\n");
	return 0;
}

void
initbuttons()
{
	XSetForeground(dpy, dc.gc, style.color.norm[ColButton]);
	XSetBackground(dpy, dc.gc, style.color.norm[ColBG]);
	initpixmap(getresource("button.iconify.pixmap", ICONPIXMAP),
	    &button[Iconify]);
	initpixmap(getresource("button.maximize.pixmap", MAXPIXMAP),
	    &button[Maximize]);
	initpixmap(getresource("button.close.pixmap", CLOSEPIXMAP), &button[Close]);
	button[Iconify].action = iconifyit;
	button[Maximize].action = togglemax;
	button[Close].action = killclient;
	button[Iconify].x = button[Close].x = button[Maximize].x = -1;
}

int
drawbutton(Drawable d, Drawable btn, unsigned long col[ColLast], int x, int y)
{
	XSetForeground(dpy, dc.gc, col[ColBG]);
	XFillRectangle(dpy, d, dc.gc, x, 0, dc.h, dc.h);
	XSetForeground(dpy, dc.gc, col[ColButton]);
	XSetBackground(dpy, dc.gc, col[ColBG]);
	XCopyPlane(dpy, btn, d, dc.gc, 0, 0, button[Iconify].pw,
	    button[Iconify].ph, x, y + button[Iconify].py, 1);
	return dc.h;
}

int
drawelement(char which, int x, int position, Client * c)
{
	int w;
	unsigned int j;
	unsigned long *color = c == sel ? style.color.sel : style.color.norm;

	switch (which) {
	case 'T':
		w = 0;
		for (j = 0; j < ntags; j++) {
			if (c->tags[j])
				w += drawtext(tags[j], c->drawable, c->xftdraw,
				    color, dc.x, dc.y, dc.w);
		}
		break;
	case '|':
		XSetForeground(dpy, dc.gc, color[ColBorder]);
		XDrawLine(dpy, c->drawable, dc.gc, dc.x + dc.h / 4, 0,
		    dc.x + dc.h / 4, dc.h);
		w = dc.h / 2;
		break;
	case 'N':
		w = drawtext(c->name, c->drawable, c->xftdraw, color, dc.x, dc.y, dc.w);
		break;
	case 'I':
		button[Iconify].x = dc.x;
		w = drawbutton(c->drawable, button[Iconify].pm, color,
		    dc.x, dc.h / 2 - button[Iconify].ph / 2);
		break;
	case 'M':
		button[Maximize].x = dc.x;
		w = drawbutton(c->drawable, button[Maximize].pm, color,
		    dc.x, dc.h / 2 - button[Maximize].ph / 2);
		break;
	case 'C':
		button[Close].x = dc.x;
		w = drawbutton(c->drawable, button[Close].pm, color, dc.x,
		    dc.h / 2 - button[Maximize].ph / 2);
		break;
	default:
		w = 0;
		break;
	}
	return w;
}

int
elementw(char which, Client * c)
{
	int w;
	unsigned int j;

	switch (which) {
	case 'I':
	case 'M':
	case 'C':
		return dc.h;
	case 'N':
		return textw(c->name);
	case 'T':
		w = 0;
		for (j = 0; j < ntags; j++) {
			if (c->tags[j])
				w += textw(tags[j]);
		}
		return w;
	case '|':
		return dc.h / 2;
	}
	return 0;
}

void
drawclient(Client * c)
{
	size_t i;
	unsigned int opacity;

	if (style.uf_opacity) {
		opacity = (c == sel) ? OPAQUE : style.uf_opacity * OPAQUE;
		setopacity(c, opacity);
	}
	if (!isvisible(c, NULL))
		return;
	if (!c->title)
		return;
	/* XXX: that's not nice. we map and unmap title all the time */
	if (!c->isfloating && !ISLTFLOATING(clientmonitor(c)) && !dectiled) {
		XUnmapWindow(dpy, c->title);
		return;
	}
	XMapRaised(dpy, c->title);
	XftDrawChange(c->xftdraw, c->drawable);
	XSetForeground(dpy, dc.gc, c == sel ? style.color.sel[ColBG] : style.color.norm[ColBG]);
	XSetLineAttributes(dpy, dc.gc, style.borderpx, LineSolid, CapNotLast, JoinMiter);
	XFillRectangle(dpy, c->drawable, dc.gc, 0, 0, c->w, c->th);
	dc.x = dc.y = 0;
	dc.w = c->w;
	if (dc.w < textw(c->name)) {
		dc.w -= dc.h;
		button[Close].x = dc.w;
		drawtext(c->name, c->drawable, c->xftdraw,
		    c == sel ? style.color.sel : style.color.norm, dc.x, dc.y, dc.w);
		drawbutton(c->drawable, button[Close].pm,
		    c == sel ? style.color.sel : style.color.norm, dc.w,
		    dc.h / 2 - button[Close].ph / 2);
		goto end;
	}
	/* Left */
	for (i = 0; i < strlen(style.titlelayout); i++) {
		if (style.titlelayout[i] == ' ' || style.titlelayout[i] == '-')
			break;
		dc.x += drawelement(style.titlelayout[i], dc.x, AlignLeft, c);
	}
	if (i == strlen(style.titlelayout) || dc.x >= dc.w)
		goto end;
	/* Center */
	dc.x = dc.w / 2;
	for (i++; i < strlen(style.titlelayout); i++) {
		if (style.titlelayout[i] == ' ' || style.titlelayout[i] == '-')
			break;
		dc.x -= elementw(style.titlelayout[i], c) / 2;
		dc.x += drawelement(style.titlelayout[i], 0, AlignCenter, c);
	}
	if (i == strlen(style.titlelayout) || dc.x >= dc.w)
		goto end;
	/* Right */
	dc.x = dc.w;
	for (i = strlen(style.titlelayout); i-- ; ) {
		if (style.titlelayout[i] == ' ' || style.titlelayout[i] == '-')
			break;
		dc.x -= elementw(style.titlelayout[i], c);
		drawelement(style.titlelayout[i], 0, AlignRight, c);
	}
      end:
	if (style.drawoutline) {
		XSetForeground(dpy, dc.gc,
		    c == sel ? style.color.sel[ColBorder] : style.color.norm[ColBorder]);
		XDrawLine(dpy, c->drawable, dc.gc, 0, dc.h - 1, dc.w, dc.h - 1);
	}
	XCopyArea(dpy, c->drawable, c->title, dc.gc, 0, 0, c->w, c->th, 0, 0);
}

void
initfont(const char *fontstr)
{
	style.font = NULL;
	style.font = XftFontOpenXlfd(dpy, screen, fontstr);
	if (!style.font)
		style.font = XftFontOpenName(dpy, screen, fontstr);
	if (!style.font)
		eprint("error, cannot load font: '%s'\n", fontstr);
	dc.font.extents = emallocz(sizeof(XGlyphInfo));
	XftTextExtentsUtf8(dpy, style.font,
	    (const unsigned char *) fontstr, strlen(fontstr), dc.font.extents);
	dc.font.height = style.font->ascent + style.font->descent + 1;
	dc.font.ascent = style.font->ascent;
	dc.font.descent = style.font->descent;
}

unsigned int
textnw(const char *text, unsigned int len)
{
	XftTextExtentsUtf8(dpy, style.font,
	    (const unsigned char *) text, len, dc.font.extents);
	return dc.font.extents->xOff;
}

unsigned int
textw(const char *text)
{
	return textnw(text, strlen(text)) + dc.font.height;
}
