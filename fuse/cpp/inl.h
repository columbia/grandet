#ifndef __INL__
#define __INL__

#include <map>
#include <string>

#include "list.h"
#include "grandet.h"

typedef struct file_entry_s *file_entry_t;
typedef struct file_entry_s {
    int rc;
    file_entry_t parent;
    list_entry_s c_node;
    list_entry_s u_node;
    
    std::string name;

    struct stat  st;
    std::map<std::string, std::string> xattr;
    std::string  link;

    grandet_fetch_task_t fetch_task;

    int deleted;                /* out of tree node */
    int fetched;                /* the content is pull to local */
    int content_dirty;          /* the content is local dirty */
    int open_rc;                /* number of open */
    list_entry_s c_list;
} file_entry_s;

#endif
