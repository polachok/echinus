#include <regex.h>
#include <ctype.h>
#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <X11/Xproto.h>
#include <X11/Xutil.h>
#include <X11/Xft/Xft.h>
#include "echinus.h"
#include "config.h"

typedef struct {
	unsigned int x, y, w;
	struct {
		XGlyphInfo *extents;
		int ascent;
		int descent;
		int height;
		int width;
	} font;
	GC gc;
} DC;				/* draw context */

DC dc = { 0 };

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
	h = style.titleheight;
	y = style.titleheight / 2 + dc.font.ascent / 2 - 1 - style.outline;
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
	b->pm = XCreatePixmap(dpy, root, style.titleheight, style.titleheight, 1);
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
drawbutton(Drawable d, Button btn, unsigned long col[ColLast], int x, int y)
{
	XSetForeground(dpy, dc.gc, col[ColBG]);
	XFillRectangle(dpy, d, dc.gc, x, 0, style.titleheight, style.titleheight);
	XSetForeground(dpy, dc.gc, btn.pressed ? col[ColFG] : col[ColButton]);
	XSetBackground(dpy, dc.gc, col[ColBG]);
	XCopyPlane(dpy, btn.pm, d, dc.gc, 0, 0, button[Iconify].pw,
	    button[Iconify].ph, x, y + button[Iconify].py, 1);
	return style.titleheight;
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
		XDrawLine(dpy, c->drawable, dc.gc, dc.x + style.titleheight / 4, 0,
		    dc.x + style.titleheight / 4, style.titleheight);
		w = style.titleheight / 2;
		break;
	case 'N':
		w = drawtext(c->name, c->drawable, c->xftdraw, color, dc.x, dc.y, dc.w);
		break;
	case 'I':
		button[Iconify].x = dc.x;
		w = drawbutton(c->drawable, button[Iconify], color,
		    dc.x, style.titleheight / 2 - button[Iconify].ph / 2);
		break;
	case 'M':
		button[Maximize].x = dc.x;
		w = drawbutton(c->drawable, button[Maximize], color,
		    dc.x, style.titleheight / 2 - button[Maximize].ph / 2);
		break;
	case 'C':
		button[Close].x = dc.x;
		w = drawbutton(c->drawable, button[Close], color, dc.x,
		    style.titleheight / 2 - button[Maximize].ph / 2);
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
		return style.titleheight;
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
		return style.titleheight / 2;
	}
	return 0;
}

void
drawclient(Client * c)
{
	size_t i;

	if (style.opacity) {
		setopacity(c, c == sel ? OPAQUE : style.opacity);
	}
	if (!isvisible(c, NULL))
		return;
	if (!c->title)
		return;
	/* XXX: that's not nice. we map and unmap title all the time */
	if (c->ismax || (!c->isfloating && !ISLTFLOATING(clientmonitor(c)) && !dectiled)) {
		XUnmapWindow(dpy, c->title);
		return;
	}
	XftDrawChange(c->xftdraw, c->drawable);
	XSetForeground(dpy, dc.gc, c == sel ? style.color.sel[ColBG] : style.color.norm[ColBG]);
	XSetLineAttributes(dpy, dc.gc, style.border, LineSolid, CapNotLast, JoinMiter);
	XFillRectangle(dpy, c->drawable, dc.gc, 0, 0, c->w, style.titleheight);
	dc.x = dc.y = 0;
	dc.w = c->w;
	if (dc.w < textw(c->name)) {
		dc.w -= style.titleheight;
		button[Close].x = dc.w;
		drawtext(c->name, c->drawable, c->xftdraw,
		    c == sel ? style.color.sel : style.color.norm, dc.x, dc.y, dc.w);
		drawbutton(c->drawable, button[Close],
		    c == sel ? style.color.sel : style.color.norm, dc.w,
		    style.titleheight / 2 - button[Close].ph / 2);
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
	if (style.outline) {
		XSetForeground(dpy, dc.gc,
		    c == sel ? style.color.sel[ColBorder] : style.color.norm[ColBorder]);
		XDrawLine(dpy, c->drawable, dc.gc, 0, style.titleheight - 1, dc.w, style.titleheight - 1);
	}
	XCopyArea(dpy, c->drawable, c->title, dc.gc, 0, 0, c->w, style.titleheight, 0, 0);
	XMapRaised(dpy, c->title);
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

void
initstyle()
{
	style.color.norm[ColBorder] = getcolor(getresource("normal.border", NORMBORDERCOLOR));
	style.color.norm[ColBG] = getcolor(getresource("normal.bg", NORMBGCOLOR));
	style.color.norm[ColFG] = getcolor(getresource("normal.fg", NORMFGCOLOR));
	style.color.norm[ColButton] = getcolor(getresource("normal.button", NORMBUTTONCOLOR));

	style.color.sel[ColBorder] = getcolor(getresource("selected.border", SELBORDERCOLOR));
	style.color.sel[ColBG] = getcolor(getresource("selected.bg", SELBGCOLOR));
	style.color.sel[ColFG] = getcolor(getresource("selected.fg", SELFGCOLOR));
	style.color.sel[ColButton] = getcolor(getresource("selected.button", SELBUTTONCOLOR));

	style.color.font[Selected] = emallocz(sizeof(XftColor));
	style.color.font[Normal] = emallocz(sizeof(XftColor));
	XftColorAllocName(dpy, DefaultVisual(dpy, screen), DefaultColormap(dpy,
				screen), getresource("selected.fg", SELFGCOLOR), style.color.font[Selected]);
	XftColorAllocName(dpy, DefaultVisual(dpy, screen), DefaultColormap(dpy,
				screen), getresource("normal.fg", SELFGCOLOR), style.color.font[Normal]);
	if (!style.color.font[Normal] || !style.color.font[Normal])
		eprint("error, cannot allocate colors\n");
	initfont(getresource("font", FONT));
	style.border = atoi(getresource("border", STR(BORDERPX)));
	style.opacity = OPAQUE * atof(getresource("opacity", STR(NF_OPACITY)));
	style.outline = atoi(getresource("outline", "0"));
	strncpy(style.titlelayout, getresource("titlelayout", "N  IMC"),
	    LENGTH(style.titlelayout));
	style.titlelayout[LENGTH(style.titlelayout) - 1] = '\0';
	style.titleheight = atoi(getresource("title", STR(TITLEHEIGHT)));
	if (!style.titleheight)
		style.titleheight = dc.font.height + 2;
	dc.gc = XCreateGC(dpy, root, 0, 0);
	initbuttons();
}

void
deinitstyle()
{	
	/* XXX: more to do */
	XftColorFree(dpy, DefaultVisual(dpy, screen), DefaultColormap(dpy,
		screen), style.color.font[Normal]);
	XftColorFree(dpy, DefaultVisual(dpy, screen), DefaultColormap(dpy,
		screen), style.color.font[Selected]);
	XftFontClose(dpy, style.font);
	free(dc.font.extents);
	XFreeGC(dpy, dc.gc);
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
