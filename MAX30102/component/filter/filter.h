
#ifndef DSP_FILTER_H_   // [GUARD] Bắt đầu vệ sĩ: Ngăn file này bị include nhiều lần gây lỗi trùng lặp
#define DSP_FILTER_H_   // Định nghĩa cờ hiệu: "File này đã được nạp rồi"

#include <stdint.h>     // Thư viện chứa các kiểu dữ liệu chuẩn (uint8_t, int32_t...)

// --- CẤU HÌNH BỘ LỌC ---

/* * Kích thước cửa sổ lọc trung bình (Moving Average Window Size).
 * - Ý nghĩa: Số lượng mẫu được dùng để tính trung bình.
 * - Giá trị 4: Cân bằng tốt giữa độ mịn và độ trễ (Lag).
 * - Nếu tăng lên (vd: 8, 16): Sóng mượt hơn nhưng phản hồi chậm hơn.
 */
#define MA_SIZE 8       

// --- ĐỊNH NGHĨA CẤU TRÚC DỮ LIỆU (STRUCTS) ---

/*
 * Cấu trúc 1: Bộ lọc khử DC (DC Removal Filter)
 * Nguyên lý: Bộ lọc thông cao (High-pass) đáp ứng xung vô hạn (IIR).
 * Mục đích: Loại bỏ đường nền (Base-line) do da, thịt, xương tạo ra.
 */
typedef struct {
    float w;       // Biến trạng thái nội tại (lưu giá trị lịch sử của bộ lọc)
    float result;  // Kết quả đầu ra sau khi lọc (Tín hiệu AC sạch)
} dc_filter_t;


/*
 * Cấu trúc 2: Bộ lọc làm mịn (Moving Average Filter)
 * Nguyên lý: Bộ lọc thông thấp (Low-pass) dạng FIR.
 * Mục đích: Loại bỏ nhiễu gai (High frequency noise).
 */
typedef struct {
    float buffer[MA_SIZE];  // Mảng lưu trữ các mẫu dữ liệu gần nhất (Bộ nhớ đệm)
    uint8_t index;          // Con trỏ vị trí hiện tại trong mảng (chạy vòng tròn từ 0 đến MA_SIZE-1)
    float sum;              // Tổng tích lũy của các phần tử trong buffer (giúp tính toán nhanh hơn)
} mean_filter_t;


// --- KHAI BÁO NGUYÊN MẪU HÀM (FUNCTION PROTOTYPES) ---
// (Đây là "Menu" để file main.c biết có những hàm nào để gọi)

/**
 * @brief Khởi tạo giá trị ban đầu cho bộ lọc DC (tránh lỗi rác bộ nhớ)
 * @param filter Con trỏ đến struct dc_filter_t cần khởi tạo
 */
void dc_filter_init(dc_filter_t *filter);

/**
 * @brief Khởi tạo giá trị ban đầu cho bộ lọc Mean
 * @param filter Con trỏ đến struct mean_filter_t cần khởi tạo
 */
void mean_filter_init(mean_filter_t *filter);

/**
 * @brief Thực hiện lọc bỏ thành phần một chiều (DC)
 * @param input Giá trị thô (Raw value) từ cảm biến
 * @param filter Con trỏ đến bộ lọc đang dùng
 * @return float Giá trị xoay chiều (AC Value) dao động quanh 0
 */
float dc_removal(float input, dc_filter_t *filter);

/**
 * @brief Thực hiện làm mịn tín hiệu (Trung bình động)
 * @param input Giá trị đầu vào (thường là kết quả của bộ lọc DC)
 * @param filter Con trỏ đến bộ lọc đang dùng
 * @return float Giá trị đã được làm mịn
 */
float mean_filter(float input, mean_filter_t *filter);

#endif /* DSP_FILTER_H_ */ // [GUARD] Kết thúc vệ sĩ