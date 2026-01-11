#pragma once
#include <gui/modules/submenu.h>
#include "../app/app_fwd.h"

void pcaps_page_init(Submenu* menu, App* app);
void pcaps_page_build_menu(App* app, uint32_t keep_item_id);
