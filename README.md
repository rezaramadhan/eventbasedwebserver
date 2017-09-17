# Event-based Webserver
## nginx vs apache2 10K Problem Comparison
### Environment
Eksperimen dilakukan pada sebuah droplet DigitalOcean dengan tier paling rendah dengan spesifikasi sebagai berikut:
-   RAM: 512GB
-   CPU: Intel(R) Xeon(R) CPU E5-2630L 1 Core @ 2.40GHz
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


### Data
Data detail untuk setiap eksperimen yang dilakukan dapat dilihat pada direktori `resources/experiment/data`. Pada readme ini akan diberikan nilai rata-rata untuk beberapa variabel penting seperti persentasi request yang berhasil dilayani, jumlah request per second, waktu maksimum, minumum, dan rata-rata yang dihabiskan oleh setiap request, serta penggunaan memori pada server ketika eksperimen berlangsung.
#### 500B file
___nginx___


___apache2___
#### 20KB file
___nginx___

___apache2___

### Analysis

## Event-based Webserver Implementation
### How to Install
### Configuration Used
### Experiment: 10K problem
### Analysis