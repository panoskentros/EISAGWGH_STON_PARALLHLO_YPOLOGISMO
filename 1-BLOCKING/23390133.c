#include <stdio.h>
#include <stdlib.h>
#include "mpi.h"
#include <stdbool.h>
#define TAG 999
bool wantsToContinue(int rank);

int main(int argc, char *argv[]) {
    MPI_Init(&argc, &argv);

    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  

   int * local_arr;
   int * local_delta;
   int elementsPerProc,local_max;
   float local_var=0.0,local_sum;
   
    while (wantsToContinue(rank)) {
        if (rank == 0) {
            int size,n;
            MPI_Comm_size(MPI_COMM_WORLD, &size);
            fprintf(stdout,"Δώσε αριθμό στοιχείων: \n");
            scanf("%d",&n); 
        
            elementsPerProc = n/size;
            local_arr = (int *)malloc(elementsPerProc*sizeof(int));
            if(local_arr==NULL){
                printf("Σφάλμα στην δέσμευση μνήμης\n");
                exit(-1);
            }
            for (int p = 0; p < size; p++) {
                int *buffer = malloc(elementsPerProc * sizeof(int));

                for (int j = 0; j < elementsPerProc; j++) {
                    int val;
                    printf("Δώσε αριθμό για το στοιχείο %d: \n", p * elementsPerProc + j + 1);
                    scanf("%d", &val);
                    buffer[j] = val;
                }

                if (p == 0) {
                    for (int j = 0; j < elementsPerProc; j++)
                        local_arr[j] = buffer[j];
                } else {
                    MPI_Send(&elementsPerProc, 1, MPI_INT, p, TAG, MPI_COMM_WORLD);
                    MPI_Send(buffer, elementsPerProc, MPI_INT, p, TAG, MPI_COMM_WORLD);
                }
                free(buffer);
            }
            
            local_max = local_arr[0];
            local_sum = local_arr[0];
            for (int i = 1; i < elementsPerProc; i++) {
                local_sum = local_sum + local_arr[i];
                if(local_arr[i]>local_max)
                    local_max = local_arr[i];
            }
            float other_rank_sum,
            total_sum = local_sum;
            int other_rank_max;
            int total_max=local_max; // p0
            for (int i = 1; i < size; i++) {
                MPI_Recv(&other_rank_sum, 1, MPI_FLOAT, i, TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                total_sum += other_rank_sum;
                MPI_Recv(&other_rank_max, 1, MPI_INT, i, TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                if(other_rank_max>total_max)
                    total_max = other_rank_max;
            }
            float total_avg = (float)total_sum / n;
            printf("Μέσος όρος = %f\n", total_avg);
            printf("Μέγιστος όρος: %d\n",total_max);

            for (int i = 1; i < size; i++)
                MPI_Send(&total_avg,1,MPI_FLOAT,i,TAG,MPI_COMM_WORLD); // μ
            
            for (int i = 0; i < elementsPerProc; i++)
            {
                local_var += (local_arr[i] - total_avg) * (local_arr[i] - total_avg);
            }
            float other_rank_var,
            total_var = local_var;
            for (int i = 1; i < size; i++){
                MPI_Recv(&other_rank_var,1,MPI_FLOAT,i,TAG,MPI_COMM_WORLD, MPI_STATUS_IGNORE); 
                total_var += other_rank_var;
            }  
            total_var = total_var / n;
            printf("var = %f \n",total_var);

            for (int i = 1; i < size; i++)
                MPI_Send(&total_max,1,MPI_INT,i,TAG,MPI_COMM_WORLD); // μ
            

            // δ ερώτημα
            int *delta = (int *)malloc(n * sizeof(int));
            for (int i = 0; i < elementsPerProc; i++)
                delta[i] = (local_arr[i] - total_max) * (local_arr[i] - total_max);

            for (int p = 1; p < size; p++) {
    MPI_Recv(&delta[p * elementsPerProc], elementsPerProc, MPI_INT, p, TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
}


            // Print full Δ vector
            printf("Δ vector: ");
            for (int i = 0; i < n; i++)
                printf("%d ", delta[i]);
            printf("\n");

            free(delta);

        }else{
            MPI_Recv(&elementsPerProc, 1, MPI_INT, 0, TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            local_arr = (int *)malloc(sizeof(int)*elementsPerProc);
            if(local_arr==NULL){
                printf("Σφάλμα στην δέσμευση μνήμης\n");
                exit(-1);
            }
            MPI_Recv(local_arr, elementsPerProc, MPI_INT, 0, TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            local_max = local_arr[0];
            local_sum = local_arr[0];
            for (int i = 1; i < elementsPerProc; i++){
                local_sum = local_sum + local_arr[i];
                if(local_arr[i]>local_max)
                    local_max = local_arr[i];
            }
            MPI_Send(&local_sum, 1, MPI_FLOAT, 0, TAG, MPI_COMM_WORLD);
            MPI_Send(&local_max, 1, MPI_INT, 0, TAG, MPI_COMM_WORLD);
            float avg;
            MPI_Recv(&avg, 1, MPI_FLOAT, 0, TAG, MPI_COMM_WORLD,MPI_STATUS_IGNORE);
            for (int i = 0; i < elementsPerProc; i++)
            {
                local_var += (local_arr[i] - avg) * (local_arr[i] - avg);
            }
            MPI_Send(&local_var,1,MPI_FLOAT,0,TAG,MPI_COMM_WORLD);
            int total_max;
            MPI_Recv(&total_max,1,MPI_INT,0,TAG,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
            local_delta = (int *)malloc(elementsPerProc*sizeof(int));
            for (int i = 0; i < elementsPerProc; i++)
                local_delta[i] = (local_arr[i] - total_max) * (local_arr[i] - total_max);
            
            MPI_Send(local_delta,elementsPerProc,MPI_INT,0,TAG,MPI_COMM_WORLD);
            
            
        }
        free(local_arr);
    }
    
    MPI_Finalize();
    return 0;
}

bool wantsToContinue(int rank) {
    int choice;
    if (rank == 0) {
        do {
            fprintf(stdout, "1. Συνέχεια\n2. Έξοδος\n");
            scanf("%d", &choice);
        } while (choice != 1 && choice != 2);
        int size;
        MPI_Comm_size(MPI_COMM_WORLD, &size);
        for (int i = 1; i < size; i++)
            MPI_Send(&choice, 1, MPI_INT, i, TAG, MPI_COMM_WORLD);
    } else {
        MPI_Recv(&choice, 1, MPI_INT, 0, TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }
    return choice == 1;
}
