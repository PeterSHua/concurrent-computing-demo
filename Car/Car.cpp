#include "Car.h"
#include <time.h>

int Car::main(void)
{
	if (CheckPumpStatus() == 0)
	{
		UsePump();
	}
	else
	{
		WaitInLine();
	}
	return 0;
}

Car::Car(string Name, int CreditCard, int Grade, double Amount, int PumpNumber)
{
	this->CustomerInfo.Name = Name;
	this->CustomerInfo.CreditCard = CreditCard;
	this->CustomerInfo.Grade = Grade;
	this->CustomerInfo.Amount = Amount;
	this->PumpNumber = PumpNumber;
	this->CustomerInfo.Bill = 0;

	int num_digits = 0; // stores how many digits there are in TOTALPUMPS (ex. num_digits = 2 if TOTALPUMPS = 10)

	for (int i = TOTALPUMPS; i > 0; num_digits++)
	{
		i = i / 10;
	}

	char *buffer = new char[num_digits]; // buffer to store the number part of the Mutex/DataPool name

	for (int i = 0; i < TOTALPUMPS; i++)
	{
		// Create the semaphores, mutexes, and pipelines, and their names
		PumpNames[i] = string("Pump") + itoa(i + 1, buffer, 10);
		PipeNames[i] = string("Pipe") + itoa(i + 1, buffer, 10);
		CustomerPipeNames[i] = string("CustomerPipe") + itoa(i + 1, buffer, 10);
		PumpPipe[i] = new CTypedPipe<Customer>(PipeNames[i], 1024);
		CustomerPipe[i] = new CPipe(CustomerPipeNames[i], 1024);
		PumpStatus[i] = new CMutex(PumpNames[i]);
	}
}

int Car::WaitInLine()
{
	while (1)
	{
		if (PumpStatus[this->PumpNumber]->Read())
		{
			UsePump();
			break;
		}
		Sleep(500);
	}
	return 0;
}

int Car::CheckPumpStatus()
{
	for (int i = 0; i < TOTALPUMPS; i++)
	{
		if (PumpStatus[i]->Read())
		{
			this->PumpNumber = i;
			return 0;
		}
	}
	return 1;
}

int Car::UsePump()
{
	PumpStatus[this->PumpNumber]->Wait();

	this->CustomerInfo.Time = time(NULL);

	PumpPipe[this->PumpNumber]->Write(&this->CustomerInfo);

	int Permission;

	CustomerPipe[this->PumpNumber]->Read(&Permission, sizeof(int));

	if (Permission == DISABLE)
	{
		// If the pump is disabled, try using another random pump
		int new_pumpnumber;
		do
		{
			new_pumpnumber = rand() % TOTALPUMPS;
		} while (new_pumpnumber == this->PumpNumber);

		this->PumpNumber = new_pumpnumber;

		WaitInLine();
		return 0;
	}

	Sleep(1000);

	PumpStatus[this->PumpNumber]->Signal();
	return 0;
}

Car::~Car(){};

int main(void)
{
	string Name[TOTALCUSTOMERS] = {
			"Michael Bluth",
			"Lindsay Funke",
			"Gob Bluth",
			"Maeby Funke",
			"Buster Bluth",
			"Tobias Funke",
			"George Bluth",
			"Lucille Bluth",
			"Ron Howard",
			"Anne Veal",
			"Annyong Bluth",
			"Steve Holt",
			"Kitty Sanchez"};

	Car *Customers[TOTALCUSTOMERS];

	for (int i = 0; i < TOTALCUSTOMERS; i++)
	{
		srand((unsigned int)time(NULL));

		int CreditCard = rand() % 888888888 + 111111111;
		int Grade = rand() % 4;
		double Amount = rand() % 71;
		int PumpNumber = rand() % 4;
		int CustomerNumber = rand() % 13;
		string CustomerName = Name[rand() % 13];

		Customers[i] = new Car(CustomerName.c_str(), CreditCard, Grade, Amount, PumpNumber);
		Customers[i]->Resume();
		Sleep((rand() % 10) * 1000 + 1000);
	}

	for (int i = 0; i < TOTALCUSTOMERS; i++)
	{
		Customers[i]->WaitForThread();
	}

	return 0;
}
