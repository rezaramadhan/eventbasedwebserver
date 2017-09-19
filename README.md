# Event-based Webserver
## nginx vs apache2 10K Problem Comparison
### Environment
Eksperimen dilakukan pada sebuah droplet DigitalOcean dengan tier paling rendah dengan spesifikasi sebagai berikut:
-   RAM: 2GB
-   CPU: Intel(R) Xeon(R) CPU E5-2630L 2 Core @ 2.40GHz
-   Disk: 20GB SSD
-   OS: Ubuntu 16.04 x86_64 kernel 4.4.0-93-generic

Tools benchmark yang digunakan adalah Apache Bench yang terdapat pada package apache2-utils. Benchmark akan dijalankan dalam server yang berbeda dengan webserver. Kedua server tersebut memiliki sebuah private network yang sama sehingga diharapkan latency yang ada akan minimum.

Benchmark akan dijalankan dalam dua session berbeda, satu session bertujuan untuk mengakses file berukuran 500B dan satu session untuk file berukuran 20KB. Untuk membuat hasil eksperimen lebih dapat diterima secara statistik, akan dilakukan 3 kali benchmark dalam satu session.

Command yang digunakan dalam benchmark adalah:
```
ab -r -n 10000 -c 10000 http://10.130.34.183:80/small.html
ab -r -n 10000 -c 10000 http://10.130.34.183:80/big.html
```
Dengan small.html adalah file berukuran 500B dan big.html adalah file berukuran 20KB. Kedua file tersebut dapat dilihat pada direktori `resources/files/`

Kami melakukan sedikit tuning dengan bantuan guide resmi dari dokumentasi [apache2](https://httpd.apache.org/docs/current/misc/perf-tuning.html) dan [nginx](https://www.nginx.com/blog/tuning-nginx/). Kami menggunakan module mpm_prefork pada apache2 untuk membuat perbandingan antara apache2 dan nginx lebih signifikan

### Data
Data detail untuk setiap eksperimen yang dilakukan dapat dilihat pada direktori `resources/experiment/data`. Pada readme ini akan diberikan nilai rata-rata untuk beberapa variabel penting seperti persentasi request yang berhasil dilayani, jumlah request per second, waktu maksimum, minumum, dan rata-rata yang dihabiskan oleh setiap request, serta penggunaan memori pada server ketika eksperimen berlangsung.

#### 500B file
Parameters            | nginx         | apache2        
--------------------- |:-------------:|:-------------:
req/sec               | 621.82        | 834,347        
connection time: max  | 35414.667 ms  | 45732 ms       
connection time: min  | 1 ms          | 1.333 ms       
connection time: avg  | 959,333 ms    | 2345.333 ms    
max memory usage      | 126 MB        | 174 MB          

#### 20KB file
Parameters            | nginx         | apache2       
--------------------- |:-------------:|:------------:
req/sec               | 505.833       | 444.743        
connection time: max  | 37125 ms      | 68527 ms       
connection time: min  | 1 ms          | 1.667 ms       
connection time: avg  | 1402.667 ms   | 868.333 ms     
max memory usage      | 135 MB        | 205 MB         

## Event-based Webserver Implementation
### How to Install
Program kami hanya menggunakan library standar C, untuk melakukan kompilasi jalankan perintah berikut:

```
make
```

Atau dapat menggunakan juga perintah gcc standar:

```
gcc -o server server.c
```

### Configuration Used
Program yang kami buat akan membaca file konfigurasi default pada `resources/server.conf`. File konfigurasi tersebut harus berisi file dengan format
```
<port> <index_directory>
```
dengan port adalah port tempat aplikasi tersebut berjalan dan index_directory adalah direktori root dimana server akan membaca file untuk menjawab request.

### Experiment: 10K problem
Percobaan yang dilakukan pada bagian ini akan dilakukan dalam Environment yang sama dengan percobaan sebelumnya pada nginx dan apache. Data mentah hasil percobaan dapat dilihat pada `resources/experiment/data/[small|big]/myserver/`.
Berikut merupakan hasil perbandingan antara server yang kami buat dengan nginx dan apache2.

#### 500B file
Parameters            | nginx         | apache2       | myserver
--------------------- |:-------------:|:-------------:|:-------------:
req/sec               | 621.82        | 834,347       | 2078
connection time: min  | 1 ms          | 1.333 ms      | 10
connection time: max  | 35414.667 ms  | 45732 ms      | 4221.333
connection time: avg  | 959,333 ms    | 2345.333 ms   | 1443.667
max memory usage      | 126 MB        | 174 MB        | 164

#### 20KB file
Parameters            | nginx         | apache2       | myserver
--------------------- |:-------------:|:------------: |:-------------:
req/sec               | 505.833       | 444.743       | 322.49
connection time: min  | 1 ms          | 1.667 ms      | 13.333 ms
connection time: max  | 37125 ms      | 68527 ms      | 30915.667 ms
connection time: avg  | 1402.667 ms   | 868.333 ms    | 3087 ms
max memory usage      | 135 MB        | 205 MB        | 239 MB
