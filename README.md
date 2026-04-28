Lập trình xây dựng thiết bị đồng hồ số dựa trên board SN32F407
I. ĐẶC TẢ YÊU CẦU DỰ ÁN ĐỒNG HỒ SỐ (SYSTEM SPECIFICATION)
1. Hiển thị & Bộ đếm thời gian
•	4 LED 7SEG sẽ thể hiện giờ và phút ở chế độ thường theo định dạng: HH.MM.
•	Ngay sau khi cấp nguồn, mạch hoạt động ở chế độ thường, giờ và phút được reset về giá trị 0: LED 7SEG sẽ hiện thị 00.00.
•	Bộ đếm sẽ đếm, sau mỗi phút sẽ hiện thị phút + 1, đến phút thứ 60 sẽ reset phút về 0 và tăng giờ lên 1. Giờ sẽ được hiện thị từ 00 đến 23.
•	Trường hợp biên: Khi đồng hồ chạy đến 23:59 và qua giây 59, đồng hồ tự động reset về 00.00. Thời gian thực (giây ngầm) vẫn tiếp tục chạy liên tục không ngừng kể cả khi người dùng đang ở trong các chế độ Setup hoặc Hẹn giờ.
2. NÚT SETUP (SW3)
•	Khi đang ở chế độ thường, ấn SW3 để vào chế độ thay đổi giờ: LED 7SEG HH sẽ nhấp nháy với chu kỳ 1s (0.5s ON – 0.5s OFF).
•	Khi đang ở chế độ thay đổi giờ, ấn SW3 để vào chế độ thay đổi phút: LED 7SEG MM sẽ nhấp nháy với chu kỳ 1s (0.5s ON – 0.5s OFF).
•	Khi đang ở chế độ thay đổi phút, ấn SW3 để vào chế độ thường.
•	Trường hợp biên: Khi đang ở chế độ SETUP, phím HẸN GIỜ (SW16) sẽ bị khóa (vô hiệu hóa) để tránh xung đột trạng thái.
3. NÚT HẸN GIỜ (SW16)
•	Khi đang ở chế độ thường, ấn SW16 để vào chế độ thay đổi giờ hẹn: LED 7SEG HH sẽ nhấp nháy với chu kỳ 1s (0.5s ON – 0.5s OFF).
•	Khi đang ở chế độ thay đổi giờ hẹn, ấn SW16 để vào chế độ thay đổi phút hẹn: LED 7SEG MM sẽ nhấp nháy với chu kỳ 1s (0.5s ON – 0.5s OFF).
•	Khi đang ở chế độ thay đổi phút hẹn, ấn SW16 để ghi giờ phút hẹn vào EEPROM và thoát ra chế độ thường.
•	Trường hợp biên: Khi đang ở chế độ HẸN GIỜ, phím SETUP (SW3) sẽ bị khóa.
4. NÚT + (SW6) & NÚT - (SW10)
•	NÚT + (SW6): Khi đang ở chế độ thay đổi giờ/giờ hẹn, ấn SW6 để tăng giờ lên 1 đơn vị, khi giờ tăng đến 24 sẽ reset giờ về 0. Khi đang ở chế độ thay đổi phút/phút hẹn, ấn SW6 để tăng phút lên 1 đơn vị, khi phút tăng đến 60 sẽ reset phút về 0.
•	NÚT - (SW10): Khi đang ở chế độ thay đổi giờ/giờ hẹn, ấn SW10 để giảm giờ xuống 1 đơn vị, khi giờ = 0 mà giảm 1 đơn vị sẽ đặt giờ = 23. Khi đang ở chế độ thay đổi phút/phút hẹn, ấn SW10 để giảm phút xuống 1 đơn vị, khi phút = 0 mà giảm 1 đơn vị sẽ đặt phút = 59.
•	Trường hợp biên: Việc ấn giữ các phím SW6/SW10 sẽ chỉ được tính là 1 lần nhấn hợp lệ (chống dội phím bắt sườn), không có hiện tượng số nhảy liên tục (auto-repeat).
5. Còi (BUZZER)
•	Mỗi khi ấn các nút SW3, SW6, SW10, SW16 thì còi sẽ kêu pip trong 0.3s.
•	Khi đồng hồ chạy đến giờ hẹn (trùng khớp cả HH và MM), còi sẽ kêu pip-pip liên tục trong 5s với chu kỳ (0.5s ON – 0.5s OFF).
•	Khi hết thời gian timeout thoát khỏi chế độ thay đổi giờ/phút (thực hoặc hẹn) để về chế độ thường thì còi sẽ kêu pip trong 0.3s.
•	Trường hợp biên: Nếu người dùng ấn bất kỳ nút nào trong lúc chuông đang reo báo thức 5s, chuông sẽ lập tức bị ngắt.
6. LED (D4 / D6)
•	Ở chế độ thay đổi giờ hẹn hoặc chế độ thay đổi phút hẹn thì LED sẽ nhấp nháy với chu kỳ 1s (0.5s ON – 0.5s OFF).
•	Ngoài ra LED sẽ ở trạng thái OFF.
7. EEPROM
•	Lưu giờ hẹn và phút hẹn, giây hẹn mặc định = 0.
•	Trường hợp biên: Dữ liệu giờ hẹn được tự động đọc từ EEPROM lên ngay khi MCU vừa cấp nguồn (khởi động). Nếu EEPROM trống (chip trắng), giờ hẹn tự động gán là 00.00.
8. Timeout khi không có sự kiện bấm nút
•	Ở chế độ thay đổi giờ/phút hoặc chế độ thay đổi giờ/phút hẹn, trong vòng 30s không có sự kiện ấn nút thì sẽ tự động thoát ra chế độ thường.
•	Trường hợp biên: Bộ đếm 30s này sẽ được reset lại từ đầu mỗi khi hệ thống ghi nhận có một thao tác bấm nút hợp lệ.
II. Sơ đồ khối
