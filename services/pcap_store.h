#pragma once
#include <storage/storage.h>
#include <stdbool.h>

#define PCAP_MAX_FILES 64
#define PCAP_MAX_NAME_LEN 96
#define PCAP_LABEL_LEN 128

typedef struct {
    char files[PCAP_MAX_FILES][PCAP_MAX_NAME_LEN];
    bool selected[PCAP_MAX_FILES];
    uint32_t file_count;

    char labels[PCAP_MAX_FILES][PCAP_LABEL_LEN];

    // snapshot used to preserve selection between rescans
    char selected_snapshot[PCAP_MAX_FILES][PCAP_MAX_NAME_LEN];
    uint32_t selected_snapshot_count;
} PcapStore;

void pcap_store_clear(PcapStore* ps);
void pcap_store_snapshot_selected(PcapStore* ps);
bool pcap_store_snapshot_contains(PcapStore* ps, const char* name);

bool pcap_store_is_pcap_like(const char* name);
void pcap_store_load_preserve_selection(Storage* storage, PcapStore* ps, const char* folder);

uint32_t pcap_store_count_selected(const PcapStore* ps);
void pcap_store_build_labels(PcapStore* ps);
