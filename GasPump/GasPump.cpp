#include "GasPump.h"

GasPump::GasPump(int PumpNumber)
{
	Cursor = new CSemaphore("Cursor", 1);

	for(int i = 87; i < (87+TOTALTANKS); i++)
	{
		GradeNumbers[i-87] = i;
	}

	this->PumpNumber = PumpNumber;
	this->CustomerInfo.CreditCard = 0;
	this->CustomerInfo.Grade = -1;
	this->CustomerInfo.Amount = 0;
	this->CustomerInfo.Name = "Not in use";
	this->CustomerInfo.Time = time(NULL);
	
	this->rows = 0;

	for(int i = this->PumpNumber; i >= 0; this->rows++)
	{
		i-=4;
	}

	this->column = this->PumpNumber - (this->rows-1)*4;

	for(int i = 0; i < TOTALTANKS; i++)
	{
		GasTanks[i] = new GasTank(i, 0.5);
		GasTanks[i]->SetGasTankCapacity(TANKCAPACITY);
	}

	Cursor->Wait();
	MOVE_CURSOR(column*20, (this->rows-1)*(6+TOTALTANKS));
	printf("Name:");
	fflush(stdout);
	MOVE_CURSOR(column*20, (this->rows-1)*(6+TOTALTANKS)+1);
	printf("CC#:");
	fflush(stdout);
	MOVE_CURSOR(column*20, (this->rows-1)*(6+TOTALTANKS)+2);
	printf("Grade:");
	fflush(stdout);
	MOVE_CURSOR(column*20, (this->rows-1)*(6+TOTALTANKS)+3);
	printf("Gas:");
	fflush(stdout);		
	MOVE_CURSOR(column*20, (this->rows-1)*(6+TOTALTANKS)+4);
	printf("Bill:");
	fflush(stdout);	
	
	for(int i = 0; i < TOTALTANKS; i++)
	{
		MOVE_CURSOR(column*20, (this->rows-1)*(6+TOTALTANKS)+5+i);
		printf("Octane %i:$%.2f/L", GradeNumbers[i], GasTanks[i]->GetPrice());
		fflush(stdout);	
	}
	Cursor->Signal();
 
	int num_digits = 0; //num_digit will store how many digits there are in TOTALPUMPS (ex. num_digits = 2 if TOTALPUMPS = 10)

	for(int i = TOTALPUMPS; i > 0; num_digits++)
	{
		i = i / 10;
	}

	char* buffer = new char[num_digits]; //buffer to store the number part of the Mutex/DataPool name

	GasTankNames[this->PumpNumber] = string("GasTank") + itoa(this->PumpNumber+1, buffer, 10);
	PumpNames[this->PumpNumber] = string("Pump") + itoa(this->PumpNumber+1, buffer, 10);
	PipeNames[this->PumpNumber] = string("Pipe") + itoa(this->PumpNumber+1, buffer, 10);
	CustomerPipeNames[this->PumpNumber] = string("CustomerPipe") + itoa(this->PumpNumber+1, buffer, 10);
	PumpStatus[this->PumpNumber] = new CMutex(PumpNames[this->PumpNumber]);
	PumpPipe[this->PumpNumber] = new CTypedPipe<Customer>(PipeNames[this->PumpNumber], 1024);
	CustomerPipe[this->PumpNumber] = new CPipe(CustomerPipeNames[this->PumpNumber], 1024);
	PumpDataPoolNames[this->PumpNumber] = string("PumpDataPool") + itoa(this->PumpNumber+1, buffer, 10);
	PumpDataPools[this->PumpNumber] =  new CDataPool(PumpDataPoolNames[this->PumpNumber], sizeof(Customer));
	PSNames[this->PumpNumber] = string("PS") + itoa(this->PumpNumber+1, buffer, 10);
	PS[this->PumpNumber] = new CSemaphore(PSNames[this->PumpNumber], 0, 1);
	CSNames[this->PumpNumber] = string("CS") + itoa(this->PumpNumber+1, buffer, 10);
	CS[this->PumpNumber] = new CSemaphore(CSNames[this->PumpNumber], 1, 1);
}

GasPump::~GasPump() {};

int GasPump::GetCarInfo(void* ThreadArgs)
{
	while(1)
	{
		PumpPipe[this->PumpNumber]->Read(&this->CustomerInfo);

		PrintCarInfo();
		SendToGSC();

		if(this->CustomerInfo.Permission == YES)
		{
			FillCar();
		}

		else if(this->CustomerInfo.Permission == DISABLE)
		{			
			Cursor->Wait();
			MOVE_CURSOR(column*20, (this->rows-1)*(6+TOTALTANKS)+5+TOTALTANKS);
			printf("Pump disabled");
			fflush(stdout);
			Cursor->Signal();
			Sleep(1000);
			Cursor->Wait();
			MOVE_CURSOR(column*20, (this->rows-1)*(6+TOTALTANKS)+5+TOTALTANKS);
			printf("              ");
			fflush(stdout);
			Cursor->Signal();
		}

		else if(this->CustomerInfo.Permission == NO)
		{
			Cursor->Wait();
			MOVE_CURSOR(column*20, (this->rows-1)*(6+TOTALTANKS)+5+TOTALTANKS);
			printf("Out of gas");
			fflush(stdout);
			Cursor->Signal();
			Sleep(1000);
			Cursor->Wait();
			MOVE_CURSOR(column*20, (this->rows-1)*(6+TOTALTANKS)+5+TOTALTANKS);
			printf("              ");
			fflush(stdout);
			Cursor->Signal();
		}
		
		CustomerPipe[this->PumpNumber]->Write(&this->CustomerInfo.Permission, sizeof(this->CustomerInfo.Permission));

		ClearPump();
	}
	return 0;
}

int GasPump::PrintCarInfo(void)
{
	Cursor->Wait();
	MOVE_CURSOR(column*20+5, (this->rows-1)*(6+TOTALTANKS));
	printf("%s", this->CustomerInfo.Name.c_str());
	fflush(stdout);
	MOVE_CURSOR(column*20+4, (this->rows-1)*(6+TOTALTANKS)+1);
	printf("%i", this->CustomerInfo.CreditCard);
	fflush(stdout);
	MOVE_CURSOR(column*20+6, (this->rows-1)*(6+TOTALTANKS)+2);
	printf("%i", this->CustomerInfo.Grade);
	fflush(stdout);
	Cursor->Signal();

	return 0;
}

int GasPump::ClearPump(void)
{
	Cursor->Wait();
	MOVE_CURSOR(column*20+5, (this->rows-1)*6);
	printf("              ");
	fflush(stdout);
	MOVE_CURSOR(column*20+4, (this->rows-1)*6+1);
	printf("               ");
	fflush(stdout);
	MOVE_CURSOR(column*20+6, (this->rows-1)*6+2);
	printf("             ");
	fflush(stdout);
	MOVE_CURSOR(column*20+4, (this->rows-1)*6+3);
	printf("               ");
	fflush(stdout);
	MOVE_CURSOR(column*20+5, (this->rows-1)*6+4);
	printf("              ");
	fflush(stdout);
	Cursor->Signal();
	return 0;
}

int GasPump::SendToGSC(void)
{
	struct Customer *PumpDataPool= (struct Customer*)(PumpDataPools[this->PumpNumber]->LinkDataPool());

	CS[this->PumpNumber]->Wait();
	PumpDataPool->CreditCard = this->CustomerInfo.CreditCard;
	PumpDataPool->Grade = this->CustomerInfo.Grade;
	PumpDataPool->Amount = this->CustomerInfo.Amount;
	PumpDataPool->Name = this->CustomerInfo.Name;
	PumpDataPool->Time = this->CustomerInfo.Time;
	PS[this->PumpNumber]->Signal();

	PS[this->PumpNumber]->Wait();
	this->CustomerInfo.Permission = PumpDataPool->Permission;
	CS[this->PumpNumber]->Signal();

	return 0;
}

int GasPump::FillCar(void)
{		
	struct Customer *PumpDataPool= (struct Customer*)(PumpDataPools[this->PumpNumber]->LinkDataPool());

	for(double i = 0; (this->CustomerInfo.Amount-i) > 0; i = i + RATE)
	{
		Sleep(1000);
		if(GasTanks[this->CustomerInfo.Grade]->DecrementTank(RATE))
			break;
		Cursor->Wait();
		MOVE_CURSOR(column*20+4, (this->rows-1)*(6+TOTALTANKS)+3);
		printf("%f", i+RATE);
		fflush(stdout);
		MOVE_CURSOR(column*20+5, (this->rows-1)*(6+TOTALTANKS)+4);
		printf("%f", (i+RATE)*GasTanks[this->CustomerInfo.Grade]->GetPrice());
		fflush(stdout);
		Cursor->Signal();

		CS[this->PumpNumber]->Wait();
		PumpDataPool->Amount = (double)i+RATE;
		PS[this->PumpNumber]->Signal();
	}
	return 0;
}

int GasPump::main(void)
{
	r1 = new CRendezvous("MyRendezvous", TOTALPUMPS+1) ;
	r1->Wait();

	ClassThread<GasPump> GetCarInfoThread(this, &GasPump::GetCarInfo, ACTIVE, NULL);

	while(1)
	{
		for(int i = 0; i < TOTALTANKS; i++)
		{
			Cursor->Wait();
			MOVE_CURSOR(column*20, (this->rows-1)*(6+TOTALTANKS)+5+i);
			printf("Octane %i:$%.2f/L", GradeNumbers[i], GasTanks[i]->GetPrice());
			fflush(stdout);
			Cursor->Signal();
		}
		Sleep(200);
	}

	GetCarInfoThread.WaitForThread();

	return 0;
}

int main(void)
{
	GasPump* GasPumpList[TOTALPUMPS];

	for(int i = 0; i < TOTALPUMPS; i++)
	{
		GasPumpList[i] = new GasPump(i);
		GasPumpList[i]->Resume();
	}

	for(int i = 0; i < TOTALPUMPS; i++)
	{
		GasPumpList[i]->WaitForThread();
	}

	return 0;
}