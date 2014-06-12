
typedef enum
{
	kElevationCabDirectionUp = 1,
	kElevationCabDirectionDown = -1
} cabdir_t;

typedef struct
{
	int min;
	int max;
} tuple_range;

void elevation_init(int lshafts, int *lcabs, int lfloors, tuple_range *lshaftRanges);
void elevation_destroy();

void elevation_tick();

void elevation_call_floor(int floor, int shaft, cabdir_t direction);
void elevation_call_cab(int floor, int shaft, int cab);

int elevation_cab_get_floor(int shaft, int cab);
cabdir_t elevation_cab_get_direction(int shaft, int cab);
int elevation_cab_set_rest_floor(int shaft, int cab, int floor);
int elevation_cab_get_rest_floor(int shaft, int cab);

int elevation_cab_get_load(int shaft, int cab);
int elevation_cab_set_load(int shaft, int cab, int people);
int elevation_cab_increment_load(int shaft, int cab);
int elevation_cab_decrement_load(int shaft, int cab);

void elevation_shaft_set_range(int shaft, tuple_range range);
tuple_range elevation_shaft_get_range(int shaft);
