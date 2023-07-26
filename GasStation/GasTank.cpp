#include "GasTank.h"

GasTank::GasTank(int Grade, double Price)
{
	this->Grade = Grade;

	num_digits = 0;

	for(int i = TOTALTANKS; i > 0; num_digits++)
	{
		i = i / 10;
	}

	buffer = new char[num_digits];

	for(int i = 0; i < TOTALTANKS; i++)
	{
		GasTankDataPoolNames[i] = string("GasTankDataPool") + itoa(i+1, buffer, 10);
		GasTankMutexNames[i] = string("GasTankMutex") + itoa(i+1, buffer, 10);
		GasTankDataPools[i] = new CDataPool(GasTankDataPoolNames[i], sizeof(struct GasTankData));
		GasTankMutexes[i] = new CMutex(GasTankMutexNames[i]);
	}

	DataPtr = (struct GasTankData*)(GasTankDataPools[this->Grade]->LinkDataPool());
	DataPtr->Price = 1;

}

GasTank::~GasTank() {};

int GasTank::SetGasTankCapacity(double GasTankCapacity)
{
	GasTankMutexes[this->Grade]->Wait();
	DataPtr->GasTankCapacity = GasTankCapacity;
	DataPtr->GasTankVolume = GasTankCapacity;
	GasTankMutexes[this->Grade]->Signal();
	return 0;
}

int GasTank::SetPrice(double Price)
{
	GasTankMutexes[this->Grade]->Wait();
	DataPtr->Price = Price;
	GasTankMutexes[this->Grade]->Signal();
	return 0;
}

int GasTank::DecrementTank(double Volume)
{	
	GasTankMutexes[this->Grade]->Wait();
	if(DataPtr->GasTankVolume > 0)
		DataPtr->GasTankVolume = DataPtr->GasTankVolume - Volume;
	else
		return 1;
	GasTankMutexes[this->Grade]->Signal();
	return 0;
}

int GasTank::IncrementTank(double Volume)
{
	if(GasTankMutexes[this->Grade]->Read())
	GasTankMutexes[this->Grade]->Wait();
	DataPtr->GasTankVolume = DataPtr->GasTankVolume + Volume;
	GasTankMutexes[this->Grade]->Signal();
	return 0;	
}

double GasTank::GetGasTankVolume(void)
{
	double GasTankVolume;
	GasTankMutexes[this->Grade]->Wait();
	GasTankVolume = DataPtr->GasTankVolume;
	GasTankMutexes[this->Grade]->Signal();
	return GasTankVolume;
}

double GasTank::GetPrice(void)
{
	double Price;
	GasTankMutexes[this->Grade]->Wait();
	Price = DataPtr->Price;
	GasTankMutexes[this->Grade]->Signal();
	return Price;
}