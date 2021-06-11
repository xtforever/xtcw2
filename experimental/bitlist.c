
uint8_t *bitlist_byte(int m,int p)
{
    int len = m_len(m);
    if( p >= len ) {
	m_setlen( m, p+1 );
	memset( mls(m,len), 0, p - len + 1 );
    }
    return mls(m,p);
}

int  bitlist_test( int m, int p )
{
    int      offs  =  p/8;
    uint8_t  bit   =  p & 8;
    uint8_t  mask  =  ( 1 << bit );
    uint8_t *b     = bitlist_byte( m, offs );
    return  *b & mask;
}

void bitlist_set( int m, int p )
{
    int      offs  =  p/8;
    uint8_t  bit   =  p & 8;
    uint8_t  mask  =  ( 1 << bit );
    uint8_t *b     = bitlist_byte( m, offs );
    *b |= mask;
}

void bitlist_clr( int m, int p )
{
    int      offs  =  p/8;
    uint8_t  bit   =  p & 8;
    uint8_t  mask  =  ( 1 << bit );
    uint8_t *b     = bitlist_byte( m, offs );
    *b &= ~ mask;
}

int bitlist_find_zero( int m )
{
    uint8_t ch;
    int len = m_len(m);
    while( len-- ) {
	ch = UCHAR(m,len);
	if( ch == 0xff ) continue;
	uint8_t bit = 7;
	while( bit && (ch & (1 << bit)) ) bit--;
	return len*8+bit;
    }
    return -1;
}

int bitlist_find_one( int m )
{
    uint8_t ch;
    int len = m_len(m);
    while( len-- ) {
	ch = UCHAR(m,len);
	if( ch == 0 ) continue;
	uint8_t bit = 7;
	while( bit && ! (ch & (1 << bit)) ) bit--;
	return len*8+bit;
    }
    return -1;
}
