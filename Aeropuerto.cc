#include "simlib.c"
#include <stdio.h>

#pragma region Events

#define llegada_colectivo_parada		1
#define llegada_colectivo_areopuerto	2
#define mantenimiento_colectivo			3
#define arribo_pasajero_parada_taxis	4
#define arribo_pasajero_aeropuerto		5

#pragma endregion

#pragma region Colas

#define cola_pasajeros_aeropuerto		1
#define cola_pasajeros_parada_taxis		2
#define cola_colectivo					3

#pragma endregion

#pragma region Estadisticos

#define demora_cola						1

#pragma endregion

#pragma region Properties

int maxSizeBus;
bool doFix;

#pragma endregion

#pragma region Methods

void inicializa(void);
void reporte(void);
void llegadaColectivoParada(void);
void llegadaColectivoAeropuerto(void);
void mantenimiento(void);
void arriboTaxi(void);
void arrivoAeropuerto(void);

#pragma endregion

int main()  /* Main function. */
{
	/* Initializar Simlib */
	init_simlib();

	/* Establecer maxatr = Máximo n£mero de Atributos utilizados  */
	maxatr = 4;
	int maxTimeSim = 24 * 3600;

	/* Initializar el Sistema */
	inicializa();
	while (sim_time <= maxTimeSim)
	{
		timing();

		switch (next_event_type)
		{
		case llegada_colectivo_parada:
			llegadaColectivoParada();
			break;

		case llegada_colectivo_areopuerto:
			llegadaColectivoAeropuerto();
			break;

		case mantenimiento_colectivo:
			mantenimiento();
			break;

		case arribo_pasajero_parada_taxis:
			arriboTaxi();
			break;

		case arribo_pasajero_aeropuerto:
			arrivoAeropuerto();
			break;
		}
	}

	/* Llamada al Reporte para mostrar los resultados */
	reporte();

	getchar();
	return 0;
}

void inicializa(void)  /* Inicializar el Sistema */
{
	maxSizeBus = 25;

	//inicializacion mantenimiento
	transfer[1] = sim_time + 3600 * 4;
	transfer[2] = mantenimiento_colectivo;
	list_file(INCREASING, LIST_EVENT);

	transfer[1] = sim_time + 60 * 30;
	transfer[2] = llegada_colectivo_parada;
	list_file(INCREASING, LIST_EVENT);

	transfer[1] = sim_time + expon(12.8 * 60, arribo_pasajero_aeropuerto);
	transfer[2] = arribo_pasajero_aeropuerto;
	list_file(INCREASING, LIST_EVENT);

	transfer[1] = sim_time + expon(7.9 * 60, arribo_pasajero_parada_taxis);
	transfer[2] = arribo_pasajero_parada_taxis;
	list_file(INCREASING, LIST_EVENT);
}

void llegadaColectivoParada(void)
{
	while (list_size[cola_colectivo] > 0)
	{
		list_remove(FIRST, cola_colectivo);
	}

	if (doFix)
	{
		doFix = false;

		transfer[1] = sim_time + 60 * 15;
		transfer[2] = llegada_colectivo_parada;
		list_file(INCREASING, LIST_EVENT);
	}
	else
	{
		while (list_size[cola_pasajeros_parada_taxis] > 0 && list_size[cola_colectivo] <= maxSizeBus)
		{
			list_remove(FIRST, cola_pasajeros_parada_taxis);

			sampst(sim_time - transfer[1], demora_cola);

			transfer[2] = sim_time;

			list_file(LAST, cola_colectivo);
		}

		transfer[1] = sim_time + uniform(10 * 60, 15 * 60, llegada_colectivo_areopuerto);
		transfer[2] = llegada_colectivo_areopuerto;
		list_file(INCREASING, LIST_EVENT);
	}
}

void llegadaColectivoAeropuerto(void)
{
	while (list_size[cola_colectivo] > 0)
	{
		list_remove(FIRST, cola_colectivo);
	}

	while (list_size[cola_pasajeros_aeropuerto] > 0 && list_size[cola_colectivo] <= maxSizeBus)
	{
		list_remove(FIRST, cola_pasajeros_aeropuerto);

		sampst(sim_time - transfer[1], demora_cola);

		transfer[2] = sim_time;

		list_file(LAST, cola_colectivo);
	}

	transfer[1] = sim_time + uniform(10 * 60, 15 * 60, llegada_colectivo_areopuerto);
	transfer[2] = llegada_colectivo_parada;
	list_file(INCREASING, LIST_EVENT);
}

void mantenimiento(void)
{
	doFix = true;

	transfer[1] = sim_time + 3600 * 4;
	transfer[2] = mantenimiento_colectivo;
	list_file(INCREASING, LIST_EVENT);
}

void arrivoAeropuerto(void)
{
	transfer[1] = sim_time + expon(12.8 * 60, arribo_pasajero_aeropuerto);
	transfer[2] = arribo_pasajero_aeropuerto;
	list_file(INCREASING, LIST_EVENT);

	float pasajeros = lcgrand(arribo_pasajero_aeropuerto);
	int cant = 0;
	if (pasajeros <= 0.3)
	{
		cant = 1;
	}
	else if (pasajeros > 0.3 && pasajeros <= 0.75)
	{
		cant = 2;
	}
	else if (pasajeros > 0.75 && pasajeros <= 0.90)
	{
		cant = 3;
	}
	else
	{
		cant = 4;
	}

	for (int i = 1; i <= cant; i++)
	{
		transfer[1] = sim_time;
		list_file(LAST, cola_pasajeros_aeropuerto);
	}
}

void arriboTaxi(void)
{
	transfer[1] = sim_time + expon(7.9 * 60, arribo_pasajero_parada_taxis);
	transfer[2] = arribo_pasajero_parada_taxis;
	list_file(INCREASING, LIST_EVENT);

	float pasajeros = lcgrand(arribo_pasajero_parada_taxis);
	int cant = 0;
	if (pasajeros <= 0.3)
	{
		cant = 1;
	}
	else if (pasajeros > 0.3 && pasajeros <= 0.75)
	{
		cant = 2;
	}
	else if (pasajeros > 0.75 && pasajeros <= 0.90)
	{
		cant = 3;
	}
	else
	{
		cant = 4;
	}

	for (int i = 1; i <= cant; i++)
	{
		transfer[1] = sim_time;
		list_file(LAST, cola_pasajeros_parada_taxis);
	}
}

void reporte(void)
{
	printf("Sistema Aeropuerto - Simulacion con Simlib \n\n");
	printf("Tiempo simulado				:%10.3f segundos \n \n", sim_time);

	sampst(0.0, -demora_cola);
	printf("Demora de las personas en cola\n");
	printf("Media						:%10.3f segundos\n", transfer[1]);
	printf("Máxima						:%10.3f segundos", transfer[3]);

	printf("\n");
	printf("\n");

	float promedio = 0;
	printf("Promedio de persona en cola \n");
	filest(cola_pasajeros_parada_taxis);
	printf("Parada taxis				:%f \n", transfer[1]);
	promedio += transfer[1];
	filest(cola_pasajeros_aeropuerto);
	printf("Parada aeropuerto			:%f \n", transfer[1]);
	promedio += transfer[1];
	promedio = promedio / 2;
	printf("Promedio ambas paradas		:%f \n", promedio);
	printf("\n");
	printf("\n");

	filest(cola_colectivo);
	printf("Cantidad promedio de personas transportadas	:%f \n", transfer[1]);
}