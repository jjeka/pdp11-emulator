#ifndef PDP_MAIN_H
#define PDP_MAIN_H

#include "entry.c"

#include "start.h"
#include "utils.h"
#include "graphics.h"
#include "keyboard.h"

#ifdef PDP_ADD_SOURCE

#include "start.c"
#include "utils.c"
#include "graphics.c"
#include "keyboard.c"

#endif // PDP_ADD_SOURCE

#endif // PDP_MAIN_H