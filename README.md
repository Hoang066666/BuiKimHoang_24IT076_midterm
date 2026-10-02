# BÁO CÁO ĐỒ ÁN GIỮA KỲ – TRIỂN KHAI LỆNH `ls(1)` TRÊN UNIX (NetBSD)

- **Họ và tên sinh viên:** Bùi Kim Hoàng  
- **Mã số sinh viên (MSSV):** 24IT076  
- **Môn học:** Lập trình hệ thống / APUE (*Advanced Programming in the UNIX Environment*)  
- **Hệ điều hành thử nghiệm:** NetBSD 10.1 (amd64) trên máy ảo VirtualBox  
- **Kho lưu trữ GitHub (Repository):** [https://github.com/Hoang066666/BuiKimHoang_24IT076_midterm](https://github.com/Hoang066666/BuiKimHoang_24IT076_midterm)  

---

## 1. TỔNG QUAN ĐỀ TÀI

Lệnh `ls(1)` là một trong những tiện ích dòng lệnh nền tảng nhất của các hệ điều hành họ UNIX. Mục tiêu của đồ án này là **tự phát triển từ đầu (from scratch)** một phiên bản hoàn chỉnh của lệnh `ls(1)` bằng ngôn ngữ C (tiêu chuẩn C11 / POSIX.1-2008), tuân thủ chính xác đặc tả trong **NetBSD 10.1 General Commands Manual**.

Dự án giúp rèn luyện sâu sắc các kỹ năng lập trình hệ thống:
- Sử dụng các lời gọi hệ thống (System Calls) và thư viện POSIX: `lstat(2)`, `opendir(3)`, `readdir(3)`, `readlink(2)`, `getpwuid(3)`, `getgrgid(3)`, `isatty(3)`.
- Thao tác trực tiếp trên metadata của tệp (cấu trúc `struct stat`, `mode_t`, `time_t`, device numbers `major`/`minor`).
- Xử lý bộ nhớ động (`malloc`, `realloc`, `free`), giải phóng tài nguyên an toàn, ngăn chặn rò rỉ bộ nhớ (memory leak) và lỗi phân đoạn (segmentation fault).
- Module hóa dự án rõ ràng, quản lý biên dịch tự động qua `Makefile` và kiểm soát phiên bản bằng `git`.

---

## 2. KIẾN TRÚC MÃ NGUỒN VÀ MODULE HÓA

Toàn bộ dự án được thiết kế theo hướng module hóa chặt chẽ, mỗi module gồm một tệp tiêu đề (`.h`) định nghĩa giao diện và một tệp mã nguồn (`.c`) hiện thực chi tiết:

```
.
├── ls.h          # Định nghĩa cấu trúc Options, FileEntry và biến toàn cục
├── main.c        # Entry point: xử lý operands (file/dir), điều phối hiển thị
├── options.h     # Giao diện phân tích tham số dòng lệnh
├── options.c     # Phân tích options, xử lý xung đột cờ (last flag wins)
├── fileinfo.h    # Giao diện truy xuất metadata của file
├── fileinfo.c    # Tạo FileEntry qua lstat, mode string, type indicator (-F)
├── format.h      # Giao diện định dạng dữ liệu đầu ra
├── format.c      # Human-readable size (-h), block count (-s, BLOCKSIZE), print_name (-q/-w)
├── sort.h        # Giao diện sắp xếp danh sách file
├── sort.c        # Sắp xếp theo tên, kích thước (-S), thời gian (-t), đảo ngược (-r)
├── display.h     # Giao diện hiển thị danh mục và file
├── display.c     # Duyệt thư mục, long format (-l/-n), symlink target, đệ quy (-R)
├── Makefile      # Tự động hóa quá trình biên dịch (-Wall -Wextra -std=c11 -pedantic)
├── .gitignore    # Loại trừ tệp nhị phân 'ls' và các tệp đối tượng '*.o'
└── README.md     # Báo cáo kỹ thuật chi tiết của đồ án
```

### Chi tiết nhiệm vụ từng module:
1. **`ls.h`**:
   - `struct Options`: Chứa 19 trường cờ tương ứng với 19 tùy chọn dòng lệnh.
   - `struct FileEntry`: Đại diện cho một tệp hoặc thư mục, lưu trữ tên hiển thị (`name`), đường dẫn đầy đủ (`path`), và cấu trúc siêu dữ liệu (`struct stat st`).
   - `g_opts` & `g_exit_status`: Các biến toàn cục điều khiển trạng thái và mã trả về của tiện ích.
2. **`options.c / .h`**:
   - Hiện thực hàm `parse_options()`: duyệt qua `argv`, nhận diện cờ đơn và gộp cờ (như `-laF`), cờ `--` để kết thúc options, và áp dụng quy tắc ghi đè.
3. **`fileinfo.c / .h`**:
   - `make_entry()`: Cấp phát bộ nhớ, ghép đường dẫn và gọi `lstat()` (đảm bảo không tự động dereference symbolic link).
   - `mode_string()`: Chuyển đổi `st_mode` thành chuỗi 10 ký tự quyền chuẩn UNIX (bao gồm cả ký tự đặc biệt: `s`/`S` cho setuid/setgid, `t`/`T` cho sticky bit).
   - `get_time()`: Lựa chọn linh hoạt giữa `mtime` (mặc định), `ctime` (`-c`) hoặc `atime` (`-u`).
   - `type_indicator()`: Xác định ký tự phân loại cho `-F` (`/`, `*`, `@`, `=`, `|`).
4. **`format.c / .h`**:
   - `human_size()`: Chuyển đổi số byte thành chuỗi đọc được (`B`, `K`, `M`, `G`, `T`, `P`).
   - `block_count()`: Tính toán số block hệ thống tập tin, tự động nhận diện biến môi trường `BLOCKSIZE` hoặc cờ `-k` (1024 bytes).
   - `print_name()`: Xử lý ký tự không in được (`-q` đổi thành `?`, `-w` in thô; tự động nhận diện terminal qua `isatty`).
5. **`sort.c / .h`**:
   - Sử dụng hàm chuẩn `qsort(3)` với các hàm so sánh tùy biến: theo tên (lexicographical), theo dung lượng giảm dần (`cmp_size`), theo thời gian mới nhất (`cmp_time`). Khi hai file có cùng kích thước/thời gian, dùng tên file làm tiêu chí phân định (tie-breaker).
6. **`display.c / .h`**:
   - `list_directory()`: Đọc thư mục bằng `opendir` và `readdir`, lọc file ẩn, cấp phát mảng động `FileEntry**` tự động mở rộng dung lượng (`realloc`), thực hiện đệ quy sâu (`-R`).
   - `print_long()`: In định dạng chi tiết 7 cột, tra cứu UID/GID qua `getpwuid`/`getgrgid`, xử lý file thiết bị (in `major, minor`), hiển thị đích đến của symbolic link (`readlink`).
7. **`main.c`**:
   - Điểm vào của chương trình: phân tách các operand thành file thông thường và thư mục. Danh sách file operand được gom nhóm, sắp xếp theo đúng các cờ (`-S`, `-t`, `-r`) và hiển thị trước, sau đó mới duyệt đến các thư mục.

---

## 3. DANH SÁCH 19 TÙY CHỌN ĐÃ HIỆN THỰC

Chương trình hỗ trợ đầy đủ 19 tùy chọn theo đặc tả man page NetBSD 10.1 `ls(1)`:

| Tùy chọn | Tên chức năng | Mô tả chi tiết hành vi | Triển khai trong mã nguồn |
| :---: | :--- | :--- | :--- |
| **`-A`** | Almost all | Liệt kê tất cả các tệp, bao gồm tệp ẩn, nhưng loại bỏ `.` và `..`. Tự động bật cho super-user (root). | Bỏ qua khi `strcmp(name, ".") == 0` hoặc `".."` |
| **`-a`** | All | Liệt kê toàn bộ các tệp, bao gồm cả tệp ẩn bắt đầu bằng dấu chấm (`.`, `..`, `.gitignore`). | Cho phép tên bắt đầu bằng `.` đi qua bộ lọc |
| **`-c`** | Status change time | Sử dụng thời gian thay đổi trạng thái file (`ctime`) cho việc sắp xếp (`-t`) hoặc hiển thị (`-l`). Ghi đè `-u`. | `get_time()` trả về `st.st_ctime` |
| **`-d`** | Directory as file | Liệt kê thư mục như tệp thông thường (không duyệt vào nội dung thư mục). Ghi đè `-R`. | Kiểm tra trong `main.c`: đưa thư mục vào danh sách file thường |
| **`-F`** | File indicator | Thêm ký hiệu phân loại ngay sau tên tệp: `/` (thư mục), `*` (thực thi), `@` (symlink), `=` (socket), `\|` (FIFO). | Hàm `type_indicator()` áp dụng cho cả short và long format |
| **`-f`** | Fast (no sort) | Không sắp xếp kết quả (thứ tự đọc từ đĩa). Tự động kích hoạt cờ `-a`. | Bỏ qua `qsort` trong `sort_entries()`, gán `opt_a = 1` |
| **`-h`** | Human-readable | Hiển thị dung lượng file và tổng số block dưới dạng dễ đọc (ví dụ: `512B`, `1.5K`, `2.4M`). Ghi đè `-k`. | Hàm `human_size()` chuyển đổi lũy thừa 1024 |
| **`-i`** | Inode number | In số hiệu tệp tin (Inode number) trước thông tin mỗi tệp. | In `st.st_ino` qua `print_prefix()` |
| **`-k`** | Kilobytes | Hiển thị dung lượng và block count theo đơn vị Kilobytes (1024 bytes). Ghi đè `-h`. | Đặt block size cố định bằng 1024 |
| **`-l`** | Long format | Hiển thị chi tiết: mode quyền, số liên kết, owner, group, kích thước, thời gian, tên file và đích symlink. | Hàm `print_long()` phối hợp với `mode_string()` |
| **`-n`** | Numeric UID/GID | Như `-l`, nhưng hiển thị UID và GID dưới dạng số thay vì tra cứu tên chữ. Ghi đè `-l`. | Dùng trực tiếp `st.st_uid` và `st.st_gid` thay vì `getpwuid`/`getgrgid` |
| **`-q`** | Non-printable as ? | Thay thế tất cả các ký tự không in được trong tên tệp thành ký tự `?`. Là mặc định khi xuất ra terminal. | Hàm `print_name()` dùng `!isprint(c)` |
| **`-R`** | Recursive | Đệ quy duyệt qua tất cả các thư mục con bắt gặp. Ghi đè `-d`. | Gọi lại `list_directory()` trên các entry con là thư mục |
| **`-r`** | Reverse sort | Đảo ngược thứ tự sắp xếp (ngược bảng chữ cái, nhỏ nhất trước hoặc cũ nhất trước). | Đảo ngược mảng sau khi gọi `qsort` |
| **`-S`** | Sort by size | Sắp xếp tệp theo kích thước giảm dần (tệp lớn nhất đứng đầu). | Hàm `cmp_size` so sánh `st.st_size` |
| **`-s`** | Size in blocks | Hiển thị số block hệ thống tập tin thực tế được cấp phát cho mỗi tệp. | In `st.st_blocks` quy đổi theo block size |
| **`-t`** | Sort by time | Sắp xếp tệp theo thời gian sửa đổi gần nhất (mới nhất đứng đầu). | Hàm `cmp_time` so sánh thời gian từ `get_time()` |
| **`-u`** | Access time | Sử dụng thời gian truy cập gần nhất (`atime`) cho việc sắp xếp (`-t`) hoặc hiển thị (`-l`). Ghi đè `-c`. | `get_time()` trả về `st.st_atime` |
| **`-w`** | Raw non-printable | In nguyên bản ký tự không in được trong tên tệp. Là mặc định khi output không phải là terminal. | `print_name()` in trực tiếp byte ký tự |

---

## 4. XỬ LÝ CÁC TRƯỜNG HỢP NGOẠI LỆ VÀ QUY TẮC ĐẶC THÙ (EDGE CASES)

1. **Phòng chống vòng lặp vô hạn Symbolic Link khi đệ quy (`-R`):**
   - Chương trình sử dụng `lstat()` thay cho `stat()`. Nhờ đó, một symlink trỏ tới thư mục sẽ được định danh là `S_ISLNK` chứ không phải `S_ISDIR`, ngăn chặn hoàn toàn hiện tượng đệ quy lặp vô tận (infinite recursion loop).
   - Trong quá trình đệ quy, chương trình tuyệt đối bỏ qua hai thư mục ảo `.` và `..`.
2. **Quy tắc xung đột tùy chọn ("Last Flag Wins"):**
   - Các cặp cờ đối nghịch nhau tuân theo nguyên tắc cờ xuất hiện sau cùng sẽ chiến thắng:
     - `-c` và `-u` (ctime vs atime)
     - `-l` và `-n` (tên user/group vs UID/GID số)
     - `-d` và `-R` (chỉ xem thư mục như file vs duyệt đệ quy)
     - `-h` và `-k` (human-readable vs kilobytes)
     - `-q` và `-w` (ẩn ký tự lạ vs in thô)
     - `-a` và `-A` (bao gồm `.`/`..` vs loại trừ `.`/`..`)
3. **Quyền hạn đặc biệt trong Mode String:**
   - Hỗ trợ đầy đủ các cờ cấp phép nâng cao của UNIX:
     - Set-User-ID: hiển thị `s` nếu có quyền thực thi, `S` nếu không có quyền thực thi.
     - Set-Group-ID: hiển thị `s` nếu có quyền thực thi, `S` nếu không có quyền thực thi.
     - Sticky bit: hiển thị `t` nếu có quyền thực thi/search, `T` nếu không.
4. **Tệp thiết bị đặc biệt (Character / Block Devices):**
   - Khi gặp file thiết bị (`/dev/...`), cột kích thước sẽ tự động chuyển sang hiển thị cặp số thiết bị `major, minor` theo chuẩn hệ thống (sử dụng macro `major()`, `minor()`).
5. **Định dạng thời gian theo chuẩn 6 tháng:**
   - Nếu tệp có thời gian sửa đổi cũ hơn 6 tháng (~180 ngày) hoặc có thời gian ở tương lai, thời gian hiển thị theo định dạng `Mmm dd  yyyy` (ví dụ: `Dec 16  2024`).
   - Nếu tệp mới sửa đổi trong vòng 6 tháng, hiển thị dạng `Mmm dd hh:mm` (ví dụ: `Oct  2 07:15`).
6. **Hỗ trợ biến môi trường `BLOCKSIZE` và `TZ`:**
   - Chương trình đọc biến môi trường `BLOCKSIZE` (nếu không bật `-h` hoặc `-k`) để quy đổi số block chính xác theo yêu cầu người dùng.
   - Thư viện `localtime()` tự động nhận biết biến môi trường `TZ` để hiển thị ngày giờ chuẩn xác theo múi giờ địa phương.
7. **Xử lý mã thoát (Exit Status):**
   - Biến `g_exit_status` được duy trì xuyên suốt. Nếu có bất kỳ lỗi nào xảy ra (không tìm thấy file, thư mục bị cấm quyền `Permission denied`), chương trình xuất thông báo ra `stderr` và kết thúc với mã thoát `> 0` (mã 1), đúng theo tiêu chuẩn UNIX.

---

## 5. HƯỚNG DẪN BIÊN DỊCH VÀ CHẠY THỬ NGHIỆM

### 5.1. Yêu cầu môi trường
- Hệ điều hành: NetBSD 10.1 (khuyên dùng trên máy ảo APUE), Linux hoặc macOS.
- Trình biên dịch: `gcc` hoặc `clang` (hỗ trợ C11), công cụ `make`.

### 5.2. Lệnh biên dịch
```bash
# Biên dịch toàn bộ dự án
make

# Dọn dẹp các tệp nhị phân và file đối tượng (.o)
make clean
```

### 5.3. Các kịch bản chạy kiểm thử (Test Cases)

```bash
# 1. Liệt kê cơ bản thư mục hiện tại
./ls

# 2. Liệt kê chi tiết file ẩn và hiển thị ký hiệu loại file
./ls -laF

# 3. Xem dung lượng ở định dạng đọc được (K, M, G)
./ls -lh

# 4. Sắp xếp theo dung lượng giảm dần (file lớn nhất lên đầu)
./ls -lS

# 5. Sắp xếp theo thời gian truy cập gần nhất (atime)
./ls -ltu

# 6. Hiển thị UID/GID dạng số và số Inode
./ls -ni

# 7. Duyệt đệ quy toàn bộ thư mục con
./ls -R

# 8. Xem thư mục như file thường (không đi vào trong)
./ls -ld ..

# 9. Truyền nhiều tham số hỗn hợp (file và thư mục)
./ls -l Makefile main.c ..
```

### 5.4. Đối chiếu với lệnh `ls` gốc của hệ thống NetBSD
Trên máy ảo NetBSD 10.1, bạn có thể chạy so sánh trực tiếp kết quả đầu ra:
```bash
./ls -laF
/bin/ls -laF
```
Kết quả hiển thị về định dạng cột, căn lề, quyền hạn và kích thước hoàn toàn trùng khớp với tiện ích gốc của hệ điều hành.

---

## 6. KẾT LUẬN

Dự án đã hoàn thành **100% các mục tiêu và yêu cầu** đặt ra:
- Hiện thực đầy đủ và chính xác 19 cờ tùy chọn theo man page NetBSD 10.1.
- Mã nguồn được phân tách module mạch lạc, tường minh, có chú thích rõ ràng.
- Đảm bảo độ ổn định cao, không rò rỉ bộ nhớ, không xảy ra segmentation fault.
- Tuân thủ nghiêm ngặt các tiêu chuẩn quản lý mã nguồn Git, loại trừ toàn bộ tệp đối tượng và tệp thực thi theo đúng hướng dẫn nộp bài.
