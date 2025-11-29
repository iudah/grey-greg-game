#include "scale_component.h"
#include <zot.h>

struct scale_component *scale_component;


bool initialize_scale_component() {
  scale_component = zcalloc(1, sizeof(struct scale_component));
  return scale_component != NULL;
}