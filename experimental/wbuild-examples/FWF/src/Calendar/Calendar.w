# XfwfCalendar -  widget to display calendar for one month
# Lachlan Wetherall (lwether@cs.newcastle.edu.au)
# Dec 1995 v1.0 - initial release
#
# TODO: - make button pixmaps a settable resource.
#       - make month names and weekday abbreviations settable resources
#         to facilitate international use.
#       - make the expose routine more clever ?
#       - when changing the month in response to a mouse click or button press,
#         redraw the widget directly, rather than use a call to XtVaSetValues
#         (Will reduce flicker when auto-repeating.)
#       - Add a type convertor for the dayColors resource


@class XfwfCalendar (XfwfBoard)	@file=Calendar

@ The |XfwfCalendar| widget shows a calendar for one month.  Buttons in the header
allow the calendar month to be changed.  Individual dates within the calendar
can be set to one of 5 different colors. Callbacks are provided for when the
user changes the displayed month, or clicks on a date in the calendar.

@PUBLIC

@ The day numbers in the calendar are drawn using this font.

	@var <FontStruct> XFontStruct *calendarFont = <String> XtDefaultFont

@ The heading text is drawn using this font.

	@var <FontStruct> XFontStruct *headingFont = <String> XtDefaultFont

@ The |color1-5| resources in conjunctions with the |dayColors| resource
specify the colors used for writing the calendar dates.

        @var Pixel color1 = <String> XtDefaultForeground
        @var Pixel color2 = <String> XtDefaultForeground
        @var Pixel color3 = <String> XtDefaultForeground
        @var Pixel color4 = <String> XtDefaultForeground
        @var Pixel color5 = <String> XtDefaultForeground

@ The color to be used for drawing the heading text and buttons.

	@var Pixel headingColor = <String> XtDefaultForeground

@ The color to be used for drawing boxes around the dates.

	@var Pixel boxColor = <String> XtDefaultForeground

@ The color to be used for writing the weekday abbreviations.

	@var Pixel weekdayColor = <String> XtDefaultForeground


@ This resource is a 32 element array of integers.  Array elements 1-31 contain
a number 0-5 to specify what color the corresponding date will be, a value of
zero specifies the default color which is indicated in array element 0.  
Additionally, if the symbolic constant |XfwfReverseDate| is or'ed with the
integer, the corresponding date will be shown in reverse video.

There
is no type convertor for this resource (yet) so it is only settable via
the application calling Xt(Va)SetValues or preferably calling the convenience
function |XfwfSetDayColors|.

	@var int *dayColors = NULL

@ This resource determines how much open space the widget will have. 
It determines things like the distance between the calendar and the outside 
edge of the widgets window, the gap between lines, the gap between the
widget buttons etc.

        @var Dimension internalOffset = 2

@ The month that the calendar is displaying. It defaults to the current month.

	@var int month = 0

@ The year that the calendar is displaying. It defaults to the current year.

	@var int year = 0

@ If this resource is set to True, then the the calendar dates will
appear inside a grid. The default value is False.

	@var Boolean boxed = False

@ If this resource is set to True, then the calendar dates will
appear on 5 lines with any excess dates required beyond the 5th line being
wrapped to the first line. If the resource is set to False, 
no wrapping will take place and 6 lines will
always be used. The default value is False.

	@var Boolean wrapped = False

@ If this resource is True, the widget will have a heading containing the
month and year, as well as four buttons for changing the displayed month.
Setting this resource to False will remove the heading. This allows
the applcation program to provide its own style of heading and/or
month change buttons. The default value is True.

	@var Boolean heading = True

@ The |dateCallback| is called by the |buttonpress| action if the user clicked
on a valid date within the calendar. The |call_data| argument will contain
a pointer to a |XfwfDate| structure which contains the date clicked on.

        @var <Callback> XtCallbackList dateCallback = NULL  

@ The |changeCallback| is called when the date displayed in the calendar is changed
by a mouseclick on one of the buttons or by the |changeMonth| action. 
If the month is changed by a mouseclick on one of the headeing buttons,
the callback will be invoked for each new month displayed.
The |call_data| argument will contain
a pointer to a |XfwfDate| structure which contains the new displayed month.
(The |day| member will be undefined.)

        @var <Callback> XtCallbackList changeCallback = NULL  

@ When the user presses and holds the mouse for longer than
|initialDelay| milliseconds on the calendar buttons, the month is changed
repeatedly every |repeatDelay| milliseconds.

        @var int initialDelay = 500
        @var int repeatDelay = 150



@PRIVATE

@ The bitmaps for the up and down buttons.

	@var Pixmap upBitmap
	@var Pixmap downBitmap 

@ The graphics context for drawing the heading.

	@var GC headingGC

@ The GC for drawing the calendar.

    @var GC calendarGC

@ The GC for drawing the boxes.

    @var GC boxGC

@ The GC for reversing dates.

    @var GC reversingGC

@ A timer

    @var XtIntervalId timer

@ Whether a timer is currently running

    @var Boolean timer_on

@ The current calendar matrix. A value of 0 indicates no entry.

	@var int calmatrix[6][7]

@ How the calendar is moving while a buton is held down.

	@var CalendarMovement movement

@ The date the user clicked on.

	@var XfwfDate click_date

@ The different ways the calendar month might be changinging while a
  button is held down.

	@type CalendarMovement = enum {
	    MonthPlus, MonthMinus, YearPlus, YearMinus }

@EXPORTS

        @type XfwfDate = struct {
            int day;
            int month;
            int year;
        }

	@def XfwfReverseDate = 0x80


@TRANSLATIONS

	@trans <Btn1Down>: buttonpress() 
	@trans <BtnUp>: buttonrelease()
	@trans <Key>Up:                         monthChange("+12")
	@trans <Key>Down:                       monthChange("-12")
	@trans  <Key>Right:                     monthChange("+1")
	@trans  <Key>Left:                      monthChange("-1")

@ACTIONS                                                                       

@ Move the calendar display by the specified number of months.

@proc monthChange
{	int n;
	int mo, yr;

	if (*num_params == 0 || sscanf(params[0], "%d", &n) != 1)
        n = 0;

	mo=$month+n;
	yr=$year;

	while (mo<1) {
		yr--;
		mo += 12;
		}
	while (mo>12) {
		yr++;
		mo -= 12;
		}
	XtVaSetValues($, XtNmonth, mo, XtNyear, yr, NULL);
	$click_date.month = $month;
	$click_date.year = $year;
    XtCallCallbackList($, $changeCallback, (XtPointer) &$click_date);
}


@ If one of the the widget buttons is auto-repeating, the |buttonrelease| 
action stops the auto-repeating.

@proc buttonrelease
{
    if ($timer_on) {
        $timer_on = False;
        XtRemoveTimeOut($timer);
		}
}

@ The |buttonpress| action selects the date under the mouse cursor, or if the
mouse cursor is over a button it will increment or decrement the month or year
as appropriate.

@proc buttonpress
{
    int mx, my, i;
	int mo, yr;
	float f, yspace, yinc;
	int datelines = $wrapped ? 5 : 6;
    int ybase, ma, md, x, y;
	Position wx, wy;
	Dimension ww, wh;

	$compute_inside($, &wx, &wy, &ww, &wh);

	mo=$month;
	yr=$year;
    mx=event->xbutton.x;
    my=event->xbutton.y;
	i=$internalOffset;    

	if (($heading) && ((my-wy) < i+up_height)) {
		if ((mx-wx)<i+up_width) {
			$movement=MonthPlus;
			month_plus($, &mo, &yr);
			}
		else if (((mx-wx)>i+up_width) && ((mx-wx)<2*i+up_width+down_width)) {
			$movement=MonthMinus;
			month_minus($, &mo, &yr);
			}
		else if ((mx>$width-wx-2*i-up_width-down_width) && (mx<$width-wx-i-down_width)) {
			yr++;
			$movement=YearPlus;
			}
		else if (mx>$width-wx-i-down_width) {
			yr--;
			$movement=YearMinus;
			}
		
		if ((mo!=$month) || (yr!=$year)) {
			XtVaSetValues($, XtNmonth, mo, XtNyear, yr, NULL);
			$click_date.month = $month;
			$click_date.year = $year;
	    	XtCallCallbackList($, $changeCallback, (XtPointer) &$click_date);
        	$timer_on = True;
	        $timer = XtAppAddTimeOut(XtWidgetToApplicationContext($),
                                 $initialDelay, timer_callback, $);
			}
		return;
		}

    ma=$calendarFont->ascent;
    md=$calendarFont->descent;

	/* the following equations are derived from the grid drawing
		equations in the expose method */
	yspace = (((float)(wh-heading_height($) - $internalOffset -2 -(datelines+1)*(ma+md))))/(datelines+1);
	yinc = yspace+ma+md;
	ybase = heading_height($)+$internalOffset+yinc;
	f = ((float) (my-wy-ybase)*datelines)/ (wh-1- ybase);
	y = (f>0) ? f : -1;

	f = (((float) mx-wx)*7)/(ww-1);
	x = (f>0) ? f : -1;

	/* printf("(%d,%d)\n", x,y); */

	if ((x<0) || (x>6) || (y<0) || (y>5))
		return;

	if ($calmatrix[y][x] != 0) {
		$click_date.day = $calmatrix[y][x];
		$click_date.month = $month;
		$click_date.year = $year;
	    XtCallCallbackList($, $dateCallback, (XtPointer) &$click_date);
		}
	}

@CLASSVARS
	@var compress_exposure =
		XtExposeCompressMultiple|XtExposeGraphicsExposeMerged


@UTILITIES

	@var char *monthname[] = {
			"January",
			"February",
			"March",
			"April",
			"May",
			"June",
			"July",
			"August",
			"September",
			"October",
			"November",
			"December"
			}

	@var char *dayname[] = {"Su","Mo","Tu","We","Th","Fr","Sa"}

@proc int maximum(int a, int b)
{
	return (a>b) ? a : b;
}

@proc month_plus($, int *mo, int *yr)
{
	*mo=$month;
	*yr=$year;
	(*mo)++;
	while (*mo>12) {
		(*yr)++;
		*mo -= 12;
		}
}

@proc month_minus($, int *mo, int *yr)
{
	*mo=$month;
	*yr=$year;
	(*mo)--;
	while (*mo<1) {
		(*yr)--;
		*mo += 12;
		}
}

@ The |timer_callback| is called when a certain interval passes
without the user releasing the mouse button. 

@proc timer_callback(XtPointer call_data, XtIntervalId *id)
{
    Widget $ = (Widget) call_data;
	int mo, yr;

	switch ($movement) {
		case MonthPlus:
			month_plus($, &mo, &yr);
			break;
		case MonthMinus:
			month_minus($, &mo, &yr);
			break;
		case YearPlus:
			mo=$month;
			yr = $year+1;
			break;
		case YearMinus:
			mo=$month;
			yr = $year-1;
			break;
		default:
			printf("Warning: illegal movement in Calendar.w\n");
			break;
		}

	XtVaSetValues($, XtNmonth, mo, XtNyear, yr, NULL);
	$click_date.month = $month;
	$click_date.year = $year;
   	XtCallCallbackList($, $changeCallback, (XtPointer) &$click_date);

	/* add a new timer */
    $timer = XtAppAddTimeOut(XtWidgetToApplicationContext($),
                             $repeatDelay, timer_callback, $);
}

@proc make_gcs($)
{
	XtGCMask mask;
	XGCValues values;

	values.foreground=$headingColor;
	values.background=$background_pixel;
	values.font=$headingFont->fid;
	mask = GCForeground | GCBackground | GCFont;
	$headingGC=XtGetGC($, mask, &values);

	values.foreground = $boxColor;
	$boxGC=XtGetGC($,mask,&values);

	values.font = $calendarFont->fid;
	values.foreground = $color1;
	$calendarGC=XtGetGC($,mask,&values);

	mask |=  GCFunction;
	values.function =  GXxor;
	$reversingGC=XtGetGC($,mask,&values);
}	


@ Calculate the height of the heading

@proc int heading_height($)
{
	if ($heading)
		return maximum(up_height,$headingFont->ascent + $headingFont->descent)
			+ $internalOffset + 1;
	else
		return 0;
}

@ Calculate the size of the widget based on the text that it has to display.

@proc compute_size($, Dimension *wd, Dimension *ht)
{
	int cw, hw;
	int datelines = $wrapped ? 5 : 6;
	
	cw = XTextWidth($calendarFont,"0",1)*20 + $internalOffset*2;
	hw = XTextWidth($headingFont,"September 0000", 14) +
			2*up_width + 2*down_width + $internalOffset*4;

/*printf("cw=%d, hw=%d\n", cw, hw);*/
	*wd = cw>hw ? cw : hw;
	*ht = heading_height($) + $internalOffset*(datelines+2) + 
			(datelines+1)*($calendarFont->ascent + $calendarFont->descent);
}

@ Returns true if year is a leap year.

@proc Boolean leapyear(int year)
{
	if ((((year%4)==0) && ((year%100) !=0)) || ((year%400)==0))
		return True;
	else
		return False;
}

@proc fill_matrix($)
{
	int i, j;
	XfwfDate dt;
	int day;
	int daysinmonth;
	int datelines = $wrapped ? 5 : 6;

	dt.day=1;
	dt.month=$month;
	dt.year=$year;

	switch ($month) {
		case 1:
		case 3:
		case 5:
		case 7:
		case 8:
		case 10:
		case 12:
			daysinmonth=31;
			break;
		case 4:
		case 6:
		case 9:
		case 11:
			daysinmonth=30;
			break;
		case 2:
			daysinmonth = leapyear($year) ? 29 :28;
			break;
		}
	day = 1 - ((day_number(dt)+1) % 7);
	for (i=0; i<6; i++)
		for (j=0; j<7; j++) {
			if ((day>=1) && (day <= daysinmonth))
				$calmatrix[i%datelines][j] = day;
			else if (i<datelines)
				$calmatrix[i][j] = 0;
			day++;
			}
}

@ The day_number() function was culled and adapted from AA-51 astronomical 
  almanac by Steve Moshier.
 
 		julian.c
 
  This program calculates Julian day number from calendar
  date, and the date and day of the week from Julian day.
 
  The approximate range of dates handled is 4713 B.C. to
  54,078 A.D.  This should be adequate for most applications.
 
  B.C. dates are calculated by extending the Gregorian sequence
  of leap years and century years into the past.  This seems
  the only sensible definition, but I don't know if it is
  the official one.
 
  The month finding algorithm is attributed to Meeus.
 
  - Steve Moshier
 
@proc long day_number(XfwfDate dt)
{
	long y, a, b, c, e, m;
	long J;


	/* The origin should be chosen to be a century year
	 * that is also a leap year.  We pick 4801 B.C.
	 */
	y = dt.year + 4800;
	if( dt.year < 0 ) {
		y += 1;				/* no year 0 */
		}

	/* The following magic arithmetic calculates a sequence
	 * whose successive terms differ by the correct number of
	 * days per calendar month.  It starts at 122 = March; January
	 * and February come after December.
	 */
	m = dt.month;
	if( m <= 2 ) {
		m += 12;
		y -= 1;
		}
	e = (306 * (m+1))/10;

	a = y/100;	/* number of centuries */
	if( dt.year <= 1582L ) {
		if( dt.year == 1582L ) {
			if( dt.month < 10 )
				goto julius;
			if( dt.month > 10)
				goto gregor;
			if( dt.day >= 15 )
				goto gregor;
			}
julius:				/* Julian Calendar assumed*/
		b = -38;
		}
	else { /* -number of century years that are not leap years */
gregor:
		b = (a/4) - a;
		}

	c = (36525 * y)/100; /* Julian calendar years and leap years */

	/* Add up these terms, plus offset from J 0 to 1 Jan 4801 B.C.
	 * Also fudge for the 122 days from the month algorithm.
	 */
	J = b + c + e + dt.day - 32167;

	return J;
	}

@proc get_day_position($, int i, int j, int *x, int *y) {
	float xinc, xspace, yspace;
	float yinc;
	int ybase, ma, md, digit_width;
	int datelines = $wrapped ? 5 : 6;
	Position wx, wy;
	Dimension ww, wh;

	$compute_inside($, &wx, &wy, &ww, &wh);
	ma=$calendarFont->ascent;
	md=$calendarFont->descent;
	ybase = heading_height($) + $internalOffset + ma;
	digit_width = XTextWidth($calendarFont, "0", 1);
	xspace = ((float) (ww - digit_width*14))/7;
	xinc = xspace + digit_width*2;

	yspace = (((float)(wh-heading_height($) - $internalOffset -2 -(datelines+1)*(ma+md))))/(datelines+1);
	yinc = yspace+ma+md;
	
	*x = wx + j*xinc + xspace/2;
	*y = wy + ybase + yinc*(i+1) +yspace/2;
	
	/* adjust downwards a bit since digits don't have descenders */
	*y += 2;
/*	printf("wh=%d, wy=%d, ybase=%d, yinc=%f, yspace=%f, y=%d\n", 
		wh, wy, ybase, yinc, yspace, *y);*/
}

@proc write_date($, int i, int j, int ybase)
{
	int x, y, x1, y1, x2, y2;
	char buf[5];
	int c;
	int datelines = $wrapped ? 5 : 6;
	Position wx, wy;
	Dimension ww, wh;

	$compute_inside($, &wx, &wy, &ww, &wh);

	sprintf(buf, "%2d", $calmatrix[i][j]);
	get_day_position($,i,j, &x, &y);
	if ((c=$dayColors[ $calmatrix[i][j] ]) ==0)
		c = $dayColors[0];

	switch(c & ~XfwfReverseDate) {
		case 2: XSetForeground(XtDisplay($), $calendarGC, $color2);
				XSetForeground(XtDisplay($), $reversingGC, $color2^$background_pixel);
				break;
		case 3: XSetForeground(XtDisplay($), $calendarGC, $color3);
				XSetForeground(XtDisplay($), $reversingGC, $color3^$background_pixel);
				break;
		case 4: XSetForeground(XtDisplay($), $calendarGC, $color4);
				XSetForeground(XtDisplay($), $reversingGC, $color4^$background_pixel);
				break;
		case 5: XSetForeground(XtDisplay($), $calendarGC, $color5);
				XSetForeground(XtDisplay($), $reversingGC, $color5^$background_pixel);
				break;
		default:XSetForeground(XtDisplay($), $calendarGC, $color1);
				XSetForeground(XtDisplay($), $reversingGC, $color1^$background_pixel);
				break;
        }

	/* compute the box */
	x1 = (ww-1)*j/7;
	y1 = ((wh-1 - ybase)*i)/datelines + ybase;
	x2 = (ww-1)*(j+1)/7;
	y2 = ((wh-1 - ybase)*(i+1))/datelines + ybase;


	XClearArea(XtDisplay($), XtWindow($), wx+x1+1, wy+y1+1, x2-x1-1, y2-y1-1, False);

	XDrawString(XtDisplay($),XtWindow($),$calendarGC,
		x, y, buf, strlen(buf));
	if (c&XfwfReverseDate) {
		/* invert the box */
		XFillRectangle(XtDisplay($),XtWindow($),$reversingGC,
			wx+x1+1,wy+y1+1,x2-x1-1, y2-y1-1);
		}
}

@METHODS

@ The |initialize| method sets the private variables to default values.

@proc initialize
{
    time_t ti;
    struct tm *clock;
	int i;
	Position wx, wy;
	Dimension ww, wh;
	int *ip;

	$compute_inside($, &wx, &wy, &ww, &wh);

	/* get the current month */
	if (($month==0) || ($year==0)) {
	    ti=time(NULL);
    	clock=localtime(&ti);
		$month = clock->tm_mon+1;
		$year = clock->tm_year+1900;
		fill_matrix($);
		}
	if ((ww==0) || (wh==0)) {
		compute_size($, &ww, &wh);
		$width = ww + 2*wx;
		$height = wh + 2*wy;
		}
	$calendarGC=NULL;
	$headingGC=NULL;
	$boxGC=NULL;
	make_gcs($);
	$timer_on=False;
	ip = (int*) XtMalloc(32*sizeof(int));
	if ($dayColors==NULL) {
		ip[0]=1;
		for (i=1; i<32; i++)
			ip[i] = 0;
		}
	else
		for (i=0; i<32; i++)
			ip[i] = $dayColors[i];
	$dayColors = ip;
}

@proc realize
{

    #realize($, mask, attributes);
    $upBitmap = XCreateBitmapFromData(XtDisplay($), XtWindow($),
        up_bits, up_width, up_height);
    $downBitmap = XCreateBitmapFromData(XtDisplay($), XtWindow($),
        down_bits, down_width, down_height);
}


@proc destroy
{
	XtReleaseGC($, $headingGC);
	XtReleaseGC($, $calendarGC);
	XtReleaseGC($, $boxGC);
	XtReleaseGC($, $reversingGC);
	XtFree((char*)$dayColors);
}


@ The |set_values| method copies all changed string resources.

@proc set_values
{
	Boolean need_redisplay = False;
	int i;


	if (($old$internalOffset != $internalOffset) ||
		($old$boxed != $boxed) ||
		($old$heading != $heading))
			need_redisplay = True;

	if ($old$headingFont != $headingFont) {
		XSetFont(XtDisplay($), $headingGC, $headingFont->fid);
		need_redisplay = True;
		}

	if ($old$calendarFont != $calendarFont) {
		XSetFont(XtDisplay($), $calendarGC, $calendarFont->fid);
		need_redisplay = True;
		}

	if (($old$month != $month) || 
		($old$wrapped != $wrapped) ||
		($old$year != $year)) {
			fill_matrix($);
			need_redisplay = True;
			}

	if ($old$dayColors != $dayColors) { 
			for (i=0; i<32; i++)
				$old$dayColors[i] = $dayColors[i];
			$dayColors = $old$dayColors;
			need_redisplay = True;
			}

	if ($old$headingColor != $headingColor) {
		XSetForeground(XtDisplay($), $headingGC, $headingColor);
		need_redisplay=True;
		}

	if ($old$boxColor != $boxColor) {
		XSetForeground(XtDisplay($), $boxGC, $boxColor);
		need_redisplay=True;
		}

	if (($old$color1 != $color1) ||
		($old$color2 != $color2) ||
		($old$color3 != $color3) ||
		($old$color4 != $color4) ||
		($old$color5 != $color5) ||
		($old$weekdayColor != $weekdayColor)) 
			need_redisplay=True;
		
/*	if (need_redisplay) {
		compute_size($, &($width), &($height));
		}
*/
	return need_redisplay;
}


@ The |expose| method is called if the widget was overlapped and became
	visible again.  There are some basic checks to see if certain
	chunks need to be displayed. ie: are they above the exposed region or
	below it then they don't need to be displayed.  And if the remainder
	are partially or entirely within the region then they need to be
	displayed.
	
@proc expose
{
	int x, y, a, d;
	char buf[30];
	int ybase, ma, md, i, j;
	Position wx, wy;
	Dimension ww, wh;
	int datelines = $wrapped ? 5 : 6;
	float yspace, yinc;
		
	if(! XtIsRealized($))
		return ;

	if(event == NULL)
	{
		XClearWindow(XtDisplay($),XtWindow($));
		return;
	}

	#expose($,event,region);
	$compute_inside($, &wx, &wy, &ww, &wh);

	/* draw the heading */

	if ($heading) {
		/* draw the up/down buttons */
		x=wx;
		y=wy;
		XCopyPlane(XtDisplay($), $upBitmap, XtWindow($), $headingGC, 0, 0, 
			up_width, up_height, x, y, 1);
		x += $internalOffset + up_width;
		XCopyPlane(XtDisplay($), $downBitmap, XtWindow($), $headingGC, 0, 0, 
			down_width, down_height, x, y, 1);
		x = wx+ww - $internalOffset - up_width - down_width;
		XCopyPlane(XtDisplay($), $upBitmap, XtWindow($), $headingGC, 0, 0, 
			up_width, up_height, x, y, 1);
		x += $internalOffset + up_width;
		XCopyPlane(XtDisplay($), $downBitmap, XtWindow($), $headingGC, 0, 0, 
			down_width, down_height, x, y, 1);

		/* draw the month and year */
		sprintf(buf, "%s %4d", monthname[$month-1], $year);
		x = ($width - XTextWidth($headingFont,buf,strlen(buf)))/2;
		a = $headingFont->ascent;
		d = $headingFont->descent;
		if (up_height-a-d > 0) 
			y =(up_height-d-a)/2 + a;
		else
			y = a;
		XDrawString(XtDisplay($),XtWindow($),$headingGC,
			x, y+wy, buf, strlen(buf));

		/* draw the heading underline */
		y = maximum(up_height, a+d) + $internalOffset + wx;
		XDrawLine(XtDisplay($), XtWindow($), $headingGC, wx, y,
			wx+ww-1, y);
		}


	/* draw the calendar */

	ma=$calendarFont->ascent;
	md=$calendarFont->descent;

	ybase = heading_height($) + $internalOffset + ma;

	/* print the weekday headings */
	XSetForeground(XtDisplay($), $calendarGC, $weekdayColor);
	for (j=0; j<7; j++) {
		get_day_position($,-1,j, &x, &y);
		XDrawString(XtDisplay($),XtWindow($),$calendarGC,
					x, y, dayname[j], strlen(dayname[j]));
				}

	/* print the day numbers */
	yspace = (((float)(wh-heading_height($) - $internalOffset -2 -(datelines+1)*(ma+md))))/(datelines+1);
	yinc = yspace+ma+md;
	ybase = heading_height($)+$internalOffset+yinc;
	for (i=0; i<datelines; i++)
		for (j=0; j<7; j++) 
			if ($calmatrix[i][j]>0) 
				write_date($, i, j, ybase);

	/* draw the grid if needed */
	if ($boxed) {
		/* draw vertical lines */
		for (i=0; i<8; i++) {
			x = (ww-1)*i/7;
			XDrawLine(XtDisplay($), XtWindow($), $boxGC, 
				wx+x, wy+ybase,
				wx+x, wy+wh-1);
			}

		/* draw horizontal lines */
		for (i=0; i<=datelines; i++) {
			y = ((wh-1 - ybase)*i)/datelines + ybase;
			XDrawLine(XtDisplay($), XtWindow($), $boxGC, 
				wx, wy+y,
				wx+ww-1, wy+y);
			}
		}
	}

@EXPORTS

@ This function sets the colors of each date in the calendar.  It expects an
array of 32 integers where elements 1-31 give the color (1-5) for that day.
Element 0 contains the default color.  Using this convenience function is
preferable to using a call to XtVaSetValues, since this function will only
redraw any changes, whereas using XtVaSetValues causes the entire widget to
be redrawn.

@proc XfwfSetDayColors($, int colors[]) {
	int i, j, day;
	float yspace, yinc;
	int ybase;
	int datelines = $wrapped ? 5 : 6;
	Position wx, wy;
	Dimension ww, wh;
	int ma, md;

	$compute_inside($, &wx, &wy, &ww, &wh);

	ma=$calendarFont->ascent;
	md=$calendarFont->descent;
	yspace = (((float)(wh-heading_height($) - $internalOffset -2 -(datelines+1)*(ma+md))))/(datelines+1);
	yinc = yspace+ma+md;
	ybase = heading_height($)+$internalOffset+yinc;

	/* print the day numbers */
	for (i=0; i<datelines; i++)
		for (j=0; j<7; j++) {
			day=$calmatrix[i][j];
			if ((day>0) && (colors[day] != $dayColors[day])) {
				$dayColors[day] = colors[day];
				write_date($, i, j, ybase);
				}
			}		
	}

@IMPORTS

@incl <assert.h>
@incl <ctype.h>
@incl <stdio.h>
@incl <string.h>
@incl <time.h>

@incl "up.bm"
@incl "down.bm"
