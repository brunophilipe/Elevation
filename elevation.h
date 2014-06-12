
typedef enum
{
	kElevationCabDirectionUp = 1,
	kElevationCabDirectionDown = -1
} cabdir_t;

void elevation_init(int shafts, int *cabs, int floors);
void elevation_destroy();

void elevation_tick();

void elevation_call_floor(int floor, int shaft);
void elevation_call_cab(int floor, int shaft, int cab);

int elevation_cab_get_load(int shaft, int cab);
int elevation_cab_get_floor(int shaft, int cab);
cabdir_t elevation_cab_get_direction(int shaft, int cab);

int elevation_cab_set_load(int shaft, int cab, int people);
int elevation_cab_increment_load(int shaft, int cab);
int elevation_cab_decrement_load(int shaft, int cab);
