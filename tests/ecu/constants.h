// constants.sw

// names for the base values of time, mass,  distance, volume and force

#ifndef _CONSTANTS_
#define _CONSTANTS_

const double 	musec 	= 1.0,
			 	msec	= musec * 1000.0,
				sec		= msec * 1000.0,
				min		= sec * 60.0,
				hour	= min * 60.0,
				day		= hour * 24.0,
				
				mugram	= 1.0,
				mgram	= mugram * 1000.0,
				gram	= mgram * 1000.0,
				kg		= gram * 1000.0,
				
				mum		= 1.0,
				mm		= 1000.0 * mum,
				meter	= 1000.0 * mm,
				km		= 1000.0 * meter,
				
				mul		= 1.0,
				ml		= 1000.0 * mul,
				liter	= 1000.0 * ml,
				
				mn		= 1000.0,
				newton	= 1000.0 * mn;

#endif
