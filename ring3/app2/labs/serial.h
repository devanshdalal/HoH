#pragma once

#include "util/io.h"

namespace serial{

  static inline bool is_transmitter_ready(io_t baseport){

    uint8_t res = io::read8(baseport,5);
    return res&(1<<5);

  }
  static inline void writechar(uint8_t c, io_t baseport){

    io::write8(baseport, 0 , c );

  }

} //end serial
