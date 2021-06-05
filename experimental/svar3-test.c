#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <time.h>
#include "mls.h"

#define SVAR_TABLE_SIZE 0x3ff;
#define SVAR_MAX 32

char *mstr(int k) { return m_buf(k); }

int s_strcpy(int dst, int src, int max)
{
    if( m_bufsize(dst) < max ) {
	m_setlen(dst,max);
    }

    int c = Min(max,m_len(src));
    memcpy( m_buf(dst), m_buf(src), c-1 );
    m_setlen( dst, c );
    CHAR(dst,c-1)=0;

    return c;
}




// Dedicated to Pippip, the main character in the 'Das Totenschiff' roman, actually the B.Traven himself, his real name was Hermann Albert Otto Maksymilian Feige.
// CAUTION: Add 8 more bytes to the buffer being hashed, usually malloc(...+8) - to prevent out of boundary reads!
// Many thanks go to Yurii 'Hordi' Hordiienko, he lessened with 3 instructions the original 'Pippip', thus:
#define _PADr_KAZE(x, n) ( ((x) << (n))>>(n) )
static uint32_t FNV1A_Pippip_Yurii(const char *str, size_t wrdlen)
{
	const uint32_t PRIME = 591798841; uint32_t hash32; uint64_t hash64 = 14695981039346656037UL;
	size_t Cycles, NDhead;
	if (wrdlen > 8) {
		Cycles = ((wrdlen - 1)>>4) + 1; NDhead = wrdlen - (Cycles<<3);

        	for(; Cycles--; str += 8) {
			hash64 = ( hash64 ^ (*(uint64_t *)(str)) ) * PRIME;
			hash64 = ( hash64 ^ (*(uint64_t *)(str+NDhead)) ) * PRIME;
		}
	} else {
		hash64 = ( hash64 ^ _PADr_KAZE(*(uint64_t *)(str+0), (8-wrdlen)<<3) ) * PRIME;
	}
	hash32 = (uint32_t)(hash64 ^ (hash64>>32));
	hash32 ^= (hash32 >> 16);
	return hash32 & SVAR_TABLE_SIZE;
} // Last update: 2019-Oct-30, 14 C lines strong, Kaze.


static uint32_t svar_hash(int sbuf)
{
    return FNV1A_Pippip_Yurii( m_buf(sbuf), m_len(sbuf) );
}


/* 
   lookup:
   returns hash of key
   example key = atest,btest,ctest
   loop:
     append(x , get key until dot )
     h = hash(x)
     if( seperator is zero ) return h
     skip separator
     write h to x
     goto loop

 */

static int seperate(int dst, int src, int *start)
{
    while(1) {
	char ch = CHAR(src,*start);
	if( ch == ',' || ch == 0 ) return ch;
	m_putc(dst,ch);
	(*start)++;
    }
}

static int parse_var(int buf)
{
    int ch;
    int h;
    int i=0;
    int key = m_create(SVAR_MAX, 1);

    while(1) {
	ch = seperate(key, buf, & i );
	h = svar_hash(key);

	printf("%04x %s\n", h, mstr(key) );

	if( ch == 0 ) break;
	i++;
	s_printf(key,0,"%04x", h);	
	memset(m_buf(key)+4,0,SVAR_MAX-4);
	m_setlen(key,4);
    }
    m_free(key);
    return h;
}


int main(void)
{
    m_init();
    printf("hello\n");
    int b1 = m_create(1,1);
    int b2 = s_printf(0,0, "test,Atest,Btest,Ctest" );
    s_strcpy(b1,b2, SVAR_MAX );
    puts( mstr(b1) );
    parse_var( b2 );
    m_free(b1);
    m_free(b2);
    m_destruct();
}
