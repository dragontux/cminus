int globe;
int n;
extern getchar( );
extern putchar( int n );

int add( int a, int b ){
	return a + b;
}

int sub( int a, int b ){
	int i;
	int j;

	i = b;
	j = a;

	i = i + 1;
	j = j + 2;

	return i - j;
}

int fact( int n ){
	putchar( 70 );
	putchar( 10 );

	if ( n ) return fact( n - 1 ) * n;
	else     return 1;
}

int bar( int n ){
	globe = fact( 5 ) / n;

	return 0;
}

int meh( ){
	int i;

	i = 0;

	while ( i < 10 ){
		putchar( 109 );
		putchar( 10 );

		i = i + 1;
	}

	return 0;
}

int main( int argc ){
	int foo;
	int i;

	foo = 2 - 2;
	i = meh( 0 );

	if ( 1 ){
		putchar( getchar( ));

	} else {
		bar( 2 );
		return globe;
	}

	return argc;
}
