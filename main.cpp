#include <iostream>
#include "MemoryConstants.h"
#include "Utilities.h"
int main ()
{
  std::cout << get_address_offset_in_level((uint64_t)0b10110010001101000101, (uint16_t)0)<< std::endl;
  return 0;
}
