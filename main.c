#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>
#include <unistd.h>

int N; // Кол-во помещений
int K; // Кол-во покупателей
int *S; // Массив помещений склада
int *L; // Спрос покупателей
int secs; // Время работы

int exit_now = 0;
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

double wtime()
{
    struct timeval t;
    gettimeofday(&t, NULL);
    return (double)t.tv_sec + (double)t.tv_usec * 1E-6;
}

int getrand(int min, int max)
{
    return (int)(((double)rand() / (RAND_MAX + 1.0)) * (max - min) + min);
}

void *alarm_thread(void *arg)
{
    unsigned int s;
    s = *((unsigned int *) arg);
    sleep(s);
    return NULL;
}

void *loader_thread(void *arg)
{
    while (!exit_now) {
        pthread_mutex_lock(&lock);
        for (int i = 0; i < N; ++i) {
            if (S[i] == 0) {
                S[i] = 40;
                printf("L: S[%d] filled\n", i);
                break;
            }
        }
        pthread_mutex_unlock(&lock);
        sleep(5);
    }
    return NULL;
}

void *buyer_thread(void *arg)
{
    int k;
    k = *((int *) arg);
    while (!exit_now && L[k] > 0) {
        pthread_mutex_lock(&lock);
        for (int i = 0; i < N; ++i) {
            if (S[i] > 0) {
                if (S[i] > L[k]) {
                    int d = S[i] - L[k];
                    printf("K:%d S[%d] took %d/%d. left: %d\n", k, i, L[k], L[k], d);
                    S[i] = d;
                    L[k] = 0;
                } else {
                    printf("K:%d S[%d] took %d/%d. left: 0\n", k, i, S[i], L[k]);
                    L[k] -= S[i];
                    S[i] = 0;
                }
                break;
            }
        }
        pthread_mutex_unlock(&lock);
        sleep(5);
    }
    return NULL;
}

int main(int argc, char *argv[])
{
    N = 5; K = 3; secs = 60;

    int c;
    while ((c = getopt(argc, argv, "n:k:s:")) != -1) {
        switch (c) {
            case 'n':
                N = atoi(optarg);
                break;
            case 'k':
                K = atoi(optarg);
                break;
            case 's':
                secs = atoi(optarg);
                break;
            default:
                printf("?? getopt returned character code 0%o ??\n", c);
        }
    }


    S = malloc(sizeof(int) * N);
    printf("S [ ");
    for (int i = 0; i < N; ++i) {
        srand((unsigned int) (i * (int)wtime()));
        S[i] = getrand(1, 40);
        printf("%d ", S[i]);
    }
    printf("]\n");

    L = malloc(sizeof(int) * K);
    pthread_t *buyer = malloc(sizeof(pthread_t) * K);
    printf("L [ ");
    for (int i = 0; i < K; ++i) {
        srand((unsigned int) (i * (int)wtime()));
        L[i] = getrand(1, 1000);
        printf("%d ", L[i]);
    }
    printf("]\n");
    for (int i = 0; i < K; ++i) {
        if (pthread_create (&buyer[i], NULL, buyer_thread, &i) != 0) {
            return EXIT_FAILURE;
        }
    }


    pthread_t alarm;
    if (pthread_create (&alarm, NULL, alarm_thread, &secs) != 0) {
        return EXIT_FAILURE;
    }

    pthread_t loader;
    if (pthread_create (&loader, NULL, loader_thread, NULL) != 0) {
        return EXIT_FAILURE;
    }

    // Ждем срабатывания таймера
    if (pthread_join(alarm, NULL) != 0) {
        return EXIT_FAILURE;
    }
    printf("Exit now!...\n");
    exit_now = 1;

    if (pthread_join(loader, NULL) != 0) {
        return EXIT_FAILURE;
    }
    for (int i = 0; i < K; ++i) {
        if (pthread_join(buyer[i], NULL) != 0) {
            return EXIT_FAILURE;
        }
    }

    printf("S [ ");
    for (int i = 0; i < N; ++i) {
        printf("%d ", S[i]);
    }
    printf("]\n");

    printf("L [ ");
    for (int i = 0; i < K; ++i) {
        printf("%d ", L[i]);
    }
    printf("]\n");

    return 0;
}
