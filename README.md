ID,Kịch bản Test,Hành động,Kết quả mong đợi,Trạng thái
TC_01,Chuyển phút,Đợi đồng hồ chạy đến XX:59 và qua giây 59,"Phút reset về 00, Giờ tăng lên 1.",Pass   
TC_02,Chuyển ngày (Biên cực đại),Set đồng hồ chạy đến 23:59 và qua giây 59.,Đồng hồ reset về 00:00.,Pass   
TC_03,Khởi động lại nguồn,Rút nguồn mạch và cắm lại. Giờ hẹn sẽ được đọc từ EPPROM lên giữ nguyên giá trị ban đầu.,"Giờ hiện tại mất, reset về 00.00",Pass   
