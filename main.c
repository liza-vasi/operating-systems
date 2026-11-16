
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

#define COUNT 10

typedef struct {
    int number;
} Event;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

Event event;
int ready = 0;

// Отправка событий
void *producer(void *arg) {
    for (int i = 1; i <= COUNT; i++) {
        sleep(1);

        pthread_mutex_lock(&mutex);

        while (ready)
            pthread_cond_wait(&cond, &mutex);

        event.number = i;
        ready = 1;

        printf("Отправлено событие %d\n", event.number);
        pthread_cond_signal(&cond);

        while (ready)
            pthread_cond_wait(&cond, &mutex);

        pthread_mutex_unlock(&mutex);
    }

    return NULL;
}

// Получение событий
void *consumer(void *arg) {
    for (int i = 0; i < COUNT; i++) {
        pthread_mutex_lock(&mutex);

        while (!ready)
            pthread_cond_wait(&cond, &mutex);

        printf("Обработано событие %d\n", event.number);

        ready = 0;

        pthread_cond_signal(&cond);
        pthread_mutex_unlock(&mutex);
    }

    return NULL;
}

int main(void) {
    pthread_t t1, t2;

    pthread_create(&t1, NULL, producer, NULL);
    pthread_create(&t2, NULL, consumer, NULL);

    pthread_join(t1, NULL);
    pthread_join(t2, NULL);

    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&cond);

    return 0;
}
