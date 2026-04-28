# Lập trình xây dựng thiết bị đồng hồ số dựa trên board SN32F407

## I. ĐẶC TẢ YÊU CẦU DỰ ÁN ĐỒNG HỒ SỐ (SYSTEM SPECIFICATION)

### 1. Hiển thị & Bộ đếm thời gian
* 4 LED 7SEG sẽ thể hiện giờ và phút ở chế độ thường theo định dạng: HH.MM.
* Ngay sau khi cấp nguồn, mạch hoạt động ở chế độ thường, giờ và phút được reset về giá trị 0: LED 7SEG sẽ hiện thị 00.00.
* Bộ đếm sẽ đếm, sau mỗi phút sẽ hiện thị phút + 1, đến phút thứ 60 sẽ reset phút về 0 và tăng giờ lên 1. Giờ sẽ được hiện thị từ 00 đến 23.
* **Trường hợp biên:** Khi đồng hồ chạy đến 23:59 và qua giây 59, đồng hồ tự động reset về 00.00.
* Thời gian thực (giây ngầm) vẫn tiếp tục chạy liên tục không ngừng kể cả khi người dùng đang ở trong các chế độ Setup hoặc Hẹn giờ.

### 2. NÚT SETUP (SW3)
* Khi đang ở chế độ thường, ấn SW3 để vào chế độ thay đổi giờ: LED 7SEG HH sẽ nhấp nháy với chu kỳ 1s (0.5s ON – 0.5s OFF).
* Khi đang ở chế độ thay đổi giờ, ấn SW3 để vào chế độ thay đổi phút: LED 7SEG MM sẽ nhấp nháy với chu kỳ 1s (0.5s ON – 0.5s OFF).
* Khi đang ở chế độ thay đổi phút, ấn SW3 để vào chế độ thường.
* **Trường hợp biên:** Khi đang ở chế độ SETUP, phím HẸN GIỜ (SW16) sẽ bị khóa (vô hiệu hóa) để tránh xung đột trạng thái.

### 3. NÚT HẸN GIỜ (SW16)
* Khi đang ở chế độ thường, ấn SW16 để vào chế độ thay đổi giờ hẹn: LED 7SEG HH sẽ nhấp nháy với chu kỳ 1s (0.5s ON – 0.5s OFF).
* Khi đang ở chế độ thay đổi giờ hẹn, ấn SW16 để vào chế độ thay đổi phút hẹn: LED 7SEG MM sẽ nhấp nháy với chu kỳ 1s (0.5s ON – 0.5s OFF).
* Khi đang ở chế độ thay đổi phút hẹn, ấn SW16 để ghi giờ phút hẹn vào EEPROM và thoát ra chế độ thường.
* **Trường hợp biên:** Khi đang ở chế độ HẸN GIỜ, phím SETUP (SW3) sẽ bị khóa.

### 4. NÚT + (SW6) & NÚT - (SW10)
* **NÚT + (SW6):** Khi đang ở chế độ thay đổi giờ/giờ hẹn, ấn SW6 để tăng giờ lên 1 đơn vị, khi giờ tăng đến 24 sẽ reset giờ về 0. Khi đang ở chế độ thay đổi phút/phút hẹn, ấn SW6 để tăng phút lên 1 đơn vị, khi phút tăng đến 60 sẽ reset phút về 0.
* **NÚT - (SW10):** Khi đang ở chế độ thay đổi giờ/giờ hẹn, ấn SW10 để giảm giờ xuống 1 đơn vị, khi giờ = 0 mà giảm 1 đơn vị sẽ đặt giờ = 23. Khi đang ở chế độ thay đổi phút/phút hẹn, ấn SW10 để giảm phút xuống 1 đơn vị, khi phút = 0 mà giảm 1 đơn vị sẽ đặt phút = 59.
* **Trường hợp biên:** Việc ấn giữ các phím SW6/SW10 sẽ chỉ được tính là 1 lần nhấn hợp lệ (chống dội phím bắt sườn), không có hiện tượng số nhảy liên tục (auto-repeat).

### 5. Còi (BUZZER)
* Mỗi khi ấn các nút SW3, SW6, SW10, SW16 thì còi sẽ kêu pip trong 0.3s.
* Khi đồng hồ chạy đến giờ hẹn (trùng khớp cả HH và MM), còi sẽ kêu pip-pip liên tục trong 5s với chu kỳ (0.5s ON – 0.5s OFF).
* Khi hết thời gian timeout thoát khỏi chế độ thay đổi giờ/phút (thực hoặc hẹn) để về chế độ thường thì còi sẽ kêu pip trong 0.3s.
* **Trường hợp biên:** Nếu người dùng ấn bất kỳ nút nào trong lúc chuông đang reo báo thức 5s, chuông sẽ lập tức bị ngắt.

### 6. LED (D4 / D6)
* Ở chế độ thay đổi giờ hẹn hoặc chế độ thay đổi phút hẹn thì LED sẽ nhấp nháy với chu kỳ 1s (0.5s ON – 0.5s OFF).
* Ngoài ra LED sẽ ở trạng thái OFF.

### 7. EEPROM
* Lưu giờ hẹn và phút hẹn, giây hẹn mặc định = 0.
* **Trường hợp biên:** Dữ liệu giờ hẹn được tự động đọc từ EEPROM lên ngay khi MCU vừa cấp nguồn (khởi động). Nếu EEPROM trống (chip trắng), giờ hẹn tự động gán là 00.00.

### 8. Timeout khi không có sự kiện bấm nút
* Ở chế độ thay đổi giờ/phút hoặc chế độ thay đổi giờ/phút hẹn, trong vòng 30s không có sự kiện ấn nút thì sẽ tự động thoát ra chế độ thường.
* **Trường hợp biên:** Bộ đếm 30s này sẽ được reset lại từ đầu mỗi khi hệ thống ghi nhận có một thao tác bấm nút hợp lệ.

## II. Sơ đồ khối

<img width="702" height="539" alt="image" src="https://github.com/user-attachments/assets/ad44b0d6-2a29-4ac8-8fb4-a1fe8e8c1720" />
## III. [cite_start]Kế hoạch Kiểm thử & Các trường hợp biên (Test Plan & Edge Cases) 

### [cite_start]A. Test Case: Biên đếm thời gian tự động (Time Rollover) [cite: 40]

| ID | Kịch bản Test | Hành động | Kết quả mong đợi | Trạng thái |
| :--- | :--- | :--- | :--- | :--- |
| TC_01 | Chuyển phút | Đợi đồng hồ chạy đến XX:59 và qua giây 59 | [cite_start]Phút reset về 00, Giờ tăng lên 1. [cite: 41] | [cite_start]Pass [cite: 41] |
| TC_02 | Chuyển ngày (Biên cực đại) | Set đồng hồ chạy đến 23:59 và qua giây 59. | [cite_start]Đồng hồ reset về 00:00. [cite: 41] | [cite_start]Pass [cite: 41] |
| TC_03 | Khởi động lại nguồn | Rút nguồn mạch và cắm lại. Giờ hẹn sẽ được đọc từ EPPROM lên giữ nguyên giá trị ban đầu. | [cite_start]Giờ hiện tại mất, reset về 00.00 [cite: 41] | [cite_start]Pass [cite: 41] |

### [cite_start]B. Test Case: Biên cài đặt bằng phím tay (Manual Setting Rollover) [cite: 42]

| ID | Kịch bản Test | Hành động | Kết quả mong đợi | Trạng thái |
| :--- | :--- | :--- | :--- | :--- |
| TC_04 | Tăng phút vượt quá 59 | Ở chế độ Cài phút (SW3), bấm SW6 liên tục khi đang ở 59. | [cite_start]Phút nhảy từ 59 về 00. Còi kêu "pip" 0.3s mỗi lần bấm [cite: 43] | [cite_start]Pass [cite: 43] |
| TC_05 | Giảm phút dưới 0 | Ở chế độ Cài phút (SW3), bấm SW10 khi đang ở 00. | [cite_start]Phút nhảy lùi từ 00 lên 59. [cite: 43] | [cite_start]Pass [cite: 43] |
| TC_06 | Tăng giờ vượt quá 23 | Ở chế độ Cài giờ, bấm SW6 khi đang ở 23. | [cite_start]Giờ nhảy từ 23 về 00. [cite: 43] | [cite_start]Pass [cite: 43] |
| TC_07 | Giảm giờ dưới 0 | Ở chế độ Cài giờ, bấm SW10 khi đang ở 00. | [cite_start]Giờ nhảy lùi từ 00 lên 23. [cite: 43] | [cite_start]Pass [cite: 43] |

### [cite_start]C. Test Case: Xung đột trạng thái & Luồng thực thi (State Collisions & Concurrency) [cite: 44]

| ID | Kịch bản Test | Hành động | Kết quả mong đợi | Trạng thái |
| :--- | :--- | :--- | :--- | :--- |
| TC_08 | Xung đột nút chức năng | Đang ở chế độ nhấp nháy cài giờ thực (ấn SW3), bất ngờ ấn nút hẹn giờ (SW16). | [cite_start]Hệ thống bỏ qua nút SW16, giữ nguyên chế độ hiện tại [cite: 45] | [cite_start]Pass [cite: 45] |
| TC_09 | Chạy ngầm thời gian | Set đồng hồ là 10:59. Ấn SW16 để vào cài Alarm, đứng yên đợi 2 phút, sau đó thoát ra. | Màn hình chính phải hiện 11:01. [cite_start]Chứng minh thời gian vẫn chạy ngầm khi đang ở chế độ cài đặt. [cite: 45] | [cite_start]Pass [cite: 45] |
| TC_10 | Timeout 30 giây | Vào chế độ chỉnh giờ (SW3) hoặc hẹn giờ (SW16), bỏ tay ra không bấm nút nào trong 30 giây. | Tự động thoát về chế độ thường. [cite_start]Còi kêu "pip" 0.3s. [cite: 45] | [cite_start]Pass [cite: 45] |
| TC_11 | Timeout có can thiệp | Vào chế độ chỉnh giờ, đợi 25 giây, sau đó bấm SW6 (+). Đợi thêm 10 giây nữa. | [cite_start]Màn hình KHÔNG tự thoát ở giây thứ 30. Biến đếm 30s phải được reset mỗi khi có nút bấm mới. [cite: 45] | [cite_start]Pass [cite: 45] |

### [cite_start]D. Test Case: Chuông báo (Alarm Actions) [cite: 46]

| ID | Kịch bản Test | Hành động | Kết quả mong đợi | Trạng thái |
| :--- | :--- | :--- | :--- | :--- |
| TC_12 | Kích hoạt chuông đúng giờ | Set giờ thực và giờ Alarm trùng khớp nhau (HH:MM == Alarm_HH:Alarm_MM). | [cite_start]Còi kêu pip-pip liên tục trong 5 giây (0.5s ON, 0.5s OFF). [cite: 47] | [cite_start]Pass [cite: 47] |
| TC_13 | Bấm phím khi đang reo chuông | Đang lúc chuông reo (trong 5 giây đó), bấm bất kỳ nút nào (ví dụ SW3 hoặc SW6). | [cite_start]Chuông đang reo hẹn giờ, khi có nút bấm thì sẽ dừng reo chuông và kêu bíp 0.3s [cite: 47] | [cite_start]Pass [cite: 47] |

### [cite_start]E. Test Case: Xử lý ngoại lệ phím bấm cơ học (Hardware & Debounce) [cite: 48]

| ID | Kịch bản Test | Hành động | Kết quả mong đợi | Trạng thái |
| :--- | :--- | :--- | :--- | :--- |
| TC_14 | Giữ chặt phím | Đang ở chế độ chỉnh giờ, bấm và giữ nguyên nút SW6 (+) trong 5 giây. | Giờ chỉ tăng đúng 1 đơn vị rồi dừng lại (Nhờ kỹ thuật bắt sườn lên trong KeyScan()). [cite_start]Hệ thống không được tăng giờ liên tục (Auto-repeat). [cite: 49] | [cite_start]Pass [cite: 49] |
| TC_15 | Bấm nhiều phím cùng lúc | Đang ở chế độ thường, dùng 2 ngón tay ấn mạnh SW3 và SW16 cùng một lúc. | Hệ thống ưu tiên xử lý 1 phím theo thứ tự quét của phần cứng, hoặc bỏ qua để tránh nhiễu. [cite_start]Không được treo MCU. [cite: 49] | [cite_start]Pass [cite: 49] |
| TC_16 | Nhấn phím cực nhanh | Bấm nhấp nhả nút SW6 liên tục với tốc độ cao nhất có thể. | Số lần giờ tăng lên phải tương ứng với số lần bấm hợp lệ (đã qua màng lọc debounce 50 chu kỳ). [cite_start]MCU không bị đơ hoặc reset. [cite: 49] | [cite_start]Pass [cite: 49] |
