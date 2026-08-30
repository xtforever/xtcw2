/*

    Copyright (C) 1994 Clark McGrew

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public
    License along with this library; if not, write to the Free
    Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

 */

#include <X11/IntrinsicP.h>
#include <X11/StringDefs.h>

#include <stdio.h>
#include <math.h>

#include <Xfwf/UGrafP.h>

/*
 *----------------------------------------------------------------------
 *
 * Results:
 *    Convert an hsv color to an rgb color.
 *
 * Side effects:
 *    None.
 *
 *----------------------------------------------------------------------
 */
#define range(v,l,h) (((v)<(l)) ? (l): (((v)>(h)) ? (h) : (v)))
static void HSVtoXColor(pw, h, s, v, c)
     UGrafWidget pw;
     float h;
     float s;
     float v;
     XColor *c;
{
  int i; 
  double f, p, q, t;
  double rr, gg, bb;
  double hh = h, ss = s/100.0, vv = v/100.0;
  double gmma;

  if (ss<=0.0) {
    rr = vv;
    gg = vv;
    bb = vv;
  }
  else {
    for (;hh>=360.0;hh = hh - 360.0);
    for (;hh<0.0;hh = hh + 360.0);
    hh = hh/60.0;
    i = hh;
    i = i % 6;
    f = hh - i;
    range(f,0.0,1.0);
    p = vv*(1.0-ss);
    q = vv*(1.0-(ss*f));
    t = vv*(1.0-(ss*(1.0-f)));
    range(p,0.0,1.0);
    range(q,0.0,1.0);
    range(t,0.0,1.0);
    switch (i) {
    case 0:			/* 0.0 pure red */
      rr = vv; gg = t; bb = p; break;
    case 1:			/* 1.0 pure yellow */
      rr = q; gg = vv; bb = p; break;
    case 2:			/* 2.0 pure green */
      rr = p; gg = vv; bb = t; break;
    case 3:			/* 3.0 pure cyan */
      rr = p; gg = q; bb = vv; break;
    case 4:			/* 4.0 pure blue */
      rr = t; gg = p; bb = vv; break;
    case 5:			/* 5.0 pure magenta */
      rr = vv; gg = p; bb = q; break;
    default:
      printf("invalid h s v parameters");
    }
  }
  /* Adjust the Gamma */
  gmma = 1.0/UGrafX(pw)->color.gamma_red/UGrafX(pw)->color.gamma_int;
  rr = pow(rr+1.0E-9,gmma);
  gmma = 1.0/UGrafX(pw)->color.gamma_green/UGrafX(pw)->color.gamma_int;
  gg = pow(gg+1.0E-9,gmma);
  gmma = 1.0/UGrafX(pw)->color.gamma_blue/UGrafX(pw)->color.gamma_int;
  bb = pow(bb+1.0E-9,gmma);
  /* Set the XColor. */
  c->red = 65535*rr;
  c->green = 65535*gg;
  c->blue = 65535*bb;
  c->flags = DoRed + DoGreen + DoBlue;
}

static void DefineGrays(pw)
     UGrafWidget pw;
 {
    int i;

    for (i=0; i<MaxGrays; ++i) {
	float c = 100.0*i/(MaxGrays-1);
	HSVtoXColor(pw,0.0,0.0,c,UGrafGrays(pw,i));
    }
}    

static void DefineColors(pw)
     UGrafWidget pw;
 {
    int i;
    XColor c;

    HSVtoXColor(pw,  0.0,  0.0,  0.0,UGrafColors(pw,0));
    HSVtoXColor(pw,  0.0,  0.0,100.0,UGrafColors(pw,1));
    HSVtoXColor(pw,  0.0,100.0,100.0,UGrafColors(pw,2));
    HSVtoXColor(pw,120.0,100.0,100.0,UGrafColors(pw,3));
    HSVtoXColor(pw,240.0,100.0,100.0,UGrafColors(pw,4));
    HSVtoXColor(pw, 60.0,100.0,100.0,UGrafColors(pw,5));
    HSVtoXColor(pw,180.0,100.0,100.0,UGrafColors(pw,6));
    HSVtoXColor(pw,300.0,100.0,100.0,UGrafColors(pw,7));
}    

static void DefineShades(pw)
     UGrafWidget pw;
 {
    int H,V;
    XColor c;
    float low_hue = UGrafX(pw)->color.low_hue;
    float high_hue = UGrafX(pw)->color.high_hue;
    float low_shade = UGrafX(pw)->color.low_shade;
    float high_shade = UGrafX(pw)->color.high_shade;
    float low_value = UGrafX(pw)->color.low_value;
    float high_value = UGrafX(pw)->color.high_value;

    for (V=0; V<MaxValues; ++V) {
	for (H=0; H<MaxHues; ++H) {
	    float h = 1.0*H/(MaxHues-1);
	    float s = 1.0*V/(MaxValues-1);
	    double vv = s;
	    double gmma = UGrafX(pw)->color.gamma_int;
	    float v = pow(vv,gmma);
	    v = (high_value-low_value)*v+low_value;
	    s = (high_shade-low_shade)*s+low_shade;
	    h = (high_hue-low_hue)*h + low_hue;
	    HSVtoXColor(pw,h,s,v,UGrafShades(pw,H,V));
	}
    }
}    

void DefinePalette(pw)
     UGrafWidget pw;
 {
    Pixel pxls[ColorsInPalette];
    int i;

    XAllocColorCells(XtDisplay(pw),UGrafCore(pw)->colormap,False,0,0,
		     pxls,ColorsInPalette);

    for (i=0; i<ColorsInPalette; ++i) Palette(pw,i)->pixel = pxls[i];

    DefineGrays(pw);
    DefineColors(pw);
    DefineShades(pw);

    XStoreColors(XtDisplay(pw),UGrafCore(pw)->colormap,
		   Palette(pw,0),ColorsInPalette);
}

#ifdef NeedFunctionPrototypes
void XfwfUGGray(UGrafWidget pw, float g)
#else
void XfwfUGGray(pw, g)
     UGrafWidget pw;
     float g;
#endif
 {
    int i;
    if (g<0.0) g = 0.0;
    if (g>0.9999) g = 0.9999;
    i = g*MaxGrays;
    UGrafX(pw)->foreground = UGrafGrays(pw,i)->pixel;
    XSetForeground(XtDisplay(pw),UGrafX(pw)->gc,UGrafX(pw)->foreground);
}

void XfwfUGColor(pw, c)
     UGrafWidget pw;
     int c;
 {
    c = c % MaxColors;
    UGrafX(pw)->foreground = UGrafColors(pw,c)->pixel;
    XSetForeground(XtDisplay(pw),UGrafX(pw)->gc,UGrafX(pw)->foreground);
}

#ifdef NeedFunctionPrototypes
void XfwfUGShade(UGrafWidget pw, float h, float v)
#else
void XfwfUGShade(pw, h, v)
     UGrafWidget pw;
     float h;
     float v;
#endif
 {
    int i, j;
    h = range(h,0.0,0.999);
    v = range(v,0.0,0.999);
    i = floor(h*MaxHues);
    j = floor(v*MaxValues);
    i = range(i,0,MaxHues-1);
    j = range(j,0,MaxValues-1);
    UGrafX(pw)->foreground = UGrafShades(pw,i,j)->pixel;
    XSetForeground(XtDisplay(pw),UGrafX(pw)->gc,UGrafX(pw)->foreground);
}
