#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <pthread.h>

FILE *infile; 

/*Dictionary struct holds shared objects.*/

typedef struct dict {
	char *word;
	int count;
	struct dict *next;
} dict_t;

dict_t* wd;

/*Makes a nessecary word used in dict struct*/
char *
make_word( char *word ) {
	return strcpy( malloc( strlen( word )+1 ), word );
}

/*Takes in a word and creates a dict*/
dict_t *
make_dict(char *word) {
	dict_t *nd = (dict_t *) malloc( sizeof(dict_t) );
	nd->word   = make_word( word );
	nd->count  = 1;
	nd->next   = NULL;
	return nd;
}

/*Inserts a work into a dict type*/
void
insert_word(char* word) {
	// Insert word into the main dict or increment count if already there
	// return pointer to the updated dict
	dict_t *nd;
	dict_t *pd = NULL; // prior to insertion point
	dict_t *di = wd;   // following insertion point
	
	// Search down list to find if present or point of insertion
	while(di && ( strcmp(word, di->word ) >= 0) ) {

		if( strcmp( word, di->word ) == 0 ) {
			di->count++; // increment count
			return;      // return
		}

	pd = di; // advance ptr pair
	di = di->next;
	}

	nd       = make_dict(word); // not found, make entry
	nd->next = di;              // entry bigger than word or tail

	if (pd) {
		pd->next = nd;
		return;
	}
	
	wd = nd;
	return;
}

/*Prints out dict*/
void print_dict(void) {
	while (wd) {
		printf("[%d] %s\n", wd->count, wd->word);
		wd = wd->next;
	}

}

/*Runs through a file a gets the word within the textfile*/
int
get_word( char *buf) {
	int inword = 0;
	int c;
	
	while( (c = fgetc(infile)) != EOF ) {
		
		if (inword && !isalpha(c)) {
			buf[inword] = '\0'; // terminate the word string
			return 1;
		}
	
		if (isalpha(c)) {
			buf[inword++] = c;
		}

	}
	
	return 0; // no more words

}

#define MAXWORD 1024

pthread_mutex_t flagLock;
pthread_mutex_t flagMutex;

/*Runs through a file and continues to insert_word while get_word is working*/
void*
thread_stuff(void* arg) {
	char word[MAXWORD];
	int okgo = 1;

	while (okgo) {
		pthread_mutex_lock(&flagLock);
		okgo = get_word(word);
		pthread_mutex_unlock(&flagLock);
			
		if (okgo == 0) break;

		pthread_mutex_lock(&flagMutex);
		insert_word(word);
		pthread_mutex_unlock(&flagMutex);
	}

	pthread_exit(NULL);	

}

#define NOTHREADS 4

/*Sorting the threads*/
void
words() {
	wd = NULL;
	pthread_t threads[NOTHREADS];
	pthread_attr_t attr;

	int t_ret;
	
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
	pthread_mutex_init(&flagMutex, NULL);
	pthread_mutex_init(&flagLock, NULL);
	
	for ( int i = 0; i < NOTHREADS; i++) {
		t_ret = pthread_create(&threads[i], &attr,thread_stuff, NULL);
		
		if (t_ret) {
			exit(-1);
		}
	
	}

  //Waiting for other threads
	for( int j = 0; j < NOTHREADS; j++) {
		pthread_join(threads[i], NULL);
	}

	pthread_mutex_destroy(&flagMutex);
	pthread_mutex_destroy(&flagLock);
	pthread_attr_destroy(&attr);
}

int
main( int argc, char *argv[] ) {
	wd = NULL;
	infile = stdin;
	
	if (argc >= 2) {
		infile = fopen (argv[1], "r");
	}

	if( !infile ) {
		printf("Unable to open %s\n", argv[1]);
		exit( EXIT_FAILURE );
	}

	words();	
	print_dict();
	fclose( infile );
}
