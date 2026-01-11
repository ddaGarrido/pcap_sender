#include "app/app.h"

int32_t pcap_sender_app(void* p) {
    (void)p;
    App* app = app_alloc();
    app_run(app);
    app_free(app);
    return 0;
}
