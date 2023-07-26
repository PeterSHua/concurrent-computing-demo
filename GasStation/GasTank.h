#include <stdio.h>
#include <string>
#include "..\rt.h"
#define WARNINGREICH 200
#define TANKCAPACITY 500

class GasTank
{
public:
	GasTank(int Grade, double Price);
	~GasTank();
	int SetGasTankCapacity(double GasTankCapacity);
	int DecrementTank(double Volume);
	int IncrementTank(double Volume);
	double GetGasTankVolume(void);
	int SetPrice(double Price);
	double GetPrice(void);
private:
	struct GasTankData
	{
		double GasTankCapacity;
		double GasTankVolume;
		double Price;
	};

	int Grade;
	char* buffer;
	int num_digits;
	CDataPool* GasTankDataPools[TOTALTANKS];
	CMutex* GasTankMutexes[TOTALTANKS];
	GasTankData* DataPtr;
	string GasTankDataPoolNames[TOTALTANKS];
	string GasTankMutexNames[TOTALTANKS];
};