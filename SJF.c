#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

#define NUM_THREADS 5
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
void *shortest_job_first();

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
    pthread_create(&scheduler, NULL, shortest_job_first, NULL);

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
        printf("Processo %d executado por %ds.\n", process->pid, process->burst_time);
        process->burst_time = 0; // Executa completamente o processo
        pthread_mutex_unlock(&process->mutex);
    }
    printf("Processo %d finalizado.\n", process->pid);
    return NULL;
}

// Função comparadora de tempo de execução
int compare_burst_time(const void *a, const void *b)
{
    const Process *p1 = (const Process *)a;
    const Process *p2 = (const Process *)b;

    return p1->burst_time - p2->burst_time;
}

void *shortest_job_first()
{
    while (1)
    {
        // Ordena o vetor de processos de forma crescente quanto ao tempo de execução
        qsort(processes, NUM_THREADS, sizeof(Process), compare_burst_time);
        // Percorre o vetor de processos executando do menor ao maior, em relação ao tempo de execução.
        for (int t = 0; t < NUM_THREADS; t++)
        {
            pthread_mutex_lock(&processes[t].mutex);
            pthread_cond_signal(&processes[t].cond);
            pthread_mutex_unlock(&processes[t].mutex);
            sleep(1);
        }
    }
}
