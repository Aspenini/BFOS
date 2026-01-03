/* Command Line Shell
 * Interactive shell for BFOS
 */

#include "kernel.h"

#define MAX_LINE_LENGTH 256
#define MAX_ARGS 16

/* VGA entry helper */
static inline uint16_t vga_entry(unsigned char uc, uint8_t color) {
    return (uint16_t) uc | (uint16_t) color << 8;
}

/* Command buffer */
static char command_buffer[MAX_LINE_LENGTH];
static size_t command_pos = 0;

/* Parse command line into arguments */
static size_t parse_args(char* line, char* args[], size_t max_args) {
    size_t arg_count = 0;
    size_t i = 0;
    int in_arg = 0;
    
    while (line[i] != '\0' && line[i] != '\n' && arg_count < max_args) {
        if (line[i] == ' ' || line[i] == '\t') {
            if (in_arg) {
                line[i] = '\0';
                in_arg = 0;
            }
        } else {
            if (!in_arg) {
                args[arg_count++] = (char*)&line[i];
                in_arg = 1;
            }
        }
        i++;
    }
    
    if (in_arg && arg_count < max_args) {
        line[i] = '\0';
    }
    
    return arg_count;
}

/* Find command in system path */
static fs_entry* find_command(const char* cmd_name) {
    /* First check sys/components/ */
    char sys_path[MAX_PATH];
    size_t i = 0;
    sys_path[i++] = '/';
    sys_path[i++] = 's';
    sys_path[i++] = 'y';
    sys_path[i++] = 's';
    sys_path[i++] = '/';
    sys_path[i++] = 'c';
    sys_path[i++] = 'o';
    sys_path[i++] = 'm';
    sys_path[i++] = 'p';
    sys_path[i++] = 'o';
    sys_path[i++] = 'n';
    sys_path[i++] = 'e';
    sys_path[i++] = 'n';
    sys_path[i++] = 't';
    sys_path[i++] = 's';
    sys_path[i++] = '/';
    
    size_t j = 0;
    while (cmd_name[j] != '\0' && i < MAX_PATH - 5) {
        sys_path[i++] = cmd_name[j++];
    }
    sys_path[i++] = '.';
    sys_path[i++] = 'b';
    sys_path[i++] = 'f';
    sys_path[i] = '\0';
    
    fs_entry* entry = fs_find_file(sys_path);
    if (entry && entry->type == FS_TYPE_FILE) {
        return entry;
    }
    
    /* Check current directory */
    char cwd_path[MAX_PATH];
    fs_get_cwd(cwd_path, MAX_PATH);
    
    if (cwd_path[0] != '\0' && cwd_path[0] != '/') {
        return 0;
    }
    
    i = 0;
    while (cwd_path[i] != '\0') {
        sys_path[i] = cwd_path[i];
        i++;
    }
    if (i > 0 && sys_path[i-1] != '/') {
        sys_path[i++] = '/';
    }
    
    j = 0;
    while (cmd_name[j] != '\0' && i < MAX_PATH - 5) {
        sys_path[i++] = cmd_name[j++];
    }
    sys_path[i++] = '.';
    sys_path[i++] = 'b';
    sys_path[i++] = 'f';
    sys_path[i] = '\0';
    
    entry = fs_find_file(sys_path);
    if (entry && entry->type == FS_TYPE_FILE) {
        return entry;
    }
    
    return 0;
}

/* Execute brainfuck command with arguments */
static void execute_bf_command(fs_entry* file, char* args[] __attribute__((unused)), size_t arg_count __attribute__((unused))) {
    if (!file || file->type != FS_TYPE_FILE) {
        return;
    }
    
    /* For now, just execute the brainfuck file */
    /* TODO: Pass arguments to brainfuck program via system calls */
    bf_load_and_run(file->data);
}

/* Forward declarations */
static void ls_callback(const char* name, uint8_t type);

/* Handle cd command */
static void handle_cd(char* args[], size_t arg_count) {
    if (arg_count < 2) {
        terminal_setcolor(vga_entry(COLOR_LIGHT_RED, COLOR_BLACK));
        terminal_writestring("cd: missing argument\n");
        return;
    }
    
    if (fs_chdir(args[1]) != 0) {
        terminal_setcolor(vga_entry(COLOR_LIGHT_RED, COLOR_BLACK));
        terminal_writestring("cd: directory not found\n");
    }
}

/* Handle ls command */
static void handle_ls(char* args[], size_t arg_count) {
    fs_entry* dir = fs_get_cwd_entry();
    
    if (arg_count >= 2) {
        fs_entry* target = fs_find_file(args[1]);
        if (target && target->type == FS_TYPE_DIR) {
            dir = target;
        } else if (target) {
            terminal_setcolor(vga_entry(COLOR_LIGHT_RED, COLOR_BLACK));
            terminal_writestring("ls: not a directory\n");
            return;
        } else {
            terminal_setcolor(vga_entry(COLOR_LIGHT_RED, COLOR_BLACK));
            terminal_writestring("ls: directory not found\n");
            return;
        }
    }
    
    fs_list_dir(dir, ls_callback);
    terminal_putchar('\n');
}

/* List directory callback */
static void ls_callback(const char* name, uint8_t type) {
    if (type == FS_TYPE_DIR) {
        terminal_setcolor(vga_entry(COLOR_LIGHT_BLUE, COLOR_BLACK));
    } else {
        terminal_setcolor(vga_entry(COLOR_LIGHT_GREY, COLOR_BLACK));
    }
    terminal_writestring(name);
    if (type == FS_TYPE_DIR) {
        terminal_writestring("/");
    }
    terminal_writestring("  ");
}

/* Handle run command */
static void handle_run(char* args[], size_t arg_count) {
    if (arg_count < 2) {
        terminal_setcolor(vga_entry(COLOR_LIGHT_RED, COLOR_BLACK));
        terminal_writestring("run: missing argument\n");
        return;
    }
    
    fs_entry* file = fs_find_file(args[1]);
    if (!file || file->type != FS_TYPE_FILE) {
        terminal_setcolor(vga_entry(COLOR_LIGHT_RED, COLOR_BLACK));
        terminal_writestring("run: file not found\n");
        return;
    }
    
    char* bf_args[MAX_ARGS];
    size_t bf_arg_count = arg_count - 1;
    for (size_t i = 0; i < bf_arg_count; i++) {
        bf_args[i] = args[i + 1];
    }
    
    execute_bf_command(file, bf_args, bf_arg_count);
    terminal_putchar('\n');
}

/* Handle txt command - display text file contents */
static void handle_txt(char* args[], size_t arg_count) {
    if (arg_count < 2) {
        terminal_setcolor(vga_entry(COLOR_LIGHT_RED, COLOR_BLACK));
        terminal_writestring("txt: missing argument\n");
        return;
    }
    
    fs_entry* file = fs_find_file(args[1]);
    if (!file) {
        terminal_setcolor(vga_entry(COLOR_LIGHT_RED, COLOR_BLACK));
        terminal_writestring("txt: file not found\n");
        return;
    }
    
    if (file->type != FS_TYPE_FILE) {
        terminal_setcolor(vga_entry(COLOR_LIGHT_RED, COLOR_BLACK));
        terminal_writestring("txt: not a file\n");
        return;
    }
    
    /* Display file contents */
    terminal_setcolor(vga_entry(COLOR_LIGHT_GREY, COLOR_BLACK));
    if (file->data && file->size > 0) {
        for (size_t i = 0; i < file->size; i++) {
            char c = file->data[i];
            if (c == '\0') {
                break; /* Null terminator */
            }
            terminal_putchar(c);
        }
    }
    terminal_putchar('\n');
}

/* Handle clear command - clear terminal screen */
static void handle_clear(char* args[] __attribute__((unused)), size_t arg_count __attribute__((unused))) {
    terminal_clear();
}

/* Handle play command - interactive brainfuck session */
static void handle_play(char* args[] __attribute__((unused)), size_t arg_count __attribute__((unused))) {
    terminal_setcolor(vga_entry(COLOR_LIGHT_CYAN, COLOR_BLACK));
    terminal_writestring("Brainfuck Play Session\n");
    terminal_writestring("Type brainfuck code and press Enter to run.\n");
    terminal_writestring("Press Ctrl+Q to exit.\n\n");
    
    char bf_code[MAX_LINE_LENGTH];
    int running = 1;
    
    while (running) {
        /* Show prompt */
        terminal_setcolor(vga_entry(COLOR_LIGHT_GREEN, COLOR_BLACK));
        terminal_writestring("playsession> ");
        terminal_setcolor(vga_entry(COLOR_LIGHT_GREY, COLOR_BLACK));
        
        /* Read line */
        size_t pos = 0;
        bf_code[0] = '\0';
        
        while (pos < MAX_LINE_LENGTH - 1) {
            int c = keyboard_getchar();
            
            if (c == -1) {
                continue;
            }
            
            /* Check for Ctrl+Q (0x11) */
            if (c == 0x11) {
                terminal_putchar('\n');
                terminal_setcolor(vga_entry(COLOR_LIGHT_CYAN, COLOR_BLACK));
                terminal_writestring("Exiting play session.\n");
                terminal_setcolor(vga_entry(COLOR_LIGHT_GREY, COLOR_BLACK));
                running = 0;
                break;
            }
            
            /* Handle backspace */
            if (c == '\b' || c == 127) {
                if (pos > 0) {
                    pos--;
                    bf_code[pos] = '\0';
                    terminal_putchar('\b');
                    terminal_putchar(' ');
                    terminal_putchar('\b');
                }
                continue;
            }
            
            /* Handle enter */
            if (c == '\n' || c == '\r') {
                terminal_putchar('\n');
                bf_code[pos] = '\0';
                
                /* Execute brainfuck code if not empty */
                if (pos > 0) {
                    terminal_setcolor(vga_entry(COLOR_LIGHT_GREY, COLOR_BLACK));
                    bf_reset();
                    bf_execute(bf_code);
                    terminal_putchar('\n');
                }
                
                break;
            }
            
            /* Regular character */
            if (c >= 32 && c < 127) {
                bf_code[pos] = c;
                pos++;
                bf_code[pos] = '\0';
                terminal_putchar(c);
            }
        }
    }
}

/* Execute command */
static void execute_command(const char* line) {
    char line_copy[MAX_LINE_LENGTH];
    size_t i = 0;
    while (line[i] != '\0' && i < MAX_LINE_LENGTH - 1) {
        line_copy[i] = line[i];
        i++;
    }
    line_copy[i] = '\0';
    
    char* args[MAX_ARGS];
    size_t arg_count = parse_args(line_copy, args, MAX_ARGS);
    
    if (arg_count == 0) {
        return;
    }
    
    /* Built-in commands */
    size_t cmd_len = 0;
    while (args[0][cmd_len] != '\0') {
        cmd_len++;
    }
    
    if (cmd_len == 2 && args[0][0] == 'c' && args[0][1] == 'd') {
        handle_cd(args, arg_count);
        return;
    }
    
    if (cmd_len == 2 && args[0][0] == 'l' && args[0][1] == 's') {
        handle_ls(args, arg_count);
        return;
    }
    
    if (cmd_len == 3 && args[0][0] == 'r' && args[0][1] == 'u' && args[0][2] == 'n') {
        handle_run(args, arg_count);
        return;
    }
    
    if (cmd_len == 3 && args[0][0] == 't' && args[0][1] == 'x' && args[0][2] == 't') {
        handle_txt(args, arg_count);
        return;
    }
    
    if (cmd_len == 5 && args[0][0] == 'c' && args[0][1] == 'l' && args[0][2] == 'e' && 
        args[0][3] == 'a' && args[0][4] == 'r') {
        handle_clear(args, arg_count);
        return;
    }
    
    if (cmd_len == 4 && args[0][0] == 'p' && args[0][1] == 'l' && args[0][2] == 'a' && 
        args[0][3] == 'y') {
        handle_play(args, arg_count);
        return;
    }
    
    /* Try to find as brainfuck command */
    fs_entry* cmd_file = find_command(args[0]);
    if (cmd_file) {
        execute_bf_command(cmd_file, args, arg_count);
        terminal_putchar('\n');
        return;
    }
    
    terminal_setcolor(vga_entry(COLOR_LIGHT_RED, COLOR_BLACK));
    terminal_writestring("Command not found: ");
    terminal_writestring(args[0]);
    terminal_putchar('\n');
}

/* Read line from keyboard */
static void read_line(char* buffer, size_t max_len) {
    command_pos = 0;
    buffer[0] = '\0';
    
    /* Show cursor at prompt position */
    terminal_show_cursor();
    
    while (1) {
        /* Aggressively poll keyboard - call multiple times */
        keyboard_handle_interrupt();
        keyboard_handle_interrupt(); /* Poll twice to catch rapid keystrokes */
        int c = keyboard_getchar();
        if (c == -1) {
            /* Update cursor while waiting */
            terminal_update_cursor();
            /* Small delay to give CPU time */
            for (volatile int i = 0; i < 5000; i++);
            /* Don't use HLT - it might prevent keyboard polling on some systems */
            continue;
        }
        
        if (c == '\n' || c == '\r') {
            terminal_hide_cursor();
            terminal_putchar('\n');
            buffer[command_pos] = '\0';
            return;
        }
        
        if (c == '\b' || c == 127) {
            if (command_pos > 0) {
                command_pos--;
                buffer[command_pos] = '\0';
                /* Move cursor back */
                size_t col = terminal_get_column();
                if (col > 0) {
                    terminal_set_position(col - 1, terminal_get_row());
                    terminal_putchar(' ');
                    terminal_set_position(col - 1, terminal_get_row());
                } else {
                    terminal_putchar('\b');
                    terminal_putchar(' ');
                    terminal_putchar('\b');
                }
                terminal_update_cursor();
            }
            continue;
        }
        
        if (c >= 32 && c < 127 && command_pos < max_len - 1) {
            buffer[command_pos++] = (char)c;
            buffer[command_pos] = '\0';
            terminal_putchar((char)c);
            terminal_update_cursor();
        }
    }
}

/* Print prompt */
static void print_prompt(void) {
    char cwd[MAX_PATH];
    fs_get_cwd(cwd, MAX_PATH);
    
    terminal_setcolor(vga_entry(COLOR_LIGHT_GREEN, COLOR_BLACK));
    terminal_writestring("bfos");
    terminal_setcolor(vga_entry(COLOR_LIGHT_GREY, COLOR_BLACK));
    terminal_writestring("@");
    terminal_setcolor(vga_entry(COLOR_LIGHT_CYAN, COLOR_BLACK));
    terminal_writestring(cwd);
    terminal_setcolor(vga_entry(COLOR_LIGHT_GREY, COLOR_BLACK));
    terminal_writestring("$ ");
    
    /* Cursor will be shown by read_line */
}

/* Shell main loop */
void shell_main(void) {
    while (1) {
        print_prompt();
        read_line(command_buffer, MAX_LINE_LENGTH);
        
        if (command_buffer[0] != '\0') {
            execute_command(command_buffer);
        }
    }
}

