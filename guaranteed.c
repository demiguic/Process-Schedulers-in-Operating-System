#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

#define NUM_THREADS 5
#define CPU_TIME 10
#define MAX_TIME 30

double cpu_time = 10;
double exe_time;
int num_processes = NUM_THREADS;

typedef struct
{
    int pid;           // ID do processo
    double burst_time; // Tempo de processo restante
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
        printf("Processo %d. Tempo de execução: %.1fs\n", processes[t].pid, processes[t].burst_time);
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
        if (process->burst_time >= exe_time)
        {
            process->burst_time -= exe_time; // Diminui o tempo restante no valor de exe_time
            printf("Processo %d executado por %.1fs. Restam %.1fs\n", process->pid, exe_time, process->burst_time);
        }
        else
        {
            process->burst_time = 0; // Tempo finalizado, caso o tempo de execução > tempo restante
        }
        pthread_mutex_unlock(&process->mutex);
    }
    num_processes--;
    printf("Processo %d finalizado. Restam %d processos.\n", process->pid, num_processes);
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
            exe_time = cpu_time / num_processes; // Escalonador define o tempo de execução
            pthread_mutex_unlock(&processes[t].mutex);
            sleep(1);
        }
    }
}
