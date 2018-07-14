#!/bin/sh                                                                                                                                                                                    

set -e 

get_stats()
{
        printf "%-20s" $1
        grep "${1}" /tmp/renderer.log | awk '{print $5}' | awk '{if(min==""){min=max=$1};if($1>max) {max=$1}; if($1<min) {min=$1}; total+=$1; count+=1} END {print total/count, max, min, total}' 
}
get_totals()
{
        printf "%-20s" $1
        grep "2018-06" /tmp/renderer.log | awk '{print $6}' | awk '{if(min==""){min=max=$1}; if($1>max) {max=$1}; if($1<min) {min=$1}; total+=$1; count+=1} END {print total/count, max, min, total}' 
}

echo "${0}: avg max min total"
get_stats "parse_page_spec" 
get_stats "file_exists" 
get_stats "get_query_result" 
get_stats "render_template" 
get_stats "write_page" 
get_stats "write_pjax" 
get_stats "cleanup" 
get_totals


