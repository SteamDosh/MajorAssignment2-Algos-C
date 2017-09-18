#include <iostream>									//i/o stream
#include <string>									//string lib
#include <fstream>									//i/o stream
#include <cctype>									//char handling
#include <iomanip>									//text formatting
#include <sstream>
using namespace std;

//**********************************************************
//*****           Classes and Structs            ***********
//**********************************************************
struct customer 
{
	double arrivalTime;
	double time;
};

struct server
{
	double efficiency;
	int customersServed;
	double idleTime;
	double busyTime;
	double startTime;

	server* Ptr;
	server* BeforePtr;
};

struct event 
{
	int type;
	double time;
};

//**********************************************************
//*****              Global Variables            ***********
//**********************************************************
const int g_MaxServers = 20;
int g_CurrentServers = -1;

server* g_ServerLinkedList = NULL;
server* g_BusyServers[g_MaxServers];

event g_nextEvent;

customer* g_Queue[10];
int g_CurrentQueue = 0;

double g_currentTime = 0.0;

//**********************************************************
//*****                Misc  Functions           ***********
//**********************************************************

void insertServer(server* aServer)
{
	if (g_ServerLinkedList)
	{
		if (g_ServerLinkedList->Ptr == NULL)
		{
			if (g_ServerLinkedList->efficiency > aServer->efficiency)
			{
				aServer->Ptr = g_ServerLinkedList;
				g_ServerLinkedList->BeforePtr = aServer;
				g_ServerLinkedList = aServer;
			}
			else
			{
				g_ServerLinkedList->Ptr = aServer;
				aServer->BeforePtr = g_ServerLinkedList;
			}
			return;
		}
		server* tmp = g_ServerLinkedList;
		server* prevtmp = NULL;
		while (tmp)
		{
			if (tmp->efficiency > aServer->efficiency)
			{
				aServer->Ptr = tmp;											//shift old server back
				if(tmp->BeforePtr) tmp->BeforePtr->Ptr = aServer;			//Make previous ptr look at me
				aServer->BeforePtr = tmp->BeforePtr;						//Update previous Ptr of new server
				tmp->BeforePtr = aServer;									//Update previous ptr of old server
				return;
			}
			else
			{
				prevtmp = tmp;
				tmp = tmp->Ptr;
			}
		}
		prevtmp->Ptr = aServer;
		aServer->BeforePtr = prevtmp;
	}
	else
	{
		g_ServerLinkedList = aServer;
	}
}

void insertBusyServer(server* aServer, customer* aCustomer)
{
	g_currentTime = aCustomer->arrivalTime;
	aServer->customersServed++;
	aServer->BeforePtr = NULL;
	aServer->Ptr = NULL;
	aServer->busyTime = g_currentTime + (aServer->efficiency * aCustomer->time);
	aServer->startTime = g_currentTime;
	
	for (int i = 0; i < g_MaxServers; i++)
	{
		if (!g_BusyServers[i])
		{
			g_BusyServers[i] = aServer;
			break;
		}
	}
}

//**********************************************************
//*****                Queue Functions           ***********
//**********************************************************

void pop(server* aServer, customer* aCustomer)
{
	if (g_ServerLinkedList->Ptr)
	{
		g_ServerLinkedList = aServer->Ptr;
		g_ServerLinkedList->BeforePtr = NULL;
	}
	else
	{
		g_ServerLinkedList = NULL;
	}

	insertBusyServer(aServer,aCustomer);
}

int getNextServer()
{
	int tmp = -1;
	for (int i = 0; i < g_MaxServers; i++)
	{
		if (g_BusyServers[i])
		{
			if (tmp != -1)
			{
				if (g_BusyServers[i]->busyTime < g_BusyServers[tmp]->busyTime)
				{
					tmp = i;
				}
			}
			else
			{
				tmp = i;
			}
		}
	}

	return tmp;
}

void checkBusy()
{
	server* nextServer = g_BusyServers[getNextServer()];

	if (nextServer)
	{
		g_currentTime = nextServer->busyTime;
		nextServer->busyTime = 0;
		insertServer(nextServer);
	}
}

int getNextEvent(customer* aCustomer)
{
	double tmp = g_currentTime;
	server* servertmp = g_BusyServers[getNextServer()];

	if (g_ServerLinkedList)
	{
		return 0;	//Server is available
	}
	else if (!g_ServerLinkedList)
	{
		return 1;	//Customer is in Queue
	}
	else if (g_currentTime > servertmp->busyTime)
	{
		return 2;	//Customer is Leaving
	}

	return 0;		//Customer is next Event
}

void checkQueue(customer* aCustomer)
{
	while (getNextEvent(aCustomer) != 1)
	{
		checkBusy();
	}

	if (g_ServerLinkedList != NULL)//Available Server
	{
		pop(g_ServerLinkedList, aCustomer);
	}

}


//**********************************************************
//*****                Reading File              ***********
//**********************************************************
void ReadFile(string aFileName)
{
	ifstream fin(aFileName.c_str());

	if (!fin.good())
	{
		cerr << "Could not open data file!\n";
		return;
	}

	int index = 0;
	int processedServers = 0;
	string a;
	while (getline(fin,a)) 
	{
		if (g_CurrentServers == -1)	//Assign amount of Servers
		{
			g_CurrentServers = stoi(a);
			continue;
		}
		else if (processedServers != g_CurrentServers)	//Assign Servers
		{
			insertServer(new server{ stod(a), processedServers, 0, 0, 0, NULL, NULL});
			processedServers++;
		}
		else //Read in Customers
		{
			auto iss = istringstream(a);

			double a, b;
			iss >> a >> b;
			checkQueue(new customer{ a, b });
		}
	}

	server* tmp = g_ServerLinkedList;
	while (tmp)
	{
		cout << tmp->efficiency << endl;
		tmp = tmp->Ptr;
	}
	for (int i = 0; i < g_CurrentQueue; i++)
	{
		if (g_Queue[i])
		{
			//cout << g_Queue[i]->arrivalTime << " " << g_Queue[i]->time << endl;

		}
	}

	fin.close();

	cout << "Finished loading file" << endl;
}

void SelectFile()
{
	cout << "\nEnter the name of the file (ie words.txt)" << endl;
	string input;
	cin >> input;

	ReadFile(input);
}

//**********************************************************
//*****            Interface and Menu            ***********
//**********************************************************
char Menu()
{
	char Cmd;
	cout << endl << "*********************************" << endl;
	cout << "*     Assignment 2              *" << endl;
	cout << "*     (r)ead file               *" << endl;
	cout << "*     (q)uit                    *" << endl;
	cout << "*********************************" << endl;
	cout << "Command: ";
	cin >> Cmd;
	cin.ignore();  // eat up '\n'
	Cmd = tolower(Cmd);
	return Cmd;
}

int main()
{
	bool Quit = false;

	do
	{
		char Cmd = Menu();
		switch (Cmd)
		{
		case 'r':
			SelectFile();
			break;
		case 'q':
			Quit = true;
			break;
		default:
			cerr << "Invalid command!\n";
		}
	} while (!Quit);

	return 0;
}
