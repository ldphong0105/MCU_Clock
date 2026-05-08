# Lập trình xây dựng thiết bị đồng hồ số dựa trên board SN32F407

## I. Chỉ tiêu kỹ thuật (System Specifications)

| Thành phần | Đặc tả kỹ thuật |
| :--- | :--- |
| **Giao diện đầu vào (Inputs)** | 4 x Nút nhấn (SW3, SW6, SW10, SW16). Tích hợp chống dội phím (Debounce) và bắt sườn tín hiệu (Edge Detection). |
| **Giao diện đầu ra (Outputs)** | - 04 x LED 7 đoạn (Quét đa hợp).<br>- 01 x Còi báo (Active Buzzer).<br>- 01 x LED đơn chỉ báo trạng thái (D6). |
| **Định dạng thời gian** | 24 giờ (00:00 - 23:59). |
| **Độ phân giải hiệu chỉnh** | 1 đơn vị (Giờ/Phút) cho mỗi lần nhấn phím. |
| **Bộ nhớ phi tuyến** | EEPROM tích hợp, hỗ trợ đọc/ghi thông số báo thức. |
| **Cơ chế bảo vệ phần mềm** | - Khóa chéo phím chức năng (State Interlock).<br>- Bộ đếm thời gian chờ tự động (Auto-exit Timer 30s). |
| **Điều kiện khởi tạo** | Tự động Reset về 00.00 và nạp dữ liệu từ EEPROM ngay khi cấp nguồn. |

---

## II. Danh mục các chức năng của đồng hồ (System Features)

### 1. Chức năng hiển thị và vận hành thời gian thực
* **Hiển thị mặc định:** Hệ thống sử dụng 4 LED 7 đoạn để biểu diễn thời gian theo định dạng giờ và phút (HH.MM).
* **Đếm giờ tự động:** Bộ đếm tự động tăng phút sau mỗi 60 giây và tăng giờ sau mỗi 60 phút (từ 00 đến 23 giờ).
* **Xử lý tràn thời gian:** Khi đạt ngưỡng 23:59:59, hệ thống tự động quay về giá trị 00:00.
* **Duy trì thời gian ngầm:** Đảm bảo thời gian thực liên tục vận hành chính xác ngay cả khi người dùng đang thao tác trong menu cài đặt hoặc báo thức.

### 2. Chức năng tùy chỉnh thời gian hệ thống (Manual Setup)
* **Hiệu chỉnh giờ và phút:** Cho phép thay đổi giá trị thời gian thông qua chế độ nhấp nháy (chu kỳ 1s) để người dùng dễ dàng nhận diện đối tượng đang điều chỉnh.
* **Bảo vệ trạng thái:** Tự động khóa phím Hẹn giờ khi đang trong chế độ Setup để tránh các xung đột logic trong phần mềm.

### 3. Chức năng báo thức và lưu trữ (Alarm & Storage)
* **Thiết lập báo thức:** Người dùng có thể cài đặt giờ/phút hẹn giờ với chỉ báo đèn LED nhấp nháy.
* **Lưu trữ vĩnh viễn:** Giá trị báo thức được ghi vào EEPROM, cho phép hệ thống tự động khôi phục dữ liệu hẹn giờ ngay sau khi khởi động lại nguồn.
* **Thông báo âm thanh:** Còi báo (Buzzer) phát tín hiệu pip-pip liên tục trong 5 giây khi thời gian thực trùng khớp với giờ hẹn.

### 4. Chức năng tương tác và phản hồi người dùng
* **Phản hồi phím bấm:** Phát tín hiệu âm thanh ngắn (0.3s) mỗi khi có tác động vào phím bấm hợp lệ.
* **Tự động thoát chế độ (Timeout):** Hệ thống tự động chuyển về màn hình chính nếu không nhận được thao tác nào trong vòng 30 giây tại các menu cài đặt.
* **Hủy báo thức nhanh:** Cho phép ngắt chuông báo ngay lập tức bằng cách ấn bất kỳ phím nào trên bo mạch.

---

## III. Sơ đồ khối và Trạng thái hệ thống

### 1. Các trạng thái (States)
* **`STATE_NORMAL`**: Trạng thái hoạt động mặc định sau khi cấp nguồn hoặc khi thoát khỏi các chế độ cài đặt. Tại đây, màn hình hiển thị giờ và phút thực tế theo định dạng HH.MM.
* **`STATE_SET_HOUR`**: Trạng thái cài đặt giờ thực tế. LED hiển thị giờ (HH) sẽ nhấp nháy để báo hiệu người dùng có thể điều chỉnh.
* **`STATE_SET_MINUTE`**: Trạng thái cài đặt phút thực tế. LED hiển thị phút (MM) sẽ nhấp nháy trong suốt quá trình này.
* **`STATE_SET_ALARM_HOUR`**: Trạng thái cài đặt giờ cho báo thức (Alarm). LED hiển thị giờ sẽ nhấp nháy và LED chỉ báo (D4/D6) bắt đầu hoạt động.
* **`STATE_SET_ALARM_MINUTE`**: Trạng thái cài đặt phút cho báo thức. LED hiển thị phút sẽ nhấp nháy.

### 2. Các tín hiệu điều khiển và chuyển trạng thái (Signals & Transitions)

* **Nút Setup (SW3):**
    * Chuyển từ `STATE_NORMAL` sang `STATE_SET_HOUR`.
    * Chuyển từ `STATE_SET_HOUR` sang `STATE_SET_MINUTE`.
    * Chuyển từ `STATE_SET_MINUTE` quay về `STATE_NORMAL`.
* **Nút Hẹn giờ (SW16):**
    * Chuyển từ `STATE_NORMAL` sang `STATE_SET_ALARM_HOUR`.
    * Chuyển từ `STATE_SET_ALARM_HOUR` sang `STATE_SET_ALARM_MINUTE`.
    * Chuyển từ `STATE_SET_ALARM_MINUTE` quay về `STATE_NORMAL` và thực hiện ghi dữ liệu vào EEPROM.
* **Nút + và - (SW6 || SW10):**
    * Thực hiện tăng hoặc giảm giá trị giờ/phút tại các trạng thái cài đặt tương ứng.
    * Các nút này không làm thay đổi trạng thái hiện tại (giữ nguyên tại vòng lặp của state đó).
* **Timeout_30s:**
    * Tín hiệu tự động được kích hoạt sau 30 giây nếu không có bất kỳ thao tác phím nào.
    * Đưa hệ thống từ bất kỳ trạng thái cài đặt nào (`SET_HOUR`, `SET_MINUTE`, `SET_ALARM_HOUR`, `SET_ALARM_MINUTE`) về thẳng `STATE_NORMAL`.
* **Start:** Tín hiệu khởi tạo ngay khi cấp nguồn, đưa MCU vào `STATE_NORMAL` và đọc dữ liệu từ EEPROM.
  
<img width="702" height="539" alt="image" src="https://github.com/user-attachments/assets/727465d2-6948-4718-ba0c-ff254605b5ec" />


## IV. Kế hoạch kiểm thử & Các trường hợp biên (Test Plan & Edge Cases)

### A. Biên đếm thời gian tự động (Time Rollover)

| ID | Kịch bản Test | Hành động | Kết quả mong đợi | Trạng thái | Chức năng tham chiếu |
| :--- | :--- | :--- | :--- | :--- | :--- |
| **TC_01** | Chuyển phút | Ban đầu đồng hồ ở 00.00, đợi đến giây 59 | Sau 1 phút thực tế, led hiển thị phút tăng lên 1 | Pass | 1.2 |
| **TC_02** | Chuyển phút (Edge) | Đợi đồng hồ chạy đến XX:59 và qua giây 59 | Phút reset về 00, Giờ tăng lên 1. | Pass | 1.2 |
| **TC_03** | Chuyển ngày (Biên cực đại) | Set đồng hồ chạy đến 23:59 và qua giây 59. | Đồng hồ reset về 00:00. | Pass | 1.3 |
| **TC_04** | Khởi động lại nguồn | Rút nguồn mạch và cắm lại. Giờ hẹn sẽ được đọc từ EPPROM lên giữ nguyên giá trị ban đầu. | Giờ hiện tại mất, reset về 00.00 | Pass | 1.1, 3.2 |

### B. Biên cài đặt bằng phím tay (Manual Setting Rollover)

| ID | Kịch bản Test | Hành động | Kết quả mong đợi | Trạng thái | Chức năng tham chiếu |
| :--- | :--- | :--- | :--- | :--- | :--- |
| **TC_05** | Tăng phút vượt quá 59 | Ở chế độ Cài phút (SW3), bấm SW6 liên tục khi đang ở 59. | Phút nhảy từ 59 về 00. Còi kêu "pip" 0.3s mỗi lần bấm | Pass | 2.1, 4.1 |
| **TC_06** | Giảm phút dưới 0 | Ở chế độ Cài phút (SW3), bấm SW10 khi đang ở 00. | Phút nhảy lùi từ 00 lên 59. | Pass | 2.1 |
| **TC_07** | Tăng giờ vượt quá 23 | Ở chế độ Cài giờ, bấm SW6 khi đang ở 23. | Giờ nhảy từ 23 về 00. | Pass | 2.1 |
| **TC_08** | Giảm giờ dưới 0 | Ở chế độ Cài giờ, bấm SW10 khi đang ở 00. | Giờ nhảy lùi từ 00 lên 23. | Pass | 2.1 |

### C. Xung đột trạng thái & Luồng thực thi (State Collisions & Concurrency)

| ID | Kịch bản Test | Hành động | Kết quả mong đợi | Trạng thái | Chức năng tham chiếu |
| :--- | :--- | :--- | :--- | :--- | :--- |
| **TC_09** | Xung đột nút chức năng | Đang ở chế độ nhấp nháy cài giờ thực (ấn SW3), bất ngờ ấn nút hẹn giờ (SW16). | Hệ thống bỏ qua nút SW16, giữ nguyên chế độ hiện tại | Pass | 2.2 |
| **TC_10** | Chạy ngầm thời gian | Set đồng hồ là 10:59. Ấn SW16 để vào cài Alarm, đứng yên đợi 2 phút, sau đó thoát ra. | Màn hình chính phải hiện 11:01. Chứng minh thời gian vẫn chạy ngầm khi đang ở chế độ cài đặt. | Pass | 1.4, 3.1 |
| **TC_11** | Timeout 30 giây | Vào chế độ chỉnh giờ (SW3) hoặc hẹn giờ (SW16), bỏ tay ra không bấm nút nào trong 30 giây. | Tự động thoát về chế độ thường. Còi kêu "pip" 0.3s. | Pass | 4.1, 4.2 |
| **TC_12** | Timeout có can thiệp | Vào chế độ chỉnh giờ, đợi 25 giây, sau đó bấm SW6 (+). Đợi thêm 10 giây nữa. | Màn hình KHÔNG tự thoát ở giây thứ 30. Biến đếm 30s phải được reset mỗi khi có nút bấm mới. | Pass | 4.2 |

### D. Chuông báo (Alarm Actions)

| ID | Kịch bản Test | Hành động | Kết quả mong đợi | Trạng thái | Chức năng tham chiếu |
| :--- | :--- | :--- | :--- | :--- | :--- |
| **TC_13** | Kích hoạt chuông đúng giờ | Set giờ thực và giờ Alarm trùng khớp nhau (HH:MM == Alarm_HH:Alarm_MM). | Còi kêu pip-pip liên tục trong 5 giây (0.5s ON, 0.5s OFF). | Pass | 3.3 |
| **TC_14** | Bấm phím khi đang reo chuông | Đang lúc chuông reo (trong 5 giây đó), bấm bất kỳ nút nào (ví dụ SW3 hoặc SW6). | Chuông đang reo hẹn giờ, khi có nút bấm thì sẽ dừng reo chuông và kêu bíp 0.3s | Pass | 4.1, 4.3 |

### E. Xử lý ngoại lệ phím bấm cơ học (Hardware & Debounce)

| ID | Kịch bản Test | Hành động | Kết quả mong đợi | Trạng thái | Chức năng tham chiếu |
| :--- | :--- | :--- | :--- | :--- | :--- |
| **TC_15** | Giữ chặt phím | Đang ở chế độ chỉnh giờ, bấm và giữ nguyên nút SW6 (+) trong 5 giây. | Giờ chỉ tăng đúng 1 đơn vị rồi dừng lại (Nhờ kỹ thuật bắt sườn lên trong KeyScan()). Hệ thống không được tăng giờ liên tục (Auto-repeat). | Pass | Giao diện đầu vào (Inputs) |
| **TC_16** | Bấm nhiều phím cùng lúc | Đang ở chế độ thường, dùng 2 ngón tay ấn mạnh SW3 và SW16 cùng một lúc. | Hệ thống ưu tiên xử lý 1 phím theo thứ tự quét của phần cứng, hoặc bỏ qua để tránh nhiễu. Không được treo MCU. | Pass | Giao diện đầu vào (Inputs) |
| **TC_17** | Nhấn phím cực nhanh | Bấm nhấp nhả nút SW6 liên tục với tốc độ cao nhất có thể. | Số lần giờ tăng lên phải tương ứng với số lần bấm hợp lệ (đã qua màng lọc debounce 50 chu kỳ). MCU không bị đơ hoặc reset. | Pass | Giao diện đầu vào (Inputs), 4.1 |
