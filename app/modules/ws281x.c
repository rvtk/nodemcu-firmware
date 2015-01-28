// Module for interfacing with WS2811/WS2812/WS2812B leds using GPIO

//#include "lua.h"
//#include "lualib.h"
#include "lauxlib.h"
#include "platform.h"
//#include "auxmods.h"
//#include "lrotable.h"

//#include "c_types.h"
//#include "c_string.h"

#define FLOAT PLATFORM_GPIO_FLOAT
#define OUTPUT PLATFORM_GPIO_OUTPUT

// ----------------------------------------------------------------------------
// -- This WS2812 code must be compiled with -O2 to get the timing right.
// -- http://wp.josh.com/2014/05/13/ws2812-neopixels-are-not-so-finicky-once-you-get-to-know-them/

// The ICACHE_FLASH_ATTR is there to trick the compiler and get the very first pulse width correct.
static void ICACHE_FLASH_ATTR send_ws_0(uint8_t gpio, uint8_t t0, uint8_t t1)
{
  uint8_t i;
  i = t0 /*4*/; while (i--) GPIO_REG_WRITE(GPIO_OUT_W1TS_ADDRESS, 1 << gpio);
  i = t1 /*9*/; while (i--) GPIO_REG_WRITE(GPIO_OUT_W1TC_ADDRESS, 1 << gpio);
}

static void ICACHE_FLASH_ATTR send_ws_1(uint8_t gpio, uint8_t t2, uint8_t t3)
{
  uint8_t i;
  i = t2 /*8*/; while (i--) GPIO_REG_WRITE(GPIO_OUT_W1TS_ADDRESS, 1 << gpio);
  i = t3 /*6*/; while (i--) GPIO_REG_WRITE(GPIO_OUT_W1TC_ADDRESS, 1 << gpio);
}

// Lua: ws2812(pin, "string")
// Byte triples in the string are interpreted as G R B values.
// gpio.ws2812(4, string.char(0, 255, 0)) uses GPIO2 and sets the first LED red.
// gpio.ws2812(3, string.char(0, 0, 255):rep(2)) uses GPIO0 and sets ten LEDs blue.
// gpio.ws2812(4, string.char(255, 0, 0, 255, 255, 255)) first LED green, second LED white.
static int ICACHE_FLASH_ATTR lws281x_run(lua_State* L)
{
  const uint8_t pin = luaL_checkinteger(L, 1);
  size_t length;
  const char *buffer = luaL_checklstring(L, 2, &length);
  
  const uint8_t t0 = luaL_checkinteger(L, 3);
  const uint8_t t1 = luaL_checkinteger(L, 4);
  const uint8_t t2 = luaL_checkinteger(L, 5);
  const uint8_t t3 = luaL_checkinteger(L, 6);

  platform_gpio_mode(pin, OUTPUT, FLOAT);
  platform_gpio_write(pin, 0);
  os_delay_us(100);

  os_intr_lock();
  const char * const end = buffer + length;
  while (buffer != end) {
    uint8_t mask = 0x80;
    while (mask) {
      (*buffer & mask) ? send_ws_1(pin_num[pin], t2, t3) : send_ws_0(pin_num[pin], t0, t1);
      mask >>= 1;
    }
    ++buffer;
  }
  os_intr_unlock();

  return 0;
}

// Module function map
#define MIN_OPT_LEVEL 2
#include "lrodefs.h"
const LUA_REG_TYPE ws281x_map[] = 
{
  { LSTRKEY( "run" ), LFUNCVAL( lws281x_run ) },
  //{ LSTRKEY( "on" ), LFUNCVAL( lws281x_on ) },
  //{ LSTRKEY( "getColor" ), LFUNCVAL( lws281x_getColor ) },  
  //{ LSTRKEY( "setColor" ), LFUNCVAL( lws281x_setColor ) },
  //{ LSTRKEY( "getNumberOfLeds" ), LFUNCVAL( lws281x_getNumberOfLeds ) },
  //{ LSTRKEY( "setNumberOfLeds" ), LFUNCVAL( lws281x_setNumberOfLeds ) },
  { LNILKEY, LNILVAL }
};

LUALIB_API int luaopen_ws281x (lua_State *L) {
  LREGISTER( L, "ws281x", ws281x_map );
}
