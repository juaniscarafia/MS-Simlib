/* Modelo ensambladora de 3 piezas - A, B, C  */


#include "simlib.c"
#include <stdio.h>
#include <stdlib.h> 


#define Arribo_piezas							1  /* Tipo de Evento 1: arriban piezas de tipo A, B, C           */
#define Partida_ensambladora					2  /* Tipo de Evento 2: partida del producto terminado           */
#define Piezas_A								1  /* Lista 1: Piezas A								 			 */
#define Piezas_B								2  /* Lista 2: Piezas B								    		 */
#define Piezas_C								3  /* Lista 3: Piezas C							  			     */
#define Ensambladora							4  /* Lista 4: Ensambladora										 */
#define Demora_media_piezas_A					1  /* Sampst 1: Demora en Cola				 					 */


/* Declaraci¢n de variables propias */

float media_arribo_piezas, tiempo_fabricacion, tipo_pieza, fallida;
int productos, cantidad_B, cantidad_B_defectuosas;



/* Declaraci¢n de Funciones propias */

void inicializa(void);
void arribo_piezas(void);
void partida_ensambladora(void);
void reporte(void);


int main()  /* Main function. */
{
	/* Apertura de Archivos que sean necesarios */
	productos = 0;
	media_arribo_piezas = 4.5;
	tiempo_fabricacion = 6.9;
	cantidad_B = 0;
	cantidad_B_defectuosas = 0;
	/* Initializar Simlib */
	init_simlib();

	/* Establecer maxatr = M ximo n£mero de Atributos utilizados  */
	maxatr = 4;

	/* Initializar el Sistema */
	inicializa();

	/* Ejecutar la simulacion. */

	while (productos <= 10)
	{
		/* Determinar pr¢ximo Evento */
		timing();

		/* Invoke the appropriate event function. */
		switch (next_event_type)
		{
		case Arribo_piezas:
			arribo_piezas();
			break;
		case Partida_ensambladora:
			partida_ensambladora();
			break;
		}
	}
	reporte();
	system("pause");
}


void inicializa(void)  /* Inicializar el Sistema */
{
	/* Se carga el primer Arribo en la Lista de Eventos */

	transfer[1] = sim_time + expon(media_arribo_piezas, Arribo_piezas);
	transfer[2] = Arribo_piezas;
	list_file(INCREASING, LIST_EVENT);

}


void arribo_piezas(void)  /* Evento Arribo avion */
{
	/* Determinar pr¢ximo arribo y cargar en Lista de Eventos */
	transfer[1] = sim_time + expon(media_arribo_piezas, Arribo_piezas);
	transfer[2] = Arribo_piezas;
	list_file(INCREASING, LIST_EVENT);
	tipo_pieza = lcgrand(10);
	fallida = lcgrand(10);
	if (tipo_pieza < 0.30)
	{
		if (list_size[Ensambladora] == 0)
		{
			if ((list_size[Piezas_B] != 0) && (list_size[Piezas_C] != 0))
			{
				transfer[1] = sim_time + tiempo_fabricacion;
				transfer[2] = Partida_ensambladora;
				list_file(INCREASING, LIST_EVENT);
				list_remove(FIRST, Piezas_B);
				list_remove(FIRST, Piezas_C);
				list_file(FIRST, Ensambladora);
			}
			else
			{
				list_file(FIRST, Piezas_A);
				sampst(0.0, Demora_media_piezas_A);
			}
		}
		else
		{
			list_file(FIRST, Piezas_A);
			sampst(0.0, Demora_media_piezas_A);
		}
	}
	else if (tipo_pieza < 0.75)
	{
		cantidad_B = cantidad_B++;
		if (list_size[Ensambladora] == 0)
		{
			if ((list_size[Piezas_A] != 0) && (list_size[Piezas_C] != 0))
			{
				if (fallida < 0.10)
				{
					cantidad_B_defectuosas = cantidad_B_defectuosas++;
					list_file(FIRST, Piezas_B);
				}
				else
				{
					transfer[1] = sim_time + tiempo_fabricacion;
					transfer[2] = Partida_ensambladora;
					list_file(INCREASING, LIST_EVENT);
					list_remove(FIRST, Piezas_A);
					sampst(sim_time - transfer[1], Demora_media_piezas_A);
					list_remove(FIRST, Piezas_C);
					list_file(FIRST, Ensambladora);
				}
			}
			else
			{
				list_file(FIRST, Piezas_B);
			}
		}
		else
		{
			list_file(FIRST, Piezas_B);
		}

	}
	else
	{
		if (list_size[Ensambladora] == 0)
		{
			if ((list_size[Piezas_A] != 0) && (list_size[Piezas_B] != 0))
			{
				transfer[1] = sim_time + tiempo_fabricacion;
				transfer[2] = Partida_ensambladora;
				list_file(INCREASING, LIST_EVENT);
				list_remove(FIRST, Piezas_A);
				sampst(sim_time - transfer[1], Demora_media_piezas_A);
				list_remove(FIRST, Piezas_B);
				list_file(FIRST, Ensambladora);
			}
			else
			{
				list_file(FIRST, Piezas_C);
			}
		}
		else
		{
			list_file(FIRST, Piezas_C);
		}
	}
}


void partida_ensambladora(void)  /* Evento arribo aduana*/
{
	list_remove(FIRST, Ensambladora);
	int bandera = 0;
	productos = productos++;
	fallida = lcgrand(10);
	if ((list_size[Piezas_A] != 0) && (list_size[Piezas_B] != 0) && (list_size[Piezas_C] != 0))
	{
		while ((list_size[Piezas_B] != 0) || (bandera != 1))
		{
			if (fallida < 0.10)
			{
				cantidad_B_defectuosas = cantidad_B_defectuosas++;
				list_remove(FIRST, Piezas_B);
			}
			else
			{
				list_remove(FIRST, Piezas_A);
				sampst(sim_time - transfer[1], Demora_media_piezas_A);
				list_remove(FIRST, Piezas_B);
				list_remove(FIRST, Piezas_C);
				transfer[1] = sim_time + tiempo_fabricacion;
				transfer[2] = Partida_ensambladora;
				list_file(INCREASING, LIST_EVENT);
				list_file(FIRST, Ensambladora);
				bandera = 1;
				break;
			}
		}
	}
}


void reporte(void)  /* Generar Reporte de la Simulaci¢n */
{
	/* Mostrar Par metros de Entrada */

	/* -------- Por pantalla -------- */

	printf("Sistema ensambladora 3 piezas \n\n");

	/* Calcular los Estad¡sticos */

	/* Estad¡sticos Escalaras - Sampst */
	sampst(0.0, -Demora_media_piezas_A);
	printf("\nDemora media de la pieza A            : %f \n ", transfer[1]);

	/* Estad¡sticos Temporales - Timest y Filest */

	filest(Piezas_A);
	printf("\nNumero medio en cola de piezas A   : %f \n ", transfer[1]);
	filest(Piezas_B);
	printf("\nNumero medio en cola de piezas B   : %f \n ", transfer[1]);
	filest(Ensambladora);
	float porcentaje_defectuosas_B = 0;
	porcentaje_defectuosas_B = (cantidad_B * cantidad_B_defectuosas) / 100;
	printf("\nUtilizacion de ensambladora        : %f \n ", transfer[1]);
	printf("\nTiempo Final de Simulation     : %10.3f minutes\n", sim_time);
}
