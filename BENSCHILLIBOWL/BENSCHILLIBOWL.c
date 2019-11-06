#include "BENSCHILLIBOWL.h"

#include <assert.h>
#include <stdlib.h>
#include <time.h>

bool IsEmpty(BENSCHILLIBOWL* mcg);
bool IsFull(BENSCHILLIBOWL* mcg);
void AddOrderToBack(Order **orders, Order *order);

MenuItem BENSCHILLIBOWLMenu[] = { 
    "BensChilli", 
    "BensHalfSmoke", 
    "BensHotDog", 
    "BensChilliCheeseFries", 
    "BensShake",
    "BensHotCakes",
    "BensCake",
    "BensHamburger",
    "BensVeggieBurger",
    "BensOnionRings",
};
int BENSCHILLIBOWLMenuLength = 10;

/* Select a random item from the Menu and return it */
MenuItem PickRandomMenuItem() {
    return BENSCHILLIBOWLMenu[rand() % BENSCHILLIBOWLMenuLength];
}

/* Allocate memory for the Restaurant, then create the mutex and condition variables needed to instantiate the Restaurant */

BENSCHILLIBOWL* OpenRestaurant(int max_size, int expected_num_orders) {
    BENSCHILLIBOWL *bcb = (BENSCHILLIBOWL*) malloc(sizeof(BENSCHILLIBOWL));
    bcb->orders = NULL;
    bcb->current_size = 0;
    bcb->max_size = max_size;
    bcb->next_order_number=1;
    bcb->orders_handled=0;
    bcb->expected_num_orders=expected_num_orders;
  
    // Initialize the mutex lock
    pthread_mutex_init(&(bcb->mutex), NULL);
    printf("Restaurant is open!\n");
    return bcb;
}


/* check that the number of orders received is equal to the number handled (ie.fullfilled). Remember to deallocate your resources */

void CloseRestaurant(BENSCHILLIBOWL* mcg) {
    // Ensure all orders were handled.
    if (mcg->orders_handled != mcg->expected_num_orders) {
        fprintf(stderr, "Not all the orders were fulfilled.\n");
        exit(0);
    }
    // Delete the synchronization variables.
    pthread_mutex_destroy(&(mcg->mutex));
    // Free the space used by restaurant.
    free(mcg);
    printf("Restaurant is closed!\n");
}

/* add an order to the back of queue */
int AddOrder(BENSCHILLIBOWL* mcg, Order* order) {
    // Order queue is critical section. Grab the lock
    pthread_mutex_lock(&(mcg->mutex));
  
    // Wait until the restaurant is not full.
    while (mcg->current_size == mcg->max_size) {
        pthread_cond_wait(&(mcg->can_add_orders), &(mcg->mutex));
    }
  
    order->order_number = mcg->next_order_number;
    AddOrderToBack(&(mcg->orders), order);
  
    // Update the next_order_number and the size.
    mcg->next_order_number++;
    mcg->current_size++;
    
    // Send signal to indicate an order had been added.
    pthread_cond_broadcast(&(mcg->can_get_orders));
        
    // Release the lock.
    pthread_mutex_unlock(&(mcg->mutex));
    
    return order->order_number;
}

/* remove an order from the queue */
Order *GetOrder(BENSCHILLIBOWL* mcg) {
    // Critical section. Grab the lock.
    pthread_mutex_lock(&(mcg->mutex));
    
    // Wait until the restaurant is not empty.   
    while(mcg->current_size == 0) {
        
        // If all the orders have been already fulfilled notify cook.
        if (mcg->orders_handled >= mcg->expected_num_orders) {
            pthread_mutex_unlock(&(mcg->mutex));
            return NULL;
        }
        
        pthread_cond_wait(&(mcg->can_get_orders), &(mcg->mutex));
    }
    
    // Get the order from the front.
    Order *order = mcg->orders;
    mcg->orders = mcg->orders->next;
    
    // Update the current order size and orders_handled.
    mcg->current_size--;
    mcg->orders_handled++;
    
    pthread_cond_broadcast(&(mcg->can_add_orders));
        
    // Release the lock.
    pthread_mutex_unlock(&(mcg->mutex));
    
    
    return order;

}


/* this methods adds order to rear of queue */
void AddOrderToBack(Order **orders, Order *order) {
  if (*orders == NULL) { // If there were no orders.
        *orders = order;
    } else {  // If orders isn't empty.
        Order *curr_order = *orders;
        while (curr_order->next) {
            curr_order = curr_order->next;
        }
        curr_order->next = order;
    }
}

