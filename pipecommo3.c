int p2c[2], c2p[2], val=0, dblval =0;

main(){

  if(fork() == 0){
    close(p2c[1]); close(c2p[0]);
    while(read(p2c[0], &val, sizeof(int)) !=0){
      dblval = 2*val;
      write(c2p[1], &dblval, sizeof(int));
    }
    exit(0)
  }else{
    close(c2p[1]); close(p2c[0]);
    for(val=1; val<=3; val++){
      fprintf(stderr, Asking
