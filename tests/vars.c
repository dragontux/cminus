int globe;
int n;

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

int bar( int n ){
	globe = 2 + 3;

	return 0;
}

int main( ){
	int foo;

	foo = 2 - 2;

	if ( foo ){
		return 100 + sub( 100, 50 ) - 100;

	} else {
		bar( 2 );
		return globe;
	}
}
