/*! @file
  @brief
  exception 

  <pre>
  Copyright (C) 2015-2018 Kyushu Institute of Technology.
  Copyright (C) 2015-2018 Shimane IT Open-Innovation Center.

  This file is distributed under BSD 3-Clause License.


  </pre>
*/

#ifndef MRBC_SRC_ERROR_H_
#define MRBC_SRC_ERROR_H_

#include "value.h"

#ifdef __cplusplus
extern "C" {
#endif

void mrbc_init_class_exception(struct VM *vm);
void c_mrbc_raise(mrbc_vm *vm, mrbc_error_code err, char *msg);

#ifdef __cplusplus
}
#endif
#endif
