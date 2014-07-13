#ifndef PCP__H_
#define PCP__H_


#include "Kernel.h"

typedef struct {
	t_PCB* pcb;
	int IDCpu;
	char razon;
	int dispositivoIO;
	int tiempo;
} t_paquete_recibir_CPU;



typedef struct {
	int retardo;
	sem_t* colaVacia;
	t_queue* procesosBloqueados;
	sem_t* mutexCola;
} t_estructuraDispositivoIO;

typedef struct
{
	t_PCB* pcb;
	int tiempo;
} t_estructuraProcesoBloqueado;

/*typedef struct {
	int idCPU;
	int estado;
	int idProceso;
} t_estructuraCPU;*/

sem_t * CPUsLibres = NULL;
sem_t * sPLP = NULL;
sem_t * sYaInicializoElMT = NULL;
sem_t * sBloqueado = NULL;
sem_t * colaExecVacia = NULL;
sem_t * semCPUDesconectadaMutex = NULL;
int laSenialDeTerminar = 0;
t_queue* colaExec;
pthread_t ejecutar;
pthread_t multiplexorCPUs;
pthread_t envCPU;
int retMandarAEjecutar, retEnviarCPU, retMultiplexorCPUs;
int* sinParametros = NULL;
t_dictionary* diccionarioDispositivos;
t_list* CPUs;
int idUltimaCPUDesconectada;

#endif /* PCP__H_ */
