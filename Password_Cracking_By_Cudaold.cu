// to compile
// nvcc -o pc Password_Cracking_By_Cudaold.cu  -lm -lcrypt

#include <cuda_runtime_api.h>
#include <stdio.h>

__device__ int is_a_match(char *check) {	// Compares each password attempt.
  char password[] = "BI1245";

  char *b = check;
  char *c = password;

  while(*b == *c) {
    if(*b == '\0') {
      return 1;
    }
    b++;
    c++;
  }
  return 0; //returns 0 
}

__global__ void kernel() {
  //char *alphabet = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
  char alpha[26] = {'A','B','C','D','E','F','G','H','I','J','K','L','M','N','O','P','Q','R','S','T','U','V','W','X','Y','Z'};
  //alphabet[27] = '\0';
  char num[10] = {'0','1','2','3','4','5','6','7','8','9'};
  //numbers[11] = '\0';
  char check[7];
  check[6] = '\0';
  int i, j, k, l;
  for(i=0;i<10;i++) {
     for(j=0; j<10; j++) {
	for(k=0;k<10;k++) {
     	   for(l=0; l<10; l++) {
          //printf("%c%c\n", alphabet[blockIdx.x], alphabet[threadIdx.x], numbers[i], numbers[j]);
          check[0] = alpha[blockIdx.x];
          check[1] = alpha[threadIdx.x];
          check[2] = num[i];
          check[3] = num[j];
	  check[4] = num[k];
          check[5] = num[l];
          //printf("%s \n", test);

          if(is_a_match(check)) {
          printf("Password successfully cracked: %s\n", check);
          } //else {
               //printf("tried: %s\n", check);
          //}
     }
}}
  }
}

int time_difference(struct timespec *start, struct timespec *finish, long long int *difference)
 {
	  long long int ds =  finish->tv_sec - start->tv_sec; 
	  long long int dn =  finish->tv_nsec - start->tv_nsec; 

	  if(dn < 0 ) 
	  {
	    ds--;
	    dn += 1000000000; 
          } 

	  *difference = ds * 1000000000 + dn;
	  return !(*difference > 0);
}





	//Calculating time



int main(int argc, char *argv[])
{

	
  kernel <<<26, 26>>>();

  cudaThreadSynchronize();

  	
	struct timespec start, finish;   
  	long long int time_elapsed;

  	clock_gettime(CLOCK_MONOTONIC, &start);


	clock_gettime(CLOCK_MONOTONIC, &finish);
	  time_difference(&start, &finish, &time_elapsed);
	  printf("Time elapsed was %lldns or %0.9lfs\n", time_elapsed,
		                                 (time_elapsed/1.0e9)); 
  return 0;
}


