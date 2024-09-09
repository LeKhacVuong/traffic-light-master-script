#include "sm_logger.h"
#include "linux_serial.h"
#include <time.h>
#include <sys/time.h>
#include "light_master_service.h"
#include "sm_time_utils.h"

#define TAG "main"
#define USB_PORT "/dev/ttyUSB0"
int g_fd;
light_master_t* g_light_master;


int64_t get_tick_count() {
    struct timespec ts;
    int32_t tick = 0U;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    tick = ts.tv_nsec / 1000000;
    tick += ts.tv_sec * 1000;
    return tick;
}

void getTimeString(char *buffer, size_t bufferSize) {
    struct timeval tv;
    struct tm *timeinfo;
    char timeStr[9];
    gettimeofday(&tv, NULL);
    timeinfo = localtime(&tv.tv_sec);
    strftime(timeStr, sizeof(timeStr), "%H:%M:%S", timeinfo);
    snprintf(buffer, bufferSize, "%s:%03ld", timeStr, tv.tv_usec / 1000);
}


void log_put(const char* _log) {
    char timeString[13];
    getTimeString(timeString, sizeof(timeString));

    printf("%s: %s\n",timeString, _log);
}

int32_t mb_send_if(const uint8_t* _data, int32_t _len, int32_t _timeout, void* _arg){
    return 	serial_send_bytes(g_fd, _data, _len);
}

int32_t mb_rcv_if(uint8_t* _buf, int32_t _len, int32_t _timeout, void* _arg){

    elapsed_timer_t timeout;
    elapsed_timer_resetz(&timeout, _timeout);

    do {
        if(serial_data_available(g_fd) >= _len){
            return serial_recv_bytes(g_fd, _buf, _len);
        }
    }
    while(elapsed_timer_get_remain(&timeout));

    return 0;
}

void app_control_light_if(LIGHT_VALUE _color){
//    LOG_INF(TAG, "Control master light color to %s", getLightColorString(_color));
}


void help_control_light(){
    LOG_INF(TAG, "Please Enter slave ID (1 - 3) first and light color (0: Green, 1: Red, 2: Yellow)!!!");
    while (getc(stdin) != '\n');
    int id, value;
    scanf("%d%d", &id, &value);
    if(id > 3 || id < 1 || value < 0 || value > 2){
        LOG_WRN(TAG, "Invalid slave ID or light color !!!");
        return;
    }
    LOG_INF(TAG, "Set slave %d to light color %d", id, value);

}

void help_set_light_duration(){
    LOG_INF(TAG, "Please Enter slave ID (1 - 3) first, light color (0: Green, 1: Red, 2: Yellow), duration (sec max 60s)!!!");
    while (getc(stdin) != '\n');
    int id, value, duration;
    scanf("%d%d%d", &id, &value, &duration);
    if(id > 3 || id < 1 || value < 0 || value > 2 || duration < 0 || duration > 60){
        LOG_WRN(TAG, "Invalid slave ID or light color !!!");
        return;
    }
    LOG_INF(TAG, "Set slave %d light color %d duration to %d", id, value, duration);

}

void print_help(const char _c){

    switch (_c) {
        case 'a':{
            help_control_light();
            break;
        }
        case 'b':{
            help_set_light_duration();
            break;
        }
        case 'c':{
            break;
        }
        default:
            LOG_WRN(TAG, "Choose again  !!!");
            break;
    }
}

int main() {
    sm_logger_init(log_put, LOG_LEVEL_INFO);

    g_fd = serial_init(USB_PORT, 115200, SERIAL_FLAG_BLOCKING);
    if (g_fd < 0) {
        LOG_INF(TAG, "cannot open serial port\n");
        return -1;
    }

    g_light_master = light_master_create(mb_send_if, mb_rcv_if, NULL);
    light_master_set_ctrl_light_if(g_light_master, app_control_light_if);

    while (1){
        light_master_process(g_light_master);
    }

}
