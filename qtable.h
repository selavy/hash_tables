#pragma once

#include "quadratic_open_addressing.h"

QOA__TYPES(qtable, int, int)
QOA__PROTOS(q, qtable, int, int)
#define qkey         qoa_key
#define qval         qoa_val
#define qend         qoa_end
#define qsize        qoa_size
typedef qoa_result_t qresult_t;
typedef qoa_iter_t   qiter_t;
