#pragma once
#include <gui/view.h>
#include "../app/app_fwd.h"

typedef struct {
    View* view;
    char send_text[2048];
    const char* send_lines[256];
    uint16_t send_line_count;
    uint16_t send_scroll;
} SendPage;

void send_page_create(SendPage* page);
void send_page_destroy(SendPage* page);

View* send_page_get_view(SendPage* page);
void send_page_bind(SendPage* page, App* app);

// page actions
void send_page_open(App* app);
