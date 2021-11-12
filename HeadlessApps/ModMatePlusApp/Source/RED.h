/*
RED.h
2015-11-18
Public Domain
*/

#ifndef RED_H
#define RED_H

typedef void (*RED_CB_t)(int);

struct _RED_s;

typedef struct _RED_s RED_t;

#define RED_MODE_DETENT 0
#define RED_MODE_STEP   1

/*

RED starts a rotary encoder on Pi pi with GPIO gpioA,
GPIO gpioB, mode mode, and callback cb_func.  The mode
determines whether the four steps in each detent are
reported as changes or just the detents.

If cb_func in not null it will be called at each position
change with the new position.

The current position can be read with RED_get_position and
set with RED_set_position.

Mechanical encoders may suffer from switch bounce.
RED_set_glitch_filter may be used to filter out edges
shorter than glitch microseconds.  By default a glitch
filter of 1000 microseconds is used.

At program end the rotary encoder should be cancelled using
RED_cancel.  This releases system resources.

*/

RED_t *RED                   (int pi,
                              int gpioA,
                              int gpioB,
                              int mode,
                              RED_CB_t cb_func);

void   RED_cancel            (RED_t *renc);

void   RED_set_glitch_filter (RED_t *renc, int glitch);

void   RED_set_position      (RED_t *renc, int position);

int    RED_get_position      (RED_t *renc);

#endif

