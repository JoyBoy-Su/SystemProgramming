# lab1-shell

实现一个简单的shell程序以读取命令行，能够完成执行用户输入的命令、I/O重定向、管道操作；

##### 项目结构

```
lab1-shell
|	README.md
|	.gitignore
|   CMakeLists.txt    
|--- doc（项目文档）
|   |   Shell.md
|   |	设计文档.md
|   |---imgs
|       |   ...
|--- src（shell源代码）
|	|---cat
|	|	|	main.c
|	|---ls
|	|	|	main.c
|	|---echo
|	|	|	main.c
|	|---mkdir
|	|	|	main.c
|	|---pwd
|	|	|	main.c
|   |	command.h
|	|	command.c
|	|	shell.c
|	|	CMakeLists.txt
|--- test（测试代码）
|	...
```

##### 运行

项目运行由cmake管理，需要配置cmake的Linux环境。进入项目根目录后执行如下指令：

```shell
mkdir build
cd build
cmake ..
make
```

执行过`cmake .. `指令后若项目结构不变则不需要重新执行，只需要执行make即可。

`make`会生成如下三部分的可执行文件：

- shell
- shell执行依赖的指令包
- 单元测试文件
