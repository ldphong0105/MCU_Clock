# [cite_start]Lập trình xây dựng thiết bị đồng hồ số dựa trên board SN32F407 [cite: 1]

## [cite_start]I. Chỉ tiêu kỹ thuật (System Specifications) [cite: 2]

| Thành phần | Đặc tả kỹ thuật |
| :--- | :--- |
| **Giao diện đầu vào (Inputs)** | 4 x Nút nhấn (SW3, SW6, SW10, SW16). [cite_start]Tích hợp chống dội phím (Debounce) và bắt sườn tín hiệu (Edge Detection). [cite: 3] |
| **Giao diện đầu ra (Outputs)** | - [cite_start]04 x LED 7 đoạn (Quét đa hợp).<br>- 01 x Còi báo (Active Buzzer).<br>- 01 x LED đơn chỉ báo trạng thái (D6). [cite: 3] |
| **Định dạng thời gian** | [cite_start]24 giờ (00:00 - 23:59). [cite: 3] |
| **Độ phân giải hiệu chỉnh** | [cite_start]1 đơn vị (Giờ/Phút) cho mỗi lần nhấn phím. [cite: 3] |
| **Bộ nhớ phi tuyến** | [cite_start]EEPROM tích hợp, hỗ trợ đọc/ghi thông số báo thức. [cite: 3] |
| **Cơ chế bảo vệ phần mềm** | - [cite_start]Khóa chéo phím chức năng (State Interlock).<br>- Bộ đếm thời gian chờ tự động (Auto-exit Timer 30s). [cite: 3] |
| **Điều kiện khởi tạo** | [cite_start]Tự động Reset về 00.00 và nạp dữ liệu từ EEPROM ngay khi cấp nguồn. [cite: 3] |

---

## II. [cite_start]Danh mục các chức năng của đồng hồ (System Features) [cite: 4]

### [cite_start]1. Chức năng hiển thị và vận hành thời gian thực [cite: 5]
* [cite_start]**Hiển thị mặc định:** Hệ thống sử dụng 4 LED 7 đoạn để biểu diễn thời gian theo định dạng giờ và phút (HH.MM)[cite: 6].
* [cite_start]**Đếm giờ tự động:** Bộ đếm tự động tăng phút sau mỗi 60 giây và tăng giờ sau mỗi 60 phút (từ 00 đến 23 giờ)[cite: 7].
* [cite_start]**Xử lý tràn thời gian:** Khi đạt ngưỡng 23:59:59, hệ thống tự động quay về giá trị 00:00[cite: 8].
* [cite_start]**Duy trì thời gian ngầm:** Đảm bảo thời gian thực liên tục vận hành chính xác ngay cả khi người dùng đang thao tác trong menu cài đặt hoặc báo thức[cite: 9].

### [cite_start]2. Chức năng tùy chỉnh thời gian hệ thống (Manual Setup) [cite: 10]
* [cite_start]**Hiệu chỉnh giờ và phút:** Cho phép thay đổi giá trị thời gian thông qua chế độ nhấp nháy (chu kỳ 1s) để người dùng dễ dàng nhận diện đối tượng đang điều chỉnh[cite: 11].
* [cite_start]**Bảo vệ trạng thái:** Tự động khóa phím Hẹn giờ khi đang trong chế độ Setup để tránh các xung đột logic trong phần mềm[cite: 12].

### [cite_start]3. Chức năng báo thức và lưu trữ (Alarm & Storage) [cite: 13]
* [cite_start]**Thiết lập báo thức:** Người dùng có thể cài đặt giờ/phút hẹn giờ với chỉ báo đèn LED nhấp nháy[cite: 14].
* [cite_start]**Lưu trữ vĩnh viễn:** Giá trị báo thức được ghi vào EEPROM, cho phép hệ thống tự động khôi phục dữ liệu hẹn giờ ngay sau khi khởi động lại nguồn[cite: 15].
* [cite_start]**Thông báo âm thanh:** Còi báo (Buzzer) phát tín hiệu pip-pip liên tục trong 5 giây khi thời gian thực trùng khớp với giờ hẹn[cite: 16].

### [cite_start]4. Chức năng tương tác và phản hồi người dùng [cite: 17]
* [cite_start]**Phản hồi phím bấm:** Phát tín hiệu âm thanh ngắn (0.3s) mỗi khi có tác động vào phím bấm hợp lệ[cite: 18].
* [cite_start]**Tự động thoát chế độ (Timeout):** Hệ thống tự động chuyển về màn hình chính nếu không nhận được thao tác nào trong vòng 30 giây tại các menu cài đặt[cite: 19].
* [cite_start]**Hủy báo thức nhanh:** Cho phép ngắt chuông báo ngay lập tức bằng cách ấn bất kỳ phím nào trên bo mạch[cite: 20].

---

## III. [cite_start]Sơ đồ khối và Trạng thái hệ thống [cite: 21]

### [cite_start]1. Các trạng thái (States) [cite: 22]
* [cite_start]**`STATE_NORMAL`**: Trạng thái hoạt động mặc định sau khi cấp nguồn hoặc khi thoát khỏi các chế độ cài đặt[cite: 23]. [cite_start]Tại đây, màn hình hiển thị giờ và phút thực tế theo định dạng HH.MM[cite: 24].
* [cite_start]**`STATE_SET_HOUR`**: Trạng thái cài đặt giờ thực tế[cite: 25]. [cite_start]LED hiển thị giờ (HH) sẽ nhấp nháy để báo hiệu người dùng có thể điều chỉnh[cite: 25].
* [cite_start]**`STATE_SET_MINUTE`**: Trạng thái cài đặt phút thực tế[cite: 26]. [cite_start]LED hiển thị phút (MM) sẽ nhấp nháy trong suốt quá trình này[cite: 26].
* [cite_start]**`STATE_SET_ALARM_HOUR`**: Trạng thái cài đặt giờ cho báo thức (Alarm)[cite: 27]. [cite_start]LED hiển thị giờ sẽ nhấp nháy và LED chỉ báo (D4/D6) bắt đầu hoạt động[cite: 28].
* [cite_start]**`STATE_SET_ALARM_MINUTE`**: Trạng thái cài đặt phút cho báo thức[cite: 29]. [cite_start]LED hiển thị phút sẽ nhấp nháy[cite: 29].

### [cite_start]2. Các tín hiệu điều khiển và chuyển trạng thái (Signals & Transitions) [cite: 30]

* **Nút Setup (SW3)[cite: 31]:**
    * [cite_start]Chuyển từ `STATE_NORMAL` sang `STATE_SET_HOUR`[cite: 32].
    * Chuyển từ `STATE_SET_HOUR` sang `STATE_SET_MINUTE`[cite: 33].
    * [cite_start]Chuyển từ `STATE_SET_MINUTE` quay về `STATE_NORMAL`[cite: 34].
* **Nút Hẹn giờ (SW16)[cite: 35]:**
    * [cite_start]Chuyển từ `STATE_NORMAL` sang `STATE_SET_ALARM_HOUR`[cite: 36].
    * [cite_start]Chuyển từ `STATE_SET_ALARM_HOUR` sang `STATE_SET_ALARM_MINUTE`[cite: 37].
    * Chuyển từ `STATE_SET_ALARM_MINUTE` quay về `STATE_NORMAL` và thực hiện ghi dữ liệu vào EEPROM[cite: 38].
* [cite_start]**Nút + và - (SW6 || SW10)[cite: 39]:**
    * [cite_start]Thực hiện tăng hoặc giảm giá trị giờ/phút tại các trạng thái cài đặt tương ứng[cite: 40].
    * Các nút này không làm thay đổi trạng thái hiện tại (giữ nguyên tại vòng lặp của state đó)[cite: 41].
* [cite_start]**Timeout_30s[cite: 42]:**
    * Tín hiệu tự động được kích hoạt sau 30 giây nếu không có bất kỳ thao tác phím nào[cite: 43].
    * [cite_start]Đưa hệ thống từ bất kỳ trạng thái cài đặt nào (`SET_HOUR`, `SET_MINUTE`, `SET_ALARM_HOUR`, `SET_ALARM_MINUTE`) về thẳng `STATE_NORMAL`[cite: 44].
* [cite_start]**Start [cite: 45][cite_start]:** * Tín hiệu khởi tạo ngay khi cấp nguồn, đưa MCU vào `STATE_NORMAL` và đọc dữ liệu từ EEPROM[cite: 45].

*(Lưu ý: Bạn có thể chèn hình ảnh sơ đồ khối vào đây bằng cú pháp `![Sơ đồ khối](link_ảnh_của_bạn)` trên GitHub).*

---

## IV. [cite_start]Kế hoạch kiểm thử & Các trường hợp biên (Test Plan & Edge Cases) [cite: 46]

### [cite_start]A. Biên đếm thời gian tự động (Time Rollover) [cite: 47]

| ID | Kịch bản Test | Hành động | Kết quả mong đợi | Trạng thái | Chức năng tham chiếu |
| :--- | :--- | :--- | :--- | :--- | :--- |
| **TC_01** | Chuyển phút | Ban đầu đồng hồ ở 00.00, đợi đến giây 59 | Sau 1 phút thực tế, led hiển thị phút tăng lên 1 | Pass | [cite_start]1.2 [cite: 48] |
| **TC_02** | Chuyển phút (Edge) | Đợi đồng hồ chạy đến XX:59 và qua giây 59 | Phút reset về 00, Giờ tăng lên 1. | Pass | [cite_start]1.2 [cite: 48] |
| **TC_03** | Chuyển ngày (Biên cực đại) | Set đồng hồ chạy đến 23:59 và qua giây 59. | Đồng hồ reset về 00:00. | Pass | [cite_start]1.3 [cite: 48] |
| **TC_04** | Khởi động lại nguồn | Rút nguồn mạch và cắm lại. Giờ hẹn sẽ được đọc từ EPPROM lên giữ nguyên giá trị ban đầu. | Giờ hiện tại mất, reset về 00.00 | Pass | [cite_start]1.1, 3.2 [cite: 48] |

### [cite_start]B. Biên cài đặt bằng phím tay (Manual Setting Rollover) [cite: 49]

| ID | Kịch bản Test | Hành động | Kết quả mong đợi | Trạng thái | Chức năng tham chiếu |
| :--- | :--- | :--- | :--- | :--- | :--- |
| **TC_05** | Tăng phút vượt quá 59 | Ở chế độ Cài phút (SW3), bấm SW6 liên tục khi đang ở 59. | Phút nhảy từ 59 về 00. Còi kêu "pip" 0.3s mỗi lần bấm | Pass | [cite_start]2.1, 4.1 [cite: 50] |
| **TC_06** | Giảm phút dưới 0 | Ở chế độ Cài phút (SW3), bấm SW10 khi đang ở 00. | Phút nhảy lùi từ 00 lên 59. | Pass | [cite_start]2.1 [cite: 50] |
| **TC_07** | Tăng giờ vượt quá 23 | Ở chế độ Cài giờ, bấm SW6 khi đang ở 23. | Giờ nhảy từ 23 về 00. | Pass | [cite_start]2.1 [cite: 50] |
| **TC_08** | Giảm giờ dưới 0 | Ở chế độ Cài giờ, bấm SW10 khi đang ở 00. | Giờ nhảy lùi từ 00 lên 23. | Pass | [cite_start]2.1 [cite: 50] |

### [cite_start]C. Xung đột trạng thái & Luồng thực thi (State Collisions & Concurrency) [cite: 51]

| ID | Kịch bản Test | Hành động | Kết quả mong đợi | Trạng thái | Chức năng tham chiếu |
| :--- | :--- | :--- | :--- | :--- | :--- |
| **TC_09** | Xung đột nút chức năng | Đang ở chế độ nhấp nháy cài giờ thực (ấn SW3), bất ngờ ấn nút hẹn giờ (SW16). | Hệ thống bỏ qua nút SW16, giữ nguyên chế độ hiện tại | Pass | [cite_start]2.2 [cite: 52] |
| **TC_10** | Chạy ngầm thời gian | Set đồng hồ là 10:59. Ấn SW16 để vào cài Alarm, đứng yên đợi 2 phút, sau đó thoát ra. | Màn hình chính phải hiện 11:01. Chứng minh thời gian vẫn chạy ngầm khi đang ở chế độ cài đặt. | Pass | [cite_start]1.4, 3.1 [cite: 52] |
| **TC_11** | Timeout 30 giây | Vào chế độ chỉnh giờ (SW3) hoặc hẹn giờ (SW16), bỏ tay ra không bấm nút nào trong 30 giây. | Tự động thoát về chế độ thường. Còi kêu "pip" 0.3s. | Pass | [cite_start]4.1, 4.2 [cite: 52] |
| **TC_12** | Timeout có can thiệp | Vào chế độ chỉnh giờ, đợi 25 giây, sau đó bấm SW6 (+). Đợi thêm 10 giây nữa. | Màn hình KHÔNG tự thoát ở giây thứ 30. Biến đếm 30s phải được reset mỗi khi có nút bấm mới. | Pass | [cite_start]4.2 [cite: 52] |

### [cite_start]D. Chuông báo (Alarm Actions) [cite: 53]

| ID | Kịch bản Test | Hành động | Kết quả mong đợi | Trạng thái | Chức năng tham chiếu |
| :--- | :--- | :--- | :--- | :--- | :--- |
| **TC_13** | Kích hoạt chuông đúng giờ | Set giờ thực và giờ Alarm trùng khớp nhau (HH:MM == Alarm_HH:Alarm_MM). | Còi kêu pip-pip liên tục trong 5 giây (0.5s ON, 0.5s OFF). | Pass | [cite_start]3.3 [cite: 54] |
| **TC_14** | Bấm phím khi đang reo chuông | Đang lúc chuông reo (trong 5 giây đó), bấm bất kỳ nút nào (ví dụ SW3 hoặc SW6). | Chuông đang reo hẹn giờ, khi có nút bấm thì sẽ dừng reo chuông và kêu bíp 0.3s | Pass | [cite_start]4.1, 4.3 [cite: 54] |

### [cite_start]E. Xử lý ngoại lệ phím bấm cơ học (Hardware & Debounce) [cite: 55]

| ID | Kịch bản Test | Hành động | Kết quả mong đợi | Trạng thái | Chức năng tham chiếu |
| :--- | :--- | :--- | :--- | :--- | :--- |
| **TC_15** | Giữ chặt phím | Đang ở chế độ chỉnh giờ, bấm và giữ nguyên nút SW6 (+) trong 5 giây. | Giờ chỉ tăng đúng 1 đơn vị rồi dừng lại (Nhờ kỹ thuật bắt sườn lên trong KeyScan()). Hệ thống không được tăng giờ liên tục (Auto-repeat). | Pass | [cite_start]Giao diện đầu vào (Inputs) [cite: 56] |
| **TC_16** | Bấm nhiều phím cùng lúc | Đang ở chế độ thường, dùng 2 ngón tay ấn mạnh SW3 và SW16 cùng một lúc. | Hệ thống ưu tiên xử lý 1 phím theo thứ tự quét của phần cứng, hoặc bỏ qua để tránh nhiễu. Không được treo MCU. | Pass | [cite_start]Giao diện đầu vào (Inputs) [cite: 56] |
| **TC_17** | Nhấn phím cực nhanh | Bấm nhấp nhả nút SW6 liên tục với tốc độ cao nhất có thể. | Số lần giờ tăng lên phải tương ứng với số lần bấm hợp lệ (đã qua màng lọc debounce 50 chu kỳ). MCU không bị đơ hoặc reset. | Pass | [cite_start]Giao diện đầu vào (Inputs), 4.1 [cite: 56] |
