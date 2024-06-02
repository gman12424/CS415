/************************************************************************
 			         ttt.c
 Simple ttt client. No queries, no timeouts.
 Uses deprecated address translation functions.

 Phil Kearns
 April 12, 1998
 Modified March 2014
************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include "common.h"
#include "tttchild.h"

void dump_board();

void print_win(FILE* file,  char *board){

	if((board[0] == board[1]) && (board[1] == board[2]) && (board[0] != ' ')){
		fprintf(file, ".c create line 0 50 300 50 -width 3 -fill red \n");
	}else if((board[3] == board[4]) && (board[4] == board[5]) && (board[3] != ' ')){
		fprintf(file, ".c create line 0 150 300 150 -width 3 -fill red \n");
	}else if((board[6] == board[7]) && (board[7] == board[8]) && (board[6] != ' ')){
		fprintf(file, ".c create line 0 250 300 250 -width 3 -fill red \n");
	}else if((board[0] == board[3]) && (board[3] == board[6]) && (board[0] != ' ')){
		fprintf(file, ".c create line 50 0 50 300 -width 3 -fill red \n");
	}else if((board[1] == board[4]) && (board[4] == board[7]) && (board[1] != ' ')){
		fprintf(file, ".c create line 150 0  150 300 -width 3 -fill red \n");
	}else if((board[2] == board[5]) && (board[8] == board[5]) && (board[2] != ' ')){
		fprintf(file, ".c create line 250 0 250 3000 -width 3 -fill red \n");
	}else if((board[0] == board[4]) && (board[4] == board[8]) && (board[0] != ' ')){
		fprintf(file, ".c create line 0 0 300 300 -width 3 -fill red \n");
	}else if((board[2] == board[4]) && (board[4] == board[6]) && (board[2] != ' ')){
		fprintf(file, ".c create line 300 0 0 300 -width 3 -fill red \n");
        }
        
}
        
        
        
        



int main(int argc, char **argv)

{
  char hostid[128], handle[32], opphandle[32], junk;
  char my_symbol; /* X or O ... specified by server in MATCH message */
  char board[9];
  unsigned short xrport;
  int sock, sfile;
  struct sockaddr_in remote;
  struct hostent *h;
  int num, i, move, valid, finished;
  struct tttmsg inmsg, outmsg;
  
  /*for child*/
  int childpid;
  FILE *read_from, *write_to;
  char result[80], canvas_name[5];

  if (argc != 1) {
    fprintf(stderr,"ttt:usage is ttt\n");
    exit(1);
  }

  /* Get host,port of server from file. */

  if ( (sfile = open(SFILE, O_RDONLY)) < 0) {
    perror("TTT:sfile");
    exit(1);
  }
  i=0;
  while (1) {
    num = read(sfile, &hostid[i], 1);
    if (num == 1) {
      if (hostid[i] == '\0') break;
      else i++;
    }
    else {
      fprintf(stderr, "ttt:error reading hostname\n");
      exit(1);
    }
  }
  if (read(sfile, &xrport, sizeof(int)) != sizeof(unsigned short)) {
    fprintf(stderr, "ttt:error reading port\n");
      exit(1);
  }
  close(sfile);


  /* Got the info. Connect. */

  if ( (sock = socket( AF_INET, SOCK_STREAM, 0 )) < 0 ) {
    perror("ttt:socket");
    exit(1);
  }

  bzero((char *) &remote, sizeof(remote));
  remote.sin_family = AF_INET;
  if ((h = gethostbyname(hostid)) == NULL) {
    perror("ttt:gethostbyname");
    exit(1);
  }
  bcopy((char *)h->h_addr, (char *)&remote.sin_addr, h->h_length);
  remote.sin_port = xrport;
  if ( connect(sock, (struct sockaddr *)&remote, sizeof(remote)) < 0) {
    perror("ttt:connect");
    exit(1);
  }

  /* We're connected to the server. Engage in the prescribed dialog */

  /* Await WHO */

  bzero((char *)&inmsg, sizeof(inmsg));  
  getmsg(sock, &inmsg);
  if (inmsg.type != WHO) protocol_error(WHO, &inmsg);
  
  /* Send HANDLE */

  printf("Enter handle (31 char max):");
  fgets(handle, 31, stdin);
  bzero((char *)&outmsg, sizeof(outmsg));
  outmsg.type = HANDLE;
  strncpy(outmsg.data, handle, 31); outmsg.data[31] = '\0';
  putmsg(sock, &outmsg);
  
  
  /*Set up child*/
  childpid = start_child("wish", &read_from, &write_to);
  fprintf(write_to, "source tttplot.tcl\n");
  /*Blocks on read from wish */
  if(fgets(result, 80, read_from) <= 0) exit(0);
	/*Exit if wish dies */
  /*Scan the string from wish, will prompt for update to name*/
  if((sscanf(result, "p %s", canvas_name)) ==1)
	strtok(handle, "\n");
	fprintf(write_to," .lf.you configure -text \"You: %s\"\n", handle);
 
  /* Await MATCH */

  bzero((char *)&inmsg, sizeof(inmsg));  
  getmsg(sock, &inmsg);
  if (inmsg.type != MATCH) protocol_error(MATCH, &inmsg);
  my_symbol = inmsg.board[0];
  strncpy(opphandle, inmsg.data, 31); opphandle[31] = '\0';
  strtok(opphandle, "\n");
  printf("You are playing %c\t your opponent is %s\n\n", my_symbol, opphandle);
  //if(fgets(result, 80, read_from) <= 0) exit(0);
	/*Exit if wish dies */
  /*Scan the string from wish, will prompt for update to name*/
  //if((sscanf(result, "p %s", canvas_name)) ==1)
	fprintf(write_to," .lf.you configure -text \"You: %s (%c)\"\n", handle, my_symbol);
	fprintf(write_to," set player %c\n", my_symbol);
	if(my_symbol== 'O')
		fprintf(write_to," .lf.opponent configure -text \"Opponent: %s (X)\"\n", opphandle);
 	else
		fprintf(write_to," .lf.opponent configure -text \"Opponent: %s (O)\"\n", opphandle);
	fprintf(write_to," set status \"Status: Awaiting Opponent Move\"; .lf.status configure -text $status\n"); fflush(write_to);
  /* In the match */

  for(i=0; i<9; i++){
	 board[i]=' ';
//	 inmsg.board[i] = ' ';
  }
  finished = 0;
  while(!finished){
    bzero((char *)&inmsg, sizeof(inmsg));  
    getmsg(sock, &inmsg);
    switch (inmsg.type) {

    case WHATMOVE:  

      for(i=0; i<9; i++) board[i]=inmsg.board[i];
      dump_board(stdout,board);

      fprintf(write_to, ".c delete move\n");
      for (i = 1; i < 10; i++){
        if(board[i-1] == 'X'){

	  if(i == 1){
	    fprintf(write_to,".c create line 0 0 100 100 -width 4 -tag move\n");
  	    fprintf(write_to,".c create line 0 100 100 0 -width 4 -tag move\n");
	  }else if(i == 2){
	    fprintf(write_to,".c create line 100 0 200 100 -width 4 -tag move\n");
	    fprintf(write_to,".c create line 100 100 200 0 -width 4 -tag move\n");
	  }else if(i == 3){
	    fprintf(write_to,".c create line 200 0 300 100 -width 4 -tag move\n");
	    fprintf(write_to,".c create line 200 100 300 0 -width 4 -tag move\n");
	  }else if(i == 4){
	    fprintf(write_to,".c create line 0 100 100 200 -width 4 -tag move\n");
	    fprintf(write_to,".c create line 100 100 0 200 -width 4 -tag move\n");
	  }else if(i == 5){
	    fprintf(write_to,".c create line 100 100 200 200 -width 4 -tag move\n");
	    fprintf(write_to,".c create line 200 100 100 200 -width 4 -tag move\n");
 	  }else if(i == 6){
	    fprintf(write_to,".c create line 200 100 300 200 -width 4 -tag move\n");
	    fprintf(write_to,".c create line 200 200 300 100 -width 4 -tag move\n");
	  }else if(i == 7){
	    fprintf(write_to,".c create line 0 200 100 300 -width 4 -tag move\n");
	    fprintf(write_to,".c create line 100 200 0 300 -width 4 -tag move\n");
	  }else if(i == 8){
	    fprintf(write_to,".c create line 100 200 200 300 -width 4 -tag move\n");
	    fprintf(write_to,".c create line 200 200 100 300 -width 4 -tag move\n");
	  }else if(i == 9){
	    fprintf(write_to,".c create line 200 200 300 300 -width 4 -tag move\n");
	    fprintf(write_to,".c create line 200 300 300 200 -width 4 -tag move\n");
	  }
        }else if(board[i-1] == 'O'){
	  if(i == 1){
	    fprintf(write_to,".c create oval 0 0 100 100 -width 4 -tag move\n");
	  }else if(i == 2){
	    fprintf(write_to,".c create oval 100 0 200 100 -width 4 -tag move\n");
	  }else if(i == 3){
	    fprintf(write_to,".c create oval 200 0 300 100 -width 4 -tag move\n");
//	    fprintf(write_to,".c create line 200 100 300 0 -width 4\n");
	  }else if(i == 4){
	    fprintf(write_to,".c create oval 0 100 100 200 -width 4 -tag move\n");
//	    fprintf(write_to,".c create line 100 100 0 200 -width 4\n");
	  }else if(i == 5){
	    fprintf(write_to,".c create oval 100 100 200 200 -width 4 -tag move\n");
//	    fprintf(write_to,".c create line 200 100 100 200 -width 4\n");
	  }else if(i == 6){
	    fprintf(write_to,".c create oval 200 100 300 200 -width 4 -tag move\n");
//	    fprintf(write_to,".c create line 200 200 300 100 -width 4\n");
	  }else if(i == 7){
	    fprintf(write_to,".c create oval 0 200 100 300 -width 4 -tag move\n");
//	    fprintf(write_to,".c create line 100 200 0 300 -width 4\n");
	  }else if(i == 8){
	    fprintf(write_to,".c create oval 100 200 200 300 -width 4 -tag move\n");
//	    fprintf(write_to,".c create line 200 200 100 300 -width 4\n");
	  }else{
	    fprintf(write_to,".c create oval 200 200 300 300 -width 4 -tag move\n");
//	    fprintf(write_to,".c create line 200 300 300 200 -width 4\n");
	  }
        }
      }   

    /* Await WHATMOVE/RESULT from server */
    
 /*   bzero((char *)&inmsg, sizeof(inmsg));  
    getmsg(sock, &inmsg);
    switch (inmsg.type) {

    case WHATMOVE:  */

      do {
      	fprintf(write_to,"set status \"Status: Your Move\"; .lf.status configure -text $status; ring 0\n");
      //do {
        valid = 0;
        printf("Enter your move: ");
        num = fscanf(read_from, "%d", &move);
	if (num == EOF) {
	  fprintf(stderr,"ttt:unexpected EOF on standard input\n");
	  exit(1);
	}
	if (num == 0) {
	  if (fread(&junk, 1, 1, stdin)==EOF) {
	    fprintf(stderr,"ttt:unexpected EOF on standard input\n");
	    exit(1);
	  }
	continue;
	}
	//printf("Move: %d", move);
	if (move == 10){
          //printf("resigning\n");
          bzero((char *)&outmsg, sizeof(outmsg));
          outmsg.type = RESIGN;
          putmsg(sock, &outmsg);
	  valid = 1;
	            
	}else{
	  if ((num == 1) && (move >= 1) && (move <= 9) ) valid=1;
          if ((valid) && (board[move-1] != ' '))valid=0;
	  //insert ping here
        }
      } while (!valid);
    
      /* Send MOVE to server */

      if(my_symbol == 'X'){

	if(move == 1){
	  fprintf(write_to,".c create line 0 0 100 100 -width 4 -tag move\n");
	  fprintf(write_to,".c create line 0 100 100 0 -width 4 -tag move\n");
	}else if(move == 2){
	  fprintf(write_to,".c create line 100 0 200 100 -width 4 -tag move\n");
	  fprintf(write_to,".c create line 100 100 200 0 -width 4 -tag move\n");
	}else if(move == 3){
	  fprintf(write_to,".c create line 200 0 300 100 -width 4 -tag move\n");
	  fprintf(write_to,".c create line 200 100 300 0 -width 4 -tag move\n");
	}else if(move == 4){
	  fprintf(write_to,".c create line 0 100 100 200 -width 4 -tag move\n");
	  fprintf(write_to,".c create line 100 100 0 200 -width 4 -tag move\n");
	}else if(move == 5){
	  fprintf(write_to,".c create line 100 100 200 200 -width 4 -tag move\n");
	  fprintf(write_to,".c create line 200 100 100 200 -width 4 -tag move\n");
	}else if(move == 6){
	  fprintf(write_to,".c create line 200 100 300 200 -width 4 -tag move\n");
	  fprintf(write_to,".c create line 200 200 300 100 -width 4 -tag move\n");
	}else if(move == 7){
	  fprintf(write_to,".c create line 0 200 100 300 -width 4 -tag move\n");
	  fprintf(write_to,".c create line 100 200 0 300 -width 4 -tag move\n");
	}else if(move == 8){
	  fprintf(write_to,".c create line 100 200 200 300 -width 4 -tag move\n");
	  fprintf(write_to,".c create line 200 200 100 300 -width 4 -tag move\n");
	}else if(move == 9){
	  fprintf(write_to,".c create line 200 200 300 300 -width 4 -tag move\n");
	  fprintf(write_to,".c create line 200 300 300 200 -width 4 -tag move\n");
	}
      }else{
	if(move == 1){
	  fprintf(write_to,".c create oval 0 0 100 100 -width 4 -tag move\n");
	}else if(move == 2){
	  fprintf(write_to,".c create oval 100 0 200 100 -width 4 -tag move\n");
	}else if(move == 3){
	  fprintf(write_to,".c create oval 200 0 300 100 -width 4 -tag move\n");
//	  fprintf(write_to,".c create line 200 100 300 0 -width 4\n");
	}else if(move == 4){
	  fprintf(write_to,".c create oval 0 100 100 200 -width 4 -tag move\n");
//	  fprintf(write_to,".c create line 100 100 0 200 -width 4\n");
	}else if(move == 5){
	  fprintf(write_to,".c create oval 100 100 200 200 -width 4 -tag move\n");
//	  fprintf(write_to,".c create line 200 100 100 200 -width 4\n");
	}else if(move == 6){
	  fprintf(write_to,".c create oval 200 100 300 200 -width 4 -tag move\n");
//	  fprintf(write_to,".c create line 200 200 300 100 -width 4\n");
	}else if(move == 7){
	  fprintf(write_to,".c create oval 0 200 100 300 -width 4 -tag move\n");
//	  fprintf(write_to,".c create line 100 200 0 300 -width 4\n");
	}else if(move == 8){
	  fprintf(write_to,".c create oval 100 200 200 300 -width 4 -tag move\n");
//	  fprintf(write_to,".c create line 200 200 100 300 -width 4\n");
	}else if (move == 9){
	  fprintf(write_to,".c create oval 200 200 300 300 -width 4 -tag move\n");
//	  fprintf(write_to,".c create line 200 300 300 200 -width 4\n");
	}

      }
      if(move != 10){
        fprintf(write_to,"set status \"Status: Awaiting Opponent Move\"; .lf.status configure -text $status\n");
        bzero((char *)&outmsg, sizeof(outmsg));
        outmsg.type = MOVE;
        sprintf(&outmsg.res, "%c", move-1);
        putmsg(sock, &outmsg);
      }
      break;

    case RESULT:
      for(i=0; i<9; i++) board[i]=inmsg.board[i];
      dump_board(stdout,board);
      fprintf(write_to, ".c delete move\n");
      for (i = 1; i < 10; i++){
        if(board[i-1] == 'X'){

	  if(i == 1){
	    fprintf(write_to,".c create line 0 0 100 100 -width 4 -tag move\n");
  	    fprintf(write_to,".c create line 0 100 100 0 -width 4 -tag move\n");
	  }else if(i == 2){
	    fprintf(write_to,".c create line 100 0 200 100 -width 4 -tag move\n");
	    fprintf(write_to,".c create line 100 100 200 0 -width 4 -tag move\n");
	  }else if(i == 3){
	    fprintf(write_to,".c create line 200 0 300 100 -width 4 -tag move\n");
	    fprintf(write_to,".c create line 200 100 300 0 -width 4 -tag move\n");
	  }else if(i == 4){
	    fprintf(write_to,".c create line 0 100 100 200 -width 4 -tag move\n");
	    fprintf(write_to,".c create line 100 100 0 200 -width 4 -tag move\n");
	  }else if(i == 5){
	    fprintf(write_to,".c create line 100 100 200 200 -width 4 -tag move\n");
	    fprintf(write_to,".c create line 200 100 100 200 -width 4 -tag move\n");
 	  }else if(i == 6){
	    fprintf(write_to,".c create line 200 100 300 200 -width 4 -tag move\n");
	    fprintf(write_to,".c create line 200 200 300 100 -width 4 -tag move\n");
	  }else if(i == 7){
	    fprintf(write_to,".c create line 0 200 100 300 -width 4 -tag move\n");
	    fprintf(write_to,".c create line 100 200 0 300 -width 4 -tag move\n");
	  }else if(i == 8){
	    fprintf(write_to,".c create line 100 200 200 300 -width 4 -tag move\n");
	    fprintf(write_to,".c create line 200 200 100 300 -width 4 -tag move\n");
	  }else if(i == 9){
	    fprintf(write_to,".c create line 200 200 300 300 -width 4 -tag move\n");
	    fprintf(write_to,".c create line 200 300 300 200 -width 4 -tag move\n");
	  }
        }else if(board[i-1] == 'O'){
	  if(i == 1){
	    fprintf(write_to,".c create oval 0 0 100 100 -width 4 -tag move\n");
	  }else if(i == 2){
	    fprintf(write_to,".c create oval 100 0 200 100 -width 4 -tag move\n");
	  }else if(i == 3){
	    fprintf(write_to,".c create oval 200 0 300 100 -width 4 -tag move\n");
//	    fprintf(write_to,".c create line 200 100 300 0 -width 4\n");
	  }else if(i == 4){
	    fprintf(write_to,".c create oval 0 100 100 200 -width 4 -tag move\n");
//	    fprintf(write_to,".c create line 100 100 0 200 -width 4\n");
	  }else if(i == 5){
	    fprintf(write_to,".c create oval 100 100 200 200 -width 4 -tag move\n");
//	    fprintf(write_to,".c create line 200 100 100 200 -width 4\n");
	  }else if(i == 6){
	    fprintf(write_to,".c create oval 200 100 300 200 -width 4 -tag move\n");
//	    fprintf(write_to,".c create line 200 200 300 100 -width 4\n");
	  }else if(i == 7){
	    fprintf(write_to,".c create oval 0 200 100 300 -width 4 -tag move\n");
//	    fprintf(write_to,".c create line 100 200 0 300 -width 4\n");
	  }else if(i == 8){
	    fprintf(write_to,".c create oval 100 200 200 300 -width 4 -tag move\n");
//	    fprintf(write_to,".c create line 200 200 100 300 -width 4\n");
	  }else{
	    fprintf(write_to,".c create oval 200 200 300 300 -width 4 -tag move\n");
//	    fprintf(write_to,".c create line 200 300 300 200 -width 4\n");
	  }
        }
      }   


      switch (inmsg.res) {
      case 'W':
	printf("You win\n");
	fprintf(write_to, "set status \"Status: You win\"; .lf.status configure -text $status\n");
	print_win(write_to, board);
	break;
      case 'L':
	printf("You lose\n");
	fprintf(write_to, "set status \"Status: You lose\"; .lf.status configure -text $status\n");
	print_win(write_to, board);
	break;
      case 'D':
	printf("Draw\n");
	fprintf(write_to, "set status \"Status: Draw\"; .lf.status configure -text $status\n");
	break;
      case 'R':
	printf("Opponent Resigned, You Win\n");
	fprintf(write_to, "set status \"Status: Opponent Resigned, You Win\"; .lf.status configure -text $status\n");
      default:
	fprintf(stderr,"Invalid result code\n");
	exit(1);
      }
      finished = 1;
      break;

    default:
      protocol_error(MOVE, &inmsg);
    }
  }
  return(0);
}


void
dump_board(FILE *s, char *board)
{
  fprintf(s,"%c | %c | %c\n", board[0], board[1], board[2]);
  fprintf(s,"----------\n");
  fprintf(s,"%c | %c | %c\n", board[3], board[4], board[5]);
  fprintf(s,"----------\n");
  fprintf(s,"%c | %c | %c\n", board[6], board[7], board[8]);
}

