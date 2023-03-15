#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

#define NUM_THREADS 5
#define QUANTUM 5
#define MAX_TIME 30

typedef struct
{
    int pid;        // ID do processo
    int burst_time; // Tempo de processo restante
    pthread_mutex_t mutex;
    pthread_cond_t cond;

} Process;

Process processes[NUM_THREADS]; // Vetor de processos

void *process_function(void *arg);
void *round_robin();

int main()
{
    srand(time(NULL)); // Inicializando a seed com base no relógio para geração de números aleatórios

    // Criando as informações para cada processo
    for (int t = 0; t < NUM_THREADS; t++)
    {
        processes[t].pid = t + 1;                        // Atribuição do ID
        processes[t].burst_time = rand() % MAX_TIME + 1; // Definindo um tempo de execução entre 1 e o tempo máximo
        pthread_mutex_init(&processes[t].mutex, NULL);
        pthread_cond_init(&processes[t].cond, NULL);
        printf("Processo %d. Tempo de execução: %d\n", processes[t].pid, processes[t].burst_time);
    }

    printf("\n");

    // Criando uma thread para cada processo
    pthread_t threads[NUM_THREADS];
    for (int t = 0; t < NUM_THREADS; t++)
    {
        pthread_create(&threads[t], NULL, process_function, (void *)&processes[t]);
    }

    // Criando uma thread para o escalonador de processos
    pthread_t scheduler;
    pthread_create(&scheduler, NULL, round_robin, NULL);

    // Espera as threads terminarem
    for (int t = 0; t < NUM_THREADS; t++)
    {
        pthread_join(threads[t], NULL);
    }

    printf("Todos os processos foram finalizados.\n");

    return 0;
}

void *process_function(void *arg)
{
    Process *process = (Process *)arg;
    while (process->burst_time > 0)
    {
        pthread_mutex_lock(&process->mutex);
        pthread_cond_wait(&process->cond, &process->mutex);
        sleep(1); // Simula um processo
        if (process->burst_time >= QUANTUM)
        {
            process->burst_time -= QUANTUM; // Diminui o tempo restante em QUANTUM
            printf("Processo %d executado. Tempo restante: %d.\n", process->pid, process->burst_time);
        }
        else
        {
            process->burst_time = 0; // Zera o tempo caso Quantum > tempo restante
        }
        pthread_mutex_unlock(&process->mutex);
    }
    printf("Processo %d finalizado.\n", process->pid);
    return NULL;
}

// Função escalonadora
void *round_robin()
{
    while (1)
    {
        // Percorre o vetor de processos permitindo que todos sejam executados de forma circular
        for (int t = 0; t < NUM_THREADS; t++)
        {
            pthread_mutex_lock(&processes[t].mutex);
            pthread_cond_signal(&processes[t].cond);
            pthread_mutex_unlock(&processes[t].mutex);
            sleep(1);
        }
    }
}
