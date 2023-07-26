#include <stdio.h>
#include <string>
#include <time.h>
#include "..\..\rt.h"
#define TOTALCUSTOMERS 13
#define YES 0
#define NO 1
#define DISABLE 2

class Car : public ActiveClass 
{
public:
	Car(string Name, int CreditCard, int Grade, double Amount, int PumpNumber);
	~Car();
	int CheckPumpStatus();
	int WaitInLine();
	int UsePump();
	
private:
	struct Customer
	{
		int CreditCard;
		int Grade;
		double Amount;
		string Name;
		time_t Time;
		int Permission;
		double Bill;
	};

	struct Customer CustomerInfo;

	int PumpNumber;

	string PumpNames[TOTALPUMPS];
	string PipeNames[TOTALPUMPS];
	string CustomerPipeNames[TOTALPUMPS];

	CMutex* PumpStatus[TOTALPUMPS];
	CTypedPipe<Customer>* PumpPipe[TOTALPUMPS];
	CPipe* CustomerPipe[TOTALPUMPS]; //Pipe between GSC and GasPump process to disable a gas pump object

	int main(void);
} ;