//
// configuration.sw
//

// configuration globals

#include "plasma.h"

#ifndef _CONFIGURATION_
#define _CONFIGURATION_

extern const char *version;
extern const char *copyright;

typedef int int32;

// platform
extern plasma::Processor zen;
extern plasma::Processor tpu;
extern plasma::Processor hardware;

// performance
extern int32 cpu_MIPS[3];		// slot n holds mips for cpu n

extern const int32 zen_mips;
extern const int32 tpu_mips;

// ticks per second
extern const int32 ticks_per_second;			// a tick is a microsecond

// globals
extern double fuelrate;					// units of fuel per second: 	ml per sec
extern double fuelpush;					// impulse per unit of fuel: 	newton-seconds per ml
extern double mass;						// 'weight' of car :			kg
extern double speed;					// car speed:					meters/sec

extern double speedratio;				// 'rpm' per unit speed
extern double friction;					// coefficient of friction

extern int32 num_teeth;				// number of teeth on flywheel
extern int32 num_cylinders;			// number of cylinders.
const int32 cylinder_max = 64;	    // don't handle engines with more than this num of cyls

extern int32 tooth_chunk;
								 
extern const int32 circle;	// a complete circle has this number of units (a million)

// the shared accelerator variable
extern int32 gAccelerator;

// ecu asking injector to squirt fuel
struct fuel_request {
  int32 squirt_time;
  double fuel_amount;
};

typedef plasma::Channel<int> Chan;
typedef plasma::Channel<fuel_request> FuelChan;

typedef std::vector<Chan> Chans;
typedef std::vector<FuelChan> FuelChans;

// logging file.
extern FILE *logfile;
	
// the shared variables for angle management
extern int32 measured_angle, est_angle_rate, est_angle_accel;		// parameters of angle movement
extern int32 measured_t;									// time at which params valid

#endif
