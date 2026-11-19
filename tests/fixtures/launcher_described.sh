#!/bin/bash
#<launcher-begin>
# title: Described Demo
# version: 1
# args:
#   -c --customer $custid (
#       type: string mandatory
#       default: ""
#       widget-type: sql-query
#       sql_query_str: "select custid,dbservid from cust"
#       sql_disp_field: "$custid ($dbservid)"
#       sql_return_string: "$custid"
#   )
#   -l --logfile $logfile (
#       type: string mandatory
#       default: "/tmp/log"
#       widget-type: none
#   )
#   -v --verbose (
#       type: switch optional
#       widget-type: checkbox
#       widget-label: "Verbose"
#   )
#<launcher-end>
echo "args:$*"
exit 0
