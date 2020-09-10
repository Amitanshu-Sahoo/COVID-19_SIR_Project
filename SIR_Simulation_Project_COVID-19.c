#include<stdio.h>
#include<stdlib.h>
#include<time.h>
#define MAX_VERTICES 10000
#define MAX_EDGES 3000

struct node{
    unsigned char** person ;
    int position ;
    char status ;
    char action ;
    int time ;
    struct node* next ;
    struct node* prev ;
};

/* EXPLAINING THE STRUCTURE VARIABLES

    The structure for the queue contains 7 features :-

    unsigned char ** person = this variable stores the node from the graph in the structure .
    int position =  There is a number associated with each of the node in the graph .positon variable stores that number
    char status = This variable stores the status of the in the queue which can be susceptible, infected or recovered .
    char action = This variable stores the action to be taken on the node in consideration. It is the same as the event for the node.
    int time =  It stores the time when the node will be entitled to the action .
    next and prev =  these are the two pointers that link the node to the next and previous nodes .
*/

/* EXPLAINING THE DIFFERENT TYPES OF NODES

    There are only certain combinations of the status and action/event that can enqueued in the priority queue

    1. Nodes with infected status and event/action as transmit :- These are some of the nodes that are initially present in the queue when the algorithm starts
                                                                  It denotes the people who are infected at time 0. The action transmit here means that they were
                                                                  infected at time 0 by an unknown source.

    2. Nodes with susceptible status and event/action as transmit :- In these nodes the time variable denotes the time of the event in the node as the time when the event is applied to the node itself .
                                                                     This mean that at the mentioned time the node will get infected and will be removed from the susceptible list and put in the infected list .
                                                                     Until it comes to priority in the queue it remains as susceptible transmit node in the queue.

    3. Nodes with Infected status and event/action as transmit :- These nodes are made for the people in graph once their Susceptible/Transmit or Infected/Transmit node come to priority in the queue
                                                                  The nodes stores the time when the person will recover after getting infected .
*/

typedef struct node node ;      // Alias name for the struct
typedef unsigned char vertex;       // Alias name for unsigned char data type
int time_max = 300 ;    // denoting the maximum time which is 300 days for which the algorithm will compute the spread
double gama = 0.2 ;     // bias for recovery
double tau = 0.5 ;      // bias for transmit
int infection_curve_day[301] ;      // Stores the day numbers
int infection_curve_susceptible[301] ;      // Stores the number of susceptibles in each of the days until the algorithm runs
int infection_curve_infected[301] ;     // Stores the number of infecteds in each day until the algorithm runs
int infection_curve_recover[301] ;      // Stores the number of recovered nodes until the algorithm is run
int max_num_vertices ;

int coin_toss(double bias) ;     // this function simulates the tossing of the coin and returns 0 or 1 based on the bias of the coin
int predicted_days(double bias) ;        // this function uses the coin toss function to repeatedly toss a coin and returns the number of tosses after which head occurs
node* PQ_Insert(node* head, unsigned char** person_num, int time, char stat, char eve, int num) ; // used to insert event in the priority queue according to time
node* PQ_Dequeue(node* head) ; // this function dequeues the node in the front of the queue after the event is finished
node* initiate_infection(unsigned char** array[],int number_of_vertices, int S[],int I[], int R[], int Q[]) ; // starts the infection in the population
node* initiate_transmit_susceptibles(unsigned char** array[], node* priority_queue, int S[],int I[], int R[], int Q[], double tau, double gama, int max_num_edges, int max_num_vertices) ; // converts the normal people in the neighbourhood of infecteds into susceptible status and transmit event
node* initiate_recovery(node* priority_queue, int S[],int I[], int R[],double gama, int recover_time) ; // adds a recovery event for a infected/susceptible status transmit event and dequeues the node the highest priority node
node* recover(node* priority_queue, int S[],int I[], int R[]) ; // if the node in priority in the queue has event recover and status infected the node is dequeued
node* obtain_node(node* priority_queue, char status, char action, int position) ; // returns the time associated with the node if it is present in the queue else returns -1
void print_SIR_status(int S[],int I[], int R[], int max_num_vertices) ; // prints the number of susceptible , infected and recovered people at the time when the function is called
void update_infection_curves(node* head, int infection_curve_susceptible[], int infecton_curve_infected[], int infection_curve_recover[],int S[], int I[], int R[], int max_num_vertices) ; // updates the infection curve variables
void update_non_event_days(int infection_curve_susceptible[], int infection_curve_infected[], int infection_curve_recover[]) ; // certain days in the infection curve might just remain blank because the curve is updated based on events. This function assigns the record of the last updated day to the non-event days
void print_infection_curves(int infection_curve_day[],int infection_curve_susceptible[],int infection_curve_infected[], int infection_curve_recover[]) ; // prints the infection variables


node* PQ_Insert(node* head, unsigned char** person_num, int time, char stat, char act, int num){

    node* newnode = NULL ;                                                     // The information of the newnode to be inserted is stored in a new struct instance
    newnode = (struct node*)malloc(sizeof(struct node)) ;
    newnode->person = person_num ;
    newnode->time = time ;
    newnode->status = stat ;
    newnode->action = act ;
    newnode->position = num ;

    node* temp = NULL ;     // temp is the head of the priority queue
    temp = head ;

    if(temp==NULL){             // If the priority queue is null then the newnode is inserted as the first node and assigned to temp .Then temp is returned
        newnode->next = NULL ;
        newnode->prev = NULL ;
        temp = newnode ;
        return temp ;
    }
    else{
        node* flag_pos = NULL ;

        while(temp != NULL){             // If the queue is not null then the position for the newnode to be inserted is found according to the time of event
            if(temp->time <= time){      // This is done by manipulating the position of temp and another node called flag pose
                flag_pos = temp ;
                temp = temp->prev ;
            }
            else if(temp->time > time){
                break ;
            }
        }

        if(temp==NULL){                 // If after manipulation temp is equal to null then the newnode has to be inserted at the end of the queue as the last event
            newnode->next = flag_pos ;
            flag_pos->prev = newnode ;
            newnode->prev = NULL ;
            return head ;
        }
        else if(flag_pos!=NULL){       // If the above if statement fails that means the new node has to be inserted somewhere in between or else in the starting .
            flag_pos->prev = NULL ;    // In this case flag pose is not equals to null the node has to be inserted somewhere in between
            temp->next = NULL ;        // therefore the required pointer manipulation is done.
            flag_pos->prev = newnode ;
            newnode->next = flag_pos ;
            temp->next = newnode ;
            newnode->prev = temp ;
            return head ;               // head is returned after manipulation
        }
        else{
            temp->next = newnode ;      // if the above two conditions fail then the node has to be inserted at the beginning of the queue which is done by this condition .
            newnode->prev = temp ;
            newnode->next = NULL ;
            head = newnode ;
            return head ;
        }
    }
}

node* PQ_Dequeue(node* head){
    if(head!=NULL){     // Dequeue can only be done from the head of the queue .It can only be done if the queue is not null .
        node* temp = NULL ;     // If the queue is not null then head is dequeued with the given pointer manipulation
        temp = head ;
        head = head->prev ;
        temp->prev = NULL ;
        if(head!=NULL){        // If after the dequeue head is not null i.e if the dequeued node was the last node in the queue ,only then head->next is assigned to null
            head->next = NULL ;}
        free(temp) ;
        return head ;       // head of the queue is returned after manipulation
    }
    else{
        return head ;       // If the queue was already null then it is returned
    }
}

int coin_toss(double bias){

    int num = rand()%((int)(1/bias));      // randomly choosing a decimal number between 0 and 1
    if(num == 0){
        return 1 ;      // 1 is returned for heads
    }
    else{
        return 0 ;      // 0 is returned for tails
    }
}

int predicted_days(double bias){
    int num_days = 0 ;      // stores the number of days to the event

    while(1){
        if(coin_toss(bias)){        // The loop adds up the number of days for which heads does not come and returns when heads(1) comes up.
            return num_days+1 ;
        }
        else{
            ++num_days ;
        }
    }
}

node* initiate_infection(unsigned char** array[], int number_of_vertices,int S[],int I[], int R[], int Q[] ){

    srand(time(NULL)) ;
    int num_infect ;

    if(number_of_vertices == 0){              // These if else statements decide the number of people to be infected according to the number of vertices .
        return NULL ;          }
    if(number_of_vertices<=100){
        num_infect = 2 ;      }
    else if(number_of_vertices>100  && number_of_vertices <1000){
        num_infect = 3 ;
    }
    else if(number_of_vertices > 1000){
        num_infect = 4 ;
    }
    node* queue = NULL ;
    for(int i=0; i<num_infect; ++i){          // The for loop randomly infects the decided number of people from the graph. The ideology behind infecting a person
        int num = rand()%number_of_vertices ; // is removing the person from the susceptible list and putting them in the infected list and passing an infected/transmit
        S[num] = 0 ;                          // node in the queue for the person.
        I[num] = 1 ;
        R[num] = 0 ;
        Q[num] = 1 ;
        queue = PQ_Insert(queue,array[num],0,'I','T',num) ;
    }
    return queue ;
}

node* initiate_recovery(node* priority_queue, int S[],int I[], int R[], double gama,int recover_time){

    node* temp = NULL ;         // This function simply enqueues an infected/recovery node in the queue if the conditions are satisfied in the initiate_transmit_susceptible function
    temp = priority_queue ;
    temp = PQ_Insert(temp,temp->person,recover_time,'I','R',temp->position) ;

    return temp ;

}

node* recover(node* priority_queue, int S[],int I[], int R[]){

    node* temp = NULL ;         // This function is same as the process_REC function . Once an infected/recovery node comes to priority in queue, the status of the person
    temp = priority_queue ;     // associated with the node is changed from infected list to recovery list and node is dequeued form the queue.
    S[temp->position] = 0 ;
    I[temp->position] = 0 ;
    R[temp->position] = 1 ;
    update_infection_curves(temp,infection_curve_susceptible,infection_curve_infected,infection_curve_recover,S,I,R,max_num_vertices) ;     // updating the infection curves variables
    temp = PQ_Dequeue(temp) ;
    return temp ;

}

node* obtain_node(node* priority_queue, char status, char action, int position){

    node* temp = NULL ;         // Given the status ,action and position of  a certain node in the queue this function locates the desired node in the queue and returns it
    temp = priority_queue ;

    while(temp!=NULL){
        if((temp->position==position) && (temp->status == status) && (temp->action==action)){
            return temp ;
        }
        else{
            temp = temp->prev ;
        }
    }

    if(temp==NULL){
        return NULL;
    }
}

node* initiate_transmit_susceptibles(unsigned char** array[],node* priority_queue, int S[],int I[], int R[],int Q[] , double tau, double gama, int max_num_edges,int max_num_vertices){

    /* This is the main function in the algorithm . It does the combine work of process_trans and find_trans. Once a node of the type susceptible/Transmit or infected/transmit
       comes to priority this function initiates declares them as infected and initiates an infected/recovery node for them in the queue. It also initiates susceptible/transmit
       nodes for the neighbours of the node. These enqueue process are subject to certain conditions as explained below.
    */

    node* temp = NULL ;
    temp = priority_queue ;
    int time_of_infection ;
    int source_recovery_time ;

    S[temp->position] = 0 ;     // updating the current status of the population
    I[temp->position] = 1 ;
    R[temp->position] = 0 ;


    source_recovery_time = temp->time + predicted_days(gama) ;      // calculating the recovery time for the person in priority
    if(source_recovery_time < time_max){        // If its recovery time is within the time_max days, only then an infected/recovery node for the person is enqueued
        temp = initiate_recovery(temp,S,I,R,gama,source_recovery_time) ;
    }

    for(int i = 0; i< max_num_edges; ++i){      //************************************************************************************************************************
        if(array[temp->position][i] != NULL){
            for(int j=0; j<max_num_vertices; ++j){
                if(array[temp->position][i]==array[j]){
                    if(S[j]){
                        time_of_infection = temp->time + predicted_days(tau) ;
                        if(Q[j]){
                            node* needed_node = obtain_node(temp,'S','T',j) ;
                            if((time_of_infection < source_recovery_time) && (time_of_infection < time_max) && (time_of_infection < needed_node->time)){

                                node* left = NULL ;
                                node* right = NULL ;
                                if((needed_node->next != NULL) && (needed_node->prev!=NULL)){                       // This part of the function in between the star marked area finds out the neighbours of the person in priority
                                    left = needed_node->next ;                                                      // which are not NULL .Q is the array thats stores 1 opposite to the person who has already come into the queue
                                    right = needed_node->prev ;                                                     // as a susceptible/transmit or infected/transmit node.Hence it helps to check if the neighbour is already in the
                                    left->prev = right ;                                                            // queue. If the neighbour node is already in the queue then the time of infection of the node, source recovery time
                                    right->next = left ;                                                            // and max_time is compared to the currently generated infection time for the node. If the currently generated time
                                    needed_node->next = NULL ;                                                      // is less than the node's infection time,source recovery time and max_time the node's position in the queue
                                    needed_node->prev = NULL ;                                                      // is changed to a new position with the current infection time .If the neighbour node was not already in queue then the currently generated
                                    free(needed_node) ;                                                             // infection time is compared to source recovery time and max time .If it is less than both the values then neighbour the node is
                                }                                                                                   // enqueued . After all the neighbours of the person are traversed, the for loop ends. The infection curve variables are updated.
                                else if((needed_node->next != NULL) && (needed_node->prev==NULL)){                  // The person's transmit event node is dequeued.
                                    left = needed_node->next ;
                                    left->prev = NULL ;
                                    needed_node->next = NULL ;
                                    free(needed_node) ;
                                }


                                temp = PQ_Insert(temp,array[j],time_of_infection,'S','T',j) ;
                                Q[j] = 1 ;
                            }
                        }
                        else if(!Q[j]){
                            if((time_of_infection < source_recovery_time) && (time_of_infection < time_max)){
                                temp = PQ_Insert(temp,array[j],time_of_infection,'S','T',j) ;
                                Q[j] = 1 ;
                            }
                        }

                        break ;

                    }
                    else{
                        break ;
                    }
                }
            }
        }
    }
    //***********************************************************************************************************************************************************************
    update_infection_curves(temp,infection_curve_susceptible,infection_curve_infected,infection_curve_recover,S,I,R,max_num_vertices) ;
    temp = PQ_Dequeue(temp) ;
    return temp ;

}

void print_SIR_status(int S[],int I[], int R[], int max_num_vertices){

    int S_NUM = 0 ;                     // This function calculates the total number of susceptibles, infected and recovered nodes at the time when it is called and prints them
    int I_NUM = 0 ;
    int R_NUM = 0 ;

    for(int i=0; i<max_num_vertices; ++i){      // This for loop calculates the total of each category and stores them in the three variables initialized at the starting

        if(S[i]==1){
            ++S_NUM ;
        }
        if(R[i]==1){
            ++R_NUM ;
        }
        if(I[i]==1){
            ++I_NUM ;
        }
    }

    printf("\n Susceptibles = %d, Recovered = %d, Infected = %d",S_NUM,R_NUM,I_NUM) ;
}

void update_infection_curves(node* head, int infection_curve_susceptible[], int infecton_curve_infected[], int infection_curve_recover[],int S[], int I[], int R[], int max_num_vertices){

    int S_NUM = 0 ;         // This function has the same structure as the print_SIR_status function. Only differences is that this function updates the
    int I_NUM = 0 ;         // infection curve variables on the event days
    int R_NUM = 0 ;

    for(int i=0; i<max_num_vertices; ++i){

        if(S[i]==1){
            ++S_NUM ;
        }
        if(R[i]==1){
            ++R_NUM ;
        }
        if(I[i]==1){
            ++I_NUM ;
        }
    }

    infection_curve_susceptible[head->time] = S_NUM ;       // updating the variables
    infection_curve_infected[head->time] = I_NUM ;
    infection_curve_recover[head->time] = R_NUM ;

}

void update_non_event_days(int infection_curve_susceptible[], int infection_curve_infected[], int infection_curve_recover[]){

    for(int l=0; l<301; ++l){
        if((infection_curve_susceptible[l] == 0) && (infection_curve_infected[l] == 0) && (infection_curve_recover[l] == 0)){       // On some days no events are scheduled according to the
            infection_curve_susceptible[l] = infection_curve_susceptible[l-1] ;                                                     // algorithm. Those days just carry forward the record of the
            infection_curve_infected[l] = infection_curve_infected[l-1] ;                                                           // last event day in the time span. This function helps
            infection_curve_recover[l] = infection_curve_recover[l-1] ;                                                             // in doing so.
        }
    }
}

void print_infection_curves(int infection_curve_day[], int infection_curve_susceptible[], int infection_curve_infected[], int infection_curve_recover[]){

    printf("\n\n") ;        // This function prints the infection variables for each each day of the time span.
    for(int i=0 ; i<301; ++i){
        printf("\nday : %d", infection_curve_day[i]) ;
        printf("\nSusceptibles : %d", infection_curve_susceptible[i]) ;
        printf("\nInfected : %d", infection_curve_infected[i]) ;
        printf("\nRecovered : %d", infection_curve_recover[i]) ;
        printf("\n\n") ;
    }
}

int main(){

//***********************************************************************************************************************************GRAPH_CONSTRUCTOR
    srand ( time(NULL) );
    int numberOfVertices =  rand() % MAX_VERTICES;                   // The graph function is taken from the link given in the project explanation pdf
    srand ( time(NULL) );
    int maxNumberOfEdges =  rand() % MAX_EDGES;

    if( numberOfVertices == 0)
        numberOfVertices++;
    vertex ***graph;
    //printf("Total Vertices = %d, Max # of Edges = %d\n",numberOfVertices, maxNumberOfEdges);

    if ((graph = (vertex ***) malloc(sizeof(vertex **) * numberOfVertices)) == NULL){
        printf("Could not allocate memory for graph\n");
        exit(1);
    }

    int vertexCounter = 0;
    int edgeCounter = 0;

    for (vertexCounter = 0; vertexCounter < numberOfVertices; vertexCounter++){
        if ((graph[vertexCounter] = (vertex **) malloc(sizeof(vertex *) * maxNumberOfEdges)) == NULL){
            printf("Could not allocate memory for edges\n");
            exit(1);
        }

        for (edgeCounter = 0; edgeCounter < maxNumberOfEdges; edgeCounter++){
            if ((graph[vertexCounter][edgeCounter] = (vertex *) malloc(sizeof(vertex))) == NULL){
                printf("Could not allocate memory for vertex\n");
                exit(1);
            }
        }
    }
    vertexCounter = 0;edgeCounter = 0;
    for (vertexCounter = 0; vertexCounter < numberOfVertices; vertexCounter++){
        //printf("%d:\t",vertexCounter);
        for (edgeCounter=0; edgeCounter < maxNumberOfEdges; edgeCounter++){
            if (rand()%2 == 1){
                int linkedVertex = rand() % numberOfVertices;
                graph[vertexCounter][edgeCounter] = graph[linkedVertex];
                //printf("%d, ", linkedVertex);
            }

            else{
                graph[vertexCounter][edgeCounter] = NULL;
            }
        }
        //printf("\n");
    }
//*********************************************************************************************************************************GRAPH_CONSTRUCTOR
//******************************************************************************************************************************************FAST_SIR
    srand ( time(NULL) );
    max_num_vertices = numberOfVertices ;

    unsigned char** people[numberOfVertices] ;      // Initializing the S,I,R current time update variables
    int S[numberOfVertices] ;                       // These variables store 1 or 0 opposites to the position of the person from the graph in the array
    int I[numberOfVertices] ;                       // If the person is at position num and is infected then I[num] = 1 ,S[num] = 0 , R[num] = 0 .
    int R[numberOfVertices] ;
    int Q[numberOfVertices] ;


    for(int i=0;i<numberOfVertices; ++i){           // Initializing the S array to 1 and others to zero because initially everybody is susceptible
        S[i] = 1 ;
        I[i] = 0 ;
        R[i] = 0 ;
        Q[i] = 0 ;
    }

    for(int j = 0; j < 301; ++j){       // Initialising the infection variables to 0 for each day at the starting
        infection_curve_day[j] = j ;
        infection_curve_susceptible[j] = 0 ;
        infection_curve_infected[j] = 0 ;
        infection_curve_recover[j] = 0 ;
    }

    for(int i=0; i<numberOfVertices; ++i){
        people[i] = graph[i] ;      // people is just another name for the graph variable made in order for the algorithm to look more logical.
    }

    node* head_queue = NULL ;

    // Initiating infection in some people in the population .The initial source of infection is not taken into account as it is not human to human transmit
    // initially .

   head_queue = initiate_infection(people,numberOfVertices,S,I,R,Q) ;
   print_SIR_status(S,I,R,numberOfVertices) ;

   // Starting the pandemic in the population
   int last ;
   while(head_queue!=NULL){     // Based on the type of node in priority in the queue either of the functions are called to spread infection and recover the people

    if((head_queue->status == 'I' && head_queue->action == 'T') || (head_queue->status == 'S' && head_queue->action == 'T')){
        head_queue = initiate_transmit_susceptibles(people,head_queue,S,I,R,Q,tau,gama,maxNumberOfEdges,numberOfVertices) ;
    }
    else if(head_queue->status == 'I' && head_queue->action == 'R'){
       head_queue = recover(head_queue,S,I,R) ;
    }

    if(head_queue!=NULL){
        last  = head_queue->time ;
    }

   }

   update_non_event_days(infection_curve_susceptible,infection_curve_infected,infection_curve_recover) ;
   print_infection_curves(infection_curve_day,infection_curve_susceptible,infection_curve_infected,infection_curve_recover) ;
   print_SIR_status(S,I,R,numberOfVertices) ;
   printf("\n last event day : %d",last-1) ;

//******************************************************************************************************************************************FAST_SIR
}
