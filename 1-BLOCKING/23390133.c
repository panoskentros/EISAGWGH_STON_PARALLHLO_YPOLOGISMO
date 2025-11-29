#include <stdio.h>
#include <stdlib.h>
#include "mpi.h"
#include <stdbool.h>
#define TAG 999
bool wantsToContinue(int rank); // δηλωση συναρτησης που ελεγχει αν ο χρηστης θελει να συνεχισει

int main(int argc, char *argv[]) {
    MPI_Init(&argc, &argv);

    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank); // αποθηκευω στην μεταβλητη rank το rank της διεργασιας. Για καθε διεργασια η  μεταβλητη προφανως λαμβανει και διαφορετικο χωρο στη μνημη
  

   int * local_arr; // πινακας με τα τοπικα στοιχεια της καθε διεργασιας
   int * local_delta; // πινακας με το δελτα καθε διεργασιας
   int local_max; // μεταβλητη που υπολογιζει ποσα στοιχεια θα εχει καθε διεργασια, μεταβλητη που υπολογιζει το max καθε διεργασιας
   float local_var=0.0,local_sum; // μεταβλητη που υπολογιζει το τοπικο var και το τοπικο sum
   
    while (wantsToContinue(rank)) { // οσο ο χρηστης θελει να προχωρησει εκτελειται ο κωδικας εντος του while 
        if (rank == 0) { // εδω θα μπει η διεργασια με rank 0 
            int size,n; // μεταβλητη που θα βαλουμε τον αριθμο των διεργασιων, μεταβλητη με τον αριθμο των στοιχειων
            MPI_Comm_size(MPI_COMM_WORLD, &size); // αποθηκευουμε στη θεση μνημης της μεταβλητης size τον ακεραιο αριθμο των διεργασιων 
            fprintf(stdout,"Δώσε αριθμό στοιχείων: \n");
            scanf("%d",&n); // αποθηκευουμε στη θεση μνημης της μεταβλητης size τον ακεραιο αριθμο των στοιχειων του διανυσματος
        
            int base = n / size; // ελαχιστος αριθμος στοιχειων ανα διεργασια
            int rem  = n % size; // ειναι τα υπολοιπομενα στοιχεια αν n % size != 0 
            int rankZeroElements; // τα στοιχεια που θα υπολογισει η διεργασια 0. δεν ειναι απαραιτητη η μεταβλητη αυτη
            rankZeroElements = base + (rem > 0 ? 1 : 0); // δηλαδη p = 0 < rem οπως βλεπουμε και παρακατω. Ετσι καταφερνω την ομοιομορφη κατανομη των στοιχειων
            // πχ αν n=9 size=4 τοτε base=2, η συνθηκη rem=1 > 0 ειναι αληθης αρα η διεργασια με ρανκ 0 θα υπολογισει 3 στοιχεια και οι υπολοιπες 2
            // πχ αν n=10 size=4 τοτε base=2, η συνθηκη rem=2 > 0 για τη διεργασια με ρανκ 0 και 1 αρα οι δυο πρωτες διεργασιες θα υπολογισουν 3 στοιχεια η καθε μια και οι δυο τελευταιες διεργασιες απο 2 η καθε μια
            local_arr = (int *)malloc(rankZeroElements*sizeof(int)); // δεσμευω την απαραιτητη μνημη για να χωρεσουν τα στοιχεια της διεργασιας με rank 0. Ξερω οτι παντα η διεργασια με ρανκ 0 θα υπολογιζει στοιχεια πληθους base
            if(local_arr==NULL){ // ελεγχος δεσμευσης μνημης
                printf("Σφάλμα στην δέσμευση μνήμης\n");
                exit(-1); // στην αποτυχια τερματιζω το προγραμμα
            }

            for (int p = 0; p < size; p++) {
                int elementsPerProc = base + (p < rem ? 1 : 0); // υπολογισμος των στοιχειων ανα διεργασια
                int *buffer = malloc(elementsPerProc * sizeof(int));// δεσμευω την απαραιτητη μνημη. Δεν χρειαζεται να δεσμευσω πινακα ισου μεγεθους με το size.
                // δεσμευω την απαραιτητη μνημη για να χωραει ο αριθμος των στοιχειων ανα διεργασια

                for (int j = 0; j < elementsPerProc; j++) { // διαβαζω τα στοιχεια απο το χρηστη
                    int val;
                    printf("Δώσε αριθμό για το στοιχείο %d: \n", p * base + j + 1);
                    scanf("%d", &val);
                    buffer[j] = val; // τα αποθηκευω προσωρινα στο buffer 
                }

                if (p == 0) { // στη πρωτη επεναληψη διεργασια τα αποθηκευει στο local_arr. Το p δεν ειναι το ρανκ της τρεχουσας διεργασιας αλλα ο αριθμος της επαναληψης. 
                    //δηλαδη αν p == 1 αυτο σημαινει οτι τα στοιχεια που διαβασα πρεπει να τα στειλω στη διεργασια με rank 1. Η διεργασια που τα στελνει ειναι η διεργασια με ρανκ 0
                    // αυτο ειναι αντιληπτο απτη γραμμη 21 οπου εχω ελεγξει το ρανκ της διεργασιας. Επειδη p=0 και rank = 0 αντι να τα στειλω απλως τα αποθηκευω σε ενα τοπικο πινακα
                    for (int j = 0; j < rankZeroElements; j++)
                        local_arr[j] = buffer[j];
                } else { // εδω rank = 0 παντα αλλα p != 0. Αυτό σημαίνει οτι η τρέχουσα διεργασία με ρανκ 0 πρεπει στειλει τα στοιχεια που διαβασε στη διεργασια με rank == p
                    MPI_Send(&elementsPerProc, 1, MPI_INT, p, TAG, MPI_COMM_WORLD); // αρχικα στελνω τον αριθμο των στοιχειων ωστε να ξερει η διεργασια p ποσα στοιχεια να διαβασει στο επομενο βημα
                    MPI_Send(buffer, elementsPerProc, MPI_INT, p, TAG, MPI_COMM_WORLD); // εδω στελνω τα στοιχεια του πινακα buffer που διαβασα
                }
                free(buffer); //  εδω ειναι αναγκαιο να αποδεσμευσω την μνημη του buffer ωστε στην επομενη επαναληψη να δεσμευσω καινουρια μνημη για τα νεα δεδομενα της επομενης διεργασιας
            }
            
            local_max = local_arr[0]; //  η διεργασια με ρανκ 0 βαζει προσωρινα για μαξ το πρωτο στοιχειο του πινακα
            local_sum = local_arr[0]; // και για αθροισμα παλι το πρωτο στοιχειο

            for (int i = 1; i < rankZeroElements; i++) { // ετσι μπορω να γλυτωσω 1 επαναληψη. Η διεργασια με ρανκ 0 εχει παντα πληθος base στοιχεια
                local_sum = local_sum + local_arr[i]; // προσθετω στο αθροισμα καθε στοιχειο του πινακα
                if(local_arr[i]>local_max) // αν το τρεχον στοιχειο του πινακα ειναι μεγαλυτερο απτο μαξ τοτε το αναθετω
                    local_max = local_arr[i];
            }
            float other_rank_sum, // το αθροισμα των αλλων διεργασιων καθως και το συνολικο αθροισμα
            total_sum = local_sum; // αναθετω για αρχικη τιμη του συνολικου αθροισματος το αθροισμα της διεργασιας 0 
            int other_rank_max; // το μεγιστο στοιχειο των αλλων διεργασιων
            int total_max=local_max; // το μεγιστο στοιχειο ολων των διεργασιων
            for (int i = 1; i < size; i++) { // ετσι, γλυτωνω παλι 1 επαναληψη αλλα και 1 if(i==0)  
                MPI_Recv(&other_rank_sum, 1, MPI_FLOAT, i, TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE); // λαμβανω το αθροισμα της διεργασιας i 
                total_sum += other_rank_sum; // και το προσθετω στο συνολικο αθροισμα
                MPI_Recv(&other_rank_max, 1, MPI_INT, i, TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE); // λαμβανω το μεγιστο στοιχειο της διεργασιας i
                if(other_rank_max>total_max) //αν ειναι μεγαλυτερο απο το συνολικο μεγιστο, το αναθετω
                    total_max = other_rank_max;
            }
            float total_avg = (float)total_sum / n; // κανω cast σε float για να μη χαθει η ακριβεια
            printf("Μέσος όρος = %f\n", total_avg); // εκτυπωνω μυνηματα
            printf("Μέγιστος όρος: %d\n",total_max);

            for (int i = 1; i < size; i++)
                MPI_Send(&total_avg,1,MPI_FLOAT,i,TAG,MPI_COMM_WORLD); // στελνω τη συνολικη μεση τιμη σε ολες τις διεργασιες επειδη θα χρειαστει σε επομενο ερωτημα
            
            for (int i = 0; i < rankZeroElements; i++)
            {
                local_var += (local_arr[i] - total_avg) * (local_arr[i] - total_avg); // υπολιγιζω για το ερωτημα (γ) το var της διεργασιας με rank == 0
            }
            float other_rank_var, // το var των αλλων διεργασιων
            total_var = local_var; // το συνολικο var. αρχικοποιω με το τοπικο var για να γλυτωσω 1 επαναληψη και 1 if
            for (int i = 1; i < size; i++){
                MPI_Recv(&other_rank_var,1,MPI_FLOAT,i,TAG,MPI_COMM_WORLD, MPI_STATUS_IGNORE);  // διαβασω το var των αλλων διεργασιων που ειναι 1 μεταβλητη τυπου MPI_FLOAT
                total_var += other_rank_var; // και το προσθετω στο συνολικο var
            }  
            total_var = total_var / n; // υπολογιζω το συνολικο var διαιρωντας δια n  
            printf("var = %f \n",total_var);

            for (int i = 1; i < size; i++)
                MPI_Send(&total_max,1,MPI_INT,i,TAG,MPI_COMM_WORLD); // στελνω το τη μεγιστη τιμη m σε ολες τις διεργασιες επειδη θα χρειαστει για το (δ) ερωτημα 
            

            // δ ερώτημα
            int *delta = (int *)malloc(n * sizeof(int)); // δεσμευω μνημη για τον πινακα δελτα
      
            int offset = 0;
            for (int p = 0; p < size; p++) {
                int elementsPerProc = base + (p < rem ? 1 : 0);
                if (p == 0) {
                    for (int i = 0; i < elementsPerProc; i++)
                        delta[i] = (local_arr[i] - total_max) * (local_arr[i] - total_max);
                } else {
                    MPI_Recv(&delta[offset], elementsPerProc, MPI_INT, p, TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                }
                offset += elementsPerProc;
            }

            printf("Δ vector: ");
            for (int i = 0; i < n; i++)
                printf("%d ", delta[i]); // εκτυπωνω στο συνολικο δελτα
            printf("\n");

            free(delta);
        

        }else{ // διεργασιες με rank != 0
            int elems;
            MPI_Recv(&elems, 1, MPI_INT, 0, TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE); // η διεργασια σταματαει την εκτελεση της εδω, μεχρι να λαβει το πληθος των στοιχειων (ένας ακέραιος) απο την πρώτη διεργασια (με ρανκ 0) 
            local_arr = (int *)malloc(sizeof(int)*elems); // δεσμευει την απαραιτητη μνημη για να χωρεσουν τα στοιχεια
            if(local_arr==NULL){ // ελεγχος δεσμευσης 
                printf("Σφάλμα στην δέσμευση μνήμης\n");
                exit(-1);
            }
            MPI_Recv(local_arr, elems, MPI_INT, 0, TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE); // σταματαει την εκτελεση της εδω μεχρι να λαβει τον πινακα μεγεθους elems με τα στοιχεια απτη διεργασια με ρανκ 0
            local_max = local_arr[0]; // θετω το τοπικο μεγιστο με το πρωτο στοιχειο του πινακα για να γλυτωσω 1 επαναληψη
            local_sum = local_arr[0];
            for (int i = 1; i < elems; i++){ 
                local_sum = local_sum + local_arr[i]; // προσθετω το στοιχειο στο αθροισμα
                if(local_arr[i]>local_max) // αν το στοιχειο ειναι μεγαλυτερο απτο μεγιστο τοτε αυτό θα γινει το πλεον μεγιστο
                    local_max = local_arr[i];
            }
            MPI_Send(&local_sum, 1, MPI_FLOAT, 0, TAG, MPI_COMM_WORLD); // η διεργασια σταματαει την εκτελεση της εδω μεχρι να στειλει το τοπικο αθροισμα στη διεργασια με ρανκ 0
            MPI_Send(&local_max, 1, MPI_INT, 0, TAG, MPI_COMM_WORLD); // η διεργασια σταματαει την εκτελεση της εδω μεχρι να στειλει το τοπικο μεγιστο στην διεργασια με ρανκ 0
            float avg;
            MPI_Recv(&avg, 1, MPI_FLOAT, 0, TAG, MPI_COMM_WORLD,MPI_STATUS_IGNORE); // η διεργασια σταματαει την εκτελεση της εδω μεχρι να λαβει τη μεση τιμή του ερωτηματος (α) απο τη διεργασια με ρανκ 0
            for (int i = 0; i < elems; i++)
            {
                local_var += (local_arr[i] - avg) * (local_arr[i] - avg); // υπολογισμος της ταυτοτητας
            }
            MPI_Send(&local_var,1,MPI_FLOAT,0,TAG,MPI_COMM_WORLD); // η διεργασια σταματαει την εκτελεση της εδω μεχρι  να στειλει το τοπικο var στη διεργασια με ρανκ 0
            int total_max;
            MPI_Recv(&total_max,1,MPI_INT,0,TAG,MPI_COMM_WORLD,MPI_STATUS_IGNORE); // η διεργασια σταματαει την εκτελεση της εδω μεχρι να λαβει το ολικό μεγιστο του ερωτηματος (β) απο τη διεργασια με ρανκ 0
            local_delta = (int *)malloc(elems*sizeof(int));  // δεσμευση της απαραιτητης μνημης
            if(local_delta==NULL){ // ελεγχος δεσμευσης μνημης
                printf("Σφάλμα στην δέσμευση μνήμης\n");
                exit(-1);
            }
            for (int i = 0; i < elems; i++) // υπολογισμος της ταυτοτητας του δελτα για καθε στοιχειο
                local_delta[i] = (local_arr[i] - total_max) * (local_arr[i] - total_max);
            
            MPI_Send(local_delta,elems,MPI_INT,0,TAG,MPI_COMM_WORLD);  // η διεργασια σταματαει την εκτελεση της εδω μεχρι να στειλει το πινακα που περιεχει το δελτα του καθε στοιχειου στη διεργασια με ρανκ 0
            
            free(local_delta);
            
        }
        free(local_arr); // αποδεσμευση της μνημης του πινακα local_arr ωστε να γινει σωστο malloc σε περιπτωση συνεχειας της εκτελεσης της while λουπας
    }
    
    MPI_Finalize(); // τερματισμος του mpi
    return 0; // επιστροφη εγκυρου status στην γονικη διεργασια για την εγκυρη εκτελεση του προγραμματος
}

bool wantsToContinue(int rank) {
    int choice;
    if (rank == 0) { // η διεργασια με rank 0 αποφασιζει για την εππιλογη: συνέχισε ή έξοδος
        do {
            fprintf(stdout, "1. Συνέχεια\n2. Έξοδος\n");
            scanf("%d", &choice);
        } while (choice != 1 && choice != 2); // πρεπει η επιλογη που δωση ο χρηστης να ειναι 1 ή 2 διαφορετικα ξαναδιαβασε
        int size;
        MPI_Comm_size(MPI_COMM_WORLD, &size); // παιρνει τον αριθμο των διεργασιων και στελνει σε ολες την επιλογη που εκανε ο χρηστης
        for (int i = 1; i < size; i++) // ξεκιναει απτο 1 για να μην στειλει στον εαυτο του
            MPI_Send(&choice, 1, MPI_INT, i, TAG, MPI_COMM_WORLD); // στελνει σε ολες τις διεργασιες την επιλογη, δηλαδη 1 μετβλητη τυπου MPI_INT 
    } else {
        MPI_Recv(&choice, 1, MPI_INT, 0, TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE); // ολες οι διεργασιες σταματανε την εκτελεση τους εδω, περιμενωντας απο την διεργασια με ρανκ 0 την επιλογη του χρηστη
    }
    return choice == 1; // επιστρεφουμε την αποτιμηση της συνθηκης (true/false), δηλαδη αν ο χρηστης επιλεξει συνεχεια που είναι ο ακεραιος 1 τοτε επιστρεφουμε true αλλιως false 
}
