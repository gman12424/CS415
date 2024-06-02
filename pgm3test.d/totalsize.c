#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <errno.h>
#include <sys/sysmacros.h>
#include <time.h>
#include <string.h>


struct fileNode{
  long idNumber;
  struct fileNode * next;

};


int main(argc,argv)
int argc; char *argv[];
{

  struct stat info;
  char fileName[4096];
  struct fileNode* head = NULL;
  struct fileNode* current = head;
  int pressent = 0;
  struct fileNode* temp;
  long long totalSize = 0;


  if (argc != 1) {
    fprintf(stderr, "totalsize: usage totalsize\n\t totalsize expects a \
	pipe of file names.\n");
    exit(1);
  }

  memset(fileName, '\0', 4096);
  while(fscanf(stdin, "%s", fileName)!=EOF){
   
    if(stat(fileName, &info)== 0){
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
        if(!pressent){

	  temp = (struct fileNode *) malloc(sizeof(struct fileNode));
	  temp->idNumber = info.st_ino;
	  temp->next = head;
	  head = temp;
	
	  totalSize += info.st_size;

        }

        }
    }
    memset(fileName, '\0', 4096);
    
    
  }

  fprintf(stdout, "%lld\n", totalSize);
  exit(0);
}
