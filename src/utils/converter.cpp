#include "utils/converter.h"

namespace webserver {

std::string ConvertIP(int ip_addr) {
  char ipstr[64];
  unsigned char *pch = (unsigned char *)(&ip_addr);
  sprintf(ipstr, "%d.%d.%d.%d", *pch, *(pch + 1), *(pch + 2), *(pch + 3));
  return std::string(ipstr);
}

}  // namespace webserver
