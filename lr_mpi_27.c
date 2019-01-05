#include <stdio.h>
#include <math.h>
#include <time.h>
#include <mpi.h>
#include <malloc.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

/******************************************************************************
 * This program takes an initial estimate of m and c and finds the associated 
 * rms error. It is then as a base to generate and evaluate 8 new estimates, 
 * which are steps in different directions in m-c space. The best estimate is 
 * then used as the base for another iteration of "generate and evaluate". This 
 * continues until none of the new estimates are better than the base. This is
 * a gradient search for a minimum in mc-space.
 * 
 * To compile:
 *   mpicc -o lr_coursework_27 lr_coursework_27.c -lm
 * 
 * To run:
 *   mpirun -n 9 ./lr_coursework_27
 * 
 * Dr Kevan Buckley, University of Wolverhampton, 2018
 *****************************************************************************/

typedef struct point_t
{
   double x;
   double y;
} point_t;

int n_data = 1000;
point_t data[];

double residual_error (double x, double y, double m, double c)
{
   double e = (m * x) + c - y;
   return e * e;
}

double rms_error (double m, double c)
{
   int i;
   double Actualmean;
   double sum_error = 0;

   for (i = 0; i < n_data; i++)
   {
      sum_error += residual_error (data[i].x, data[i].y, m, c);
   }

   Actualmean = sum_error / n_data;

   return sqrt (Actualmean);
}
 

// setting time differences

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

int main () {

   struct timespec start, finish; 
  
   long long int time_elapsed;

   clock_gettime(CLOCK_MONOTONIC, &start);

   int rank, size;

   int i;
   double bm = 1.3;
   double bc = 10;
   double be;
   double dm[8];
   double dc[8];
   double e[8];
   double step = 0.01;
   double best_error = 999999999;
   int best_error_i;
   int minimum_found = 0;
   double pError = 0;
   double baseMC[2];

   double om[] = { 0, 1, 1, 1, 0, -1, -1, -1 };
   double oc[] = { 1, 1, 0, -1, -1, -1, 0, 1 };


   MPI_Init (NULL, NULL);
   MPI_Comm_size (MPI_COMM_WORLD, &size);
   MPI_Comm_rank (MPI_COMM_WORLD, &rank);

   be = rms_error (bm, bc);

   if (size != 9)
   {
      if (rank == 0)
      {
         printf
            ("This program is needs to run with exactly 9 processes.\n");
         return 0;
      }
   }

   while (!minimum_found)
   {

      if (rank != 0)
      {
         i = rank - 1;
         dm[i] = bm + (om[i] * step);
         dc[i] = bc + (oc[i] * step);
         pError = rms_error (dm[i], dc[i]);

         MPI_Send (&pError, 1, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD);
         MPI_Send (&dm[i], 1, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD);
         MPI_Send (&dc[i], 1, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD);


         MPI_Recv (&bm, 1, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
         MPI_Recv (&bc, 1, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
         MPI_Recv (&minimum_found, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

      }
      else
      {
         for (i = 1; i < size; i++)
         {
            MPI_Recv (&pError, 1, MPI_DOUBLE, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            MPI_Recv (&dm[i-1], 1, MPI_DOUBLE, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            MPI_Recv (&dc[i-1], 1, MPI_DOUBLE, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            if (pError < best_error)
            {
               best_error = pError;
               best_error_i = i - 1;

            }
         }
          printf ("The best m,c is %lf,%lf with an error %lf in direction: %d\n",
          dm[best_error_i], dc[best_error_i], best_error, best_error_i);
         if (best_error < be)
         {
            be = best_error;
            bm = dm[best_error_i];
            bc = dc[best_error_i];
         }
         else
         {
            minimum_found = 1;
         }

         for (i = 1; i < size; i++)
         {
            MPI_Send (&bm, 1, MPI_DOUBLE, i, 0, MPI_COMM_WORLD);
            MPI_Send (&bc, 1, MPI_DOUBLE, i, 0, MPI_COMM_WORLD);
            MPI_Send (&minimum_found, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
         }
      }
   }

   if(rank==0) {
	   
      printf ("The minimum searched values for m,c is %lf,%lf with error %lf\n are:", bm, bc, be);

      clock_gettime(CLOCK_MONOTONIC, &finish);

      time_difference(&start, &finish, &time_elapsed);

      printf("Time elapsed taken was %lldns or %0.9lfs\n", time_elapsed, 
         (time_elapsed/1.0e9));
   }

   MPI_Finalize();
   return 0;
}
point_t data[] = {
  {76.94,124.69},{85.12,149.73},{75.51,146.60},{69.29,123.38},
  {83.91,157.47},{84.32,143.29},{89.13,134.82},{78.85,145.88},
  {69.64,137.13},{83.35,140.60},{20.23,70.20},{60.53,122.96},
  {37.60,92.40},{67.00,127.74},{31.88,71.49},{62.09,114.35},
  {38.53,93.58},{97.47,155.92},{70.17,131.58},{22.21,59.54},
  {22.53,52.86},{ 4.40,32.43},{98.61,177.92},{97.69,166.69},
  {18.26,38.36},{17.96,66.61},{92.16,188.25},{84.05,153.40},
  {36.34,87.75},{93.43,167.58},{70.70,140.10},{30.27,93.51},
  {22.22,72.54},{26.98,75.73},{70.90,126.93},{34.89,90.32},
  {56.06,113.83},{84.21,148.38},{93.04,165.13},{78.67,152.01},
  {88.46,146.74},{67.75,134.87},{98.05,177.26},{48.88,109.70},
  {13.98,64.97},{81.58,151.75},{62.93,128.67},{69.73,140.58},
  {71.51,144.60},{45.09,91.52},{66.02,138.53},{ 6.39,40.31},
  {78.87,154.79},{22.25,86.28},{27.01,72.97},{22.58,62.03},
  {36.51,76.63},{ 9.22,41.94},{24.31,70.67},{43.02,108.85},
  {54.97,116.25},{85.56,138.70},{59.56,126.52},{75.43,139.92},
  {20.53,55.18},{99.26,184.17},{47.44,107.14},{12.33,56.25},
  {63.02,128.19},{30.19,92.37},{97.47,167.03},{92.94,184.03},
  {66.70,136.10},{86.99,173.29},{30.01,79.94},{19.51,68.97},
  {30.62,78.34},{97.03,165.92},{85.27,159.80},{ 8.59,39.06},
  {22.78,72.57},{50.12,99.23},{78.25,151.84},{ 5.80,21.49},
  {99.94,174.26},{79.15,143.38},{76.25,135.36},{46.84,90.82},
  {24.68,77.30},{37.63,93.44},{88.78,163.67},{81.14,157.57},
  {24.51,46.28},{48.00,107.36},{31.54,78.37},{70.84,124.41},
  { 9.43,53.77},{36.22,87.08},{12.57,43.87},{81.03,136.30},
  {16.92,60.97},{38.41,88.95},{75.34,163.51},{23.04,58.70},
  {50.24,96.84},{93.45,170.19},{50.57,116.51},{16.85,68.63},
  {47.97,93.94},{58.72,121.60},{73.24,138.17},{96.56,176.73},
  {36.80,88.24},{70.05,129.77},{60.49,131.82},{ 5.79,61.99},
  {44.45,105.21},{82.61,146.91},{94.34,165.19},{60.71,128.19},
  {88.85,150.83},{54.16,97.19},{35.67,89.33},{34.63,87.93},
  {73.40,129.27},{66.70,134.01},{81.98,165.35},{ 3.66,56.48},
  {32.69,86.30},{ 8.01,42.09},{26.77,74.46},{78.15,138.41},
  {68.84,135.45},{43.28,111.13},{91.20,175.14},{37.84,95.33},
  {88.47,166.44},{75.16,154.58},{50.15,93.19},{27.64,76.93},
  {84.90,150.89},{54.61,104.22},{13.53,63.03},{13.65,57.03},
  {23.63,67.65},{23.70,58.81},{38.69,111.70},{70.63,129.42},
  {79.79,152.66},{47.27,90.28},{97.16,183.35},{48.34,115.36},
  {41.15,86.60},{29.52,81.66},{ 5.14,45.53},{76.64,161.05},
  {99.98,161.44},{75.56,142.78},{18.51,45.96},{93.90,176.34},
  {31.23,86.13},{67.13,135.17},{15.96,48.96},{38.67,89.85},
  {74.90,129.87},{89.97,153.28},{ 2.50,29.99},{84.41,147.07},
  {12.98,36.60},{ 2.02,43.03},{51.76,120.81},{36.21,93.15},
  {63.93,124.03},{66.46,132.36},{79.92,149.78},{92.36,171.86},
  {86.96,148.81},{65.53,125.73},{12.79,60.01},{63.06,125.93},
  {50.81,113.01},{61.74,129.85},{ 8.02,44.07},{44.87,117.19},
  {38.30,84.43},{75.20,140.98},{82.07,153.94},{38.40,84.38},
  {99.95,179.39},{51.84,99.20},{73.60,149.21},{23.78,72.31},
  { 5.21,49.78},{22.81,88.63},{59.80,106.55},{65.23,122.87},
  { 4.07,46.60},{23.42,75.02},{97.44,159.57},{92.80,155.81},
  {61.52,91.23},{ 0.09,47.09},{56.03,106.05},{57.76,111.59},
  {45.74,111.27},{23.39,56.86},{11.55,50.89},{81.80,160.73},
  {97.67,168.64},{25.95,66.05},{75.03,130.36},{58.11,122.69},
  {19.06,74.80},{23.47,82.34},{70.18,117.20},{18.91,65.19},
  {12.05,41.99},{28.78,75.68},{23.18,65.81},{17.42,67.69},
  {65.23,116.74},{33.27,78.30},{77.97,151.22},{87.74,153.45},
  {95.61,169.76},{59.49,121.01},{25.65,81.63},{38.67,89.19},
  {11.40,59.22},{ 2.34,39.39},{14.83,47.35},{55.13,117.21},
  {12.03,64.77},{97.76,161.00},{87.17,165.48},{ 1.90,52.12},
  { 4.68,43.22},{ 8.17,55.96},{65.56,120.43},{16.95,49.81},
  {75.95,155.13},{54.87,112.50},{31.77,77.71},{86.83,143.69},
  {52.54,108.18},{28.81,78.34},{ 9.31,33.19},{20.88,72.73},
  {76.41,143.86},{32.15,78.14},{26.30,63.99},{98.94,165.11},
  {85.77,136.01},{12.19,59.40},{79.76,152.44},{45.38,101.45},
  {44.80,107.98},{65.88,145.47},{32.52,90.77},{28.45,77.94},
  {32.11,87.10},{40.64,87.17},{30.08,85.39},{79.08,147.28},
  {78.45,140.98},{71.29,139.23},{55.25,113.40},{49.85,105.06},
  {87.99,173.48},{26.14,68.27},{94.37,196.05},{32.95,80.49},
  {52.41,99.85},{ 6.63,47.96},{82.20,145.91},{74.62,152.49},
  { 6.11,61.65},{29.65,66.28},{ 7.74,54.35},{13.33,68.27},
  {65.80,128.05},{12.55,57.99},{65.55,123.38},{91.30,168.39},
  {80.75,142.64},{ 0.30,30.65},{23.88,71.93},{55.73,108.33},
  {30.32,99.46},{ 3.82,43.67},{16.48,64.31},{60.73,117.98},
  {96.16,177.71},{82.21,140.88},{85.75,141.71},{29.67,46.80},
  {69.19,137.89},{30.13,69.29},{12.05,61.28},{33.73,84.93},
  {78.84,157.15},{29.72,90.48},{88.45,161.11},{51.52,131.57},
  {83.33,153.38},{27.68,71.65},{38.90,95.98},{12.55,40.85},
  {86.10,153.92},{52.57,96.57},{68.49,130.39},{63.79,116.10},
  {47.89,89.20},{ 0.47,26.48},{99.64,161.53},{67.36,129.66},
  {43.87,108.32},{16.02,68.54},{93.00,165.22},{91.42,154.58},
  {17.97,68.57},{66.51,112.87},{99.13,171.38},{72.62,150.98},
  {39.53,101.11},{39.64,103.34},{77.99,148.00},{43.75,91.39},
  {62.81,113.89},{84.88,139.75},{10.26,51.88},{98.34,174.86},
  {58.29,99.95},{59.65,129.51},{67.69,126.98},{37.72,84.93},
  {83.42,147.53},{96.70,167.91},{22.26,72.11},{43.27,112.34},
  {61.91,125.82},{77.50,135.75},{41.88,89.87},{47.96,95.98},
  {40.61,93.42},{14.44,65.64},{15.82,58.43},{ 9.85,62.30},
  {26.03,70.40},{30.38,79.71},{89.23,164.43},{79.35,143.56},
  {21.87,68.53},{14.02,34.23},{55.33,121.06},{36.36,89.25},
  { 8.32,51.15},{67.99,166.35},{78.55,150.12},{39.83,87.52},
  {57.79,102.69},{46.47,77.87},{41.02,84.11},{50.80,103.29},
  {73.81,122.69},{ 6.42,52.70},{11.88,59.41},{28.73,86.15},
  {62.03,106.70},{27.32,72.34},{72.94,132.20},{ 0.30,32.44},
  {86.65,164.44},{58.49,102.01},{72.38,147.17},{ 1.51,39.27},
  {67.44,133.31},{58.01,129.12},{95.56,159.37},{37.83,107.91},
  { 4.20,59.30},{87.20,144.53},{67.79,129.22},{99.79,176.18},
  {16.80,77.15},{96.97,173.56},{70.47,134.40},{29.41,90.22},
  {58.22,131.84},{ 1.65,33.08},{57.71,121.84},{39.15,82.95},
  {15.90,44.97},{33.69,96.70},{ 8.61,59.97},{34.82,83.08},
  {39.87,106.38},{32.41,84.68},{65.44,111.44},{ 5.13,49.35},
  {18.12,63.76},{23.81,76.56},{59.76,119.97},{74.84,147.99},
  {70.64,143.18},{95.70,171.42},{87.74,158.87},{32.42,83.65},
  {42.68,114.73},{55.62,127.52},{ 0.76,37.01},{40.64,92.40},
  {60.94,137.90},{16.63,52.92},{91.46,174.79},{51.40,104.22},
  {54.13,101.22},{69.99,124.73},{49.95,97.62},{10.47,50.29},
  {77.35,140.02},{62.77,129.66},{46.95,104.46},{10.00,48.83},
  {80.13,144.87},{31.58,90.46},{86.84,148.97},{13.01,58.30},
  {68.17,125.29},{74.62,145.55},{99.38,164.77},{23.32,66.83},
  {42.53,98.41},{47.74,109.49},{81.99,144.26},{83.56,159.05},
  {77.77,143.62},{49.36,98.86},{ 0.92,14.87},{39.57,93.42},
  {56.59,98.18},{90.64,172.55},{60.37,129.79},{ 4.06,44.60},
  {71.63,142.93},{16.44,58.27},{34.72,88.83},{11.61,55.62},
  {55.55,112.78},{ 2.63,43.396},{50.21,109.25},{10.61,65.78},
  {58.26,119.03},{74.24,156.99},{13.94,57.91},{18.28,55.93},
  {50.73,80.82},{ 3.70,23.40},{ 6.07,46.69},{83.66,160.83},
  {52.55,124.89},{68.51,116.63},{99.30,169.38},{99.07,174.67},
  {70.81,157.00},{49.02,104.62},{63.59,123.82},{25.28,75.22},
  {92.62,175.42},{78.88,145.42},{31.93,82.91},{69.67,121.40},
  {23.55,53.65},{62.35,133.88},{84.30,173.36},{99.55,191.78},
  {97.79,148.12},{39.26,107.23},{ 8.45,51.29},{92.26,164.87},
  {48.59,102.82},{55.74,107.10},{ 7.01,32.94},{86.16,159.98},
  {80.92,137.25},{78.42,147.01},{ 1.89,44.75},{21.94,85.06},
  {24.48,86.90},{69.87,113.67},{33.09,74.78},{42.31,75.96},
  {49.03,92.39},{11.95,40.35},{29.02,82.57},{92.08,156.79},
  {73.43,144.00},{79.18,182.89},{97.57,168.50},{22.87,57.14},
  {94.29,153.15},{27.79,63.79},{29.31,86.34},{13.01,37.32},
  {23.83,78.39},{38.75,85.29},{76.09,131.04},{17.75,56.79},
  { 5.01,51.67},{ 8.04,63.87},{95.95,160.31},{48.15,104.24},
  {58.62,113.30},{62.09,119.33},{88.14,159.41},{76.78,139.99},
  {46.07,94.12},{18.90,71.48},{14.00,60.97},{44.13,122.19},
  {73.91,139.62},{48.62,102.26},{81.77,152.70},{92.29,143.32},
  { 9.89,40.78},{70.04,128.77},{72.16,126.83},{32.68,77.02},
  { 4.78,36.80},{ 4.52,25.96},{46.72,98.31},{67.67,148.21},
  {44.06,90.31},{45.33,92.759},{53.24,116.86},{83.65,141.72},
  {30.34,83.87},{20.31,53.79},{33.07,90.82},{35.84,84.35},
  {37.35,81.15},{22.60,50.96},{84.53,142.81},{75.58,132.74},
  {95.70,171.00},{53.68,110.30},{27.61,93.09},{83.50,161.57},
  {71.31,134.44},{ 5.53,35.49},{73.73,135.30},{78.89,145.01},
  {13.45,57.05},{20.93,54.46},{25.54,57.96},{75.07,142.54},
  { 3.69,40.71},{63.26,124.82},{79.51,140.61},{59.07,115.52},
  {39.58,69.45},{89.65,158.32},{ 8.83,59.04},{99.44,178.84},
  {68.77,132.12},{52.49,106.17},{31.39,71.40},{19.08,72.71},
  {68.13,115.40},{38.33,90.09},{52.14,125.64},{ 5.51,41.20},
  {43.23,88.64},{38.60,86.59},{79.83,143.51},{16.94,75.13},
  {29.46,73.32},{17.50,70.92},{14.74,61.71},{45.57,93.64},
  {68.55,129.83},{46.02,111.94},{63.60,125.36},{84.93,155.21},
  {25.13,68.54},{65.64,138.43},{25.33,81.27},{92.97,171.53},
  {35.93,86.53},{86.28,161.12},{44.57,100.01},{97.59,175.01},
  {18.61,58.73},{50.88,112.87},{88.48,161.71},{73.83,131.60},
  {99.28,184.56},{15.04,84.69},{28.50,59.38},{12.60,64.62},
  { 9.11,33.01},{28.97,90.08},{60.13,115.57},{63.92,138.23},
  {11.40,51.62},{60.63,106.82},{99.62,173.35},{ 6.11,35.62},
  {27.81,72.53},{ 4.03,27.95},{98.43,171.81},{32.63,74.73},
  {25.99,69.03},{96.31,163.70},{44.54,89.37},{92.97,160.80},
  {63.24,128.75},{ 9.55,53.37},{76.22,146.16},{94.23,165.23},
  {60.92,93.41},{18.66,60.57},{12.48,32.10},{92.75,150.29},
  { 7.15,45.48},{82.82,135.01},{94.80,182.72},{78.83,136.70},
  { 1.78,46.39},{ 0.28,29.81},{20.03,58.07},{78.67,159.85},
  {93.02,161.76},{50.08,93.81},{65.25,143.20},{ 5.97,40.75},
  {92.05,164.26},{92.13,148.76},{92.45,161.58},{ 5.41,38.37},
  {23.22,78.27},{97.09,174.75},{60.45,129.86},{42.00,89.25},
  {87.16,153.53},{19.28,64.51},{ 9.98,43.11},{49.65,104.88},
  {89.90,172.18},{35.30,80.57},{ 4.23,23.32},{20.49,74.64},
  {71.16,146.24},{ 9.35,68.21},{73.32,122.41},{17.18,59.31},
  {72.41,139.63},{14.89,64.82},{99.66,171.55},{36.64,87.08},
  {58.48,112.65},{ 8.22,46.77},{73.60,138.34},{51.49,98.61},
  {10.81,52.85},{43.38,105.22},{70.87,146.66},{43.70,112.35},
  {24.20,60.33},{38.70,85.61},{27.70,83.16},{99.64,158.22},
  { 2.47,60.98},{44.83,99.80},{ 6.54,35.17},{53.37,99.18},
  {95.92,178.74},{29.02,73.35},{ 1.25,31.28},{ 4.09,45.63},
  {44.13,107.44},{81.51,135.33},{49.05,103.72},{93.28,173.40},
  {54.42,101.30},{74.66,143.49},{91.37,167.32},{ 3.01,42.76},
  {54.82,119.26},{85.70,147.08},{33.31,85.93},{98.93,196.63},
  {54.97,108.60},{91.51,172.46},{93.86,168.32},{28.34,80.04},
  {71.05,145.29},{38.81,107.70},{97.70,181.16},{44.40,100.31},
  {19.20,62.98},{29.62,79.92},{64.37,127.28},{70.61,129.59},
  {95.43,171.12},{91.08,163.10},{51.78,121.56},{73.39,134.04},
  {20.12,71.68},{72.87,166.21},{17.45,60.38},{26.49,84.29},
  {32.74,82.33},{42.20,99.64},{85.38,157.54},{89.36,162.22},
  {24.64,65.67},{72.46,131.96},{65.86,126.91},{44.99,104.03},
  { 3.88,46.41},{27.60,83.88},{74.58,137.71},{88.74,163.55},
  {71.74,142.05},{22.26,58.44},{40.30,96.44},{87.97,159.53},
  {61.71,137.22},{52.84,113.53},{98.08,156.03},{80.27,159.75},
  {51.72,112.51},{13.92,74.92},{41.91,95.60},{54.53,101.10},
  {50.64,106.94},{ 9.83,48.95},{60.28,121.14},{64.74,116.54},
  {70.06,142.05},{58.11,134.04},{54.83,90.68},{86.87,160.67},
  {90.71,172.80},{ 1.79,34.67},{64.02,133.94},{ 5.77,47.90},
  {91.87,152.15},{50.45,102.58},{94.22,177.81},{62.21,128.39},
  {52.28,107.19},{98.57,171.88},{14.25,59.73},{43.16,83.07},
  {82.84,150.04},{51.00,92.64},{53.02,102.83},{ 9.47,37.69},
  {59.60,119.25},{99.56,166.42},{68.72,139.24},{ 3.46,40.50},
  {21.70,76.81},{78.92,141.56},{17.47,57.96},{80.48,128.40},
  { 6.76,29.28},{16.14,54.19},{55.58,97.02},{78.85,165.78},
  {95.99,163.18},{64.92,126.83},{61.03,123.99},{93.97,170.55},
  {97.03,181.61},{99.16,187.83},{33.99,81.64},{51.60,109.25},
  {12.42,61.71},{23.09,68.16},{61.86,125.34},{47.16,111.12},
  {62.14,119.37},{95.73,162.44},{12.54,73.40},{79.54,145.97},
  {66.33,119.82},{88.68,163.37},{ 8.30,49.86},{ 7.06,51.59},
  {54.64,110.52},{19.58,63.04},{40.40,76.01},{71.79,147.24},
  {65.33,119.61},{24.25,73.73},{93.71,173.33},{84.98,156.37},
  {71.69,140.90},{88.82,163.04},{28.70,71.92},{63.51,125.82},
  { 2.52,56.53},{16.59,57.19},{ 4.69,51.71},{66.16,128.12},
  {99.16,175.08},{85.20,150.40},{67.52,131.68},{98.44,157.07},
  {23.50,74.14},{13.83,53.49},{36.83,96.33},{50.81,97.21},
  {90.89,151.05},{88.77,149.26},{24.56,65.91},{29.39,87.09},
  {40.68,94.57},{37.06,92.04},{41.94,84.73},{78.34,159.44},
  {58.06,120.10},{66.08,137.57},{38.76,95.59},{90.16,162.35},
  { 1.23,32.05},{18.40,79.85},{83.68,160.43},{70.55,137.08},
  {48.80,116.13},{58.34,134.98},{64.55,151.88},{19.11,54.99},
  {17.15,62.20},{ 8.76,41.24},{69.37,127.38},{ 0.02,19.91},
  {89.31,173.50},{25.05,65.73},{29.51,71.31},{74.47,127.67},
  { 7.79,49.80},{ 9.76,46.79},{44.56,97.59},{12.82,48.87},
  {26.64,74.62},{ 8.68,48.98},{99.55,161.34},{54.80,119.52},
  {40.01,86.37},{31.59,97.22},{84.72,153.95},{91.17,156.54},
  {59.85,123.74},{97.10,176.28},{86.33,145.80},{43.16,92.40},
  {76.57,132.84},{38.40,93.39},{24.22,72.02},{67.22,139.69},
  {44.30,125.48},{ 9.13,23.05},{85.36,153.44},{82.06,146.76},
  {95.08,163.29},{78.47,153.07},{97.25,155.67},{24.26,59.22},
  {29.48,73.16},{11.24,52.71},{40.39,89.98},{89.41,168.71},
  {51.84,117.90},{79.26,151.83},{37.47,91.62},{56.44,110.52},
  { 9.75,54.62},{37.20,103.71},{56.91,123.87},{62.93,128.80},
  {55.27,118.51},{76.67,170.89},{82.59,158.13},{ 4.32,35.69},
  {80.02,143.24},{92.93,165.87},{57.21,115.22},{48.28,97.73},
  {34.12,88.32},{23.01,74.70},{43.39,88.40},{53.99,114.82},
  {43.23,85.31},{37.38,78.87},{53.70,99.14},{80.86,140.55},
  {83.41,152.02},{41.76,92.07},{46.13,87.93},{71.33,150.25},
  {57.03,111.85},{53.63,111.67},{57.60,105.70},{84.09,153.43},
  {94.65,188.65},{26.18,60.44},{61.56,130.96},{71.92,133.33},
  {73.47,148.70},{70.59,155.82},{55.25,106.47},{39.05,78.82},
  {22.82,59.61},{18.96,64.30},{48.51,105.28},{72.98,127.46},
  {45.37,101.92},{95.44,171.66},{66.34,123.39},{82.91,157.36},
  {32.95,90.63},{74.25,133.83},{ 5.38,38.85},{16.07,52.25},
  {14.03,65.20},{72.95,123.91},{ 5.49,27.28},{23.20,65.30},
  {26.05,74.95},{ 8.96,41.39},{38.88,90.58},{36.53,77.95},
  {32.58,96.24},{67.62,138.86},{33.63,96.46},{ 3.97,46.13},
  { 9.36,51.30},{89.54,151.56},{ 2.04,50.74},{91.76,168.71},
  { 2.97,24.38},{77.29,155.71},{79.72,141.21},{55.39,115.18},
  {27.87,68.07},{88.52,158.19},{67.42,122.05},{75.84,146.67},
  {54.66,83.52},{52.04,113.77},{57.15,110.66},{ 7.28,31.72},
  {80.30,144.21},{79.46,150.44},{85.21,161.08},{25.16,76.32},
  {59.84,126.36},{15.75,73.98},{15.06,36.26},{83.62,150.75},
  {81.55,144.19},{98.43,171.96},{57.64,120.44},{21.58,59.74},
  {53.96,113.42},{99.61,166.69},{61.04,104.13},{23.26,60.93},
  {91.26,148.91},{78.49,135.20},{ 3.76,59.75},{45.98,99.50},
  {76.59,135.69},{14.28,53.24},{50.91,116.54},{92.53,191.37},
  {42.29,90.46},{32.91,85.46},{91.97,178.74},{38.66,82.60},
  {65.44,119.45},{44.22,109.72},{47.66,83.32},{51.49,118.12},
  {26.70,81.33},{25.40,78.19},{17.66,63.50},{ 2.67,57.07},
  {99.41,164.10},{41.46,89.46},{39.25,84.32},{15.97,56.02},
  {49.31,104.93},{69.45,127.07},{77.58,157.60},{89.97,174.81}
};
