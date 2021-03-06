/*
   Copyright (c) 2018, The LineageOS Project

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are
   met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above
      copyright notice, this list of conditions and the following
      disclaimer in the documentation and/or other materials provided
      with the distribution.
    * Neither the name of The Linux Foundation nor the names of its
      contributors may be used to endorse or promote products derived
      from this software without specific prior written permission.

   THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED
   WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
   MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT
   ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
   BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
   CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
   SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
   BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
   WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
   OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
   IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <fcntl.h>
#include <stdlib.h>

#include "vendor_init.h"
#include "property_service.h"
#include "log.h"
#include "util.h"

static char board_id[PROP_VALUE_MAX];

static int read_file2(const char *fname, char *data, int max_size)
{
    int fd, rc;

    if (max_size < 1)
        return 0;

    fd = open(fname, O_RDONLY);
    if (fd < 0) {
        ERROR("failed to open '%s'\n", fname);
        return 0;
    }

    rc = read(fd, data, max_size - 1);
    if ((rc > 0) && (rc < max_size))
        data[rc] = '\0';
    else
        data[0] = '\0';
    close(fd);

    return 1;
}

static void import_cmdline(char *name, int for_emulator)
{
    char *value = strchr(name, '=');
    int name_len = strlen(name);
    const char s[2] = ":";

    if (value == 0) return;
    *value++ = 0;
    if (name_len == 0) return;

    if (!strcmp(name, "board_id")) {
        value = strtok(value, s);
        strlcpy(board_id, value, sizeof(board_id));
    }
}

static void init_alarm_boot_properties()
{
    char const *alarm_file = "/proc/sys/kernel/boot_reason";
    char buf[64];
    char tmp[PROP_VALUE_MAX];

    property_get("ro.boot.alarmboot", tmp);

    if (read_file2(alarm_file, buf, sizeof(buf))) {
        /*
         * Setup ro.alarm_boot value to true when it is RTC triggered boot up
         * For existing PMIC chips, the following mapping applies
         * for the value of boot_reason:
         *
         * 0 -> unknown
         * 1 -> hard reset
         * 2 -> sudden momentary power loss (SMPL)
         * 3 -> real time clock (RTC)
         * 4 -> DC charger inserted
         * 5 -> USB charger insertd
         * 6 -> PON1 pin toggled (for secondary PMICs)
         * 7 -> CBLPWR_N pin toggled (for external power supply)
         * 8 -> KPDPWR_N pin toggled (power key pressed)
         */
        if (buf[0] == '3' || !strcmp(tmp,"true"))
            property_set("ro.alarm_boot", "true");
        else
            property_set("ro.alarm_boot", "false");
    }
}

void vendor_load_properties()
{
    char device[PROP_VALUE_MAX];
    int rc;

    rc = property_get("ro.cm.device", device);
    if (!rc || strncmp(device, "santoni", PROP_VALUE_MAX))
        return;

    import_kernel_cmdline(0, import_cmdline);

    property_set("ro.product.wt.boardid", board_id);
    ERROR("Detected board ID=%s\n", board_id);

    if (!strcmp(board_id, "S88536AA2")) {
        property_set("ro.build.display.wtid", "SW_S88536AA2_V028_M11_XM_A13N_USR_TEST");
    } else if (!strcmp(board_id, "S88536BA2")) {
        property_set("ro.build.display.wtid", "SW_S88536BA2_V028_M11_XM_A13N_USR_TEST");
    } else if (!strcmp(board_id, "S88536CA2")) {
        property_set("ro.build.display.wtid", "SW_S88536CA2_V028_M11_XM_A13N_USR_TEST");
    }

    if (!strcmp(board_id, "S88536CA2")) {
        property_set("ro.product.model", "Redmi 4");
    } else {
        property_set("ro.product.model", "Redmi 4X");
    }

    init_alarm_boot_properties();
}
