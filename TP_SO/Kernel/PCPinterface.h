/*
 * PCPinterface.h
 *
 *  Created on: 11/07/2014
 *      Author: utnso
 */

#ifndef PCPINTERFACE_H_
#define PCPINTERFACE_H_

#include <stdbool.h>
#include <commons/collections/list.h>
#include <biblioteca_comun/definiciones.h>
#include <commons/collections/queue.h>

typedef struct
 {
 t_PCB* pcb;
 int tiempo;
 } t_estructuraProcesoBloqueado;

typedef struct {
 int idCPU;
 int estado;
 int idProceso;
 } t_estructuraCPU;

void moverAColaExityLiberarCPU(t_PCB*,int);
void* pcp_main(void*);
void crearHilosPrincipales();
void* mandarAEjecutar(void*);
void enviarCPU(t_PCB*);
void* bloquearYDevolverAReady(void*);
int buscarRetardo(char*);
int encontrarPrimeraCpuLibre(t_list*);
bool estaLibre(t_estructuraCPU*);
void nuevaCPU(int);
void programaSalioPorQuantum(t_PCB*, int);
void moverAColaExit(t_PCB*, int);
void programaSalioPorBloqueo(t_PCB*, int, char*, int);
void seLiberoUnaCPU(int,t_PCB*);
void cpu_destroy(t_estructuraCPU*);
bool tieneID(t_estructuraCPU*);
void seDesconectoCPU(int);
void seDesconectoCPUSigusr(int, t_PCB*);
int posicionEnLaLista(t_list*, int);
int estaLibreID(int);
void mostrarColaDeProcesosEnEjecucion();
void mostrarColaDeProcesosBloqueados();
void mostrarColaDeProcesosListos();
void mostrarColaDeProcesosFinalizados();
int cantidadDispositivos();
void mostrarColaDePCBsBloqueados(t_queue*);
void mostrarColaDePCBsBloqueadosSem(t_queue*);
void imprimirNodosPCBsBloqueados(t_estructuraProcesoBloqueado*);
void imprimirNodosPCBsBloqueadosSem(int*);
void mostrarColaDePCBs2(t_queue*);
void imprimirNodosPCBs2(t_PCB*);
int buscarIDPrograma(int);
void cambiar_Proceso_Activo(int);
int posicionEnLaListaExec(t_list*, int);



#endif /* PCPINTERFACE_H_ */
