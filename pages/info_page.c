#include "info_page.h"
#include "../app/app_state.h"
#include "../app/app_view_ids.h"

static bool info_dialog_input_callback(InputEvent* event, void* context) {
    App* app = context;

    if(event->type == InputTypeShort) {
        if(event->key == InputKeyOk || event->key == InputKeyBack) {
            view_dispatcher_switch_to_view(app->view_dispatcher, ViewHome);
            return true;
        }
    }
    return false;
}

void info_page_init(DialogEx* dialog, App* app) {
    View* info_view = dialog_ex_get_view(dialog);
    view_set_context(info_view, app);
    view_set_input_callback(info_view, info_dialog_input_callback);
}
