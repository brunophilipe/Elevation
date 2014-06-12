#include <stdio.h>
#include <unistd.h>

#include "elevation.h"

int main(int argc, char **argv)
{
	int cabspshaft[1] = {1};

	elevation_init(1,cabspshaft,6);

	printf("Initializing run loop...\n");

	for (int i = 0; i < 100; ++i)
	{
		printf("Cab location: %d %d", elevation_cab_get_floor(0, 0), elevation_cab_get_direction(0, 0));

		switch (i)
		{
			case 0:
			elevation_call_cab(5, 0, 0);
			printf(" | Received call on floor 5");
			break;

			case 10:
			elevation_call_cab(0, 0, 0);
			printf(" | Received call on floor 0");
			break;

			case 12:
			elevation_call_cab(2, 0, 0);
			printf(" | Received call on floor 2");
			break;
		}

		printf("\n");

		sleep(1);
		elevation_tick();
	}

	elevation_destroy();
}