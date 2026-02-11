/*
 * File: pulse_calc.h
 * Mô tả: Thư viện tính toán nhịp tim (BPM) và nồng độ oxy (SpO2)
 */

#ifndef PULSE_CALC_H_
#define PULSE_CALC_H_

#include <stdint.h>
#include <stdbool.h>

// --- CẤU HÌNH ---
#define BPM_BUFFER_SIZE 4   // Lưu 4 giá trị BPM gần nhất để tính trung bình cho mượt
#define MIN_BPM 40          // Nhịp tim thấp nhất chấp nhận được
#define MAX_BPM 200         // Nhịp tim cao nhất chấp nhận được

// --- CẤU TRÚC DỮ LIỆU ---
typedef struct {
    // Biến cho phát hiện nhịp tim (Beat Detection)
    float threshold;        // Ngưỡng động để bắt đỉnh sóng
    uint32_t last_beat_time;// Thời gian (ms) của nhịp tim trước đó
    uint32_t current_bpm;   // Giá trị BPM hiện tại (tức thời)
    
    // Biến cho bộ đệm trung bình BPM
    uint8_t bpm_buffer[BPM_BUFFER_SIZE];
    uint8_t bpm_index;
    uint32_t avg_bpm;       // KẾT QUẢ CUỐI CÙNG: BPM trung bình
    
    // Biến cho tính SpO2
    float ac_red_max;       // Biên độ đỉnh của đèn Red
    float ac_ir_max;        // Biên độ đỉnh của đèn IR
    uint8_t spo2;           // KẾT QUẢ CUỐI CÙNG: SpO2 (%)
    
} pulse_oximeter_t;

// --- KHAI BÁO HÀM ---

/**
 * @brief Khởi tạo các biến tính toán
 */
void pulse_calc_init(pulse_oximeter_t *oximeter);

/**
 * @brief Hàm quan trọng nhất: Cập nhật dữ liệu và tính toán
 * @param oximeter Con trỏ đến struct dữ liệu
 * @param ir_ac Tín hiệu AC của IR (đã lọc) -> Dùng để tìm nhịp tim
 * @param red_ac Tín hiệu AC của Red (đã lọc) -> Dùng để tính SpO2
 * @param ir_dc Tín hiệu thô (DC) của IR -> Dùng để chuẩn hóa SpO2
 * @param red_dc Tín hiệu thô (DC) của Red -> Dùng để chuẩn hóa SpO2
 * @return true nếu phát hiện một nhịp tim mới, false nếu không
 */
bool pulse_calc_update(pulse_oximeter_t *oximeter, float ir_ac, float red_ac, uint32_t ir_dc, uint32_t red_dc);

#endif /* PULSE_CALC_H_ */