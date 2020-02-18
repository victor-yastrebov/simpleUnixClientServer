#include"UDSServerLibEvent.h"

int main(int argc, char **argv)
{
   UDSServerLibEvent sle;
   sle.Run();
   std::cout << "OK" << std::endl;

   return 1;
}