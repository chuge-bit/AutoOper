#include<iostream>
#include<algorithm>
#include<cstring>
#include<cstdlib>
#include<cmath>
#include<print>

#include<queue>
#include <vector>
#include <list>

#include<fstream>
#include<sstream>//getline
#include<strstream>

#include<windows.h>

//#include<imgui.h>


class Oper;
class Key;
class Mos;
std::list<Oper*> Opers_list;
Oper* LastOper = nullptr;  //to do  考虑优化掉
std::vector<Oper*> Opers_vec;//暂时没用
POINT MOU;
void sleep(int time)//基于可移植性的考量，自定义
{
	Sleep(time);
}


class Oper
{
public:
	virtual ~Oper() {};

	virtual void execute() = 0;
	virtual void modify() = 0;
	virtual void query() = 0;

	virtual void setTime(int t) = 0;
	virtual void setMode(bool m) = 0;
	virtual void setData(int v) {}
	virtual void setData(int a, int b) {}
	virtual Oper& operator=(int t) = 0;
	virtual void write(std::ofstream& out) = 0;
};

class Key :public Oper
{
private:
	int vk;
	bool mode;//true->down;false->up
	int time;
public:
	Key(int val, bool m, int t = 0) :vk(val), mode(m), time(t) {}
	~Key() {}
	Key& operator=(int t) override//<-明确是对基类中一个虚函数的重载
	{
		time = t;
		return *this;
	}
	void down()
	{
		keybd_event(vk, 0, 0, 0);
	}
	void up()
	{
		keybd_event(vk, 0, KEYEVENTF_KEYUP, 0);
	}

	void execute()override
	{
		if (mode)down();
		else	up();
		sleep(time);
	}

	void query()override
	{
		std::println("vk:{} mode:{} time:{}", vk, mode, time);
	}

	void modify() override {}

	void setTime(int t)override
	{
		time = t;
	}
	void setMode(bool m)override
	{
		mode = m;
	}
	void setData(int v)override
	{
		vk = v;
	}

	void write(std::ofstream& out)override{
		int type = 1; // 1表示键盘事件
		int modeset = mode ? 1 : 0; //相当于把bool换成int
		out.write(reinterpret_cast<char*>(&type), sizeof(type));
		out.write(reinterpret_cast<char*>(&vk), sizeof(vk));
		out.write(reinterpret_cast<char*>(&modeset), sizeof(modeset));
		out.write(reinterpret_cast<char*>(&time), sizeof(time));
	}

};
class Mos :public Oper
{
private:
	int vk;//1左键2右键3中键
	int x, y;//不要用POINT->复杂了
	bool mode;//1按下0弹起
	int time;
public:
	Mos(int v = 1, int a = 0, int b = 0, int m = 0, int t = 0)
		:vk(v), x(a), y(b), mode(m), time(t) {
	}
	~Mos() {}
	Mos& operator=(int t) override//<-明确是对基类中一个虚函数的重载
	{
		time = t;
		return *this;
	}
	void down()
	{
		SetCursorPos(x, y);
		switch (vk)
		{
		case 1:
			mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0);
			break;
		case 2:
			mouse_event(MOUSEEVENTF_RIGHTDOWN, 0, 0, 0, 0);
			break;
		case 3:
			mouse_event(MOUSEEVENTF_MIDDLEDOWN, 0, 0, 0, 0);
			break;
		}
	}
	void up()
	{
		SetCursorPos(x, y);
		switch (vk)
		{
		case 1:
			mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
			break;
		case 2:
			mouse_event(MOUSEEVENTF_RIGHTUP, 0, 0, 0, 0);
			break;
		case 3:
			mouse_event(MOUSEEVENTF_MIDDLEUP, 0, 0, 0, 0);
			break;
		}
	}
	void execute() override
	{
		if (mode)down();
		else	up();
		//printf("x:%d y:%d mode:%d time:%d\n", x, y, mode, time);
		sleep(time);
	}

	void query()override
	{
		std::println("vk:{} x:{} y:{} mode:{} time:{}", vk, x, y, mode, time);
	}

	void modify()override {}

	void setTime(int t)override
	{
		time = t;
	}
	void setMode(bool m)override
	{
		mode = m;
	}
	void setData(int a, int b)override
	{
		x = a;
		y = b;
	}
	void setData(int v)override
	{
		vk = v;
	}

	void write(std::ofstream& out)override{
		int type = 0; // 0表示鼠标事件
		int modeset = mode ? 1 : 0; //相当于把bool换成int
		out.write(reinterpret_cast<char*>(&type), sizeof(type));
		out.write(reinterpret_cast<char*>(&vk), sizeof(vk));
		out.write(reinterpret_cast<char*>(&x), sizeof(x));
		out.write(reinterpret_cast<char*>(&y), sizeof(y));
		out.write(reinterpret_cast<char*>(&modeset), sizeof(modeset));
		out.write(reinterpret_cast<char*>(&time), sizeof(time));
	}
};



//**************************"录制"功能->钩子+消息(MFC)实现****
void Get(POINT& MOU)
{
	GetCursorPos(&MOU); // 获取当前鼠标位置
}
// 全局钩子句柄
HHOOK keyHook = nullptr;
HHOOK msHook = nullptr;
int Esc_if = 0;//退出用
static int preTime = 0;  // 事件时间戳--可考虑用windows的DWORD（无符号32位整数） 
											//-->仅考虑，感觉不如unsigned int
int Timeval = 0;


// 键盘事件回调函数       录制核心！！！！！！
LRESULT CALLBACK KeyProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	if (nCode >= 0)
	{ // HC_ACTION 有效事件（nCode>=0）
		// 输出虚拟键码
		KBDLLHOOKSTRUCT* p = reinterpret_cast<KBDLLHOOKSTRUCT*>(lParam);

		//退出要求
		if (p->vkCode == VK_ESCAPE && (p->flags & LLKHF_UP) == 0) //Esc退出
		{
			Esc_if = 1;
			//printf("YES!\n");
			return CallNextHookEx(keyHook, nCode, wParam, lParam);//不记录Esc
		}
		//LLKHF_REPEAT无法识别

// 计算相邻事件时间差
		if (preTime)
		{
			int curTime = p->time;
			Timeval = curTime - preTime;
			if (Timeval > 18e5)//超过半小时
			{
				std::cerr << "记录超时!\n";
				return CallNextHookEx(keyHook, nCode, wParam, lParam);
			}
			std::println("时间间隔: {} ms", Timeval);
		}
		// 更新时间戳
		preTime = p->time;
		Oper* op = nullptr;//其实没必要
		if (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN)
		{
			DWORD evTime = p->time;  // 获取事件时间戳
			std::println("键盘 按下 - 键码: {}",p->vkCode);
			op = new Key(p->vkCode, true);//创建----------------
		}
		else if (wParam == WM_KEYUP || wParam == WM_SYSKEYUP)
		{
			std::println("键盘 弹起 - 键码: {}",p->vkCode);
			op = new Key(p->vkCode, false);
		}

		//处理时间后缀
		if (LastOper != nullptr)
		{
			LastOper->setTime(Timeval);
		}
		LastOper = op;
		Opers_list.push_back(op);

	}
	return CallNextHookEx(keyHook, nCode, wParam, lParam);
}

// 鼠标事件回调函数
LRESULT CALLBACK MouseProc(int nCode, WPARAM wParam, LPARAM lParam)
{

	if (nCode >= 0)
	{
		MSLLHOOKSTRUCT* p = reinterpret_cast<MSLLHOOKSTRUCT*>(lParam);
		Oper* op = nullptr;//这里真有必要
		// 捕获鼠标点击事件
		switch (wParam)
		{
		case WM_LBUTTONDOWN:
			/*std::cout << "鼠标 左键按下 —— 坐标: ("
				<< p->pt.x << ", " << p->pt.y << ")\n";*/
				//不能用pt.x--》坐标不同
				//用Get!!!!
			Get(MOU);
			std::println("鼠标 左键按下 —— 坐标: ({}, {})",MOU.x, MOU.y);
			op = new Mos(1, MOU.x, MOU.y, 1);
			break;
		case WM_RBUTTONDOWN:
			Get(MOU);
			std::println("鼠标 右键按下 —— 坐标: ({}, {})",MOU.x, MOU.y);
			op = new Mos(2, MOU.x, MOU.y, 1);
			break;
		case WM_MBUTTONDOWN:
			Get(MOU);
			std::println("鼠标 中键按下 —— 坐标: ({}, {})",MOU.x, MOU.y);
			op = new Mos(3, MOU.x, MOU.y, 1);
			break;
		case WM_LBUTTONUP:
			Get(MOU);
			std::println("鼠标 左键弹起 —— 坐标: ({}, {})",MOU.x, MOU.y);
			op = new Mos(1, MOU.x, MOU.y, 0);
			break;
		case WM_RBUTTONUP:
			Get(MOU);
			std::println("鼠标 右键弹起 —— 坐标: ({}, {})",MOU.x, MOU.y);
			op = new Mos(2, MOU.x, MOU.y, 0);
			break;
		case WM_MBUTTONUP:
			Get(MOU);
			std::println("鼠标 中键弹起 —— 坐标: ({}, {})",MOU.x, MOU.y);
			op = new Mos(3, MOU.x, MOU.y, 0);
			break;
		}

		if ( wParam != 512 && wParam != 522 ) //不加鼠标移动||滚滚轮
		{
			std::println("wParam:%Iu", wParam);//Iu:windows专用
			if (preTime)
			{
				int curTime = p->time;
				Timeval = curTime - preTime;
				if (Timeval > 18e5)//超过半小时
				{
					std::cerr << "记录超时!\n";
					return CallNextHookEx(msHook, nCode, wParam, lParam);
				}
				std::println("时间间隔: {} ms", Timeval);
			}

			//设置时间间隔
			if (LastOper != nullptr)
			{
				LastOper->setTime(Timeval);
			}
			LastOper = op;
			Opers_list.push_back(op);
			preTime = p->time;
		}
	}
	return CallNextHookEx(msHook, nCode, wParam, lParam);
}

// 安装双钩子（API调用规范）
bool InstallHooks()
{
	HINSTANCE hInstance = GetModuleHandle(nullptr);//获取当前进程实例句柄的标准方法

	// 安装键盘钩子
	keyHook = SetWindowsHookEx(WH_KEYBOARD_LL, KeyProc, hInstance, 0);
	if (!keyHook)
	{
		std::cerr << "键盘钩子安装失败 (错误码: "
			<< GetLastError() << ")" << '\n';
		return false;
	}

	// 安装鼠标钩子
	msHook = SetWindowsHookEx(WH_MOUSE_LL, MouseProc, hInstance, 0);
	if (!msHook)
	{
		std::cerr << "鼠标钩子安装失败 (错误码: "
			<< GetLastError() << ")" << '\n';
		UnhookWindowsHookEx(keyHook);
		return false;
	}

	return true;
}

void Oper_taking()//录制开始
{
	//初始化
	Esc_if = 0;
	Opers_list.clear();
	LastOper = nullptr;
	preTime = 0;//初始化计时器
	Timeval = 0;//初始化时间间隔
	std::println("******************按ESC退出******************\n");
	if (!InstallHooks())
	{//安装钩子(自定义函数)
		std::cerr << "错误：无法安装钩子";
		exit(1);
	}

	// 消息循环
	MSG msg;
	while (1)
	{
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))//尝试PeekMessage(可能遗漏消息?)
		{
			// 处理消息
			TranslateMessage(&msg);
			DispatchMessage(&msg);
			if (msg.message == WM_QUIT) break;
		}
		else
		{
			if (Esc_if)
			{
				std::println("退出");
				break;
			}
			sleep(1);  // 避免占用过高
		}
	}
	// 卸载钩子
	UnhookWindowsHookEx(keyHook);
	UnhookWindowsHookEx(msHook);
	std::println("录制已退出");
}

void QueryAllOpers(const std::list<Oper*>& ops)//遍历查询
{
	for (Oper* op : ops)
	{
		if (op != nullptr)
		{
			op->query();
		}
		else
		{
			std::cerr << "错误:链表含空指针\n";
		}
	}
}

void ExecuteOpers(const std::list<Oper*>& ops)//遍历执行
{
	for (Oper* op : ops)
	{
		if (op != nullptr)
		{
			op->execute();
		}
		else
		{
			std::cerr << "错误:链表含空指针\n";
			return;
		}
	}
}

void DetimeOpers(const std::list<Oper*>& ops,int time)//遍历缩短时间
{
	for (Oper* op : ops)
	{
		if (op != nullptr)
		{
			(*op)=time;
		}
		else
		{
			std::cerr << "错误:链表含空指针\n";
		}
	}
}

//保存功能
void Save(const std::list<Oper*>& ops){
	int sz = static_cast<int>(ops.size()); //超级显示转换
	std::println("Oper count:{}", sz);
	std::ofstream out("../save/data.bin", std::ios::binary | std::ios::trunc);

	out.write(reinterpret_cast<char*>(&sz), sizeof(sz));
	for(Oper* t : ops) t -> write(out);
}

void Load(std::list<Oper*>& ops){

	for(Oper* t : ops)delete t;
    ops.clear();

	std::ifstream in("../save/data.bin", std::ios::binary);
	if(!in){
		std::cerr << "Reading can`t work\n";
		return;
	}

	int ct{0};
	in.read(reinterpret_cast<char*>(&ct), sizeof(ct));
	std::println("{} tokens", ct);
	
	int vk, x, y, mode, time;
	int type{0};
	while(ct--){
		in.read(reinterpret_cast<char*>(&type), sizeof(type));
		if(type){//1 -> Key event
			in.read(reinterpret_cast<char*>(&vk), sizeof(vk));
			in.read(reinterpret_cast<char*>(&mode), sizeof(mode));
			in.read(reinterpret_cast<char*>(&time), sizeof(time));

			ops.push_back(new Key(vk, mode, time));
		}
		else {//Mouse event
			in.read(reinterpret_cast<char*>(&vk), sizeof(vk));
			in.read(reinterpret_cast<char*>(&x), sizeof(x));
			in.read(reinterpret_cast<char*>(&y), sizeof(y));
			in.read(reinterpret_cast<char*>(&mode), sizeof(mode));
			in.read(reinterpret_cast<char*>(&time), sizeof(time));

			ops.push_back(new Mos(vk, x, y, mode, time));
		}
	}
}