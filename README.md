# fumidb
A simple and lightweight database written in C

## Documentation
Most part of this README and the docs in `api` folder is written in Chinese. If you want to use it but have any problem in language, please contact me directly through the issues.

## 支持的功能
- [x] 创建[数据库文件](/api/dbfile.md#数据库文件格式)和[表](/api/dbfile.md#表)
- [ ] 删除表
- [x] 对主键进行[索引](/api/index.md)
- [ ] 对外键进行[索引](/api/index.md)
- [x] [插入表项](/api/table.md#表项的增加)
- [x] 查询表项
- [x] 按索引修改表项
- [ ] 按索引[删除表项](/api/table.md#表项的删除)
- [ ] 对非主键（无[unique约束](/api/types.md#类型修饰符)）进行[索引](/api/index.md)

## 关于并发
不支持并发，如需要并发，请自行加锁以免数据损坏。

## Thanks
- [asciiflow](https://asciiflow.com/)