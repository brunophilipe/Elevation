#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "elevation.h"

typedef struct
{
	int direction;
	int curFloor;
	int restFloor;
	short load;
	short stopped;

	short *calls_in;
	short *calls_out;
} cabinfo_t;

typedef enum {
	kElevationCallGoUp = 1<<1,
	kElevationCallGoDown = 1<<2
} calldir_t;

int count_shafts;
int count_floors;
int count_totalCabs;

int *array_cabsCount;
cabinfo_t *array_cabs;
tuple_range *array_shaftsRange;

#pragma mark - Private Functions

int _cabindex(int shaft, int cab)
{
	int index = 0;

	for (int s = 0; s < count_shafts; s++)
	{
		if (s != shaft)
		{
			index += array_cabsCount[s];
		}
		else
		{
			index += cab;
			break;
		}
	}

	return index;
}

int _shaftForCab(int cabindex)
{
	int index = 0;
	int s;

	for (s = 0; s < count_shafts; ++s)
	{
		if (cabindex < index+array_cabsCount[s])
		{
			index += array_cabsCount[s];
		}
	}

	return s-1;
}

int _cabShouldMoveLong(int cabindex, cabdir_t direction, short ignoreRestFloor)
{
	cabinfo_t cab = array_cabs[cabindex];
	cabdir_t chosenDirection = 0;

	int shaft = _shaftForCab(cabindex);

	// Answer requests from inside the cab first
	if (direction == kElevationCabDirectionUp)
	{
		for (int f = cab.curFloor+1; f <= array_shaftsRange[shaft].max; f++)
		{
			if (cab.calls_in[f] > 0)
			{
				// Has call from inside the cab on floor f
				chosenDirection = kElevationCabDirectionUp; //Cab should continue going upwards
				break;
			}
		}
	}
	else if (direction == kElevationCabDirectionDown)
	{
		for (int f = cab.curFloor; f >= array_shaftsRange[shaft].min; f--)
		{
			if (cab.calls_in[f] > 0)
			{
				// Has call from inside the cab on floor f
				chosenDirection = kElevationCabDirectionDown; //Cab should continue going downwards
				break;
			}
		}
	}

	// Answer requests from the outside
	if (chosenDirection == 0 && direction == kElevationCabDirectionUp)
	{
		// Check for calls in the same direction as cab movement
		for (int f = cab.curFloor+1; f <= array_shaftsRange[shaft].max; f++)
		{
			if (cab.calls_out[f] & kElevationCallGoUp) {
				chosenDirection = kElevationCabDirectionUp;
				break;
			}
		}
	}

	if (chosenDirection == 0 && direction == kElevationCabDirectionUp) {
		// Check for calls in the oposite direction from top to the bottom
		for (int f = array_shaftsRange[shaft].max; f >= cab.curFloor; f--)
		{
			if (cab.calls_out[f] & kElevationCallGoDown) {
				chosenDirection = kElevationCabDirectionUp;
				break;
			}
		}
	}

	if (chosenDirection == 0 && direction == kElevationCabDirectionDown)
	{
		// Check for calls in the same direction from top to the bottom (because we're going down!)
		for (int f = cab.curFloor; f >= array_shaftsRange[shaft].min; f--)
		{
			if (cab.calls_out[f] & kElevationCallGoDown) {
				chosenDirection = kElevationCabDirectionDown;
				break;
			}
		}
	}

	if (chosenDirection == 0 && direction == kElevationCabDirectionDown) {
		// Check for calls in the opposite direction as cab movement
		for (int f = array_shaftsRange[shaft].min; f < cab.curFloor; f++)
		{
			if (cab.calls_out[f] & kElevationCallGoUp) {
				chosenDirection = kElevationCabDirectionDown;
				break;
			}
		}
	}

	if (!ignoreRestFloor) {
		// No requests to answer, go back to rest floor
		if (chosenDirection == 0 && cab.restFloor > cab.curFloor) {
			chosenDirection = kElevationCabDirectionUp;
		} else if (chosenDirection == 0 && cab.restFloor < cab.curFloor) {
			chosenDirection = kElevationCabDirectionDown;
		}
	}

	return chosenDirection;
}

int _cabShouldMove(int cabindex, cabdir_t direction)
{
	return _cabShouldMoveLong(cabindex, direction, 0);
}

#pragma mark - Master Functions

void elevation_init(int lshafts, int *lcabs, int lfloors, tuple_range *lshaftRanges)
{
	printf("Initializing elevation...\n");

	count_shafts = lshafts;
	count_floors = lfloors;

	count_totalCabs = 0;

	for (int s = 0; s < count_shafts; ++s)
	{
		count_totalCabs += lcabs[s];
	}

	array_cabsCount = malloc(sizeof(int) * count_shafts);
	memcpy(array_cabsCount, lcabs, sizeof(int) * count_shafts);

	array_shaftsRange = malloc(sizeof(tuple_range) * lshafts);
	memcpy(array_shaftsRange, lshaftRanges, sizeof(tuple_range) * count_shafts);

	array_cabs = malloc(sizeof(cabinfo_t) * count_totalCabs);

	for (int s = 0; s < count_shafts; ++s)
	{
		for (int c = 0; c < lcabs[s]; ++c)
		{
			// Initialize each cab
			printf("Initializing cab %d\n", _cabindex(s, c));

			array_cabs[_cabindex(s, c)].direction = kElevationCabDirectionUp;
			array_cabs[_cabindex(s, c)].curFloor = lshaftRanges[s].min;
			array_cabs[_cabindex(s, c)].restFloor = lshaftRanges[s].min;
			array_cabs[_cabindex(s, c)].load = 0;

			array_cabs[_cabindex(s, c)].calls_in = malloc(sizeof(short)*count_floors);
			array_cabs[_cabindex(s, c)].calls_out = malloc(sizeof(short)*count_floors);

			for (int i = 0; i < count_floors; ++i)
			{
				array_cabs[_cabindex(s, c)].calls_in[i] = 0;
				array_cabs[_cabindex(s, c)].calls_out[i] = 0;
			}
		}

		array_shaftsRange[s].min = 0;
		array_shaftsRange[s].max = count_floors-1;
	}

	printf("Initialization done!\n");
}

void elevation_destroy()
{
	for (int i=0; i<count_totalCabs; i++) {
		free(array_cabs[i].calls_in);
		free(array_cabs[i].calls_out);
	}

	free(array_cabsCount);
	free(array_cabs);
}

/**
 *  Ticks the simulator. After each tick each cab moves one floor if it needs to.
 */
void elevation_tick()
{
	cabinfo_t *cab;

	for (int i = 0; i < count_totalCabs; ++i)
	{
		cab = &array_cabs[i];

		if (cab->calls_in[cab->curFloor] > 0 || (_cabShouldMove(i, cab->direction) == 0 && cab->restFloor == cab->curFloor))
		{
			cab->calls_in[cab->curFloor] = 0;

			if (!cab->stopped) {
				printf("Cab %d stopped at floor %d\n", i, cab->curFloor);
			}
			cab->stopped = 1;
		}
		else if (cab->calls_out[cab->curFloor] > 0)
		{
			if (cab->calls_out[cab->curFloor] == kElevationCallGoUp && cab->direction == kElevationCabDirectionUp)
			{
				cab->calls_out[cab->curFloor] &= ~kElevationCallGoUp;
			}
			else if (cab->calls_out[cab->curFloor] == kElevationCallGoDown && cab->direction == kElevationCabDirectionDown)
			{
				cab->calls_out[cab->curFloor] &= ~kElevationCallGoDown;
			}
			else if (_cabShouldMoveLong(i, kElevationCabDirectionUp, 1) == 0)
			{
				cab->calls_out[cab->curFloor] = 0;
			}

			if (!cab->stopped) {
				printf("Cab %d stopped at floor %d\n", i, cab->curFloor);
			}
			cab->stopped = 1;
		}
		else
		{
			switch(cab->direction)
			{
				case kElevationCabDirectionUp:
					if (_cabShouldMove(i, kElevationCabDirectionUp) > 0)
					{
						cab->curFloor++;
						cab->stopped = 0;
					}
					else if (_cabShouldMove(i, kElevationCabDirectionDown) < 0)
						//					{
						cab->direction = kElevationCabDirectionDown;
					//						cab->curFloor--;
					//						cab->stopped = 0;
					//					}
					break;

				case kElevationCabDirectionDown:
					if (_cabShouldMove(i, kElevationCabDirectionDown) < 0)
					{
						cab->curFloor--;
						cab->stopped = 0;
					}
					else if (_cabShouldMove(i, kElevationCabDirectionUp) > 0)
						//					{
						cab->direction = kElevationCabDirectionUp;
					//						cab->curFloor++;
					//						cab->stopped = 0;
					//					}
					break;
			}
		}
	}
}

#pragma mark - Operation Functions

/**
 *  Sets a request on a shaft from a floor.
 *
 *  @param floor     The floor where the request was made.
 *  @param shaft     The index of the shaft the requester sent the request to.
 *  @param direction If the request is to go up or down.
 */
void elevation_call_floor(int floor, int shaft, cabdir_t direction)
{
	// Find best cab in shaft to serve the floor
	cabinfo_t *cab;
	int *distances = malloc(sizeof(int)*array_cabsCount[shaft]);
	int *loads = malloc(sizeof(int)*array_cabsCount[shaft]);
	int nearest = count_floors;
	int leastLoadedIndex = -1, smallestLoad = 50;

	for (int i=0; i < array_cabsCount[shaft]; i++) {
		distances[i] = 0;
		cab = &array_cabs[_cabindex(shaft, i)];

		// If the cab is stopped at the floor, give it the request
		if (cab->stopped && cab->curFloor == floor) {
			cab->calls_out[floor] |= (direction == kElevationCabDirectionUp ? kElevationCallGoUp : kElevationCallGoDown);
			break;
		}

		// If not lets find the closest cab
		if (cab->direction == direction || cab->stopped)
		{
			distances[i] = abs(cab->curFloor - floor);
		} else {
			int highestCall = -1, lowestCall = -1;

			for (int f = array_shaftsRange[shaft].min; f < array_shaftsRange[shaft].max; f++) {
				if (cab->calls_in[f] > 0) {
					if (lowestCall < 0) lowestCall = f;
					highestCall = f;
				}
				if (cab->calls_out[f] != 0) {
					if (lowestCall < 0) lowestCall= f;
					highestCall = f;
				}
			}

			if (highestCall > 0 && lowestCall > 0) {
				distances[i] = (highestCall - cab->curFloor)+(highestCall - lowestCall)+(cab->curFloor - lowestCall);
			} else {
				distances[i] = abs(cab->curFloor - floor);
			}
		}

		loads[i] = cab->load;
	}

	// Check for shortest distance
	for (int i=0; i < array_cabsCount[shaft]; i++) {
		if (distances[i] < nearest) {
			nearest = distances[i];
		}
	}

	// From the closest cabs, find the least loaded
	for (int i=0; i < array_cabsCount[shaft]; i++) {
		if (distances[i] == nearest && loads[i] < smallestLoad) {
			smallestLoad = loads[i];
			leastLoadedIndex = i;
		}
	}

	array_cabs[leastLoadedIndex].calls_out[floor] |= (direction == kElevationCabDirectionUp ? kElevationCallGoUp : kElevationCallGoDown);

	free(distances);
	free(loads);
}

/**
 *  Sets a request on a floor from within a cab.
 *
 *  @param floor The floor to which the requester demands to go.
 *  @param shaft The index of the shaft containing the cab from which the request was sent.
 *  @param cab   The index of the cab within the cabs of the parameter shaft.
 */
void elevation_call_cab(int floor, int shaft, int cab)
{
	array_cabs[_cabindex(shaft, cab)].calls_in[floor] = 1;
}

int elevation_cab_get_floor(int shaft, int cab)
{
	return array_cabs[_cabindex(shaft, cab)].curFloor;
}

cabdir_t elevation_cab_get_direction(int shaft, int cab)
{
	return array_cabs[_cabindex(shaft, cab)].direction;
}

int elevation_cab_set_rest_floor(int shaft, int cab, int floor)
{
	if (floor >= array_shaftsRange[shaft].min && floor <= array_shaftsRange[shaft].max) {
		array_cabs[_cabindex(shaft, cab)].restFloor = floor;
		return 1;
	}
	return 0;
}

int elevation_cab_get_rest_floor(int shaft, int cab)
{
	return array_cabs[_cabindex(shaft, cab)].restFloor;
}

int elevation_cab_get_load(int shaft, int cab)
{
	return array_cabs[_cabindex(shaft, cab)].load;
}

int elevation_cab_set_load(int shaft, int cab, int people)
{
	if (people < 21)
	{
		array_cabs[_cabindex(shaft, cab)].load = people;
		return 1;
	} else
		return 0;
}

int elevation_cab_increment_load(int shaft, int cab)
{
	if (array_cabs[_cabindex(shaft, cab)].load < 21)
	{
		array_cabs[_cabindex(shaft, cab)].load++;
		return 1;
	} else
		return 0;
}

int elevation_cab_decrement_load(int shaft, int cab)
{
	if (array_cabs[_cabindex(shaft, cab)].load > 0)
	{
		array_cabs[_cabindex(shaft, cab)].load--;
		return 1;
	} else
		return 0;
}

#pragma mark - Shaft Functions

void elevation_shaft_set_range(int shaft, tuple_range range)
{
	array_shaftsRange[shaft] = range;
	if (range.max > count_floors) {
		count_floors = range.max;
	}
}

tuple_range elevation_shaft_get_range(int shaft)
{
	return array_shaftsRange[shaft];
}
