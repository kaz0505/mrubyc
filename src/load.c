/*! @file
  @brief
  mruby bytecode loader.

  <pre>
  Copyright (C) 2015-2020 Kyushu Institute of Technology.
  Copyright (C) 2015-2020 Shimane IT Open-Innovation Center.

  This file is distributed under BSD 3-Clause License.

  </pre>
*/

#include "vm_config.h"
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>

#include "vm.h"
#include "load.h"
#include "value.h"
#include "alloc.h"
#include "console.h"

//
// This is a dummy code for raise
//
#define mrbc_raise(vm,err,msg) console_printf("<raise> %s:%d\n", __FILE__, __LINE__);


// IREP TT
enum irep_pool_type {
  IREP_TT_STR   = 0,          /* string (need free) */
  IREP_TT_SSTR  = 2,          /* string (static) */
  IREP_TT_INT32 = 1,          /* 32bit integer */
  IREP_TT_INT64 = 3,          /* 64bit integer */
  IREP_TT_FLOAT = 5,          /* float (double/float) */
};



//================================================================
/*! Parse header section.

  @param  vm    A pointer of VM.
  @param  pos	A pointer of pointer of RITE header.
  @return int	zero if no error.

  <pre>
  Structure
   "RITE"     identifier
   "01"       major version
   "00"       minor version
   0000_0000  total size
   "MATZ"     compiler name
   "0000"     compiler version
  </pre>
*/
static int load_header(struct VM *vm, const uint8_t **pos)
{
  const uint8_t *p = *pos;

  if( memcmp(p, "RITE02", 6) != 0 ) {
    mrbc_raise(vm, E_BYTECODE_ERROR, NULL);
    return -1;
  }

  /* Ignore CRC */

  /* Ignore size */

  if( memcmp(p + 12, "MATZ", 4) != 0 ) {
    mrbc_raise(vm, E_BYTECODE_ERROR, NULL);
    return -1;
  }
  if( memcmp(p + 16, "0000", 4) != 0 ) {
    mrbc_raise(vm, E_BYTECODE_ERROR, NULL);
    return -1;
  }

  *pos += 20;
  return 0;
}



//================================================================
/*! read one irep section.

  @param  vm    A pointer of VM.
  @param  pos	A pointer of pointer of IREP section.
  @return       Pointer of allocated mrbc_irep or NULL

  <pre>
   (loop n of child irep bellow)
   0000_0000	record size
   0000		n of local variable
   0000		n of register
   0000		n of child irep

   0000_0000	n of byte code  (ISEQ BLOCK)
   ...		byte codes

   0000_0000	n of pool	(POOL BLOCK)
   (loop n of pool)
     00		type
     0000	length
     ...	pool data

   0000_0000	n of symbol	(SYMS BLOCK)
   (loop n of symbol)
     0000	length
     ...	symbol data
  </pre>
*/
static mrbc_irep * load_irep_1(struct VM *vm, const uint8_t **pos)
{
  const uint8_t *p = *pos + 4;			// skip record size

  // new irep
  mrbc_irep *irep = mrbc_irep_alloc(0);
  if( irep == NULL ) {
    mrbc_raise(vm, E_BYTECODE_ERROR, NULL);
    return NULL;
  }

  // nlocals,nregs,rlen
  irep->nlocals = bin_to_uint16(p);	p += 2;
  irep->nregs = bin_to_uint16(p);	p += 2;
  irep->rlen = bin_to_uint16(p);	p += 2;
  irep->clen = bin_to_uint16(p);        p += 2;
  irep->ilen = bin_to_uint16(p);	p += 2;

  // allocate memory for child irep's pointers
  if( irep->rlen ) {
    irep->reps = (mrbc_irep **)mrbc_alloc(0, sizeof(mrbc_irep *) * irep->rlen);
    if( irep->reps == NULL ) {
      mrbc_raise(vm, E_BYTECODE_ERROR, NULL);
      return NULL;
    }
  }

  // ISEQ (code) BLOCK
  irep->code = (uint8_t *)p;
  p += irep->ilen + sizeof(mrbc_irep_catch_handler) * irep->clen;
  assert( sizeof(mrbc_irep_catch_handler) == 13 );
  
  // POOL BLOCK
  irep->plen = bin_to_uint16(p);	p += 2;
  if( irep->plen ) {
    irep->pools = (mrbc_object**)mrbc_alloc(0, sizeof(void*) * irep->plen);
    if(irep->pools == NULL ) {
      mrbc_raise(vm, E_BYTECODE_ERROR, NULL);
      return NULL;
    }
  }

  int i;
  for( i = 0; i < irep->plen; i++ ) {
    int tt = *p++;
    mrbc_object *obj = mrbc_alloc(0, sizeof(mrbc_object));
    if( obj == NULL ) {
      mrbc_raise(vm, E_BYTECODE_ERROR, NULL);
      return NULL;
    }
    switch( tt ) {
#if MRBC_USE_STRING
    case IREP_TT_STR:
    case IREP_TT_SSTR: {
      int pool_data_len = bin_to_uint16(p);
      p += sizeof(uint16_t);
      obj->tt = MRBC_TT_STRING;
      obj->str = (char*)p;
      p += pool_data_len + 1;
    } break;
#endif
    case IREP_TT_INT32: {
      uint32_t value = bin_to_uint32(p);
      p += sizeof(uint32_t);
      obj->tt = MRBC_TT_FIXNUM;
      obj->i = value;
    } break;
#if MRBC_USE_FLOAT
    case IREP_TT_FLOAT: {
      double value;
      memcpy(&value, p, sizeof(double));
      p += sizeof(double);
      obj->tt = MRBC_TT_FLOAT;
      obj->d = value;
    } break;
#endif
    case IREP_TT_INT64: {
#ifdef MRBC_INT64
      uint64_t value = bin_to_uint32(p);
      p += sizeof(uint32_t);
      value <<= 32;
      value |= bin_to_uint32(p);
      p += sizeof(uint32_t);      
      obj->tt = MRBC_TT_FIXNUM;
      obj->i = value;
#else
      p += sizeof(uint32_t) * 2;
      mrbc_raise(vm, E_BYTECODE_ERROR, NULL);
#endif
    } break;
    default:
      assert(!"Unknown tt");
    }

    irep->pools[i] = obj;
  }

  // SYMS BLOCK
  irep->ptr_to_sym = (uint8_t*)p;
  int slen = bin_to_uint16(p);		p += 2;
  while( --slen >= 0 ) {
    int s = bin_to_uint16(p);		p += 2;
    p += s+1;
  }

  *pos = p;
  return irep;
}



//================================================================
/*! read all irep section.

  @param  vm    A pointer of VM.
  @param  pos	A pointer of pointer of IREP section.
  @return       Pointer of allocated mrbc_irep or NULL
*/
static mrbc_irep * load_irep_0(struct VM *vm, const uint8_t **pos)
{
  mrbc_irep *irep = load_irep_1(vm, pos);
  if( !irep ) return NULL;

  int i;
  for( i = 0; i < irep->rlen; i++ ) {
    irep->reps[i] = load_irep_0(vm, pos);
  }

  return irep;
}



//================================================================
/*! Parse IREP section.

  @param  vm    A pointer of VM.
  @param  pos	A pointer of pointer of IREP section.
  @return int	zero if no error.

  <pre>
  Structure
   "IREP"	section identifier
   0000_0000	section size
   "0000"	rite version
  </pre>
*/
static int load_irep(struct VM *vm, const uint8_t **pos)
{
  const uint8_t *p = *pos;                      // start at "IREP"
  p += 4;		  	                // skip "IREP"
  int section_size = bin_to_uint32(p);
  p += 4;
  if( memcmp(p, "0300", 4) != 0 ) {		// rite version
    mrbc_raise(vm, E_BYTECODE_ERROR, NULL);
    return -1;
  }
  p += 4;
  vm->irep = load_irep_0(vm, &p);
  if( vm->irep == NULL ) {
    return -1;
  }

  *pos += section_size;
  return 0;
}



//================================================================
/*! Parse LVAR section.

  @param  vm    A pointer of VM.
  @param  pos	A pointer of pointer of LVAR section.
  @return int	zero if no error.
*/
static int load_lvar(struct VM *vm, const uint8_t **pos)
{
  const uint8_t *p = *pos;

  /* size */
  *pos += bin_to_uint32(p+4);

  return 0;
}


//================================================================
/*! Load the VM bytecode.

  @param  vm    Pointer to VM.
  @param  ptr	Pointer to bytecode.

*/
int mrbc_load_mrb(struct VM *vm, const uint8_t *ptr)
{
  int ret = -1;
  vm->mrb = ptr;

  ret = load_header(vm, &ptr);
  while( ret == 0 ) {
    if( memcmp(ptr, "IREP", 4) == 0 ) {
      ret = load_irep(vm, &ptr);
    }
    else if( memcmp(ptr, "LVAR", 4) == 0 ) {
      ret = load_lvar(vm, &ptr);
    }
    else if( memcmp(ptr, "END\0", 4) == 0 ) {
      break;
    }
  }

  return ret;
}
