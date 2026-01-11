#include "send_page.h"
#include "../app/app_state.h"
#include "../app/app_view_ids.h"
#include "../ui/ui_strings.h"
#include "../ui/ui_helpers.h"
#include "../services/pcap_store.h"

static void send_build_lines(App* app) {
    if(!app) return;
    
    // Clear text buffer
    memset(app->send_page.send_text, 0, sizeof(app->send_page.send_text));
    app->send_page.send_line_count = 0;
    memset(app->send_page.send_lines, 0, sizeof(app->send_page.send_lines));

    uint32_t selected_count = pcap_store_count_selected(&app->pcaps);

    if(selected_count == 0) {
        snprintf(app->send_page.send_text, sizeof(app->send_page.send_text),
            "SEND FILES\n"
            "\n"
            "No files selected.\n"
            "\n"
            "Go to PCAP files and\n"
            "select one or more.\n"
            "\n"
            "UP/DOWN: scroll\n"
            "OK: submit (mock)\n"
            "BACK: return\n");
    } else {
        snprintf(app->send_page.send_text, sizeof(app->send_page.send_text),
            "SEND FILES\n"
            "\n"
            "Ready: %lu file(s)\n"
            "\n", (unsigned long)selected_count);

        for(uint32_t i = 0; i < app->pcaps.file_count; i++) {
            if(!app->pcaps.selected[i]) continue;
            strlcat(app->send_page.send_text, "- ", sizeof(app->send_page.send_text));
            strlcat(app->send_page.send_text, app->pcaps.files[i], sizeof(app->send_page.send_text));
            strlcat(app->send_page.send_text, "\n", sizeof(app->send_page.send_text));
        }

        strlcat(app->send_page.send_text, "\nServer:\n", sizeof(app->send_page.send_text));
        strlcat(app->send_page.send_text, (strlen(app->config.server) ? app->config.server : "(not set)"), sizeof(app->send_page.send_text));
        strlcat(app->send_page.send_text,
            "\n\nUP/DOWN: scroll\nOK: submit (mock)\nBACK: return\n",
            sizeof(app->send_page.send_text));
    }

    // build line index
    app->send_page.send_line_count = 0;
    app->send_page.send_lines[app->send_page.send_line_count++] = app->send_page.send_text;

    for(char* p = app->send_page.send_text; *p != '\0' && app->send_page.send_line_count < 256; p++) {
        if(*p == '\n') {
            app->send_page.send_lines[app->send_page.send_line_count++] = p + 1;
        }
    }

    // clamp scroll
    if(app->send_page.send_scroll >= app->send_page.send_line_count) {
        app->send_page.send_scroll = 0;
    }
}

static void send_view_draw_callback(Canvas* canvas, void* context) {
    App* app = context;
    if(!app || !canvas) return;

    canvas_clear(canvas);

    // Header
    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str(canvas, 2, 10, "Send files");

    // Ensure lines are built if not already
    if(app->send_page.send_line_count == 0) {
        send_build_lines(app);
    }

    // Body (scrollable)
    canvas_set_font(canvas, FontSecondary);

    uint16_t line = app->send_page.send_scroll;
    uint8_t y = SEND_BODY_Y;

    for(uint8_t i = 0; i < SEND_LINES_VISIBLE; i++) {
        if(line >= app->send_page.send_line_count) break;

        // draw until newline
        const char* s = app->send_page.send_lines[line];
        if(!s) break;
        
        char buf[128];
        size_t j = 0;
        while(s[j] != '\0' && s[j] != '\n' && j < sizeof(buf) - 1) {
            buf[j] = s[j];
            j++;
        }
        buf[j] = '\0';

        canvas_draw_str(canvas, 2, y, buf);

        y += SEND_LINE_H;
        line++;
    }

    // Soft hints at bottom
    canvas_set_font(canvas, FontSecondary);
    canvas_draw_str(canvas, 2, 62, "< Back      OK Submit");
}

static bool send_view_input_callback(InputEvent* event, void* context) {
    App* app = context;
    if(!app) return false;
    if(event->type != InputTypeShort) return false;

    if(event->key == InputKeyBack) {
        view_dispatcher_switch_to_view(app->view_dispatcher, ViewHome);
        return true;
    }

    if(event->key == InputKeyOk) {
        if(pcap_store_count_selected(&app->pcaps) == 0) {
            ui_show_info(app->info_dialog, app->view_dispatcher, ViewInfo, "Nothing to send", "Select PCAPs first.");
        } else {
            ui_show_info(app->info_dialog, app->view_dispatcher, ViewInfo, "Mock submit", "Later: send via ESP32 HTTP.");
        }
        return true;
    }

    if(event->key == InputKeyUp) {
        if(app->send_page.send_scroll > 0) app->send_page.send_scroll--;
        view_dispatcher_switch_to_view(app->view_dispatcher, ViewSend);
        return true;
    }

    if(event->key == InputKeyDown) {
        uint16_t max_scroll = (app->send_page.send_line_count > SEND_LINES_VISIBLE) ?
            (app->send_page.send_line_count - SEND_LINES_VISIBLE) : 0;
    
        if(app->send_page.send_scroll < max_scroll) app->send_page.send_scroll++;
        view_dispatcher_switch_to_view(app->view_dispatcher, ViewSend);
        return true;
    }

    return false;
}

static uint32_t send_view_prev_callback(void* context) {
    App* app = context;
    if(!app) return ViewHome;
    view_dispatcher_switch_to_view(app->view_dispatcher, ViewHome);
    return ViewHome;
}

void send_page_create(SendPage* page) {
    if(!page) return;
    memset(page, 0, sizeof(SendPage));
    page->view = view_alloc();
}

void send_page_destroy(SendPage* page) {
    if(!page) return;
    if(page->view) {
        view_free(page->view);
        page->view = NULL;
    }
}

View* send_page_get_view(SendPage* page) {
    if(!page) return NULL;
    return page->view;
}

void send_page_bind(SendPage* page, App* app) {
    if(!page || !app || !page->view) return;
    view_set_context(page->view, app);
    view_set_draw_callback(page->view, send_view_draw_callback);
    view_set_input_callback(page->view, send_view_input_callback);
    view_set_previous_callback(page->view, send_view_prev_callback);
}

void send_page_open(App* app) {
    if(!app || !app->send_page.view || !app->view_dispatcher) return;
    
    // Ensure PCAPs are loaded before showing send page
    pcap_store_load_preserve_selection(app->storage, &app->pcaps, PCAP_DIR);
    
    app->send_page.send_scroll = 0;
    send_build_lines(app);
    view_dispatcher_switch_to_view(app->view_dispatcher, ViewSend);
}
