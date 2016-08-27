#include "simlib.c"

#pragma region Events

#define GeneratedClientRequestEvent			1
#define GenerateServerRequestEvent			2
#define FinishRequestEvent					3
#define GenerateChannelFailEvent			4

#pragma endregion

#pragma region Queue

#define serverQueue							1
#define clientQueue							2
#define channelQueue						3

#pragma endregion

#pragma region Statistical

int systemFails;
int serverRequestCount;
#define ClientStatistical1      1  /* Demora en Cola estacion 1    */
#define ClientStatistical2      2  /* Demora en Cola estacion 2    */
#define ClientStatistical3      3  /* Demora en Cola estacion 3    */
#define ClientStatistical4      4  /* Demora en Cola estacion 4    */
#define ClientStatistical5      5  /* Demora en Cola estacion 5    */
#define ClientStatistical6      6  /* Demora en Cola estacion 6    */

#pragma endregion

#pragma region Properties

#define server								1
#define client								2
#define fullUsage							3
#define partialUsage						2
int channelCapacity;
bool disablePrint;

#pragma endregion

#pragma region Transfer properties

#define Transfer_EventTime						1 // Tiempo del evento.
#define	Transfer_EventType						2 // Tipo de evento.
#define Transfer_ChannelUser					3 // Indica quien usa el canal: Servidor o cliente.
#define Transfer_ClientNumber					4 // Numero de la estación.
#define Transfer_RemainderTime					5 // Tiempo restante.
#define Transfer_ChannelUsage					6 // Uso del canal

#pragma endregion

typedef struct { float v[9]; } transferStruct;

#pragma region Methods

void Initialize(void);
void GenerateReport(void);
void GenerateServerRequest(void);
void GenerateClientRequest(void);
void FinishRequest(void);
void GenerateChannelFail(void);
int ServerChannelUsage(void);
void RemoveServerForChannel(void);
void AddEventToList(int option, int list);
transferStruct CopyTransfer(void);

#pragma endregion

int main()  /* Main function. */
{
	/* Initializar Simlib */
	init_simlib();

	/* Establecer maxatr = Máximo número de Atributos utilizados  */
	maxatr = 6;
	int maxTimeSim = 24 * 3600;
	channelCapacity = 3;
	systemFails = 0;
	disablePrint = false;

	list_rank[channelQueue] = Transfer_EventType;
	list_rank[serverQueue] = Transfer_EventType;
	list_rank[clientQueue] = Transfer_EventTime;

	/* Initializar el Sistema */
	Initialize();
	while (sim_time <= maxTimeSim)
	{
		timing();

		switch (next_event_type)
		{
		case GeneratedClientRequestEvent:
			GenerateClientRequest();
			break;
		case GenerateServerRequestEvent:
			GenerateServerRequest();
			break;
		case FinishRequestEvent:
			FinishRequest();
			break;
		case GenerateChannelFailEvent:
			GenerateChannelFail();
			break;
		}
	}

	/* Llamada al Reporte para mostrar los resultados */
	GenerateReport();

	getchar();
	return 0;
}

void Initialize(void)  /* Inicializar el Sistema */
{
	for (int i = 0; i < 6; i++)
	{
		transfer[Transfer_EventTime] = sim_time + expon(1.2, GeneratedClientRequestEvent);
		transfer[Transfer_EventType] = GeneratedClientRequestEvent;
		transfer[Transfer_ChannelUser] = client;
		transfer[Transfer_ClientNumber] = i; // Estación
		AddEventToList(INCREASING, LIST_EVENT);
	}

	transfer[Transfer_EventTime] = sim_time + expon(3.7, GenerateServerRequestEvent);
	transfer[Transfer_EventType] = GenerateServerRequestEvent;
	transfer[Transfer_ChannelUser] = server;
	AddEventToList(INCREASING, LIST_EVENT);

	transfer[Transfer_EventTime] = sim_time + uniform(3600 * 3, 3600 * 4, GenerateChannelFailEvent);
	transfer[Transfer_EventType] = GenerateChannelFailEvent;
	AddEventToList(INCREASING, LIST_EVENT);
}

int GetChannelUsage()
{
	float channelUsage = lcgrand(channelQueue);
	int usage = 0;
	if (channelUsage <= 0.1)
	{
		usage = fullUsage;
	}
	else
	{
		usage = partialUsage;
	}

	return usage;
}

void AddServerToChannel(int usage, float timeUsage)
{
	if (timeUsage == 0) // Si es un pedido nuevo.
	{
		timeUsage = sim_time + expon(2.4, GenerateServerRequestEvent); //0.75
	}
	else
	{
		timeUsage += sim_time;
	}

	for (int i = 0; i < usage; i++)
	{
		transfer[Transfer_EventTime] = sim_time;
		transfer[Transfer_ChannelUser] = server;
		transfer[Transfer_ChannelUsage] = usage;
		transfer[Transfer_RemainderTime] = timeUsage;
		AddEventToList(LAST, channelQueue);
	}

	transfer[Transfer_EventTime] = timeUsage;
	transfer[Transfer_EventType] = FinishRequestEvent;
	transfer[Transfer_ChannelUser] = server;
	AddEventToList(INCREASING, LIST_EVENT);
}

void AddClientToChannel(int clientNumber, float timeUsage)
{
	if (timeUsage == 0) // Si es un pedido nuevo.
	{
		timeUsage = sim_time + expon(3.7, GeneratedClientRequestEvent); //1.2);
	}
	else
	{
		timeUsage += sim_time;
	}

	transfer[Transfer_EventTime] = sim_time;
	transfer[Transfer_ChannelUser] = client;
	transfer[Transfer_ChannelUsage] = 1;
	transfer[Transfer_ClientNumber] = clientNumber;
	transfer[Transfer_RemainderTime] = timeUsage;
	AddEventToList(LAST, channelQueue);

	transfer[Transfer_EventTime] = timeUsage;
	transfer[Transfer_EventType] = FinishRequestEvent;
	transfer[Transfer_ChannelUser] = client;
	transfer[Transfer_ClientNumber] = clientNumber;
	AddEventToList(INCREASING, LIST_EVENT);
}

void RemoveClientForChannel(int usage)
{
	int channelUsage = list_size[channelQueue];
	for (int i = 0; i < channelUsage; i++)
	{
		if (list_size[channelQueue] > 0)
		{
			list_remove(LAST, channelQueue);

			if (transfer[Transfer_ChannelUser] == client)
			{
				serverRequestCount++;
				transfer[Transfer_RemainderTime] = transfer[Transfer_RemainderTime] - sim_time;
				// Tiempo en el que tendría que finalizar el pedido - tiempo actual.
				int clientNumber = transfer[Transfer_ClientNumber];
				AddEventToList(FIRST, clientQueue);

				int eventCount = list_size[LIST_EVENT];
				if (eventCount > 0)
				{
					for (int i = 0; i < eventCount; i++)
					{
						list_remove(FIRST, LIST_EVENT);

						bool isFinishClientEvent = transfer[Transfer_EventType] == FinishRequestEvent
							&& transfer[Transfer_ChannelUser] == client
							&& transfer[Transfer_ClientNumber] == clientNumber;
						if (!isFinishClientEvent)
						{
							AddEventToList(LAST, LIST_EVENT);
						}
					}
				}
			}
			else // Si es un servidor sigue utilizando el canal.
			{
				AddEventToList(FIRST, channelQueue);
			}
		}
	}
}

void RemoveServerForChannel(void)
{
	int channelUsage = list_size[channelQueue];
	for (int i = 0; i < channelUsage; i++)
	{
		if (list_size[channelQueue] > 0)
		{
			list_remove(LAST, channelQueue);

			if (transfer[Transfer_ChannelUser] == server)
			{
				transfer[Transfer_RemainderTime] = transfer[Transfer_RemainderTime] - sim_time;
				// Tiempo en el que tendría que finalizar el pedido - tiempo actual.
				AddEventToList(FIRST, serverQueue);

				int eventCount = list_size[LIST_EVENT];
				if (eventCount > 0)
				{
					for (int i = 0; i < eventCount; i++)
					{
						list_remove(FIRST, LIST_EVENT);

						bool isFinishServerEvent = transfer[Transfer_EventType] == FinishRequestEvent
							&& transfer[Transfer_ChannelUser] == server;
						if (!isFinishServerEvent)
						{
							AddEventToList(LAST, LIST_EVENT);
						}
					}
				}
			}
			else // Si es un estación sigue utilizando el canal.
			{
				AddEventToList(FIRST, channelQueue);
			}
		}
	}
}

void AddRequestToServerQueue(int usage)
{
	transfer[Transfer_EventTime] = sim_time;
	transfer[Transfer_ChannelUser] = server;
	transfer[Transfer_ChannelUsage] = usage;
	AddEventToList(DECREASING, serverQueue);
}

/// Determina el uso del canal de comunación con el pedido generado.
bool IsChannelAvailable(int usage)
{
	int channelUsage = list_size[channelQueue] + usage;
	return channelUsage <= channelCapacity;
}

void GenerateServerRequest(void)
{
	// Determino que uso del canal va a tener el servidor
	int usage = GetChannelUsage();

	bool isChannelAvalible = IsChannelAvailable(usage);
	if (!isChannelAvalible)
	{
		int serverChannelUsage = ServerChannelUsage();

		if (serverChannelUsage == 0)
		{
			RemoveClientForChannel(usage);

			AddServerToChannel(usage, 0);
		}
		else if (serverChannelUsage == fullUsage) // Existe un pedido full del servidor.
		{
			AddRequestToServerQueue(usage);
		}
		else // El server esta haciendo uso del canal.
		{
			if (usage == partialUsage)
			{
				AddRequestToServerQueue(usage);
			}
			else
			{
				RemoveClientForChannel(usage);
				RemoveServerForChannel();

				AddServerToChannel(usage, 0);
			}
		}
	}
	else
	{
		AddServerToChannel(usage, 0);
	}

	// Genero el siguiente pedido del servidor.
	transfer[Transfer_EventTime] = sim_time + expon(3.7, GenerateServerRequestEvent);
	transfer[Transfer_EventType] = GenerateServerRequestEvent;
	transfer[Transfer_ChannelUser] = server;
	AddEventToList(INCREASING, LIST_EVENT);
}

void AddRequestToClientQueue(int clientNumber)
{
	transfer[Transfer_EventTime] = sim_time;
	transfer[Transfer_ChannelUser] = client;
	transfer[Transfer_ClientNumber] = clientNumber;
	transfer[Transfer_ChannelUsage] = 1; // Uso del servidor

	AddEventToList(INCREASING, clientQueue);
}

void GenerateClientRequest(void)
{
	int clientNumber = transfer[Transfer_ClientNumber];
	bool isChannelAvailable = IsChannelAvailable(1);
	if (isChannelAvailable)
	{
		int i = clientNumber + 1;
		sampst(0.0, i);
		AddClientToChannel(clientNumber, 0);
	}
	else
	{
		AddRequestToClientQueue(clientNumber);
	}
}

void FinishRequest(void)
{
	transferStruct tempTransfer = CopyTransfer();

	// El transfer tiene la información del pedido que finalizo.
	bool isServer = tempTransfer.v[Transfer_ChannelUser] == server;
	int clientNumber = tempTransfer.v[Transfer_ClientNumber];

	int channelUsage = list_size[channelQueue];
	for (int i = 0; i < channelUsage; i++)
	{
		list_remove(FIRST, channelQueue);

		bool remove = transfer[Transfer_ChannelUser] == tempTransfer.v[Transfer_ChannelUser]
			&& transfer[Transfer_ClientNumber] == tempTransfer.v[Transfer_ClientNumber];

		if (!remove)
		{
			AddEventToList(LAST, channelQueue);
		}
	}

	if (!isServer) // Si es una estación genero su siguiente pedido.
	{
		transfer[Transfer_EventTime] = sim_time + expon(1.2, GeneratedClientRequestEvent);
		transfer[Transfer_EventType] = GeneratedClientRequestEvent;
		transfer[Transfer_ChannelUser] = client;
		transfer[Transfer_ClientNumber] = clientNumber;
		transfer[Transfer_RemainderTime] = 0;
		transfer[Transfer_ChannelUsage] = 1;
		AddEventToList(INCREASING, LIST_EVENT);
	}

	if (isServer && list_size[serverQueue] > 0)
	{
		list_remove(FIRST, serverQueue);

		// Simpre va existir espacio en el canal para el uso del servidor.
		// La terminación de un pedido Full puede generar el inicio de otro pedido Full o parcial.
		// Pero la terminación de un pedido parcial unicamente va a generar pedidos parciles y no full,
		// ya que si se hubiera genera un pedido full durante el uso del pedido parcial,
		// este ultimo habria sido removido del canal. (No puede existir pedido Full en cola en este caso)
		AddServerToChannel(transfer[Transfer_ChannelUsage], transfer[Transfer_RemainderTime]); // Utilizo nuevamente el servidor(uso, tiempo restante)
	}

	while (list_size[channelQueue] < channelCapacity && list_size[clientQueue]>0)
	{
		list_remove(FIRST, clientQueue);

		// Demora de la estación en cola.
		float delay = sim_time - transfer[Transfer_EventTime];
		int clientStatistica = transfer[Transfer_ClientNumber] + 1;
		sampst(delay, clientStatistica);

		AddClientToChannel(transfer[Transfer_ClientNumber], transfer[Transfer_RemainderTime]);
	}
}

void GenerateChannelFail(void)
{
	systemFails++;

	// Cancelo la finalización de pedidos que se encuentra usando el canal.
	bool cancelEvents = true;
	while (cancelEvents)
	{
		int event = event_cancel(FinishRequestEvent);
		if (event == 0)
		{
			cancelEvents = false;
		}
	}

	// limpia las colas de pedidos del servidor y las estaciones.
	while (list_size[clientQueue] > 0)
	{
		list_remove(FIRST, clientQueue);
	}

	while (list_size[serverQueue] > 0)
	{
		list_remove(FIRST, serverQueue);
	}

	while (list_size[channelQueue] > 0)
	{
		list_remove(FIRST, channelQueue);
	}

	/*Inicializa los eventos nuevamente*/
	Initialize();
}

int ServerChannelUsage(void)
{
	int serverChannelUsage = 0;
	for (int i = 0; i < list_size[channelQueue]; i++)
	{
		list_remove(FIRST, channelQueue);
		if (transfer[Transfer_ChannelUser] == server)
		{
			serverChannelUsage++;
		}

		AddEventToList(LAST, channelQueue);
	}

	return serverChannelUsage;
}

void GenerateReport(void)
{
	printf("Sistema Servidor - Simulacion con Simlib \n\n");
	printf("Tiempo simulado				:%10.3f segundos \n\n", sim_time);

	filest(channelQueue);
	float channelUsage = transfer[1] / 3 * 100;
	printf("Uso del canal de comunicación: %10.3f %% \n\n", channelUsage);

	filest(serverQueue);
	float mediaQueue = transfer[1];
	printf("Cantidad media de pedidos del servidor en cola de espera: %10.3f \n\n", transfer[1]);

	filest(clientQueue);
	mediaQueue += transfer[1];
	mediaQueue = mediaQueue / 2;
	printf("Cantidad media de pedidos en cola de espera: %10.3f \n\n", mediaQueue);

	printf("Cantidad de pedido de estaciones que dejan el canal de comunicación para uso del servidor: %i \n\n", serverRequestCount);
	printf("Cantidad de fallas generales ocuridas: %i \n\n", systemFails);

	for (int i = 1; i < 7; i++)
	{
		sampst(0.0, -i);
		printf("Tiempo medio de espera para los pedidos de la estacion %i:  %f \n", i, transfer[1]);
	}
}

void AddEventToList(int option, int list)
{
	if (disablePrint) {
		printf("%10.3f : Tiempo del evento : %10.3f  Tipo de evento: %10.3f  Usuario del canal: %10.3f  Estación: %10.3f  Tiempo restante: %10.3f  Uso del canal : %10.3f \n \n",
			sim_time,
			transfer[Transfer_EventTime],
			transfer[Transfer_EventType],
			transfer[Transfer_ChannelUser],
			transfer[Transfer_ClientNumber],
			transfer[Transfer_RemainderTime],
			transfer[Transfer_ChannelUsage]);
	}

	list_file(option, list);
}

transferStruct CopyTransfer(void)
{
	transferStruct temp;
	for (int i = 1; i <= maxatr; i++)
	{
		temp.v[i] = transfer[i];
	}

	return temp;
}