
define (stuff bar) {
  readhook = {
    int i = 1;
  };
  writehook = func(intbv<32> val) {
    int i = 1;
  };
}
