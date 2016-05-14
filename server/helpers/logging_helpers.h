#ifndef __SERVER_UTILS__
#define __SERVER_UTILS__

#define MAX_LOG_LENGTH 100

#ifdef __LOGGING__
#define LOG_INFO(str, msg, ...)     \
	sprintf(str, msg, ##__VA_ARGS__); \
	log_info(str)
#define LOG_ERROR(str, msg, ...)    \
	sprintf(str, msg, ##__VA_ARGS__); \
	log_error(str)
#define LOG_WARN(str, msg, ...)     \
	sprintf(str, msg, ##__VA_ARGS__); \
	log_warning(str)
#else
#define LOG_INFO(str, msg, ...) INFO(msg, ##__VA_ARGS__)
#define LOG_ERROR(str, msg, ...) ERROR(msg, ##__VA_ARGS__)
#define LOG_WARN(str, msg, ...) WARN(msg, ##__VA_ARGS__)
#endif

void init_mq();

void log_info(char* msg);
void log_error(char* msg);
void log_warning(char* msg);
void log_success(char* msg);

#endif  // __SERVER_UTILS__