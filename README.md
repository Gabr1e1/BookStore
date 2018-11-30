# Bookstore 设计提示

## Database : 块状链表
/*每个块内: 按照ISBN号储存
每个block都是一个文件
block内储存的东西: n个pair(ISBN, 文件名字)，指针
以上想法完全错误！！
*/
每一个块状链表存在一个文件里
在文件里自己维护地址，自己寻址

## Account

## DataAccess

## GUI

疑问:
1. 要维护多个数据结构?? 对的，一个关键字一个
2. 没有删除？ modify就相当于是删除