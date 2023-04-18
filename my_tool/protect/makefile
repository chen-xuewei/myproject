# 开发框架头文件路径。
PUBINCL = -I/home/chen/project/public

# 开发框架cpp文件名，这里直接包含进来，没有采用链接库，是为了方便调试。
PUBCPP = /home/chen/project/public/_public.cpp

STD = -std=c++11

# 编译参数。
CFLAGS = -g 

all:pprotect

pprotect:
	g++  $(CFLAGS) pprotect.cpp $(PUBINCL) $(PUBCPP) $(STD) -o pprotect