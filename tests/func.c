int a;
int b;
int func( int test, char this ){
	while ( test < this ){
		test = test + 1;
		this = this + test - 1;
	}

	return 1;
}

int woot( int a ){
	return 0;
}
