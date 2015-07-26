#include "util/io.h"

namespace vgatext{

   static inline void writechar(int loc, uint8_t c, uint8_t bg, uint8_t fg, addr_t base){

    mmio::write8(base,loc*2,  (c&255) );
	mmio::write8(base,loc*2+1, (bg<<4) + fg );
	
	// mmio::write16(base,loc*2, (bg<<12) + (fg<<8) + (c&255) );
   }

}//namespace vgatext
