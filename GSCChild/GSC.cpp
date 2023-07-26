#include <stdio.h>
#include <string>
#include <math.h>
#include "..\..\rt.h"
#include <list>
#include <time.h>
#include "..\GasPump.h"
#define TANKCAPACITY 500
#define YES 0
#define NO 1
#define DIABLE 2
#define EMPTY 0
#define NOTEMPTY 1
#define TOTALPUMPS 5
#define TOTALTANKS 4

CSemaphore* CursorGSC;
CDataPool* PumpDataPools[TOTALPUMPS];
CSemaphore* PS[TOTALPUMPS];
CSemaphore* CS[TOTALPUMPS];
CSemaphore* LinkedList;
CRendezvous* r1;
GasTank* GasTanks[TOTALTANKS];

int Permissions[TOTALPUMPS];
int TankStatus[TOTALTANKS];
string PumpDataPoolNames[TOTALPUMPS];
string PSNames[TOTALPUMPS];
string CSNames[TOTALPUMPS];
string GasTankNames[TOTALTANKS];

struct tm  *ts;
char buf[80];
int rows; //Number of rows used for printing gas pumps
int console_history; //Flag that prevents printing on the GSC while displaying history
int num_digits_pump; //stores how many digits there are in TOTALPUMPS (ex. num_digits = 2 if TOTALPUMPS = 10)
int num_digits_tank; //stores how many digits there are in TOTALTANKS

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
	
list<Customer> CustomerData;

int Console(void)
{
	for(int i = 0; i < TOTALTANKS; i++)
	{
		CursorGSC->Wait();
		MOVE_CURSOR(0, (rows)*6+3*i);
		printf("Tank %i:", i);
		fflush(stdout);
		MOVE_CURSOR(0, (rows)*6+TOTALTANKS*3+5);
		CursorGSC->Signal();
	}

	int row = 0;
	int end = TOTALPUMPS; //If not printing on the last row, 4 pumps to print
	int limit;

	for(int i = TOTALPUMPS; i >= 0; row++)
	{
		i-=4;
	}

	for(int y = 0; y < row; y++)
	{
		if(end > 4)
			limit = 4;
		else
			limit = end;

		for(int x = 0; x < limit; x++)
		{
			CursorGSC->Wait();
			MOVE_CURSOR(x*20, y*6);
			printf("Name:");
			fflush(stdout);
			MOVE_CURSOR(x*20, y*6+1);
			printf("CC#:");
			fflush(stdout);
			MOVE_CURSOR(x*20, y*6+2);
			printf("Grade:");
			fflush(stdout);
			MOVE_CURSOR(x*20, y*6+3);
			printf("Gas:");
			fflush(stdout);		
			MOVE_CURSOR(x*20, y*6+4);
			printf("Bill:");
			fflush(stdout);
			CursorGSC->Signal();
		}
		end-=4;
	}

	CursorGSC->Wait();
	MOVE_CURSOR(0, rows*6+TOTALTANKS*3);
	printf("p, the pump number, and enter to give permission (ex. f1)");
	MOVE_CURSOR(0, rows*6+TOTALTANKS*3+1);
	printf("t, the tank number, and enter to refill (ex. t1)");
	MOVE_CURSOR(0, rows*6+TOTALTANKS*3+2);
	printf("d, the pump number, and enter to disable (ex. d1)");
	MOVE_CURSOR(0, rows*6+TOTALTANKS*3+3);
	printf("s, and enter to view history");
	MOVE_CURSOR(0, rows*6+TOTALTANKS*3+4);
	printf("c, the grade number, and enter to change the price");
	MOVE_CURSOR(0, (rows)*6+TOTALTANKS*3+5);
	CursorGSC->Signal();

	return 0;
}


UINT __stdcall GetFromPump(void* ThreadArgs)
{		
	int PumpNumber = *(int*)ThreadArgs;	

	struct Customer *PumpDataPool = (struct Customer*)(PumpDataPools[PumpNumber]->LinkDataPool());

	int pumprow = 0;

	for(int i = PumpNumber; i >= 0; pumprow++)
	{
		i-=4;
	}

	int column = PumpNumber - (pumprow-1)*4;

	while(1)
	{
		PS[PumpNumber]->Wait();
		Customer temp;
		temp.CreditCard = PumpDataPool->CreditCard;
		temp.Grade = PumpDataPool->Grade;
		temp.Amount = PumpDataPool->Amount;
		temp.Name = PumpDataPool->Name;
		temp.Time = PumpDataPool->Time;
		CS[PumpNumber]->Signal();
	
		if(console_history == NO)
		{
			CursorGSC->Wait();
			MOVE_CURSOR(column*20+5, (pumprow-1)*6);
			printf("%s", temp.Name.c_str());
			fflush(stdout);
			MOVE_CURSOR(column*20+4, (pumprow-1)*6+1);
			printf("%i", temp.CreditCard);
			fflush(stdout);
			MOVE_CURSOR(column*20+6, (pumprow-1)*6+2);
			printf("%i", temp.Grade);
			fflush(stdout);
			MOVE_CURSOR(0, (rows)*6+TOTALTANKS*3+5);
			CursorGSC->Signal();
		}

		int Permission;
		int next; //Go to the next iteration and wait for another customer if don't have permission

		while(1)
		{
			if(console_history == NO)
			{
				//Print out customer info continously since show history will clear it
				CursorGSC->Wait();
				MOVE_CURSOR(column*20+5, (pumprow-1)*6);
				printf("%s", temp.Name.c_str());
				fflush(stdout);
				MOVE_CURSOR(column*20+4, (pumprow-1)*6+1);
				printf("%i", temp.CreditCard);
				fflush(stdout);
				MOVE_CURSOR(column*20+6, (pumprow-1)*6+2);
				printf("%i", temp.Grade);
				fflush(stdout);
				MOVE_CURSOR(0, (rows)*6+TOTALTANKS*3+5);
				CursorGSC->Signal();
			}

			if(Permissions[PumpNumber] == DISABLE)
			{
				Permission = DISABLE;
				next = YES;
				break;
			}

			if(TankStatus[temp.Grade] == EMPTY)
			{
				if(Permissions[PumpNumber] != DISABLE)
					Permission = NO;
				next = YES;
				break;
			}

			if(Permissions[PumpNumber] == YES)
			{
				Permission = YES;
				next = NO;
				break;
			}

			Sleep(200);
		}

		CS[PumpNumber]->Wait();	
		PumpDataPool->Permission = Permission;
		PS[PumpNumber]->Signal();

		if(next == YES)
		{
			CursorGSC->Wait();
			MOVE_CURSOR(column*20+5, (pumprow-1)*6);
			printf("              ", temp.Name.c_str());
			fflush(stdout);
			MOVE_CURSOR(column*20+4, (pumprow-1)*6+1);
			printf("               ", temp.CreditCard);
			fflush(stdout);
			MOVE_CURSOR(0, (rows)*6+TOTALTANKS*3+5);
			CursorGSC->Signal();
			continue;
		}

		//Set permission back to no for the next customer
		Permissions[PumpNumber] = NO;

		double consumed = 0;

		do
		{
			PS[PumpNumber]->Wait();
			consumed = PumpDataPool->Amount;
			CS[PumpNumber]->Signal();
			if(console_history == NO)
			{
				//Print out customer info continously since show history will clear it
				CursorGSC->Wait();
				MOVE_CURSOR(column*20+5, (pumprow-1)*6);
				printf("%s", temp.Name.c_str());
				fflush(stdout);
				MOVE_CURSOR(column*20+4, (pumprow-1)*6+1);
				printf("%i", temp.CreditCard);
				fflush(stdout);
				MOVE_CURSOR(column*20+6, (pumprow-1)*6+2);
				printf("%i", temp.Grade);
				fflush(stdout);
				MOVE_CURSOR(column*20+5, (pumprow-1)*6+3);
				printf("%.2f", consumed);
				fflush(stdout);
				MOVE_CURSOR(column*20+6, (pumprow-1)*6+4);
				printf("%.2f", consumed*GasTanks[temp.Grade]->GetPrice());
				fflush(stdout);
				MOVE_CURSOR(0, (rows)*6+TOTALTANKS*3+5);
				CursorGSC->Signal();
			}
		} while(consumed < temp.Amount);

		temp.Amount = consumed;
		temp.Bill = temp.Amount*GasTanks[temp.Grade]->GetPrice();

		LinkedList->Wait();
		CustomerData.push_back(temp);
		LinkedList->Signal();

		if(console_history == NO)
		{
			CursorGSC->Wait();
			MOVE_CURSOR(column*20+5, (pumprow-1)*6);
			printf("              ", temp.Name.c_str());
			fflush(stdout);
			MOVE_CURSOR(column*20+4, (pumprow-1)*6+1);
			printf("               ", temp.CreditCard);
			fflush(stdout);
			MOVE_CURSOR(column*20+6, (pumprow-1)*6+2);
			printf("             ", temp.Grade);
			fflush(stdout);
			MOVE_CURSOR(column*20+5, (pumprow-1)*6+3);
			printf("               ", 0);
			fflush(stdout);
			MOVE_CURSOR(column*20+6, (pumprow-1)*6+4);
			printf("              ", 0);
			fflush(stdout);
			MOVE_CURSOR(0, (rows)*6+TOTALTANKS*3+5);
			CursorGSC->Signal();
		}
	}

	return 0;
}

UINT __stdcall TestGasTankVolume(void* ThreadArgs)
{	
	int counter = 0;
	
	while(1)
	{
		for(int i = 0; i < TOTALTANKS; i++)
		{
			if(console_history == NO)
			{
				CursorGSC->Wait();
				MOVE_CURSOR(0, (rows)*6+3*i+1);
				printf("%f", GasTanks[i]->GetGasTankVolume());
				fflush(stdout);
				MOVE_CURSOR(0, (rows)*6+TOTALTANKS*3+5);
				CursorGSC->Signal();
			}

			if(GasTanks[i]->GetGasTankVolume() <= WARNINGREICH)
			{
				if(console_history == NO)
				{
					CursorGSC->Wait();
					MOVE_CURSOR(0, (rows)*6+3*i+2);
					TEXT_COLOUR(12, 0);
					printf("WARNING REICH");
					TEXT_COLOUR(7, 0);
					fflush(stdout);
					MOVE_CURSOR(0, (rows)*6+TOTALTANKS*3+5);
					CursorGSC->Signal();
				}
	
				TankStatus[i] = EMPTY;

				//Create the illusion of "flashing" by erasing the warning text every second loop
				if(counter % 2 == 0)
				{
					if(console_history == NO)
					{
						CursorGSC->Wait();
						MOVE_CURSOR(0, (rows)*6+3*i+2);
						printf("             ");
						fflush(stdout);
						MOVE_CURSOR(0, (rows)*6+TOTALTANKS*3+5);
						CursorGSC->Signal();
					}
				}
			}
			else
			{
				if(console_history == NO)
				{
					TankStatus[i] = NOTEMPTY;
					CursorGSC->Wait();
					MOVE_CURSOR(0, (rows)*6+3*i+2);
					printf("             ");
					fflush(stdout);
					MOVE_CURSOR(0, (rows)*6+TOTALTANKS*3+5);
					CursorGSC->Signal();
				}
			}
		}
		counter++;
		Sleep(200);
	}
	return 0;
}

UINT __stdcall Commands(void* ThreadArgs)
{	
	char c;

	while(1)
	{
		c = getchar();

		if(c == 'd')
		{
			int* c_number = new int[num_digits_pump];

			for(int i = num_digits_pump; i > 0; i--)
			{
				c = getchar();
				c_number[num_digits_pump-i] = atoi(&c);
			}
			
			int i_number = 0;

			for(int i = 0; i < num_digits_pump; i++)
			{
				i_number += c_number[i]*(int)pow((double)10, i);
			}

			CursorGSC->Wait();
			MOVE_CURSOR(0, rows*6+TOTALTANKS*3+6);
			printf(" ");
			fflush(stdout);
			MOVE_CURSOR(0, rows*6+TOTALTANKS*3+5);
			printf("Disable pump %i", i_number);
			fflush(stdout);
			MOVE_CURSOR(0, (rows)*6+TOTALTANKS*3+5);
			Permissions[i_number] = DISABLE;
			CursorGSC->Signal();
		}

		if(c == 'p')
		{
			int* c_number = new int[num_digits_pump];

			for(int i = num_digits_pump; i > 0; i--)
			{
				c = getchar();
				c_number[num_digits_pump-i] = atoi(&c);
			}
			
			int i_number = 0;

			for(int i = 0; i < num_digits_pump; i++)
			{
				i_number += c_number[i]*(int)pow((double)10, i);
			}

			CursorGSC->Wait();
			MOVE_CURSOR(0, rows*6+TOTALTANKS*3+6);
			printf(" ");
			fflush(stdout);
			MOVE_CURSOR(0, rows*6+TOTALTANKS*3+5);
			printf("Fill pump %i", i_number);
			fflush(stdout);
			MOVE_CURSOR(0, (rows)*6+TOTALTANKS*3+5);
			Permissions[i_number] = YES;
			CursorGSC->Signal();
		}

		if(c == 't')
		{
			int* c_number = new int[num_digits_tank];

			for(int i = num_digits_tank; i > 0; i--)
			{
				c = getchar();
				c_number[num_digits_tank-i] = atoi(&c);
			}
			
			int i_number = 0;

			for(int i = 0; i < num_digits_tank; i++)
			{
				i_number += c_number[i]*(int)pow((double)10, i);
			}

			CursorGSC->Wait();
			MOVE_CURSOR(0, rows*6+TOTALTANKS*3+6);
			printf(" ");
			fflush(stdout);
			MOVE_CURSOR(0, rows*6+TOTALTANKS*3+5);
			printf("Fill tank %i", num_digits_tank);
			fflush(stdout);
			MOVE_CURSOR(0, (rows)*6+TOTALTANKS*3+5);
			CursorGSC->Signal();

			while(GasTanks[atoi(&c)]->GetGasTankVolume() < TANKCAPACITY)
			{
				GasTanks[atoi(&c)]->IncrementTank(0.5);
				Sleep(500);
			}
			CursorGSC->Wait();
			MOVE_CURSOR(0, rows*6+TOTALTANKS*3+5);
			printf("             ");
			fflush(stdout);
			MOVE_CURSOR(0, (rows)*6+TOTALTANKS*3+5);
			CursorGSC->Signal();
		}

		if(c == 's')
		{
			console_history = YES;

			int j = 0;
			list<Customer>::iterator i;

			for(int i = 0; i < (rows)*6+TOTALTANKS*3+5+1; i++)
			{
				CursorGSC->Wait();
				MOVE_CURSOR(0, i);
				printf("                                                                             ");
				fflush(stdout);
				CursorGSC->Signal();
			}

			for(i = CustomerData.begin(); i != CustomerData.end(); i++) 
			{
				CursorGSC->Wait();
				MOVE_CURSOR(0, j);
				printf("Name:%s", (*i).Name.c_str());
				fflush(stdout);
				j++;
				MOVE_CURSOR(0, j);
				printf("CC#:%i", (*i).CreditCard);
				fflush(stdout);
				j++;
				MOVE_CURSOR(0, j);
				printf("Grade:%i", (*i).Grade);
				fflush(stdout);
				j++;
				MOVE_CURSOR(0, j);
				printf("Gas:%.2fL", (*i).Amount);
				j++;
				MOVE_CURSOR(0, j);
				printf("Bill:$%.2f", (*i).Bill);
				j++;		
				ts = localtime(&(*i).Time);
				strftime(buf, sizeof(buf), "%a %Y-%m-%d %H:%M:%S %Z", ts);
				MOVE_CURSOR(0,j);
				printf("%s\n", buf);
				fflush(stdout);
				CursorGSC->Signal();
				j+=3;
			}
			
			CursorGSC->Wait();
			MOVE_CURSOR(0, j);
			printf("Press q to return\n");
			fflush(stdout);
			CursorGSC->Signal();

			while(1)
			{
				if(getchar() == 'q')
				{
					j+=2;
					for(int i = 0; i < j; i++)
					{
						CursorGSC->Wait();
						MOVE_CURSOR(0, i);
						printf("                                                                             ");
						fflush(stdout);
						CursorGSC->Signal();
					}
					CursorGSC->Wait();
					MOVE_CURSOR(0, (rows)*6+TOTALTANKS*3+5);
					CursorGSC->Signal();
					break;
				}
			}

			console_history = NO;
			Console();
		}

		if(c == 'c')
		{
			char tanknumber = getchar();
			CursorGSC->Wait();
			MOVE_CURSOR(0, rows*6+TOTALTANKS*3+6);
			printf(" ");
			fflush(stdout);
			MOVE_CURSOR(0, (rows)*6+TOTALTANKS*3+5);
			CursorGSC->Signal();

			c = getchar();

			CursorGSC->Wait();
			MOVE_CURSOR(0, rows*6+TOTALTANKS*3+5);
			printf("Change grade %i price to %.2f", atoi(&tanknumber), atof(&c));
			fflush(stdout);
			MOVE_CURSOR(0, (rows)*6+TOTALTANKS*3+5);
			CursorGSC->Signal();
			GasTanks[atoi(&tanknumber)]->SetPrice(atof(&c));
			
			CursorGSC->Wait();
			Sleep(1000);
			MOVE_CURSOR(0, rows*6+TOTALTANKS*3+5);
			printf("                            ");
			fflush(stdout);
			MOVE_CURSOR(0, (rows)*6+TOTALTANKS*3+5);
			CursorGSC->Signal();
		}

		Sleep(200);
	}
	return 0;
}

int main(void)
{
	r1 = new CRendezvous("MyRendezvous", TOTALPUMPS+1);
	r1->Wait();

	HWND Window = GetConsoleWindow();
	MoveWindow(Window, 0, 0, 1000, 800, TRUE);

	console_history = NO;

	char* buffer; //buffer to store the number part of the Mutex/DataPool name
	char* buffer2; //buffer to store the number part of the GasTank name

	CursorGSC = new CSemaphore("CursorGSC", 1);
	LinkedList = new CSemaphore("LinkedList", 1);

	rows = 0;

	for(int i = TOTALPUMPS; i >= 0; rows++)
	{
		i-=4;
	}

	num_digits_tank= 0;

	for(int i = TOTALTANKS; i > 0; num_digits_tank++)
	{
		i = i / 10;
	}

	buffer2 = new char[num_digits_tank];

	for(int i = 0; i < TOTALTANKS; i++)
	{
		TankStatus[i] = NOTEMPTY;
	}

	for(int i = 0; i < TOTALTANKS; i++)
	{
		GasTankNames[i] = string("GasTank") + itoa(i+1, buffer2, 10);
		GasTanks[i] = new GasTank(i, 0.5+i);
	}

	num_digits_pump = 0;

	for(int i = TOTALPUMPS; i > 0; num_digits_pump++)
	{
		i = i / 10;
	}

	buffer = new char[num_digits_pump];

	for(int i = 0; i < TOTALPUMPS; i++)
	{
		Permissions[i] = NO;
		PumpDataPoolNames[i] = string("PumpDataPool") + itoa(i+1, buffer, 10);
		PumpDataPools[i] =  new CDataPool(PumpDataPoolNames[i], sizeof(Customer));
		PSNames[i] = string("PS") + itoa(i+1, buffer, 10);
		PS[i] = new CSemaphore(PSNames[i], 0, 1);
		CSNames[i] = string("CS") + itoa(i+1, buffer, 10);
		CS[i] = new CSemaphore(CSNames[i], 1, 1);
	}
	
	Console();

	CThread CommandsThread(Commands, ACTIVE, NULL);
	CommandsThread.Resume();

	CThread TestGasTankVolumeThread(TestGasTankVolume, ACTIVE, NULL);
	TestGasTankVolumeThread.Resume();

	CThread* GetFromPumpThreads[TOTALPUMPS];

	int PumpNumbers[TOTALPUMPS];

	for(int i = 0; i < TOTALPUMPS; i++)
	{
		PumpNumbers[i] = i;
		GetFromPumpThreads[i] = new CThread(GetFromPump, ACTIVE, &PumpNumbers[i]);
		GetFromPumpThreads[i]->Resume();
	}

	for(int i = 0; i < TOTALPUMPS; i++)
	{
		GetFromPumpThreads[i]->WaitForThread();
	}
	
	TestGasTankVolumeThread.WaitForThread();
	CommandsThread.WaitForThread();

	return 0;
}