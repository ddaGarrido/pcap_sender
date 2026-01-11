#pragma once
#include <storage/storage.h>
#include <gui/modules/dialog_ex.h>
#include <gui/view_dispatcher.h>

static inline bool ui_dir_exists(Storage* storage, const char* path) {
    FileInfo info;
    if(storage_common_stat(storage, path, &info) != FSE_OK) return false;
    return (info.flags & FSF_DIRECTORY);
}

static inline void ui_trim_newline(char* s) {
    if(!s) return;
    size_t n = strlen(s);
    while(n > 0 && (s[n - 1] == '\n' || s[n - 1] == '\r')) {
        s[n - 1] = '\0';
        n--;
    }
}

static inline void ui_show_info(
    DialogEx* dialog,
    ViewDispatcher* dispatcher,
    uint32_t info_view_id,
    const char* header,
    const char* text) {

    dialog_ex_reset(dialog);
    dialog_ex_set_header(dialog, header, 64, 10, AlignCenter, AlignTop);
    dialog_ex_set_text(dialog, text, 64, 32, AlignCenter, AlignCenter);
    dialog_ex_set_left_button_text(dialog, NULL);
    dialog_ex_set_center_button_text(dialog, "OK");
    dialog_ex_set_right_button_text(dialog, NULL);

    view_dispatcher_switch_to_view(dispatcher, info_view_id);
}
