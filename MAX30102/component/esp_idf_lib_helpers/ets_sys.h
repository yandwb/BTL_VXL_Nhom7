#if CONFIG_IDF_TARGET_ESP32
#include <esp32/rom/ets_sys.h>
#elif CONFIG_IDF_TARGET_ESP32S2
#include <esp32s2/rom/ets_sys.h>
#elif CONFIG_IDF_TARGET_ESP32S3
#include <esp32s3/rom/ets_sys.h>
#elif CONFIG_IDF_TARGET_ESP32C3
#include <esp32c3/rom/ets_sys.h>
#elif CONFIG_IDF_TARGET_ESP32H2
#include <esp32h2/rom/ets_sys.h>
#elif CONFIG_IDF_TARGET_ESP32C2
#include <esp32c2/rom/ets_sys.h>
#endif
/*Đây là file chứa các hàm cấp thấp nằm trong bộ nhớ ROM (Read-Only Memory) của chip. 
Hàm quan trọng nhất thường dùng từ file này là ets_delay_us().
Khi giao tiếp với cảm biến (như I2C bit-banging hoặc chờ cảm biến khởi động), 
ta cần delay chính xác từng micro giây (us). 
Hàm ets_delay_us gọi trực tiếp vào ROM nên độ chính xác rất cao, 
không bị ảnh hưởng nhiều bởi hệ điều hành FreeRTOS.*/