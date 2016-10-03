#ifndef __GRANDET__
#define __GRANDET__

#include <string>
#include <map>

typedef struct grandet_fetch_task_s *grandet_fetch_task_t;
typedef struct grandet_sync_task_s *grandet_sync_task_t;
typedef struct grandet_remove_task_s *grandet_remove_task_t;

void grandet_init(const char *base_path, int latency_requirement, int bandwidth_requirement);

int  grandet_fetch_task_start(const char *path, const char *key, const char *data_path, long size, grandet_fetch_task_t *ret);
void grandet_fetch_task_get(grandet_fetch_task_t task);
void grandet_fetch_task_put(grandet_fetch_task_t task);
void grandet_fetch_task_cancel(grandet_fetch_task_t task);
void grandet_fetch_task_wait(grandet_fetch_task_t task);
void grandet_fetch_task_end(grandet_fetch_task_t task);
void grandet_sync_task_start(const char *key, const char *data_path, const std::map<std::string, std::string> &xattr, grandet_sync_task_t *ret);
void grandet_sync_task_wait(grandet_sync_task_t task);
void grandet_sync_task_end(grandet_sync_task_t task);
void grandet_remove_task_start(const char *key, const char *data_path, grandet_remove_task_t *ret);
void grandet_remove_task_wait(grandet_remove_task_t task);
void grandet_remove_task_end(grandet_remove_task_t task);

#endif
