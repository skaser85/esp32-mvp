#define NOB_IMPLEMENTATION
#define NOB_STRIP_PREFIX
#include "nob.h"
#include <stdlib.h>
#include <string.h>

int main(int argc, char **argv) {
    Cmd cmd = {0};

    // Default target project directory if the user provides no arguments
    const char *target_dir = "desk_tx"; 
    const char *action = "build";

    // Parse commands: e.g., "./nob monitor flash"
    if (argc > 1) {
        if (strcmp(argv[1], "desk") == 0) {
            target_dir = "desk_tx";
        } else if (strcmp(argv[1], "monitor") == 0) {
            target_dir = "monitor_rx";
        } else if (strcmp(argv[1], "clean") == 0) {
            action = "clean";
        }
    }
    
    if (argc > 2) {
        action = argv[2]; // Capture "flash" or "monitor" instructions
    }

    if (strcmp(action, "clean") == 0) {
        nob_log(NOB_INFO, "--- Cleaning Build Outputs ---");
        cmd_append(&cmd, "idf.py", "-C", "desk_tx", "fullclean");
        cmd_run_sync(cmd);
        cmd.count = 0;
        cmd_append(&cmd, "idf.py", "-C", "monitor_rx", "fullclean");
        return cmd_run_sync(cmd) ? 0 : 1;
    }

    // Execute build commands targeted directly inside the selected folder path
    if (strcmp(action, "flash") == 0) {
        nob_log(NOB_INFO, "--- Flashing Target [ %s ] ---", target_dir);
        cmd_append(&cmd, "idf.py", "-C", target_dir, "flash");
        if (!cmd_run_sync(cmd)) return 1;
        
        cmd.count = 0;
        cmd_append(&cmd, "idf.py", "-C", target_dir, "monitor");
        return cmd_run_sync(cmd) ? 0 : 1;
    } else {
        nob_log(NOB_INFO, "--- Compiling Target Binary [ %s ] ---", target_dir);
        cmd_append(&cmd, "idf.py", "-C", target_dir, "build");
        return cmd_run_sync(cmd) ? 0 : 1;
    }
}

