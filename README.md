# BuiKimHoang_24IT076_midterm

- Họ tên: Bùi Kim Hoàng  
- MSSV: 24IT076  
- Repository: https://github.com/Hoang066666/BuiKimHoang_24IT076_midterm  
- Dự án giữa kỳ: Triển khai lệnh `ls(1)` trên UNIX (NetBSD 10.1)  

---

## Tính năng đã triển khai

Chương trình hỗ trợ toàn bộ các tùy chọn theo man page NetBSD ls(1):

| Tùy chọn | Mô tả |
|----------|-------|
| `-A` | Liệt kê tất cả trừ `.` và `..` |
| `-a` | Bao gồm file ẩn (bắt đầu bằng `.`) |
| `-c` | Dùng ctime để sắp xếp/hiển thị |
| `-d` | Liệt kê thư mục như file thường |
| `-F` | Thêm ký hiệu loại file (`/`, `*`, `@`, `=`, `\|`) |
| `-f` | Không sắp xếp (bao hàm `-a`) |
| `-h` | Kích thước dạng người đọc được (1.5K, 2M...) |
| `-i` | Hiển thị inode number |
| `-k` | Kích thước tính theo kilobytes |
| `-l` | Long format (mode, links, owner, group, size, time, name) |
| `-n` | Như `-l` nhưng dùng UID/GID số thay vì tên |
| `-q` | Thay ký tự không in được bằng `?` |
| `-R` | Đệ quy vào các thư mục con |
| `-r` | Đảo ngược thứ tự sắp xếp |
| `-S` | Sắp xếp theo kích thước (lớn nhất trước) |
| `-s` | Hiển thị số block thực tế sử dụng |
| `-t` | Sắp xếp theo thời gian chỉnh sửa |
| `-u` | Dùng atime để sắp xếp/hiển thị |
| `-w` | In nguyên ký tự không in được |

---

## Cấu trúc dự án

```
.
├── main.c       # Entry point, phân tách operand file/dir
├── options.c/h  # Parse tùy chọn dòng lệnh
├── fileinfo.c/h # lstat, mode string, time selection
├── format.c/h   # Human-readable size, block count, print_name
├── sort.c/h     # Sắp xếp theo tên/kích thước/thời gian
├── display.c/h  # In danh sách, long format, đệ quy
├── ls.h         # Struct Options, FileEntry, global g_opts
├── Makefile
├── README.md
└── .gitignore
```

---

## Cách biên dịch

```bash
make
```

Yêu cầu: gcc, hệ thống UNIX/Linux (hoặc macOS).

## Cách chạy

```bash
# Liệt kê thư mục hiện tại
./ls

# Long format, hiển thị file ẩn
./ls -la

# Đệ quy, sắp xếp theo kích thước, human-readable
./ls -Rh -S /path/to/dir

# Nhiều đường dẫn
./ls -l file1.txt /home /tmp
```

## Ghi chú

- Không xảy ra segfault khi gặp symlink vòng lặp (dừng ở lstat).
- Biến môi trường `BLOCKSIZE` và `TZ` được hỗ trợ.
- Tùy chọn xung đột (`-c`/`-u`, `-l`/`-n`, `-R`/`-d`, `-q`/`-w`, `-h`/`-k`) tuân theo quy tắc "tùy chọn cuối cùng thắng".
