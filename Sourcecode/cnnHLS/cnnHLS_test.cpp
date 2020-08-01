#include <stdio.h>
#include <math.h> 
#include "cnnHLS.h"
void cnnHLS (
int img[784],
int indexoutput[1]
  );

int main () {
	  int input[784] = {
	  		#include "2t.dat"
	  		};
  int output[1];


	  cnnHLS(input,output);
 //  	  printf("%f %f\n",input,output);
//   	  fprintf(fp,"%i %d %d\n",i,signal,output);


  return 0;
}
