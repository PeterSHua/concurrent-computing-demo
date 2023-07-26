#include <stdio.h>
#include <string>
#include "..\rt.h"
#include <time.h>
#include <list>
#include "GasTank.h"
#define YES 0
#define NO 1
#define DISABLE 2
#define RATE 0.5
#define TOTALPUMPS 5
#define TOTALTANKS 4

using namespace std;

class GasPump : public ActiveClass
{
public:
	GasPump(int PumpNumber);
	~GasPump();

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

	Customer CustomerInfo;

	int main(void);
	int GetCarInfo(void* ThreadArgs);
	int PrintCarInfo(void);
	int SendToGSC(void);
	int FillCar(void);
	int ClearPump(void);

	int PumpNumber;
	int rows; //Number of rows used for printing gas pumps
	int column; //Column the pump will be printed at

	CSemaphore* Cursor;
	CTypedPipe<Customer>* PumpPipe[TOTALPUMPS];
	CPipe* CustomerPipe[TOTALPUMPS]; //Pipe between GSC and GasPump process to disable a gas pump object
	CMutex* PumpStatus[TOTALPUMPS];
	CDataPool* PumpDataPools[TOTALPUMPS];
	CSemaphore* PS[TOTALPUMPS];
	CSemaphore* CS[TOTALPUMPS];
	GasTank* GasTanks[TOTALTANKS];
	CRendezvous* r1;

	string GasTankNames[TOTALTANKS];
	string PumpNames[TOTALPUMPS];
	string PipeNames[TOTALPUMPS];
	string PumpDataPoolNames[TOTALPUMPS];
	string PSNames[TOTALPUMPS];
	string CSNames[TOTALPUMPS];
	string CustomerPipeNames[TOTALPUMPS];

	int GradeNumbers[TOTALTANKS];
};