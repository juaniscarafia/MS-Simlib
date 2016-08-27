/* Modelo MM1 - Un Servidor, Una Cola  */

#include "simlib.c"

#define Arribo            1  /* Tipo de Evento 1: Arribos           */
#define Partida           2  /* Tipo de Evento 2: Partidas          */
#define Servidor          1  /* Lista 1: Servidor                   */
#define Cola              2  /* Lista 2: Cola                       */
#define Servidor2         3  /* Lista 1: Servidor 2                 */
#define Demora_cola       1  /* Sampst 1: Demora en Cola            */

/* Declaraci¢n de variables propias */

float media_interarribos, media_servicio;
int num_clientes, clientes_act, i;

/* Declaraci¢n de Funciones propias */

void inicializa(void);
void Rutina_arribos(void);
void Rutina_partidas(void);
void reporte(void);

int main()  /* Main function. */
{
	/* Apertura de Archivos que sean necesarios */

	//infile  = fopen("\\entrada.txt", "r");

	media_interarribos = 0.3;
	media_servicio = 0.5;
	num_clientes = 1000;

	/* Initializar Simlib */
	init_simlib();

	/* Establecer maxatr = M ximo n£mero de Atributos utilizados  */
	maxatr = 4;

	/* Initializar el Sistema */
	inicializa();

	/* Ejecutar la simulaci¢n. */

	clientes_act = 0;

	while (clientes_act < num_clientes)
	{
		/* Determinar pr¢ximo Evento */
		timing();

		/* Invoke the appropriate event function. */

		switch (next_event_type)
		{
		case Arribo:
			Rutina_arribos();
			break;
		case Partida:
			Rutina_partidas();
			break;
		}
	}

	/* Llamada al Reporte para mostrar los resultados */
	reporte();
	getchar();
}

void inicializa(void)  /* Inicializar el Sistema */
{
	/* Se carga el primer Arribo en la Lista de Eventos */

	transfer[1] = sim_time + expon(media_interarribos, Arribo);
	transfer[2] = Arribo;
	list_file(INCREASING, LIST_EVENT);
}

void Rutina_arribos(void)  /* Evento Arribo */
{
	/* Determinar pr¢ximo arribo y cargar en Lista de Eventos */

	transfer[1] = sim_time + expon(media_interarribos, Arribo);
	transfer[2] = Arribo;
	list_file(INCREASING, LIST_EVENT);

	/* Chequear si el Servidor est  desocupado */

	if (list_size[Servidor] == 0)
	{
		++clientes_act;

		/* Si est  desocupado ocuparlo y generar la partida */

		list_file(FIRST, Servidor);
		sampst(0.0, Demora_cola);

		transfer[1] = sim_time + expon(media_servicio, Partida);
		transfer[2] = Partida;
		transfer[3] = Servidor;
		list_file(INCREASING, LIST_EVENT);
	}
	else
	{
		if (list_size[Servidor2] == 0) {
			++clientes_act;
			list_file(FIRST, Servidor2);
			sampst(0.0, Demora_cola);

			transfer[1] = sim_time + expon(media_servicio, Partida);
			transfer[2] = Partida;
			transfer[3] = Servidor2;
			list_file(INCREASING, LIST_EVENT);
		}
		else
		{
			/* Si el Servidor est  ocupado poner el Trabajo en Cola */

			transfer[1] = sim_time;
			list_file(LAST, Cola);
		}
	}
}

void Rutina_partidas(void)  /* Evento Partida */
{
	/* Desocupar el Servidor */

	int temp_servidor = transfer[3];

	if (temp_servidor == Servidor)
	{
		list_remove(FIRST, Servidor);
	}
	else if (temp_servidor == Servidor2)
	{
		list_remove(FIRST, Servidor2);
	}

	/* Ver si hay trabajos en cola */

	if (list_size[Cola] > 0)
	{
		/* Sacar el primero de la cola y actualizar Demoras */

		++clientes_act;
		list_remove(FIRST, Cola);
		sampst(sim_time - transfer[1], Demora_cola);

		/* Cargar en el Servidor y generar la partida */
		list_file(FIRST, temp_servidor);

		transfer[1] = sim_time + expon(media_servicio, Partida);
		transfer[2] = Partida;
		transfer[3] = temp_servidor;
		list_file(INCREASING, LIST_EVENT);
	}
}

void reporte(void)  /* Generar Reporte de la Simulaci¢n */
{
	/* Mostrar Par metros de Entrada */

	/* -------- Por pantalla -------- */

	printf("Sistema M/M/1 - Simulaci¢n con Simlib \n\n");
	printf("Media de Interarribos:			 %8.3f minutos\n", media_interarribos);
	printf("Media de Servicios:			 %8.3f minutos\n", media_servicio);
	printf("Cantidad de Clientes Demorados:  %i \n\n", num_clientes);

	/* Calcular los Estad¡sticos */

	/* Estad¡sticos Escalaras - Sampst */
	sampst(0.0, -Demora_cola);
	printf("\nDemora en Cola                 : %f \n ", transfer[1]);

	/* Estad¡sticos Temporales - Timest y Filest */

	filest(Cola);
	printf("\nN£mero Promedio en Cola        : %f \n ", transfer[1]);
	filest(Servidor);
	printf("\nUtilizaci¢n Servidor           : %f \n ", transfer[1]);
	printf("\nTiempo Final de Simulation     : %10.3f minutes\n", sim_time);
}