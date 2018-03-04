//
// Created by Timo Widdau on 27.10.17.
//

#include <stdlib.h>
#include <nfc/nfc.h>
#include <freefare.h>

int NFC_MAX_DEVICES = 1;

int main(int argc, char **argv) {
    int result;
    int _error = 0;

    nfc_device *device = NULL;
    FreefareTag *tags = NULL;
    FreefareTag tag = NULL;

    nfc_connstring devices[NFC_MAX_DEVICES];
    size_t device_count;

    nfc_context *context;
    nfc_init(&context);

    if (context == NULL) {
        printf("Unable to init libnfc (malloc)\n");
        exit(EXIT_FAILURE);
    }

    device_count = nfc_list_devices(context, devices, 8);
    if (device_count <= 0) {
        printf("No NFC device found.\n");
        exit(EXIT_FAILURE);
    }

    device = nfc_open(context, devices[0]);
    if (!device) {
        printf("nfc_open() failed.\n");
        exit(EXIT_FAILURE);
    }

    tags = freefare_get_tags(device);
    if (!tags) {
        nfc_close(device);
        printf("Error listing Mifare DESFire tags.\n");
        exit(EXIT_FAILURE);
    }

    if (!tags[0]) {
        nfc_close(device);
        printf("Error: No tag found.\n");
        exit(EXIT_FAILURE);
    }

    if (tags[0] && tags[1]) {
        nfc_close(device);
        printf("Error: More than tag found.\n");
        exit(EXIT_FAILURE);
    }

    tag = tags[0];

    enum freefare_tag_type type = freefare_get_tag_type(tag);
    if (type != MIFARE_DESFIRE) {
        nfc_close(device);
        printf("Error: RFID card is not a Mifare DESFire.\n");
        _error = EXIT_FAILURE; goto CLOSE;
    }

    result = mifare_desfire_connect(tag);
    if (result < 0) {
        printf("Can't connect to Mifare DESFire target.\n");
        _error = EXIT_FAILURE; goto CLOSE;
    }

    struct mifare_desfire_version_info info;
    result = mifare_desfire_get_version(tag, &info);
    if (result < 0) {
        printf("Error reading Mifare DESFire Version.\n");
        _error = EXIT_FAILURE;
        goto CLOSE;
    }

    if (info.software.version_major < 1) {
        printf("Found old DESFire card - cannot use this card.\n");
        _error = EXIT_FAILURE; goto CLOSE;
    }

    char *tag_uid = freefare_get_tag_uid(tag);
    printf("%s\n", tag_uid);


CLOSE:
    freefare_free_tag(tag);
    nfc_close(device);
    exit(_error);
}
