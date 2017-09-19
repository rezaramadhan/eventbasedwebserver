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
ab -r -n 20000 -c 10000 http://10.130.34.183:80/small.html
ab -r -n 20000 -c 10000 http://10.130.34.183:80/big.html
```
Dengan small.html adalah file berukuran 500B dan big.html adalah file berukuran 20KB. Kedua file tersebut dapat dilihat pada direktori `resources/files/`

Kami melakukan sedikit tuning dengan bantuan guide resmi dari dokumentasi [apache2](https://httpd.apache.org/docs/current/misc/perf-tuning.html) dan [nginx](https://www.nginx.com/blog/tuning-nginx/). Kami menggunakan module mpm_prefork pada apache2 untuk membuat perbandingan antara apache2 dan nginx lebih signifikan

### Data
Data detail untuk setiap eksperimen yang dilakukan dapat dilihat pada direktori `resources/experiment/data`. Pada readme ini akan diberikan nilai rata-rata untuk beberapa variabel penting seperti persentasi request yang berhasil dilayani, jumlah request per second, waktu maksimum, minumum, dan rata-rata yang dihabiskan oleh setiap request, serta penggunaan memori pada server ketika eksperimen berlangsung.
#### 500B file
Parameters            | nginx         | apache2
--------------------- |:-------------:| :-------:
req/sec               | 621.82              | 834,347
failed request        | 18228              | 133
connection time: max  | 35414.667 ms              | 45732 ms
connection time: min  | 1 ms              | 1.333 ms
connection time: avg  | 959,333 ms              | 2345.333 ms
max memory usage      | 126MB              | 174MB

#### 20KB file
Parameters            | nginx         | apache2
--------------------- |:-------------:| :-------:
req/sec               | 505.833              | 444.743
failed request        | 18836              | 1054
connection time: max  | 37125 ms              | 68527 ms
connection time: min  | 1 ms              | 1.667 ms
connection time: avg  | 1402.667 ms              | 868.333 ms
max memory usage      | 135MB               | 205MB

## Event-based Webserver Implementation
### How to Install
Untuk melakukan instalasi implementasi server, jalankan makefile dengan menjalankan perintah 'make' di terminal pada direktori server. Lalu, jalankan perintah './server' di terminal untuk menjalankan implementasi server.
### Configuration Used
Konfigurasi yang digunakan adalah port 8080 dan direktori file yang diakses yaitu 'resources/files/'.
### Experiment: 10K problem
