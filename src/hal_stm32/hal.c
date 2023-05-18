/*! @file
  @brief
  Hardware abstraction layer
        for STM32

  <pre>
  Copyright (C) 2016-2023 Kyushu Institute of Technology.
  Copyright (C) 2016-2023 Shimane IT Open-Innovation Center.

  This file is distributed under BSD 3-Clause License.
  </pre>
*/

/***** Feature test switches ************************************************/
/***** System headers *******************************************************/
#include <mbed.h>

/***** Local headers ********************************************************/
#include "hal.h"

/***** Constat values *******************************************************/
/***** Macros ***************************************************************/
/***** Typedefs *************************************************************/
/***** Function prototypes **************************************************/
/***** Local variables ******************************************************/
/***** Global variables *****************************************************/
/***** Signal catching functions ********************************************/
/***** Local functions ******************************************************/
/***** Global functions *****************************************************/

#if 0
// This is one sample implementation.
//================================================================
/*!@brief
  Write

  @param  fd		dummy, but 1.
  @param  buf		pointer to buffer.
  @param  nbytes	output byte length.
*/
int hal_write(int fd, const void *buf, int nbytes)
{
  // TODO: hal_write
  return nbytes;
}

//================================================================
/*!@brief
  Flush write baffer

  @param  fd	dummy, but 1.
*/
int hal_flush(int fd)
{
  // TODO: hal_flush
  return 0;
}

//================================================================
/*!@brief
  abort program

  @param s	additional message.
*/
void hal_abort(const char *s)
{
  // TODO: hal_abort
}
#endif
