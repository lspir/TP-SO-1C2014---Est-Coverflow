/*
 * atencionInterna.c
 *
 *  Created on: 11/06/2014
 *      Author: utnso
 */
#include "atencionInterna.h"
#include "atencioninterna_interfaz.h"
#include <biblioteca_comun/Serializacion.h>
#include <biblioteca_comun/definiciones.h>

pthread_mutex_t mutexOperacion = PTHREAD_MUTEX_INITIALIZER;
//typedef enum {
//	Kernel = 1, CPU = 2
//} saludos_internos;

void* atencionInterna(void* sinParametro) {
	char* saludoKernel = malloc(7);
	char* saludoCpu = malloc(4);

	int socket = crearServidor(puertoUMV, logger);
	log_debug(logger, "Se creó servidor para atención interna");
	while (1) {
		int* socketKernel=malloc(sizeof(int));
		*socketKernel = aceptarConexion(socket, logger);
		recv(*socketKernel, (void*) saludoKernel, 30, 0);
		log_info(logger, "Se intenta conectar un posible Kernel. Comprobando...");

		if (!strncmp(saludoKernel, "Kernel", 6)) {
			log_info(logger, "El handshake con el Kernel salió bien.");
			send(*socketKernel, "UMV", 4, 0);
			//free(saludoKernel);FIXME va acá?
			//Creo hilo de atención al kernel.
			if (pthread_create(&hiloKernel, NULL, (void *) atencionKernel,
					(void*) socketKernel)) {
				log_error(logger,
						"El hilo del Kernel no se creó correctamente.");
			} else {
				log_debug(logger, "El hilo del Kernel se creó correctamente.");
			}
			break;
		} else {
			close(*socketKernel);
			free(socketKernel);
			log_error(logger, "No es un Kernel válido");
		}

	} //Cierra while de Kernel

	while (1) {
		int *socketCPU=malloc(sizeof(int));
		*socketCPU = aceptarConexion(socket, logger);
		recv(*socketCPU, (void*) saludoCpu, 30, 0);
		log_info(logger, "Se intenta conectar un posible CPU. Comprobando...");

		if (!strncmp(saludoCpu, "CPU", 3)) {
			log_info(logger, "El handshake con la CPU salió bien.");
			send(*socketCPU, "UMV", 4, 0);
			//Creo hilo de atención a CPU.
			if (pthread_create(&hiloCpu, NULL, (void *) atencionCpu,
					(void*) socketCPU)) {
				log_error(logger, "El hilo de CPU no se creó correctamente.");
			} else {
				log_info(logger, "El hilo de CPU se creó correctamente.");
				cantCpu++;
			}

		} else {
			close(*socketCPU);
			free(socketCPU);
			log_error(logger, "No es una CPU válida.");
		}
	}			//Cierra while de CPU

}			//Cierra atencionInterna

void atencionKernel(int* socketKernel) {
	char *header = malloc(16);
	int j = 0;
	int *razon = malloc(4);
	int *tamanoMensaje = malloc(4);
	int procesoActivo = 0, parametro[3], basesLogicas[4];
	log_info(logger, "Entró a atencioKernel.");
	int *pid = malloc(sizeof(int));
	while (1) {

		j++;
		char *mensaje = recibirConRazon(*socketKernel, razon, logger);
		if((mensaje==NULL) && (*razon!=DESTRUIR_SEGMENTOS) && (*razon!=CONFIRMACION)){
			log_error(logger,"Se desconecto abruptamente el Kernel, sd: %d",*socketKernel);
			break;//el close lo hace recibirConRazon
		}
		aplicarRetardo(retardo);
		switch (*razon) {
		case CONFIRMACION:
			pthread_mutex_lock(&mutexOperacion);
			enviarConRazon(*socketKernel,logger,CONFIRMACION, NULL);
			pthread_mutex_unlock(&mutexOperacion);
			break;

		case CREAR_SEGMENTOS_PROGRAMA:
			log_debug(logger, "Recibí crear segmentos programa, de parte del Kernel.");
			pthread_mutex_lock(&mutexOperacion);
			int *tamano1 = malloc(sizeof(int)), *tamano2 = malloc(sizeof(int)),
					*tamano3 = malloc(sizeof(int)), *tamano4 = malloc(
							sizeof(int));
			desempaquetar2(mensaje, tamano1, tamano2, tamano3, tamano4, 0);
			int i;
			cambiarProcesoActivo(procesoActivo);
			basesLogicas[0] = crearSegmento(*tamano1);
			basesLogicas[1] = crearSegmento(*tamano2);
			basesLogicas[2] = crearSegmento(*tamano3);
			basesLogicas[3] = crearSegmento(*tamano4);
			for (i = 0; i < 4; i++) {
//				log_info(logger, "La base lógica %d es %d", i, basesLogicas[i]);
//				//tendrían que volar el log y el printf, qué opinan?
//				printf("la base logica %d es %d\n", i, basesLogicas[i]);
				if (basesLogicas[i] == -1) {
					destruirTodosLosSegmentos();
					*razon = MEMORY_OVERLOAD;
					*tamanoMensaje = 0;
					break;
				}
				*razon = BASES_LOGICAS;
				*tamanoMensaje = 32;
			}
			pthread_mutex_unlock(&mutexOperacion);
			enviarConRazon(*socketKernel, logger, *razon,
					serializar2(crear_nodoVar(&basesLogicas[0], 4),
							crear_nodoVar(&basesLogicas[1], 4),
							crear_nodoVar(&basesLogicas[2], 4),
							crear_nodoVar(&basesLogicas[3], 4), 0));
			log_debug(logger, "Terminé de crear segmentos programa, de parte del Kernel.");
			break;

		case DESTRUIR_SEGMENTOS:
			log_debug(logger, "Recibí destruir segmentos, de parte del Kernel.");
			pthread_mutex_lock(&mutexOperacion);
			cambiarProcesoActivo(procesoActivo);
			destruirTodosLosSegmentos();
			pthread_mutex_unlock(&mutexOperacion);
			log_debug(logger, "Terminé de destruir segmentos, de parte del Kernel.");
			break;

		case ESCRIBIR_EN_UMV:
			log_debug(logger, "Recibí escribir en umv, de parte del Kernel.");
			pthread_mutex_lock(&mutexOperacion);
			cambiarProcesoActivo(procesoActivo);

			char *buffer = malloc(2048);
			desempaquetar2(mensaje, &parametro[0], &parametro[1], &parametro[2],
					buffer, 0);
			buffer=realloc(buffer, parametro[2]);
			log_info(logger, "Los parámetros que envio fueron:\nbase= %d\noffset= %d\ntamano= %d\nbuffer= %s",parametro[0],parametro[1],parametro[2],buffer);
//			log_info(logger, "los parámetros que envio fueron:");
//			log_info(logger, "base= %d",parametro[0]);
//			log_info(logger, "offset= %d",parametro[1]);
//			log_info(logger, "tamano= %d",parametro[2]);
//			log_info(logger, "buffer= %s", buffer);
//			printf("Los parametros que envio fueron:\n base= %d\n offset=%d\n tamano= %d\n buffer= %s\n", parametro[0], parametro[1], parametro[2], buffer);
			enviarUnosBytes(parametro[0], parametro[1], parametro[2], buffer);
			pthread_mutex_unlock(&mutexOperacion);
			free(buffer);
			log_debug(logger, "Terminé de escribir en umv, de parte del Kernel.");
			break;

		case CAMBIAR_PROCESO_ACTIVO:
			log_debug(logger, "Recibí cambiar proceso activo, de parte del Kernel.");
			pthread_mutex_lock(&mutexOperacion);
			desempaquetar2(mensaje, &procesoActivo, 0);
			log_info(logger, "El valor del pid es: %d",procesoActivo);
			//printf("\nel valor del pid es: %d\n", procesoActivo);
//			cambiarProcesoActivo(procesoActivo);
			pthread_mutex_unlock(&mutexOperacion);
			log_debug(logger, "Terminé de cambiar proceso activo, de parte del Kernel.");
			break;
		case CREAR_PROCESO_NUEVO:
			log_debug(logger, "Recibí crear proceso nuevo, de parte del Kernel.");
			pthread_mutex_lock(&mutexOperacion);
			//log_debug(logger, "[antes de deserializar]El PID que recibí es: %d", *pid);
			//printf("el pid que recibi es:%d\n", *pid);
			desempaquetar2(mensaje, pid, 0);
			log_debug(logger, "El PID que recibí es: %d", *pid);
			//printf("el pid que recibi es:%d\n", *pid);
			if (listaProcesos == NULL ) {
				log_info(logger, "lista procesos era null");
				//printf("lista procesos era null");
				listaProcesos = list_create();
			}
			agregarProceso(*pid, 'c');
			pthread_mutex_unlock(&mutexOperacion);
			log_debug(logger, "Terminé de crear proceso nuevo, de parte del Kernel.");
			break;
		}

		free(mensaje);
	}
	free(header);
	free(tamanoMensaje);
	free(socketKernel);
}
void atencionCpu(int *socketCPU) {
	log_info(logger, "Entró a atencionCpu");
	int fin=0;

	int procesoActivo = 0, parametro[4];

	int *tamanoMensaje = malloc(4);
	int * razon = malloc(sizeof(int));

	while(1){
	aplicarRetardo(retardo);
	char *mensaje=recibirConRazon(*socketCPU, razon, logger);
	if(mensaje==NULL){
			log_info(logger,"Se desconecto abruptamente una CPU, sd %d",*socketCPU); //TODO hay que hacer algo??
				break;//el close al socket lo hace recibirConRazon
			}
	switch (*razon) {

	case SOLICITAR_A_UMV:
		log_debug(logger, "Recibí solicitar a umv, de la CPU: %d",*socketCPU);
		pthread_mutex_lock(&mutexOperacion);
		cambiarProcesoActivo(procesoActivo);
		desempaquetar2(mensaje, &parametro[0], &parametro[1], &parametro[2], 0);
		log_debug(logger, "CPU %d solicitó base: %d, offset: %d, y tamano: %d.",*socketCPU, parametro[0], parametro[1], parametro[2]);
		char *respuesta = solicitarBytes(parametro[0], parametro[1],
				parametro[2]);

		log_debug(logger, "Y el resultado es buffer=%s", respuesta);
		pthread_mutex_unlock(&mutexOperacion);
		if (!strcmp(respuesta, "error")) {
			enviarConRazon(*socketCPU, logger, SEGMENTATION_FAULT, NULL);
			break;
		}
		*razon = RESPUESTA_A_SOLICITAR_BUFFER;
		*tamanoMensaje = parametro[2];
		if(!enviarConRazon(*socketCPU, logger, RESPUESTA_A_SOLICITAR_BUFFER, serializar2(
				crear_nodoVar(respuesta, *tamanoMensaje), 0))){
			fin=1;
			break;
		}
		free(respuesta);
		log_debug(logger, "Terminé solicitar a umv, de la CPU: %d",*socketCPU);
		break;

	case ESCRIBIR_EN_UMV:
		log_debug(logger, "Recibí escribir en umv, de la CPU: %d",*socketCPU);
		pthread_mutex_lock(&mutexOperacion);
		cambiarProcesoActivo(procesoActivo);
		char *buffer = malloc(*tamanoMensaje+1);
		desempaquetar2(mensaje, &parametro[0], &parametro[1], &parametro[2],
				buffer, 0);
		buffer[parametro[2]]= 0;
		log_debug(logger, "CPU %d escribió en, base: %d, offset: %d, tamano: %d, buffer: %s",*socketCPU, parametro[0], parametro[1], parametro[2], buffer);
		int resultado = enviarUnosBytes(parametro[0], parametro[1], parametro[2], buffer);
		if (resultado){
			enviarConRazon(*socketCPU, logger, CONFIRMACION, NULL);
		}else{
			enviarConRazon(*socketCPU, logger, SEGMENTATION_FAULT, NULL);
		}
		pthread_mutex_unlock(&mutexOperacion);
		free(buffer);
		log_debug(logger, "Terminé escribir en umv, de la CPU: %d",*socketCPU);
		break;
		case AGREGAR_VARIABLE:
			log_debug(logger, "Recibí escribir en umv, de la CPU: %d",*socketCPU);
			pthread_mutex_lock(&mutexOperacion);
			cambiarProcesoActivo(procesoActivo);
			char *buffer2 = malloc(*tamanoMensaje+1);
			desempaquetar2(mensaje, &parametro[0], &parametro[1], &parametro[2],
					buffer2, 0);
			buffer2[parametro[2]]= 0;
			log_debug(logger, "CPU %d escribió en, base: %d, offset: %d, tamano: %d, buffer: %s",*socketCPU, parametro[0], parametro[1], parametro[2], buffer2);
			int resultado2 = enviarUnosBytesPCPU(parametro[0], parametro[1], parametro[2], buffer2);
			if (resultado2){
				enviarConRazon(*socketCPU, logger, CONFIRMACION, NULL);
			}else{
				enviarConRazon(*socketCPU, logger, SEGMENTATION_FAULT, NULL);
			}
			pthread_mutex_unlock(&mutexOperacion);
			free(buffer2);
			log_debug(logger, "Terminé escribir en umv, de la CPU: %d",*socketCPU);
			break;
	case CAMBIAR_PROCESO_ACTIVO:
		log_debug(logger, "Recibí cambiar proceso activo, de la CPU: %d",*socketCPU);
		//printf("recibi cambiar proceso activo");
		pthread_mutex_lock(&mutexOperacion);
		int *pid2 = malloc(sizeof(int));
		desempaquetar2(mensaje, pid2, 0);
		procesoActivo = *pid2;
		log_info(logger, "El valor del PID es: %d", *pid2);
		//printf("\nel valor del pid es: %d\n", *pid2);
//		cambiarProcesoActivo(*pid2);
		free(pid2);
		pthread_mutex_unlock(&mutexOperacion);
		//puts("termine de cambiar proceso activo");
//		free(pid2); //TODO lo puse arriba del mutex... no se en qué varía pero creo q es mas seguro asi.
		log_debug(logger, "Terminé cambiar proceso activo, de la CPU: %d",*socketCPU);
		break;
	}
	free(mensaje);
	if(fin){
		break;
	}
	}
	free(tamanoMensaje);
	free(razon);
	free(socketCPU);
}

