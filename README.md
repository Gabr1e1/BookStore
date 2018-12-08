# Bookstore 设计提示

## Database : 块状链表
~~每个块内: 按照ISBN号储存~~~~
~~每个block都是一个文件~~
~~block内储存的东西: n个pair(ISBN, 文件名字)，指针~~
~~以上想法完全错误！！~~~~
每一个块状链表存在一个文件里
在文件里自己维护地址，自己寻址

疑问:
1. 要维护多个数据结构?? 对的，一个关键字一个
2. 没有删除？ ~~modify就相当于是删除,~~ ~~好像还是不需要删除。。要么原地覆盖，要么直接插入~~ 完全错误，压根儿覆盖不上去。。关键字得要是有序的

Task list:
1. Add const specifier
2. Add & specifier (write : const char*)
3. **把Account System 和 Finance System 做成继承一个基类System的 ** ✔
4. 判断是否已经存在改文件 ✔
5. 多次modify合并为一次
6. size of curSelected should always be one

Note:

1. 读取或者写入的时候错误都会直接让这个fstream不可用！

2. std::ios::beg, end, in之类的全都是一个数值，不能直接seekg(std::ios::beg), 这就相当于在seekg一个普通的数



