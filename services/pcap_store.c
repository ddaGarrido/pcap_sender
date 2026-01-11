
#include "pcap_store.h"
#include "../ui/ui_helpers.h"

void pcap_store_clear(PcapStore* ps) {
    if(!ps) return;
    ps->file_count = 0;
    for(uint32_t i = 0; i < PCAP_MAX_FILES; i++) {
        ps->files[i][0] = '\0';
        ps->labels[i][0] = '\0';
        ps->selected[i] = false;
    }
}

bool pcap_store_is_pcap_like(const char* name) {
    if(!name) return false;
    const char* dot = strrchr(name, '.');
    if(!dot) return false;
    return (strcmp(dot, ".pcap") == 0) || (strcmp(dot, ".pcapng") == 0) || (strcmp(dot, ".cap") == 0);
}

void pcap_store_snapshot_selected(PcapStore* ps) {
    if(!ps) return;
    ps->selected_snapshot_count = 0;

    for(uint32_t i = 0; i < ps->file_count; i++) {
        if(ps->selected[i]) {
            strncpy(ps->selected_snapshot[ps->selected_snapshot_count], ps->files[i], PCAP_MAX_NAME_LEN - 1);
            ps->selected_snapshot[ps->selected_snapshot_count][PCAP_MAX_NAME_LEN - 1] = '\0';
            ps->selected_snapshot_count++;
            if(ps->selected_snapshot_count >= PCAP_MAX_FILES) break;
        }
    }
}

bool pcap_store_snapshot_contains(PcapStore* ps, const char* name) {
    if(!ps || !name) return false;
    for(uint32_t i = 0; i < ps->selected_snapshot_count; i++) {
        if(strcmp(ps->selected_snapshot[i], name) == 0) return true;
    }
    return false;
}

void pcap_store_load_preserve_selection(Storage* storage, PcapStore* ps, const char* folder) {
    if(!storage || !ps || !folder) return;

    pcap_store_snapshot_selected(ps);
    pcap_store_clear(ps);

    if(!ui_dir_exists(storage, folder)) return;

    File* dir = storage_file_alloc(storage);
    if(!storage_dir_open(dir, folder)) {
        storage_file_free(dir);
        return;
    }

    FileInfo info;
    char name[PCAP_MAX_NAME_LEN];

    while(storage_dir_read(dir, &info, name, sizeof(name))) {
        if(info.flags & FSF_DIRECTORY) continue;
        if(!pcap_store_is_pcap_like(name)) continue;
        if(ps->file_count >= PCAP_MAX_FILES) break;

        strncpy(ps->files[ps->file_count], name, PCAP_MAX_NAME_LEN - 1);
        ps->files[ps->file_count][PCAP_MAX_NAME_LEN - 1] = '\0';

        ps->selected[ps->file_count] = pcap_store_snapshot_contains(ps, name);

        ps->file_count++;
    }

    storage_dir_close(dir);
    storage_file_free(dir);
}

uint32_t pcap_store_count_selected(const PcapStore* ps) {
    if(!ps) return 0;
    uint32_t c = 0;
    for(uint32_t i = 0; i < ps->file_count; i++) {
        if(ps->selected[i]) c++;
    }
    return c;
}

void pcap_store_build_labels(PcapStore* ps) {
    if(!ps) return;
    for(uint32_t i = 0; i < ps->file_count; i++) {
        snprintf(ps->labels[i], PCAP_LABEL_LEN, "[%c] %s", ps->selected[i] ? 'x' : ' ', ps->files[i]);
    }
}
