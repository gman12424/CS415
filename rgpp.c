#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define NILNODE (struct node * )0

struct node{

	char fileName[256];
	int lineNumber;
	char line[2048];
	struct node *next;
};

char* printReverse(char* line, char* word);
int printFile(struct node* fileStart, char* word, int lFlag,  int nFlag);
int scanFile(char* fileName, char* word);
char* strcstr( char* s1, char* s2);
int strcount(char* s1, char* s2);
struct node* read();

int main(int argc, char *argv[]){

	struct node *head;
	struct node *current;
	int linesMatch = 0; 
	int wordsMatch = 0;
	int arg;
	int wordFlag, lineFlag, nFlag, bFlag;
	char *word;
	wordFlag = 0;
	lineFlag = 0;
	nFlag = 0;
	bFlag = 0;
	char * currentFile;


	if(argc <2){
		
		
		fprintf(stderr,"Error: Incorrect invocation of rgpp."
			" rgpp [-l | -w word] {-n} {-b} expected\n");
		exit(1);
	}
	
	

//read in the arguments


	arg = 2; // so that the for loop start correctly
	if (strcmp(argv[1], "-l") ==0){
		lineFlag = 1;
		//printf("-l selected\n");
	}else if(strcmp(argv[1], "-w")==0){
		wordFlag = 1;
		arg++;
		word = argv[2];
		//printf("-w selected, word: %s\n", word);
	}else{
		fprintf(stderr, "Error: [-l | -w word] expected as first"
			"argument, %s provided.\n", argv[1]);
		exit(1);
	}
	for(; arg < argc; arg++){

		if(strcmp(argv[arg], "-n") == 0){

			nFlag = 1;
		
			//printf("-n selected\n");
		}else if(strcmp(argv[arg], "-b") == 0){

			bFlag = 1;
			//printf("-b selected\n");
		}else if(strcmp(argv[arg], "-l") == 0 || strcmp(argv[arg], 
			"-w") == 0){
			fprintf(stderr, "Error: either -l or -w excepted," 
				"%s provided.\n",argv[arg] );
		}else{
			fprintf(stderr, "Error: [-n] [-b] expected,%s"
				"provided\n.", argv[arg]);
			exit(1);
		}
	}



	//read input from stream
	if((head = read()) == NULL){

		fprintf(stderr, "ERROR: no input\n");
		exit(1);
	}
	current = head;
	struct node *tmp;
	linesMatch++;
	while((tmp = read()) != NULL ){

		current->next = tmp;
		current = current->next;
		linesMatch++;

	}

	//assume for -l
	if(bFlag && lineFlag){
		fprintf(stdout, "\nTHERE ARE %d LINES THAT MATCH\n\n", 
linesMatch);
	}else if(bFlag&& wordFlag){ //-w selected with -b
		current = head;
		currentFile = " "; 
		while(current != NULL){
			//if new file, search it
			if (strcmp(currentFile, current->fileName) !=0){
				currentFile = current->fileName;
				wordsMatch += scanFile(currentFile, word);
			}
			current = current->next;
		}
		fprintf(stdout, "\nTHERE ARE %d MATCHING WORDS\n\n", 
wordsMatch);
	}


	//printFileLines
	current = head;
	currentFile = " ";
	
	while(current != NULL){
		currentFile = current->fileName;
		printFile(current, word, lineFlag,  nFlag);
		
		while(current != NULL && strcmp(currentFile, 
			current->fileName)==0){
			
			current = current->next;
		}
	}
return 0;
}


//modified form of strstr from 
//http://www.firmcodes.com/write-c-code-to-implement-the-strstr-function-to-search-for-a-substring/
//to ignore cases
char* strcstr( char* s1, char* s2){
	char* haystack = s1;
	char* needle = s2;
	
	while(*haystack != '\0'&& *haystack != '\n'){
		char *Begin = haystack;
		char *pattern = needle;
		//If first character of substring match, check for whole 
			//string
		while(*haystack  && *pattern && tolower(*haystack) == 
			tolower(*pattern)){

			haystack++;
			pattern++;
		}
		if (!*pattern){
			return Begin;
		}
		haystack = Begin + 1; //increment Main String
		}
	return NULL;
}


//function that returns # of occurance of word in line

int strcount( char* s1, char* s2){


	char* line = s1;
	char* word = s2;
	int count = 0;


	while((line = (char*)strcstr(line, word)) != NULL){
		line+=1;
		count ++;
		

	}
	return count;
	

}

//function to read in input

struct node *read(){

	char fileName[256];
	long lineNumber;
	char lineNumberS[256];
	char line[2048];
	struct node *tmp;
	char *end;
	char newLine[1];

	if(fscanf(stdin, "%[^:]:%[^:]:%[^\n]%c", fileName, 
lineNumberS, line, newLine)!=EOF){
		tmp = (struct node *) malloc(sizeof(struct node));
		strcpy(tmp->fileName, fileName);
		strcpy(tmp->line, strcat(line, "\n"));
		lineNumber = strtol(lineNumberS, &end, 10);
		if(lineNumber <=0){
			fprintf(stderr, "ERROR: grep must be invoked with arguments -r -H -n -s -I -i\n"); 
			exit(0);
		}
		tmp->lineNumber = lineNumber;
		return tmp;

	}
	return NULL;

}

int scanFile(char* fileName, char* word){

	FILE * file = fopen(fileName, "r");
	if(file == NULL){
		fprintf(stderr, "ERROR: %s not accessible\n", fileName);
	}
	int wordsMatch = 0;
	char* line = malloc(sizeof(char)*2048);


	if(file == NULL){
		fprintf(stderr, "Error: %s file not accessible\n", 
			fileName);
		exit(0);
	}
	
	while(fgets(line, 2048, file)){
		wordsMatch += strcount(line, word);
	memset(line, '\0', 2048);
	}
	
	fclose(file);
	return wordsMatch;


}

//prints a file out with correct formatting
int printFile(struct node* fileStart, char* word, int lFlag,  int 
nFlag){

	
	FILE * file;
	char* fileLine = malloc(sizeof(char)*2048);
	int i;
	int line;
	struct node* current = fileStart;
	


	fprintf(stdout, "======================%s\n\n", 
current->fileName);
	file = fopen(current->fileName, "r");
	if(file == NULL){
		fprintf(stderr, "Error: %s file not accessible\n", 
			current->fileName);
		exit(0);
	}
	i = 1;
	//cycle through all of file
	while(current != NULL && (strcmp(fileStart->fileName, 
current->fileName)==0)){

		line = current->lineNumber;
		while(fgets(fileLine, 2048, file) != NULL && i != line){
			if(nFlag){
				fprintf(stdout, " %d: %s", i, fileLine);
			}else{
				fprintf(stdout, "  %s", fileLine);
			}
			i++;
			memset(fileLine, '\0', 2048);
		}
		if(nFlag){
			if(lFlag){
				fprintf(stdout, "*%d: %s", i, fileLine);
			}else{
				fprintf(stdout, " %d: %s",i,  
					printReverse(fileLine, word));
			}
			
		}else{
			if(lFlag){
				fprintf(stdout, "* %s", fileLine);
			}else{
				fprintf(stdout,"%s", 
					printReverse(fileLine, word));
			}
		}
		i++;
		
		current = current->next;
	}


	//print out remainder of the file;
	while(fgets(fileLine, 2048, file) != NULL){
		if(nFlag){
			fprintf(stdout, " %d: %s\n", i, fileLine);
		}else{
			fprintf(stdout, "  %s\n", fileLine);
		}
	
	}
	return 1;
}


// build the return string in word mode for reverse xterm characters
char* printReverse(char* line, char* word){

	char* reverse = "\e[7m";
	char* reset = "\e[0m";

	char returnString[2100];
	memset(returnString, '\0', 2100); 
	char* linePrev = line;
	int offset;
	char lineWord[strlen(word)+1];
	char block[2100];

	while((line = (char*)strcstr(line, word)) != NULL){
		offset = line - linePrev;
		strncpy(lineWord, line, strlen(word));
		lineWord[strlen(word)] = '\0';
		strncpy(block, linePrev, offset);
		block[offset] = '\0';
		strcat(block, reverse);
		strcat(block, lineWord);
		strcat(block, reset);
		line = line + strlen(word);
		linePrev = line;
		strcat(returnString, block);
		memset(block, '\0', 2100);

	}
	strcat(returnString, linePrev);
	line = returnString;
	return line;



}
