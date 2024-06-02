#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <math.h>

int main(int argc, char *argv[]){

long size;

long beetles;
double time;
double totalTime;
double x, xDelta;
double y, yDelta;
int direction;
double finalDirection;
long i;
int  dead;
char *end;

if(argc != 3){
	
	fprintf(stderr, "Error: 'beetle int int' expected.\n");
	exit(1);
}

size = strtol(argv[1], &end, 10);

if(size <=0 || size == LONG_MIN || size == LONG_MAX ){
	fprintf(stderr, "Error: 'beetle int int' expected, first argument not allowed\n");
	exit(1);
}

beetles = strtol(argv[2], &end, 10);

if(beetles == 0 || beetles == LONG_MIN || beetles == LONG_MAX){

	fprintf(stderr, "Error:'beetle int int' expected, second argument not allowed\n");

	exit(1);

}


for(i = 0; i < beetles; i++){

	dead = 0;
	x=((double) size)/2;
	y= ((double) size)/2;
	time = 0;
	//fprintf(stdout, "beetle %ld: x = %f, y = %f, t = %f\n", i, x, y, time);
	while(!dead){

		direction = random();
		direction = direction %360;
		finalDirection = direction * M_PI/180;
		xDelta = cos(finalDirection);
		yDelta = sin(finalDirection);
		x += xDelta;
		y += yDelta;
		time +=1;
		//fprintf(stdout, "x = %f, y = %f, t = %f, direction =%d,  %f\n", x,
		//	y, time, direction,  finalDirection);

		//check if still alive
		if(x <= size && x >= 0 && y <= size && y >= 0){
			time +=1; //beetle is still alive and sleeps, repeats
		}else{ //beetle is dead
			dead = 1; //ends experiement
		}
	}
	totalTime += time;
	//fprintf(stdout, "died, time = %f, x = %f, y = %f\n", time, x, y);



}

fprintf(stdout, "%ld by %ld square, %ld beetles, mean beetle lifetime is %.1f\n", 
size, 
size,  beetles, totalTime/beetles);

return 0;
}
