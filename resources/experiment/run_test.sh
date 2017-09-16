#!/bin/bash
server_app=$1
for req_type in 'small' 'big'
do
        for i in {1..4}
        do
                result_file="data/$req_type/$server_app/result_$i"
                ab -r -n 20000 -c 10000 http://10.130.34.183:80/$req_type.html > $result_file
                echo $result_file
        done
done

