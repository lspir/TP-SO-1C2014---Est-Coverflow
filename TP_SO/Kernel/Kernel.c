/*
 * Kernel.c
 *
 *  Created on: 27/05/2014
 *      Author: utnso
 */

#include "Kernel.h"




int kernel_main(int argc, char** argv) {
	//Verifico que se haya recibido por parámetro el archivo config.
	if (argc <= 1) {
		perror(
				"Se debe ingresar la dirección de un archivo de configuración\n");
		return EXIT_FAILURE;
	}
	t_config *config = config_create(argv[1]);

	//Verifico que estén todos los valores esperados en el config.
	if (validarConfig(config)) {
		config_destroy(config);
		perror("El archivo de configuración no es correcto");
		return EXIT_FAILURE;
	}

	//Cargo parámetros del config en variables de Kernel.
	cargarConfig(config);
	config_destroy(config);
	colaReady = queue_create();
	colaExit= queue_create();
	sem_init(colaExitMutex, 0,1);
	sem_init(colaExitVacio,0,0);
	sem_init(mutexVG, 0, 1);
	sem_init(grado_Multiprogramacion, 0, multiprogramacion);
	sem_init(colaReadyMutex, 0, 1);
	sem_init(vacioReady, 0, 0);
	/*struct sockaddr_in address;
	int sd_UMV;
	if ((sd_UMV = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
			perror("socket failed");
			exit(EXIT_FAILURE);
		}
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(puertoUMV);
*/



	pthread_t plp, pcp;
	int iretPLP, iretPCP;
int* parametrosPCP= NULL;
	iretPCP = pthread_create(&pcp, NULL, pcp_main, (void*) parametrosPCP);
	if (iretPCP) {
		fprintf(stderr, "Error - pthread_create() return code: %d\n", iretPCP);
		exit(EXIT_FAILURE);
	}
	printf("Hilo pcp exitoso");
	int* parametroPLP = NULL;
	iretPLP = pthread_create(&plp, NULL, plp_main, (void*) parametroPLP);
	if (iretPLP) {
		fprintf(stderr, "Error - pthread_create() return code: %d\n", iretPLP);
		exit(EXIT_FAILURE);
	}
	printf("Hilo plp exitoso");
	pthread_join(pcp, NULL );
	pthread_join(plp, NULL );
	return EXIT_FAILURE;
}

int32_t validarConfig(t_config *config) {
	if (!config_has_property(config, "PUERTO_PROG")) {
		perror("Falta PUERTO_PROG");
		return EXIT_FAILURE;
	}
	if (!config_has_property(config, "PUERTO_CPU")) {
		perror("Falta PUERTO_CPU");
		return EXIT_FAILURE;
	}
	if (!config_has_property(config, "QUANTUM")) {
		perror("Falta QUANTUM");
		return EXIT_FAILURE;
	}
	if (!config_has_property(config, "RETARDO")) {
		perror("Falta RETARDO");
		return EXIT_FAILURE;
	}
	if (!config_has_property(config, "MULTIPROGRAMACION")) {
		perror("Falta MULTIPROGRAMACION");
		return EXIT_FAILURE;
	}
	if (!config_has_property(config, "VALOR_SEMAFOROS")) {
		perror("Falta VALOR_SEMAFOROS");
		return EXIT_FAILURE;
	}
	if (!config_has_property(config, "SEMAFOROS")) {
		perror("Falta SEMAFOROS");
		return EXIT_FAILURE;
	}
	if (!config_has_property(config, "HIO")) {
		perror("Falta HIO");
		return EXIT_FAILURE;
	}
	if (!config_has_property(config, "ID_HIO")) {
		perror("Falta ID_HIO");
		return EXIT_FAILURE;
	}
	if (!config_has_property(config, "VARIABLES_GLOBALES")) {
		perror("Falta VARIABLES_GLOBALES");
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}

void cargarConfig(t_config *config) {
	puerto_programa = config_get_int_value(config, "PUERTO_PROG");
	puerto_CPU = config_get_int_value(config, "PUERTO_CPU");
	quantum = config_get_int_value(config, "QUANTUM");
	retardo = config_get_int_value(config, "RETARDO");
	multiprogramacion = config_get_int_value(config, "MULTIPROGRAMACION");
	//FIXME fijate como hicimos con las cosas de config arriba... por eso te tira error.
	valor_semaforos = config_get_int_value(config, "VALOR_SEMAFOROS");
	semaforos = config_get_int_value(config, "SEMAFOROS");
	hio = config_get_int_value(config, "HIO");
	idhio = config_get_int_value(config, "IDHIO");
	variables_globales = config_get_int_value(config, "VARIABLES_GLOBALES");
}
