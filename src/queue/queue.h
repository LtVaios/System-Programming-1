typedef struct queue_struct* queue;
typedef struct queuenode* node;

//Initializes a new empty queue
queue init_q(void);

//pops the pid at the top of the queue and returns it
int pop(queue q);

//pushes a new node with the given pid in the queue
void push(queue q, int new_pid);

//deallocates resources and destroys the queue
void destroy_q(queue q);