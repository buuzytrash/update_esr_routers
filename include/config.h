#ifndef CONFIG_H
#define CONFIG_H

#define ROUTER_IP "192.168.56.101"
#define HOST_IP "192.168.56.1"
#define ROUTER_USER "admin"
#define ROUTER_PASS "Medigri03!"
#define LOCAL_FIRMWARE_PATH "vesr-1.24.1-build1.firmware"
#define TFTP_SERVER_PATH "/tmp/vesr-1.24.1-build1.firmware"
#define TFTP_NAME "tftp://"

#define MAX_STR_LEN 256
#define LOG_FILE "firmware_uploader.log"
#define CONFIG_FILE "config.ini"

typedef struct {
    char router_ip[MAX_STR_LEN];
    char router_user[MAX_STR_LEN];
    char router_pass[MAX_STR_LEN];
    char firmware_file[MAX_STR_LEN];
    int dry_run;
} UploadConfig;

int load_config_from_ini(UploadConfig *cfg, const char *filename);
void log_message(const char *format, ...);

#endif  // CONFIG_H