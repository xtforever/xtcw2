%{
#include <stdio.h>
#include "mls.h"
#include "njson_read.h"
#define NJSONSTYPE char*  

  int yylex(void);    
  void yyerror(const char *str);
  extern int njsonlineno;
  
%}

%define api.prefix {njson}
%token true false null
%left O_BEGIN O_END A_BEGIN A_END
%left COMMA
%left COLON

%token  IDENT
%token  NUMBER 
%token  STRING
%token  UNEXPECTED

%%
START: OBJECT ;

OBJECT: O_BEGIN O_END { njson_new("",5); njson_close(); }
| O_BEGIN       { TRACE(3,"OPEN"); njson_new("",5); }
  MEMBERS O_END { TRACE(3,"CLOSE"); njson_close(); }
;

MEMBER: VALUE | PAIR

MEMBERS: MEMBER | MEMBER COMMA MEMBERS
;

PAIR: IDENT VALUE { TRACE(3,"NAME:%s", $1); njson_name($1); }
;

VALUE: STRING { njson_new(yylval, 0);   }
| NUMBER      { njson_new(yylval, 1);   }
| OBJECT      
| true        { njson_new("true",  2);  }
| false       { njson_new("false", 2);  }
| null        { njson_new("null",  3);  }
;
%%

       
void yyerror(const char *str)
{
  printf("ERROR: '%s' in Line:%d\n",str,njsonlineno );
}
      

