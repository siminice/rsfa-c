#!/bin/csh

curl --get "http://www.soccerway.com/matches/"$1"/" > "/home/radu/rsssf/sat/web/html-daily/"$1".html"

