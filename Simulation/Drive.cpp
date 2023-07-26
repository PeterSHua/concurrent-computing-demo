#include "..\..\rt.h"
#define TOTALCUSTOMERS 13

int main(void)
{
	HWND Window = GetConsoleWindow();
	MoveWindow(Window, 0, 0, 1000, 800, TRUE);

	CProcess GasPump("C:\\Documents and Settings\\peter\\My Documents\\Visual Studio 2008\\Projects\\GasStationWork\\debug\\Q2A.exe",
			NORMAL_PRIORITY_CLASS,
			PARENT_WINDOW,
			ACTIVE
			);

	CProcess Attendant("C:\\Documents and Settings\\peter\\My Documents\\Visual Studio 2008\\Projects\\GasStationWork\\debug\\GSC.exe",
			NORMAL_PRIORITY_CLASS,
			OWN_WINDOW,
			ACTIVE
			);

	CProcess Customers("C:\\Documents and Settings\\peter\\My Documents\\Visual Studio 2008\\Projects\\GasStationWork\\debug\\Car.exe",
			NORMAL_PRIORITY_CLASS,
			PARENT_WINDOW,
			ACTIVE
		);

	Attendant.WaitForProcess();
	GasPump.WaitForProcess();
	return 0;
}