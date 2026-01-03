/* In-Memory File System
 * Simple hierarchical file system for BFOS
 */

#include "kernel.h"

#define MAX_FILES 256
#define MAX_DIRS 64
#define MAX_FILE_SIZE 8192

/* Root directory */
static fs_entry* fs_root = 0;
static fs_entry fs_entries[MAX_FILES + MAX_DIRS];
static size_t fs_entry_count = 0;
static char fs_file_data[MAX_FILES][MAX_FILE_SIZE];
static size_t fs_file_data_used = 0;

/* Current working directory */
static fs_entry* fs_cwd = 0;

/* Initialize file system */
void fs_initialize(void) {
    fs_entry_count = 0;
    fs_file_data_used = 0;
    
    /* Create root directory */
    fs_root = &fs_entries[fs_entry_count++];
    fs_root->name[0] = '/';
    fs_root->name[1] = '\0';
    fs_root->type = FS_TYPE_DIR;
    fs_root->size = 0;
    fs_root->data = 0;
    fs_root->parent = 0;
    fs_root->next = 0;
    
    fs_cwd = fs_root;
}

/* Find entry in directory */
static fs_entry* fs_find_entry(fs_entry* dir, const char* name) {
    if (dir->type != FS_TYPE_DIR) {
        return 0;
    }
    
    fs_entry* child = (fs_entry*)dir->data;
    while (child) {
        size_t i = 0;
        int match = 1;
        while (name[i] != '\0' && child->name[i] != '\0') {
            if (name[i] != child->name[i]) {
                match = 0;
                break;
            }
            i++;
        }
        if (match && name[i] == '\0' && child->name[i] == '\0') {
            return child;
        }
        child = child->next;
    }
    return 0;
}

/* Add entry to directory */
static void fs_add_entry(fs_entry* dir, fs_entry* entry) {
    entry->parent = dir;
    entry->next = (fs_entry*)dir->data;
    dir->data = (char*)entry;
}

/* Create directory */
fs_entry* fs_mkdir(const char* name) {
    if (fs_entry_count >= MAX_FILES + MAX_DIRS) {
        return 0;
    }
    
    /* Check if already exists */
    if (fs_find_entry(fs_cwd, name)) {
        return 0;
    }
    
    fs_entry* dir = &fs_entries[fs_entry_count++];
    size_t i = 0;
    while (name[i] != '\0' && i < MAX_FILENAME - 1) {
        dir->name[i] = name[i];
        i++;
    }
    dir->name[i] = '\0';
    dir->type = FS_TYPE_DIR;
    dir->size = 0;
    dir->data = 0;
    dir->next = 0;
    
    fs_add_entry(fs_cwd, dir);
    return dir;
}

/* Create file with content */
fs_entry* fs_create_file(const char* name, const char* content) {
    if (fs_entry_count >= MAX_FILES + MAX_DIRS) {
        return 0;
    }
    
    if (fs_file_data_used >= MAX_FILES) {
        return 0;
    }
    
    /* Check if already exists */
    if (fs_find_entry(fs_cwd, name)) {
        return 0;
    }
    
    fs_entry* file = &fs_entries[fs_entry_count++];
    size_t i = 0;
    while (name[i] != '\0' && i < MAX_FILENAME - 1) {
        file->name[i] = name[i];
        i++;
    }
    file->name[i] = '\0';
    file->type = FS_TYPE_FILE;
    
    /* Copy content */
    char* file_data = fs_file_data[fs_file_data_used++];
    size_t len = 0;
    if (content) {
        /* For binary data, we need to find the actual size */
        /* Try to determine size - if it's a C string literal with escapes, use strlen */
        /* Otherwise, copy until null or max size */
        while (len < MAX_FILE_SIZE - 1) {
            /* Check for null byte, but also handle escaped sequences */
            if (content[len] == '\0') {
                /* Check if this is an escaped null (\x00) */
                if (len >= 4 && content[len-4] == '\\' && content[len-3] == 'x' && 
                    content[len-2] == '0' && content[len-1] == '0') {
                    /* This is \x00, treat as binary zero */
                    file_data[len-4] = 0;
                    len = len - 3; /* Continue after the escape sequence */
                    continue;
                } else {
                    break; /* Real null terminator */
                }
            }
            file_data[len] = content[len];
            len++;
        }
    }
    file_data[len] = '\0';
    file->size = len;
    file->data = file_data;
    file->next = 0;
    
    fs_add_entry(fs_cwd, file);
    return file;
}

/* Create file with binary content (size specified) */
fs_entry* fs_create_file_binary(const char* name, const uint8_t* content, size_t content_size) {
    if (fs_entry_count >= MAX_FILES + MAX_DIRS) {
        return 0;
    }
    
    if (fs_file_data_used >= MAX_FILES) {
        return 0;
    }
    
    /* Check if already exists */
    if (fs_find_entry(fs_cwd, name)) {
        return 0;
    }
    
    fs_entry* file = &fs_entries[fs_entry_count++];
    size_t i = 0;
    while (name[i] != '\0' && i < MAX_FILENAME - 1) {
        file->name[i] = name[i];
        i++;
    }
    file->name[i] = '\0';
    file->type = FS_TYPE_FILE;
    
    /* Copy binary content */
    char* file_data = fs_file_data[fs_file_data_used++];
    size_t len = content_size;
    if (len > MAX_FILE_SIZE - 1) {
        len = MAX_FILE_SIZE - 1;
    }
    
    if (content) {
        for (size_t j = 0; j < len; j++) {
            file_data[j] = content[j];
        }
    }
    file->size = len;
    file->data = file_data;
    file->next = 0;
    
    fs_add_entry(fs_cwd, file);
    return file;
}

/* Change directory */
int fs_chdir(const char* path) {
    if (path[0] == '/') {
        fs_cwd = fs_root;
        path++;
    }
    
    if (path[0] == '\0') {
        return 0;
    }
    
    /* Parse path */
    char component[MAX_FILENAME];
    size_t comp_idx = 0;
    size_t path_idx = 0;
    
    while (path[path_idx] != '\0') {
        if (path[path_idx] == '/') {
            if (comp_idx > 0) {
                component[comp_idx] = '\0';
                
                /* Handle special directories */
                if (component[0] == '.' && component[1] == '.' && component[2] == '\0') {
                    /* Go to parent directory */
                    if (fs_cwd->parent) {
                        fs_cwd = fs_cwd->parent;
                    }
                } else if (component[0] == '.' && component[1] == '\0') {
                    /* Current directory - do nothing */
                } else {
                fs_entry* entry = fs_find_entry(fs_cwd, component);
                if (!entry || entry->type != FS_TYPE_DIR) {
                    return -1;
                }
                fs_cwd = entry;
                }
                comp_idx = 0;
            }
        } else {
            if (comp_idx < MAX_FILENAME - 1) {
                component[comp_idx++] = path[path_idx];
            }
        }
        path_idx++;
    }
    
    if (comp_idx > 0) {
        component[comp_idx] = '\0';
        
        /* Handle special directories */
        if (component[0] == '.' && component[1] == '.' && component[2] == '\0') {
            /* Go to parent directory */
            if (fs_cwd->parent) {
                fs_cwd = fs_cwd->parent;
            }
        } else if (component[0] == '.' && component[1] == '\0') {
            /* Current directory - do nothing */
        } else {
        fs_entry* entry = fs_find_entry(fs_cwd, component);
        if (!entry || entry->type != FS_TYPE_DIR) {
            return -1;
        }
        fs_cwd = entry;
        }
    }
    
    return 0;
}

/* Get current directory path */
void fs_get_cwd(char* path, size_t max_len) {
    if (fs_cwd == fs_root) {
        path[0] = '/';
        path[1] = '\0';
        return;
    }
    
    /* Build path by traversing up */
    char temp_path[MAX_PATH];
    size_t temp_len = 0;
    fs_entry* current = fs_cwd;
    
    while (current && current != fs_root) {
        size_t name_len = 0;
        while (current->name[name_len] != '\0') {
            name_len++;
        }
        
        if (temp_len + name_len + 1 < MAX_PATH) {
            /* Move existing path */
            for (size_t i = temp_len; i > 0; i--) {
                temp_path[i + name_len] = temp_path[i - 1];
            }
            /* Insert name */
            for (size_t i = 0; i < name_len; i++) {
                temp_path[i] = current->name[i];
            }
            temp_path[name_len] = '/';
            temp_len += name_len + 1;
        }
        current = current->parent;
    }
    
    temp_path[temp_len] = '\0';
    
    /* Copy to output */
    size_t i = 0;
    while (temp_path[i] != '\0' && i < max_len - 1) {
        path[i] = temp_path[i];
        i++;
    }
    path[i] = '\0';
}

/* Find file by path */
fs_entry* fs_find_file(const char* path) {
    fs_entry* start_dir = fs_cwd;
    
    if (path[0] == '/') {
        start_dir = fs_root;
        path++;
    }
    
    if (path[0] == '\0') {
        return start_dir;
    }
    
    /* Parse path */
    char component[MAX_FILENAME];
    size_t comp_idx = 0;
    size_t path_idx = 0;
    fs_entry* current = start_dir;
    
    while (path[path_idx] != '\0') {
        if (path[path_idx] == '/') {
            if (comp_idx > 0) {
                component[comp_idx] = '\0';
                fs_entry* entry = fs_find_entry(current, component);
                if (!entry) {
                    return 0;
                }
                if (entry->type == FS_TYPE_DIR) {
                    current = entry;
                } else {
                    /* Last component, return file */
                    return entry;
                }
                comp_idx = 0;
            }
        } else {
            if (comp_idx < MAX_FILENAME - 1) {
                component[comp_idx++] = path[path_idx];
            }
        }
        path_idx++;
    }
    
    if (comp_idx > 0) {
        component[comp_idx] = '\0';
        return fs_find_entry(current, component);
    }
    
    return current;
}

/* List directory contents */
void fs_list_dir(fs_entry* dir, void (*callback)(const char* name, uint8_t type)) {
    if (!dir || dir->type != FS_TYPE_DIR) {
        return;
    }
    
    fs_entry* child = (fs_entry*)dir->data;
    while (child) {
        callback(child->name, child->type);
        child = child->next;
    }
}

/* Get current working directory */
fs_entry* fs_get_cwd_entry(void) {
    return fs_cwd;
}

