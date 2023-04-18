/*
 *  程序名：crtsurfdata.cpp  本程序用于生成全国气象站点观测的分钟数据。
*/
#include"_public.h"

// 全国气象站点参数结构体
struct st_meteorology {
    char prov_name[31];
    char st_id[11];
    char st_name[31];
    double lat;
    double lon;
    double height;
}; 

vector<struct st_meteorology> vst;

// 全国气象站点分钟观测数据结构
struct st_surfdata
{
  char obtid[11];      // 站点代码。
  char ddatetime[21];  // 数据时间：格式yyyymmddhh24miss
  int  t;              // 气温：单位，0.1摄氏度。
  int  p;              // 气压：0.1百帕。
  int  u;              // 相对湿度，0-100之间的值。
  int  wd;             // 风向，0-360之间的值。
  int  wf;             // 风速：单位0.1m/s
  int  r;              // 降雨量：0.1mm。
  int  vis;            // 能见度：0.1米。
};

vector<struct st_surfdata> vsurfdata;  // 存放全国气象站点分钟观测数据的容器

char curtime[21];
// 把站点参数文件中加载到vstcode容器中。
bool LoadSTCode(const char *inifile);

// 模拟生成全国气象站点分钟观测数据，存放在vsurfdata容器中。
void CrtSurfData();

// 把容器vsurfdata中的全国气象站点分钟观测数据写入文件。
bool CrtSurfFile(const char *outpath, const char *datafmt);

//日志类
CLogFile logfile;

int main(int argc, char* argv[]){
    if(argc != 5) {
        // 如果参数非法，给出帮助文档。
        printf("Example:/home/chen/project/my_idc/c/crtsurfdata /home/chen/project/my_idc/ini/stcode.ini /home/chen/tmp/surfdata /home/chen/log/my_idc/crtsurfdata.log csv,xml,json\n");
        printf("inifile 全国气象站点参数文件名。\n");
        printf("outpath 全国气象站点数据文件存放的目录。\n");
        printf("logfile 本程序运行的日志文件名。\n\n");
        printf("datafmt 生成数据文件的格式, 支持xml、json和csv三种格式, 中间用逗号分隔。\n\n");
        return -1;

    }

    if(logfile.Open(argv[3]) == false) {
        printf("logfile.Open(%s) failed.\n", argv[3]);
        return -1;
    }
    logfile.Write("crtsurfdata 开始运行。\n");

    // 在这里插入处理业务的代码。
    if(LoadSTCode(argv[1]) == false) return -1;

    // 模拟生成全国气象站点分钟观测数据，存放在vsurfdata容器中。
    CrtSurfData();
    
    // 把容器vsurfdata中的全国气象站点分钟观测数据写入文件。
    if (strstr(argv[4],"xml")!=0) CrtSurfFile(argv[2],"xml");
    if (strstr(argv[4],"json")!=0) CrtSurfFile(argv[2],"json");
    if (strstr(argv[4],"csv")!=0) CrtSurfFile(argv[2],"csv");

    logfile.WriteEx("crtsurfdata 运行结束。\n");
    return 0;
}


//把站点参数文件加载到结构体容器中
bool LoadSTCode(const char *inifile){
    CFile file;
    
    if(file.Open(inifile, "r")==false) {
        logfile.Write("File.Open(%s) failed.\n",inifile);
        return false;
    }

    char strBuffer[301];

    CCmdStr CmdStr;

    st_meteorology stcode;

    while(true) {

        //读取文件一行
        if(file.Fgets(strBuffer, 300, true) == false) break;

        //把读到的一行拆分
        CmdStr.SplitToCmd(strBuffer, ",", true);
        
        //扔掉第一行
        if(CmdStr.CmdCount() != 6) continue;

        memset(&stcode, 0, sizeof(st_meteorology));
        CmdStr.GetValue(0, stcode.prov_name, 30);
        CmdStr.GetValue(1, stcode.st_id, 10);
        CmdStr.GetValue(2, stcode.st_name, 30);
        CmdStr.GetValue(3, &stcode.lat);
        CmdStr.GetValue(4,&stcode.lon);
        CmdStr.GetValue(5,&stcode.height);

        vst.push_back(stcode);
    }

    return true;

}


// 模拟生成全国气象站点分钟观测数据，存放在vsurfdata容器中。
void CrtSurfData() {

    // 播随机数种子。
    srand(time(0));
    // 获取当前时间，当作观测时间。
    memset(curtime, 0, sizeof(curtime));
    LocalTime(curtime, "yyyymmddhh24miss");

    struct st_surfdata stsurfdata;

    // 遍历气象站点参数的vstcode容器。
    for(int i = 0; i < vst.size(); i++) {
        memset(&stsurfdata, 0, sizeof(stsurfdata));
        strncpy(stsurfdata.obtid, vst[i].st_id, sizeof(vst[i].st_id));
        strncpy(stsurfdata.ddatetime, curtime, sizeof(curtime));
        stsurfdata.t=rand()%351;       // 气温：单位，0.1摄氏度
        stsurfdata.p=rand()%265+10000; // 气压：0.1百帕
        stsurfdata.u=rand()%100+1;     // 相对湿度，0-100之间的值。
        stsurfdata.wd=rand()%360;      // 风向，0-360之间的值。
        stsurfdata.wf=rand()%150;      // 风速：单位0.1m/s
        stsurfdata.r=rand()%16;        // 降雨量：0.1mm
        stsurfdata.vis=rand()%5001+100000;  // 能见度：0.1米
        vsurfdata.push_back(stsurfdata);
    }
    printf("this is test\n");
}

// 把容器vsurfdata中的全国气象站点分钟观测数据写入文件。
bool CrtSurfFile(const char *outpath,const char *datafmt) {
    CFile File;

    // 拼接生成数据的文件名，例如：/tmp/idc/surfdata/SURF_ZH_20210629092200_2254.csv
    char strFileName[301];
    sprintf(strFileName, "%s/SURF_ZH_%s_%d.%s", outpath, curtime, getpid(), datafmt);

    //open the file 
    if (File.OpenForRename(strFileName, "w") == false) {
        logfile.Write("File.OpenForRename(%s) failed.\n",strFileName);
        return false;
    }

    // write title line, only for csv
    if(strcmp(datafmt, "csv") == 0) {
        File.Fprintf("站点代码,数据时间,气温,气压,相对湿度,风向,风速,降雨量,能见度\n");
    }

    if (strcmp(datafmt,"xml")==0) File.Fprintf("<data>\n");
    
    if (strcmp(datafmt,"json")==0) File.Fprintf("{\"data\":[\n");

    // traverse vsurfdata container
    for (int ii=0;ii<vsurfdata.size();ii++)
    {
        // write one line
        if (strcmp(datafmt,"csv")==0)
            File.Fprintf("%s,%s,%.1f,%.1f,%d,%d,%.1f,%.1f,%.1f\n",\
            vsurfdata[ii].obtid,vsurfdata[ii].ddatetime,vsurfdata[ii].t/10.0,vsurfdata[ii].p/10.0,\
            vsurfdata[ii].u,vsurfdata[ii].wd,vsurfdata[ii].wf/10.0,vsurfdata[ii].r/10.0,vsurfdata[ii].vis/10.0);

        if (strcmp(datafmt,"xml")==0)
            File.Fprintf("<obtid>%s</obtid><ddatetime>%s</ddatetime><t>%.1f</t><p>%.1f</p>"\
                   "<u>%d</u><wd>%d</wd><wf>%.1f</wf><r>%.1f</r><vis>%.1f</vis><endl/>\n",\
            vsurfdata[ii].obtid,vsurfdata[ii].ddatetime,vsurfdata[ii].t/10.0,vsurfdata[ii].p/10.0,\
            vsurfdata[ii].u,vsurfdata[ii].wd,vsurfdata[ii].wf/10.0,vsurfdata[ii].r/10.0,vsurfdata[ii].vis/10.0);

        if (strcmp(datafmt,"json")==0)
        {
            File.Fprintf("{\"obtid\":\"%s\",\"ddatetime\":\"%s\",\"t\":\"%.1f\",\"p\":\"%.1f\","\
                   "\"u\":\"%d\",\"wd\":\"%d\",\"wf\":\"%.1f\",\"r\":\"%.1f\",\"vis\":\"%.1f\"}",\
            vsurfdata[ii].obtid,vsurfdata[ii].ddatetime,vsurfdata[ii].t/10.0,vsurfdata[ii].p/10.0,\
            vsurfdata[ii].u,vsurfdata[ii].wd,vsurfdata[ii].wf/10.0,vsurfdata[ii].r/10.0,vsurfdata[ii].vis/10.0);

        if (ii<vsurfdata.size()-1) File.Fprintf(",\n");
        else   File.Fprintf("\n");
        }  
  }
    
    if (strcmp(datafmt,"xml")==0) File.Fprintf("</data>\n");
    if (strcmp(datafmt,"json")==0) File.Fprintf("]}\n");

    File.CloseAndRename();

    logfile.Write("生成数据文件%s成功, 数据时间%s, 记录数%d。\n",strFileName,curtime,vsurfdata.size());
}