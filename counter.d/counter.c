



/*	P-5 Submission for Geoffrey Ringlee.
	The main thread will read in lines and pass them to a buffer.
	Consumer threads will read from the buffer lines, and parse 
	lines into words, and insert each word into a list of read 
	odd or even words, respectively. Afterwards, the main thread
	will read from the lists the words, and numeber of occurances
	and print them to stdout.					*/

#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <stdlib.h>

void insert_word_even(char*);
void insert_word_odd(char*);

int debug = 0; /*for debugging*/
int numConsumers = 0;
char consumerName = 'a';
int consumerMax = 0;
int bufferSize = 0;
int single = 0;
char * consumerNames;

pthread_mutex_t lockOdd;
pthread_mutex_t lockEven;
struct timespec sleepConsumer, sleepConsumer2, sleepProducer, sleepProducer2;

struct Buffer{
	pthread_mutex_t lock; 	/*mutex ensuring exclusive access to 
					lineBuffer */
	int readpos, writepos;	/*positions for reading and writing 
							into buffer */
	pthread_cond_t notempty;	/*signaled when buffer is not empty*/
	pthread_cond_t notfull; 	/*signaled when buffer is not full*/
	int done, singleFull;
	char** buffer; 		/*pointer to array of char strings*/
};

struct Buffer lineBuffer;

typedef struct word{

	char* word;
	int count;
	struct word * next;
}word_t;

word_t *oddHead;
word_t *evenHead;
int single;

void sleeper(struct timespec sleep){
	if(debug){
		fprintf(stderr, "sleeping: %ld\n", sleep.tv_nsec);
	}
	struct timespec temp;
	temp.tv_sec = sleep.tv_sec;
	temp.tv_nsec = sleep.tv_nsec;
	while(nanosleep(&temp, &temp) == -1){
		if(debug){
			fprintf(stderr, "sleep %ld\n", temp.tv_nsec);
		}	
	}
	
}


static char* get_line_single(struct Buffer *b){
	if(debug){
		fprintf(stderr, "in get_line_single\n");
	}
	char* line;
	pthread_mutex_lock(&b->lock);
	if(b->done && b->singleFull ==0){
		pthread_mutex_unlock(&b->lock);
		return NULL;
	}
	/*wait until buffer is not empty */
	while(b->singleFull == 0){
		if(debug){
			fprintf(stderr, "singleEmpty\n");
		}
		pthread_cond_wait(&b->notempty, &b->lock);
		if(b->singleFull == 0 && b->done){
			pthread_mutex_unlock(&b->lock);
			return NULL;
		}
	}
	/*read the data and change flag*/
	line = b->buffer[0];
	b->buffer[0] = NULL;
	b->singleFull = 0;
	/*signla that the buffer is now not full */
	pthread_cond_signal (&b->notfull);
	pthread_mutex_unlock(&b->lock);
	return line;
}

/*read line from buffer*/
static char* get_line(struct Buffer *b){
	if(debug){
		fprintf(stderr, "in get_line\n");
	}
	char* line;
	pthread_mutex_lock(&b->lock);

	//check to make sure not done
	if(b->writepos == b->readpos && b->done){
		pthread_mutex_unlock(&b->lock);
		return NULL;
	}
	/*wait until buffer is not empty*/
	while(b->writepos == b->readpos ){

		pthread_cond_wait(&b->notempty, &b->lock);

		if(b->writepos == b->readpos && b->done){
			pthread_mutex_unlock(&b->lock);
			return NULL;
		}
		
	}
	/*read data if note finished, else continue*/
	line = b->buffer[b->readpos];
	b->buffer[b->readpos] = NULL;
	b->readpos++;
	if(b->readpos >= bufferSize){
		b->readpos = 0;
	}
	/*Signal that the buffer is now not full*/
	pthread_cond_signal (&b->notfull);
	pthread_mutex_unlock(&b->lock);
	return line;
	
}



static void* consumer (void *data){

	char* line;
	char *delimiter = " \t\f\n\r\v";
	char* word, *context;
	int done = 0;
	char name = *((char*)data);

	if(debug){
		fprintf(stderr, "consumer created: name: %c, data:%s\n", name,
				 (char *) data);
	}

	
	while(!done){

		if(single){
			line = get_line_single(&lineBuffer);
		}else{
			line = get_line(&lineBuffer);
			if(debug){
				fprintf(stderr, "got line\n");
			}
		}
		sleeper(sleepConsumer);
		if(line == NULL){
			done = 1;
		
		}else{
			int i = 0;
			word = strtok_r(line, delimiter, &context);
			while(word != NULL){
				if(strlen(word) %2 == 0){
	
					insert_word_even(word);
				}else{
					insert_word_odd(word);
				}
				fprintf(stderr, "%c,", name);
				if(debug){
					fprintf(stderr, "%c:itter%d\n", name, i);
				}
				word = strtok_r(NULL, delimiter, &context);
				i++;
			}
		
		}

	}
	return NULL;

}



void create_counter(pthread_t * consumers){
	if(debug){
		fprintf(stderr, "in create_counter\n");
	}
	consumerNames[numConsumers] = 'a' + numConsumers;
	if(numConsumers<consumerMax){
		pthread_create(&consumers[numConsumers], NULL, consumer, 
				&consumerNames[numConsumers]);
		numConsumers++;
		if(debug){
			fprintf(stderr, "thread created, name: %c\n", 
					consumerNames[numConsumers]);
		}
		
	}

}



/* 	Store a line in the buffer.
	If the buffer is full, create a new counter thread. 
	line must be null terminated.*/
static void put_line(struct Buffer *b, char* line, pthread_t* consumers){
	if(debug){
		fprintf(stderr, "in put_line\n");
	}
	char* lineTemp;
	pthread_mutex_lock (&b->lock);
	/*wait until buffer is not full */
	while((b->writepos+1)%bufferSize == b->readpos){

		create_counter(consumers); /*creates new counter thread*/
		pthread_cond_wait(&b->notfull, &b->lock);
		/*waits until no longer full, reaquired lock before 
			returning*/
	}
	/*Write the data and advance write pointer*/
	lineTemp = (char *) calloc(strlen(line)+1, sizeof(char));
	strcpy(lineTemp, line);
	b->buffer[b->writepos] = lineTemp;
	b->writepos++;
	if(b->writepos >= bufferSize){
		b->writepos = 0;
	}
	/*signal that the buffer is now not empty*/
	pthread_cond_signal (&b->notempty);
	pthread_mutex_unlock(&b->lock);

	
}


static void put_line_single(struct Buffer *b, char* line, pthread_t * consumers){
	if(debug){
		fprintf(stderr, "in put_line_single\n");
	}

	char* lineTemp;
	pthread_mutex_lock (&b->lock);
	/*wait until buffer is not full, if full, create new reader */
	while(b->singleFull == 1){
		if(debug){
			fprintf(stderr, "singleFull\n");
		}
		create_counter(consumers);
		pthread_cond_wait(&b->notfull, &b->lock);
	}
	/*write the data and alter full flag*/
	lineTemp = (char *) calloc(strlen(line)+1, sizeof(char));
	strcpy(lineTemp, line);
	b->buffer[0] = lineTemp;
	b->singleFull =1;
	pthread_cond_signal(&b->notempty);
	pthread_mutex_unlock(&b->lock);
	if(debug){
		fprintf(stderr, "exiting put_line_single\n");	
	}
}



void insert_word_even(char* word){
	if(debug){
		fprintf(stderr, "in insert_word_even\n");
	}

	int found = 0;
	word_t* temp;
	pthread_mutex_lock(&lockEven);
	word_t * current = evenHead;
	while(current != NULL && !found){
		if(debug){
			fprintf(stderr, "itteration:search = %s, word = %s\n", 
					word, current->word);
		}
		if(strcmp(current->word, word)==0){/*found*/
			current->count ++;
			found = 1;
		}else if(strcmp(current->word, word)<0){/*before word*/
			current = current->next;
		}else{/*passed slot*/
			found = 1;
			temp = (word_t *) calloc(1, sizeof(word_t));
			temp->word = current->word;
			temp->next = current->next;
			temp->count = current->count;
			
			current->word = word;
			current->next = temp;
			current->count = 1;
		}
		
	}
	if(current == NULL){
			
		current = evenHead;
		if(current != NULL){//at end of chain

			while(current->next != NULL){
				current = current->next;
			}
		
			temp = calloc(1, sizeof(word_t));
			temp->word = word;
			temp->next = NULL;
			temp->count = 1;
			current->next = temp;

		}else{//at new chain

			temp = calloc(1, sizeof(word_t));
			temp->word = word;
			temp->next=NULL;
			temp->count = 1;
			temp->next = NULL;
			evenHead = temp;

		}
	}
	if(debug){
		fprintf(stderr, "inserted\n");
	}
	pthread_mutex_unlock(&lockEven);
}



void insert_word_odd(char* word){
	if(debug){
		fprintf(stderr, "in insert_word_odd\n");
	}
	int found = 0;
	word_t* temp;
	pthread_mutex_lock(&lockOdd);
	word_t * current = oddHead;
	while(current != NULL && !found){
		if(debug){
			fprintf(stderr, "searching: search: %s, word: %s\n", word,
					current->word);
		}
		if(strcmp(current->word, word)==0){/*found*/
			current->count ++;
			found = 1;
		}else if(strcmp(current->word, word)<0){/*before word*/
			current = current->next;
		}else{/*passed slot*/
			found = 1;
			temp = (word_t *) calloc(1, sizeof(word_t));
			temp->word = current->word;
			temp->next = current->next;
			temp->count = current->count;
			
			current->word = word;
			current->next = temp;
			current->count = 1;
		}
	}
	if(current == NULL){
			
		current = oddHead;
		if(current != NULL){//at end of chain

			while(current->next != NULL){
				current = current->next;
			}
		
			temp = calloc(1, sizeof(word_t));
			temp->word = word;
			temp->next = NULL;
			temp->count = 1;
			current->next = temp;

		}else{//at new chain

			temp = calloc(1, sizeof(word_t));
			temp->word = word;
			temp->next=NULL;
			temp->count = 1;
			temp->next = NULL;
			oddHead = temp;

		}
	}
	if(debug){
		fprintf(stderr, "inserted\n");
	}
	pthread_mutex_unlock(&lockOdd);
}





void init(struct Buffer *b){
	if(debug){
		fprintf(stderr, "in init\n");
	}
	pthread_mutex_init(&b->lock, NULL);
	pthread_cond_init(&b->notempty, NULL);
	pthread_cond_init(&b->notfull, NULL);
	b->readpos = 0;
	b->writepos = 0;
	b->done = 0;
	b->singleFull = 0;
	b->buffer = (char **) calloc (bufferSize, sizeof(char*));
	pthread_mutex_init(&lockOdd, NULL);
	pthread_mutex_init(&lockEven, NULL);
}

int main(argc, argv)
int argc; char *argv[];
{
	FILE *fp;
	char line[2048];
	int i,j;
	void *retval;
	char* temp;
	pthread_t *consumers;
	long milisec;
	int B=0;
	int t = 0;
	int d=0;
	int D = 0;
	if(argc < 10){

		fprintf(stderr, "counter: usage counter -b numlines -t maxcounters -d filedelay -D threaddelay file...\n");
		exit(1);
	}

	for(i = 1; i < 9; i ++){

		if(strcmp(argv[i], "-b") ==0){
			i++;
			bufferSize = atoi(argv[i]);
			if(bufferSize <1){
				fprintf(stderr, "counter: numlines must\
						greater than 0\n");
			}
			if(bufferSize == 1){
				single = 1;
			}
			B = 1;
		} else if(strcmp(argv[i], "-t") == 0){
			i++;
			consumerMax = atoi(argv[i]);
			if(consumerMax < 1 || consumerMax > 26){
				fprintf(stderr, "counter: maxcounters must\
					positive non-zero integer no greater\
					than 26\n");
			}
			consumers = calloc(consumerMax, sizeof(pthread_t));
			consumerNames = calloc(consumerMax, sizeof(char));
			t = 1;
		}else if (strcmp(argv[i], "-d") == 0){
			i++;
			milisec = atol(argv[i]);
			sleepProducer.tv_sec = (int)(milisec/1000);
			milisec = milisec - (sleepProducer.tv_sec*1000);
			sleepProducer.tv_nsec = milisec*1000000L;
			if(debug){
				fprintf(stderr, "counter: filedelay set to %ld\n",
					atol(argv[i])*1000000);
			}
			if(sleepProducer.tv_nsec < 0){
				fprintf(stderr, "counter: filedelay\
					must be non-negative\n");
			}
			d = 1;
		}else if(strcmp(argv[i], "-D") == 0){
			i++;
			milisec = atol(argv[i]);
			sleepConsumer.tv_sec = (int) (milisec/1000);
			milisec = milisec - (sleepConsumer.tv_sec*1000);
			sleepConsumer.tv_nsec = milisec*1000000L;
			if(debug){
				fprintf(stderr, "counter: threaddelay set to %ld\n"
						,(atol(argv[i])*1000000));
			}
			if(sleepConsumer.tv_nsec <0){
				fprintf(stderr, "counter: threaddelay\
						must be non-negative\n");
			}
			D = 1;
		}else{

			fprintf(stderr, "counter: unknown argument %s\n", 
					argv[i]);
			exit(1);
		}

	}

	if(!B || !t || !d || !D){
		fprintf(stderr, "counter: argument error, usage counter -b numlines -t maxcounters -d filedelay -D threaddelay file...\n");
		if(debug){
			fprintf(stderr, "%d%d%d%d\n", B, t, d, D);
		}
		exit(1);
		
	}
	init(&lineBuffer);

	create_counter(consumers);
	if(debug){
		fprintf(stderr, "begin reads\n");
	}
	while(i < argc){

		fp = fopen(argv[i], "r");
		if(fp == NULL){
			fprintf(stderr, "counter: can't open input file %s\n", 
					argv[i]);
		}else{
		
			while(fgets(line, 2048, fp) != NULL){
				sleeper(sleepProducer);
				temp = calloc(strlen(line), sizeof(char));
				strcpy(temp, line);
				if(single){
					put_line_single(&lineBuffer, temp, 
							consumers);
				}else{
					put_line(&lineBuffer, temp, consumers);
				}

			}	
		
			fclose(fp);
		}
		i++;
	}
	if(debug){
		fprintf(stderr, "finished reads\n");
	}
	pthread_mutex_lock(&lineBuffer.lock);
	lineBuffer.done = 1;
	pthread_cond_signal (&lineBuffer.notempty);
	pthread_mutex_unlock(&lineBuffer.lock);
	for(j = 0; j < numConsumers; j++){
		pthread_join(consumers[j], &retval);
	}
	
	fprintf(stdout, "\n\n*****Odd words and frequency:*****\n\n");
	word_t* current = oddHead;
	word_t* previous = oddHead;
	while(current != NULL){
		fprintf(stdout, "%s\t%d\n", current->word, current->count);
		previous = current;
		current = current->next;
		free(previous);
	}
	fprintf(stdout, "\n\n*****Even words and frequency:*****\n\n");
	current = evenHead;
	previous = evenHead;
	while(current!=NULL){
		fprintf(stdout, "%s\t%d\n", current->word, current->count);
		previous = current;
		current= current->next;
		free(previous);
	}
	return 0;
}
