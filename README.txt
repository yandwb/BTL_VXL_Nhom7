# ESP32-C3 Health Monitor: Nhịp Tim & Nhịp Thở (Heart Rate & Respiration Monitor)

## 1. Giới thiệu (Overview)
Dự án thiết kế thiết bị theo dõi sức khỏe tích hợp (SoC) sử dụng vi điều khiển **ESP32-C3**. Hệ thống có khả năng đo và phân tích tín hiệu sinh tồn theo thời gian thực:
- **Nhịp tim (Heart Rate - BPM):** Sử dụng phương pháp quang phổ thể tích (PPG) với cảm biến MAX30102.
- **Nhịp thở (Respiration Rate - RPM):** Sử dụng phân tích âm thanh luồng khí với Microphone I2S INMP441.

Hệ thống hoạt động trên nền tảng hệ điều hành thời gian thực **FreeRTOS**, đảm bảo khả năng đa nhiệm, độ trễ thấp và tiết kiệm năng lượng.

---

## 2. Tính năng nổi bật (Key Features)
* **Xử lý tín hiệu số (DSP Pipeline):** Tích hợp bộ lọc khử trôi dòng một chiều (DC Removal) và bộ lọc trung bình trượt (Mean Filter) để làm sạch tín hiệu.
* **Hệ điều hành thời gian thực (FreeRTOS):** Quản lý đa tác vụ (Multitasking) với cơ chế Mutex bảo vệ I2C Bus và Queue để truyền dữ liệu an toàn.
* **Cơ chế hướng sự kiện (Event-driven):** Sử dụng ngắt phần cứng (Interrupt) từ cảm biến để đánh thức vi điều khiển, loại bỏ hoàn toàn việc Polling lãng phí tài nguyên.
* **Phần cứng chống nhiễu:** Thiết kế mạch lọc nguồn phân tán (Decoupling) và ổn định giao tiếp I2C.

---

## 3. Cấu hình Phần cứng (Hardware Setup)

### 3.1. Sơ đồ đấu nối (Pinout) cho ESP32-C3
| Thiết bị | Chân Module | GPIO (ESP32-C3) | Ghi chú |
| :--- | :--- | :--- | :--- |
| **I2C Bus** | SDA | **GPIO 8** | Dùng chung (OLED + MAX30102) |
| | SCL | **GPIO 9** | Trở kéo 4.7kΩ |
| **MAX30102** | INT | **GPIO 3** | Ngắt phần cứng (Active Low) |
| **INMP441** | SCK | **GPIO 6** | I2S Clock |
| | WS | **GPIO 5** | I2S Word Select |
| | SD | **GPIO 4** | I2S Data |
| **Nguồn** | 3.3V / GND | | Cấp từ module nguồn ngoài MB102 |

### 3.2. Thiết kế bộ lọc nhiễu (Hardware Filters)
Để đảm bảo tín hiệu sạch trong môi trường nhiễu (WiFi/Bluetooth), hệ thống áp dụng các tầng lọc:
1.  **Nguồn tổng:** Sử dụng Module MB102 (AMS1117) cấp dòng 800mA, cách ly khỏi nhiễu cổng USB.
2.  **Khối Tim (MAX30102):** Tụ 10µF + 0.1µF song song tại chân nguồn. Trở kéo I2C tổng hợp ~3kΩ-4kΩ.
3.  **Khối Phổi (INMP441):** Mạch lọc LC (hoặc RC với R=10Ω) để chặn nhiễu cao tần từ vi xử lý.
4.  **Khối Thẻ nhớ (Optional):** Tụ bù dòng 100µF chống sụt áp khi ghi dữ liệu.

---

## 4. Kiến trúc Phần mềm (Software Architecture)

### 4.1. Lưu đồ thuật toán (System Flowchart)
Hệ thống hoạt động theo mô hình **Pipeline (Đường ống)**:
1.  **Acquisition:** Thu thập dữ liệu thô từ cảm biến (qua Ngắt hoặc DMA).
2.  **Preprocessing:** Khử DC Offset và đảo dấu tín hiệu (Signal Inversion).
3.  **Filtering:** Lọc trung bình (Mean Filter) để loại bỏ nhiễu gai.
4.  **Analysis:** Đếm đỉnh (Peak Detection) để tính BPM/RPM.
5.  **Output:** Hiển thị OLED và Log dữ liệu.

*(Xem chi tiết tại thư mục `/docs/flowchart.png`)*

### 4.2. Máy trạng thái (State Machine)
Hệ thống được quản lý bởi 5 trạng thái chính:
* **INITIALIZATION:** Cấu hình Driver, tạo Task/Queue.
* **IDLE:** Chế độ ngủ đông, CPU bị Block chờ sự kiện.
* **ACQUISITION:** Đánh thức bởi Ngắt (MAX30102) hoặc I2S DMA đầy.
* **PROCESSING:** Thực thi thuật toán DSP.
* **DISPLAY:** Cập nhật màn hình (Chiếm quyền Mutex I2C).

---

## 5. Giải thuật xử lý tín hiệu (DSP Algorithms)

### 5.1. Xử lý nhịp tim (PPG Signal)
Do tín hiệu nhịp tim (AC) chỉ chiếm 1-2% so với nhiễu nền (DC), thuật toán xử lý như sau:
* **Bước 1: DC Removal (High-Pass):**
    `AC_Value = -(Raw_Data - DC_Estimator)`
    *Tác dụng: Loại bỏ ánh sáng phản xạ tĩnh từ da/xương, giữ lại biến thiên dòng máu. Đảo dấu để đỉnh sóng trùng với nhịp tim.*
* **Bước 2: Mean Filter (Low-Pass):**
    `Filtered = (Current + Previous_Samples) / N`
    *Tác dụng: Làm mượt đường sóng, loại bỏ nhiễu rung động (Motion Artifact).*

### 5.2. Xử lý nhịp thở (Breath Sound)
* **Bước 1:** Thu thập mẫu âm thanh qua I2S.
* **Bước 2:** Chỉnh lưu (Rectification) -> `abs(sample)`.
* **Bước 3:** Tách đường bao (Envelope Detection) để loại bỏ tần số âm thanh, giữ lại nhịp thở chậm.
* **Bước 4:** Schmitt Trigger để đếm nhịp thở và loại bỏ nhiễu nền.

---

## 6. Cài đặt và Biên dịch (Installation)

### Yêu cầu:
* ESP-IDF v5.x trở lên.
* Toolchain cho ESP32-C3 (RISC-V).

### Hướng dẫn Build:
1.  Clone dự án về máy.
2.  Mở terminal ESP-IDF.
3.  Cấu hình target:
    ```bash
    idf.py set-target esp32c3
    ```
4.  Biên dịch và nạp code:
    ```bash
    idf.py build flash monitor
    ```

---

## Link Nhật ký Kỹ thuật: "https://docs.google.com/document/d/13b5Zm203hD_uzbtLI1kKgH4xEwOYUa8le9fOPnYgleY/edit?tab=t.0"