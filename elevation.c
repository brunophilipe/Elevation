#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "elevation.h"

typedef struct
{
	int direction;
	int curfloor;
	short load;

	short *calls_in;
	short *calls_out;
} cabinfo_t;

int shafts;
int floors;
int totalCabs;

int *cabsPerShaft;
cabinfo_t *cabsp;

int _cabindex(int shaft, int cab)
{
	int index = 0;

	for (int s = 0; s < shafts; ++s)
	{
		if (s != shaft)
		{
			index += cabsPerShaft[s];
		}
		else
		{
			index += cab;
		}
	}

	return index;
}

int _cabShouldMove(int cabindex, cabdir_t direction)
{
	cabinfo_t cab = cabsp[cabindex];
	if (direction == kElevationCabDirectionUp)
	{
		for (int f = cab.curfloor+1; f < floors; f++)
		{
			if (cab.calls_in[f] > 0)
			{
				// Has call from inside the cab on floor f
				return kElevationCabDirectionUp; //Cab should continue going upwards
			}
		}
	}
	else if (direction == kElevationCabDirectionDown)
	{
		for (int f = cab.curfloor; f >= 0; f--)
		{
			if (cab.calls_in[f] > 0)
			{
				// Has call from inside the cab on floor f
				return kElevationCabDirectionDown; //Cab should continue going downwards
			}
		}
	}

	return 0;
}

void elevation_init(int lshafts, int *lcabs, int lfloors)
{
	printf("Initializing elevation...\n");

	shafts = lshafts;
	floors = lfloors;

	totalCabs = 0;

	for (int s = 0; s < shafts; ++s)
	{
		totalCabs += lcabs[s];
	}

	cabsPerShaft = malloc(sizeof(int) * shafts);
	memcpy(cabsPerShaft, lcabs, sizeof(int) * shafts);

	cabsp = malloc(sizeof(cabinfo_t) * totalCabs);

	for (int s = 0; s < shafts; ++s)
	{
		for (int c = 0; c < lcabs[s]; ++c)
		{
			// Initialize each cab
			printf("Initializing cab %d\n", _cabindex(s, c));

			cabsp[_cabindex(s, c)].direction = kElevationCabDirectionUp;
			cabsp[_cabindex(s, c)].curfloor = 0;
			cabsp[_cabindex(s, c)].load = 0;

			cabsp[_cabindex(s, c)].calls_in = malloc(sizeof(short)*floors);
			cabsp[_cabindex(s, c)].calls_out = malloc(sizeof(short)*floors);

			for (int i = 0; i < floors; ++i)
			{
				cabsp[_cabindex(s, c)].calls_in[i] = 0;
				cabsp[_cabindex(s, c)].calls_out[i] = 0;
			}
		}
	}

	printf("Initialization done!\n");
}

void elevation_destroy()
{
	free(cabsPerShaft);
	free(cabsp);
}

void elevation_tick()
{
	cabinfo_t *cab;
	for (int i = 0; i < totalCabs; ++i)
	{
		cab = &cabsp[i];

		if (cab->calls_in[cab->curfloor] > 0)
		{
			cab->calls_in[cab->curfloor] = 0;
			printf("Cab %d stopped at floor %d\n", i, cab->curfloor);
		}
		else
		{
			switch(cab->direction)
			{
				case kElevationCabDirectionUp:
				if (_cabShouldMove(i, kElevationCabDirectionUp) > 0) cab->curfloor++;
				else if (_cabShouldMove(i, kElevationCabDirectionDown) < 0)
				{
					cab->direction = kElevationCabDirectionDown;
				}
				break;

				case kElevationCabDirectionDown:
				if (_cabShouldMove(i, kElevationCabDirectionDown) < 0) cab->curfloor--;
				else if (_cabShouldMove(i, kElevationCabDirectionUp) > 0)
				{
					cab->direction = kElevationCabDirectionUp;
				}
				break;
			}
		}
	}
}


void elevation_call_floor(int floor, int shaft)
{

}

void elevation_call_cab(int floor, int shaft, int cab)
{
	cabsp[_cabindex(shaft, cab)].calls_in[floor] = 1;
}

int elevation_cab_get_load(int shaft, int cab)
{
	return cabsp[_cabindex(shaft, cab)].load;
}

int elevation_cab_get_floor(int shaft, int cab)
{
	return cabsp[_cabindex(shaft, cab)].curfloor;
}

cabdir_t elevation_cab_get_direction(int shaft, int cab)
{
	return cabsp[_cabindex(shaft, cab)].direction;
}

int elevation_cab_set_load(int shaft, int cab, int people)
{
	if (people < 21)
	{
		cabsp[_cabindex(shaft, cab)].load = people;
		return 1;
	} else
		return 0;
}

int elevation_cab_increment_load(int shaft, int cab)
{
	if (cabsp[_cabindex(shaft, cab)].load < 21)
	{
		cabsp[_cabindex(shaft, cab)].load++;
		return 1;
	} else
		return 0;
}

int elevation_cab_decrement_load(int shaft, int cab)
{
	if (cabsp[_cabindex(shaft, cab)].load > 0)
	{
		cabsp[_cabindex(shaft, cab)].load--;
		return 1;
	} else
		return 0;
}
