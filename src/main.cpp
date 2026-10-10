#include"AOper.hpp"
#include"imgui.h"



void Help()
{
    std::println("这是一个基于c++面向windows的自动按键程序");
    sleep(1000);
    std::println("您可以先进行“录制”");
    sleep(1000);
    std::println("录制”会记录您的所有按键操作(除Esc外)和鼠标的按下弹起操作");
    sleep(1000);
    std::println("“录制”按下“Esc”结束后，您可以开始“执行”操作");
    sleep(1000);
    std::println("“执行”会复读您录制的所有操作");
    sleep(1000);
    std::println("“查看或修改录制的操作”可以查看所有操作的虚拟码");
    sleep(1000);
    std::println("进入后输入“1”可以控制所有操作的时间间隔");
    sleep(1000);
    std::println("注意");
    std::println("权限和杀毒软件可能会干扰程序执行");
    std::println("Windows 鼠标的“提高指针精确度”可能会影响功能");
    sleep(1000);
}

void Clean()//清空输入缓存区用
{
    std::cin.ignore();
}


int main(void)
{
    //Windows 鼠标的“提高指针精确度”会影响功能
    //Windows is a piece of shit
    int menu;
    while (1)
    {
        std::println("请选择工具:\n1 ->执行\n2 ->录制\n3 ->帮助\n4 ->查看或修改录制的操作");
        std::println("5 ->保存\n6 ->读取");
        std::println("7 ->退出");
        
        std::cin >> menu;

        int ct = 0, n = 0;//无法写入case->正好省时间
        switch (menu)
        {
        case 1://执行
            std::println("请输入循环次数和延迟执行秒数(如:3 3)(返回则第一项输入-1)");
            scanf("%d%d", &n, &ct);
            
            if (n == -1)break;//-1返回上一级
            if ( !ct )sleep(100);//至少100ms(否则录上回车)<---什么史山
            for (;ct > 0;ct--)
            {
                std::println("{}秒后开始执行", ct);
                sleep(1000);
            }
            for (;n > 0;n--)
            {
                std::println("倒数第{}次循环", n);
                ExecuteOpers(Opers_list);
            }
            Clean();//清空输入缓存区(防止意外操作，如:开着脚本在控制台玩)
            break;

        case 2://录制
            std::println("几秒后开始录制？(整数)");
            std::cin >> ct;
            if (ct == 0)sleep(100);//至少100ms(否则录上回车)
            for (;ct > 0;ct--)
            {
                std::println("{}秒后开始录制", ct);
                sleep(1000);
            }
            Oper_taking();
            Clean();//清空输入缓存区
            QueryAllOpers(Opers_list);
            break;

        case 3://帮助
            Help();
            break;

        case 4://查看或修改录制的操作
            QueryAllOpers(Opers_list);
            std::println("是否需要统一时间?（1->是  0->否）");
            std::cin >> ct;
            if (ct)
            {
                std::println("请输入时间(单位:毫秒)：");
                std::cin >> n;
                DetimeOpers(Opers_list, n);
            }

            break;

        case 5://保存存档
            Save(Opers_list);
            std::println("Done.");
            break;
        
        case 6://读取存档
            Load(Opers_list);
            std::println("Done.");
            break;

        case 7://退出程序
            return 0;
            break;

        default:
            std::println("请输入规定值或格式");
            sleep(1000);
            break;
        }
    }
     

}