#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

#define NUM_THREADS 5
#define QUANTUM 5
#define MAX_TIME 30
#define MAX_PRIORITY 10

typedef struct
{
    int pid;        // ID do processo
    int burst_time; // Tempo de processo restante
    int priority;   // prioridade do processo
    pthread_mutex_t mutex;
    pthread_cond_t cond;
} Process;

Process processes[NUM_THREADS]; // Vetor de processos

void *process_function(void *arg);
void *priority_scheduling();

int main()
{
    srand(time(NULL)); // Inicializando a seed com base no relógio para geração de números aleatórios

    // Criando as informações para cada processo
    for (int t = 0; t < NUM_THREADS; t++)
    {
        processes[t].pid = t + 1;                          // Atribuição do ID
        processes[t].burst_time = rand() % MAX_TIME + 1;   // Definindo um tempo de execução entre 1 e o tempo máximo
        processes[t].priority = rand() % MAX_PRIORITY + 1; // Definindo uma prioridade entre 1 e a prioridade máxima
        pthread_mutex_init(&processes[t].mutex, NULL);
        pthread_cond_init(&processes[t].cond, NULL);
        printf("Processo %d. Tempo de execução: %d. Prioridade: %d.\n", processes[t].pid, processes[t].burst_time, processes[t].priority);
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
    pthread_create(&scheduler, NULL, priority_scheduling, NULL);

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
            printf("Processo %d executado. Tempo restante: %d. Prioridade: %d\n", process->pid, process->burst_time, process->priority);
        }
        else
        {
            process->burst_time = 0; // Zera o tempo caso Quantum > tempo restante
        }
        if (process->priority > 1)
        {
            process->priority--; // Prioridade decrementada em 1
        }
        pthread_mutex_unlock(&process->mutex);
    }
    process->priority = -1; // Uma vez finalizado, a prioridade do processo torna-se negativa
    printf("Processo %d finalizado.\n", process->pid);
    return NULL;
}

void *priority_scheduling()
{
    while (1)
    {
        int max_priority_idx = -1; // índice do processo com maior prioridade
        int max_priority = -1;     // prioridade máxima
        // Percorre o vetor de processos em busca do processo com a maior prioridade
        for (int t = 0; t < NUM_THREADS; t++)
        {
            pthread_mutex_lock(&processes[t].mutex);
            if (processes[t].priority > max_priority)
            {
                max_priority = processes[t].priority;
                max_priority_idx = t;
            }
            pthread_mutex_unlock(&processes[t].mutex);
        }
        // Caso tenha encontrado um processo com prioridade válida, executa-o
        if (max_priority_idx >= 0)
        {
            pthread_mutex_lock(&processes[max_priority_idx].mutex);
            pthread_cond_signal(&processes[max_priority_idx].cond);
            pthread_mutex_unlock(&processes[max_priority_idx].mutex);
            sleep(2);
        }
    }
}