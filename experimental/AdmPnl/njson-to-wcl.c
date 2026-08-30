#include "mls.h"
#include "njson_read.h"
#include "var5.h"
#include "m_tool.h"

int EMPTY_MSTR  = -1;

static void mvar_assign(int mstr )
{
    int ls =  m_split_list( m_str(mstr), "=" );
    if( m_len(ls) != 2 ) goto leave;
    int v = mvar_parse( INT(ls,0), VAR_STRING );
    mvar_put_string(v, m_str(INT(ls,1)), 0);

 leave:
    m_free_list(ls);
}


int njson_read( char *inf )
{
   FILE *fp;
   ASSERT( (fp=fopen( inf, "r" )));
   int opts = njson_from_file( fp );
   fclose(fp);
   return opts;
}

int njson_cmp(char *name, struct njson_st *j)
{
  if( j->name ) return mstrcmp(j->name,0,name);
  return 1;
}

struct njson_st *njson_find_data(int opts, char *name)
{
  int p;
  struct njson_st *j;
  m_foreach(opts,p,j) {
    if( !njson_cmp(name,j) ) return j;
    if( j->typ >= NJSON_ARR ) {
      j=njson_find_data(j->d,name);
      if( j ) return j;
    }
  }
  return NULL;
}

int njson_find_obj(int opts, char *name)
{
    struct njson_st *j = njson_find_data(opts,name);
    if(j && j->typ >= NJSON_ARR ) {
	return j->d;
    }
    return 0;
}

int njson_get_entry(int field, char *name )
{
    struct njson_st *j;
    j = njson_find_data( field, name );
    if( !j || j->typ >= NJSON_ARR ) return EMPTY_MSTR;
    return j->d;
}


char *header_part1=
" admpanel.WcChildren: gb1                  \n"
" admpanel.prefix: $PREFIX		    \n"
" admpanel.initscript: $INITSCRIPT \n"
  " *gb1.WcClass: gridbox			    \n"
" *gb1.WcChildren: header values footer	    \n"
" *header.gridy: 0			    \n"
" *values.gridy: 1			    \n"
" *footer.gridy: 2			    \n"
" *WeditMV.foreground:    #008c7c	    \n"
" *WeditMV.cursorColor:   #95a5a6	    \n"
" *background:            #ecf0f1	    \n"
" *WeditMV.fontName: Source Code Pro-12	    \n"
" *header.wcClass: Wlabel		    \n"
" *header.label:   $TITLE		    \n"
" *header.weightx: 100			    \n"
" *header.weighty: 1			    \n"
" *header.fill: Width                       \n"
" *values.weightx: 100			    \n"
" *values.weighty: 100			    \n"
" *values.fill: Both                        \n"
"*cmd.wcClass: Wexecgui                                                \n"
"*cmd.shell_cmd: $SCRIPT                                               \n"
"*cmd.prefix: $PREFIX						       \n"
"*cmd.bg_norm: Black						       \n"
"*cmd.bg_sel: Black						       \n"
"*cmd.fg_norm: Green						       \n"
"*cmd.fg_sel: Green						       \n"
"								       \n"
"*footer.wcClass: Gridbox					       \n"
"*footer.wcChildren: cmd exec quit				       \n"
"								       \n"
"*exec.gridx: 0							       \n"
"*quit.gridx: 1							       \n"
"*exec.gridy: 1							       \n"
"*quit.gridy: 1							       \n"
"*cmd.gridWidth: 2						       \n"
"								       \n"
"% if cmd resizes exec end quit will be repositioned in the new grid   \n"
"*cmd.fill: Both						       \n"
"*cmd.weightx: 100						       \n"
"*cmd.weighty: 100						       \n"
"								       \n"
"								       \n"
"*exec.wcClass: Wbutton						       \n"
"*exec.label:   Execute 					       \n"
"	       							       \n"
"*quit.wcClass: Wbutton						       \n"
"*quit.label: Quit 						       \n"
"*quit.gravity: east						       \n"
"% *quit.fill: none						       \n"
"*quit.callback: WcQuit						       \n"
    "                                                                      \n";



struct str_dispatch_st {
    char *name;
    int (*func) ( int what, int where, void *ctx );
};

int str_dispatch( int mstr, struct str_dispatch_st *kw, int what, int where, void *fp )
{
    for( int i=0; i < 1000 && kw[i].name; i++ ) {
	if( mstrcmp(mstr,0,kw[i].name) == 0 ) {
	    return kw[i].func(what,where,fp);
	}
    }
    return -1;
}


//static int  print_dummy( int field, int w, void *fp )
//{
//   puts("dummy");
//  return 0;
//}


void njson_array_to_mvar( int lst, char *mvars )
{
    int mvar = s_printf(0,0,mvars);
    int id = mvar_parse( mvar, VAR_STRING );
    m_free(mvar);
    mvar_clear(id);

    int len = mvar_length( id );
    TRACE(4,"current length: %d", len );

    struct njson_st *j;
    int p;
    m_foreach( lst, p, j ) {
	mvar_put_string(id, m_str( j->d ), p);
    }


    len = mvar_length( id );
    TRACE(4,"current length: %d", len );

}

void mvars_put_string( char *name, char *str, int row )
{
    int m = s_printf(0,0,"%s", name );
    int v = mvar_parse( m, VAR_STRING );
    mvar_put_string(v, str, row );
    m_free(m);
}

int m_slice( int m, int p, int src, int a, int len )
{
    if( !m ) m=m_create(100,1);
    m_setlen(m,p);
    while( len-- ) {
	void *x = mls(src,a++);
	m_put( m, x );
    }
    return m;
}

int mvars_assign_mp_va( int m, int p, char *format, va_list ap )
{
    char *d; int n;
    int mv;
    m = vas_printf( m,p, format, ap );
    m_foreach(m,n,d) {
	if( *d == '=' ) {
	    int var = m_slice( 0,0, m,0, n );
	    m_putc(var,0);
	    mv = mvar_parse(var,VAR_STRING);
	    mvar_put_string(mv, (char*)mls(m,n+1), 0 );
	    m_free(var);

	    m_free(m);
	    return mv;
	}
    }
    mv = mvar_parse(m,VAR_STRING);
    m_free(m);
    return mv;
}


int mvars_assign( char *format, ... )
{
    va_list ap;
    va_start(ap,format);
    int mv = mvars_assign_mp_va(0,0,format,ap);
    va_end(ap);
    return mv;
}

int mvars_assign_mp( int m, int p, char *format, ... )
{
    va_list ap;
    va_start(ap,format);
    int mv=mvars_assign_mp_va( m,p,format, ap );
    va_end(ap);
    return mv;
}

static void print_resource( const char *s, FILE *fp )
{
    int buf=m_create(100,1);
    int ch=1; const char *d=s;
    int p=-1;

    while(ch) {
	ch = *d++;
	if( ch == '\t' ) ch=32;

	if( ch == '\n' || ch==0 ) {
	    fwrite( m_buf(buf), p>=0 ? p : m_len(buf), 1, fp );
	    fprintf(fp, "\n" );
	    m_clear(buf);
	    p=-1;
	    continue;
	}

	if( isspace(ch) ) {
	    if( p < 0 ) p=m_len(buf); // first seen space
	} else p=-1;

	m_putc(buf, ch);
    }
    m_free(buf);
}

static int print_widget(char *pattern, int opts, int row, void *fp )
{

    int title =  njson_get_entry(opts,"title");
    int var   =  njson_get_entry(opts,"variable");
    mvars_assign( "opt.w0=*l%d0", row );
    mvars_assign( "opt.w1=*l%d1", row );
    mvars_assign( "opt.varname=task1.%s", m_str(var) );
    mvars_assign( "opt.title=%s", m_str(title) );

    const char *s = mvar_str_string( "opt", pattern );
    print_resource(s,fp);

    return 0;
}


static void print_extra_vars(int opts, int row, void *fp )
{
    int lst = njson_find_obj( opts, "extra_vars" );
    if(!lst) return;
    struct njson_st *j;
    int p;
    m_foreach( lst, p, j ) {
	char *name = m_str( j->name );
	char *val  = m_str( j->d    );
	fprintf(fp,"*l%d1.%s: %s\n", row, name, val );
    }
}

static int  print_input( int opts, int row, void *fp )
{
    char *part=
	"$w0.wcClass: Wlabel				\n"
	"$w0.label  : $title  				\n"
	"$w1.wcClass: Weditmv 				\n"
	"$w1.label: 					\n"
	"$w1.onChanged: CallAction( *cmd UpdateCmd )	\n"
	"$w1.weightx: 1000				\n"
	"$w1.var5: $varname				\n";


    return print_widget( part, opts, row, fp );
}


static int  print_kuerzel( int opts, int row, void *fp )
{

    char *part=
	"$w0.wcClass: Wlabel					\n"
	"$w0.label  : $title					\n"
	"$w1.wcClass: Wsqlcombo					\n"
	"$w1.sqlcombo_cb:  CallAction( *cmd UpdateCmd )		\n"
	"$w1.sql_result: $varname				\n";

    return print_widget( part, opts, row, fp );
}

static int  print_query( int opts, int row, void *fp )
{

    char *part=
	"$w0.wcClass: Wlabel					\n"
	"$w0.label  : $title					\n"
	"$w1.wcClass: Wsqlcombo					\n"
	"$w1.sqlcombo_cb:  CallAction( *cmd UpdateCmd )		\n"
	"$w1.sql_result: $varname				\n";

    return print_widget( part, opts, row, fp );
}



static int  print_option( int opts, int row, void *fp )
{
    char *part=
    "$w0.wcClass:  Wlabel                         \n"
    "$w0.label  :  $title  		          \n"
    "$w1.wcClass:  radio2 		          \n"
    "$w1.lst:      $optlist[*]		          \n"
    "$w1.value:    $optlist[0]		          \n"
    "$w1.callback: CallAction( *cmd UpdateCmd )   \n"
    "$w1.var5:     $varname                       \n";

    int title =  njson_get_entry(opts,"title");
    int var   =  njson_get_entry(opts,"variable");

    int vn;
    vn = s_printf(0,0, "opt.w0=*l%d0", row );
    mvar_assign(vn);
    s_printf(vn,0, "opt.w1=*l%d1", row );
    mvar_assign(vn);
    s_printf(vn,0, "opt.varname=task1.%s", m_str(var) );
    mvar_assign(vn);

    mvars_put_string( "opt.title", m_str(title), 0 );

    int val = njson_find_obj( opts, "values"  );
    njson_array_to_mvar(val, "opt.optlist" );

    const char *s = mvar_str_string( "opt", part );
    print_resource(s,fp);

    m_free(vn);
    return 0;
}


static void print_field( int field, int row, FILE *fp )
{
    int t = njson_get_entry(field,"type");

    TRACE(4, "type: %s", m_str(t) );

    struct str_dispatch_st kw[] = {
	{ "kuerzel", print_kuerzel },
	{ "input", print_input    },
	{ "option",  print_option },
	{ "query", print_query    },
	{ NULL, NULL } };

    str_dispatch( t, kw, field, row, fp );

    print_extra_vars( field, row, fp );

}


static void create_layout(int rows, FILE *fp )
{
    fprintf(fp, "*values.WcClass: Gridbox\n*values.wcChildren: " );
    for(int i=0;i<rows;i++) fprintf(fp, "l%d0 l%d1 ", i,i);
    fprintf(fp, "\n*l01.weightx: 1000\n*l00.weightx: 100\n*l01.fill: Width\n" );
    for(int i=0;i<rows;i++) fprintf(fp,
				    "*l%d0.gridx: 0\n*l%d0.gridy: %d\n"
				    "*l%d1.gridx: 1\n*l%d1.gridy: %d\n"
				    "*l%d1.onEnter: SetFocus(*l%d1)\n", i,i,i, i,i,i,    i,i+1);
}


static void fill_values( int fields, FILE *fp )
{
    int p;
    create_layout( m_len(fields), fp );

    /* 'fields' contains objects with (type,...) */
    struct njson_st *j;
    m_foreach(fields,p,j) {
	print_field( j->d, p, fp );
    }

}



static void print_header(int title,int script,int initscript, FILE *fp)
{
    int var;
    /* define some variables with prefix gg */
    var = s_printf(0,0, "gg.TITLE=%s", m_str(title) );
    mvar_assign( var );
    s_printf(var,0, "gg.SCRIPT=%s", m_str(script) );
    mvar_assign( var );
    s_printf(var,0, "gg.PREFIX=task1" );
    mvar_assign( var );
    s_printf(var,0, "gg.INITSCRIPT=%s", m_str(initscript) );
    mvar_assign( var );

    /* use variables with prefix gg to replace strings in header_part1 */
    const char *s = mvar_str_string( "gg", header_part1 );
    print_resource(s,fp);

    m_free(var);
}


int main(int argc, char **argv)
{
    FILE *fp = stdout;
    trace_level=4;
    m_init();
    mvar_init();
    EMPTY_MSTR = m_create(1,1);
    m_putc( EMPTY_MSTR, 0 );

    TRACE(1,"start");
    char *fn = "/dev/stdin" ;
    if( argc > 1 ) fn =  argv[1];

    int opts =   njson_read( fn );
    int title =  njson_get_entry(opts,"title");
    int script = njson_get_entry(opts,"script");
    int fields = njson_find_obj(opts,"fields");
    int initscript =  njson_get_entry(opts,"init");

    print_header( title, script, initscript, fp );
    fill_values( fields, fp );
    njson_free( opts );

    m_free(EMPTY_MSTR);
    mvar_destruct();
    m_destruct();  return EXIT_SUCCESS;
}
