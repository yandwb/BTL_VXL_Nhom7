/*
 * File: pulse_calc.c
 * Mô tả: Thực thi thuật toán đếm nhịp và tính SpO2
 */

#include "pulse_calc.h"
#include "esp_timer.h" // Thư viện lấy thời gian của ESP32
#include <math.h>

// Hàm khởi tạo
void pulse_calc_init(pulse_oximeter_t *oximeter) {
    oximeter->threshold = 20.0; // Ngưỡng bắt đầu (sẽ tự động điều chỉnh sau)
    oximeter->last_beat_time = 0;
    oximeter->current_bpm = 0;
    oximeter->avg_bpm = 0;
    oximeter->bpm_index = 0;
    oximeter->spo2 = 98; // Giả định ban đầu
    
    for(int i=0; i<BPM_BUFFER_SIZE; i++) {
        oximeter->bpm_buffer[i] = 0;
    }
}

// Hàm tính toán chính
bool pulse_calc_update(pulse_oximeter_t *oximeter, float ir_ac, float red_ac, uint32_t ir_dc, uint32_t red_dc) {
    
    // 1. Lấy thời gian hiện tại (tính bằng milliseconds)
    uint32_t now = (uint32_t)(esp_timer_get_time() / 1000);
    
    // 2. Tìm biên độ đỉnh (Dùng cho SpO2)
    // Chúng ta liên tục cập nhật giá trị lớn nhất trong chu kỳ tim
    if (ir_ac > oximeter->ac_ir_max) oximeter->ac_ir_max = ir_ac;
    if (red_ac > oximeter->ac_red_max) oximeter->ac_red_max = red_ac;

    // 3. THUẬT TOÁN PHÁT HIỆN NHỊP TIM (BEAT DETECTION)
    // Sử dụng tín hiệu IR vì nó rõ hơn Red
    // Điều kiện: Tín hiệu vượt ngưỡng VÀ đã qua thời gian chờ (refractory period)
    // 60000ms / 200bpm = 300ms. Không thể có 2 nhịp tim cách nhau dưới 300ms.
    
    if (ir_ac > oximeter->threshold && (now - oximeter->last_beat_time > 300)) {
        
        // --- Đã tìm thấy một nhịp tim! ---
        
        // A. Tính BPM
        uint32_t delta_time = now - oximeter->last_beat_time;
        oximeter->last_beat_time = now;
        
        // Công thức: BPM = 60000 ms / khoảng cách giữa 2 nhịp
        uint32_t instant_bpm = 60000 / delta_time;
        
        // Lọc bỏ các giá trị vô lý (nhiễu)
        if (instant_bpm > MIN_BPM && instant_bpm < MAX_BPM) {
            oximeter->current_bpm = instant_bpm;
            
            // Tính trung bình cộng (Moving Average) cho BPM mượt mà
            oximeter->bpm_buffer[oximeter->bpm_index++] = (uint8_t)instant_bpm;
            if (oximeter->bpm_index >= BPM_BUFFER_SIZE) oximeter->bpm_index = 0;
            
            uint16_t sum = 0;
            uint8_t count = 0;
            for(int i=0; i<BPM_BUFFER_SIZE; i++) {
                if(oximeter->bpm_buffer[i] > 0) {
                    sum += oximeter->bpm_buffer[i];
                    count++;
                }
            }
            if(count > 0) oximeter->avg_bpm = sum / count;
        }

        // B. Tính SpO2 (Chỉ tính khi có nhịp tim)
        // Công thức Ratio of Ratios: R = (AC_Red / DC_Red) / (AC_IR / DC_IR)
        float r_ratio = (oximeter->ac_red_max / (float)red_dc) / (oximeter->ac_ir_max / (float)ir_dc);
        
        // Công thức thực nghiệm (Calibration Curve):
        // SpO2 = -45.060 * R * R + 30.354 * R + 94.845 (Ví dụ của Maxim)
        // Hoặc công thức tuyến tính đơn giản: SpO2 = 110 - 25 * R
        float calc_spo2 = 110.0 - 25.0 * r_ratio;
        
        // Giới hạn giá trị 0-100%
        if (calc_spo2 > 100) calc_spo2 = 100;
        if (calc_spo2 < 0) calc_spo2 = 0;
        
        oximeter->spo2 = (uint8_t)calc_spo2;
        
        // Reset biên độ đỉnh để chuẩn bị cho nhịp tim tiếp theo
        oximeter->ac_ir_max = 0;
        oximeter->ac_red_max = 0;
        
        // Tự động điều chỉnh ngưỡng (Dynamic Threshold)
        // Ngưỡng = 50% biên độ đỉnh vừa đo được
        oximeter->threshold = ir_ac * 0.5;
        if (oximeter->threshold < 20.0) oximeter->threshold = 20.0; // Mức sàn

        return true; // Báo hiệu đã có nhịp tim mới
    }
    
    // Nếu tín hiệu giảm sâu quá, reset ngưỡng từ từ để đón nhịp tiếp theo
    if (ir_ac < oximeter->threshold * 0.5) {
        oximeter->threshold *= 0.99;
    }

    return false;
}