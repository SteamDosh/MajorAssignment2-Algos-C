#include <iostream>
#include <iomanip>
#include <string>
#include <fstream>
#include <cstring>
#include <stdio.h>
using namespace std;

const int g_totalServers = 20;
const int g_totalEvents = 1000;
const int g_totalCustomers = 1000;

struct Server
{
	double priority;
	int custServerId;
	double finish_time;
	double totalservicetime;
	bool idle;

	Server()
	{
		idle = true;
		priority = 0;
		custServerId = 0;
		finish_time = 0;
		totalservicetime = 0;
	}
};

struct Event
{
	int eventType;
	double eventTime;
	double serviceTime;
};

double g_Time = 0;
Server g_Servers[g_totalServers];
int g_ServerNum = 0;
Event g_Events[g_totalEvents];
int g_EventCount = 0;

struct node
{
	double arrivalTime;
	double serviceTime;
};

node g_Cust[g_totalCustomers];
int g_frontCust = 0;
int g_rearCust = 0;
int g_Counter = 0;
int g_TCounter = 0;
double g_TotalTimeQue = 0;
int g_MaxQueLen = 0;

//-------------------------------
int findServer();
bool QueEmpty();
bool EventQueEmpty();
void removeEvent(int& eventType, double& eventTime, double& serviceTime);
void enqueue(double arrivalTime, double serviceTime);
void dequeue(double& arrivalTime, double& serviceTime);
void insertEvent(int eventType, double eventTime, double serviceTime);

int main()
{
	ifstream input;
	double startTime;
	char filename[20];
	cout << "Enter File name:" << endl;
	gets_s(filename);

	input.open(filename);
	if (!input)
	{
		cout << "Error : Failed to open the file!";
		return 0;
	}

	input >> g_ServerNum;
	for (int i = 0; i < g_ServerNum; i++)
	{
		input >> g_Servers[i].priority;
	}

	double eventTime, arrivalTime, serviceTime;
	input >> arrivalTime >> serviceTime;

	insertEvent(-1, arrivalTime, serviceTime);

	double prevTime = -1, initTime = arrivalTime;
	double AvgQueLen = 0, AvgTimeQue = 0;

	while (!EventQueEmpty())
	{
		int eventType;
		removeEvent(eventType, eventTime, serviceTime);
		g_Time = eventTime;

		if (prevTime >= 0)// Do Stats
		{
			double event_duration = g_Time - prevTime;
			AvgQueLen += g_Counter * event_duration;
		}
		prevTime = g_Time;
		if (eventType == -1) // Customer Arriving
		{
			enqueue(g_Time, serviceTime);
			if (input >> arrivalTime >> serviceTime)
			{
				insertEvent(-1, arrivalTime, serviceTime);
			}
		}
		else // Customer gone
		{
			g_Servers[eventType].idle = true;
			g_Servers[eventType].cust_served++;
		}

		if (!QueEmpty()) // Customer is waiting
		{
			int serv = findServer();
			if (serv != -1)
			{
				double serviceTime;
				dequeue(arrivalTime, serviceTime);
				g_TotalTimeQue += g_Time - arrivalTime;
				serviceTime = serviceTime * g_Servers[serv].priority;
				g_Servers[serv].totalservicetime = g_Servers[serv].totalservicetime + serviceTime;
				g_Servers[serv].idle = false;
				double finish_time = g_Time + serviceTime;
				insertEvent(serv, finish_time, 0);
				g_TCounter++;
			}
		}
	}

	input.close();

	startTime = g_Time - initTime;
	AvgQueLen /= startTime;

	cout << setprecision(7);
	cout << "Number of customers served: " << g_TCounter << endl;
	cout << "Time last customer completed service: " << g_Time << endl;
	cout << "Greatest length reached by the queue: " << g_MaxQueLen << endl;
	cout << "Average length of the queue: " << AvgQueLen << endl;
	cout << "Average customer waiting time: " << g_TotalTimeQue / g_TCounter << endl << endl;
	cout << "Server   Priority   CustsServed   IdleTime" << endl;

	for (int i = 0; i<g_ServerNum; i++)
	{
		cout << setw(3) << i
			<< setw(13) << g_Servers[i].priority
			<< setw(9) << g_Servers[i].cust_served
			<< setw(15) << startTime - g_Servers[i].totalservicetime
			<< endl;
	}
	return 0;
}

void insertEvent(int eventType, double eventTime, double serviceTime)
{
	int i = g_EventCount;
	while (i>0 && eventTime < g_Events[i - 1].eventTime)
	{
		g_Events[i] = g_Events[i - 1];
		i--;
	}
	g_Events[i].eventType = eventType;
	g_Events[i].eventTime = eventTime;
	g_Events[i].serviceTime = serviceTime;
	g_EventCount++;
}

void removeEvent(int& eventType, double& eventTime, double& serviceTime)
{
	eventType = g_Events[0].eventType;
	eventTime = g_Events[0].eventTime;
	serviceTime = g_Events[0].serviceTime;
	g_EventCount--;
	for (int i = 0; i<g_EventCount; i++)
	{
		g_Events[i] = g_Events[i + 1];
	}
}

bool EventQueEmpty()
{
	return g_EventCount == 0;
}

bool QueEmpty()
{
	return g_Counter == 0;
}

int findServer()
{
	int serv = -1;
	double maxpriority = 9999999;
	for (int i = 0; i<g_ServerNum; i++)
	{
		if (g_Servers[i].idle && g_Servers[i].priority < maxpriority) {
			maxpriority = g_Servers[i].priority;
			serv = i;
		}
	}
	return serv;
}

void enqueue(double arrivalTime, double serviceTime)
{
	g_Cust[g_rearCust].arrivalTime = arrivalTime;
	g_Cust[g_rearCust].serviceTime = serviceTime;

	g_rearCust++;
	if (g_rearCust == g_totalCustomers) g_rearCust = 0;
	g_Counter++;
	if (g_Counter>g_MaxQueLen) g_MaxQueLen = g_Counter;
}

void dequeue(double &arrivalTime, double &serviceTime)
{
	serviceTime = g_Cust[g_frontCust].serviceTime;
	arrivalTime = g_Cust[g_frontCust].arrivalTime;

	g_frontCust++;
	if (g_frontCust == g_totalCustomers) g_frontCust = 0;
	g_Counter--;
}