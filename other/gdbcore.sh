#! /bin/bash

LogFileName="/home/yull/mount/LinuxSharedFolder/epollAndStateMachine/bin/check.log"
ServerLogName=

mklog()
{
    logcontent=$1
    if [ "X$logcontent" == "X" ];then
        echo " " >> $LogFileName
    else
        logtime=`date "+%Y-%m-%d %H:%M:%S"`
        echo -e "[$logtime]-[$logcontent]" >> $LogFileName
    fi
}

APPID=`ps -fu $USER | grep "Server" | grep -v grep | awk '{print $2}'`

if [ "X$APPID" = "X" ];then
    exit;
fi

ErrorInfo="SvcServer resource is Exhaustion"
#errorCount=`grep -c "ErrorInfo" $ServerLogName`
#if [ $errorCount -ne 0 ];then
    gcore $APPID
    sleep 1

    for i in `seq 1 4`;do
        logtime=`date "+%Y-%m-%d %H:%M:%S"`
        echo -e "[$logtime]-[Begin get thread info]" >> $LogFileName.$i
        gstack $APPID >> ${LogFileName}.$i
        sleep 1
    done

    kill -9 $APPID
#fi
