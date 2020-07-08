
### BEGIN INIT INFO

# Provides:          Wang

# Required-Start:    $network

# Required-Stop::    $network 

# Should-Start:      $all

# Should-Stop:       $all

# Default-Start:    0 

# Default-Stop:      0 

# Short-Description: agv

# Description:       agv

### END INIT INFO

 



#chkconfig:2345 80 05 --指定在哪几个级别执行，0一般指关机，6指的是重启，其他为正常启动。80为启动的优先级，05为关闭的优先级别    
RETVAL=0  
start()
{ 
	#启动服务的入口函数  
	echo  "simple example service is started..."
      sudo /home/linaro/agv_release/qt_console 1	
}  
      
stop()
{	
	#关闭服务的入口函数  
	echo  "simple example service is stoped..."  
}  
	  
#使用case选择  
case $1 in  
	start)  
		start  #调用上面的start函数
;;  
	stop)  
		stop   #调用上面的stop函数
;;  
	*)  
		echo "error choice ! please input start or stop"
;;  
esac  
exit $RETVA 
		    
