#include <stdio.h>
#include <unistd.h>

#include "elevation.h"

int main(int argc, char **argv)
{
	int cabspshaft[1] = {1};//, 2};
	tuple_range shaftsRange[1] = {0, 5};//{{0, 5}, {1, 7}};

	elevation_init(1, cabspshaft, 8, shaftsRange);

	elevation_cab_set_rest_floor(0, 0, 1);

	printf("Initializing run loop...\n");

	for (int i = 0; i < 100; ++i)
	{
		printf("Cabs location: {%d %d}",//,{%d %d},{%d %d}",
			   elevation_cab_get_floor(0, 0), elevation_cab_get_direction(0, 0));//,
//			   elevation_cab_get_floor(1, 0), elevation_cab_get_direction(1, 0),
//			   elevation_cab_get_floor(1, 1), elevation_cab_get_direction(1, 1));

		switch (i)
		{
			case 0:
				elevation_call_cab(5, 0, 0);
				break;

			case 3:
				elevation_call_floor(2, 0, kElevationCabDirectionUp);
				break;

			case 4:
//				elevation_call_cab(3, 1, 1);
//				elevation_call_cab(0, 0, 0);
				break;

			case 10:

				break;

			case 12:
//				elevation_call_cab(1, 0, 0);
				break;
		}

		printf("\n");

		sleep(1);
		elevation_tick();
	}

	elevation_destroy();
}