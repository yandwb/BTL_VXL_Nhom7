/*
 * File: dsp_filter.c
 * Mô tả: Mã nguồn thực thi các thuật toán lọc
 */

#include "filter.h" // Nhúng file bản vẽ vào để biết cấu trúc dữ liệu

// -------------------------------------------------------------------------
// PHẦN 1: CÁC HÀM KHỞI TẠO (INIT)
// -------------------------------------------------------------------------

/* Hàm Reset bộ lọc DC */
void dc_filter_init(dc_filter_t *filter) {
    filter->w = 0;       // Đặt trạng thái lịch sử về 0
    filter->result = 0;  // Đặt kết quả về 0
}

/* Hàm Reset bộ lọc Mean */
void mean_filter_init(mean_filter_t *filter) {
    filter->index = 0;   // Đặt con trỏ về đầu mảng
    filter->sum = 0;     // Đặt tổng về 0
    
    // Vòng lặp xóa sạch dữ liệu trong mảng buffer
    for(int i = 0; i < MA_SIZE; i++) {
        filter->buffer[i] = 0; // Gán tất cả phần tử về 0
    }
}

// -------------------------------------------------------------------------
// PHẦN 2: THUẬT TOÁN LỌC DC (IIR Filter)
// -------------------------------------------------------------------------
/*
 * Công thức toán học: y[n] = x[n] - x[n-1] + alpha * y[n-1]
 * Đây là dạng biến đổi của bộ lọc thông cao RC trong mạch điện tử.
 */
float dc_removal(float input, dc_filter_t *filter) {
    // [HỆ SỐ ALPHA]: 0.95
    // - Quyết định tần số cắt (Cut-off frequency). 
    // - 0.95 tương đương mạch tụ điện chặn DC nhưng cho nhịp tim (0.5Hz - 3Hz) đi qua.
    const float alpha = 0.992; 

    // Bước 1: Tính toán trạng thái mới (w) dựa trên đầu vào và trạng thái cũ
    // w_new = input + alpha * w_old
    float w = input + alpha * filter->w;

    // Bước 2: Tính kết quả đầu ra
    // Output = w_new - w_old
    // Phép trừ này chính là hành động "loại bỏ nền"
    float result = w - filter->w;

    // Bước 3: Lưu lại trạng thái mới để dùng cho lần gọi sau (đệ quy)
    filter->w = w;

    // Trả về kết quả
    return result;
}

// -------------------------------------------------------------------------
// PHẦN 3: THUẬT TOÁN LỌC TRUNG BÌNH (Moving Average - Optimized)
// -------------------------------------------------------------------------
/*
 * Kỹ thuật tối ưu: Thay vì cộng lại toàn bộ mảng mỗi lần (tốn CPU),
 * ta dùng kỹ thuật "Trừ Cũ - Cộng Mới" trên cửa sổ trượt.
 */
float mean_filter(float input, mean_filter_t *filter) {
    // Bước 1: TRỪ CŨ
    // Lấy giá trị cũ nhất (đang nằm tại vị trí con trỏ index) trừ ra khỏi tổng.
    filter->sum -= filter->buffer[filter->index];
    
    // Bước 2: CẬP NHẬT MỚI
    // Ghi đè giá trị input mới vào vị trí đó (thay thế giá trị cũ).
    filter->buffer[filter->index] = input;
    
    // Bước 3: CỘNG MỚI
    // Cộng giá trị input mới vào tổng.
    filter->sum += input;
    
    // Bước 4: DI CHUYỂN CON TRỎ (Vòng tròn)
    filter->index++; // Nhích sang ô tiếp theo
    
    // Nếu con trỏ đi quá cuối mảng, quay nó về đầu (Tạo thành Ring Buffer)
    if (filter->index >= MA_SIZE) {
        filter->index = 0;
    }
    
    // Bước 5: TÍNH TRUNG BÌNH
    // Kết quả = Tổng / Số phần tử
    return filter->sum / MA_SIZE;
}