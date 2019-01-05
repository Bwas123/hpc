
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <crypt.h>
#include <time.h>
#include <pthread.h>
#include <ctype.h>
/***********************************************************************

/***********************************************************************
*******
  Demonstrates how to crack an encrypted password using a simple
  "brute force" algorithm. Works on passwords that consist only of 2
uppercase
  letters and a 2 digit integer. Your personalised data set is included
in the
  code.

  Compile with:
    cc -o Multithread Multithread.c -lcrypt

  If you want to analyse the results then use the redirection operator
to send
  output to a file that you can view using an editor or the less
utility:

    ./Multithread > results.txt

  Dr Kevan Buckley, University of Wolverhampton, 2018
************************************************************************
******/
int n_passwords = 4;

char *encrypted_passwords[] = {

"$6$KB$H8s0k9/1RQ783G9gF69Xkn.MI.Dq5Ox0va/dFlkknNjO7trgekVOjTv1BKCb.nm3vqxmtO2mOplhmFkwZXecz0",

"$6$KB$WksuNcTfYjZWjDC4Zt3ZAmQ38OrsWwHyDgf/grFJ2Sgg.qpOz56lMpBVfWYdQZa9Pksa2TJRVYVb3K.mbYx4Y1",

"$6$KB$yk0NRG09eqN4G24tQQuCtEyvTzuAWpXdf6cvkpyeZw0BWyy.4rJUrtbFLgleAhWIuVFlZDfEUa/nDwRZUa9MU.",

"$6$KB$IturYhPYxh873EI91wQQkQ7WPlPe3ApR.ozpuaf.1aL33pG4xMbDv5aJzjqB5x2Pw25W54RrnjrH5NHoybAaB/"
};


/**
 This function can crack the kind of password explained above. All
combinations
 that are tried are displayed and when the password is found, #, is put
at the
 start of the line. Note that one of the most time consuming operations
that
 it performs is the output of intermediate results, so performance
experiments
 for this kind of program should not include this. i.e. comment out the
printfs.
*/

void substr(char *dest, char *src, int start, int length){
  memcpy(dest, src + start, length);
  *(dest + length) = '\0';
}


void crack(){
   int k;
  pthread_t t1, t2;

  void *first();
  void *second();
  
  for(k=0;k<n_passwords;k<k++) {
    pthread_create(&t1, NULL, first, encrypted_passwords[k] );
    pthread_create(&t2, NULL, second, encrypted_passwords[k]);
  
  pthread_join(t1, NULL);
  pthread_join(t2, NULL);
}
}


void *first(char *salt_and_encrypted){
 int a,b,c;     			// Loop counters
  char salt[7];    			// String used in hashing the password. Need space
  char plain[7];   			// The combination of letters currently being checked
  char *enc;       			// Pointer to the encrypted password
  int count = 0;   			// The number of combinations explored so far

  substr(salt, salt_and_encrypted, 0, 6);

  for(a='A'; a<='M'; a++){
    for(b='A'; b<='Z'; b++){
       for(c=0; c<=99; c++){
        sprintf(plain, "%c%c%02d", a,b,c);
        enc = (char *) crypt(plain, salt);
        count++;
        if(strcmp(salt_and_encrypted, enc) == 0){
          printf("#%-8d%s %s\n", count, plain, enc);
        } else {
          printf(" %-8d%s %s\n", count, plain, enc);
        }
       }
    }
  }
printf("%d solutions explored\n", count);
}


void *second(char *salt_and_encrypted){
int x, y, z;     
  char salt[7];    
  char plain[7];   
  char *enc;       
  int count = 0;  
  substr(salt, salt_and_encrypted, 0, 6);

  for(x='N'; x<='Z'; x++){
    for(y='A'; y<='Z'; y++){
       for(z=0; z<=99; z++){
        sprintf(plain, "%c%c%02d", x, y, z);
        enc = (char *) crypt(plain, salt);
        count++;
        if(strcmp(salt_and_encrypted, enc) == 0){
          printf("#%-8d%s %s\n", count, plain, enc);
        } else {
          printf(" %-8d%s %s\n", count, plain, enc);
        }
       }
    }
  }
   printf("%d solutions explored\n", count);
}


int time_difference(struct timespec *start, struct timespec *finish, 
                    long long int *difference) {
  long long int ds =  finish->tv_sec - start->tv_sec; 
  long long int dn =  finish->tv_nsec - start->tv_nsec; 

  if(dn < 0 ) {
    ds--;
    dn += 1000000000; 
  } 
  *difference = ds * 1000000000 + dn;
  return !(*difference > 0);
}


int main(int argc, char *argv[]){
 
 struct timespec start, finish;   
  long long int time_elapsed;

  clock_gettime(CLOCK_MONOTONIC, &start);
crack();

  clock_gettime(CLOCK_MONOTONIC, &finish);
  time_difference(&start, &finish, &time_elapsed);
  printf("Time elapsed was %lldns or %0.9lfs\n", time_elapsed,
                                         (time_elapsed/1.0e9)); 

  return 0;
}
