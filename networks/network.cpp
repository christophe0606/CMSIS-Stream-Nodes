extern "C" {
#include "network.h"
}

namespace arm {
namespace app {
namespace kws {

uint8_t *GetModelPointer();
size_t GetModelLen();

}  // namespace kws
}  // namespace app
}  


const uint8_t * GetModelPointer()
{
   return arm::app::kws::GetModelPointer();
}

size_t GetModelLen()
{
   return arm::app::kws::GetModelLen();
}

