/*
 * Process Scheduler - CMP 697
 * City University of New York at Lehman College
 * @author: Marcos Davila
 * @date: 9/1/2013
 *
 * A C program that simulates four different short-term CPU schedulers.
 * The different algorithms that are simulated are FCFS, Round Robin,
 * Modified Round Robin and Modified Halfed Round Robin. Modified Halfed
 * Round Robin is an algorithm of my own devising.
 */

#define MAX_SIZE_QUEUE 20
#define STEADY_STATE 12
#define JOB_POOL_SIZE 500
#define PERCENT_LONG 65
#define SMALLEST_SMALL_PROC 1
#define LARGEST_SMALL_PROC 125	
#define SMALLEST_LARGE_PROC 151
#define LARGEST_LARGE_PROC 250

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#ifdef __unix__
    # include <unistd.h>
#elif defined _WIN32
    # include <windows.h>
    #define sleep(x) Sleep(x)
#endif

/* A Queue has five properties.
 *  - capacity stands for the maximum number of elements Queue can hold.
 *  - Size stands for the current size of the Queue.
 *  - elements is the array of elements.
 *  - front is the index of first element
 *  - rear is the index of last element
 */
typedef struct Queue
{
        int capacity;
        int size;
        int front;
        int rear;
        int *elements;
} Queue;

// Function Prototypes
void FCFS(Queue *ready, Queue *pool);
int front(Queue *Q);
void enqueue(Queue *Q, int element);
void dequeue(Queue *Q);
Queue* createQueue(int maxElements);
void transfer(Queue *Q, Queue *R, int amt);
void createJobPool();
void RoundRobin(Queue *ready, Queue *pool);
void ModifiedRoundRobin(Queue *ready, Queue *pool);
void ModifiedHalfedRoundRobin(Queue *ready, Queue *pool);
void incrementGrouping(int quanta_of_job, int *grouping);
int isGroupingFilled(int *grouping);
void copyQueue(Queue *dest, Queue *orig);

int main(){
	Queue *ready_queue;
	Queue *FCFS_queue, *RR_queue, *MRR_queue, *MHRR_queue;
	Queue *FCFS_pool, *RR_pool, *MRR_pool, *MHRR_pool;
	ready_queue = createQueue(MAX_SIZE_QUEUE);

	createJobPool();
	int i = 0;
	FILE *f;
	f = fopen("job_pool.txt", "r");

	// Copy the job pool data into another queue for each scheduling
    	// method and copy the queue of elements into another queue for
    	// each scheduling method
	printf("%s\n", "Generating jobs.");
    	FCFS_pool = createQueue(JOB_POOL_SIZE);
    	RR_pool = createQueue(JOB_POOL_SIZE);
    	MRR_pool = createQueue(JOB_POOL_SIZE);
    	MHRR_pool = createQueue(JOB_POOL_SIZE);

    	FCFS_queue = createQueue(MAX_SIZE_QUEUE);
    	RR_queue = createQueue(MAX_SIZE_QUEUE);
    	MRR_queue = createQueue(MAX_SIZE_QUEUE);
    	MHRR_queue = createQueue(MAX_SIZE_QUEUE);

    	for (i = 0; i < JOB_POOL_SIZE; i++){
    	    int quanta;
    	    fscanf(f, "%i", &quanta);
    	    enqueue(FCFS_pool, quanta);
    	    enqueue(RR_pool, quanta);
    	    enqueue(MRR_pool, quanta);
    	    enqueue(MHRR_pool, quanta);
    	}
	
    	// Transfer jobs to reach the steady state.
	printf("%s\n", "Transferring elements to reach the steady state.");
    	transfer(FCFS_queue, FCFS_pool, STEADY_STATE);
    	transfer(RR_queue, RR_pool, STEADY_STATE);
    	transfer(MRR_queue, MRR_pool, STEADY_STATE);
    	transfer(MHRR_queue, MHRR_pool, STEADY_STATE);
	
	srand(time(NULL));
	
    	// Now run each of the processes on the same exact data
	printf("%s\n", "Starting FCFS.");
    	FCFS(FCFS_queue, FCFS_pool);
	printf("%s\n", "Starting Round Robin");
    	RoundRobin(RR_queue, RR_pool);
	printf("%s\n", "Starting Modified Round Robin");
    	ModifiedRoundRobin(MRR_queue, MRR_pool);
	printf("%s\n", "Modified Half Round Robin");
    	ModifiedHalfedRoundRobin(MHRR_queue, MHRR_pool);

	printf("%s\n", "Freeing allocated memory.");
	// Free all allocated memory
	free(ready_queue);
	free(FCFS_queue);
	free(RR_queue);
	free(MRR_queue);
	free(MHRR_queue);
	free(FCFS_pool);
	free(RR_pool);
	free(MRR_pool);
	free(MHRR_pool);
	
	return 0;
}

// Fill the job pool with available jobs for the CPU to "work" on.
void createJobPool(){
    int lcv;
    FILE *file;
    file = fopen("job_pool.txt", "w+");

    for (lcv = 0; lcv < JOB_POOL_SIZE; lcv++){

        int longorshort = rand() % 100;
        int max, min, quanta;

        if (longorshort > PERCENT_LONG){
            max = LARGEST_SMALL_PROC;
            min = SMALLEST_SMALL_PROC;
        } else {
            max = LARGEST_LARGE_PROC;
            min = SMALLEST_LARGE_PROC;
        }

	quanta = (double) rand() / (RAND_MAX+1.0) * (max-min) + min;
        fprintf(file, "%i\n", quanta);
	}

	fclose(file);
}

// Move jobs from the ready_pool to the read_queue
void transfer(Queue *Q, Queue *R, int amt){
	int i;

	for (i = 0; i < amt; i++) {
	    int value = front(R);
		dequeue(R);
		enqueue(Q, value);
	}
}

// Supply the maximum size that a Queue can be and create a queue with predetermined
// fields. A pointer to the queue is returned.
Queue* createQueue(int maxElements){
        // Create a Queue
        Queue *Q;
        Q = (Queue *)malloc(sizeof(Queue));
        // Initialize its properties
        Q->elements = (int *)malloc(sizeof(int)*maxElements);
        Q->size = 0;
        Q->capacity = maxElements;
        Q->front = 0;
        Q->rear = -1;

        return Q;
}

// A method that allows us to push an element onto a given Queue Q. It will not push an element
// onto the queue if there is no space in the array for it and instead will print a message to
// the user alerting them that the queue is full. Queues are filled in a circular fashion.
void enqueue(Queue *Q, int element){
        // check if the queue is full
        if(Q->size == Q->capacity)
        {
                printf("Queue is Full\n");
        }
        else
        {
                Q->size++;
                Q->rear = Q->rear + 1;

				// Since the queue is being filled in a circular fashion if the queue is
				// fill the rear should equal the first element of the queue
				if (Q->rear == Q->capacity){
					Q->rear = 0;
				}
				// Queue the element at the rear
                Q->elements[Q->rear] = element;
        }
}

// A method that allows us to remove an element onto a given Queue Q. It will not pop an element
// onto the queue if the array is emptyand instead will print a message to
// the user alerting them that the queue is empty.
void dequeue(Queue *Q){
		// Check if the queue is empty
		if (Q->size == 0){
			printf("There are no more elements in the queue.n");
		} else {
        	// Since we are filling the queue in a circular fashion, removing an element
		// is equivalent to incrementing the front variable by one and decreasing the
		// "size" of the Queue. If we remove the last element from the queue, set the
		// front to the beginning of the buffer.
                Q->size--;
                Q->front++;

                if(Q->front==Q->capacity)
                {
                        Q->front=0;
                }
        }
}

// A method to get the next element in a queue.
int front(Queue *Q){
        if(Q->size==0)
        {
                printf("Queue is Empty\n");
                exit(0);
        }

        // Return the element which is at the front
        return Q->elements[Q->front];
}

void incrementGrouping(int quanta_of_job, int *grouping){
        if (quanta_of_job > 1 && quanta_of_job <= 5){
            grouping[0]++;
        } else if (quanta_of_job > 5 && quanta_of_job <= 10){
            grouping[1]++;
        } else if (quanta_of_job > 10 && quanta_of_job <= 15){
            grouping[2]++;
        } else if (quanta_of_job > 15 && quanta_of_job <= 20){
            grouping[3]++;
        } else if (quanta_of_job > 20 && quanta_of_job <= 25){
            grouping[4]++;
        } else if (quanta_of_job > 25 && quanta_of_job <= 30){
            grouping[5]++;
        } else if (quanta_of_job > 30 && quanta_of_job <= 35){
            grouping[6]++;
        } else if (quanta_of_job > 35 && quanta_of_job <= 40){
            grouping[7]++;
        } else if (quanta_of_job > 40 && quanta_of_job <= 45){
            grouping[8]++;
        } else if (quanta_of_job > 45 && quanta_of_job <= 50){
            grouping[9]++;
        } else if (quanta_of_job > 50 && quanta_of_job <= 55){
            grouping[10]++;
        } else if (quanta_of_job > 55 && quanta_of_job <= 60){
            grouping[11]++;
        } else if (quanta_of_job > 60 && quanta_of_job <= 65){
            grouping[12]++;
        }
}

// A First Come, First Serve scheduler implementation. By taking
// a pointer to a queue Q, the first element on the queue will be
// completed and removed. The FCFS method does 100 jobs before returning
void FCFS(Queue *ready, Queue *pool){
    int num_jobs_completed = 0;
    int MAX_NUM_JOBS = 100;
    int MIN_NUM_JOBS = 5;
    FILE *file1 = fopen("FCFSGrouping.txt","w+");
    FILE *file2 = fopen("FCFSWaitTime.txt", "w+");
    FILE *file3 = fopen("FCFSQueueSize.txt", "w+");
    FILE *file4 = fopen("FCFSTotalTime.txt", "w+");
    int *grouping = (int*) calloc (13, sizeof(int));
    int *wait_time = (int*) calloc (ready->capacity, sizeof(int));
    int total_time = 0;

    while(isGroupingFilled(grouping)){
        // Push the element out of the queue and record the time quanta
        int quanta_of_job = front(ready);
        dequeue(ready);

        // Sleep to simulate the PC being remaining busy for that amount of time.
        sleep(quanta_of_job);
        // Print out the information of how long the process' execution and wait time was.
        incrementGrouping(quanta_of_job, grouping);


        fprintf(file2, "%i\n", wait_time[0]);
        fprintf(file3, "%i\n", ready->size);
        fprintf(file4, "%i\n", total_time);

        total_time += quanta_of_job;

        // The time that it took for the current job to complete
        // should be factored in for all subsequent jobs. Jobs also
        // need to be shifted one to the left to account for the
        // current job no longer existing in the system.
        int i;
        for (i = 1; i < ready->size; i++){
            wait_time[i] += quanta_of_job;
        }

        for (i = 0; i < ready->size; i++){
            wait_time[i] = wait_time[i+1];
        }

        wait_time[ready->size - 1] = 0;

		// Transfer jobs if the amount left in the queue is too small. Devise a random number
		// to decide how many jobs should be added to the queue.
		if (ready->size < MIN_NUM_JOBS){
		    int size = ready->size;
			int num_jobs = (rand() % 15 - (ready->size)) + 1;
			transfer(ready, pool, num_jobs);

			for (i = 0; i < num_jobs; i++){
			    int loc = (i+size)-1;
                wait_time[loc] = 0;
			}
		}
    }

    int i;

    for (i = 0; i < 13; i++){
        fprintf(file1, "%i\n", grouping[i]);
    }

    fclose(file1);
    fclose(file2);
    fclose(file3);
    fclose(file4);
}

// Round Robin takes in a queue of job elements Q. Each element, regardless of how
// long they take to complete, will receive the same amount of CPU time. Each time
// the queue gets one pass-through, the size of the queue is checked and more elements
// are added to the queue if necessary.
void RoundRobin(Queue *ready, Queue *pool){

    int num_jobs_completed = 0;
    int MAX_NUM_JOBS = 100;
    int MIN_NUM_JOBS = 5;
    int TIME_SLICE = 10;
    FILE *file1;
    FILE *file2;
    FILE *file3;
    file1 = fopen("RRGrouping.txt","w+");
    file2 = fopen("RRWaitTime.txt", "w+");
    file3 = fopen("RRQueueSize.txt", "w+");
    FILE *file4 = fopen("RRTotalTime.txt", "w+");
    int *grouping = (int*) calloc (13, sizeof(int));
    int *wait_time = (int*) calloc (ready->capacity, sizeof(int));
    int total_time = 0;

    while(isGroupingFilled(grouping)){
        // Push the element out of the queue and record the time quanta
        int quanta_of_job = front(ready);
        dequeue(ready);

        incrementGrouping(quanta_of_job, grouping);
        int i;

        // Sleep to simulate the PC being remaining busy for that amount of time. If the time
        // of the job is less than the time slice, sleep only for that amount of time.
        if (quanta_of_job < TIME_SLICE){
            sleep(quanta_of_job);
            fprintf(file4, "%i\n", total_time);
            total_time += quanta_of_job;

            // The current job will finish its execution, so all other jobs should be
            // incremented by the time it took for the job to finish and then they
            // should be shifted down the list.
            for (i = 1; i < ready->size; i++){
                wait_time[i] += quanta_of_job;
            }

            for (i = 0; i < ready->size; i++){
                wait_time[i] = wait_time[i+1];
            }

            wait_time[ready->size - 1] = 0;

        } else {
            // The current job needs to be saved in a temporary variable. All other
            // jobs in the queue should be increased by the time slice. Then all jobs
            // should be shifted one to the left and the current job should be inserted
            // at the end of the list.
            sleep(TIME_SLICE);
            fprintf(file4, "%i\n", total_time);
            total_time += TIME_SLICE;

            int temp = wait_time[0];

            for (i = 1; i < ready->size; i++){
                wait_time[i] += TIME_SLICE;
                wait_time[i-1] = wait_time[i];
            }

            wait_time[ready->size- 1] = temp;
        }

        int quanta = quanta_of_job - TIME_SLICE;
        // Print out the information of how long the process' execution and wait time was.

        fprintf(file3, "%i\n", wait_time[0]);
        fprintf(file2, "%i\n", ready->size);

		// Transfer jobs if the amount left in the queue is too small. Devise a random number
		// to decide how many jobs should be added to the queue.
		if (ready->size < MIN_NUM_JOBS){
			int num_jobs = (rand() % 15-(ready->size)) + 1;
			transfer(ready, pool, num_jobs);
		}
    }

    int i;

    for (i = 0; i < 13; i++){
        fprintf(file1, "%i\n", grouping[i]);
    }

    fclose(file1);
    fclose(file2);
    fclose(file3);
    fclose(file4);
}

void ModifiedHalfedRoundRobin(Queue *ready, Queue *pool){

    int num_jobs_completed = 0;
    int MAX_NUM_JOBS = 100;
    int MIN_NUM_JOBS = 5;
    int TIME_SLICE = 10;
    FILE *file1;
    FILE *file2;
    FILE *file3;
    file1 = fopen("MHRRGrouping.txt","w+");
    file2 = fopen("MHRRWaitTime.txt", "w+");
    file3 = fopen("MHRRQueueSize.txt", "w+");
    FILE *file4 = fopen("MHRRTotalTime.txt", "w+");
    int *grouping = (int*) calloc (13, sizeof(int));
    int *wait_time = (int*) calloc (ready->capacity, sizeof(int));
    int total_time = 0;

    while(isGroupingFilled(grouping)){
        // Push the element out of the queue and record the time quanta
        int quanta_of_job = front(ready);
        dequeue(ready);

        incrementGrouping(quanta_of_job, grouping);
        int i;

        // Sleep to simulate the PC being remaining busy for that amount of time. If the time
        // of the job is less than the time slice, sleep only for that amount of time.
        if (quanta_of_job < TIME_SLICE){
            sleep(quanta_of_job);
            fprintf(file4, "%i\n", total_time);
            total_time += quanta_of_job;

            // The current job will finish its execution, so all other jobs should be
            // incremented by the time it took for the job to finish and then they
            // should be shifted down the list.
            for (i = 1; i < ready->size; i++){
                wait_time[i] += quanta_of_job;
            }

            for (i = 0; i < ready->size; i++){
                wait_time[i] = wait_time[i+1];
            }

            wait_time[ready->size - 1] = 0;

        } else {
            // The current job needs to be saved in a temporary variable. All other
            // jobs in the queue should be increased by the time slice. Then all jobs
            // should be shifted one to the left and the current job should be inserted
            // at the end of the list.
            sleep(quanta_of_job / 2);
            fprintf(file4, "%i\n", total_time);
            total_time += quanta_of_job / 2;

            int temp = wait_time[0];

            for (i = 1; i < ready->size; i++){
                wait_time[i] += quanta_of_job / 2;
                wait_time[i-1] = wait_time[i];
            }

            wait_time[ready->size- 1] = temp;
        }

        int quanta = quanta_of_job - (quanta_of_job / 2);
        // Print out the information of how long the process' execution and wait time was.

        fprintf(file3, "%i\n", wait_time[0]);
        fprintf(file2, "%i\n", ready->size);

		// Transfer jobs if the amount left in the queue is too small. Devise a random number
		// to decide how many jobs should be added to the queue.
		if (ready->size < MIN_NUM_JOBS){
			int num_jobs = (rand() % 15-(ready->size)) + 1;
			transfer(ready, pool, num_jobs);
		}
    }

    int i;

    for (i = 0; i < 13; i++){
        fprintf(file1, "%i\n", grouping[i]);
    }

    fclose(file1);
    fclose(file2);
    fclose(file3);
    fclose(file4);
}

void ModifiedRoundRobin(Queue *ready, Queue *pool){
    int num_jobs_completed = 0;
    int MAX_NUM_JOBS = 100;
    int MIN_NUM_JOBS = 5;
    int TIME_SLICE = 10;
    int SLICE_INCREASE = 2;
    int location = 0;
    FILE *file1;
    FILE *file2;
    FILE *file3;
    file1 = fopen("MRRGrouping.txt","w+");
    file2 = fopen("MRRWaitTime.txt", "w+");
    file3 = fopen("MRRQueueSize.txt", "w+");
    FILE *file4 = fopen("MRRTotalTime.txt", "w+");
    int *grouping = (int*) calloc (13, sizeof(int));
    int *wait_time = (int*) calloc (ready->capacity, sizeof(int));
    int total_time = 0;

    // keeps track of what elements have been processed by MRR and
    // how much of an additional time slice they should receive. All
    // processes should start out with 0 additional time slices
    int *tracker = (int*) calloc (ready->capacity, sizeof(int));

    while(isGroupingFilled(grouping)){
        // Push the element out of the queue and record the time quanta
        int quanta_of_job = front(ready);
        dequeue(ready);

        incrementGrouping(quanta_of_job, grouping);

        // Sleep to simulate the PC being remaining busy for that amount of time. If the time
        // of the job is less than the time slice, sleep only for that amount of time.
	int time_executed = TIME_SLICE * tracker[location];
        if ( quanta_of_job < time_executed ){
            sleep(time_executed );
            fprintf(file4, "%i\n", total_time);
            total_time += time_executed;

            // All the remaining elements in tracker need to be shifted one to the left since
            // this element will now be removed
            int i;
            for (i = location; i < ready->size; i++){
                tracker[i] = tracker[i+1];
            }

            // Last element in tracker should be set back to 0
            tracker[ready->size - 1] = 0;

			// The current job will finish its execution, so all other jobs should be
            // incremented by the time it took for the job to finish and then they
            // should be shifted down the list.
            for (i = 1; i < ready->size; i++){
                wait_time[i] += time_executed;
            }

            for (i = 0; i < ready->size; i++){
                wait_time[i] = wait_time[i+1];
            }

            wait_time[ready->size - 1] = 0;

        } else {
            sleep( time_executed );
            fprintf(file4, "%i\n", total_time);
            total_time += time_executed;
            tracker[location] += 1;

			int i;
			int temp = wait_time[0];
			for (i = 1; i < ready->size; i++){
                wait_time[i] += time_executed;
                wait_time[i-1] = wait_time[i];
            }

            wait_time[ready->size- 1] = temp;

        }

        int quanta = quanta_of_job - (time_executed);
        // Print out the information of how long the process' execution and wait time was.
        //printf("Job time quanta: %i milliseconds.\n", quanta_of_job);
        fprintf(file3, "%i\n", wait_time[0]);

        // Calculate the wait time of all remaining processes in the system. They should all
        // wait the same amount unless one job executes in less time than it's allotted time
        // slice
        if (quanta > 0){
            enqueue(ready, quanta);
        }

        fprintf(file2, "%i\n", ready->size);

		// Transfer jobs if the amount left in the queue is too small. Devise a random number
		// to decide how many jobs should be added to the queue.
		if (ready->size < MIN_NUM_JOBS){
			int num_jobs = (rand() % MIN_NUM_JOBS) + 1;
			transfer(ready, pool, num_jobs);
		}

		location++;

		// Increase the location within the tracker to correspond to the next element. If the location
		// exceeds the size of the array, wrap around to start from the beginning again.
		if (location > ready->size){
            location = 0;
		}
    }

    int i;
    for (i = 0; i < 13; i++){
        fprintf(file1, "%i\n", grouping[i]);
    }

    fclose(file1);
    fclose(file2);
    fclose(file3);
    fclose(file4);
}

int isGroupingFilled(int *grouping){
	return grouping[0] < 12 || grouping[1] < 12 || grouping[2] < 12 || grouping[3] < 12
		|| grouping[4] < 12 || grouping[5] < 12 || grouping[6] < 12 || grouping[7] < 12
		|| grouping[8] < 12 || grouping[9] < 12 || grouping[10] < 12 || grouping[11] < 12
		|| grouping[12] < 12;
}
