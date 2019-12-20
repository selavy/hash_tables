#pragma once

#include "quadratic_open_addressing.h"

QOA__TYPES(q, qoa_table_t(q), int, int)
// TODO: would be nice to have a version that lets you
//       control the "namespace"
QOA__PROTOS(q)
#define qkey           qoa_key
#define qval           qoa_val
#define qend           qoa_end
#define qsize          qoa_size
#define qcreate()      qoa_create(q)
#define qdestroy(t)    qoa_destroy(q, t)
#define qput(t, k)     qoa_put(q, t, k)
typedef qoa_result_t   qresult_t;
typedef qoa_iter_t     qiter_t;
typedef qoa_key_t(q)   qkey_t;
typedef qoa_val_t(q)   qval_t;
typedef qoa_table_t(q) qtable;
