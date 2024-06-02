#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <sys/sysmacros.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <limits.h>

struct fileNode{
  long idNumber;
  struct fileNode *next;
};

int main(argc, argv)
int argc; char *argv[];
{

  struct stat info;
  char fileName[4096];
  struct fileNode* head = NULL;
  struct fileNode* current = head;
  int pressent = 0;
  struct fileNode* temp;
  int num = 0;
  char *end;
  long long seconds;
  long long day = 86400;

  if(argc != 2){

    fprintf(stderr, "accessed: usage accessed days\n");
    exit(1);

  }

  num = (int) strtol(argv[1], &end, 10);
  if(num == 0 || num == INT_MIN || num == INT_MAX){

    fprintf(stderr, "ERROR: 'accessed int', with int non-zero and non-MIN/MAX\n");

  }


  memset(fileName, '\0', 4096);
  while(fscanf(stdin, "%s", fileName) !=EOF){

    if(stat (fileName, &info) == 0){
      if(S_ISREG(info.st_mode) && !S_ISLNK(info.st_mode) && 
        !S_ISDIR(info.st_mode)){
        current = head;
        pressent = 0;
        while(current != NULL && pressent != 1){
	
	  if(current->idNumber == info.st_ino){

	    pressent = 1;
	  }
	  current = current->next;

        }
        seconds = time(NULL) - info.st_atime;
	seconds = seconds / day;
        if(!pressent){

	  temp = (struct fileNode *) malloc(sizeof(struct fileNode));
	  temp->idNumber = info.st_ino;
	  temp->next = head;
	  head = temp;
	  if((num>0 &&seconds > num) || (num < 0 && seconds < -num)){
	    fprintf(stdout, "%s\n", fileName);
	  }

        }
      }
    }
    memset(fileName, '\0', 4096);


  }

  exit(0);
  

}
