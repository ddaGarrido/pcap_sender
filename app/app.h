#pragma once
#include "app_state.h"

App* app_alloc(void);
void app_run(App* app);
void app_free(App* app);
