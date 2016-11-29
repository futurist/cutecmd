# cutecmd

程序员命令启动器，与MacOS下的 Alfred / QuickSilver 类似。

显示一个类似于**运行**命令的对话框，但更快速，且**脚本优先**

项目目标：

 - **脚本优先**，若找到[AutoIt](https://www.autoitscript.com/site/autoit/)脚本，优先运行，否则运行普通命令行
 - 运行程序，或切换至
 - 小巧(目前是8K)，低内存占用

![cutecmd snapshot](../screenshot.gif)


## 安装

1. 下载 AutoIt [官方](https://www.autoitscript.com/site/autoit/downloads/) / [太平洋下载](http://dl.pconline.com.cn/download/1057.html), 默认选项安装.

2. 拷贝 **shortcuts** 目录到任意盘(D:), 如: `D:\shortcuts\`

3. 添加 `;D:\shortcuts\` 至 **PATH** 环境变量的结尾

完成，可以使用了。使用方法如下

## 使用

运行 **cutecmd_x86.exe**(32位Windows) or **cutecmd_x64.exe**(64位Windows), **Windows 7+ 需管理员权限**

**双击** **Ctrl键** 会打开一个窗口

输入 `shortcuts` 中相匹配的文件名, 如: `st` 表示Sublime Text

按 <kbd>Space</kbd> 或 <kbd>Enter</kbd> 确定; 按 <kbd>Ctrl+G</kbd> 或 <kbd>Esc</kbd> 取消


## Open.au3

注意每个快捷方式的 **路径**, <kbd>Alt+Enter</kbd> 打开并检查 `open.au3` 参数.

**open.au3 参数**

1. 可执行文件路径

2. 窗口匹配方法: **title** 或 **class**

3. 窗口匹配关键字

例子: `C:\WINDOWS\open.au3 "chrome.exe" title "Google Chrome"`


## 编译

要编辑`cutecmd`, 使用 [TCC](http://bellard.org/tcc/) 进行，目的是要小。

导入 `shell32.dll` 中的API定义，如下

    C:\tcc>tiny_impdef.exe c:\WINDOWS\system32\shell32.dll
    move shell32.def lib\

编译:

    tcc cutecmd.c -lshell32






