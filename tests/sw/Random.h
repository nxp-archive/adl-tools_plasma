//
// Random number generation.  There are N independent streams, where N
// is set by the user.
//

#ifndef _RANDOM_H_
#define _RANDOM_H_

class Random {
public:
  Random(unsigned num_streams);
  unsigned num_streams() const;
  int get(unsigned stream);
  int uniform(unsigned stream, int base,int limit);
  void reset(int num_streams);
private:
  unsigned  _ns;
  int      *_streams;
};

#endif
