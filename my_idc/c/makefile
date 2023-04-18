# 开发框架头文件路径。
PUBINCL = -I/home/chen/project/public

# 开发框架cpp文件名，这里直接包含进来，没有采用链接库，是为了方便调试。
PUBCPP = /home/chen/project/public/_public.cpp

# mysql头文件存放路径
MYSQLINCL = -I/usr/local/mysql/include  -I/project/public/db/mysql

# mysql库文件保存路径
MYSQLLIB = -L/usr/local/mysql/lib

# mysql链接库
MYSQLLIBS = -lmysqlclientpwd

# 开发框架mysql的cpp文件名，这里直接包含进来，没有采用链接库，是为了方便调试。
MYSQLCPP = /home/chen/project/public/db/mysql/_mysql.cpp

# 编译参数。
CFLAGS = -g 

all:crtsurfdata

crtsurfdata:crtsurfdata.cpp
	g++  $(CFLAGS) crtsurfdata.cpp $(PUBINCL) $(PUBCPP) -lm -lc -o crtsurfdata

clean:
	rm crtsurfdata