#pragma once

#include "generated/lpc_kbd.dev.h"

namespace lpc_kbd{

  static inline bool has_key(lpc_kbd_t& dev){

  	lpc_kbd_status_t res = lpc_kbd_status_rd(&dev);
    return lpc_kbd_status_obf_extract(res)&(0x1);
    // return 1;

  }

  static inline uint8_t get_key(lpc_kbd_t& dev){

    return lpc_kbd_input_rd(&dev);

  }

}// namespace lpc_kbd

