#define SHMSIZE 27
#define MAXPIDS 20

struct shrd_mem {
	bool choosing[MAXPIDS];
	int numbers[MAXPIDS];
};