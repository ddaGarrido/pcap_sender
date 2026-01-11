#pragma once
#include <gui/modules/submenu.h>
#include <gui/modules/text_input.h>
#include "../app/app_fwd.h"

void config_page_init(Submenu* menu, App* app);
void config_page_build_menu(App* app);
void config_page_setup_text_input(TextInput* text_input, App* app);
